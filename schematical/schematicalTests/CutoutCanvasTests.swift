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
}
