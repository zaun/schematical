import AppKit
import CoreGraphics
import Foundation
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// plan-61-B: a `.schematical` file read, written and edited through
/// `SchematicNSDocument`.
@MainActor
final class SchematicNSDocumentTests: XCTestCase {
    /// Documents made by tests, kept for the life of the test process (as
    /// `TestSupport.newDocument()` keeps its documents).
    private static var kept: [SchematicNSDocument] = []
    private static let type = SchematicFile.contentType.identifier

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func newDocument() -> SchematicNSDocument {
        let document = SchematicNSDocument()
        Self.kept.append(document)
        return document
    }

    /// The repository's `examples` folder.
    private static var examples: URL {
        URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent()
            .deletingLastPathComponent().appendingPathComponent("examples")
    }

    private static func exampleFiles() throws -> [URL] {
        let files = FileManager.default.enumerator(at: examples, includingPropertiesForKeys: nil)?
            .compactMap { $0 as? URL }.filter { $0.pathExtension == SchematicFile.fileExtension } ?? []
        return files.sorted { $0.path < $1.path }
    }

    /// A new folder in the temporary directory, removed after the test.
    private func temporaryFolder() throws -> URL {
        let folder = FileManager.default.temporaryDirectory
            .appendingPathComponent("SchematicNSDocumentTests-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        addTeardownBlock { try? FileManager.default.removeItem(at: folder) }
        return folder
    }

    /// One turn of the run loop: `NSDocument` takes in its undo manager's
    /// notifications (and so its Edited state) on the next turn, not at once.
    private func turnRunLoop() {
        RunLoop.main.run(until: Date().addingTimeInterval(0.05))
    }

    private func place(_ name: String, in document: SchematicNSDocument, at point: CGPoint) throws -> PlacedChip {
        let part = try XCTUnwrap(PartCatalogStore.shared.part(named: name), "no part \(name)")
        let doc = document.model.document
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return try XCTUnwrap(doc.chips.last)
    }

    /// A document with an EEPROM whose load file is `<folder>/A/rom.bin`.
    private func documentWithLoadFile(in folder: URL) throws -> (document: SchematicNSDocument, rom: URL) {
        let a = folder.appendingPathComponent("A")
        try FileManager.default.createDirectory(at: a, withIntermediateDirectories: true)
        let rom = a.appendingPathComponent("rom.bin")
        try Data([0xEA, 0xEA]).write(to: rom)
        let document = newDocument()
        let eeprom = try place("AT28C256-15PU", in: document, at: p(300, 300))
        document.model.document.setMemoryFile(eeprom.id, MemoryFile(url: rom, bookmark: nil))
        return (document, rom)
    }

    private func memoryPaths(at url: URL) throws -> [String] {
        try SchematicFile.decode(Data(contentsOf: url)).parts.compactMap { $0.memoryFile?.path }
    }

    // MARK: Phase 2 — read and write

    /// F3: what the document writes for each example is what the app wrote
    /// before it had a document class.
    func testEachExampleReadsAndWritesAsBefore() throws {
        let files = try Self.exampleFiles()
        XCTAssertEqual(files.count, 10, "the examples: \(files.map(\.lastPathComponent))")
        for url in files {
            let document = newDocument()
            try document.read(from: url, ofType: Self.type)
            document.fileURL = url
            let written = try SchematicFile.decode(document.data(ofType: Self.type))

            let reference = TestSupport.newDocument(library: PartCatalogStore.shared.library)
            _ = try SchematicFile.decode(Data(contentsOf: url))
                .apply(to: reference, catalog: PartCatalogStore.shared, fileURL: url)
            XCTAssertEqual(written, SchematicFile(document: reference, fileURL: url), url.lastPathComponent)
            XCTAssertFalse(document.model.document.undoManager.canUndo, "an opened file has no undo history")
            XCTAssertFalse(document.isDocumentEdited, url.lastPathComponent)
        }
    }

    /// F2: load-file paths are written relative to where the write ends up,
    /// and absolute for a document with no place of its own yet.
    func testLoadFilePathsFollowTheDestination() throws {
        let folder = try temporaryFolder()
        let made = try documentWithLoadFile(in: folder)
        let b = folder.appendingPathComponent("B")
        try FileManager.default.createDirectory(at: b, withIntermediateDirectories: true)
        let design = b.appendingPathComponent("design.schematical")
        try made.document.writeSafely(to: design, ofType: Self.type, for: .saveAsOperation)
        XCTAssertEqual(try memoryPaths(at: design), ["../A/rom.bin"])

        // Beside the load file's folder: one level down.
        let beside = folder.appendingPathComponent("beside.schematical")
        try made.document.writeSafely(to: beside, ofType: Self.type, for: .saveToOperation)
        XCTAssertEqual(try memoryPaths(at: beside), ["A/rom.bin"])

        let autosaves = folder.appendingPathComponent("Autosave Information")
        try FileManager.default.createDirectory(at: autosaves, withIntermediateDirectories: true)
        let autosaved = autosaves.appendingPathComponent("Unsaved Schematical Document.schematical")
        try made.document.writeSafely(to: autosaved, ofType: Self.type, for: .autosaveElsewhereOperation)
        XCTAssertEqual(try memoryPaths(at: autosaved), [made.rom.standardizedFileURL.path])
    }

    /// A file in the autosave folder is read with its absolute load-file
    /// path; a file with a place resolves a relative one against its folder.
    func testReadingResolvesLoadFilePaths() throws {
        let folder = try temporaryFolder()
        let made = try documentWithLoadFile(in: folder)
        let rom = made.rom.resolvingSymlinksInPath().standardizedFileURL.path

        let autosaves = folder.appendingPathComponent("Autosave Information")
        try FileManager.default.createDirectory(at: autosaves, withIntermediateDirectories: true)
        let autosaved = autosaves.appendingPathComponent("Unsaved.schematical")
        try made.document.writeSafely(to: autosaved, ofType: Self.type, for: .autosaveElsewhereOperation)
        let fromAutosave = newDocument()
        try fromAutosave.read(from: autosaved, ofType: Self.type)
        XCTAssertEqual(fromAutosave.model.document.chips.first?.memoryFile?.url.resolvingSymlinksInPath().path, rom)

        let b = folder.appendingPathComponent("B")
        try FileManager.default.createDirectory(at: b, withIntermediateDirectories: true)
        let design = b.appendingPathComponent("design.schematical")
        try made.document.writeSafely(to: design, ofType: Self.type, for: .saveAsOperation)
        let fromFile = newDocument()
        try fromFile.read(from: design, ofType: Self.type)
        XCTAssertEqual(fromFile.model.document.chips.first?.memoryFile?.url.resolvingSymlinksInPath().path, rom)
    }

    /// The document's undo manager is the schematic's: an edit marks the
    /// document edited, undo clears the mark, and a write ends a typing step.
    func testEditsUndoAndWritesGoThroughTheDocumentsUndoManager() throws {
        let document = newDocument()
        let doc = document.model.document
        XCTAssertTrue(document.undoManager === doc.undoManager)
        XCTAssertFalse(document.isDocumentEdited)

        let label = doc.addNetLabel(at: p(100, 100))
        turnRunLoop()
        XCTAssertTrue(document.isDocumentEdited, "an edit marks the document")
        XCTAssertTrue(doc.undo())
        turnRunLoop()
        XCTAssertFalse(document.isDocumentEdited, "undo back to the start clears the mark")
        XCTAssertTrue(doc.redo())
        turnRunLoop()
        XCTAssertTrue(document.isDocumentEdited)

        XCTAssertTrue(doc.setNetLabelName(label, "C"))
        _ = try document.data(ofType: Self.type)
        XCTAssertTrue(doc.setNetLabelName(label, "CL"))
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.netLabels.first?.name, "C", "the character typed after the write is its own step")
        XCTAssertTrue(doc.undo())
        XCTAssertNotEqual(doc.netLabels.first?.name, "C")
        XCTAssertEqual(doc.undoName, "Add Net Label")
    }

    /// Reading into a document again (Revert) stops its simulation and ends
    /// its undo history.
    func testReadingAgainStopsTheSimulationAndClearsUndo() throws {
        let folder = try temporaryFolder()
        let document = newDocument()
        _ = try place("SN74HC00N", in: document, at: p(200, 200))
        let saved = folder.appendingPathComponent("saved.schematical")
        try document.writeSafely(to: saved, ofType: Self.type, for: .saveAsOperation)

        _ = document.model.document.addNetLabel(at: p(100, 100))
        document.model.simulation.start(document: document.model.document)
        XCTAssertTrue(document.model.simulation.isRunning, document.model.simulation.errorMessage ?? "")
        try document.read(from: saved, ofType: Self.type)
        XCTAssertFalse(document.model.simulation.isRunning)
        XCTAssertTrue(document.model.document.netLabels.isEmpty)
        XCTAssertEqual(document.model.document.chips.count, 1)
        XCTAssertFalse(document.model.document.undoManager.canUndo)
    }
}
