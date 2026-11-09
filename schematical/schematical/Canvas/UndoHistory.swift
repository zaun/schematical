import Foundation

// Undo and redo (plan-31). The Schematic page and the PCB page each keep an
// undo list and a redo list. A step is the per-item difference between the
// document's saved state before and after one action, so undoing it on one
// page never reverts what the other page did to other items since.

/// The saved part of a document, as a value. Arrays are copy-on-write, so
/// taking one is cheap.
struct DocumentState: Equatable {
    var chips: [PlacedChip] = []
    var wires: [Wire] = []
    var junctions: [Junction] = []
    var buses: [Bus] = []
    var notes: [SchematicNote] = []
    var nextReference = 1
    var board = BoardSettings()
    var texts: [BoardText] = []
    var traces: [BoardTrace] = []
}

/// One thing a step can change. A part is split three ways, so the two pages
/// can undo their own side of it apart: `.part` (everything the Schematic
/// shows), `.reference` (shown on both) and `.footprint` (what the PCB shows).
enum UndoKey: Hashable {
    case part(UUID), reference(UUID), footprint(UUID)
    case wire(UUID), junction(UUID), bus(UUID), note(UUID), nextReference
    case text(UUID), trace(UUID), board

    /// The pages that show this key. A step goes on the list of each.
    var pages: Set<EditorPage> {
        switch self {
        case .part, .wire, .junction, .bus, .note, .nextReference: [.schematic]
        case .reference: [.schematic, .pcb]
        case .footprint, .text, .trace, .board: [.pcb]
        }
    }
}

/// One key's value on one side of a step. Items carry their array index so a
/// deleted item goes back where it was (clamped to the current count).
enum UndoValue: Equatable {
    /// For `.part`, `.reference` and `.footprint`: the whole chip.
    case chip(PlacedChip, index: Int)
    case wire(Wire, index: Int), junction(Junction, index: Int)
    case bus(Bus, index: Int), note(SchematicNote, index: Int)
    case text(BoardText, index: Int), trace(BoardTrace, index: Int)
    case nextReference(Int), board(BoardSettings)
}

/// A key's value before and after a step. Nil is "not there".
struct UndoChange: Equatable {
    var before: UndoValue?
    var after: UndoValue?
}

typealias UndoChanges = [UndoKey: UndoChange]

/// One undo step. A class, so a step on both pages is one object on both
/// pages' stacks. Nonisolated: freeing a main-actor class goes through the
/// isolated-deinit path that aborts the test host (see `TestSupport`).
nonisolated final class UndoStep {
    let name: String
    let coalescing: String?
    var changes: UndoChanges
    var pages: Set<EditorPage>

    init(name: String, coalescing: String? = nil, changes: UndoChanges, pages: Set<EditorPage>) {
        self.name = name
        self.coalescing = coalescing
        self.changes = changes
        self.pages = pages
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
        a.board == b.board && a.footprintID == b.footprintID && a.part.name == b.part.name
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
        diff(old.texts, new.texts, key: UndoKey.text, wrap: UndoValue.text, into: &changes)
        diff(old.traces, new.traces, key: UndoKey.trace, wrap: UndoValue.trace, into: &changes)
        if old.nextReference != new.nextReference {
            changes[.nextReference] = UndoChange(before: .nextReference(old.nextReference),
                                                 after: .nextReference(new.nextReference))
        }
        if old.board != new.board {
            changes[.board] = UndoChange(before: .board(old.board), after: .board(new.board))
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
            case let .text(id): texts.append((id, value))
            case let .trace(id): traces.append((id, value))
            case .nextReference:
                if case let .nextReference(n)? = value { state.nextReference = n }
            case .board:
                if case let .board(b)? = value { state.board = b }
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
            }
        }
        place(wires, in: &state.wires, unwrap: { if case let .wire(w, i) = $0 { (w, i) } else { nil } })
        place(junctions, in: &state.junctions, unwrap: { if case let .junction(j, i) = $0 { (j, i) } else { nil } })
        place(buses, in: &state.buses, unwrap: { if case let .bus(b, i) = $0 { (b, i) } else { nil } })
        place(notes, in: &state.notes, unwrap: { if case let .note(n, i) = $0 { (n, i) } else { nil } })
        place(texts, in: &state.texts, unwrap: { if case let .text(t, i) = $0 { (t, i) } else { nil } })
        place(traces, in: &state.traces, unwrap: { if case let .trace(t, i) = $0 { (t, i) } else { nil } })
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

// MARK: - The stacks

/// Each page's undo stack and redo stack. A step on two pages is on both
/// pages' stacks of the same kind at once.
struct UndoHistory {
    private(set) var undo: [EditorPage: [UndoStep]] = [:]
    private(set) var redo: [EditorPage: [UndoStep]] = [:]
    /// Whether the top step may still take in a push with its coalescing key.
    private(set) var coalescingOpen = false

    /// Steps kept per page undo stack.
    static let limit = 500

    func undoTop(on page: EditorPage) -> UndoStep? { undo[page]?.last }
    func redoTop(on page: EditorPage) -> UndoStep? { redo[page]?.last }

    /// Adds a step to the undo stack of each of its pages and clears those
    /// pages' redo stacks (a shared step cleared this way leaves every
    /// page's redo stack). A step with the same coalescing key as the top
    /// step on its page, while coalescing is open, merges into it instead.
    mutating func push(_ step: UndoStep) {
        clearRedo(on: step.pages)
        let tops = step.pages.compactMap { undo[$0]?.last }
        if let key = step.coalescing, coalescingOpen, let top = tops.first(where: { $0.coalescing == key }) {
            merge(step, into: top)
            return
        }
        coalescingOpen = step.coalescing != nil
        for page in step.pages {
            undo[page, default: []].append(step)
            if undo[page]!.count > Self.limit {
                let dropped = undo[page]!.removeFirst()
                // Kept on the other page, if it is on one; its keys are per item.
                dropped.pages.remove(page)
            }
        }
    }

    private mutating func merge(_ newer: UndoStep, into top: UndoStep) {
        for (key, change) in newer.changes {
            if let older = top.changes[key] {
                top.changes[key] = UndoChange(before: older.before, after: change.after)
            } else {
                top.changes[key] = change
            }
        }
        top.changes = top.changes.filter { $0.value.before != $0.value.after }
        for page in newer.pages.subtracting(top.pages) {
            top.pages.insert(page)
            undo[page, default: []].append(top)
        }
        if top.changes.isEmpty { remove(top, from: \.undo) }
    }

    private mutating func clearRedo(on pages: Set<EditorPage>) {
        let gone = pages.flatMap { redo[$0] ?? [] }
        for step in gone { remove(step, from: \.redo) }
    }

    private mutating func remove(_ step: UndoStep, from stacks: WritableKeyPath<UndoHistory, [EditorPage: [UndoStep]]>) {
        for page in step.pages {
            self[keyPath: stacks][page]?.removeAll { $0 === step }
        }
    }

    /// Takes the top undo step of `page` off the undo stack of each of its pages.
    mutating func popUndo(on page: EditorPage) -> UndoStep? {
        coalescingOpen = false
        guard let step = undo[page]?.last else { return nil }
        remove(step, from: \.undo)
        return step
    }

    /// Takes the top redo step of `page` off the redo stack of each of its pages.
    mutating func popRedo(on page: EditorPage) -> UndoStep? {
        coalescingOpen = false
        guard let step = redo[page]?.last else { return nil }
        remove(step, from: \.redo)
        return step
    }

    /// Puts an undone step on top of each of its pages' redo stacks.
    mutating func pushRedo(_ step: UndoStep) {
        for page in step.pages { redo[page, default: []].append(step) }
    }

    /// Puts a redone step back on top of each of its pages' undo stacks.
    mutating func pushUndo(_ step: UndoStep) {
        for page in step.pages { undo[page, default: []].append(step) }
    }

    /// The next push starts a new step even with the same coalescing key.
    mutating func endCoalescing() {
        coalescingOpen = false
    }

    mutating func clear() {
        undo = [:]
        redo = [:]
        coalescingOpen = false
    }
}
