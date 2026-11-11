import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// A single diode's footprint shows a cathode band at its cathode end.
@MainActor
final class CathodeBandTests: XCTestCase {
    /// DO-41: pin 1 (K) is the band end. The band crosses the body, nearer
    /// the cathode pad than the anode pad, and follows the part when turned.
    func testBandSitsAtTheCathodeEnd() throws {
        var chip = try TestSupport.place(TestSupport.newDocument(), "1N4001-E3/54", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(chip.footprint?.id, "DIODE-DO-41")
        var placement = try XCTUnwrap(chip.board)
        for rotation in [0, 90, 180, 270] {
            placement.rotation = rotation
            chip.board = placement
            let band = try XCTUnwrap(chip.cathodeBand)
            let mid = CGPoint(x: (band.a.x + band.b.x) / 2, y: (band.a.y + band.b.y) / 2)
            let k = try XCTUnwrap(chip.boardPads.first { $0.designator == "1" }).rect
            let a = try XCTUnwrap(chip.boardPads.first { $0.designator == "2" }).rect
            XCTAssertLessThan(hypot(mid.x - k.midX, mid.y - k.midY), hypot(mid.x - a.midX, mid.y - a.midY),
                              "rotation \(rotation)")
            let body = try XCTUnwrap(chip.boardBody).rect.insetBy(dx: -1e-9, dy: -1e-9)
            XCTAssertTrue(body.contains(band.a) && body.contains(band.b), "rotation \(rotation)")
        }
    }

    /// Parts that aren't a single diode get no band.
    func testOtherPartsHaveNoBand() throws {
        let chip = try TestSupport.place(TestSupport.newDocument(), "SN74HC00D", at: CGPoint(x: 100, y: 100))
        XCTAssertNil(chip.cathodeBand)
    }
}
