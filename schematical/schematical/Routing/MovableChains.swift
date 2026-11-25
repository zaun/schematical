import CoreGraphics
import Foundation

extension SchematicDocument {
    /// The existing copper push and shove may move (plan-66-B § 3): each net's
    /// unlocked traces and vias cut into chains, a chain being the stretch
    /// from one anchor to the next. Anchors are the points where copper
    /// must stay put: a pad touches it, three or more trace ends meet, it is
    /// a free end or the meeting of two traces, or it is a via with other than
    /// two ends. Bends and two-ended vias are inside a chain.
    ///
    /// A chain is left out (its copper stays an obstacle) when it is locked,
    /// has two widths or via sizes, lies on a layer the run leaves out, is
    /// touched by other copper away from its anchors, has no two anchors, or
    /// differs in width or via size from the net's first chain. `routed` are
    /// the nets that are never chained: the nets the run routes, and nets
    /// that cannot be hosted. Chains come in the order of their first segment.
    func movableChains(in copper: BoardConnectivity, netOfRoot: [Int: Int], excluding routed: Set<Int>,
                       routable: [CopperLayer], grid: Double) -> [MovableChain] {
        var finder = ChainFinder(copper: copper, netOfRoot: netOfRoot, routed: routed, routable: Set(routable), grid: grid)
        return finder.chains()
    }
}

/// A trace segment: trace index in the connectivity, segment index in it.
private nonisolated struct SegmentID: Hashable, Comparable {
    let trace: Int
    let index: Int

    static func < (a: SegmentID, b: SegmentID) -> Bool {
        (a.trace, a.index) < (b.trace, b.index)
    }
}

/// Which cell a point or box falls in; a hash of cells, for "what is near here".
private nonisolated struct CellIndex {
    private let size = 2.0
    private var cells: [Int: [Int]] = [:]

    private func key(_ cx: Int, _ cy: Int) -> Int { (cx + 100_000) * 200_003 + (cy + 100_000) }

    private func range(_ box: CGRect) -> (x: ClosedRange<Int>, y: ClosedRange<Int>) {
        (Int((box.minX / size).rounded(.down))...Int((box.maxX / size).rounded(.down)),
         Int((box.minY / size).rounded(.down))...Int((box.maxY / size).rounded(.down)))
    }

    mutating func insert(_ item: Int, box: CGRect) {
        let r = range(box)
        for cx in r.x { for cy in r.y { cells[key(cx, cy), default: []].append(item) } }
    }

    /// Items whose box shares a cell with `box`, once each, ascending.
    func near(_ box: CGRect) -> [Int] {
        let r = range(box)
        var found = Set<Int>()
        for cx in r.x { for cy in r.y { found.formUnion(cells[key(cx, cy)] ?? []) } }
        return found.sorted()
    }
}

private struct ChainFinder {
    let copper: BoardConnectivity
    let netOfRoot: [Int: Int]
    let routed: Set<Int>
    let routable: Set<CopperLayer>
    let grid: Double

    private struct Key: Hashable {
        let layer: CopperLayer
        let x: Int
        let y: Int
    }

    private struct PointKey: Hashable {
        let x: Int
        let y: Int
    }

    private enum Piece {
        case segment(SegmentID)
        case via(Int)
    }

    private struct Edge {
        let piece: Piece
        let a: Int
        let b: Int
    }

    private enum Kind {
        case interior
        /// One end of a clean via: the edge to the other end is `via`.
        case viaVertex
        case anchor
    }

    // The graph.
    private var nodeOf: [Key: Int] = [:]
    private var nodeLayer: [CopperLayer] = []
    private var nodePoint: [CGPoint] = []
    /// Each node's occurrences: (trace index, vertex index).
    private var occurrences: [[(trace: Int, vertex: Int)]] = []
    private var edges: [Edge] = []
    private var incident: [[Int]] = []
    private var kind: [Kind] = []
    /// What touches each anchor: pads, else a via, else nothing but copper.
    private var anchorPads: [Int: [Int]] = [:]
    private var anchorVia: [Int: Int] = [:]
    private var contaminated = Set<SegmentID>()
    private var viaAt: [PointKey: [Int]] = [:]
    private var padIndex = CellIndex()
    private var segmentIndex = CellIndex()
    private var segmentList: [SegmentID] = []

    init(copper: BoardConnectivity, netOfRoot: [Int: Int], routed: Set<Int>, routable: Set<CopperLayer>, grid: Double) {
        self.copper = copper
        self.netOfRoot = netOfRoot
        self.routed = routed
        self.routable = routable
        self.grid = grid
    }

    private func key(_ layer: CopperLayer, _ p: CGPoint) -> Key {
        Key(layer: layer, x: Int((p.x * 1e4).rounded()), y: Int((p.y * 1e4).rounded()))
    }

    private func pointKey(_ p: CGPoint) -> PointKey {
        PointKey(x: Int((p.x * 1e4).rounded()), y: Int((p.y * 1e4).rounded()))
    }

    /// Whether pad `p` is copper on `layer` (a plated through-hole pad is on every layer).
    private func padIsOn(_ p: Int, _ layer: CopperLayer) -> Bool {
        let pad = copper.pads[p]
        return pad.pad.plated && (pad.pad.drill > 0 || pad.side.copperLayer == layer)
    }

    private func padLayers(_ p: Int) -> Set<CopperLayer>? {
        copper.pads[p].pad.drill > 0 ? nil : [copper.pads[p].side.copperLayer]
    }

    private func netOfTrace(_ t: Int) -> Int? { netOfRoot[copper.root[copper.traceNode(t)]] }

    // MARK: Building

    private mutating func node(_ layer: CopperLayer, _ p: CGPoint) -> Int {
        let k = key(layer, p)
        if let n = nodeOf[k] { return n }
        let n = nodeLayer.count
        nodeOf[k] = n
        nodeLayer.append(layer)
        nodePoint.append(p)
        occurrences.append([])
        incident.append([])
        return n
    }

    private mutating func addEdge(_ piece: Piece, _ a: Int, _ b: Int) {
        edges.append(Edge(piece: piece, a: a, b: b))
        incident[a].append(edges.count - 1)
        incident[b].append(edges.count - 1)
    }

    private mutating func index() {
        for (p, pad) in copper.pads.enumerated() where pad.pad.plated {
            padIndex.insert(p, box: pad.pad.rect.insetBy(dx: -1e-3, dy: -1e-3))
        }
        for (v, via) in copper.vias.enumerated() { viaAt[pointKey(via.centre), default: []].append(v) }
        var firstSegmentNode: [Int] = []
        for (t, trace) in copper.traces.enumerated() {
            var nodes: [Int] = []
            for (i, p) in trace.points.enumerated() {
                let n = node(trace.layer, p)
                occurrences[n].append((t, i))
                nodes.append(n)
            }
            firstSegmentNode.append(nodes.first ?? -1)
            for s in 0..<(trace.points.count - 1) {
                let id = SegmentID(trace: t, index: s)
                addEdge(.segment(id), nodes[s], nodes[s + 1])
                segmentList.append(id)
                let a = trace.points[s], b = trace.points[s + 1]
                segmentIndex.insert(segmentList.count - 1, box: CGRect(x: min(a.x, b.x), y: min(a.y, b.y),
                                                                       width: abs(a.x - b.x), height: abs(a.y - b.y))
                                        .insetBy(dx: -(trace.widthMM / 2 + 1e-3), dy: -(trace.widthMM / 2 + 1e-3)))
            }
        }
    }

    private func pads(touching point: CGPoint, on layer: CopperLayer) -> [Int] {
        padIndex.near(CGRect(x: point.x, y: point.y, width: 0, height: 0).insetBy(dx: -1e-3, dy: -1e-3))
            .filter { padIsOn($0, layer) && SchematicDocument.point(point, touches: copper.pads[$0].pad) }
    }

    private func segment(_ id: SegmentID) -> (CGPoint, CGPoint) {
        let points = copper.traces[id.trace].points
        return (points[id.index], points[id.index + 1])
    }

    /// Marks the segments other copper touches away from a vertex of their own,
    /// and finds each via that is clean (two ends, nothing else on its metal).
    private mutating func scanContacts() -> [Int: (Int, Int)] {
        // Pads touching a segment where neither of its ends is in the pad.
        for id in segmentList {
            let (a, b) = segment(id)
            let layer = copper.traces[id.trace].layer
            let box = CGRect(x: min(a.x, b.x), y: min(a.y, b.y), width: abs(a.x - b.x), height: abs(a.y - b.y))
            for p in padIndex.near(box.insetBy(dx: -1e-3, dy: -1e-3)) where padIsOn(p, layer) {
                let pad = copper.pads[p].pad
                guard SchematicDocument.segment(a, b, touches: pad) else { continue }
                if !SchematicDocument.point(a, touches: pad), !SchematicDocument.point(b, touches: pad) {
                    contaminated.insert(id)
                }
            }
        }
        // A vertex of one trace on the middle of another's segment (or its own).
        for (t, trace) in copper.traces.enumerated() {
            for (i, p) in trace.points.enumerated() {
                let box = CGRect(x: p.x, y: p.y, width: 0, height: 0).insetBy(dx: -2e-3, dy: -2e-3)
                for s in segmentIndex.near(box) {
                    let id = segmentList[s]
                    let other = copper.traces[id.trace]
                    guard other.layer == trace.layer, !(id.trace == t && (id.index == i || id.index == i - 1)) else { continue }
                    let (a, b) = segment(id)
                    if SchematicDocument.distance(from: p, toSegment: a, b) < 1e-4,
                       !TraceRoute.same(p, a), !TraceRoute.same(p, b) {
                        contaminated.insert(id)
                    }
                }
            }
        }
        // Vias: the metal's contacts.
        let widest = copper.traces.map(\.widthMM).max() ?? 0
        var clean: [Int: (Int, Int)] = [:]
        for (v, via) in copper.vias.enumerated() {
            let r = via.diameter / 2
            let metal = CGRect(x: via.centre.x, y: via.centre.y, width: 0, height: 0)
                .insetBy(dx: -(r + widest / 2 + 1e-3), dy: -(r + widest / 2 + 1e-3))
            var touching: [SegmentID] = []
            for s in segmentIndex.near(metal) {
                let id = segmentList[s]
                let (a, b) = segment(id)
                if SchematicDocument.distance(from: via.centre, toSegment: a, b)
                    <= r + copper.traces[id.trace].widthMM / 2 { touching.append(id) }
            }
            // The segments with an end at the centre are the via's ends.
            var ended: [SegmentID] = []
            for id in touching {
                let (a, b) = segment(id)
                if TraceRoute.same(a, via.centre) || TraceRoute.same(b, via.centre) { ended.append(id) } else { contaminated.insert(id) }
            }
            let padHit = padIndex.near(metal).contains { SchematicDocument.circle(via.centre, r, meets: copper.pads[$0].pad) }
            // Its ends: the nodes at the centre, on any layer.
            let layers = Set(copper.traces.map(\.layer))
            let atCentre = layers.compactMap { nodeOf[key($0, via.centre)] }.sorted()
            guard atCentre.count == 2, !padHit, (viaAt[pointKey(via.centre)] ?? []).count == 1,
                  atCentre.allSatisfy({ occurrences[$0].count == 1 && (occurrences[$0][0].vertex == 0
                      || occurrences[$0][0].vertex == copper.traces[occurrences[$0][0].trace].points.count - 1) }),
                  ended.count == 2 else { continue }
            clean[v] = (atCentre[0], atCentre[1])
        }
        return clean
    }

    private mutating func classify(clean: [Int: (Int, Int)]) {
        var viaOfNode: [Int: Int] = [:]
        for (v, ends) in clean {
            viaOfNode[ends.0] = v
            viaOfNode[ends.1] = v
        }
        kind = Array(repeating: .anchor, count: nodeLayer.count)
        for n in nodeLayer.indices {
            let touching = pads(touching: nodePoint[n], on: nodeLayer[n])
            if !touching.isEmpty {
                anchorPads[n] = touching
            } else if let v = viaOfNode[n] {
                kind[n] = .viaVertex
                if n == clean[v]!.0 { addEdge(.via(v), clean[v]!.0, clean[v]!.1) }
            } else if let v = viaAt[pointKey(nodePoint[n])]?.first {
                anchorVia[n] = v
            } else if occurrences[n].count == 1, let o = occurrences[n].first,
                      o.vertex > 0, o.vertex < copper.traces[o.trace].points.count - 1 {
                kind[n] = .interior
            }
        }
    }

    // MARK: Walking

    private struct Path {
        var nodes: [Int]
        var edges: [Int]
    }

    private func walk() -> [Path] {
        var seen = Set<Int>()
        var paths: [Path] = []
        for start in nodeLayer.indices where kind[start] == .anchor {
            for first in incident[start] where !seen.contains(first) {
                var path = Path(nodes: [start], edges: [])
                var edge = first, from = start
                while true {
                    seen.insert(edge)
                    let e = edges[edge]
                    let to = e.a == from ? e.b : e.a
                    path.edges.append(edge)
                    path.nodes.append(to)
                    if kind[to] == .anchor { break }
                    guard let next = incident[to].first(where: { $0 != edge }) else { break }
                    edge = next
                    from = to
                }
                paths.append(path)
            }
        }
        return paths
    }

    private func segmentKey(_ edge: Int) -> SegmentID? {
        if case let .segment(id) = edges[edge].piece { return id }
        return nil
    }

    // MARK: Chains

    mutating func chains() -> [MovableChain] {
        index()
        let clean = scanContacts()
        classify(clean: clean)
        // Each path oriented so its first segment is the lower one, then in that order.
        var keyed: [(first: SegmentID, path: Path)] = []
        for path in walk() {
            let segments = path.edges.compactMap(segmentKey)
            guard let first = segments.first, let last = segments.last else { continue }
            let trace = copper.traces[first.trace]
            let lowEnd = nodeOf[key(trace.layer, trace.points[first.index])]
            if last < first || (last == first && path.nodes.first != lowEnd) {
                keyed.append((last, Path(nodes: path.nodes.reversed(), edges: path.edges.reversed())))
            } else {
                keyed.append((first, path))
            }
        }
        keyed.sort { $0.first < $1.first }

        var result: [MovableChain] = []
        // Each net's profile: the first kept chain's width, and via size once one has a via.
        var width: [Int: Double] = [:], viaSize: [Int: (drill: Double, diameter: Double)] = [:]
        for (_, path) in keyed {
            guard let chain = build(path, id: result.count) else { continue }
            if let w = width[chain.net], abs(w - chain.width) > 1e-9 { continue }
            if !chain.vias.isEmpty, let size = viaSize[chain.net],
               abs(size.drill - chain.viaDrill) > 1e-9 || abs(size.diameter - chain.viaDiameter) > 1e-9 { continue }
            width[chain.net] = chain.width
            if !chain.vias.isEmpty, viaSize[chain.net] == nil { viaSize[chain.net] = (chain.viaDrill, chain.viaDiameter) }
            result.append(chain)
        }
        return result
    }

    /// The chain a path makes, or nil when a rule leaves its copper where it is.
    private func build(_ path: Path, id: Int) -> MovableChain? {
        var runs: [MovableChain.Run] = []
        var runTraces: [Int] = []
        var vias: [MovableChain.Via] = []
        var viaIndexes: [Int] = []
        var open = false
        for (k, edge) in path.edges.enumerated() {
            switch edges[edge].piece {
            case let .segment(s):
                let trace = copper.traces[s.trace]
                // Rule 4: other copper touches it away from its anchors.
                if contaminated.contains(s) { return nil }
                let forward = nodeOf[key(trace.layer, trace.points[s.index])] == path.nodes[k]
                if open, runTraces[runTraces.count - 1] == s.trace {
                    if forward { runs[runs.count - 1].last = s.index + 1 } else { runs[runs.count - 1].first = s.index }
                } else {
                    runs.append(MovableChain.Run(trace: trace.id, layer: trace.layer, first: s.index, last: s.index + 1,
                                                 reversed: !forward, points: []))
                    runTraces.append(s.trace)
                    open = true
                }
            case let .via(v):
                vias.append(MovableChain.Via(id: copper.vias[v].id, centre: copper.vias[v].centre))
                viaIndexes.append(v)
                open = false
            }
        }
        // Rule 5: two distinct anchors; and one run per trace between vias.
        guard !runs.isEmpty, runs.count == vias.count + 1, path.nodes.first! != path.nodes.last! else { return nil }
        // One net for every trace and via, and not one the run keeps.
        var net: Int?
        for t in runTraces {
            guard let n = netOfTrace(t) else { return nil }
            if let net, net != n { return nil }
            net = n
        }
        guard let net, !routed.contains(net) else { return nil }
        for v in viaIndexes where netOfRoot[copper.root[copper.viaNode(v)]] != net { return nil }
        // Rule 1: locked.
        if runTraces.contains(where: { copper.traces[$0].locked }) { return nil }
        if viaIndexes.contains(where: { copper.vias[$0].locked }) { return nil }
        // Rule 2: one width, one via size.
        let widths = Set(runTraces.map { copper.traces[$0].width })
        guard widths.count == 1 else { return nil }
        let drills = Set(viaIndexes.map { copper.vias[$0].drill }), diameters = Set(viaIndexes.map { copper.vias[$0].diameter })
        guard drills.count <= 1, diameters.count <= 1 else { return nil }
        // Rule 3: only layers the run enables.
        guard runs.allSatisfy({ routable.contains($0.layer) }) else { return nil }
        // The two ends.
        guard let a = end(at: path.nodes.first!, net: net, layer: runs.first!.layer),
              let b = end(at: path.nodes.last!, net: net, layer: runs.last!.layer) else { return nil }
        for k in runs.indices {
            let points = Array(copper.traces[runTraces[k]].points[runs[k].first...runs[k].last])
            runs[k].points = runs[k].reversed ? points.reversed() : points
        }
        return MovableChain(id: id, net: net, width: Double(widths.first!) * 0.0254,
                            viaDrill: drills.first ?? 0, viaDiameter: diameters.first ?? 0,
                            runs: runs, vias: vias, ends: [a, b])
    }

    private func end(at n: Int, net: Int, layer: CopperLayer) -> MovableChain.End? {
        var anchor = RoutingComponent()
        if let pads = anchorPads[n] {
            guard pads.allSatisfy({ copper.netOfPad[$0] == net }) else { return nil }
            for p in pads {
                anchor.pads.append(CopperGeometry.shape(of: copper.pads[p].pad, layers: padLayers(p), node: p))
                anchor.padLabels.append(copper.pads[p].label)
            }
        } else if let v = anchorVia[n] {
            guard netOfRoot[copper.root[copper.viaNode(v)]] == net else { return nil }
            anchor.vias.append(RoutedVia(centre: copper.vias[v].centre, drill: copper.vias[v].drill, diameter: copper.vias[v].diameter))
        } else {
            let p = nodePoint[n]
            anchor.pads.append(CopperShape(core: .segment(p, p), radius: grid, layers: [nodeLayer[n]], node: -2))
            anchor.padLabels.append("junction")
        }
        return MovableChain.End(point: nodePoint[n], layer: layer, anchor: anchor)
    }
}
