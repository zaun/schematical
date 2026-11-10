import CoreGraphics

// MARK: - Units

/// The PCB view draws board millimetres as world units, 20 per mm, so at 100 %
/// a 14-pin SOIC (8.65 mm) is 173 points tall.
nonisolated enum BoardScale {
    static let pointsPerMM: CGFloat = 20
}

/// Footprints and texts snap to the board's component grid
/// (`BoardSettings.grid`, 0.635 mm = 25 mil by default) by their origin
/// point; by default the top-left corner of their bounding box (body plus
/// pads, as turned), so edges of parts line up. Trace ends snap to the
/// trace grid, which has a point between each two of the component grid's
/// (`BoardSettings.traceGrid`).
nonisolated enum BoardGrid {
    /// The default component grid.
    static let spacing: CGFloat = 0.635

    static func snap(_ value: CGFloat, spacing: CGFloat = spacing) -> CGFloat {
        // Adding 0 turns -0 into 0.
        (value / spacing).rounded() * spacing + 0
    }

    static func snap(_ point: CGPoint, spacing: CGFloat = spacing) -> CGPoint {
        CGPoint(x: snap(point.x, spacing: spacing), y: snap(point.y, spacing: spacing))
    }
}

// MARK: - Placement

/// Where a part's footprint sits on the board: its body centre, in mm, a
/// clockwise rotation in 90° steps (as seen from above, on both sides) and
/// its side. Also the footprint's origin point (the point that snaps to the
/// grid) and where its marks go.
///
/// A bottom-side footprint is mirrored left to right about its body centre
/// before it is turned, so every derived point (pads, body, bounds, origin,
/// pin-1 mark) mirrors through `board(_:)`.
nonisolated struct BoardPlacement: Hashable, Sendable {
    var centre: CGPoint
    /// 0, 90, 180 or 270, clockwise.
    var rotation: Int
    /// New placements use the centre of pad 1.
    var origin: FootprintOrigin = .pad1Center
    /// The body outline (the "bounding box" mark).
    var outline: MarkLayer = .document
    var pin1: MarkLayer = .silkscreen
    var reference: MarkLayer = .silkscreen
    /// Full Module or Connector Only; only parts with a connector footprint
    /// use it.
    var style: FootprintStyle = .module
    var side: BoardSide = .top
    /// Locked: dragging, the sidebar's X and Y, and Rotate leave it where
    /// it is. New placements are unlocked.
    var locked = false

    static let rotations = [0, 90, 180, 270]

    /// A footprint point (mm, origin = body centre, y down) on the board:
    /// mirrored on Bottom, then turned, then moved to `centre`.
    func board(_ local: CGPoint) -> CGPoint {
        let flipped = side == .bottom ? CGPoint(x: -local.x, y: local.y) : local
        let turned = BoardPlacement.rotate(flipped, by: rotation)
        return CGPoint(x: centre.x + turned.x, y: centre.y + turned.y)
    }

    /// A footprint-aligned rectangle centred on `local`, on the board. A
    /// mirror leaves the size alone; only the turn swaps it.
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
    let radius: Double
    let chamfer: Double
    /// Already turned with the placement.
    let corners: Footprint.Corners
    /// A slotted hole's length, along the pad's longer side; 0 = round hole.
    var slotLength: Double = 0
    /// False for a non-plated hole: a hole with no copper.
    var plated: Bool = true

    /// The hole's outline box: a `drill` circle, or a `drill` × `slotLength`
    /// slot along the pad's longer side. Nil for a surface-mount pad.
    var holeRect: CGRect? {
        guard drill > 0 else { return nil }
        let long = max(slotLength, drill)
        let size = rect.width >= rect.height ? CGSize(width: long, height: drill) : CGSize(width: drill, height: long)
        return CGRect(x: rect.midX - size.width / 2, y: rect.midY - size.height / 2,
                      width: size.width, height: size.height)
    }

    /// The hole as a segment and radius (a capsule; a point for a round hole).
    var holeSegment: (a: CGPoint, b: CGPoint, radius: Double)? {
        guard let hole = holeRect else { return nil }
        let r = drill / 2
        if hole.width >= hole.height {
            return (CGPoint(x: hole.minX + r, y: hole.midY), CGPoint(x: hole.maxX - r, y: hole.midY), r)
        }
        return (CGPoint(x: hole.midX, y: hole.minY + r), CGPoint(x: hole.midX, y: hole.maxY - r), r)
    }
}

// MARK: - A placed part's footprint

extension PlacedChip {
    /// The footprint on the board: the connector footprint when the style is
    /// Connector Only and the part has one, else the chosen footprint choice,
    /// else the part's footprint.
    nonisolated var footprint: Footprint? {
        let base = footprintChoice?.footprint ?? part.footprint
        return board?.style == .connector ? part.connectorFootprint ?? base : base
    }

    /// The footprint's pads on the board, each belonging to the pin of the same
    /// designator. Pads of `absent` pins are left out; an unnumbered centre pad
    /// is kept. Empty when the part has no footprint or no placement.
    nonisolated var boardPads: [PlacedPad] {
        guard let footprint, let board else { return [] }
        let hidden = Set(part.pins.filter(\.isHidden).map(\.designator))
        return footprint.pads.compactMap { pad in
            if let designator = pad.designator, hidden.contains(designator) { return nil }
            return PlacedPad(designator: pad.designator,
                             rect: board.board(centre: pad.centre, size: pad.size),
                             drill: pad.drill, shape: pad.shape, radius: pad.radius,
                             chamfer: pad.chamfer, corners: pad.corners.mirrored(board.side == .bottom).rotated(by: board.rotation),
                             slotLength: pad.slotLength, plated: pad.plated)
        }
    }

    /// The body outline on the board, and whether it is drawn as an ellipse.
    nonisolated var boardBody: (rect: CGRect, round: Bool)? {
        guard let footprint, let board else { return nil }
        return (board.board(centre: .zero, size: footprint.body), footprint.bodyRound)
    }

    /// Body and pads together, for drawing the selection and hit testing.
    nonisolated var boardBounds: CGRect? {
        guard let footprint, let board else { return nil }
        return BoardLayout.localBounds(of: footprint).applying(board)
    }

    /// The footprint's origin point on the board (see `FootprintOrigin`).
    nonisolated var boardOrigin: CGPoint? {
        guard let footprint, let board else { return nil }
        return BoardLayout.originPoint(of: board, footprint: footprint, pad1: part.firstPinDesignator)
    }

    /// Where the pin-1 mark goes: 0.5 mm outside the body corner nearest the
    /// pad of the part's first pin ("1", or "A1" on a BGA).
    nonisolated var pin1Mark: CGPoint? {
        guard let footprint, let board, let first = part.pins.first,
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

    /// The footprint's origin point on the board. Box points are of the
    /// bounding box (body plus pads, as turned); "Pad 1 Center" is the centre
    /// of the pad named `pad1`, the part's first pin.
    static func originPoint(of placement: BoardPlacement, footprint: Footprint, pad1: String) -> CGPoint {
        let bounds = localBounds(of: footprint).applying(placement)
        switch placement.origin {
        case .center:
            return CGPoint(x: bounds.midX, y: bounds.midY)
        case .topLeft:
            return CGPoint(x: bounds.minX, y: bounds.minY)
        case .centerLeft:
            return CGPoint(x: bounds.minX, y: bounds.midY)
        case .bottomLeft:
            return CGPoint(x: bounds.minX, y: bounds.maxY)
        case .pad1Center:
            // Every part with a footprint has a pad for its first pin (checked
            // over the whole catalog); the body centre only stands in should a
            // part file ever break that.
            guard let pad = footprint.pads.first(where: { $0.designator == pad1 }) else {
                return placement.centre
            }
            return placement.board(pad.centre)
        }
    }

    /// The placement moved so its origin point sits on the component grid
    /// (`grid`, mm).
    static func snapped(_ placement: BoardPlacement, footprint: Footprint, pad1: String,
                        grid: CGFloat = BoardGrid.spacing) -> BoardPlacement {
        let point = originPoint(of: placement, footprint: footprint, pad1: pad1)
        var snapped = placement
        snapped.centre.x += BoardGrid.snap(point.x, spacing: grid) - point.x
        snapped.centre.y += BoardGrid.snap(point.y, spacing: grid) - point.y
        return snapped
    }

    /// Where a newly placed footprint goes: the first at (0, 0); later ones to
    /// the right of everything already placed, 2.54 mm apart, top edges
    /// aligned. Origin the centre of pad 1 (`pad1`, the part's first pin),
    /// snapped to the grid; default marks.
    static func nextSpot(for footprint: Footprint, among chips: [PlacedChip], pad1: String,
                         grid: CGFloat = BoardGrid.spacing) -> BoardPlacement {
        let placed = chips.compactMap(\.boardBounds)
        guard let first = placed.first else {
            return snapped(BoardPlacement(centre: .zero, rotation: 0), footprint: footprint, pad1: pad1, grid: grid)
        }
        let union = placed.dropFirst().reduce(first) { $0.union($1) }
        let local = localBounds(of: footprint)
        let centre = CGPoint(x: union.maxX + 2.54 - local.minX, y: union.minY - local.minY)
        return snapped(BoardPlacement(centre: centre, rotation: 0), footprint: footprint, pad1: pad1, grid: grid)
    }
}

extension CGRect {
    /// This footprint-local rectangle on the board.
    nonisolated func applying(_ placement: BoardPlacement) -> CGRect {
        placement.board(centre: CGPoint(x: midX, y: midY), size: size)
    }
}
