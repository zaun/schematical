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
    }

    func testStandardPrefixes() {
        XCTAssertEqual(ReferenceCategory.led.standardPrefix, "D")
        XCTAssertEqual(ReferenceCategory.zenerDiode.standardPrefix, "D")
        XCTAssertEqual(ReferenceCategory.oscillator.standardPrefix, "G")
        XCTAssertEqual(ReferenceCategory.femaleConnector.standardPrefix, "J")
        XCTAssertEqual(ReferenceCategory.maleConnector.standardPrefix, "P")
        XCTAssertEqual(ReferenceCategory.switchPart.standardPrefix, "S")
        XCTAssertEqual(ReferenceCategory.processor.standardPrefix, "U")
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
}
