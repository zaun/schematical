import AppKit
import CoreGraphics
import Foundation
import SwiftUI
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

    /// These tests close windows; the app would quit after the last one.
    override func setUp() {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

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
        // The document is never given the example's URL: nothing here may
        // lead AppKit to write into the repository.
        let output = try temporaryFolder()
        for url in files {
            let document = newDocument()
            try document.read(from: url, ofType: Self.type)
            XCTAssertNil(document.fileURL)
            // Written beside the example (in name only: `data(ofType:)` is
            // given the destination by `writeSafely`), so load-file paths
            // are relative to the example's folder, as the reference's are.
            let copy = output.appendingPathComponent(url.lastPathComponent)
            try document.writeSafely(to: copy, ofType: Self.type, for: .saveToOperation)
            let written = try SchematicFile.decode(Data(contentsOf: copy))

            let reference = TestSupport.newDocument(library: PartCatalogStore.shared.library)
            _ = try SchematicFile.decode(Data(contentsOf: url))
                .apply(to: reference, catalog: PartCatalogStore.shared, fileURL: url)
            XCTAssertEqual(written, SchematicFile(document: reference, fileURL: copy), url.lastPathComponent)
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

    // MARK: Phase 4 — child windows

    /// A document listed among the app's open documents until the test ends.
    private func openDocument() -> SchematicNSDocument {
        TestSupport.openSystemDocument(of: TestSupport.newDocument(library: TestSupport.catalog.library),
                                       catalog: TestSupport.catalog, until: self)
    }

    /// Each document has its own Bill of Materials window, and closing a
    /// document closes only its own.
    func testChildWindowsBelongToTheirDocument() throws {
        let center = ChildWindowCenter.shared
        let a = openDocument(), b = openDocument()
        let idA = a.model.id, idB = b.model.id
        XCTAssertNotNil(SchematicEditorModel.session(idA))
        ChildWindowOpener(documentID: idA)(id: "bom", value: BillOfMaterialsTarget(documentID: idA))
        ChildWindowOpener(documentID: idB)(id: "bom", value: BillOfMaterialsTarget(documentID: idB))
        let windowA = try XCTUnwrap(center.windows(of: idA).first)
        let windowB = try XCTUnwrap(center.windows(of: idB).first)
        XCTAssertEqual(center.windows(of: idA).count, 1)
        XCTAssertEqual(center.windows(of: idB).count, 1)
        XCTAssertFalse(windowA === windowB)
        XCTAssertTrue(windowA.isVisible && windowB.isVisible)
        XCTAssertEqual(windowA.title, "Bill of Materials")
        XCTAssertFalse(windowA.isRestorable)

        // Asked again, the same window comes forward; a second target is a second window.
        ChildWindowOpener(documentID: idA)(id: "bom", value: BillOfMaterialsTarget(documentID: idA))
        XCTAssertEqual(center.windows(of: idA).count, 1)
        ChildWindowOpener(documentID: idA)(id: "drc", value: DesignRuleCheckTarget(documentID: idA))
        XCTAssertEqual(center.windows(of: idA).count, 2)
        let both = center.windows(of: idA)

        a.close()
        XCTAssertTrue(center.windows(of: idA).isEmpty)
        XCTAssertTrue(both.allSatisfy { !$0.isVisible })
        XCTAssertNil(SchematicEditorModel.session(idA), "a closed document is no longer found")
        XCTAssertEqual(center.windows(of: idB).count, 1)
        XCTAssertTrue(windowB.isVisible, "the other document's window stays")

        // The user closing a child window forgets it; opening it again makes a new one.
        windowB.close()
        XCTAssertTrue(center.windows(of: idB).isEmpty)
        ChildWindowOpener(documentID: idB)(id: "bom", value: BillOfMaterialsTarget(documentID: idB))
        XCTAssertEqual(center.windows(of: idB).count, 1)
        ChildWindowCloser(documentID: idB)(id: "bom", value: BillOfMaterialsTarget(documentID: idB))
        XCTAssertTrue(center.windows(of: idB).isEmpty)
        b.close()
    }

    /// The editor is hosted in its document's window, not in a scene. Its
    /// views still open the app's scene windows (Datasheet, Part Editor,
    /// Help) with SwiftUI's `openWindow`.
    func testAHostedViewOpensTheAppsSceneWindows() throws {
        final class Box { var open: OpenWindowAction? }
        struct Probe: View {
            @Environment(\.openWindow) private var openWindow
            let box: Box
            var body: some View {
                Color.clear.frame(width: 50, height: 50).onAppear { box.open = openWindow }
            }
        }
        let box = Box()
        let window = NSWindow(contentViewController: NSHostingController(rootView: Probe(box: box)))
        window.isReleasedWhenClosed = false
        window.makeKeyAndOrderFront(nil)
        defer { window.close() }
        RunLoop.main.run(until: Date().addingTimeInterval(0.3))
        let open = try XCTUnwrap(box.open, "the hosted view appeared")
        func helpWindows() -> [NSWindow] { NSApp.windows.filter { $0.isVisible && $0.title == "PCB Help" } }
        XCTAssertTrue(helpWindows().isEmpty)
        open(id: "help", value: EditorPage.pcb)
        RunLoop.main.run(until: Date().addingTimeInterval(1.0))
        let opened = helpWindows()
        XCTAssertEqual(opened.count, 1, "windows: \(NSApp.windows.filter(\.isVisible).map(\.title))")
        opened.forEach { $0.close() }
    }

    // MARK: Phase 5 — a closed document is freed

    private func spin(_ seconds: Double) {
        let end = Date().addingTimeInterval(seconds)
        while Date() < end {
            autoreleasepool { RunLoop.main.run(until: Date().addingTimeInterval(0.1)) }
        }
    }

    /// Waits up to eight seconds for the app that hosts the tests to be the
    /// active app. It cannot take the front by itself while another app
    /// holds it (`NSApp.activate` is then refused): run from Xcode it is in
    /// front already; run from a shell, `open -a <the Debug app>` from that
    /// shell brings it forward while this waits.
    private func waitForTheAppToBeActive() {
        let end = Date().addingTimeInterval(8)
        while !NSApp.isActive, Date() < end {
            NSApp.activate(ignoringOtherApps: true)
            spin(0.25)
        }
    }

    /// Makes a document with its window on screen, its simulation running
    /// and a Memory window open, closes it, and gives the run loop two
    /// seconds. Returns weak references to what the document edited, and
    /// whether its window was key when it closed.
    private func closeADocument(key: Bool) throws
        -> (alive: [String], wasKey: Bool) {
        weak var model: SchematicEditorModel?
        weak var schematic: SchematicDocument?
        weak var simulation: SimulationController?
        weak var exporter: SchematicFileController?
        weak var system: SchematicNSDocument?
        weak var window: NSWindow?
        var wasKey = false
        try autoreleasepool {
            let document = SchematicNSDocument()
            NSDocumentController.shared.addDocument(document)
            model = document.model
            schematic = document.model.document
            simulation = document.model.simulation
            exporter = document.model.file
            system = document
            _ = try place("SN74HC00N", in: document, at: p(200, 200))
            let eeprom = try place("AT28C256-15PU", in: document, at: p(500, 300))
            document.makeWindowControllers()
            document.showWindows()
            window = document.windowControllers.first?.window
            XCTAssertEqual(window?.isVisible, true)
            if key { waitForTheAppToBeActive() }
            spin(0.5)
            // The editor's key monitors are installed: they are what kept a
            // closed document alive.
            XCTAssertGreaterThanOrEqual(document.model.eventMonitors.count, 2)

            document.model.simulation.start(document: document.model.document)
            XCTAssertTrue(document.model.simulation.isRunning, document.model.simulation.errorMessage ?? "")
            let id = document.model.id
            ChildWindowOpener(documentID: id)(id: "memory", value: MemoryWindowTarget(documentID: id, chipID: eeprom.id))
            XCTAssertEqual(ChildWindowCenter.shared.windows(of: id).count, 1)
            spin(0.5)
            // Back to the document's window, which the user closes.
            window?.makeKeyAndOrderFront(nil)
            spin(0.5)
            wasKey = window?.isKeyWindow == true && NSApp.isActive

            document.close()
            XCTAssertTrue(ChildWindowCenter.shared.windows(of: id).isEmpty)
            XCTAssertFalse(NSDocumentController.shared.documents.contains(document))
        }
        spin(2)
        var alive: [String] = []
        if simulation != nil { alive.append("simulation") }
        if schematic != nil { alive.append("schematic") }
        if exporter != nil { alive.append("exporter") }
        if model != nil { alive.append("model") }
        if system != nil { alive.append("system document") }
        if window != nil { alive.append("window") }
        return (alive, wasKey)
    }

    /// The owner's must-fix (plan-59 A6): closing a document frees what it
    /// edited: its model, schematic, simulation and exporter, the system
    /// document and the window.
    func testClosingFreesTheDocument() throws {
        let result = try closeADocument(key: false)
        XCTAssertEqual(result.alive, [], "still alive two seconds after the close")
    }

    /// The same with the window key in the active app, as when the user
    /// closes it: only then does SwiftUI publish the editor's focused values
    /// to the menus, and they must not keep the closed document either.
    /// Skipped when the app is not in front (see `waitForTheAppToBeActive`);
    /// what was in front before gets the front back.
    func testClosingAKeyWindowFreesTheDocument() throws {
        let front = NSWorkspace.shared.frontmostApplication
        defer {
            if let front, front != NSRunningApplication.current { front.activate() }
        }
        let result = try closeADocument(key: true)
        try XCTSkipUnless(result.wasKey, "the app hosting the tests is not in front, so its window was never key")
        XCTAssertEqual(result.alive, [], "still alive two seconds after the close")
    }

    // MARK: plan-61-C phase 1 — title and tab setting

    /// Sets (or, with nil, clears) the Open documents in setting for this
    /// test, and puts back what was there.
    private func setDocumentTabs(_ value: String?) {
        let defaults = UserDefaults.standard
        let old = defaults.object(forKey: PreferenceKey.documentTabs)
        addTeardownBlock { defaults.set(old, forKey: PreferenceKey.documentTabs) }
        defaults.set(value, forKey: PreferenceKey.documentTabs)
    }

    /// The window a new document makes; closed when the test ends.
    private func windowOfANewDocument() throws -> NSWindow {
        let document = newDocument()
        document.makeWindowControllers()
        let window = try XCTUnwrap(document.windowControllers.first?.window)
        addTeardownBlock { @MainActor in document.close() }
        return window
    }

    func testTheWindowShowsItsTitleAndFollowsTheTabSetting() throws {
        setDocumentTabs(nil)
        XCTAssertEqual(DocumentTabs.current(), .tabs, "unset means tabs")
        let byDefault = try windowOfANewDocument()
        XCTAssertEqual(byDefault.tabbingMode, .preferred)
        XCTAssertEqual(byDefault.titleVisibility, .visible)
        XCTAssertEqual(byDefault.toolbarStyle, .unifiedCompact)

        UserDefaults.standard.set(DocumentTabs.windows.rawValue, forKey: PreferenceKey.documentTabs)
        XCTAssertEqual(try windowOfANewDocument().tabbingMode, .disallowed)
        UserDefaults.standard.set(DocumentTabs.system.rawValue, forKey: PreferenceKey.documentTabs)
        XCTAssertEqual(try windowOfANewDocument().tabbingMode, .automatic)
        UserDefaults.standard.set("nonsense", forKey: PreferenceKey.documentTabs)
        XCTAssertEqual(try windowOfANewDocument().tabbingMode, .preferred, "an unknown value means tabs")
    }

    /// Changing the setting reaches the document windows already open.
    func testChangingTheTabSettingReachesOpenDocuments() throws {
        setDocumentTabs(nil)
        let document = openDocument()
        document.makeWindowControllers()
        let window = try XCTUnwrap(document.windowControllers.first?.window)
        XCTAssertEqual(window.tabbingMode, .preferred)
        UserDefaults.standard.set(DocumentTabs.windows.rawValue, forKey: PreferenceKey.documentTabs)
        DocumentTabs.applyToOpenDocuments()
        XCTAssertEqual(window.tabbingMode, .disallowed)
        UserDefaults.standard.set(DocumentTabs.system.rawValue, forKey: PreferenceKey.documentTabs)
        DocumentTabs.applyToOpenDocuments()
        XCTAssertEqual(window.tabbingMode, .automatic)
        document.close()
    }

    // MARK: plan-61-C phase 2 — launch and the blank Untitled

    func testAPlainLaunchWithNoDocumentOpensUntitled() {
        let plain = AEEventID(kAEOpenApplication)
        XCTAssertTrue(AppDelegate.opensUntitled(launchEvent: plain, documentCount: 0, isTestHost: false))
        XCTAssertFalse(AppDelegate.opensUntitled(launchEvent: plain, documentCount: 1, isTestHost: false),
                       "a document was restored")
        XCTAssertFalse(AppDelegate.opensUntitled(launchEvent: nil, documentCount: 0, isTestHost: false),
                       "the app was launched to open a file")
        XCTAssertFalse(AppDelegate.opensUntitled(launchEvent: AEEventID(kAEOpenDocuments), documentCount: 0, isTestHost: false))
        XCTAssertFalse(AppDelegate.opensUntitled(launchEvent: plain, documentCount: 0, isTestHost: true))
        XCTAssertTrue(AppDelegate.isTestHost, "these tests run in the app")
    }

    /// A schematic file on disk with one part.
    private func savedFile(in folder: URL) throws -> URL {
        let source = newDocument()
        _ = try place("SN74HC00N", in: source, at: p(200, 200))
        let url = folder.appendingPathComponent("design.schematical")
        try source.writeSafely(to: url, ofType: Self.type, for: .saveAsOperation)
        return url
    }

    /// An open, untitled document; out of the open documents when the test ends.
    private func openUntitled() -> SchematicNSDocument {
        let document = newDocument()
        NSDocumentController.shared.addDocument(document)
        addTeardownBlock { @MainActor in NSDocumentController.shared.removeDocument(document) }
        return document
    }

    /// A document opened from `url` and given its window, as File › Open does.
    private func openFile(_ url: URL) throws -> SchematicNSDocument {
        let document = TestSupport.keep(try SchematicNSDocument(contentsOf: url, ofType: Self.type))
        NSDocumentController.shared.addDocument(document)
        addTeardownBlock { @MainActor in document.close() }
        document.makeWindowControllers()
        return document
    }

    func testOpeningAFileReplacesAnUntouchedUntitled() throws {
        let url = try savedFile(in: temporaryFolder())
        let blank = openUntitled()
        XCTAssertTrue(blank.isBlankUntitled)
        blank.makeWindowControllers()
        let opened = try openFile(url)
        XCTAssertEqual(opened.fileURL, url)
        XCTAssertFalse(opened.isBlankUntitled)
        let open = NSDocumentController.shared.documents
        XCTAssertFalse(open.contains(blank), "the blank Untitled closed")
        XCTAssertTrue(open.contains(opened))
        XCTAssertTrue(blank.windowControllers.isEmpty)
    }

    func testOpeningAFileKeepsAnUntitledWithWork() throws {
        let url = try savedFile(in: temporaryFolder())
        let untitled = openUntitled()
        untitled.makeWindowControllers()
        addTeardownBlock { @MainActor in untitled.close() }
        _ = try place("SN74HC00N", in: untitled, at: p(200, 200))
        XCTAssertFalse(untitled.isBlankUntitled)
        let opened = try openFile(url)
        let open = NSDocumentController.shared.documents
        XCTAssertTrue(open.contains(untitled), "an Untitled with a part stays")
        XCTAssertTrue(open.contains(opened))

        // A new Untitled document never closes another one.
        let second = openUntitled()
        second.makeWindowControllers()
        addTeardownBlock { @MainActor in second.close() }
        let third = openUntitled()
        third.makeWindowControllers()
        addTeardownBlock { @MainActor in third.close() }
        XCTAssertTrue(NSDocumentController.shared.documents.contains(second))
    }

    // MARK: plan-61-C phase 4 — print

    /// The document's own print operation (what File › Print… and the
    /// system's print action run) is `PrintExport`'s pages, named after it.
    func testThePrintOperationIsTheDocuments() throws {
        let document = newDocument()
        _ = try place("SN74HC00N", in: document, at: p(200, 200))
        let operation = try document.printOperation(withSettings: [:])
        XCTAssertEqual(operation.jobTitle, document.displayName)
        XCTAssertEqual(operation.jobTitle, "Untitled")
        XCTAssertTrue(document.model.file.systemDocument === document)
    }

    /// The unit tests never autosave: a test's document must not be written
    /// to disk, or asked about, behind the test's back.
    func testDocumentsDoNotAutosaveInTheTestHost() {
        XCTAssertTrue(AppDelegate.isTestHost)
        XCTAssertTrue(SchematicNSDocument.autosavesInPlace, "the app's documents autosave in place")
        let document = newDocument()
        _ = document.model.document.addNetLabel(at: p(100, 100))
        TestSupport.turnRunLoop()
        XCTAssertTrue(document.isDocumentEdited)
        document.scheduleAutosaving()
        XCTAssertFalse(document.hasUnautosavedChanges && document.autosavedContentsFileURL != nil)
        XCTAssertNil(document.autosavedContentsFileURL)
    }
}
