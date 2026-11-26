import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-68-W: the Details rows of every part in the catalog, each as the
/// sidebar shows it: the overview first, no row left empty, and every
/// kind's facts named.
@MainActor
final class PartFactsSweepTests: XCTestCase {
    private func chip(_ part: CatalogPart) -> PlacedChip {
        PlacedChip(reference: "U1", part: part, symbol: symbol(for: part), origin: .zero)
    }

    func testEveryPartsRows() throws {
        let parts = TestSupport.allParts
        XCTAssertGreaterThan(parts.count, 1000)
        var labels = Set<String>()
        for part in parts {
            let rows = PartFacts.rows(for: chip(part))
            XCTAssertEqual(rows.first?.label, "Kind", part.name)
            XCTAssertEqual(rows.first?.value, PartFacts.kindName(part), part.name)
            XCTAssertTrue(rows.allSatisfy { !$0.value.isEmpty }, "\(part.name): \(rows)")
            let pins = rows.first { $0.label == "Pins" }?.value
            XCTAssertEqual(pins, String(part.pins.filter { !$0.isHidden }.count), part.name)
            labels.formUnion(rows.map(\.label))
        }
        for label in ["Function", "Package", "Footprint", "Family", "Size", "Positions", "Color"] {
            XCTAssertTrue(labels.contains(label), "some part has \(label)")
        }
    }

    /// Each kind's facts accessor answers for its own kind only, and for
    /// some part of it; the drawer's labels and the voltage label are never
    /// empty text.
    func testEveryPartsAccessors() {
        let parts = TestSupport.allParts
        func check(_ name: String, _ kinds: [PartKind], _ has: (CatalogPart) -> Bool) {
            let with = parts.filter(has)
            XCTAssertFalse(with.isEmpty, "some part has \(name)")
            for part in with { XCTAssertTrue(kinds.contains(part.kind), "\(part.name) has \(name)") }
        }
        check("transistor", [PART_TRANSISTOR]) { $0.transistor != nil }
        check("regulator", [PART_REGULATOR]) { $0.regulator != nil }
        check("powerswitch", [PART_POWERSWITCH]) { $0.powerswitch != nil }
        check("crystal", [PART_CRYSTAL]) { $0.crystal != nil && $0.crystalHz != nil }
        check("inductor", [PART_INDUCTOR]) { $0.inductor != nil }
        check("fuse", [PART_FUSE]) { $0.fuse != nil }
        check("relay", [PART_RELAY]) { $0.relay != nil }
        check("buzzer", [PART_BUZZER]) { $0.buzzer != nil }
        check("timer", [PART_TIMER]) { $0.timer != nil }
        check("opamp", [PART_OPAMP]) { $0.opamp != nil }
        check("resistance", [PART_RESISTOR]) { $0.resistorOhms != nil }
        check("capacitance", [PART_CAPACITOR]) { $0.capacitorFarads != nil }
        check("a terminal", [PART_SERIAL]) { $0.hasTerminal }
        check("a memory size", [PART_SRAM, PART_EEPROM]) { $0.memorySize != nil }
        check("a logic family", [PART_GATE, PART_BUFFER, PART_DECODER, PART_MUX, PART_DFF, PART_JKFF, PART_LATCH,
                                 PART_COUNTER, PART_SHIFT, PART_REGISTER, PART_ARITH]) { $0.logicFamily != nil }
        check("a display", [PART_7SEG, PART_LCD]) { $0.display != nil }
        check("a nixie", [PART_NIXIE]) { $0.nixie != nil }
        check("a dekatron", [PART_DEKATRON]) { $0.dekatron != nil }
        check("a character LCD", [PART_CHARLCD]) { $0.characterLCD != nil }
        check("a header size", [PART_HEADER]) { $0.headerSize != nil }
        check("genders", [PART_HEADER]) { !$0.genders.isEmpty }
        check("normally closed", [PART_SWITCH]) { $0.isNormallyClosed }
        for part in parts {
            if let label = part.voltageLabel { XCTAssertFalse(label.isEmpty, part.name) }
            if let label = part.trailingLabel { XCTAssertFalse(label.isEmpty, part.name) }
            XCTAssertFalse(part.detailLabels.contains(""), "\(part.name): \(part.detailLabels)")
            XCTAssertFalse(part.drawerLeaf.isEmpty, part.name)
            XCTAssertEqual(part.id, part.name)
            XCTAssertEqual(part.listed(in: "Elsewhere").drawer.first, part.drawer.isEmpty ? nil : "Elsewhere")
        }
        XCTAssertEqual(Set(parts).count, Set(parts.map { "\($0.name)/\($0.uuid ?? "")" }).count, "hashed by name and uuid")
    }

    /// The part's chosen footprint, socket and LED color are the placed part's.
    func testAPlacedPartsChoicesShow() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let color = try XCTUnwrap(led.part.colors.last)
        doc.setPartColor(led.id, color)
        let placed = try XCTUnwrap(doc.chips.first { $0.id == led.id })
        XCTAssertTrue(PartFacts.rows(for: placed).contains { $0.label == "Color" && $0.value.contains(color) },
                      "\(PartFacts.rows(for: placed))")
    }
}
