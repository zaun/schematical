import AppKit
import PDFKit
import SwiftUI
import XCTest
@testable import schematical

/// The datasheet download (through a stub protocol, never the network) and
/// the Open Datasheet window (plan-68-Z).
@MainActor
final class DatasheetWindowTests: XCTestCase {
    nonisolated deinit {}

    /// Answers every request with `DatasheetWindowTests.answer`.
    final class Stub: URLProtocol {
        nonisolated(unsafe) static var answer: (status: Int, body: Data)?
        nonisolated(unsafe) static var agent: String?

        override class func canInit(with request: URLRequest) -> Bool { true }
        override class func canonicalRequest(for request: URLRequest) -> URLRequest { request }

        override func startLoading() {
            Self.agent = request.value(forHTTPHeaderField: "User-Agent")
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
    }

    private static let url = URL(string: "https://example.com/sheets/part.pdf")!
    /// Kept for the life of the process (see `TestSupport.newDocument()`).
    private static var kept: [AnyObject] = []
    private var windows: [NSWindow] = []

    /// These tests close windows; the app would quit after the last one.
    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        windows.forEach { $0.close() }
        windows = []
        Stub.answer = nil
    }

    private func session() -> URLSession {
        let configuration = URLSessionConfiguration.ephemeral
        configuration.protocolClasses = [Stub.self]
        return URLSession(configuration: configuration)
    }

    private func pdfData() throws -> Data {
        try XCTUnwrap(DatasheetFinderTests.pdf(headings: ["Features"]).dataRepresentation())
    }

    // MARK: Loader

    func testTheLoaderSendsABrowserAgentAndReadsThePDF() async throws {
        Stub.answer = (200, try pdfData())
        let result = await DatasheetLoader.load(Self.url, session: session())
        XCTAssertEqual(Stub.agent, DatasheetLoader.userAgent)
        guard case let .success(document) = result else { return XCTFail("\(result)") }
        XCTAssertEqual(document.pageCount, 1)
    }

    func testTheLoaderReportsAStatusAndANetworkFailure() async throws {
        Stub.answer = (404, Data("missing".utf8))
        let refused = await DatasheetLoader.load(Self.url, session: session())
        guard case .failure(.status(404)) = refused else { return XCTFail("\(refused)") }

        Stub.answer = nil
        let offline = await DatasheetLoader.load(Self.url, session: session())
        guard case let .failure(.network(text)) = offline else { return XCTFail("\(offline)") }
        XCTAssertFalse(text.isEmpty)
        XCTAssertEqual(DatasheetLoader.Failure.network("No network").message, "No network")
    }

    // MARK: Model

    private func model(_ answers: [Result<PDFDocument, DatasheetLoader.Failure>]) -> (DatasheetWindowModel, () -> Int) {
        var remaining = answers
        var calls = 0
        let target = DatasheetWindowTarget(url: Self.url, partName: "SN74HC00")
        let model = DatasheetWindowModel(target: target) { url in
            XCTAssertEqual(url, Self.url)
            calls += 1
            return remaining.isEmpty ? .failure(.notPDF) : remaining.removeFirst()
        }
        Self.kept.append(model)
        return (model, { calls })
    }

    func testTheModelGoesFromLoadingToLoadedOrFailed() async throws {
        let document = try DatasheetFinderTests.pdf(headings: ["One", "Two"])
        let (model, calls) = model([.failure(.status(500)), .success(document)])
        guard case .loading = model.phase else { return XCTFail("starts loading") }
        await model.load()
        guard case let .failed(message) = model.phase else { return XCTFail("\(model.phase)") }
        XCTAssertTrue(message.hasPrefix("The manufacturer's site answered 500"))
        await model.load()
        guard case let .loaded(shown) = model.phase else { return XCTFail("\(model.phase)") }
        XCTAssertTrue(shown === document)
        XCTAssertEqual(calls(), 2)
    }

    func testATargetIsTheSameWindowByURLOnly() {
        let a = DatasheetWindowTarget(url: Self.url, partName: "A")
        let b = DatasheetWindowTarget(url: Self.url, partName: "B")
        XCTAssertEqual(a, b)
        XCTAssertEqual(Set([a, b]).count, 1)
        XCTAssertNotEqual(a, DatasheetWindowTarget(url: URL(string: "https://example.com/other.pdf")!, partName: "A"))
    }

    // MARK: Window

    func testAFailureShowsTheMessageAndTryAgainLoadsAgain() throws {
        let document = try DatasheetFinderTests.pdf(headings: ["One"])
        let (model, calls) = model([.failure(.notPDF), .success(document)])
        let probe = ViewProbe(DatasheetWindowView(model: model))
        windows.append(probe.window)
        try waitFor { calls() == 1 }
        probe.settle()
        XCTAssertTrue(probe.has("Can't Open Datasheet"), "\(probe.labels())")
        XCTAssertTrue(probe.has("The manufacturer's site did not send a PDF."))
        XCTAssertTrue(probe.has("Open in Browser"), "\(probe.labels())")
        XCTAssertTrue(probe.views(PDFView.self).isEmpty)

        try probe.press("Try Again")
        try waitFor { calls() == 2 }
        probe.settle()
        let shown = try XCTUnwrap(probe.views(PDFView.self).first)
        XCTAssertTrue(shown.document === document)
        XCTAssertTrue(shown.autoScales)
        XCTAssertEqual(shown.displayMode, .singlePageContinuous)
    }

    func testThePDFViewFollowsANewDocumentAndTheFinder() throws {
        let first = try DatasheetFinderTests.pdf(headings: ["One"])
        let second = try DatasheetFinderTests.pdf(headings: ["Two"])
        let finder = DatasheetFinder()
        Self.kept.append(finder)
        let source = Source(first)
        Self.kept.append(source)
        let probe = ViewProbe(SwitchingPDF(source: source, finder: finder))
        windows.append(probe.window)
        let view = try XCTUnwrap(probe.views(PDFView.self).first)
        XCTAssertTrue(view.document === first)
        XCTAssertTrue(finder.view === view)
        source.document = second
        probe.settle()
        XCTAssertTrue(view.document === second)
        XCTAssertTrue(probe.views(PDFView.self).first === view)
    }

    @Observable
    final class Source {
        nonisolated deinit {}
        var document: PDFDocument
        init(_ document: PDFDocument) { self.document = document }
    }

    private struct SwitchingPDF: View {
        let source: Source
        let finder: DatasheetFinder
        var body: some View { PDFDocumentView(document: source.document, finder: finder) }
    }

    /// The button opens the app's datasheet window for its target; a file URL
    /// the test wrote stands in for the manufacturer's site.
    func testOpenDatasheetOpensTheWindow() throws {
        let file = FileManager.default.temporaryDirectory.appendingPathComponent("sheet-\(UUID().uuidString).pdf")
        try pdfData().write(to: file)
        defer { try? FileManager.default.removeItem(at: file) }
        let name = "Probe\(UInt32.random(in: 0...UInt32.max))"
        let probe = ViewProbe(OpenDatasheetButton(target: DatasheetWindowTarget(url: file, partName: name)),
                              size: NSSize(width: 240, height: 80))
        windows.append(probe.window)
        try probe.press("Open Datasheet")
        try waitFor { NSApp.windows.contains { $0.title == "\(name) Datasheet" } }
        let opened = try XCTUnwrap(NSApp.windows.first { $0.title == "\(name) Datasheet" })
        windows.append(opened)
        try waitFor { !ViewProbe(window: opened).views(PDFView.self).isEmpty }
    }

    struct TimedOut: Error {}

    /// Runs the main run loop until `condition` holds; fails after `timeout`.
    func waitFor(_ condition: () -> Bool, timeout: TimeInterval = 10) throws {
        let end = Date().addingTimeInterval(timeout)
        while !condition() {
            if Date() > end { throw TimedOut() }
            RunLoop.main.run(until: Date().addingTimeInterval(0.05))
        }
    }
}
