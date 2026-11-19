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
/// A run handed to `DispatchQueue.concurrentPerform`: the closure guards
/// what it shares with a lock, which the compiler can't see.
nonisolated private struct UncheckedWork: @unchecked Sendable {
    let run: (Int) -> Void
}

nonisolated enum Router {
    /// How hard a strategy tries.
    struct Strategy: Sendable {
        var rounds: Int
        var viaCost: Double
        var historyStep: Float
        /// How often one connection may be ripped up in a run (`Run.ripBudget`).
        var maxRips: Int
        /// The via costs of the independent runs `route` makes, the best kept; empty is one run at `viaCost`.
        var viaCosts: [Double] = []
        /// The first round's price per cell of crossing routed copper.
        var present: Double = 2
        var presentGrowth: Double = 1.5
        /// Rounds in a row with no change in the failed set before a run stops.
        var stall = 3
        /// Planning passes before any copper is put down (`RoutingPlan`); 0 plans nothing.
        var planPasses = 0
        /// The first pass's price, in cells, of a trace centre where another net's plan wants to be.
        var planPrice = 6.0
        var planGrowth = 1.3
        /// The price, in cells, never grows past this.
        var planPriceMax = 30.0
        /// What a contested node's history grows by, per other net, each pass.
        var planHistory: Float = 1
        /// Passes in a row without fewer contested nodes before planning stops.
        var planStall = 99
        /// The price, in cells, the detailed routing pays to use a place another net's plan still wants.
        var commitPrice = 20.0
        /// Improvement passes over the routed connections (`RoutingPolish`); 0 makes none.
        var polishPasses = 0
        /// What a via and a bend cost, in cells, while improving: both in the searches and in the score.
        var polishVia = 80.0
        var polishBend = 1.0
        /// The price per cell, while improving, of a way through other nets' copper (they are then routed again); 0 never tries one.
        var polishPresent = 3.0

        static func of(_ strategy: RoutingOptions.Strategy) -> Strategy {
            switch strategy {
            case .fast:
                return Strategy(rounds: 6, viaCost: 12, historyStep: 0.5, maxRips: 8)
            case .balanced:
                var balanced = Strategy(rounds: 30, viaCost: 12, historyStep: 1, maxRips: 12)
                balanced.viaCosts = [12, 25, 40, 60]
                balanced.planPasses = 14
                balanced.polishPasses = 20
                return balanced
            case .thorough:
                // Balanced's runs exactly, and as many again at other via
                // costs: the best of a superset, so never worse than Balanced.
                var thorough = of(.balanced)
                thorough.viaCosts += [18, 32, 50, 80]
                return thorough
            }
        }
    }

    /// Routes `problem`. `progress` is called after each connection;
    /// `isCancelled` is polled during the search and between connections.
    ///
    /// A strategy with several first-routing via costs (`Strategy.viaCosts`)
    /// routes the board once with each, at the same time on separate
    /// threads, and keeps the best result (`better`): how the first routing
    /// prices a via decides which connections change layers, and no one
    /// price is best on every board. Each run is deterministic and the
    /// choice depends only on the results, so the outcome is too. Progress
    /// is reported from the first run still going. `isCancelled` must be
    /// safe to call from any thread.
    static func route(_ problem: RoutingProblem, progress: (RoutingProgress) -> Void = { _ in },
                      isCancelled: () -> Bool = { false }) -> RoutingResult {
        let start = Date()
        let strategy = Strategy.of(problem.options.strategy)
        let deadline = problem.options.timeLimit.map { start.addingTimeInterval($0) }
        let costs = strategy.viaCosts.isEmpty ? [strategy.viaCost] : strategy.viaCosts
        let lock = NSLock()
        var results = [RoutingResult?](repeating: nil, count: costs.count)
        var finished = [Bool](repeating: false, count: costs.count)
        return withoutActuallyEscaping(progress) { progress in
            withoutActuallyEscaping(isCancelled) { isCancelled in
                let stop: () -> Bool = { isCancelled() || deadline.map { Date() > $0 } ?? false }
                let one: (Int) -> Void = { index in
                    var own = strategy
                    own.viaCost = costs[index]
                    var run = Run(problem: problem, strategy: own, seed: problem.options.seed)
                    run.routeAll(progress: { report in
                        lock.lock()
                        defer { lock.unlock() }
                        if finished.firstIndex(of: false) == index { progress(report) }
                    }, isCancelled: stop)
                    let copper = run.finishedCopper()
                    var result = run.result(traces: copper.traces, vias: copper.vias)
                    if !problem.groups.isEmpty {
                        result = Self.tuned(result, problem: problem, pairs: run.pairStatus)
                    }
                    result.stats.bends = Self.bends(result.traces)
                    result.stats.seconds = Date().timeIntervalSince(start)
                    lock.lock()
                    results[index] = result
                    finished[index] = true
                    lock.unlock()
                }
                if costs.count == 1 {
                    one(0)
                } else {
                    // Each run writes only its own slot, under `lock`.
                    let work = UncheckedWork(run: one)
                    DispatchQueue.concurrentPerform(iterations: costs.count) { work.run($0) }
                }
                var best: RoutingResult?
                for case let result? in results where best.map({ Self.better(result, than: $0, strategy: strategy, pitch: problem.options.grid) }) ?? true {
                    best = result
                }
                best?.stats.seconds = Date().timeIntervalSince(start)
                return best ?? RoutingResult()
            }
        }
    }

    /// `result` with its length-match groups tuned (plan-38-I): any tuned
    /// trace the verifier then faults goes back as it was, so tuning never
    /// makes a result illegal.
    static func tuned(_ result: RoutingResult, problem: RoutingProblem,
                      pairs: [UUID: GroupOutcome.Status] = [:]) -> RoutingResult {
        var tuned = result
        var traces = result.traces
        let tune = problem.options.tuneGroups
        tuned.groups = RoutingTuning.tune(problem, traces: &traces, vias: result.vias, outcomes: result.nets, pairs: pairs,
                                          tuneLengths: tune, tuneSkew: tune)
        let faulted = Set(RoutingVerifier.verify(problem, traces: traces, vias: result.vias).flatMap(\.traces))
        for t in faulted where t < result.traces.count { traces[t] = result.traces[t] }
        tuned.traces = traces
        tuned.stats.wireLength = RoutingLength.of(traces)
        if !faulted.isEmpty {
            // Measure the groups again on what was kept.
            tuned.groups = tuned.groups.map { outcome in
                var outcome = outcome
                for net in outcome.lengths.keys {
                    outcome.lengths[net] = RoutingTuning.length(of: net, traces: traces, problem: problem)
                }
                let values = outcome.lengths.values
                outcome.spread = (values.max() ?? 0) - (values.min() ?? 0)
                return outcome
            }
        }
        return tuned
    }

    /// More routed wins, then the lower score: wire, with every via and
    /// bend counted as the wire the improvement passes price them at.
    static func better(_ a: RoutingResult, than b: RoutingResult, strategy: Strategy, pitch: Double) -> Bool {
        func score(_ r: RoutingResult) -> Double {
            r.stats.wireLength + pitch * (strategy.polishVia * Double(r.stats.vias) + strategy.polishBend * Double(r.stats.bends))
        }
        return a.stats.routed != b.stats.routed ? a.stats.routed > b.stats.routed : score(a) < score(b) - 1e-9
    }

    /// The corners in `traces`: vertices where a trace changes direction.
    static func bends(_ traces: [RoutedTrace]) -> Int {
        var count = 0
        for trace in traces {
            let p = trace.points
            guard p.count >= 3 else { continue }
            for i in 1..<(p.count - 1) {
                let cross = (p[i].x - p[i - 1].x) * (p[i + 1].y - p[i].y) - (p[i].y - p[i - 1].y) * (p[i + 1].x - p[i].x)
                if abs(cross) > 1e-9 { count += 1 }
            }
        }
        return count
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
        /// The nets routed as coupled pairs (plan-38-J), and each pair's outcome.
        var pairNets = Set<Int>()
        var pairStatus: [UUID: GroupOutcome.Status] = [:]
        /// The plan the detailed routing is priced by, once made.
        var plan: RoutingPlan?
        /// Coupled pairs ripped up this round, routed again at the next.
        var rippedPairs = Set<UUID>()
        /// Whether the run is improving what it routed (`polish`): searches then use the improvement costs.
        var polishing = false
        /// Improvement moves tried and kept.
        var polishTried = 0
        var polishKept = 0
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
            rules.viaCost = polishing ? strategy.polishVia : strategy.viaCost
            if polishing { rules.bendCost = strategy.polishBend }
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
            if problem.options.routePairs { routePairs(isCancelled: isCancelled) }
            for net in routable.sorted(by: { $0.id < $1.id }) where fansOut(net) {
                if isCancelled() { stopped = true; break }
                fannedOut[net.id] = fanOut(net, progress: progress, isCancelled: isCancelled)
            }
            planAll(progress: progress, isCancelled: isCancelled)
            var failures: [Int: Int] = [:]
            var lastFailed: [Int] = []
            var unchanged = 0
            var present = strategy.present
            for round in 0...strategy.rounds {
                if stopped || isCancelled() { stopped = true; break }
                rounds = round
                // Pairs ripped up last round go first.
                if !rippedPairs.isEmpty {
                    let again = rippedPairs
                    rippedPairs = []
                    routePairs(only: again, isCancelled: isCancelled)
                }
                // A coupled pair's nets are routed only as their pair.
                let queue = ordered(routable.filter { !fansOut($0) && !pairNets.contains($0.id) && unrouted($0) > 0 },
                                    failures: failures)
                if queue.isEmpty && rippedPairs.isEmpty { break }
                var pending = queue.map(\.id)
                var done = Set<Int>()
                // Round 0 is the single pass: hard searches only.
                let relaxed = round > 0
                while let id = pending.first {
                    pending.removeFirst()
                    guard !done.contains(id), !pairNets.contains(id), let net = state.netsByID[id] else { continue }
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
            // A pair still ripped when the rounds end: once more, coupled, and
            // failing that as two ordinary nets.
            if !rippedPairs.isEmpty, !stopped {
                let again = rippedPairs
                rippedPairs = []
                routePairs(only: again, isCancelled: isCancelled)
                for group in problem.groups where again.contains(group.id) {
                    for net in group.nets where !pairNets.contains(net) {
                        if let routing = state.netsByID[net] {
                            _ = join(routing, present: 0, round: rounds, progress: progress, isCancelled: isCancelled)
                        }
                    }
                }
            }
            polish(progress: progress, isCancelled: isCancelled)
        }

        /// Routes the differential pairs `only` names (all, when nil), each as
        /// one coupled unit (plan-38-J), clear of the copper the ledger holds:
        /// its copper goes in the ledger as one entry per net, each joining
        /// that net's two pads, and is never cleaned. A pair ripped up as a
        /// victim comes back here, first, at the next round. A pair that can't
        /// be coupled is routed as two ordinary nets from then on.
        mutating func routePairs(only: Set<UUID>? = nil, isCancelled: () -> Bool) {
            for group in problem.groups.sorted(by: { $0.id.uuidString < $1.id.uuidString })
            where group.mode == .diffPair && only.map({ $0.contains(group.id) }) ?? true {
                if isCancelled() { stopped = true; return }
                let mine = Set(group.nets)
                let extra = ledgerObstacles(excluding: mine)
                switch RoutingPair.route(group, problem: problem, extra: extra, isCancelled: isCancelled) {
                case let .success(pair):
                    var added = true
                    for (net, traces) in [(group.nets[0], pair.p), (group.nets[1], pair.n)] {
                        guard let terminals = state.terminals[net], terminals.count == 2,
                              let a = terminals[0].first, let b = terminals[1].first else { added = false; continue }
                        let allowed = grid.layers.map { _ in true }
                        state.add(net: net, path: [a.node, b.node], traces: traces,
                                  vias: pair.vias.filter { $0.net == net }, allowed: allowed)
                    }
                    if added {
                        pairNets.formUnion(mine)
                        pairStatus[group.id] = .coupled(uncoupled: pair.uncoupled)
                    } else {
                        uncouple(group, PairFailure.notTwoPin.reason)
                    }
                case let .failure(failure):
                    uncouple(group, failure.reason)
                }
            }
        }

        /// From now on the pair's nets route as ordinary nets.
        mutating func uncouple(_ group: RoutingGroup, _ reason: String) {
            state.rip(state.ledger.filter { group.nets.contains($0.net) }.map(\.id))
            pairNets.subtract(group.nets)
            pairStatus[group.id] = .uncoupled(reason: reason)
        }

        /// The coupled pair `net` belongs to, if any.
        func coupledPair(of net: Int) -> RoutingGroup? {
            guard pairNets.contains(net) else { return nil }
            return problem.groups.first { $0.mode == .diffPair && $0.nets.contains(net) }
        }

        /// The ledger's copper, but for `nets`, as obstacles.
        func ledgerObstacles(excluding nets: Set<Int>) -> [RoutingObstacle] {
            var obstacles: [RoutingObstacle] = []
            for entry in state.ledger where !nets.contains(entry.net) {
                for trace in entry.traces {
                    for (a, b) in zip(trace.points, trace.points.dropFirst()) {
                        obstacles.append(RoutingObstacle(shape: CopperShape(core: .segment(a, b), radius: trace.width / 2,
                                                                            layers: [trace.layer], node: 20_000_000 + obstacles.count),
                                                         net: entry.net, kind: .trace))
                    }
                }
                for via in entry.vias {
                    let core = CopperShape.Core.segment(via.centre, via.centre)
                    obstacles.append(RoutingObstacle(shape: CopperShape(core: core, radius: via.diameter / 2, layers: nil,
                                                                        node: 20_000_000 + obstacles.count),
                                                     net: entry.net, kind: .via,
                                                     drill: CopperShape(core: core, radius: via.drill / 2, layers: nil, node: 0)))
                }
            }
            return obstacles
        }

        /// Joins `net`'s groups to the group of its first component, nearest
        /// first. A hard search first; failing that (when `present` > 0), a
        /// relaxed one, whose victims are ripped up before the connection is
        /// searched hard again. Returns the nets that lost copper.
        mutating func join(_ net: RoutingNet, present: Double, round: Int, shove: Bool = false,
                           progress: (RoutingProgress) -> Void, isCancelled: () -> Bool) -> [Int] {
            var lost: [Int] = []
            var skipped = Set<Int>()
            var hard = rules(net)
            hard.present = 0
            var relaxed = hard
            relaxed.present = present
            // Its plan becomes real copper; what is left of the plan is other nets'.
            releasePlan(of: net.id)
            let prices = planPrices()
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
                var path = find(hard, sources: sources, targets: targets, plan: prices, isCancelled: isCancelled)
                // An improvement move (`shove`) also looks through other nets'
                // copper when the legal way needs a via, and takes that way
                // when it needs fewer.
                let hardVias = path.map { Router.viaCount($0, cells: grid.cellCount) }
                if path == nil || (shove && hardVias ?? 0 > 0), present > 0, !stopped,
                   let detour = find(relaxed, sources: sources, targets: targets, plan: prices, isCancelled: isCancelled),
                   hardVias.map({ Router.viaCount(detour, cells: grid.cellCount) < $0 }) ?? true {
                    var all = state.victims(of: detour, net: net.id)
                    // A coupled pair goes as a unit: both nets' copper, to be
                    // routed again, coupled, first thing next round.
                    let pairs = Set(all.compactMap { state.entries[$0].flatMap { coupledPair(of: $0.net) }?.id })
                    for group in problem.groups where pairs.contains(group.id) {
                        all += state.ledger.filter { group.nets.contains($0.net) && !all.contains($0.id) }.map(\.id)
                    }
                    let nets = Set(all.compactMap { state.entries[$0]?.net })
                    if polishing {
                        // A move is kept or undone whole (`attempt`): no budget, no
                        // history, and never a coupled pair's copper.
                        if !all.isEmpty, pairs.isEmpty, nets.isDisjoint(with: pinned), nets.isDisjoint(with: pairNets) {
                            lost += state.rip(all)
                            path = find(hard, sources: sources, targets: targets, plan: prices, isCancelled: isCancelled) ?? path
                        }
                    } else if !all.isEmpty, nets.isDisjoint(with: pinned),
                       nets.allSatisfy({ (ripped[$0] ?? 0) < ripBudget($0) }) {
                        for victimNet in nets.sorted() { ripped[victimNet, default: 0] += 1 }
                        rips += all.count
                        lost += state.rip(all)
                        rippedPairs.formUnion(pairs)
                        state.addHistory(detour, step: strategy.historyStep)
                        path = find(hard, sources: sources, targets: targets, plan: prices, isCancelled: isCancelled)
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

        mutating func find(_ rules: SearchRules, sources: [Int], targets: [Int], plan: RoutingPlanCosts? = nil,
                           isCancelled: () -> Bool) -> [Int]? {
            guard !sources.isEmpty, !targets.isEmpty else { return nil }
            switch search.search(grid, rules: rules, sources: sources, targets: targets, goal: grid.cellBox(targets),
                                 history: rules.present > 0 ? state.history : nil, plan: plan, isCancelled: isCancelled) {
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
            result.stats.planPasses = plan?.iterations ?? 0
            result.stats.planOveruse = plan?.overuse ?? 0
            result.stats.polishTried = polishTried
            result.stats.polishKept = polishKept
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

    /// The layer changes along a searched path.
    static func viaCount(_ path: [Int], cells: Int) -> Int {
        zip(path, path.dropFirst()).reduce(0) { $0 + ($1.0 / cells != $1.1 / cells ? 1 : 0) }
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
