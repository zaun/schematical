import CoreGraphics
import Foundation

/// The board's pours, keep-outs, cutouts and holes, and their edits
/// (plan-68-E). The trace grid and the layers come in as parameters.
struct BoardFeatures {
    var holes: [BoardHole]
    var pours: [BoardPour]
    var keepouts: [BoardKeepout]
    var cutouts: [BoardCutout]

    /// `rect` snapped to `grid`, at least `BoardPour.minimumSide` wide and tall.
    static func snapped(_ rect: CGRect, grid: Double) -> CGRect {
        let r = rect.standardized
        let minX = BoardGrid.snap(r.minX, spacing: grid), minY = BoardGrid.snap(r.minY, spacing: grid)
        let maxX = BoardGrid.snap(r.maxX, spacing: grid), maxY = BoardGrid.snap(r.maxY, spacing: grid)
        return BoardPour.normalized(CGRect(x: minX, y: minY, width: maxX - minX, height: maxY - minY))
    }

    // MARK: Pours

    /// Adds a pour over `rect` (snapped to `grid`) on `layer`, with the
    /// settings of the last pour; the first takes the start values and
    /// ground when `ground` (the schematic has it).
    mutating func addPour(rect: CGRect, layer: CopperLayer, grid: Double, ground: Bool) -> UUID {
        var pour = BoardPour(id: UUID(), rect: Self.snapped(rect, grid: grid), layer: layer)
        if let like = pours.last {
            pour.net = like.net
            pour.clearance = like.clearance
            pour.connection = like.connection
            pour.thermalGap = like.thermalGap
            pour.spokeWidth = like.spokeWidth
            pour.minWidth = like.minWidth
        } else {
            pour.clearance = BoardPour.startClearance
            pour.minWidth = BoardPour.startMinWidth
            if ground { pour.net = .ground }
        }
        pours.append(pour)
        return pour.id
    }

    /// Changes pour `id` (unlocked only when `unlockedOnly`).
    private mutating func changePour(_ id: UUID, unlockedOnly: Bool = false, _ change: (inout BoardPour) -> Void) {
        guard let index = pours.firstIndex(where: { $0.id == id }), !unlockedOnly || !pours[index].locked else { return }
        var pour = pours[index]
        change(&pour)
        if pour != pours[index] { pours[index] = pour }
    }

    /// Moves an unlocked pour's top-left corner to `origin` snapped to `grid`.
    mutating func movePour(_ id: UUID, to origin: CGPoint, grid: Double) {
        changePour(id, unlockedOnly: true) { $0.rect = CGRect(origin: BoardGrid.snap(origin, spacing: grid), size: $0.rect.size) }
    }

    mutating func setPourRect(_ id: UUID, _ rect: CGRect, grid: Double) {
        changePour(id, unlockedOnly: true) { $0.rect = Self.snapped(rect, grid: grid) }
    }

    mutating func setPourLayer(_ id: UUID, _ layer: CopperLayer) {
        changePour(id) { $0.layer = layer }
    }

    mutating func setPourNet(_ id: UUID, _ net: PlaneNet?) {
        changePour(id) { $0.net = net }
    }

    /// A pour's clearance (mm), clamped to `LayerStyle.clearanceRange`; nil
    /// is the DRC's minimum.
    mutating func setPourClearance(_ id: UUID, _ mm: Double?) {
        let value = mm.map { min(max($0, LayerStyle.clearanceRange.lowerBound), LayerStyle.clearanceRange.upperBound) }
        changePour(id) { $0.clearance = value }
    }

    mutating func setPourConnection(_ id: UUID, _ connection: PourConnection) {
        changePour(id) { $0.connection = connection }
    }

    mutating func setPourThermalGap(_ id: UUID, _ mm: Double) {
        changePour(id) { $0.thermalGap = min(max(mm, BoardPour.gapRange.lowerBound), BoardPour.gapRange.upperBound) }
    }

    mutating func setPourSpokeWidth(_ id: UUID, _ mm: Double) {
        changePour(id) { $0.spokeWidth = min(max(mm, BoardPour.spokeRange.lowerBound), BoardPour.spokeRange.upperBound) }
    }

    mutating func setPourMinWidth(_ id: UUID, _ mm: Double) {
        changePour(id) { $0.minWidth = min(max(mm, BoardPour.minWidthRange.lowerBound), BoardPour.minWidthRange.upperBound) }
    }

    // MARK: Keep-outs

    /// Adds a keep-out over `rect` (snapped to `grid`) on `layers` (nil: every layer).
    mutating func addKeepout(rect: CGRect, layers: Set<CopperLayer>?, grid: Double) -> UUID {
        let keepout = BoardKeepout(id: UUID(), rect: Self.snapped(rect, grid: grid), layers: layers)
        keepouts.append(keepout)
        return keepout.id
    }

    private mutating func changeKeepout(_ id: UUID, unlockedOnly: Bool, _ change: (inout BoardKeepout) -> Void) {
        guard let index = keepouts.firstIndex(where: { $0.id == id }), !unlockedOnly || !keepouts[index].locked else { return }
        var keepout = keepouts[index]
        change(&keepout)
        if keepout != keepouts[index] { keepouts[index] = keepout }
    }

    mutating func moveKeepout(_ id: UUID, to origin: CGPoint, grid: Double) {
        changeKeepout(id, unlockedOnly: true) {
            $0.rect = CGRect(origin: BoardGrid.snap(origin, spacing: grid), size: $0.rect.size)
        }
    }

    mutating func setKeepoutRect(_ id: UUID, _ rect: CGRect, grid: Double) {
        changeKeepout(id, unlockedOnly: true) { $0.rect = Self.snapped(rect, grid: grid) }
    }

    /// The layers a keep-out applies to; nil is every layer.
    mutating func setKeepoutLayers(_ id: UUID, _ layers: Set<CopperLayer>?) {
        changeKeepout(id, unlockedOnly: false) { $0.layers = layers }
    }

    // MARK: Cutouts

    /// Adds a cutout of `shape` (already checked by `BoardCutout.problem`):
    /// a rounded shape's box snapped to `grid`, a circle kept square and a
    /// circle or slot fully rounded; a polygon stored clockwise.
    mutating func addCutout(_ shape: BoardCutout.Shape, grid: Double) -> UUID {
        let stored: BoardCutout.Shape
        switch shape {
        case let .rounded(rect, radius):
            let kind = BoardCutout(id: UUID(), shape: shape).kind
            var box = Self.snapped(rect, grid: grid)
            if kind == .circle {
                let side = max(box.width, box.height)
                box.size = CGSize(width: side, height: side)
            }
            let fitted = BoardCutout.normalized(box, radius: radius)
            stored = .rounded(rect: fitted.rect,
                              radius: kind != .rectangle ? min(fitted.rect.width, fitted.rect.height) / 2 : fitted.radius)
        case let .polygon(points):
            stored = .polygon(BoardGeometry.clockwise(points))
        }
        let cutout = BoardCutout(id: UUID(), shape: stored)
        cutouts.append(cutout)
        return cutout.id
    }

    /// Changes an unlocked cutout's shape.
    private mutating func changeCutout(_ id: UUID, _ change: (BoardCutout) -> BoardCutout.Shape?) {
        guard let index = cutouts.firstIndex(where: { $0.id == id }), !cutouts[index].locked,
              let shape = change(cutouts[index]), shape != cutouts[index].shape else { return }
        cutouts[index].shape = shape
    }

    /// Moves a cutout so its bounds' top-left is at `origin` snapped to `grid`.
    mutating func moveCutout(_ id: UUID, to origin: CGPoint, grid: Double) {
        changeCutout(id) { cutout in
            let target = BoardGrid.snap(origin, spacing: grid)
            let from = cutout.shape.bounds.origin
            return cutout.shape.translated(by: CGSize(width: target.x - from.x, height: target.y - from.y))
        }
    }

    /// A rounded cutout's box (snapped to `grid`); its radius stays within
    /// the box, a circle or slot fully rounded. A polygon is left alone.
    mutating func setCutoutRect(_ id: UUID, _ rect: CGRect, grid: Double) {
        changeCutout(id) { cutout in
            guard case let .rounded(_, radius) = cutout.shape else { return nil }
            let fitted = BoardCutout.normalized(Self.snapped(rect, grid: grid), radius: radius)
            return .rounded(rect: fitted.rect, radius: cutout.kind != .rectangle
                                ? min(fitted.rect.width, fitted.rect.height) / 2 : fitted.radius)
        }
    }

    /// A rounded cutout's corner radius, within 0 to half its short side.
    mutating func setCutoutRadius(_ id: UUID, _ radius: Double) {
        changeCutout(id) { cutout in
            guard case let .rounded(rect, _) = cutout.shape else { return nil }
            let fitted = BoardCutout.normalized(rect, radius: radius)
            return .rounded(rect: fitted.rect, radius: fitted.radius)
        }
    }

    // MARK: Holes

    /// The hole under `point` (mm): the last one drawn there, within its
    /// land (plated) or its hole.
    func hole(at point: CGPoint) -> BoardHole? {
        holes.last { SchematicDocument.distance(from: point, to: $0.centre) <= $0.outer / 2 }
    }

    /// Adds a hole at `point` snapped to `grid`, with the size, plating and
    /// net of the last hole; the first is a non-plated M3.
    mutating func addHole(at point: CGPoint, grid: Double) -> UUID {
        let like = holes.last ?? BoardHole(id: UUID(), centre: .zero)
        let hole = BoardHole(id: UUID(), centre: BoardGrid.snap(point, spacing: grid), size: like.size,
                             drill: like.drill, diameter: like.diameter, plated: like.plated, net: like.net)
        holes.append(hole)
        return hole.id
    }

    private mutating func changeHole(_ id: UUID, unlockedOnly: Bool = false, _ change: (inout BoardHole) -> Void) {
        guard let index = holes.firstIndex(where: { $0.id == id }), !unlockedOnly || !holes[index].locked else { return }
        var hole = holes[index]
        change(&hole)
        if hole != holes[index] { holes[index] = hole }
    }

    /// Moves an unlocked hole's centre to `point` (already where it goes).
    mutating func setHoleCentre(_ id: UUID, _ point: CGPoint) {
        changeHole(id, unlockedOnly: true) { $0.centre = point }
    }

    /// A screw size sets the hole and land.
    mutating func setHoleSize(_ id: UUID, _ size: HoleSize) {
        changeHole(id) { $0.apply(size) }
    }

    /// A hole's drill, clamped to `BoardHole.drillRange`; the land grows to
    /// keep the thinnest ring, and a changed drill makes the size Custom.
    mutating func setHoleDrill(_ id: UUID, _ mm: Double) {
        changeHole(id) { hole in
            let drill = min(max(mm, BoardHole.drillRange.lowerBound), BoardHole.drillRange.upperBound)
            if hole.drill != drill { hole.size = .custom }
            hole.drill = drill
            hole.diameter = max(hole.diameter, drill + 2 * BoardHole.minimumRing)
        }
    }

    /// A hole's land, clamped to `BoardHole.diameterRange` and the thinnest
    /// ring round its hole; a changed land makes the size Custom.
    mutating func setHoleDiameter(_ id: UUID, _ mm: Double) {
        changeHole(id) { hole in
            let least = max(BoardHole.diameterRange.lowerBound, hole.drill + 2 * BoardHole.minimumRing)
            let value = max(min(mm, BoardHole.diameterRange.upperBound), least)
            guard hole.diameter != value else { return }
            hole.diameter = value
            hole.size = .custom
        }
    }

    mutating func setHolePlated(_ id: UUID, _ plated: Bool) {
        changeHole(id) { $0.plated = plated }
    }

    mutating func setHoleNet(_ id: UUID, _ net: PlaneNet?) {
        changeHole(id) { $0.net = net }
    }

    // MARK: Locks, removal and group drags

    mutating func setLocked(_ item: BoardItem, _ locked: Bool) {
        switch item {
        case let .hole(id): if let i = holes.firstIndex(where: { $0.id == id }) { holes[i].locked = locked }
        case let .pour(id): if let i = pours.firstIndex(where: { $0.id == id }) { pours[i].locked = locked }
        case let .keepout(id): if let i = keepouts.firstIndex(where: { $0.id == id }) { keepouts[i].locked = locked }
        case let .cutout(id): if let i = cutouts.firstIndex(where: { $0.id == id }) { cutouts[i].locked = locked }
        default: break
        }
    }

    mutating func remove(holes holeIDs: Set<UUID>, pours pourIDs: Set<UUID>, keepouts keepoutIDs: Set<UUID>,
                         cutouts cutoutIDs: Set<UUID>) {
        holes.removeAll { holeIDs.contains($0.id) }
        pours.removeAll { pourIDs.contains($0.id) }
        keepouts.removeAll { keepoutIDs.contains($0.id) }
        cutouts.removeAll { cutoutIDs.contains($0.id) }
    }

    /// One frame of a group drag: everything in `base` moves by `offset`
    /// from where it was at the press.
    mutating func moveGroup(_ base: BoardGroupBase, by offset: CGSize) {
        func moved(_ point: CGPoint) -> CGPoint { CGPoint(x: point.x + offset.width, y: point.y + offset.height) }
        for index in pours.indices {
            guard let rect = base.pours[pours[index].id] else { continue }
            let target = CGRect(origin: moved(rect.origin), size: rect.size)
            if pours[index].rect != target { pours[index].rect = target }
        }
        for index in keepouts.indices {
            guard let rect = base.keepouts[keepouts[index].id] else { continue }
            let target = CGRect(origin: moved(rect.origin), size: rect.size)
            if keepouts[index].rect != target { keepouts[index].rect = target }
        }
        for index in cutouts.indices {
            guard let shape = base.cutouts[cutouts[index].id] else { continue }
            let target = shape.translated(by: offset)
            if cutouts[index].shape != target { cutouts[index].shape = target }
        }
        for index in holes.indices {
            guard let centre = base.holes[holes[index].id] else { continue }
            let target = moved(centre)
            if holes[index].centre != target { holes[index].centre = target }
        }
    }

    // MARK: Hits

    /// Whether a keep-out is drawn with `visible` on a board with `layers`:
    /// any layer it applies to shows.
    static func keepoutIsShown(_ keepout: BoardKeepout, _ visible: BoardVisibility, layers: [CopperLayer]) -> Bool {
        guard let applies = keepout.layers else { return visible.showsAnyCopper }
        return applies.contains { visible.shows(.copper($0)) && layers.contains($0) }
    }

    /// The pour, keep-out or cutout whose edge is within `tolerance` of
    /// `point` (mm): the last pour, else the last keep-out, else the last
    /// cutout. Pressing inside one does not hit it, only its edge does.
    func item(at point: CGPoint, tolerance: CGFloat, visible: BoardVisibility, layers: [CopperLayer]) -> BoardItem? {
        if let pour = pours.last(where: {
            visible.shows(.copper($0.layer)) && layers.contains($0.layer)
                && Self.distance(from: point, toBoundaryOf: $0.rect) <= tolerance
        }) { return .pour(pour.id) }
        if let keepout = keepouts.last(where: {
            Self.keepoutIsShown($0, visible, layers: layers) && Self.distance(from: point, toBoundaryOf: $0.rect) <= tolerance
        }) { return .keepout(keepout.id) }
        if let cutout = cutouts.last(where: { Double($0.shape.distance(to: point)) <= Double(tolerance) }) {
            return .cutout(cutout.id)
        }
        return nil
    }

    /// How far `point` is from the boundary of `rect`, inside or out.
    static func distance(from point: CGPoint, toBoundaryOf rect: CGRect) -> CGFloat {
        let dx = max(rect.minX - point.x, 0, point.x - rect.maxX)
        let dy = max(rect.minY - point.y, 0, point.y - rect.maxY)
        if dx > 0 || dy > 0 { return hypot(dx, dy) }
        return min(point.x - rect.minX, rect.maxX - point.x, point.y - rect.minY, rect.maxY - point.y)
    }
}
