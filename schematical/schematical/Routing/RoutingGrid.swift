import CoreGraphics
import Foundation

/// What a net's copper needs from the grid: half its trace width, its
/// clearance and its via's radius, in mm.
nonisolated struct RoutingProfile: Hashable, Sendable {
    var halfWidth: Double
    var clearance: Double
    var viaRadius: Double
    /// Half its via's drill: a via keeps the DRC's hole-to-hole distance
    /// from every other hole, whatever its net.
    var viaDrillRadius: Double
}

/// The routing grid (plan-38-E § 3): cells `pitch` apart from the outline's
/// bounding-box corner, one plane of cells per routable layer, and for each
/// net profile two maps per layer — where a trace centre of that profile
/// may go (`traceMaps`) and where a via of it may stand (`viaMaps`). A cell
/// holds `free` (-1), a net id n ≥ 0 (covered by net n's copper only, so
/// passable for n), or `blocked` (-2: two nets, a hole, the edge, outside).
/// Every stamp is by exact distance to the copper's core
/// (`CopperGeometry`), never by disks.
nonisolated struct RoutingGrid: Sendable {
    static let free: Int32 = -1
    static let blocked: Int32 = -2

    let pitch: Double
    /// The centre of cell (0, 0).
    let origin: CGPoint
    let nx: Int
    let ny: Int
    var cellCount: Int { nx * ny }
    /// The routable layers, in stack order; a layer index is a place here.
    let layers: [CopperLayer]
    let profiles: [RoutingProfile]
    /// Each routed net's profile index.
    let profileOfNet: [Int: Int]
    /// The clearance each net's copper keeps (routed nets' own; others the
    /// DRC's minimum).
    let clearanceOfNet: [Int: Double]
    let rules: DesignRules

    /// One flat array per kind, indexed
    /// `(profile * layers.count + layer) * cellCount + cell`.
    var traceMaps: [Int32]
    var viaMaps: [Int32]
    /// The maps with only the fixed copper (obstacles, kept copper, edge),
    /// before anything is routed: what un-stamping restores (plan-38-F).
    private(set) var fixedTraceMaps: [Int32] = []
    private(set) var fixedViaMaps: [Int32] = []

    init(problem: RoutingProblem) {
        let rules = problem.rules
        self.rules = rules
        pitch = problem.options.grid
        let xs = problem.outline.map(\.x), ys = problem.outline.map(\.y)
        let minX = xs.min() ?? 0, minY = ys.min() ?? 0, maxX = xs.max() ?? 0, maxY = ys.max() ?? 0
        origin = CGPoint(x: minX, y: minY)
        nx = Int(((maxX - minX) / pitch).rounded(.down)) + 1
        ny = Int(((maxY - minY) / pitch).rounded(.down)) + 1
        layers = problem.routable

        var profiles: [RoutingProfile] = []
        var index: [RoutingProfile: Int] = [:]
        var profileOfNet: [Int: Int] = [:]
        var clearanceOfNet: [Int: Double] = [:]
        for net in problem.nets.sorted(by: { $0.id < $1.id }) {
            let profile = RoutingProfile(halfWidth: net.width / 2, clearance: net.clearance,
                                         viaRadius: net.viaDiameter / 2, viaDrillRadius: net.viaDrill / 2)
            if index[profile] == nil {
                index[profile] = profiles.count
                profiles.append(profile)
            }
            profileOfNet[net.id] = index[profile]
            clearanceOfNet[net.id] = net.clearance
        }
        if profiles.isEmpty {
            let standard = NetRules.defaults(for: rules)
            profiles.append(RoutingProfile(halfWidth: max(rules.minTraceWidth, Double(standard.width) * 0.0254) / 2,
                                           clearance: rules.minClearance, viaRadius: standard.viaDiameter / 2,
                                           viaDrillRadius: standard.viaDrill / 2))
        }
        self.profiles = profiles
        self.profileOfNet = profileOfNet
        self.clearanceOfNet = clearanceOfNet

        let count = nx * ny
        let maps = profiles.count * max(layers.count, 1)
        traceMaps = Array(repeating: Self.free, count: count * maps)
        viaMaps = traceMaps

        // Fixed copper and holes. The outline is the edge keep-out, below.
        for obstacle in problem.obstacles where obstacle.shape.node != -1 {
            let owner = obstacle.net.map { Int32($0) } ?? Self.blocked
            // A non-plated hole keeps the copper-to-edge distance, as the DRC holds it.
            let clearance = obstacle.kind == .hole ? rules.minCopperToEdge
                : obstacle.net.flatMap { clearanceOfNet[$0] } ?? rules.minClearance
            stamp(obstacle.shape.core, radius: obstacle.shape.radius, clearance: clearance, owner: owner,
                  layers: layerIndices(obstacle.shape.layers))
            if let drill = obstacle.drill { stampHole(drill.core, radius: drill.radius) }
        }
        stampEdge(problem.edge)
        fixedTraceMaps = traceMaps
        fixedViaMaps = viaMaps
    }

    // MARK: Cells

    func cell(_ ix: Int, _ iy: Int) -> Int { iy * nx + ix }
    func ix(_ cell: Int) -> Int { cell % nx }
    func iy(_ cell: Int) -> Int { cell / nx }

    /// A cell's centre, mm.
    func point(_ cell: Int) -> CGPoint {
        CGPoint(x: origin.x + Double(cell % nx) * pitch, y: origin.y + Double(cell / nx) * pitch)
    }

    /// The cell whose centre is nearest `point`, clamped to the grid.
    func nearestCell(_ point: CGPoint) -> Int {
        let x = min(max(Int(((point.x - origin.x) / pitch).rounded()), 0), nx - 1)
        let y = min(max(Int(((point.y - origin.y) / pitch).rounded()), 0), ny - 1)
        return cell(x, y)
    }

    /// The cell index range whose centres lie within `rect` grown by `margin`.
    func cellRange(_ rect: CGRect, margin: Double) -> (x: ClosedRange<Int>, y: ClosedRange<Int>)? {
        let x0 = max(0, Int(((rect.minX - margin - origin.x) / pitch).rounded(.up)))
        let x1 = min(nx - 1, Int(((rect.maxX + margin - origin.x) / pitch).rounded(.down)))
        let y0 = max(0, Int(((rect.minY - margin - origin.y) / pitch).rounded(.up)))
        let y1 = min(ny - 1, Int(((rect.maxY + margin - origin.y) / pitch).rounded(.down)))
        guard x0 <= x1, y0 <= y1 else { return nil }
        return (x0...x1, y0...y1)
    }

    /// The layer indices of `layers` (nil: every routable layer).
    func layerIndices(_ set: Set<CopperLayer>?) -> [Int] {
        guard let set else { return Array(layers.indices) }
        return layers.indices.filter { set.contains(layers[$0]) }
    }

    func layerIndex(_ layer: CopperLayer) -> Int? { layers.firstIndex(of: layer) }

    // MARK: Passability

    /// The profile index of net `net`.
    func profile(of net: Int) -> Int { profileOfNet[net] ?? 0 }

    /// Whether a trace centre of net `net` may be at `cell` on layer `layer`.
    func isPassable(_ cell: Int, layer: Int, net: Int, profile: Int) -> Bool {
        let value = traceMaps[(profile * layers.count + layer) * cellCount + cell]
        return value == Self.free || value == Int32(net)
    }

    /// Whether a via of net `net` may stand at `cell`: free or the net's own
    /// on every routable layer (a via goes through them all).
    func viaFits(_ cell: Int, net: Int, profile: Int) -> Bool {
        let base = profile * layers.count
        for layer in layers.indices {
            let value = viaMaps[(base + layer) * cellCount + cell]
            if value != Self.free && value != Int32(net) { return false }
        }
        return true
    }

    /// The trace-map value at a cell (tests and diagnostics).
    func traceValue(_ cell: Int, layer: Int, profile: Int) -> Int32 {
        traceMaps[(profile * layers.count + layer) * cellCount + cell]
    }

    func viaValue(_ cell: Int, layer: Int, profile: Int) -> Int32 {
        viaMaps[(profile * layers.count + layer) * cellCount + cell]
    }

    // MARK: Stamping

    private static func mark(_ map: inout [Int32], _ index: Int, _ owner: Int32) {
        let value = map[index]
        if value == free {
            map[index] = owner
        } else if value != owner {
            map[index] = blocked
        }
    }

    /// The distance from `point` to a core.
    static func distance(_ point: CGPoint, _ core: CopperShape.Core) -> Double {
        switch core {
        case let .segment(a, b): SchematicDocument.distance(from: point, toSegment: a, b)
        case let .rect(rect): CopperGeometry.pointDistance(point, rect)
        }
    }

    /// Stamps copper (a core grown by `radius`, keeping `clearance`) owned
    /// by `owner` (a net, or `blocked`) on `layers`, into every profile's
    /// maps: a trace centre of profile p may not come within
    /// `max(clearance, p.clearance) + radius + p.halfWidth` of the core,
    /// nor a via centre within `… + p.viaRadius` (strictly closer is
    /// covered).
    mutating func stamp(_ core: CopperShape.Core, radius: Double, clearance: Double, owner: Int32, layers indices: [Int]) {
        guard !indices.isEmpty else { return }
        let box = CopperShape(core: core, radius: 0, layers: nil, node: 0).bounds
        let reach = profiles.map { max(clearance, $0.clearance) + radius + max($0.halfWidth, $0.viaRadius) }.max() ?? 0
        guard let range = cellRange(box, margin: reach) else { return }
        let count = layers.count, cells = cellCount
        for y in range.y {
            for x in range.x {
                let c = cell(x, y)
                let d = Self.distance(point(c), core)
                guard d < reach else { continue }
                for (p, profile) in profiles.enumerated() {
                    let gap = max(clearance, profile.clearance) + radius
                    let onTrace = d < gap + profile.halfWidth, onVia = d < gap + profile.viaRadius
                    guard onTrace || onVia else { continue }
                    for layer in indices {
                        if onTrace { Self.mark(&traceMaps, (p * count + layer) * cells + c, owner) }
                        if onVia { Self.mark(&viaMaps, (p * count + layer) * cells + c, owner) }
                    }
                }
            }
        }
    }

    /// Blocks every cell outside the outline, and every cell where a trace
    /// or via centre would put copper within the DRC's copper-to-edge
    /// distance of it. This holds at edge fingers too: the DRC exempts only
    /// the finger pad itself, never the trace that enters it (`checkEdge`),
    /// so a net reaches a finger through its inner part.
    private mutating func stampEdge(_ boardEdge: BoardEdge) {
        guard boardEdge.outline.count >= 3 else { return }
        let count = layers.count
        let edges = boardEdge.segments
        let outlineEdges = Array(zip(boardEdge.outline, boardEdge.outline.dropFirst() + boardEdge.outline.prefix(1)))
        let cutoutEdges = boardEdge.cutouts.filter { $0.count >= 3 }.map { loop in
            Array(zip(loop, loop.dropFirst() + loop.prefix(1)))
        }
        /// The sorted x where a loop's edges cross the row at `py`.
        func crossingsOf(_ loop: [(CGPoint, CGPoint)], at py: Double) -> [Double] {
            var result: [Double] = []
            for (a, b) in loop where (a.y > py) != (b.y > py) {
                result.append(a.x + (py - a.y) * (b.x - a.x) / (b.y - a.y))
            }
            return result.sorted()
        }
        // Off the material: per row, the spans outside the outline and inside any cutout (plan-65-D).
        for y in 0..<ny {
            let py = origin.y + Double(y) * pitch
            let crossings = crossingsOf(outlineEdges, at: py)
            let cutoutCrossings = cutoutEdges.map { crossingsOf($0, at: py) }
            for x in 0..<nx {
                let px = origin.x + Double(x) * pitch
                let inside = crossings.filter { $0 < px }.count % 2 == 1
                    && !cutoutCrossings.contains { $0.filter { $0 < px }.count % 2 == 1 }
                guard !inside else { continue }
                let c = cell(x, y)
                for map in 0..<(traceMaps.count / cellCount) {
                    traceMaps[map * cellCount + c] = Self.blocked
                    viaMaps[map * cellCount + c] = Self.blocked
                }
            }
        }
        let edge = rules.minCopperToEdge
        for (a, b) in edges {
            let core = CopperShape.Core.segment(a, b)
            let box = CopperShape(core: core, radius: 0, layers: nil, node: 0).bounds
            let reach = edge + (profiles.map { max($0.halfWidth, $0.viaRadius) }.max() ?? 0)
            guard let range = cellRange(box, margin: reach) else { continue }
            for y in range.y {
                for x in range.x {
                    let c = cell(x, y)
                    let d = Self.distance(point(c), core)
                    for (p, profile) in profiles.enumerated() {
                        for layer in 0..<count {
                            if d < edge + profile.halfWidth { traceMaps[(p * count + layer) * cellCount + c] = Self.blocked }
                            if d < edge + profile.viaRadius { viaMaps[(p * count + layer) * cellCount + c] = Self.blocked }
                        }
                    }
                }
            }
        }
    }

    /// Stamps a routed trace of net `net` (width in mm) on `layer`.
    mutating func stampTrace(_ points: [CGPoint], width: Double, layer: Int, net: Int) {
        let clearance = clearanceOfNet[net] ?? rules.minClearance
        for (a, b) in zip(points, points.dropFirst()) {
            stamp(.segment(a, b), radius: width / 2, clearance: clearance, owner: Int32(net), layers: [layer])
        }
    }

    /// Stamps a routed via of net `net` on every layer, and its hole.
    mutating func stampVia(_ centre: CGPoint, diameter: Double, drill: Double, net: Int) {
        let clearance = clearanceOfNet[net] ?? rules.minClearance
        stamp(.segment(centre, centre), radius: diameter / 2, clearance: clearance, owner: Int32(net),
              layers: Array(layers.indices))
        stampHole(.segment(centre, centre), radius: drill / 2)
    }

    /// Blocks, on every via map, the via centres whose hole would come
    /// within the DRC's hole-to-hole distance of a hole (a pad's drill, a
    /// via's) of any net: closer than `radius + minHoleToHole + the
    /// profile's via drill radius`.
    mutating func stampHole(_ core: CopperShape.Core, radius: Double) {
        let box = CopperShape(core: core, radius: 0, layers: nil, node: 0).bounds
        let reach = radius + rules.minHoleToHole + (profiles.map(\.viaDrillRadius).max() ?? 0)
        guard let range = cellRange(box, margin: reach) else { return }
        let count = layers.count, cells = cellCount
        for y in range.y {
            for x in range.x {
                let c = cell(x, y)
                let d = Self.distance(point(c), core)
                for (p, profile) in profiles.enumerated() where d < radius + rules.minHoleToHole + profile.viaDrillRadius {
                    for layer in 0..<count { viaMaps[(p * count + layer) * cells + c] = Self.blocked }
                }
            }
        }
    }
}
