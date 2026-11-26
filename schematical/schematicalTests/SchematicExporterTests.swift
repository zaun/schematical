import AppKit
import PDFKit
import SwiftUI
import XCTest
@testable import schematical

/// The answers a test gives `ExportPresenter`: the URLs its panels choose and
/// the buttons its alerts press, in order, and what was put up.
@MainActor
final class ExportAnswers {
    nonisolated deinit {}
    var panels: [NSSavePanel] = []
    var alerts: [NSAlert] = []
    var urls: [URL?] = []
    var buttons: [NSApplication.ModalResponse] = []
    var printedDocuments: [NSDocument] = []
    var operations: [NSPrintOperation] = []

    var presenter: ExportPresenter {
        var presenter = ExportPresenter()
        presenter.choose = { [self] panel, _ in
            panels.append(panel)
            return urls.isEmpty ? nil : urls.removeFirst()
        }
        presenter.alert = { [self] alert, _ in
            alerts.append(alert)
            return buttons.isEmpty ? .cancel : buttons.removeFirst()
        }
        presenter.keyWindow = { nil }
        presenter.printDocument = { [self] in printedDocuments.append($0) }
        presenter.print = { [self] operation, _ in operations.append(operation) }
        return presenter
    }
}

/// plan-68-Y: File › Export, Import and Print through `SchematicExporter`, its
/// panels and alerts answered by the test (`ExportPresenter`), its sheets on
/// a window of the test's and pressed through the accessibility tree.
@MainActor
final class SchematicExporterTests: XCTestCase {
    private var parent: NSWindow!
    private var answers: ExportAnswers!
    private var folder: URL!

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
        parent = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 900, height: 700), styleMask: [.titled],
                          backing: .buffered, defer: false)
        parent.isReleasedWhenClosed = false
        parent.orderFront(nil)
        answers = ExportAnswers()
        folder = FileManager.default.temporaryDirectory.appendingPathComponent("exporter-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
    }

    override func tearDown() async throws {
        if let sheet = parent.attachedSheet { parent.endSheet(sheet) }
        parent.close()
        try? FileManager.default.removeItem(at: folder)
    }

    /// An exporter of `doc` (one gate when nil) with its sheets on the test's window.
    private func exporter(_ given: SchematicDocument? = nil) throws -> (file: SchematicExporter, doc: SchematicDocument) {
        let doc: SchematicDocument
        if let given {
            doc = given
        } else {
            doc = TestSupport.newDocument()
            try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        }
        let file = SchematicExporter(document: doc, catalog: TestSupport.catalog)
        file.window = parent
        file.presenter = answers.presenter
        return (file, doc)
    }

    private func wait(_ timeout: TimeInterval = 5, until condition: () -> Bool) async {
        let end = Date().addingTimeInterval(timeout)
        while !condition(), Date() < end { try? await Task.sleep(for: .milliseconds(50)) }
    }

    /// The sheet on the test's window once it is up.
    private func sheet() async throws -> ViewProbe {
        await wait { parent.attachedSheet != nil }
        let sheet = try XCTUnwrap(parent.attachedSheet, "no sheet")
        return ViewProbe(window: sheet)
    }

    private var missing: URL { folder.appendingPathComponent("missing/none") }

    // MARK: KiCad and EasyEDA

    func testKiCadProjectIsWrittenIntoTheFolderAndAnOldOneIsReplacedOnlyWhenAsked() async throws {
        let (file, _) = try exporter()
        file.fileURL = folder.appendingPathComponent("Clock.schematical")
        answers.urls = [folder]
        await file.exportKiCadProject()
        let panel = try XCTUnwrap(answers.panels.first as? NSOpenPanel)
        XCTAssertTrue(panel.canChooseDirectories)
        XCTAssertFalse(panel.canChooseFiles)
        XCTAssertEqual(panel.prompt, "Export")
        XCTAssertEqual(panel.message, "Choose the folder for Clock.kicad_pro, Clock.kicad_sch, and Clock.kicad_pcb.")
        XCTAssertEqual(panel.directoryURL?.standardizedFileURL, folder.standardizedFileURL)
        let project = folder.appendingPathComponent("Clock.kicad_pro")
        for url in file.kiCadFiles(in: folder) { XCTAssertTrue(FileManager.default.fileExists(atPath: url.path), url.path) }

        // Again: asked; Cancel leaves the files.
        try Data("old".utf8).write(to: project)
        answers.urls = [folder]
        answers.buttons = [.alertSecondButtonReturn]
        await file.exportKiCadProject()
        let alert = try XCTUnwrap(answers.alerts.last)
        XCTAssertEqual(alert.messageText, "Replace the KiCad project “Clock” in “\(folder.lastPathComponent)”?")
        XCTAssertTrue(alert.informativeText.hasSuffix("Exporting replaces them."), alert.informativeText)
        XCTAssertEqual(try String(contentsOf: project, encoding: .utf8), "old")

        // Replace writes them again.
        answers.urls = [folder]
        answers.buttons = [.alertFirstButtonReturn]
        await file.exportKiCadProject()
        XCTAssertNotEqual(try String(contentsOf: project, encoding: .utf8), "old")

        // One old file: "replaces it".
        for url in file.kiCadFiles(in: folder) { try FileManager.default.removeItem(at: url) }
        try Data("old".utf8).write(to: project)
        answers.urls = [folder]
        await file.exportKiCadProject()
        XCTAssertTrue(try XCTUnwrap(answers.alerts.last).informativeText.hasSuffix("Exporting replaces it."))

        // Cancelled: nothing more is asked.
        let asked = answers.alerts.count
        await file.exportKiCadProject()
        XCTAssertEqual(answers.alerts.count, asked)
    }

    func testAFailedExportSaysWhy() async throws {
        let (file, _) = try exporter()
        answers.urls = [missing]
        await file.exportKiCadProject()
        await wait { !answers.alerts.isEmpty }
        XCTAssertEqual(answers.alerts.first?.messageText, "Couldn't export “Untitled”")
        XCTAssertFalse(answers.alerts.first?.informativeText.isEmpty ?? true)

        answers.urls = [missing.appendingPathExtension("epro")]
        await file.exportEasyEDAProProject()
        await wait { answers.alerts.count == 2 }
        XCTAssertEqual(answers.alerts.last?.messageText, "Couldn't export “Untitled”")
    }

    func testEasyEDAProProjectIsOneFile() async throws {
        let (file, _) = try exporter()
        await file.exportEasyEDAProProject()
        XCTAssertEqual(answers.panels.first?.nameFieldStringValue, "Untitled.epro", "cancelled: nothing written")
        let url = folder.appendingPathComponent("Board.epro")
        answers.urls = [url]
        await file.exportEasyEDAProProject()
        XCTAssertTrue(FileManager.default.fileExists(atPath: url.path))
    }

    // MARK: Image

    func testImageSheetThenPanelThenFile() async throws {
        let (file, _) = try exporter()
        file.fileURL = folder.appendingPathComponent("Clock.schematical")
        let url = folder.appendingPathComponent("Clock.png")
        answers.urls = [url]
        let task = Task { await file.exportImage() }
        let probe = try await sheet()
        XCTAssertTrue(probe.has("Export Image"), "\(probe.labels())")
        try probe.press("Export…")
        await task.value
        let panel = try XCTUnwrap(answers.panels.first)
        XCTAssertTrue(panel.nameFieldStringValue.hasPrefix("Clock "), panel.nameFieldStringValue)
        XCTAssertEqual(panel.directoryURL?.standardizedFileURL, folder.standardizedFileURL)
        XCTAssertTrue(panel.nameFieldStringValue.hasSuffix(".\(ImageExportOptions.saved().format.fileExtension)"))
        XCTAssertFalse(try Data(contentsOf: url).isEmpty, "the picture is written")
    }

    func testAnImageThatCannotBeWrittenSaysWhy() async throws {
        let (file, _) = try exporter()
        answers.urls = [missing]
        let task = Task { await file.exportImage() }
        try await sheet().press("Export…")
        await task.value
        await wait { !answers.alerts.isEmpty }
        XCTAssertEqual(answers.alerts.first?.messageText, "Couldn't export “Untitled”")
    }

    func testPrintingNoPagesSaysWhy() async throws {
        let (file, doc) = try exporter()
        var options = PrintOptions()
        options.hidden = Set(PrintExport.kinds(of: doc))
        file.print(options)
        await wait { !answers.alerts.isEmpty }
        XCTAssertEqual(answers.alerts.first?.messageText, "Couldn't print “Untitled”")
        XCTAssertEqual(answers.alerts.first?.informativeText, "No pages are chosen, or none has anything to draw.")
        XCTAssertTrue(answers.operations.isEmpty)
    }

    func testImageCancelledInTheSheetAsksNothingMore() async throws {
        let (file, _) = try exporter()
        let task = Task { await file.exportImage() }
        let probe = try await sheet()
        try probe.press("Cancel")
        // Pressed again, the sheet's finish runs once.
        try probe.press("Cancel")
        await task.value
        XCTAssertTrue(answers.panels.isEmpty)
        XCTAssertNil(parent.attachedSheet)
    }

    func testAnImageOfAnEmptySchematicSaysThereIsNothing() async throws {
        let (file, _) = try exporter(TestSupport.newDocument())
        var options = ImageExportOptions()
        options.view = .schematic
        XCTAssertThrowsError(try file.writeImage(options, to: folder.appendingPathComponent("e.png"))) {
            XCTAssertEqual($0.localizedDescription, "The schematic has no parts, wires or buses.")
        }
        XCTAssertEqual(SchematicExporter.ExportError.nothingToPrint.errorDescription,
                       "No pages are chosen, or none has anything to draw.")
        // Through the sheet: Export… is off.
        let task = Task { await file.exportImage() }
        let probe = try await sheet()
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Export…", role: "AXButton")))
        try probe.press("Cancel")
        await task.value
    }

    // MARK: PDF and Print

    func testPDFSheetThenPanelThenFile() async throws {
        let (file, _) = try exporter()
        file.fileURL = folder.appendingPathComponent("Clock.schematical")
        let url = folder.appendingPathComponent("Clock.pdf")
        answers.urls = [url]
        let task = Task { await file.exportPDF() }
        let probe = try await sheet()
        XCTAssertTrue(probe.has("Export PDF"), "\(probe.labels())")
        try probe.press("Save PDF…")
        await task.value
        XCTAssertEqual(answers.panels.first?.nameFieldStringValue, "Clock.pdf")
        XCTAssertEqual(answers.panels.first?.directoryURL?.standardizedFileURL, folder.standardizedFileURL)
        XCTAssertNotNil(PDFDocument(url: url))

        // A folder that is not there: the reason is shown.
        answers.urls = [missing]
        let again = Task { await file.exportPDF() }
        try await sheet().press("Save PDF…")
        await again.value
        await wait { !answers.alerts.isEmpty }
        XCTAssertEqual(answers.alerts.first?.messageText, "Couldn't export “Clock”")

        // Cancelled in the panel or the sheet: nothing written.
        let cancelled = Task { await file.exportPDF() }
        try await sheet().press("Cancel")
        await cancelled.value
        let panels = answers.panels.count
        let unchosen = Task { await file.exportPDF() }
        try await sheet().press("Save PDF…")
        await unchosen.value
        XCTAssertEqual(answers.panels.count, panels + 1)
    }

    func testWritingAPDFOfNoPagesThrows() throws {
        let (file, doc) = try exporter()
        var options = PrintOptions()
        options.hidden = Set(PrintExport.kinds(of: doc))
        XCTAssertThrowsError(try file.writePDF(options, to: folder.appendingPathComponent("none.pdf")))
        XCTAssertThrowsError(try file.printOperation(options, jobTitle: "none"))
    }

    func testPrintGoesThroughTheSystemDocumentWithTheSheetsPages() async throws {
        let (file, _) = try exporter()
        let system = TestSupport.systemDocument(of: TestSupport.newDocument())
        file.systemDocument = system
        let task = Task { await file.printPages() }
        let probe = try await sheet()
        XCTAssertTrue(probe.has("Print"), "\(probe.labels())")
        try probe.press("Print…")
        await task.value
        XCTAssertTrue(answers.printedDocuments.first === system)
        let chosen = try XCTUnwrap(file.optionsForNextPrint)
        XCTAssertEqual(file.takePrintOptions(), chosen)
        XCTAssertNil(file.optionsForNextPrint, "taken once")
        XCTAssertEqual(file.takePrintOptions(), PrintOptions.saved())
    }

    func testPrintWithNoSystemDocumentRunsTheOperationHere() async throws {
        let (file, _) = try exporter()
        let task = Task { await file.printPages() }
        try await sheet().press("Print…")
        await task.value
        XCTAssertEqual(answers.operations.first?.jobTitle, "Untitled")
        XCTAssertTrue(answers.printedDocuments.isEmpty)

        let cancelled = Task { await file.printPages() }
        try await sheet().press("Cancel")
        await cancelled.value
        XCTAssertEqual(answers.operations.count, 1)
    }

    // MARK: Gerber

    func testGerberOfACleanBoardIsAZip() async throws {
        let (file, _) = try exporter(TestSupport.newDocument())
        file.fileURL = folder.appendingPathComponent("My Board!.schematical")
        XCTAssertEqual(file.gerberBaseName, "My_Board_")
        let url = folder.appendingPathComponent("b.zip")
        answers.urls = [url]
        await file.exportGerber()
        XCTAssertTrue(answers.alerts.isEmpty, "a clean board is not questioned")
        XCTAssertEqual(answers.panels.first?.nameFieldStringValue, "My Board! Gerbers.zip")
        XCTAssertEqual(answers.panels.first?.directoryURL?.standardizedFileURL, folder.standardizedFileURL)
        XCTAssertEqual(Array(try Data(contentsOf: url).prefix(2)), [0x50, 0x4B])

        answers.urls = [missing]
        await file.exportGerber()
        await wait { !answers.alerts.isEmpty }
        XCTAssertEqual(answers.alerts.first?.messageText, "Couldn't export “My Board!”")
    }

    func testABoardWithProblemsIsQuestionedFirst() async throws {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let two = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 600, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "1Y")), .pin(try TestSupport.pin(two, "1A"))))
        let issues = doc.designRuleCheck(rules: doc.board.rules)
        XCTAssertFalse(issues.isEmpty)
        let (file, _) = try exporter(doc)
        await file.exportGerber()
        let alert = try XCTUnwrap(answers.alerts.first)
        XCTAssertEqual(alert.messageText,
                       "The board has \(issues.count) design rule \(issues.count == 1 ? "problem" : "problems")")
        XCTAssertTrue(alert.informativeText.hasPrefix(SchematicExporter.problemSummary(issues)))
        XCTAssertTrue(answers.panels.isEmpty, "Cancel: no panel")

        answers.buttons = [.alertSecondButtonReturn]
        await file.exportGerber()
        XCTAssertEqual(answers.panels.count, 1, "Export Anyway goes on to the panel")
    }

    // MARK: DXF

    private func dxf(_ text: String, _ name: String = "board.dxf") throws -> URL {
        let url = folder.appendingPathComponent(name)
        try Data(text.utf8).write(to: url)
        return url
    }

    func testImportDXFShowsTheSheetAndImportsTheOutline() async throws {
        let (file, doc) = try exporter(TestSupport.newDocument())
        file.fileURL = folder.appendingPathComponent("Clock.schematical")
        answers.urls = [try dxf(DXFFixtures.roundedBoard)]
        let task = Task { await file.importDXF() }
        let probe = try await sheet()
        XCTAssertTrue(probe.has("Import “board.dxf”"), "\(probe.labels())")
        try probe.press("Import")
        await task.value
        XCTAssertEqual(answers.panels.first?.directoryURL?.standardizedFileURL, folder.standardizedFileURL)
        XCTAssertFalse(try XCTUnwrap(answers.panels.first as? NSOpenPanel).allowsMultipleSelection)
        XCTAssertEqual(doc.undoName, "Import DXF")
        XCTAssertTrue(doc.board.isFreeForm)
    }

    func testImportDXFCancelledOrUnreadable() async throws {
        let (file, doc) = try exporter(TestSupport.newDocument())
        let before = doc.board
        answers.urls = [try dxf(DXFFixtures.roundedBoard)]
        let task = Task { await file.importDXF() }
        try await sheet().press("Cancel")
        await task.value
        XCTAssertEqual(doc.board, before)

        await file.importDXF()
        XCTAssertEqual(answers.panels.count, 2, "a cancelled panel imports nothing")

        answers.urls = [try dxf("not a drawing", "bad.dxf")]
        await file.importDXF()
        await wait { !answers.alerts.isEmpty }
        XCTAssertEqual(answers.alerts.first?.messageText, "Couldn't import “bad.dxf”")
        XCTAssertEqual(doc.board, before)
        XCTAssertThrowsError(try file.loadDXF(from: folder.appendingPathComponent("none.dxf")))
    }

    // MARK: The User Library offer

    func testTheOfferToAddNamesThePartsAndNotNowChangesNothing() async throws {
        let (file, _) = try exporter()
        await file.offerToAdd([])
        XCTAssertEqual(answers.alerts.count, 1)
        answers.buttons = [.alertFirstButtonReturn]
        await file.offerToAdd([])
        XCTAssertEqual(answers.alerts.count, 2, "nothing to add: no failures to show")
    }

    // MARK: A sheet with no window

    func testASheetWithNoWindowIsAModalWindowOfItsOwn() async throws {
        let (file, _) = try exporter()
        file.window = nil
        var shown: [String] = []
        let timer = Timer(timeInterval: 0.5, repeats: false) { _ in
            MainActor.assumeIsolated {
                guard let modal = NSApp.modalWindow else { return }
                let probe = ViewProbe(window: modal)
                shown = probe.labels()
                try? probe.press("Cancel")
            }
        }
        RunLoop.main.add(timer, forMode: .modalPanel)
        await file.exportImage()
        XCTAssertTrue(shown.contains { $0.contains("Export Image") }, "\(shown)")
        XCTAssertNil(NSApp.modalWindow)
        XCTAssertTrue(answers.panels.isEmpty)
    }

    // MARK: The system's presenter

    /// The real panels and alerts, put up and taken down by the test at once.
    func testTheSystemsPanelsAndAlertsCanBeCancelled() async {
        let presenter = ExportPresenter()
        XCTAssertTrue(presenter.keyWindow() === NSApp.keyWindow)

        // On a window: a sheet, ended by the test.
        let sheetTimer = Timer(timeInterval: 0.5, repeats: false) { [parent] _ in
            MainActor.assumeIsolated { (parent?.attachedSheet as? NSSavePanel)?.cancel(nil) }
        }
        RunLoop.main.add(sheetTimer, forMode: .common)
        let onWindow = await presenter.choose(NSSavePanel(), parent)
        XCTAssertNil(onWindow)

        // On its own: app-modal, aborted.
        let modalTimer = Timer(timeInterval: 0.5, repeats: false) { _ in MainActor.assumeIsolated { NSApp.abortModal() } }
        RunLoop.main.add(modalTimer, forMode: .modalPanel)
        let alone = await presenter.choose(NSSavePanel(), nil)
        XCTAssertNil(alone)

        let alert = NSAlert()
        alert.messageText = "plan-68-Y"
        let alertTimer = Timer(timeInterval: 0.5, repeats: false) { [parent] _ in
            MainActor.assumeIsolated {
                if let sheet = parent?.attachedSheet { parent?.endSheet(sheet, returnCode: .alertSecondButtonReturn) }
            }
        }
        RunLoop.main.add(alertTimer, forMode: .common)
        let pressed = await presenter.alert(alert, parent)
        XCTAssertEqual(pressed, .alertSecondButtonReturn)

        let abort = Timer(timeInterval: 0.5, repeats: false) { _ in MainActor.assumeIsolated { NSApp.abortModal() } }
        RunLoop.main.add(abort, forMode: .modalPanel)
        let modal = await presenter.alert(NSAlert(), nil)
        XCTAssertEqual(modal, .abort)
    }

    /// A print that saves a PDF into the test's folder, never to a printer.
    private func savingOperation(_ name: String) -> NSPrintOperation {
        let info = NSPrintInfo()
        info.jobDisposition = .save
        info.dictionary()[NSPrintInfo.AttributeKey.jobSavingURL] = folder.appendingPathComponent("\(name).pdf")
        return NSPrintOperation(view: NSView(frame: NSRect(x: 0, y: 0, width: 100, height: 100)), printInfo: info)
    }

    /// A document whose print is `savingOperation`'s.
    private final class SavingDocument: NSDocument {
        var operation: NSPrintOperation!
        override func printOperation(withSettings printSettings: [NSPrintInfo.AttributeKey: Any]) throws -> NSPrintOperation {
            operation
        }
    }

    /// The real print panels: on a window, and on their own, cancelled by the test.
    func testTheSystemsPrintPanelsCanBeCancelled() {
        let presenter = ExportPresenter()
        var ended = false
        let sheetTimer = Timer(timeInterval: 0.5, repeats: false) { [parent] _ in
            MainActor.assumeIsolated {
                if let sheet = parent?.attachedSheet {
                    parent?.endSheet(sheet, returnCode: .cancel)
                    ended = true
                }
            }
        }
        RunLoop.main.add(sheetTimer, forMode: .common)
        presenter.print(savingOperation("sheet"), parent)
        let end = Date().addingTimeInterval(3)
        while !ended, Date() < end { RunLoop.main.run(until: Date().addingTimeInterval(0.05)) }
        XCTAssertTrue(ended, "the print panel came up on the window")

        var aborted = 0
        func abortSoon() {
            let timer = Timer(timeInterval: 0.5, repeats: false) { _ in
                MainActor.assumeIsolated {
                    aborted += 1
                    NSApp.abortModal()
                }
            }
            RunLoop.main.add(timer, forMode: .modalPanel)
        }
        abortSoon()
        presenter.print(savingOperation("alone"), nil)
        let document = SavingDocument()
        document.operation = savingOperation("document")
        abortSoon()
        presenter.printDocument(document)
        let stop = Date().addingTimeInterval(3)
        while aborted < 2, Date() < stop { RunLoop.main.run(until: Date().addingTimeInterval(0.05)) }
        XCTAssertEqual(aborted, 2)
        XCTAssertNil(NSApp.modalWindow)
    }
}

/// plan-68-Y: File › Load Part… and the User Library's Add Part…, their panel
/// and alerts answered by the test.
@MainActor
final class PartFileLoadingTests: XCTestCase {
    private var directory: URL!
    private var answers: ExportAnswers!
    private static var kept: [PartCatalogStore] = []

    override func setUp() async throws {
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("loading-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        answers = ExportAnswers()
        PartFileLoading.presenter = answers.presenter
    }

    override func tearDown() async throws {
        PartFileLoading.presenter = ExportPresenter()
        try? FileManager.default.removeItem(at: directory)
    }

    private func store() -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        store.attachUserLibrary(UserPartLibrary(directory: directory.appendingPathComponent("library")))
        return store
    }

    private func partFile(_ text: String, _ name: String = "part.schpart") throws -> URL {
        let url = directory.appendingPathComponent(name)
        try Data(text.utf8).write(to: url)
        return url
    }

    private func text(_ name: String) -> String {
        PartRegistrationTests.fileText(uuid: UUID().uuidString, name: name, description: "Single 2-input NAND gate")
    }

    func testAddPartAddsTheFileToTheLibraryWithNoQuestion() async throws {
        let store = store()
        await PartFileLoading.addToLibrary(catalog: store)
        XCTAssertEqual(answers.panels.first?.message, "Choose a part file to add to your User Library")
        XCTAssertEqual(answers.panels.first?.allowedContentTypes, PartFileType.openableTypes)

        answers.urls = [try partFile(text("PFL-A"))]
        await PartFileLoading.addToLibrary(catalog: store)
        XCTAssertNotNil(store.part(named: "PFL-A"))
        XCTAssertTrue(answers.alerts.isEmpty)

        answers.urls = [try partFile("not json", "bad.schpart")]
        await PartFileLoading.addToLibrary(catalog: store)
        XCTAssertEqual(answers.alerts.first?.messageText, "Couldn't add “bad.schpart”")
    }

    func testLoadPartAsksAndAnswersEachWay() async throws {
        let store = store()
        await PartFileLoading.loadPart(catalog: store)
        XCTAssertEqual(answers.panels.first?.message, "Choose a part file to load")

        // Cancel: nothing is added.
        answers.urls = [try partFile(text("PFL-B"), "b.schpart")]
        await PartFileLoading.loadPart(catalog: store)
        XCTAssertNil(store.part(named: "PFL-B"))
        XCTAssertEqual(answers.alerts.count, 1, "asked")

        // Use Once: in the catalog, not the library.
        answers.urls = [try partFile(text("PFL-C"), "c.schpart")]
        answers.buttons = [.alertSecondButtonReturn]
        await PartFileLoading.loadPart(catalog: store)
        XCTAssertNotNil(store.part(named: "PFL-C"))

        // Add, then the same file again: already in the library.
        let same = try partFile(text("PFL-D"), "d.schpart")
        answers.urls = [same]
        answers.buttons = [.alertFirstButtonReturn]
        await PartFileLoading.loadPart(catalog: store)
        XCTAssertNotNil(store.part(named: "PFL-D"))
        let asked = answers.alerts.count
        answers.urls = [same]
        await PartFileLoading.loadPart(catalog: store)
        XCTAssertEqual(answers.alerts.count, asked + 1)
        XCTAssertTrue(try XCTUnwrap(answers.alerts.last).messageText.contains("PFL-D"), answers.alerts.last?.messageText ?? "")

        // A file that is not a part: the reason.
        answers.urls = [try partFile("not json", "e.schpart")]
        await PartFileLoading.loadPart(catalog: store)
        XCTAssertEqual(answers.alerts.last?.messageText, "Couldn't load “e.schpart”")
    }

    func testThePartFileTypeOpensByItsExtension() {
        XCTAssertEqual(PartFileType.fileExtension, "schpart")
        XCTAssertEqual(PartFileType.openableTypes.first, PartFileType.contentType)
    }
}
