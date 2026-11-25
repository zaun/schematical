import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// A Custom board whose edge is a polygon (plan-65-B).
@MainActor
final class FreeFormOutlineTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// An L: 60 × 40 with the top-right 20 × 15 missing, counter-clockwise.
    private var lShapeCounterClockwise: [CGPoint] {
        [p(0, 0), p(0, 40), p(60, 40), p(60, 15), p(40, 15), p(40, 0)]
    }

    func testOutlineIsThePolygonWhileCustomAndTheRectangleAfterReset() throws {
        let doc = TestSupport.newDocument()
        XCTAssertFalse(doc.board.isFreeForm)
        XCTAssertTrue(doc.setBoardOutline(lShapeCounterClockwise))
        XCTAssertTrue(doc.board.isFreeForm)
        XCTAssertEqual(doc.board.outline.count, 6)
        XCTAssertEqual(Set(doc.board.outline), Set(lShapeCounterClockwise))
        XCTAssertGreaterThan(BoardGeometry.area(doc.board.outline), 0, "stored clockwise")
        XCTAssertEqual(doc.board.width, 60)
        XCTAssertEqual(doc.board.height, 40)
        XCTAssertEqual(doc.boardEdge.outline, doc.board.outline)

        doc.resetBoardOutline()
        XCTAssertEqual(doc.undoName, "Reset Board Outline")
        XCTAssertFalse(doc.board.isFreeForm)
        XCTAssertEqual(doc.board.outline, [p(0, 0), p(60, 0), p(60, 40), p(0, 40)])
        XCTAssertEqual(doc.board.width, 60)
        XCTAssertEqual(doc.board.height, 40)
    }

    func testRefusalsLeaveNoStep() {
        let doc = TestSupport.newDocument()
        let before = doc.board
        XCTAssertFalse(doc.setBoardOutline([p(0, 0), p(10, 10), p(10, 0), p(0, 10)]), "bow-tie")
        XCTAssertFalse(doc.setBoardOutline([p(0, 0), p(10, 0)]), "two points")
        XCTAssertFalse(doc.setBoardOutline([p(5, 0), p(15, 0), p(15, 10), p(5, 10)]), "top-left is (5, 0)")
        XCTAssertFalse(doc.setBoardOutline([p(0, 0), p(1200, 0), p(1200, 10), p(0, 10)]), "1200 mm wide")
        XCTAssertNil(doc.undoName)
        XCTAssertEqual(doc.board, before)
    }

    func testProblemMessagesAreTheFileDetails() {
        XCTAssertEqual(BoardSettings.problem(outline: [p(0, 0), p(10, 0)]), "needs at least 3 points")
        XCTAssertEqual(BoardSettings.problem(outline: [p(0, 0), p(10, 10), p(10, 0), p(0, 10)]), "crosses itself")
        XCTAssertEqual(BoardSettings.problem(outline: [p(5, 2), p(15, 2), p(15, 10), p(5, 10)]),
                       "must start at (0, 0): its top-left is (5, 2)")
        XCTAssertEqual(BoardSettings.problem(outline: [p(0, 0), p(1200, 0), p(1200, 10), p(0, 10)]),
                       "is 1200 mm wide; sizes are 1 to 1000 mm")
        XCTAssertEqual(BoardSettings.problem(outline: [p(0, 0), p(10, 0), p(10, 1200), p(0, 1200)]),
                       "is 1200 mm high; sizes are 1 to 1000 mm")
        XCTAssertNil(BoardSettings.problem(outline: [p(0, 0), p(10, 0), p(10, 10), p(0, 10)]))
    }

    func testSetBoardSizeLeavesAFreeFormBoardAlone() {
        let doc = TestSupport.newDocument()
        doc.setBoardOutline(lShapeCounterClockwise)
        let before = doc.board
        doc.setBoardSize(width: 100, height: 100)
        XCTAssertEqual(doc.board, before)
    }

    func testLeavingCustomDropsTheOutline() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardOutline(lShapeCounterClockwise)
        let card = try XCTUnwrap(BoardType.allCases.first { $0 == .customCard })
        doc.setBoardType(card)
        XCTAssertNil(doc.board.customOutline)
        doc.setBoardType(.custom)
        XCTAssertFalse(doc.board.isFreeForm)
        XCTAssertEqual(doc.board.outline.count, 4)
    }

    func testUndoAndRedoRestoreTypeOutlineAndSize() throws {
        let doc = TestSupport.newDocument()
        let before = doc.board
        doc.setBoardOutline(lShapeCounterClockwise)
        XCTAssertEqual(doc.undoName, "Change Board Outline")
        let after = doc.board
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.board, before)
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.board, after)
        XCTAssertEqual(doc.board.type, .custom)
        XCTAssertEqual(doc.board.width, 60)
        XCTAssertEqual(doc.board.height, 40)
    }

    func testCutoutsAndPartsDoNotMove() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: CGRect(x: 10, y: 10, width: 8, height: 4), radius: 0)))
        let cutouts = doc.cutouts
        let chips = doc.chips
        doc.setBoardOutline(lShapeCounterClockwise)
        XCTAssertEqual(doc.cutouts, cutouts)
        XCTAssertEqual(doc.chips, chips)
        XCTAssertEqual(doc.cutouts.map(\.id), [id])
    }
}
