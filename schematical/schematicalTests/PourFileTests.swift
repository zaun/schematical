import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Pours and keep-outs in the file (plan-52-B): `board.pours` and
/// `board.keepouts`.
@MainActor
final class PourFileTests: XCTestCase {
    private let id1 = "11111111-1111-1111-1111-111111111111"
    private let id2 = "22222222-2222-2222-2222-222222222222"
    private let id3 = "33333333-3333-3333-3333-333333333333"

    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect {
        CGRect(x: x, y: y, width: w, height: h)
    }

    private func error(layers: Int = 4, pours: String = "[]", keepouts: String = "[]", traces: String = "[]") -> String? {
        BoardLayersTests.pcbError {
            _ = try BoardLayersTests.open(board: """
                { "layers": \(layers), "traces": \(traces), "pours": \(pours), "keepouts": \(keepouts),
                  "layerStyles": \(layers == 4 ? "[ { \"layer\": \"inner2\", \"type\": \"wire\" } ]" : "[]") }
                """)
        }
    }

    private func pour(_ extra: String) -> String {
        "[ { \"id\": \"\(id1)\", \"x\": 10, \"y\": 5, \"width\": 40, \"height\": 30, \"layer\": \"top\"\(extra) } ]"
    }

    func testRoundTripOfEverySetting() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        doc.setLayerType(.inner(2), .wire)
        let a = try XCTUnwrap(doc.addPour(rect: r(9.525, 5.08, 40.005, 30.48), layer: .top))
        doc.setPourNet(a, .rail(mv: 5000))
        doc.setPourClearance(a, 0.3)
        doc.setPourConnection(a, .solid)
        doc.setPourThermalGap(a, 0.6)
        doc.setPourSpokeWidth(a, 0.4)
        doc.setPourMinWidth(a, 0.25)
        doc.setLocked(.pour(a), true)
        let b = try XCTUnwrap(doc.addPour(rect: r(1, 1, 5, 5), layer: .inner(2)))
        doc.setPourNet(b, nil)
        doc.setPourClearance(b, nil)
        doc.setPourConnection(b, .thermal)
        doc.setPourThermalGap(b, 0.5)
        doc.setPourSpokeWidth(b, 0.5)
        doc.setPourMinWidth(b, 0.2)
        let k1 = doc.addKeepout(rect: r(20, 10, 8, 6), layers: nil)
        let k2 = doc.addKeepout(rect: r(30, 10, 8, 6), layers: [.top, .bottom])
        let k3 = doc.addKeepout(rect: r(40, 10, 8, 6), layers: [])
        doc.setLocked(.keepout(k2), true)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.pours, doc.pours)
        XCTAssertEqual(opened.keepouts, doc.keepouts)
        XCTAssertEqual(opened.pours.count, 2)
        XCTAssertEqual(opened.keepouts.map(\.id), [k1, k2, k3])
        XCTAssertNil(opened.keepouts[0].layers)
        XCTAssertEqual(opened.keepouts[2].layers, [])
    }

    func testDocumentWithNoneSavesNeitherKey() throws {
        let doc = TestSupport.newDocument()
        let text = try String(decoding: SchematicFile(document: doc, fileURL: nil).encoded(), as: UTF8.self)
        XCTAssertFalse(text.contains("\"pours\""))
        XCTAssertFalse(text.contains("\"keepouts\""))
        // Added then removed: the bytes are those of a document that never had one.
        let before = try SchematicFile(document: doc, fileURL: nil).encoded()
        let id = try XCTUnwrap(doc.addPour(rect: r(0, 0, 5, 5), layer: .top))
        let keepout = doc.addKeepout(rect: r(0, 0, 5, 5))
        XCTAssertTrue(String(decoding: try SchematicFile(document: doc, fileURL: nil).encoded(), as: UTF8.self).contains("\"pours\""))
        doc.deletePour(id)
        doc.deleteKeepout(keepout)
        XCTAssertEqual(try SchematicFile(document: doc, fileURL: nil).encoded(), before)
    }

    func testMissingFieldsTakeTheirDefaults() throws {
        let doc = try BoardLayersTests.open(board: "{ \"layers\": 2, \"pours\": \(pour("")) }")
        let pour = try XCTUnwrap(doc.pours.first)
        XCTAssertEqual(pour.rect, r(10, 5, 40, 30))
        XCTAssertNil(pour.net)
        XCTAssertNil(pour.clearance)
        XCTAssertEqual(pour.connection, .thermal)
        XCTAssertEqual(pour.thermalGap, 0.5)
        XCTAssertEqual(pour.spokeWidth, 0.5)
        XCTAssertEqual(pour.minWidth, 0.2)
        XCTAssertFalse(pour.locked)
    }

    func testPourErrors() {
        XCTAssertEqual(error(pours: pour(", \"net\": \"vcc\"")),
                       "pour \(id1) net \"vcc\" must be ground, power:<mV> or pin:<part id>/<pin>")
        XCTAssertEqual(error(pours: "[ { \"id\": \"\(id1)\", \"x\": 0, \"y\": 0, \"width\": 5, \"height\": 5, \"layer\": \"middle\" } ]"),
                       "pour \(id1) layer \"middle\" must be top, inner2 to inner5, or bottom")
        XCTAssertEqual(error(layers: 2, pours: "[ { \"id\": \"\(id1)\", \"x\": 0, \"y\": 0, \"width\": 5, \"height\": 5, \"layer\": \"inner4\" } ]"),
                       "pour \(id1) layer \"inner4\" is not on a 2-layer board")
        XCTAssertEqual(error(pours: "[ { \"id\": \"\(id1)\", \"x\": 0, \"y\": 0, \"width\": 5, \"height\": 5, \"layer\": \"inner3\" } ]"),
                       "pour \(id1) layer inner3 is a plane")
        XCTAssertEqual(error(pours: "[ { \"id\": \"\(id1)\", \"x\": 0, \"y\": 0, \"width\": 0.1, \"height\": 5, \"layer\": \"top\" } ]"),
                       "pour \(id1) width 0.1 must be at least 0.5 mm")
        XCTAssertEqual(error(pours: "[ { \"id\": \"\(id1)\", \"x\": 0, \"y\": 0, \"width\": 5, \"height\": 0.1, \"layer\": \"top\" } ]"),
                       "pour \(id1) height 0.1 must be at least 0.5 mm")
        XCTAssertEqual(error(pours: pour(", \"clearance\": 20")), "pour \(id1) clearance 20 must be 0.05 to 10 mm")
        XCTAssertEqual(error(pours: pour(", \"connection\": \"x\"")), "pour \(id1) connection \"x\" must be thermal or solid")
        XCTAssertEqual(error(pours: pour(", \"thermalGap\": 9")), "pour \(id1) thermalGap 9 must be 0.1 to 5 mm")
        XCTAssertEqual(error(pours: pour(", \"spokeWidth\": 9")), "pour \(id1) spokeWidth 9 must be 0.1 to 5 mm")
        XCTAssertEqual(error(pours: pour(", \"minWidth\": 9")), "pour \(id1) minWidth 9 must be 0.05 to 2 mm")
    }

    func testKeepoutErrors() {
        XCTAssertEqual(error(keepouts: "[ { \"id\": \"\(id2)\", \"x\": 0, \"y\": 0, \"width\": 5, \"height\": 5, \"layers\": [\"middle\"] } ]"),
                       "keepout \(id2) layer \"middle\" must be top, inner2 to inner5, or bottom")
        XCTAssertEqual(error(keepouts: "[ { \"id\": \"\(id2)\", \"x\": 0, \"y\": 0, \"width\": 0.1, \"height\": 5 } ]"),
                       "keepout \(id2) width 0.1 must be at least 0.5 mm")
    }

    func testAKeepoutLayerTheBoardLacksIsKept() throws {
        let doc = try BoardLayersTests.open(board: """
            { "layers": 2, "keepouts": [ { "id": "\(id2)", "x": 0, "y": 0, "width": 5, "height": 5, "layers": ["inner4", "top"] } ] }
            """)
        XCTAssertEqual(doc.keepouts.first?.layers, [.inner(4), .top])
    }

    func testAnIDSharedWithATraceFails() {
        let trace = "[ { \"id\": \"\(id1)\", \"layer\": \"top\", \"points\": [ { \"x\": 1, \"y\": 1 }, { \"x\": 5, \"y\": 1 } ] } ]"
        do {
            _ = try BoardLayersTests.open(board: "{ \"layers\": 2, \"traces\": \(trace), \"pours\": \(pour("")) }")
            XCTFail("no error")
        } catch SchematicFileError.duplicateID(let id) {
            XCTAssertEqual(id.uuidString.lowercased(), id1)
        } catch {
            XCTFail("unexpected \(error)")
        }
    }

    func testAFailedOpenLeavesTheDocumentAlone() throws {
        let doc = TestSupport.newDocument()
        _ = doc.addPour(rect: r(0, 0, 5, 5), layer: .top)
        let kept = doc.pours
        let json = """
            { "format": "\(SchematicFile.formatName)", "version": 3, "nextReference": 1, "parts": [], "junctions": [], "wires": [],
              "board": { "layers": 2, "pours": \(pour(", \"clearance\": 20")) } }
            """
        XCTAssertThrowsError(try SchematicFile.decode(Data(json.utf8)).apply(to: doc, catalog: TestSupport.catalog, fileURL: nil))
        XCTAssertEqual(doc.pours, kept)
    }
}
