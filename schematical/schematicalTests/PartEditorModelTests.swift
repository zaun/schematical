import AppKit
import PDFKit
import SwiftUI
import XCTest
@testable import schematical

/// The Part Editor model's panels, alerts, close guard and datasheet pane
/// (plan-68-AD): every panel and alert is answered by an injected presenter.
@MainActor
final class PartEditorModelTests: XCTestCase {
    /// Kept for the life of the process (freeing a model inside a test aborts the host).
    private static var kept: [AnyObject] = []
    private var directory: URL!
    private var windows: [NSWindow] = []
    /// The alerts put up, by message, and the panels' messages.
    private var alerts: [String] = []
    private var panels = 0

    override func setUp() async throws {
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("pemodel-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        alerts = []
        panels = 0
        // Closing a test's window must not quit the host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        windows.forEach { $0.close() }
        windows = []
        try? FileManager.default.removeItem(at: directory)
        Stub.answer = nil
    }

    /// A model whose panels choose `url` and whose alerts answer `answer`.
    private func model(_ source: PartEditorTarget.Source, catalog: PartCatalogStore = .shared,
                       choosing url: URL? = nil,
                       answer: NSApplication.ModalResponse = .alertFirstButtonReturn) -> PartEditorModel {
        let model = PartEditorModel(target: PartEditorTarget(source: source), catalog: catalog)
        Self.kept.append(model)
        model.presenter.choose = { [weak self] _, _ in
            self?.panels += 1
            return url
        }
        model.presenter.alert = { [weak self] alert, _ in
            self?.alerts.append(alert.messageText)
            return answer
        }
        return model
    }

    private func file(_ name: String) -> URL {
        directory.appendingPathComponent(name)
    }

    // MARK: Save and Save As…

    func testSaveAsWritesTheChosenFileAndSaveRewritesIt() async throws {
        let url = file("copy.schpart")
        let editor = model(.copy(name: "SN74HC00N"), choosing: url)
        await editor.save()
        XCTAssertEqual(panels, 1)
        XCTAssertEqual(editor.fileURL, url)
        XCTAssertEqual(editor.status, "Saved copy.schpart")
        XCTAssertFalse(editor.isDirty)

        editor.draft.description = "Changed"
        await editor.save()
        XCTAssertEqual(panels, 1)
        XCTAssertFalse(editor.isDirty)
        XCTAssertTrue(try String(contentsOf: url, encoding: .utf8).contains("Changed"))
        XCTAssertEqual(alerts, [])
    }

    func testSaveAsCancelledWritesNothing() async {
        let editor = model(.copy(name: "SN74HC00N"), choosing: nil)
        await editor.saveAs()
        XCTAssertEqual(panels, 1)
        XCTAssertNil(editor.fileURL)
        XCTAssertNil(editor.status)
    }

    func testARejectedDraftShowsWhySaveFailed() async throws {
        let url = file("bad.schpart")
        let editor = model(.new(kind: "generic"), choosing: url)
        await editor.saveAs()
        XCTAssertEqual(alerts, ["Couldn't save the part"])
        XCTAssertFalse(FileManager.default.fileExists(atPath: url.path))

        // Saved once (valid), then broken: Save shows the same alert.
        let good = model(.copy(name: "SN74HC00N"), choosing: file("good.schpart"))
        await good.saveAs()
        good.draft.pinouts[0].pins = []
        await good.save()
        XCTAssertEqual(alerts, ["Couldn't save the part", "Couldn't save the part"])
    }

    // MARK: Open

    func testOpenAsksBeforeDiscardingChanges() async throws {
        let source = file("source.schpart")
        let saver = model(.copy(name: "SN74HC00N"), choosing: source)
        try saver.write(to: source)

        let cancelled = model(.copy(name: "SN74HC00N"), choosing: source, answer: .alertSecondButtonReturn)
        cancelled.draft.description = "Edited"
        await cancelled.open()
        XCTAssertEqual(alerts, ["Discard the changes to this part?"])
        XCTAssertEqual(panels, 0)
        XCTAssertNil(cancelled.fileURL)

        let discarded = model(.copy(name: "SN74HC00N"), choosing: source)
        discarded.draft.description = "Edited"
        await discarded.open()
        XCTAssertEqual(panels, 1)
        XCTAssertEqual(discarded.fileURL, source)
        XCTAssertEqual(discarded.status, "Opened source.schpart")

        XCTAssertEqual(alerts.count, 2)

        // Unchanged: no question; a cancelled panel opens nothing.
        let clean = model(.copy(name: "SN74HC00N"), choosing: nil)
        await clean.open()
        XCTAssertEqual(alerts.count, 2)
        XCTAssertEqual(panels, 2)
        XCTAssertNil(clean.fileURL)
    }

    func testAFileThatDoesNotLoadShowsWhy() async throws {
        let junk = file("junk.schpart")
        try Data("not json".utf8).write(to: junk)
        let editor = model(.copy(name: "SN74HC00N"))
        let before = editor.draft
        await editor.open(junk)
        XCTAssertEqual(alerts, ["Couldn't open “junk.schpart”"])
        XCTAssertEqual(editor.draft, before)
    }

    // MARK: The User Library

    func testLibraryFailuresShowAlerts() async throws {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        store.attachUserLibrary(UserPartLibrary(directory: directory))
        let uuid = "F2000000-0000-4000-8000-000000000001"
        var draft = try XCTUnwrap(PartEditorModel.copy(named: "SN74HC00N", catalog: store))
        draft.uuid = uuid
        draft.name = "PEM-A"
        try store.addToUserLibrary(fileText: draft.fileText(), source: "PEM-A.schpart")

        let editor = model(.libraryPart(uuid: uuid), catalog: store)
        XCTAssertEqual(editor.libraryUUID, uuid)
        editor.draft.pinouts[0].pins = []
        await editor.save()
        await editor.saveAs()
        XCTAssertEqual(alerts, ["Couldn't update the part", "Couldn't add the part"])
        XCTAssertEqual(panels, 0)

        let plain = model(.new(kind: "generic"), catalog: store)
        await plain.addToLibraryFromMenu()
        XCTAssertEqual(alerts.last, "Couldn't add the part")
    }

    func testLibrarySaveAndSaveAsSucceedWithoutAPanel() async throws {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        store.attachUserLibrary(UserPartLibrary(directory: directory))
        let uuid = "F2000000-0000-4000-8000-000000000002"
        var draft = try XCTUnwrap(PartEditorModel.copy(named: "SN74HC00N", catalog: store))
        draft.uuid = uuid
        draft.name = "PEM-B"
        try store.addToUserLibrary(fileText: draft.fileText(), source: "PEM-B.schpart")

        let editor = model(.libraryPart(uuid: uuid), catalog: store)
        editor.draft.description = "Updated"
        await editor.save()
        XCTAssertEqual(editor.status, "Updated PEM-B in your User Library")
        await editor.saveAs()
        XCTAssertNotEqual(editor.libraryUUID, uuid)
        XCTAssertEqual(editor.status?.hasPrefix("Added PEM-B"), true)
        XCTAssertEqual(alerts, [])
        XCTAssertEqual(panels, 0)
        await editor.addToLibraryFromMenu()
        XCTAssertEqual(alerts, [])
    }

    // MARK: Closing a changed window

    private func window(for editor: PartEditorModel) -> NSWindow {
        let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 200, height: 100), styleMask: [.titled, .closable],
                              backing: .buffered, defer: true)
        window.isReleasedWhenClosed = false
        windows.append(window)
        editor.window = window
        return window
    }

    /// Asks the window to close as its close button would, and lets the
    /// question that follows run.
    private func askToClose(_ window: NSWindow) async -> Bool {
        let allowed = window.delegate?.windowShouldClose?(window) ?? true
        for _ in 0..<20 { await Task.yield() }
        try? await Task.sleep(for: .milliseconds(50))
        return allowed
    }

    func testAnUnchangedWindowClosesWithoutAsking() async {
        let editor = model(.copy(name: "SN74HC00N"))
        let window = window(for: editor)
        let allowed = await askToClose(window)
        XCTAssertTrue(allowed)
        XCTAssertEqual(alerts, [])
    }

    func testClosingAChangedWindowAsksAndSaves() async throws {
        let url = file("closing.schpart")
        let editor = model(.copy(name: "SN74HC00N"), choosing: url)
        editor.draft.description = "Edited"
        let window = window(for: editor)
        window.orderFront(nil)
        let allowed = await askToClose(window)
        XCTAssertFalse(allowed)
        XCTAssertEqual(alerts.first?.hasPrefix("Save the changes to "), true)
        XCTAssertEqual(editor.fileURL, url)
        XCTAssertFalse(window.isVisible)
    }

    func testClosingAChangedWindowCanDiscardOrCancel() async {
        let kept = model(.copy(name: "SN74HC00N"), answer: .alertSecondButtonReturn)
        kept.draft.description = "Edited"
        let keptWindow = window(for: kept)
        keptWindow.orderFront(nil)
        _ = await askToClose(keptWindow)
        XCTAssertTrue(keptWindow.isVisible)
        XCTAssertTrue(kept.isDirty)

        let discarded = model(.copy(name: "SN74HC00N"), answer: .alertThirdButtonReturn)
        discarded.draft.description = "Edited"
        let discardedWindow = window(for: discarded)
        discardedWindow.orderFront(nil)
        _ = await askToClose(discardedWindow)
        XCTAssertFalse(discardedWindow.isVisible)
        XCTAssertFalse(discarded.isDirty)

        // Save… whose save fails leaves the window open.
        let failing = model(.new(kind: "generic"), choosing: file("never.schpart"))
        failing.draft.description = "Edited"
        let failingWindow = window(for: failing)
        failingWindow.orderFront(nil)
        _ = await askToClose(failingWindow)
        XCTAssertTrue(failingWindow.isVisible)
        XCTAssertTrue(alerts.contains("Couldn't save the part"))
    }

    // MARK: The datasheet pane

    final class Stub: URLProtocol {
        nonisolated(unsafe) static var answer: (status: Int, body: Data)?

        override class func canInit(with request: URLRequest) -> Bool { true }
        override class func canonicalRequest(for request: URLRequest) -> URLRequest { request }

        override func startLoading() {
            guard let answer = Self.answer, let url = request.url else {
                client?.urlProtocol(self, didFailWithError: URLError(.notConnectedToInternet))
                return
            }
            let response = HTTPURLResponse(url: url, statusCode: answer.status, httpVersion: nil, headerFields: nil)!
            client?.urlProtocol(self, didReceive: response, cacheStoragePolicy: .notAllowed)
            client?.urlProtocol(self, didLoad: answer.body)
            client?.urlProtocolDidFinishLoading(self)
        }

        override func stopLoading() {}
        nonisolated deinit {}
    }

    private static func pdfData() throws -> Data {
        let document = PDFDocument()
        document.insert(PDFPage(), at: 0)
        return try XCTUnwrap(document.dataRepresentation())
    }

    private func stubbedSession() -> URLSession {
        let configuration = URLSessionConfiguration.ephemeral
        configuration.protocolClasses = [Stub.self]
        return URLSession(configuration: configuration)
    }

    func testLoadDatasheetShowsThePDFOrWhyNot() async throws {
        let editor = model(.new(kind: "generic"))
        editor.datasheetSession = stubbedSession()
        editor.draft.setDatasheetURL(nil)
        await editor.loadDatasheet()
        XCTAssertEqual(editor.datasheetPhase, .failed("There is no datasheet address to load."))

        editor.draft.setDatasheetURL("https://example.com/sheets/part.pdf")
        Stub.answer = (404, Data())
        await editor.loadDatasheet()
        XCTAssertEqual(editor.datasheetPhase, .failed(DatasheetLoader.Failure.status(404).message))
        XCTAssertEqual(editor.guidedPane, .block)

        Stub.answer = (200, try Self.pdfData())
        await editor.loadDatasheet()
        XCTAssertEqual(editor.datasheetPhase, .loaded("example.com"))
        XCTAssertEqual(editor.guidedPane, .datasheet)
        XCTAssertNotNil(editor.datasheet.document)
    }

    func testALocalFileShowsWhenItIsAPDF() throws {
        let editor = model(.new(kind: "generic"))
        let text = file("notes.txt")
        try Data("hello".utf8).write(to: text)
        editor.showLocalDatasheet(text)
        XCTAssertEqual(editor.datasheetPhase, .failed("“notes.txt” is not a PDF."))

        let pdf = file("sheet.pdf")
        try Self.pdfData().write(to: pdf)
        editor.showLocalDatasheet(pdf)
        XCTAssertEqual(editor.datasheetPhase, .loaded("sheet.pdf"))
        XCTAssertEqual(editor.guidedPane, .datasheet)

        editor.guidedPane = .footprint
        editor.findInDatasheet(["Pin Configuration"])
        XCTAssertEqual(editor.guidedPane, .datasheet)
    }

    // MARK: The File menu's Part Editor items

    func testTheMenuItemsFollowTheFocusedEditor() async throws {
        // One window at a time: the probes' windows would sit on each other.
        let none = ViewProbe(VStack { PartEditorCommands(editor: nil) })
        XCTAssertTrue(none.has("New Part…", role: "AXButton"))
        XCTAssertFalse(none.has("Save Part As…"))
        none.close()

        let invalid = model(.new(kind: "generic"), choosing: nil)
        let rejected = ViewProbe(VStack { PartEditorCommands(editor: invalid) })
        XCTAssertFalse(ViewProbe.isEnabled(try rejected.element("Save Part As…", role: "AXButton")))
        XCTAssertFalse(ViewProbe.isEnabled(try rejected.element("Add to Library", role: "AXButton")))
        rejected.close()

        let valid = model(.copy(name: "SN74HC00N"), choosing: nil)
        _ = valid.validateNow()
        XCTAssertTrue(valid.isValid)
        let probe = ViewProbe(VStack { PartEditorCommands(editor: valid) })
        defer { probe.close() }
        XCTAssertTrue(ViewProbe.isEnabled(try probe.element("Save Part As…", role: "AXButton")), "\(probe.labels())")
        try probe.press("Save Part As…")
        // The button starts a task on the main actor: suspend so it runs.
        for _ in 0..<40 where panels == 0 { try await Task.sleep(for: .milliseconds(50)) }
        XCTAssertEqual(panels, 1)
        // Add to Library with a rejected draft shows why.
        valid.draft.pinouts[0].pins = []
        try probe.press("Add to Library")
        for _ in 0..<40 where alerts.isEmpty { try await Task.sleep(for: .milliseconds(50)) }
        XCTAssertEqual(alerts, ["Couldn't add the part"])
    }

    func testALibraryWindowHasNoAddToLibraryItem() throws {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        store.attachUserLibrary(UserPartLibrary(directory: directory))
        let uuid = "F2000000-0000-4000-8000-000000000003"
        var draft = try XCTUnwrap(PartEditorModel.copy(named: "SN74HC00N", catalog: store))
        draft.uuid = uuid
        draft.name = "PEM-C"
        try store.addToUserLibrary(fileText: draft.fileText(), source: "PEM-C.schpart")
        let probe = ViewProbe(VStack { PartEditorCommands(editor: model(.libraryPart(uuid: uuid), catalog: store)) })
        defer { probe.close() }
        XCTAssertTrue(probe.has("Save Part As…", role: "AXButton"))
        XCTAssertFalse(probe.has("Add to Library"))
    }

    func testNewPartOpensAWizardWindow() throws {
        let before = Set(NSApp.windows.map(ObjectIdentifier.init))
        let probe = ViewProbe(VStack { PartEditorCommands(editor: nil) })
        defer { probe.close() }
        try probe.press("New Part…")
        var opened: [NSWindow] = []
        for _ in 0..<40 {
            opened = NSApp.windows.filter { !before.contains(ObjectIdentifier($0)) && $0 !== probe.window && $0.isVisible }
            if !opened.isEmpty { break }
            probe.settle(0.05)
        }
        defer { opened.forEach { $0.close() } }
        XCTAssertEqual(opened.map(\.title), ["New Part"])
    }

    // MARK: Opening targets, and what the window shows

    func testTargetsThatCannotOpenSayWhy() {
        let missing = model(.copy(name: "NO-SUCH-PART-\(UUID().uuidString)"))
        XCTAssertEqual(missing.openError?.hasPrefix("There is no part named NO-SUCH-PART-"), true)
        XCTAssertEqual(missing.draft.kind, "gate")

        let unreadable = model(.file(file("absent.schpart")))
        XCTAssertNotNil(unreadable.openError)
        XCTAssertNil(unreadable.fileURL)
    }

    func testTheValidationMessageAndTheLibraryName() throws {
        let rejected = model(.new(kind: "generic"))
        _ = rejected.validateNow()
        XCTAssertNotNil(rejected.validationMessage)

        let copy = model(.copy(name: "SN74HC00N"))
        _ = copy.validateNow()
        XCTAssertNil(copy.validationMessage)
        // A copy keeps the library part's name, which another part has: Add
        // to Library suffixes it.
        XCTAssertEqual(copy.draft.name, "SN74HC00N")
        XCTAssertEqual(copy.libraryName, "SN74HC00N.2")
        copy.draft.name = "PEM-UNIQUE-\(UUID().uuidString.prefix(8))"
        XCTAssertEqual(copy.libraryName, copy.draft.name)
    }

    func testACopiedPartLoadsTheLibrarysDatasheet() throws {
        let part = try XCTUnwrap(PartCatalogStore.shared.part(named: "SN74HC00N"))
        XCTAssertNotNil(part.datasheetURL)
        let copy = model(.copy(name: part.name))
        XCTAssertNotNil(copy.draft.datasheet)
        XCTAssertNil(copy.draft.datasheetURL)
        guard case .success = copy.validateNow() else { return XCTFail(copy.validationMessage ?? "rejected") }
        XCTAssertEqual(copy.bundledDatasheetURL, part.datasheetURL)
        XCTAssertEqual(copy.datasheetAddress, part.datasheetURL)
    }
}
