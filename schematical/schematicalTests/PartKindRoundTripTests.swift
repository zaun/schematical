import Foundation
import XCTest
@testable import schematical

/// Every shipped part of each kind with a form round-trips through the Part
/// Editor's draft with no key left without a control, and the backend
/// accepts each encoded file (plan-32-F … J).
@MainActor
final class PartKindRoundTripTests: XCTestCase {
    /// The shipped part count of each kind (`rg -o '"manufacturer"' Parts/<kind>.json | wc -l`).
    static let expected: [String: Int] = [
        "dff": 867, "jkff": 186, "mux": 346, "decoder": 322,
        "latch": 750, "buffer": 2135, "arith": 107,
        "counter": 730, "shift": 583,
        "sram": 172, "eeprom": 174, "switch": 2067, "led": 1, "7seg": 49, "oscillator": 1138, "supply": 14,
        "cpu": 21, "pio": 7, "serial": 247, "charlcd": 72, "connector": 374, "converter": 1911, "rtc": 14,
        "diode": 11152, "header": 80, "register": 220,
    ]

    private func check(_ kind: String) {
        let count = PartKindRoundTrip.check(kind: kind)
        print("checked \(count.checked) parts of \(kind), \(count.mismatched.count) mismatched, "
            + "\(count.withExtras.count) with extras, \(count.rejected.count) rejected")
        XCTAssertEqual(count.checked, Self.expected[kind], kind)
        let report = PartKindRoundTrip.report(count, kind: kind)
        XCTAssertEqual(count.mismatched.count, 0, report)
        XCTAssertEqual(count.withExtras.count, 0, report)
        XCTAssertEqual(count.rejected.count, 0, report)
    }

    func testDff() { check("dff") }
    func testJkff() { check("jkff") }
    func testMux() { check("mux") }
    func testDecoder() { check("decoder") }
    func testLatch() { check("latch") }
    func testBuffer() { check("buffer") }
    func testArith() { check("arith") }
    func testCounter() { check("counter") }
    func testShift() { check("shift") }
    func testSram() { check("sram") }
    func testEeprom() { check("eeprom") }
    func testSwitch() { check("switch") }
    func testLed() { check("led") }
    func testSevenSeg() { check("7seg") }
    func testOscillator() { check("oscillator") }
    func testSupply() { check("supply") }
    func testCpu() { check("cpu") }
    func testPio() { check("pio") }
    func testSerial() { check("serial") }
    func testCharLcd() { check("charlcd") }
    func testConnector() { check("connector") }
    func testConverter() { check("converter") }
    func testRtc() { check("rtc") }
    func testDiode() { check("diode") }
    func testHeader() { check("header") }
    func testRegister() { check("register") }
}
