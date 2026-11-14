import SwiftUI
import XCTest
@testable import schematical

/// `ChipPin.paintedName` (plan-39-C § 3): the drawn text and its overlines.
final class PinNameTextTests: XCTestCase {
    private func pin(_ name: String, mask: UInt32) -> ChipPin {
        ChipPin(index: 0, designator: "1", name: name, role: .input, group: ChipPin.defaultGroup,
                activeLowMask: mask)
    }

    private func painted(_ name: String, _ mask: UInt32) -> (String, [Range<Int>]) {
        let text = pin(name, mask: mask).paintedName(size: 10, weight: .medium, monospaced: true, ink: .secondary)
        XCTAssertEqual(text.size, 10)
        XCTAssertEqual(text.weight, .medium)
        XCTAssertTrue(text.monospaced)
        XCTAssertEqual(text.ink, .secondary)
        return (text.string, text.overlines)
    }

    func testFlaggedHashNameDropsTheHash() {
        let (string, overlines) = painted("CE#", 1)
        XCTAssertEqual(string, "CE")
        XCTAssertEqual(overlines, [0..<2])
    }

    func testFlaggedNameWithoutHash() {
        let (string, overlines) = painted("OE", 1)
        XCTAssertEqual(string, "OE")
        XCTAssertEqual(overlines, [0..<2])
    }

    func testOneAlternativeOfTwo() {
        let (string, overlines) = painted("A0/CS", 2)
        XCTAssertEqual(string, "A0/CS")
        XCTAssertEqual(overlines, [3..<5])
    }

    func testEveryAlternative() {
        let (string, overlines) = painted("NRST/PA0", 3)
        XCTAssertEqual(string, "NRST/PA0")
        XCTAssertEqual(overlines, [0..<4, 5..<8])
    }

    func testALoneHashKeepsIt() {
        let (string, overlines) = painted("#", 1)
        XCTAssertEqual(string, "#")
        XCTAssertEqual(overlines, [0..<1])
    }

    func testUnflaggedHashNameIsUnchanged() {
        let (string, overlines) = painted("CE#", 0)
        XCTAssertEqual(string, "CE#")
        XCTAssertEqual(overlines, [])
    }

    func testHashInsideAMultiplexedName() {
        let (string, overlines) = painted("A0/CS#", 2)
        XCTAssertEqual(string, "A0/CS")
        XCTAssertEqual(overlines, [3..<5])
    }
}
