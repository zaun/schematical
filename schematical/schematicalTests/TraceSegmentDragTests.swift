import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// A plain drag on a trace slides the segment under the pointer; the trace
/// follows with its ends and joins staying put. (Cmd-drag moves it whole,
/// `moveBoardGroup`.)
@MainActor
final class TraceSegmentDragTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    // MARK: TraceRoute.slide

    /// A horizontal run with a 45° leg at each end: sliding the run keeps the
    /// legs at 45°, shorter or longer.
    func testSlidingTheMiddleKeepsTheNeighboursAngles() {
        let points = [p(0, 10), p(10, 0), p(30, 0), p(40, 10)]
        // Up (left of the direction is up on screen: y grows down).
        let slid = TraceRoute.slide(points, segment: 1, by: 4, anchored: [])
        XCTAssertEqual(slid.count, 4)
        XCTAssertEqual(slid[0], p(0, 10))
        XCTAssertEqual(slid[3], p(40, 10))
        // `by: 4` is to the left of +x, which is +y here: 4 mm down.
        XCTAssertEqual(slid[1].x, 6, accuracy: 1e-9)
        XCTAssertEqual(slid[1].y, 4, accuracy: 1e-9)
        XCTAssertEqual(slid[2].x, 34, accuracy: 1e-9)
        XCTAssertEqual(slid[2].y, 4, accuracy: 1e-9)
    }

    func testTheEndOfATraceKeepsItsPlaceAndAJogIsAdded() {
        let points = [p(0, 0), p(20, 0), p(20, 20)]
        let slid = TraceRoute.slide(points, segment: 0, by: 5, anchored: [])
        // The start stays; a leg runs to the slid segment, which meets the
        // second segment where it was.
        XCTAssertEqual(slid.first, p(0, 0))
        XCTAssertEqual(slid.last, p(20, 20))
        XCTAssertEqual(slid.count, 4)
        XCTAssertEqual(slid[1].y, 5, accuracy: 1e-9)
        XCTAssertEqual(slid[2].x, 20, accuracy: 1e-9)
        XCTAssertEqual(slid[2].y, 5, accuracy: 1e-9)
    }

    func testAnAnchoredCornerStaysAndGetsALeg() {
        let points = [p(0, 10), p(10, 0), p(30, 0), p(40, 10)]
        let slid = TraceRoute.slide(points, segment: 1, by: 4, anchored: [1])
        XCTAssertTrue(slid.contains(p(10, 0)), "the anchored corner stays")
        XCTAssertEqual(slid.count, 5)
        XCTAssertEqual(slid[2].x, 10, accuracy: 1e-9)
        XCTAssertEqual(slid[2].y, 4, accuracy: 1e-9)
    }

    func testNoDistanceChangesNothing() {
        let points = [p(0, 10), p(10, 0), p(30, 0), p(40, 10)]
        XCTAssertEqual(TraceRoute.slide(points, segment: 1, by: 0, anchored: []), points)
    }

    func testSlidingBackToWhereItWasRestoresTheTrace() {
        let points = [p(0, 10), p(10, 0), p(30, 0), p(40, 10)]
        let slid = TraceRoute.slide(points, segment: 1, by: 4, anchored: [])
        let back = TraceRoute.slide(slid, segment: 1, by: -4, anchored: [])
        XCTAssertEqual(back.count, points.count)
        for (a, b) in zip(back, points) {
            XCTAssertEqual(a.x, b.x, accuracy: 1e-9)
            XCTAssertEqual(a.y, b.y, accuracy: 1e-9)
        }
    }

    // MARK: The document

    private func doc(trace points: [CGPoint]) throws -> (doc: SchematicDocument, id: UUID) {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addTrace(points: points, layer: .top))
        doc.select(.trace(id))
        return (doc, id)
    }

    func testDraggingASegmentSlidesItOntoTheTraceGrid() throws {
        let (doc, id) = try doc(trace: [p(0, 0), p(10, 0), p(10, 10)])
        let base = try XCTUnwrap(doc.traceSegmentBase(id, at: p(5, 0)))
        XCTAssertEqual(base.segment, 0)
        doc.dragTraceSegment(base, by: CGSize(width: 3, height: 2.6))
        let points = try XCTUnwrap(doc.traces.first?.points)
        // Only the part at right angles to the segment counts: 2.6 mm,
        // snapped to the trace grid.
        let grid = doc.board.traceGrid
        let y = BoardGrid.snap(2.6, spacing: grid)
        XCTAssertEqual(points.first, p(0, 0), "the end stays")
        XCTAssertEqual(points[1].y, y, accuracy: 1e-9)
        XCTAssertEqual(points[1].x, 0, accuracy: 1e-9)
        XCTAssertEqual(points.last, p(10, 10))
    }

    func testTheSegmentNearestThePressIsTheOneHeld() throws {
        let (doc, id) = try doc(trace: [p(0, 0), p(10, 0), p(10, 10)])
        XCTAssertEqual(doc.traceSegmentBase(id, at: p(10, 8))?.segment, 1)
    }

    func testAJoinedCornerIsAnchored() throws {
        let (doc, id) = try doc(trace: [p(0, 0), p(10, 0), p(10, 10)])
        _ = doc.addTrace(points: [p(10, 0), p(20, 0)], layer: .top)
        let base = try XCTUnwrap(doc.traceSegmentBase(id, at: p(5, 0)))
        XCTAssertTrue(base.anchored.contains(1))
    }

    func testSegmentDragIsOneUndoStep() throws {
        let (doc, id) = try doc(trace: [p(0, 0), p(10, 0), p(10, 10)])
        let before = try XCTUnwrap(doc.traces.first?.points)
        doc.beginGesture(.pcb)
        let base = try XCTUnwrap(doc.traceSegmentBase(id, at: p(5, 0)))
        doc.dragTraceSegment(base, by: CGSize(width: 0, height: 1.5))
        doc.dragTraceSegment(base, by: CGSize(width: 0, height: 3))
        doc.settleTraceSegment(base)
        doc.endGesture()
        XCTAssertNotEqual(doc.traces.first?.points, before)
        XCTAssertEqual(doc.undoName(on: .pcb), "Move Segment")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.traces.first?.points, before)
    }

    func testALockedTraceCannotBeHeld() throws {
        let (doc, id) = try doc(trace: [p(0, 0), p(10, 0)])
        doc.setLocked(.trace(id), true)
        XCTAssertNil(doc.traceSegmentBase(id, at: p(5, 0)))
    }
}
