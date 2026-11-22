import CoreGraphics
import Foundation

/// The design rules a router's new copper must meet, checked in value types
/// with the DRC's own geometry (`CopperGeometry`) and tolerance, so the
/// engine can check itself off the main actor (plan-38-H § 3 step 7). The
/// DRC stays the authority; parity tests hold this to it.
nonisolated enum RoutingVerifier {
    enum Kind: String, Hashable, Sendable {
        case clearance, edge, traceWidth, via, holeToHole, unconnected
    }

    /// A broken rule, and the new copper that breaks it (indices into the
    /// checked traces and vias).
    struct Violation: Hashable, Sendable {
        var kind: Kind
        var traces: [Int] = []
        var vias: [Int] = []
        var detail: String
    }

    /// The DRC's tolerance: differences under it are rounding.
    static let tolerance = 1e-4

    /// Checks `traces` and `vias` (new copper, each with its net) against
    /// each other and against `problem`'s fixed copper: clearance (the
    /// larger of the two nets' clearances, never under the DRC's), copper to
    /// edge (fingers and holes are fixed copper, never checked), trace width,
    /// via sizes, hole to hole, and that every trace end and via joins
    /// something.
    static func verify(_ problem: RoutingProblem, traces: [RoutedTrace], vias: [RoutedVia]) -> [Violation] {
        let rules = problem.rules
        var clearanceOf: [Int: Double] = [:]
        for net in problem.nets { clearanceOf[net.id] = max(net.clearance, rules.minClearance) }
        func clearance(_ net: Int?) -> Double { net.flatMap { clearanceOf[$0] } ?? rules.minClearance }

        // Every piece of copper; `node` indexes `owners`. New copper first.
        struct Owner { var net: Int?; var trace: Int?; var via: Int? }
        var shapes: [CopperShape] = []
        var owners: [Owner] = []
        func add(_ core: CopperShape.Core, _ radius: Double, _ layers: Set<CopperLayer>?, _ owner: Owner) {
            shapes.append(CopperShape(core: core, radius: radius, layers: layers, node: owners.count))
            owners.append(owner)
        }
        for (t, trace) in traces.enumerated() {
            for (a, b) in zip(trace.points, trace.points.dropFirst()) {
                add(.segment(a, b), trace.width / 2, [trace.layer], Owner(net: trace.net, trace: t))
            }
        }
        for (v, via) in vias.enumerated() {
            add(.segment(via.centre, via.centre), via.diameter / 2, nil, Owner(net: via.net, via: v))
        }
        let newCount = owners.count
        for obstacle in problem.obstacles where [.pad, .trace, .via].contains(obstacle.kind) {
            add(obstacle.shape.core, obstacle.shape.radius, obstacle.shape.layers, Owner(net: obstacle.net))
        }

        var violations: [Violation] = []

        // Clearance: new copper against all other copper of another net.
        let margin = (clearanceOf.values.max() ?? 0) + rules.minClearance
        CopperGeometry.nearPairs(shapes, margin: margin) { a, b in
            guard a.node < newCount || b.node < newCount, a.shares(b) else { return }
            let oa = owners[a.node], ob = owners[b.node]
            if let na = oa.net, let nb = ob.net, na == nb { return }
            let need = max(clearance(oa.net), clearance(ob.net))
            let gap = a.gap(to: b)
            guard gap < need - tolerance else { return }
            var v = Violation(kind: .clearance, detail: String(format: "gap %.4f mm; min %.4f mm", gap, need))
            for o in [oa, ob] {
                if let t = o.trace { v.traces.append(t) }
                if let i = o.via { v.vias.append(i) }
            }
            violations.append(v)
        }

        // Copper to the board's edge.
        let outline = problem.outline
        let edges = zip(outline, outline.dropFirst() + outline.prefix(1)).map {
            CopperShape(core: .segment($0, $1), radius: 0, layers: nil, node: -1)
        }
        for shape in shapes.prefix(newCount) {
            let gap = edges.map { shape.gap(to: $0) }.min() ?? .infinity
            let centre = CGPoint(x: shape.bounds.midX, y: shape.bounds.midY)
            guard !inside(centre, outline) || gap < rules.minCopperToEdge - tolerance else { continue }
            let owner = owners[shape.node]
            violations.append(Violation(kind: .edge, traces: owner.trace.map { [$0] } ?? [], vias: owner.via.map { [$0] } ?? [],
                                        detail: String(format: "%.4f mm from the edge", gap)))
        }

        // Copper to a non-plated hole: the copper-to-edge distance, as the DRC holds it.
        for obstacle in problem.obstacles where obstacle.kind == .hole {
            for shape in shapes.prefix(newCount) {
                let gap = shape.gap(to: obstacle.shape)
                guard gap < rules.minCopperToEdge - tolerance else { continue }
                let owner = owners[shape.node]
                violations.append(Violation(kind: .clearance, traces: owner.trace.map { [$0] } ?? [],
                                            vias: owner.via.map { [$0] } ?? [],
                                            detail: String(format: "gap %.4f mm to a non-plated hole; min %.4f mm", gap,
                                                           rules.minCopperToEdge)))
            }
        }

        for (t, trace) in traces.enumerated() where trace.width < rules.minTraceWidth - tolerance {
            violations.append(Violation(kind: .traceWidth, traces: [t], detail: "width \(trace.width) mm"))
        }
        for (v, via) in vias.enumerated()
        where via.drill < rules.minViaDrill - tolerance || via.diameter < rules.minViaDiameter - tolerance
            || (via.diameter - via.drill) / 2 < rules.minAnnularRing - tolerance {
            violations.append(Violation(kind: .via, vias: [v], detail: "drill \(via.drill), diameter \(via.diameter) mm"))
        }

        // Hole to hole: new vias' holes against every hole.
        var holes: [CopperShape] = []
        for via in vias {
            holes.append(CopperShape(core: .segment(via.centre, via.centre), radius: via.drill / 2, layers: nil,
                                     node: holes.count))
        }
        let newHoles = holes.count
        for obstacle in problem.obstacles {
            guard let drill = obstacle.drill else { continue }
            holes.append(CopperShape(core: drill.core, radius: drill.radius, layers: nil, node: holes.count))
        }
        CopperGeometry.nearPairs(holes, margin: rules.minHoleToHole) { a, b in
            guard a.node < newHoles || b.node < newHoles else { return }
            let gap = a.gap(to: b)
            guard gap < rules.minHoleToHole - tolerance else { return }
            violations.append(Violation(kind: .holeToHole, vias: [a.node, b.node].filter { $0 < newHoles },
                                        detail: String(format: "holes %.4f mm apart", gap)))
        }

        // Connections: each new trace end lands on a pad on its layer, a
        // via, or another trace on its layer; each new via is touched by 2
        // or more traces and pads.
        let pads = problem.obstacles.filter { $0.kind == .pad }
        // Each segment with its trace: a new trace's index, or a kept
        // trace's node (negative, so the two never meet).
        var otherTraces: [(layer: CopperLayer, width: Double, a: CGPoint, b: CGPoint, trace: Int)] = []
        for (t, trace) in traces.enumerated() {
            for (a, b) in zip(trace.points, trace.points.dropFirst()) { otherTraces.append((trace.layer, trace.width, a, b, t)) }
        }
        var allVias: [(centre: CGPoint, diameter: Double)] = vias.map { ($0.centre, $0.diameter) }
        for obstacle in problem.obstacles {
            guard case let .segment(a, b) = obstacle.shape.core else { continue }
            switch obstacle.kind {
            case .trace:
                if let layer = obstacle.shape.layers?.first {
                    otherTraces.append((layer, obstacle.shape.radius * 2, a, b, -1 - obstacle.shape.node))
                }
            case .via: allVias.append((a, obstacle.shape.radius * 2))
            default: break
            }
        }
        for (t, trace) in traces.enumerated() {
            for (end, point) in [("start", trace.points.first!), ("end", trace.points.last!)] {
                let onPad = pads.contains { pad in
                    (pad.shape.layers?.contains(trace.layer) ?? true)
                        && pad.shape.bounds.insetBy(dx: -tolerance, dy: -tolerance).contains(point)
                }
                let onVia = allVias.contains { hypot($0.centre.x - point.x, $0.centre.y - point.y) <= $0.diameter / 2 + trace.width / 2 }
                let onTrace = otherTraces.contains {
                    $0.trace != t && $0.layer == trace.layer
                        && SchematicDocument.distance(from: point, toSegment: $0.a, $0.b) < 1e-4
                }
                if !onPad && !onVia && !onTrace {
                    violations.append(Violation(kind: .unconnected, traces: [t],
                                                detail: String(format: "%@ at (%.4f, %.4f) joins nothing", end, point.x, point.y)))
                }
            }
        }
        for (v, via) in vias.enumerated() {
            let r = via.diameter / 2
            var touched = Set<Int>()
            for segment in otherTraces
            where SchematicDocument.distance(from: via.centre, toSegment: segment.a, segment.b) <= r + segment.width / 2 {
                touched.insert(segment.trace)
            }
            let padTouches = pads.filter { pad in
                let box = pad.shape.bounds
                let x = min(max(via.centre.x, box.minX), box.maxX), y = min(max(via.centre.y, box.minY), box.maxY)
                return hypot(via.centre.x - x, via.centre.y - y) <= r
            }.count
            let touching = touched.count + padTouches
            if touching < 2 {
                violations.append(Violation(kind: .unconnected, vias: [v], detail: "touched by \(touching)"))
            }
        }
        return violations
    }

    static func inside(_ point: CGPoint, _ outline: [CGPoint]) -> Bool {
        var inside = false
        var j = outline.count - 1
        for i in outline.indices {
            let a = outline[i], b = outline[j]
            if (a.y > point.y) != (b.y > point.y), point.x < (b.x - a.x) * (point.y - a.y) / (b.y - a.y) + a.x {
                inside.toggle()
            }
            j = i
        }
        return inside
    }
}
