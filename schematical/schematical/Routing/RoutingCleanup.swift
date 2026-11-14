import CoreGraphics
import Foundation

/// Copper by 1 mm tile, for asking "is this new piece clear of every other
/// net's copper" without looking at the whole board.
nonisolated struct CopperIndex: Sendable {
    struct Item: Sendable {
        var shape: CopperShape
        var net: Int?
        /// What it belongs to: a cleanup trace (>= 0), a cleanup via
        /// (-2 - index), fixed copper (-1).
        var owner: Int
    }

    static let tile = 1.0
    private(set) var items: [Int: Item] = [:]
    private var tiles: [Int: [Int]] = [:]
    private var byOwner: [Int: [Int]] = [:]
    private var nextID = 0

    private func keys(_ box: CGRect) -> [Int] {
        let x0 = Int((box.minX / Self.tile).rounded(.down)), x1 = Int((box.maxX / Self.tile).rounded(.down))
        let y0 = Int((box.minY / Self.tile).rounded(.down)), y1 = Int((box.maxY / Self.tile).rounded(.down))
        var keys: [Int] = []
        for y in y0...max(y0, y1) { for x in x0...max(x0, x1) { keys.append(y &* 1_000_003 &+ x) } }
        return keys
    }

    mutating func insert(_ shape: CopperShape, net: Int?, owner: Int) {
        let id = nextID
        nextID += 1
        items[id] = Item(shape: shape, net: net, owner: owner)
        for key in keys(shape.bounds) { tiles[key, default: []].append(id) }
        if owner != -1 { byOwner[owner, default: []].append(id) }
    }

    mutating func remove(owner: Int) {
        for id in byOwner[owner] ?? [] {
            if let item = items[id] { for key in keys(item.shape.bounds) { tiles[key]?.removeAll { $0 == id } } }
            items[id] = nil
        }
        byOwner[owner] = nil
    }

    /// The items whose bounds come within `margin` of `box`, by id.
    func near(_ box: CGRect, margin: Double) -> [Item] {
        var ids = Set<Int>()
        for key in keys(box.insetBy(dx: -margin, dy: -margin)) { ids.formUnion(tiles[key] ?? []) }
        return ids.sorted().compactMap { items[$0] }
    }
}

/// The cleanup of a routed result (plan-38-H § 3 steps 1–6): each net's new
/// traces as chains between anchors (their ends, the points where other
/// copper of the net attaches, via centres); chains straightened into
/// octilinear runs, 90° corners chamfered, via hops over short runs taken
/// out, stubs and collinear vertices removed. Every change is checked
/// exactly against every other net's copper and the board's edge, and
/// never makes a trace longer; anchors never move.
nonisolated struct RoutingCleanup {
    let problem: RoutingProblem
    /// The new copper, as it stands; each piece's ledger entry.
    private(set) var traces: [RoutedTrace]
    private(set) var vias: [RoutedVia]
    private(set) var traceEntry: [Int]
    private(set) var viaEntry: [Int]
    /// Pieces taken out (via hops, stubs): skipped in every pass.
    private(set) var deadTraces = Set<Int>()
    private(set) var deadVias = Set<Int>()
    private var index = CopperIndex()
    private let clearanceOf: [Int: Double]
    private let maxClearance: Double
    private let edges: [CopperShape]

    init(problem: RoutingProblem, traces: [RoutedTrace], vias: [RoutedVia], traceEntry: [Int], viaEntry: [Int]) {
        self.problem = problem
        self.traces = traces
        self.vias = vias
        self.traceEntry = traceEntry
        self.viaEntry = viaEntry
        var clearance: [Int: Double] = [:]
        for net in problem.nets { clearance[net.id] = max(net.clearance, problem.rules.minClearance) }
        clearanceOf = clearance
        maxClearance = max(clearance.values.max() ?? 0, problem.rules.minClearance)
        let outline = problem.outline
        edges = zip(outline, outline.dropFirst() + outline.prefix(1)).map {
            CopperShape(core: .segment($0, $1), radius: 0, layers: nil, node: -1)
        }
        for obstacle in problem.obstacles where [.pad, .trace, .via].contains(obstacle.kind) {
            index.insert(obstacle.shape, net: obstacle.net, owner: -1)
        }
        for t in traces.indices { indexTrace(t) }
        for v in vias.indices {
            index.insert(Self.viaShape(vias[v]), net: vias[v].net, owner: -2 - v)
        }
    }

    /// The copper left, with each piece's entry.
    var result: (traces: [RoutedTrace], vias: [RoutedVia], traceEntry: [Int], viaEntry: [Int]) {
        let t = traces.indices.filter { !deadTraces.contains($0) }
        let v = vias.indices.filter { !deadVias.contains($0) }
        return (t.map { traces[$0] }, v.map { vias[$0] }, t.map { traceEntry[$0] }, v.map { viaEntry[$0] })
    }

    static func viaShape(_ via: RoutedVia) -> CopperShape {
        CopperShape(core: .segment(via.centre, via.centre), radius: via.diameter / 2, layers: nil, node: 0)
    }

    private mutating func indexTrace(_ t: Int) {
        index.remove(owner: t)
        guard !deadTraces.contains(t) else { return }
        let trace = traces[t]
        for (a, b) in zip(trace.points, trace.points.dropFirst()) {
            index.insert(CopperShape(core: .segment(a, b), radius: trace.width / 2, layers: [trace.layer], node: 0),
                         net: trace.net, owner: t)
        }
    }

    // MARK: Legality

    private func clearance(_ net: Int?) -> Double {
        net.flatMap { clearanceOf[$0] } ?? problem.rules.minClearance
    }

    /// Whether a run of `points` on `layer`, `width` wide, of net `net`
    /// keeps every rule against everything not of its net.
    func isLegal(_ points: [CGPoint], layer: CopperLayer, width: Double, net: Int?) -> Bool {
        let mine = clearance(net)
        for (a, b) in zip(points, points.dropFirst()) {
            let shape = CopperShape(core: .segment(a, b), radius: width / 2, layers: [layer], node: 0)
            for item in index.near(shape.bounds, margin: maxClearance + mine) {
                if let n = item.net, n == net { continue }
                guard item.shape.shares(shape) else { continue }
                if shape.gap(to: item.shape) < max(mine, clearance(item.net)) { return false }
            }
            for edge in edges where shape.gap(to: edge) < problem.rules.minCopperToEdge { return false }
            let mid = CGPoint(x: (a.x + b.x) / 2, y: (a.y + b.y) / 2)
            if !RoutingVerifier.inside(mid, problem.outline) { return false }
        }
        return true
    }

    // MARK: Anchors

    /// The points of trace `t` other copper of its net hangs on, inserted
    /// as vertices: other traces' ends on it (same layer), via centres on
    /// it. Returns the vertex indices that are anchors, ends included.
    mutating func anchors(of t: Int) -> [Int] {
        let trace = traces[t]
        var points: [CGPoint] = []
        for (o, other) in traces.enumerated() where o != t && !deadTraces.contains(o)
            && other.net == trace.net && other.layer == trace.layer {
            points.append(other.points.first!)
            points.append(other.points.last!)
        }
        for (v, via) in vias.enumerated() where !deadVias.contains(v) && via.net == trace.net { points.append(via.centre) }
        for obstacle in problem.obstacles where obstacle.net == trace.net {
            switch obstacle.kind {
            case .via:
                if case let .segment(a, _) = obstacle.shape.core { points.append(a) }
            case .trace:
                if obstacle.shape.layers?.contains(trace.layer) == true, case let .segment(a, b) = obstacle.shape.core {
                    points.append(a)
                    points.append(b)
                }
            default: break
            }
        }
        var vertices = trace.points
        var anchored = Set([0, vertices.count - 1])
        for point in points {
            if let k = vertices.firstIndex(where: { TraceRoute.same($0, point) || hypot($0.x - point.x, $0.y - point.y) < 1e-6 }) {
                anchored.insert(k)
                continue
            }
            for k in 0..<(vertices.count - 1)
            where SchematicDocument.distance(from: point, toSegment: vertices[k], vertices[k + 1]) < 1e-6 {
                vertices.insert(point, at: k + 1)
                anchored = Set(anchored.map { $0 > k ? $0 + 1 : $0 })
                anchored.insert(k + 1)
                break
            }
        }
        traces[t].points = vertices
        return anchored.sorted()
    }

    // MARK: Passes

    /// Every step of § 3 1–6, in order.
    mutating func run() {
        for t in traces.indices where !deadTraces.contains(t) { straighten(t) }
        if problem.options.angle == .octilinear {
            for t in traces.indices where !deadTraces.contains(t) { chamfer(t) }
        }
        removeViaHops()
        tidy()
    }

    static func length(_ points: [CGPoint]) -> Double {
        zip(points, points.dropFirst()).reduce(0) { $0 + hypot($1.1.x - $1.0.x, $1.1.y - $1.0.y) }
    }

    /// The octilinear (or orthogonal) routes of 1 or 2 segments from `a`
    /// to `b`: the straight line when it is 0°, 45° or 90° (0° or 90°), else
    /// a straight run and a 45° leg in both orders (an L both ways).
    func shortRoutes(_ a: CGPoint, _ b: CGPoint) -> [[CGPoint]] {
        let dx = b.x - a.x, dy = b.y - a.y
        let eps = 1e-9
        let straight = abs(dx) < eps || abs(dy) < eps
        if problem.options.angle == .orthogonal {
            if straight { return [[a, b]] }
            return [[a, CGPoint(x: b.x, y: a.y), b], [a, CGPoint(x: a.x, y: b.y), b]]
        }
        if straight || abs(abs(dx) - abs(dy)) < eps { return [[a, b]] }
        let sx: CGFloat = dx < 0 ? -1 : 1, sy: CGFloat = dy < 0 ? -1 : 1
        if abs(dx) > abs(dy) {
            let run = abs(dx) - abs(dy)
            return [[a, CGPoint(x: a.x + sx * run, y: a.y), b], [a, CGPoint(x: a.x + sx * abs(dy), y: b.y), b]]
        }
        let run = abs(dy) - abs(dx)
        return [[a, CGPoint(x: a.x, y: a.y + sy * run), b], [a, CGPoint(x: b.x, y: a.y + sy * abs(dx)), b]]
    }

    /// Straightens trace `t`'s chains: from each vertex, the farthest later
    /// vertex of the chain a legal short route reaches, no longer than the
    /// chain's way there.
    mutating func straighten(_ t: Int) {
        let anchored = anchors(of: t)
        let trace = traces[t]
        var points: [CGPoint] = [trace.points[0]]
        for (from, to) in zip(anchored, anchored.dropFirst()) {
            let chain = Array(trace.points[from...to])
            var i = 0
            while i < chain.count - 1 {
                var advanced = false
                var j = chain.count - 1
                while j > i + 1 {
                    let budget = Self.length(Array(chain[i...j])) + 1e-9
                    if let route = shortRoutes(chain[i], chain[j]).first(where: {
                        Self.length($0) <= budget && isLegal($0, layer: trace.layer, width: trace.width, net: trace.net)
                    }) {
                        points += route.dropFirst()
                        i = j
                        advanced = true
                        break
                    }
                    j -= 1
                }
                if !advanced {
                    points.append(chain[i + 1])
                    i += 1
                }
            }
        }
        traces[t].points = TraceRoute.clean(points)
        indexTrace(t)
    }

    /// Cuts each 90° corner of trace `t` that is no anchor into two 45°
    /// bends, by half the shorter side or 4 widths, whichever is less,
    /// where the cut is legal.
    mutating func chamfer(_ t: Int) {
        let anchored = Set(anchors(of: t))
        var points = traces[t].points
        let width = traces[t].width
        // From the end back, so an insertion never moves a corner still to come.
        for k in stride(from: points.count - 2, through: 1, by: -1) {
            guard !anchored.contains(k) else { continue }
            let a = points[k - 1], v = points[k], b = points[k + 1]
            let u1 = CGPoint(x: v.x - a.x, y: v.y - a.y), u2 = CGPoint(x: b.x - v.x, y: b.y - v.y)
            let l1 = hypot(u1.x, u1.y), l2 = hypot(u2.x, u2.y)
            guard l1 > 1e-9, l2 > 1e-9, abs(u1.x * u2.x + u1.y * u2.y) < 1e-9 * l1 * l2 else { continue }
            let cut = min(0.5 * min(l1, l2), 4 * width)
            guard cut > 1e-6 else { continue }
            let p = CGPoint(x: v.x - u1.x / l1 * cut, y: v.y - u1.y / l1 * cut)
            let q = CGPoint(x: v.x + u2.x / l2 * cut, y: v.y + u2.y / l2 * cut)
            guard isLegal([p, q], layer: traces[t].layer, width: width, net: traces[t].net) else { continue }
            points.replaceSubrange(k...k, with: [p, q])
        }
        traces[t].points = TraceRoute.clean(points)
        indexTrace(t)
    }

    /// The traces (live) with an end at `point`, of net `net`.
    private func ends(at point: CGPoint, net: Int?) -> [Int] {
        traces.indices.filter { t in
            !deadTraces.contains(t) && traces[t].net == net
                && (TraceRoute.same(traces[t].points.first!, point) || TraceRoute.same(traces[t].points.last!, point))
        }
    }

    /// Whether anything but the given traces touches the via at `v`: a
    /// pad, kept copper, or another trace through it.
    private func onlyTouchedBy(_ v: Int, _ allowed: Set<Int>) -> Bool {
        let via = vias[v]
        let r = via.diameter / 2
        for (t, trace) in traces.enumerated() where !deadTraces.contains(t) && !allowed.contains(t) {
            if zip(trace.points, trace.points.dropFirst()).contains(where: {
                SchematicDocument.distance(from: via.centre, toSegment: $0.0, $0.1) <= r + trace.width / 2
            }) { return false }
        }
        for obstacle in problem.obstacles where obstacle.net == via.net && [.pad, .trace, .via].contains(obstacle.kind) {
            if obstacle.shape.gap(to: Self.viaShape(via)) <= 0 { return false }
        }
        return true
    }

    /// A Top → Bottom → Top hop (or any layer pair) whose two vias carry
    /// only the hop: the middle run goes back on the outer layer when a
    /// legal short route there is no longer, and both vias go.
    mutating func removeViaHops() {
        for v in vias.indices where !deadVias.contains(v) {
            let first = vias[v]
            let atFirst = ends(at: first.centre, net: first.net)
            guard atFirst.count == 2 else { continue }
            let layers = atFirst.map { traces[$0].layer }
            guard layers[0] != layers[1] else { continue }
            for (outer, middle) in [(atFirst[0], atFirst[1]), (atFirst[1], atFirst[0])] {
                let run = traces[middle]
                let far = TraceRoute.same(run.points.first!, first.centre) ? run.points.last! : run.points.first!
                guard let w = vias.indices.first(where: { !deadVias.contains($0) && $0 != v && vias[$0].net == first.net
                    && TraceRoute.same(vias[$0].centre, far) }) else { continue }
                let atSecond = ends(at: far, net: first.net)
                guard atSecond.count == 2, atSecond.contains(middle),
                      let back = atSecond.first(where: { $0 != middle }), traces[back].layer == traces[outer].layer,
                      onlyTouchedBy(v, [outer, middle]), onlyTouchedBy(w, [middle, back]) else { continue }
                let layer = traces[outer].layer
                let budget = Self.length(run.points) + 1e-9
                guard let route = shortRoutes(first.centre, far).first(where: {
                    Self.length($0) <= budget && isLegal($0, layer: layer, width: run.width, net: run.net)
                }) else { continue }
                traces[middle].layer = layer
                traces[middle].points = route
                indexTrace(middle)
                deadVias.insert(v)
                deadVias.insert(w)
                index.remove(owner: -2 - v)
                index.remove(owner: -2 - w)
                break
            }
        }
    }

    /// Whether an end of trace `t` at `point` joins something: a pad on
    /// its layer, a via, or another trace on its layer (the DRC's rule).
    func endJoins(_ point: CGPoint, of t: Int) -> Bool {
        let trace = traces[t]
        let tolerance = RoutingVerifier.tolerance
        for obstacle in problem.obstacles {
            switch obstacle.kind {
            case .pad:
                if obstacle.shape.layers?.contains(trace.layer) ?? true,
                   obstacle.shape.bounds.insetBy(dx: -tolerance, dy: -tolerance).contains(point) { return true }
            case .via:
                if case let .segment(c, _) = obstacle.shape.core,
                   hypot(c.x - point.x, c.y - point.y) <= obstacle.shape.radius + trace.width / 2 { return true }
            case .trace:
                if obstacle.shape.layers?.contains(trace.layer) == true, case let .segment(a, b) = obstacle.shape.core,
                   SchematicDocument.distance(from: point, toSegment: a, b) < 1e-4 { return true }
            default: break
            }
        }
        for (v, via) in vias.enumerated() where !deadVias.contains(v)
            && hypot(via.centre.x - point.x, via.centre.y - point.y) <= via.diameter / 2 + trace.width / 2 { return true }
        for (o, other) in traces.enumerated() where o != t && !deadTraces.contains(o) && other.layer == trace.layer {
            for (a, b) in zip(other.points, other.points.dropFirst())
            where SchematicDocument.distance(from: point, toSegment: a, b) < 1e-4 { return true }
        }
        return false
    }

    /// Removes zero-length segments and vertices on a straight line, then
    /// stubs: traces with an end that joins nothing, until none is left.
    mutating func tidy() {
        for t in traces.indices where !deadTraces.contains(t) {
            traces[t].points = TraceRoute.simplified(traces[t].points)
            if traces[t].points.count < 2 { deadTraces.insert(t) }
            indexTrace(t)
        }
        var changed = true
        while changed {
            changed = false
            for t in traces.indices where !deadTraces.contains(t) {
                let trace = traces[t]
                guard !endJoins(trace.points.first!, of: t) || !endJoins(trace.points.last!, of: t) else { continue }
                deadTraces.insert(t)
                indexTrace(t)
                changed = true
            }
        }
    }
}
