import SwiftUI
import XCTest
@testable import schematical

/// plan-50-B: the Stackup section's weights and rows, and that it builds.
@MainActor
final class StackupSectionTests: XCTestCase {
    func testCopperWeights() {
        XCTAssertEqual(CopperWeight(thickness: 0.035), .one)
        XCTAssertEqual(CopperWeight(thickness: 0.0175), .half)
        XCTAssertEqual(CopperWeight(thickness: 0.07), .two)
        XCTAssertNil(CopperWeight(thickness: 0.0152), "JLCPCB's inner copper is Custom")
        XCTAssertEqual(CopperWeight(thickness: 0.03504), .one, "within 0.1 µm")
    }

    func testRowsInterleaveCopperAndDielectrics() {
        func rows(_ layers: Int) -> [StackupRow] {
            StackupRows.rows(for: .standard(layers: layers), layers: CopperLayer.allCases(for: layers))
        }
        XCTAssertEqual(rows(2).map(\.kind), [.copper(0), .dielectric(0), .copper(1)])
        XCTAssertEqual(rows(2).map(\.title), ["Top copper", "Top to Bottom", "Bottom copper"])
        XCTAssertEqual(rows(1).map(\.kind), [.copper(0), .dielectric(0)])
        XCTAssertEqual(rows(1).map(\.title), ["Top copper", "Substrate"])
        XCTAssertEqual(rows(4).map(\.kind), [.copper(0), .dielectric(0), .copper(1), .dielectric(1), .copper(2),
                                            .dielectric(2), .copper(3)])
        let six = rows(6)
        XCTAssertEqual(six.filter { if case .copper = $0.kind { true } else { false } }.count, 6)
        XCTAssertEqual(six.filter { if case .dielectric = $0.kind { true } else { false } }.count, 5)
        XCTAssertEqual(Set(six.map(\.title)).count, six.count, "titles are unique")
        XCTAssertEqual(six[1].title, "Top to Layer 2")
    }

    func testValueFieldFormatting() {
        XCTAssertEqual(ValueField.format(35, places: 1), "35")
        XCTAssertEqual(ValueField.format(15.2, places: 1), "15.2")
        XCTAssertEqual(ValueField.format(4.5, places: 2), "4.5")
        XCTAssertEqual(ValueField.format(4.16, places: 2), "4.16")
    }

    func testSectionBuildsForEveryLayerCount() throws {
        for layers in BoardSettings.layerCounts {
            let doc = TestSupport.newDocument()
            doc.setLayerCount(layers)
            let view = Form { StackupSection(document: doc) }.frame(width: 360, height: 700)
            XCTAssertNotNil(ImageRenderer(content: view).cgImage, "\(layers) layers")
        }
    }
}
