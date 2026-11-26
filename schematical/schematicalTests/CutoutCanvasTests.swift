import AppKit
import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// The PCB page and cutouts (plan-65-E): the board drawn with holes, the
/// Cutout tool, selecting, moving and resizing.
@MainActor
final class CutoutCanvasTests: XCTestCase {
    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect { CGRect(x: x, y: y, width: w, height: h) }
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    // MARK: Phase 1 — drawing

    func testTheBoardPathHasTheCutoutAsAHole() {
        let hole = BoardCutout.Shape.rounded(rect: r(10, 10, 10, 10), radius: 0).loop
        let edge = BoardEdge(outline: [p(0, 0), p(60, 0), p(60, 40), p(0, 40)], cutouts: [hole])
        let path = PCBCanvasView.boardPath(edge).cgPath
        XCTAssertTrue(path.contains(p(5, 5), using: .evenOdd))
        XCTAssertFalse(path.contains(p(15, 15), using: .evenOdd))
    }

    func testWithNoCutoutsTheBoardPathIsTheOutline() {
        let outline = [p(0, 0), p(60, 0), p(60, 40), p(0, 40)]
        let drawn = PCBCanvasView.boardPath(BoardEdge(outline: outline))
        let old = Path { path in path.addLines(outline); path.closeSubpath() }
        XCTAssertEqual(drawn, old)
    }

    /// The alpha of the pixel of a transparent PCB export at board point `mm`.
    private func alpha(_ data: Data, _ doc: SchematicDocument, at mm: CGPoint) throws -> CGFloat {
        let rep = try XCTUnwrap(NSBitmapImageRep(data: data))
        let bounds = ImageExport.pcbBounds(doc)
        let ppm = BoardScale.pointsPerMM * CGFloat(rep.pixelsWide) / (ceil(bounds.width * BoardScale.pointsPerMM))
        let x = Int((mm.x - bounds.minX) * ppm), y = Int((mm.y - bounds.minY) * ppm)
        return try XCTUnwrap(rep.colorAt(x: x, y: y)).alphaComponent
    }

    func testAnExportedBoardHasNothingInsideACutout() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 60, height: 40)
        _ = try XCTUnwrap(doc.addCutout(.rounded(rect: r(20, 10, 20, 20), radius: 0)))
        var options = ImageExportOptions()
        options.view = .pcb
        options.format = .png
        options.scale = 1
        options.background = .transparent
        let data = try XCTUnwrap(ImageExport.data(for: doc, options))
        XCTAssertEqual(try alpha(data, doc, at: p(30, 20)), 0, accuracy: 0.01, "the hole is empty")
        XCTAssertGreaterThan(try alpha(data, doc, at: p(17, 20)), 0.9, "3 mm outside the cutout is board")
    }

    // MARK: Phase 2 — the tool, selecting, resizing, deleting

    func testTheToolsStringsAndSymbol() {
        XCTAssertEqual(PCBTool.cutout.title, "Cutout")
        XCTAssertFalse(PCBTool.cutout.tooltip.isEmpty)
        XCTAssertNotNil(NSImage(systemSymbolName: PCBTool.cutout.symbol, accessibilityDescription: nil), PCBTool.cutout.symbol)
        XCTAssertEqual(CutoutShape.allCases.map(\.title), ["Rectangle", "Circle", "Slot"])
    }

    func testFinishRectMakesEachShape() throws {
        let doc = TestSupport.newDocument()
        XCTAssertTrue(PCBCanvasView.finishRect(doc, kind: .cutout, from: p(10, 10), to: p(30, 20), moved: 50, layer: .top,
                                               cutoutShape: .rectangle))
        XCTAssertTrue(PCBCanvasView.finishRect(doc, kind: .cutout, from: p(10, 30), to: p(30, 36), moved: 50, layer: .top,
                                               cutoutShape: .slot))
        // A circle is square: the longer drag side, in the drag's direction.
        XCTAssertTrue(PCBCanvasView.finishRect(doc, kind: .cutout, from: p(40, 20), to: p(46, 26), moved: 50, layer: .top,
                                               cutoutShape: .circle))
        XCTAssertEqual(doc.cutouts.map(\.kind), [.rectangle, .slot, .circle])
        let circle = try XCTUnwrap(doc.cutouts.last).shape.bounds
        XCTAssertEqual(circle.width, circle.height, accuracy: 1e-9)
        XCTAssertTrue(PCBCanvasView.finishRect(doc, kind: .cutout, from: p(55, 20), to: p(51, 24), moved: 50, layer: .top,
                                               cutoutShape: .circle), "dragged up and to the left")
        XCTAssertEqual(doc.cutouts.last?.kind, .circle)
        XCTAssertLessThan(try XCTUnwrap(doc.cutouts.last).shape.bounds.maxX, 55.5)
    }

    func testATooSmallDragMakesNothing() {
        let doc = TestSupport.newDocument()
        XCTAssertFalse(PCBCanvasView.finishRect(doc, kind: .cutout, from: p(10, 10), to: p(30, 20), moved: 2, layer: .top))
        XCTAssertFalse(PCBCanvasView.finishRect(doc, kind: .cutout, from: p(10, 10), to: p(10.3, 20), moved: 50, layer: .top))
        XCTAssertTrue(doc.cutouts.isEmpty)
        XCTAssertNil(doc.undoName)
    }

    func testPourItemFindsACutoutByItsEdgeOnly() throws {
        let doc = TestSupport.newDocument()
        let polygon = try XCTUnwrap(doc.addCutout(.polygon([p(10, 10), p(30, 10), p(20, 24)])))
        let circle = try XCTUnwrap(doc.addCutout(.circle(centre: p(50, 20), diameter: 10)))
        XCTAssertEqual(doc.pourItem(at: p(20, 10), tolerance: 0.5), .cutout(polygon), "on the polygon's top edge")
        XCTAssertNil(doc.pourItem(at: p(20, 14), tolerance: 0.5), "inside the polygon")
        XCTAssertEqual(doc.pourItem(at: p(55.1, 20), tolerance: 0.5), .cutout(circle), "on the circle's edge")
        XCTAssertNil(doc.pourItem(at: p(50, 20), tolerance: 0.5), "the circle's middle")
        XCTAssertNil(doc.pourItem(at: p(58, 20), tolerance: 0.5), "3 mm outside")
        // The circle's bounding-box corner is not its edge.
        XCTAssertNil(doc.pourItem(at: p(45, 15), tolerance: 0.5))
        XCTAssertEqual(doc.boardItem(at: p(55.1, 20), tolerance: 0.5), .cutout(circle), "the Select tool's hit test")
    }

    func testAHandleDragResizesARoundedCutoutAsOneStep() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 10, 6), radius: 0)))
        let before = doc.cutouts
        let box = try XCTUnwrap(doc.cutouts.first).shape.bounds
        let hit = try XCTUnwrap(doc.pourHandle(at: p(box.maxX, box.maxY), tolerance: 0.5))
        XCTAssertEqual(hit.item, .cutout(id))
        XCTAssertEqual(hit.handle, .bottomRight)
        doc.resizePourItem(hit.item, handle: hit.handle, to: p(box.maxX + 5, box.maxY + 3))
        XCTAssertGreaterThan(try XCTUnwrap(doc.cutouts.first).shape.bounds.width, box.width)
        XCTAssertEqual(doc.undoName, "Resize")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.cutouts, before)
    }

    func testAPolygonAndALockedCutoutHaveNoHandles() throws {
        let doc = TestSupport.newDocument()
        let polygon = try XCTUnwrap(doc.addCutout(.polygon([p(10, 10), p(30, 10), p(20, 24)])))
        XCTAssertNil(doc.pourHandle(at: p(10, 10), tolerance: 1))
        XCTAssertNil(doc.resizableRect)
        doc.deleteCutout(polygon)
        let rounded = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 10, 6), radius: 0)))
        XCTAssertNotNil(doc.resizableRect)
        doc.setLocked(.cutout(rounded), true)
        XCTAssertNil(doc.resizableRect)
    }

    func testTheDeleteToolRemovesACutoutOnItsEdge() throws {
        let doc = TestSupport.newDocument()
        _ = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 10, 6), radius: 0)))
        XCTAssertFalse(doc.deleteBoardItem(at: p(15, 13), tolerance: 0.5), "3 mm inside it")
        XCTAssertEqual(doc.cutouts.count, 1)
        let edgeX = try XCTUnwrap(doc.cutouts.first).shape.bounds.minX
        XCTAssertTrue(doc.deleteBoardItem(at: p(edgeX, 13), tolerance: 0.5))
        XCTAssertTrue(doc.cutouts.isEmpty)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.cutouts.count, 1)
    }
}
