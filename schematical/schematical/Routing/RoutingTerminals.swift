import CoreGraphics
import Foundation

/// A grid node a connection may start or end at, and the exact point its
/// trace ties to there: a pad's centre, the nearest point of a kept
/// trace's segment (a trace joins another only on its line), a kept via's
/// centre. Nil: the node is on new copper and needs no tie.
nonisolated struct RoutingTerminal: Hashable, Sendable {
    var node: Int
    var tie: CGPoint?
}

extension RoutingGrid {
    /// The terminals of `component` for net `net`, on the layers `allowed`
    /// marks, sorted by node: every cell centre inside a pad's copper (a
    /// through-hole pad's on every layer) that the net may use — or, for a
    /// pad too small to hold one, the usable cells of the 4 nearest its
    /// centre; the cells a kept trace's segments run through; a kept via's
    /// cell on every allowed layer.
    func terminals(of component: RoutingComponent, net: Int, profile: Int, allowed: [Bool]) -> [RoutingTerminal] {
        var found: [Int: CGPoint] = [:]
        var order: [Int] = []
        func add(_ node: Int, _ tie: CGPoint) {
            if found[node] == nil { found[node] = tie; order.append(node) }
        }
        for shape in component.pads {
            let layerList = layerIndices(shape.layers).filter { allowed[$0] }
            guard !layerList.isEmpty else { continue }
            let box = shape.bounds
            let centre = CGPoint(x: box.midX, y: box.midY)
            var inside: [Int] = []
            if let range = cellRange(box, margin: 0) {
                for y in range.y {
                    for x in range.x {
                        let c = cell(x, y)
                        if Self.distance(point(c), shape.core) <= shape.radius + 1e-9 { inside.append(c) }
                    }
                }
            }
            var placed = false
            for layer in layerList {
                for c in inside where isPassable(c, layer: layer, net: net, profile: profile) {
                    add(layer * cellCount + c, centre)
                    placed = true
                }
            }
            if !placed {
                let fx = (centre.x - origin.x) / pitch, fy = (centre.y - origin.y) / pitch
                let xs = [Int(fx.rounded(.down)), Int(fx.rounded(.up))], ys = [Int(fy.rounded(.down)), Int(fy.rounded(.up))]
                for y in ys where y >= 0 && y < ny {
                    for x in xs where x >= 0 && x < nx {
                        for layer in layerList where isPassable(cell(x, y), layer: layer, net: net, profile: profile) {
                            add(layer * cellCount + cell(x, y), centre)
                        }
                    }
                }
            }
        }
        for trace in component.traces {
            guard let layer = layerIndex(trace.layer), allowed[layer] else { continue }
            for (a, b) in zip(trace.points, trace.points.dropFirst()) {
                let box = CopperShape(core: .segment(a, b), radius: 0, layers: nil, node: 0).bounds
                guard let range = cellRange(box, margin: pitch) else { continue }
                for y in range.y {
                    for x in range.x {
                        let c = cell(x, y)
                        let p = point(c)
                        guard SchematicDocument.distance(from: p, toSegment: a, b) <= pitch * 0.5 + 1e-9,
                              isPassable(c, layer: layer, net: net, profile: profile) else { continue }
                        add(layer * cellCount + c, Self.project(p, a, b))
                    }
                }
            }
        }
        for via in component.vias {
            let c = nearestCell(via.centre)
            for layer in layers.indices where allowed[layer] && isPassable(c, layer: layer, net: net, profile: profile) {
                add(layer * cellCount + c, via.centre)
            }
        }
        return order.sorted().map { RoutingTerminal(node: $0, tie: found[$0]) }
    }

    /// The point of segment a–b nearest `p`.
    static func project(_ p: CGPoint, _ a: CGPoint, _ b: CGPoint) -> CGPoint {
        let dx = b.x - a.x, dy = b.y - a.y
        let length = dx * dx + dy * dy
        guard length > 0 else { return a }
        let t = min(max(((p.x - a.x) * dx + (p.y - a.y) * dy) / length, 0), 1)
        return CGPoint(x: a.x + t * dx, y: a.y + t * dy)
    }

    /// The bounding box, in cell coordinates, of `nodes`.
    func cellBox(_ nodes: [Int]) -> (x: ClosedRange<Int>, y: ClosedRange<Int>)? {
        guard !nodes.isEmpty else { return nil }
        var x0 = Int.max, x1 = Int.min, y0 = Int.max, y1 = Int.min
        for node in nodes {
            let c = node % cellCount
            x0 = min(x0, c % nx); x1 = max(x1, c % nx)
            y0 = min(y0, c / nx); y1 = max(y1, c / nx)
        }
        return (x0...x1, y0...y1)
    }
}
