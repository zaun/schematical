import Foundation
import XCTest
@testable import schematical

/// The Part Editor's kinds and its help (plan-32-C Phases 1 and 5).
@MainActor
final class PartEditorWindowTests: XCTestCase {
    /// Kept for the life of the test process (see PartFileSaveOpenTests).
    static var kept: [PartEditorModel] = []

    /// (a) Every kind has a form (plan-32-J Phase 4) but the three without one.
    func testEnabledKinds() {
        // capacitor (plan-33), mcu (plan-36) and resistor (plan-34) have no form
        // (plan-33-A, plan-36-H, plan-34-G § Decisions).
        let formless: Set = ["capacitor", "mcu", "resistor", "transistor"]
        XCTAssertEqual(PartDraft.enabledKinds, PartDraft.kindNames.filter { !formless.contains($0) })
        XCTAssertEqual(PartDraft.enabledKinds.count, PartDraft.kindNames.count - formless.count)
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
        XCTAssertEqual(files.count, PartKinds.all.count)
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

    // MARK: Active-low pins (plan-39-B)

    private func activeLowDraft() throws -> PartDraft {
        let text = ChipPinActiveLowTests.fileText(uuid: "C39B0000-0000-4000-8000-000000000002", name: "LOW-E")
        let file = try JSONDecoder().decode(JSONValue.self, from: Data(text.utf8))
        return try PartDraft(file: file)
    }

    private func pins(_ file: JSONValue) -> [JSONValue] {
        guard case let .object(root) = file, case let .array(devices)? = root["devices"],
              case let .object(device)? = devices.first, case let .object(pinouts)? = device["pinouts"],
              case let .array(pins)? = pinouts["p6"] else { return [] }
        return pins
    }

    private func activeLow(_ pin: JSONValue) -> JSONValue? {
        guard case let .object(keys) = pin else { return nil }
        return keys["activeLow"]
    }

    /// Opening a flagged part and saving it unedited writes the same pins:
    /// `true` stays `true`, a list stays the list.
    func testActiveLowRoundTrips() throws {
        let text = ChipPinActiveLowTests.fileText(uuid: "C39B0000-0000-4000-8000-000000000002", name: "LOW-E")
        let file = try JSONDecoder().decode(JSONValue.self, from: Data(text.utf8))
        let draft = try activeLowDraft()
        XCTAssertEqual(draft.extraKeys, [])
        XCTAssertEqual(pins(draft.encoded()), pins(file))
        XCTAssertEqual(activeLow(pins(draft.encoded())[0]), .bool(true))
        XCTAssertEqual(activeLow(pins(draft.encoded())[1]), .array([.string("CS")]))
        XCTAssertNil(activeLow(pins(draft.encoded())[3]))
        if case let .failure(failure) = PartValidator.shared.validate(draft, source: "low.schpart") {
            XCTFail(failure.message)
        }
    }

    /// Renaming a pin drops flags that are no longer an alternative; a
    /// flagged pin made power loses its flag; ticking every alternative
    /// writes `true`.
    func testActiveLowIsWrittenCanonically() throws {
        var draft = try activeLowDraft()
        let row = try XCTUnwrap(draft.pinouts.firstIndex { $0.name == "p6" })
        draft.pinouts[row].pins[1].name = "A0/CSB"
        XCTAssertNil(activeLow(pins(draft.encoded())[1]))

        draft.pinouts[row].pins[0].role = "power"
        XCTAssertNil(activeLow(pins(draft.encoded())[0]))

        var pin = draft.pinouts[row].pins[5]
        XCTAssertEqual(pin.menuAlternatives, ["AIN7", "GPIO3", "DRDY", "CLK"])
        for alternative in pin.menuAlternatives { pin.activeLow.insert(alternative) }
        draft.pinouts[row].pins[5] = pin
        XCTAssertEqual(activeLow(pins(draft.encoded())[5]), .bool(true))

        // Name order, whatever order the menu ticked them in.
        draft.pinouts[row].pins[5].activeLow = ["CLK", "AIN7"]
        XCTAssertEqual(activeLow(pins(draft.encoded())[5]), .array([.string("AIN7"), .string("CLK")]))
    }

    /// A pin whose "activeLow" is not true or a list of strings makes the
    /// pinouts uneditable (they stay an extra), as a bad "side" does.
    func testMalformedActiveLowIsNotLoadedIntoTheGrid() throws {
        let text = ChipPinActiveLowTests.fileText(uuid: "C39B0000-0000-4000-8000-000000000003", name: "LOW-F")
            .replacingOccurrences(of: "\"activeLow\":true", with: "\"activeLow\":false")
        let file = try JSONDecoder().decode(JSONValue.self, from: Data(text.utf8))
        let draft = try PartDraft(file: file)
        XCTAssertTrue(draft.pinouts.isEmpty)
        XCTAssertTrue(draft.extraKeys.contains("device.pinouts"), "\(draft.extraKeys)")
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
