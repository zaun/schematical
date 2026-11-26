import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Wires

    /// Full route of a wire, from one end's tip to the other.
    func path(forWire id: UUID) -> [CGPoint] {
        sheet.wiring.path(forWire: id, tips)
    }

    /// `path(forWire:)` for a wire already in hand: no search of `wires`, so
    /// a loop over every wire stays linear.
    func path(for wire: Wire) -> [CGPoint] {
        sheet.wiring.path(for: wire, tips)
    }

    var wirePaths: [[CGPoint]] {
        let wiring = sheet.wiring, tips = tips
        return wiring.wires.map { wiring.path(for: $0, tips) }
    }

    /// Connects two anchors with an orthogonal route. A grid point on an existing
    /// wire becomes a junction: that wire is split there and the new wire branches
    /// from it.
    ///
    /// Refused, with nothing changed, when the wire would put a voltage out of
    /// range on a part (a problem the drawing doesn't already have). Returns
    /// why, which is also `editorMessage`; nil otherwise.
    func connect(_ start: LineAnchor, _ end: LineAnchor, path: [CGPoint]? = nil) -> String? {
        return edit(.schematic, "Add Wire") { scope -> String? in
            let tips = tips, wiring = sheet.wiring
            guard start != end,
                  let a = wiring.anchorTip(start, tips), let b = wiring.anchorTip(end, tips), a != b else {
                return nil
            }
            // The check runs on stand-in ends before anything changes: resolving a
            // point on a wire for real splits that wire.
            if let library, let standInStart = wiring.standIn(for: start, tips),
               let standInEnd = wiring.standIn(for: end, tips) {
                let candidate = wires + [Wire(start: standInStart, end: standInEnd, interior: [])]
                if let issue = VoltageCheck.firstNew(before: chips, wires, after: chips, candidate,
                                                     buses: buses, labels: netLabels,
                                                     known: currentVoltageIssues(library), library: library) {
                    editorMessage = VoltageCheck.message(issue, chips: chips, edit: .connect)
                    return editorMessage
                }
            }
            editorMessage = nil
            let first = anchorAxis(start), last = anchorAxis(end)
            if sheet.updateWiring(scope, { $0.connect(start, end, path: path, first: first, last: last, tips) }) {
                pruneSelection()
            }
            return nil
        }
    }

    /// Whether a wire end sits on a pin of the given part.
    func attaches(_ end: WireEnd, to chipID: UUID) -> Bool {
        if case let .pin(ref) = end {
            return ref.chipID == chipID
        }
        return false
    }

    /// Drops junctions that no wire ends at any more. Every removal of
    /// parts or wires ends here, so it also drops what left the selection.
    func pruneJunctions(_ scope: EditScope) {
        let tips = tips
        sheet.updateWiring(scope) { $0.pruneJunctions(tips) }
        pruneSelection()
    }

    /// `Wiring.loosen` with the document's tips: run it while the pins that
    /// go still resolve.
    func loosen(_ scope: EditScope, where matches: (WireEnd) -> Bool) {
        let tips = tips
        sheet.updateWiring(scope) { $0.loosen(where: matches, tips) }
    }

    /// `Wiring.settle` with the document's tips.
    func settle(_ scope: EditScope, where touches: (WireEnd) -> Bool) {
        let tips = tips
        sheet.updateWiring(scope) { $0.settle(where: touches, tips) }
    }

    /// Re-fits the wires on a bus port's pins after the port changed (pins
    /// added, removed or moved across), as `settleWires(attachedTo:)` does for
    /// a part.
    func settleWires(attachedToPort portID: UUID) {
        edit(.schematic, "Move") { scope in
            settle(scope) { BusNetwork.endsOn($0, port: portID) }
        }
    }

    /// Re-fits the wires on every port of a bus, when a bus drag ends, and
    /// connects loose ends its ports' pins landed on.
    func settleWires(attachedToBus busID: UUID) {
        edit(.schematic, "Move") { scope in
            let ports = bus(busID)?.ports.map(\.id) ?? []
            for port in ports {
                settle(scope) { BusNetwork.endsOn($0, port: port) }
            }
            adoptLooseEnds(ports: Set(ports))
        }
    }

    /// Whether either end of the wire is on a loose junction (plan-30).
    func hasLooseEnd(_ wire: Wire) -> Bool {
        sheet.wiring.hasLooseEnd(wire)
    }

    /// Whether a junction gets a dot: always for a plain junction; for a
    /// loose one only where 3 or more wire ends meet, since a dot reads as a
    /// connection.
    func showsDot(_ junction: Junction) -> Bool {
        sheet.wiring.showsDot(junction)
    }

    /// Connects loose ends to pins after a drop (plan-30). A loose junction
    /// joins a pin when its point is that pin's tip and either the junction
    /// or the pin's part or port is in the dropped set; every wire end on the
    /// junction moves to the pin, and the junction goes away. Each candidate
    /// goes through the same voltage check as `connect`; one that fails stays
    /// loose and sets `editorMessage`. When two pins share a tip, the first
    /// part in `chips` order wins, then bus order. Removals never call this.
    func adoptLooseEnds(chips chipIDs: Set<UUID> = [], ports portIDs: Set<UUID> = [],
                        junctions junctionIDs: Set<UUID> = [], labels labelIDs: Set<UUID> = []) {
        edit(.schematic, "Connect") { scope in
            guard junctions.contains(where: \.isLoose) else { return }
            @MainActor func tips(of placed: [PlacedChip], ports includes: (BusPort) -> Bool,
                                 labels: [NetLabel]) -> [CGPoint: WireEnd] {
                var map: [CGPoint: WireEnd] = [:]
                for chip in placed {
                    for slot in chip.layout.slots {
                        let ref = PinRef(chipID: chip.id, index: slot.pin.index)
                        if let tip = pinTip(ref), map[tip] == nil { map[tip] = .pin(ref) }
                    }
                }
                for bus in buses {
                    for port in bus.ports where includes(port) {
                        for wire in bus.pinWires(of: port) {
                            let ref = BusPinRef(portID: port.id, wireID: wire.id)
                            if let tip = busPinTip(ref), map[tip] == nil { map[tip] = .busPin(ref) }
                        }
                    }
                }
                for label in labels where map[label.tip] == nil {
                    map[label.tip] = .label(label.id)
                }
                return map
            }
            let dropped = tips(of: chips.filter { chipIDs.contains($0.id) }, ports: { portIDs.contains($0.id) },
                               labels: netLabels.filter { labelIDs.contains($0.id) })
            // A dragged loose end may land on any pin.
            let every = junctionIDs.isEmpty ? [:] : tips(of: chips, ports: { _ in true }, labels: netLabels)
            let adopted = sheet.updateWiring(scope) { wiring in
                wiring.adoptLooseEnds(dropped: dropped, every: every, draggedJunctions: junctionIDs) { before, after in
                    guard let library else { return true }
                    // The problems the drawing has before this candidate: an
                    // earlier adoption in this loop may have changed them.
                    let known = before == wires ? currentVoltageIssues(library)
                        : VoltageCheck.issues(chips: chips, wires: before, buses: buses, labels: netLabels, library: library)
                    guard let issue = VoltageCheck.firstNew(before: chips, before, after: chips, after,
                                                            buses: buses, labels: netLabels,
                                                            known: known, library: library) else { return true }
                    editorMessage = VoltageCheck.message(issue, chips: chips, edit: .connect)
                    return false
                }
            }
            if adopted { pruneSelection() }
        }
    }

    /// Called when a block is released. Re-fits the wires attached to it and
    /// stores the result, so the change stays rather than being recomputed per frame.
    func settleWires(attachedTo chipID: UUID) {
        edit(.schematic, "Move") { scope in
            settle(scope) { attaches($0, to: chipID) }
        }
    }

    /// Stores a new route for a wire. The pin tips stay where they are.
    func setWirePath(_ id: UUID, _ path: [CGPoint]) {
        edit(.schematic, "Move Wire") { scope in
            sheet.updateWiring(scope) { $0.setPath(id, path) }
        }
    }

    /// A bend point on a wire, if `point` is on one.
    func nodeHit(at point: CGPoint, tolerance: CGFloat = 6) -> (wire: UUID, index: Int)? {
        sheet.wiring.nodeHit(at: point, tolerance: tolerance, tips)
    }

    /// A segment of a wire, if `point` is on one. `index` is the segment's
    /// start point within the wire's path.
    func segmentHit(at point: CGPoint, tolerance: CGFloat = 6) -> (wire: UUID, index: Int)? {
        sheet.wiring.segmentHit(at: point, tolerance: tolerance, tips)
    }

    /// Double-click on a wire: adds a bend at that point.
    func insertNode(at point: CGPoint, tolerance: CGFloat = 6) {
        edit(.schematic, "Add Bend") { scope in
            let tips = tips
            sheet.updateWiring(scope) { $0.insertNode(at: point, tolerance: tolerance, tips) }
        }
    }
}
