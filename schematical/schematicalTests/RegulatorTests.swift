import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-42-I: regulator-kind parts in the app. A regulator takes the next U,
/// draws as a block, and the sidebar names the kind ("Dual voltage
/// regulator" …) and its facts; the voltage check and play mode read a
/// placed GENERIC-RESISTOR's own value, so an adjustable regulator's output
/// follows the divider the user drew.
@MainActor
final class RegulatorTests: XCTestCase {
    private func row(_ chip: PlacedChip, _ label: String) -> String? {
        PartFacts.rows(for: chip).first { $0.label == label }?.value
    }

    private func place(_ doc: SchematicDocument, _ name: String, _ x: CGFloat, _ y: CGFloat) throws -> PlacedChip {
        try TestSupport.place(doc, name, at: CGPoint(x: x, y: y))
    }

    // MARK: - Facts, kind names, labels

    /// GENERIC-REG-3V3 is U1 on TO-220-3 with two footprint choices, drawn as
    /// a block, named "Voltage regulator".
    func testPlacingTheGenericRegulator() throws {
        let doc = TestSupport.newDocument()
        let u = try place(doc, "GENERIC-REG-3V3", 100, 100)
        XCTAssertEqual(u.reference, "U1")
        XCTAssertEqual(u.part.package, "TO-220")
        XCTAssertEqual(u.footprintID, "TO-220-3")
        XCTAssertEqual(u.part.footprintChoices.map(\.package), ["TO-220", "DPAK"])
        XCTAssertEqual(u.part.footprintChoices.map(\.footprint.id), ["TO-220-3", "DPAK-3"])
        XCTAssertEqual(u.symbol, .block)
        XCTAssertEqual(symbol(for: u.part), .block)
        XCTAssertEqual(PartFacts.kindName(u.part), "Voltage regulator")
        XCTAssertEqual(row(u, "Type"), "Linear")
        XCTAssertEqual(row(u, "Output"), "3.3 V")
        XCTAssertEqual(row(u, "Dropout"), "2 V")
        XCTAssertNil(row(u, "Input"), "a generic has no input rating")
        XCTAssertEqual(u.part.detailLabels, ["3.3 V"])
        XCTAssertEqual(ReferenceCategory.of(u.part), .integratedCircuit)
        for name in ["GENERIC-REG-5V", "GENERIC-REG-ADJ"] {
            XCTAssertEqual(try TestSupport.part(name).footprintChoices.map(\.footprint.id), ["TO-220-3", "DPAK-3"], name)
        }
    }

    /// A fixed linear part: LM1117MPX-3.3/NOPB.
    func testFixedLinearFacts() throws {
        let doc = TestSupport.newDocument()
        let u = try place(doc, "LM1117MPX-3.3/NOPB", 100, 100)
        XCTAssertEqual(PartFacts.kindName(u.part), "Voltage regulator")
        XCTAssertEqual(row(u, "Type"), "Linear")
        XCTAssertNil(row(u, "Regulators"))
        XCTAssertEqual(row(u, "Output"), "3.3 V")
        XCTAssertEqual(row(u, "Input"), "up to 15 V")
        XCTAssertEqual(row(u, "Dropout"), "1.2 V")
        XCTAssertEqual(row(u, "Current"), "0.8 A")
        XCTAssertEqual(row(u, "Quiescent"), "5 mA")
        XCTAssertNil(row(u, "Switching"))
        XCTAssertEqual(u.part.detailLabels, ["3.3 V", "0.8 A"])
        XCTAssertEqual(u.symbol, .block)
    }

    /// An adjustable part: "Adjustable, Vref 1.25 V".
    func testAdjustableFacts() throws {
        let doc = TestSupport.newDocument()
        let u = try place(doc, "GENERIC-REG-ADJ", 100, 100)
        XCTAssertEqual(row(u, "Output"), "Adjustable, Vref 1.25 V")
        XCTAssertEqual(u.part.detailLabels, ["ADJ"])
        // A catalog adjustable part names its highest output.
        let part = try XCTUnwrap(TestSupport.allParts.first { part in
            guard let info = part.regulator else { return false }
            return info.adjustable && info.regulators == 1 && info.voutMaxMV > 0 && info.ioutMA > 0
        }, "no adjustable regulator with a top output")
        let chip = try place(doc, part.name, 300, 100)
        let info = try XCTUnwrap(part.regulator)
        XCTAssertTrue(try XCTUnwrap(row(chip, "Output")).hasPrefix("Adjustable, Vref "))
        XCTAssertTrue(try XCTUnwrap(row(chip, "Output")).hasSuffix("(to \(PartFacts.milli(info.voutMaxMV)) V)"))
        XCTAssertEqual(part.detailLabels.first, "ADJ")
    }

    /// A switching part: LM2576HVS-3.3 (Buck, 6–60 V, 3 A, 52 kHz).
    func testSwitchingFacts() throws {
        let doc = TestSupport.newDocument()
        let u = try place(doc, "LM2576HVS-3.3", 100, 100)
        XCTAssertEqual(row(u, "Type"), "Buck")
        XCTAssertEqual(row(u, "Output"), "3.3 V")
        XCTAssertEqual(row(u, "Input"), "6–60 V")
        XCTAssertNil(row(u, "Dropout"), "a switching part has no dropout row")
        XCTAssertEqual(row(u, "Current"), "3 A")
        XCTAssertEqual(row(u, "Switching"), "52 kHz")
    }

    /// A dual part: ADP222ACPZ-1825-R7 (1.8 V and 2.5 V).
    func testDualFacts() throws {
        let doc = TestSupport.newDocument()
        let u = try place(doc, "ADP222ACPZ-1825-R7", 100, 100)
        XCTAssertEqual(PartFacts.kindName(u.part), "Dual voltage regulator")
        XCTAssertEqual(row(u, "Regulators"), "2")
        XCTAssertEqual(row(u, "Output"), "1.8 V / 2.5 V")
        XCTAssertEqual(row(u, "Input"), "2.5–5.5 V")
        XCTAssertEqual(row(u, "Dropout"), "0.26 V")
        XCTAssertEqual(row(u, "Current"), "0.3 A")
        XCTAssertEqual(row(u, "Quiescent"), "150 µA")
        XCTAssertEqual(u.part.detailLabels, ["1.8/2.5 V", "0.3 A"])
        // A triple or quad part is named by its count, picked by its facts.
        for (count, name) in [(3, "Triple voltage regulator"), (4, "Quad voltage regulator")] {
            if let part = TestSupport.allParts.first(where: { $0.regulator?.regulators == count }) {
                XCTAssertEqual(PartFacts.kindName(part), name)
            }
        }
    }

    // MARK: - The divider through the app's netlist

    private struct Divider {
        let doc: SchematicDocument
        let reg: PlacedChip
        let r1: PlacedChip
        let r2: PlacedChip
        let rail: PlacedChip
        let gnd: PlacedChip
    }

    /// 5 V → IN; R1 from OUT to ADJ, R2 from ADJ to GND (R1 = `r1`, R2 = `r2`).
    private func divider(r1 r1Ohms: Double, r2 r2Ohms: Double) throws -> Divider {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let reg = try place(doc, "GENERIC-REG-ADJ", 300, 300)
        let r1 = try place(doc, "GENERIC-RESISTOR", 500, 200)
        let r2 = try place(doc, "GENERIC-RESISTOR", 500, 400)
        // 12 V: a regulator regulates only with its dropout of headroom (2 V
        // for a generic), which 5 V in does not give for a 5 V out.
        let rail = try place(doc, "12V", 100, 200)
        let gnd = try place(doc, "GND", 500, 600)
        doc.setPartOhms(r1.id, r1Ohms)
        doc.setPartOhms(r2.id, r2Ohms)
        func pin(_ chip: PlacedChip, _ name: String) throws -> LineAnchor {
            .pin(try TestSupport.pin(doc.chips.first { $0.id == chip.id } ?? chip, name))
        }
        XCTAssertNil(doc.connect(try pin(rail, "VCC"), try pin(reg, "IN")))
        XCTAssertNil(doc.connect(try pin(reg, "OUT"), try pin(r1, "1")))
        XCTAssertNil(doc.connect(try pin(reg, "ADJ"), try pin(r1, "2")))
        XCTAssertNil(doc.connect(try pin(reg, "ADJ"), try pin(r2, "1")))
        XCTAssertNil(doc.connect(try pin(r2, "2"), try pin(gnd, "GND")))
        return Divider(doc: doc, reg: reg, r1: r1, r2: r2, rail: rail, gnd: gnd)
    }

    /// The message of connecting `part`'s VCC to the regulator's OUT.
    private func refusal(_ d: Divider, _ part: String) throws -> String? {
        let load = try place(d.doc, part, 700, 300)
        let vcc = try TestSupport.pin(load, "VCC")
        return d.doc.connect(.pin(vcc), .pin(try TestSupport.pin(d.doc.chips.first { $0.id == d.reg.id } ?? d.reg, "OUT")))
    }

    /// 240 Ω / 720 Ω gives Vref × (1 + R2/R1) = 5 V on OUT: a 0.8–2.7 V
    /// part there is refused with "5 V"; changing R2 to 390 Ω gives 3.28 V,
    /// the new value (the part's own 1000 Ω would give 6.5 V).
    func testTheDividerSetsTheOutput() throws {
        let five = try divider(r1: 240, r2: 720)
        let first = try XCTUnwrap(refusal(five, "SN74AUC1G04DBVR"))
        XCTAssertTrue(first.hasPrefix("Can't connect"), first)
        XCTAssertTrue(first.contains("5 V on"), first)
        XCTAssertTrue(first.contains("needs 0.8–2.7 V"), first)

        let lower = try divider(r1: 240, r2: 390)
        let second = try XCTUnwrap(refusal(lower, "SN74AUC1G04DBVR"))
        XCTAssertTrue(second.contains("3.28 V on"), second)

        // Changing a placed resistor after the fact moves the level too.
        five.doc.setPartOhms(five.r2.id, 390)
        let third = try XCTUnwrap(refusal(five, "SN74AUC1G04DCKR"))
        XCTAssertTrue(third.contains("3.28 V on"), third)

        // A 1.65–5.5 V part takes either level.
        XCTAssertNil(try refusal(try divider(r1: 240, r2: 720), "SN74LVC1G04DBVR"))
        XCTAssertNil(try refusal(try divider(r1: 240, r2: 390), "SN74LVC1G04DBVR"))
    }

    /// The netlist carries a generic resistor's value only (a pull's stays out).
    func testTheNetlistCarriesGenericResistors() throws {
        let d = try divider(r1: 240, r2: 720)
        let netlist = NetlistBuilder.build(chips: d.doc.chips, wires: d.doc.wires, buses: d.doc.buses,
                                           labels: d.doc.netLabels)
        XCTAssertEqual(netlist.ohms.count, d.doc.chips.count)
        let byName = Dictionary(uniqueKeysWithValues: zip(d.doc.chips.map(\.id), netlist.ohms))
        XCTAssertEqual(byName[d.r1.id] ?? nil, 240)
        XCTAssertEqual(byName[d.r2.id] ?? nil, 720)
        XCTAssertNil(byName[d.reg.id] ?? nil)
        XCTAssertNil(byName[d.rail.id] ?? nil)
    }

    // MARK: - The messages

    /// A supply above a part's input maximum on its VIN is refused with it
    /// (the library's supplies are 3V3, 5V, 12V, 170V, 200V, …: 24 V is not
    /// one, so 170 V stands in for the plan's 24 V).
    func testInputOverMessage() throws {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let reg = try place(doc, "LM1117MPX-3.3/NOPB", 300, 300)
        let supply = try place(doc, "170V", 100, 300)
        let message = try XCTUnwrap(doc.connect(.pin(try TestSupport.pin(supply, "VCC")),
                                                .pin(try TestSupport.pin(reg, "VIN"))))
        XCTAssertTrue(message.hasPrefix("Can't connect: U1 pin 3 (VIN) gets 170 V;"), message)
        XCTAssertTrue(message.hasSuffix("LM1117MPX-3.3/NOPB takes at most 15 V in."), message)
    }

    /// A 3.3 V regulator output on the 5 V rail: the lower voltage first.
    func testSupplyClashMessage() throws {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let reg = try place(doc, "GENERIC-REG-3V3", 300, 300)
        let twelve = try place(doc, "12V", 100, 300)
        let inPin = try TestSupport.pin(reg, "IN")
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(twelve, "VCC")), .pin(inPin)))
        let other = try place(doc, "5V", 500, 300)  // the "5 V (…)" of the message
        let message = try XCTUnwrap(doc.connect(.pin(try TestSupport.pin(other, "VCC")),
                                                .pin(try TestSupport.pin(reg, "OUT"))))
        XCTAssertTrue(message.hasPrefix("Can't connect: 3.3 V (U1) and 5 V ("), message)
        XCTAssertTrue(message.hasSuffix(") supplies on one net."), message)
    }
}
