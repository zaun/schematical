import Foundation
import XCTest
@testable import schematical

/// The Part Editor's kinds and its help (plan-32-C Phases 1 and 5).
@MainActor
final class PartEditorWindowTests: XCTestCase {
    /// Kept for the life of the test process (see PartFileSaveOpenTests).
    static var kept: [PartEditorModel] = []

    /// (a) Every kind has a form (plan-46-C gave the last five theirs).
    func testEnabledKinds() {
        XCTAssertEqual(PartDraft.enabledKinds, PartDraft.kindNames)
        XCTAssertEqual(PartDraft.enabledKinds.count, 47)  // plan-47-K, L added inductor, fuse, relay, buzzer
    }

    /// A new generic part (plan-46-B, C) starts unsimulated and the backend
    /// accepts it once it has pins, a name and a footprint.
    func testNewGenericDraftValidates() throws {
        var draft = PartDraft(newOf: "generic")
        XCTAssertEqual(draft.kindDevice, ["simulated": .bool(false)])
        XCTAssertEqual(draft.kindPart, [:])
        XCTAssertEqual(draft.manufacturer, "Generic")
        draft.name = "GENERIC-NEW-\(UUID().uuidString.prefix(8))"
        draft.function = "Sensor"
        draft.description = "A two-pin part"
        draft.package = "0603"
        draft.pinouts[0].pins = [PinDraft(designator: "1", name: "A", role: "inout"),
                                 PinDraft(designator: "2", name: "B", role: "inout")]
        let footprint = try XCTUnwrap(PartValidator.shared.footprints.first { $0.pads.count == 2 })
        draft.footprint = footprint.id
        XCTAssertEqual(draft.extraKeys, [])
        switch PartValidator.shared.validate(draft, source: "generic.schpart") {
        case let .success(part):
            XCTAssertEqual(part.kind, PART_GENERIC)
            XCTAssertFalse(part.isSimulated)
            XCTAssertEqual(part.pins.count, 2)
        case let .failure(failure):
            XCTFail(failure.message)
        }
        // Read back, it is the same draft with no key left over.
        let reopened = try PartDraft(fileText: draft.fileText())
        XCTAssertEqual(reopened, draft)
        XCTAssertEqual(reopened.extraKeys, [])
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
        // plan-46-N: the wizard and its two modes.
        XCTAssertTrue(text.contains("Part Wizard"))
        XCTAssertTrue(text.contains("Guided"))
        XCTAssertTrue(text.contains("All fields"))
        XCTAssertTrue(text.contains("Paste a pin table"))
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
        XCTAssertEqual(model.title, "New Logic Gate Part")
        // A new part is unchanged until edited (Open Part… asks only then).
        XCTAssertFalse(model.isDirty)
        model.draft.name = "CHANGED"
        XCTAssertTrue(model.isDirty)
    }

    /// plan-54-O: the connector kind's schema has the card-slot keys, and a library card slot opens as a draft
    /// with its `card` (and `cardPitch`) in the kind's device keys and nothing left over.
    func testConnectorSchemaHasTheCardSlotKeys() throws {
        let schema = try XCTUnwrap(PartKinds.schemas["connector"])
        XCTAssertTrue(schema.keys.isSuperset(of: ["card", "cardPitch"]), "\(schema.keys)")
        let card = try XCTUnwrap(schema.device.first { $0.key == "card" })
        let pitch = try XCTUnwrap(schema.device.first { $0.key == "cardPitch" })
        if case .suggested = card.type {} else { XCTFail("card is \(card.type)") }
        if case .number(unit: "mm") = pitch.type {} else { XCTFail("cardPitch is \(pitch.type)") }
        for part in try CardSlotTests.representatives().compactMap(\.part) {
            let draft = try PartDraft(file: XCTUnwrap(BundledPartFile.copy(name: part.name, kind: "connector"), part.name))
            XCTAssertEqual(draft.extraKeys, [], part.name)
            XCTAssertTrue(draft.kindDevice.keys.contains("card"), part.name)
            XCTAssertEqual(draft.kindDevice.keys.contains("cardPitch"), part.detailLabels.count > 1, part.name)
        }
    }

    /// plan-45-H: the connector kind's schema has `terminal`, `jst` and `pitch` (millimetres), and a library terminal
    /// block and JST header open as drafts with their keys in the kind's device keys and nothing left over.
    func testConnectorSchemaHasTheTerminalKeys() throws {
        let schema = try XCTUnwrap(PartKinds.schemas["connector"])
        XCTAssertTrue(schema.keys.isSuperset(of: ["terminal", "jst", "pitch"]), "\(schema.keys)")
        let terminal = try XCTUnwrap(schema.device.first { $0.key == "terminal" })
        let jst = try XCTUnwrap(schema.device.first { $0.key == "jst" })
        let pitch = try XCTUnwrap(schema.device.first { $0.key == "pitch" })
        if case let .suggested(words) = terminal.type {
            XCTAssertEqual(Set(words), ["screw", "spring", "push-in", "pluggable"])
        } else { XCTFail("terminal is \(terminal.type)") }
        if case let .suggested(series) = jst.type {
            XCTAssertEqual(Set(series), ["XH", "PH", "EH", "ZH", "SH", "GH", "VH"])
        } else { XCTFail("jst is \(jst.type)") }
        if case .number(unit: "mm") = pitch.type {} else { XCTFail("pitch is \(pitch.type)") }
        for name in ["GENERIC-TERMINAL-5.08-03", "GENERIC-JST-XH-04"] {
            let draft = try PartDraft(file: XCTUnwrap(BundledPartFile.copy(name: name, kind: "connector"), name))
            XCTAssertEqual(draft.extraKeys, [], name)
            XCTAssertTrue(draft.kindDevice.keys.contains("pitch"), name)
            XCTAssertTrue(draft.kindDevice.keys.contains(name.contains("JST") ? "jst" : "terminal"), name)
        }
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

    /// (e) plan-54-F: a custom footprint's opposite pad and edge mount are
    /// written only when set (`"opposite": true`, `"edgeMount": true`), read
    /// back into the draft, and load in the backend.
    func testOppositePadAndEdgeMountRoundTrip() throws {
        var draft = CustomFootprintDraft.starter(id: "CUSTOM-ABCDEF12", pins: 2)
        for index in draft.pads.indices { draft.pads[index].drill = 0 }
        guard case let .object(plain) = draft.encoded() else { return XCTFail("shape") }
        XCTAssertNil(plain["edgeMount"])
        for pad in draft.pads {
            guard case let .object(keys) = pad.encoded() else { return XCTFail("pad") }
            XCTAssertNil(keys["opposite"])
        }

        draft.pads[1].opposite = true
        draft.edgeMount = true
        guard case let .object(keys) = draft.encoded(), case let .array(pads)? = keys["pads"],
              case let .object(first) = pads[0], case let .object(second) = pads[1] else { return XCTFail("shape") }
        XCTAssertEqual(second["opposite"], .bool(true))
        XCTAssertNil(first["opposite"])
        XCTAssertEqual(keys["edgeMount"], .bool(true))
        // The entry reads back to the same draft.
        XCTAssertEqual(CustomFootprintDraft(json: draft.encoded()), draft)
        // The backend loads it, and the draft of the loaded footprint is the same draft.
        guard case let .success(footprint) = CustomFootprintDraftTests.load(draft.encoded()) else {
            return XCTFail("rejected")
        }
        XCTAssertEqual(footprint.pads.map(\.opposite), [false, true])
        XCTAssertTrue(footprint.edgeMount)
        XCTAssertEqual(CustomFootprintDraft(footprint: footprint), draft)
    }
}
