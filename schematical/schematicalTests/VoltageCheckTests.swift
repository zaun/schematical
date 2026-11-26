import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-68-W: the voltage check's issues as values (equal across drawings
/// whatever their net) and the refusal message of each kind.
@MainActor
final class VoltageCheckTests: XCTestCase {
    private func issue(_ kind: VoltageProblemKind, chip: UUID, pin: Int = 0, other: UUID? = nil, net: Int = 0,
                       mv: UInt32, min: UInt32 = 0, max: UInt32 = 0) -> VoltageIssue {
        VoltageIssue(kind: kind.rawValue, chipID: chip, pin: pin, otherChipID: other ?? chip, net: net, mv: mv,
                     minMV: min, maxMV: max)
    }

    func testIssuesAreTheSameProblemWhateverTheirNet() {
        let chip = UUID()
        let a = issue(VOLTAGE_INPUT_OVER, chip: chip, net: 1, mv: 5000, max: 3600)
        let b = issue(VOLTAGE_INPUT_OVER, chip: chip, net: 7, mv: 5000, max: 3600)
        XCTAssertEqual(a, b)
        XCTAssertEqual(Set([a, b]).count, 1)
        XCTAssertNotEqual(a, issue(VOLTAGE_INPUT_OVER, chip: chip, net: 1, mv: 5000, max: 4000))
        XCTAssertNotEqual(a, issue(VOLTAGE_POWER_RANGE, chip: chip, net: 1, mv: 5000, max: 3600))
        XCTAssertEqual(Set([a, issue(VOLTAGE_INPUT_OVER, chip: chip, pin: 2, mv: 5000, max: 3600)]).count, 2)
    }

    func testEachKindsMessage() throws {
        let doc = TestSupport.newDocument()
        let five = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let three = try TestSupport.place(doc, "3V3", at: CGPoint(x: 200, y: 60))
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 300, y: 300))
        let chips = doc.chips
        let pin = gate.part.pins[0]
        let pinText = "pin \(pin.designator) (\(pin.name))"

        // A clash names the lower supply first, whichever of the two the issue is on.
        let onLow = issue(VOLTAGE_SUPPLY_CLASH, chip: three.id, other: five.id, mv: 5000, min: 3300)
        XCTAssertEqual(VoltageCheck.message(onLow, chips: chips),
                       "Can't connect: 3.3 V (\(three.reference)) and 5 V (\(five.reference)) supplies on one net.")
        let onHigh = issue(VOLTAGE_SUPPLY_CLASH, chip: five.id, other: three.id, mv: 5000, min: 3300)
        XCTAssertEqual(VoltageCheck.message(onHigh, chips: chips),
                       "Can't connect: 3.3 V (\(three.reference)) and 5 V (\(five.reference)) supplies on one net.")
        let gone = issue(VOLTAGE_SUPPLY_CLASH, chip: three.id, other: UUID(), mv: 5000, min: 3300)
        XCTAssertEqual(VoltageCheck.message(gone, chips: chips, edit: .turnOn),
                       "Can't turn on: 3.3 V (\(three.reference)) and 5 V (?) supplies on one net.")

        XCTAssertEqual(VoltageCheck.message(issue(VOLTAGE_INPUT_OVER, chip: gate.id, mv: 12000, max: 5500), chips: chips),
                       "Can't connect: \(gate.reference) \(pinText) gets 12 V; \(gate.part.name) inputs take at most 5.5 V.")
        for kind in [VOLTAGE_REGULATOR_INPUT_OVER, VOLTAGE_POWERSWITCH_INPUT_OVER] {
            XCTAssertEqual(VoltageCheck.message(issue(kind, chip: gate.id, mv: 24000, max: 15000), chips: chips),
                           "Can't connect: \(gate.reference) \(pinText) gets 24 V; \(gate.part.name) takes at most 15 V in.")
        }
        XCTAssertEqual(VoltageCheck.message(issue(VOLTAGE_SWITCH_OVER, chip: gate.id, mv: 2970, max: 2500), chips: chips),
                       "Can't connect: \(gate.reference) \(pinText) gets 2.97 V; \(gate.part.name) is rated 2.5 V.")
        XCTAssertEqual(VoltageCheck.message(issue(VOLTAGE_POWER_RANGE, chip: gate.id, pin: 999, mv: 12000, min: 2000, max: 6000),
                                            chips: chips),
                       "Can't connect: 12 V on \(gate.reference) a pin. \(gate.part.name) needs 2–6 V.")
        XCTAssertEqual(VoltageCheck.message(issue(VOLTAGE_INPUT_OVER, chip: UUID(), mv: 1), chips: chips), "Can't connect.")
    }
}
