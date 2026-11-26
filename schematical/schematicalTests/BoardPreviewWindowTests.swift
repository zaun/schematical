import AppKit
import SwiftUI
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// The Board Preview window: its view, its toolbar and Save Image… (plan-68-Z).
/// The save panel and the error alert are answered from inside their modal loop.
@MainActor
final class BoardPreviewWindowTests: XCTestCase {
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

    private func model(_ doc: SchematicDocument) -> BoardPreviewModel {
        let model = BoardPreviewModel(document: doc)
        Self.kept.append(model)
        return model
    }

    /// `view` in a window whose toolbar shows the view's, as a child window's does.
    private func host<V: View>(_ view: V) -> ViewProbe {
        let controller = NSHostingController(rootView: view)
        controller.sceneBridgingOptions = [.title, .toolbars]
        let window = NSWindow(contentViewController: controller)
        window.setContentSize(NSSize(width: 700, height: 500))
        window.isReleasedWhenClosed = false
        windows.append(window)
        return ViewProbe(window: window)
    }

    private func board() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 30, height: 20)
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        doc.moveFootprint(chip.id, to: CGPoint(x: 15, y: 10))
        return doc
    }

    func testAClosedSchematicSaysSo() {
        let probe = host(BoardPreviewWindowView(target: BoardPreviewTarget(documentID: UUID())))
        XCTAssertTrue(probe.has("Schematic Closed"), "\(probe.labels())")
        XCTAssertTrue(probe.has("The board this preview belongs to is no longer open."))
    }

    func testAnOpenSchematicsBoardIsShown() throws {
        let doc = try board()
        let system = TestSupport.openSystemDocument(of: doc, catalog: TestSupport.catalog, until: self)
        let probe = host(BoardPreviewWindowView(target: BoardPreviewTarget(documentID: system.model.id)))
        XCTAssertFalse(probe.has("Schematic Closed"))
        XCTAssertTrue(probe.has("Save Image…"), "\(probe.labels())")
    }

    func testTheToolbarPicksTheViewAndSaves() throws {
        let model = model(try board())
        let probe = host(BoardPreviewView(model: model))
        XCTAssertTrue(probe.has("Front"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Back"), "\(probe.labels())")
        // The canvas reports its laid-out size.
        XCTAssertNotEqual(model.size, CGSize(width: 900, height: 640))
        XCTAssertGreaterThan(model.size.width, 400)

        var asked: [String] = []
        model.chooseURL = { asked.append($0); return nil }
        model.face = .back
        try probe.press("Save Image…")
        XCTAssertEqual(asked, ["Board Back.png"])
    }

    func testSaveImageWritesAPNG() throws {
        let model = model(try board())
        model.size = CGSize(width: 120, height: 80)
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("board-\(UUID().uuidString).png")
        defer { try? FileManager.default.removeItem(at: url) }
        model.chooseURL = { _ in url }
        model.showError = { XCTFail("\($0)") }
        model.saveImage()
        let image = try XCTUnwrap(NSImage(contentsOf: url))
        let rep = try XCTUnwrap(image.representations.first)
        XCTAssertEqual(rep.pixelsWide, 240, "twice the view's size")
        XCTAssertEqual(rep.pixelsHigh, 160)
    }

    func testAFailedWriteIsShown() throws {
        let model = model(try board())
        model.size = CGSize(width: 40, height: 30)
        model.chooseURL = { _ in URL(fileURLWithPath: "/nonexistent-\(UUID().uuidString)/board.png") }
        var errors: [Error] = []
        model.showError = { errors.append($0) }
        model.saveImage()
        XCTAssertEqual(errors.count, 1)
    }

    func testNothingToDrawSavesNothing() throws {
        let model = model(try board())
        model.size = .zero
        XCTAssertNil(model.pngData())
        var asked = 0
        model.chooseURL = { _ in asked += 1; return nil }
        model.saveImage()
        XCTAssertEqual(asked, 0)
    }

    /// The app's panel and alert, each stopped from inside its modal loop.
    func testThePanelAndTheAlertRunModal() throws {
        try XCTSkipIf(NSApp.modalWindow != nil, "a modal session is already running")
        var panel: NSSavePanel?
        RunLoop.main.perform(inModes: [.modalPanel]) {
            panel = NSApp.modalWindow as? NSSavePanel
            NSApp.stopModal(withCode: .cancel)
        }
        XCTAssertNil(BoardPreviewModel.savePanel(name: "Board Front.png"))
        XCTAssertEqual(panel?.nameFieldStringValue, "Board Front.png")
        XCTAssertEqual(panel?.allowedContentTypes, [.png])

        let model = model(try board())
        var alert: NSWindow?
        RunLoop.main.perform(inModes: [.modalPanel]) {
            alert = NSApp.modalWindow
            NSApp.stopModal(withCode: .alertFirstButtonReturn)
        }
        model.showError(CocoaError(.fileWriteNoPermission))
        XCTAssertNotNil(alert)
        XCTAssertFalse(alert?.isVisible ?? true)
    }
}
