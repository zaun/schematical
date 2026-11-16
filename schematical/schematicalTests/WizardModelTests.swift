import Foundation
import XCTest
@testable import schematical

/// plan-46-E Phase 2: the Part Editor's Guided | All fields mode, the
/// wizard's step navigation and each step's issues.
@MainActor
final class WizardModelTests: XCTestCase {
    /// Kept for the life of the test process (see PartFileSaveOpenTests).
    private static var kept: [PartEditorModel] = []

    private func model(_ source: PartEditorTarget.Source) -> PartEditorModel {
        let model = PartEditorModel(target: PartEditorTarget(source: source), catalog: .shared)
        Self.kept.append(model)
        return model
    }

    func testANewPartOpensGuidedAndTheOthersOpenAllFields() throws {
        XCTAssertEqual(model(.new(kind: "gate")).mode, .guided)
        XCTAssertEqual(model(.copy(name: "SN74HC00N")).mode, .allFields)

        let saved = model(.copy(name: "SN74HC00N"))
        saved.draft.name = "WIZARD-FILE"
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("\(UUID().uuidString).schpart")
        defer { try? FileManager.default.removeItem(at: url) }
        try saved.write(to: url)
        let opened = model(.file(url))
        XCTAssertNil(opened.openError)
        XCTAssertEqual(opened.mode, .allFields)

        XCTAssertEqual(PartEditorModel.EditorMode.allCases.map(\.title), ["Guided", "All fields"])
    }

    /// The mode is window state: switching it changes neither the draft nor
    /// whether the part counts as edited.
    func testSwitchingModeKeepsTheDraft() {
        let model = model(.new(kind: "gate"))
        model.draft.name = "MY-NAND"
        let draft = model.draft
        let text = model.draft.fileText()
        model.mode = .allFields
        XCTAssertEqual(model.draft, draft)
        model.mode = .guided
        XCTAssertEqual(model.draft, draft)
        XCTAssertEqual(model.draft.fileText(), text)
        XCTAssertFalse(text.contains("guided"))
    }

    func testNextWalksEveryStepToReview() {
        let model = model(.new(kind: "gate"))
        let steps = model.steps
        XCTAssertEqual(steps.map(\.id), ["kind", "identity", "datasheet", "pinout", "kind.supply",
                                         "kind.gates", "kind.timing", "footprint", "review"])
        // The kind is known, so a new gate starts on the part number; the
        // kind step is one Back away (plan-46-F).
        XCTAssertEqual(model.currentStepID, "identity")
        XCTAssertTrue(model.canGoBack)
        model.goBack()
        XCTAssertEqual(model.currentStepID, "kind")
        XCTAssertFalse(model.canGoBack)
        var visited = [model.currentStepID]
        for _ in 0 ..< steps.count - 1 {
            XCTAssertTrue(model.canGoNext)
            model.goNext()
            visited.append(model.currentStepID)
        }
        XCTAssertEqual(visited, steps.map(\.id))
        XCTAssertEqual(model.currentStep?.page, .review)
        XCTAssertFalse(model.canGoNext)
        model.goNext()
        XCTAssertEqual(model.currentStepID, "review", "Next on the last step stays there")

        model.goBack()
        XCTAssertEqual(model.currentStepID, "footprint")
        model.go(to: "pinout")
        XCTAssertEqual(model.currentStepID, "pinout")
        XCTAssertEqual(model.currentStepIndex, 3)
        model.go(to: "no-such-step")
        XCTAssertEqual(model.currentStepID, "pinout")
        // Issues never stop Next: the gate draft is empty and Next still moved.
        XCTAssertFalse(model.steps.allSatisfy { $0.issues(in: model.draft).isEmpty })
    }

    /// The current step disappears when another model is chosen: the wizard
    /// moves to the first step after the last earlier one that still exists.
    func testAStepThatDisappearsMovesToTheNextSurvivingStep() throws {
        let model = model(.new(kind: "counter"))
        model.draft.kindDevice["model"] = .string("ripple")
        let before = model.steps.map(\.id)
        let gone = "kind.sections"
        let index = try XCTUnwrap(before.firstIndex(of: gone))
        model.go(to: gone)
        XCTAssertEqual(model.currentStepID, gone)

        model.draft.kindDevice["model"] = .string("sync")
        let after = model.steps.map(\.id)
        XCTAssertFalse(after.contains(gone))
        // The last step before it that still exists, then one on.
        let survivor = try XCTUnwrap(before[..<index].last { after.contains($0) })
        let expected = after[min(try XCTUnwrap(after.firstIndex(of: survivor)) + 1, after.count - 1)]
        XCTAssertEqual(model.currentStepID, expected)
        XCTAssertTrue(after.contains(model.currentStepID))
        print("ripple steps: \(before)\nsync steps: \(after)\nmoved to: \(model.currentStepID)")

        // A step that survives the change stays current.
        model.go(to: "pinout")
        model.draft.kindDevice["model"] = .string("ripple")
        XCTAssertEqual(model.currentStepID, "pinout")
    }

    func testAnEmptyGateDraftReportsWhatIsMissing() {
        let draft = PartDraft(newOf: "gate")
        let steps = PartEditorModel.steps(for: draft)
        let issues = Dictionary(uniqueKeysWithValues: steps.map { ($0.id, $0.issues(in: draft)) })
        XCTAssertEqual(issues["identity"], ["The part has no part number"])
        XCTAssertEqual(issues["datasheet"], [])
        XCTAssertEqual(issues["pinout"], ["The part has no pins"])
        XCTAssertEqual(issues["kind.supply"], [], "the supply range is optional")
        XCTAssertEqual(issues["kind.gates"], ["Gates is empty"])
        XCTAssertEqual(issues["kind.timing"], ["Family is empty", "Timing is empty"])
        XCTAssertEqual(issues["footprint"], ["No footprint is chosen"])
        XCTAssertEqual(issues["review"], [])
    }

    func testIssuesReachNestedKeysAndPinNames() {
        var draft = PartDraft(newOf: "gate")
        draft.name = "MY-NAND"
        draft.footprint = "SOIC-14"
        draft.pinouts[0].pins = [PinDraft(designator: "1", name: "A", role: "input"),
                                 PinDraft(designator: "2", name: "Y", role: "output")]
        draft.kindDevice["gates"] = .array([.object([
            "op": .string("nand"), "inputs": .array([.string("A"), .string("B")]), "output": .string("Q"),
        ]), .object(["op": .string("not"), "inputs": .array([])])])
        draft.kindDevice["timing"] = .object(["HC": .object(["rise": .number(9), "fall": .number(9)])])
        draft.kindPart["family"] = .string("HC")
        let steps = PartEditorModel.steps(for: draft)
        let essentials = steps.first { $0.id == "kind.gates" }
        XCTAssertEqual(essentials?.issues(in: draft), [
            "Gates 1 › Inputs: there is no pin named “B”",
            "Gates 1 › Output: there is no pin named “Q”",
            "Gates 2 › Inputs is empty",
            "Gates 2 › Output is empty",
        ])
        for id in ["identity", "pinout", "footprint"] {
            XCTAssertEqual(steps.first { $0.id == id }?.issues(in: draft), [], id)
        }

        // Fixed, the step is complete.
        draft.kindDevice["gates"] = .array([.object([
            "op": .string("nand"), "inputs": .array([.string("A")]), "output": .string("Y"),
        ])])
        XCTAssertEqual(essentials?.issues(in: draft), [])
    }

    /// plan-46-H: a pasted table replaces the part's pins, and a pin renamed
    /// in the pin table is renamed in the kind's settings.
    func testApplyingPinsAndCommittingARename() throws {
        let model = model(.new(kind: "gate"))
        let table = PinTableParser.parse(PinTableParserTests.tiTable)
        let columns = PinTableParser.guessColumns(table)
        model.applyPins(PinTableParser.pins(table, columns: columns).pins, style: PinTableParser.style(columns))
        XCTAssertEqual(model.draft.pinCount, 14)
        XCTAssertEqual(model.draft.pinouts[0].style, .numbered)
        XCTAssertEqual(model.steps.first { $0.id == "pinout" }?.issues(in: model.draft), [])

        model.draft.kindDevice["gates"] = .array([.object([
            "op": .string("nand"), "inputs": .array([.string("1A"), .string("1B")]), "output": .string("1Y"),
        ])])
        let pinout = model.draft.pinouts[0]
        let pin = try XCTUnwrap(pinout.pins.first { $0.name == "1A" })
        XCTAssertEqual(model.commitPinName(pinout: pinout.id, pin: pin.id, to: "A1"), 1)
        XCTAssertEqual(model.renameStatus, "Renamed in 1 setting")
        XCTAssertEqual(model.draft.pinouts[0].pins[0].name, "A1")
        XCTAssertEqual(model.draft.kindDevice["gates"], .array([.object([
            "op": .string("nand"), "inputs": .array([.string("A1"), .string("1B")]), "output": .string("1Y"),
        ])]))

        // A pin no setting names: the table changes, nothing else, no status.
        let vcc = try XCTUnwrap(model.draft.pinouts[0].pins.first { $0.name == "VCC" })
        XCTAssertEqual(model.commitPinName(pinout: pinout.id, pin: vcc.id, to: "VDD"), 0)
        XCTAssertNil(model.renameStatus)
        XCTAssertEqual(model.draft.pinouts[0].pins[13].name, "VDD")
        // The same name again is no rename.
        XCTAssertEqual(model.commitPinName(pinout: pinout.id, pin: vcc.id, to: "VDD"), 0)

        // A name another pin has: the pin is renamed (two GND pins are
        // legitimate), the settings are left for the user.
        let b = try XCTUnwrap(model.draft.pinouts[0].pins.first { $0.name == "1B" })
        XCTAssertEqual(model.commitPinName(pinout: pinout.id, pin: b.id, to: "1Y"), 0)
        XCTAssertEqual(model.draft.pinouts[0].pins[1].name, "1Y")

        // No table to copy: N numbered pins.
        let blank = self.model(.new(kind: "generic"))
        blank.startWithNumberedPins(3)
        XCTAssertEqual(blank.draft.pinouts[0].pins.map(\.designator), ["1", "2", "3"])
        XCTAssertEqual(PinoutStepPage.summary(pins: 14, skipped: 1), "14 pins, 1 row skipped")
        XCTAssertEqual(PinoutStepPage.summary(pins: 1, skipped: 0), "1 pin, 0 rows skipped")
    }

    /// A zero or a "no" is an answer; an untouched supply range is not.
    func testWhatCountsAsEmpty() {
        XCTAssertFalse(FieldType.integer.isEmpty(.number(0)))
        XCTAssertFalse(FieldType.bool.isEmpty(.bool(false)))
        XCTAssertFalse(FieldType.choice(["a", "b"]).isEmpty(.string("")))
        XCTAssertTrue(FieldType.text.isEmpty(.string("")))
        XCTAssertTrue(FieldType.pin.isEmpty(.string("")))
        XCTAssertTrue(FieldType.pins.isEmpty(.array([])))
        XCTAssertTrue(FieldType.voltRange.isEmpty(.array([.number(0), .number(0)])))
        XCTAssertFalse(FieldType.voltRange.isEmpty(.array([.number(2), .number(6)])))
        XCTAssertTrue(FieldType.object([]).isEmpty(.object([:])))
    }

    /// The sections a step hosts are the form's own, picked by the step's keys.
    func testKindFieldsSectionPicksAStepsKeys() throws {
        let schema = try XCTUnwrap(PartKinds.schemas["gate"])
        let all = KindFieldsSection.fields(schema: schema, variant: nil, keys: nil)
        XCTAssertEqual(all.device.map(\.key), ["gates", "output", "timing", "units"])
        XCTAssertEqual(all.part.map(\.key), ["family"], "the form's Supply section has vcc and inputMax")
        let essentials = KindFieldsSection.fields(schema: schema, variant: nil,
                                                  keys: ["device.gates", "device.timing", "part.family"])
        XCTAssertEqual(essentials.device.map(\.key), ["gates", "timing"])
        XCTAssertEqual(essentials.part.map(\.key), ["family"])
        let supply = KindFieldsSection.fields(schema: schema, variant: nil, keys: ["part.vcc", "part.inputMax"])
        XCTAssertEqual(supply.device.map(\.key), [])
        XCTAssertEqual(supply.part.map(\.key), ["vcc", "inputMax"])
    }
}
