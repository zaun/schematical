import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The PCB sidebar's Nets section (plan-38-C): its rows, the highlight's
/// copper lookup and the filter.
@MainActor
final class NetListTests: XCTestCase {
    private func centre(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
        let designator = chip.part.pins[ref.index].designator
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// Two LEDs, K of the first wired to A of the second.
    private func wiredPair() throws -> (doc: SchematicDocument, one: PlacedChip, k: PinRef, a: PinRef) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        let k = try TestSupport.pin(one, "K"), a = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(k), .pin(a)))
        return (doc, one, k, a)
    }

    // MARK: Phase 1 — rows

    func testWiredPairIsOneRow() throws {
        let (doc, one, k, a) = try wiredPair()
        let rows = doc.netRows()
        XCTAssertEqual(rows.count, 1, "\(rows)")
        let row = try XCTUnwrap(rows.first)
        XCTAssertEqual(row.pads, 2)
        XCTAssertEqual(row.unrouted, 1)
        XCTAssertEqual(row.key, .pin(chip: one.id, designator: one.part.pins[k.index].designator))
        XCTAssertNil(row.classID)
        XCTAssertNil(row.groupID)

        XCTAssertNotNil(doc.addTrace(from: try centre(doc, k), to: try centre(doc, a), layer: .top, diagonalFirst: false))
        XCTAssertEqual(doc.netRows().first?.unrouted, 0)
    }

    func testGroundNetKeyIsGroundAndRailsComeFirst() throws {
        let (doc, _, _, _) = try wiredPair()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 700, y: 100))
        let other = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 1000, y: 100))
        for chip in [led, other] {
            let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: chip.origin.x, y: 400))
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(chip, "K")), .pin(PinRef(chipID: gnd.id, index: 0))))
        }
        let rows = doc.netRows()
        XCTAssertEqual(rows.count, 2)
        XCTAssertEqual(rows.first?.key, .ground)
        XCTAssertEqual(rows.first?.name, "GND")
        XCTAssertEqual(rows.first?.isRail, true)
    }

    func testClassAssignmentShowsInTheRow() throws {
        let (doc, _, _, _) = try wiredPair()
        let key = try XCTUnwrap(doc.netRows().first?.key)
        let power = doc.addNetClass(named: "Power")
        doc.assignNet(key, to: power)
        XCTAssertEqual(doc.netRows().first?.classID, power)
        doc.setNetLayerRule(key, LayerRule(mode: .only, layers: [.top]))
        XCTAssertEqual(doc.netRows().first?.layerRule, LayerRule(mode: .only, layers: [.top]))
        let group = doc.addNetGroup(named: "G", mode: .lengthMatch)
        XCTAssertTrue(doc.assignNet(key, toGroup: group))
        XCTAssertEqual(doc.netRows().first?.groupID, group)
    }
}
