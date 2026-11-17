import XCTest
@testable import schematical

/// The supply menu's Custom… voltage: rail names, and the rails the catalog
/// makes on demand.
@MainActor
final class CustomSupplyTests: XCTestCase {
    func testTheMenuStopsAtTwelveVolts() {
        XCTAssertEqual(SupplyChoice.names, ["5V", "3V3", "2V5", "1V8", "1V2", "12V"])
    }

    func testRailNames() {
        XCTAssertEqual(SupplyChoice.name(volts: 170), "170V")
        XCTAssertEqual(SupplyChoice.name(volts: 3.3), "3V3")
        XCTAssertEqual(SupplyChoice.name(volts: 1.25), "1V25")
        XCTAssertEqual(SupplyChoice.name(volts: 0.9), "0V9")
        XCTAssertEqual(SupplyChoice.name(volts: 300), "300V")
        XCTAssertNil(SupplyChoice.name(volts: 0))
        XCTAssertNil(SupplyChoice.name(volts: -5))
        XCTAssertNil(SupplyChoice.name(volts: 300.5))
        XCTAssertNil(SupplyChoice.name(volts: .nan))
        // Every menu rail is its own voltage's name.
        for name in SupplyChoice.names {
            let mv = try? XCTUnwrap(SupplyChoice.millivolts(name: name))
            XCTAssertEqual(mv.flatMap { SupplyChoice.name(volts: Double($0) / 1000) }, name)
        }
    }

    func testNamesBackToVolts() {
        XCTAssertEqual(SupplyChoice.millivolts(name: "170V"), 170_000)
        XCTAssertEqual(SupplyChoice.millivolts(name: "3V3"), 3300)
        XCTAssertEqual(SupplyChoice.millivolts(name: "1V25"), 1250)
        for bad in ["VCC", "GND", "V", "3V30", "301V", "0V", "PULLUP-5V", "5 V", "3.3V", "SN74HC00N", ""] {
            XCTAssertNil(SupplyChoice.millivolts(name: bad), bad)
        }
    }

    func testTypedVoltages() {
        XCTAssertEqual(SupplyChoice.name(typed: "170"), "170V")
        XCTAssertEqual(SupplyChoice.name(typed: " 170 V "), "170V")
        XCTAssertEqual(SupplyChoice.name(typed: "170v"), "170V")
        XCTAssertEqual(SupplyChoice.name(typed: "3.3"), "3V3")
        XCTAssertEqual(SupplyChoice.name(typed: "3,3"), "3V3")
        XCTAssertEqual(SupplyChoice.name(typed: "3V3"), "3V3")
        XCTAssertEqual(SupplyChoice.name(typed: "5.0"), "5V")
        for bad in ["", "abc", "0", "-5", "301", "1e2", "12 volts"] {
            XCTAssertNil(SupplyChoice.name(typed: bad), bad)
        }
    }

    func testARailOfAnyVoltageExistsWhenAskedFor() throws {
        let catalog = TestSupport.catalog
        let rail = try XCTUnwrap(catalog.part(named: "48V"))
        guard case let .supply(level, mv, pull) = rail.details else { return XCTFail("48V is no supply") }
        XCTAssertEqual(level, 1)
        XCTAssertEqual(mv, 48_000)
        XCTAssertFalse(pull)
        XCTAssertEqual(symbol(for: rail), .vcc)
        XCTAssertTrue(rail.drawer.isEmpty, "a rail is not listed in the drawer")

        let pullUp = try XCTUnwrap(catalog.part(named: "PULLUP-48V"))
        guard case let .supply(_, pullMV, isPull) = pullUp.details else { return XCTFail("no pull-up") }
        XCTAssertEqual(pullMV, 48_000)
        XCTAssertTrue(isPull)
        XCTAssertEqual(symbol(for: pullUp), .pullUp)
        XCTAssertEqual(pullUp.footprint?.id, "RES-RADIAL-2.54")

        // Asked for the other way round, and again: the same parts.
        XCTAssertNotNil(catalog.part(named: "PULLUP-7V5"))
        XCTAssertEqual(catalog.part(named: "7V5")?.name, "7V5")
        XCTAssertEqual(catalog.part(named: "48V"), rail)
        // The C library has them, so a circuit finds them by name.
        XCTAssertNotNil(part_library_find(catalog.library.pointer, "48V"))
    }

    func testOnlyRailNamesAreMade() {
        let catalog = TestSupport.catalog
        for name in ["301V", "0V", "PULLUP-", "PULLUP-VCC", "NOSUCHPART", "PULLDOWN-5V"] {
            XCTAssertNil(catalog.part(named: name), name)
        }
        // Bundled rails stay the bundled parts.
        XCTAssertEqual(catalog.part(named: "170V")?.name, "170V")
        XCTAssertEqual(catalog.part(named: "PULLUP-5V")?.name, "PULLUP-5V")
    }
}
