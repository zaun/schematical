import AppKit
import SwiftUI
import WebKit
import XCTest
@testable import schematical

/// The Markdown datasheet's web page, its messages and its window (plan-68-Z).
@MainActor
final class DatasheetPageTests: XCTestCase {
    nonisolated deinit {}

    private static var kept: [AnyObject] = []
    private var windows: [NSWindow] = []
    private static let target = MarkdownDatasheetTarget(path: "gate-and/sn74hc08.md", partName: "SN74HC08N")

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        for window in windows {
            if let sheet = window.attachedSheet { window.endSheet(sheet, returnCode: .cancel) }
            window.close()
        }
        windows = []
    }

    private func page(citations: Bool = false, bundle: Bundle = .main) -> DatasheetPage {
        let page = DatasheetPage(showsCitations: citations, bundle: bundle)
        Self.kept.append(page)
        return page
    }

    private func spin(until condition: () -> Bool, timeout: TimeInterval = 15) throws {
        let end = Date().addingTimeInterval(timeout)
        while !condition() {
            if Date() > end { throw DatasheetWindowTests.TimedOut() }
            RunLoop.main.run(until: Date().addingTimeInterval(0.05))
        }
    }

    /// The result of `script` in the page.
    private func evaluate(_ page: DatasheetPage, _ script: String) throws -> Any? {
        var result: Any?
        var done = false
        page.webView.evaluateJavaScript(script) { value, _ in
            result = value
            done = true
        }
        try spin { done }
        return result
    }

    /// A page with nothing in it but the message handlers, loaded.
    private func blank(citations: Bool) throws -> DatasheetPage {
        let page = page(citations: citations)
        page.webView.loadHTMLString("<p>blank</p>", baseURL: nil)
        try spin { !page.webView.isLoading && (try? evaluate(page, "document.body.innerText")) as? String == "blank" }
        return page
    }

    func testATargetIsTheSameWindowByPathOnly() {
        let other = MarkdownDatasheetTarget(path: Self.target.path, partName: "SN74LS08N")
        XCTAssertEqual(Self.target, other)
        XCTAssertEqual(Set([Self.target, other]).count, 1)
        XCTAssertNotEqual(Self.target, MarkdownDatasheetTarget(path: "gate-nand/sn74hc00.md", partName: "SN74HC08N"))
    }

    // MARK: Loading

    func testALoadedPageRendersTheDatasheet() throws {
        let page = page()
        page.load(Self.target)
        XCTAssertNil(page.failure)
        XCTAssertEqual(page.title, "SNx4HC08 quadruple 2-input AND gates")
        try spin { (try? evaluate(page, "document.getElementById('page').innerText.length")) as? Int ?? 0 > 100 }
        let text = try XCTUnwrap(evaluate(page, "document.getElementById('page').innerText") as? String)
        XCTAssertTrue(text.contains("AND"), text.prefix(200).description)
    }

    func testAMissingFileOrViewerIsAFailure() throws {
        let missing = page()
        missing.load(MarkdownDatasheetTarget(path: "gate-and/nope.md", partName: "X"))
        XCTAssertEqual(missing.failure, "The datasheet file gate-and/nope.md is missing from the app.")

        let noViewer = page(bundle: Bundle(for: Self.self))
        noViewer.load(Self.target)
        XCTAssertEqual(noViewer.failure, "The datasheet viewer is missing from the app.")
        XCTAssertEqual(noViewer.title, "SNx4HC08 quadruple 2-input AND gates")
    }

    /// A page that finishes loading with no payload is left alone; one whose
    /// render call fails shows the error.
    func testARenderErrorIsAFailure() throws {
        let idle = try blank(citations: false)
        XCTAssertNil(idle.failure)

        let page = page()
        page.load(Self.target)
        try spin { (try? evaluate(page, "document.getElementById('page').innerText.length")) as? Int ?? 0 > 100 }
        XCTAssertNil(page.failure)
        // Another page (without the renderer) finishes in its place: rendering it fails.
        page.webView.loadHTMLString("<p>no renderer</p>", baseURL: nil)
        try spin { page.failure != nil }
        XCTAssertFalse(page.failure?.isEmpty ?? true)
    }

    // MARK: Links

    final class Action: WKNavigationAction {
        nonisolated deinit {}
        let type: WKNavigationType
        let url: URL
        init(_ type: WKNavigationType, _ url: URL) {
            self.type = type
            self.url = url
            super.init()
        }
        override var navigationType: WKNavigationType { type }
        override var request: URLRequest { URLRequest(url: url) }
    }

    func testLinksOutOfThePageOpenInTheBrowser() {
        let page = page()
        var opened: [URL] = []
        page.openExternal = { opened.append($0) }
        var decisions: [WKNavigationActionPolicy] = []
        let web = URL(string: "https://www.ti.com/lit/ds/symlink/sn74hc08.pdf")!
        page.webView(page.webView, decidePolicyFor: Action(.linkActivated, web)) { decisions.append($0) }
        page.webView(page.webView, decidePolicyFor: Action(.linkActivated, URL(fileURLWithPath: "/tmp/a.html"))) {
            decisions.append($0)
        }
        page.webView(page.webView, decidePolicyFor: Action(.other, web)) { decisions.append($0) }
        XCTAssertEqual(decisions, [.cancel, .allow, .allow])
        XCTAssertEqual(opened, [web])
    }

    // MARK: Messages

    func testCitationClicksReachThePage() throws {
        let page = try blank(citations: true)
        var clicks: [CitationClick] = []
        page.onCitation = { clicks.append($0) }
        let post = "window.webkit.messageHandlers.cite.postMessage"
        _ = try evaluate(page, "\(post)({page: 3, ref: 'Table 6-3', text: 'The outputs'}); 1")
        _ = try evaluate(page, "\(post)({page: 2, ref: '2'}); 1")
        // Not a citation: no page, a page before the first, not an object.
        _ = try evaluate(page, "\(post)({ref: 'x'}); \(post)({page: 0, ref: 'x'}); \(post)('x'); 1")
        try spin { clicks.count >= 2 }
        _ = try evaluate(page, "1")
        XCTAssertEqual(clicks, [CitationClick(page: 3, reference: "Table 6-3", text: "The outputs"),
                                CitationClick(page: 2, reference: "2", text: "")])
    }

    func testEditMessagesReachThePage() throws {
        let page = try blank(citations: true)
        var states: [DatasheetEditState] = []
        var asked = 0
        page.onEdit = { states.append($0) }
        page.onPDFPageRequest = { asked += 1 }
        let post = "window.webkit.messageHandlers.edit.postMessage"
        _ = try evaluate(page, """
            \(post)({type: 'state', path: 'a.md', markdown: 'text', modified: true, edited: 2, undo: 1});
            \(post)({type: 'state', markdown: 'bare'});
            \(post)({type: 'state', path: 'no markdown'});
            \(post)({type: 'pdfPage'});
            \(post)({type: 'other'});
            \(post)({path: 'no type'});
            \(post)(7);
            1
            """)
        try spin { states.count >= 2 && asked >= 1 }
        XCTAssertEqual(states, [
            DatasheetEditState(path: "a.md", markdown: "text", modified: true, edited: 2, undo: 1),
            DatasheetEditState(path: "", markdown: "bare", modified: false, edited: 0, undo: 0),
        ])
        XCTAssertEqual(asked, 1)
    }

    /// The editor calls reach the page's `dsEdit` (a stand-in that reports
    /// each call back as an edit state).
    func testTheEditorCallsReachThePage() throws {
        let page = try blank(citations: true)
        var calls: [String] = []
        page.onEdit = { calls.append($0.markdown) }
        _ = try evaluate(page, """
            const say = (m) => window.webkit.messageHandlers.edit.postMessage({type: 'state', markdown: m});
            window.dsEdit = {setEditing: (on) => say('editing ' + on), undo: () => say('undo'),
                             revertAll: () => say('revert'), setPdfPage: (n) => say('page ' + n)};
            1
            """)
        page.setEditing(true)
        page.undo()
        page.revertAll()
        page.deliverPDFPage(4)
        try spin { calls.count == 4 }
        XCTAssertEqual(calls, ["editing true", "undo", "revert", "page 4"])
    }

    // MARK: The window

    private func openWindow(_ target: MarkdownDatasheetTarget) throws -> NSWindow {
        let probe = ViewProbe(ShowDatasheetButton(target: target), size: NSSize(width: 240, height: 80))
        windows.append(probe.window)
        try probe.press("Show Datasheet")
        let title = "\(target.partName) Datasheet"
        try spin { NSApp.windows.contains { $0.title == title && $0.isVisible } }
        let window = try XCTUnwrap(NSApp.windows.first { $0.title == title && $0.isVisible })
        windows.append(window)
        return window
    }

    func testTheWindowShowsTheDatasheetAndPrintsIt() throws {
        let window = try openWindow(MarkdownDatasheetTarget(path: Self.target.path, partName: "PZ\(UInt16.random(in: 0...9999))"))
        let probe = ViewProbe(window: window)
        try spin { !probe.views(WKWebView.self).isEmpty }
        XCTAssertEqual(window.subtitle, "SNx4HC08 quadruple 2-input AND gates")
        XCTAssertFalse(probe.has("Can't Show Datasheet"))
        let print = try probe.element("Print", role: "AXButton")
        XCTAssertTrue(ViewProbe.isEnabled(print))
        try probe.press("Print")
        try spin { window.attachedSheet != nil }
        let sheet = try XCTUnwrap(window.attachedSheet)
        window.endSheet(sheet, returnCode: .cancel)
        try spin { window.attachedSheet == nil }
    }

    func testTheWindowSaysWhenTheFileIsMissing() throws {
        let window = try openWindow(MarkdownDatasheetTarget(path: "gate-and/nope.md", partName: "PZ\(UInt16.random(in: 0...9999))"))
        let probe = ViewProbe(window: window)
        try spin { probe.has("Can't Show Datasheet") }
        XCTAssertTrue(probe.has("The datasheet file gate-and/nope.md is missing from the app."))
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Print", role: "AXButton")))
    }

    /// Printing with the page in no window does nothing.
    func testPrintingOutOfAWindowDoesNothing() {
        let page = page()
        page.print()
        XCTAssertNil(page.webView.window)
        XCTAssertNil(NSPrintOperation.current)
    }
}
