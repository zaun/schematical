//
//  DesignRuleCheck.swift
//  schematical
//
//  The PCB's design rule check: everything that keeps the board from being
//  made or from working as the schematic says, against the board's own
//  design rules (`BoardSettings.rules`, saved in its file). Connections first
//  (shorts, unrouted nets, copper that goes nowhere), then placement, then
//  each rule. Lengths are board mm.
//

import AppKit
import CoreGraphics
import Foundation

/// One problem the check found.
nonisolated struct DesignRuleIssue: Identifiable, Hashable, Sendable {
    /// What kind of problem: the order the list groups them in.
    enum Kind: Int, Comparable, Sendable {
        case short, unrouted, unconnected, placement, layer, layerRule, clearance, edge,
             drill, annularRing, holeToHole, via, traceWidth, silkscreen, solderMask, lengthMatch, diffPair, pour

        static func < (a: Kind, b: Kind) -> Bool { a.rawValue < b.rawValue }
    }

    /// Its line number, from 1.
    var index = 0
    let kind: Kind
    /// The part, pad, trace, via or text with the problem ("U1", "U1.3",
    /// "Trace 2", "Via 1"), or several of them.
    let reference: String
    /// What is wrong.
    let issue: String
    /// What selecting the line selects on the PCB.
    let items: Set<BoardItem>

    var id: Int { index }
}

extension SchematicDocument {
    /// Every problem on the board under `rules`, numbered from 1.
    func designRuleCheck(rules: DesignRules) -> [DesignRuleIssue] {
        var checker = DesignRuleChecker(document: self, rules: rules)
        return checker.run()
    }
}

/// One run of the check. Built on the main actor (it reads the document),
/// run once.
@MainActor
private struct DesignRuleChecker {
    let rules: DesignRules
    let chips: [PlacedChip]
    let board: BoardSettings
    let texts: [BoardText]
    let pours: [BoardPour]
    let copper: BoardConnectivity
    let layers: [CopperLayer]
    let outline: [(CGPoint, CGPoint)]
    let outlinePoints: [CGPoint]
    /// The layers each net with an Only layer rule may use, by net.
    let onlyLayers: [Int: EffectiveLayers]
    /// The clearance each net's class asks for, by net; a net in no class
    /// (or one with no clearance of its own) is not here and keeps the
    /// board's minimum.
    let classClearance: [Int: Double]
    var issues: [DesignRuleIssue] = []

    /// mm: differences smaller than this are rounding, not a problem.
    static let tolerance = 1e-4
    /// The width the PCB page strokes a footprint's outline at.
    static let outlineWidth = 0.15
    /// The pin-1 mark's diameter.
    static let pin1Diameter = 0.4
    /// A board text's strokes, as a share of its size (the system font at
    /// medium weight).
    static let textStrokeShare = 0.1

    init(document: SchematicDocument, rules: DesignRules) {
        self.rules = rules
        chips = document.chips
        board = document.board
        texts = document.texts
        pours = document.pours
        let copper = document.connectivity()
        self.copper = copper
        layers = board.copperLayers
        outlinePoints = board.outline
        outline = Array(zip(outlinePoints, outlinePoints.dropFirst() + outlinePoints.prefix(1)))
        var onlyLayers: [Int: EffectiveLayers] = [:]
        if !board.netOverrides.isEmpty || board.netClasses.contains(where: { $0.layerRule != nil }) {
            for net in Set(copper.netOfPad.values) {
                let effective = document.effectiveLayers(ofNet: net, in: copper)
                if effective.rule?.mode == .only { onlyLayers[net] = effective }
            }
        }
        self.onlyLayers = onlyLayers
        var classClearance: [Int: Double] = [:]
        if board.netClasses.contains(where: { $0.clearance != nil }) {
            for net in Set(copper.netOfPad.values) {
                if let clearance = document.netClass(ofNet: net, in: copper)?.clearance, clearance > rules.minClearance {
                    classClearance[net] = clearance
                }
            }
        }
        self.classClearance = classClearance
    }

    /// The gap two copper groups must keep: the larger of their nets'
    /// class clearances, never under the board's minimum (as the router and
    /// `RoutingVerifier` hold it).
    private func requiredClearance(_ rootA: Int, _ rootB: Int, netOfRoot: [Int: Int]) -> Double {
        max(rules.minClearance, netOfRoot[rootA].flatMap { classClearance[$0] } ?? 0,
            netOfRoot[rootB].flatMap { classClearance[$0] } ?? 0)
    }

    mutating func run() -> [DesignRuleIssue] {
        checkConnections()
        checkPlacement()
        checkLayers()
        checkLayerRules()
        checkClearance()
        checkEdge()
        checkNonPlatedHoles()
        checkHoles()
        checkVias()
        checkTraceWidths()
        checkSilkscreen()
        checkSolderMask()
        checkLengthMatch()
        checkDiffPairs()
        checkPours()
        // Stable: within a kind, the order each check found them in.
        var sorted = issues.enumerated().sorted { a, b in
            a.element.kind != b.element.kind ? a.element.kind < b.element.kind : a.offset < b.offset
        }.map(\.element)
        for i in sorted.indices { sorted[i].index = i + 1 }
        return sorted
    }

    private mutating func add(_ kind: DesignRuleIssue.Kind, _ reference: String, _ issue: String,
                              _ items: Set<BoardItem> = []) {
        issues.append(DesignRuleIssue(kind: kind, reference: reference, issue: issue, items: items))
    }

    // MARK: Names

    private func mm(_ value: Double) -> String {
        LengthField.format(max(0, value), units: .mm, precise: true) + " mm"
    }

    private func traceName(_ index: Int) -> String { "Trace \(index + 1)" }
    private func viaName(_ index: Int) -> String { "Via \(index + 1)" }

    private func textName(_ text: BoardText) -> String {
        let line = text.text.split(whereSeparator: \.isNewline).first.map(String.init) ?? text.text
        return "Text “\(line.count > 24 ? line.prefix(23) + "…" : line)”"
    }

    /// What selecting a pad selects: its footprint, or the drilled hole it
    /// is (an edge finger or board hole, nothing).
    private func item(ofPad pad: Int) -> Set<BoardItem> {
        if let hole = copper.pads[pad].hole { return [.hole(hole)] }
        let chip = copper.pads[pad].chip
        return chip >= 0 ? [.footprint(chips[chip].id)] : []
    }

    /// A node's name and what selecting it selects.
    private func describe(node: Int) -> (name: String, items: Set<BoardItem>) {
        if node < copper.pads.count {
            return (copper.pads[node].label, item(ofPad: node))
        }
        let t = node - copper.pads.count
        if t < copper.traces.count {
            return (traceName(t), [.trace(copper.traces[t].id)])
        }
        let v = t - copper.traces.count
        return (viaName(v), [.via(copper.vias[v].id)])
    }

    private func list(_ names: [String], limit: Int = 4) -> String {
        names.count <= limit ? names.joined(separator: ", ")
            : names.prefix(limit).joined(separator: ", ") + " +\(names.count - limit) more"
    }

    // MARK: Connections

    private mutating func checkConnections() {
        // Group every node by its copper.
        var padsOfRoot: [Int: [Int]] = [:]
        for p in copper.pads.indices { padsOfRoot[copper.root[p], default: []].append(p) }
        var nodesOfRoot: [Int: [Int]] = [:]
        for node in copper.root.indices { nodesOfRoot[copper.root[node], default: []].append(node) }

        // Shorts: copper that joins pads of different nets, or a pad of a
        // net to a pin the schematic leaves unwired.
        for root in padsOfRoot.keys.sorted() {
            let pads = padsOfRoot[root]!
            let nets = Set(pads.compactMap { copper.netOfPad[$0] })
            let copperItems = Set(nodesOfRoot[root]!.filter { $0 >= copper.pads.count }
                .flatMap { describe(node: $0).items })
            // Pads of one pin are joined in the footprint; only copper the
            // user drew can short anything.
            guard !copperItems.isEmpty || Set(pads.map { copper.pads[$0].chip }).count > 1 else { continue }
            let items = copperItems.union(pads.flatMap { item(ofPad: $0) })
            if nets.count > 1 {
                // One pad of each net.
                var seen = Set<Int>(), names: [String] = []
                for p in pads { if let net = copper.netOfPad[p], seen.insert(net).inserted { names.append(copper.pads[p].label) } }
                add(.short, list(names), "Short circuit: copper joins \(nets.count) different nets", items)
            } else if !nets.isEmpty {
                let loose = pads.filter { copper.netOfPad[$0] == nil }.map { copper.pads[$0].label }
                guard !loose.isEmpty else { continue }
                let netPad = pads.first { copper.netOfPad[$0] != nil }.map { copper.pads[$0].label } ?? ""
                // A plated hole has no schematic wire: its net is set on it.
                let onlyHoles = pads.allSatisfy { copper.netOfPad[$0] != nil || copper.pads[$0].hole != nil }
                add(.short, list(loose),
                    onlyHoles ? "Joined to the net of \(netPad), but the hole's net isn't set to it"
                        : "Joined to the net of \(netPad), but not wired to anything in the schematic", items)
            }
        }

        // Unrouted: each net's copper groups, joined by the shortest lines
        // (the ratsnest), one line each.
        for (_, groups) in copper.netGroups() {
            let points = groups.map { $0.pads.map { copper.pads[$0].centre } }
            for (a, b) in Self.spanningPairs(points) {
                let pa = groups[a.group].pads[a.point], pb = groups[b.group].pads[b.point]
                add(.unrouted, "\(copper.pads[pa].label) – \(copper.pads[pb].label)",
                    "Unrouted: no copper joins these pads", item(ofPad: pa).union(item(ofPad: pb)))
            }
        }

        // Copper that goes nowhere.
        for (t, trace) in copper.traces.enumerated() {
            let node = copper.traceNode(t)
            let pads = padsOfRoot[copper.root[node]] ?? []
            if pads.isEmpty {
                add(.unconnected, traceName(t), "Not connected to any pad", [.trace(trace.id)])
                continue
            }
            if pads.allSatisfy({ copper.netOfPad[$0] == nil }) {
                add(.unconnected, traceName(t),
                    "Only joins pins that aren't wired in the schematic (\(list(pads.map { copper.pads[$0].label })))",
                    [.trace(trace.id)])
            }
            for (end, point) in [("Start", trace.points.first!), ("End", trace.points.last!)]
            where !endIsConnected(point, of: t) {
                add(.unconnected, traceName(t),
                    "\(end) at (\(mm(point.x)), \(mm(point.y))) is not connected to anything", [.trace(trace.id)])
            }
        }
        for (v, via) in copper.vias.enumerated() {
            let touching = touchingCount(via: v)
            if touching == 0 {
                add(.unconnected, viaName(v), "Not connected to anything", [.via(via.id)])
            } else if touching == 1 {
                add(.unconnected, viaName(v), "Goes nowhere: only one trace or pad touches it", [.via(via.id)])
            } else if (padsOfRoot[copper.root[copper.viaNode(v)]] ?? []).isEmpty {
                add(.unconnected, viaName(v), "Not connected to any pad", [.via(via.id)])
            }
        }
    }

    /// Whether a trace end lands on a pad on its layer, a via, or another
    /// trace on its layer.
    private func endIsConnected(_ point: CGPoint, of index: Int) -> Bool {
        let trace = copper.traces[index]
        let reach = trace.widthMM / 2
        for pad in copper.pads where pad.pad.plated && (pad.pad.drill > 0 || pad.side.copperLayer == trace.layer) {
            if SchematicDocument.point(point, touches: pad.pad) { return true }
        }
        for via in copper.vias where SchematicDocument.distance(from: point, to: via.centre) <= via.diameter / 2 + reach {
            return true
        }
        for (o, other) in copper.traces.enumerated() where o != index && other.layer == trace.layer {
            if other.segments.contains(where: { SchematicDocument.distance(from: point, toSegment: $0.0, $0.1) < 1e-4 }) {
                return true
            }
        }
        return pourContains(point, on: trace.layer)
    }

    /// Whether a kept piece of a pour on `layer` holds `point`.
    private func pourContains(_ point: CGPoint, on layer: CopperLayer) -> Bool {
        pours.contains { pour in
            pour.layer == layer && (copper.pourFills[pour.id]?.pieces.contains { $0.contains(point, using: .winding) } ?? false)
        }
    }

    /// How many traces and pads touch a via's metal.
    private func touchingCount(via index: Int) -> Int {
        let via = copper.vias[index]
        let traces = copper.traces.filter { trace in
            trace.segments.contains {
                SchematicDocument.distance(from: via.centre, toSegment: $0.0, $0.1) <= via.diameter / 2 + trace.widthMM / 2
            }
        }.count
        let pads = copper.pads.filter { SchematicDocument.circle(via.centre, via.diameter / 2, meets: $0.pad) }.count
        // A pour that joins it counts as one more thing touching it.
        let poured = copper.pourFills.values.contains { $0.joined.contains { $0.contains(.via(via.id)) } } ? 1 : 0
        return traces + pads + poured
    }

    /// Prim's tree over groups of points, as the ratsnest draws it: each
    /// pair is (group, point in it) at both ends.
    static func spanningPairs(_ groups: [[CGPoint]]) -> [((group: Int, point: Int), (group: Int, point: Int))] {
        guard groups.count > 1 else { return [] }
        var inTree = [0]
        var rest = Array(groups.indices.dropFirst())
        var pairs: [((group: Int, point: Int), (group: Int, point: Int))] = []
        while !rest.isEmpty {
            var best: (rest: Int, a: (Int, Int), b: (Int, Int), d: CGFloat)?
            for (r, g) in rest.enumerated() {
                for t in inTree {
                    for (i, p) in groups[t].enumerated() {
                        for (j, q) in groups[g].enumerated() {
                            let d = hypot(p.x - q.x, p.y - q.y)
                            if d < best?.d ?? .infinity { best = (r, (t, i), (g, j), d) }
                        }
                    }
                }
            }
            guard let best else { break }
            pairs.append(((best.a.0, best.a.1), (best.b.0, best.b.1)))
            inTree.append(rest.remove(at: best.rest))
        }
        return pairs
    }

    // MARK: Placement

    private mutating func checkPlacement() {
        for chip in chips where chip.part.footprint != nil && chip.board == nil {
            add(.placement, chip.reference, "Not placed on the board", [.footprint(chip.id)])
        }
        let placed = chips.filter { $0.board != nil }
        for chip in placed {
            guard let bounds = chip.boardBounds else { continue }
            if !rectIsOnBoard(bounds) {
                add(.placement, chip.reference,
                    rectTouchesBoard(bounds) ? "Hangs off the board's edge" : "Off the board",
                    [.footprint(chip.id)])
            }
        }
        // Bodies on one side that overlap.
        for (i, a) in placed.enumerated() {
            guard let bodyA = a.boardBody?.rect else { continue }
            for b in placed.dropFirst(i + 1) where b.board?.side == a.board?.side {
                guard let bodyB = b.boardBody?.rect else { continue }
                let overlap = bodyA.intersection(bodyB)
                if !overlap.isNull, overlap.width > Self.tolerance, overlap.height > Self.tolerance {
                    add(.placement, "\(a.reference), \(b.reference)",
                        "Bodies overlap on \(a.board!.side.title)", [.footprint(a.id), .footprint(b.id)])
                }
            }
        }
        for text in texts where !rectIsOnBoard(text.boardRect) {
            add(.placement, textName(text), "Off the board", [.text(text.id)])
        }
    }

    private func rectIsOnBoard(_ rect: CGRect) -> Bool {
        let corners = [CGPoint(x: rect.minX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.minY),
                       CGPoint(x: rect.maxX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.maxY)]
        guard corners.allSatisfy(inside) else { return false }
        // A notch can cut into a rectangle whose corners are all inside.
        let inner = rect.insetBy(dx: Self.tolerance, dy: Self.tolerance)
        return !outline.contains { SchematicDocument.segment($0.0, $0.1, touches: inner) }
    }

    private func rectTouchesBoard(_ rect: CGRect) -> Bool {
        inside(CGPoint(x: rect.midX, y: rect.midY))
            || outline.contains { SchematicDocument.segment($0.0, $0.1, touches: rect) }
    }

    /// Whether a point is inside the board outline (even-odd).
    private func inside(_ point: CGPoint) -> Bool {
        var result = false
        for (a, b) in outline where (a.y > point.y) != (b.y > point.y) {
            let x = a.x + (point.y - a.y) / (b.y - a.y) * (b.x - a.x)
            if point.x < x { result.toggle() }
        }
        return result
    }

    // MARK: Layers

    private mutating func checkLayers() {
        for (t, trace) in copper.traces.enumerated() {
            if !layers.contains(trace.layer) {
                add(.layer, traceName(t),
                    "On \(trace.layer.title), which a \(board.layers)-layer board doesn't have", [.trace(trace.id)])
            } else if board.style(of: trace.layer).type == .plane {
                add(.layer, traceName(t), "On \(trace.layer.title), a Plane layer", [.trace(trace.id)])
            }
        }
        if !layers.contains(.bottom) {
            for chip in chips where chip.board?.side == .bottom {
                add(.layer, chip.reference, "On Bottom, but a 1-layer board has only Top", [.footprint(chip.id)])
            }
            for text in texts where text.side == .bottom {
                add(.layer, textName(text), "On Bottom, but a 1-layer board has only Top", [.text(text.id)])
            }
        }
    }

    /// A net held to some layers by an Only rule: one line per net for its
    /// traces on any other layer and, when it is held to one layer, its
    /// vias (a via joins layers). A rule that leaves the net no trace layer
    /// is a line of its own.
    private mutating func checkLayerRules() {
        guard !onlyLayers.isEmpty else { return }
        let netOfRoot = copper.netOfRoot()
        func netOf(_ node: Int) -> Int? { netOfRoot[copper.root[node]] }
        for net in onlyLayers.keys.sorted() {
            guard let effective = onlyLayers[net] else { continue }
            let name = copper.netNames[net] ?? "Net \(net + 1)"
            let traces = copper.traces.indices.filter { netOf(copper.traceNode($0)) == net }
            if effective.allowed.isEmpty {
                add(.layerRule, name, "Layer rule names no routable layer",
                    Set(traces.map { .trace(copper.traces[$0].id) }))
                continue
            }
            let stray = traces.filter { !effective.allowed.contains(copper.traces[$0].layer) }
            let vias = effective.allowed.count == 1
                ? copper.vias.indices.filter { netOf(copper.viaNode($0)) == net } : []
            guard !stray.isEmpty || !vias.isEmpty else { continue }
            let strayLayers = layers.filter { layer in stray.contains { copper.traces[$0].layer == layer } }
            var what: [String] = []
            if !strayLayers.isEmpty { what.append("Copper on " + strayLayers.map(\.title).joined(separator: " and ")) }
            if !vias.isEmpty { what.append(vias.count == 1 ? "a via" : "\(vias.count) vias") }
            var issue = what.joined(separator: " and ")
            issue = issue.prefix(1).uppercased() + issue.dropFirst()
            let limit = effective.allowed.map(\.title).joined(separator: " and ")
            add(.layerRule, name, "\(issue); this net is limited to \(limit)",
                Set(stray.map { .trace(copper.traces[$0].id) } + vias.map { .via(copper.vias[$0].id) }))
        }
    }

    // MARK: Copper shapes

    private func padLayers(_ pad: BoardConnectivity.Pad) -> Set<CopperLayer>? {
        pad.pad.drill > 0 ? nil : [pad.side.copperLayer]
    }

    /// Every piece of copper: pads, trace segments, vias.
    private func copperShapes() -> [CopperShape] {
        // A non-plated hole is no copper.
        var shapes = copper.pads.enumerated().compactMap { p, pad in
            pad.pad.plated ? CopperGeometry.shape(of: pad.pad, layers: padLayers(pad), node: p) : nil
        }
        for (t, trace) in copper.traces.enumerated() {
            for (a, b) in trace.segments {
                shapes.append(CopperShape(core: .segment(a, b), radius: trace.widthMM / 2, layers: [trace.layer],
                                    node: copper.traceNode(t)))
            }
        }
        for (v, via) in copper.vias.enumerated() {
            shapes.append(CopperShape(core: .segment(via.centre, via.centre), radius: via.diameter / 2, layers: nil,
                                node: copper.viaNode(v)))
        }
        return shapes
    }

    // MARK: Clearance

    private mutating func checkClearance() {
        let shapes = copperShapes()
        let netOfRoot = copper.netOfRoot()
        // The closest gap per pair of items (two chips' pads count once).
        struct Key: Hashable { let a: String; let b: String }
        var worst: [Key: (gap: Double, need: Double, a: Int, b: Int)] = [:]
        let margin = max(rules.minClearance, classClearance.values.max() ?? 0)
        CopperGeometry.nearPairs(shapes, margin: margin) { a, b in
            guard a.shares(b) else { return }
            let ra = copper.root[a.node], rb = copper.root[b.node]
            // Joined copper is one conductor; two groups of one net are
            // just unrouted.
            guard ra != rb else { return }
            if let na = netOfRoot[ra], let nb = netOfRoot[rb], na == nb { return }
            let gap = a.gap(to: b)
            let need = requiredClearance(ra, rb, netOfRoot: netOfRoot)
            guard gap < need - Self.tolerance else { return }
            let (first, second) = a.node < b.node ? (a.node, b.node) : (b.node, a.node)
            let key = Key(a: owner(first), b: owner(second))
            if gap < worst[key]?.gap ?? .infinity { worst[key] = (gap, need, first, second) }
        }
        for (key, found) in worst.sorted(by: { ($0.value.a, $0.value.b) < ($1.value.a, $1.value.b) }) {
            let one = describe(node: found.a), two = describe(node: found.b)
            let items = one.items.union(two.items)
            if key.a == key.b {
                add(.clearance, key.a,
                    "Pads \(Self.padName(one.name)) and \(Self.padName(two.name)) are \(gapText(found.gap)) apart; "
                        + "min clearance \(mm(found.need))", items)
            } else if found.gap <= Self.tolerance {
                add(.clearance, "\(one.name), \(two.name)", "Copper touches copper of another net", items)
            } else {
                add(.clearance, "\(one.name), \(two.name)",
                    "Clearance \(mm(found.gap)); min \(mm(found.need))", items)
            }
        }
    }

    private func gapText(_ gap: Double) -> String { gap <= Self.tolerance ? "touching, 0 mm" : mm(gap) }

    /// "U1.3" → "3".
    private static func padName(_ label: String) -> String {
        label.split(separator: ".", maxSplits: 1).last.map(String.init) ?? label
    }

    /// What a node belongs to for grouping: a pad its part, else itself.
    private func owner(_ node: Int) -> String {
        if node < copper.pads.count {
            let chip = copper.pads[node].chip
            // The board's own: "Edge", "Header", or a mounting hole.
            return chip >= 0 ? chips[chip].reference
                : String(copper.pads[node].label.prefix { $0 != "." })
        }
        return describe(node: node).name
    }

    // MARK: Board edge

    private mutating func checkEdge() {
        let edges = outline.map { CopperShape(core: .segment($0.0, $0.1), radius: 0, layers: nil, node: -1) }
        var worst: [String: (gap: Double, node: Int, off: Bool)] = [:]
        let offBoardChips = Set(chips.filter { chip in chip.boardBounds.map { !rectIsOnBoard($0) } ?? false }.map(\.reference))
        for shape in copperShapes() {
            // Fingers belong at the edge; a part already off the board is
            // listed once, under Placement.
            if shape.node < copper.pads.count {
                let chip = copper.pads[shape.node].chip
                if chip < 0 ? copper.pads[shape.node].hole == nil : offBoardChips.contains(chips[chip].reference) {
                    continue
                }
            }
            let gap = edges.map { shape.gap(to: $0) }.min() ?? .infinity
            let centre = CGPoint(x: shape.bounds.midX, y: shape.bounds.midY)
            let off = !inside(centre)
            guard off || gap < rules.minCopperToEdge - Self.tolerance else { continue }
            let key = owner(shape.node)
            // Off the board outranks close to its edge.
            if let found = worst[key], found.off || (!off && found.gap <= gap) { continue }
            worst[key] = (gap, shape.node, off)
        }
        for (key, found) in worst.sorted(by: { $0.value.node < $1.value.node }) {
            let items = describe(node: found.node).items
            let name = found.node < copper.pads.count ? key : describe(node: found.node).name
            if found.off {
                add(.edge, name, "Copper is off the board", items)
            } else {
                add(.edge, name, "Copper \(mm(max(0, found.gap))) from the board edge; min \(mm(rules.minCopperToEdge))",
                    items)
            }
        }
    }

    // MARK: Non-plated holes

    /// Copper over, or too near, a non-plated hole: the drill goes through
    /// it. Held to the copper-to-edge distance, as the planes and pours
    /// keep it round such a hole (`planeClearances`, `PourCopper.hole`) and
    /// the router does. One line per hole and owner of the copper.
    private mutating func checkNonPlatedHoles() {
        let drilled = copper.pads.indices.filter { !copper.pads[$0].pad.plated && copper.pads[$0].pad.holeSegment != nil }
        guard !drilled.isEmpty else { return }
        let drilledSet = Set(drilled)
        var shapes = copperShapes()
        for p in drilled {
            guard let hole = copper.pads[p].pad.holeSegment else { continue }
            shapes.append(CopperShape(core: .segment(hole.a, hole.b), radius: hole.radius, layers: nil, node: p))
        }
        // Copper shapes never have a non-plated pad's node (`copperShapes`).
        struct Key: Hashable { let copper: String; let hole: Int }
        var worst: [Key: (gap: Double, copper: Int, hole: Int)] = [:]
        CopperGeometry.nearPairs(shapes, margin: rules.minCopperToEdge) { a, b in
            let aHole = drilledSet.contains(a.node), bHole = drilledSet.contains(b.node)
            guard aHole != bHole else { return }
            let (hole, metal) = aHole ? (a, b) : (b, a)
            let gap = hole.gap(to: metal)
            guard gap < rules.minCopperToEdge - Self.tolerance else { return }
            let key = Key(copper: owner(metal.node), hole: hole.node)
            if gap < worst[key]?.gap ?? .infinity { worst[key] = (gap, metal.node, hole.node) }
        }
        for (key, found) in worst.sorted(by: { ($0.value.hole, $0.value.copper) < ($1.value.hole, $1.value.copper) }) {
            let metal = describe(node: found.copper), hole = describe(node: found.hole)
            let what = found.copper < copper.pads.count ? "Pad \(Self.padName(metal.name))" : "Copper"
            let issue = found.gap <= Self.tolerance
                ? "\(what) runs over the non-plated hole of \(hole.name)"
                : "\(what) is \(mm(found.gap)) from the non-plated hole of \(hole.name); min \(mm(rules.minCopperToEdge))"
            add(.clearance, key.copper == hole.name ? key.copper : "\(metal.name), \(hole.name)", issue,
                metal.items.union(hole.items))
        }
    }

    // MARK: Holes

    private mutating func checkHoles() {
        // Plated pad holes, per part and size.
        var small: [String: (chip: Int, drill: Double, pads: [String])] = [:]
        var thin: [String: (chip: Int, ring: Double, pads: [String])] = [:]
        for pad in copper.pads where pad.pad.drill > 0 && pad.chip >= 0 {
            let name = Self.padName(pad.label)
            if pad.pad.drill < rules.minDrill - Self.tolerance {
                small["\(pad.chip)/\(pad.pad.drill)", default: (pad.chip, pad.pad.drill, [])].pads.append(name)
            }
            guard pad.pad.plated else { continue }  // a non-plated hole has no ring
            let ring = (min(pad.pad.rect.width, pad.pad.rect.height) - pad.pad.drill) / 2
            if ring < rules.minAnnularRing - Self.tolerance {
                thin["\(pad.chip)/\(ring)", default: (pad.chip, ring, [])].pads.append(name)
            }
        }
        for found in small.values.sorted(by: { ($0.chip, $0.drill) < ($1.chip, $1.drill) }) {
            add(.drill, chips[found.chip].reference,
                "Hole \(mm(found.drill)) (pad\(found.pads.count == 1 ? "" : "s") \(list(found.pads, limit: 6))); "
                    + "min drill \(mm(rules.minDrill))", [.footprint(chips[found.chip].id)])
        }
        for found in thin.values.sorted(by: { ($0.chip, $0.ring) < ($1.chip, $1.ring) }) {
            add(.annularRing, chips[found.chip].reference,
                "Annular ring \(mm(found.ring)) (pad\(found.pads.count == 1 ? "" : "s") \(list(found.pads, limit: 6))); "
                    + "min \(mm(rules.minAnnularRing))", [.footprint(chips[found.chip].id)])
        }

        // Hole to hole: every pad hole and via, edge to edge.
        var holes: [CopperShape] = []
        for (p, pad) in copper.pads.enumerated() {
            // A slot is a capsule: a segment along the pad's long side.
            guard let hole = pad.pad.holeSegment else { continue }
            holes.append(CopperShape(core: .segment(hole.a, hole.b), radius: hole.radius, layers: nil, node: p))
        }
        for (v, via) in copper.vias.enumerated() {
            holes.append(CopperShape(core: .segment(via.centre, via.centre), radius: via.drill / 2, layers: nil,
                               node: copper.viaNode(v)))
        }
        struct Key: Hashable { let a: String; let b: String }
        var worst: [Key: (gap: Double, a: Int, b: Int)] = [:]
        CopperGeometry.nearPairs(holes, margin: rules.minHoleToHole) { a, b in
            let gap = a.gap(to: b)
            guard gap < rules.minHoleToHole - Self.tolerance else { return }
            let (first, second) = a.node < b.node ? (a.node, b.node) : (b.node, a.node)
            let key = Key(a: owner(first), b: owner(second))
            if gap < worst[key]?.gap ?? .infinity { worst[key] = (gap, first, second) }
        }
        for (key, found) in worst.sorted(by: { ($0.value.a, $0.value.b) < ($1.value.a, $1.value.b) }) {
            let one = describe(node: found.a), two = describe(node: found.b)
            let gap = found.gap <= Self.tolerance ? "overlap" : "are \(mm(found.gap)) apart"
            add(.holeToHole, key.a == key.b ? key.a : "\(one.name), \(two.name)",
                key.a == key.b ? "Holes of pads \(Self.padName(one.name)) and \(Self.padName(two.name)) \(gap); "
                    + "min hole to hole \(mm(rules.minHoleToHole))"
                    : "Holes \(gap); min hole to hole \(mm(rules.minHoleToHole))",
                one.items.union(two.items))
        }
    }

    // MARK: Vias

    private mutating func checkVias() {
        for (v, via) in copper.vias.enumerated() {
            var problems: [String] = []
            if via.drill < rules.minViaDrill - Self.tolerance {
                problems.append("drill \(mm(via.drill)) under the \(mm(rules.minViaDrill)) minimum")
            }
            if via.diameter < rules.minViaDiameter - Self.tolerance {
                problems.append("diameter \(mm(via.diameter)) under the \(mm(rules.minViaDiameter)) minimum")
            }
            if via.ring < rules.minAnnularRing - Self.tolerance {
                problems.append("annular ring \(mm(via.ring)) under the \(mm(rules.minAnnularRing)) minimum")
            }
            guard !problems.isEmpty else { continue }
            let text = problems.joined(separator: "; ")
            add(.via, viaName(v), text.prefix(1).uppercased() + text.dropFirst(), [.via(via.id)])
        }
    }

    // MARK: Traces

    private mutating func checkTraceWidths() {
        for (t, trace) in copper.traces.enumerated() where trace.widthMM < rules.minTraceWidth - Self.tolerance {
            add(.traceWidth, traceName(t),
                "Width \(trace.width) mil (\(mm(trace.widthMM))); min \(mm(rules.minTraceWidth))", [.trace(trace.id)])
        }
    }

    // MARK: Length match

    /// Each length-match group whose routed members' lengths spread more
    /// than its tolerance: one line naming the shortest and longest; and a
    /// line for each member with more than two pins, which can't be
    /// matched. Unrouted members are left out (they have unrouted lines).
    private mutating func checkLengthMatch() {
        for group in board.netGroups where group.mode == .lengthMatch {
            var lengths: [(name: String, length: Double, net: Int)] = []
            for member in group.members {
                guard let net = copper.net(of: member) else { continue }
                let name = copper.netNames[net] ?? "Net \(net + 1)"
                if copper.pinCount(ofNet: net) > 2 {
                    add(.lengthMatch, name, "Net \(name) has more than two pins; its length is not matched",
                        traces(ofNet: net))
                    continue
                }
                if let length = RoutingLength.of(net: net, in: copper) { lengths.append((name, length, net)) }
            }
            guard lengths.count >= 2, let shortest = lengths.min(by: { $0.length < $1.length }),
                  let longest = lengths.max(by: { $0.length < $1.length }),
                  longest.length - shortest.length > group.tolerance + Self.tolerance else { continue }
            add(.lengthMatch, group.name,
                "Group \(group.name): nets differ by \(mm(longest.length - shortest.length)) "
                    + "(\(longest.name) \(mm(longest.length)), \(shortest.name) \(mm(shortest.length))); "
                    + "tolerance \(mm(group.tolerance))",
                traces(ofNet: longest.net).union(traces(ofNet: shortest.net)))
        }
    }

    /// Each differential pair with both nets routed whose traces run apart
    /// (not at its gap, or on different layers) for more than its
    /// `maxUncoupled` (`PairGeometry.uncoupledLength`).
    /// A pour that can't do its job: no net, a net the schematic lacks, no copper left, spokes
    /// under the minimum trace width, or one that overlaps a pour of another net (plan-52-H).
    private mutating func checkPours() {
        for (n, pour) in pours.enumerated() {
            let name = "Pour \(n + 1)"
            let items: Set<BoardItem> = [.pour(pour.id)]
            let on = "Pour on \(pour.layer.title)"
            guard let netName = pour.net else {
                add(.pour, name, "\(on) has no net; it fills nothing", items)
                continue
            }
            guard let net = copper.net(of: netName) else {
                add(.pour, name, "\(on): its net is not in the schematic", items)
                continue
            }
            if copper.pourFills[pour.id]?.isEmpty ?? true {
                add(.pour, name, "\(on) keeps no copper: it reaches no pin of its net", items)
            }
            if pour.connection == .thermal, pour.spokeWidth < rules.minTraceWidth - Self.tolerance {
                add(.pour, name, "\(on): thermal spokes \(mm(pour.spokeWidth)) are narrower than the minimum trace width \(mm(rules.minTraceWidth))", items)
            }
            for other in pours[..<n] where other.layer == pour.layer && other.rect.intersects(pour.rect) {
                guard let otherName = other.net, let otherNet = copper.net(of: otherName), otherNet != net else { continue }
                add(.pour, name, "\(on) overlaps the pour of \(copper.netNames[otherNet] ?? "Net \(otherNet + 1)"); the later one gives way", items)
            }
        }
    }

    private mutating func checkDiffPairs() {
        let netOfRoot = copper.netOfRoot()
        func traces(of net: Int) -> [RoutedTrace] {
            copper.traces.indices.filter { netOfRoot[copper.root[copper.traceNode($0)]] == net }.map {
                let trace = copper.traces[$0]
                return RoutedTrace(layer: trace.layer, width: trace.widthMM, points: trace.points, net: net)
            }
        }
        for group in board.netGroups where group.mode == .diffPair && group.members.count == 2 {
            let nets = group.members.compactMap { copper.net(of: $0) }
            guard nets.count == 2, nets.allSatisfy({ RoutingLength.of(net: $0, in: copper) != nil }) else { continue }
            let gap = max(group.gap, rules.minClearance)
            let apart = PairGeometry.uncoupledLength(p: traces(of: nets[0]), n: traces(of: nets[1]), gap: gap)
            guard apart > group.maxUncoupled + Self.tolerance else { continue }
            add(.diffPair, group.name, "Pair \(group.name): \(mm(apart)) runs apart (max \(mm(group.maxUncoupled)))",
                self.traces(ofNet: nets[0]).union(self.traces(ofNet: nets[1])))
        }
    }

    /// The traces of net `net`'s copper, as selection items.
    private func traces(ofNet net: Int) -> Set<BoardItem> {
        let netOfRoot = copper.netOfRoot()
        return Set(copper.traces.indices.filter { netOfRoot[copper.root[copper.traceNode($0)]] == net }
            .map { .trace(copper.traces[$0].id) })
    }

    // MARK: Silkscreen

    /// A piece of silkscreen ink, on one side.
    private struct Ink {
        let shape: CopperShape
        let side: BoardSide
        let owner: String
        /// "Outline", "Pin-1 mark", "Reference" or "Text".
        let what: String
        let items: Set<BoardItem>
    }

    private mutating func checkSilkscreen() {
        var inks: [Ink] = []
        var thinOutlines: [PlacedChip] = []
        var shortReferences: [PlacedChip] = []
        for chip in chips {
            guard let placement = chip.board, let body = chip.boardBody else { continue }
            let items: Set<BoardItem> = [.footprint(chip.id)]
            if placement.outline == .silkscreen {
                if Self.outlineWidth < rules.minSilkscreenWidth - Self.tolerance { thinOutlines.append(chip) }
                let band = chip.cathodeBand.map { [($0.a, $0.b)] } ?? []
                for (a, b) in Self.outlineSegments(body.rect, round: body.round) + band {
                    inks.append(Ink(shape: CopperShape(core: .segment(a, b), radius: Self.outlineWidth / 2, layers: nil, node: -1),
                                    side: placement.side, owner: chip.reference, what: "Outline", items: items))
                }
            }
            if placement.pin1 == .silkscreen, let mark = chip.pin1Mark {
                inks.append(Ink(shape: CopperShape(core: .segment(mark, mark), radius: Self.pin1Diameter / 2, layers: nil, node: -1),
                                side: placement.side, owner: chip.reference, what: "Pin-1 mark", items: items))
            }
            if placement.reference == .silkscreen, let rect = chip.boardReferenceRect() {
                if BoardReference.height < rules.minSilkscreenHeight - Self.tolerance { shortReferences.append(chip) }
                inks.append(Ink(shape: CopperShape(core: .rect(rect), radius: 0, layers: nil, node: -1),
                                side: placement.side, owner: chip.reference, what: "Reference", items: items))
            }
        }
        if !thinOutlines.isEmpty {
            add(.silkscreen, list(thinOutlines.map(\.reference), limit: 6),
                "Outline line \(mm(Self.outlineWidth)); min silkscreen line width \(mm(rules.minSilkscreenWidth))",
                Set(thinOutlines.map { .footprint($0.id) }))
        }
        if !shortReferences.isEmpty {
            add(.silkscreen, list(shortReferences.map(\.reference), limit: 6),
                "Reference text \(mm(BoardReference.height)) tall; min silkscreen text height \(mm(rules.minSilkscreenHeight))",
                Set(shortReferences.map { .footprint($0.id) }))
        }
        for text in texts where text.layer == .silkscreen {
            let items: Set<BoardItem> = [.text(text.id)]
            if text.sizeMM < rules.minSilkscreenHeight - Self.tolerance {
                add(.silkscreen, textName(text),
                    "Text \(mm(text.sizeMM)) tall; min silkscreen text height \(mm(rules.minSilkscreenHeight))", items)
            }
            let stroke = text.sizeMM * Self.textStrokeShare
            if stroke < rules.minSilkscreenWidth - Self.tolerance {
                add(.silkscreen, textName(text),
                    "Text strokes about \(mm(stroke)) wide; min silkscreen line width \(mm(rules.minSilkscreenWidth))",
                    items)
            }
            inks.append(Ink(shape: CopperShape(core: .rect(text.boardRect), radius: 0, layers: nil, node: -1),
                            side: text.side, owner: textName(text), what: "Text", items: items))
        }

        // Ink near an exposed pad on its side: the closest pad per ink
        // owner and part.
        struct Key: Hashable { let ink: String; let pad: String }
        var worst: [Key: (gap: Double, ink: Ink, pad: Int)] = [:]
        for side in BoardSide.allCases {
            let sideInks = inks.filter { $0.side == side }
            guard !sideInks.isEmpty else { continue }
            var shapes = sideInks.enumerated().map { i, ink in
                CopperShape(core: ink.shape.core, radius: ink.shape.radius, layers: nil, node: -1 - i)
            }
            for (p, pad) in copper.pads.enumerated() where pad.pad.drill > 0 || pad.side == side {
                shapes.append(CopperGeometry.shape(of: pad.pad, layers: nil, node: p))
            }
            CopperGeometry.nearPairs(shapes, margin: rules.minSilkscreenToPad) { a, b in
                guard (a.node < 0) != (b.node < 0) else { return }
                let (inkShape, padShape) = a.node < 0 ? (a, b) : (b, a)
                let gap = inkShape.gap(to: padShape)
                guard gap < rules.minSilkscreenToPad - Self.tolerance else { return }
                let ink = sideInks[-1 - inkShape.node]
                let key = Key(ink: ink.owner, pad: owner(padShape.node))
                if gap < worst[key]?.gap ?? .infinity { worst[key] = (gap, ink, padShape.node) }
            }
        }
        for (_, found) in worst.sorted(by: { ($0.key.ink, $0.value.pad) < ($1.key.ink, $1.value.pad) }) {
            let pad = copper.pads[found.pad]
            let where_ = found.gap <= Self.tolerance ? "over pad \(pad.label)" : "\(mm(found.gap)) from pad \(pad.label)"
            add(.silkscreen, found.ink.owner,
                "\(found.ink.what) \(where_); min silkscreen to pad \(mm(rules.minSilkscreenToPad))",
                found.ink.items.union(item(ofPad: found.pad)))
        }
    }

    /// A body outline as segments: its rectangle, or its ellipse in 32.
    static func outlineSegments(_ rect: CGRect, round: Bool) -> [(CGPoint, CGPoint)] {
        let points: [CGPoint]
        if round {
            points = (0..<32).map { i in
                let angle = Double(i) / 32 * 2 * .pi
                return CGPoint(x: rect.midX + rect.width / 2 * cos(angle), y: rect.midY + rect.height / 2 * sin(angle))
            }
        } else {
            points = [CGPoint(x: rect.minX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.minY),
                      CGPoint(x: rect.maxX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.maxY)]
        }
        return Array(zip(points, points.dropFirst() + points.prefix(1)))
    }

    // MARK: Solder mask

    private mutating func checkSolderMask() {
        // Two pads on one side whose mask openings (each grown by the
        // expansion) leave too thin a strip between them.
        let expansion = rules.solderMaskExpansion
        struct Key: Hashable { let a: String; let b: String }
        var worst: [Key: (dam: Double, a: Int, b: Int)] = [:]
        for side in BoardSide.allCases {
            let shapes = copper.pads.enumerated().compactMap { p, pad -> CopperShape? in
                guard pad.pad.drill > 0 || pad.side == side else { return nil }
                return CopperGeometry.shape(of: pad.pad, layers: nil, node: p)
            }
            CopperGeometry.nearPairs(shapes, margin: rules.minSolderMaskDam + 2 * expansion) { a, b in
                // Pads of one pin share an opening.
                if copper.root[a.node] == copper.root[b.node], copper.pads[a.node].chip == copper.pads[b.node].chip,
                   copper.pads[a.node].pad.designator == copper.pads[b.node].pad.designator { return }
                let dam = a.gap(to: b) - 2 * expansion
                guard dam < rules.minSolderMaskDam - Self.tolerance else { return }
                let (first, second) = a.node < b.node ? (a.node, b.node) : (b.node, a.node)
                let key = Key(a: owner(first), b: owner(second))
                if dam < worst[key]?.dam ?? .infinity { worst[key] = (dam, first, second) }
            }
        }
        for (key, found) in worst.sorted(by: { ($0.value.a, $0.value.b) < ($1.value.a, $1.value.b) }) {
            let one = describe(node: found.a), two = describe(node: found.b)
            let dam = found.dam <= Self.tolerance ? "No solder mask dam (the openings merge)"
                : "Solder mask dam \(mm(found.dam))"
            let between = key.a == key.b ? " between pads \(Self.padName(one.name)) and \(Self.padName(two.name))" : ""
            add(.solderMask, key.a == key.b ? key.a : "\(one.name), \(two.name)",
                "\(dam)\(between); min \(mm(rules.minSolderMaskDam))", one.items.union(two.items))
        }
    }
}
