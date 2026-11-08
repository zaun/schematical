import CoreGraphics
import XCTest
@testable import schematical

/// Re-fitting a wire's vertices after one of its ends moved
/// (`OrthogonalRoute.settled`): no vertex is left past the end, and a
/// vertex that lands on an end collapses into it.
final class WireSettleTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    func testVertexOnTheEndCollapsesIntoIt() {
        let interior = OrthogonalRoute.settled(start: p(0, 0), interior: [p(50, 0)], end: p(50, 0))
        XCTAssertEqual(interior, [])
    }

    func testEndMovedBackPastTheVertexDoesNotDoubleBack() {
        // A straight wire 0...100 with its midpoint vertex; the end moves to 30.
        let interior = OrthogonalRoute.settled(start: p(0, 0), interior: [p(50, 0)], end: p(30, 0))
        XCTAssertEqual(interior, [])
    }

    func testStartMovedPastTheVertexDoesNotDoubleBack() {
        let interior = OrthogonalRoute.settled(start: p(80, 0), interior: [p(50, 0)], end: p(100, 0))
        XCTAssertEqual(interior, [])
    }

    func testEndMovedPastSeveralVerticesKeepsTheOnesInFront() {
        // Z-shaped wire; the end comes back along its last run past the last vertex.
        let interior = OrthogonalRoute.settled(
            start: p(0, 0), interior: [p(50, 0), p(50, 20), p(80, 20)], end: p(60, 20))
        XCTAssertEqual(interior, [p(50, 0), p(50, 20)])
    }

    func testVertexBesideTheEndIsKept() {
        // A real corner (the run leaves vertically) is not a double back.
        let interior = OrthogonalRoute.settled(start: p(0, 0), interior: [p(50, 0), p(50, 20)], end: p(100, 20))
        XCTAssertEqual(interior, [p(50, 0), p(50, 20)])
    }
}
