import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// File ▸ Import ▸ DXF…: the sheet's state, the placement and the document step (plan-65-I).
@MainActor
final class DXFImportTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func makeState(_ text: String, doc: SchematicDocument? = nil, name: String = "board.dxf") throws -> (DXFImportState, SchematicDocument) {
        let doc = doc ?? TestSupport.newDocument()
        return (DXFImportState(fileName: name, drawing: try DXFReader.parse(text), board: doc.board), doc)
    }

    private func bounds(_ points: [CGPoint]) -> CGRect { BoardSettings.bounds(of: points) }

    /// A drawing of `entities` (DXF text lines) on layer `layer`, in mm.
    private func drawing(_ body: String, units: Int = 4) -> String {
        "  0\nSECTION\n  2\nHEADER\n  9\n$INSUNITS\n 70\n\(units)\n  0\nENDSEC\n  0\nSECTION\n  2\nENTITIES\n\(body)  0\nENDSEC\n  0\nEOF\n"
    }

    private func rect(_ x: Double, _ y: Double, _ w: Double, _ h: Double, layer: String = "Edge") -> String {
        let corners = [(x, y), (x + w, y), (x + w, y + h), (x, y + h)]
        var out = "  0\nLWPOLYLINE\n  8\n\(layer)\n 90\n4\n 70\n1\n"
        for (cx, cy) in corners { out += " 10\n\(cx)\n 20\n\(cy)\n" }
        return out
    }

    private func circle(_ x: Double, _ y: Double, _ r: Double, layer: String = "Edge") -> String {
        "  0\nCIRCLE\n  8\n\(layer)\n 10\n\(x)\n 20\n\(y)\n 40\n\(r)\n"
    }

    // MARK: Phase 1 — placement and the document step

    func testOutlineAndCutoutsMakeAFreeFormBoard() throws {
        let (state, doc) = try makeState(DXFFixtures.roundedBoard)
        XCTAssertEqual(state.mode, .outlineAndCutouts)
        XCTAssertNil(state.problem)
        doc.applyDXFImport(state.placed, mode: state.mode)
        XCTAssertEqual(doc.undoName, "Import DXF")
        XCTAssertTrue(doc.board.isFreeForm)
        XCTAssertEqual(doc.board.type, .custom)
        XCTAssertEqual(doc.board.width, 60, accuracy: 0.01)
        XCTAssertEqual(doc.board.height, 40, accuracy: 0.01)
        XCTAssertEqual(doc.cutouts.count, 2)
        XCTAssertEqual(doc.boardSelection.items.count, 2, "the new cutouts are selected")
        let box = bounds(doc.board.outline)
        XCTAssertEqual(box.minX, 0, accuracy: 1e-9)
        XCTAssertEqual(box.minY, 0, accuracy: 1e-9)
    }

    func testKiCadsYUpAndNegativeCoordinatesAreFlippedAndMoved() throws {
        let (state, doc) = try makeState(DXFFixtures.kicadEdgeProbe)
        doc.applyDXFImport(state.placed, mode: .outlineAndCutouts)
        let outline = bounds(doc.board.outline)
        XCTAssertEqual(outline.width, 40, accuracy: 1e-6)
        XCTAssertEqual(outline.height, 30, accuracy: 1e-6)
        XCTAssertEqual(outline.minY, 0, accuracy: 1e-9)
        // The 3 mm circle's centre was at y = -15 in the DXF: 15 from the top once flipped.
        let circle = try XCTUnwrap(doc.cutouts.map(\.shape.bounds).max { $0.height < $1.height && $0.width < 7 })
        XCTAssertEqual(Double(circle.minY), 12, accuracy: 0.1)
        XCTAssertEqual(Double(circle.maxY), 18, accuracy: 0.1)
        XCTAssertEqual(doc.cutouts.count, 2)
    }

    func testTheFilesUnitsAreUsedAndTheDocumentsWhenItHasNone() throws {
        let (inch, doc) = try makeState(DXFFixtures.inchBoard)
        XCTAssertEqual(inch.units, .inch)
        doc.applyDXFImport(inch.placed, mode: .outlineAndCutouts)
        XCTAssertEqual(doc.board.width, 50.8, accuracy: 0.01)
        XCTAssertEqual(doc.board.height, 25.4, accuracy: 0.01)

        let unknown = TestSupport.newDocument()
        let (none, _) = try makeState(DXFFixtures.openChain, doc: unknown)
        XCTAssertEqual(none.units, .millimetre, "the file has none: the board's units (mm)")
        unknown.setBoardUnits(.inch)
        let (inches, _) = try makeState(DXFFixtures.openChain, doc: unknown)
        XCTAssertEqual(inches.units, .inch)
        inches.units = .millimetre
        XCTAssertEqual(try XCTUnwrap(inches.placed.outline.map(bounds)).width, 10, accuracy: 1e-9)
    }

    func testCutoutsOnlyPutsTheOriginWhereTheOffsetSays() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 60, height: 40)
        let before = doc.board
        let (state, _) = try makeState(drawing(circle(15, 20, 1.6) + rect(40, 5, 6, 4)), doc: doc)
        state.mode = .cutoutsOnly
        XCTAssertEqual(state.offsetX, 0)
        XCTAssertEqual(state.offsetY, 40, "the board's bottom-left")
        XCTAssertNil(state.problem)
        doc.applyDXFImport(state.placed, mode: .cutoutsOnly)
        XCTAssertEqual(doc.board, before, "the board's edge is untouched")
        XCTAssertEqual(doc.cutouts.count, 2)
        let circle = try XCTUnwrap(doc.cutouts.first { $0.shape.bounds.width < 4 }).shape.bounds
        XCTAssertEqual(Double(circle.midX), 15, accuracy: 0.05)
        XCTAssertEqual(Double(circle.midY), 20, accuracy: 0.05, "y 20 up from the bottom of a 40 mm board")
        state.offsetX = 10
        state.offsetY = 30
        let moved = try XCTUnwrap(state.placed.cutouts.first { bounds($0).width < 4 }.map(bounds))
        XCTAssertEqual(Double(moved.midX), 25, accuracy: 0.05)
        XCTAssertEqual(Double(moved.midY), 10, accuracy: 0.05)
    }

    func testAnImportIsOneExactlyUndoableStep() throws {
        let doc = TestSupport.newDocument()
        _ = doc.addCutout(.rounded(rect: CGRect(x: 5, y: 5, width: 4, height: 4), radius: 0))
        _ = doc.addCutout(.rounded(rect: CGRect(x: 20, y: 5, width: 4, height: 4), radius: 0))
        let card = try XCTUnwrap(BoardType.allCases.first { $0 != .custom && $0 != .customCard })
        doc.setBoardType(card)
        doc.clearBoardSelection()
        let before = doc.state
        let (state, _) = try makeState(DXFFixtures.roundedBoard, doc: doc)
        doc.applyDXFImport(state.placed, mode: .outlineAndCutouts)
        XCTAssertEqual(doc.board.type, .custom)
        XCTAssertEqual(doc.cutouts.count, 2, "the two it had are replaced by the DXF's two")
        XCTAssertNotEqual(doc.state, before)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.state, before, "board, size, type and cutouts come back together")
        XCTAssertTrue(doc.redo())
        XCTAssertTrue(doc.board.isFreeForm)
    }

    func testRefusalsAndTheirReasons() throws {
        // 1200 mm wide.
        let (wide, _) = try makeState(drawing(rect(0, 0, 1200, 10)))
        XCTAssertEqual(wide.problem, "The outline is 1200 mm wide; sizes are 1 to 1000 mm.")
        let doc = TestSupport.newDocument()
        let before = doc.state
        doc.applyDXFImport(wide.placed, mode: .outlineAndCutouts)
        XCTAssertEqual(doc.state, before)
        XCTAssertNil(doc.undoName)
        // No layers chosen.
        wide.layers = []
        XCTAssertEqual(wide.problem, "The drawing has no closed outline on the chosen layers.")
        wide.mode = .cutoutsOnly
        XCTAssertEqual(wide.problem, "The drawing has no closed loops on the chosen layers.")
        // An outline and nothing in it is fine in outline mode and gives no cutouts to add in the other.
        let (onlyOutline, _) = try makeState(drawing(rect(0, 0, 60, 40)))
        XCTAssertNil(onlyOutline.problem)
        XCTAssertEqual(onlyOutline.summary, "1 outline, 0 cutouts")
    }

    func testTheSummaryAndTheNotes() throws {
        let (rounded, _) = try makeState(DXFFixtures.roundedBoard)
        XCTAssertEqual(rounded.summary, "1 outline, 2 cutouts")
        XCTAssertEqual(rounded.notes, ["TEXT ×1 skipped"])
        let (kicad, _) = try makeState(DXFFixtures.kicadEdgeProbe)
        kicad.mode = .cutoutsOnly
        XCTAssertEqual(kicad.summary, "1 cutout")
        XCTAssertEqual(kicad.notes, ["2 inside a cutout"])
        let (open, _) = try makeState(DXFFixtures.openChain)
        XCTAssertEqual(open.notes, ["1 open chain not closed"])
    }

    func testTheDefaultLayers() throws {
        let body = rect(0, 0, 60, 40, layer: "Edge") + rect(5, 5, 10, 10, layer: "Dims")
        let (state, _) = try makeState(drawing(body))
        XCTAssertEqual(state.layers, ["Edge"])
        state.layers = ["Edge", "Dims"]
        XCTAssertEqual(state.plan.cutouts.count, 1)
        let (plain, _) = try makeState(drawing(rect(0, 0, 60, 40, layer: "A") + rect(5, 5, 10, 10, layer: "B")))
        XCTAssertEqual(plain.layers, ["A", "B"])
    }

    func testThePreviewNamesEachLoopsRole() throws {
        let (state, _) = try makeState(DXFFixtures.roundedBoard)
        XCTAssertEqual(state.previewLoops.map(\.role).sorted { "\($0)" < "\($1)" }, [.cutout, .cutout, .outline])
        state.mode = .cutoutsOnly
        XCTAssertEqual(state.previewLoops.filter { $0.role == .ignored }.count, 2)
    }
}
