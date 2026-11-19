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
        guard drawing == nil else { return ratsnest(in: connectivity(drawing: drawing), drawing: drawing) }
        // Kept until the copper or the schematic changes (`copperRevision`).
        let revision = copperRevision
        if let kept = keptRatsnest, kept.revision == revision { return kept.lines }
        let lines = ratsnest(in: connectivity())
        keptRatsnest = (revision, lines)
        return lines
    }

    /// The unrouted lines of `copper` (built with `drawing`), of every net
    /// or of net `net` alone.
    func ratsnest(in copper: BoardConnectivity, drawing: BoardTrace? = nil, net only: Int? = nil) -> [(CGPoint, CGPoint)] {
        guard !copper.netOfPad.isEmpty else { return [] }
        let drawingRoot = drawing.map { _ in copper.root[copper.traceNode(copper.traces.count - 1)] }
        var tracePoints: [Int: [CGPoint]] = [:]
        for (t, trace) in copper.traces.enumerated() where trace.id != drawing?.id {
            tracePoints[copper.root[copper.traceNode(t)], default: []] += trace.points
        }
        var lines: [(CGPoint, CGPoint)] = []
        for (net, groups) in copper.netGroups() where only.map({ $0 == net }) ?? true {
            let points: [[CGPoint]] = groups.map { root, pads in
                if root == drawingRoot, let end = drawing?.points.last { return [end] }
                return pads.map { copper.pads[$0].centre } + (tracePoints[root] ?? [])
            }
            lines += Self.spanningLines(points)
        }
        return lines
    }

    /// The board's copper and which of it is joined (see `BoardConnectivity`).
    /// `drawing`, a trace not added yet, counts as the last trace. Traces
    /// and vias in `excludingTraces` / `excludingVias` are left out, as if
    /// deleted (the router's view after clearing copper).
    ///
    /// The plain call (nothing drawn, nothing left out) is built once per
    /// change to the copper or the schematic (`copperRevision`) and kept:
    /// the canvas and the sidebar ask for it on every redraw.
    func connectivity(drawing: BoardTrace? = nil, excludingTraces: Set<UUID> = [],
                      excludingVias: Set<UUID> = []) -> BoardConnectivity {
        guard drawing == nil, excludingTraces.isEmpty, excludingVias.isEmpty else {
            return buildConnectivity(drawing: drawing, excludingTraces: excludingTraces, excludingVias: excludingVias)
        }
        let revision = copperRevision
        if let kept = keptConnectivity, kept.revision == revision { return kept.copper }
        let copper = buildConnectivity(drawing: nil, excludingTraces: [], excludingVias: [])
        keptConnectivity = (revision, copper)
        return copper
    }

    private func buildConnectivity(drawing: BoardTrace?, excludingTraces: Set<UUID>,
                                   excludingVias: Set<UUID>) -> BoardConnectivity {
        let traces = excludingTraces.isEmpty ? self.traces : self.traces.filter { !excludingTraces.contains($0.id) }
        let vias = excludingVias.isEmpty ? self.vias : self.vias.filter { !excludingVias.contains($0.id) }
        let netlist = NetlistBuilder.build(chips: chips, wires: wires, buses: buses, labels: netLabels)
        // The card edge is a hidden port on its bus: pin i is the bus's wire
        // i, on whatever net that wire's port pins are wired into.
        var fingerNets: [String: Int] = [:]
        // Its wires' nets include ports on the buses linked with it.
        if let busID = edgeBusID, let bus = bus(busID) {
            let group = buses.filter { $0.id == busID || bus.isLinked(with: $0) }
            let portIDs = Set(group.flatMap { $0.ports.map(\.id) })
            let linked = Bus.linkedWireIDs(buses)
            var netOfBusWire: [UUID: Int] = [:]
            for wire in wires {
                for end in [wire.start, wire.end] {
                    guard case let .busPin(ref) = end, portIDs.contains(ref.portID),
                          let net = netlist.netOfWire[wire.id] else { continue }
                    netOfBusWire[linked[ref.wireID] ?? ref.wireID] = net
                }
            }
            for (name, wire) in zip(board.pinNames, bus.wires) {
                if let net = netOfBusWire[linked[wire.id] ?? wire.id] { fingerNets[name] = net }
            }
        }

        // Nodes: every placed pad, then every trace (`drawing` last).
        typealias Pad = BoardConnectivity.Pad
        var pads: [Pad] = []
        var padsOfPin: [String: [Int]] = [:]
        let chipPads = chipPads
        for (index, chip) in chips.enumerated() {
            guard let side = chip.board?.side else { continue }
            for pad in chipPads[index] {
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
        // In key order, so the roots (and the ratsnest's ties) are the same every run.
        for group in padsOfPin.keys.sorted().compactMap({ padsOfPin[$0] }) {
            for other in group.dropFirst() { union(group[0], other) }
        }
        // A trace joins the pads it touches on its layer, and the traces it
        // shares a point with there.
        // The pads and every trace segment are sorted into a coarse grid, so
        // each trace and via is tested only against the copper near it.
        // What touches is found first and joined after, in index order (the
        // order of the joins decides the roots, and so the ratsnest's ties).
        let segments = copper.map(\.segments)
        func code(_ layer: CopperLayer) -> Int {
            switch layer {
            case .top: 1
            case let .inner(place): place
            case .bottom: 100
            }
        }
        let layers = copper.map { code($0.layer) }
        let padBoxes = pads.map { PointBox($0.pad.rect) }
        // A plated through-hole pad is on every layer; a plated
        // surface-mount pad on its side's; a non-plated hole on none.
        let padThrough = pads.map { $0.pad.plated && $0.pad.drill > 0 }
        let padLayer = pads.map { $0.pad.plated ? code($0.side.copperLayer) : -2 }
        let padGrid = BoxGrid(padBoxes)
        var segmentTrace: [Int] = [], segmentEnds: [(CGPoint, CGPoint)] = []
        for (t, list) in segments.enumerated() {
            for ends in list {
                segmentTrace.append(t)
                segmentEnds.append(ends)
            }
        }
        let segmentGrid = BoxGrid(segmentEnds.map { PointBox($0.0, $0.1) })
        let widest = copper.map(\.widthMM).max() ?? 0

        // Two traces on one layer meet where a point of one is on a segment
        // of the other: `meets[t]` is the later traces trace t meets.
        var meets: [[Int]] = Array(repeating: [], count: copper.count)
        for (t, trace) in copper.enumerated() {
            for point in trace.points {
                for s in segmentGrid.near(PointBox(point, point), within: 1e-3) {
                    let other = segmentTrace[s]
                    guard other != t, layers[other] == layers[t],
                          Self.distance(from: point, toSegment: segmentEnds[s].0, segmentEnds[s].1) < 1e-4 else { continue }
                    meets[min(t, other)].append(max(t, other))
                }
            }
        }
        for t in copper.indices {
            let node = pads.count + t
            let layer = layers[t]
            var touched: [Int] = []
            for (a, b) in segments[t] {
                let box = PointBox(a, b)
                for p in padGrid.near(box, within: 1e-6)
                where (padThrough[p] || padLayer[p] == layer) && box.reaches(padBoxes[p], within: 1e-6) {
                    if Self.segment(a, b, touches: pads[p].pad) { touched.append(p) }
                }
            }
            for p in Set(touched).sorted() { union(node, p) }
            for other in Set(meets[t]).sorted() { union(node, pads.count + other) }
        }

        // A via goes through every layer: it joins every trace that touches
        // its metal and every pad its metal overlaps.
        for (v, via) in vias.enumerated() {
            let node = viaBase + v
            let centre = PointBox(via.centre, via.centre)
            var touched: [Int] = []
            for s in segmentGrid.near(centre, within: via.diameter / 2 + widest / 2 + 1e-3) {
                let t = segmentTrace[s]
                if Self.distance(from: via.centre, toSegment: segmentEnds[s].0, segmentEnds[s].1)
                    <= via.diameter / 2 + copper[t].widthMM / 2 { touched.append(t) }
            }
            for t in Set(touched).sorted() { union(node, pads.count + t) }
            for p in padGrid.near(centre, within: via.diameter / 2 + 1e-3) {
                if Self.circle(via.centre, via.diameter / 2, meets: pads[p].pad.rect) { union(node, p) }
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
            for pad in chipPads[index] {
                guard let designator = pad.designator, !pins.contains(designator) else { continue }
                if railNet[mv] == nil {
                    railNet[mv] = nextNet
                    nextNet += 1
                }
                for p in padsOfPin["\(index)/\(designator)"] ?? [] { netOfPad[p] = railNet[mv] }
            }
        }
        netOfPad = netOfPad.mapValues(merged)

        // The schematic net a Plane, plated hole or net class names: a
        // rail's, or a pin's (its pads' net, else the net its schematic pin
        // is wired into).
        var names = BoardConnectivity(pads: [], traces: [], vias: [], root: [], netOfPad: [:])
        names.railNets = railNet.mapValues(merged)
        for (index, chip) in chips.enumerated() {
            for pad in chipPads[index] {
                guard let designator = pad.designator, let first = padsOfPin["\(index)/\(designator)"]?.first,
                      let net = netOfPad[first] else { continue }
                names.pinNets["\(chip.id.uuidString)/\(designator)"] = net
            }
        }
        for connection in netlist.connections {
            let key = "\(chips[connection.part].id.uuidString)/\(connection.pin)"
            if names.pinNets[key] == nil { names.pinNets[key] = merged(connection.net) }
        }
        let resolve = names.net(of:)
        for (n, hole) in holes.enumerated() where hole.plated {
            if let net = hole.net.flatMap(resolve) { netOfPad[holeBase + n] = net }
        }
        names.netNames = netNames(netlist: netlist, merged: merged, pads: pads, netOfPad: netOfPad,
                                  nets: Set(netOfPad.values).union(names.railNets.values).union(names.pinNets.values))
        for (mv, net) in names.railNets.sorted(by: { $0.key < $1.key }) where names.netKeys[net] == nil {
            names.netKeys[net] = .rail(mv: mv)
        }
        var keyPad: [Int: (chip: Int, designator: String)] = [:]
        for (p, net) in netOfPad where names.netKeys[net] == nil {
            let pad = pads[p]
            guard pad.chip >= 0, let designator = pad.pad.designator else { continue }
            if let best = keyPad[net], best.chip < pad.chip || best.chip == pad.chip
                && best.designator.compare(designator, options: .numeric) != .orderedDescending { continue }
            keyPad[net] = (pad.chip, designator)
        }
        for (net, pad) in keyPad { names.netKeys[net] = .pin(chip: chips[pad.chip].id, designator: pad.designator) }

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
            for (pad, net) in netOfPad.sorted(by: { $0.key > $1.key }) { netOfRoot[find(pad)] = net }
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
        var pinOfPad: [Int: String] = [:]
        for (key, list) in padsOfPin { for p in list { pinOfPad[p] = key } }
        return BoardConnectivity(pads: pads, traces: copper, vias: vias,
                                 root: parent.indices.map(find), netOfPad: netOfPad, planeNets: planeNets,
                                 railNets: names.railNets, pinNets: names.pinNets, netNames: names.netNames,
                                 netKeys: names.netKeys, pinOfPad: pinOfPad)
    }

    /// A name for each of `nets`, by this priority: a supply symbol's part
    /// name ("GND", "5V"; a rail of several symbols takes the first in chip
    /// order), else the name of a net label wired onto the net, else the
    /// name of a bus wire whose port pin is wired onto it, else its first
    /// pad as "U1.3" (lowest chip index, then designator in natural order;
    /// a net of board pads only takes its first, "Edge.A1"), else
    /// "Net <n>". A name used twice gets " (2)", " (3)", … in net order.
    private func netNames(netlist: SchematicNetlist, merged: (Int) -> Int, pads: [BoardConnectivity.Pad],
                          netOfPad: [Int: Int], nets: Set<Int>) -> [Int: String] {
        var name: [Int: String] = [:]
        func offer(_ net: Int, _ text: String) {
            if name[net] == nil, !text.isEmpty { name[net] = text }
        }
        // Supply symbols, in chip order.
        var netOfPin: [String: Int] = [:]
        for connection in netlist.connections { netOfPin["\(connection.part)/\(connection.pin)"] = connection.net }
        for (index, chip) in chips.enumerated() {
            guard case .supply(_, _, false) = chip.part.details else { continue }
            for pin in chip.part.pins {
                if let net = netOfPin["\(index)/\(pin.designator)"] { offer(merged(net), chip.partNumber) }
            }
        }
        // Net labels wired onto the net, in label order.
        var netOfLabel: [UUID: Int] = [:]
        var netOfBusWire: [UUID: Int] = [:]
        for wire in wires {
            guard let net = netlist.netOfWire[wire.id] else { continue }
            for end in [wire.start, wire.end] {
                switch end {
                case let .label(id): if netOfLabel[id] == nil { netOfLabel[id] = merged(net) }
                case let .busPin(ref): if netOfBusWire[ref.wireID] == nil { netOfBusWire[ref.wireID] = merged(net) }
                default: break
                }
            }
        }
        for label in netLabels where label.isNet {
            if let net = netOfLabel[label.id] { offer(net, label.name) }
        }
        // Bus wires whose port pins are wired onto the net.
        for bus in buses {
            for wire in bus.wires {
                if let net = netOfBusWire[wire.id] { offer(net, wire.name) }
            }
        }
        // The first pad.
        var firstPad: [Int: Int] = [:]
        func earlier(_ a: Int, _ b: Int) -> Bool {
            let p = pads[a], q = pads[b]
            if (p.chip < 0) != (q.chip < 0) { return p.chip >= 0 }
            if p.chip != q.chip { return p.chip < q.chip }
            let x = p.pad.designator ?? "", y = q.pad.designator ?? ""
            if x != y { return x.compare(y, options: .numeric) == .orderedAscending }
            return a < b
        }
        for (pad, net) in netOfPad where firstPad[net].map({ earlier(pad, $0) }) ?? true { firstPad[net] = pad }
        for (net, pad) in firstPad { offer(net, pads[pad].label) }

        var used: [String: Int] = [:]
        var result: [Int: String] = [:]
        for net in nets.sorted() {
            let base = name[net] ?? "Net \(net + 1)"
            let count = (used[base] ?? 0) + 1
            used[base] = count
            result[net] = count == 1 ? base : "\(base) (\(count))"
        }
        return result
    }

    /// Whether a circle meets a rectangle.
    /// Whether a trace's centre line a-b reaches a pad's copper: its box, or
    /// for a round pad its ellipse (a line past the corner of a round pad's
    /// box does not touch the pad).
    static func segment(_ a: CGPoint, _ b: CGPoint, touches pad: PlacedPad) -> Bool {
        guard segment(a, b, touches: pad.rect) else { return false }
        guard pad.shape == .round, pad.rect.width > 0, pad.rect.height > 0 else { return true }
        // Scaled so the ellipse is the unit circle.
        func unit(_ p: CGPoint) -> CGPoint {
            CGPoint(x: (p.x - pad.rect.midX) / (pad.rect.width / 2), y: (p.y - pad.rect.midY) / (pad.rect.height / 2))
        }
        return distance(from: .zero, toSegment: unit(a), unit(b)) <= 1
    }

    static func circle(_ centre: CGPoint, _ radius: CGFloat, meets rect: CGRect) -> Bool {
        let x = min(max(centre.x, rect.minX), rect.maxX), y = min(max(centre.y, rect.minY), rect.maxY)
        return hypot(centre.x - x, centre.y - y) <= radius
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
        // Each group still outside the tree keeps its shortest line to the
        // tree, and only the group just added is measured against it. A tie
        // stays with the group that joined the tree first, then the group
        // first in `groups`, so the lines are the same every run.
        var rest = Array(groups.indices.dropFirst())
        var nearest = rest.map { closest(groups[0], groups[$0]) }
        var lines: [(CGPoint, CGPoint)] = []
        while !rest.isEmpty {
            var best: Int?
            for i in rest.indices where nearest[i].2 < (best.map { nearest[$0].2 } ?? .infinity) { best = i }
            guard let best else { break }
            lines.append((nearest[best].0, nearest[best].1))
            let added = rest.remove(at: best)
            nearest.remove(at: best)
            for i in rest.indices {
                let line = closest(groups[added], groups[rest[i]])
                if line.2 < nearest[i].2 { nearest[i] = line }
            }
        }
        return lines
    }
}

/// A box round a pad or a trace segment, for ruling out copper too far
/// away to touch it before the exact tests.
nonisolated struct PointBox {
    var minX: CGFloat, minY: CGFloat, maxX: CGFloat, maxY: CGFloat

    init(minX: CGFloat, minY: CGFloat, maxX: CGFloat, maxY: CGFloat) {
        (self.minX, self.minY, self.maxX, self.maxY) = (minX, minY, maxX, maxY)
    }

    /// The box round a segment.
    init(_ a: CGPoint, _ b: CGPoint) {
        self.init(minX: min(a.x, b.x), minY: min(a.y, b.y), maxX: max(a.x, b.x), maxY: max(a.y, b.y))
    }

    init(_ rect: CGRect) {
        self.init(minX: rect.minX, minY: rect.minY, maxX: rect.maxX, maxY: rect.maxY)
    }

    /// Whether `other` comes within `distance` of this box, edges included.
    func reaches(_ other: PointBox, within distance: CGFloat) -> Bool {
        other.minX <= maxX + distance && other.maxX >= minX - distance
            && other.minY <= maxY + distance && other.maxY >= minY - distance
    }

}

/// Boxes sorted into a coarse grid over the area they cover, to find the
/// ones near another box without looking at them all.
nonisolated struct BoxGrid {
    private static let side = 32
    private let originX: CGFloat, originY: CGFloat, cellWidth: CGFloat, cellHeight: CGFloat
    /// The indices of the boxes touching each cell, ascending.
    private var cells: [[Int]]

    init(_ boxes: [PointBox]) {
        var all = boxes.first ?? PointBox(minX: 0, minY: 0, maxX: 0, maxY: 0)
        for box in boxes {
            all.minX = min(all.minX, box.minX); all.maxX = max(all.maxX, box.maxX)
            all.minY = min(all.minY, box.minY); all.maxY = max(all.maxY, box.maxY)
        }
        originX = all.minX
        originY = all.minY
        cellWidth = max((all.maxX - all.minX) / CGFloat(Self.side), 1e-3)
        cellHeight = max((all.maxY - all.minY) / CGFloat(Self.side), 1e-3)
        cells = Array(repeating: [], count: Self.side * Self.side)
        for (index, box) in boxes.enumerated() {
            let span = span(of: box, within: 0)
            for row in span.rows {
                for column in span.columns { cells[row * Self.side + column].append(index) }
            }
        }
    }

    /// The cells a box grown by `distance` covers. A box off the grid
    /// lands in the edge cells, the same way for a stored box and a
    /// search, so boxes that overlap always share a cell.
    private func span(of box: PointBox, within distance: CGFloat) -> (columns: ClosedRange<Int>, rows: ClosedRange<Int>) {
        func cell(_ value: CGFloat) -> Int { Int(min(max(value.rounded(.down), 0), CGFloat(Self.side - 1))) }
        return (cell((box.minX - distance - originX) / cellWidth)...cell((box.maxX + distance - originX) / cellWidth),
                cell((box.minY - distance - originY) / cellHeight)...cell((box.maxY + distance - originY) / cellHeight))
    }

    /// The indices of every stored box that might come within `distance`
    /// of `box` (and some that don't), ascending, each once.
    func near(_ box: PointBox, within distance: CGFloat) -> [Int] {
        let span = span(of: box, within: distance)
        if span.columns.count == 1, span.rows.count == 1 {
            return cells[span.rows.lowerBound * Self.side + span.columns.lowerBound]
        }
        var found: [Int] = []
        for row in span.rows {
            for column in span.columns { found += cells[row * Self.side + column] }
        }
        found.sort()
        var unique: [Int] = []
        unique.reserveCapacity(found.count)
        for index in found where unique.last != index { unique.append(index) }
        return unique
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
    /// Each supply rail's net, by voltage in mV (0 is ground).
    var railNets: [UInt32: Int] = [:]
    /// The net of each part pin on one, by "<chip UUID>/<designator>": the
    /// net of its pads, or the net its schematic pin is wired into.
    var pinNets: [String: Int] = [:]
    /// Each net's display name (`SchematicDocument.netNames`), unique.
    var netNames: [Int: String] = [:]
    /// Each net's stable name (`planeNet(ofNet:)`).
    var netKeys: [Int: PlaneNet] = [:]
    /// The pin each pad is of ("<chip index>/<designator>", "edge/<name>"):
    /// the pads of one pin are one pin however many there are. A pad of no
    /// pin (an unnumbered centre pad, a hole) is a pin of its own.
    var pinOfPad: [Int: String] = [:]

    /// How many pins net `net` has on the board.
    func pinCount(ofNet net: Int) -> Int {
        Set(netOfPad.filter { $0.value == net }.keys.map { pinOfPad[$0] ?? "pad/\($0)" }).count
    }

    /// The stable name of net `net`, the one a class or group member uses
    /// for it: `.rail(mv)` for a supply rail (the lowest voltage, if one
    /// net is several), else the `.pin` of its first numbered part pad (by
    /// chip index, then designator in natural order). Nil for a net with no
    /// such pad and no rail.
    func planeNet(ofNet net: Int) -> PlaneNet? { netKeys[net] }

    /// The net a Plane, plated hole or net class member names, if the
    /// schematic still has it.
    func net(of name: PlaneNet) -> Int? {
        switch name {
        case let .rail(mv): railNets[mv]
        case let .pin(chip, designator): pinNets["\(chip.uuidString)/\(designator)"]
        }
    }

    func traceNode(_ index: Int) -> Int { pads.count + index }

    /// Each group's net, by root: that of a pad in it. A group reaching no
    /// pad on a net has none.
    func netOfRoot() -> [Int: Int] {
        var found: [Int: Int] = [:]
        // The lowest pad's net wins where copper shorts two nets, so it is
        // the same every run.
        for (pad, net) in netOfPad.sorted(by: { $0.key > $1.key }) { found[root[pad]] = net }
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
