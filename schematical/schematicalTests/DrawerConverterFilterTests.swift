import XCTest
@testable import schematical

/// The part picker's ADC and DAC groups are split by bits and filter by
/// channels, each group on its own.
@MainActor
final class DrawerConverterFilterTests: XCTestCase {
    private func converterGroups(_ filter: DrawerFilter) -> [PartCatalogStore.FunctionGroup] {
        TestSupport.catalog.groups(matching: filter)
            .flatMap { $0.functions + $0.subcategories.flatMap(\.functions) }
            .filter { $0.function == "ADC" || $0.function == "DAC" }
    }

    private func converters(_ filter: DrawerFilter) -> [CatalogPart] {
        converterGroups(filter).flatMap(\.parts)
    }

    private func facts(_ part: CatalogPart) -> (type: String, bits: Int, channels: Int)? {
        guard case let .converter(type, bits, channels) = part.details else { return nil }
        return (type, bits, channels)
    }

    /// Picking 4-channel ADCs keeps only those ADCs and leaves the DACs alone.
    func testChannelsNarrowOnlyTheirGroup() throws {
        let all = converters(DrawerFilter())
        let dacCount = all.filter { facts($0)?.type == "dac" }.count

        var filter = DrawerFilter()
        filter.converterChannels["adc"] = 4
        let narrowed = converters(filter)

        let adcs = narrowed.filter { facts($0)?.type == "adc" }
        XCTAssertFalse(adcs.isEmpty)
        XCTAssertTrue(adcs.allSatisfy { facts($0)?.channels == 4 })
        XCTAssertEqual(narrowed.filter { facts($0)?.type == "dac" }.count, dacCount)
    }

    /// Each ADC and DAC group is split by bits, fewest first, and the split
    /// holds every part of the group exactly once.
    func testGroupsSplitByBits() {
        let groups = converterGroups(DrawerFilter())
        XCTAssertFalse(groups.isEmpty)
        for group in groups {
            XCTAssertFalse(group.bitGroups.isEmpty, group.function)
            let bits = group.bitGroups.map(\.bits)
            XCTAssertEqual(bits, bits.sorted())
            XCTAssertEqual(Set(bits).count, bits.count)
            for bitGroup in group.bitGroups {
                XCTAssertTrue(bitGroup.parts.allSatisfy { facts($0)?.bits == bitGroup.bits })
            }
            XCTAssertEqual(group.bitGroups.flatMap(\.parts).map(\.name).sorted(),
                           group.parts.map(\.name).sorted())
        }
        XCTAssertGreaterThan(groups.map(\.bitGroups.count).max() ?? 0, 1)
    }

    /// Groups of other parts are not split.
    func testOtherGroupsHaveNoBitGroups() {
        let others = TestSupport.catalog.groups(matching: DrawerFilter())
            .flatMap { $0.functions + $0.subcategories.flatMap(\.functions) }
            .filter { $0.function != "ADC" && $0.function != "DAC" }
        XCTAssertFalse(others.isEmpty)
        XCTAssertTrue(others.allSatisfy(\.bitGroups.isEmpty))
    }

    /// The channels menu lists the counts its type has.
    func testChannelsMenu() {
        XCTAssertTrue(TestSupport.catalog.converterChannels(type: "dac").contains(96))
    }
}
