import CoreGraphics
import Foundation
@testable import schematical

extension RoutingOptions {
    /// What the tests route the example boards with: the defaults — but
    /// Fast in an unoptimised build, where Balanced (several planned and
    /// improved runs) takes many minutes on AFU. Balanced and Thorough on
    /// the examples are `RouterQualityFixtureTests`, in an optimised build.
    static var fixture: RoutingOptions {
        var options = RoutingOptions.standard
        #if DEBUG
        options.strategy = .fast
        #endif
        return options
    }
}

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
    /// Cutouts of the board, each a loop (mm).
    var cutouts: [[CGPoint]] = []
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
    /// Each net's group, if in one.
    var groupOf: [Int: UUID] = [:]
    /// Nets left out of `nets` (their pads are obstacles only).
    var unrouted: Set<Int> = []
    var impossible: [Int: String] = [:]
    var nameOf: [Int: String] = [:]
    /// Existing copper push and shove may move (plan-66-C): see `chain`.
    var chains: [MovableChain] = []

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
                                             net: net, kind: .trace))
        }
    }

    /// A movable chain of one run on `layer` along `points`: its ends are the
    /// pads `from` and `to` (indices into `pads`), or a free end at the first
    /// or last point when nil. The net is never routed (as the copper of a net
    /// the run leaves alone), and has a bystander net of this width.
    mutating func chain(_ points: [CGPoint], width: Double = 0.254, layer: CopperLayer = .top, net: Int,
                        from: Int? = nil, to: Int? = nil) {
        func end(_ point: CGPoint, _ pad: Int?) -> MovableChain.End {
            var anchor = RoutingComponent()
            if let pad {
                anchor.pads = [shape(pads[pad], node: pad)]
                anchor.padLabels = [pads[pad].label]
            } else {
                anchor.pads = [CopperShape(core: .segment(point, point), radius: options.grid, layers: [layer], node: -2)]
                anchor.padLabels = ["junction"]
            }
            return MovableChain.End(point: point, layer: layer, anchor: anchor)
        }
        let run = MovableChain.Run(trace: UUID(), layer: layer, first: 0, last: points.count - 1, reversed: false, points: points)
        chains.append(MovableChain(id: chains.count, net: net, width: width, viaDrill: 0, viaDiameter: 0, runs: [run], vias: [],
                                   ends: [end(points[0], from), end(points[points.count - 1], to)]))
        unrouted.insert(net)
    }

    /// This board with each of its chains as a kept trace instead (the
    /// copper as fixed obstacles, as push and shove leaves it off).
    static func chainAsObstacle(_ board: RoutingTestBoard) -> RoutingTestBoard {
        var result = board
        result.chains = []
        for chain in board.chains {
            for run in chain.runs { result.trace(run.points, width: chain.width, layer: run.layer, net: chain.net) }
        }
        return result
    }

    /// This board with each of its kept traces (of a net) as a chain with
    /// free ends instead, one per segment.
    func tracesAsChains() -> RoutingTestBoard {
        var result = self
        result.obstacles = []
        for obstacle in obstacles {
            guard obstacle.kind == .trace, let net = obstacle.net, case let .segment(a, b) = obstacle.shape.core,
                  let layer = obstacle.shape.layers?.first else {
                result.obstacles.append(obstacle)
                continue
            }
            result.chain([a, b], width: obstacle.shape.radius * 2, layer: layer, net: net)
        }
        return result
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
                                             net: nil, kind: .hole,
                                             drill: CopperShape(core: .segment(hole.centre, hole.centre),
                                                                radius: hole.drill / 2, layers: nil, node: 5_000 + h)))
        }
        obstacles += self.obstacles
        for (a, b) in BoardEdge(outline: outline, cutouts: cutouts).segments {
            obstacles.append(RoutingObstacle(shape: CopperShape(core: .segment(a, b), radius: 0, layers: nil, node: -1),
                                             net: nil, kind: .outline))
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
                              group: groupOf[id], impossible: impossible[id],
                              pins: pads.filter { $0.net == id }.count)
        }
        // A bystander net of rules for every net that has a chain.
        var bystanders: [RoutingNet] = []
        for id in Set(chains.map(\.net)).sorted() {
            let mine = chains.filter { $0.net == id }
            bystanders.append(RoutingNet(id: id, name: nameOf[id] ?? "N\(id)", key: .pin(chip: UUID(), designator: "\(id)"),
                                         width: mine[0].width, clearance: clearance[id] ?? rules.minClearance,
                                         viaDrill: viaDrill[id] ?? 0.4, viaDiameter: viaDiameter[id] ?? 0.8,
                                         allowed: Set(routable), preferred: [], components: [], planeLayer: nil, group: nil,
                                         impossible: nil, pins: pads.filter { $0.net == id }.count))
        }
        return RoutingProblem(outline: outline, cutouts: cutouts, layers: layers, routable: routable, planes: planes, rules: rules,
                              options: options, nets: nets, obstacles: obstacles, groups: groups,
                              removedTraces: [], removedVias: [], movable: chains, bystanders: bystanders)
    }
}

/// Boards shared by the router's tests and the regression anchors.
extension RoutingTestBoard {
    /// Two gaps in a wall of net 9, one layer, no vias: A (y 9.4–10.6) fits
    /// either net, B (y 3.6–4.4) only net 1, since net 2 keeps a 0.2 mm
    /// clearance. The single pass routes net 1 first (shorter), through A;
    /// net 2 can then only be routed by moving net 1 to B.
    static func twoGaps() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20, layers: [.top])
        board.options.grid = 0.1
        board.options.vias = .none
        board.pad(3, 10, 1, 1, net: 1)
        board.pad(17, 10, 1, 1, net: 1)
        board.pad(3, 13, 1, 1, net: 2)
        board.pad(17, 16, 1, 1, net: 2)
        board.clearance[2] = 0.2
        board.trace([CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 3.6)], net: 9)
        board.trace([CGPoint(x: 10, y: 4.4), CGPoint(x: 10, y: 9.4)], net: 9)
        board.trace([CGPoint(x: 10, y: 10.6), CGPoint(x: 10, y: 20)], net: 9)
        return board
    }

    /// Six nets from the left to the right in reversed order, two layers.
    static func reversedChannel() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20)
        board.options.grid = 0.1
        for i in 0..<6 {
            board.pad(2, 4 + Double(i) * 2.4, 1, 1, net: i + 1)
            board.pad(18, 4 + Double(5 - i) * 2.4, 1, 1, net: i + 1)
        }
        // Walls above and below the channel.
        board.trace([CGPoint(x: 5, y: 2), CGPoint(x: 15, y: 2)], width: 0.5, net: 9)
        board.trace([CGPoint(x: 5, y: 18), CGPoint(x: 15, y: 18)], width: 0.5, net: 9)
        return board
    }

    /// Eight nets from a left column to a right column in the opposite
    /// order: every pair crosses.
    static func crossing() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 30, height: 24)
        board.options.grid = 0.127
        for i in 0..<8 {
            board.pad(4, 5 + 2 * Double(i), 1.2, 0.8, net: i + 1)
            board.pad(26, 19 - 2 * Double(i), 1.2, 0.8, net: i + 1)
        }
        return board
    }

    /// Three straight, unobstructed nets on Top: the first pass routes all of
    /// them and nothing is ever ripped up.
    static func parallelRows() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20)
        board.options.grid = 0.127
        for (net, y) in [(1, 10.0), (2, 6.0), (3, 14.0)] {
            board.pad(3, y, 1, 1, net: net)
            board.pad(17, y, 1, 1, net: net)
        }
        return board
    }

    /// One layer, no vias. Net 2 from (3, 6) to (17, 6) must cross a wall of net 9 at x = 10 through a
    /// gap (y 9.4 to 10.6) that the existing copper of net 5, from (3, 10) to (17, 10), sits in (plan-66-D).
    /// The wall's copper ends at 9.527 and 10.473, so a trace centre may sit between 9.754 and 10.246 (width
    /// 0.254, clearance 0.1); two traces need centres 0.354 apart. The chain at y = 10 leaves none; moved to
    /// 10.2 it leaves 9.8. Net 5 is a chain of existing copper (`ledgerShoveGap` has the run route it).
    /// Net 2's pads are both on one side of net 5's line: on one layer a net that has to cross it could
    /// never be laid beside it.
    static func shoveGap(gap: ClosedRange<Double> = 9.4...10.6) -> RoutingTestBoard {
        var board = ledgerShoveGap(gap: gap)
        board.chain([CGPoint(x: 3, y: 10), CGPoint(x: 17, y: 10)], net: 5, from: 2, to: 3)
        return board
    }

    /// `shoveGap` with net 5 routed by the run (its pads two components of a net the run routes), no chain.
    static func ledgerShoveGap(gap: ClosedRange<Double> = 9.4...10.6) -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20, layers: [.top])
        board.options.grid = 0.1
        board.options.vias = .none
        board.pad(3, 6, 1, 1, net: 2)
        board.pad(17, 6, 1, 1, net: 2)
        board.pad(3, 10, 1, 1, net: 5)
        board.pad(17, 10, 1, 1, net: 5)
        board.trace([CGPoint(x: 10, y: 0), CGPoint(x: 10, y: gap.lowerBound)], net: 9)
        board.trace([CGPoint(x: 10, y: gap.upperBound), CGPoint(x: 10, y: 20)], net: 9)
        return board
    }
}
