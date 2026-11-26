import CoreGraphics
import Foundation

/// How much planned copper sits near each grid node: the soft half of the
/// router's two-stage routing. Every planned net adds one count to
/// each node where a trace centre of profile `q` would come too near its
/// planned copper — exactly where `RoutingGrid.stamp` would block `q` if that
/// copper were stamped — on `q`'s own layer map. A node's count is therefore
/// "how many other nets this trace would have to share its place with".
///
/// A net counts once per node however many of its connections pass it, and
/// one net's own copper is removed (`remove`) before it is planned again, so
/// the count never charges a net for itself.
nonisolated struct RoutingDemand: Sendable {
    private let nx: Int
    private let ny: Int
    private let cells: Int
    private let layerCount: Int
    private let profileCount: Int
    /// Indexed like the grid's trace maps: `(profile * layers + layer) * cells + cell`.
    private(set) var counts: [Int16]
    private var marks: [UInt32]
    /// Marks `marks`; wraps to 1 after `UInt32.max` applies (set by a test).
    var generation: UInt32 = 0
    /// `[a * profiles + q]`: the offsets, in cells, from a planned trace point
    /// of profile `a` that a trace centre of profile `q` may not stand on.
    private let traceDisks: [[(dx: Int, dy: Int)]]
    /// The same against a planned via of profile `a`, on every layer.
    private let viaDisks: [[(dx: Int, dy: Int)]]

    init(grid: RoutingGrid) {
        nx = grid.nx
        ny = grid.ny
        cells = grid.cellCount
        layerCount = max(grid.layers.count, 1)
        profileCount = grid.profiles.count
        let size = profileCount * layerCount * cells
        counts = Array(repeating: 0, count: size)
        marks = Array(repeating: 0, count: size)
        func disk(_ radius: Double) -> [(dx: Int, dy: Int)] {
            let reach = Int((radius / grid.pitch).rounded(.up))
            var offsets: [(dx: Int, dy: Int)] = []
            for dy in -reach...reach {
                for dx in -reach...reach where hypot(Double(dx), Double(dy)) * grid.pitch < radius { offsets.append((dx, dy)) }
            }
            return offsets
        }
        var trace: [[(dx: Int, dy: Int)]] = [], via: [[(dx: Int, dy: Int)]] = []
        for a in grid.profiles {
            for q in grid.profiles {
                let gap = max(a.clearance, q.clearance)
                trace.append(disk(gap + a.halfWidth + q.halfWidth))
                via.append(disk(gap + a.viaRadius + q.halfWidth))
            }
        }
        traceDisks = trace
        viaDisks = via
    }

    /// Adds (`sign` 1) or removes (-1) the copper of one net, as the node
    /// paths of its connections, planned with profile `a`.
    private mutating func apply(_ paths: [[Int]], profile a: Int, sign: Int16) {
        guard !paths.isEmpty else { return }
        generation &+= 1
        if generation == 0 {
            marks = Array(repeating: 0, count: marks.count)
            generation = 1
        }
        for q in 0..<profileCount {
            let traceDisk = traceDisks[a * profileCount + q], viaDisk = viaDisks[a * profileCount + q]
            for path in paths {
                for (i, node) in path.enumerated() {
                    let layer = node / cells, cell = node % cells
                    stamp(traceDisk, cell: cell, layer: layer, profile: q, sign: sign)
                    // A layer change at a cell is a via: on every layer.
                    if i + 1 < path.count, path[i + 1] / cells != layer {
                        for l in 0..<layerCount { stamp(viaDisk, cell: cell, layer: l, profile: q, sign: sign) }
                    }
                }
            }
        }
    }

    private mutating func stamp(_ disk: [(dx: Int, dy: Int)], cell: Int, layer: Int, profile q: Int, sign: Int16) {
        let x = cell % nx, y = cell / nx
        let base = (q * layerCount + layer) * cells
        for offset in disk {
            let xx = x + offset.dx, yy = y + offset.dy
            guard xx >= 0, yy >= 0, xx < nx, yy < ny else { continue }
            let index = base + yy * nx + xx
            if marks[index] != generation {
                marks[index] = generation
                counts[index] += sign
            }
        }
    }

    mutating func add(_ paths: [[Int]], profile: Int) { apply(paths, profile: profile, sign: 1) }
    mutating func remove(_ paths: [[Int]], profile: Int) { apply(paths, profile: profile, sign: -1) }

    /// The count at a node for a trace centre of `profile`.
    func count(node: Int, profile: Int) -> Int {
        Int(counts[profile * layerCount * cells + node])
    }

    /// Where a net whose copper (`paths`, profile `profile`) is in the map
    /// shares a place with other nets: each node of its paths and the number
    /// of other nets there.
    func conflicts(_ paths: [[Int]], profile: Int) -> [(node: Int, others: Int)] {
        var seen = Set<Int>()
        var found: [(node: Int, others: Int)] = []
        for path in paths {
            for node in path where seen.insert(node).inserted {
                let others = count(node: node, profile: profile) - 1
                if others > 0 { found.append((node, others)) }
            }
        }
        return found
    }
}

/// The routing plan: every connection routed once over the fixed
/// copper alone, against a soft map of what the other nets' plans need, and
/// again and again — each net taking its own copper out, planning afresh,
/// putting it back — with the price of sharing rising each pass and the
/// contested nodes remembered, until no two nets' plans want the same place
/// (negotiated congestion, as in PathFinder). The plan stamps nothing; it
/// only prices the detailed routing that follows, so what is committed stays
/// exactly legal.
nonisolated struct RoutingPlan: Sendable {
    var demand: RoutingDemand
    /// Contested nodes, indexed `layer * cells + cell`.
    var history: [Float]
    /// Each net's planned connections, as node paths, while its plan is in `demand`.
    private(set) var paths: [Int: [[Int]]] = [:]
    private(set) var profiles: [Int: Int] = [:]
    /// Passes made, and the nodes still shared after the last.
    var iterations = 0
    var overuse = 0
    /// The nodes shared after each pass.
    var overuseByPass: [Int] = []
    private let nx: Int
    private let ny: Int
    private let cells: Int

    init(grid: RoutingGrid) {
        demand = RoutingDemand(grid: grid)
        history = Array(repeating: 0, count: grid.cellCount * max(grid.layers.count, 1))
        nx = grid.nx
        ny = grid.ny
        cells = grid.cellCount
    }

    /// Puts a net's planned connections in the map.
    mutating func set(net: Int, profile: Int, paths new: [[Int]]) {
        release(net: net)
        guard !new.isEmpty else { return }
        paths[net] = new
        profiles[net] = profile
        demand.add(new, profile: profile)
    }

    /// Takes a net's plan out of the map (it is routed, or about to be planned again).
    mutating func release(net: Int) {
        guard let old = paths[net], let profile = profiles[net] else { return }
        demand.remove(old, profile: profile)
        paths[net] = nil
        profiles[net] = nil
    }

    /// A node's remembered congestion never exceeds this many cells of price.
    static let historyCap: Float = 30

    var plannedNets: [Int] { paths.keys.sorted() }

    /// How many nodes of net `net`'s plan other nets' plans share.
    func overuse(net: Int) -> Int {
        guard let own = paths[net], let profile = profiles[net] else { return 0 }
        return demand.conflicts(own, profile: profile).count
    }

    /// Makes every node a net shares with others dearer for the next pass:
    /// `step` per other net, and half that on the eight nodes round it.
    mutating func remember(net: Int, step: Float) {
        guard let own = paths[net], let profile = profiles[net] else { return }
        let layers = history.count / cells
        for (node, others) in demand.conflicts(own, profile: profile) {
            let layer = node / cells, cell = node % cells
            let x = cell % nx, y = cell / nx
            for dy in -1...1 {
                for dx in -1...1 {
                    let xx = x + dx, yy = y + dy
                    guard xx >= 0, yy >= 0, xx < nx, yy < ny, layer < layers else { continue }
                    let index = layer * cells + yy * nx + xx
                    history[index] = min(history[index] + step * Float(others) * (dx == 0 && dy == 0 ? 1 : 0.5), RoutingPlan.historyCap)
                }
            }
        }
    }

    /// The prices a detailed search of a net of `profile` adds, at `lambda`.
    func costs(lambda: Double) -> RoutingPlanCosts {
        RoutingPlanCosts(demand: demand.counts, history: history, lambda: Float(lambda))
    }
}

nonisolated extension Router.Run {
    /// The nets that take part in the plan: not coupled pairs and not
    /// plane-fan-out nets (their copper is already down), not nets with
    /// nothing left to join.
    var plannedCandidates: [RoutingNet] {
        routable.filter { !fansOut($0) && !pairNets.contains($0.id) && unrouted($0) > 0 }
    }

    /// Plans every connection of every candidate net (see `RoutingPlan`);
    /// the result is in `plan`. Does nothing for a strategy without plan
    /// passes, or when fewer than two nets could compete.
    mutating func planAll(progress: (RoutingProgress) -> Void, isCancelled: () -> Bool) {
        guard strategy.planPasses > 0, !stopped else { return }
        let nets = ordered(plannedCandidates, failures: [:])
        guard nets.count > 1 else { return }
        var plan = RoutingPlan(grid: grid)
        var lambda = strategy.planPrice
        var order = nets
        var shared: [Int: Int] = [:]
        var best: RoutingPlan?
        var stalled = 0
        for pass in 0..<strategy.planPasses {
            // The first pass plans every net; later ones only the nets whose
            // plans share a place with another's.
            let replan = pass == 0 ? order : order.filter { (shared[$0.id] ?? 0) > 0 }
            for net in replan {
                if stopped || isCancelled() { stopped = true; break }
                let profile = grid.profile(of: net.id)
                plan.release(net: net.id)
                var hard = rules(net)
                hard.present = 0
                let paths = planNet(net, rules: hard, plan: plan.costs(lambda: lambda), isCancelled: isCancelled)
                plan.set(net: net.id, profile: profile, paths: paths)
            }
            if stopped { break }
            var total = 0
            shared = [:]
            for net in nets {
                let over = plan.overuse(net: net.id)
                shared[net.id] = over
                total += over
                plan.remember(net: net.id, step: strategy.planHistory)
            }
            plan.iterations = pass + 1
            plan.overuse = total
            plan.overuseByPass.append(total)
            var report = RoutingProgress(routed: 0, total: problem.connections, round: 0, net: "Planning, pass \(pass + 1)")
            report.current = 0
            progress(report)
            if best == nil || total < best!.overuse { best = plan; stalled = 0 } else { stalled += 1 }
            if total == 0 || stalled >= strategy.planStall { break }
            lambda = min(lambda * strategy.planGrowth, strategy.planPriceMax)
            // The nets in the most contested places go first next pass.
            let rank = Dictionary(uniqueKeysWithValues: nets.enumerated().map { ($1.id, $0) })
            order = nets.sorted { a, b in
                let (sa, sb) = (shared[a.id] ?? 0, shared[b.id] ?? 0)
                return sa != sb ? sa > sb : (rank[a.id] ?? 0) < (rank[b.id] ?? 0)
            }
        }
        // Keep the plan that shared the least, not the last one made.
        if var kept = best {
            kept.iterations = plan.iterations
            kept.overuseByPass = plan.overuseByPass
            plan = kept
        }
        self.plan = plan
    }

    /// One net's connections, planned: its groups joined nearest first, as
    /// the detailed routing will join them, each by the cheapest path from the
    /// groups reached so far (and the plan's own copper of this net) to the
    /// next group, priced by `plan`. A group no path reaches is left out.
    mutating func planNet(_ net: RoutingNet, rules: SearchRules, plan: RoutingPlanCosts,
                          isCancelled: () -> Bool) -> [[Int]] {
        let groups = state.groups(of: net.id)
        guard groups.components.count > 1 else { return [] }
        var reached: Set<Int> = [0], skipped = Set<Int>()
        var plannedNodes: [Int] = []
        var paths: [[Int]] = []
        let cells = grid.cellCount
        while reached.count + skipped.count < groups.components.count {
            var treePoints: [CGPoint] = []
            for g in reached.sorted() {
                for c in groups.components[g] { treePoints += Router.padPoints(net.components[c]).map(\.point) }
            }
            var next: Int?
            var nearest = Double.infinity
            for g in groups.components.indices where !reached.contains(g) && !skipped.contains(g) {
                for c in groups.components[g] {
                    for (point, _) in Router.padPoints(net.components[c]) {
                        for q in treePoints {
                            let d = hypot(point.x - q.x, point.y - q.y)
                            if d < nearest { nearest = d; next = g }
                        }
                    }
                }
            }
            guard let target = next else { break }
            let treeComponents = reached.sorted().flatMap { groups.components[$0] }
            let treeEntries = reached.sorted().flatMap { groups.entries[$0] }
            let sources = Set(state.nodes(components: treeComponents, entries: treeEntries, net: net.id)).union(plannedNodes).sorted()
            let targets = state.nodes(components: groups.components[target], entries: groups.entries[target], net: net.id)
            guard !sources.isEmpty, !targets.isEmpty else { skipped.insert(target); continue }
            let outcome = search.search(grid, rules: rules, sources: sources, targets: targets, goal: grid.cellBox(targets),
                                        plan: plan, isCancelled: isCancelled)
            switch outcome {
            case let .success(path):
                paths.append(path)
                reached.insert(target)
                plannedNodes += path
                // A via of the plan is a place the net can be joined on every layer.
                for (i, node) in path.enumerated() where i + 1 < path.count && path[i + 1] / cells != node / cells {
                    for layer in grid.layers.indices where rules.allowed[layer] { plannedNodes.append(layer * cells + node % cells) }
                }
            case .failure(.cancelled):
                stopped = true
                return paths
            case .failure:
                skipped.insert(target)
            }
        }
        return paths
    }

    /// Takes net `net` out of the plan before it is routed: from then on its
    /// copper is real, and what is left of the plan is other nets'.
    mutating func releasePlan(of net: Int) {
        plan?.release(net: net)
    }

    /// The prices the detailed routing of a net adds from what is still planned.
    func planPrices() -> RoutingPlanCosts? {
        polishing ? nil : plan?.costs(lambda: strategy.commitPrice)
    }
}
