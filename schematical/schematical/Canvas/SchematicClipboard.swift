import CoreGraphics
import Foundation

/// What ⌘C took from the Schematic: the selected parts, net labels, text
/// blocks, junctions and wires, plus every wire that runs between copied
/// items, as they were. A wire end that was on something not copied (a pin
/// of another part, a label not copied, a bus port pin) is a loose
/// junction at that spot instead, so the copy stands alone. Buses and
/// ports are never copied. Ids are the originals'; a paste gives each item
/// a new one (`SchematicDocument.pasteSchematic`).
nonisolated struct SchematicCopy: Equatable {
    var chips: [PlacedChip] = []
    var wires: [Wire] = []
    var junctions: [Junction] = []
    var notes: [SchematicNote] = []
    var labels: [NetLabel] = []
    /// Everything copied, in canvas units: what a paste centres on the
    /// pointer.
    var bounds: CGRect = .null

    var isEmpty: Bool {
        chips.isEmpty && wires.isEmpty && junctions.isEmpty && notes.isEmpty && labels.isEmpty
    }
}

/// The Schematic's clipboard, shared by every window of the app. A paste
/// lands centred on the pointer (`offset(centring:on:)`); with the pointer
/// off the canvas, each paste lands one more `step` right and down from
/// the copy instead (`nextOffset`).
@MainActor
enum SchematicClipboard {
    private(set) static var contents: SchematicCopy?
    /// Pastes since the last copy.
    private(set) static var pastes = 0
    /// Two grid steps.
    static let step: CGFloat = 2 * SchematicGrid.spacing

    static func copy(_ copied: SchematicCopy) {
        contents = copied
        pastes = 0
    }

    /// The offset that puts the middle of the copy on `point`, in whole
    /// grid steps so everything stays on the grid.
    static func offset(centring copied: SchematicCopy, on point: CGPoint) -> CGSize {
        guard !copied.bounds.isNull else { return .zero }
        return CGSize(width: SchematicGrid.snap(point.x - copied.bounds.midX),
                      height: SchematicGrid.snap(point.y - copied.bounds.midY))
    }

    /// The offset for the next paste with the pointer off the canvas,
    /// counting it.
    static func nextOffset() -> CGSize {
        pastes += 1
        let d = step * CGFloat(pastes)
        return CGSize(width: d, height: d)
    }
}

extension SchematicDocument {
    /// The Schematic selection as a copy (`SchematicCopy`); nil when
    /// nothing copyable is selected.
    func schematicCopy() -> SchematicCopy? {
        var chipIDs = Set<UUID>(), labelIDs = Set<UUID>(), noteIDs = Set<UUID>()
        var junctionIDs = Set<UUID>(), wireIDs = Set<UUID>()
        for item in schematicSelection.items {
            switch item {
            case let .chip(id): chipIDs.insert(id)
            case let .label(id): labelIDs.insert(id)
            case let .note(id): noteIDs.insert(id)
            case let .junction(id): junctionIDs.insert(id)
            case let .wire(id): wireIDs.insert(id)
            }
        }
        func inside(_ end: WireEnd) -> Bool {
            switch end {
            case let .pin(ref): chipIDs.contains(ref.chipID)
            case let .label(id): labelIDs.contains(id)
            case let .junction(id): junctionIDs.contains(id)
            case .busPin: false
            }
        }
        // Wires that run between copied items come along.
        for wire in wires where inside(wire.start) && inside(wire.end) {
            wireIDs.insert(wire.id)
        }

        var copy = SchematicCopy()
        copy.chips = chips.filter { chipIDs.contains($0.id) }
        copy.labels = netLabels.filter { labelIDs.contains($0.id) }
        copy.notes = notes.filter { noteIDs.contains($0.id) }
        // A junction a copied wire ends on comes along too.
        for wire in wires where wireIDs.contains(wire.id) {
            for end in [wire.start, wire.end] {
                if case let .junction(id) = end { junctionIDs.insert(id) }
            }
        }
        copy.junctions = junctions.filter { junctionIDs.contains($0.id) }

        // Ends on anything not copied become loose junctions, one per spot.
        var looseAt: [CGPoint: UUID] = [:]
        func kept(_ end: WireEnd) -> WireEnd? {
            if inside(end) { return end }
            if case let .junction(id) = end, junctionIDs.contains(id) { return end }
            guard let tip = endTip(end) else { return nil }
            if let id = looseAt[tip] { return .junction(id) }
            let loose = Junction(point: tip, isLoose: true)
            copy.junctions.append(loose)
            looseAt[tip] = loose.id
            return .junction(loose.id)
        }
        for wire in wires where wireIDs.contains(wire.id) {
            guard let start = kept(wire.start), let end = kept(wire.end) else { continue }
            copy.wires.append(Wire(id: wire.id, start: start, end: end, interior: wire.interior))
        }
        // A junction left with one wire end is a loose end now; one with
        // none is dropped.
        let copiedWires = copy.wires
        func ends(at id: UUID) -> Int {
            let here = WireEnd.junction(id)
            return copiedWires.reduce(0) { $0 + ($1.start == here ? 1 : 0) + ($1.end == here ? 1 : 0) }
        }
        copy.junctions = copy.junctions.compactMap { junction in
            var junction = junction
            switch ends(at: junction.id) {
            case 0: return nil
            case 1: junction.isLoose = true
            default: break
            }
            return junction
        }

        var bounds = CGRect.null
        for chip in copy.chips { bounds = bounds.union(chip.bodyRect) }
        for label in copy.labels { bounds = bounds.union(label.rect) }
        for note in copy.notes { bounds = bounds.union(note.rect) }
        for wire in copy.wires {
            for point in path(forWire: wire.id) { bounds = bounds.union(CGRect(origin: point, size: .zero)) }
        }
        copy.bounds = bounds
        return copy.isEmpty ? nil : copy
    }
}
