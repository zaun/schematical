import CoreGraphics
import Foundation

/// A copper trace on the PCB: a polyline on one copper layer, every segment
/// at a multiple of 45°. Traces that touch on the same layer share a vertex
/// at each contact point, so connection is just shared vertices (see
/// `SchematicDocument.join`).
nonisolated struct BoardTrace: Identifiable, Hashable, Sendable {
    let id: UUID
    var layer: CopperLayer
    /// mil, within `widthRange`.
    var width: Int
    /// mm; at least 2, consecutive points distinct, every segment at a
    /// multiple of 45° (`TraceRoute.isLegal`). Not snapped to the grid: a
    /// crossing of two diagonals, or a pad centre, can be off it.
    var points: [CGPoint]

    static let widthRange = 5...100
    static let defaultWidth = 10

    /// The width in mm (1 mil = 0.0254 mm).
    var widthMM: Double { Double(width) * 0.0254 }

    /// Consecutive point pairs.
    var segments: [(CGPoint, CGPoint)] { Array(zip(points, points.dropFirst())) }
}

/// 45° routing for traces.
nonisolated enum TraceRoute {
    /// mm. Points closer than this are the same point.
    static let epsilon: CGFloat = 1e-6

    /// A route from `a` to `b` with at most one bend. When `a` and `b` are
    /// already on a 0°, 45° or 90° line it is the straight line. Otherwise
    /// it is a straight run along the longer axis plus a diagonal, the
    /// straight part first unless `diagonalFirst`.
    static func route(from a: CGPoint, to b: CGPoint, diagonalFirst: Bool) -> [CGPoint] {
        let dx = b.x - a.x, dy = b.y - a.y
        if isMultipleOf45(dx: dx, dy: dy) { return [a, b] }
        let d = min(abs(dx), abs(dy))
        let sx: CGFloat = dx < 0 ? -1 : 1, sy: CGFloat = dy < 0 ? -1 : 1
        let bend: CGPoint
        if diagonalFirst {
            bend = CGPoint(x: a.x + sx * d, y: a.y + sy * d)
        } else if abs(dx) > abs(dy) {
            bend = CGPoint(x: a.x + sx * (abs(dx) - d), y: a.y)
        } else {
            bend = CGPoint(x: a.x, y: a.y + sy * (abs(dy) - d))
        }
        return [a, bend, b]
    }

    /// Whether a segment of this run is at a multiple of 45°.
    static func isMultipleOf45(dx: CGFloat, dy: CGFloat) -> Bool {
        abs(dx) < epsilon || abs(dy) < epsilon || abs(abs(dx) - abs(dy)) < epsilon
    }

    /// At least 2 points, no zero-length segment, every segment at a
    /// multiple of 45°.
    static func isLegal(_ points: [CGPoint]) -> Bool {
        problem(points) == nil
    }

    /// Why `points` isn't a legal trace, as a file error detail after
    /// "trace <id> ", or nil.
    static func problem(_ points: [CGPoint]) -> String? {
        guard points.count >= 2 else { return "has fewer than 2 points" }
        for (index, (a, b)) in zip(points, points.dropFirst()).enumerated() {
            if same(a, b) { return "segment \(index + 1) has no length" }
            if !isMultipleOf45(dx: b.x - a.x, dy: b.y - a.y) {
                return "segment \(index + 1) is not at a multiple of 45°"
            }
        }
        return nil
    }

    /// The points with consecutive duplicates removed.
    static func clean(_ points: [CGPoint]) -> [CGPoint] {
        var result: [CGPoint] = []
        for point in points where result.last.map({ !same($0, point) }) ?? true {
            result.append(point)
        }
        return result
    }

    static func same(_ a: CGPoint, _ b: CGPoint) -> Bool {
        abs(a.x - b.x) < epsilon && abs(a.y - b.y) < epsilon
    }
}
