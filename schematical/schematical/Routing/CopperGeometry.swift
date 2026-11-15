import CoreGraphics
import Foundation

/// A piece of copper: a core (a segment, or a rectangle) grown by
/// `radius` all round, on some layers. A trace segment is a `.segment` core
/// grown by half its width; a via is a zero-length segment grown by half
/// its diameter, on every layer. Shared by the design rule check and the
/// router (plan-38-D), so both measure gaps the same way.
nonisolated struct CopperShape: Hashable, Sendable {
    enum Core: Hashable, Sendable {
        case segment(CGPoint, CGPoint)
        case rect(CGRect)
    }

    let core: Core
    let radius: Double
    /// Nil: every layer (a through-hole pad, a via).
    let layers: Set<CopperLayer>?
    /// Its connectivity node.
    let node: Int

    var bounds: CGRect {
        let box: CGRect
        switch core {
        case let .segment(a, b):
            box = CGRect(x: min(a.x, b.x), y: min(a.y, b.y), width: abs(a.x - b.x), height: abs(a.y - b.y))
        case let .rect(rect): box = rect
        }
        return box.insetBy(dx: -radius, dy: -radius)
    }

    func shares(_ other: CopperShape) -> Bool {
        guard let mine = layers, let theirs = other.layers else { return true }
        return !mine.isDisjoint(with: theirs)
    }

    /// The gap between the two shapes' copper; 0 or less when they
    /// touch.
    func gap(to other: CopperShape) -> Double {
        CopperGeometry.distance(core, other.core) - radius - other.radius
    }
}

/// The distance functions the design rule check measures copper with.
nonisolated enum CopperGeometry {
    /// A pad's copper: a rounded rectangle is its rectangle shrunk by the
    /// radius, grown back round; a circle is a point grown; an oval the
    /// stadium round it. Chamfers are left square.
    static func shape(of pad: PlacedPad, layers: Set<CopperLayer>?, node: Int) -> CopperShape {
        let rect = pad.rect
        switch pad.shape {
        case .round:
            let r = min(rect.width, rect.height) / 2
            let core: CopperShape.Core = rect.width >= rect.height
                ? .segment(CGPoint(x: rect.minX + r, y: rect.midY), CGPoint(x: rect.maxX - r, y: rect.midY))
                : .segment(CGPoint(x: rect.midX, y: rect.minY + r), CGPoint(x: rect.midX, y: rect.maxY - r))
            return CopperShape(core: core, radius: r, layers: layers, node: node)
        case .rect:
            let r = min(pad.radius, min(rect.width, rect.height) / 2)
            return CopperShape(core: .rect(rect.insetBy(dx: r, dy: r)), radius: r, layers: layers, node: node)
        }
    }

    /// Each pair of shapes whose bounds come within `margin`, once, by a
    /// sweep along x.
    static func nearPairs(_ shapes: [CopperShape], margin: Double, _ body: (CopperShape, CopperShape) -> Void) {
        let order = shapes.indices.sorted { shapes[$0].bounds.minX < shapes[$1].bounds.minX }
        let bounds = shapes.map(\.bounds)
        for (k, i) in order.enumerated() {
            let a = bounds[i]
            for j in order[(k + 1)...] {
                let b = bounds[j]
                if b.minX > a.maxX + margin { break }
                if b.minY > a.maxY + margin || a.minY > b.maxY + margin { continue }
                body(shapes[i], shapes[j])
            }
        }
    }

    static func distance(_ a: CopperShape.Core, _ b: CopperShape.Core) -> Double {
        switch (a, b) {
        case let (.segment(p, q), .segment(r, s)): segmentDistance(p, q, r, s)
        case let (.segment(p, q), .rect(rect)), let (.rect(rect), .segment(p, q)): segmentDistance(p, q, rect)
        case let (.rect(r1), .rect(r2)): rectDistance(r1, r2)
        }
    }

    static func segmentDistance(_ a: CGPoint, _ b: CGPoint, _ c: CGPoint, _ d: CGPoint) -> Double {
        if segmentsCross(a, b, c, d) { return 0 }
        return min(SchematicDocument.distance(from: a, toSegment: c, d),
                   SchematicDocument.distance(from: b, toSegment: c, d),
                   SchematicDocument.distance(from: c, toSegment: a, b),
                   SchematicDocument.distance(from: d, toSegment: a, b))
    }

    static func segmentDistance(_ a: CGPoint, _ b: CGPoint, _ rect: CGRect) -> Double {
        if SchematicDocument.segment(a, b, touches: rect) { return 0 }
        let corners = [CGPoint(x: rect.minX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.minY),
                       CGPoint(x: rect.maxX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.maxY)]
        return min(pointDistance(a, rect), pointDistance(b, rect),
                   corners.map { SchematicDocument.distance(from: $0, toSegment: a, b) }.min()!)
    }

    static func pointDistance(_ point: CGPoint, _ rect: CGRect) -> Double {
        let dx = max(rect.minX - point.x, 0, point.x - rect.maxX)
        let dy = max(rect.minY - point.y, 0, point.y - rect.maxY)
        return hypot(dx, dy)
    }

    static func rectDistance(_ a: CGRect, _ b: CGRect) -> Double {
        let dx = max(a.minX - b.maxX, 0, b.minX - a.maxX)
        let dy = max(a.minY - b.maxY, 0, b.minY - a.maxY)
        return hypot(dx, dy)
    }

    /// Whether two segments cross or touch.
    static func segmentsCross(_ a: CGPoint, _ b: CGPoint, _ c: CGPoint, _ d: CGPoint) -> Bool {
        func cross(_ o: CGPoint, _ p: CGPoint, _ q: CGPoint) -> Double {
            (p.x - o.x) * (q.y - o.y) - (p.y - o.y) * (q.x - o.x)
        }
        let d1 = cross(c, d, a), d2 = cross(c, d, b), d3 = cross(a, b, c), d4 = cross(a, b, d)
        return ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) && ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))
    }
}
