import XCTest
@testable import schematical

/// The part picker's ADC and DAC groups filter by bits and channels, each
/// group on its own.
@MainActor
final class DrawerConverterFilterTests: XCTestCase {
    private func converters(_ filter: DrawerFilter) -> [CatalogPart] {
        TestSupport.catalog.groups(matching: filter)
            .flatMap(\.functions)
            .filter { $0.function == "ADC" || $0.function == "DAC" }
            .flatMap(\.parts)
    }

    private func facts(_ part: CatalogPart) -> (type: String, bits: Int, channels: Int)? {
        guard case let .converter(type, bits, channels) = part.details else { return nil }
        return (type, bits, channels)
    }

    /// Picking 12-bit, 4-channel ADCs keeps only those ADCs and leaves the DACs alone.
    func testBitsAndChannelsNarrowOnlyTheirGroup() throws {
        let all = converters(DrawerFilter())
        let dacCount = all.filter { facts($0)?.type == "dac" }.count

        var filter = DrawerFilter()
        filter.converterBits["adc"] = 12
        filter.converterChannels["adc"] = 4
        let narrowed = converters(filter)

        let adcs = narrowed.filter { facts($0)?.type == "adc" }
        XCTAssertFalse(adcs.isEmpty)
        XCTAssertTrue(adcs.allSatisfy { facts($0)?.bits == 12 && facts($0)?.channels == 4 })
        XCTAssertEqual(narrowed.filter { facts($0)?.type == "dac" }.count, dacCount)
    }

    /// Each menu lists only values that some part has with the other menu's choice.
    func testMenusListOnlyMatchingPairs() {
        let catalog = TestSupport.catalog
        XCTAssertTrue(catalog.converterChannels(type: "dac", bits: nil).contains(96))
        XCTAssertEqual(catalog.converterBits(type: "dac", channels: 96), [12])
        XCTAssertFalse(catalog.converterChannels(type: "dac", bits: 8).contains(96))
    }
}
