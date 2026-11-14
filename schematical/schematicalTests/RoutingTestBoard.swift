import CoreGraphics
import Foundation
@testable import schematical

/// A hand-built routing problem for engine tests: a rectangle board, pads
/// (each its own component unless given one), holes, and the nets' rules.
/// Net ids are small integers; every net with a pad is routed.
struct RoutingTestBoard {
    var boardWidth = 20.0
    var boardHeight = 20.0
    var layers: [CopperLayer] = [.top, .bottom]
    var rules = DesignRules.customDefault
    var options = RoutingOptions()

    struct Pad {
        var rect: CGRect
        var net: Int?
        /// Nil: through-hole (every layer).
        var layers: Set<CopperLayer>?
        var component: Int?
        var exemptFromEdge = false
        var label: String
    }

    var pads: [Pad] = []
    var holes: [(centre: CGPoint, drill: Double)] = []
    /// Extra obstacles (kept traces of other nets and so on).
    var obstacles: [RoutingObstacle] = []
    /// Per-net rule changes: width (mm), clearance, via, layers.
    var width: [Int: Double] = [:]
    var clearance: [Int: Double] = [:]
    var viaDiameter: [Int: Double] = [:]
    var viaDrill: [Int: Double] = [:]
    var allowed: [Int: Set<CopperLayer>] = [:]
    var preferred: [Int: Set<CopperLayer>] = [:]
    var planeLayer: [Int: CopperLayer] = [:]
    var planes: [CopperLayer: Int] = [:]
    var groups: [RoutingGroup] = []
    /// Nets left out of `nets` (their pads are obstacles only).
    var unrouted: Set<Int> = []
    var impossible: [Int: String] = [:]
    var nameOf: [Int: String] = [:]

    init(width: Double = 20, height: Double = 20, layers: [CopperLayer] = [.top, .bottom]) {
        boardWidth = width
        boardHeight = height
        self.layers = layers
    }

    /// A pad, its rect centred on (x, y).
    @discardableResult
    mutating func pad(_ x: Double, _ y: Double, _ w: Double, _ h: Double, net: Int?,
                      layers: Set<CopperLayer>? = [.top], component: Int? = nil, exempt: Bool = false) -> Int {
        pads.append(Pad(rect: CGRect(x: x - w / 2, y: y - h / 2, width: w, height: h), net: net, layers: layers,
                        component: component, exemptFromEdge: exempt, label: "P\(pads.count + 1)"))
        return pads.count - 1
    }

    mutating func hole(_ x: Double, _ y: Double, drill: Double) {
        holes.append((CGPoint(x: x, y: y), drill))
    }

    /// A kept trace of net `net` (an obstacle only).
    mutating func trace(_ points: [CGPoint], width: Double = 0.254, layer: CopperLayer = .top, net: Int?) {
        for (a, b) in zip(points, points.dropFirst()) {
            obstacles.append(RoutingObstacle(shape: CopperShape(core: .segment(a, b), radius: width / 2,
                                                                layers: [layer], node: 10_000 + obstacles.count),
                                             net: net))
        }
    }

    func shape(_ pad: Pad, node: Int) -> CopperShape {
        CopperShape(core: .rect(pad.rect), radius: 0, layers: pad.layers, node: node)
    }

    var outline: [CGPoint] {
        [CGPoint(x: 0, y: 0), CGPoint(x: boardWidth, y: 0), CGPoint(x: boardWidth, y: boardHeight), CGPoint(x: 0, y: boardHeight)]
    }

    func problem() -> RoutingProblem {
        let routable = options.enabled(layers)
        var obstacles: [RoutingObstacle] = pads.enumerated().map { p, pad in
            RoutingObstacle(shape: shape(pad, node: p), net: pad.net, exemptFromEdge: pad.exemptFromEdge)
        }
        for (h, hole) in holes.enumerated() {
            obstacles.append(RoutingObstacle(shape: CopperShape(core: .segment(hole.centre, hole.centre),
                                                                radius: hole.drill / 2, layers: nil, node: 5_000 + h),
                                             net: nil))
        }
        obstacles += self.obstacles
        for (a, b) in zip(outline, outline.dropFirst() + outline.prefix(1)) {
            obstacles.append(RoutingObstacle(shape: CopperShape(core: .segment(a, b), radius: 0, layers: nil, node: -1),
                                             net: nil))
        }
        let netIDs = Set(pads.compactMap(\.net)).subtracting(unrouted).sorted()
        let nets: [RoutingNet] = netIDs.map { id in
            var components: [Int: RoutingComponent] = [:]
            var order: [Int] = []
            for (p, pad) in pads.enumerated() where pad.net == id {
                let key = pad.component ?? (1_000_000 + p)
                if components[key] == nil { order.append(key); components[key] = RoutingComponent() }
                components[key]!.pads.append(shape(pad, node: p))
                components[key]!.padLabels.append(pad.label)
            }
            let allow = (allowed[id] ?? Set(routable)).intersection(routable)
            return RoutingNet(id: id, name: nameOf[id] ?? "N\(id)", key: .pin(chip: UUID(), designator: "\(id)"),
                              width: width[id] ?? 0.254, clearance: clearance[id] ?? rules.minClearance,
                              viaDrill: viaDrill[id] ?? 0.4, viaDiameter: viaDiameter[id] ?? 0.8,
                              allowed: allow, preferred: (preferred[id] ?? []).intersection(allow),
                              components: order.map { components[$0]! }, planeLayer: planeLayer[id],
                              group: nil, impossible: impossible[id])
        }
        return RoutingProblem(outline: outline, layers: layers, routable: routable, planes: planes, rules: rules,
                              options: options, nets: nets, obstacles: obstacles, groups: groups,
                              removedTraces: [], removedVias: [])
    }
}
