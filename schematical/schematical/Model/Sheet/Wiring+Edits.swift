import CoreGraphics
import Foundation

/// The group drag's edits to wires and junctions (plan-68-D).
extension Wiring {
    /// One frame of a group drag: each junction in `points` and each wire's
    /// bends in `bends` go to `shift` of where they were at the press.
    mutating func moveGroup(junctions points: [UUID: CGPoint], bends: [UUID: [CGPoint]],
                            by shift: (CGPoint) -> CGPoint) {
        for index in junctions.indices {
            guard let point = points[junctions[index].id] else { continue }
            let moved = shift(point)
            if junctions[index].point != moved { junctions[index].point = moved }
        }
        for index in wires.indices {
            guard let wireBends = bends[wires[index].id] else { continue }
            let moved = wireBends.map(shift)
            if wires[index].interior != moved { wires[index].interior = moved }
        }
    }

    /// Re-fits each wire in `paths` (its route at the press) to where its
    /// ends are now, keeping its shape (`OrthogonalRoute.refit`).
    mutating func refit(_ paths: [UUID: [CGPoint]], _ tips: PinTips) {
        for index in wires.indices {
            guard let route = paths[wires[index].id],
                  let start = endTip(wires[index].start, tips), let end = endTip(wires[index].end, tips) else { continue }
            let fitted = Array(OrthogonalRoute.refit(route, start: start, end: end).dropFirst().dropLast())
            if wires[index].interior != fitted { wires[index].interior = fitted }
        }
    }

    /// The end of a group drag: each wire in `affected` is re-fitted once;
    /// one in `orthogonal` (all right angles at the press) gets corners
    /// wherever the move left a slanted segment.
    mutating func settleGroup(affected: Set<UUID>, orthogonal: Set<UUID>, _ tips: PinTips) {
        for index in wires.indices where affected.contains(wires[index].id) {
            guard let start = endTip(wires[index].start, tips), let end = endTip(wires[index].end, tips) else { continue }
            var interior = OrthogonalRoute.settled(start: start, interior: wires[index].interior, end: end)
            if orthogonal.contains(wires[index].id) {
                interior = Array(OrthogonalRoute.normalize([start] + interior + [end]).dropFirst().dropLast())
            }
            wires[index].interior = interior
        }
    }
}
