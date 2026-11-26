import AppKit
import PDFKit
import SwiftUI
import WebKit
import XCTest
@testable import schematical

/// Window › Verify Datasheet: its model (search, the PDF, citations, edits),
/// its window, and the Option-only menu items (plan-68-Z). The PDF comes
/// from a stub, never the network.
@MainActor
final class VerifyDatasheetTests: XCTestCase {
    nonisolated deinit {}

    private static var kept: [AnyObject] = []
    private var windows: [NSWindow] = []

    /// These tests close windows; the app would quit after the last one.
    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        windows.forEach { $0.close() }
        windows = []
    }

    /// A model whose PDF fetches answer from `answer` (by call), counted.
    private final class Fetcher {
        nonisolated deinit {}
        var calls: [URL] = []
        var answer: (URL) -> Result<PDFDocument, DatasheetLoader.Failure>
        /// When set, a fetch waits until it is opened.
        var gate = false
        init(_ answer: @escaping (URL) -> Result<PDFDocument, DatasheetLoader.Failure>) { self.answer = answer }
    }

    private func model(_ fetcher: Fetcher) -> VerifyDatasheetModel {
        let model = VerifyDatasheetModel(catalog: .shared) { url in
            fetcher.calls.append(url)
            while fetcher.gate { try? await Task.sleep(nanoseconds: 20_000_000) }
            return fetcher.answer(url)
        }
        Self.kept.append(model)
        Self.kept.append(fetcher)
        return model
    }

    private func spin(until condition: () -> Bool, timeout: TimeInterval = 15) throws {
        let end = Date().addingTimeInterval(timeout)
        while !condition() {
            if Date() > end { throw DatasheetWindowTests.TimedOut() }
            RunLoop.main.run(until: Date().addingTimeInterval(0.05))
        }
    }

    private func evaluate(_ web: WKWebView, _ script: String) throws {
        var done = false
        web.evaluateJavaScript(script) { _, _ in done = true }
        try spin { done }
    }

    private func part(_ name: String) throws -> CatalogPart { try TestSupport.part(name) }

    private func pdf(_ headings: [String]) throws -> PDFDocument { try DatasheetFinderTests.pdf(headings: headings) }

    private var isLoaded: (VerifyDatasheetModel) -> Bool {
        { if case .loaded = $0.pdf { true } else { false } }
    }

    // MARK: Search

    func testMatchesEveryWordNamesStartingFirst() throws {
        let model = model(Fetcher { _ in .failure(.notPDF) })
        XCTAssertFalse(model.candidates.isEmpty)
        XCTAssertTrue(model.candidates.allSatisfy { $0.datasheetMarkdown != nil })
        XCTAssertEqual(model.matches("", limit: 3).map(\.name), Array(model.candidates.prefix(3)).map(\.name))
        let found = model.matches("sn74hc08 texas")
        XCTAssertTrue(found.first?.name.hasPrefix("SN74HC08") ?? false, "\(found.map(\.name))")
        XCTAssertTrue(found.allSatisfy { $0.name.lowercased().hasPrefix("sn74hc08") })
        // "and gate" matches by description; names starting with the first word come first.
        let gates = model.matches("and")
        if let firstOther = gates.firstIndex(where: { !$0.name.lowercased().hasPrefix("and") }) {
            XCTAssertTrue(gates[firstOther...].allSatisfy { !$0.name.lowercased().hasPrefix("and") })
        }
        XCTAssertEqual(model.matches("no-such-part-anywhere"), [])
    }

    func testTheFieldOpensChoosesClearsAndHides() throws {
        let model = model(Fetcher { _ in .failure(.notPDF) })
        model.query = "SN74HC08"
        model.queryChanged()
        XCTAssertTrue(model.showsResults)
        let first = try XCTUnwrap(model.matches("SN74HC08").first)
        model.submit()
        XCTAssertEqual(model.selected?.name, first.name)
        XCTAssertEqual(model.query, first.name)
        XCTAssertFalse(model.showsResults)
        // The field now names the part shown: the list stays closed.
        model.queryChanged()
        XCTAssertFalse(model.showsResults)
        model.clearQuery()
        XCTAssertEqual(model.query, "")
        XCTAssertTrue(model.showsResults)
        model.hideResults()
        XCTAssertFalse(model.showsResults)
        // Nothing matches: Return does nothing.
        model.query = "no-such-part-anywhere"
        model.submit()
        XCTAssertEqual(model.selected?.name, first.name)
    }

    func testTexts() throws {
        let part = try part("SN74HC08N")
        XCTAssertEqual(VerifyDatasheetModel.detail(of: part), "\(part.manufacturer) · \(part.description)")
        XCTAssertEqual(VerifyDatasheetModel.editedText(1), "1 block edited")
        XCTAssertEqual(VerifyDatasheetModel.editedText(3), "3 blocks edited")
        let model = model(Fetcher { _ in .failure(.notPDF) })
        XCTAssertEqual(model.subtitle, "")
        model.choose(part)
        XCTAssertEqual(model.subtitle, "SN74HC08N — \(model.page.title)")
        XCTAssertFalse(model.page.title.isEmpty)
    }

    // MARK: The PDF

    func testThePDFLoadsOnceAndIsCached() throws {
        let document = try pdf(["One"])
        let fetcher = Fetcher { _ in .success(document) }
        let model = model(fetcher)
        let part = try part("SN74HC08N")
        model.select(part)
        guard case .loading = model.pdf else { return XCTFail("\(model.pdf)") }
        try spin { isLoaded(model) }
        model.select(try self.part("SN74HC00N"))
        try spin { fetcher.calls.count == 2 }
        model.select(part)
        guard case let .loaded(shown) = model.pdf else { return XCTFail("\(model.pdf)") }
        XCTAssertTrue(shown === document)
        XCTAssertEqual(fetcher.calls.count, 2)
    }

    func testAFailureAndAPartWithNoAddress() throws {
        let model = model(Fetcher { _ in .failure(.status(404)) })
        model.select(try part("SN74HC08N"))
        try spin { if case .failed = model.pdf { true } else { false } }
        guard case let .failed(message) = model.pdf else { return XCTFail() }
        XCTAssertTrue(message.hasPrefix("The manufacturer's site answered 404"))

        let bare = try XCTUnwrap(PartCatalogStore.shared.allParts.first { $0.datasheetURL == nil })
        model.loadPDF(for: bare)
        guard case let .failed(none) = model.pdf else { return XCTFail() }
        XCTAssertEqual(none, "\(bare.name) has no datasheet address in the catalog.")
    }

    /// An answer for a part no longer shown is dropped.
    func testALateAnswerIsDropped() throws {
        let late = try pdf(["Late"])
        let fetcher = Fetcher { _ in .success(late) }
        fetcher.gate = true
        let model = model(fetcher)
        let first = try part("SN74HC08N"), second = try part("SN74HC00N")
        model.select(first)
        try spin { fetcher.calls.count == 1 }
        // The second part's fetch is cancelled-and-replaced; the first one's answer arrives after.
        let other = try pdf(["Other"])
        model.select(second)
        try spin { fetcher.calls.count == 2 }
        fetcher.answer = { $0 == second.datasheetURL ? .success(other) : .success(late) }
        fetcher.gate = false
        try spin { isLoaded(model) }
        RunLoop.main.run(until: Date().addingTimeInterval(0.3))
        guard case let .loaded(shown) = model.pdf else { return XCTFail() }
        XCTAssertTrue(shown === other)
    }

    // MARK: Citations

    func testACitationNeedsTheLoadedPDF() throws {
        let model = model(Fetcher { _ in .failure(.notPDF) })
        model.show(CitationClick(page: 1, reference: "1", text: ""))
        XCTAssertEqual(model.status, "The PDF is not loaded yet.")
        XCTAssertNil(model.currentPDFPage())
    }

    func testACitationScrollsThePDF() throws {
        let document = try pdf(["Features", "Table 6-3 Electrical characteristics", "Pin Functions"])
        let model = model(Fetcher { _ in .success(document) })
        model.select(try part("SN74HC08N"))
        try spin { isLoaded(model) }
        let view = PDFView(frame: NSRect(x: 0, y: 0, width: 300, height: 400))
        view.document = document
        model.finder.view = view
        XCTAssertEqual(model.currentPDFPage(), 1)

        model.show(CitationClick(page: 2, reference: "Table 6-3", text: ""))
        XCTAssertEqual(model.status, "Page 2, Table 6-3")
        XCTAssertEqual(view.currentSelection?.string, "Table 6-3")
        XCTAssertEqual(model.currentPDFPage(), 2)

        model.show(CitationClick(page: 3, reference: "9", text: "Words that are nowhere on the page at all"))
        XCTAssertEqual(model.status, "Page 3, 9: spot not found, showing the page")
        XCTAssertNil(view.currentSelection)
        XCTAssertEqual(model.currentPDFPage(), 3)

        // A paragraph number: the spot is the paragraph's text.
        model.show(CitationClick(page: 3, reference: "4", text: "Pin Functions"))
        XCTAssertEqual(model.status, "Page 3, 4")
        XCTAssertEqual(view.currentSelection?.string, "Pin Functions")

        model.show(CitationClick(page: 7, reference: "1", text: ""))
        XCTAssertEqual(model.status, "The PDF has no page 7 (it has 3).")
    }

    // MARK: Edits

    /// The page's citation clicks, edit reports and PDF-page requests reach
    /// the model; its edit calls reach the page (a stand-in `dsEdit`).
    func testThePageAndTheModelTalk() throws {
        let document = try pdf(["One", "Two"])
        let model = model(Fetcher { _ in .success(document) })
        let part = try part("SN74HC08N")
        model.select(part)
        try spin { isLoaded(model) }
        let view = PDFView(frame: NSRect(x: 0, y: 0, width: 300, height: 400))
        view.document = document
        view.go(to: try XCTUnwrap(document.page(at: 1)))
        model.finder.view = view
        // The page in its place: no renderer, the handlers and a dsEdit that echoes.
        let web = model.page.webView
        web.loadHTMLString("<p>stand-in</p>", baseURL: nil)
        try spin { !web.isLoading }
        RunLoop.main.run(until: Date().addingTimeInterval(0.2))
        let path = try XCTUnwrap(part.datasheetMarkdown)
        try evaluate(web, """
            window.calls = [];
            const edit = (m) => window.webkit.messageHandlers.edit.postMessage(m);
            window.dsEdit = {setEditing: (on) => window.calls.push('editing ' + on), undo: () => window.calls.push('undo'),
                             revertAll: () => window.calls.push('revert'), setPdfPage: (n) => window.calls.push('page ' + n)};
            1
            """)
        model.setEditing(true)
        XCTAssertTrue(model.isEditing)
        model.undo()
        model.revertAll()
        try evaluate(web, "window.webkit.messageHandlers.edit.postMessage({type: 'pdfPage'}); 1")
        try evaluate(web, """
            window.webkit.messageHandlers.edit.postMessage({type: 'state', path: '\(path)', markdown: 'changed',
                                                            modified: true, edited: 2, undo: 1});
            window.webkit.messageHandlers.edit.postMessage({type: 'state', path: 'other.md', markdown: 'x',
                                                            modified: true, edited: 9, undo: 9});
            window.webkit.messageHandlers.cite.postMessage({page: 2, ref: 'Two', text: ''});
            1
            """)
        try spin { model.editedBlocks == 2 && model.status.hasPrefix("Page 2") }
        XCTAssertEqual(model.edits, [path: "changed"])
        XCTAssertEqual(model.undoDepth, 1)
        var calls: [String] = []
        try spin {
            var value: [String]?
            web.evaluateJavaScript("window.calls") { result, _ in value = result as? [String] }
            RunLoop.main.run(until: Date().addingTimeInterval(0.1))
            calls = value ?? calls
            return calls.count >= 4
        }
        XCTAssertEqual(calls, ["editing true", "undo", "revert", "page 2"])

        // Back to the file: the edit is dropped.
        try evaluate(web, """
            window.webkit.messageHandlers.edit.postMessage({type: 'state', path: '\(path)', markdown: 'x',
                                                            modified: false, edited: 0, undo: 2});
            1
            """)
        try spin { model.undoDepth == 2 }
        XCTAssertEqual(model.edits, [:])
    }

    // MARK: The window

    private func host(_ model: VerifyDatasheetModel) -> ViewProbe {
        let probe = ViewProbe(VerifyDatasheetWindowView(model: model), size: NSSize(width: 1000, height: 620))
        windows.append(probe.window)
        return probe
    }

    func testTheWindowPicksAPartAndShowsBothPanes() throws {
        let document = try pdf(["One"])
        let model = model(Fetcher { _ in .success(document) })
        let probe = host(model)
        XCTAssertTrue(probe.has("Pick a Part"), "\(probe.labels())")
        XCTAssertTrue(probe.has("\(model.candidates.count.formatted()) parts"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Edit", role: "AXCheckBox"))

        model.query = "SN74HC08"
        model.queryChanged()
        probe.settle()
        let part = try XCTUnwrap(model.matches("SN74HC08").first)
        try probe.press(part.name)
        XCTAssertEqual(model.selected?.name, part.name)
        try spin { isLoaded(model) }
        probe.settle()
        XCTAssertFalse(probe.has("Pick a Part"))
        XCTAssertFalse(probe.views(WKWebView.self).isEmpty)
        XCTAssertTrue(probe.views(PDFView.self).first?.document === document)
        XCTAssertTrue(model.finder.view === probe.views(PDFView.self).first)

        // The clear button empties the field and opens the list.
        try probe.press("Clear")
        XCTAssertEqual(model.query, "")
        XCTAssertTrue(model.showsResults)
        model.query = "no-such-part-anywhere"
        probe.settle()
        XCTAssertTrue(probe.has("No part with a converted datasheet matches."), "\(probe.labels())")
    }

    func testTheEditControls() throws {
        let fetcher = Fetcher { _ in .failure(.notPDF) }
        let model = model(fetcher)
        let part = try part("SN74HC08N")
        model.choose(part)
        let probe = host(model)
        let toggle = try XCTUnwrap(probe.find("Edit").first { ViewProbe.role($0) != "AXStaticText" }, "\(probe.labels())")
        XCTAssertFalse(probe.has("Revert All"))
        _ = toggle.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        XCTAssertTrue(model.isEditing)
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Undo", role: "AXButton")))
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Revert All", role: "AXButton")))
        XCTAssertFalse(probe.has("edited"))

        let web = model.page.webView
        try spin { !web.isLoading }
        let path = try XCTUnwrap(part.datasheetMarkdown)
        try evaluate(web, """
            window.webkit.messageHandlers.edit.postMessage({type: 'state', path: '\(path)', markdown: 'changed',
                                                            modified: true, edited: 3, undo: 2});
            1
            """)
        try spin { model.editedBlocks == 3 }
        probe.settle()
        XCTAssertTrue(probe.has("3 blocks edited"), "\(probe.labels())")
        XCTAssertTrue(ViewProbe.isEnabled(try probe.element("Undo", role: "AXButton")))
        try probe.press("Undo")
        try probe.press("Revert All")
        // The PDF failed: its pane says so, with Try Again.
        XCTAssertTrue(probe.has("Can't Open the PDF"), "\(probe.labels())")
        XCTAssertTrue(probe.has("The manufacturer's site did not send a PDF."))
        XCTAssertTrue(probe.has("Open in Browser"))
        let fetched = fetcher.calls.count
        try probe.press("Try Again")
        try spin { fetcher.calls.count == fetched + 1 }
    }

    func testTheDatasheetPaneShowsARenderFailure() throws {
        let model = model(Fetcher { _ in .failure(.notPDF) })
        model.choose(try part("SN74HC08N"))
        let probe = host(model)
        // A page without the renderer finishes in its place: the render call fails.
        model.page.webView.loadHTMLString("<p>no renderer</p>", baseURL: nil)
        try spin { model.page.failure != nil }
        probe.settle()
        XCTAssertTrue(probe.has("Can't Show Datasheet"), "\(probe.labels())")
    }

    // MARK: Option-only menu items

    private func items() throws -> (verify: NSMenuItem?, duplicate: NSMenuItem, saveAs: NSMenuItem) {
        let file = try XCTUnwrap(NSApp.mainMenu?.items.first { $0.title == "File" }?.submenu)
        let duplicate = try XCTUnwrap(file.items.first { $0.title == DocumentMenuCommands.duplicateTitle })
        let saveAs = try XCTUnwrap(file.items.first { $0.title == DocumentMenuCommands.saveAsTitle })
        let verify = NSApp.windowsMenu?.items.first { $0.title == VerifyDatasheetScene.title }
        return (verify, duplicate, saveAs)
    }

    func testOptionShowsSaveAsAndVerifyDatasheet() throws {
        defer { OptionOnlyMenuItems.apply(optionDown: false) }
        OptionOnlyMenuItems.apply(optionDown: true)
        var (verify, duplicate, saveAs) = try items()
        XCTAssertTrue(duplicate.isHidden)
        XCTAssertFalse(saveAs.isHidden)
        XCTAssertTrue(saveAs.allowsKeyEquivalentWhenHidden)
        if let verify { XCTAssertFalse(verify.isHidden) }
        OptionOnlyMenuItems.apply(optionDown: false)
        (verify, duplicate, saveAs) = try items()
        XCTAssertFalse(duplicate.isHidden)
        XCTAssertTrue(saveAs.isHidden)
        if let verify { XCTAssertTrue(verify.isHidden) }
    }

    /// Tracking a menu follows the Option key; an item added to the Window
    /// menu starts hidden. (The notifications are posted by the test.)
    func testTheMenuNotificationsApplyTheState() throws {
        OptionOnlyMenuItems.install()
        OptionOnlyMenuItems.install()
        let center = NotificationCenter.default
        OptionOnlyMenuItems.apply(optionDown: true)
        center.post(name: NSMenu.didBeginTrackingNotification, object: NSApp.mainMenu)
        // The Option key is up: the timer keeps Save As hidden.
        RunLoop.main.run(until: Date().addingTimeInterval(0.2))
        XCTAssertTrue(try items().saveAs.isHidden)
        OptionOnlyMenuItems.apply(optionDown: true)
        RunLoop.main.run(until: Date().addingTimeInterval(0.2))
        XCTAssertTrue(try items().saveAs.isHidden)
        // A second begin while following changes nothing.
        center.post(name: NSMenu.didBeginTrackingNotification, object: NSApp.mainMenu)
        center.post(name: NSMenu.didEndTrackingNotification, object: NSApp.mainMenu)
        OptionOnlyMenuItems.apply(optionDown: true)
        RunLoop.main.run(until: Date().addingTimeInterval(0.2))
        // Stopped: nothing re-applies until an item is added.
        XCTAssertFalse(try items().saveAs.isHidden)
        let other = NSMenu(title: "Other")
        center.post(name: NSMenu.didAddItemNotification, object: other)
        center.post(name: NSMenu.didAddItemNotification, object: nil)
        RunLoop.main.run(until: Date().addingTimeInterval(0.1))
        XCTAssertFalse(try items().saveAs.isHidden)
        let file = try XCTUnwrap(NSApp.mainMenu?.items.first { $0.title == "File" }?.submenu)
        center.post(name: NSMenu.didAddItemNotification, object: file)
        try spin { (try? items().saveAs.isHidden) == true }
        OptionOnlyMenuItems.apply(optionDown: true)
        if let windows = NSApp.windowsMenu {
            center.post(name: NSMenu.didAddItemNotification, object: windows)
            try spin { (try? items().saveAs.isHidden) == true }
        }
    }
}

// MARK: - Where a citation points (the cases `CitationLocatorTests` leaves)

extension VerifyDatasheetTests {
    /// A label only mentioned in running text, twice: the first mention.
    func testALabelOnlyInRunningTextIsItsFirstMention() throws {
        let text = "Limits (see Figure 2-1) and more (Figure 2-1 again).\nOther text"
        let range = try XCTUnwrap(CitationLocator.labelRange("Figure 2-1", in: CitationLocator.Folded(text), text: text))
        XCTAssertEqual(range.location, (text as NSString).range(of: "Figure").location)
        XCTAssertEqual((text as NSString).substring(with: range), "Figure 2-1")
    }

    /// A paragraph of one or two words is looked for whole when long enough.
    func testAShortParagraphIsFoundWholeOrNotAtAll() throws {
        let text = "8.1 Overview\nFour NAND gates."
        let folded = CitationLocator.Folded(text)
        let range = try XCTUnwrap(CitationLocator.textRange("NAND gates", in: folded))
        XCTAssertEqual((text as NSString).substring(with: range), "NAND gates")
        XCTAssertNil(CitationLocator.textRange("Four", in: folded), "under six letters")
        XCTAssertNil(CitationLocator.textRange("Missing words", in: folded))
    }

    func testThePDFPaneShowsTheDownload() throws {
        let fetcher = Fetcher { _ in .failure(.notPDF) }
        fetcher.gate = true
        let model = model(fetcher)
        let probe = host(model)
        model.choose(try part("SN74HC08N"))
        probe.settle()
        XCTAssertTrue(probe.has("Downloading datasheet…"), "\(probe.labels())")
        fetcher.gate = false
        try spin { if case .failed = model.pdf { true } else { false } }
    }
}
