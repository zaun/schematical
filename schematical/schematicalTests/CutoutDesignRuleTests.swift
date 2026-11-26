import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The design rule check and cutouts (plan-65-C): copper must clear them,
/// bodies may overhang them, and two cutout problems of their own.
@MainActor
final class CutoutDesignRuleTests: XCTestCase {
    private var rules: DesignRules {
        var rules = DesignRules.pcbway
        rules.minCopperToEdge = 0.3
        return rules
    }

    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect { CGRect(x: x, y: y, width: w, height: h) }
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func centre(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
        let designator = chip.part.pins[ref.index].designator
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// Two LEDs well inside the board, K of one wired to A of the other, at (30, 40) and (60, 40).
    private func wiredPair() throws -> (doc: SchematicDocument, one: PlacedChip, two: PlacedChip, k: PinRef, a: PinRef) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        let k = try TestSupport.pin(one, "K"), a = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(k), .pin(a)))
        return (doc, one, two, k, a)
    }

    private func edgeIssues(_ doc: SchematicDocument) -> [DesignRuleIssue] {
        doc.designRuleCheck(rules: rules).filter { $0.kind == .edge }
    }

    func testAPadInACutoutIsCopperInACutout() throws {
        let (doc, one, _, _, _) = try wiredPair()
        XCTAssertTrue(edgeIssues(doc).isEmpty)
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(24, 34, 12, 12), radius: 0)))
        let issues = edgeIssues(doc)
        XCTAssertFalse(issues.isEmpty)
        XCTAssertTrue(issues.contains { $0.issue == "Copper is in a cutout" && $0.items.contains(.footprint(one.id)) }, "\(issues)")
        XCTAssertFalse(issues.contains { $0.issue == "Copper is off the board" })
        doc.deleteCutout(id)
        XCTAssertTrue(edgeIssues(doc).isEmpty)
    }

    func testATraceTooCloseToACutoutAndOneFarEnough() throws {
        let (doc, _, _, k, a) = try wiredPair()
        let from = try centre(doc, k), to = try centre(doc, a)
        let trace = try XCTUnwrap(doc.addTrace(points: [from, p(from.x, from.y + 6), p(to.x, from.y + 6), to], layer: .top))
        let half = (try XCTUnwrap(doc.traces.first { $0.id == trace }).widthMM) / 2
        let runY = from.y + 6
        let midX = (from.x + to.x) / 2
        // The cutout's top edge 0.05 mm below the trace's lower edge (before snapping to the 0.3175 mm trace grid).
        let near = try XCTUnwrap(doc.addCutout(.rounded(rect: r(midX - 3, runY + half + 0.05, 6, 6), radius: 0)))
        let close = edgeIssues(doc)
        XCTAssertTrue(close.contains { $0.issue.hasPrefix("Copper ") && $0.issue.contains("from the board edge; min 0.3") }, "\(close)")
        doc.deleteCutout(near)
        _ = try XCTUnwrap(doc.addCutout(.rounded(rect: r(midX - 3, runY + half + 1.0, 6, 6), radius: 0)))
        XCTAssertTrue(edgeIssues(doc).isEmpty, "\(edgeIssues(doc))")
    }

    func testABodyOverACutoutIsNotAnIssue() throws {
        // A DIP's body sits between its two rows of pads: a window in the middle of it is over the body, clear of copper.
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        doc.moveFootprint(chip.id, to: CGPoint(x: 50, y: 40))
        let placed = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
        let body = try XCTUnwrap(placed.boardBody).rect
        let window = CGRect(x: body.midX - 1.5, y: body.midY - 1.5, width: 3, height: 3)
        let nearest = placed.boardPads.map { pad -> CGFloat in
            let dx = max(pad.rect.minX - window.maxX, window.minX - pad.rect.maxX, 0)
            let dy = max(pad.rect.minY - window.maxY, window.minY - pad.rect.maxY, 0)
            return hypot(dx, dy)
        }.min() ?? 0
        XCTAssertGreaterThan(nearest, 1, "the window is clear of every pad")
        let before = doc.designRuleCheck(rules: rules).filter { $0.kind == .edge || $0.kind == .placement }
        _ = try XCTUnwrap(doc.addCutout(.rounded(rect: window, radius: 0)))
        let after = doc.designRuleCheck(rules: rules).filter { $0.kind == .edge || $0.kind == .placement }
        XCTAssertEqual(after.map(\.issue), before.map(\.issue), "a body over a cutout adds no edge or placement issue")
    }

    func testCopperOffTheOutlineStillSaysOffTheBoard() throws {
        let (doc, _, _, _, _) = try wiredPair()
        _ = try XCTUnwrap(doc.addVia(at: p(-2, 10)))
        XCTAssertTrue(edgeIssues(doc).contains { $0.issue == "Copper is off the board" })
    }

    func testACutoutCrossingTheEdgeIsNamed() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(-5, 10, 10, 10), radius: 0)))
        let issues = edgeIssues(doc)
        XCTAssertEqual(issues.map(\.reference), ["Cutout 1"])
        XCTAssertEqual(issues.map(\.issue), ["Crosses the board edge"])
        XCTAssertEqual(issues.first?.items, [.cutout(id)])
    }

    func testTwoCutoutsThatOverlapAreNamedAndTwoApartAreNot() throws {
        let doc = TestSupport.newDocument()
        let a = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 10, 10), radius: 0)))
        let b = try XCTUnwrap(doc.addCutout(.rounded(rect: r(15, 12, 10, 10), radius: 0)))
        var issues = edgeIssues(doc)
        XCTAssertEqual(issues.map(\.reference), ["Cutouts 1, 2"])
        XCTAssertEqual(issues.map(\.issue), ["Overlap"])
        XCTAssertEqual(issues.first?.items, [.cutout(a), .cutout(b)])
        doc.moveCutout(b, to: p(40, 12))
        issues = edgeIssues(doc)
        XCTAssertTrue(issues.isEmpty, "\(issues)")
        // One inside the other, no edges crossing.
        doc.deleteCutout(b)
        _ = try XCTUnwrap(doc.addCutout(.rounded(rect: r(12, 12, 4, 4), radius: 0)))
        XCTAssertEqual(edgeIssues(doc).map(\.issue), ["Overlap"])
    }

    func testAFreeFormOutlineKeepsPlacementWithinItsShape() throws {
        let (doc, one, _, _, _) = try wiredPair()
        // An L that is missing the part of the board LED one is on: its x range 30 ± 5 and y range 40 ± 5.
        XCTAssertTrue(doc.setBoardOutline([p(0, 0), p(100, 0), p(100, 30), p(20, 30), p(20, 80), p(0, 80)]))
        let issues = doc.designRuleCheck(rules: rules)
        XCTAssertTrue(issues.contains { $0.kind == .placement && $0.reference == one.reference }, "\(issues)")
    }

    func testFortyRoundCutoutsAndFourHundredViasFinishInTime() throws {
        let doc = TestSupport.newDocument()
        for row in 0..<4 {
            for column in 0..<10 {
                _ = try XCTUnwrap(doc.addCutout(.circle(centre: p(5 + 9 * CGFloat(column), 5 + 9 * CGFloat(row)), diameter: 3.2)))
            }
        }
        for row in 0..<20 {
            for column in 0..<20 {
                doc.addVia(at: p(2 + 4.5 * CGFloat(column), 45 + 1.7 * CGFloat(row)))
            }
        }
        XCTAssertEqual(doc.cutouts.count, 40)
        XCTAssertGreaterThan(doc.vias.count, 300)
        let started = Date()
        _ = doc.designRuleCheck(rules: rules)
        let seconds = Date().timeIntervalSince(started)
        print("CUTOUT_DRC_SECONDS \(seconds)")
        XCTAssertLessThan(seconds, 10)
    }
}
