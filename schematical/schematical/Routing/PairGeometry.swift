import CoreGraphics
import Foundation

/// The geometry of a coupled differential pair (plan-38-J § 3): which pads
/// pair up at each end, the two traces as offsets of one centreline, the
/// staggered via pair at a layer change, and how much of a pair runs apart.
nonisolated enum PairGeometry {
    /// Which of N's two pad centres goes with P's first: the pairing with
    /// the smaller sum of distances. Returns (p1, n1, p2, n2).
    static func ends(p: (CGPoint, CGPoint), n: (CGPoint, CGPoint)) -> (CGPoint, CGPoint, CGPoint, CGPoint) {
        func d(_ a: CGPoint, _ b: CGPoint) -> Double { hypot(a.x - b.x, a.y - b.y) }
        let straight = d(p.0, n.0) + d(p.1, n.1), crossed = d(p.0, n.1) + d(p.1, n.0)
        return straight <= crossed ? (p.0, n.0, p.1, n.1) : (p.0, n.1, p.1, n.0)
    }

    /// `polyline` moved `distance` to its left (negative: right), each
    /// corner a mitre: the meeting point of the two moved segments. Along
    /// every straight part it is exactly `distance` from the centreline; at
    /// an outer corner the mitre point stands off up to `distance / cos(θ/2)`;
    /// nowhere is it closer.
    static func offset(_ polyline: [CGPoint], by distance: Double) -> [CGPoint] {
        guard polyline.count >= 2 else { return polyline }
        func normal(_ a: CGPoint, _ b: CGPoint) -> CGPoint {
            let l = hypot(b.x - a.x, b.y - a.y)
            return CGPoint(x: -(b.y - a.y) / l, y: (b.x - a.x) / l)
        }
        var result = [CGPoint]()
        let n0 = normal(polyline[0], polyline[1])
        result.append(CGPoint(x: polyline[0].x + n0.x * distance, y: polyline[0].y + n0.y * distance))
        for k in 1..<(polyline.count - 1) {
            let a = normal(polyline[k - 1], polyline[k]), b = normal(polyline[k], polyline[k + 1])
            let dot = a.x * b.x + a.y * b.y
            // The mitre: along the bisector of the two normals, 1/cos(θ/2) out.
            let scale = distance / (1 + dot)
            result.append(CGPoint(x: polyline[k].x + (a.x + b.x) * scale, y: polyline[k].y + (a.y + b.y) * scale))
        }
        let last = polyline.count - 1
        let nl = normal(polyline[last - 1], polyline[last])
        result.append(CGPoint(x: polyline[last].x + nl.x * distance, y: polyline[last].y + nl.y * distance))
        return result
    }

    /// How far apart along the run the pair's two vias stand, so their
    /// metal keeps `clearance`: `√((D + c)² − s²)`, 0 when the centre
    /// spacing `s` already keeps it.
    static func stagger(diameter: Double, clearance: Double, spacing: Double) -> Double {
        let need = diameter + clearance
        return spacing >= need ? 0 : (need * need - spacing * spacing).squareRoot()
    }

    /// How far apart, centre to centre, a pair's two vias stand side by side:
    /// far enough that their metal keeps `clearance` (D + c) and that each
    /// trace, flared out to its via, keeps it from the other's via
    /// (D/2 + w/2 + c); never closer than the pair's own spacing.
    static func viaSpacing(diameter: Double, clearance: Double, width: Double, spacing: Double) -> Double {
        max(diameter + clearance, diameter / 2 + width / 2 + clearance, spacing)
    }

    /// The pair's two vias at a layer change at `centre`, the run going
    /// `along` (a unit vector): P's on the left, set back half the stagger,
    /// N's on the right, set forward half.
    static func vias(at centre: CGPoint, along: CGPoint, spacing: Double, stagger: Double) -> (p: CGPoint, n: CGPoint) {
        let lateral = CGPoint(x: -along.y, y: along.x)
        let p = CGPoint(x: centre.x + lateral.x * spacing / 2 - along.x * stagger / 2,
                        y: centre.y + lateral.y * spacing / 2 - along.y * stagger / 2)
        let n = CGPoint(x: centre.x - lateral.x * spacing / 2 + along.x * stagger / 2,
                        y: centre.y - lateral.y * spacing / 2 + along.y * stagger / 2)
        return (p, n)
    }

    /// The length of the pair that runs apart: every stretch of either net's
    /// traces where the edge-to-edge gap to the other net's traces on the
    /// same layer isn't `gap ± 0.02 mm` (on another layer is apart),
    /// measured every 0.01 mm, summed over both nets.
    static func uncoupledLength(p: [RoutedTrace], n: [RoutedTrace], gap: Double, step: Double = 0.01) -> Double {
        func apart(_ mine: [RoutedTrace], _ theirs: [RoutedTrace]) -> Double {
            var total = 0.0
            for trace in mine {
                for (a, b) in zip(trace.points, trace.points.dropFirst()) {
                    let length = hypot(b.x - a.x, b.y - a.y)
                    let samples = max(1, Int((length / step).rounded(.up)))
                    let piece = length / Double(samples)
                    for s in 0..<samples {
                        let t = (Double(s) + 0.5) / Double(samples)
                        let point = CGPoint(x: a.x + (b.x - a.x) * t, y: a.y + (b.y - a.y) * t)
                        var nearest = Double.infinity
                        for other in theirs where other.layer == trace.layer {
                            for (c, d) in zip(other.points, other.points.dropFirst()) {
                                nearest = min(nearest, SchematicDocument.distance(from: point, toSegment: c, d)
                                    - trace.width / 2 - other.width / 2)
                            }
                        }
                        if abs(nearest - gap) > 0.02 { total += piece }
                    }
                }
            }
            return total
        }
        return apart(p, n) + apart(n, p)
    }
}
