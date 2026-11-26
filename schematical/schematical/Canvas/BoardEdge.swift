import CoreGraphics
import Foundation

/// The board's edge (plan-65): its outline and every cutout's loop. Every
/// reader of the edge (the pour fill, the design rule check, the router, the
/// exports) asks one of these, so none keeps its own point-in-polygon test.
/// Material is what is inside the outline (even-odd) and inside no cutout:
/// the union rule, so a cutout that overlaps another or crosses the outline
/// still gives a defined answer (the design rule check reports those two).
nonisolated struct BoardEdge: Hashable, Sendable {
    /// The board's outline, clockwise from the top-left, mm.
    let outline: [CGPoint]
    /// Each cutout's loop, mm.
    let cutouts: [[CGPoint]]

    init(outline: [CGPoint], cutouts: [[CGPoint]] = []) {
        self.outline = outline
        self.cutouts = cutouts
    }

    /// The outline first, then every cutout.
    var loops: [[CGPoint]] { [outline] + cutouts }

    /// Every loop's segments, each loop closed.
    var segments: [(CGPoint, CGPoint)] {
        loops.flatMap { loop in Array(zip(loop, loop.dropFirst() + loop.prefix(1))) }
    }

    /// The outline's bounds; zero for an empty outline.
    var bounds: CGRect {
        guard let minX = outline.map(\.x).min(), let maxX = outline.map(\.x).max(),
              let minY = outline.map(\.y).min(), let maxY = outline.map(\.y).max() else { return .zero }
        return CGRect(x: minX, y: minY, width: maxX - minX, height: maxY - minY)
    }

    /// Every loop in one path; filled even-odd, the cutouts are holes.
    var path: CGPath {
        let path = CGMutablePath()
        for loop in loops where loop.count >= 3 {
            path.addLines(between: loop)
            path.closeSubpath()
        }
        return path
    }

    /// True on board material: inside the outline and inside no cutout.
    func contains(_ point: CGPoint) -> Bool {
        guard Self.contains(point, in: outline) else { return false }
        return !cutouts.contains { Self.contains(point, in: $0) }
    }

    /// True when `point` is inside a cutout that is itself inside the outline.
    func isInCutout(_ point: CGPoint) -> Bool {
        Self.contains(point, in: outline) && cutouts.contains { Self.contains(point, in: $0) }
    }

    /// Whether `point` is inside the closed `loop` by the even-odd rule.
    static func contains(_ point: CGPoint, in loop: [CGPoint]) -> Bool {
        guard loop.count >= 3 else { return false }
        var result = false
        var j = loop.count - 1
        for i in loop.indices {
            let a = loop[i], b = loop[j]
            if (a.y > point.y) != (b.y > point.y) {
                let x = a.x + (point.y - a.y) / (b.y - a.y) * (b.x - a.x)
                if point.x < x { result.toggle() }
            }
            j = i
        }
        return result
    }
}

/// Geometry of simple polygons, shared by the cutouts, the free-form outline
/// and the DXF reader.
nonisolated enum BoardGeometry {
    /// The signed area (y down: positive is clockwise on screen).
    static func area(_ points: [CGPoint]) -> Double {
        var sum = 0.0
        for i in points.indices {
            let a = points[i], b = points[(i + 1) % points.count]
            sum += Double(a.x * b.y - b.x * a.y)
        }
        return sum / 2
    }

    /// True when fewer than 3 points, or two edges that are not neighbours cross.
    static func crossesItself(_ points: [CGPoint]) -> Bool {
        let n = points.count
        guard n >= 3 else { return true }
        for i in 0..<n {
            let a = points[i], b = points[(i + 1) % n]
            for j in (i + 1)..<n {
                // Edges that share an end point are neighbours.
                if j == i + 1 || (i == 0 && j == n - 1) { continue }
                let c = points[j], d = points[(j + 1) % n]
                if CopperGeometry.segmentsCross(a, b, c, d) { return true }
            }
        }
        return false
    }

    /// `points` clockwise on screen (y down): reversed when they run the other way.
    static func clockwise(_ points: [CGPoint]) -> [CGPoint] {
        area(points) < 0 ? points.reversed() : points
    }
}
