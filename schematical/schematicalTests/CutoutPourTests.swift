import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// A pour's fill stays out of cutouts and a free-form outline's notches
/// (plan-65-C).
@MainActor
final class CutoutPourTests: XCTestCase {
    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect { CGRect(x: x, y: y, width: w, height: h) }
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private var rules: DesignRules {
        var rules = DesignRules.customDefault
        rules.minClearance = 0.3
        rules.minCopperToEdge = 0.5
        return rules
    }

    private let outline = [CGPoint(x: 0, y: 0), CGPoint(x: 60, y: 0), CGPoint(x: 60, y: 40), CGPoint(x: 0, y: 40)]

    /// The fill of a pour over `rect` with one same-net pad (at (50, 10) unless `pad` says) on `edge`.
    private func fill(_ edge: BoardEdge, rect: CGRect = CGRect(x: 2, y: 2, width: 56, height: 36),
                      pad padRect: CGRect = CGRect(x: 49, y: 9, width: 2, height: 2)) throws -> PourFill {
        var pour = BoardPour(id: UUID(), rect: rect, layer: .top)
        pour.connection = .solid
        let pad = PourCopper.pad(0, rect: padRect, net: 1)
        let result = PourFiller.fill(pours: [pour], nets: [pour.id: 1], keepouts: [], copper: [pad], board: edge,
                                     edge: rules.minCopperToEdge, rules: rules)
        return try XCTUnwrap(result[pour.id])
    }

    /// Every point on a 0.25 mm grid of the fill is material and at least `distance` from every cutout segment.
    private func assertClear(_ fill: PourFill, of edge: BoardEdge, distance: Double, file: StaticString = #filePath,
                             line: UInt = #line) {
        var inFill = 0
        var x = 0.0
        while x <= 60 {
            var y = 0.0
            while y <= 40 {
                let point = p(x, y)
                if fill.path.contains(point, using: .winding) {
                    inFill += 1
                    XCTAssertTrue(edge.contains(point), "fill on non-material at \(point)", file: file, line: line)
                    for loop in edge.cutouts {
                        for (a, b) in zip(loop, loop.dropFirst() + loop.prefix(1)) {
                            let gap = Double(SchematicDocument.distance(from: point, toSegment: a, b))
                            XCTAssertGreaterThanOrEqual(gap, distance - 1e-6, "fill \(gap) mm from a cutout at \(point)",
                                                        file: file, line: line)
                        }
                    }
                }
                y += 0.25
            }
            x += 0.25
        }
        XCTAssertGreaterThan(inFill, 1000, "there is a fill", file: file, line: line)
    }

    func testARectangularCutoutHasAHoleAndTheCopperToEdgeBand() throws {
        let hole = BoardCutout.Shape.rounded(rect: r(20, 12, 10, 10), radius: 0).loop
        let edge = BoardEdge(outline: outline, cutouts: [hole])
        let result = try fill(edge)
        assertClear(result, of: edge, distance: 0.5)
        XCTAssertFalse(result.path.contains(p(25, 17), using: .winding), "inside the cutout")
        XCTAssertFalse(result.path.contains(p(19.8, 17), using: .winding), "0.2 mm from its edge")
        XCTAssertTrue(result.path.contains(p(15, 17), using: .winding), "5 mm away")
    }

    func testARoundCutout() throws {
        let hole = BoardCutout.Shape.circle(centre: p(30, 20), diameter: 6).loop
        let edge = BoardEdge(outline: outline, cutouts: [hole])
        let result = try fill(edge)
        assertClear(result, of: edge, distance: 0.5)
        XCTAssertFalse(result.path.contains(p(30, 20), using: .winding))
        XCTAssertTrue(result.path.contains(p(30, 12), using: .winding), "8 mm above the centre")
    }

    func testACutoutCrossingTheOutlineStillGivesAFill() throws {
        let notch = BoardCutout.Shape.rounded(rect: r(-5, 10, 15, 10), radius: 0).loop
        let edge = BoardEdge(outline: outline, cutouts: [notch])
        let result = try fill(edge)
        XCTAssertFalse(result.isEmpty)
        XCTAssertFalse(result.path.contains(p(5, 15), using: .winding), "in the notch")
        XCTAssertTrue(result.path.contains(p(30, 30), using: .winding))
    }

    func testAFreeFormOutlinesNotchIsKeptEmpty() throws {
        // An L: the top-right 20 × 15 is missing.
        let l = [p(0, 0), p(40, 0), p(40, 15), p(60, 15), p(60, 40), p(0, 40)]
        let result = try fill(BoardEdge(outline: l), pad: r(19, 4, 2, 2))
        XCTAssertFalse(result.path.contains(p(50, 5), using: .winding), "in the notch")
        XCTAssertFalse(result.path.contains(p(50, 14.8), using: .winding), "0.2 mm below the notch's edge")
        XCTAssertTrue(result.path.contains(p(50, 20), using: .winding))
        XCTAssertTrue(result.path.contains(p(20, 5), using: .winding))
    }

    func testAMovedCutoutRefillsTheKeptFill() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 100, y: 400))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
        doc.moveFootprint(led.id, to: CGPoint(x: 80, y: 60))
        let pour = doc.addPour(rect: CGRect(x: 0, y: 0, width: doc.board.width, height: doc.board.height), layer: .top)
        XCTAssertNotNil(pour)
        let cutout = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 10, 10), radius: 0)))
        PourFiller.fillCount = 0
        let first = doc.connectivity().pourFills
        XCTAssertEqual(PourFiller.fillCount, 1)
        _ = doc.connectivity()
        XCTAssertEqual(PourFiller.fillCount, 1, "nothing changed: kept")
        let centre = p(15, 15)
        XCTAssertFalse(try XCTUnwrap(first.values.first).path.contains(centre, using: .winding))

        doc.moveCutout(cutout, to: p(40, 10))
        let second = doc.connectivity().pourFills
        XCTAssertEqual(PourFiller.fillCount, 2, "a moved cutout refills")
        let path = try XCTUnwrap(second.values.first).path
        XCTAssertTrue(path.contains(centre, using: .winding), "the old place is copper now")
        XCTAssertFalse(path.contains(p(45, 15), using: .winding), "the new place is the hole")
    }
}
