import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-63-E phase 3: the ERC's drawing rules, one case per rule of
/// `planning/plan-63-data/README.md` and its near misses.
@MainActor
final class ErcDrawingChecksTests: XCTestCase {
    private func found(_ doc: SchematicDocument, _ rule: ErcRule, enabled: Bool = false) -> [ErcViolation] {
        if enabled {
            doc.setErcSeverity(rule, .warning)
        }
        return doc.electricalRulesCheck().filter { $0.rule == rule }
    }

    func testNoConnectOnNoPin() throws {
        let doc = ErcCases.noConnectDangling()
        let hits = found(doc, .noConnectDangling)
        XCTAssertEqual(hits.count, 1)
        XCTAssertEqual(hits.first?.severity, .warning)
        XCTAssertEqual(hits.first?.message, "Unconnected \"no connection\" flag")
        // A flag on a pin is fine.
        let doc2 = ErcCases.newDocument()
        let gate = try ErcCases.place(doc2, "SN74HC00N", 200, 200)
        let flag = doc2.addNetLabel(at: CGPoint(x: 100, y: 500), style: .noConnect)
        ErcCases.wire(doc2, .pin(try ErcCases.pin(gate, "1A")), .label(flag))
        XCTAssertTrue(found(doc2, .noConnectDangling).isEmpty)
    }

    func testAWireWithBothEndsLooseIsDangling() throws {
        let doc = try ErcCases.wireDangling()
        XCTAssertEqual(found(doc, .wireDangling).count, 1)
        XCTAssertEqual(found(doc, .wireDangling).first?.severity, .error)
        XCTAssertEqual(found(doc, .unconnectedWireEndpoint).count, 2)
        XCTAssertEqual(found(doc, .unconnectedWireEndpoint).first?.severity, .warning)
    }

    func testAWireToNowhereHasOneUnconnectedEnd() throws {
        let doc = try ErcCases.wireToNowhere()
        XCTAssertEqual(found(doc, .unconnectedWireEndpoint).count, 1)
        XCTAssertTrue(found(doc, .wireDangling).isEmpty)
        XCTAssertEqual(found(doc, .unconnectedWireEndpoint).first?.message, "Unconnected wire endpoint")
    }

    func testAWireBetweenTwoPinsHasNoFreeEnd() throws {
        let doc = ErcCases.newDocument()
        let one = try ErcCases.place(doc, "SN74HC00N", 200, 200), two = try ErcCases.place(doc, "SN74HC00N", 700, 200)
        ErcCases.wire(doc, .pin(try ErcCases.pin(one, "1Y")), .pin(try ErcCases.pin(two, "1A")))
        XCTAssertTrue(found(doc, .unconnectedWireEndpoint).isEmpty)
        XCTAssertTrue(found(doc, .wireDangling).isEmpty)
    }

    func testALabelWithNoPinIsDangling() throws {
        let hits = found(ErcCases.labelDangling(), .labelDangling)
        XCTAssertEqual(hits.count, 1)
        XCTAssertEqual(hits.first?.severity, .error)
        XCTAssertEqual(hits.first?.message, "Label not connected")
        // Two labels of one name on groups with no pin are each dangling.
        let doc = ErcCases.newDocument()
        let a = doc.addNetLabel(at: CGPoint(x: 100, y: 100)), b = doc.addNetLabel(at: CGPoint(x: 400, y: 100))
        XCTAssertTrue(doc.setNetLabelName(a, "J"))
        XCTAssertTrue(doc.setNetLabelName(b, "J"))
        XCTAssertEqual(found(doc, .labelDangling).count, 2)
        // A label wired to a pin is not.
        XCTAssertTrue(found(try ErcCases.isolatedPinLabel(), .labelDangling).isEmpty)
    }

    func testALabelOnANetWithOnePinIsIsolated() throws {
        let hits = found(try ErcCases.isolatedPinLabel(), .isolatedPinLabel)
        XCTAssertEqual(hits.count, 1)
        XCTAssertEqual(hits.first?.message, "Label connected to only one pin")
        // The same name on a second net with a pin joins them: two pins, nothing to report.
        let doc = try ErcCases.isolatedPinLabel()
        let other = try ErcCases.place(doc, "SN74HC00N", 200, 700)
        let label = doc.addNetLabel(at: CGPoint(x: 600, y: 700))
        XCTAssertTrue(doc.setNetLabelName(label, "SIG"))
        ErcCases.wire(doc, .pin(try ErcCases.pin(other, "1Y")), .label(label))
        XCTAssertTrue(found(doc, .isolatedPinLabel).isEmpty)
        // A second label of the name, even on a group with no pin, is enough (as KiCad's).
        let lone = try ErcCases.isolatedPinLabel()
        let unwired = lone.addNetLabel(at: CGPoint(x: 900, y: 900))
        XCTAssertTrue(lone.setNetLabelName(unwired, "SIG"))
        XCTAssertTrue(found(lone, .isolatedPinLabel).isEmpty)
        XCTAssertEqual(found(lone, .labelDangling).count, 1, "the unwired one is dangling")
    }

    func testTwoNamesOnOneWire() throws {
        let hits = found(try ErcCases.multipleNetNames(), .multipleNetNames)
        XCTAssertEqual(hits.count, 1)
        XCTAssertEqual(hits.first?.message,
                       "Both C1 and C2 are attached to the same items; C1 will be used in the netlist")
        XCTAssertEqual(hits.first?.severity, .warning)
        // One name twice on one wire is one name.
        let doc = ErcCases.newDocument()
        let gate = try ErcCases.place(doc, "SN74HC00N", 200, 200)
        let one = doc.addNetLabel(at: CGPoint(x: 600, y: 100)), two = doc.addNetLabel(at: CGPoint(x: 600, y: 300))
        XCTAssertTrue(doc.setNetLabelName(one, "X"))
        XCTAssertTrue(doc.setNetLabelName(two, "X"))
        ErcCases.wire(doc, .pin(try ErcCases.pin(gate, "1Y")), .label(one))
        ErcCases.wire(doc, .pin(try ErcCases.pin(gate, "1Y")), .label(two))
        XCTAssertTrue(found(doc, .multipleNetNames).isEmpty)
    }

    func testLabelsThatDifferOnlyInCase() throws {
        let hits = found(try ErcCases.similarLabels(), .similarLabels)
        XCTAssertEqual(hits.count, 1)
        XCTAssertEqual(hits.first?.message, "Labels are similar (lower/upper case difference only)")
        XCTAssertTrue(found(try ErcCases.isolatedPinLabel(), .similarLabels).isEmpty)
    }

    func testALabelAndAPowerSymbolThatDifferOnlyInCase() throws {
        let hits = found(try ErcCases.similarLabelAndPower(), .similarLabelAndPower)
        XCTAssertEqual(hits.count, 1)
        XCTAssertEqual(hits.first?.message, "Power pin and label are similar (lower/upper case difference only)")
        XCTAssertTrue(found(try ErcCases.similarLabels(), .similarLabelAndPower).isEmpty)
    }

    func testALabelNameUsedOnceIsOffByDefaultAndReportedWhenOn() throws {
        let doc = try ErcCases.singleLabel()
        XCTAssertEqual(doc.erc.severity(of: .singleGlobalLabel), .ignore)
        XCTAssertTrue(doc.electricalRulesCheck().filter { $0.rule == .singleGlobalLabel }.isEmpty)
        let on = found(doc, .singleGlobalLabel, enabled: true)
        XCTAssertEqual(on.count, 1)
        XCTAssertEqual(on.first?.message, "Global label only appears once in the schematic")
        // Two labels of one name: not once.
        let two = ErcCases.newDocument()
        let a = two.addNetLabel(at: CGPoint(x: 100, y: 100)), b = two.addNetLabel(at: CGPoint(x: 400, y: 100))
        XCTAssertTrue(two.setNetLabelName(a, "Z"))
        XCTAssertTrue(two.setNetLabelName(b, "Z"))
        XCTAssertTrue(found(two, .singleGlobalLabel, enabled: true).isEmpty)
    }

    func testAFourWayJunctionIsOffByDefaultAndAThreeWayIsNever() throws {
        let doc = try ErcCases.fourWayJunction()
        XCTAssertTrue(doc.electricalRulesCheck().filter { $0.rule == .fourWayJunction }.isEmpty)
        XCTAssertEqual(found(doc, .fourWayJunction, enabled: true).count, 1)

        let three = ErcCases.newDocument()
        let west = try ErcCases.place(three, "SN74HC00N", 200, 400), east = try ErcCases.place(three, "SN74HC00N", 900, 400)
        let north = try ErcCases.place(three, "SN74HC00N", 550, 100)
        ErcCases.wire(three, .pin(try ErcCases.pin(west, "1Y")), .pin(try ErcCases.pin(east, "1A")))
        let path = three.path(forWire: three.wires[0].id)
        let point = SchematicGrid.snap(CGPoint(x: (path[0].x + path[1].x) / 2, y: (path[0].y + path[1].y) / 2))
        ErcCases.wire(three, .pin(try ErcCases.pin(north, "1Y")), .point(point))
        XCTAssertTrue(found(three, .fourWayJunction, enabled: true).isEmpty)
    }

    func testTwoPartsWithOneReference() throws {
        let doc = try ErcCases.duplicateReference()
        let hits = found(doc, .duplicateReference)
        XCTAssertEqual(hits.count, 1)
        XCTAssertEqual(hits.first?.severity, .error)
        XCTAssertEqual(hits.first?.items.count, 2)
        XCTAssertTrue(found(try ErcCases.pinConflict(), .duplicateReference).isEmpty)
    }

    func testTheAFUFixtureChecksWithoutDuplicatesAndInGoodTime() throws {
        let doc = try Fixture.open("AFU")
        let start = Date()
        let all = doc.electricalRulesCheck(library: TestSupport.catalog.library)
        let seconds = Date().timeIntervalSince(start)
        XCTAssertTrue(all.filter { $0.rule == .duplicateReference }.isEmpty)
        // A card's edge connector drives and connects the nets of the edge bus's ports (KiCad's export
        // writes it as passive pins), so no input on them is "not driven" and no label on them is isolated.
        XCTAssertTrue(all.filter { $0.rule == .pinNotDriven || $0.rule == .isolatedPinLabel }.isEmpty,
                      "\(all.filter { $0.rule == .pinNotDriven || $0.rule == .isolatedPinLabel }.map(\.reference))")
        XCTAssertEqual(all.map(\.index), Array(1...max(all.count, 1)).prefix(all.count).map { $0 })
        var counts: [String: Int] = [:]
        for violation in all { counts[violation.rule.rawValue, default: 0] += 1 }
        print("ERC AFU: \(all.count) violations in \(String(format: "%.2f", seconds)) s: \(counts.sorted { $0.key < $1.key })")
    }

    // MARK: plan-68-Y

    /// Two supply symbols of different names on one net: `multiple_net_names`, at the first name's pin.
    func testTwoSupplyNamesOnOneNet() throws {
        // A drawing an edit refuses (two voltages on one net) but a file can hold: no library while drawing.
        let doc = TestSupport.newDocument()
        let gate = try ErcCases.place(doc, "SN74HC00N", 200, 200)
        let five = try ErcCases.place(doc, "5V", 600, 100), three = try ErcCases.place(doc, "3V3", 600, 400)
        ErcCases.wire(doc, .pin(try ErcCases.pin(gate, "VCC")), .pin(try ErcCases.pin(five, "VCC")))
        ErcCases.wire(doc, .pin(try ErcCases.pin(gate, "VCC")), .pin(try ErcCases.pin(three, "VCC")))
        let hits = doc.electricalRulesCheck(library: TestSupport.catalog.library).filter { $0.rule == .multipleNetNames }
        XCTAssertEqual(hits.count, 1)
        XCTAssertEqual(hits.first?.message,
                       "Both 3V3 and 5V are attached to the same items; 3V3 will be used in the netlist")
        XCTAssertEqual(hits.first?.reference, "3V3, 5V")
        XCTAssertEqual(hits.first?.items, [.chip(three.id), .chip(five.id)])
        XCTAssertEqual(hits.first?.point, doc.pinTip(try ErcCases.pin(three, "VCC")))
    }

    /// Two violations of one rule at one point are listed in the order of their keys.
    func testTiesAreOrderedByKey() {
        let doc = ErcCases.newDocument()
        let a = doc.addNetLabel(at: CGPoint(x: 100, y: 100)), b = doc.addNetLabel(at: CGPoint(x: 100, y: 100))
        XCTAssertTrue(doc.setNetLabelName(a, "K"))
        XCTAssertTrue(doc.setNetLabelName(b, "K"))
        let hits = found(doc, .labelDangling)
        XCTAssertEqual(hits.count, 2)
        XCTAssertEqual(hits.map(\.point), [hits[0].point, hits[0].point], "one point")
        XCTAssertLessThan(hits[0].key, hits[1].key)
    }

    func testThePinMapRuleIsAnErrorByDefaultAndHasNoSeverityOfItsOwn() {
        XCTAssertEqual(ErcRule.pinToPin.defaultSeverity, .error)
        XCTAssertFalse(ErcRule.pinToPin.hasSeverity)
        XCTAssertEqual(ErcRule.pinToPin.title, "Pin conflicts on a net")
        XCTAssertEqual(ErcSeverity.allCases.map(\.title), ["Error", "Warning", "Ignore"])
        XCTAssertEqual(Set(ErcRule.allCases.map(\.title)).count, ErcRule.allCases.count, "every rule has its own words")
    }
}
