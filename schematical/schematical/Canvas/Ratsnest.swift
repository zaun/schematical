import CoreGraphics
import Foundation

/// The PCB's unrouted connections: for each schematic net, the pads its
/// pins have on the board (and the card's edge fingers, through the bus the
/// card edge uses, as if it were a hidden port with every wire of that
/// bus), grouped by the copper that already joins them
/// (traces, through-hole pads and vias between layers, pads of one pin). Each net
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
        let netlist = NetlistBuilder.build(chips: chips, wires: wires, buses: buses)
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
        // Every finger of the card, as pads of the card (chip −1).
        for finger in board.fingers {
            padsOfPin["edge/\(finger.name)", default: []].append(pads.count)
            pads.append(Pad(chip: -1, pad: PlacedPad(designator: finger.name, rect: finger.rect, drill: 0,
                                                     shape: .rect, radius: 0, chamfer: 0, corners: []),
                            side: finger.side, label: "Edge.\(finger.name)"))
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
            where pad.pad.drill > 0 || pad.side.copperLayer == trace.layer {
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
        return BoardConnectivity(pads: pads, traces: copper, vias: vias,
                                 root: parent.indices.map(find), netOfPad: netOfPad)
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
/// (the parts' pads, then the card's edge fingers), then every trace, then
/// every via. Pads of one pin are joined; a trace joins the pads it
/// touches on its layer (any through-hole pad) and the traces it meets
/// there; a via joins whatever touches its metal.
nonisolated struct BoardConnectivity {
    struct Pad {
        /// Index into the document's chips; −1 for an edge finger.
        let chip: Int
        let pad: PlacedPad
        let side: BoardSide
        /// "U1.3", "U1.EP" for an unnumbered centre pad, "Edge.A1".
        let label: String

        var centre: CGPoint { CGPoint(x: pad.rect.midX, y: pad.rect.midY) }
    }

    let pads: [Pad]
    let traces: [BoardTrace]
    let vias: [BoardVia]
    /// Each node's group: the node index of its root.
    let root: [Int]
    /// The schematic net of each pad on one (pad index → net).
    let netOfPad: [Int: Int]

    func traceNode(_ index: Int) -> Int { pads.count + index }
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
