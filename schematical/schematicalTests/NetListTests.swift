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

    // MARK: Phase 2 — highlight lookup and filter

    func testCopperItemsAreExactlyTheNetsCopper() throws {
        let (doc, _, k, a) = try wiredPair()
        let from = try centre(doc, k), to = try centre(doc, a)
        let routed = try XCTUnwrap(doc.addTrace(points: [from, CGPoint(x: from.x, y: from.y + 6),
                                                         CGPoint(x: to.x, y: from.y + 6), to], layer: .top))
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: from.x + 2, y: from.y + 6)))
        // Copper on no net: a lone trace and via far away.
        let stray = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 5, y: 75), CGPoint(x: 15, y: 75)], layer: .top))
        let lone = try XCTUnwrap(doc.addVia(at: CGPoint(x: 30, y: 75)))
        let key = try XCTUnwrap(doc.netRows().first?.key)
        let items = try XCTUnwrap(doc.copperItems(ofNet: key))
        XCTAssertEqual(items.pads.count, 2)
        XCTAssertTrue(items.traces.contains(routed))
        XCTAssertFalse(items.traces.contains(stray))
        XCTAssertEqual(items.vias, [via])
        XCTAssertFalse(items.vias.contains(lone))
        let copper = doc.connectivity()
        XCTAssertEqual(Set(items.pads.map { copper.netOfPad[$0] }), [copper.net(of: key)])
    }

    func testFilterMatchesNameAndPadLabel() throws {
        let row = NetRow(net: 0, key: .ground, name: "GND", pads: 2, unrouted: 1, classID: nil, layerRule: nil,
                         groupID: nil, padLabels: ["U1.7", "U2.14"])
        XCTAssertTrue(row.matches(""))
        XCTAssertTrue(row.matches("  "))
        XCTAssertTrue(row.matches("gn"))
        XCTAssertTrue(row.matches("u2.1"))
        XCTAssertFalse(row.matches("VCC"))
    }

    func testLayerRuleMenuToggles() {
        let only = NetLayerRuleMenu.toggled(nil, .top, mode: .only)
        XCTAssertEqual(only, LayerRule(mode: .only, layers: [.top]))
        let both = NetLayerRuleMenu.toggled(only, .bottom, mode: .only)
        XCTAssertEqual(both, LayerRule(mode: .only, layers: [.top, .bottom]))
        XCTAssertEqual(NetLayerRuleMenu.toggled(both, .top, mode: .only), LayerRule(mode: .only, layers: [.bottom]))
        XCTAssertNil(NetLayerRuleMenu.toggled(only, .top, mode: .only), "the last layer off is no rule")
        XCTAssertEqual(NetLayerRuleMenu.toggled(both, .bottom, mode: .prefer), LayerRule(mode: .prefer, layers: [.bottom]))
        XCTAssertEqual(NetLayerRuleMenu.title(both), "Only Top, Bottom")
        XCTAssertEqual(NetLayerRuleMenu.title(nil), "Any")
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
