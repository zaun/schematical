import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Cutouts and a free-form outline in the file (plan-65-B): `board.cutouts`
/// and `board.outline`.
@MainActor
final class CutoutFileTests: XCTestCase {
    private let id1 = "11111111-1111-1111-1111-111111111111"
    private let id2 = "22222222-2222-2222-2222-222222222222"

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func error(_ board: String) -> String? {
        BoardLayersTests.pcbError { _ = try BoardLayersTests.open(board: board) }
    }

    private func json(_ doc: SchematicDocument) throws -> String {
        String(decoding: try SchematicFile(document: doc, fileURL: nil).encoded(), as: UTF8.self)
    }

    func testRoundTripOfCutoutsAndAFreeFormOutline() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardOutline([p(0, 0), p(0, 40), p(60, 40), p(60, 15), p(40, 15), p(40, 0)])
        let a = try XCTUnwrap(doc.addCutout(.rounded(rect: CGRect(x: 10.16, y: 8.255, width: 6.35, height: 2.54), radius: 1.27)))
        let b = try XCTUnwrap(doc.addCutout(.polygon([p(30, 20), p(34, 20), p(32, 24)])))
        doc.setLocked(.cutout(b), true)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.cutouts, doc.cutouts)
        XCTAssertEqual(opened.cutouts.map(\.id), [a, b])
        XCTAssertEqual(opened.cutouts[1].locked, true)
        XCTAssertEqual(opened.board.customOutline, doc.board.customOutline)
        XCTAssertEqual(opened.board.outline, doc.board.outline)
        XCTAssertEqual(opened.board.width, 60)
        XCTAssertEqual(opened.board.height, 40)
        XCTAssertTrue(opened.board.isFreeForm)
    }

    func testADocumentWithNeitherWritesNeitherKey() throws {
        let doc = TestSupport.newDocument()
        let text = try json(doc)
        XCTAssertFalse(text.contains("\"cutouts\""))
        XCTAssertFalse(text.contains("\"outline\""))
        // A free-form board that is reset again is a plain one.
        doc.setBoardOutline([p(0, 0), p(10, 0), p(10, 10), p(0, 10)])
        XCTAssertTrue(try json(doc).contains("\"outline\""))
        doc.resetBoardOutline()
        XCTAssertFalse(try json(doc).contains("\"outline\""))
    }

    func testTheOutlinesBoundsOverrideWidthAndHeight() throws {
        let doc = try BoardLayersTests.open(board: """
            { "width": 100, "height": 80,
              "outline": [ {"x":0,"y":0}, {"x":60,"y":0}, {"x":60,"y":40}, {"x":0,"y":40} ] }
            """)
        XCTAssertEqual(doc.board.width, 60)
        XCTAssertEqual(doc.board.height, 40)
        XCTAssertTrue(doc.board.isFreeForm)
    }

    func testACounterClockwiseOutlineAndPolygonAreStoredClockwise() throws {
        let doc = try BoardLayersTests.open(board: """
            { "outline": [ {"x":0,"y":0}, {"x":0,"y":40}, {"x":60,"y":40}, {"x":60,"y":0} ],
              "cutouts": [ { "id": "\(id1)", "shape": "polygon",
                             "points": [ {"x":10,"y":10}, {"x":10,"y":20}, {"x":20,"y":20} ] } ] }
            """)
        XCTAssertGreaterThan(BoardGeometry.area(doc.board.outline), 0)
        guard case let .polygon(points)? = doc.cutouts.first?.shape else { return XCTFail("not a polygon") }
        XCTAssertGreaterThan(BoardGeometry.area(points), 0)
    }

    func testEveryErrorMessage() {
        let square = "[ {\"x\":0,\"y\":0}, {\"x\":10,\"y\":0}, {\"x\":10,\"y\":10}, {\"x\":0,\"y\":10} ]"
        XCTAssertEqual(error("{ \"type\": \"customCard\", \"outline\": \(square) }"), "board outline needs the board type custom")
        XCTAssertEqual(error("{ \"outline\": [ {\"x\":0,\"y\":0}, {\"x\":5,\"y\":5} ] }"), "board outline needs at least 3 points")
        XCTAssertEqual(error("{ \"outline\": [ {\"x\":0,\"y\":0}, {\"x\":10,\"y\":10}, {\"x\":10,\"y\":0}, {\"x\":0,\"y\":10} ] }"),
                       "board outline crosses itself")
        XCTAssertEqual(error("{ \"outline\": [ {\"x\":5,\"y\":0}, {\"x\":15,\"y\":0}, {\"x\":15,\"y\":10}, {\"x\":5,\"y\":10} ] }"),
                       "board outline must start at (0, 0): its top-left is (5, 0)")
        XCTAssertEqual(error("{ \"outline\": [ {\"x\":0,\"y\":0}, {\"x\":1200,\"y\":0}, {\"x\":1200,\"y\":10}, {\"x\":0,\"y\":10} ] }"),
                       "board outline is 1200 mm wide; sizes are 1 to 1000 mm")
        XCTAssertEqual(error("{ \"cutouts\": [ { \"id\": \"\(id1)\", \"shape\": \"oval\" } ] }"),
                       "cutout \(id1) shape \"oval\" must be rounded or polygon")
        XCTAssertEqual(error("{ \"cutouts\": [ { \"id\": \"\(id1)\", \"shape\": \"rounded\", \"x\": 1, \"y\": 1, \"width\": 0.1, \"height\": 4 } ] }"),
                       "cutout \(id1) width 0.1 must be at least 0.5 mm")
        XCTAssertEqual(error("{ \"cutouts\": [ { \"id\": \"\(id1)\", \"shape\": \"rounded\", \"x\": 1, \"y\": 1, \"width\": 4, \"height\": 0.2 } ] }"),
                       "cutout \(id1) height 0.2 must be at least 0.5 mm")
        XCTAssertEqual(error("{ \"cutouts\": [ { \"id\": \"\(id1)\", \"shape\": \"rounded\", \"x\": 1, \"y\": 1, \"width\": 4, \"height\": 4, \"radius\": 5 } ] }"),
                       "cutout \(id1) radius 5 must be 0 to 2 mm")
        XCTAssertEqual(error("{ \"cutouts\": [ { \"id\": \"\(id1)\", \"shape\": \"rounded\", \"x\": 1, \"y\": 1, \"width\": 4 } ] }"),
                       "cutout \(id1) needs x, y, width and height")
        XCTAssertEqual(error("{ \"cutouts\": [ { \"id\": \"\(id1)\", \"shape\": \"polygon\" } ] }"), "cutout \(id1) needs points")
        XCTAssertEqual(error("{ \"cutouts\": [ { \"id\": \"\(id1)\", \"shape\": \"polygon\", \"points\": [ {\"x\":0,\"y\":0}, {\"x\":4,\"y\":0} ] } ] }"),
                       "cutout \(id1) needs at least 3 points")
        XCTAssertEqual(error("{ \"cutouts\": [ { \"id\": \"\(id1)\", \"shape\": \"polygon\", \"points\": [ {\"x\":0,\"y\":0}, {\"x\":4,\"y\":4}, {\"x\":4,\"y\":0}, {\"x\":0,\"y\":4} ] } ] }"),
                       "cutout \(id1) crosses itself")
    }

    func testTwoCutoutsWithOneIdAreRefused() {
        let entry = "{ \"id\": \"\(id1)\", \"shape\": \"rounded\", \"x\": 1, \"y\": 1, \"width\": 4, \"height\": 4 }"
        XCTAssertThrowsError(try BoardLayersTests.open(board: "{ \"cutouts\": [ \(entry), \(entry) ] }")) { thrown in
            guard case SchematicFileError.duplicateID = thrown else { return XCTFail("\(thrown)") }
        }
    }

    func testARoundedCutoutWithNoRadiusIsARectangle() throws {
        let doc = try BoardLayersTests.open(board: """
            { "cutouts": [ { "id": "\(id1)", "shape": "rounded", "x": 1, "y": 2, "width": 4, "height": 3 } ] }
            """)
        XCTAssertEqual(doc.cutouts.first?.shape, .rounded(rect: CGRect(x: 1, y: 2, width: 4, height: 3), radius: 0))
    }
}
