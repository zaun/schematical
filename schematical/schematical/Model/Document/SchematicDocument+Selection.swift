import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Box select

    /// The parts, junctions, wires, text blocks, net labels and buses a box
    /// (schematic coordinates) touches. A bus is in when any of its lines
    /// is.
    func schematicItems(in rect: CGRect) -> Set<SchematicItem> {
        var items = Set<SchematicItem>()
        for chip in chips where !chip.pieces(in: rect).isEmpty {
            items.insert(.chip(chip.id))
        }
        for junction in junctions where rect.contains(junction.point) {
            items.insert(.junction(junction.id))
        }
        for note in notes where note.rect.intersects(rect) {
            items.insert(.note(note.id))
        }
        if let block = titleBlockRect, block.intersects(rect) { items.insert(.titleBlock) }
        for label in netLabels where label.rect.intersects(rect) {
            items.insert(.label(label.id))
        }
        for wire in wires {
            let path = path(for: wire)
            if zip(path, path.dropFirst()).contains(where: { Self.segment($0, $1, touches: rect) }) {
                items.insert(.wire(wire.id))
            }
        }
        for bus in buses where bus.lines.contains(where: { Self.segment($0.a, $0.b, touches: rect) }) {
            items.insert(.bus(bus.id))
        }
        return items
    }

    /// The drawn footprints, texts and traces a box (board mm) touches.
    func boardItems(in rect: CGRect, visible: BoardVisibility = BoardVisibility()) -> Set<BoardItem> {
        var items = Set<BoardItem>()
        for chip in chips where chip.board != nil && visible.showsFootprint(chip) {
            if let bounds = chip.boardBounds, bounds.intersects(rect) { items.insert(.footprint(chip.id)) }
        }
        for text in texts where visible.showsText(text) && text.boardRect.intersects(rect) {
            items.insert(.text(text.id))
        }
        for trace in traces where board.copperLayers.contains(trace.layer) && visible.showsTrace(on: trace.layer) {
            let grown = rect.insetBy(dx: -trace.widthMM / 2, dy: -trace.widthMM / 2)
            if trace.segments.contains(where: { Self.segment($0.0, $0.1, touches: grown) }) {
                items.insert(.trace(trace.id))
            }
        }
        if visible.showsAnyCopper {
            for via in vias where via.rect.intersects(rect) { items.insert(.via(via.id)) }
            for hole in holes where hole.rect.intersects(rect) { items.insert(.hole(hole.id)) }
        }
        // A pour or keep-out is taken whole: its rectangle inside the box.
        for pour in pours where visible.shows(.copper(pour.layer)) && rect.contains(pour.rect) {
            items.insert(.pour(pour.id))
        }
        for keepout in keepouts where keepoutIsShown(keepout, visible) && rect.contains(keepout.rect) {
            items.insert(.keepout(keepout.id))
        }
        // A cutout likewise: its bounds inside the box.
        for cutout in cutouts where rect.contains(cutout.shape.bounds) { items.insert(.cutout(cutout.id)) }
        return items
    }

    /// A Shift-drag on empty Schematic canvas: the selection is what it was
    /// at the press (`base`) plus everything the box touches.
    func boxSelectSchematic(_ rect: CGRect, base: Set<SchematicItem>) {
        let items = base.union(schematicItems(in: rect))
        if items != schematicSelection.items { schematicSelection.set(items) }
        // Of a part drawn as its units, the box picks the units it touches
        // (plan-67). A part that was selected before the box keeps its picks.
        for chip in chips where chip.isSplit && items.contains(.chip(chip.id)) && !base.contains(.chip(chip.id)) {
            let touched = Set(chip.pieces(in: rect).compactMap(\.unitKey))
            if pickedUnits[chip.id] != touched { pickedUnits[chip.id] = touched }
        }
    }

    /// A Shift-drag on empty board: as `boxSelectSchematic`, on the PCB page.
    func boxSelectBoard(_ rect: CGRect, base: Set<BoardItem>, visible: BoardVisibility = BoardVisibility()) {
        let items = base.union(boardItems(in: rect, visible: visible))
        if items != boardSelection.items { boardSelection.set(items) }
    }

    /// Selects exactly `items` on the PCB page (a Design Rule Check line).
    func selectBoardItems(_ items: Set<BoardItem>) {
        if items != boardSelection.items { boardSelection.set(items) }
    }

    /// Selects exactly `items` on the Schematic page (an Electrical Rules Check line).
    func selectSchematicItems(_ items: Set<SchematicItem>) {
        if items != schematicSelection.items { schematicSelection.set(items) }
    }

    /// Edit › Select All on the Schematic: every part, wire, junction, text
    /// block, net label and bus.
    func selectAllSchematic() {
        var items = Set<SchematicItem>()
        items.formUnion(chips.map { .chip($0.id) })
        items.formUnion(wires.map { .wire($0.id) })
        items.formUnion(junctions.map { .junction($0.id) })
        items.formUnion(notes.map { .note($0.id) })
        items.formUnion(netLabels.map { .label($0.id) })
        items.formUnion(buses.map { .bus($0.id) })
        if documentInfo.titleBlock != nil { items.insert(.titleBlock) }
        if items != schematicSelection.items { schematicSelection.set(items) }
    }

    /// Edit › Select All on the PCB: every footprint, text, trace, via and
    /// hole drawn with `visible` (as a box select would take them).
    func selectAllBoard(visible: BoardVisibility = BoardVisibility()) {
        var items = Set<BoardItem>()
        for chip in chips where chip.board != nil && visible.showsFootprint(chip) {
            items.insert(.footprint(chip.id))
        }
        for text in texts where visible.showsText(text) { items.insert(.text(text.id)) }
        for trace in traces where board.copperLayers.contains(trace.layer) && visible.showsTrace(on: trace.layer) {
            items.insert(.trace(trace.id))
        }
        if visible.showsAnyCopper {
            items.formUnion(vias.map { .via($0.id) })
            items.formUnion(holes.map { .hole($0.id) })
        }
        for pour in pours where visible.shows(.copper(pour.layer)) { items.insert(.pour(pour.id)) }
        for keepout in keepouts where keepoutIsShown(keepout, visible) { items.insert(.keepout(keepout.id)) }
        for cutout in cutouts { items.insert(.cutout(cutout.id)) }
        if items != boardSelection.items { boardSelection.set(items) }
    }

    /// Clears the Schematic page's selection: items (buses included) and
    /// port.
    func clearSchematicSelection() {
        schematicSelection.clear()
        selectedPortID = nil
    }

    func clearBoardSelection() {
        boardSelection.clear()
    }

    /// Clears both pages' selections (opening a file, starting a run).
    func clearAllSelections() {
        clearSchematicSelection()
        clearBoardSelection()
    }

    /// Drops selected ids that are no longer in the document.
    func pruneSelection() {
        let chipIDs = Set(chips.map(\.id)), wireIDs = Set(wires.map(\.id))
        let junctionIDs = Set(junctions.map(\.id)), textIDs = Set(texts.map(\.id))
        let noteIDs = Set(notes.map(\.id)), labelIDs = Set(netLabels.map(\.id))
        let busIDs = Set(buses.map(\.id))
        schematicSelection.filter { item in
            switch item {
            case let .chip(id): chipIDs.contains(id)
            case let .wire(id): wireIDs.contains(id)
            case let .junction(id): junctionIDs.contains(id)
            case let .note(id): noteIDs.contains(id)
            case let .label(id): labelIDs.contains(id)
            case let .bus(id): busIDs.contains(id)
            case .titleBlock: documentInfo.titleBlock != nil
            }
        }
        let footprints = Set(chips.filter { $0.board != nil }.map(\.id))
        let traceIDs = Set(traces.map(\.id)), viaIDs = Set(vias.map(\.id)), holeIDs = Set(holes.map(\.id))
        let pourIDs = Set(pours.map(\.id)), keepoutIDs = Set(keepouts.map(\.id)), cutoutIDs = Set(cutouts.map(\.id))
        boardSelection.filter { item in
            switch item {
            case let .footprint(id): footprints.contains(id)
            case let .text(id): textIDs.contains(id)
            case let .trace(id): traceIDs.contains(id)
            case let .via(id): viaIDs.contains(id)
            case let .hole(id): holeIDs.contains(id)
            case let .pour(id): pourIDs.contains(id)
            case let .keepout(id): keepoutIDs.contains(id)
            case let .cutout(id): cutoutIDs.contains(id)
            }
        }
    }
}
