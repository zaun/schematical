import Foundation
import XCTest
@testable import schematical

/// plan-46-D Phase 3: `PartDraft.renamePin(from:to:)` carries a pin's new
/// name into every kind key that names it, and refuses a collision.
@MainActor
final class PinRenameTests: XCTestCase {
    private func gateDraft() -> PartDraft {
        var draft = PartDraft(newOf: "gate")
        draft.pinouts[0].pins = [
            PinDraft(designator: "1", name: "A", role: "input"),
            PinDraft(designator: "2", name: "B", role: "input"),
            PinDraft(designator: "3", name: "Y", role: "output"),
            PinDraft(designator: "4", name: "GND", role: "ground"),
            PinDraft(designator: "5", name: "VCC", role: "power"),
        ]
        draft.kindDevice["gates"] = .array([.object([
            "op": .string("nand"), "inputs": .array([.string("A"), .string("B")]), "output": .string("Y"),
        ])])
        return draft
    }

    func testAGateInputIsRenamed() {
        var draft = gateDraft()
        XCTAssertEqual(draft.renamePin(from: "A", to: "1A"), 1)
        XCTAssertEqual(draft.kindDevice["gates"], .array([.object([
            "op": .string("nand"), "inputs": .array([.string("1A"), .string("B")]), "output": .string("Y"),
        ])]))
        // The pin table is the caller's to change.
        XCTAssertEqual(draft.pinouts[0].pins[0].name, "A")

        // The single `.pin` value of the same object.
        var output = gateDraft()
        XCTAssertEqual(output.renamePin(from: "Y", to: "1Y"), 1)
        guard case let .array(gates)? = output.kindDevice["gates"], case let .object(gate)? = gates.first else {
            return XCTFail("no gates")
        }
        XCTAssertEqual(gate["output"], .string("1Y"))
        XCTAssertEqual(gate["op"], .string("nand"), "a text value that is not a pin is left")
    }

    func testACollisionRewritesNothing() {
        var draft = gateDraft()
        let before = draft
        XCTAssertEqual(draft.renamePin(from: "A", to: "B"), 0, "B is another pin's name")
        XCTAssertEqual(draft.renamePin(from: "Q", to: "Z"), 0, "no pin is named Q")
        XCTAssertEqual(draft.renamePin(from: "A", to: "A"), 0)
        XCTAssertEqual(draft.renamePin(from: "A", to: ""), 0)
        XCTAssertEqual(draft, before)

        // Two pins with the old name: which one the keys mean is not known.
        draft.pinouts[0].pins.append(PinDraft(designator: "6", name: "A", role: "input"))
        XCTAssertEqual(draft.renamePin(from: "A", to: "1A"), 0)
    }

    func testWholeNamesOnly() {
        var draft = gateDraft()
        draft.pinouts[0].pins[0].name = "A/CS"
        draft.kindDevice["gates"] = .array([.object([
            "op": .string("nand"), "inputs": .array([.string("A/CS"), .string("B")]), "output": .string("Y"),
        ])])
        draft.pinouts[0].pins.append(PinDraft(designator: "6", name: "A", role: "input"))
        // "A" is its own pin; the reference to "A/CS" is not touched.
        XCTAssertEqual(draft.renamePin(from: "A", to: "X"), 0)
        XCTAssertEqual(draft.renamePin(from: "A/CS", to: "A0/CS"), 1)
    }

    /// Nested shapes: an object inside an object (`bus.cs` as `{pin,
    /// active}`, the first alternative of a `oneOf`), the other alternative
    /// (`bus.cs` as a bare pin), a `.pins` list (`bus.data`), and a `.json`
    /// value, which has no schema and is left.
    func testConverterBusPins() throws {
        var draft = PartDraft(newOf: "converter")
        draft.pinouts[0].pins = [
            PinDraft(designator: "1", name: "CS", role: "input"),
            PinDraft(designator: "2", name: "D0", role: "inout"),
            PinDraft(designator: "3", name: "D1", role: "inout"),
        ]
        draft.kindDevice["bus"] = .object([
            "style": .string("parallel"),
            "cs": .object(["pin": .string("CS"), "active": .number(0)]),
            "data": .array([.string("D0"), .string("D1")]),
            "selectSampled": .string("CS"),
        ])
        XCTAssertEqual(draft.renamePin(from: "CS", to: "nCS"), 1)
        XCTAssertEqual(draft.renamePin(from: "D1", to: "DB1"), 1)
        XCTAssertEqual(draft.kindDevice["bus"], .object([
            "style": .string("parallel"),
            "cs": .object(["pin": .string("nCS"), "active": .number(0)]),
            "data": .array([.string("D0"), .string("DB1")]),
            "selectSampled": .string("CS"),
        ]))

        // The bare-pin shape of the same key.
        draft.pinouts[0].pins[0].name = "nCS"
        draft.kindDevice["bus"] = .object(["style": .string("spi"), "cs": .string("nCS")])
        XCTAssertEqual(draft.renamePin(from: "nCS", to: "SS"), 1)
        XCTAssertEqual(draft.kindDevice["bus"], .object(["style": .string("spi"), "cs": .string("SS")]))
    }

    /// A `.map` (gate timing per family) has no pins: nothing is rewritten,
    /// and nothing else changes.
    func testValuesThatAreNotPinsAreLeft() {
        var draft = gateDraft()
        draft.kindDevice["timing"] = .object(["A": .object(["rise": .number(9), "fall": .number(9)])])
        draft.kindPart["family"] = .string("A")
        XCTAssertEqual(draft.renamePin(from: "A", to: "1A"), 1)
        XCTAssertEqual(draft.kindDevice["timing"], .object(["A": .object(["rise": .number(9), "fall": .number(9)])]))
        XCTAssertEqual(draft.kindPart["family"], .string("A"))
    }
}
