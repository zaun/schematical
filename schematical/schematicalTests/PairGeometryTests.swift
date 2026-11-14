import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// A differential pair's geometry (plan-38-J Phase 1).
final class PairGeometryTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// Every 0.01 mm along `polyline`, its distance to `centre`.
    private func distances(_ polyline: [CGPoint], to centre: [CGPoint]) -> [Double] {
        var result: [Double] = []
        for (a, b) in zip(polyline, polyline.dropFirst()) {
            let length = hypot(b.x - a.x, b.y - a.y)
            let samples = max(1, Int((length / 0.01).rounded(.up)))
            for s in 0...samples {
                let t = Double(s) / Double(samples)
                let point = CGPoint(x: a.x + (b.x - a.x) * t, y: a.y + (b.y - a.y) * t)
                result.append(zip(centre, centre.dropFirst()).map {
                    SchematicDocument.distance(from: point, toSegment: $0.0, $0.1)
                }.min()!)
            }
        }
        return result
    }

    func testOffsetsKeepTheSpacing() {
        let s = 0.227
        let cases: [[CGPoint]] = [
            [p(0, 0), p(10, 0)],
            [p(0, 0), p(5, 0), p(5, 5)],
            [p(0, 0), p(4, 0), p(5, 1), p(5, 5)],
        ]
        for centre in cases {
            for side in [1.0, -1.0] {
                let moved = PairGeometry.offset(centre, by: side * s)
                XCTAssertEqual(moved.count, centre.count)
                let d = distances(moved, to: centre)
                // Never closer than s anywhere.
                XCTAssertGreaterThanOrEqual(d.min()!, s - 1e-6, "\(centre) \(side)")
                // Exactly s along each straight part, away from the corners.
                for (k, (a, b)) in zip(moved, moved.dropFirst()).enumerated() {
                    let mid = CGPoint(x: (a.x + b.x) / 2, y: (a.y + b.y) / 2)
                    let toCentre = SchematicDocument.distance(from: mid, toSegment: centre[k], centre[k + 1])
                    XCTAssertEqual(toCentre, s, accuracy: 1e-6, "segment \(k) of \(centre)")
                }
            }
        }
        // A 90° mitre stands off s√2 from the corner; a 45° one s / cos 22.5°.
        let l = PairGeometry.offset([p(0, 0), p(5, 0), p(5, 5)], by: -s)
        XCTAssertEqual(hypot(l[1].x - 5, l[1].y), s * 2.squareRoot(), accuracy: 1e-9)
    }

    func testPairingPicksTheSmallerSum() {
        let ends = PairGeometry.ends(p: (p(0, 0), p(20, 0)), n: (p(20, 1), p(0, 1)))
        XCTAssertEqual(ends.0, p(0, 0))
        XCTAssertEqual(ends.1, p(0, 1))
        XCTAssertEqual(ends.2, p(20, 0))
        XCTAssertEqual(ends.3, p(20, 1))
    }

    func testStaggerAtTheCustomDefaults() {
        // Via 0.5 mm, clearance 0.1 mm, centre spacing 0.254 + 0.2.
        let stagger = PairGeometry.stagger(diameter: 0.5, clearance: 0.1, spacing: 0.454)
        XCTAssertEqual(stagger, 0.392, accuracy: 0.001)
        XCTAssertEqual(PairGeometry.stagger(diameter: 0.5, clearance: 0.1, spacing: 0.7), 0)
        let vias = PairGeometry.vias(at: p(5, 5), along: p(1, 0), spacing: 0.454, stagger: stagger)
        let apart = hypot(vias.p.x - vias.n.x, vias.p.y - vias.n.y)
        XCTAssertGreaterThanOrEqual(apart - 0.5, 0.1 - 1e-9, "the metal keeps the clearance")
        XCTAssertEqual(vias.p.y - vias.n.y, 0.454, accuracy: 1e-9)
    }

    func testUncoupledLength() {
        let w = 0.254, g = 0.2, s = w + g
        let pTrace = RoutedTrace(layer: .top, width: w, points: [p(0, 0), p(10, 0)], net: 1)
        let together = RoutedTrace(layer: .top, width: w, points: [p(0, s), p(10, s)], net: 2)
        XCTAssertEqual(PairGeometry.uncoupledLength(p: [pTrace], n: [together], gap: g), 0, accuracy: 1e-9)
        // N runs alongside for 7 mm, then goes up 3 mm, away.
        let away = RoutedTrace(layer: .top, width: w, points: [p(0, s), p(7, s), p(7, s + 3)], net: 2)
        let length = PairGeometry.uncoupledLength(p: [pTrace], n: [away], gap: g)
        // P is coupled to N's corner until √((s + 0.02)² − s²) past it, N's rise for
        // its first 0.02 mm; the rest of both 3 mm stretches is apart.
        let expected = 3 - ((s + 0.02) * (s + 0.02) - s * s).squareRoot() + 3 - 0.02
        XCTAssertEqual(length, expected, accuracy: 0.02)
        let otherLayer = RoutedTrace(layer: .bottom, width: w, points: [p(0, s), p(10, s)], net: 2)
        XCTAssertEqual(PairGeometry.uncoupledLength(p: [pTrace], n: [otherLayer], gap: g), 20, accuracy: 1e-9,
                       "on another layer is apart")
    }
}
