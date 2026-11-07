import CoreGraphics
import XCTest
@testable import schematical

/// plan-25-A phase 1: one drawn leg (`OrthogonalRoute.leg`) and the clean-up
/// of a drawn route (`OrthogonalRoute.simplified`).
final class RouteLegTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func leg(_ a: CGPoint, _ b: CGPoint, _ first: RouteAxis?, _ last: RouteAxis?,
                     preferred: RouteAxis = .horizontal) -> [CGPoint] {
        OrthogonalRoute.leg(from: a, to: b, first: first, last: last, preferred: preferred)
    }

    func testSamePointIsEmpty() {
        XCTAssertEqual(leg(p(10, 10), p(10, 10), .horizontal, .horizontal), [])
    }

    func testAlignedIsStraightWhateverIsForced() {
        XCTAssertEqual(leg(p(0, 0), p(100, 0), .vertical, .vertical), [p(100, 0)])
        XCTAssertEqual(leg(p(0, 0), p(0, 80), .horizontal, nil), [p(0, 80)])
    }

    func testBothHorizontalIsZAtMidX() {
        XCTAssertEqual(leg(p(0, 0), p(100, 60), .horizontal, .horizontal),
                       [p(50, 0), p(50, 60), p(100, 60)])
    }

    func testBothVerticalIsZAtMidY() {
        XCTAssertEqual(leg(p(0, 0), p(100, 60), .vertical, .vertical),
                       [p(0, 30), p(100, 30), p(100, 60)])
    }

    func testDifferentForcedAxesIsAnL() {
        XCTAssertEqual(leg(p(0, 0), p(100, 60), .horizontal, .vertical), [p(100, 0), p(100, 60)])
        XCTAssertEqual(leg(p(0, 0), p(100, 60), .vertical, .horizontal), [p(0, 60), p(100, 60)])
    }

    func testOnlyFirstForced() {
        XCTAssertEqual(leg(p(0, 0), p(100, 60), .horizontal, nil, preferred: .vertical), [p(100, 0), p(100, 60)])
        XCTAssertEqual(leg(p(0, 0), p(100, 60), .vertical, nil, preferred: .horizontal), [p(0, 60), p(100, 60)])
    }

    func testOnlyLastForcedArrivesAlongIt() {
        XCTAssertEqual(leg(p(0, 0), p(100, 60), nil, .horizontal, preferred: .horizontal), [p(0, 60), p(100, 60)])
        XCTAssertEqual(leg(p(0, 0), p(100, 60), nil, .vertical, preferred: .vertical), [p(100, 0), p(100, 60)])
    }

    func testNothingForcedFollowsPreferred() {
        XCTAssertEqual(leg(p(0, 0), p(100, 60), nil, nil, preferred: .horizontal), [p(100, 0), p(100, 60)])
        XCTAssertEqual(leg(p(0, 0), p(100, 60), nil, nil, preferred: .vertical), [p(0, 60), p(100, 60)])
    }

    func testRouteIsTheOldMidpointZ() {
        XCTAssertEqual(OrthogonalRoute.route(from: p(0, 0), to: p(100, 60)),
                       [p(0, 0), p(50, 0), p(50, 60), p(100, 60)])
        XCTAssertEqual(OrthogonalRoute.route(from: p(0, 0), to: p(100, 0)), [p(0, 0), p(100, 0)])
    }

    func testSimplifiedDropsStraightThroughLock() {
        XCTAssertEqual(OrthogonalRoute.simplified([p(0, 0), p(50, 0), p(100, 0), p(100, 40)]),
                       [p(0, 0), p(100, 0), p(100, 40)])
    }

    func testSimplifiedDropsDoubleBack() {
        XCTAssertEqual(OrthogonalRoute.simplified([p(0, 0), p(100, 0), p(60, 0), p(60, 40)]),
                       [p(0, 0), p(60, 0), p(60, 40)])
    }

    func testSimplifiedKeepsAU() {
        let u = [p(0, 0), p(50, 0), p(50, 100), p(0, 100)]
        XCTAssertEqual(OrthogonalRoute.simplified(u), u)
    }
}
