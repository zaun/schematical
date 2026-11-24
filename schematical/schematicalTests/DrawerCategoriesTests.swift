import Foundation
import XCTest
@testable import schematical

/// The parts drawer's category tree after the 2026-11-22 regrouping: what
/// sits under each category, and where a person's own parts are listed.
@MainActor
final class DrawerCategoriesTests: XCTestCase {
    /// Stores made by tests, kept for the life of the process (freeing a
    /// `PartCatalogStore` inside a test aborts the host; see
    /// `TestSupport.newDocument()`).
    private static var kept: [PartCatalogStore] = []

    private var groups: [PartCatalogStore.CategoryGroup] {
        TestSupport.catalog.groups(matching: DrawerFilter())
    }

    private func category(_ name: String) throws -> PartCatalogStore.CategoryGroup {
        try XCTUnwrap(groups.first { $0.category == name }, name)
    }

    /// The drawer path of every listed part, below `category`, as a set of
    /// space-joined paths.
    private func paths(under category: String) -> Set<String> {
        Set(TestSupport.catalog.allParts.filter { $0.drawer.first == category }.map { $0.drawer.joined(separator: " > ") })
    }

    func testTopLevelCategories() {
        XCTAssertEqual(TestSupport.catalog.categories,
                       ["Analog", "Clocks", "Connector", "Discrete", "Displays & Indicators", "Interface", "Logic",
                        "Memory", "Microcontroller", "Power", "Processor", "Switches & Relays", "Vintage"])
    }

    /// Interface keeps the serial and parallel I/O chips; converters moved to
    /// Analog and the real-time clocks to Clocks.
    func testInterfaceAnalogAndClocks() throws {
        XCTAssertEqual(paths(under: "Interface"), ["Interface > Parallel I/O", "Interface > Serial"])
        let interface = try category("Interface")
        let serial = try XCTUnwrap(interface.subcategories.first { $0.subcategory == "Serial" })
        XCTAssertEqual(Set(serial.functions.map(\.function)), ["UART", "Bridge UART", "I2C controller"])
        let parallel = try XCTUnwrap(interface.subcategories.first { $0.subcategory == "Parallel I/O" })
        XCTAssertEqual(Set(parallel.functions.map(\.function)), ["PIA", "VIA"])

        XCTAssertEqual(paths(under: "Analog"), ["Analog > Data converters"])
        let converters = try XCTUnwrap(try category("Analog").subcategories.first)
        XCTAssertEqual(Set(converters.functions.map(\.function)), ["ADC", "DAC"])

        let clocks = try category("Clocks")
        XCTAssertEqual(clocks.functions.map(\.function), ["RTC"])
        XCTAssertEqual(paths(under: "Clocks"),
                       ["Clocks", "Clocks > Crystal", "Clocks > Crystal > kHz", "Clocks > Crystal > MHz",
                        "Clocks > Ceramic resonator", "Clocks > Oscillator"])
    }

    func testDisplaysAndIndicators() {
        XCTAssertEqual(paths(under: "Displays & Indicators"),
                       ["Displays & Indicators > 7-Segment", "Displays & Indicators > Character LCD",
                        "Displays & Indicators > LCD glass", "Displays & Indicators > LED"])
    }

    /// Vintage is old technology: the tubes under one group, and the support
    /// chips; nothing is there only because it is obsolete.
    func testVintage() throws {
        XCTAssertEqual(Set(paths(under: "Vintage")),
                       ["Vintage > Support ICs", "Vintage > Display tubes > Nixie", "Vintage > Display tubes > Numitron",
                        "Vintage > Display tubes > VFD", "Vintage > Display tubes > Neon",
                        "Vintage > Display tubes > Dekatron"])
        let vintage = try category("Vintage")
        XCTAssertEqual(Set(vintage.subcategories.map(\.subcategory)), ["Display tubes", "Support ICs"])
        let tubes = try XCTUnwrap(vintage.subcategories.first { $0.subcategory == "Display tubes" })
        XCTAssertEqual(Set(tubes.subgroups.map(\.subcategory)), ["Nixie", "Numitron", "VFD", "Neon", "Dekatron"])
    }

    func testMicrocontrollerAndMemory() throws {
        let mcu = try category("Microcontroller")
        XCTAssertEqual(Set(mcu.subcategories.map(\.subcategory)), ["AVR", "ESP32"])
        let avr = try XCTUnwrap(mcu.subcategories.first { $0.subcategory == "AVR" })
        XCTAssertEqual(Set(avr.subgroups.map(\.subcategory)),
                       ["AT90", "ATmega", "ATtiny", "Dx", "Ex", "Lx/Sx", "XMEGA", "megaAVR 0", "tinyAVR 0/1/2"])
        let memory = try category("Memory")
        XCTAssertEqual(Set(memory.subcategories.map(\.subcategory)), ["EEPROM", "SRAM", "Serial"])
        let serial = try XCTUnwrap(memory.subcategories.first { $0.subcategory == "Serial" })
        XCTAssertEqual(Set(serial.subgroups.map(\.subcategory)), ["EEPROM", "FRAM", "Flash"])
    }

    /// The groups with sub-groups own no part of their own kind: the digit and
    /// size menus are the 7-segment sub-groups' (Vintage → Display tubes).
    func testGroupsListTheirSubgroupsKinds() {
        XCTAssertTrue(TestSupport.catalog.kinds(inGroup: ["Vintage", "Display tubes"]).contains(PART_7SEG.rawValue))
        XCTAssertTrue(TestSupport.catalog.kinds(inGroup: ["Vintage", "Display tubes", "Numitron"])
            .contains(PART_7SEG.rawValue))
    }

    /// The User Library, then the File Library, come last, whatever the names
    /// of the built-in categories.
    func testLibrariesSortLast() throws {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        let directory = FileManager.default.temporaryDirectory.appendingPathComponent("drawer-order-\(UUID().uuidString)")
        defer { try? FileManager.default.removeItem(at: directory) }
        store.attachUserLibrary(UserPartLibrary(directory: directory))
        _ = try store.register(fileText: PartRegistrationTests.fileText(
            uuid: "A5000000-0000-4000-8000-000000000001", name: "ORDER-FILE", drawer: ["Custom", "Gates"]), source: "f")
        _ = try store.addToUserLibrary(fileText: PartRegistrationTests.fileText(
            uuid: "A5000000-0000-4000-8000-000000000002", name: "ORDER-USER", drawer: ["Custom", "Gates"]), source: "u")
        let built = ["Analog", "Clocks", "Connector", "Discrete", "Displays & Indicators", "Interface", "Logic",
                     "Memory", "Microcontroller", "Power", "Processor", "Switches & Relays", "Vintage"]
        XCTAssertEqual(store.categories, built + ["User Library", "File Library"])
        XCTAssertEqual(store.groups(matching: DrawerFilter()).map(\.category), built + ["User Library", "File Library"])
        XCTAssertTrue(PartCategory.precedes("Vintage", "User Library"))
        XCTAssertTrue(PartCategory.precedes("User Library", "File Library"))
        XCTAssertFalse(PartCategory.precedes("File Library", "Analog"))
    }
}
