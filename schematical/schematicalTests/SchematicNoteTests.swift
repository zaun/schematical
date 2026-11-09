import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The schematic Text tool's text blocks: editing, selection, moving,
/// erasing, saving and exporting.
@MainActor
final class SchematicNoteTests: XCTestCase {
    private func newDocument() -> SchematicDocument { TestSupport.newDocument() }

    private func note(_ doc: SchematicDocument, _ id: UUID) throws -> SchematicNote {
        try XCTUnwrap(doc.notes.first { $0.id == id })
    }

    private func reopened(_ doc: SchematicDocument) throws -> SchematicDocument {
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let opened = newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        return opened
    }

    func testAddSnapsSelectsAndAsksToEdit() throws {
        let doc = newDocument()
        let id = doc.addNote(at: CGPoint(x: 104, y: 57))
        let added = try note(doc, id)
        XCTAssertEqual(added.origin, CGPoint(x: 100, y: 60))
        XCTAssertEqual(added.text, SchematicNote.placeholder)
        XCTAssertEqual(doc.selectedNoteID, id)
        XCTAssertEqual(doc.editingNoteID, id)
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .note(id))
    }

    func testWordsKeepLinesAndRefuseEmpty() throws {
        let doc = newDocument()
        let id = doc.addNote(at: .zero)
        let short = try note(doc, id).rect
        doc.setNoteText(id, "Clock divider\n\nfeeds U3")
        XCTAssertEqual(try note(doc, id).lines, ["Clock divider", "", "feeds U3"])
        let tall = try note(doc, id).rect
        XCTAssertGreaterThan(tall.height, short.height)
        XCTAssertGreaterThan(tall.width, short.width)
        // The frame lies on the grid.
        XCTAssertEqual(tall.width.truncatingRemainder(dividingBy: SchematicGrid.spacing), 0)
        XCTAssertEqual(tall.height.truncatingRemainder(dividingBy: SchematicGrid.spacing), 0)
        doc.setNoteText(id, "")
        XCTAssertEqual(try note(doc, id).text, "Clock divider\n\nfeeds U3")
    }

    func testSizeIsClamped() throws {
        let doc = newDocument()
        let id = doc.addNote(at: .zero)
        doc.setNoteSize(id, 1000)
        XCTAssertEqual(try note(doc, id).size, SchematicNote.sizeRange.upperBound)
        doc.setNoteSize(id, 0)
        XCTAssertEqual(try note(doc, id).size, SchematicNote.sizeRange.lowerBound)
    }

    func testHitBoxSelectAndGroupMove() throws {
        let doc = newDocument()
        let id = doc.addNote(at: CGPoint(x: 200, y: 200))
        let rect = try note(doc, id).rect
        XCTAssertEqual(doc.note(at: CGPoint(x: rect.midX, y: rect.midY))?.id, id)
        XCTAssertNil(doc.note(at: CGPoint(x: rect.maxX + 5, y: rect.midY)))

        doc.clearSchematicSelection()
        doc.boxSelectSchematic(CGRect(x: 190, y: 190, width: 20, height: 20), base: [])
        XCTAssertTrue(doc.isSelected(.note(id)))

        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 400))
        doc.toggleSelection(.chip(led.id))
        let base = doc.schematicGroupBase()
        doc.moveSchematicGroup(base, by: CGSize(width: 32, height: -18))
        XCTAssertEqual(try note(doc, id).origin, CGPoint(x: 230, y: 180))
        XCTAssertEqual(doc.chips.first { $0.id == led.id }?.origin,
                       CGPoint(x: led.origin.x + 30, y: led.origin.y - 20))
    }

    func testEraseAndDeleteDropTheSelection() throws {
        let doc = newDocument()
        let a = doc.addNote(at: .zero)
        let b = doc.addNote(at: CGPoint(x: 300, y: 0))
        doc.toggleSelection(.note(a))
        XCTAssertEqual(doc.schematicSelection.count, 2)
        let rect = try note(doc, a).rect
        doc.erase(at: CGPoint(x: rect.midX, y: rect.midY))
        XCTAssertEqual(doc.notes.map(\.id), [b])
        XCTAssertEqual(doc.schematicSelection.items, [.note(b)])
        doc.deleteNote(b)
        XCTAssertTrue(doc.notes.isEmpty)
        XCTAssertTrue(doc.schematicSelection.isEmpty)
    }

    func testRoundTrip() throws {
        let doc = newDocument()
        let id = doc.addNote(at: CGPoint(x: 40, y: 70))
        doc.setNoteText(id, "Line one\nLine two")
        doc.setNoteSize(id, 18)
        doc.setNoteFramed(id, false)
        let opened = try reopened(doc)
        XCTAssertEqual(opened.notes, doc.notes)
        XCTAssertTrue(opened.schematicSelection.isEmpty)
    }

    func testFilesWithoutNotesOpen() throws {
        let json = """
        { "format": "\(SchematicFile.formatName)", "version": 4, "nextReference": 1,
          "parts": [], "junctions": [], "wires": [] }
        """
        let opened = newDocument()
        try SchematicFile.decode(Data(json.utf8)).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertTrue(opened.notes.isEmpty)
    }

    func testBadNotesAreRefused() throws {
        func open(_ note: String) throws {
            let json = """
            { "format": "\(SchematicFile.formatName)", "version": 4, "nextReference": 1,
              "parts": [], "junctions": [], "wires": [], "notes": [\(note)] }
            """
            try SchematicFile.decode(Data(json.utf8)).apply(to: newDocument(), catalog: TestSupport.catalog,
                                                            fileURL: nil)
        }
        let id = UUID().uuidString
        XCTAssertThrowsError(try open(#"{ "id": "\#(id)", "text": "", "x": 0, "y": 0 }"#)) { error in
            guard case SchematicFileError.badNote = error else { return XCTFail("\(error)") }
        }
        XCTAssertThrowsError(try open(#"{ "id": "\#(id)", "text": "A", "x": 0, "y": 0, "size": 200 }"#)) { error in
            guard case SchematicFileError.badNote = error else { return XCTFail("\(error)") }
        }
        // Snapped on open, with defaults for missing keys.
        let opened = newDocument()
        let json = """
        { "format": "\(SchematicFile.formatName)", "version": 4, "nextReference": 1,
          "parts": [], "junctions": [], "wires": [],
          "notes": [ { "id": "\(id)", "text": "A", "x": 13, "y": 26 } ] }
        """
        try SchematicFile.decode(Data(json.utf8)).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        let placed = try XCTUnwrap(opened.notes.first)
        XCTAssertEqual(placed.origin, CGPoint(x: 10, y: 30))
        XCTAssertEqual(placed.size, SchematicNote.defaultSize)
        XCTAssertTrue(placed.framed)
    }

    func testExportDrawsEachLine() throws {
        let doc = newDocument()
        let id = doc.addNote(at: .zero)
        doc.setNoteText(id, "First line\nSecond & last")
        let scene = SchematicScene(doc)
        XCTAssertFalse(scene.isEmpty)
        let svg = String(decoding: try XCTUnwrap(SchematicExport.svg(scene)), as: UTF8.self)
        XCTAssertTrue(svg.contains(">First line</text>"))
        XCTAssertTrue(svg.contains(">\(SVGPainter.escaped("Second & last"))</text>"))
    }
}
