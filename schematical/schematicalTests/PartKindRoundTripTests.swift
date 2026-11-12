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
        "dff": 706, "jkff": 186, "mux": 346, "decoder": 322,
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
}
