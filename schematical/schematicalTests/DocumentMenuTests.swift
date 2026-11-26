import AppKit
import SwiftUI
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// The File menu's document items (Close All, Open Recent, the actions sent), the
/// document's page and file-URL plumbing, the open-file offer, and the User
/// Library's alerts as sheets and as an app-modal alert (plan-68-Z).
@MainActor
final class DocumentMenuTests: XCTestCase {
    nonisolated deinit {}

    private static var kept: [AnyObject] = []
    private var windows: [NSWindow] = []

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        SchematicNSDocument.autosaves = false
        for window in windows {
            if let sheet = window.attachedSheet { window.endSheet(sheet) }
            window.close()
        }
        windows = []
    }

    private func makeWindow() -> NSWindow {
        let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 420, height: 300), styleMask: [.titled],
                              backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        window.orderFront(nil)
        windows.append(window)
        return window
    }

    private func spin(until condition: () -> Bool, timeout: TimeInterval = 10) throws {
        let end = Date().addingTimeInterval(timeout)
        while !condition() {
            if Date() > end { throw DatasheetWindowTests.TimedOut() }
            RunLoop.main.run(until: Date().addingTimeInterval(0.05))
        }
    }

    // MARK: The menu's actions

    /// Close All closes the open documents: here one untouched Untitled document.
    func testCloseAllClosesTheDocuments() throws {
        let controller = NSDocumentController.shared
        try XCTSkipUnless(controller.documents.allSatisfy { !$0.isDocumentEdited },
                          "an edited document is open: Close All would ask about it")
        let document = TestSupport.keep(SchematicNSDocument())
        controller.addDocument(document)
        XCTAssertTrue(controller.documents.contains(document))
        DocumentMenuCommands.closeAll()
        try spin { !controller.documents.contains(document) }
    }

    func testAnActionNothingAnswersIsNotTaken() {
        XCTAssertFalse(DocumentMenuCommands.send("plan68ZNoSuchAction:"))
    }

    func testOpenRecentListsTheFilesAndClears() throws {
        var calls: [String] = []
        let urls = [URL(fileURLWithPath: "/tmp/Alpha.schematical"), URL(fileURLWithPath: "/tmp/Beta.schematical")]
        let menu = RecentDocumentsMenu(urls: urls, open: { calls.append("open \($0.lastPathComponent)") },
                                       clear: { calls.append("clear") })
        let probe = ViewProbe(VStack { menu }, size: NSSize(width: 300, height: 200))
        windows.append(probe.window)
        XCTAssertTrue(probe.has("Alpha.schematical", role: "AXButton"), "\(probe.labels())")
        try probe.press("Beta.schematical")
        try probe.press("Clear Menu")
        XCTAssertEqual(calls, ["open Beta.schematical", "clear"])
    }

    // MARK: The document

    func testTheTestHostNeverAutosaves() {
        XCTAssertFalse(SchematicNSDocument.autosaves)
        let document = TestSupport.keep(SchematicNSDocument())
        SchematicNSDocument.autosaves = true
        // A document with nothing to autosave: AppKit schedules nothing and writes nothing.
        document.scheduleAutosaving()
        XCTAssertNil(document.autosavedContentsFileURL)
        XCTAssertFalse(document.hasUnautosavedChanges)
    }

    func testAFileURLSetOffTheMainThreadReachesTheModel() throws {
        let document = TestSupport.keep(SchematicNSDocument())
        let url = URL(fileURLWithPath: "/tmp/plan68Z-\(UUID().uuidString).schematical")
        let done = expectation(description: "set")
        DispatchQueue.global().async {
            document.fileURL = url
            done.fulfill()
        }
        wait(for: [done], timeout: 5)
        try spin { document.model.fileURL == url }
        XCTAssertEqual(document.fileURL, url)
    }

    /// A window that counts the times its saved state goes out of date.
    final class CountingWindow: NSWindow {
        nonisolated deinit {}
        var invalidated = 0
        override func invalidateRestorableState() {
            invalidated += 1
            super.invalidateRestorableState()
        }
    }

    func testAPageChangeInvalidatesEveryWindowsState() {
        let document = TestSupport.keep(SchematicNSDocument())
        let window = CountingWindow(contentRect: NSRect(x: 0, y: 0, width: 100, height: 100), styleMask: [.titled],
                                    backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        windows.append(window)
        let controller = NSWindowController(window: window)
        document.addWindowController(controller)
        defer { document.removeWindowController(controller) }
        let before = window.invalidated
        document.pageDidChange()
        XCTAssertEqual(window.invalidated, before + 1)
    }

    /// A file with a custom part the User Library lacks, read into a document
    /// that has a window: the offer comes up as a sheet on it.
    func testReadingAFileWithANewCustomPartOffersIt() throws {
        let folder = FileManager.default.temporaryDirectory.appendingPathComponent("plan68Z-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: folder) }
        let uuid = "C6800000-0000-4000-8000-\(String(format: "%012X", UInt64.random(in: 0...0xFFFF_FFFF_FFFF)))"
        let name = "PZ-\(uuid.suffix(6))"
        let object: [String: Any] = [
            "format": "schematical", "version": 5, "nextReference": 2, "junctions": [], "wires": [],
            "customParts": [try JSONSerialization.jsonObject(with: Data(PartRegistrationTests.fileText(uuid: uuid, name: name).utf8))],
            "parts": [["id": UUID().uuidString, "reference": "U1", "partNumber": name, "partUUID": uuid,
                       "x": 200, "y": 120, "flipped": false] as [String: Any]],
        ]
        let url = folder.appendingPathComponent("offer.schematical")
        try JSONSerialization.data(withJSONObject: object).write(to: url)

        let document = TestSupport.keep(SchematicNSDocument())
        let window = makeWindow()
        let controller = NSWindowController(window: window)
        document.addWindowController(controller)
        defer { document.removeWindowController(controller) }
        document.model.file.window = window
        try document.read(from: url, ofType: SchematicFile.contentType.identifier)
        XCTAssertEqual(document.model.document.chips.map(\.part.name), [name])
        try spin { window.attachedSheet != nil }
        let alert = try XCTUnwrap(window.attachedSheet)
        let texts = Self.texts(in: alert)
        XCTAssertTrue(texts.contains { $0.contains("custom parts that are not in your User Library") }, "\(texts)")
        XCTAssertTrue(texts.contains { $0.contains(name) }, "\(texts)")
        window.endSheet(alert, returnCode: .alertSecondButtonReturn)
        try spin { window.attachedSheet == nil }
        XCTAssertNotEqual(PartCatalogStore.shared.listing(of: document.model.document.chips[0].part), .user)
    }

    // MARK: User Library alerts

    func testRemovingSeveralPartsSaysHowMany() {
        let alert = UserLibraryPrompts.removeAlert(count: 3)
        XCTAssertEqual(alert.messageText, "Remove 3 parts from your User Library?")
        XCTAssertEqual(alert.informativeText, "Schematics that use them keep their own copy.")
        XCTAssertEqual(alert.buttons.map(\.title), ["Remove", "Cancel"])
    }

    func testAnAlertOnAWindowIsASheetThatAnswers() throws {
        let window = makeWindow()
        let alert = UserLibraryPrompts.removeAlert(count: 2)
        var answer: NSApplication.ModalResponse?
        Task { answer = await UserLibraryPrompts.run(alert, on: window) }
        try spin { window.attachedSheet != nil }
        XCTAssertTrue(window.attachedSheet === alert.window)
        window.endSheet(alert.window, returnCode: .alertSecondButtonReturn)
        try spin { answer != nil }
        XCTAssertEqual(answer, .alertSecondButtonReturn)
    }

    /// No window and no key window: the alert runs app-modal; the test
    /// stops it from inside the modal loop.
    func testAnAlertWithNoWindowRunsModal() async throws {
        let key = NSApp.keyWindow
        try XCTSkipIf(key != nil, "a key window is up: the alert would be a sheet on it")
        let alert = UserLibraryPrompts.alreadyInLibraryAlert(name: "PZ")
        RunLoop.main.perform(inModes: [.modalPanel]) {
            NSApp.stopModal(withCode: .alertFirstButtonReturn)
        }
        let answer = await UserLibraryPrompts.run(alert)
        XCTAssertEqual(answer, .alertFirstButtonReturn)
        XCTAssertFalse(alert.window.isVisible)
    }

    func testFailuresToAddAreShownAndNoFailureShowsNothing() throws {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        let folder = FileManager.default.temporaryDirectory.appendingPathComponent("plan68Z-lib-\(UUID().uuidString)")
        store.attachUserLibrary(UserPartLibrary(directory: folder))
        defer { try? FileManager.default.removeItem(at: folder) }
        let window = makeWindow()

        // A bundled part cannot be added: the failures sheet names it.
        let bundled = try XCTUnwrap(store.part(named: "SN74HC00N"))
        var done = false
        Task {
            await UserLibraryPrompts.addShowingFailures([bundled], catalog: store, on: window)
            done = true
        }
        try spin { window.attachedSheet != nil }
        let sheet = try XCTUnwrap(window.attachedSheet)
        let texts = Self.texts(in: sheet)
        XCTAssertTrue(texts.contains("Couldn't add to your User Library"), "\(texts)")
        XCTAssertTrue(texts.contains { $0.hasPrefix("SN74HC00N: ") }, "\(texts)")
        window.endSheet(sheet, returnCode: .alertFirstButtonReturn)
        try spin { done }

        let custom = try store.register(fileText: PartRegistrationTests.fileText(
            uuid: "C6800000-0000-4000-8000-0000000000A1", name: "PZ-ADD"),
                                        source: "pz")
        done = false
        Task {
            await UserLibraryPrompts.addShowingFailures([custom], catalog: store, on: window)
            done = true
        }
        try spin { done }
        XCTAssertNil(window.attachedSheet)
        XCTAssertEqual(store.listing(of: custom), .user)
    }

    /// The strings of every text field in `window`'s views (an `NSAlert`'s
    /// message and informative text).
    static func texts(in window: NSWindow) -> [String] {
        var found: [String] = []
        func walk(_ view: NSView) {
            if let field = view as? NSTextField { found.append(field.stringValue) }
            view.subviews.forEach(walk)
        }
        if let content = window.contentView { walk(content) }
        return found
    }
}
