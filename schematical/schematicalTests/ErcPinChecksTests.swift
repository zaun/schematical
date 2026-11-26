import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-63-E phase 2: the ERC's pin rules on a drawing (`erc_check` through
/// `SchematicDocument.electricalRulesCheck`).
@MainActor
final class ErcPinChecksTests: XCTestCase {
    private func rules(_ violations: [ErcViolation]) -> [ErcRule] { violations.map(\.rule) }

    func testAnUnwiredInputIsNotConnected() throws {
        let doc = try ErcCases.pinNotConnected()
        let found = doc.electricalRulesCheck()
        let unconnected = found.filter { $0.rule == .pinNotConnected }
        // The gate has 13 pins (all but GND... every pin is unwired): 14 pins, all not connected.
        XCTAssertEqual(unconnected.count, 14, "\(found)")
        XCTAssertTrue(unconnected.allSatisfy { $0.severity == .error && $0.message == "Pin not connected" })
        XCTAssertEqual(unconnected.first?.index, 1)
        XCTAssertEqual(found.map(\.index), Array(1...found.count), "numbered from 1")
        let chip = try XCTUnwrap(doc.chips.first)
        XCTAssertEqual(unconnected.first?.items, [.chip(chip.id)])
        XCTAssertTrue(unconnected.first?.reference.hasPrefix("\(chip.reference).") == true)
    }

    func testTwoOutputsOnOneNetConflict() throws {
        let doc = try ErcCases.pinConflict()
        let conflicts = doc.electricalRulesCheck().filter { $0.rule == .pinToPin }
        XCTAssertEqual(conflicts.count, 1)
        XCTAssertEqual(conflicts.first?.severity, .error)
        XCTAssertEqual(conflicts.first?.message, "Pins of type Output and Output are connected")
        XCTAssertEqual(conflicts.first?.items.count, 2)
    }

    func testAnInputNotDrivenAndAPowerPinNotDriven() throws {
        // Two inputs wired together: one violation, on the first (U1.1); every unwired input
        // is a net of its own and is reported too (16 inputs, 14 alone, 1 net).
        let inputs = try ErcCases.pinNotDriven().electricalRulesCheck()
        let undriven = inputs.filter { $0.rule == .pinNotDriven }
        XCTAssertEqual(undriven.count, 15)
        XCTAssertEqual(undriven.filter { $0.reference == "U1.1" }.count, 1)
        XCTAssertTrue(undriven.filter { $0.reference == "U2.1" }.isEmpty, "one per net, on the first pin")
        XCTAssertEqual(undriven.first?.message, "Input pin not driven by any Output pins")

        // VCC (pin 14) wired to a header pin: the net has no driver.
        let power = try ErcCases.powerPinNotDriven().electricalRulesCheck().filter { $0.rule == .powerPinNotDriven }
        XCTAssertEqual(power.filter { $0.reference == "U1.14" }.count, 1)
        XCTAssertEqual(power.first { $0.reference == "U1.14" }?.message,
                       "Input Power pin not driven by any Output Power pins")
    }

    func testARailSymbolDrivesItsNetAndAPowerFlagClearsTheRest() throws {
        let doc = ErcCases.newDocument()
        let gate = try ErcCases.place(doc, "SN74HC00N", 200, 200)
        let rail = try ErcCases.place(doc, "5V", 600, 100)
        ErcCases.wire(doc, .pin(try ErcCases.pin(gate, "VCC")), .pin(try ErcCases.pin(rail, "VCC")))
        XCTAssertTrue(doc.electricalRulesCheck().filter { $0.rule == .powerPinNotDriven && $0.reference == "U1.14" }.isEmpty,
                      "a rail symbol is a driver")

        // A header pin feeding the gate's VCC needs a flag.
        let other = ErcCases.newDocument()
        let g2 = try ErcCases.place(other, "SN74HC00N", 200, 200)
        let header = try ErcCases.place(other, "GENERIC-HEADER-1X02", 700, 200)
        ErcCases.wire(other, .pin(try ErcCases.pin(g2, "VCC")), .pin(try ErcCases.pin(header, "1")))
        XCTAssertEqual(other.electricalRulesCheck().filter { $0.rule == .powerPinNotDriven && $0.reference == "U1.14" }.count, 1)
        let flag = other.addNetLabel(at: CGPoint(x: 700, y: 500), style: .powerFlag)
        ErcCases.wire(other, .pin(try ErcCases.pin(header, "1")), .label(flag))
        let after = other.electricalRulesCheck()
        XCTAssertTrue(after.filter { $0.rule == .powerPinNotDriven && $0.reference == "U1.14" }.isEmpty, "the flag drives the net")
        XCTAssertTrue(after.filter { $0.reference == "Power Flag" }.isEmpty, "a wired flag is connected")
    }

    func testANoConnectClearsAnUnusedPin() throws {
        let doc = ErcCases.newDocument()
        let gate = try ErcCases.place(doc, "SN74HC00N", 200, 200)
        let before = doc.electricalRulesCheck().filter { $0.rule == .pinNotConnected }.count
        let flag = doc.addNetLabel(at: CGPoint(x: 100, y: 500), style: .noConnect)
        ErcCases.wire(doc, .pin(try ErcCases.pin(gate, "4B")), .label(flag))
        let after = doc.electricalRulesCheck()
        XCTAssertEqual(after.filter { $0.rule == .pinNotConnected }.count, before - 1)
        XCTAssertTrue(after.filter { $0.rule == .noConnectConnected }.isEmpty)
    }

    func testANoConnectOnAWiredPinIsReported() throws {
        let found = try ErcCases.noConnectConnected().electricalRulesCheck().filter { $0.rule == .noConnectConnected }
        XCTAssertEqual(found.count, 1)
        XCTAssertEqual(found.first?.severity, .warning)
        XCTAssertEqual(found.first?.message, "A pin with a \"no connection\" flag is connected")
    }

    func testSettingsChangeWhatIsReported() throws {
        let doc = try ErcCases.pinConflict()
        XCTAssertEqual(doc.electricalRulesCheck().filter { $0.rule == .pinToPin }.count, 1)
        // The pin map: output + output as a warning, then off.
        doc.setErcPinMap(1, 1, to: 1)
        XCTAssertEqual(doc.electricalRulesCheck().first { $0.rule == .pinToPin }?.severity, .warning)
        doc.setErcPinMap(1, 1, to: 0)
        XCTAssertTrue(doc.electricalRulesCheck().filter { $0.rule == .pinToPin }.isEmpty)
        // A severity: ignore drops the rule; warning changes it.
        doc.setErcSeverity(.pinNotConnected, .warning)
        XCTAssertTrue(doc.electricalRulesCheck().filter { $0.rule == .pinNotConnected }.allSatisfy { $0.severity == .warning })
        doc.setErcSeverity(.pinNotConnected, .ignore)
        XCTAssertTrue(doc.electricalRulesCheck().filter { $0.rule == .pinNotConnected }.isEmpty)
    }

    func testAnExclusionHidesOneViolationAndKeepsTheNext() throws {
        let doc = try ErcCases.pinNotConnected()
        let all = doc.electricalRulesCheck()
        let first = try XCTUnwrap(all.first)
        doc.excludeErc([first.key])
        let shown = doc.electricalRulesCheck()
        XCTAssertEqual(shown.count, all.count - 1)
        XCTAssertFalse(shown.contains { $0.key == first.key })
        let withExcluded = doc.electricalRulesCheck(includeExcluded: true)
        XCTAssertEqual(withExcluded.count, all.count)
        XCTAssertEqual(withExcluded.filter(\.isExcluded).map(\.key), [first.key])
        // The key does not depend on the rest of the drawing.
        _ = try ErcCases.place(doc, "SN74HC00N", 900, 600)
        XCTAssertTrue(doc.electricalRulesCheck().allSatisfy { $0.key != first.key })
        doc.includeErc([first.key])
        XCTAssertTrue(doc.electricalRulesCheck().contains { $0.key == first.key })
    }

    func testErrorsComeFirst() throws {
        let doc = try ErcCases.noConnectConnected()
        let found = doc.electricalRulesCheck()
        let severities = found.map(\.severity)
        XCTAssertEqual(severities, severities.sorted { $0 == .error && $1 != .error })
    }
}
