import CoreGraphics
import Foundation

/// One piece of copper a pour reads, named the way `BoardConnectivity` names
/// its nodes: a pad by its index, a trace or via by its id.
nonisolated enum PourNode: Hashable, Sendable {
    case pad(Int), trace(UUID), via(UUID)
}

/// The copper a pour is filled round (plan-52-C): one pad, via, trace or
/// bare hole, in board mm. Values only, so the fill needs no document.
nonisolated struct PourCopper: @unchecked Sendable {
    var node: PourNode
    /// The schematic net it is on; nil is on no net.
    var net: Int?
    /// The layers it is on; nil is every layer (a through-hole pad, a via).
    var layers: Set<CopperLayer>?
    /// The metal.
    var shape: CGPath
    /// The points that decide whether a piece of fill touches it.
    var samples: [CGPoint]
    /// A plated pad's rect: a pad of the pour's net gets a ring and spokes round it.
    var padRect: CGRect?
    /// A non-plated hole: cut away by the copper-to-edge distance, never joined.
    var isHole = false

    /// A plated pad whose metal is its rectangle.
    static func pad(_ index: Int, rect: CGRect, net: Int?, layers: Set<CopperLayer>? = nil,
                    shape: CGPath? = nil) -> PourCopper {
        PourCopper(node: .pad(index), net: net, layers: layers, shape: shape ?? CGPath(rect: rect, transform: nil),
                   samples: rectSamples(rect), padRect: rect)
    }

    static func via(_ id: UUID, centre: CGPoint, diameter: Double, net: Int?) -> PourCopper {
        let r = diameter / 2
        let rect = CGRect(x: centre.x - r, y: centre.y - r, width: diameter, height: diameter)
        let rim = [CGPoint(x: centre.x + r, y: centre.y), CGPoint(x: centre.x - r, y: centre.y),
                   CGPoint(x: centre.x, y: centre.y + r), CGPoint(x: centre.x, y: centre.y - r)]
        return PourCopper(node: .via(id), net: net, layers: nil, shape: CGPath(ellipseIn: rect, transform: nil),
                          samples: [centre] + rim, padRect: nil)
    }

    static func trace(_ id: UUID, points: [CGPoint], width: Double, layer: CopperLayer, net: Int?) -> PourCopper {
        let line = CGMutablePath()
        if let first = points.first {
            line.move(to: first)
            for point in points.dropFirst() { line.addLine(to: point) }
            if points.count == 1 { line.addLine(to: first) }
        }
        let metal = line.copy(strokingWithWidth: width, lineCap: .round, lineJoin: .round, miterLimit: 1)
        var samples = points
        for (a, b) in zip(points, points.dropFirst()) { samples.append(CGPoint(x: (a.x + b.x) / 2, y: (a.y + b.y) / 2)) }
        return PourCopper(node: .trace(id), net: net, layers: [layer], shape: metal, samples: samples, padRect: nil)
    }

    /// A non-plated hole (a drill) as a bare rect.
    static func hole(_ index: Int, rect: CGRect, shape: CGPath? = nil) -> PourCopper {
        PourCopper(node: .pad(index), net: nil, layers: nil, shape: shape ?? CGPath(rect: rect, transform: nil),
                   samples: [], padRect: nil, isHole: true)
    }

    /// The centre, the four corners and the four edge midpoints.
    static func rectSamples(_ rect: CGRect) -> [CGPoint] {
        [CGPoint(x: rect.midX, y: rect.midY),
         CGPoint(x: rect.minX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.minY),
         CGPoint(x: rect.maxX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.maxY),
         CGPoint(x: rect.midX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.midY),
         CGPoint(x: rect.midX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.midY)]
    }
}

/// What a pour holds: the pieces of its rectangle that are left once the
/// other nets' copper, the keep-outs and the edge are cut away, and which
/// pads, vias and traces of its net each piece touches.
nonisolated struct PourFill: @unchecked Sendable {
    let pour: UUID
    /// The kept pieces, each its outer loop and its holes.
    let pieces: [CGPath]
    /// `joined[i]`: the nodes of the pour's net that touch `pieces[i]`, sorted.
    let joined: [[PourNode]]
    /// The union of the pieces, for drawing.
    let path: CGPath

    var isEmpty: Bool { pieces.isEmpty }
}

/// The pour fill (plan-52-C): a pure function of the pours, keep-outs and
/// copper. See `planning/plan-52-C-pour-fill.md` and `planning/plan-52-data`
/// for what it was measured against.
nonisolated enum PourFiller {
    /// How many times `fill` has run; tests read it to see when a refill happens.
    nonisolated(unsafe) static var fillCount = 0

    /// The fill of every pour that has a resolved net (`nets[pour.id]`), in
    /// drawing order: each pour gives way to the rectangle of an earlier pour
    /// of another net on its layer.
    static func fill(pours: [BoardPour], nets: [UUID: Int], keepouts: [BoardKeepout], copper: [PourCopper],
                     outline: [CGPoint], edge: Double, rules: DesignRules) -> [UUID: PourFill] {
        fillCount += 1
        var result: [UUID: PourFill] = [:]
        guard outline.count >= 3, pours.contains(where: { nets[$0.id] != nil }) else { return result }

        // The board, pulled back from its edge.
        let board = CGMutablePath()
        board.addLines(between: outline)
        board.closeSubpath()
        var inner: CGPath = board
        if edge > 0 {
            let ring = board.copy(strokingWithWidth: 2 * edge, lineCap: .butt, lineJoin: .round, miterLimit: 1)
            inner = board.subtracting(ring, using: .winding)
        }

        var earlier: [(rect: CGRect, net: Int, layer: CopperLayer)] = []
        for pour in pours {
            guard let net = nets[pour.id] else { continue }
            defer { earlier.append((pour.rect, net, pour.layer)) }
            let clearance = pour.clearance(rules: rules)
            let region = CGPath(rect: pour.rect, transform: nil).intersection(inner, using: .winding)

            var cuts: [CGPath] = []
            var rings: [CGPath] = [], spokes: [CGPath] = []
            var same: [PourCopper] = []
            for item in copper where item.layers?.contains(pour.layer) ?? true {
                if item.isHole {
                    cuts.append(grown(item.shape, by: edge))
                } else if item.net == net {
                    same.append(item)
                    if pour.connection == .thermal, let rect = item.padRect {
                        rings.append(grown(item.shape, by: pour.thermalGap))
                        let gap = pour.thermalGap, width = pour.spokeWidth
                        spokes.append(CGPath(rect: CGRect(x: rect.minX - gap - 0.05, y: rect.midY - width / 2,
                                                          width: rect.width + 2 * gap + 0.1, height: width), transform: nil))
                        spokes.append(CGPath(rect: CGRect(x: rect.midX - width / 2, y: rect.minY - gap - 0.05,
                                                          width: width, height: rect.height + 2 * gap + 0.1), transform: nil))
                    }
                } else {
                    cuts.append(grown(item.shape, by: clearance))
                }
            }
            for keepout in keepouts where keepout.applies(to: pour.layer) {
                cuts.append(CGPath(rect: keepout.rect, transform: nil))
            }
            for other in earlier where other.layer == pour.layer && other.net != net {
                cuts.append(CGPath(rect: other.rect.insetBy(dx: -clearance, dy: -clearance), transform: nil))
            }
            if !rings.isEmpty {
                cuts.append(combine(rings).subtracting(combine(spokes), using: .winding))
            }

            let filled = cuts.isEmpty ? region : region.subtracting(combine(cuts), using: .winding)
            var pieces: [CGPath] = [], joined: [[PourNode]] = []
            for piece in filled.componentsSeparated(using: .winding) {
                let box = piece.boundingBoxOfPath
                if box.isNull || min(box.width, box.height) < pour.minWidth { continue }
                var touching: [PourNode] = []
                for item in same where item.samples.contains(where: { box.contains($0) && piece.contains($0, using: .winding) }) {
                    touching.append(item.node)
                }
                guard !touching.isEmpty else { continue }
                pieces.append(piece)
                joined.append(touching.sorted { "\($0)" < "\($1)" })
            }
            result[pour.id] = PourFill(pour: pour.id, pieces: pieces, joined: joined, path: combine(pieces))
        }
        return result
    }

    /// `path` grown by `distance` all round: the path and its round-joined stroke, twice as wide.
    static func grown(_ path: CGPath, by distance: Double) -> CGPath {
        guard distance > 0 else { return path }
        let stroke = path.copy(strokingWithWidth: 2 * distance, lineCap: .round, lineJoin: .round, miterLimit: 1)
        return path.union(stroke, using: .winding)
    }

    private static func combine(_ paths: [CGPath]) -> CGPath {
        let all = CGMutablePath()
        for path in paths { all.addPath(path) }
        return all
    }
}
