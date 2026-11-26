import CoreGraphics
import Foundation

/// The parts' footprints on the PCB (plan-68-F): moving, turning and
/// flipping them, their origin, style and marks, their reference text, and
/// the PCB group drag's parts. The board's grids and layer count come in as
/// parameters.
extension PartList {
    /// Changes the unlocked placement of part `id` (which has a footprint),
    /// given the placement, its footprint and pad 1's designator; the
    /// result is snapped to `grid` when it is not nil.
    private mutating func movePlacement(_ id: UUID, grid: Double?,
                                        _ move: (BoardPlacement, Footprint, String) -> BoardPlacement?) {
        guard let index = index(id), let board = chips[index].board, !board.locked,
              let footprint = chips[index].footprint else { return }
        let pad1 = chips[index].part.firstPinDesignator
        guard let moved = move(board, footprint, pad1) else { return }
        let placed = grid.map { BoardLayout.snapped(moved, footprint: footprint, pad1: pad1, grid: $0) } ?? moved
        if placed != board { chips[index].board = placed }
    }

    /// Moves a footprint's centre (board mm), then snaps its origin point to `grid`.
    mutating func moveFootprint(_ id: UUID, to centre: CGPoint, grid: Double) {
        movePlacement(id, grid: grid) { board, _, _ in
            var moved = board
            moved.centre = centre
            return moved
        }
    }

    /// Moves a footprint so its origin point lands on `point` (board mm),
    /// snapped to `grid`.
    mutating func moveFootprintOrigin(_ id: UUID, to point: CGPoint, grid: Double) {
        movePlacement(id, grid: grid) { board, footprint, pad1 in
            let current = BoardLayout.originPoint(of: board, footprint: footprint, pad1: pad1)
            var moved = board
            moved.centre.x += point.x - current.x
            moved.centre.y += point.y - current.y
            return moved
        }
    }

    /// `changed` moved so its origin point is where `board`'s was.
    private static func keepingOrigin(_ board: BoardPlacement, _ changed: BoardPlacement, footprint: Footprint,
                                      pad1: String) -> BoardPlacement {
        let before = BoardLayout.originPoint(of: board, footprint: footprint, pad1: pad1)
        let after = BoardLayout.originPoint(of: changed, footprint: footprint, pad1: pad1)
        var moved = changed
        moved.centre.x += before.x - after.x
        moved.centre.y += before.y - after.y
        return moved
    }

    /// Turns a footprint 90° about its origin point, which stays, then snaps.
    mutating func rotateFootprint(_ id: UUID, counterclockwise: Bool, grid: Double) {
        movePlacement(id, grid: grid) { board, footprint, pad1 in
            var turned = board
            turned.rotation = (board.rotation + (counterclockwise ? 270 : 90)) % 360
            return Self.keepingOrigin(board, turned, footprint: footprint, pad1: pad1)
        }
    }

    /// Moves a footprint to the other side of a board of more than one
    /// layer, mirrored left to right as seen from above; its origin point
    /// stays, then snaps.
    mutating func setFootprintSide(_ id: UUID, _ side: BoardSide, layers: Int, grid: Double) {
        guard layers > 1, let index = index(id), let placement = chips[index].board, placement.side != side,
              let footprint = chips[index].footprint else { return }
        let pad1 = chips[index].part.firstPinDesignator
        var flipped = placement
        flipped.side = side
        // A placement mirrors before it turns, so a board-wide left-right
        // mirror of a turned footprint is the mirror plus the opposite turn.
        flipped.rotation = (360 - placement.rotation) % 360
        let moved = Self.keepingOrigin(placement, flipped, footprint: footprint, pad1: pad1)
        chips[index].board = BoardLayout.snapped(moved, footprint: footprint, pad1: pad1, grid: grid)
    }

    /// Which point of a footprint is its origin; the footprint stays where it is.
    mutating func setFootprintOrigin(_ id: UUID, _ origin: FootprintOrigin) {
        guard let index = index(id), chips[index].board != nil else { return }
        chips[index].board?.origin = origin
    }

    /// Switches a part with a connector footprint between it and its
    /// footprint, pad 1 staying exactly where it was.
    mutating func setFootprintStyle(_ id: UUID, _ style: FootprintStyle) {
        guard let index = index(id), chips[index].part.connectorFootprint != nil,
              let board = chips[index].board, board.style != style, let before = chips[index].footprint else { return }
        let pad1 = chips[index].part.firstPinDesignator
        var changed = board
        changed.style = style
        // The reference was placed for the old body.
        changed.referenceOffset = .zero
        chips[index].board = changed
        guard let after = chips[index].footprint else { return }
        if let old = before.pads.first(where: { $0.designator == pad1 }),
           let new = after.pads.first(where: { $0.designator == pad1 }) {
            let anchor = board.board(old.centre)
            let landed = changed.board(new.centre)
            changed.centre.x += anchor.x - landed.x
            changed.centre.y += anchor.y - landed.y
        }
        chips[index].board = changed
    }

    /// Where the marks of the footprints of `ids` are drawn; a nil layer is
    /// left as it is, a part with no footprint skipped.
    mutating func setFootprintMarks(_ ids: some Collection<UUID>, outline: MarkLayer?, pin1: MarkLayer?,
                                    reference: MarkLayer?) {
        for index in chips.indices where ids.contains(chips[index].id) && chips[index].board != nil {
            if let outline { chips[index].board?.outline = outline }
            if let pin1 { chips[index].board?.pin1 = pin1 }
            if let reference { chips[index].board?.reference = reference }
        }
    }

    /// Moves a footprint's reference so its text is centred at `point`
    /// (board mm), snapped to `grid`, stored relative to the part.
    mutating func moveReference(_ id: UUID, toCentre point: CGPoint, grid: Double) {
        guard let index = index(id), let board = chips[index].board else { return }
        let local = board.local(BoardGrid.snap(point, spacing: CGFloat(grid)))
        // Whole 0.0001 mm, so the file carries no float noise.
        let offset = CGPoint(x: (local.x * 10000).rounded() / 10000 + 0, y: (local.y * 10000).rounded() / 10000 + 0)
        if board.referenceOffset != offset { chips[index].board?.referenceOffset = offset }
    }

    /// Puts a footprint's reference back on the body centre.
    mutating func resetReference(_ id: UUID) {
        guard let index = index(id), chips[index].board?.referenceOffset != .zero else { return }
        chips[index].board?.referenceOffset = .zero
    }

    mutating func setLocked(footprint id: UUID, _ locked: Bool) {
        guard let index = index(id), chips[index].board != nil else { return }
        chips[index].board?.locked = locked
    }

    /// One frame of a PCB group drag: each footprint in `centres` moves by
    /// `offset` from where it was at the press.
    mutating func moveGroup(footprints centres: [UUID: CGPoint], by offset: CGSize) {
        for index in chips.indices {
            guard let centre = centres[chips[index].id], chips[index].board != nil else { continue }
            let target = CGPoint(x: centre.x + offset.width, y: centre.y + offset.height)
            if chips[index].board?.centre != target { chips[index].board?.centre = target }
        }
    }
}
