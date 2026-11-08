import XCTest
@testable import schematical

/// Parts with a converted datasheet carry its Markdown path, and the window's
/// payload holds what the renderer draws from the catalog.
@MainActor
final class MarkdownDatasheetTests: XCTestCase {
    func testConvertedPartHasMarkdownDatasheet() throws {
        let part = try TestSupport.part("SN74HC08N")
        XCTAssertEqual(part.datasheetMarkdown, "gate-and/sn74hc08.md")
        XCTAssertNil(try TestSupport.part("SN74HC00N").datasheetMarkdown)
    }

    func testEveryMarkdownDatasheetIsBundled() throws {
        for name in ["SN74HC08N", "SN74LS08N", "SN74LVC1G08DBVR", "CD4081BE"] {
            let path = try XCTUnwrap(try TestSupport.part(name).datasheetMarkdown, name)
            let base = ((path as NSString).lastPathComponent as NSString).deletingPathExtension
            XCTAssertNotNil(Bundle.main.url(forResource: base, withExtension: "md"), path)
        }
        XCTAssertNotNil(Bundle.main.url(forResource: "datasheet", withExtension: "html"))
        XCTAssertNotNil(Bundle.main.url(forResource: "load-cmos-50pf", withExtension: "svg"))
    }

    func testPayloadHasDevicesFootprintsAndFigures() throws {
        let built = try DatasheetPayload.build(path: "gate-and/sn74hc08.md", partName: "SN74HC08N",
                                               catalog: PartCatalogStore.shared)
        let object = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(built.json.utf8)) as? [String: Any])
        XCTAssertEqual(built.title, "SNx4HC08 quadruple 2-input AND gates")
        let devices = try XCTUnwrap(object["devices"] as? [[String: Any]])
        XCTAssertEqual(devices.map { $0["id"] as? String }, ["74_08"])
        let pinouts = try XCTUnwrap(devices[0]["pinouts"] as? [String: Any])
        XCTAssertNotNil(pinouts["lccc20"])
        let footprints = try XCTUnwrap(object["footprints"] as? [String: [String: Any]])
        let lccc = try XCTUnwrap(footprints["LCCC-20"])
        XCTAssertEqual((lccc["pads"] as? [Any])?.count, 20)
        XCTAssertEqual((footprints["PDIP-14"]?["pads"] as? [Any])?.count, 14)
        // DATASHEET_PAYLOAD_DUMP=1: keep the payload (in the app's temporary
        // folder) to render outside the app.
        if ProcessInfo.processInfo.environment["DATASHEET_PAYLOAD_DUMP"] != nil {
            let url = FileManager.default.temporaryDirectory.appendingPathComponent("datasheet-payload.json")
            try built.json.write(to: url, atomically: true, encoding: .utf8)
            print("payload written to \(url.path)")
        }
    }

    func testMissingFileFails() {
        XCTAssertThrowsError(try DatasheetPayload.build(path: "gate-and/nope.md", partName: "X",
                                                        catalog: PartCatalogStore.shared))
    }
}
