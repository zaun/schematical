import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The serialmem kind in the app: the sidebar's facts, the symbol and the
/// reference prefix.
@MainActor
final class SerialMemoryTests: XCTestCase {
    private func facts(_ name: String) throws -> [String: String] {
        let chip = try TestSupport.place(TestSupport.newDocument(), name, at: CGPoint(x: 100, y: 100))
        return Dictionary(PartFacts.rows(for: chip).map { ($0.label, $0.value) }, uniquingKeysWith: { a, _ in a })
    }

    func testEepromFacts() throws {
        let part = try TestSupport.part("AT25040B-SSHL-T")
        XCTAssertEqual(part.kind, PART_SERIALMEM)
        XCTAssertEqual(PartFacts.kindName(part), "Serial memory")
        XCTAssertEqual(symbol(for: part), .block)
        XCTAssertEqual(ReferenceCategory.of(part), .memory)
        XCTAssertEqual(part.drawer, ["Memory", "Serial", "EEPROM"])
        let rows = try facts(part.name)
        XCTAssertEqual(rows["Kind"], "Serial memory")
        XCTAssertEqual(rows["Type"], "Serial EEPROM")
        XCTAssertEqual(rows["Size"], "4 Kbit (512 bytes)")
        XCTAssertEqual(rows["Address bytes"], "1")
        XCTAssertEqual(rows["Page"], "8 bytes")
        XCTAssertEqual(rows["Write cycle"], "5 ms")
        XCTAssertEqual(rows["Max clock"], "20 MHz")
        XCTAssertEqual(rows["Endurance"], "1 million cycles")
        XCTAssertEqual(rows["Retention"], "100 years")
        XCTAssertNil(rows["Page program"])
        XCTAssertNil(rows["Chip erase"])
        XCTAssertNil(rows["JEDEC ID"], "an EEPROM with no ID leaves the row out")
    }

    func testFramFacts() throws {
        let rows = try facts("FM25L16B-GTR")
        XCTAssertEqual(rows["Type"], "F-RAM")
        XCTAssertEqual(rows["Size"], "16 Kbit (2 KB)")
        XCTAssertEqual(rows["Address bytes"], "2")
        XCTAssertNil(rows["Page"], "an F-RAM has no write page")
        XCTAssertNil(rows["Write cycle"], "an F-RAM writes at bus speed")
        XCTAssertEqual(rows["Endurance"], "100 trillion cycles")
    }

    func testFlashFacts() throws {
        let rows = try facts("AT25XE021A-SSHN-T")
        XCTAssertEqual(rows["Type"], "NOR flash")
        XCTAssertEqual(rows["Size"], "2 Mbit (256 KB)")
        XCTAssertEqual(rows["Page"], "256 bytes")
        XCTAssertEqual(rows["Page program"], "2 ms")
        XCTAssertEqual(rows["Sector erase"], "45 ms")
        XCTAssertEqual(rows["Chip erase"], "2.4 s")
        XCTAssertEqual(rows["JEDEC ID"], "1F430100")
        XCTAssertEqual(rows["Endurance"], "100,000 cycles")
    }

    func testUnits() {
        XCTAssertEqual(PartFacts.density(512), "4 Kbit")
        XCTAssertEqual(PartFacts.density(262_144), "2 Mbit")
        XCTAssertEqual(PartFacts.density(100), "100 bytes")
        XCTAssertEqual(PartFacts.duration(0), "")
        XCTAssertEqual(PartFacts.duration(250_000), "250 ns")
        XCTAssertEqual(PartFacts.duration(1_400_000), "1.4 µs")
        XCTAssertEqual(PartFacts.duration(5_000_000_000), "5 ms")
        XCTAssertEqual(PartFacts.serialMemoryType("dataflash"), "DataFlash")
        XCTAssertEqual(PartFacts.serialMemoryType("pageeeprom"), "Page EEPROM")
    }
}
