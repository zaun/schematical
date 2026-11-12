import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The Part Editor's footprint preview draws the PCB page's pads (plan-32-D Phase 2).
@MainActor
final class FootprintPreviewTests: XCTestCase {
    /// A bundled part whose footprint is `id`.
    private func part(using id: String) throws -> CatalogPart {
        try XCTUnwrap(TestSupport.allParts.first { $0.footprint?.id == id }, "no part uses \(id)")
    }

    private func check(_ id: String) throws {
        let part = try part(using: id)
        let preview = FootprintPreview.chip(for: part)
        let doc = TestSupport.newDocument()
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 200, y: 200))
        let placed = try XCTUnwrap(doc.chips.last)
        let board = try XCTUnwrap(placed.board)
        XCTAssertEqual(board.rotation, 0)
        let previewPads = preview.boardPads
        let pagePads = placed.boardPads
        XCTAssertEqual(previewPads.count, pagePads.count, id)
        XCTAssertFalse(previewPads.isEmpty, id)
        for (a, b) in zip(previewPads, pagePads) {
            // The page's part sits at its placement centre; the preview's at the origin.
            let moved = a.rect.offsetBy(dx: board.centre.x, dy: board.centre.y)
            XCTAssertEqual(a.designator, b.designator, id)
            XCTAssertEqual(moved.minX, b.rect.minX, accuracy: 1e-9, id)
            XCTAssertEqual(moved.minY, b.rect.minY, accuracy: 1e-9, id)
            XCTAssertEqual(moved.width, b.rect.width, accuracy: 1e-9, id)
            XCTAssertEqual(moved.height, b.rect.height, accuracy: 1e-9, id)
            XCTAssertEqual(a.drill, b.drill, id)
            XCTAssertEqual(a.shape, b.shape, id)
            XCTAssertEqual(a.corners, b.corners, id)
        }
        XCTAssertNotNil(preview.pin1Mark, id)
    }

    func testDIP() throws { try check("PDIP-14") }
    func testSON6() throws { try check("SON-6-DRY") }
    func testX2SON8() throws { try check("X2SON-8") }
}
