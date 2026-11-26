import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-36-H: microcontrollers are the `mcu` kind, shown as
/// "Microcontroller" with their core, clock and memories, and referenced
/// `U<n>`. One with no simulation model is marked not simulated; the
/// ATmega48/88/168/328 family has a model (plan-62-G) and is simulated.
@MainActor
final class McuTests: XCTestCase {
    private func facts(_ part: CatalogPart) throws -> [String: String] {
        let chip = try TestSupport.place(TestSupport.newDocument(), part.name, at: CGPoint(x: 100, y: 100))
        return Dictionary(PartFacts.rows(for: chip).map { ($0.label, $0.value) }, uniquingKeysWith: { a, _ in a })
    }

    func testAtmega328pFacts() throws {
        let part = try TestSupport.part("ATMEGA328P-PU")
        XCTAssertEqual(part.kind, PART_MCU)
        XCTAssertTrue(part.isSimulated, "plan-62-G: the ATmega328P has a simulation model")
        XCTAssertEqual(PartFacts.kindName(part), "Microcontroller")
        let rows = try facts(part)
        XCTAssertEqual(rows["Kind"], "Microcontroller")
        XCTAssertEqual(rows["Core"], "AVR")
        XCTAssertEqual(rows["Clock"], "20 MHz")
        XCTAssertEqual(rows["Flash"], "32 KB")
        XCTAssertEqual(rows["RAM"], "2 KB")
        XCTAssertEqual(rows["EEPROM"], "1 KB")
        XCTAssertNil(rows["PSRAM"], "a zero size is left out")
        XCTAssertNil(rows["Simulation"], "a simulated part has no Simulation row")
    }

    /// An AVR outside the simulated families (the ATmega48/88/168/328 and
    /// the ATtiny25/45/85) has no model yet.
    func testAttiny13IsNotSimulated() throws {
        let part = try TestSupport.part("ATTINY13A-PU")
        XCTAssertEqual(part.kind, PART_MCU)
        XCTAssertFalse(part.isSimulated)
        let rows = try facts(part)
        XCTAssertEqual(rows["Core"], "AVR")
        XCTAssertEqual(rows["Simulation"], "Not simulated")
    }

    func testAttiny85IsSimulated() throws {
        let part = try TestSupport.part("ATTINY85-20PU")
        XCTAssertEqual(part.kind, PART_MCU)
        XCTAssertTrue(part.isSimulated)
        let rows = try facts(part)
        XCTAssertEqual(rows["Core"], "AVR")
        XCTAssertEqual(rows["Flash"], "8 KB")
        XCTAssertNil(rows["Simulation"], "a simulated part has no Simulation row")
    }

    /// The plan named ESP32-S3-WROOM-1-N8R8, which is not in stock; its
    /// external-antenna twin has the same memories.
    func testEsp32ModuleMemories() throws {
        let part = try TestSupport.part("ESP32-S3-WROOM-1U-N8R8")
        let rows = try facts(part)
        XCTAssertEqual(rows["Core"], "Xtensa LX7")
        XCTAssertEqual(rows["Flash"], "8 MB")
        XCTAssertEqual(rows["PSRAM"], "8 MB")
        XCTAssertEqual(rows["Simulation"], "Not simulated")
    }

    func testSimulatedPartsHaveNoSimulationRow() throws {
        let part = try TestSupport.part("SN74HC00N")
        XCTAssertTrue(part.isSimulated)
        XCTAssertNil(try facts(part)["Simulation"])
    }

    func testReferenceIsU() throws {
        UserDefaults.standard.removeObject(forKey: PreferenceKey.referencePrefixes)
        let part = try TestSupport.part("ATMEGA328P-PU")
        XCTAssertEqual(ReferenceCategory.of(part), .microcontroller)
        XCTAssertEqual(ReferenceCategory.microcontroller.standardPrefix, "U")
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "ATMEGA328P-PU", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(chip.reference, "U1")
    }
}
