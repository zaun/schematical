import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Net lengths and the DRC's length-match line (plan-38-I Phase 1).
@MainActor
final class NetLengthTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func centre(_ doc: SchematicDocument, _ chip: PlacedChip, _ pin: String) throws -> CGPoint {
        let current = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
        let designator = try XCTUnwrap(current.part.pins.first { $0.name == pin }).designator
        let pad = try XCTUnwrap(current.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// LEDs in a row on the board, `pairs` pairs each wired K to the next's
    /// A; returns each pair's K and A pad centres and its net key.
    private func pairs(_ count: Int, extra: Bool = false) throws
        -> (doc: SchematicDocument, ends: [(k: CGPoint, a: CGPoint, key: PlaneNet)]) {
        let doc = TestSupport.newDocument()
        var ends: [(CGPoint, CGPoint, PlaneNet)] = []
        for n in 0..<count {
            let one = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100 + 300 * Double(n)))
            let two = try TestSupport.place(doc, "GENERIC-LED", at: p(400, 100 + 300 * Double(n)))
            doc.moveFootprint(one.id, to: p(20, 15 + 15 * Double(n)))
            doc.moveFootprint(two.id, to: p(50, 15 + 15 * Double(n)))
            let k = try TestSupport.pin(one, "K")
            XCTAssertNil(doc.connect(.pin(k), .pin(try TestSupport.pin(two, "A"))))
            if extra, n == 0 {
                let three = try TestSupport.place(doc, "GENERIC-LED", at: p(700, 100))
                doc.moveFootprint(three.id, to: p(80, 15))
                XCTAssertNil(doc.connect(.pin(k), .pin(try TestSupport.pin(three, "A"))))
            }
            ends.append((try centre(doc, one, "K"), try centre(doc, two, "A"),
                         .pin(chip: one.id, designator: one.part.pins[k.index].designator)))
        }
        return (doc, ends)
    }

    /// Routes a pair below its pads, `detour` mm deeper than 5 mm.
    @discardableResult
    private func route(_ doc: SchematicDocument, _ k: CGPoint, _ a: CGPoint, detour: Double = 0) throws -> [CGPoint] {
        let y = k.y + 5 + detour / 2
        let points = [k, p(k.x, y), p(a.x, y), a]
        XCTAssertNotNil(doc.addTrace(points: points, layer: .bottom))
        return points
    }

    func testARoutedTwoPinNetHasItsTracesLength() throws {
        let (doc, ends) = try pairs(1)
        let points = try route(doc, ends[0].k, ends[0].a)
        let copper = doc.connectivity()
        let net = try XCTUnwrap(copper.net(of: ends[0].key))
        XCTAssertEqual(copper.pinCount(ofNet: net), 2)
        let expected = RoutingLength.of(segments: Array(zip(points, points.dropFirst())))
        XCTAssertEqual(try XCTUnwrap(doc.netLength(ofNet: net, in: copper)), expected, accuracy: 1e-9)
        XCTAssertEqual(expected, 5 + abs(Double(ends[0].a.x - ends[0].k.x)) + 5, accuracy: 1e-9)
    }

    func testUnroutedAndThreePinNetsHaveNoLength() throws {
        let (doc, ends) = try pairs(1)
        var copper = doc.connectivity()
        var net = try XCTUnwrap(copper.net(of: ends[0].key))
        XCTAssertNil(doc.netLength(ofNet: net, in: copper), "unrouted: two groups")

        let (three, threeEnds) = try pairs(1, extra: true)
        try route(three, threeEnds[0].k, threeEnds[0].a)
        copper = three.connectivity()
        net = try XCTUnwrap(copper.net(of: threeEnds[0].key))
        XCTAssertEqual(copper.pinCount(ofNet: net), 3)
        XCTAssertNil(three.netLength(ofNet: net, in: copper))
    }

    /// Two pairs routed 1.0 mm apart in length, in one group.
    private func grouped(tolerance: Double) throws -> (SchematicDocument, [DesignRuleIssue]) {
        let (doc, ends) = try pairs(2)
        try route(doc, ends[0].k, ends[0].a)
        try route(doc, ends[1].k, ends[1].a, detour: 1)
        let group = doc.addNetGroup(named: "Bus", mode: .lengthMatch)
        for end in ends { XCTAssertTrue(doc.assignNet(end.key, toGroup: group)) }
        doc.setNetGroupTolerance(group, tolerance)
        return (doc, doc.designRuleCheck(rules: .customDefault).filter { $0.kind == .lengthMatch })
    }

    func testAGroupOutOfToleranceIsOneLine() throws {
        let (doc, lines) = try grouped(tolerance: 0.25)
        XCTAssertEqual(lines.count, 1, "\(lines)")
        let line = try XCTUnwrap(lines.first)
        XCTAssertEqual(line.reference, "Bus")
        XCTAssertTrue(line.issue.hasPrefix("Group Bus: nets differ by 1"), line.issue)
        let copper = doc.connectivity()
        let names = doc.board.netGroups[0].members.compactMap { copper.net(of: $0).flatMap { copper.netNames[$0] } }
        for name in names { XCTAssertTrue(line.issue.contains(name), "\(name) in \(line.issue)") }
        XCTAssertFalse(line.items.isEmpty)
        XCTAssertEqual(try grouped(tolerance: 2).1, [])
    }

    func testAMemberWithMoreThanTwoPinsIsReported() throws {
        let (doc, ends) = try pairs(1, extra: true)
        let group = doc.addNetGroup(named: "G", mode: .lengthMatch)
        XCTAssertTrue(doc.assignNet(ends[0].key, toGroup: group))
        let lines = doc.designRuleCheck(rules: .customDefault).filter { $0.kind == .lengthMatch }
        XCTAssertEqual(lines.count, 1)
        XCTAssertTrue(lines[0].issue.hasSuffix("has more than two pins; its length is not matched"), lines[0].issue)
    }
}
