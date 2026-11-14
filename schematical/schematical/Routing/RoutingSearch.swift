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
    /// A relaxed search's price per cell of other nets' routed copper
    /// (times 1 + the cell's history); 0 is a hard search (plan-38-F).
    var present: Double = 0

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
    /// The cost of the last path found.
    private(set) var lastCost: Float = 0

    init(nodes: Int) {
        cost = Array(repeating: .infinity, count: nodes)
        came = Array(repeating: -1, count: nodes)
        targetMark = Array(repeating: 0, count: nodes)
    }

    /// A min-heap of open nodes, each (cost, node) packed in one `UInt64`:
    /// a non-negative Float's bits order as the number does, so the high
    /// half orders by cost and the low half breaks ties by node.
    private struct Heap {
        private var storage: UnsafeMutablePointer<UInt64>
        private var capacity: Int
        private(set) var count = 0

        init() {
            capacity = 4096
            storage = .allocate(capacity: capacity)
        }

        func release() { storage.deallocate() }

        static func pack(_ f: Float, _ node: Int) -> UInt64 { UInt64(f.bitPattern) << 32 | UInt64(UInt32(node)) }

        mutating func push(_ item: UInt64) {
            if count == capacity {
                let bigger = UnsafeMutablePointer<UInt64>.allocate(capacity: capacity * 2)
                bigger.update(from: storage, count: count)
                storage.deallocate()
                storage = bigger
                capacity *= 2
            }
            var i = count
            count += 1
            while i > 0 {
                let parent = (i - 1) >> 1
                let above = storage[parent]
                if above <= item { break }
                storage[i] = above
                i = parent
            }
            storage[i] = item
        }

        mutating func pop() -> UInt64? {
            guard count > 0 else { return nil }
            let top = storage[0]
            count -= 1
            guard count > 0 else { return top }
            let last = storage[count]
            var i = 0
            while true {
                let l = 2 * i + 1
                if l >= count { break }
                var m = l
                if l + 1 < count, storage[l + 1] < storage[l] { m = l + 1 }
                if storage[m] >= last { break }
                storage[i] = storage[m]
                i = m
            }
            storage[i] = last
            return top
        }
    }

    /// The cheapest path from any of `sources` to any of `targets` (nodes),
    /// source first, or why there is none. `goal` is the targets' bounding
    /// box in cell coordinates, for the heuristic; nil searches without one
    /// (Dijkstra). `isCancelled` is polled every 4096 expansions.
    mutating func search(_ grid: RoutingGrid, rules: SearchRules, sources: [Int], targets: [Int],
                         goal: (x: ClosedRange<Int>, y: ClosedRange<Int>)?, history: [Float]? = nil, startCosts: [Int: Float]? = nil,
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

        var cost: [Float] = [], came: [Int32] = [], touched: [Int] = []
        swap(&cost, &self.cost)
        swap(&came, &self.came)
        swap(&touched, &self.touched)
        defer {
            swap(&cost, &self.cost)
            swap(&came, &self.came)
            swap(&touched, &self.touched)
        }
        let marks = targetMark, generation = self.generation
        var local = 0
        var found: Float = 0
        let history = history ?? []
        let outcome: Result<[Int], SearchFailure> = cost.withUnsafeMutableBufferPointer { cost in
            came.withUnsafeMutableBufferPointer { came in
                marks.withUnsafeBufferPointer { marks in
                    grid.traceMaps.withUnsafeBufferPointer { traceMaps in
                        grid.viaMaps.withUnsafeBufferPointer { viaMaps in
                            grid.fixedTraceMaps.withUnsafeBufferPointer { fixedTrace in
                                grid.fixedViaMaps.withUnsafeBufferPointer { fixedVia in
                                    history.withUnsafeBufferPointer { history in
                                        Self.explore(grid, rules: rules, sources: sources, startCosts: startCosts, goal: goal, cost: cost,
                                                     came: came, marks: marks, generation: generation,
                                                     maps: Maps(trace: traceMaps, via: viaMaps, fixedTrace: fixedTrace,
                                                                fixedVia: fixedVia, history: history),
                                                     touched: &touched, expanded: &local, found: &found,
                                                     isCancelled: isCancelled)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        expansions += local
        lastCost = found
        return outcome
    }

    /// The grid's maps as the search reads them.
    private struct Maps {
        let trace: UnsafeBufferPointer<Int32>
        let via: UnsafeBufferPointer<Int32>
        let fixedTrace: UnsafeBufferPointer<Int32>
        let fixedVia: UnsafeBufferPointer<Int32>
        /// By node; empty outside a relaxed search.
        let history: UnsafeBufferPointer<Float>
    }

    private static func explore(_ grid: RoutingGrid, rules: SearchRules, sources: [Int], startCosts: [Int: Float]?,
                                goal: (x: ClosedRange<Int>, y: ClosedRange<Int>)?,
                                cost: UnsafeMutableBufferPointer<Float>, came: UnsafeMutableBufferPointer<Int32>,
                                marks: UnsafeBufferPointer<UInt32>, generation: UInt32, maps: Maps,
                                touched: inout [Int], expanded: inout Int, found: inout Float,
                                isCancelled: () -> Bool) -> Result<[Int], SearchFailure> {
        let traceMaps = maps.trace, viaMaps = maps.via
        let cells = grid.cellCount, nx = grid.nx, ny = grid.ny, layerCount = grid.layers.count
        let net = Int32(rules.net)
        let base = rules.profile * layerCount * cells
        let hasGoal = goal != nil
        let gx0 = goal?.x.lowerBound ?? 0, gx1 = goal?.x.upperBound ?? 0
        let gy0 = goal?.y.lowerBound ?? 0, gy1 = goal?.y.upperBound ?? 0
        @inline(__always) func h(_ x: Int, _ y: Int) -> Float {
            guard hasGoal else { return 0 }
            let dx = Float(max(0, gx0 - x, x - gx1)), dy = Float(max(0, gy0 - y, y - gy1))
            return dx > dy ? dx + 0.41421356 * dy : dy + 0.41421356 * dx
        }
        let relaxed = rules.present > 0 && maps.history.count == layerCount * cells
        let present = Float(rules.present)
        /// What entering a cell on a layer costs beyond the step: 0 when
        /// it is free or the net's own; in a relaxed search, the price of a
        /// cell only other nets' routed copper covers; -1 when it can't be
        /// entered at all (fixed copper, two nets, the edge).
        @inline(__always) func price(_ layer: Int, _ cell: Int) -> Float {
            let index = base + layer * cells + cell
            let value = traceMaps[index]
            if value == -1 || value == net { return 0 }
            if relaxed {
                let fixed = maps.fixedTrace[index]
                if fixed == -1 || fixed == net { return present * (1 + maps.history[layer * cells + cell]) }
            }
            return -1
        }
        @inline(__always) func passable(_ layer: Int, _ cell: Int) -> Bool { price(layer, cell) >= 0 }
        /// A via's price at a cell over every layer, as `price`.
        @inline(__always) func viaPrice(_ cell: Int) -> Float {
            var layer = 0, total: Float = 0
            while layer < layerCount {
                let index = base + layer * cells + cell
                let value = viaMaps[index]
                if value != -1 && value != net {
                    guard relaxed else { return -1 }
                    let fixed = maps.fixedVia[index]
                    guard fixed == -1 || fixed == net else { return -1 }
                    total += present * (1 + maps.history[layer * cells + cell])
                }
                layer += 1
            }
            return total
        }
        let sdx = [1, -1, 0, 0, 1, 1, -1, -1], sdy = [0, 0, 1, -1, 1, -1, 1, -1]
        let stepCount = rules.orthogonal ? 4 : 8
        var stepCost = [Float](repeating: 0, count: 8 * max(layerCount, 1))
        let anyPreferred = rules.preferred.contains(true)
        for layer in 0..<layerCount {
            let scale: Float = anyPreferred && !rules.preferred[layer] ? Float(rules.preferPenalty) : 1
            for k in 0..<8 {
                var c: Float = k < 4 ? 1 : 1.41421356
                switch rules.directions[layer] {
                case .horizontal where sdy[k] != 0, .vertical where sdx[k] != 0: c *= Float(rules.directionPenalty)
                default: break
                }
                stepCost[layer * 8 + k] = c * scale
            }
        }
        let bend = Float(rules.bendCost), viaCost = Float(rules.viaCost)
        let allowed = rules.allowed, vias = rules.vias && layerCount > 1, limit = rules.maxExpansions

        var open = Heap()
        defer { open.release() }
        for node in sources.sorted() {
            // A source may start with a cost (a pair's ends, plan-38-J).
            let start = startCosts?[node] ?? 0
            guard start < cost[node] else { continue }
            if cost[node] == .infinity { touched.append(node) }
            cost[node] = start
            let c = node % cells
            open.push(Heap.pack(start + h(c % nx, c / nx), node))
        }
        while let item = open.pop() {
            let node = Int(UInt32(truncatingIfNeeded: item))
            let f = Float(bitPattern: UInt32(item >> 32))
            let g = cost[node]
            let cell = node % cells, layer = node / cells
            let x = cell % nx, y = cell / nx
            if f > g + h(x, y) + 1e-4 { continue }
            if marks[node] == generation {
                var path = [node]
                var current = node
                while came[current] >= 0 {
                    current = Int(came[current])
                    path.append(current)
                }
                found = g
                return .success(path.reversed())
            }
            expanded += 1
            if expanded >= limit { return .failure(.tooLong) }
            if expanded & 4095 == 0, isCancelled() { return .failure(.cancelled) }

            let previous = Int(came[node])
            var lastDX = 0, lastDY = 0
            if previous >= 0, previous / cells == layer {
                let pc = previous % cells
                lastDX = x - pc % nx
                lastDY = y - pc / nx
            }
            let layerBase = layer * cells
            var k = 0
            while k < stepCount {
                let dx = sdx[k], dy = sdy[k]
                let jx = x + dx, jy = y + dy
                k += 1
                if jx < 0 || jy < 0 || jx >= nx || jy >= ny { continue }
                let next = jy * nx + jx
                let enter = price(layer, next)
                if enter < 0 { continue }
                // No squeezing past a corner: both orthogonal cells too.
                if dx != 0 && dy != 0 && (!passable(layer, y * nx + jx) || !passable(layer, jy * nx + x)) { continue }
                var step = stepCost[layer * 8 + k - 1] + enter
                if (lastDX != 0 || lastDY != 0) && (lastDX != dx || lastDY != dy) { step += bend }
                let target = layerBase + next
                let candidate = g + step
                if candidate < cost[target] {
                    if cost[target] == .infinity { touched.append(target) }
                    cost[target] = candidate
                    came[target] = Int32(node)
                    open.push(Heap.pack(candidate + h(jx, jy), target))
                }
            }
            // A via: to every other allowed layer, where one fits.
            let via = vias ? viaPrice(cell) : -1
            if via >= 0 {
                let candidate = g + viaCost + via
                for other in 0..<layerCount where other != layer && allowed[other] {
                    let target = other * cells + cell
                    if candidate < cost[target] {
                        if cost[target] == .infinity { touched.append(target) }
                        cost[target] = candidate
                        came[target] = Int32(node)
                        open.push(Heap.pack(candidate + h(x, y), target))
                    }
                }
            }
        }
        return .failure(.noPath)
    }
}
