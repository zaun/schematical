import CoreGraphics
import Foundation

/// The PCB's unrouted connections: for each schematic net, the pads its
/// pins have on the board (and the card's edge fingers, through the bus the
/// card edge uses, as if it were a hidden port with every wire of that
/// bus; every VCC or GND symbol of one voltage as one net, with the far pad
/// of each pull resistor on its rail), grouped by the copper that already joins them
/// (traces, through-hole pads and vias between layers, pads of one pin, a
/// Plane layer's through-hole pads and vias on its net). Each net
/// with more than one group gets the shortest lines that join its groups
/// (a minimum spanning tree), each between the closest points of two
/// groups: a pad centre or a trace vertex.
extension SchematicDocument {
    /// The unrouted lines, in board mm. `drawing` is a trace being drawn
    /// that isn't added yet: it counts as copper, and a group it is in is
    /// reached only from its last point, so the line follows the cursor.
    func ratsnest(drawing: BoardTrace? = nil) -> [(CGPoint, CGPoint)] {
        let copper = connectivity(drawing: drawing)
        guard !copper.netOfPad.isEmpty else { return [] }
        let drawingRoot = drawing.map { _ in copper.root[copper.traceNode(copper.traces.count - 1)] }
        var tracePoints: [Int: [CGPoint]] = [:]
        for (t, trace) in copper.traces.enumerated() where trace.id != drawing?.id {
            tracePoints[copper.root[copper.traceNode(t)], default: []] += trace.points
        }
        var lines: [(CGPoint, CGPoint)] = []
        for (_, groups) in copper.netGroups() {
            let points: [[CGPoint]] = groups.map { root, pads in
                if root == drawingRoot, let end = drawing?.points.last { return [end] }
                return pads.map { copper.pads[$0].centre } + (tracePoints[root] ?? [])
            }
            lines += Self.spanningLines(points)
        }
        return lines
    }

    /// The board's copper and which of it is joined (see `BoardConnectivity`).
    /// `drawing`, a trace not added yet, counts as the last trace.
    func connectivity(drawing: BoardTrace? = nil) -> BoardConnectivity {
        let netlist = NetlistBuilder.build(chips: chips, wires: wires, buses: buses, labels: netLabels)
        // The card edge is a hidden port on its bus: pin i is the bus's wire
        // i, on whatever net that wire's port pins are wired into.
        var fingerNets: [String: Int] = [:]
        if let busID = edgeBusID, let bus = bus(busID) {
            let portIDs = Set(bus.ports.map(\.id))
            var netOfBusWire: [UUID: Int] = [:]
            for wire in wires {
                for end in [wire.start, wire.end] {
                    guard case let .busPin(ref) = end, portIDs.contains(ref.portID),
                          let net = netlist.netOfWire[wire.id] else { continue }
                    netOfBusWire[ref.wireID] = net
                }
            }
            for (name, wire) in zip(board.pinNames, bus.wires) {
                if let net = netOfBusWire[wire.id] { fingerNets[name] = net }
            }
        }

        // Nodes: every placed pad, then every trace (`drawing` last).
        typealias Pad = BoardConnectivity.Pad
        var pads: [Pad] = []
        var padsOfPin: [String: [Int]] = [:]
        for (index, chip) in chips.enumerated() {
            guard let side = chip.board?.side else { continue }
            for pad in chip.boardPads {
                if let designator = pad.designator {
                    padsOfPin["\(index)/\(designator)", default: []].append(pads.count)
                }
                pads.append(Pad(chip: index, pad: pad, side: side,
                                label: "\(chip.reference).\(pad.designator ?? "EP")"))
            }
        }
        // Every finger or header pad of the card, as pads of the card
        // (chip −1), then its mounting holes (no copper).
        for finger in board.fingers {
            padsOfPin["edge/\(finger.name)", default: []].append(pads.count)
            pads.append(Pad(chip: -1, pad: finger.pad, side: finger.side,
                            label: "\(board.connectorName).\(finger.name)"))
        }
        // Header pads off the bus: on no net, joined only by copper.
        for pad in board.extraPads {
            pads.append(Pad(chip: -1, pad: pad.pad, side: pad.side, label: pad.name))
        }
        for (n, hole) in board.holes.enumerated() {
            pads.append(Pad(chip: -1, pad: hole.pad, side: .top, label: "Board hole \(n + 1)"))
        }
        // The drilled holes (H1, H2, … in drawing order): a plated one is a
        // pad on its net.
        let holeBase = pads.count
        for (n, hole) in holes.enumerated() {
            pads.append(Pad(chip: -1, pad: hole.pad, side: .top, label: "H\(n + 1)", hole: hole.id))
        }
        let copper = traces + (drawing.map { [$0] } ?? [])
        // Vias come after the traces.
        let viaBase = pads.count + copper.count
        var parent = Array(0..<(viaBase + vias.count))
        func find(_ node: Int) -> Int {
            var node = node
            while parent[node] != node {
                parent[node] = parent[parent[node]]
                node = parent[node]
            }
            return node
        }
        func union(_ a: Int, _ b: Int) { parent[find(a)] = find(b) }

        // Pads of one pin are one node.
        for group in padsOfPin.values {
            for other in group.dropFirst() { union(group[0], other) }
        }
        // A trace joins the pads it touches on its layer, and the traces it
        // shares a point with there.
        for (t, trace) in copper.enumerated() {
            let node = pads.count + t
            for (p, pad) in pads.enumerated()
            where pad.pad.plated && (pad.pad.drill > 0 || pad.side.copperLayer == trace.layer) {
                if trace.segments.contains(where: { Self.segment($0.0, $0.1, touches: pad.pad.rect) }) {
                    union(node, p)
                }
            }
            for (o, other) in copper.enumerated().dropFirst(t + 1) where other.layer == trace.layer {
                if Self.meet(trace, other) { union(node, pads.count + o) }
            }
        }

        // A via goes through every layer: it joins every trace that touches
        // its metal and every pad its metal overlaps.
        for (v, via) in vias.enumerated() {
            let node = viaBase + v
            for (t, trace) in copper.enumerated() where trace.segments.contains(where: {
                Self.distance(from: via.centre, toSegment: $0.0, $0.1) <= via.diameter / 2 + trace.widthMM / 2
            }) {
                union(node, pads.count + t)
            }
            for (p, pad) in pads.enumerated() where Self.circle(via.centre, via.diameter / 2, meets: pad.pad.rect) {
                union(node, p)
            }
        }

        // Each pad's net.
        var netOfPad: [Int: Int] = [:]
        for connection in netlist.connections {
            for p in padsOfPin["\(connection.part)/\(connection.pin)"] ?? [] {
                netOfPad[p] = connection.net
            }
        }
        for (name, net) in fingerNets {
            for p in padsOfPin["edge/\(name)"] ?? [] {
                netOfPad[p] = net
            }
        }

        // Power and ground: every VCC or GND symbol of one voltage is one
        // rail, so pins wired to different symbols of it share a net. A pull
        // resistor's pad that no pin has (its far end) is on its rail too.
        var railNet: [UInt32: Int] = [:]
        var alias: [Int: Int] = [:]
        func merged(_ net: Int) -> Int {
            var net = net
            while let next = alias[net] { net = next }
            return net
        }
        for connection in netlist.connections {
            guard case let .supply(_, mv, false) = chips[connection.part].part.details else { continue }
            guard let rail = railNet[mv] else { railNet[mv] = connection.net; continue }
            let a = merged(connection.net), b = merged(rail)
            if a != b { alias[a] = b }
        }
        var nextNet = netlist.netCount
        for (index, chip) in chips.enumerated() {
            guard case let .supply(_, mv, true) = chip.part.details else { continue }
            let pins = Set(chip.part.pins.map(\.designator))
            for pad in chip.boardPads {
                guard let designator = pad.designator, !pins.contains(designator) else { continue }
                if railNet[mv] == nil {
                    railNet[mv] = nextNet
                    nextNet += 1
                }
                for p in padsOfPin["\(index)/\(designator)"] ?? [] { netOfPad[p] = railNet[mv] }
            }
        }
        netOfPad = netOfPad.mapValues(merged)

        // The schematic net a Plane or plated hole names, if the schematic
        // still has it.
        func resolve(_ net: PlaneNet) -> Int? {
            switch net {
            case let .rail(mv):
                return railNet[mv].map(merged)
            case let .pin(chipID, designator):
                guard let part = chips.firstIndex(where: { $0.id == chipID }) else { return nil }
                if let pad = padsOfPin["\(part)/\(designator)"]?.first, let found = netOfPad[pad] {
                    return found
                }
                return netlist.connections.first { $0.part == part && $0.pin == designator }
                    .map { merged($0.net) }
            }
        }
        for (n, hole) in holes.enumerated() where hole.plated {
            if let net = hole.net.flatMap(resolve) { netOfPad[holeBase + n] = net }
        }

        // A Plane joins every plated through-hole pad and via on its net
        // (one whose copper already reaches a pad of the net).
        var planeNets: [CopperLayer: Int] = [:]
        for layer in board.copperLayers {
            let style = board.style(of: layer)
            guard style.type == .plane, let net = style.net else { continue }
            if let found = resolve(net) { planeNets[layer] = found }
        }
        if !planeNets.isEmpty {
            var netOfRoot: [Int: Int] = [:]
            for (pad, net) in netOfPad { netOfRoot[find(pad)] = net }
            let throughNodes = pads.indices.filter { pads[$0].pad.plated && pads[$0].pad.drill > 0 }
                + vias.indices.map { viaBase + $0 }
            // Each node's net before any plane joins, so one plane's joins
            // don't change what the next one reads.
            let nodeNets = throughNodes.map { netOfRoot[find($0)] }
            for net in Set(planeNets.values) {
                let joined = zip(throughNodes, nodeNets).filter { $0.1 == net }.map(\.0)
                for other in joined.dropFirst() { union(joined[0], other) }
            }
        }
        return BoardConnectivity(pads: pads, traces: copper, vias: vias,
                                 root: parent.indices.map(find), netOfPad: netOfPad, planeNets: planeNets)
    }

    /// Whether a circle meets a rectangle.
    static func circle(_ centre: CGPoint, _ radius: CGFloat, meets rect: CGRect) -> Bool {
        let x = min(max(centre.x, rect.minX), rect.maxX), y = min(max(centre.y, rect.minY), rect.maxY)
        return hypot(centre.x - x, centre.y - y) <= radius
    }

    /// Whether two traces on one layer meet: a point of one on a segment
    /// of the other.
    private static func meet(_ a: BoardTrace, _ b: BoardTrace) -> Bool {
        func touches(_ points: [CGPoint], _ trace: BoardTrace) -> Bool {
            points.contains { point in
                trace.segments.contains { distance(from: point, toSegment: $0.0, $0.1) < 1e-4 }
            }
        }
        return touches(a.points, b) || touches(b.points, a)
    }

    /// Prim's tree over groups of points: a group's distance to another is
    /// that of their closest points, and each line joins those points.
    static func spanningLines(_ groups: [[CGPoint]]) -> [(CGPoint, CGPoint)] {
        guard groups.count > 1 else { return [] }
        func closest(_ a: [CGPoint], _ b: [CGPoint]) -> (CGPoint, CGPoint, CGFloat) {
            var best = (a[0], b[0], CGFloat.infinity)
            for p in a {
                for q in b {
                    let d = hypot(p.x - q.x, p.y - q.y)
                    if d < best.2 { best = (p, q, d) }
                }
            }
            return best
        }
        var inTree = [0]
        var rest = Array(groups.indices.dropFirst())
        var lines: [(CGPoint, CGPoint)] = []
        while !rest.isEmpty {
            var best: (index: Int, line: (CGPoint, CGPoint, CGFloat))?
            for (i, g) in rest.enumerated() {
                for t in inTree {
                    let line = closest(groups[t], groups[g])
                    if line.2 < best?.line.2 ?? .infinity { best = (i, line) }
                }
            }
            guard let best else { break }
            lines.append((best.line.0, best.line.1))
            inTree.append(rest.remove(at: best.index))
        }
        return lines
    }
}

/// The board's copper as nodes, and which nodes copper joins: every pad
/// (the parts' pads, then the card's edge fingers or header pads and its
/// mounting holes), then every trace, then
/// every via. Pads of one pin are joined; a trace joins the pads it
/// touches on its layer (any through-hole pad) and the traces it meets
/// there; a via joins whatever touches its metal.
nonisolated struct BoardConnectivity {
    struct Pad {
        /// Index into the document's chips; −1 for an edge finger, header
        /// pad or mounting hole of the board.
        let chip: Int
        let pad: PlacedPad
        let side: BoardSide
        /// "U1.3", "U1.EP" for an unnumbered centre pad, "Edge.A1",
        /// "Header.40", "Board hole 1", "H1".
        let label: String
        /// The drilled hole (`BoardHole`) this pad is, if one.
        var hole: UUID? = nil

        var centre: CGPoint { CGPoint(x: pad.rect.midX, y: pad.rect.midY) }
    }

    let pads: [Pad]
    let traces: [BoardTrace]
    let vias: [BoardVia]
    /// Each node's group: the node index of its root.
    let root: [Int]
    /// The schematic net of each pad on one (pad index → net).
    let netOfPad: [Int: Int]
    /// The net each Plane layer carries, for the Planes that have one the
    /// schematic still has.
    var planeNets: [CopperLayer: Int] = [:]

    func traceNode(_ index: Int) -> Int { pads.count + index }

    /// Each group's net, by root: that of a pad in it. A group reaching no
    /// pad on a net has none.
    func netOfRoot() -> [Int: Int] {
        var found: [Int: Int] = [:]
        for (pad, net) in netOfPad { found[root[pad]] = net }
        return found
    }
    /// What a Plane on `layer` is cut away around: every hole through it
    /// not on its net (pad and via indices). A plated through-hole pad or
    /// via on the Plane's net joins it; a non-plated hole is always cut.
    /// With no net, every hole is cut.
    func planeClearances(on layer: CopperLayer) -> (pads: [Int], vias: [Int]) {
        let net = planeNets[layer]
        let nets = netOfRoot()
        func joins(_ node: Int) -> Bool { net != nil && nets[root[node]] == net }
        let cutPads = pads.indices.filter { p in
            pads[p].pad.drill > 0 && !(pads[p].pad.plated && joins(p))
        }
        return (cutPads, vias.indices.filter { !joins(viaNode($0)) })
    }

    func viaNode(_ index: Int) -> Int { pads.count + traces.count + index }

    /// Each net's pads grouped by the copper joining them, nets in order,
    /// groups by root: `(net, [(root, pad indices)])`.
    func netGroups() -> [(net: Int, groups: [(root: Int, pads: [Int])])] {
        var byNet: [Int: [Int: [Int]]] = [:]
        for (pad, net) in netOfPad {
            byNet[net, default: [:]][root[pad], default: []].append(pad)
        }
        return byNet.keys.sorted().compactMap { net in
            let groups = byNet[net]!
            guard groups.count > 1 else { return nil }
            return (net, groups.keys.sorted().map { ($0, groups[$0]!.sorted()) })
        }
    }
}
