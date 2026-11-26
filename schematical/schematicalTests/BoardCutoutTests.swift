import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The cutout value, its flattening and the board edge (plan-65-A).
final class BoardCutoutTests: XCTestCase {
    private func p(_ x: Double, _ y: Double) -> CGPoint { CGPoint(x: x, y: y) }
    private func r(_ x: Double, _ y: Double, _ w: Double, _ h: Double) -> CGRect { CGRect(x: x, y: y, width: w, height: h) }

    func testARectangleIsItsFourCornersFromTheTopLeftClockwise() {
        let loop = BoardCutout.Shape.rounded(rect: r(10, 20, 6, 4), radius: 0).loop
        XCTAssertEqual(loop, [p(10, 20), p(16, 20), p(16, 24), p(10, 24)])
        XCTAssertGreaterThan(BoardGeometry.area(loop), 0, "clockwise on screen")
    }

    func testACircleIsA32GonWithinTheFlatness() {
        let shape = BoardCutout.Shape.circle(centre: p(20, 15), diameter: 3.2)
        let loop = shape.loop
        XCTAssertEqual(loop.count, 32)
        for point in loop {
            XCTAssertEqual(Double(hypot(point.x - 20, point.y - 15)), 1.6, accuracy: 1e-9)
        }
        for i in loop.indices {
            let a = loop[i], b = loop[(i + 1) % loop.count]
            let mid = p(Double(a.x + b.x) / 2, Double(a.y + b.y) / 2)
            let sag = 1.6 - Double(hypot(mid.x - 20, mid.y - 15))
            XCTAssertLessThanOrEqual(sag, BoardCutout.flatness + 1e-9, "chord \(i) strays \(sag)")
        }
        XCTAssertGreaterThan(BoardGeometry.area(loop), 0)
    }

    func testQuarterSegmentsFollowTheRule() {
        XCTAssertEqual(BoardCutout.quarterSegments(radius: 0.5), 4)
        XCTAssertEqual(BoardCutout.quarterSegments(radius: 1.6), 8)
        XCTAssertEqual(BoardCutout.quarterSegments(radius: 5), 13)
        XCTAssertEqual(BoardCutout.quarterSegments(radius: 50), 40)
    }

    func testASlotHasRoundEndCapsAndAStraightMiddle() {
        let shape = BoardCutout.Shape.slot(rect: r(10, 10, 10, 2))
        let loop = shape.loop
        XCTAssertEqual(shape.bounds, r(10, 10, 10, 2))
        // Every point is on a cap (a radius of 1 from the cap centre) or on a flat side.
        for point in loop {
            let onTop = abs(point.y - 10) < 1e-9, onBottom = abs(point.y - 12) < 1e-9
            let left = hypot(point.x - 11, point.y - 11), right = hypot(point.x - 19, point.y - 11)
            XCTAssertTrue(onTop || onBottom || abs(left - 1) < 1e-9 || abs(right - 1) < 1e-9, "\(point)")
        }
        XCTAssertEqual(abs(BoardGeometry.area(loop)), 16 + Double.pi, accuracy: 0.05)
    }

    func testKind() {
        XCTAssertEqual(BoardCutout(id: UUID(), shape: .circle(centre: p(5, 5), diameter: 3)).kind, .circle)
        XCTAssertEqual(BoardCutout(id: UUID(), shape: .slot(rect: r(0, 0, 8, 2))).kind, .slot)
        XCTAssertEqual(BoardCutout(id: UUID(), shape: .rounded(rect: r(0, 0, 8, 2), radius: 0.5)).kind, .rectangle)
        XCTAssertEqual(BoardCutout(id: UUID(), shape: .rounded(rect: r(0, 0, 8, 2), radius: 0)).kind, .rectangle)
        XCTAssertEqual(BoardCutout(id: UUID(), shape: .polygon([p(0, 0), p(4, 0), p(2, 3)])).kind, .polygon)
    }

    func testNormalizedClampsTheBoxAndTheRadius() {
        let (box, radius) = BoardCutout.normalized(r(10, 10, 0.1, 6), radius: 5)
        XCTAssertEqual(box, r(10, 10, 0.5, 6))
        XCTAssertEqual(radius, 0.25, accuracy: 1e-12)
        XCTAssertEqual(BoardCutout.normalized(r(14, 10, -4, 3), radius: -1).rect, r(10, 10, 4, 3))
        XCTAssertEqual(BoardCutout.normalized(r(14, 10, -4, 3), radius: -1).radius, 0)
    }

    func testProblems() {
        XCTAssertNil(BoardCutout.problem(.rounded(rect: r(0, 0, 4, 2), radius: 1)))
        XCTAssertEqual(BoardCutout.problem(.rounded(rect: r(0, 0, 0.1, 2), radius: 0)), "width 0.1 must be at least 0.5 mm")
        XCTAssertEqual(BoardCutout.problem(.rounded(rect: r(0, 0, 4, 0.2), radius: 0)), "height 0.2 must be at least 0.5 mm")
        XCTAssertEqual(BoardCutout.problem(.rounded(rect: r(0, 0, 4, 4), radius: 5)), "radius 5 must be 0 to 2 mm")
        XCTAssertEqual(BoardCutout.problem(.rounded(rect: r(0, 0, 4, 4), radius: -1)), "radius -1 must be 0 to 2 mm")
        XCTAssertNil(BoardCutout.problem(.polygon([p(0, 0), p(4, 0), p(2, 3)])))
        XCTAssertEqual(BoardCutout.problem(.polygon([p(0, 0), p(4, 0)])), "needs at least 3 points")
        // A bow-tie.
        XCTAssertEqual(BoardCutout.problem(.polygon([p(0, 0), p(4, 4), p(4, 0), p(0, 4)])), "crosses itself")
        XCTAssertEqual(BoardCutout.problem(.polygon([p(0, 0), p(2, 0), p(4, 0)])), "has no area")
        XCTAssertEqual(BoardCutout.problem(.polygon([p(0, 0), p(.nan, 0), p(4, 4)])), "has a value that is not a number")
    }

    func testTranslatedAndBounds() {
        let shape = BoardCutout.Shape.polygon([p(1, 1), p(5, 1), p(3, 4)])
        XCTAssertEqual(shape.bounds, r(1, 1, 4, 3))
        XCTAssertEqual(shape.translated(by: CGSize(width: 2, height: -1)).bounds, r(3, 0, 4, 3))
        let rounded = BoardCutout.Shape.rounded(rect: r(1, 1, 4, 3), radius: 1)
        XCTAssertEqual(rounded.translated(by: CGSize(width: 2, height: 2)), .rounded(rect: r(3, 3, 4, 3), radius: 1))
    }

    func testDistanceToTheEdge() {
        let shape = BoardCutout.Shape.rounded(rect: r(0, 0, 10, 10), radius: 0)
        XCTAssertEqual(shape.distance(to: p(5, 5)), 5, accuracy: 1e-9)
        XCTAssertEqual(shape.distance(to: p(13, 5)), 3, accuracy: 1e-9)
        XCTAssertEqual(shape.distance(to: p(10, 5)), 0, accuracy: 1e-9)
    }

    func testClockwiseReversesACounterClockwiseSquare() {
        let cw = [p(0, 0), p(10, 0), p(10, 10), p(0, 10)]
        XCTAssertEqual(BoardGeometry.clockwise(cw), cw)
        XCTAssertEqual(BoardGeometry.clockwise(cw.reversed()), Array(cw.reversed().reversed()))
    }

    func testCrossesItself() {
        XCTAssertTrue(BoardGeometry.crossesItself([p(0, 0), p(4, 4), p(4, 0), p(0, 4)]))
        XCTAssertFalse(BoardGeometry.crossesItself([p(0, 0), p(10, 0), p(10, 5), p(5, 5), p(5, 10), p(0, 10)]))
        XCTAssertTrue(BoardGeometry.crossesItself([p(0, 0), p(1, 1)]))
    }

    // MARK: BoardEdge

    private var outline: [CGPoint] { [p(0, 0), p(60, 0), p(60, 40), p(0, 40)] }

    func testContainsOnMaterial() {
        let edge = BoardEdge(outline: outline, cutouts: [BoardCutout.Shape.rounded(rect: r(10, 10, 10, 10), radius: 0).loop])
        XCTAssertTrue(edge.contains(p(5, 5)))
        XCTAssertFalse(edge.contains(p(15, 15)), "in the cutout")
        XCTAssertFalse(edge.contains(p(70, 5)), "outside the outline")
        XCTAssertTrue(edge.isInCutout(p(15, 15)))
        XCTAssertFalse(edge.isInCutout(p(5, 5)))
        XCTAssertFalse(edge.isInCutout(p(70, 5)))
    }

    func testOverlappingCutoutsAreStillHoles() {
        let a = BoardCutout.Shape.rounded(rect: r(10, 10, 10, 10), radius: 0).loop
        let b = BoardCutout.Shape.rounded(rect: r(15, 10, 10, 10), radius: 0).loop
        let edge = BoardEdge(outline: outline, cutouts: [a, b])
        // Even-odd over all loops would call the overlap material.
        XCTAssertFalse(edge.contains(p(17, 15)), "in both")
        XCTAssertFalse(edge.contains(p(12, 15)))
        XCTAssertFalse(edge.contains(p(22, 15)))
        XCTAssertTrue(edge.contains(p(27, 15)))
    }

    func testACutoutCrossingTheOutlineStillDefinesMaterial() {
        let notch = BoardCutout.Shape.rounded(rect: r(-5, 10, 10, 10), radius: 0).loop
        let edge = BoardEdge(outline: outline, cutouts: [notch])
        XCTAssertFalse(edge.contains(p(2, 15)))
        XCTAssertTrue(edge.contains(p(8, 15)))
        XCTAssertFalse(edge.isInCutout(p(-2, 15)), "outside the outline is not 'in a cutout'")
    }

    func testSegmentsCountIsTheSumOfTheLoopsPoints() {
        let hole = BoardCutout.Shape.circle(centre: p(30, 20), diameter: 6).loop
        let edge = BoardEdge(outline: outline, cutouts: [hole, hole])
        XCTAssertEqual(edge.segments.count, 4 + 2 * hole.count)
        XCTAssertEqual(edge.loops.count, 3)
        XCTAssertEqual(edge.bounds, r(0, 0, 60, 40))
    }

    func testAnEmptyOutlineContainsNothing() {
        let edge = BoardEdge(outline: [])
        XCTAssertFalse(edge.contains(p(1, 1)))
        XCTAssertEqual(edge.bounds, .zero)
        XCTAssertTrue(edge.segments.isEmpty)
    }

    func testThePathFillsEvenOddWithHoles() {
        let edge = BoardEdge(outline: outline, cutouts: [BoardCutout.Shape.rounded(rect: r(10, 10, 10, 10), radius: 0).loop])
        XCTAssertTrue(edge.path.contains(p(5, 5), using: .evenOdd))
        XCTAssertFalse(edge.path.contains(p(15, 15), using: .evenOdd))
    }
}
