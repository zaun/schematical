import Foundation

// Undo and redo (plan-31, plan-61-A). A document has one undo manager. A step
// is the per-item difference between the document's saved state before and
// after one action; steps undo in the order they were made, whichever page
// made them.

/// The saved part of a document, as a value. Arrays are copy-on-write, so
/// taking one is cheap.
struct DocumentState: Equatable {
    var chips: [PlacedChip] = []
    var wires: [Wire] = []
    var junctions: [Junction] = []
    var buses: [Bus] = []
    var notes: [SchematicNote] = []
    var netLabels: [NetLabel] = []
    var nextReference = 1
    var board = BoardSettings()
    var texts: [BoardText] = []
    var traces: [BoardTrace] = []
    var vias: [BoardVia] = []
    var holes: [BoardHole] = []
    var pours: [BoardPour] = []
    var keepouts: [BoardKeepout] = []
    var cutouts: [BoardCutout] = []
    /// The electrical rules check's settings (plan-63-E).
    var erc = ErcSettings()
    var documentInfo = DocumentInfo()
}

/// One thing a step can change. A part is split three ways, so a step holds
/// only the side of it that it changed: `.part` (everything the Schematic
/// shows), `.reference` (shown on both) and `.footprint` (what the PCB shows).
enum UndoKey: Hashable {
    case part(UUID), reference(UUID), footprint(UUID)
    case wire(UUID), junction(UUID), bus(UUID), note(UUID), label(UUID), nextReference
    case text(UUID), trace(UUID), via(UUID), hole(UUID), pour(UUID), keepout(UUID), cutout(UUID), board, erc, documentInfo
}

/// One key's value on one side of a step. Items carry their array index so a
/// deleted item goes back where it was (clamped to the current count).
enum UndoValue: Equatable {
    /// For `.part`, `.reference` and `.footprint`: the whole chip.
    case chip(PlacedChip, index: Int)
    case wire(Wire, index: Int), junction(Junction, index: Int)
    case bus(Bus, index: Int), note(SchematicNote, index: Int), label(NetLabel, index: Int)
    case text(BoardText, index: Int), trace(BoardTrace, index: Int), via(BoardVia, index: Int)
    case hole(BoardHole, index: Int)
    case pour(BoardPour, index: Int), keepout(BoardKeepout, index: Int), cutout(BoardCutout, index: Int)
    case nextReference(Int), board(BoardSettings), erc(ErcSettings), documentInfo(DocumentInfo)
}

/// A key's value before and after a step. Nil is "not there".
struct UndoChange: Equatable {
    var before: UndoValue?
    var after: UndoValue?
}

typealias UndoChanges = [UndoKey: UndoChange]

/// One undo step. A class, so typing can fold into the step already on the
/// undo manager. Nonisolated: freeing a main-actor class goes through the
/// isolated-deinit path that aborts the test host (see `TestSupport`).
nonisolated final class UndoStep {
    let name: String
    let coalescing: String?
    var changes: UndoChanges

    init(name: String, coalescing: String? = nil, changes: UndoChanges) {
        self.name = name
        self.coalescing = coalescing
        self.changes = changes
    }
}

// MARK: - Diff and apply

enum UndoDiff {
    /// A chip as the `.part` key sees it: without its board placement and
    /// reference.
    private static func partFields(_ chip: PlacedChip) -> PlacedChip {
        var chip = chip
        chip.board = nil
        chip.reference = ""
        return chip
    }

    /// What the `.footprint` key covers.
    private static func sameFootprint(_ a: PlacedChip, _ b: PlacedChip) -> Bool {
        a.board == b.board && a.footprintID == b.footprintID && a.part.name == b.part.name && a.socket == b.socket
    }

    /// Every key whose value differs. `.part` compares the chip with `board`
    /// set to nil and `reference` set to ""; `.reference` compares
    /// `reference`; `.footprint` compares (`board`, `footprintID`,
    /// `part.name`). A chip that appears or disappears changes all three.
    static func changes(from old: DocumentState, to new: DocumentState) -> UndoChanges {
        var changes = UndoChanges()
        if old.chips != new.chips {
            let before = indexed(old.chips), after = indexed(new.chips)
            for id in Set(before.keys).union(after.keys) {
                let b = before[id], a = after[id]
                let bv = b.map { UndoValue.chip($0.item, index: $0.index) }
                let av = a.map { UndoValue.chip($0.item, index: $0.index) }
                let change = UndoChange(before: bv, after: av)
                guard let b, let a else {
                    changes[.part(id)] = change
                    changes[.reference(id)] = change
                    changes[.footprint(id)] = change
                    continue
                }
                if partFields(b.item) != partFields(a.item) { changes[.part(id)] = change }
                if b.item.reference != a.item.reference { changes[.reference(id)] = change }
                if !sameFootprint(b.item, a.item) { changes[.footprint(id)] = change }
            }
        }
        diff(old.wires, new.wires, key: UndoKey.wire, wrap: UndoValue.wire, into: &changes)
        diff(old.junctions, new.junctions, key: UndoKey.junction, wrap: UndoValue.junction, into: &changes)
        diff(old.buses, new.buses, key: UndoKey.bus, wrap: UndoValue.bus, into: &changes)
        diff(old.notes, new.notes, key: UndoKey.note, wrap: UndoValue.note, into: &changes)
        diff(old.netLabels, new.netLabels, key: UndoKey.label, wrap: UndoValue.label, into: &changes)
        diff(old.texts, new.texts, key: UndoKey.text, wrap: UndoValue.text, into: &changes)
        diff(old.traces, new.traces, key: UndoKey.trace, wrap: UndoValue.trace, into: &changes)
        diff(old.vias, new.vias, key: UndoKey.via, wrap: UndoValue.via, into: &changes)
        diff(old.holes, new.holes, key: UndoKey.hole, wrap: UndoValue.hole, into: &changes)
        diff(old.pours, new.pours, key: UndoKey.pour, wrap: UndoValue.pour, into: &changes)
        diff(old.keepouts, new.keepouts, key: UndoKey.keepout, wrap: UndoValue.keepout, into: &changes)
        diff(old.cutouts, new.cutouts, key: UndoKey.cutout, wrap: UndoValue.cutout, into: &changes)
        if old.nextReference != new.nextReference {
            changes[.nextReference] = UndoChange(before: .nextReference(old.nextReference),
                                                 after: .nextReference(new.nextReference))
        }
        if old.board != new.board {
            changes[.board] = UndoChange(before: .board(old.board), after: .board(new.board))
        }
        if old.erc != new.erc {
            changes[.erc] = UndoChange(before: .erc(old.erc), after: .erc(new.erc))
        }
        if old.documentInfo != new.documentInfo {
            changes[.documentInfo] = UndoChange(before: .documentInfo(old.documentInfo),
                                                after: .documentInfo(new.documentInfo))
        }
        return changes
    }

    private static func indexed<T: Identifiable>(_ items: [T]) -> [UUID: (item: T, index: Int)] where T.ID == UUID {
        var map: [UUID: (item: T, index: Int)] = [:]
        for (index, item) in items.enumerated() { map[item.id] = (item, index) }
        return map
    }

    /// Items differ by value only; a plain reorder (an index change alone)
    /// is not a change, since no edit reorders without changing.
    private static func diff<T: Identifiable & Equatable>(
        _ old: [T], _ new: [T], key: (UUID) -> UndoKey, wrap: (T, Int) -> UndoValue,
        into changes: inout UndoChanges
    ) where T.ID == UUID {
        guard old != new else { return }
        let before = indexed(old), after = indexed(new)
        for id in Set(before.keys).union(after.keys) {
            let b = before[id], a = after[id]
            if let b, let a, b.item == a.item { continue }
            changes[key(id)] = UndoChange(before: b.map { wrap($0.item, $0.index) },
                                          after: a.map { wrap($0.item, $0.index) })
        }
    }

    /// Writes `before` (undo) or `after` (redo) of each change into `state`.
    /// `.part` keys first (removals, then insertions by index, then field
    /// copies), then `.reference` and `.footprint` (skipped when the part is
    /// absent), then the other keys. Returns whether `state` changed.
    @discardableResult
    static func apply(_ changes: UndoChanges, backwards: Bool, to state: inout DocumentState) -> Bool {
        let original = state
        func target(_ change: UndoChange) -> UndoValue? { backwards ? change.before : change.after }

        var parts: [(id: UUID, value: UndoValue?)] = []
        var references: [(UUID, PlacedChip)] = [], footprints: [(UUID, PlacedChip)] = []
        var wires: [(UUID, UndoValue?)] = [], junctions: [(UUID, UndoValue?)] = []
        var buses: [(UUID, UndoValue?)] = [], notes: [(UUID, UndoValue?)] = []
        var texts: [(UUID, UndoValue?)] = [], traces: [(UUID, UndoValue?)] = []
        var vias: [(UUID, UndoValue?)] = [], labels: [(UUID, UndoValue?)] = []
        var holes: [(UUID, UndoValue?)] = []
        var pours: [(UUID, UndoValue?)] = [], keepouts: [(UUID, UndoValue?)] = [], cutouts: [(UUID, UndoValue?)] = []
        for (key, change) in changes {
            let value = target(change)
            switch key {
            case let .part(id): parts.append((id, value))
            case let .reference(id):
                if case let .chip(chip, _)? = value { references.append((id, chip)) }
            case let .footprint(id):
                if case let .chip(chip, _)? = value { footprints.append((id, chip)) }
            case let .wire(id): wires.append((id, value))
            case let .junction(id): junctions.append((id, value))
            case let .bus(id): buses.append((id, value))
            case let .note(id): notes.append((id, value))
            case let .label(id): labels.append((id, value))
            case let .text(id): texts.append((id, value))
            case let .trace(id): traces.append((id, value))
            case let .via(id): vias.append((id, value))
            case let .hole(id): holes.append((id, value))
            case let .pour(id): pours.append((id, value))
            case let .keepout(id): keepouts.append((id, value))
            case let .cutout(id): cutouts.append((id, value))
            case .nextReference:
                if case let .nextReference(n)? = value { state.nextReference = n }
            case .board:
                if case let .board(b)? = value { state.board = b }
            case .erc:
                if case let .erc(e)? = value { state.erc = e }
            case .documentInfo:
                if case let .documentInfo(i)? = value { state.documentInfo = i }
            }
        }

        // Parts: their own fields, keeping today's board and reference.
        place(parts, in: &state.chips, unwrap: { if case let .chip(c, i) = $0 { (c, i) } else { nil } },
              merge: { stored, current in
                  var chip = stored
                  chip.board = current.board
                  chip.reference = current.reference
                  return chip
              })
        for (id, stored) in references {
            if let index = state.chips.firstIndex(where: { $0.id == id }) {
                state.chips[index].reference = stored.reference
            }
        }
        for (id, stored) in footprints {
            if let index = state.chips.firstIndex(where: { $0.id == id }) {
                state.chips[index].board = stored.board
                state.chips[index].footprintID = stored.footprintID
                state.chips[index].socket = stored.socket
            }
        }
        place(wires, in: &state.wires, unwrap: { if case let .wire(w, i) = $0 { (w, i) } else { nil } })
        place(junctions, in: &state.junctions, unwrap: { if case let .junction(j, i) = $0 { (j, i) } else { nil } })
        place(buses, in: &state.buses, unwrap: { if case let .bus(b, i) = $0 { (b, i) } else { nil } })
        place(notes, in: &state.notes, unwrap: { if case let .note(n, i) = $0 { (n, i) } else { nil } })
        place(labels, in: &state.netLabels, unwrap: { if case let .label(l, i) = $0 { (l, i) } else { nil } })
        place(texts, in: &state.texts, unwrap: { if case let .text(t, i) = $0 { (t, i) } else { nil } })
        place(traces, in: &state.traces, unwrap: { if case let .trace(t, i) = $0 { (t, i) } else { nil } })
        place(vias, in: &state.vias, unwrap: { if case let .via(v, i) = $0 { (v, i) } else { nil } })
        place(holes, in: &state.holes, unwrap: { if case let .hole(h, i) = $0 { (h, i) } else { nil } })
        place(pours, in: &state.pours, unwrap: { if case let .pour(p, i) = $0 { (p, i) } else { nil } })
        place(keepouts, in: &state.keepouts, unwrap: { if case let .keepout(k, i) = $0 { (k, i) } else { nil } })
        place(cutouts, in: &state.cutouts, unwrap: { if case let .cutout(c, i) = $0 { (c, i) } else { nil } })
        return state != original
    }

    /// Removes the items whose target is nil, inserts the missing ones at
    /// their index (in index order, clamped to the count), then replaces the
    /// rest in place through `merge`.
    private static func place<T: Identifiable & Equatable>(
        _ targets: [(id: UUID, value: UndoValue?)], in items: inout [T],
        unwrap: (UndoValue) -> (T, Int)?, merge: (T, T) -> T = { stored, _ in stored }
    ) where T.ID == UUID {
        guard !targets.isEmpty else { return }
        let removed = Set(targets.filter { $0.value == nil }.map(\.id))
        if !removed.isEmpty { items.removeAll { removed.contains($0.id) } }
        let present = targets.compactMap { target in target.value.flatMap(unwrap) }
        var inserts: [(T, Int)] = [], replaces: [T] = []
        for (item, index) in present {
            if items.contains(where: { $0.id == item.id }) { replaces.append(item) } else { inserts.append((item, index)) }
        }
        for (item, index) in inserts.sorted(by: { $0.1 < $1.1 }) {
            items.insert(item, at: min(max(index, 0), items.count))
        }
        for item in replaces {
            guard let index = items.firstIndex(where: { $0.id == item.id }) else { continue }
            let merged = merge(item, items[index])
            if items[index] != merged { items[index] = merged }
        }
    }
}

// MARK: - The document's undo manager

/// A document's undo manager (plan-61-A). One recorded step is one group
/// (`groupsByEvent` is off, so no run loop turn is needed to close one). While
/// the simulation runs it is locked: `canUndo` and `canRedo` are false, which
/// disables any menu item validated against it. `undo()` and `redo()` skip a
/// step that no longer changes anything and go on to the next.
nonisolated final class SchematicUndoManager: UndoManager, @unchecked Sendable {
    /// True while the simulation runs.
    var isLocked = false
    /// Set by a step's handler when it changed the document.
    var applied = false

    override init() {
        super.init()
        groupsByEvent = false
        levelsOfUndo = 500
    }

    override var canUndo: Bool { !isLocked && super.canUndo }
    override var canRedo: Bool { !isLocked && super.canRedo }

    override func undo() {
        applied = false
        while !isLocked, !applied, super.canUndo { super.undo() }
    }

    override func redo() {
        applied = false
        while !isLocked, !applied, super.canRedo { super.redo() }
    }

    /// Takes the top undo step off, for one whose changes have become empty:
    /// its handler changes nothing and registers nothing.
    func dropTop() {
        if super.canUndo { super.undo() }
    }
}
