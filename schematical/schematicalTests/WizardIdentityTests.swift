import Foundation
import XCTest
@testable import schematical

/// plan-46-F Phase 1: the Part Wizard's first two steps as model rules —
/// choosing and changing the kind, starting from a similar part, and the
/// keys derived from the part number.
@MainActor
final class WizardIdentityTests: XCTestCase {
    /// Kept for the life of the test process (see PartFileSaveOpenTests).
    private static var kept: [PartEditorModel] = []

    private func model(_ source: PartEditorTarget.Source) -> PartEditorModel {
        let model = PartEditorModel(target: PartEditorTarget(source: source), catalog: .shared)
        Self.kept.append(model)
        return model
    }

    func testTheWizardStartsOnTheKindStepWithNoKindChosen() {
        let model = model(.wizard)
        XCTAssertEqual(model.mode, .guided)
        XCTAssertFalse(model.kindChosen)
        XCTAssertTrue(model.isNewPart)
        XCTAssertEqual(model.currentStepID, "kind")
        XCTAssertEqual(model.draft.kind, "generic", "a placeholder until the kind is chosen")
        // No kind steps are listed for the placeholder.
        XCTAssertEqual(model.steps.map(\.id), ["kind", "identity", "datasheet", "pinout", "footprint", "review"])
        XCTAssertEqual(model.issues(of: .kind), ["Choose what you are making"])
        XCTAssertEqual(model.title, "New Part")
        XCTAssertFalse(model.isDirty)

        model.chooseKind("gate")
        XCTAssertTrue(model.kindChosen)
        XCTAssertEqual(model.draft.kind, "gate")
        XCTAssertEqual(model.currentStepID, "identity")
        XCTAssertEqual(model.issues(of: .kind), [])
        XCTAssertEqual(model.steps.map(\.id), ["kind", "identity", "datasheet", "pinout", "kind.supply",
                                               "kind.gates", "kind.timing", "footprint", "review"])
        XCTAssertEqual(Set(model.draft.kindDevice.keys), ["gates", "timing"])
        XCTAssertEqual(model.title, "New Logic Gate Part")

        // The no-simulation choice is the generic kind, unsimulated from the start.
        let generic = self.model(.wizard)
        generic.chooseKind("generic")
        XCTAssertTrue(generic.kindChosen)
        XCTAssertEqual(generic.draft.kindDevice, ["simulated": .bool(false)])
        XCTAssertEqual(generic.currentStepID, "identity")
    }

    func testChangingKindKeepsTheGenericKeysAndDropsTheKindsOwn() throws {
        var draft = PartDraft(newOf: "gate")
        draft.name = "MY-PART"
        draft.manufacturer = "Texas Instruments"
        draft.package = "SOIC"
        draft.description = "A part"
        draft.note = "a note"
        draft.flags = ["automotive"]
        draft.setDatasheetURL("https://example.com/my.pdf")
        draft.pinouts[0].pins = [PinDraft(designator: "1", name: "A", role: "input"),
                                 PinDraft(designator: "2", name: "K", role: "output")]
        draft.footprint = "SOIC-14"
        draft.kindDevice["gates"] = .array([.object(["op": .string("not"), "inputs": .array([.string("A")]),
                                                     "output": .string("K")])])
        draft.kindPart["family"] = .string("HC")
        draft.deviceExtras["leftover"] = .bool(true)

        let led = draft.changingKind(to: "led")
        XCTAssertEqual(led.kind, "led")
        XCTAssertEqual(led.uuid, draft.uuid)
        XCTAssertEqual(led.name, "MY-PART")
        XCTAssertEqual(led.manufacturer, "Texas Instruments")
        XCTAssertEqual(led.package, "SOIC")
        XCTAssertEqual(led.description, "A part")
        XCTAssertEqual(led.note, "a note")
        XCTAssertEqual(led.flags, ["automotive"])
        XCTAssertEqual(led.datasheet, draft.datasheet)
        XCTAssertEqual(led.datasheets, draft.datasheets)
        XCTAssertEqual(led.pinouts, draft.pinouts)
        XCTAssertEqual(led.pinout, draft.pinout)
        XCTAssertEqual(led.footprint, "SOIC-14")
        XCTAssertNil(led.kindDevice["gates"])
        XCTAssertNil(led.kindPart["family"])
        XCTAssertEqual(led.kindDevice, PartDraft(newOf: "led").kindDevice)
        XCTAssertEqual(led.kindPart, PartDraft(newOf: "led").kindPart)
        XCTAssertEqual(led.deviceExtras, [:])
        XCTAssertEqual(led.extraKeys, [])
    }

    /// The view asks before a change that clears settings, and only then.
    func testAKindChangeAsksOnlyWhenSettingsWouldBeLost() {
        let model = model(.wizard)
        XCTAssertFalse(model.kindChangeClearsSettings(to: "gate"), "no kind chosen yet")
        model.chooseKind("gate")
        XCTAssertFalse(model.kindChangeClearsSettings(to: "led"), "the gate's keys are still a new part's")
        XCTAssertFalse(model.kindChangeClearsSettings(to: "gate"), "the same kind changes nothing")
        model.draft.kindPart["family"] = .string("HC")
        XCTAssertTrue(model.kindChangeClearsSettings(to: "led"))
        model.draft.name = "KEEP-ME"
        model.chooseKind("led")
        XCTAssertEqual(model.draft.kind, "led")
        XCTAssertEqual(model.draft.name, "KEEP-ME")
        XCTAssertNil(model.draft.kindPart["family"])
        XCTAssertEqual(KindStepPage.clearMessage(from: "gate"),
                       "The logic gate's own settings will be cleared. Pins, name, datasheet and footprint are kept.")
    }

    func testStartingFromASimilarPart() throws {
        let model = model(.wizard)
        let matches = model.similarParts(matching: "SN74HC00N")
        XCTAssertTrue(matches.contains { $0.name == "SN74HC00N" }, "\(matches.map(\.name))")
        XCTAssertLessThanOrEqual(model.similarParts(matching: "74").count, 20)
        XCTAssertEqual(model.similarParts(matching: "  "), [])
        let part = try XCTUnwrap(matches.first { $0.name == "SN74HC00N" })
        XCTAssertEqual(KindStepPage.similarRow(part), "SN74HC00N — NAND, DIP, Texas Instruments")

        let placeholder = model.draft.uuid
        XCTAssertTrue(model.chooseSimilar(named: "SN74HC00N"))
        XCTAssertTrue(model.kindChosen)
        XCTAssertEqual(model.draft.kind, "gate")
        XCTAssertEqual(model.currentStepID, "identity")
        XCTAssertNotNil(UUID(uuidString: model.draft.uuid))
        XCTAssertNotEqual(model.draft.uuid, placeholder)
        // The library's suffixed name: it never collides with the original.
        XCTAssertEqual(model.draft.name, "SN74HC00N.2")
        XCTAssertEqual(model.libraryName, "SN74HC00N.2")
        XCTAssertEqual(model.draft.pinCount, 14)
        // A copy keeps its own keys: nothing is derived from the name.
        XCTAssertEqual(model.autoKeys, [])
        let deviceID = model.draft.deviceID
        model.draft.name = "MY-QUAD-NAND"
        XCTAssertEqual(model.draft.deviceID, deviceID)
        if case let .failure(failure) = model.validateNow() { XCTFail(failure.message) }

        XCTAssertFalse(model.chooseSimilar(named: "NO-SUCH-PART"))
    }

    func testKeysDerivedFromThePartNumberUntilEdited() {
        XCTAssertEqual(PartEditorModel.deviceID(forName: "74hc00-x/y"), "74HC00-XY")
        XCTAssertEqual(PartEditorModel.deviceID(forName: "sn74lvc1g00.dbv r"), "SN74LVC1G00.DBVR")
        XCTAssertEqual(PartEditorModel.deviceID(forName: ""), "custom")
        XCTAssertEqual(PartEditorModel.deviceID(forName: "//"), "custom")

        let model = model(.wizard)
        XCTAssertEqual(model.autoKeys, [.deviceID, .function, .drawer])
        model.chooseKind("gate")
        XCTAssertEqual(model.draft.function, "Logic Gate")
        XCTAssertEqual(model.draft.drawer, ["Custom", "Logic gates and flip-flops"])
        XCTAssertEqual(model.draft.deviceID, "custom")
        model.draft.name = "74hc00-x/y"
        XCTAssertEqual(model.draft.deviceID, "74HC00-XY")
        XCTAssertEqual(model.autoKeys, [.deviceID, .function, .drawer], "typing the part number is not an edit of the keys")

        // Editing the drawer stops its derivation; the others go on.
        model.draft.drawer = ["Mine"]
        XCTAssertEqual(model.autoKeys, [.deviceID, .function])
        model.chooseKind("led")
        XCTAssertEqual(model.draft.drawer, ["Mine"])
        XCTAssertEqual(model.draft.function, "LED")
        model.draft.name = "RED-LED"
        XCTAssertEqual(model.draft.deviceID, "RED-LED")

        model.draft.deviceID = "my-id"
        XCTAssertEqual(model.autoKeys, [.function])
        model.draft.name = "RED-LED-2"
        XCTAssertEqual(model.draft.deviceID, "my-id")
        model.draft.function = "Indicator"
        XCTAssertEqual(model.autoKeys, [])
        model.chooseKind("gate")
        XCTAssertEqual(model.draft.function, "Indicator")

        // A new part of a known kind derives them too; a copy and a file do not.
        let gate = self.model(.new(kind: "gate"))
        XCTAssertEqual(gate.draft.function, "Logic Gate")
        XCTAssertEqual(gate.draft.drawer, ["Custom", "Logic gates and flip-flops"])
        XCTAssertFalse(gate.isDirty)
        XCTAssertEqual(self.model(.copy(name: "SN74HC00N")).autoKeys, [])
    }

    func testAllFieldsChangeGoesBackToTheKindStep() {
        let model = model(.new(kind: "gate"))
        XCTAssertTrue(model.isNewPart)
        model.mode = .allFields
        model.changeKind()
        XCTAssertEqual(model.mode, .guided)
        XCTAssertEqual(model.currentStepID, "kind")
        XCTAssertFalse(self.model(.copy(name: "SN74HC00N")).isNewPart)
    }

    func testPackageMenuFiltersByTheTypedText() {
        let all = PartCatalogStore.shared.packages
        XCTAssertFalse(all.isEmpty)
        let soic = IdentityStepPage.packages(all, matching: "soic")
        XCTAssertFalse(soic.isEmpty)
        XCTAssertTrue(soic.allSatisfy { $0.lowercased().contains("soic") })
        XCTAssertLessThanOrEqual(IdentityStepPage.packages(all, matching: "").count, 40)
        let makers = model(.wizard).manufacturers
        XCTAssertTrue(makers.contains("Texas Instruments"))
        XCTAssertFalse(makers.contains("Generic"))
    }
}
