import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-68-W: Design Rule Check lines the other classes don't reach — small
/// holes and thin rings, holes of one part too close, silkscreen too thin
/// or too short, copper joining unwired pins, a via joined to no pad, and
/// copper shorting a net to a loose pin.
@MainActor
final class DesignRuleCheckEdgeTests: XCTestCase {
    private func centre(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
        let designator = chip.part.pins[ref.index].designator
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// Two LEDs on the board, K of one wired to A of the other.
    private func pair() throws -> (doc: SchematicDocument, one: PlacedChip, two: PlacedChip) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "K")), .pin(try TestSupport.pin(two, "A"))))
        return (doc, one, two)
    }

    private func issues(_ doc: SchematicDocument, _ change: (inout DesignRules) -> Void = { _ in }) -> [DesignRuleIssue] {
        var rules = DesignRules.customDefault
        change(&rules)
        return doc.designRuleCheck(rules: rules)
    }

    func testSmallHolesThinRingsAndHolesTooClose() throws {
        let (doc, one, _) = try pair()
        let found = issues(doc) { $0.minDrill = 5; $0.minAnnularRing = 5; $0.minHoleToHole = 20 }
        let drill = try XCTUnwrap(found.first { $0.kind == .drill && $0.reference == one.reference })
        XCTAssertTrue(drill.issue.hasPrefix("Hole ") && drill.issue.contains("pads ") && drill.issue.contains("min drill 5"),
                      drill.issue)
        let ring = try XCTUnwrap(found.first { $0.kind == .annularRing && $0.reference == one.reference })
        XCTAssertTrue(ring.issue.contains("Annular ring") && ring.issue.contains("min 5"), ring.issue)
        XCTAssertTrue(found.contains { $0.kind == .holeToHole && $0.reference == one.reference && $0.issue.hasPrefix("Holes of pads") },
                      "\(found.map(\.issue))")
    }

    func testSilkscreenTooThinOrTooShort() throws {
        let (doc, one, _) = try pair()
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 700, y: 300))
        doc.moveFootprint(gate.id, to: CGPoint(x: 45, y: 70))
        doc.setFootprintMarks([one.id, gate.id], outline: .silkscreen, reference: .silkscreen)
        let text = doc.addText(at: CGPoint(x: 20, y: 70))
        doc.setTextLayer(text, .silkscreen)
        doc.setTextSize(text, 1)
        let found = issues(doc) { $0.minSilkscreenWidth = 1; $0.minSilkscreenHeight = 5 }.filter { $0.kind == .silkscreen }
        XCTAssertTrue(found.contains { $0.issue.hasPrefix("Outline line") && $0.reference.contains(one.reference)
            && $0.reference.contains(gate.reference) }, "\(found.map { "\($0.reference): \($0.issue)" })")
        XCTAssertTrue(found.contains { $0.issue.hasPrefix("Reference text") }, "\(found.map(\.issue))")
        XCTAssertTrue(found.contains { $0.issue.hasPrefix("Text ") && $0.issue.contains("tall") }, "\(found.map(\.issue))")
        XCTAssertTrue(found.contains { $0.issue.hasPrefix("Text strokes about") }, "\(found.map(\.issue))")
    }

    func testCopperThatJoinsUnwiredPinsOrNoPad() throws {
        let (doc, one, two) = try pair()
        let a = try centre(doc, try TestSupport.pin(one, "A")), k = try centre(doc, try TestSupport.pin(two, "K"))
        let unwired = try XCTUnwrap(doc.addTrace(points: [a, CGPoint(x: a.x, y: a.y + 10), CGPoint(x: k.x, y: a.y + 10), k],
                                                 layer: .bottom))
        let found = issues(doc)
        XCTAssertTrue(found.contains { $0.kind == .unconnected && $0.items == [.trace(unwired)]
            && $0.issue.hasPrefix("Only joins pins that aren't wired in the schematic") }, "\(found.map(\.issue))")

        // A via with two traces, neither reaching a pad.
        let v = CGPoint(x: 45, y: 75)
        let via = try XCTUnwrap(doc.addVia(at: v))
        XCTAssertNotNil(doc.addTrace(points: [v, CGPoint(x: 40, y: 75)], layer: .top))
        XCTAssertNotNil(doc.addTrace(points: [v, CGPoint(x: 50, y: 75)], layer: .bottom))
        XCTAssertTrue(issues(doc).contains { $0.items == [.via(via)] && $0.issue == "Not connected to any pad" },
                      "\(issues(doc).map(\.issue))")
    }

    func testCopperJoiningANetToALoosePin() throws {
        let (doc, one, _) = try pair()
        let k = try centre(doc, try TestSupport.pin(one, "K")), a = try centre(doc, try TestSupport.pin(one, "A"))
        XCTAssertNotNil(doc.addTrace(points: [k, CGPoint(x: k.x, y: k.y + 5), CGPoint(x: a.x, y: k.y + 5), a], layer: .top))
        let short = try XCTUnwrap(issues(doc).first { $0.kind == .short && $0.issue.hasPrefix("Joined to the net of") },
                                  "\(issues(doc).map(\.issue))")
        XCTAssertTrue(short.issue.hasSuffix("but not wired to anything in the schematic"), short.issue)
    }
}
