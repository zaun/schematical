import CoreGraphics
import Foundation

/// What ⌘C took from the Schematic: the selected parts, net labels, text
/// blocks, junctions, wires and buses (whole, with their ports), plus every
/// wire that runs between copied items, as they were. A wire end that was
/// on something not copied (a pin of another part, a label not copied, a
/// pin of a bus not copied) is a loose junction at that spot instead, so
/// the copy stands alone. Ids are the originals'; a paste gives each item
/// a new one (`SchematicDocument.pasteSchematic`).
nonisolated struct SchematicCopy: Equatable {
    var chips: [PlacedChip] = []
    var wires: [Wire] = []
    var junctions: [Junction] = []
    var notes: [SchematicNote] = []
    var labels: [NetLabel] = []
    var buses: [Bus] = []
    /// Everything copied, in canvas units: what a paste centres on the
    /// pointer.
    var bounds: CGRect = .null

    var isEmpty: Bool {
        chips.isEmpty && wires.isEmpty && junctions.isEmpty && notes.isEmpty && labels.isEmpty && buses.isEmpty
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
        var junctionIDs = Set<UUID>(), wireIDs = Set<UUID>(), busIDs = Set<UUID>()
        for item in schematicSelection.items {
            switch item {
            case let .chip(id): chipIDs.insert(id)
            case let .label(id): labelIDs.insert(id)
            case let .note(id): noteIDs.insert(id)
            case let .junction(id): junctionIDs.insert(id)
            case let .wire(id): wireIDs.insert(id)
            case let .bus(id): busIDs.insert(id)
            case .titleBlock: break // one per document: never copied
            }
        }
        let copiedBuses = buses.filter { busIDs.contains($0.id) }
        let portIDs = Set(copiedBuses.flatMap { $0.ports.map(\.id) })
        func inside(_ end: WireEnd) -> Bool {
            switch end {
            case let .pin(ref): chipIDs.contains(ref.chipID)
            case let .label(id): labelIDs.contains(id)
            case let .junction(id): junctionIDs.contains(id)
            case let .busPin(ref): portIDs.contains(ref.portID)
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
        copy.buses = copiedBuses
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
        for bus in copy.buses {
            for line in bus.lines {
                bounds = bounds.union(CGRect(x: min(line.a.x, line.b.x), y: min(line.a.y, line.b.y),
                                             width: abs(line.b.x - line.a.x), height: abs(line.b.y - line.a.y)))
            }
            for port in bus.ports {
                if let body = bus.body(of: port) { bounds = bounds.union(body) }
            }
        }
        for wire in copy.wires {
            for point in path(for: wire) { bounds = bounds.union(CGRect(origin: point, size: .zero)) }
        }
        copy.bounds = bounds
        return copy.isEmpty ? nil : copy
    }
}

extension SchematicDocument {
    // MARK: Copy and paste (Schematic)

    /// Adds a copy (`SchematicCopy`) moved by `offset` (snapped to the
    /// grid), as one step, and makes it the selection. Every item gets a new
    /// id; parts get new references (as a placed part does) and a footprint
    /// spot to the right of the board's parts. Net labels keep their names,
    /// so they join the nets of those names. A bus keeps its name when no
    /// bus has it, or when the bus that has it carries the same wires and
    /// type (so the two link, as labels join); else it takes the next free
    /// "BUS n". Loose ends that land on a pin connect. Refused, with nothing
    /// changed, when a pasted bus would touch another bus, or when it would
    /// put a voltage out of range on a part (`editorMessage` says why).
    /// Everything pasted is selected, wire ends (junctions) included, so the
    /// whole paste drags as one; a loose end that joins a pin is no longer a
    /// junction and drops out of the selection. Returns whether anything was
    /// pasted.
    @discardableResult
    func pasteSchematic(_ copy: SchematicCopy, offset: CGSize) -> Bool {
        guard !copy.isEmpty else { return false }
        return edit(.schematic, "Paste") { scope -> Bool in
            let dx = SchematicGrid.snap(offset.width), dy = SchematicGrid.snap(offset.height)
            func shifted(_ point: CGPoint) -> CGPoint { CGPoint(x: point.x + dx, y: point.y + dy) }
            var newID: [UUID: UUID] = [:]
            func renamed(_ id: UUID) -> UUID {
                if let id = newID[id] { return id }
                let fresh = UUID()
                newID[id] = fresh
                return fresh
            }

            // References are taken from a copy: a refused paste leaves the
            // parts, and the next reference, as they were.
            var references = parts
            var pastedChips: [PlacedChip] = []
            for chip in copy.chips {
                var placed = PlacedChip(
                    id: renamed(chip.id), reference: references.takeReference(for: chip.part, gender: chip.gender,
                                                                   alsoUsed: pastedChips.map(\.reference)),
                    part: chip.part,
                    symbol: chip.symbol, origin: shifted(chip.origin), isFlipped: chip.isFlipped,
                    rotation: chip.rotation, switchesOn: chip.switchesOn, memoryFile: chip.memoryFile, color: chip.color,
                    footprintID: chip.footprintID, gender: chip.gender, ohms: chip.ohms, farads: chip.farads,
                    socket: chip.socket
                )
                placed.showsReference = chip.showsReference
                if chip.isSplit {
                    placed.setPlacements(chip.unitPlacements.map {
                        UnitPlacement(key: $0.key, origin: shifted($0.origin), isFlipped: $0.isFlipped, rotation: $0.rotation)
                    })
                }
                placed.board = placed.footprint.map {
                    BoardLayout.nextSpot(for: $0, among: chips + pastedChips, pad1: placed.part.firstPinDesignator,
                                         hasPinOneMark: placed.part.hasPinOneMark, grid: self.board.grid)
                }
                pastedChips.append(placed)
            }
            let pastedJunctions = copy.junctions.map {
                Junction(id: renamed($0.id), point: shifted($0.point), isLoose: $0.isLoose)
            }
            let pastedLabels = copy.labels.map {
                NetLabel(id: renamed($0.id), name: $0.name, style: $0.style, origin: shifted($0.origin),
                         isFlipped: $0.isFlipped)
            }
            let pastedNotes = copy.notes.map {
                SchematicNote(id: renamed($0.id), text: $0.text, origin: shifted($0.origin), size: $0.size,
                              framed: $0.framed)
            }
            var pastedBuses: [Bus] = []
            for bus in copy.buses {
                let lines = bus.lines.map {
                    BusLine(id: renamed($0.id), a: shifted($0.a), b: shifted($0.b))
                }
                let busWires = bus.wires.map { BusWire(id: renamed($0.id), name: $0.name) }
                var horizontal: [UUID: Bool] = [:]
                for line in bus.lines { horizontal[line.id] = line.isHorizontal }
                let ports = bus.ports.map { port in
                    BusPort(id: renamed(port.id), line: renamed(port.line),
                            start: port.start + (horizontal[port.line] == true ? dx : dy), side: port.side,
                            wireIDs: Set(port.wireIDs.map(renamed)))
                }
                let lowered = bus.name.lowercased()
                let others = buses + pastedBuses
                let named = others.filter { $0.name.lowercased() == lowered }
                let links = named.allSatisfy { $0.wires.map(\.name) == bus.wires.map(\.name) && $0.card == bus.card }
                let name = !bus.name.isEmpty && links ? (named.first?.name ?? bus.name)
                    : Bus.defaultBusName(taken: others.map(\.name))
                pastedBuses.append(Bus(id: renamed(bus.id), name: name, card: bus.card, lines: lines,
                                       wires: busWires, ports: ports))
            }
            guard Self.busesAreApart(buses + pastedBuses) else {
                editorMessage = "Can't paste there: a pasted bus would touch another bus."
                return false
            }
            func end(_ end: WireEnd) -> WireEnd {
                switch end {
                case let .pin(ref): .pin(PinRef(chipID: renamed(ref.chipID), index: ref.index))
                case let .junction(id): .junction(renamed(id))
                case let .label(id): .label(renamed(id))
                // Only pins of copied buses (`schematicCopy` loosens the rest).
                case let .busPin(ref): .busPin(BusPinRef(portID: renamed(ref.portID), wireID: renamed(ref.wireID)))
                }
            }
            let pastedWires = copy.wires.map {
                Wire(id: renamed($0.id), start: end($0.start), end: end($0.end), interior: $0.interior.map(shifted))
            }

            // Labels join existing nets by name, which can put a voltage
            // out of range.
            if let library,
               let issue = VoltageCheck.firstNew(before: chips, wires, after: chips + pastedChips,
                                                 wires + pastedWires, buses: buses + pastedBuses, labels: netLabels,
                                                 afterLabels: netLabels + pastedLabels, library: library) {
                editorMessage = VoltageCheck.message(issue, chips: chips + pastedChips, edit: .connect)
                return false
            }
            editorMessage = nil
            updateParts(scope) {
                $0.chips += pastedChips
                $0.nextReference = references.nextReference
            }
            sheet.updateWiring(scope) { $0.junctions += pastedJunctions }
            sheet.updateLabels(scope) { $0.labels += pastedLabels }
            sheet.updateNotes(scope) { $0.notes += pastedNotes }
            sheet.updateBuses(scope) { $0.buses += pastedBuses }
            sheet.updateWiring(scope) { $0.wires += pastedWires }

            var selection = Set<SchematicItem>()
            selection.formUnion(pastedChips.map { .chip($0.id) })
            selection.formUnion(pastedJunctions.map { .junction($0.id) })
            selection.formUnion(pastedLabels.map { .label($0.id) })
            selection.formUnion(pastedNotes.map { .note($0.id) })
            selection.formUnion(pastedWires.map { .wire($0.id) })
            selection.formUnion(pastedBuses.map { .bus($0.id) })
            clearSchematicSelection()
            for item in selection { toggleSelection(item) }
            pruneJunctions(scope)
            adoptLooseEnds(chips: Set(pastedChips.map(\.id)),
                           ports: Set(pastedBuses.flatMap { $0.ports.map(\.id) }),
                           junctions: Set(pastedJunctions.filter(\.isLoose).map(\.id)),
                           labels: Set(pastedLabels.map(\.id)))
            return true
        }
    }
}
