import CoreGraphics
import Foundation

/// One routed connection: its path, its copper, and the box that copper
/// stamps (plan-38-F § 3).
nonisolated struct LedgerEntry: Hashable, Sendable {
    let id: Int
    let net: Int
    /// The searched nodes, source first.
    var path: [Int]
    var traces: [RoutedTrace]
    var vias: [RoutedVia]
    /// The copper's bounding box (centrelines and via centres), mm.
    var box: CGRect
    /// Every node the copper offers a later connection of its net to start
    /// from or end at: the path, and each via's cell on every allowed layer.
    var nodes: [Int]
}

/// The routing state: the grid, every routed connection in a ledger, the
/// nets' components and their terminals. Stamping and un-stamping keep the
/// grid equal, cell for cell, to one built from scratch from the fixed copper
/// and the ledger (tested): un-stamping an entry restores the fixed copper
/// over the cells its copper could reach and stamps again every other entry
/// whose copper could reach them.
nonisolated struct RoutingState: Sendable {
    let problem: RoutingProblem
    var grid: RoutingGrid
    private(set) var entries: [Int: LedgerEntry] = [:]
    private(set) var nextID = 0
    /// Entries by 64-cell tile, for finding the copper near a box.
    private var tiles: [Int: [Int]] = [:]
    /// Each routed net's terminals, by component, for its joins.
    let terminals: [Int: [[RoutingTerminal]]]
    let netsByID: [Int: RoutingNet]
    /// By node: how contested each cell has been (plan-38-F § 3); a relaxed
    /// search pays `present × (1 + history)` to cross other nets' copper.
    var history: [Float]

    static let tileCells = 64

    init(problem: RoutingProblem) {
        self.problem = problem
        let grid = RoutingGrid(problem: problem)
        self.grid = grid
        var terminals: [Int: [[RoutingTerminal]]] = [:]
        var nets: [Int: RoutingNet] = [:]
        for net in problem.nets {
            nets[net.id] = net
            guard net.impossible == nil else { continue }
            let rules = SearchRules(net: net, grid: grid, problem: problem)
            terminals[net.id] = net.components.map {
                grid.terminals(of: $0, net: net.id, profile: rules.profile, allowed: rules.allowed)
            }
        }
        self.terminals = terminals
        netsByID = nets
        history = Array(repeating: 0, count: grid.cellCount * grid.layers.count)
    }

    // MARK: Negotiation

    /// Makes every cell of `path` dearer for the next relaxed search.
    mutating func addHistory(_ path: [Int], step: Float) {
        for node in path where node < history.count { history[node] += step }
    }

    /// The entries of other nets whose copper a path of net `net` crosses:
    /// for each node, the copper within the stamp distance of a trace
    /// centre there (a via's, at a layer change).
    func victims(of path: [Int], net: Int) -> [Int] {
        guard let routing = netsByID[net] else { return [] }
        let cells = grid.cellCount
        let half = routing.width / 2, viaRadius = routing.viaDiameter / 2
        let mine = max(routing.clearance, problem.rules.minClearance)
        var found = Set<Int>()
        let reach = self.reach
        for (i, node) in path.enumerated() {
            let layer = grid.layers[node / cells]
            let p = grid.point(node % cells)
            let isVia = (i > 0 && path[i - 1] / cells != node / cells) || (i + 1 < path.count && path[i + 1] / cells != node / cells)
            let own = isVia ? viaRadius : half
            for entry in entries(near: CGRect(x: p.x - reach, y: p.y - reach, width: 2 * reach, height: 2 * reach))
            where entry.net != net && !found.contains(entry.id) {
                let theirs = netsByID[entry.net].map { max($0.clearance, problem.rules.minClearance) } ?? mine
                let gap = max(mine, theirs)
                let hit = entry.traces.contains { trace in
                    (isVia || trace.layer == layer) && zip(trace.points, trace.points.dropFirst()).contains {
                        SchematicDocument.distance(from: p, toSegment: $0.0, $0.1) < gap + trace.width / 2 + own
                    }
                } || entry.vias.contains { hypot($0.centre.x - p.x, $0.centre.y - p.y) < gap + $0.diameter / 2 + own }
                if hit { found.insert(entry.id) }
            }
        }
        return found.sorted()
    }

    /// Rips up entries, and then any entry of their nets left hanging by
    /// one end; returns the nets that lost copper, sorted.
    @discardableResult
    mutating func rip(_ ids: [Int]) -> [Int] {
        let nets = Set(ids.compactMap { entries[$0]?.net })
        remove(ids)
        var changed = true
        while changed {
            changed = false
            for net in nets.sorted() {
                let loose = dangling(of: net)
                if !loose.isEmpty { remove(loose); changed = true }
            }
        }
        return nets.sorted()
    }

    /// The ledger in the order the entries were made.
    var ledger: [LedgerEntry] { entries.keys.sorted().compactMap { entries[$0] } }

    /// How far any copper's stamp can reach past its core, mm.
    var reach: Double {
        let largest = problem.nets.map { max($0.clearance, problem.rules.minClearance) + max($0.width, $0.viaDiameter) / 2 }
            .max() ?? 0
        let profiles = grid.profiles.map { max($0.clearance, problem.rules.minClearance) + max($0.halfWidth, $0.viaRadius) }
            .max() ?? 0
        return largest + profiles + grid.pitch
    }

    // MARK: Entries

    /// Records and stamps a connection's copper; returns its id.
    @discardableResult
    mutating func add(net: Int, path: [Int], traces: [RoutedTrace], vias: [RoutedVia], allowed: [Bool]) -> Int {
        var points: [CGPoint] = traces.flatMap(\.points) + vias.map(\.centre)
        if points.isEmpty, let first = path.first { points = [grid.point(first % grid.cellCount)] }
        let xs = points.map(\.x), ys = points.map(\.y)
        let box = CGRect(x: xs.min() ?? 0, y: ys.min() ?? 0, width: (xs.max() ?? 0) - (xs.min() ?? 0),
                         height: (ys.max() ?? 0) - (ys.min() ?? 0))
        var nodes = path
        for via in vias {
            let c = grid.nearestCell(via.centre)
            nodes += grid.layers.indices.filter { allowed[$0] }.map { $0 * grid.cellCount + c }
        }
        let entry = LedgerEntry(id: nextID, net: net, path: path, traces: traces, vias: vias, box: box, nodes: nodes)
        nextID += 1
        entries[entry.id] = entry
        for tile in tileKeys(box) { tiles[tile, default: []].append(entry.id) }
        stamp(entry)
        return entry.id
    }

    private mutating func stamp(_ entry: LedgerEntry) {
        for trace in entry.traces {
            guard let layer = grid.layerIndex(trace.layer) else { continue }
            grid.stampTrace(trace.points, width: trace.width, layer: layer, net: entry.net)
        }
        for via in entry.vias { grid.stampVia(via.centre, diameter: via.diameter, net: entry.net) }
    }

    /// Removes entries and their copper from the grid exactly.
    mutating func remove(_ ids: [Int]) {
        let removed = ids.sorted().compactMap { entries[$0] }
        guard !removed.isEmpty else { return }
        for entry in removed {
            entries[entry.id] = nil
            for tile in tileKeys(entry.box) { tiles[tile]?.removeAll { $0 == entry.id } }
        }
        let reach = self.reach
        for entry in removed {
            let area = entry.box.insetBy(dx: -reach, dy: -reach)
            grid.restoreFixed(in: area)
            for other in self.entries(near: area.insetBy(dx: -reach, dy: -reach)) { stamp(other) }
        }
    }

    /// The entries whose copper box meets `box`, by id.
    func entries(near box: CGRect) -> [LedgerEntry] {
        var ids = Set<Int>()
        for tile in tileKeys(box) { ids.formUnion(tiles[tile] ?? []) }
        return ids.sorted().compactMap { entries[$0] }.filter { $0.box.insetBy(dx: -1e-9, dy: -1e-9).intersects(box) }
    }

    private func tileKeys(_ box: CGRect) -> [Int] {
        let size = Double(Self.tileCells) * grid.pitch
        let x0 = Int(((box.minX - grid.origin.x) / size).rounded(.down)), x1 = Int(((box.maxX - grid.origin.x) / size).rounded(.down))
        let y0 = Int(((box.minY - grid.origin.y) / size).rounded(.down)), y1 = Int(((box.maxY - grid.origin.y) / size).rounded(.down))
        var keys: [Int] = []
        for y in y0...max(y0, y1) { for x in x0...max(x0, x1) { keys.append(y * 1_000_003 + x) } }
        return keys
    }

    // MARK: Joins

    /// Net `net`'s groups: its components joined by the ledger's entries,
    /// each group as the sorted component indices, the group holding
    /// component 0 first. An entry joins whatever its first or last node
    /// lands on: a component's terminal, or another entry's node.
    func groups(of net: Int) -> (components: [[Int]], entries: [[Int]]) {
        guard let terminals = terminals[net] else { return ([], []) }
        let mine = ledger.filter { $0.net == net }
        let n = terminals.count
        var parent = Array(0..<(n + mine.count))
        func find(_ i: Int) -> Int {
            var i = i
            while parent[i] != i { parent[i] = parent[parent[i]]; i = parent[i] }
            return i
        }
        func union(_ a: Int, _ b: Int) { let (x, y) = (find(a), find(b)); if x != y { parent[max(x, y)] = min(x, y) } }
        var componentOfNode: [Int: Int] = [:]
        for (c, list) in terminals.enumerated() { for t in list where componentOfNode[t.node] == nil { componentOfNode[t.node] = c } }
        var entryOfNode: [Int: [Int]] = [:]
        for (e, entry) in mine.enumerated() { for node in entry.nodes { entryOfNode[node, default: []].append(e) } }
        for (e, entry) in mine.enumerated() {
            for end in [entry.path.first, entry.path.last].compactMap({ $0 }) {
                if let c = componentOfNode[end] { union(n + e, c) }
                for other in entryOfNode[end] ?? [] where other != e { union(n + e, n + other) }
            }
        }
        var byRoot: [Int: (components: [Int], entries: [Int])] = [:]
        for c in 0..<n { byRoot[find(c), default: ([], [])].components.append(c) }
        for e in mine.indices where byRoot[find(n + e)] != nil { byRoot[find(n + e)]!.entries.append(mine[e].id) }
        let roots = byRoot.keys.sorted()
        return (roots.map { byRoot[$0]!.components }, roots.map { byRoot[$0]!.entries })
    }

    /// The entries of `net` that hang by one end only (their other end
    /// lands on nothing of the net), after rip-ups.
    func dangling(of net: Int) -> [Int] {
        guard let terminals = terminals[net] else { return [] }
        let mine = ledger.filter { $0.net == net }
        var terminalNodes = Set<Int>()
        for list in terminals { for t in list { terminalNodes.insert(t.node) } }
        return mine.filter { entry in
            [entry.path.first, entry.path.last].compactMap { $0 }.contains { end in
                !terminalNodes.contains(end) && !mine.contains { $0.id != entry.id && $0.nodes.contains(end) }
            }
        }.map(\.id)
    }

    /// Every node of a group: its components' terminals and its entries'
    /// nodes, sorted.
    func nodes(components: [Int], entries ids: [Int], net: Int) -> [Int] {
        var nodes = Set<Int>()
        for c in components { for t in terminals[net]?[c] ?? [] { nodes.insert(t.node) } }
        for id in ids { nodes.formUnion(entries[id]?.nodes ?? []) }
        return nodes.sorted()
    }

    /// The tie point of `node` if it is a terminal of `net`'s `components`.
    func tie(_ node: Int, net: Int, components: [Int]) -> CGPoint? {
        for c in components {
            if let terminal = terminals[net]?[c].first(where: { $0.node == node }) { return terminal.tie }
        }
        return nil
    }
}

extension RoutingGrid {
    /// Puts the fixed copper's values back over the cells inside `box` on
    /// every map.
    mutating func restoreFixed(in box: CGRect) {
        guard let range = cellRange(box, margin: 0) else { return }
        let maps = traceMaps.count / cellCount
        for map in 0..<maps {
            let base = map * cellCount
            for y in range.y {
                let row = base + y * nx
                for x in range.x {
                    traceMaps[row + x] = fixedTraceMaps[row + x]
                    viaMaps[row + x] = fixedViaMaps[row + x]
                }
            }
        }
    }
}
