import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The PCB's red unrouted lines (`SchematicDocument.ratsnest`).
@MainActor
final class RatsnestTests: XCTestCase {
    private func centre(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
        let designator = chip.part.pins[ref.index].designator
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    private func same(_ a: CGPoint, _ b: CGPoint) -> Bool { hypot(a.x - b.x, a.y - b.y) < 1e-6 }

    /// Two LEDs, K of one wired to A of the other in the schematic.
    private func wiredPair() throws -> (doc: SchematicDocument, k: CGPoint, a: CGPoint) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        let k = try TestSupport.pin(one, "K"), a = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(k), .pin(a)))
        return (doc, try centre(doc, k), try centre(doc, a))
    }

    func testUnroutedWireIsOneLineBetweenItsPads() throws {
        let (doc, k, a) = try wiredPair()
        let lines = doc.ratsnest()
        XCTAssertEqual(lines.count, 1)
        let line = try XCTUnwrap(lines.first)
        XCTAssertTrue(same(line.0, k) && same(line.1, a) || same(line.0, a) && same(line.1, k), "\(lines)")
    }

    func testUnwiredPartsHaveNoLines() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        XCTAssertTrue(doc.ratsnest().isEmpty)
    }

    /// A trace past the corner of a round pad's box does not touch the pad
    /// (the pads of a tube sit on a circle, so traces pass them diagonally).
    func testALinePastARoundPadsCornerDoesNotTouchIt() throws {
        let doc = TestSupport.newDocument()
        let tube = try TestSupport.place(doc, "IN-14", at: CGPoint(x: 100, y: 100))
        let round = try XCTUnwrap(tube.boardPads.first { $0.designator == "2" })
        let square = try XCTUnwrap(tube.boardPads.first { $0.designator == "1" })
        XCTAssertEqual(round.shape, .round)
        XCTAssertEqual(square.shape, .rect)
        for (pad, cornerTouches) in [(round, false), (square, true)] {
            let r = pad.rect
            // Across the top-left corner, 10% of the pad inside its box.
            let a = CGPoint(x: r.minX - 1, y: r.minY + 0.1 * r.height + 1 + 0.1 * r.width)
            let b = CGPoint(x: r.minX + 0.1 * r.width + 1 + 0.1 * r.height, y: r.minY - 1)
            XCTAssertEqual(SchematicDocument.segment(a, b, touches: pad), cornerTouches, "\(pad.shape)")
            // Through the centre.
            XCTAssertTrue(SchematicDocument.segment(CGPoint(x: r.minX - 1, y: r.midY),
                                                    CGPoint(x: r.maxX + 1, y: r.midY), touches: pad))
        }
    }

    func testTraceBetweenThePadsRoutesIt() throws {
        let (doc, k, a) = try wiredPair()
        XCTAssertNotNil(doc.addTrace(from: k, to: a, layer: .top, diagonalFirst: false))
        XCTAssertTrue(doc.ratsnest().isEmpty)
    }

    func testTraceOnTheWrongLayerDoesNotRouteSurfacePads() throws {
        let (doc, k, a) = try wiredPair()
        let chip = try XCTUnwrap(doc.chips.first)
        // Through-hole pads reach every layer; only check SMD parts.
        guard chip.boardPads.allSatisfy({ $0.drill == 0 }) else { return }
        XCTAssertNotNil(doc.addTrace(from: k, to: a, layer: .bottom, diagonalFirst: false))
        XCTAssertEqual(doc.ratsnest().count, 1)
    }

    func testTraceBeingDrawnMovesTheLineToItsEnd() throws {
        let (doc, k, a) = try wiredPair()
        let end = CGPoint(x: k.x + 5.08, y: k.y)
        let drawing = BoardTrace(id: UUID(), layer: .top, width: BoardTrace.defaultWidth, points: [k, end])
        let lines = doc.ratsnest(drawing: drawing)
        XCTAssertEqual(lines.count, 1)
        let line = try XCTUnwrap(lines.first)
        XCTAssertTrue(same(line.0, end) && same(line.1, a) || same(line.0, a) && same(line.1, end), "\(lines)")
    }

    func testSpanningLinesJoinNearestGroups() {
        let groups: [[CGPoint]] = [[CGPoint(x: 0, y: 0)], [CGPoint(x: 10, y: 0)], [CGPoint(x: 1, y: 0)]]
        let lines = SchematicDocument.spanningLines(groups)
        XCTAssertEqual(lines.count, 2)
        // 0–1 then 1–10, never the long 0–10.
        XCTAssertFalse(lines.contains { Set([$0.0.x, $0.1.x]) == [0, 10] })
    }

    private func isLine(_ line: (CGPoint, CGPoint), _ a: CGPoint, _ b: CGPoint) -> Bool {
        same(line.0, a) && same(line.1, b) || same(line.0, b) && same(line.1, a)
    }

    /// Two LEDs, each `pin` wired to a supply symbol of its own.
    private func onSupplies(_ pin: String, _ one: String, _ two: String) throws -> (SchematicDocument, CGPoint, CGPoint) {
        let doc = TestSupport.newDocument()
        var centres: [CGPoint] = []
        for (n, supply) in [one, two].enumerated() {
            let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100 + 300 * n, y: 100))
            let rail = try TestSupport.place(doc, supply, at: CGPoint(x: 100 + 300 * n, y: 400))
            let ref = try TestSupport.pin(led, pin)
            XCTAssertNil(doc.connect(.pin(ref), .pin(PinRef(chipID: rail.id, index: 0))))
            centres.append(try centre(doc, ref))
        }
        return (doc, centres[0], centres[1])
    }

    func testGroundSymbolsAreOneNet() throws {
        let (doc, a, b) = try onSupplies("K", "GND", "GND")
        let lines = doc.ratsnest()
        XCTAssertEqual(lines.count, 1)
        XCTAssertTrue(isLine(try XCTUnwrap(lines.first), a, b), "\(lines)")
    }

    func testPowerSymbolsOfOneVoltageAreOneNet() throws {
        let (doc, a, b) = try onSupplies("A", "5V", "5V")
        let lines = doc.ratsnest()
        XCTAssertEqual(lines.count, 1)
        XCTAssertTrue(isLine(try XCTUnwrap(lines.first), a, b), "\(lines)")
    }

    func testDifferentVoltagesStayApart() throws {
        let (doc, _, _) = try onSupplies("A", "5V", "3V3")
        XCTAssertTrue(doc.ratsnest().isEmpty)
    }

    /// A pull-up's far pad is on its rail: it gets a line to a pin wired
    /// to that rail's symbol, and to another pull-up's far pad.
    func testPullResistorFarPadIsOnItsRail() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let rail = try TestSupport.place(doc, "5V", at: CGPoint(x: 100, y: 400))
        let anode = try TestSupport.pin(led, "A")
        XCTAssertNil(doc.connect(.pin(anode), .pin(PinRef(chipID: rail.id, index: 0))))
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 400, y: 100))
        let far = try XCTUnwrap(pull.boardPads.first { $0.designator == "2" })
        let lines = doc.ratsnest()
        XCTAssertEqual(lines.count, 1)
        XCTAssertTrue(isLine(try XCTUnwrap(lines.first), try centre(doc, anode),
                             CGPoint(x: far.rect.midX, y: far.rect.midY)), "\(lines)")

        try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 700, y: 100))
        XCTAssertEqual(doc.ratsnest().count, 2)
    }

    /// plan-68-AJ: a pull-up's far pad (one no pin names) is on its rail
    /// even with no supply symbol of that voltage on the schematic.
    func testAPullUpsFarPadIsOnItsRailAlone() throws {
        let doc = TestSupport.newDocument()
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 200, y: 200))
        let placed = try XCTUnwrap(doc.chips.first { $0.id == pull.id })
        let pins = Set(placed.part.pins.map(\.designator))
        let far = try XCTUnwrap(placed.boardPads.first { $0.designator.map { !pins.contains($0) } ?? false },
                                "a pad no pin names")
        let copper = doc.connectivity()
        let rail = try XCTUnwrap(copper.railNets[5000], "the 5 V rail has a net: \(copper.railNets)")
        let index = try XCTUnwrap(copper.pads.indices.first { copper.pads[$0].chip == 0 && copper.pads[$0].pad.designator == far.designator })
        XCTAssertEqual(copper.netOfPad[index], rail)
    }
}
