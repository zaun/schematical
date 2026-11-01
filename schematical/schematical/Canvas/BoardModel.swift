import CoreGraphics

// MARK: - Units

/// The PCB view draws board millimetres as world units, 20 per mm, so at 100 %
/// a 14-pin SOIC (8.65 mm) is 173 points tall.
nonisolated enum BoardScale {
    static let pointsPerMM: CGFloat = 20
}

/// Footprints snap to a 0.635 mm (25 mil) grid; 2.54 mm pin pitches land on it.
nonisolated enum BoardGrid {
    static let spacing: CGFloat = 0.635

    static func snap(_ value: CGFloat) -> CGFloat {
        // Adding 0 turns -0 into 0.
        (value / spacing).rounded() * spacing + 0
    }

    static func snap(_ point: CGPoint) -> CGPoint {
        CGPoint(x: snap(point.x), y: snap(point.y))
    }
}

// MARK: - Placement

/// Where a part's footprint sits on the board: its origin (the body centre),
/// in mm, and a clockwise rotation in 90° steps. Top side only.
nonisolated struct BoardPlacement: Hashable, Sendable {
    var centre: CGPoint
    /// 0, 90, 180 or 270, clockwise.
    var rotation: Int

    static let rotations = [0, 90, 180, 270]

    /// A footprint point (mm, origin = body centre, y down) on the board.
    func board(_ local: CGPoint) -> CGPoint {
        let turned = BoardPlacement.rotate(local, by: rotation)
        return CGPoint(x: centre.x + turned.x, y: centre.y + turned.y)
    }

    /// A footprint-aligned rectangle centred on `local`, on the board.
    func board(centre local: CGPoint, size: CGSize) -> CGRect {
        let point = board(local)
        let turned = BoardPlacement.rotate(size, by: rotation)
        return CGRect(x: point.x - turned.width / 2, y: point.y - turned.height / 2,
                      width: turned.width, height: turned.height)
    }

    /// Clockwise in a y-down frame: 90° maps (x, y) to (−y, x).
    static func rotate(_ point: CGPoint, by degrees: Int) -> CGPoint {
        switch ((degrees % 360) + 360) % 360 {
        case 90: return CGPoint(x: -point.y, y: point.x)
        case 180: return CGPoint(x: -point.x, y: -point.y)
        case 270: return CGPoint(x: point.y, y: -point.x)
        default: return point
        }
    }

    /// Width and height swap at 90° and 270°.
    static func rotate(_ size: CGSize, by degrees: Int) -> CGSize {
        ((degrees % 180) + 180) % 180 == 0 ? size : CGSize(width: size.height, height: size.width)
    }
}

/// One pad of a placed footprint, in board mm.
nonisolated struct PlacedPad: Hashable {
    /// Nil for an unnumbered centre pad.
    let designator: String?
    let rect: CGRect
    let drill: Double
    let shape: Footprint.PadShape
}

// MARK: - A placed part's footprint

extension PlacedChip {
    /// The footprint's pads on the board, each belonging to the pin of the same
    /// designator. Pads of `absent` pins are left out; an unnumbered centre pad
    /// is kept. Empty when the part has no footprint or no placement.
    nonisolated var boardPads: [PlacedPad] {
        guard let footprint = part.footprint, let board else { return [] }
        let hidden = Set(part.pins.filter(\.isHidden).map(\.designator))
        return footprint.pads.compactMap { pad in
            if let designator = pad.designator, hidden.contains(designator) { return nil }
            return PlacedPad(designator: pad.designator,
                             rect: board.board(centre: pad.centre, size: pad.size),
                             drill: pad.drill, shape: pad.shape)
        }
    }

    /// The body outline on the board, and whether it is drawn as an ellipse.
    nonisolated var boardBody: (rect: CGRect, round: Bool)? {
        guard let footprint = part.footprint, let board else { return nil }
        return (board.board(centre: .zero, size: footprint.body), footprint.bodyRound)
    }

    /// Body and pads together, for drawing the selection and hit testing.
    nonisolated var boardBounds: CGRect? {
        guard let footprint = part.footprint, let board else { return nil }
        return BoardLayout.localBounds(of: footprint).applying(board)
    }

    /// Where the pin-1 mark goes: 0.5 mm outside the body corner nearest the
    /// pad of the part's first pin ("1", or "A1" on a BGA).
    nonisolated var pin1Mark: CGPoint? {
        guard let footprint = part.footprint, let board, let first = part.pins.first,
              let pad = footprint.pads.first(where: { $0.designator == first.designator }) else {
            return nil
        }
        let w = footprint.body.width / 2, h = footprint.body.height / 2
        let corners = [(-1.0, -1.0), (1.0, -1.0), (1.0, 1.0), (-1.0, 1.0)].map { CGPoint(x: $0.0, y: $0.1) }
        // Ties (pin 1 on an axis, as on an LCCC) go to the first corner in this
        // order, which starts top-left.
        let nearest = corners.min { a, b in
            hypot(a.x * w - pad.centre.x, a.y * h - pad.centre.y)
                < hypot(b.x * w - pad.centre.x, b.y * h - pad.centre.y)
        }!
        let sx = nearest.x, sy = nearest.y
        let offset = 0.5 / 2.0.squareRoot()
        let corner = CGPoint(x: sx * (footprint.body.width / 2 + offset),
                             y: sy * (footprint.body.height / 2 + offset))
        return board.board(corner)
    }
}

nonisolated enum BoardLayout {
    /// A footprint's body plus pads, unrotated, around its origin.
    static func localBounds(of footprint: Footprint) -> CGRect {
        var bounds = CGRect(x: -footprint.body.width / 2, y: -footprint.body.height / 2,
                            width: footprint.body.width, height: footprint.body.height)
        for pad in footprint.pads {
            bounds = bounds.union(CGRect(x: pad.centre.x - pad.size.width / 2,
                                         y: pad.centre.y - pad.size.height / 2,
                                         width: pad.size.width, height: pad.size.height))
        }
        return bounds
    }

    /// Where a newly placed footprint goes: the first at (0, 0); later ones to
    /// the right of everything already placed, 2.54 mm apart, top edges
    /// aligned. Snapped to the grid.
    static func nextSpot(for footprint: Footprint, among chips: [PlacedChip]) -> BoardPlacement {
        let placed = chips.compactMap(\.boardBounds)
        guard let first = placed.first else {
            return BoardPlacement(centre: .zero, rotation: 0)
        }
        let union = placed.dropFirst().reduce(first) { $0.union($1) }
        let local = localBounds(of: footprint)
        let centre = CGPoint(x: union.maxX + 2.54 - local.minX, y: union.minY - local.minY)
        return BoardPlacement(centre: BoardGrid.snap(centre), rotation: 0)
    }
}

private extension CGRect {
    /// This footprint-local rectangle on the board.
    func applying(_ placement: BoardPlacement) -> CGRect {
        placement.board(centre: CGPoint(x: midX, y: midY), size: size)
    }
}
