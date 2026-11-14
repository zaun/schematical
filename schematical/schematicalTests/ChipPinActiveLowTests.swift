import Foundation
import XCTest
@testable import schematical

/// A pin's "activeLow" (plan-39-A) carried into `ChipPin` (plan-39-B).
@MainActor
final class ChipPinActiveLowTests: XCTestCase {
    /// A catalog of its own, so the part these tests add stays out of the
    /// shared one other tests read.
    private static let store: PartCatalogStore = {
        let store = PartCatalogStore()
        store.load()
        return store
    }()

    /// A single part file: a 6-pin NAND whose pins carry the four flag forms
    /// (or, with `flagged` false, the same pins with no flags).
    static func fileText(uuid: String, name: String, flagged: Bool = true) -> String {
        var pins: [[String: Any]] = [
            ["number": 1, "name": "CE#", "role": "input", "activeLow": true],
            ["number": 2, "name": "A0/CS", "role": "input", "activeLow": ["CS"]],
            ["number": 3, "name": "GND", "role": "ground"],
            ["number": 4, "name": "D0", "role": "output"],
            ["number": 5, "name": "VCC", "role": "power"],
            ["number": 6, "name": "AIN7/GPIO3/DRDY/CLK", "role": "input", "activeLow": ["DRDY"]],
        ]
        if !flagged {
            pins = pins.map { $0.filter { $0.key != "activeLow" } }
        }
        let gate: [String: Any] = ["op": "nand", "inputs": ["CE#", "A0/CS"], "output": "D0"]
        let timing: [String: Any] = ["HC": ["rise": 9, "fall": 9, "vcc": [2, 6]] as [String: Any]]
        let part: [String: Any] = ["name": name, "manufacturer": "Generic", "package": "SOT-23", "pinout": "p6",
                                   "footprint": "SOT23-6", "family": "HC"]
        let device: [String: Any] = ["id": "LOW6", "function": "NAND", "description": "Active-low test part",
                                     "drawer": ["Custom", "Gates"], "pinouts": ["p6": pins], "gates": [gate],
                                     "timing": timing, "parts": [part]]
        let file: [String: Any] = ["format": "schematical-part", "version": 1, "uuid": uuid, "kind": "gate",
                                   "devices": [device]]
        let data = try! JSONSerialization.data(withJSONObject: file, options: [.sortedKeys])
        return String(decoding: data, as: UTF8.self)
    }

    private func alternatives(_ pin: ChipPin) -> [String] {
        pin.alternatives.map { $0.isActiveLow ? "!" + $0.text : $0.text }
    }

    func testPinsReportTheFilesFlags() throws {
        let part = try Self.store.register(
            fileText: Self.fileText(uuid: "C39B0000-0000-4000-8000-000000000001", name: "LOW-6"),
            source: "low.schpart")
        let pins = Dictionary(uniqueKeysWithValues: part.pins.map { ($0.name, $0) })

        XCTAssertEqual(alternatives(pins["CE#"]!), ["!CE#"])
        XCTAssertEqual(pins["CE#"]!.activeLowMask, 1)
        XCTAssertTrue(pins["CE#"]!.isActiveLow)

        XCTAssertEqual(alternatives(pins["A0/CS"]!), ["A0", "!CS"])
        XCTAssertEqual(pins["A0/CS"]!.activeLowMask, 2)
        XCTAssertTrue(pins["A0/CS"]!.isActiveLow)

        XCTAssertEqual(alternatives(pins["D0"]!), ["D0"])
        XCTAssertEqual(pins["D0"]!.activeLowMask, 0)
        XCTAssertFalse(pins["D0"]!.isActiveLow)
        XCTAssertFalse(pins["VCC"]!.isActiveLow)

        XCTAssertEqual(alternatives(pins["AIN7/GPIO3/DRDY/CLK"]!), ["AIN7", "GPIO3", "!DRDY", "CLK"])
        XCTAssertEqual(pins["AIN7/GPIO3/DRDY/CLK"]!.activeLowMask, 4)
    }

    /// A pin built in code has no flag unless given one.
    func testDefaultIsUnflagged() {
        let pin = ChipPin(index: 0, designator: "1", name: "OE", role: .input, group: ChipPin.defaultGroup)
        XCTAssertEqual(pin.activeLowMask, 0)
        XCTAssertEqual(pin.alternatives, [ChipPin.Alternative(text: "OE", isActiveLow: false)])
    }
}
