import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The Cutout panel and the free-form board's Board panel (plan-65-F).
@MainActor
final class CutoutPanelTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect { CGRect(x: x, y: y, width: w, height: h) }

    func testTheSidebarShowsACutoutWhenItIsTheSoleSelection() throws {
        let doc = TestSupport.newDocument()
        XCTAssertEqual(PCBSidebarView.content(for: doc), .board)
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 10, 4), radius: 0)))
        XCTAssertEqual(PCBSidebarView.content(for: doc), .cutout(id))
        let hole = doc.addHole(at: p(40, 40))
        doc.select(.cutout(id))
        doc.toggleSelection(.hole(hole))
        XCTAssertEqual(PCBSidebarView.content(for: doc), .selection, "two items: the multi-selection panel")
    }

    func testEachFieldIsANamedStepAndTheRadiusIsClamped() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 10, 4), radius: 0)))
        CutoutPanelActions.setX(doc, id, 20)
        XCTAssertEqual(doc.undoName, "Move")
        XCTAssertEqual(try XCTUnwrap(CutoutPanelActions.shape(doc, id)).bounds.minX, 20, accuracy: 0.2)
        CutoutPanelActions.setY(doc, id, 25)
        XCTAssertEqual(try XCTUnwrap(CutoutPanelActions.shape(doc, id)).bounds.minY, 25, accuracy: 0.2)
        CutoutPanelActions.setWidth(doc, id, 12)
        XCTAssertEqual(doc.undoName, "Resize")
        XCTAssertEqual(try XCTUnwrap(CutoutPanelActions.shape(doc, id)).bounds.width, 12, accuracy: 0.2)
        CutoutPanelActions.setHeight(doc, id, 6)
        XCTAssertEqual(try XCTUnwrap(CutoutPanelActions.shape(doc, id)).bounds.height, 6, accuracy: 0.2)
        CutoutPanelActions.setRadius(doc, id, 99)
        XCTAssertEqual(doc.undoName, "Change Cutout")
        let box = try XCTUnwrap(CutoutPanelActions.shape(doc, id)).bounds
        XCTAssertEqual(try XCTUnwrap(CutoutPanelActions.radius(doc, id)), min(box.width, box.height) / 2, accuracy: 1e-9)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(try XCTUnwrap(CutoutPanelActions.radius(doc, id)), 0, accuracy: 1e-9)
    }

    func testALockedCutoutsSettersChangeNothing() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 10, 4), radius: 0)))
        doc.setLocked(.cutout(id), true)
        let before = doc.cutouts
        CutoutPanelActions.setX(doc, id, 30)
        CutoutPanelActions.setWidth(doc, id, 30)
        CutoutPanelActions.setRadius(doc, id, 1)
        XCTAssertEqual(doc.cutouts, before)
    }

    func testAPolygonMovesByXAndYAndKeepsItsSize() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.polygon([p(10, 10), p(30, 10), p(20, 24)])))
        let size = try XCTUnwrap(CutoutPanelActions.shape(doc, id)).bounds.size
        XCTAssertNil(CutoutPanelActions.radius(doc, id))
        CutoutPanelActions.setX(doc, id, 40)
        CutoutPanelActions.setY(doc, id, 5)
        CutoutPanelActions.setWidth(doc, id, 99)
        let box = try XCTUnwrap(CutoutPanelActions.shape(doc, id)).bounds
        XCTAssertEqual(box.minX, 40, accuracy: 0.2)
        XCTAssertEqual(box.minY, 5, accuracy: 0.2)
        XCTAssertEqual(box.width, size.width, accuracy: 1e-9)
        XCTAssertEqual(box.height, size.height, accuracy: 1e-9)
    }

    func testResetToRectangleOnAFreeFormBoard() throws {
        let doc = TestSupport.newDocument()
        XCTAssertFalse(doc.board.isFreeForm, "a plain board offers no reset")
        XCTAssertTrue(doc.setBoardOutline([p(0, 0), p(60, 0), p(60, 15), p(40, 15), p(40, 40), p(0, 40)]))
        XCTAssertTrue(doc.board.isFreeForm)
        let polygon = doc.board.outline
        doc.resetBoardOutline()
        XCTAssertEqual(doc.board.outline, [p(0, 0), p(60, 0), p(60, 40), p(0, 40)])
        XCTAssertEqual(doc.board.width, 60)
        XCTAssertEqual(doc.board.height, 40)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.board.outline, polygon)
    }
}
