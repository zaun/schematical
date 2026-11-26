import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Settings ▸ Reference Defaults: the prefix each category of part gets.
@MainActor
final class ReferencePrefixTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    override func setUp() {
        super.setUp()
        UserDefaults.standard.removeObject(forKey: PreferenceKey.referencePrefixes)
    }

    override func tearDown() {
        UserDefaults.standard.removeObject(forKey: PreferenceKey.referencePrefixes)
        super.tearDown()
    }

    func testCategoriesOfCatalogParts() throws {
        func category(_ name: String, gender: String? = nil) throws -> ReferenceCategory {
            ReferenceCategory.of(try TestSupport.part(name), gender: gender)
        }
        XCTAssertEqual(try category("GENERIC-LED"), .led)
        XCTAssertEqual(try category("GENERIC-CAPACITOR"), .capacitor)
        XCTAssertEqual(try category("PULLUP-5V"), .resistor)
        XCTAssertEqual(try category("1N4728A"), .zenerDiode)
        XCTAssertEqual(try category("W65C02S6TPG-14"), .processor)
        XCTAssertEqual(try category("ATMEGA328P-PU"), .microcontroller)
        XCTAssertEqual(try category("AS6C6264-55PCN"), .memory)
        XCTAssertEqual(try category("GENERIC-HEADER-1X02", gender: "Male"), .maleConnector)
        XCTAssertEqual(try category("GENERIC-HEADER-1X02", gender: "Female"), .femaleConnector)
        XCTAssertEqual(try category("SN74HC00N"), .integratedCircuit)
        XCTAssertEqual(try category("GENERIC-SOCKET-DIP-14-300"), .socket)
    }

    func testStandardPrefixes() {
        XCTAssertEqual(ReferenceCategory.led.standardPrefix, "D")
        XCTAssertEqual(ReferenceCategory.zenerDiode.standardPrefix, "D")
        XCTAssertEqual(ReferenceCategory.oscillator.standardPrefix, "G")
        XCTAssertEqual(ReferenceCategory.femaleConnector.standardPrefix, "J")
        XCTAssertEqual(ReferenceCategory.maleConnector.standardPrefix, "P")
        XCTAssertEqual(ReferenceCategory.switchPart.standardPrefix, "S")
        XCTAssertEqual(ReferenceCategory.processor.standardPrefix, "U")
        XCTAssertEqual(ReferenceCategory.socket.standardPrefix, "XS")
        // Every standard prefix is one a reference can have.
        for category in ReferenceCategory.allCases {
            XCTAssertTrue(ReferencePrefixes.isValid(category.standardPrefix), category.title)
        }
    }

    func testStoredPrefixOverridesStandardAndBadOneIsIgnored() {
        let data = ReferencePrefixes.encoded(["led": "led", "diode": "D1"])
        XCTAssertEqual(ReferencePrefixes.prefix(for: .led, in: data), "LED")
        XCTAssertEqual(ReferencePrefixes.prefix(for: .diode, in: data), "D")
        XCTAssertEqual(ReferencePrefixes.prefix(for: .capacitor, in: Data()), "C")
    }

    func testNewPartsUseTheirCategoryPrefix() throws {
        let doc = TestSupport.newDocument()
        XCTAssertEqual(try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100)).reference, "D1")
        XCTAssertEqual(try TestSupport.place(doc, "1N4728A", at: p(300, 100)).reference, "D2")
        XCTAssertEqual(try TestSupport.place(doc, "SN74HC00N", at: p(500, 100)).reference, "U1")
        XCTAssertEqual(doc.nextReference, 2)

        UserDefaults.standard.set(ReferencePrefixes.encoded(["led": "LED"]),
                                  forKey: PreferenceKey.referencePrefixes)
        XCTAssertEqual(try TestSupport.place(doc, "GENERIC-LED", at: p(100, 400)).reference, "LED1")
        // Reset References uses the setting too.
        doc.endCoalescing()
        XCTAssertTrue(doc.resetReferences(page: .schematic))
        XCTAssertEqual(doc.chips.map(\.reference), ["LED1", "D1", "U1", "LED2"])
    }

    func testPastedPartsWithTheSamePrefixGetDifferentNumbers() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: p(300, 100))
        doc.endCoalescing()
        doc.select(.chip(a.id))
        doc.toggleSelection(.chip(b.id))
        let copy = try XCTUnwrap(doc.schematicCopy())
        XCTAssertTrue(doc.pasteSchematic(copy, offset: CGSize(width: 0, height: 300)))
        XCTAssertEqual(Set(doc.chips.map(\.reference)), ["D1", "D2", "D3", "D4"])
    }

    /// Every kind of part has its category (plan-68-AB).
    func testEveryKindHasItsCategory() throws {
        let parts = TestSupport.catalog.allParts
        func first(_ kind: PartKind, where test: (CatalogPart) -> Bool = { _ in true }) throws -> CatalogPart {
            try XCTUnwrap(parts.first { $0.kind == kind && !$0.isPullResistor && test($0) }, "\(kind)")
        }
        let expected: [(PartKind, ReferenceCategory)] = [
            (PART_CAPACITOR, .capacitor), (PART_RESISTOR, .resistor), (PART_TRANSISTOR, .transistor),
            (PART_LED, .led), (PART_7SEG, .display), (PART_CHARLCD, .display), (PART_NIXIE, .display),
            (PART_DEKATRON, .display), (PART_LCD, .display), (PART_OSCILLATOR, .oscillator), (PART_CRYSTAL, .crystal),
            (PART_INDUCTOR, .inductor), (PART_FUSE, .fuse), (PART_RELAY, .relay), (PART_BUZZER, .speaker),
            (PART_SWITCH, .switchPart), (PART_EEPROM, .memory), (PART_SERIALMEM, .memory), (PART_SRAM, .memory),
        ]
        for (kind, category) in expected {
            XCTAssertEqual(ReferenceCategory.of(try first(kind)), category, "\(kind)")
        }
        XCTAssertEqual(ReferenceCategory.of(try first(PART_DIODE) { part in
            if case let .diode(type, _, _, _, _, _) = part.details { return type != "zener" }
            return true
        }), .diode)
        XCTAssertEqual(ReferenceCategory.of(try first(PART_CONNECTOR) { $0.isTestPoint }), .testPoint)
        XCTAssertEqual(ReferenceCategory.of(try first(PART_CONNECTOR) { !$0.isTestPoint }), .femaleConnector)
        let header = try TestSupport.part("GENERIC-HEADER-1X02")
        XCTAssertEqual(ReferenceCategory.of(header), header.genders.first == "Female" ? .femaleConnector : .maleConnector)
        let supply = try XCTUnwrap(TestSupport.catalog.part(named: "GND"))
        XCTAssertEqual(ReferenceCategory.of(supply), .powerSymbol)
    }

    func testTitlesAreDistinct() {
        let titles = ReferenceCategory.allCases.map(\.title)
        XCTAssertEqual(Set(titles).count, titles.count)
        XCTAssertEqual(ReferenceCategory.speaker.title, "Speaker / Buzzer")
        XCTAssertEqual(ReferenceCategory.socket.title, "IC Socket")
        XCTAssertEqual(ReferenceCategory.led.id, "led")
    }

    func testSettingAndCommittingAPrefix() {
        let data = ReferencePrefixes.setting("LED", for: .led, in: Data())
        XCTAssertEqual(ReferencePrefixes.stored(data), ["led": "LED"])
        XCTAssertEqual(ReferencePrefixes.stored(ReferencePrefixes.setting("D", for: .led, in: data)), [:],
                       "the standard prefix is not stored")
        XCTAssertEqual(ReferencePrefixes.stored(Data("not json".utf8)), [:])
        XCTAssertTrue(ReferencePrefixes.committed(" led ", current: "D") == ("LED", "LED"))
        XCTAssertTrue(ReferencePrefixes.committed("D1", current: "D") == (nil, "D"))
        XCTAssertTrue(ReferencePrefixes.committed("", current: "D") == (nil, "D"))
        XCTAssertFalse(ReferencePrefixes.isValid("ABCDEFGHI"))
    }

    func testPrefixOfAPartReadsTheDefaults() throws {
        let defaults = try XCTUnwrap(UserDefaults(suiteName: "ReferencePrefixTests-\(UUID())"))
        defaults.set(ReferencePrefixes.encoded(["led": "LD"]), forKey: PreferenceKey.referencePrefixes)
        XCTAssertEqual(ReferencePrefixes.prefix(for: try TestSupport.part("GENERIC-LED"), defaults: defaults), "LD")
        XCTAssertEqual(ReferencePrefixes.prefix(for: .capacitor, defaults: defaults), "C")
    }
}
