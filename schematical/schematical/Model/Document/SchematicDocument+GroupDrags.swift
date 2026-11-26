import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Group drags (Schematic)

    /// Where every selected part, junction and wire is now, for a group drag.
    func schematicGroupBase() -> SchematicGroupBase {
        var base = SchematicGroupBase()
        for item in schematicSelection.items {
            switch item {
            case let .chip(id):
                guard let chip = chips.first(where: { $0.id == id }) else { break }
                if chip.isSplit {
                    // Only the picked units move (plan-67).
                    let keys = picked(chip)
                    for placement in chip.unitPlacements where keys.contains(placement.key) {
                        base.units[UnitRef(chipID: id, key: placement.key)] = placement.origin
                    }
                } else {
                    base.chips[id] = chip.origin
                }
            case let .junction(id):
                if let junction = junctions.first(where: { $0.id == id }) { base.junctions[id] = junction.point }
            case let .wire(id):
                if let wire = wires.first(where: { $0.id == id }) { base.wires[id] = wire.interior }
            case let .note(id):
                if let note = notes.first(where: { $0.id == id }) { base.notes[id] = note.origin }
            case let .label(id):
                if let label = netLabels.first(where: { $0.id == id }) { base.labels[id] = label.origin }
            case .titleBlock:
                base.titleBlock = documentInfo.titleBlock
            case let .bus(id):
                if let bus = bus(id) {
                    base.buses[id] = bus
                    base.ports.formUnion(bus.ports.map(\.id))
                }
            }
        }
        for wire in wires where base.wires[wire.id] != nil || moves(wire.start, in: base) || moves(wire.end, in: base) {
            base.affectedWires.insert(wire.id)
            let route = path(for: wire)
            if OrthogonalRoute.isOrthogonal(route) {
                base.orthogonalWires.insert(wire.id)
                if base.wires[wire.id] == nil { base.paths[wire.id] = route }
            }
        }
        return base
    }

    /// Whether a group drag moves this wire end: a pin of a moved part, a
    /// moved junction or a moved net label.
    func moves(_ end: WireEnd, in base: SchematicGroupBase) -> Bool {
        switch end {
        case let .pin(ref):
            if base.chips[ref.chipID] != nil { return true }
            guard !base.units.isEmpty, let chip = chips.first(where: { $0.id == ref.chipID }),
                  let key = chip.unitKey(ofPin: ref.index) else { return false }
            return base.units[UnitRef(chipID: ref.chipID, key: key)] != nil
        case let .junction(id): return base.junctions[id] != nil
        case let .busPin(ref): return base.ports.contains(ref.portID)
        case let .label(id): return base.labels[id] != nil
        }
    }

    /// `bus` moved by (`dx`, `dy`): its lines, and its ports along with
    /// them (a port's start is along its line).
    static func shifted(_ bus: Bus, dx: CGFloat, dy: CGFloat) -> Bus {
        var moved = bus
        var horizontal: [UUID: Bool] = [:]
        for index in moved.lines.indices {
            horizontal[moved.lines[index].id] = moved.lines[index].isHorizontal
            moved.lines[index].a.x += dx
            moved.lines[index].a.y += dy
            moved.lines[index].b.x += dx
            moved.lines[index].b.y += dy
        }
        for index in moved.ports.indices {
            moved.ports[index].start += horizontal[moved.ports[index].line] == true ? dx : dy
        }
        return moved
    }

    /// One frame of a group drag: every part, junction, selected wire bend
    /// and bus in `base` moves by `delta` snapped to the grid, from where it
    /// was at the press. Selected wires keep their ends: only their bends
    /// move. A frame that would make a moved bus touch another bus changes
    /// nothing, so the group stays at the last place it could go.
    func moveSchematicGroup(_ base: SchematicGroupBase, by delta: CGSize) {
        edit(.schematic, "Move") { scope in
            let dx = SchematicGrid.snap(delta.width), dy = SchematicGrid.snap(delta.height)
            func shifted(_ point: CGPoint) -> CGPoint { CGPoint(x: point.x + dx, y: point.y + dy) }
            if !base.buses.isEmpty {
                let moved = buses.map { bus in base.buses[bus.id].map { Self.shifted($0, dx: dx, dy: dy) } ?? bus }
                guard Self.busesAreApart(moved) else { return }
                sheet.updateBuses(scope) { $0.buses = moved }
            }
            updateParts(scope) { $0.moveGroup(origins: base.chips, units: base.units, by: shifted) }
            sheet.updateNotes(scope) { $0.move(base.notes, by: shifted) }
            sheet.updateLabels(scope) { $0.move(base.labels, by: shifted) }
            if let origin = base.titleBlock {
                let moved = shifted(origin)
                updateInfo(scope) { $0.titleBlock = moved }
            }
            let tips = tips
            sheet.updateWiring(scope) { wiring in
                wiring.moveGroup(junctions: base.junctions, bends: base.wires, by: shifted)
                wiring.refit(base.paths, tips)
            }
        }
    }

    /// The end of a group drag: re-fits each affected wire once, as
    /// `settleWires(attachedTo:)` does. A wire that was all right angles at
    /// the press gets corners wherever the move left a slanted segment.
    func settleSchematicGroup(_ base: SchematicGroupBase) {
        edit(.schematic, "Move") { scope in
            let tips = tips
            sheet.updateWiring(scope) { $0.settleGroup(affected: base.affectedWires, orthogonal: base.orthogonalWires, tips) }
            adoptLooseEnds(chips: Set(base.chips.keys).union(base.units.keys.map(\.chipID)), ports: base.ports,
                           junctions: Set(base.junctions.keys), labels: Set(base.labels.keys))
        }
    }

    // MARK: Group drags (PCB)

    /// Where every selected footprint (its centre) and text (its anchor) is
    /// now, for a group drag. Locked items are left out, so they stay put.
    func boardGroupBase(grab: CGPoint? = nil) -> BoardGroupBase {
        var base = BoardGroupBase()
        base.grab = grab
        for item in boardSelection.items where !isLocked(item) {
            switch item {
            case let .footprint(id):
                if let centre = chips.first(where: { $0.id == id })?.board?.centre { base.footprints[id] = centre }
            case let .text(id):
                if let anchor = texts.first(where: { $0.id == id })?.anchor { base.texts[id] = anchor }
            case let .trace(id):
                if let points = traces.first(where: { $0.id == id })?.points { base.traces[id] = points }
            case let .via(id):
                if let centre = vias.first(where: { $0.id == id })?.centre { base.vias[id] = centre }
            case let .hole(id):
                if let centre = holes.first(where: { $0.id == id })?.centre { base.holes[id] = centre }
            case let .pour(id):
                if let rect = pours.first(where: { $0.id == id })?.rect { base.pours[id] = rect }
            case let .keepout(id):
                if let rect = keepouts.first(where: { $0.id == id })?.rect { base.keepouts[id] = rect }
            case let .cutout(id):
                if let shape = cutouts.first(where: { $0.id == id })?.shape { base.cutouts[id] = shape }
            }
        }
        return base
    }

    /// One frame of a group drag on the PCB. One item moves as it always
    /// has, snapped onto the grid; a lone trace by the end nearer the grab,
    /// onto the trace grid. Several all move by the same `delta`
    /// snapped to the board grid, so they keep their places relative to
    /// each other.
    func moveBoardGroup(_ base: BoardGroupBase, by delta: CGSize) {
        edit(.pcb, "Move") { scope in
            func moved(_ point: CGPoint, by offset: CGSize) -> CGPoint {
                CGPoint(x: point.x + offset.width, y: point.y + offset.height)
            }
            var offset = CGSize(width: BoardGrid.snap(delta.width, spacing: self.board.grid),
                                height: BoardGrid.snap(delta.height, spacing: self.board.grid))
            // A lone trace snaps by its end: the one nearer where it was
            // grabbed (its start without a grab) lands on the trace grid.
            // A click (no movement yet) leaves it where it is.
            if base.count == 1, delta != .zero, let points = base.traces.values.first, let first = points.first,
               let last = points.last {
                let end = base.grab.map {
                    Self.distance(from: $0, to: last) < Self.distance(from: $0, to: first) ? last : first
                } ?? first
                let target = BoardGrid.snap(moved(end, by: delta), spacing: self.board.traceGrid)
                offset = CGSize(width: target.x - end.x, height: target.y - end.y)
            }
            // Traces move by the snapped delta from where they were, alone or
            // in a group; joins wait for the release (`settleBoardGroup`).
            let traceOffset = offset
            layout.updateCopper(scope) { $0.moveGroup(traces: base.traces, vias: [:], by: traceOffset) }
            if base.count == 1 {
                if let (id, centre) = base.footprints.first { moveFootprint(id, to: moved(centre, by: delta)) }
                if let (id, anchor) = base.texts.first { moveText(id, to: moved(anchor, by: delta)) }
                // A via placed on a trace stays there until the pointer moves.
                if let (id, centre) = base.vias.first, delta != .zero { moveVia(id, to: moved(centre, by: delta)) }
                if let (id, centre) = base.holes.first { moveHole(id, to: moved(centre, by: delta)) }
                if let (id, rect) = base.pours.first { movePour(id, to: moved(rect.origin, by: delta)) }
                if let (id, rect) = base.keepouts.first { moveKeepout(id, to: moved(rect.origin, by: delta)) }
                if let (id, shape) = base.cutouts.first { moveCutout(id, to: moved(shape.bounds.origin, by: delta)) }
                return
            }
            layout.updateFeatures(scope) { $0.moveGroup(base, by: traceOffset) }
            layout.updateCopper(scope) { $0.moveGroup(traces: [:], vias: base.vias, by: traceOffset) }
            updateParts(scope) { $0.moveGroup(footprints: base.footprints, by: traceOffset) }
            layout.updateTexts(scope) { $0.move(base.texts, by: { moved($0, by: traceOffset) }) }
        }
    }

    /// The end of a PCB group drag: each moved trace joins what it now
    /// touches. Traces that weren't selected stay where they are.
    func settleBoardGroup(_ base: BoardGroupBase) {
        editCopper("Move") { copper in base.traces.keys.forEach { copper.join($0) } }
    }
}
