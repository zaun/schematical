import CoreGraphics
import Foundation

/// The costs and rules one connection's search runs under (plan-38-E § 3).
nonisolated struct SearchRules: Sendable {
    /// The net searched for, and its grid profile.
    var net: Int
    var profile: Int
    /// By layer index: may the net use it.
    var allowed: [Bool]
    /// By layer index: is it a preferred layer (all false: no preference).
    var preferred: [Bool]
    /// By layer index: the direction the layer favours.
    var directions: [RoutingOptions.Direction]
    var orthogonal = false
    /// Whether the search may change layers (by a via).
    var vias = true
    /// Costs in cells.
    var viaCost: Double = 12
    var bendCost: Double = 0.3
    var directionPenalty: Double = 1.5
    var preferPenalty: Double = 1.6
    var maxExpansions = 3_000_000

    init(net: Int, profile: Int, allowed: [Bool], preferred: [Bool], directions: [RoutingOptions.Direction]) {
        self.net = net
        self.profile = profile
        self.allowed = allowed
        self.preferred = preferred
        self.directions = directions
    }

    /// The rules for `net` in `problem` on `grid`: its allowed and
    /// preferred layers, the run's angle and layer directions, and vias
    /// unless the run has none, the net may use only one layer, or its via
    /// hole is under the DRC's minimum.
    init(net: RoutingNet, grid: RoutingGrid, problem: RoutingProblem) {
        let options = problem.options
        let allowed = grid.layers.map { net.allowed.contains($0) }
        self.init(net: net.id, profile: grid.profile(of: net.id), allowed: allowed,
                  preferred: grid.layers.map { net.preferred.contains($0) },
                  directions: grid.layers.map { options.option(for: $0).direction })
        orthogonal = options.angle == .orthogonal
        vias = options.vias == .allowed && allowed.filter { $0 }.count >= 2
            && net.viaDrill >= problem.rules.minViaDrill - 1e-9
    }
}

/// Why a search found nothing.
nonisolated enum SearchFailure: Error, Equatable, Sendable {
    /// Every reachable node was tried.
    case noPath
    /// It tried `SearchRules.maxExpansions` nodes.
    case tooLong
    case cancelled
}

/// A* over grid nodes (layer × cell), reusing its buffers between
/// searches. Deterministic: the open set orders by cost, then by node.
nonisolated struct RoutingSearch {
    /// Node = layer * cellCount + cell.
    private var cost: [Float]
    private var came: [Int32]
    private var touched: [Int] = []
    private var targetMark: [UInt32]
    private var generation: UInt32 = 0
    /// Expansions over every search so far.
    private(set) var expansions = 0

    init(nodes: Int) {
        cost = Array(repeating: .infinity, count: nodes)
        came = Array(repeating: -1, count: nodes)
        targetMark = Array(repeating: 0, count: nodes)
    }

    private struct Heap {
        var items: [(f: Float, node: Int32)] = []
        static func less(_ a: (f: Float, node: Int32), _ b: (f: Float, node: Int32)) -> Bool {
            a.f < b.f || (a.f == b.f && a.node < b.node)
        }
        mutating func push(_ f: Float, _ node: Int32) {
            items.append((f, node))
            var i = items.count - 1
            while i > 0 {
                let parent = (i - 1) / 2
                guard Self.less(items[i], items[parent]) else { break }
                items.swapAt(i, parent)
                i = parent
            }
        }
        mutating func pop() -> (f: Float, node: Int32)? {
            guard let top = items.first else { return nil }
            let last = items.removeLast()
            if !items.isEmpty {
                items[0] = last
                var i = 0
                while true {
                    let l = 2 * i + 1, r = l + 1
                    var m = i
                    if l < items.count, Self.less(items[l], items[m]) { m = l }
                    if r < items.count, Self.less(items[r], items[m]) { m = r }
                    if m == i { break }
                    items.swapAt(i, m)
                    i = m
                }
            }
            return top
        }
    }

    private static let steps: [(dx: Int, dy: Int, cost: Float)] = [
        (1, 0, 1), (-1, 0, 1), (0, 1, 1), (0, -1, 1),
        (1, 1, 1.41421356), (1, -1, 1.41421356), (-1, 1, 1.41421356), (-1, -1, 1.41421356),
    ]

    /// The cheapest path from any of `sources` to any of `targets` (nodes),
    /// source first, or why there is none. `goal` is the targets' bounding
    /// box in cell coordinates, for the heuristic; nil searches without one
    /// (Dijkstra). `isCancelled` is polled every 4096 expansions.
    mutating func search(_ grid: RoutingGrid, rules: SearchRules, sources: [Int], targets: [Int],
                         goal: (x: ClosedRange<Int>, y: ClosedRange<Int>)?,
                         isCancelled: () -> Bool) -> Result<[Int], SearchFailure> {
        for node in touched {
            cost[node] = .infinity
            came[node] = -1
        }
        touched.removeAll(keepingCapacity: true)
        generation &+= 1
        if generation == 0 {
            targetMark = Array(repeating: 0, count: targetMark.count)
            generation = 1
        }
        for node in targets { targetMark[node] = generation }

        let cells = grid.cellCount, nx = grid.nx, ny = grid.ny, layerCount = grid.layers.count
        func h(_ cell: Int) -> Float {
            guard let goal else { return 0 }
            let x = cell % nx, y = cell / nx
            let dx = Float(max(0, goal.x.lowerBound - x, x - goal.x.upperBound))
            let dy = Float(max(0, goal.y.lowerBound - y, y - goal.y.upperBound))
            return max(dx, dy) + 0.41421356 * min(dx, dy)
        }
        var open = Heap()
        for node in sources.sorted() where cost[node] != 0 {
            if cost[node] == .infinity { touched.append(node) }
            cost[node] = 0
            open.push(h(node % cells), Int32(node))
        }
        let net = rules.net, profile = rules.profile
        let anyPreferred = rules.preferred.contains(true)
        var local = 0
        while let (f, packed) = open.pop() {
            let node = Int(packed)
            let g = cost[node]
            let cell = node % cells
            if f > g + h(cell) + 1e-4 { continue }
            if targetMark[node] == generation {
                var path = [node]
                var current = node
                while came[current] >= 0 {
                    current = Int(came[current])
                    path.append(current)
                }
                expansions += local
                return .success(path.reversed())
            }
            local += 1
            if local >= rules.maxExpansions { expansions += local; return .failure(.tooLong) }
            if local & 4095 == 0, isCancelled() { expansions += local; return .failure(.cancelled) }

            let layer = node / cells, x = cell % nx, y = cell / nx
            let previous = came[node] >= 0 ? Int(came[node]) : -1
            var lastDX = 0, lastDY = 0
            if previous >= 0, previous / cells == layer {
                let pc = previous % cells
                lastDX = x - pc % nx
                lastDY = y - pc / nx
            }
            let layerScale: Float = anyPreferred && !rules.preferred[layer] ? Float(rules.preferPenalty) : 1
            let direction = rules.directions[layer]
            for (k, step) in Self.steps.enumerated() {
                if rules.orthogonal, k >= 4 { break }
                let jx = x + step.dx, jy = y + step.dy
                guard jx >= 0, jy >= 0, jx < nx, jy < ny else { continue }
                let next = jy * nx + jx
                guard grid.isPassable(next, layer: layer, net: net, profile: profile) else { continue }
                if k >= 4 {
                    // No squeezing past a corner: both orthogonal cells too.
                    guard grid.isPassable(y * nx + jx, layer: layer, net: net, profile: profile),
                          grid.isPassable(jy * nx + x, layer: layer, net: net, profile: profile) else { continue }
                }
                var stepCost = step.cost
                switch direction {
                case .horizontal where step.dy != 0, .vertical where step.dx != 0:
                    stepCost *= Float(rules.directionPenalty)
                default: break
                }
                stepCost *= layerScale
                if (lastDX != 0 || lastDY != 0), lastDX != step.dx || lastDY != step.dy { stepCost += Float(rules.bendCost) }
                let target = layer * cells + next
                let candidate = g + stepCost
                if candidate < cost[target] {
                    if cost[target] == .infinity { touched.append(target) }
                    cost[target] = candidate
                    came[target] = Int32(node)
                    open.push(candidate + h(next), Int32(target))
                }
            }
            // A via: to every other allowed layer, where one fits.
            if rules.vias, layerCount > 1, grid.viaFits(cell, net: net, profile: profile) {
                for other in 0..<layerCount where other != layer && rules.allowed[other] {
                    let target = other * cells + cell
                    let candidate = g + Float(rules.viaCost)
                    if candidate < cost[target] {
                        if cost[target] == .infinity { touched.append(target) }
                        cost[target] = candidate
                        came[target] = Int32(node)
                        open.push(candidate + h(cell), Int32(target))
                    }
                }
            }
        }
        expansions += local
        return .failure(.noPath)
    }
}
