import CoreGraphics
import Foundation

/// The Trace tool's assist (plan-38-L § 3): built once when a trace press is
/// anchored, it finds the live leg from where the leg starts to the cursor
/// that walks around every other net's pads, traces and vias and the
/// board's edge, at the drawn net's class width and clearance, on the one
/// layer drawn on (no vias). The drawn net's own copper is free to cross
/// and to end on. A start on no net treats every piece of copper as
/// foreign.
///
/// A leg is an A* search on a 0.1 mm grid (plan-38-E's engine), capped at
/// `maxExpansions`, then straightened (plan-38-H) and checked by
/// `RoutingVerifier`; nil means no legal leg. The last answer is cached,
/// so the preview and the release ask once for the same cursor.
@MainActor
final class AssistRouter {
    static let pitch = 0.1
    static let maxExpansions = 200_000
    /// The grid id of a trace started on no net: higher than any real one.
    static let noNet = 1_000_000

    let layer: CopperLayer
    /// The drawn net, in the document's connectivity at the press; nil
    /// when the start is on no net's copper.
    let net: Int?
    /// The trace's width and clearance, mm (the net's class, else Default).
    let width: Double
    let clearance: Double
    let problem: RoutingProblem
    let grid: RoutingGrid
    private let rules: SearchRules
    private var search: RoutingSearch
    private var cached: (from: CGPoint, to: CGPoint, path: [CGPoint]?)?

    /// The net's id on the grid.
    var gridNet: Int { net ?? Self.noNet }

    /// Nothing to tear down on the main actor; an isolated deinit aborts
    /// the concurrency runtime (plan-38-K Corrections).
    nonisolated deinit {}

    init(document: SchematicDocument, layer: CopperLayer, start: CGPoint,
         rules designRules: DesignRules = DesignRules.current()) {
        self.layer = layer
        let setup = Self.problem(document, layer: layer, start: start, rules: designRules)
        let problem = setup.problem
        net = setup.net
        width = problem.nets[0].width
        clearance = problem.nets[0].clearance
        self.problem = problem
        let grid = RoutingGrid(problem: problem)
        self.grid = grid
        var rules = SearchRules(net: problem.nets[0], grid: grid, problem: problem)
        rules.vias = false
        rules.maxExpansions = Self.maxExpansions
        self.rules = rules
        search = RoutingSearch(nodes: grid.cellCount * max(grid.layers.count, 1))
    }

    /// The problem new copper from `start` on `layer` is checked in: every
    /// pad, trace and via an obstacle tagged with its net, the board's
    /// edge, and one net, the drawn one (the net of the copper at `start`,
    /// else `noNet`), with its class's width, clearance and via, on `layer`.
    static func problem(_ document: SchematicDocument, layer: CopperLayer, start: CGPoint,
                        rules: DesignRules) -> (problem: RoutingProblem, net: Int?) {
        let copper = document.connectivity()
        let net = document.net(at: start, layer: layer, in: copper)
        let netClass = net.flatMap { document.netClass(ofNet: $0, in: copper) }
        let standard = NetRules.defaults(for: rules)
        var options = RoutingOptions.standard
        options.grid = pitch
        options.vias = .none
        // No net is routed: every piece of copper is an obstacle with its net.
        options.scope = .nets([])
        var problem = document.routingProblem(options: options, rules: rules)
        problem.routable = [layer]
        problem.nets = [RoutingNet(
            id: net ?? noNet, name: net.flatMap { copper.netNames[$0] } ?? "",
            key: net.flatMap { copper.planeNet(ofNet: $0) } ?? .pin(chip: UUID(), designator: ""),
            width: max(Double(netClass?.width ?? standard.width) * 0.0254, rules.minTraceWidth),
            clearance: max(netClass?.clearance ?? rules.minClearance, rules.minClearance),
            viaDrill: netClass?.viaDrill ?? standard.viaDrill,
            viaDiameter: netClass?.viaDiameter ?? standard.viaDiameter,
            allowed: [layer], preferred: [], components: [])]
        return (problem, net)
    }

    /// Why a via of the drawn net (the net at `start` on `layer`, its
    /// class's size) can't stand at `centre`, or nil when it can: it must
    /// keep clear of every other net's copper, the edge and every hole.
    static func viaProblem(_ document: SchematicDocument, at centre: CGPoint, start: CGPoint, layer: CopperLayer,
                           rules: DesignRules = DesignRules.current()) -> String? {
        let (problem, _) = problem(document, layer: layer, start: start, rules: rules)
        let net = problem.nets[0]
        let via = RoutedVia(centre: centre, drill: net.viaDrill, diameter: net.viaDiameter, net: net.id)
        let broken = RoutingVerifier.verify(problem, traces: [], vias: [via]).filter { $0.kind != .unconnected }
        return broken.isEmpty ? nil : viaMessage
    }

    static let viaMessage = "No room for a via there."

    /// The legal leg from `from` to `to`, both included, or nil when there
    /// is none (an end on another net's copper, a sealed spot, or more than
    /// `maxExpansions` nodes away).
    func route(from: CGPoint, to: CGPoint) -> [CGPoint]? {
        if let cached, TraceRoute.same(cached.from, from), TraceRoute.same(cached.to, to) { return cached.path }
        let path = find(from: from, to: to)
        cached = (from, to, path)
        return path
    }

    private func find(from: CGPoint, to: CGPoint) -> [CGPoint]? {
        guard let l = grid.layerIndex(layer) else { return nil }
        let a = grid.nearestCell(from), b = grid.nearestCell(to)
        let profile = grid.profile(of: gridNet)
        guard grid.isPassable(a, layer: l, net: gridNet, profile: profile),
              grid.isPassable(b, layer: l, net: gridNet, profile: profile) else { return nil }
        var cells = [a]
        if a != b {
            let base = l * grid.cellCount
            guard case let .success(path) = search.search(grid, rules: rules, sources: [base + a], targets: [base + b],
                                                          goal: grid.cellBox([base + b]), isCancelled: { false }) else {
                return nil
            }
            cells = path.map { $0 % grid.cellCount }
        }
        var points = TraceRoute.clean([from] + Router.merged(cells, grid: grid) + [to])
        guard points.count >= 2 else { return [from, to] }
        var cleanup = RoutingCleanup(problem: problem,
                                     traces: [RoutedTrace(layer: layer, width: width, points: points, net: gridNet)],
                                     vias: [], traceEntry: [0], viaEntry: [])
        cleanup.straighten(0)
        points = cleanup.traces[0].points
        let trace = RoutedTrace(layer: layer, width: width, points: points, net: gridNet)
        // A leg may end in open board: only the rules, not "joins something".
        let broken = RoutingVerifier.verify(problem, traces: [trace], vias: []).filter { $0.kind != .unconnected }
        return broken.isEmpty ? points : nil
    }
}
