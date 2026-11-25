import CoreGraphics
import Foundation

/// A cutout (plan-65): a closed region of the board with no material, such
/// as a window, a round hole too big to drill or a milled slot. It goes
/// through the whole board, so it has no layer and no net. Its shape is a
/// rounded rectangle (a rectangle with a corner radius; with a square box and
/// a radius of half the side, a circle; with a longer box and a radius of half
/// its short side, a slot) or a polygon (a loop from a DXF). Sizes in mm.
nonisolated struct BoardCutout: Identifiable, Hashable, Sendable {
    let id: UUID
    var shape: Shape
    /// Locked: dragging and the sidebar's X and Y leave it where it is.
    var locked = false

    enum Shape: Hashable, Sendable {
        /// A rectangle with this corner radius.
        case rounded(rect: CGRect, radius: Double)
        /// Clockwise (y down), closed implicitly.
        case polygon([CGPoint])
    }

    /// What a cutout is called in lists and panels.
    enum Kind: String, Sendable {
        case rectangle, circle, slot, polygon

        var title: String {
            switch self {
            case .rectangle: "Rectangle"
            case .circle: "Circle"
            case .slot: "Slot"
            case .polygon: "Polygon"
            }
        }
    }

    /// The thinnest a rounded cutout may be (the pour's and the keep-out's).
    static let minimumSide = BoardPour.minimumSide
    /// How far a flattened curve may stray from the true one (mm).
    static let flatness = 0.01
    /// The fewest segments in a quarter turn of a flattened curve.
    static let minimumArcSegments = 4

    /// The segments of a quarter turn of radius `radius` (mm): enough that no
    /// chord is more than `flatness` from the arc, at least `minimumArcSegments`.
    static func quarterSegments(radius: Double, flatness: Double = BoardCutout.flatness) -> Int {
        guard radius > flatness else { return minimumArcSegments }
        let half = acos(1 - flatness / radius)
        guard half > 0 else { return minimumArcSegments }
        return max(minimumArcSegments, Int(((Double.pi / 2) / (2 * half)).rounded(.up)))
    }

    /// The smallest allowed `rect` and `radius` nearest to these: the rectangle
    /// standardized with each side raised to `minimumSide` (grown right and
    /// down), the radius within 0 to half the short side.
    static func normalized(_ rect: CGRect, radius: Double) -> (rect: CGRect, radius: Double) {
        let r = rect.standardized
        let box = CGRect(x: r.minX, y: r.minY, width: max(r.width, minimumSide), height: max(r.height, minimumSide))
        return (box, min(max(radius, 0), min(box.width, box.height) / 2))
    }

    /// Why this shape is wrong, as a file error detail after "cutout <id> ",
    /// or nil.
    static func problem(_ shape: Shape) -> String? {
        func n(_ value: Double) -> String { String(format: "%g", value) }
        switch shape {
        case let .rounded(rect, radius):
            let values = [rect.minX, rect.minY, rect.width, rect.height, radius]
            if values.contains(where: { !$0.isFinite }) { return "has a value that is not a number" }
            if rect.width < minimumSide - 1e-9 { return "width \(n(rect.width)) must be at least \(n(minimumSide)) mm" }
            if rect.height < minimumSide - 1e-9 { return "height \(n(rect.height)) must be at least \(n(minimumSide)) mm" }
            let limit = min(rect.width, rect.height) / 2
            if radius < 0 || radius > limit + 1e-9 { return "radius \(n(radius)) must be 0 to \(n(limit)) mm" }
            return nil
        case let .polygon(points):
            if points.contains(where: { !$0.x.isFinite || !$0.y.isFinite }) { return "has a value that is not a number" }
            if points.count < 3 { return "needs at least 3 points" }
            if BoardGeometry.crossesItself(points) { return "crosses itself" }
            if abs(BoardGeometry.area(points)) < 1e-9 { return "has no area" }
            return nil
        }
    }
}

extension BoardCutout.Shape {
    /// A circle of `diameter` round `centre`.
    static func circle(centre: CGPoint, diameter: Double) -> BoardCutout.Shape {
        .rounded(rect: CGRect(x: centre.x - diameter / 2, y: centre.y - diameter / 2, width: diameter, height: diameter),
                 radius: diameter / 2)
    }

    /// A slot: `rect` with ends rounded to half its short side.
    static func slot(rect: CGRect) -> BoardCutout.Shape {
        let r = rect.standardized
        return .rounded(rect: r, radius: min(r.width, r.height) / 2)
    }

    var bounds: CGRect {
        switch self {
        case let .rounded(rect, _):
            return rect.standardized
        case let .polygon(points):
            guard let minX = points.map(\.x).min(), let maxX = points.map(\.x).max(),
                  let minY = points.map(\.y).min(), let maxY = points.map(\.y).max() else { return .zero }
            return CGRect(x: minX, y: minY, width: maxX - minX, height: maxY - minY)
        }
    }

    func translated(by offset: CGSize) -> BoardCutout.Shape {
        switch self {
        case let .rounded(rect, radius):
            return .rounded(rect: rect.offsetBy(dx: offset.width, dy: offset.height), radius: radius)
        case let .polygon(points):
            return .polygon(points.map { CGPoint(x: $0.x + offset.width, y: $0.y + offset.height) })
        }
    }

    /// The shape as a loop, clockwise (y down), curves flattened to
    /// `BoardCutout.flatness`, no repeated end point. A rounded rectangle
    /// starts at its top-left corner; a radius of 0 is its four corners.
    var loop: [CGPoint] {
        switch self {
        case let .polygon(points):
            return points
        case let .rounded(rect, radius):
            let r = rect.standardized
            let radius = min(max(radius, 0), min(r.width, r.height) / 2)
            guard radius > 1e-9 else {
                return [CGPoint(x: r.minX, y: r.minY), CGPoint(x: r.maxX, y: r.minY),
                        CGPoint(x: r.maxX, y: r.maxY), CGPoint(x: r.minX, y: r.maxY)]
            }
            let n = BoardCutout.quarterSegments(radius: radius)
            // Each corner's arc centre and where its quarter turn starts, in
            // degrees (y down: 90 points down): clockwise from the top-left.
            let corners: [(centre: CGPoint, from: Double)] = [
                (CGPoint(x: r.minX + radius, y: r.minY + radius), 180),
                (CGPoint(x: r.maxX - radius, y: r.minY + radius), 270),
                (CGPoint(x: r.maxX - radius, y: r.maxY - radius), 0),
                (CGPoint(x: r.minX + radius, y: r.maxY - radius), 90),
            ]
            var points: [CGPoint] = []
            for corner in corners {
                for step in 0...n {
                    let angle = (corner.from + 90 * Double(step) / Double(n)) * Double.pi / 180
                    let point = CGPoint(x: corner.centre.x + radius * cos(angle), y: corner.centre.y + radius * sin(angle))
                    // A circle's neighbouring arcs meet: keep one point.
                    if let last = points.last, hypot(last.x - point.x, last.y - point.y) < 1e-9 { continue }
                    points.append(point)
                }
            }
            if let first = points.first, let last = points.last, hypot(last.x - first.x, last.y - first.y) < 1e-9 {
                points.removeLast()
            }
            return points
        }
    }

    /// The nearest distance from `point` to the shape's edge (mm), inside or out.
    func distance(to point: CGPoint) -> Double {
        let points = loop
        guard points.count > 1 else { return .infinity }
        var best = Double.infinity
        for i in points.indices {
            let a = points[i], b = points[(i + 1) % points.count]
            best = min(best, Double(SchematicDocument.distance(from: point, toSegment: a, b)))
        }
        return best
    }
}

extension BoardCutout {
    /// `rectangle`, `circle`, `slot` or `polygon`.
    var kind: Kind {
        switch shape {
        case .polygon:
            return .polygon
        case let .rounded(rect, radius):
            let r = rect.standardized
            let half = min(r.width, r.height) / 2
            if radius < 1e-9 || abs(radius - half) > 1e-9 { return .rectangle }
            return abs(r.width - r.height) < 1e-9 ? .circle : .slot
        }
    }
}
