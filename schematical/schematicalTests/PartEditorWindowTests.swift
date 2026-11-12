import Foundation
import XCTest
@testable import schematical

/// The Part Editor's kinds and its help (plan-32-C Phases 1 and 5).
@MainActor
final class PartEditorWindowTests: XCTestCase {
    /// Kept for the life of the test process (see PartFileSaveOpenTests).
    static var kept: [PartEditorModel] = []

    /// (a) Only the kinds with a form are enabled.
    func testEnabledKinds() {
        XCTAssertEqual(PartDraft.enabledKinds, ["decoder", "dff", "gate", "jkff", "mux"])
    }

    /// (b) The picker lists one kind per bundled part file.
    func testKindNamesAreThePartFiles() throws {
        let parts = URL(fileURLWithPath: #filePath)
            .deletingLastPathComponent().deletingLastPathComponent()
            .appendingPathComponent("schematical/Parts")
        let files = try FileManager.default.contentsOfDirectory(atPath: parts.path)
            .filter { $0.hasSuffix(".json") && $0 != "footprints.json" && $0 != "datasheets.json" }
            .map { String($0.dropLast(".json".count)) }
            .sorted()
        XCTAssertEqual(files.count, 26)
        XCTAssertEqual(PartDraft.kindNames, files)
        for kind in PartDraft.kindNames {
            XCTAssertNotEqual(PartDraft.displayName(of: kind), kind, "no display name for \(kind)")
        }
    }

    /// (c) The Schematic Help page lists New Part… and its key.
    func testHelpListsNewPart() {
        let entries = EditorPage.schematic.helpSections.flatMap(\.entries)
        let text = entries.map { "\($0.keys) \($0.action)" }.joined(separator: "\n")
        XCTAssertTrue(text.contains("New Part"))
        XCTAssertTrue(text.contains("⇧⌘N"))
        XCTAssertTrue(text.contains("Copy to Editor"))
    }

    /// A new part of an enabled kind starts with the kind's required keys.
    func testNewGateDraft() {
        let draft = PartDraft(newOf: "gate")
        XCTAssertEqual(Set(draft.kindDevice.keys), ["gates", "timing"])
        XCTAssertEqual(Set(draft.kindPart.keys), ["family"])
        XCTAssertEqual(draft.extraKeys, [])
        let model = PartEditorModel(target: PartEditorTarget(source: .new(kind: "gate")))
        Self.kept.append(model)
        XCTAssertEqual(model.title, "New Gate Part")
        // A new part is unchanged until edited (Open Part… asks only then).
        XCTAssertFalse(model.isDirty)
        model.draft.name = "CHANGED"
        XCTAssertTrue(model.isDirty)
    }

    /// (d) The preview pane starts on Block, and switches to Footprint.
    func testPreviewPaneDefaultsToBlock() {
        let model = PartEditorModel(target: PartEditorTarget(source: .copy(name: "SN74HC00N")))
        Self.kept.append(model)
        XCTAssertEqual(model.previewPane, .block)
        model.previewPane = .footprint
        XCTAssertEqual(model.previewPane, .footprint)
        XCTAssertEqual(PartEditorModel.PreviewPane.allCases.map(\.title), ["Block", "Footprint"])
    }
}
