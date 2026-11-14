import CoreGraphics
import Foundation

/// The autorouter's entry point (plan-38-E…J). Nonisolated and
/// deterministic: the same problem gives the same result. It reads only the
/// `RoutingProblem` and never the document.
///
/// A run routes every net once (shortest first, wider first), then
/// negotiates (plan-38-F): a connection that finds no way is searched again
/// through other nets' routed copper at a price, the connections in its
/// way are ripped up and queued again, and each contested cell gets
/// dearer, round after round, until everything is routed, a round changes
/// nothing for 3 rounds, the strategy's rounds run out, the time limit
/// passes or the run is cancelled. Only legally stamped copper is ever in
/// the ledger, so the result at any stop is legal.
nonisolated enum Router {
    /// How hard a strategy tries.
    struct Strategy: Sendable {
        var rounds: Int
        var viaCost: Double
        var historyStep: Float
        /// How often one connection may be ripped up in a run (`Run.ripBudget`).
        var maxRips: Int
        /// Independent runs (Thorough), each with its own tie order.
        var runs: Int
        /// The first round's price per cell of crossing routed copper.
        var present: Double = 2
        var presentGrowth: Double = 1.5
        /// Rounds in a row with no change in the failed set before a run stops.
        var stall = 3

        static func of(_ strategy: RoutingOptions.Strategy) -> Strategy {
            switch strategy {
            case .fast: Strategy(rounds: 6, viaCost: 12, historyStep: 0.5, maxRips: 8, runs: 1)
            case .balanced: Strategy(rounds: 30, viaCost: 12, historyStep: 1, maxRips: 12, runs: 1)
            case .thorough: Strategy(rounds: 120, viaCost: 12, historyStep: 1, maxRips: 12, runs: 3)
            }
        }
    }

    /// Routes `problem`. `progress` is called after each connection;
    /// `isCancelled` is polled during the search and between connections.
    static func route(_ problem: RoutingProblem, progress: (RoutingProgress) -> Void = { _ in },
                      isCancelled: () -> Bool = { false }) -> RoutingResult {
        let start = Date()
        let strategy = Strategy.of(problem.options.strategy)
        let deadline = problem.options.timeLimit.map { start.addingTimeInterval($0) }
        let stop: () -> Bool = { isCancelled() || deadline.map { Date() > $0 } ?? false }
        var best: RoutingResult?
        for attempt in 0..<strategy.runs {
            var run = Run(problem: problem, strategy: strategy, seed: problem.options.seed &+ UInt64(attempt))
            run.routeAll(progress: progress, isCancelled: stop)
            let copper = run.finishedCopper()
            var result = run.result(traces: copper.traces, vias: copper.vias)
            result.stats.seconds = Date().timeIntervalSince(start)
            if best.map({ Self.better(result, than: $0) }) ?? true { best = result }
            if result.stats.routed == result.stats.connections || run.stopped { break }
        }
        return best ?? RoutingResult()
    }

    /// More routed wins, then less wire.
    static func better(_ a: RoutingResult, than b: RoutingResult) -> Bool {
        a.stats.routed != b.stats.routed ? a.stats.routed > b.stats.routed : a.stats.wireLength < b.stats.wireLength - 1e-9
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
        let strategy: Strategy
        let seed: UInt64
        var state: RoutingState
        var search: RoutingSearch
        var stopped = false
        var rounds = 0
        var rips = 0
        /// Rip-ups per net this run.
        var ripped: [Int: Int] = [:]

        /// How often net `net` may be ripped up in a run: `maxRips` for each
        /// of its connections, since the limit is per connection (a rail of
        /// 26 connections is ripped more often than a two-pin net).
        func ripBudget(_ net: Int) -> Int {
            strategy.maxRips * max(1, state.netsByID[net]?.connections ?? 1)
        }
        /// Nets on a Plane, fanned out once; their copper is never ripped
        /// (the Plane join it makes is not in the ledger's groups).
        var fannedOut: [Int: NetOutcome] = [:]
        var pinned = Set<Int>()
        /// Whether `finishedCopper` cleans the copper (plan-38-H); off, only
        /// verifies and repairs it.
        var cleaning = true
        /// Nets whose copper could not be made legal and was ripped up.
        var illegalNets = Set<Int>()
        var grid: RoutingGrid { state.grid }

        init(problem: RoutingProblem, strategy: Strategy = .of(.balanced), seed: UInt64 = 1) {
            self.problem = problem
            self.strategy = strategy
            self.seed = seed
            state = RoutingState(problem: problem)
            search = RoutingSearch(nodes: state.grid.cellCount * max(state.grid.layers.count, 1))
        }

        var routable: [RoutingNet] { problem.nets.filter { $0.impossible == nil } }

        func rules(_ net: RoutingNet) -> SearchRules {
            var rules = SearchRules(net: net, grid: grid, problem: problem)
            rules.viaCost = strategy.viaCost
            return rules
        }

        func fansOut(_ net: RoutingNet) -> Bool {
            net.planeLayer != nil && problem.options.fanoutPlanes && rules(net).vias
        }

        /// A tie-breaker for nets: the id, or (Thorough's later runs) the
        /// id mixed with the seed.
        func tieKey(_ net: Int) -> UInt64 {
            guard seed != problem.options.seed else { return UInt64(net) }
            var x = UInt64(net) &+ seed &* 0x9E37_79B9_7F4A_7C15
            x ^= x >> 31; x = x &* 0xBF58_476D_1CE4_E5B9; x ^= x >> 29
            return x
        }

        /// The queue order: nets that failed most last round, then wider,
        /// then shorter, then the tie key.
        func ordered(_ nets: [RoutingNet], failures: [Int: Int]) -> [RoutingNet] {
            nets.sorted { a, b in
                let fa = failures[a.id] ?? 0, fb = failures[b.id] ?? 0
                if fa != fb { return fa > fb }
                if a.width != b.width { return a.width > b.width }
                let (sa, sb) = (Router.span(a), Router.span(b))
                if sa != sb { return sa < sb }
                return tieKey(a.id) != tieKey(b.id) ? tieKey(a.id) < tieKey(b.id) : a.id < b.id
            }
        }

        var routedCount: Int {
            routable.reduce(0) { sum, net in sum + net.connections - unrouted(net) }
        }

        /// A net's connections still to make: its groups less one.
        func unrouted(_ net: RoutingNet) -> Int {
            if let outcome = fannedOut[net.id] { return outcome.unroutedConnections }
            return max(0, state.groups(of: net.id).components.count - 1)
        }

        mutating func routeAll(progress: (RoutingProgress) -> Void, isCancelled: () -> Bool) {
            for net in routable.sorted(by: { $0.id < $1.id }) where fansOut(net) {
                if isCancelled() { stopped = true; break }
                fannedOut[net.id] = fanOut(net, progress: progress, isCancelled: isCancelled)
            }
            var failures: [Int: Int] = [:]
            var lastFailed: [Int] = []
            var unchanged = 0
            var present = strategy.present
            for round in 0...strategy.rounds {
                if stopped || isCancelled() { stopped = true; break }
                rounds = round
                let queue = ordered(routable.filter { !fansOut($0) && unrouted($0) > 0 }, failures: failures)
                if queue.isEmpty { break }
                var pending = queue.map(\.id)
                var done = Set<Int>()
                // Round 0 is the single pass: hard searches only.
                let relaxed = round > 0
                while let id = pending.first {
                    pending.removeFirst()
                    guard !done.contains(id), let net = state.netsByID[id] else { continue }
                    done.insert(id)
                    let requeue = join(net, present: relaxed ? present : 0, round: round, progress: progress,
                                       isCancelled: isCancelled)
                    for victim in requeue where victim != id {
                        done.remove(victim)
                        if !pending.contains(victim) { pending.append(victim) }
                    }
                    if stopped { break }
                }
                failures = [:]
                for net in routable where !fansOut(net) {
                    let left = unrouted(net)
                    if left > 0 { failures[net.id] = left }
                }
                let failed = failures.keys.sorted()
                if failed.isEmpty { break }
                unchanged = failed == lastFailed ? unchanged + 1 : 0
                lastFailed = failed
                if round > 0, unchanged >= strategy.stall { break }
                if round > 0 { present *= strategy.presentGrowth }
            }
        }

        /// Joins `net`'s groups to the group of its first component, nearest
        /// first. A hard search first; failing that (when `present` > 0), a
        /// relaxed one, whose victims are ripped up before the connection is
        /// searched hard again. Returns the nets that lost copper.
        mutating func join(_ net: RoutingNet, present: Double, round: Int, progress: (RoutingProgress) -> Void,
                           isCancelled: () -> Bool) -> [Int] {
            var lost: [Int] = []
            var skipped = Set<Int>()
            var hard = rules(net)
            hard.present = 0
            var relaxed = hard
            relaxed.present = present
            while !stopped {
                let groups = state.groups(of: net.id)
                guard groups.components.count > 1 else { break }
                let main = (groups.components[0], groups.entries[0])
                // The nearest group not given up on this round.
                var best: Int?
                var bestDistance = Double.infinity
                let mainPoints = main.0.flatMap { Router.padPoints(net.components[$0]) }
                for g in 1..<groups.components.count where !skipped.contains(groups.components[g][0]) {
                    for c in groups.components[g] {
                        for (point, _) in Router.padPoints(net.components[c]) {
                            for (q, _) in mainPoints {
                                let d = hypot(point.x - q.x, point.y - q.y)
                                if d < bestDistance { bestDistance = d; best = g }
                            }
                        }
                    }
                }
                guard let other = best else { break }
                let target = (groups.components[other], groups.entries[other])
                let sources = state.nodes(components: main.0, entries: main.1, net: net.id)
                let targets = state.nodes(components: target.0, entries: target.1, net: net.id)
                if isCancelled() { stopped = true; break }
                var path = find(hard, sources: sources, targets: targets, isCancelled: isCancelled)
                if path == nil, present > 0, !stopped,
                   let detour = find(relaxed, sources: sources, targets: targets, isCancelled: isCancelled) {
                    let all = state.victims(of: detour, net: net.id)
                    let nets = Set(all.compactMap { state.entries[$0]?.net })
                    if !all.isEmpty, nets.isDisjoint(with: pinned),
                       nets.allSatisfy({ (ripped[$0] ?? 0) < ripBudget($0) }) {
                        for victimNet in nets.sorted() { ripped[victimNet, default: 0] += 1 }
                        rips += all.count
                        lost += state.rip(all)
                        state.addHistory(detour, step: strategy.historyStep)
                        path = find(hard, sources: sources, targets: targets, isCancelled: isCancelled)
                    }
                }
                guard let found = path else {
                    skipped.insert(target.0[0])
                    continue
                }
                let startTie = state.tie(found[0], net: net.id, components: main.0)
                let endTie = state.tie(found[found.count - 1], net: net.id, components: target.0)
                let copper = Router.copper(found, startTie: startTie, endTie: endTie, net: net, grid: grid)
                state.add(net: net.id, path: found, traces: copper.traces, vias: copper.vias, allowed: hard.allowed)
                report(progress, round: round, net: net.name)
            }
            return Array(Set(lost)).sorted()
        }

        /// When the last copper snapshot went out with a progress report.
        var lastSnapshot = Date.distantPast
        /// The most connections routed at once so far (`RoutingProgress.routed`).
        var bestRouted = 0
        static let snapshotInterval: TimeInterval = 0.5

        /// Reports progress, with the routed copper at most every
        /// `snapshotInterval`.
        mutating func report(_ progress: (RoutingProgress) -> Void, round: Int, net: String) {
            let current = routedCount
            bestRouted = max(bestRouted, current)
            var report = RoutingProgress(routed: bestRouted, total: problem.connections, round: round, net: net,
                                         current: current)
            let now = Date()
            if now.timeIntervalSince(lastSnapshot) >= Self.snapshotInterval {
                lastSnapshot = now
                let ledger = state.ledger
                report.traces = ledger.flatMap(\.traces)
                report.vias = ledger.flatMap(\.vias)
            }
            progress(report)
        }

        mutating func find(_ rules: SearchRules, sources: [Int], targets: [Int], isCancelled: () -> Bool) -> [Int]? {
            guard !sources.isEmpty, !targets.isEmpty else { return nil }
            switch search.search(grid, rules: rules, sources: sources, targets: targets, goal: grid.cellBox(targets),
                                 history: rules.present > 0 ? state.history : nil, isCancelled: isCancelled) {
            case let .success(path): return path
            case .failure(.cancelled): stopped = true; return nil
            case .failure: return nil
            }
        }

        /// A Plane net: each component with no through-hole pad and no via
        /// gets a via beside (or in) one of its pads, joined to the pad by a
        /// trace from its centre, so the Plane joins it; the DRC counts a
        /// via touched by the pad and the trace as connected.
        mutating func fanOut(_ net: RoutingNet, progress: (RoutingProgress) -> Void,
                             isCancelled: () -> Bool) -> NetOutcome {
            pinned.insert(net.id)
            let rules = rules(net)
            var onPlane = net.components.map { component in
                !component.vias.isEmpty || component.pads.contains { $0.layers == nil }
            }
            var unrouted: [String] = []
            for (index, component) in net.components.enumerated() where !onPlane[index] {
                let label = component.padLabels.first ?? "?"
                if stopped || isCancelled() { stopped = true; unrouted.append("\(label) – plane"); continue }
                let sources = state.terminals[net.id]?[index] ?? []
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
                let startTie = sources.first { $0.node == path[0] }?.tie
                var copper = Router.copper(path, startTie: startTie, endTie: nil, net: net, grid: grid)
                if copper.traces.isEmpty, let start = startTie {
                    copper.traces = [RoutedTrace(layer: grid.layers[path[0] / grid.cellCount], width: net.width,
                                                 points: [start, via.centre], net: net.id)]
                }
                copper.vias.append(via)
                state.add(net: net.id, path: path, traces: copper.traces, vias: copper.vias, allowed: rules.allowed)
                onPlane[index] = true
                report(progress, round: 0, net: net.name)
            }
            // Every component on the Plane is one group; the rest are one each.
            let groups = onPlane.filter { !$0 }.count + (onPlane.contains(true) ? 1 : 0)
            let left = max(0, groups - 1)
            return NetOutcome(net: net.id, name: net.name, status: Router.status(connections: net.connections, unrouted: left),
                              unroutedConnections: left, unrouted: Array(unrouted.prefix(left)))
        }

        /// The ledger's raw copper, in the order it was routed, and each
        /// net's outcome.
        var result: RoutingResult {
            var traces: [RoutedTrace] = [], vias: [RoutedVia] = []
            for entry in state.ledger {
                traces += entry.traces
                vias += entry.vias
            }
            return result(traces: traces, vias: vias)
        }

        /// A result of the given copper, with each net's outcome.
        func result(traces: [RoutedTrace], vias: [RoutedVia]) -> RoutingResult {
            var result = RoutingResult()
            result.traces = traces
            result.vias = vias
            result.nets = problem.nets.sorted { $0.id < $1.id }.map(outcome)
            result.stats.connections = problem.connections
            result.stats.routed = problem.connections - result.nets.reduce(0) { $0 + $1.unroutedConnections }
            result.stats.vias = result.vias.count
            result.stats.wireLength = result.traces.reduce(0) { sum, trace in
                sum + zip(trace.points, trace.points.dropFirst()).reduce(0) { $0 + hypot($1.1.x - $1.0.x, $1.1.y - $1.0.y) }
            }
            result.stats.expansions = search.expansions
            result.stats.stopped = stopped
            result.stats.rounds = rounds
            result.stats.rips = rips
            return result
        }

        func outcome(_ net: RoutingNet) -> NetOutcome {
            if let reason = net.impossible {
                let unrouted = net.components.dropFirst().map {
                    "\(net.components[0].padLabels.first ?? "?") – \($0.padLabels.first ?? "?")"
                }
                return NetOutcome(net: net.id, name: net.name, status: .impossible(reason),
                                  unroutedConnections: net.connections, unrouted: unrouted)
            }
            if let fanned = fannedOut[net.id] { return fanned }
            let groups = state.groups(of: net.id).components
            let main = groups.first.map { $0.flatMap { Router.padPoints(net.components[$0]) } } ?? []
            let unrouted = groups.dropFirst().map { group -> String in
                let theirs = group.flatMap { Router.padPoints(net.components[$0]) }
                let first = theirs.first ?? (.zero, "?")
                return "\(Router.nearestLabel(to: first.point, in: main)) – \(first.label)"
            }
            return NetOutcome(net: net.id, name: net.name,
                              status: Router.status(connections: net.connections, unrouted: unrouted.count),
                              unroutedConnections: unrouted.count, unrouted: unrouted,
                              reason: illegalNets.contains(net.id) && !unrouted.isEmpty ? Router.illegalReason : nil)
        }
    }

    static let illegalReason = "could not be made legal under the design rules"

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
