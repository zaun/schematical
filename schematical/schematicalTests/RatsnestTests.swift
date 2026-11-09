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
}
