import CoreGraphics
import Foundation

/// A coupled differential pair's copper, or why the pair couldn't be coupled.
nonisolated struct PairRoute: Sendable {
    var p: [RoutedTrace]
    var n: [RoutedTrace]
    var vias: [RoutedVia]
    /// mm of the pair that runs apart (`PairGeometry.uncoupledLength`).
    var uncoupled: Double
}

/// Routing a differential pair as one fat centreline (plan-38-J § 3): the
/// centreline is searched on a grid where both nets' copper is the pair's,
/// with a profile as wide as both traces and their gap, from near one end's
/// midpoint to near the other's; then cleaned, offset to the two traces at
/// ±(width + gap)/2, given a staggered via pair at each layer change and a
/// breakout leg to each pad, and checked whole against the design rules.
nonisolated enum RoutingPair {
    static func route(_ group: RoutingGroup, problem: RoutingProblem,
                      isCancelled: () -> Bool = { false }) -> Result<PairRoute, PairFailure> {
        guard group.nets.count == 2,
              let pNet = problem.nets.first(where: { $0.id == group.nets[0] }),
              let nNet = problem.nets.first(where: { $0.id == group.nets[1] }) else { return .failure(.notTwoNets) }
        guard pNet.pins == 2, nNet.pins == 2, pNet.components.count == 2, nNet.components.count == 2,
              pNet.impossible == nil, nNet.impossible == nil else { return .failure(.notTwoPin) }
        let allowed = pNet.allowed.intersection(nNet.allowed)
        guard !allowed.isEmpty else { return .failure(.noSharedLayer) }
        func centre(_ c: RoutingComponent) -> CGPoint? {
            c.pads.first.map { CGPoint(x: $0.bounds.midX, y: $0.bounds.midY) }
        }
        guard let p1 = centre(pNet.components[0]), let p2 = centre(pNet.components[1]),
              let n1 = centre(nNet.components[0]), let n2 = centre(nNet.components[1]) else { return .failure(.notTwoPin) }
        let ends = PairGeometry.ends(p: (p1, p2), n: (n1, n2))
        let pStart = ends.0, nStart = ends.1, pEnd = ends.2, nEnd = ends.3
        let m1 = CGPoint(x: (pStart.x + nStart.x) / 2, y: (pStart.y + nStart.y) / 2)
        let m2 = CGPoint(x: (pEnd.x + nEnd.x) / 2, y: (pEnd.y + nEnd.y) / 2)

        let width = pNet.width
        let gap = max(group.gap, problem.rules.minClearance)
        let spacing = width + gap
        let clearance = max(pNet.clearance, nNet.clearance)
        let diameter = max(pNet.viaDiameter, nNet.viaDiameter)
        let stagger = PairGeometry.stagger(diameter: diameter, clearance: clearance, spacing: spacing)

        // The pair's grid: both nets' copper is P's; the fat net routes.
        var pairProblem = problem
        var fat = pNet
        fat.width = 2 * width + gap
        fat.clearance = clearance
        fat.viaDiameter = spacing + diameter + stagger
        fat.viaDrill = max(pNet.viaDrill, nNet.viaDrill)
        fat.allowed = allowed
        fat.preferred = pNet.preferred.intersection(allowed)
        pairProblem.nets = problem.nets.filter { $0.id != nNet.id }.map { $0.id == pNet.id ? fat : $0 }
        pairProblem.obstacles = problem.obstacles.map { obstacle in
            var obstacle = obstacle
            if obstacle.net == nNet.id { obstacle.net = pNet.id }
            return obstacle
        }
        let grid = RoutingGrid(problem: pairProblem)
        let rules = SearchRules(net: fat, grid: grid, problem: pairProblem)
        let reach = max(group.maxUncoupled / 2, grid.pitch)
        func cells(near m: CGPoint) -> [Int: Float] {
            var found: [Int: Float] = [:]
            guard let range = grid.cellRange(CGRect(x: m.x, y: m.y, width: 0, height: 0), margin: reach) else { return found }
            for y in range.y {
                for x in range.x {
                    let c = grid.cell(x, y)
                    let point = grid.point(c)
                    let d = hypot(point.x - m.x, point.y - m.y)
                    guard d <= reach else { continue }
                    for layer in grid.layers.indices where rules.allowed[layer]
                        && grid.isPassable(c, layer: layer, net: fat.id, profile: rules.profile) {
                        found[layer * grid.cellCount + c] = Float(d / grid.pitch)
                    }
                }
            }
            return found
        }
        let sources = cells(near: m1), targets = cells(near: m2)
        guard !sources.isEmpty, !targets.isEmpty else { return .failure(.noRoom) }
        var search = RoutingSearch(nodes: grid.cellCount * grid.layers.count)
        let targetNodes = targets.keys.sorted()
        guard case let .success(path) = search.search(grid, rules: rules, sources: sources.keys.sorted(),
                                                       targets: targetNodes, goal: grid.cellBox(targetNodes),
                                                       startCosts: sources, isCancelled: isCancelled) else {
            return .failure(.noRoute)
        }

        // The centreline, run by run, each cleaned on the fat profile.
        let cellsPerLayer = grid.cellCount
        var runs: [(layer: Int, cells: [Int])] = []
        for node in path {
            let layer = node / cellsPerLayer, cell = node % cellsPerLayer
            if runs.last?.layer == layer { runs[runs.count - 1].cells.append(cell) } else { runs.append((layer, [cell])) }
        }
        var centrelines: [(layer: CopperLayer, points: [CGPoint])] = []
        for run in runs {
            let points = Router.merged(run.cells, grid: grid)
            guard points.count >= 2 else { return .failure(.noRoute) }
            var cleanup = RoutingCleanup(problem: pairProblem,
                                         traces: [RoutedTrace(layer: grid.layers[run.layer], width: fat.width,
                                                              points: points, net: fat.id)],
                                         vias: [], traceEntry: [0], viaEntry: [])
            cleanup.straighten(0)
            if problem.options.angle == .octilinear { cleanup.chamfer(0) }
            centrelines.append((grid.layers[run.layer], cleanup.traces[0].points))
        }

        // Which side P runs on: the side of the first end's P pad.
        let first = centrelines[0].points
        let dx = first[1].x - first[0].x, dy = first[1].y - first[0].y
        let side: Double = (pStart.x - first[0].x) * -dy + (pStart.y - first[0].y) * dx >= 0 ? 1 : -1
        var pRuns = centrelines.map { (layer: $0.layer, points: PairGeometry.offset($0.points, by: side * spacing / 2)) }
        var nRuns = centrelines.map { (layer: $0.layer, points: PairGeometry.offset($0.points, by: -side * spacing / 2)) }
        var vias: [RoutedVia] = []
        for k in 0..<(centrelines.count - 1) {
            let line = centrelines[k].points
            let a = line[line.count - 2], b = line[line.count - 1]
            let l = hypot(b.x - a.x, b.y - a.y)
            let along = CGPoint(x: (b.x - a.x) / l, y: (b.y - a.y) / l)
            let pair = PairGeometry.vias(at: b, along: along, spacing: spacing, stagger: stagger)
            // `vias(at:)` puts its first via on the left.
            let (vp, vn) = side > 0 ? (pair.p, pair.n) : (pair.n, pair.p)
            pRuns[k].points[pRuns[k].points.count - 1] = vp
            pRuns[k + 1].points[0] = vp
            nRuns[k].points[nRuns[k].points.count - 1] = vn
            nRuns[k + 1].points[0] = vn
            vias.append(RoutedVia(centre: vp, drill: pNet.viaDrill, diameter: pNet.viaDiameter, net: pNet.id))
            vias.append(RoutedVia(centre: vn, drill: nNet.viaDrill, diameter: nNet.viaDiameter, net: nNet.id))
        }
        func traces(_ runs: [(layer: CopperLayer, points: [CGPoint])], net: Int) -> [RoutedTrace] {
            runs.map { RoutedTrace(layer: $0.layer, width: width, points: TraceRoute.clean($0.points), net: net) }
        }
        var pTraces = traces(pRuns, net: pNet.id), nTraces = traces(nRuns, net: nNet.id)

        // Breakouts: a short octilinear leg from each trace end to its pad.
        let check = RoutingCleanup(problem: problem, traces: pTraces + nTraces, vias: vias,
                                   traceEntry: Array(0..<(pTraces.count + nTraces.count)), viaEntry: Array(vias.indices))
        func leg(from point: CGPoint, to pad: CGPoint, layer: CopperLayer, net: Int) -> [CGPoint]? {
            check.shortRoutes(pad, point).first { check.isLegal($0, layer: layer, width: width, net: net) }
        }
        guard let pIn = leg(from: pTraces[0].points[0], to: pStart, layer: pTraces[0].layer, net: pNet.id),
              let nIn = leg(from: nTraces[0].points[0], to: nStart, layer: nTraces[0].layer, net: nNet.id),
              let pOut = leg(from: pTraces[pTraces.count - 1].points.last!, to: pEnd, layer: pTraces[pTraces.count - 1].layer,
                             net: pNet.id),
              let nOut = leg(from: nTraces[nTraces.count - 1].points.last!, to: nEnd, layer: nTraces[nTraces.count - 1].layer,
                             net: nNet.id) else { return .failure(.breakout) }
        pTraces[0].points = TraceRoute.clean(pIn + pTraces[0].points.dropFirst())
        nTraces[0].points = TraceRoute.clean(nIn + nTraces[0].points.dropFirst())
        pTraces[pTraces.count - 1].points = TraceRoute.clean(Array(pTraces[pTraces.count - 1].points.dropLast()) + Array(pOut.reversed()))
        nTraces[nTraces.count - 1].points = TraceRoute.clean(Array(nTraces[nTraces.count - 1].points.dropLast()) + Array(nOut.reversed()))

        guard RoutingVerifier.verify(problem, traces: pTraces + nTraces, vias: vias).isEmpty else {
            return .failure(.illegal)
        }
        let uncoupled = PairGeometry.uncoupledLength(p: pTraces, n: nTraces, gap: gap)
        guard uncoupled <= group.maxUncoupled + 1e-4 else { return .failure(.tooFarApart(uncoupled)) }
        return .success(PairRoute(p: pTraces, n: nTraces, vias: vias, uncoupled: uncoupled))
    }
}

/// Why a pair wasn't coupled.
nonisolated enum PairFailure: Error, Hashable, Sendable {
    case notTwoNets, notTwoPin, noSharedLayer, noRoom, noRoute, breakout, illegal
    case tooFarApart(Double)

    var reason: String {
        switch self {
        case .notTwoNets: "a pair needs exactly 2 routed nets"
        case .notTwoPin: "both nets must be two-pin nets"
        case .noSharedLayer: "the two nets' layer rules share no layer"
        case .noRoom: "no room for the pair at an end"
        case .noRoute: "no coupled route found"
        case .breakout: "no legal breakout to a pad"
        case .illegal: "the coupled route breaks a design rule"
        case let .tooFarApart(mm): String(format: "%.2f mm would run apart", mm)
        }
    }
}
