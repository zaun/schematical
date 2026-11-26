import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Board texts

    /// Runs `change` on the board's texts as one undo step named `name`.
    func editTexts<T>(_ name: String, _ change: (inout BoardTexts) -> T) -> T {
        edit(.pcb, name) { scope in layout.updateTexts(scope, change) }
    }

    /// Adds a "Text" on the silkscreen with its top-left at `point` (snapped)
    /// and selects it.
    @discardableResult
    func addText(at point: CGPoint) -> UUID {
        return edit(.pcb, "Add Text") { scope -> UUID in
            let grid = board.grid
            let text = layout.updateTexts(scope) { $0.add(at: point, grid: grid) }
            select(.text(text.id))
            return text.id
        }
    }

    /// Changes a text's words. An empty string is refused and the text kept.
    func setText(_ id: UUID, _ value: String) {
        editTexts("Edit Text") { $0.setText(id, value) }
    }

    /// Sets a text's font size in mil, clamped to `BoardText.sizeRange`.
    func setTextSize(_ id: UUID, _ size: Int) {
        editTexts("Change Font Size") { $0.setSize(id, size) }
    }

    func setTextLayer(_ id: UUID, _ layer: TextLayer) {
        editTexts("Change Layer") { $0.setLayer(id, layer) }
    }

    /// Sets the layer of every text in `ids`, as one undo step.
    func setTextLayer(_ ids: some Collection<UUID>, _ layer: TextLayer) {
        editTexts("Change Layer") { texts in ids.forEach { texts.setLayer($0, layer) } }
    }

    /// Sets the font size (mil) of every text in `ids`, as one undo step.
    func setTextSize(_ ids: some Collection<UUID>, _ size: Int) {
        editTexts("Change Font Size") { texts in ids.forEach { texts.setSize($0, size) } }
    }

    /// Puts a text on the top or bottom of the board. Its box stays where it
    /// is. Does nothing on a 1-layer board, except to bring a text back to Top.
    func setTextSide(_ id: UUID, _ side: BoardSide) {
        let layers = board.layers
        editTexts("Change Side") { $0.setSide(id, side, layers: layers) }
    }

    /// Moves a text's anchor to `point`, snapped to the grid.
    func moveText(_ id: UUID, to point: CGPoint) {
        let grid = board.grid
        editTexts("Move") { $0.move(id, to: point, grid: grid) }
    }

    /// Sets a text's rotation: 0, 90, 180 or 270. Other values are ignored.
    func setTextRotation(_ id: UUID, _ rotation: Int) {
        editTexts("Rotate") { $0.setRotation(id, rotation) }
    }

    /// Turns a text 90° clockwise about its anchor.
    func rotateText(_ id: UUID, counterclockwise: Bool = false) {
        editTexts("Rotate") { $0.rotate(id, counterclockwise: counterclockwise) }
    }

    func deleteText(_ id: UUID) {
        edit(.pcb, "Delete Text") { scope in
            layout.updateTexts(scope) { $0.remove([id]) }
            pruneSelection()
        }
    }

    /// The text under a board point (mm): the last one drawn there, a Top
    /// text before a Bottom one. A text that isn't drawn can't be hit.
    func text(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> BoardText? {
        layout.boardTexts.text(at: point, visible: visible)
    }

    /// The footprint under a board point (mm): the last one drawn there, a
    /// Top footprint before a Bottom one. It is hit by its bounds while
    /// anything of it is drawn.
    func footprint(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> PlacedChip? {
        for side in [BoardSide.top, .bottom] {
            if let hit = chips.last(where: {
                $0.board?.side == side && ($0.boardBounds?.contains(point) ?? false) && visible.showsFootprint($0)
            }) { return hit }
        }
        return nil
    }

    /// The footprint whose drawn reference text is under a board point (mm),
    /// within `tolerance` of its box: Top before Bottom, the last one drawn
    /// first. `height` is the text's drawn height (mm); a reference that is
    /// hidden, or whose layer is off, can't be hit.
    func referenceHit(at point: CGPoint, tolerance: CGFloat, height: Double = BoardReference.height,
                      visible: BoardVisibility = BoardVisibility()) -> UUID? {
        for side in [BoardSide.top, .bottom] {
            if let hit = chips.last(where: { chip in
                guard let placement = chip.board, placement.side == side,
                      visible.showsMark(placement.reference, side: side),
                      let rect = chip.boardReferenceRect(height: height) else { return false }
                return rect.insetBy(dx: -tolerance, dy: -tolerance).contains(point)
            }) { return hit.id }
        }
        return nil
    }

    /// The pad under a board point (mm) and what to call it, for the PCB
    /// page's hover label: a part's pad ("U3.5 A0"; "U3.5" when the pin's
    /// name is just its number; "U3 centre pad" for an unnumbered one), or a
    /// card's connector pin ("Edge.13 A0", with the edge bus's wire name) or
    /// named header pad off the bus. Top before Bottom; a pad that
    /// isn't drawn can't be hit.
    func padLabel(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> (pad: PlacedPad, label: String)? {
        func hits(_ pad: PlacedPad) -> Bool {
            guard pad.rect.contains(point) else { return false }
            return Self.point(point, touches: pad)
        }
        for side in [BoardSide.top, .bottom] {
            for (chip, pads) in zip(chips, chipPads).reversed() {
                guard let placed = chip.board?.side else { continue }
                // A pad is on its layer's side, which an opposite pad's is
                // not its part's.
                guard let pad = pads.last(where: {
                    $0.plated && $0.layerSide(on: placed) == side && visible.showsPad(drill: $0.drill, side: side)
                        && hits($0)
                }) else { continue }
                guard let designator = pad.designator else { return (pad, "\(chip.reference) centre pad") }
                let name = chip.part.pins.first { $0.designator == designator }?.name ?? ""
                let label = "\(chip.reference).\(designator)"
                return (pad, name.isEmpty || name == designator ? label : "\(label) \(name)")
            }
        }
        let named = (board.fingers + board.extraPads).filter { !$0.name.isEmpty }
        guard let finger = named.last(where: {
            visible.showsPad(drill: $0.drill, side: $0.side) && hits($0.pad)
        }) else { return nil }
        // A connector pin is named like a part's pad ("Edge.13"); one on the
        // bus also shows the edge bus's wire of the same position
        // (`BoardSettings.pinNames`), as "Edge.13 A0".
        guard board.fingers.contains(finger) else { return (finger.pad, finger.name) }
        let label = "\(board.connectorName).\(finger.name)"
        if let busID = edgeBusID, let bus = bus(busID),
           let index = board.pinNames.firstIndex(of: finger.name), index < bus.wires.count {
            let wire = bus.wires[index].name
            if !wire.isEmpty, wire != finger.name { return (finger.pad, "\(label) \(wire)") }
        }
        return (finger.pad, label)
    }
}
