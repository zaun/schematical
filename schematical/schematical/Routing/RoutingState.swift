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
    /// The `MovableChain.id` this entry is, for the existing copper of a
    /// bystander net (plan-66-C); nil for copper the run lays.
    var chain: Int? = nil
    /// Whether a push has moved this chain's copper.
    var touched = false
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
    /// Each routed net's terminal nodes and the component each belongs to
    /// (the first, when two components share a node).
    let componentOfNode: [Int: [Int: Int]]
    /// Each net's entries, by id, oldest first.
    private var entriesOfNet: [Int: [Int]] = [:]
    let netsByID: [Int: RoutingNet]
    /// The nets that only hold existing copper (`problem.bystanders`): never
    /// routed, never ripped up.
    let bystanders: Set<Int>
    /// Where each chain may start and end when it is laid again (plan-66-D), by
    /// `MovableChain.id`: the cells of each end's anchor, every one tied to the
    /// end's own point, so a chain re-laid keeps its ends exactly where they are.
    let chainTerminals: [Int: (a: [RoutingTerminal], b: [RoutingTerminal])]
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
        var componentOfNode: [Int: [Int: Int]] = [:]
        for (net, lists) in terminals {
            var byNode: [Int: Int] = [:]
            for (c, list) in lists.enumerated() { for t in list where byNode[t.node] == nil { byNode[t.node] = c } }
            componentOfNode[net] = byNode
        }
        self.componentOfNode = componentOfNode
        for net in problem.bystanders { nets[net.id] = net }
        var chainTerminals: [Int: (a: [RoutingTerminal], b: [RoutingTerminal])] = [:]
        for chain in problem.movable {
            guard let net = nets[chain.net], chain.ends.count == 2 else { continue }
            let rules = SearchRules(net: net, grid: grid, problem: problem)
            let ends = chain.ends.map { end in
                grid.terminals(of: end.anchor, net: chain.net, profile: rules.profile, allowed: rules.allowed)
                    .map { RoutingTerminal(node: $0.node, tie: end.point) }
            }
            chainTerminals[chain.id] = (ends[0], ends[1])
        }
        self.chainTerminals = chainTerminals
        netsByID = nets
        bystanders = Set(problem.bystanders.map(\.id))
        history = Array(repeating: 0, count: grid.cellCount * grid.layers.count)
        // The existing copper push and shove may move, held as entries: stamped
        // like routed copper, and crossable only at a push price.
        let everywhere = grid.layers.map { _ in true }
        for chain in problem.movable {
            let nodes = chain.ends.map { (grid.layerIndex($0.layer) ?? 0) * grid.cellCount + grid.nearestCell($0.point) }
            let traces = chain.runs.map {
                RoutedTrace(layer: $0.layer, width: chain.width, points: $0.points, net: chain.net)
            }
            let vias = chain.vias.map {
                RoutedVia(centre: $0.centre, drill: chain.viaDrill, diameter: chain.viaDiameter, net: chain.net)
            }
            add(net: chain.net, path: nodes, traces: traces, vias: vias, allowed: everywhere, chain: chain.id)
        }
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
        // Existing copper is never ripped up, whatever asks (plan-66-C).
        let ids = ids.filter { entries[$0]?.chain == nil }
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

    /// Net `net`'s entries, in the order they were made.
    func ledger(of net: Int) -> [LedgerEntry] { (entriesOfNet[net] ?? []).compactMap { entries[$0] } }

    /// How far any copper's stamp can reach past its core, mm.
    var reach: Double {
        let largest = (problem.nets + problem.bystanders)
            .map { max($0.clearance, problem.rules.minClearance) + max($0.width, $0.viaDiameter) / 2 }.max() ?? 0
        let profiles = grid.profiles.map { max($0.clearance, problem.rules.minClearance) + max($0.halfWidth, $0.viaRadius) }
            .max() ?? 0
        // A via's hole keeps the hole-to-hole distance from other holes.
        let drill = (problem.nets.map(\.viaDrill).max() ?? 0) / 2 + (grid.profiles.map(\.viaDrillRadius).max() ?? 0)
        return max(largest + profiles, drill + problem.rules.minHoleToHole) + grid.pitch
    }

    // MARK: Entries

    /// Records and stamps a connection's copper; returns its id.
    @discardableResult
    mutating func add(net: Int, path: [Int], traces: [RoutedTrace], vias: [RoutedVia], allowed: [Bool],
                      chain: Int? = nil, touched: Bool = false) -> Int {
        let box = copperBox(path: path, traces: traces, vias: vias)
        var nodes = path
        for via in vias {
            let c = grid.nearestCell(via.centre)
            nodes += grid.layers.indices.filter { allowed[$0] }.map { $0 * grid.cellCount + c }
        }
        var entry = LedgerEntry(id: nextID, net: net, path: path, traces: traces, vias: vias, box: box, nodes: nodes)
        entry.chain = chain
        entry.touched = touched
        nextID += 1
        entries[entry.id] = entry
        entriesOfNet[net, default: []].append(entry.id)
        for tile in tileKeys(box) { tiles[tile, default: []].append(entry.id) }
        stamp(entry)
        return entry.id
    }

    /// The box of centrelines and via centres (a lone path cell when there is no copper).
    private func copperBox(path: [Int], traces: [RoutedTrace], vias: [RoutedVia]) -> CGRect {
        var points: [CGPoint] = traces.flatMap(\.points) + vias.map(\.centre)
        if points.isEmpty, let first = path.first { points = [grid.point(first % grid.cellCount)] }
        let xs = points.map(\.x), ys = points.map(\.y)
        return CGRect(x: xs.min() ?? 0, y: ys.min() ?? 0, width: (xs.max() ?? 0) - (xs.min() ?? 0),
                      height: (ys.max() ?? 0) - (ys.min() ?? 0))
    }

    private mutating func stamp(_ entry: LedgerEntry) {
        let bystander = entry.chain != nil
        for trace in entry.traces {
            guard let layer = grid.layerIndex(trace.layer) else { continue }
            grid.stampTrace(trace.points, width: trace.width, layer: layer, net: entry.net, bystander: bystander)
        }
        for via in entry.vias {
            grid.stampVia(via.centre, diameter: via.diameter, drill: via.drill, net: entry.net, bystander: bystander)
        }
    }

    /// Swaps an entry's copper for other copper exactly (un-stamp, stamp),
    /// keeping its id, net, path, nodes and chain (plan-66-C). The entry
    /// stays where it was in its net's order.
    mutating func replace(_ id: Int, traces: [RoutedTrace], vias: [RoutedVia], touched: Bool) {
        guard var entry = entries[id] else { return }
        let position = entriesOfNet[entry.net]?.firstIndex(of: id)
        remove([id])
        entry.traces = traces
        entry.vias = vias
        entry.box = copperBox(path: entry.path, traces: traces, vias: vias)
        entry.touched = touched
        entries[id] = entry
        if let position, position <= entriesOfNet[entry.net]?.count ?? 0 {
            entriesOfNet[entry.net]?.insert(id, at: position)
        } else {
            entriesOfNet[entry.net, default: []].append(id)
        }
        for tile in tileKeys(entry.box) { tiles[tile, default: []].append(id) }
        stamp(entry)
    }

    /// Removes entries and their copper from the grid exactly.
    mutating func remove(_ ids: [Int]) {
        let removed = ids.sorted().compactMap { entries[$0] }
        guard !removed.isEmpty else { return }
        for entry in removed {
            entries[entry.id] = nil
            entriesOfNet[entry.net]?.removeAll { $0 == entry.id }
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
        guard let terminals = terminals[net], let componentOfNode = componentOfNode[net] else { return ([], []) }
        let mine = ledger(of: net)
        let n = terminals.count
        var parent = Array(0..<(n + mine.count))
        func find(_ i: Int) -> Int {
            var i = i
            while parent[i] != i { parent[i] = parent[parent[i]]; i = parent[i] }
            return i
        }
        func union(_ a: Int, _ b: Int) { let (x, y) = (find(a), find(b)); if x != y { parent[max(x, y)] = min(x, y) } }
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
        guard let terminalNodes = componentOfNode[net] else { return [] }
        let mine = ledger(of: net)
        return mine.filter { entry in
            [entry.path.first, entry.path.last].compactMap { $0 }.contains { end in
                terminalNodes[end] == nil && !mine.contains { $0.id != entry.id && $0.nodes.contains(end) }
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

nonisolated extension RoutingGrid {
    /// Puts the fixed copper's values back over the cells inside `box` on
    /// every map.
    mutating func restoreFixed(in box: CGRect) {
        guard let range = cellRange(box, margin: 0) else { return }
        clearMasks(in: range)
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
