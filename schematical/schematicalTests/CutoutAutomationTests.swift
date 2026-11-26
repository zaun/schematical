import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The agent tools for cutouts and the free-form outline (plan-65-J).
@MainActor
final class CutoutAutomationTests: XCTestCase {
    private struct Result {
        let text: String
        let isError: Bool
    }

    private func newWindow() -> (id: String, doc: SchematicDocument) {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let open = TestSupport.openSystemDocument(of: doc, catalog: TestSupport.catalog, until: self)
        return (open.model.id.uuidString, doc)
    }

    private func call(_ tool: String, _ window: String, _ args: [String: Any] = [:]) async -> Result {
        var args = args
        args["window"] = window
        let result = await AutomationTools.call(tool, args)
        let content = result["content"] as? [[String: Any]] ?? []
        return Result(text: content.compactMap { $0["text"] as? String }.joined(separator: "\n"),
                      isError: result["isError"] as? Bool ?? false)
    }

    func testEachShapeThroughAddCutout() async throws {
        let (id, doc) = newWindow()
        var result = await call("add_cutout", id, ["x": 10, "y": 10, "width": 8, "height": 4])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertTrue(result.text.contains("rectangle"), result.text)
        result = await call("add_cutout", id, ["shape": "circle", "x": 40, "y": 30, "diameter": 6])
        XCTAssertTrue(result.text.contains("circle"), result.text)
        result = await call("add_cutout", id, ["shape": "slot", "x": 10, "y": 30, "width": 10, "height": 2])
        XCTAssertTrue(result.text.contains("slot"), result.text)
        result = await call("add_cutout", id, ["shape": "polygon", "points": [[60, 10], [70, 10], [65, 18]]])
        XCTAssertTrue(result.text.contains("polygon"), result.text)
        XCTAssertTrue(result.text.contains("points=3"), result.text)
        XCTAssertEqual(doc.cutouts.map(\.kind), [.rectangle, .circle, .slot, .polygon])
        let circle = try XCTUnwrap(doc.cutouts.first { $0.kind == .circle }).shape.bounds
        XCTAssertEqual(Double(circle.midX), 40, accuracy: 0.2, "x, y are a circle's centre")
        XCTAssertEqual(Double(circle.midY), 30, accuracy: 0.2)
        result = await call("add_cutout", id, ["x": 10, "y": 50, "width": 8, "height": 4, "radius": 1])
        XCTAssertTrue(result.text.contains("radius=1"), result.text)
    }

    func testABadShapeIsAnErrorThatNamesTheRule() async {
        let (id, doc) = newWindow()
        var result = await call("add_cutout", id, ["x": 10, "y": 10, "width": 0.1, "height": 4])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("at least 0.5 mm"), result.text)
        result = await call("add_cutout", id, ["shape": "polygon", "points": [[0, 0], [4, 4], [4, 0], [0, 4]]])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("crosses itself"), result.text)
        result = await call("add_cutout", id, ["shape": "circle", "x": 10])
        XCTAssertTrue(result.isError, "a circle needs y and diameter")
        XCTAssertTrue(doc.cutouts.isEmpty)
        XCTAssertNil(doc.undoName)
    }

    func testSetCutoutChangesAndLocks() async throws {
        let (id, doc) = newWindow()
        _ = await call("add_cutout", id, ["x": 10, "y": 10, "width": 8, "height": 4])
        let cutout = try XCTUnwrap(doc.cutouts.first)
        let name = "cutout:\(String(cutout.id.uuidString.prefix(8)))"
        var result = await call("set_cutout", id, ["id": name, "x": 20, "y": 25, "width": 12, "radius": 1])
        XCTAssertFalse(result.isError, result.text)
        let box = try XCTUnwrap(doc.cutouts.first).shape.bounds
        XCTAssertEqual(Double(box.minX), 20, accuracy: 0.2)
        XCTAssertEqual(Double(box.minY), 25, accuracy: 0.2)
        XCTAssertEqual(Double(box.width), 12, accuracy: 0.2)
        result = await call("set_cutout", id, ["id": name, "locked": true])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertTrue(result.text.contains("locked"))
        result = await call("set_cutout", id, ["id": name, "x": 40])
        XCTAssertTrue(result.isError, "locked")
        result = await call("set_cutout", id, ["id": name, "x": 40, "locked": false])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(Double(try XCTUnwrap(doc.cutouts.first).shape.bounds.minX), 40, accuracy: 0.2)
        result = await call("set_cutout", id, ["id": "cutout:zzzzzzzz", "x": 1])
        XCTAssertTrue(result.isError)
    }

    func testAPolygonTakesOnlyXYAndLocked() async throws {
        let (id, doc) = newWindow()
        _ = await call("add_cutout", id, ["shape": "polygon", "points": [[60, 10], [70, 10], [65, 18]]])
        let name = "cutout:\(String(try XCTUnwrap(doc.cutouts.first).id.uuidString.prefix(8)))"
        var result = await call("set_cutout", id, ["id": name, "width": 20])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("polygon"), result.text)
        result = await call("set_cutout", id, ["id": name, "x": 30, "y": 30])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(Double(try XCTUnwrap(doc.cutouts.first).shape.bounds.minX), 30, accuracy: 0.2)
    }

    func testGetBoardListsThemAndTheFreeFormMarker() async {
        let (id, _) = newWindow()
        _ = await call("add_cutout", id, ["x": 10, "y": 10, "width": 8, "height": 4])
        _ = await call("add_cutout", id, ["shape": "polygon", "points": [[60, 10], [70, 10], [65, 18]]])
        var result = await call("get_board", id)
        XCTAssertTrue(result.text.contains("CUTOUTS"), result.text)
        XCTAssertTrue(result.text.contains("rectangle"))
        XCTAssertTrue(result.text.contains("polygon"))
        XCTAssertFalse(result.text.contains("outline=free-form"))
        result = await call("set_board", id, ["outline": [[0, 0], [60, 0], [60, 15], [40, 15], [40, 40], [0, 40]]])
        XCTAssertFalse(result.isError, result.text)
        result = await call("get_board", id)
        XCTAssertTrue(result.text.contains("outline=free-form"), result.text)
        XCTAssertTrue(result.text.contains("size=60x40mm"), result.text)
    }

    func testSetBoardOutlineAndRectangle() async {
        let (id, doc) = newWindow()
        var result = await call("set_board", id, ["outline": [[0, 0], [60, 0], [60, 15], [40, 15], [40, 40], [0, 40]]])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertTrue(doc.board.isFreeForm)
        XCTAssertEqual(doc.board.width, 60)
        XCTAssertEqual(doc.board.height, 40)
        result = await call("set_board", id, ["outline": [[5, 0], [60, 0], [60, 40]]])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("must start at (0, 0)"), result.text)
        result = await call("set_board", id, ["rectangle": true])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertFalse(doc.board.isFreeForm)
        XCTAssertEqual(doc.board.width, 60)
        result = await call("set_board", id, [:])
        XCTAssertTrue(result.isError)
        XCTAssertTrue(result.text.contains("outline"), "the message lists the new keys")
    }

    func testDeleteAndUndoACutout() async throws {
        let (id, doc) = newWindow()
        _ = await call("add_cutout", id, ["x": 10, "y": 10, "width": 8, "height": 4])
        let name = "cutout:\(String(try XCTUnwrap(doc.cutouts.first).id.uuidString.prefix(8)))"
        var result = await call("delete", id, ["items": [name]])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertTrue(doc.cutouts.isEmpty)
        result = await call("undo", id, ["page": "pcb"])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(doc.cutouts.count, 1)
    }

    func testARunDRCOnAPadInACutoutSaysSo() async throws {
        let (id, doc) = newWindow()
        _ = await call("place_part", id, ["part": "GENERIC-LED", "x": 100, "y": 100])
        _ = await call("move_footprint", id, ["reference": "D1", "x": 30, "y": 40])
        let result = await call("add_cutout", id, ["x": 24, "y": 34, "width": 12, "height": 12])
        XCTAssertFalse(result.isError, result.text)
        let drc = await call("run_drc", id)
        XCTAssertTrue(drc.text.contains("Copper is in a cutout"), drc.text)
        XCTAssertEqual(doc.cutouts.count, 1)
    }
}
