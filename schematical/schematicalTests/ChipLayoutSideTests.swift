import XCTest
@testable import schematical

/// The block layout: pins in name order, and a pin's "side" overriding its role.
final class ChipLayoutSideTests: XCTestCase {
    private func pin(_ index: Int, _ name: String, _ role: ChipPinRole,
                     side: ChipPinSide = .auto) -> ChipPin {
        ChipPin(index: index, designator: "\(index + 1)", name: name, role: role,
                group: ChipPin.defaultGroup, side: side)
    }

    private func slots(_ layout: ChipLayout, _ side: ChipLayout.Side) -> [String] {
        layout.slots.filter { $0.side == side }.sorted { $0.centerY < $1.centerY }.map(\.pin.name)
    }

    /// A 2T45-style translator: every signal is an inout; A goes left, B right.
    func testSideOverridesRole() {
        let layout = ChipLayout(pins: [
            pin(0, "VCCA", .power),
            pin(1, "A1", .inOut, side: .input),
            pin(2, "A2", .inOut, side: .input),
            pin(3, "GND", .ground),
            pin(4, "DIR", .input),
            pin(5, "B2", .inOut, side: .output),
            pin(6, "B1", .inOut, side: .output),
            pin(7, "VCCB", .power),
        ])
        XCTAssertEqual(slots(layout, .left), ["VCCA", "VCCB", "A1", "A2", "DIR"])
        XCTAssertEqual(slots(layout, .right), ["B1", "B2", "GND"])
    }

    /// Without a side, inouts stay on the right with the outputs.
    func testInoutDefaultsRight() {
        let layout = ChipLayout(pins: [pin(0, "A", .input), pin(1, "IO", .inOut), pin(2, "Y", .output)])
        XCTAssertEqual(slots(layout, .left), ["A"])
        XCTAssertEqual(slots(layout, .right), ["IO", "Y"])
    }
}
