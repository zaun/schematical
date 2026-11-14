import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The Trace tool's assist router (plan-38-L Phase 1).
@MainActor
final class AssistRouterTests: XCTestCase {
    private let rules = DesignRules.customDefault

    private func centre(_ doc: SchematicDocument, _ chip: PlacedChip, _ pin: String) throws -> CGPoint {
        let ref = try TestSupport.pin(chip, pin)
        let placed = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
        let designator = placed.part.pins[ref.index].designator
        let pad = try XCTUnwrap(placed.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// Two 8-pin headers in one row, 30 mm apart, pin 1 of each wired: the
    /// straight line from one pin 1 to the other runs over the first
    /// header's other seven pads (on no net).
    private func headers() throws -> (doc: SchematicDocument, from: CGPoint, to: CGPoint, key: PlaneNet) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: CGPoint(x: 400, y: 100))
        doc.moveFootprint(one.id, to: CGPoint(x: 15, y: 30))
        doc.moveFootprint(two.id, to: CGPoint(x: 45, y: 30))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "1")), .pin(try TestSupport.pin(two, "1"))))
        let from = try centre(doc, one, "1"), to = try centre(doc, two, "1")
        let copper = doc.connectivity()
        let net = try XCTUnwrap(doc.net(at: from, layer: .top, in: copper))
        return (doc, from, to, try XCTUnwrap(copper.planeNet(ofNet: net)))
    }

    /// The smallest gap from `points` (a trace `width` wide on Top) to any
    /// pad not on net `net`.
    private func foreignGap(_ doc: SchematicDocument, _ points: [CGPoint], width: Double, net: Int?) -> Double {
        let copper = doc.connectivity()
        var smallest = Double.infinity
        for (p, pad) in copper.pads.enumerated() where copper.netOfPad[p] != net || net == nil {
            let shape = CopperGeometry.shape(of: pad.pad, layers: nil, node: 0)
            for (a, b) in zip(points, points.dropFirst()) {
                let leg = CopperShape(core: .segment(a, b), radius: width / 2, layers: [.top], node: 1)
                smallest = min(smallest, leg.gap(to: shape))
            }
        }
        return smallest
    }

    func testARouteGoesAroundAForeignPad() throws {
        let (doc, from, to, _) = try headers()
        let copper = doc.connectivity()
        let net = doc.net(at: from, layer: .top, in: copper)
        XCTAssertLessThan(foreignGap(doc, [from, to], width: 0.254, net: net), 0, "the straight line crosses foreign pads")
        let assist = AssistRouter(document: doc, layer: .top, start: from, rules: rules)
        XCTAssertEqual(assist.net, net)
        let path = try XCTUnwrap(assist.route(from: from, to: to))
        XCTAssertEqual(path.first, from)
        XCTAssertEqual(path.last, to)
        XCTAssertGreaterThan(path.count, 2, "it bends around")
        XCTAssertGreaterThanOrEqual(foreignGap(doc, path, width: assist.width, net: net), assist.clearance - 1e-4)
    }

    func testASolidForeignWallHasNoRoute() throws {
        let (doc, from, to, _) = try headers()
        // A trace on no net from the board's top edge to its bottom edge,
        // between the headers.
        let ys = doc.board.outline.map(\.y)
        let x = (from.x + to.x) / 2 + 3
        XCTAssertNotNil(doc.addTrace(points: [CGPoint(x: x, y: ys.min()!), CGPoint(x: x, y: ys.max()!)], layer: .top, width: 10))
        let assist = AssistRouter(document: doc, layer: .top, start: from, rules: rules)
        XCTAssertNil(assist.route(from: from, to: to))
        // The Bottom layer has no wall.
        let bottom = AssistRouter(document: doc, layer: .bottom, start: from, rules: rules)
        XCTAssertNotNil(bottom.route(from: from, to: to))
    }

    func testARouteMayCrossAndEndOnItsOwnPads() throws {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: CGPoint(x: 100, y: 100))
        doc.moveFootprint(one.id, to: CGPoint(x: 20, y: 30))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "1")), .pin(try TestSupport.pin(one, "2"))))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "2")), .pin(try TestSupport.pin(one, "3"))))
        let from = try centre(doc, one, "1"), to = try centre(doc, one, "3")
        let assist = AssistRouter(document: doc, layer: .top, start: from, rules: rules)
        XCTAssertNotNil(assist.net)
        XCTAssertEqual(assist.route(from: from, to: to), [from, to], "straight over pin 2, its own")
    }

    func testAClassStartUsesItsWidthAndClearance() throws {
        let (doc, from, to, key) = try headers()
        let fat = doc.addNetClass(named: "Power")
        doc.setNetClassWidth(fat, 20)
        doc.setNetClassClearance(fat, 0.4)
        doc.assignNet(key, to: fat)
        let assist = AssistRouter(document: doc, layer: .top, start: from, rules: rules)
        XCTAssertEqual(assist.width, 20 * 0.0254, accuracy: 1e-9)
        XCTAssertEqual(assist.clearance, 0.4, accuracy: 1e-9)
        let path = try XCTUnwrap(assist.route(from: from, to: to))
        XCTAssertGreaterThanOrEqual(foreignGap(doc, path, width: assist.width, net: assist.net), 0.4 - 1e-4)
    }

    func testAStartOnNoNetTreatsEveryPadAsForeign() throws {
        let (doc, from, _, _) = try headers()
        let open = CGPoint(x: from.x, y: from.y + 10)
        let assist = AssistRouter(document: doc, layer: .top, start: open, rules: rules)
        XCTAssertNil(assist.net)
        XCTAssertNil(assist.route(from: open, to: from), "pin 1 is another net's pad to it")
        XCTAssertNotNil(assist.route(from: open, to: CGPoint(x: open.x + 10, y: open.y)))
    }

    func testAFUIsFastAndTheRouteDeterministic() throws {
        let doc = try Fixture.open("AFU")
        let copper = doc.connectivity()
        let gnd = try XCTUnwrap(copper.netNames.first { $0.value == "GND" }?.key)
        let pads = copper.netOfPad.filter { $0.value == gnd }.keys.sorted().map { copper.pads[$0] }
            .filter { $0.pad.drill > 0 || $0.side == .top }
        let start = try XCTUnwrap(pads.first).centre
        let end = try XCTUnwrap(pads.max { hypot($0.centre.x - start.x, $0.centre.y - start.y)
                                    < hypot($1.centre.x - start.x, $1.centre.y - start.y) }).centre
        var clock = Date()
        let assist = AssistRouter(document: doc, layer: .top, start: start, rules: rules)
        let built = Date().timeIntervalSince(clock)
        clock = Date()
        let path = try XCTUnwrap(assist.route(from: start, to: end), "GND across AFU")
        let routed = Date().timeIntervalSince(clock)
        print(String(format: "ASSIST AFU grid %dx%d, init %.3f s, route %.3f s (%.1f mm away), %d points",
                     assist.grid.nx, assist.grid.ny, built, routed, hypot(end.x - start.x, end.y - start.y), path.count))
        XCTAssertLessThan(built, 1)
        XCTAssertLessThan(routed, 0.25)
        // Past the cache: another leg, then this one again.
        _ = assist.route(from: start, to: CGPoint(x: start.x + 1, y: start.y))
        XCTAssertEqual(assist.route(from: start, to: end), path)
        XCTAssertEqual(AssistRouter(document: doc, layer: .top, start: start, rules: rules).route(from: start, to: end), path)
    }
}
