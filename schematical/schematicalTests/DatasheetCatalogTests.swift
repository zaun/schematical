import XCTest
@testable import schematical

/// The bundled catalog carries each part's manufacturer and datasheet URL.
@MainActor
final class DatasheetCatalogTests: XCTestCase {
    func testTIPartHasManufacturerAndDatasheet() throws {
        let part = try TestSupport.part("SN74HC00N")
        XCTAssertEqual(part.manufacturer, "Texas Instruments")
        XCTAssertEqual(part.datasheetURL?.absoluteString,
                       "https://www.ti.com/lit/ds/symlink/sn74hc00.pdf")
    }

    func testGenericPartHasNoDatasheet() throws {
        let part = try TestSupport.part("5V")
        XCTAssertEqual(part.manufacturer, "Generic")
        XCTAssertNil(part.datasheetURL)
    }

    func testOtherManufacturer() throws {
        let part = try TestSupport.part("AT28C256-15PU")
        XCTAssertEqual(part.manufacturer, "Microchip Technology")
        XCTAssertEqual(part.datasheetURL?.host(), "ww1.microchip.com")
    }
}
