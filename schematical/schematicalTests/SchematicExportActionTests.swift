import Foundation
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// plan-28-D: what Schematical > Export writes, without the save panel.
@MainActor
final class SchematicExportActionTests: XCTestCase {
    /// Kept for the life of the test process, like `TestSupport`'s documents:
    /// freeing a main-actor object in a test aborts the host.
    private static var kept: [SchematicFileController] = []

    private func controller() throws -> SchematicFileController {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let file = SchematicFileController(document: doc, catalog: TestSupport.catalog, willReplaceDocument: {})
        Self.kept.append(file)
        return file
    }

    private func temporaryURL(_ name: String) -> URL {
        FileManager.default.temporaryDirectory.appendingPathComponent("p28-\(UUID().uuidString)-\(name)")
    }

    func testEachFormatWritesItsFile() throws {
        let file = try controller()
        let unsavedBefore = file.hasUnsavedChanges
        let expected: [SchematicExportFormat: [UInt8]] = [
            .png: [0x89, 0x50, 0x4E, 0x47],
            .jpg: [0xFF, 0xD8],
        ]
        for format in SchematicExportFormat.allCases {
            let url = temporaryURL("export.\(format.fileExtension)")
            try file.writeExport(format, to: url)
            let data = try Data(contentsOf: url)
            if format == .svg {
                let text = String(decoding: data, as: UTF8.self)
                XCTAssertTrue(text.hasPrefix("<?xml"), "svg starts \(text.prefix(20))")
                XCTAssertTrue(text.contains("<svg"))
            } else {
                let magic = try XCTUnwrap(expected[format])
                XCTAssertEqual(Array(data.prefix(magic.count)), magic, "\(format) header")
            }
            try? FileManager.default.removeItem(at: url)
        }
        XCTAssertNil(file.fileURL, "an export is not a save")
        XCTAssertEqual(file.hasUnsavedChanges, unsavedBefore)
    }

    func testMenuFormats() {
        XCTAssertEqual(SchematicExportFormat.allCases.map(\.menuTitle), ["SVG Schematic", "PNG Schematic", "JPG Schematic"])
        XCTAssertEqual(SchematicExportFormat.allCases.map(\.fileExtension), ["svg", "png", "jpg"])
        XCTAssertEqual(SchematicExportFormat.allCases.map(\.contentType), [.svg, .png, .jpeg])
    }

    func testWritingIntoAMissingFolderThrows() throws {
        let file = try controller()
        let url = temporaryURL("missing").appendingPathComponent("export.svg")
        XCTAssertThrowsError(try file.writeExport(.svg, to: url))
    }

    func testEmptySchematicThrows() {
        let file = SchematicFileController(document: TestSupport.newDocument(), catalog: TestSupport.catalog,
                                           willReplaceDocument: {})
        Self.kept.append(file)
        XCTAssertThrowsError(try file.writeExport(.png, to: temporaryURL("empty.png")))
    }
}
