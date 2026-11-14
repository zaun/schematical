import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Length-tuning bumps (plan-38-I Phase 2).
final class MeanderTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    private let a = CGPoint(x: 0, y: 0), b = CGPoint(x: 10, y: 0)

    func testARectangularBumpAddsTwiceItsAmplitude() {
        let points = Meander.bump(from: a, to: b, at: 3, side: 1, amplitude: 2, spacing: 1, cut: 0)
        XCTAssertEqual(RoutingCleanup.length(points), 10 + 4, accuracy: 1e-12)
        XCTAssertEqual(Meander.added(amplitude: 2, cut: 0), 4)
        XCTAssertEqual(points, [a, p(3, 0), p(3, 2), p(4, 2), p(4, 0), b])
    }

    func testAChamferedBumpLosesWhatItsCornersCut() {
        let cut = 0.3
        let points = Meander.bump(from: a, to: b, at: 3, side: 1, amplitude: 2, spacing: 1, cut: cut)
        // Each corner cut by `cut` on both sides: 2 × cut becomes cut × √2.
        let loss = 4 * cut * (2 - 2.squareRoot())
        XCTAssertEqual(RoutingCleanup.length(points), 10 + 4 - loss, accuracy: 1e-12)
        XCTAssertEqual(Meander.added(amplitude: 2, cut: cut), 4 - loss, accuracy: 1e-12)
        for (u, v) in zip(points, points.dropFirst()) {
            let angle = abs(atan2(Double(v.y - u.y), Double(v.x - u.x)) * 180 / .pi)
            XCTAssertTrue([0, 45, 90, 135, 180].contains { abs($0 - angle) < 1e-9 }, "\(angle)")
        }
    }

    func testAnAccordionAlternatesAndKeepsTheHostsEnds() {
        let points = Meander.accordion(from: a, to: b, at: 1, count: 5, side: 1, amplitude: 1, spacing: 0.8, cut: 0)
        XCTAssertEqual(points.first, a)
        XCTAssertEqual(points.last, b)
        // The tops: y = +1, −1, +1, −1, +1.
        var tops: [CGFloat] = []
        for point in points where abs(point.y) == 1 && tops.last != point.y { tops.append(point.y) }
        XCTAssertEqual(tops, [1, -1, 1, -1, 1])
        XCTAssertEqual(RoutingCleanup.length(points), 10 + 5 * 2, accuracy: 1e-12)
    }

    func testAmplitudeForAnAddedLengthHitsItsTarget() {
        for target in [0.3, 1.7, 3.141] {
            let amplitude = Meander.amplitude(forAdded: target, cut: 0.2, spacing: 0.8, in: 0...10)
            let cut = Meander.fitted(cut: 0.2, amplitude: amplitude, spacing: 0.8)
            XCTAssertEqual(Meander.added(amplitude: amplitude, cut: cut), target, accuracy: 1e-4)
        }
        XCTAssertEqual(Meander.amplitude(forAdded: 100, cut: 0, spacing: 1, in: 0...4), 4, "the range's end")
    }

    func testLegsAreTheSpacingApart() {
        let spacing = 0.254 + 0.508
        let points = Meander.bump(from: a, to: b, at: 2, side: -1, amplitude: 1.5, spacing: spacing, cut: 0)
        let legs = points.filter { abs($0.y + 1.5) < 1e-12 }.map(\.x)
        XCTAssertEqual(legs.count, 2)
        XCTAssertEqual(legs[1] - legs[0], spacing, accuracy: 1e-12)
    }
}
