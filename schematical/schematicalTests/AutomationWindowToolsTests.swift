import AppKit
import CoreGraphics
import Foundation
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// plan-61-D: the agent-control tools about windows and files (`new_window`,
/// `open`, `save`, `list_windows`, `focus_window`) act on the app's open
/// documents. Driven through `AutomationTools.call`, the dispatcher the MCP
/// server calls, with a temporary folder shared in place of the user's.
@MainActor
final class AutomationWindowToolsTests: XCTestCase {
    private var folder: URL!
    private var unshare: (() -> Void)?
    /// The documents open before the test: left alone.
    private var before: [NSDocument] = []

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
        folder = FileManager.default.temporaryDirectory
            .appendingPathComponent("AutomationWindowToolsTests-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        unshare = AutomationFolder.shared.shareTemporarily(folder)
        before = NSDocumentController.shared.documents
    }

    override func tearDown() async throws {
        // Every document a tool opened is closed (never saved, never asked about).
        for document in NSDocumentController.shared.documents where !before.contains(document) {
            TestSupport.keep(document as! SchematicNSDocument)
            document.close()
        }
        unshare?()
        unshare = nil
        try? FileManager.default.removeItem(at: folder)
    }

    private struct Result {
        let text: String
        let isError: Bool
    }

    private func call(_ tool: String, _ args: [String: Any] = [:]) async -> Result {
        let result = await AutomationTools.call(tool, args)
        let content = result["content"] as? [[String: Any]] ?? []
        return Result(text: content.compactMap { $0["text"] as? String }.joined(separator: "\n"),
                      isError: result["isError"] as? Bool ?? false)
    }

    /// The documents the tools opened in this test.
    private var opened: [SchematicNSDocument] {
        NSDocumentController.shared.documents.filter { !before.contains($0) }.compactMap { $0 as? SchematicNSDocument }
    }

    /// The id (as agents see it) in "opened window 1A2B3C4D" and the like.
    private func windowID(in text: String) throws -> String {
        let id = try XCTUnwrap(text.split(separator: " ").last.map(String.init), text)
        return id.trimmingCharacters(in: CharacterSet(charactersIn: "()"))
    }

    private func document(_ shortID: String) throws -> SchematicNSDocument {
        try XCTUnwrap(opened.first { $0.model.id.uuidString.hasPrefix(shortID) }, "no document \(shortID)")
    }

    /// An example copied into the shared folder, by its name there.
    private func shareExample(_ name: String) throws -> String {
        let source = URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent()
            .deletingLastPathComponent().appendingPathComponent("examples/\(name)")
        try FileManager.default.copyItem(at: source, to: folder.appendingPathComponent(name))
        return name
    }

    /// The net label names in the schematic file at `url`, read as the app reads it.
    private func labelNames(in url: URL) throws -> [String] {
        let reader = TestSupport.keep(SchematicNSDocument())
        try reader.read(from: url, ofType: SchematicFile.contentType.identifier)
        return reader.model.document.netLabels.map(\.name)
    }

    private func settle() async throws {
        try await Task.sleep(nanoseconds: 200_000_000)
    }

    func testNewWindowAddsADocument() async throws {
        let result = await call("new_window")
        XCTAssertFalse(result.isError, result.text)
        XCTAssertTrue(result.text.hasPrefix("opened window "), result.text)
        XCTAssertEqual(opened.count, 1)
        let made = try document(try windowID(in: result.text))
        XCTAssertNil(made.fileURL)
        XCTAssertEqual(made.windowControllers.count, 1, "it has its window")

        let second = await call("new_window")
        XCTAssertFalse(second.isError, second.text)
        XCTAssertEqual(opened.count, 2)
        XCTAssertNotEqual(try windowID(in: second.text), try windowID(in: result.text))
    }

    func testOpenOpensAFileOnce() async throws {
        let name = try shareExample("6502.schematical")
        let first = await call("open", ["path": name])
        XCTAssertFalse(first.isError, first.text)
        XCTAssertTrue(first.text.hasPrefix("opened in window "), first.text)
        XCTAssertEqual(opened.count, 1)
        let id = try windowID(in: first.text)
        let document = try document(id)
        XCTAssertEqual(document.fileURL?.lastPathComponent, name)
        XCTAssertFalse(document.model.document.chips.isEmpty)

        // The same file again: no second document, the same window.
        let again = await call("open", ["path": name])
        XCTAssertFalse(again.isError, again.text)
        XCTAssertEqual(opened.count, 1)
        XCTAssertEqual(again.text, "opened (in window \(id))")

        // A file that is not there, and one outside the folder.
        let missing = await call("open", ["path": "nothing.schematical"])
        XCTAssertTrue(missing.isError)
        XCTAssertTrue(missing.text.hasPrefix("can't open nothing.schematical"), missing.text)
        let outside = await call("open", ["path": "/etc/hosts"])
        XCTAssertTrue(outside.isError)
        XCTAssertTrue(outside.text.contains("outside the shared folder"), outside.text)
        XCTAssertEqual(opened.count, 1)
    }

    func testSaveNeedsAPathForAnUntitledDocumentAndWritesTheFile() async throws {
        let made = await call("new_window")
        let id = try windowID(in: made.text)
        let document = try document(id)
        let label = document.model.document.addNetLabel(at: CGPoint(x: 100, y: 100))
        XCTAssertTrue(document.model.document.setNetLabelName(label, "CLK"))

        let refused = await call("save", ["window": id])
        XCTAssertTrue(refused.isError)
        XCTAssertEqual(refused.text, "This window has no file yet; pass a path.")

        let saved = await call("save", ["window": id, "path": "made.schematical"])
        XCTAssertFalse(saved.isError, saved.text)
        let url = folder.appendingPathComponent("made.schematical")
        XCTAssertEqual(saved.text, "saved to \(url.resolvingSymlinksInPath().path)")
        XCTAssertEqual(try labelNames(in: url), ["CLK"])
        // With a path it is a Save As: the document is at the new file.
        XCTAssertEqual(document.fileURL?.lastPathComponent, "made.schematical")
        XCTAssertFalse(document.isDocumentEdited)

        // Saved again with no path: to its own file.
        XCTAssertTrue(document.model.document.setNetLabelName(label, "CLOCK"))
        let again = await call("save", ["window": id])
        XCTAssertFalse(again.isError, again.text)
        XCTAssertEqual(try labelNames(in: url), ["CLOCK"])

        let outside = await call("save", ["window": id, "path": "/tmp/outside.schematical"])
        XCTAssertTrue(outside.isError)
        XCTAssertTrue(outside.text.contains("outside the shared folder"), outside.text)
    }

    func testListWindowsListsEveryDocumentAndMarksTheEditedOne() async throws {
        let name = try shareExample("6502.schematical")
        let untouched = try windowID(in: await call("open", ["path": name]).text)
        let edited = try windowID(in: await call("new_window").text)
        _ = try document(edited).model.document.addNetLabel(at: CGPoint(x: 100, y: 100))
        try await settle()

        let listing = await call("list_windows")
        XCTAssertFalse(listing.isError, listing.text)
        let lines = listing.text.split(separator: "\n").map(String.init)
        let mine = lines.filter { $0.hasPrefix(untouched) || $0.hasPrefix(edited) }
        XCTAssertEqual(mine.count, 2, listing.text)
        XCTAssertEqual(lines.count, NSDocumentController.shared.documents.count, "one line per open document")
        let untouchedLine = try XCTUnwrap(mine.first { $0.hasPrefix(untouched) })
        let editedLine = try XCTUnwrap(mine.first { $0.hasPrefix(edited) })
        XCTAssertTrue(untouchedLine.contains("  6502  "), untouchedLine)
        XCTAssertTrue(untouchedLine.contains("parts=\(try document(untouched).model.document.chips.count) "), untouchedLine)
        XCTAssertFalse(untouchedLine.contains("unsaved"), untouchedLine)
        XCTAssertTrue(editedLine.contains("  Untitled  "), editedLine)
        XCTAssertTrue(editedLine.contains("parts=0 wires=0"), editedLine)
        XCTAssertTrue(editedLine.contains("  unsaved"), editedLine)
    }

    func testFocusWindowBringsADocumentsWindowForward() async throws {
        let first = try windowID(in: await call("new_window").text)
        let second = try windowID(in: await call("new_window").text)
        let result = await call("focus_window", ["window": first])
        XCTAssertFalse(result.isError, result.text)
        XCTAssertEqual(result.text, "window \(first) is frontmost")
        let front = NSApp.orderedWindows.first { window in opened.contains { $0.windowControllers.first?.window === window } }
        XCTAssertTrue(front === (try document(first)).windowControllers.first?.window)
        // An id that names no window is refused.
        let unknown = await call("focus_window", ["window": "00000000"])
        XCTAssertTrue(unknown.isError)
        XCTAssertTrue(unknown.text.hasPrefix("no window 00000000"), unknown.text)
        _ = second
    }
}
