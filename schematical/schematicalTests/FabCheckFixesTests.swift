import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// What the design rule check and the connectivity must agree with the fab on:
/// copper near a non-plated hole, a net class's clearance, the shape of a
/// pad's copper, and the check before a Gerber export.
@MainActor
final class FabCheckFixesTests: XCTestCase {
    private let rules = DesignRules.pcbway

    private func centre(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
        let designator = chip.part.pins[ref.index].designator
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// Two LEDs, K of one wired to A of the other, routed down, across and up on Top.
    private func routedPair(at origin: CGPoint, lift: Double = 6) throws
        -> (doc: SchematicDocument, one: PlacedChip, k: PinRef, across: Double) {
        let doc = TestSupport.newDocument()
        return try addRoutedPair(to: doc, at: origin, lift: lift)
    }

    private func addRoutedPair(to doc: SchematicDocument, at origin: CGPoint, lift: Double)
        throws -> (doc: SchematicDocument, one: PlacedChip, k: PinRef, across: Double) {
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100 + origin.y * 10))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200 + origin.y * 10))
        doc.moveFootprint(one.id, to: origin)
        doc.moveFootprint(two.id, to: CGPoint(x: origin.x + 30, y: origin.y))
        let k = try TestSupport.pin(one, "K"), a = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(k), .pin(a)))
        let from = try centre(doc, k), to = try centre(doc, a)
        XCTAssertNotNil(doc.addTrace(points: [from, CGPoint(x: from.x, y: from.y + lift),
                                              CGPoint(x: to.x, y: from.y + lift), to], layer: .top))
        return (doc, one, k, from.y + lift)
    }

    private func problems(_ doc: SchematicDocument, containing text: String) -> [DesignRuleIssue] {
        doc.designRuleCheck(rules: rules).filter { $0.issue.contains(text) }
    }

    // MARK: Non-plated holes

    func testTraceOverANonPlatedHoleIsListed() throws {
        let (doc, _, k, across) = try routedPair(at: CGPoint(x: 30, y: 40))
        XCTAssertEqual(problems(doc, containing: "non-plated hole"), [])
        let id = doc.addHole(at: .zero)
        XCTAssertFalse(try XCTUnwrap(doc.holes.first).plated, "the first hole is a non-plated M3")
        let from = try centre(doc, k)
        doc.setHoleCentre(id, CGPoint(x: from.x + 10, y: across))
        let found = problems(doc, containing: "non-plated hole")
        XCTAssertEqual(found.count, 1, "\(found)")
        XCTAssertTrue(found.first?.issue.contains("runs over the non-plated hole") == true, found.first?.issue ?? "")
        XCTAssertTrue(found.first?.items.contains(.hole(id)) == true)
    }

    func testTraceNearANonPlatedHoleKeepsTheCopperToEdgeDistance() throws {
        let (doc, _, k, across) = try routedPair(at: CGPoint(x: 30, y: 40))
        let id = doc.addHole(at: .zero)
        let hole = try XCTUnwrap(doc.holes.first)
        let radius = hole.drill / 2
        let width = try XCTUnwrap(doc.traces.first).widthMM
        let from = try centre(doc, k)
        // The hole's edge 0.1 mm from the trace's edge: under the 0.25 minimum.
        doc.setHoleCentre(id, CGPoint(x: from.x + 10, y: across + width / 2 + radius + 0.1))
        let near = problems(doc, containing: "non-plated hole")
        XCTAssertEqual(near.count, 1, "\(near)")
        XCTAssertTrue(near.first?.issue.contains("min 0.25 mm") == true, near.first?.issue ?? "")
        // 0.4 mm away is clear.
        doc.setHoleCentre(id, CGPoint(x: from.x + 10, y: across + width / 2 + radius + 0.4))
        XCTAssertEqual(problems(doc, containing: "non-plated hole"), [])
    }

    // MARK: Net class clearance

    func testANetClassClearanceHoldsHandDrawnCopper() throws {
        let (doc, one, k, across) = try routedPair(at: CGPoint(x: 30, y: 40))
        // A second net, its middle run 0.45 mm under the first's, centre to centre.
        let second = try addRoutedPair(to: doc, at: CGPoint(x: 33, y: 65), lift: 0)
        let from = try centre(doc, try TestSupport.pin(second.one, "K"))
        let secondTrace = try XCTUnwrap(doc.traces.last)
        let to = try XCTUnwrap(secondTrace.points.last)
        let width = secondTrace.widthMM
        // Replace the second net's route with one running 0.45 mm below the first's.
        doc.deleteTrace(secondTrace.id)
        XCTAssertNotNil(doc.addTrace(points: [from, CGPoint(x: from.x, y: across + 0.45),
                                              CGPoint(x: to.x, y: across + 0.45), to], layer: .top))
        _ = (one, k)
        let gap = 0.45 - width
        XCTAssertGreaterThan(gap, rules.minClearance, "the board's own minimum is met")
        XCTAssertEqual(problems(doc, containing: "Clearance"), [])

        let id = doc.addNetClass(named: "Wide")
        doc.setNetClassClearance(id, 0.5)
        doc.assignNet(.pin(chip: one.id, designator: one.part.pins[k.index].designator), to: id)
        let found = problems(doc, containing: "Clearance")
        XCTAssertFalse(found.isEmpty, "the class's clearance is held, not only the board's minimum")
        XCTAssertTrue(found.allSatisfy { $0.issue.hasSuffix("min 0.5 mm") }, "\(found)")
    }

    // MARK: A pad's copper

    private func pad(_ rect: CGRect, shape: Footprint.PadShape, radius: Double = 0) -> PlacedPad {
        PlacedPad(designator: "1", rect: rect, drill: 0, shape: shape, radius: radius, chamfer: 0, corners: [])
    }

    func testALineClippingARoundedCornerDoesNotTouchThePad() {
        let rounded = pad(CGRect(x: -1, y: -1, width: 2, height: 2), shape: .rect, radius: 0.8)
        let corner = CGPoint(x: 0.95, y: 0.95)
        XCTAssertFalse(SchematicDocument.point(corner, touches: rounded), "inside the box, off the rounded corner")
        XCTAssertTrue(SchematicDocument.point(CGPoint(x: 0.95, y: 0), touches: rounded))
        XCTAssertFalse(SchematicDocument.circle(corner, 0.05, meets: rounded))
        XCTAssertTrue(SchematicDocument.circle(corner, 0.5, meets: rounded))
    }

    func testAnOvalPadIsAStadiumInEveryPlace() throws {
        let oval = pad(CGRect(x: -2, y: -1, width: 4, height: 2), shape: .round)
        // Inside a stadium's end, outside an ellipse of the same box.
        let point = CGPoint(x: 1.5, y: 0.75)
        XCTAssertTrue(SchematicDocument.point(point, touches: oval))
        XCTAssertTrue(PCBCanvasView.outline(of: oval).contains(point), "the page draws, and the Gerber exports, the stadium")
        XCTAssertFalse(SchematicDocument.point(CGPoint(x: 1.9, y: 0.9), touches: oval))
        XCTAssertFalse(PCBCanvasView.outline(of: oval).contains(CGPoint(x: 1.9, y: 0.9)))
        let circle = pad(CGRect(x: -1, y: -1, width: 2, height: 2), shape: .round)
        XCTAssertFalse(SchematicDocument.point(CGPoint(x: 0.9, y: 0.9), touches: circle))
    }

    // MARK: Before a Gerber export

    func testTheExportWarningNamesWhatTheCheckFound() {
        func issue(_ kind: DesignRuleIssue.Kind) -> DesignRuleIssue {
            DesignRuleIssue(kind: kind, reference: "", issue: "", items: [])
        }
        XCTAssertEqual(SchematicExporter.problemSummary([issue(.short)]), "The check found 1 short circuit.")
        XCTAssertEqual(SchematicExporter.problemSummary([issue(.short), issue(.unrouted), issue(.unrouted), issue(.edge)]),
                       "The check found 1 short circuit, 2 unrouted connections and 1 other problem.")
    }
}
