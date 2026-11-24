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
        "dff": 867, "jkff": 186, "mux": 346, "decoder": 352,
        "latch": 768, "buffer": 2137, "arith": 107,
        "counter": 732, "shift": 597,
        "sram": 172, "eeprom": 174, "switch": 2070, "led": 1, "7seg": 60, "oscillator": 1145, "supply": 18,
        "cpu": 21, "pio": 7, "serial": 247, "charlcd": 72, "connector": 951, "converter": 1914, "rtc": 16,
        "diode": 11166, "header": 80, "register": 220,
        // plan-46-C.
        "resistor": 1975, "capacitor": 1032, "transistor": 1256, "mcu": 1424,
        "nixie": 22,
        "dekatron": 13,
        "lcd": 7,
        "serialmem": 92,
        "regulator": 603,
        "crystal": 604,
        "powerswitch": 817,
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
    func testResistor() { check("resistor") }
    func testCapacitor() { check("capacitor") }
    func testTransistor() { check("transistor") }
    func testMcu() { check("mcu") }
    func testNixie() { check("nixie") }
    func testDekatron() { check("dekatron") }
    func testLcd() { check("lcd") }
    func testSerialmem() { check("serialmem") }
    func testRegulator() { check("regulator") }
    func testCrystal() { check("crystal") }
    func testPowerswitch() { check("powerswitch") }
}
