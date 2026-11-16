import Foundation
import XCTest
@testable import schematical

/// plan-46-K Phase 1: the loader's real messages point at the wizard step
/// that holds the problem. Each case breaks a valid gate draft one way,
/// validates it, and runs the locator on what the backend said.
@MainActor
final class WizardIssueLocatorTests: XCTestCase {
    /// A valid part: 14 pins, four NAND gates, family HC, SOIC-14.
    static func gateDraft() -> PartDraft {
        var draft = PartDraft(newOf: "gate")
        draft.name = "LOCATOR-\(UUID().uuidString.prefix(8))"
        draft.manufacturer = "Texas Instruments"
        draft.package = "SOIC"
        draft.function = "NAND"
        draft.description = "Quad 2-input NAND gate"
        let table = PinTableParser.parse(PinTableParserTests.tiTable)
        draft.pinouts[0].pins = PinTableParser.pins(table, columns: PinTableParser.guessColumns(table)).pins
        draft.footprint = "SOIC-14"
        draft.kindDevice["gates"] = .array((1...4).map { n in
            .object(["op": .string("nand"), "inputs": .array([.string("\(n)A"), .string("\(n)B")]),
                     "output": .string("\(n)Y")])
        })
        draft.kindDevice["timing"] = .object(["HC": .object(["rise": .number(9), "fall": .number(9),
                                                             "vcc": .array([.number(2), .number(6)])])])
        draft.kindPart["family"] = .string("HC")
        return draft
    }

    /// Validates a broken draft and returns the step the locator names.
    private func locate(_ label: String, _ change: (inout PartDraft) -> Void) -> String? {
        var draft = Self.gateDraft()
        change(&draft)
        let steps = WizardPlan.steps(for: draft)
        switch PartValidator.shared.validate(draft, source: "part.schpart") {
        case .success:
            XCTFail("\(label): the backend accepted the draft")
            return nil
        case let .failure(failure):
            let step = WizardIssueLocator.step(for: failure.message, steps: steps, draft: draft)
            print("locator \(label): \(failure.message) -> \(step ?? "nil")")
            return step
        }
    }

    func testTheValidDraftLoads() {
        if case let .failure(failure) = PartValidator.shared.validate(Self.gateDraft()) { XCTFail(failure.message) }
        // The step that holds each key, in a gate's derived steps.
        let steps = WizardPlan.steps(for: Self.gateDraft())
        XCTAssertEqual(steps.first { $0.keys.contains("device.gates") }?.id, "kind.essentials")
        XCTAssertEqual(steps.first { $0.keys.contains("part.family") }?.id, "kind.essentials")
        XCTAssertEqual(steps.first { $0.keys.contains("part.vcc") }?.id, "kind.supply")
    }

    func testRealMessagesPointAtTheirSteps() {
        // 1. A gate's output names a pin the part does not have.
        XCTAssertEqual(locate("1 gate output Q") { draft in
            draft.kindDevice["gates"] = .array([.object([
                "op": .string("nand"), "inputs": .array([.string("1A"), .string("1B")]), "output": .string("Q"),
            ])])
        }, "kind.essentials")
        // 2. A footprint with 8 pads for 14 pins.
        XCTAssertEqual(locate("2 footprint SOIC-8") { $0.footprint = "SOIC-8" }, "footprint")
        // 3. Two pins with the number 3.
        XCTAssertEqual(locate("3 duplicate pin number") { $0.pinouts[0].pins[3].designator = "3" }, "pinout")
        // 4. A family the timing table does not have.
        XCTAssertEqual(locate("4 family ZZ") { $0.kindPart["family"] = .string("ZZ") }, "kind.essentials")
        // 5. An inline datasheet that is not https.
        XCTAssertEqual(locate("5 http datasheet") { draft in
            draft.datasheets = [DatasheetDraft(id: "CUSTOM-X", url: "http://example.com/x.pdf")]
            draft.datasheet = "CUSTOM-X"
        }, "datasheet")
        // 6. No name.
        XCTAssertEqual(locate("6 empty name") { $0.name = "" }, "identity")
        // 7. A supply range with its minimum above its maximum.
        XCTAssertEqual(locate("7 vcc [5, 2]") { $0.kindPart["vcc"] = .array([.number(5), .number(2)]) },
                       "kind.supply")
    }

    /// More of the loader's messages, beyond the plan's seven.
    func testMoreMessages() {
        XCTAssertEqual(locate("no pins") { $0.pinouts[0].pins = [] }, "pinout")
        XCTAssertEqual(locate("no footprint") { $0.footprint = nil }, "footprint")
        XCTAssertEqual(locate("unknown footprint") { $0.footprint = "NO-SUCH" }, "footprint")
        XCTAssertEqual(locate("unknown datasheet") { $0.datasheet = "no-such-sheet" }, "datasheet")
        XCTAssertEqual(locate("no gates") { $0.kindDevice["gates"] = .array([]) }, "kind.essentials")
        XCTAssertEqual(locate("bad timing") { draft in
            draft.kindDevice["timing"] = .object(["HC": .object(["rise": .number(-1), "fall": .number(9)])])
        }, "kind.essentials")
        XCTAssertEqual(locate("bad output type") { $0.kindDevice["output"] = .string("totem") }, "kind.other")
    }

    /// The Review page's data: what is still to do, where a message leads,
    /// and each pin's pad (plan-46-K Phase 2).
    func testReviewPageData() throws {
        let model = PartEditorModel(target: PartEditorTarget(source: .new(kind: "gate")), catalog: .shared)
        Self.kept.append(model)
        // An empty gate: everything is still to do, each with its step.
        let todo = ReviewStepPage.todo(model)
        XCTAssertEqual(todo.map(\.issue), ["The part has no part number", "The part has no pins", "Gates is empty",
                                           "Timing is empty", "Family is empty", "No footprint is chosen"])
        XCTAssertEqual(todo.map(\.title), ["Part number", "Pins", "Essentials", "Essentials", "Essentials",
                                           "Footprint"])
        XCTAssertEqual(ReviewStepPage.pinRows(draft: model.draft, part: nil), [])

        // A valid part: nothing to do, every pin on its pad.
        model.draft = Self.gateDraft()
        XCTAssertEqual(ReviewStepPage.todo(model), [])
        let part = try model.validateNow().get()
        let rows = ReviewStepPage.pinRows(draft: model.draft, part: part)
        XCTAssertEqual(rows.count, 14)
        XCTAssertEqual(rows.map(\.designator), (1...14).map(String.init))
        XCTAssertEqual(rows[0].name, "1A")
        XCTAssertEqual(rows[6].role, "ground")
        XCTAssertTrue(rows.allSatisfy { $0.pad == .found })
        // While the draft is rejected the pads are not known.
        XCTAssertTrue(ReviewStepPage.pinRows(draft: model.draft, part: nil).allSatisfy { $0.pad == .unknown })

        // A gate naming a pin the part lacks: the message leads to its step.
        model.draft.kindDevice["gates"] = .array([.object([
            "op": .string("nand"), "inputs": .array([.string("1A"), .string("1B")]), "output": .string("Q"),
        ])])
        guard case let .failure(failure) = model.validateNow() else { return XCTFail("accepted") }
        let target = try XCTUnwrap(ReviewStepPage.target(of: failure.message, in: model))
        XCTAssertEqual(target.id, "kind.essentials")
        XCTAssertEqual(target.title, "Essentials")
        model.go(to: "review")
        model.go(to: target.id)
        XCTAssertEqual(model.currentStepID, "kind.essentials")
        // The wizard's own check names the same step.
        XCTAssertEqual(ReviewStepPage.todo(model).map(\.issue), ["Gates 1 › Output: there is no pin named “Q”"])
        XCTAssertNil(ReviewStepPage.target(of: "part.schpart: out of memory", in: model))

        // A footprint with too few pads, and an absent pin.
        var draft = Self.gateDraft()
        draft.pinouts[0].pins[2].role = "absent"
        let absent = ReviewStepPage.pinRows(draft: draft, part: part)
        XCTAssertEqual(absent[2].pad, .notNeeded)
        var short = Self.gateDraft()
        short.pinouts[0].pins.append(PinDraft(designator: "15", name: "X", role: "input"))
        XCTAssertEqual(ReviewStepPage.pinRows(draft: short, part: part).last?.pad, .missing)
    }

    private static var kept: [PartEditorModel] = []

    func testHelpers() {
        XCTAssertEqual(WizardIssueLocator.identifier(of: "gates[0]"), "gates")
        XCTAssertEqual(WizardIssueLocator.identifier(of: "timing.HC"), "timing")
        XCTAssertEqual(WizardIssueLocator.identifier(of: "devices[0] (1G00)"), "devices")
        XCTAssertEqual(WizardIssueLocator.quotedWords(in: #"unknown key "gates" in "x y""#), ["gates", "x y"])
        // A message nothing places gets no step.
        let draft = Self.gateDraft()
        XCTAssertNil(WizardIssueLocator.step(for: "part.schpart: out of memory", steps: WizardPlan.steps(for: draft),
                                             draft: draft))
        // A nested key points at the step of the key it is inside.
        XCTAssertEqual(WizardIssueLocator.kindStep(named: "inputs", steps: WizardPlan.steps(for: draft), draft: draft),
                       "kind.essentials")
        XCTAssertNil(WizardIssueLocator.kindStep(named: "nonsense", steps: WizardPlan.steps(for: draft), draft: draft))
    }
}
