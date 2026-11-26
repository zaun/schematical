import AppKit
import PDFKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AE: the Part Wizard's Datasheet step and the datasheet pane as they
/// are shown, the address field's rule, and Open a downloaded PDF… with its
/// panel answered by the test. Downloads go through a stubbed session.
@MainActor
final class DatasheetStepPageTests: XCTestCase {
    private static var kept: [AnyObject] = []
    private var probes: [ViewProbe] = []
    private var directory: URL!
    private var chosen: URL?
    private var panels = 0

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("dsstep-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        chosen = nil
        panels = 0
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
        try? FileManager.default.removeItem(at: directory)
        PartEditorModelTests.Stub.answer = nil
    }

    private func model(_ source: PartEditorTarget.Source) -> PartEditorModel {
        let model = PartEditorModel(target: PartEditorTarget(source: source), catalog: .shared)
        Self.kept.append(model)
        model.presenter.choose = { [weak self] _, _ in
            self?.panels += 1
            return self?.chosen
        }
        let configuration = URLSessionConfiguration.ephemeral
        configuration.protocolClasses = [PartEditorModelTests.Stub.self]
        model.datasheetSession = URLSession(configuration: configuration)
        return model
    }

    private func show<V: View>(_ view: V) -> ViewProbe {
        let probe = ViewProbe(view, size: NSSize(width: 800, height: 700))
        probes.append(probe)
        return probe
    }

    private func page(_ model: PartEditorModel) -> ViewProbe {
        show(Form { DatasheetStepPage(model: model) }.formStyle(.grouped))
    }

    private func pdf(_ name: String) throws -> URL {
        let document = PDFDocument()
        document.insert(PDFPage(), at: 0)
        let url = directory.appendingPathComponent(name)
        try XCTUnwrap(document.dataRepresentation()).write(to: url)
        return url
    }

    private func wait(until done: () -> Bool) {
        let deadline = Date().addingTimeInterval(10)
        while !done(), Date() < deadline { RunLoop.main.run(until: Date().addingTimeInterval(0.05)) }
    }

    private static let prompt = "https://www.ti.com/lit/ds/symlink/sn74hc00.pdf"

    /// A new gate from a maker, so its datasheet is an address.
    private func makerModel() -> PartEditorModel {
        let model = model(.new(kind: "gate"))
        model.draft.manufacturer = "Texas Instruments"
        return model
    }

    // MARK: The step

    func testAPartWithNoMakerHasNoDatasheet() throws {
        let model = model(.new(kind: "gate"))
        model.draft.manufacturer = nil
        model.go(to: WizardStep.datasheet.id)
        let probe = page(model)
        XCTAssertTrue(probe.has(DatasheetURLRule.needsManufacturer), "\(probe.labels())")
        XCTAssertTrue(probe.has("The datasheet opens in the pane on the right"))
        try probe.press("Go to Part number")
        XCTAssertEqual(model.currentStepID, WizardStep.identity.id)
    }

    func testTheAddressFieldKeepsAnHTTPSAddressAndOffersOne() throws {
        let model = makerModel()
        let probe = page(model)
        XCTAssertTrue(probe.has("The manufacturer's PDF"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Only the address is saved in the part."))
        let load = try probe.element("Load", role: "AXButton")
        XCTAssertFalse(ViewProbe.isEnabled(load), "nothing to load yet")

        try probe.type("http://example.com/a.pdf", intoField: Self.prompt)
        XCTAssertNil(model.draft.datasheetURL, "an http address is not kept")
        XCTAssertTrue(probe.has("Datasheets are opened over https."), "\(probe.labels())")
        try probe.press("Use https://example.com/a.pdf")
        XCTAssertEqual(model.draft.datasheetURL, "https://example.com/a.pdf")

        try probe.type("ftp://example.com/a.pdf", intoField: Self.prompt)
        XCTAssertTrue(probe.has(DatasheetURLRule.needsHTTPS), "\(probe.labels())")
        XCTAssertEqual(model.draft.datasheetURL, "https://example.com/a.pdf", "a refused address leaves the kept one")

        try probe.type("", intoField: Self.prompt)
        XCTAssertNil(model.draft.datasheetURL, "an emptied field removes it")
    }

    func testCommittingTheAddress() {
        let maker = makerModel()
        maker.commitDatasheetAddress("https://example.com/b.pdf", replacing: false)
        XCTAssertEqual(maker.draft.datasheetURL, "https://example.com/b.pdf")
        let draft = maker.draft
        maker.commitDatasheetAddress(" https://example.com/b.pdf ", replacing: false)
        XCTAssertEqual(maker.draft, draft, "the same address changes nothing")
        maker.commitDatasheetAddress("http://example.com/b.pdf", replacing: false)
        XCTAssertEqual(maker.draft.datasheetURL, "https://example.com/b.pdf")
        maker.commitDatasheetAddress("", replacing: false)
        XCTAssertNil(maker.draft.datasheetURL)
        XCTAssertNil(maker.draft.datasheet)

        let library = model(.copy(name: "SN74HC00N"))
        let id = library.draft.datasheet
        XCTAssertNotNil(id)
        library.commitDatasheetAddress("", replacing: false)
        XCTAssertEqual(library.draft.datasheet, id, "an empty field leaves the library's datasheet")
    }

    func testTheLibrarysDatasheetLoadsOrIsReplaced() throws {
        let model = model(.copy(name: "SN74HC00N"))
        wait { model.bundledDatasheetURL != nil }
        let address = try XCTUnwrap(model.bundledDatasheetURL)
        let probe = page(model)
        XCTAssertTrue(probe.has("The library's datasheet"), "\(probe.labels())")
        XCTAssertTrue(probe.has(address.absoluteString))

        PartEditorModelTests.Stub.answer = (200, try Data(contentsOf: pdf("lib.pdf")))
        try probe.press("Load")
        wait { model.datasheetPhase != .none && model.datasheetPhase != .loading }
        XCTAssertEqual(model.datasheetPhase, .loaded(address.host() ?? ""))
        probe.settle()
        XCTAssertTrue(probe.has("Showing \(address.host() ?? "")"), "\(probe.labels())")

        try probe.press("Replace…")
        XCTAssertFalse(probe.has("The library's datasheet"))
        XCTAssertTrue(probe.has("The manufacturer's PDF"), "\(probe.labels())")
    }

    func testOpenADownloadedPDFShowsItOrNothing() throws {
        let model = makerModel()
        let probe = page(model)
        try probe.press("Open a downloaded PDF…")
        wait { panels == 1 }
        XCTAssertEqual(panels, 1)
        XCTAssertEqual(model.datasheetPhase, .none, "a cancelled panel shows nothing")

        chosen = try pdf("mine.pdf")
        try probe.press("Open a downloaded PDF…")
        wait { model.datasheetPhase != .none }
        XCTAssertEqual(panels, 2)
        XCTAssertEqual(model.datasheetPhase, .loaded("mine.pdf"))
        XCTAssertNil(model.draft.datasheetURL, "the part keeps no address for a local file")
    }

    func testTheStepShowsADownloadAndAFailure() throws {
        let model = makerModel()
        model.draft.setDatasheetURL("https://example.com/slow.pdf")
        let configuration = URLSessionConfiguration.ephemeral
        configuration.protocolClasses = [Hang.self]
        let session = URLSession(configuration: configuration)
        model.datasheetSession = session
        let probe = page(model)
        try probe.press("Load")
        XCTAssertEqual(model.datasheetPhase, .loading)
        XCTAssertTrue(probe.has("Downloading the datasheet…"), "\(probe.labels())")
        let pane = show(DatasheetPane(model: model))
        XCTAssertTrue(pane.has("Downloading…"), "\(pane.labels())")

        session.invalidateAndCancel()
        wait { model.datasheetPhase != .loading }
        guard case let .failed(message) = model.datasheetPhase else { return XCTFail("\(model.datasheetPhase)") }
        probe.settle()
        XCTAssertTrue(probe.has(message), "\(probe.labels())")
        XCTAssertTrue(probe.has("save the PDF in your browser"))
        pane.settle()
        XCTAssertTrue(pane.has(message), "\(pane.labels())")
    }

    // MARK: The pane

    func testThePaneShowsThePDFAndItsSearch() throws {
        let model = makerModel()
        let pane = show(DatasheetPane(model: model))
        XCTAssertTrue(pane.has("Give the address on the Datasheet step and press Load."), "\(pane.labels())")
        try pane.press("Go to Datasheet")
        XCTAssertEqual(model.currentStepID, WizardStep.datasheet.id)

        model.showLocalDatasheet(try pdf("pane.pdf"))
        pane.settle()
        XCTAssertFalse(pane.has("No datasheet"))
        XCTAssertFalse(pane.views(PDFView.self).isEmpty, "the PDF is shown")
        model.findInDatasheet(["No Such Heading"])
        pane.settle()
        XCTAssertTrue(pane.has("Not found: No Such Heading"), "\(pane.labels())")
    }

    /// A request that never answers until its session is cancelled.
    final class Hang: URLProtocol {
        override class func canInit(with request: URLRequest) -> Bool { true }
        override class func canonicalRequest(for request: URLRequest) -> URLRequest { request }
        override func startLoading() {}
        override func stopLoading() {
            client?.urlProtocol(self, didFailWithError: URLError(.cancelled))
        }
        nonisolated deinit {}
    }
}
