import CoreGraphics
import Foundation

/// The autorouter's entry point (plan-38-E…J). Nonisolated and
/// deterministic: the same problem gives the same result. It reads only the
/// `RoutingProblem` and never the document.
nonisolated enum Router {
    /// Routes `problem`. `progress` is called after each connection;
    /// `isCancelled` is polled during the search and between connections.
    static func route(_ problem: RoutingProblem, progress: (RoutingProgress) -> Void = { _ in },
                      isCancelled: () -> Bool = { false }) -> RoutingResult {
        let start = Date()
        var run = Run(problem: problem)
        let order = problem.nets.filter { $0.impossible == nil }.sorted { a, b in
            let (sa, sb) = (span(a), span(b))
            return sa != sb ? sa < sb : a.id < b.id
        }
        var outcomes: [Int: NetOutcome] = [:]
        for net in order {
            if run.stopped || isCancelled() { run.stopped = true; break }
            outcomes[net.id] = run.route(net, progress: progress, isCancelled: isCancelled)
        }
        var result = run.result
        result.nets = problem.nets.sorted { $0.id < $1.id }.map { net in
            if let outcome = outcomes[net.id] { return outcome }
            let unrouted = net.components.dropFirst().map { "\(net.components[0].padLabels.first ?? "?") – \($0.padLabels.first ?? "?")" }
            if let reason = net.impossible {
                return NetOutcome(net: net.id, name: net.name, status: .impossible(reason),
                                  unroutedConnections: net.connections, unrouted: unrouted)
            }
            return NetOutcome(net: net.id, name: net.name, status: net.connections == 0 ? .complete : .failed,
                              unroutedConnections: net.connections, unrouted: unrouted)
        }
        result.stats.connections = problem.connections
        result.stats.routed = problem.connections - result.nets.reduce(0) { $0 + $1.unroutedConnections }
        result.stats.vias = result.vias.count
        result.stats.wireLength = result.traces.reduce(0) { sum, trace in
            sum + zip(trace.points, trace.points.dropFirst()).reduce(0) { $0 + hypot($1.1.x - $1.0.x, $1.1.y - $1.0.y) }
        }
        result.stats.expansions = run.search.expansions
        result.stats.stopped = run.stopped
        result.stats.seconds = Date().timeIntervalSince(start)
        return result
    }

    /// A net's bounding-box half-perimeter over its pads, mm: short nets
    /// go first.
    static func span(_ net: RoutingNet) -> Double {
        let boxes = net.components.flatMap { $0.pads.map(\.bounds) }
        guard let first = boxes.first else { return 0 }
        let box = boxes.dropFirst().reduce(first) { $0.union($1) }
        return box.width + box.height
    }

    /// One run's working state.
    struct Run {
        let problem: RoutingProblem
        var grid: RoutingGrid
        var search: RoutingSearch
        var result = RoutingResult()
        var stopped = false
        var routedSoFar = 0

        init(problem: RoutingProblem) {
            self.problem = problem
            grid = RoutingGrid(problem: problem)
            search = RoutingSearch(nodes: grid.cellCount * max(grid.layers.count, 1))
        }

        /// Routes one net: its components joined as a tree, or, for a net on
        /// a Plane, each surface-only component tied to the Plane by a via.
        mutating func route(_ net: RoutingNet, progress: (RoutingProgress) -> Void,
                            isCancelled: () -> Bool) -> NetOutcome {
            let rules = SearchRules(net: net, grid: grid, problem: problem)
            if net.planeLayer != nil, problem.options.fanoutPlanes, rules.vias {
                return fanOut(net, rules: rules, progress: progress, isCancelled: isCancelled)
            }
            let terminals = net.components.map {
                grid.terminals(of: $0, net: net.id, profile: rules.profile, allowed: rules.allowed)
            }
            var treeNodes: [Int] = terminals[0].map(\.node)
            var tie: [Int: CGPoint] = [:]
            for terminal in terminals[0] { if let point = terminal.tie { tie[terminal.node] = point } }
            var treePoints: [(point: CGPoint, label: String)] = Self.padPoints(net.components[0])
            var remaining = Array(net.components.indices.dropFirst())
            var unrouted: [String] = []
            while !remaining.isEmpty {
                // The component nearest the tree, by pad centres.
                var best = 0, bestDistance = Double.infinity
                for (r, index) in remaining.enumerated() {
                    for (point, _) in Self.padPoints(net.components[index]) {
                        for (q, _) in treePoints {
                            let d = hypot(point.x - q.x, point.y - q.y)
                            if d < bestDistance { bestDistance = d; best = r }
                        }
                    }
                }
                let index = remaining.remove(at: best)
                let component = net.components[index]
                let targets = terminals[index]
                let label = component.padLabels.first ?? "?"
                let near = Self.nearestLabel(to: Self.padPoints(component).first?.point ?? .zero, in: treePoints)
                if stopped || isCancelled() {
                    stopped = true
                    unrouted.append("\(near) – \(label)")
                    continue
                }
                var found: [Int]?
                if !targets.isEmpty, !treeNodes.isEmpty {
                    switch search.search(grid, rules: rules, sources: treeNodes, targets: targets.map(\.node),
                                         goal: grid.cellBox(targets.map(\.node)), isCancelled: isCancelled) {
                    case let .success(path): found = path
                    case .failure(.cancelled): stopped = true
                    case .failure: break
                    }
                }
                guard let path = found else {
                    unrouted.append("\(near) – \(label)")
                    continue
                }
                var targetTie: [Int: CGPoint] = [:]
                for terminal in targets { if let point = terminal.tie { targetTie[terminal.node] = point } }
                let copper = Router.copper(path, startTie: tie[path[0]], endTie: targetTie[path[path.count - 1]],
                                         net: net, grid: grid)
                add(copper, net: net)
                treeNodes += path
                for via in copper.vias {
                    let c = grid.nearestCell(via.centre)
                    treeNodes += grid.layers.indices.filter { rules.allowed[$0] }.map { $0 * grid.cellCount + c }
                }
                treeNodes += targets.map(\.node)
                for terminal in targets { if let point = terminal.tie, tie[terminal.node] == nil { tie[terminal.node] = point } }
                treePoints += Self.padPoints(component)
                routedSoFar += 1
                progress(RoutingProgress(routed: routedSoFar, total: problem.connections, round: 0, net: net.name))
            }
            return Self.outcome(net, unrouted: unrouted)
        }

        /// A Plane net: each component with no through-hole pad and no via
        /// gets a via beside (or in) one of its pads, joined to the pad by a
        /// trace from its centre, so the Plane joins it; the DRC counts a
        /// via touched by the pad and the trace as connected.
        mutating func fanOut(_ net: RoutingNet, rules: SearchRules, progress: (RoutingProgress) -> Void,
                             isCancelled: () -> Bool) -> NetOutcome {
            var onPlane = net.components.map { component in
                !component.vias.isEmpty || component.pads.contains { $0.layers == nil }
            }
            var unrouted: [String] = []
            for (index, component) in net.components.enumerated() where !onPlane[index] {
                let label = component.padLabels.first ?? "?"
                if stopped || isCancelled() { stopped = true; unrouted.append("\(label) – plane"); continue }
                let sources = grid.terminals(of: component, net: net.id, profile: rules.profile, allowed: rules.allowed)
                var tie: [Int: CGPoint] = [:]
                for terminal in sources { if let point = terminal.tie { tie[terminal.node] = point } }
                // Where a via of the net fits and its metal meets a pad.
                var targets: [Int] = []
                let radius = net.viaDiameter / 2
                for pad in component.pads {
                    let box = pad.bounds
                    let centre = CGPoint(x: box.midX, y: box.midY)
                    guard let range = grid.cellRange(box, margin: radius) else { continue }
                    let padLayers = grid.layerIndices(pad.layers).filter { rules.allowed[$0] }
                    for y in range.y {
                        for x in range.x {
                            let c = grid.cell(x, y)
                            let p = grid.point(c)
                            guard hypot(p.x - centre.x, p.y - centre.y) > 1e-6,
                                  CopperGeometry.pointDistance(p, box) <= radius,
                                  grid.viaFits(c, net: net.id, profile: rules.profile) else { continue }
                            targets += padLayers.map { $0 * grid.cellCount + c }
                        }
                    }
                }
                var found: [Int]?
                if !sources.isEmpty, !targets.isEmpty {
                    switch search.search(grid, rules: rules, sources: sources.map(\.node), targets: targets,
                                         goal: nil, isCancelled: isCancelled) {
                    case let .success(path): found = path
                    case .failure(.cancelled): stopped = true
                    case .failure: break
                    }
                }
                guard let path = found, let end = path.last else { unrouted.append("\(label) – plane"); continue }
                let via = RoutedVia(centre: grid.point(end % grid.cellCount), drill: net.viaDrill,
                                    diameter: net.viaDiameter, net: net.id)
                var copper = Router.copper(path, startTie: tie[path[0]], endTie: nil, net: net, grid: grid)
                if copper.traces.isEmpty, let start = tie[path[0]] {
                    copper.traces = [RoutedTrace(layer: grid.layers[path[0] / grid.cellCount], width: net.width,
                                                 points: [start, via.centre], net: net.id)]
                }
                copper.vias.append(via)
                add(copper, net: net)
                onPlane[index] = true
                routedSoFar += 1
                progress(RoutingProgress(routed: routedSoFar, total: problem.connections, round: 0, net: net.name))
            }
            _ = onPlane
            // Every component on the Plane is one group; the rest are one each.
            let groups = onPlane.filter { !$0 }.count + (onPlane.contains(true) ? 1 : 0)
            var outcome = Self.outcome(net, unrouted: unrouted)
            outcome.unroutedConnections = max(0, groups - 1)
            outcome.unrouted = Array(unrouted.prefix(outcome.unroutedConnections))
            outcome.status = Self.status(connections: net.connections, unrouted: outcome.unroutedConnections)
            return outcome
        }

        /// Adds new copper to the result and stamps it into the grid.
        mutating func add(_ copper: (traces: [RoutedTrace], vias: [RoutedVia]), net: RoutingNet) {
            for trace in copper.traces {
                guard let layer = grid.layerIndex(trace.layer) else { continue }
                grid.stampTrace(trace.points, width: trace.width, layer: layer, net: net.id)
            }
            for via in copper.vias { grid.stampVia(via.centre, diameter: via.diameter, net: net.id) }
            result.traces += copper.traces
            result.vias += copper.vias
        }

        static func outcome(_ net: RoutingNet, unrouted: [String]) -> NetOutcome {
            NetOutcome(net: net.id, name: net.name, status: status(connections: net.connections, unrouted: unrouted.count),
                       unroutedConnections: unrouted.count, unrouted: unrouted)
        }

        static func status(connections: Int, unrouted: Int) -> NetStatus {
            if unrouted == 0 { return .complete }
            return unrouted >= connections ? .failed : .partial(unrouted: unrouted)
        }

        static func padPoints(_ component: RoutingComponent) -> [(point: CGPoint, label: String)] {
            zip(component.pads, component.padLabels).map { (CGPoint(x: $0.bounds.midX, y: $0.bounds.midY), $1) }
        }

        static func nearestLabel(to point: CGPoint, in points: [(point: CGPoint, label: String)]) -> String {
            points.min { hypot($0.point.x - point.x, $0.point.y - point.y) < hypot($1.point.x - point.x, $1.point.y - point.y) }?
                .label ?? "?"
        }
    }

    /// A searched path as copper: a trace per run on one layer (collinear
    /// steps merged), a via at each layer change, and the ties at both
    /// ends (a pad's centre, a point on a kept trace, a via's centre).
    static func copper(_ path: [Int], startTie: CGPoint?, endTie: CGPoint?, net: RoutingNet,
                       grid: RoutingGrid) -> (traces: [RoutedTrace], vias: [RoutedVia]) {
        guard let first = path.first else { return ([], []) }
        let cells = grid.cellCount
        var runs: [(layer: Int, cells: [Int])] = [(first / cells, [first % cells])]
        var vias: [RoutedVia] = []
        for node in path.dropFirst() {
            let layer = node / cells, cell = node % cells
            if layer != runs[runs.count - 1].layer {
                vias.append(RoutedVia(centre: grid.point(cell), drill: net.viaDrill, diameter: net.viaDiameter, net: net.id))
                runs.append((layer, [cell]))
            } else {
                runs[runs.count - 1].cells.append(cell)
            }
        }
        var traces: [RoutedTrace] = []
        for (r, run) in runs.enumerated() {
            var points = merged(run.cells, grid: grid)
            if r == 0, let tie = startTie { points.insert(tie, at: 0) }
            if r == runs.count - 1, let tie = endTie { points.append(tie) }
            points = TraceRoute.clean(points)
            guard points.count >= 2 else { continue }
            traces.append(RoutedTrace(layer: grid.layers[run.layer], width: net.width, points: points, net: net.id))
        }
        return (traces, vias)
    }

    /// Cell centres with every vertex between two collinear steps dropped.
    static func merged(_ cells: [Int], grid: RoutingGrid) -> [CGPoint] {
        guard cells.count > 2 else { return cells.map(grid.point) }
        var kept = [cells[0]]
        for i in 1..<(cells.count - 1) {
            let a = cells[i - 1], b = cells[i], c = cells[i + 1]
            let d1 = (grid.ix(b) - grid.ix(a), grid.iy(b) - grid.iy(a))
            let d2 = (grid.ix(c) - grid.ix(b), grid.iy(c) - grid.iy(b))
            if d1 != d2 { kept.append(b) }
        }
        kept.append(cells[cells.count - 1])
        return kept.map(grid.point)
    }
}
