import AppKit
import Foundation
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// File > Export > Image…: what each choice in the options sheet writes,
/// without the sheet or the save panel.
@MainActor
final class SchematicExportActionTests: XCTestCase {
    /// A system document, with one part unless `empty`; its exporter is
    /// what File › Export uses.
    private func system(empty: Bool = false) throws -> SchematicNSDocument {
        let doc = TestSupport.newDocument()
        if !empty { try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200)) }
        return TestSupport.systemDocument(of: doc)
    }

    private func controller(empty: Bool = false) throws -> (file: SchematicFileController, document: SchematicDocument) {
        let system = try system(empty: empty)
        return (system.model.file, system.model.document)
    }

    private func temporaryURL(_ name: String) -> URL {
        FileManager.default.temporaryDirectory.appendingPathComponent("img-\(UUID().uuidString)-\(name)")
    }

    private func bitmap(_ data: Data) throws -> NSBitmapImageRep {
        try XCTUnwrap(NSBitmapImageRep(data: data))
    }

    func testEveryViewAndFormatWritesItsFile() throws {
        let system = try system()
        let file = system.model.file, document = system.model.document
        TestSupport.turnRunLoop()
        let editedBefore = system.isDocumentEdited
        let undoBefore = document.undoName
        let magic: [ImageExportFormat: [UInt8]] = [.png: [0x89, 0x50, 0x4E, 0x47], .jpg: [0xFF, 0xD8]]
        for view in ImageExportView.allCases {
            for format in view.formats {
                var options = ImageExportOptions()
                options.view = view
                options.format = format
                options.scale = 1
                let url = temporaryURL("export.\(format.fileExtension)")
                try file.writeImage(options, to: url)
                let data = try Data(contentsOf: url)
                if format == .svg {
                    let text = String(decoding: data, as: UTF8.self)
                    XCTAssertTrue(text.hasPrefix("<?xml") && text.contains("<svg"), "\(view) svg")
                } else {
                    XCTAssertEqual(Array(data.prefix(magic[format]!.count)), magic[format], "\(view) \(format) header")
                    let rep = try bitmap(data)
                    let size = try XCTUnwrap(ImageExport.pixelSize(of: document, options))
                    XCTAssertEqual(CGFloat(rep.pixelsWide), size.width, "\(view) \(format) width")
                    XCTAssertEqual(CGFloat(rep.pixelsHigh), size.height, "\(view) \(format) height")
                }
                try? FileManager.default.removeItem(at: url)
            }
        }
        XCTAssertNil(system.fileURL, "an export is not a save")
        XCTAssertNil(file.fileURL)
        TestSupport.turnRunLoop()
        XCTAssertEqual(system.isDocumentEdited, editedBefore)
        XCTAssertEqual(document.undoName, undoBefore, "and not an edit")
    }

    func testScaleSetsThePixelSize() throws {
        let (_, document) = try controller()
        for view in ImageExportView.allCases {
            var options = ImageExportOptions()
            options.view = view
            options.scale = 1
            let one = try XCTUnwrap(ImageExport.pixelSize(of: document, options))
            options.scale = 2
            let two = try XCTUnwrap(ImageExport.pixelSize(of: document, options))
            XCTAssertEqual(two.width, one.width * 2, accuracy: 1, "\(view)")
            XCTAssertEqual(two.height, one.height * 2, accuracy: 1, "\(view)")
            options.scale = 8
            let big = try XCTUnwrap(ImageExport.pixelSize(of: document, options))
            XCTAssertLessThanOrEqual(max(big.width, big.height), SchematicExport.maxPixels + 1, "\(view) capped")
        }
    }

    func testBackgrounds() throws {
        let (_, document) = try controller()
        func corner(_ options: ImageExportOptions) throws -> NSColor {
            let rep = try bitmap(try XCTUnwrap(ImageExport.data(for: document, options)))
            return try XCTUnwrap(rep.colorAt(x: 0, y: 0)?.usingColorSpace(.sRGB))
        }
        var options = ImageExportOptions()
        options.scale = 1
        for view in ImageExportView.allCases {
            options.view = view
            options.background = .transparent
            XCTAssertEqual(try corner(options).alphaComponent, 0, accuracy: 0.01, "\(view) transparent")
            options.background = .white
            let white = try corner(options)
            XCTAssertEqual(white.alphaComponent, 1, accuracy: 0.01, "\(view) white")
            XCTAssertEqual(white.redComponent, 1, accuracy: 0.01, "\(view) white")
        }
        options.view = .pcb
        options.background = .canvas
        XCTAssertLessThan(try corner(options).redComponent, 0.3, "the PCB page's dark canvas")
    }

    func testNormalizedOptions() {
        var options = ImageExportOptions()
        options.view = .pcb
        options.format = .svg
        XCTAssertEqual(options.normalized().format, .png, "no SVG PCB")
        options.format = .jpg
        options.background = .transparent
        XCTAssertEqual(options.normalized().background, .canvas, "a JPG is never transparent")
        options.view = .schematic
        options.format = .png
        options.background = .canvas
        XCTAssertEqual(options.normalized().background, .transparent, "the schematic has no canvas background")
        options.scale = 3
        XCTAssertEqual(options.normalized().scale, 2)
    }

    func testOptionsAreRemembered() throws {
        let defaults = try XCTUnwrap(UserDefaults(suiteName: "img-\(UUID().uuidString)"))
        XCTAssertEqual(ImageExportOptions.saved(defaults), ImageExportOptions())
        var options = ImageExportOptions()
        options.view = .back
        options.format = .jpg
        options.scale = 4
        options.background = .white
        options.angle = .top
        options.save(defaults)
        XCTAssertEqual(ImageExportOptions.saved(defaults), options)
    }

    func testWritingIntoAMissingFolderThrows() throws {
        let (file, _) = try controller()
        let url = temporaryURL("missing").appendingPathComponent("export.png")
        XCTAssertThrowsError(try file.writeImage(ImageExportOptions(), to: url))
    }

    func testEmptySchematicThrowsButItsBoardExports() throws {
        let (file, document) = try controller(empty: true)
        XCTAssertThrowsError(try file.writeImage(ImageExportOptions(), to: temporaryURL("empty.png")))
        var options = ImageExportOptions()
        options.view = .pcb
        options.scale = 1
        XCTAssertNotNil(ImageExport.data(for: document, options), "a board with nothing on it still draws")
    }

    func testMenu() {
        XCTAssertEqual(ImageExportView.allCases.map(\.title), ["Schematic", "PCB", "Front Render", "Back Render"])
        XCTAssertEqual(ImageExportFormat.allCases.map(\.contentType), [.png, .jpeg, .svg])
    }
}
