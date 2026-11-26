import XCTest
@testable import schematical

/// Parts with a converted datasheet carry its Markdown path, and the window's
/// payload holds what the renderer draws from the catalog.
@MainActor
final class MarkdownDatasheetTests: XCTestCase {
    func testConvertedPartHasMarkdownDatasheet() throws {
        let part = try TestSupport.part("SN74HC08N")
        XCTAssertEqual(part.datasheetMarkdown, "gate-and/sn74hc08.md")
        // The NAND's was converted too (it was this test's example of a part without one).
        XCTAssertEqual(try TestSupport.part("SN74HC00N").datasheetMarkdown, "gate-nand/sn74hc00.md")
        // A part with no datasheet has no Markdown.
        XCTAssertNil(try TestSupport.part("GENERIC-LED").datasheetMarkdown)
    }

    func testEveryMarkdownDatasheetIsBundled() throws {
        for name in ["SN74HC08N", "SN74LS08N", "SN74LVC1G08DBVR", "CD4081BE"] {
            let path = try XCTUnwrap(try TestSupport.part(name).datasheetMarkdown, name)
            let base = ((path as NSString).lastPathComponent as NSString).deletingPathExtension
            XCTAssertNotNil(Bundle.main.url(forResource: base, withExtension: "md"), path)
        }
        XCTAssertNotNil(Bundle.main.url(forResource: "datasheet", withExtension: "html"))
        XCTAssertNotNil(Bundle.main.url(forResource: "datasheet-edit", withExtension: "js"))
        XCTAssertNotNil(Bundle.main.url(forResource: "datasheet-edit-model", withExtension: "js"))
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

    /// The Verify Datasheet window shows an edited copy in place of the file and keeps the file as
    /// `original`, so the editor can tell what changed.
    func testEditedMarkdownIsShownWithTheFileAsOriginal() throws {
        func object(_ json: String) throws -> [String: Any] {
            try XCTUnwrap(JSONSerialization.jsonObject(with: Data(json.utf8)) as? [String: Any])
        }
        let plain = try object(DatasheetPayload.build(path: "gate-and/sn74hc08.md", partName: "SN74HC08N",
                                                      catalog: PartCatalogStore.shared).json)
        XCTAssertNil(plain["original"])
        XCTAssertEqual(plain["editing"] as? Bool, false)
        XCTAssertEqual(plain["path"] as? String, "gate-and/sn74hc08.md")
        let file = try XCTUnwrap(plain["markdown"] as? String)

        let edited = file + "\nAn added paragraph.\n"
        let shown = try object(DatasheetPayload.build(path: "gate-and/sn74hc08.md", partName: "SN74HC08N",
                                                      catalog: PartCatalogStore.shared, citations: true,
                                                      markdown: edited, editing: true).json)
        XCTAssertEqual(shown["markdown"] as? String, edited)
        XCTAssertEqual(shown["original"] as? String, file)
        XCTAssertEqual(shown["editing"] as? Bool, true)
        XCTAssertEqual(shown["showCitations"] as? Bool, true)
    }

    func testMissingFileFails() {
        XCTAssertThrowsError(try DatasheetPayload.build(path: "gate-and/nope.md", partName: "X",
                                                        catalog: PartCatalogStore.shared))
    }
}
