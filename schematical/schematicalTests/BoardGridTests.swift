import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The PCB's two snap grids: the component grid (set in the sidebar's Grid
/// section, saved in the file) and the trace grid, half of it.
@MainActor
final class BoardGridTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    func testDefaultsAndTraceGridIsHalf() {
        let doc = TestSupport.newDocument()
        XCTAssertEqual(doc.board.grid, 0.635)
        XCTAssertEqual(doc.board.traceGrid, 0.3175)
        doc.setBoardGrid(1.27)
        XCTAssertEqual(doc.board.traceGrid, 0.635)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Grid")
    }

    func testGridClamps() {
        let doc = TestSupport.newDocument()
        doc.setBoardGrid(0)
        XCTAssertEqual(doc.board.grid, BoardSettings.gridRange.lowerBound)
        doc.setBoardGrid(50)
        XCTAssertEqual(doc.board.grid, BoardSettings.gridRange.upperBound)
    }

    /// Footprints and texts snap to the component grid.
    func testFootprintsAndTextsSnapToTheComponentGrid() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardGrid(1.27)
        let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        doc.moveFootprintOrigin(chip.id, to: p(3.1, 4.5))
        let origin = try XCTUnwrap(doc.chips.first { $0.id == chip.id }?.boardOrigin)
        XCTAssertEqual(origin.x, 2.54, accuracy: 1e-9)
        XCTAssertEqual(origin.y, 5.08, accuracy: 1e-9)

        let text = doc.addText(at: p(1.0, 1.9))
        XCTAssertEqual(doc.texts.first { $0.id == text }?.anchor, p(1.27, 1.27))
    }

    /// A trace end off any pad or trace lands on the trace grid, which has
    /// a point halfway between component-grid points.
    func testTraceEndsSnapToTheTraceGrid() {
        let doc = TestSupport.newDocument()
        let end = doc.traceAnchor(at: p(0.33, 0.95), layer: .top, tolerance: 0.01)
        XCTAssertEqual(end.x, 0.3175, accuracy: 1e-9)
        XCTAssertEqual(end.y, 0.9525, accuracy: 1e-9)
        doc.setBoardGrid(2.54)
        let coarse = doc.traceAnchor(at: p(1.0, 2.0), layer: .top, tolerance: 0.01)
        XCTAssertEqual(coarse, p(1.27, 2.54))
    }

    /// Changing the grid moves nothing.
    func testChangingTheGridMovesNothing() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        let before = try XCTUnwrap(doc.chips.first { $0.id == chip.id }?.board)
        doc.setBoardGrid(1.0)
        XCTAssertEqual(doc.chips.first { $0.id == chip.id }?.board, before)
    }

    func testGridRoundTripsAndDefaultsWhenMissing() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardGrid(0.5)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.grid, 0.5)

        // An older file without the key gets the default.
        XCTAssertEqual(try BoardLayersTests.open(board: #"{ "layers": 2 }"#).board.grid, 0.635)
        // A stored text snaps to the file's grid, not the default.
        let id = UUID()
        let withText = try BoardLayersTests.open(board: """
            { "grid": 2, "texts": [ { "id": "\(id)", "text": "A", "x": 2.9, "y": 1.2 } ] }
            """)
        XCTAssertEqual(withText.texts.first?.anchor, p(2, 2))
    }

    func testBadGridIsRefused() {
        for value in ["0", "11", "-1"] {
            let message = BoardLayersTests.pcbError {
                _ = try BoardLayersTests.open(board: #"{ "grid": \#(value) }"#)
            }
            XCTAssertEqual(message, "board grid \(value) must be 0.01 to 10 mm")
        }
    }

    func testPreciseFormat() {
        XCTAssertEqual(LengthField.format(0.635, units: .mm, precise: true), "0.635")
        XCTAssertEqual(LengthField.format(0.3175, units: .mm, precise: true), "0.3175")
        XCTAssertEqual(LengthField.format(0.3175, units: .inch, precise: true), "0.0125")
        XCTAssertEqual(LengthField.format(0.635, units: .mm), "0.64", "other fields unchanged")
    }

    // MARK: The default origin

    /// A newly placed footprint's origin is pad 1's centre, on the grid.
    func testNewFootprintsUsePad1Centre() throws {
        let doc = TestSupport.newDocument()
        for _ in 0..<2 {
            let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
            let placed = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
            XCTAssertEqual(placed.board?.origin, .pad1Center)
            let pad = try XCTUnwrap(placed.boardPads.first { $0.designator == placed.part.firstPinDesignator })
            let origin = try XCTUnwrap(placed.boardOrigin)
            XCTAssertEqual(origin.x, pad.rect.midX, accuracy: 1e-9)
            XCTAssertEqual(origin.y, pad.rect.midY, accuracy: 1e-9)
            for value in [origin.x, origin.y] {
                let steps = value / BoardGrid.spacing
                XCTAssertEqual(steps, steps.rounded(), accuracy: 1e-9, "pad 1 on the grid")
            }
        }
        // Saved and reopened, it stays pad 1.
        XCTAssertEqual(try BoardLayersTests.roundTrip(doc).chips.map(\.board?.origin), [.pad1Center, .pad1Center])
    }

    /// A file saved before origins existed (no `origin`) still opens as top left.
    func testOldPlacementWithoutAnOriginIsTopLeft() throws {
        let part = """
            [ { "id": "\(UUID())", "reference": "U1", "partNumber": "SN74HC00D", "x": 0, "y": 0,
                "flipped": false, "pcb": { "x": 10, "y": 10, "rotation": 0 } } ]
            """
        let opened = try BoardLayersTests.open(board: #"{ "layers": 2 }"#, parts: part)
        XCTAssertEqual(opened.chips.first?.board?.origin, .topLeft)
    }
}
