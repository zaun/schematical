import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-16-D phase 2: anchors, joins and connectivity.
@MainActor
final class TraceJoinTests: XCTestCase {
    private func doc(layers: Int = 2) -> SchematicDocument {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(layers)
        return doc
    }

    private func trace(_ doc: SchematicDocument, _ id: UUID?) throws -> BoardTrace {
        let id = try XCTUnwrap(id)
        return try XCTUnwrap(doc.traces.first { $0.id == id })
    }

    private func has(_ trace: BoardTrace, _ point: CGPoint) -> Bool {
        trace.points.contains { TraceRoute.same($0, point) }
    }

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    func testCrossingOnTopJoinsBoth() throws {
        let doc = doc()
        let h = doc.addTrace(from: p(0, 5), to: p(10, 5), layer: .top, diagonalFirst: false)
        let v = doc.addTrace(from: p(5, 0), to: p(5, 10), layer: .top, diagonalFirst: false)
        XCTAssertEqual(try trace(doc, h).points, [p(0, 5), p(5, 5), p(10, 5)])
        XCTAssertEqual(try trace(doc, v).points, [p(5, 0), p(5, 5), p(5, 10)])
        XCTAssertEqual(Set(doc.connectedRun(of: try XCTUnwrap(h))), [h!, v!])
        for t in doc.traces { XCTAssertTrue(TraceRoute.isLegal(t.points)) }
    }

    func testDifferentLayersNeverJoin() throws {
        let doc = doc()
        let h = doc.addTrace(from: p(0, 5), to: p(10, 5), layer: .top, diagonalFirst: false)
        let v = doc.addTrace(from: p(5, 0), to: p(5, 10), layer: .bottom, diagonalFirst: false)
        XCTAssertEqual(try trace(doc, h).points.count, 2)
        XCTAssertEqual(try trace(doc, v).points.count, 2)
        XCTAssertEqual(doc.connectedRun(of: h!), [h!])
        XCTAssertEqual(doc.connectedRun(of: v!), [v!])
    }

    func testDiagonalsCrossOffGrid() throws {
        let doc = doc()
        // These cross at (0.3175, 0.3175): half a grid step.
        let a = doc.addTrace(from: p(0, 0), to: p(0.635, 0.635), layer: .top, diagonalFirst: false)
        let b = doc.addTrace(from: p(0, 0.635), to: p(0.635, 0), layer: .top, diagonalFirst: false)
        let cross = p(0.3175, 0.3175)
        XCTAssertTrue(has(try trace(doc, a), cross))
        XCTAssertTrue(has(try trace(doc, b), cross))
        XCTAssertEqual(Set(doc.connectedRun(of: a!)), [a!, b!])
    }

    func testCollinearOverlapSharesBothEnds() throws {
        let doc = doc()
        let a = doc.addTrace(from: p(0, 0), to: p(10, 0), layer: .top, diagonalFirst: false)
        let b = doc.addTrace(from: p(5, 0), to: p(15, 0), layer: .top, diagonalFirst: false)
        XCTAssertEqual(try trace(doc, a).points, [p(0, 0), p(5, 0), p(10, 0)])
        XCTAssertEqual(try trace(doc, b).points, [p(5, 0), p(10, 0), p(15, 0)])
    }

    func testEndOnSegmentLandsAndJoins() throws {
        let doc = doc()
        let a = doc.addTrace(from: p(0, 0), to: p(10, 0), layer: .top, diagonalFirst: false)
        // Released 0.1 mm off the segment, within tolerance: lands on it.
        let end = doc.traceAnchor(at: p(3.3, 0.1), layer: .top, tolerance: 0.3)
        XCTAssertTrue(TraceRoute.same(end, p(3.3, 0)), "\(end)")
        let start = doc.traceAnchor(at: p(3.3, 5), layer: .top, tolerance: 0.3)
        XCTAssertEqual(start, BoardGrid.snap(p(3.3, 5)), "off any trace: the grid")
        let b = doc.addTrace(from: start, to: end, layer: .top, diagonalFirst: false)
        XCTAssertTrue(has(try trace(doc, a), end))
        XCTAssertEqual(Set(doc.connectedRun(of: b!)), [a!, b!])
        // A vertex within tolerance wins over the segment.
        XCTAssertEqual(doc.traceAnchor(at: p(3.4, 0.05), layer: .top, tolerance: 0.3), end)
    }

    /// A Top part's surface-mount pad: the end lands on its centre and the
    /// pad is in the run's pads. The same on Bottom does not reach it.
    func testSMDPadFollowsItsSide() throws {
        let doc = doc()
        let part = try TestSupport.place(doc, "SN74HC00D", at: p(100, 100))
        let chip = try XCTUnwrap(doc.chips.first { $0.id == part.id })
        let pad1 = try XCTUnwrap(chip.boardPads.first { $0.designator == "1" })
        let centre = p(pad1.rect.midX, pad1.rect.midY)
        let inside = p(pad1.rect.minX + 0.05, pad1.rect.minY + 0.05)

        XCTAssertTrue(TraceRoute.same(doc.traceAnchor(at: inside, layer: .top, tolerance: 0.1), centre))
        let away = BoardGrid.snap(p(centre.x - 10, centre.y))
        let top = doc.addTrace(from: away, to: doc.traceAnchor(at: inside, layer: .top, tolerance: 0.1),
                               layer: .top, diagonalFirst: false)
        XCTAssertEqual(doc.pads(touching: doc.connectedRun(of: top!)), ["\(chip.reference).1"])

        let bottomEnd = doc.traceAnchor(at: inside, layer: .bottom, tolerance: 0.1)
        XCTAssertFalse(TraceRoute.same(bottomEnd, centre), "no pad on Bottom: the grid")
        let bottom = doc.addTrace(from: p(away.x, away.y + 20), to: p(away.x + 20, away.y + 20), layer: .bottom,
                                  diagonalFirst: false)
        doc.moveTraces([bottom!], by: CGSize(width: centre.x - away.x - 20, height: centre.y - away.y - 20))
        XCTAssertTrue(has(try trace(doc, bottom), centre), "the bottom trace ends on the pad centre")
        XCTAssertEqual(doc.pads(touching: doc.connectedRun(of: bottom!)), [])
    }

    func testThroughHolePadJoinsFromInnerWireLayer() throws {
        let doc = doc(layers: 4)
        doc.setLayerType(.inner(2), .wire)
        let part = try TestSupport.place(doc, "SN74HC00N", at: p(100, 100))
        let chip = try XCTUnwrap(doc.chips.first { $0.id == part.id })
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == "3" })
        XCTAssertGreaterThan(pad.drill, 0)
        let centre = p(pad.rect.midX, pad.rect.midY)
        let end = doc.traceAnchor(at: p(centre.x + 0.1, centre.y), layer: .inner(2), tolerance: 0.1)
        XCTAssertTrue(TraceRoute.same(end, centre))
        let t = doc.addTrace(from: BoardGrid.snap(p(centre.x - 10, centre.y - 5)), to: end, layer: .inner(2),
                             diagonalFirst: false)
        XCTAssertEqual(doc.pads(touching: doc.connectedRun(of: t!)), ["\(chip.reference).3"])
    }

    func testPlaneLayerRefusesTraces() {
        let doc = doc(layers: 4)
        XCTAssertEqual(doc.board.style(of: .inner(2)).type, .plane)
        XCTAssertNil(doc.addTrace(from: p(0, 0), to: p(5, 0), layer: .inner(2), diagonalFirst: false))
        XCTAssertNil(doc.addTrace(from: p(0, 0), to: p(0, 0), layer: .top, diagonalFirst: false), "zero length")
        XCTAssertNil(doc.addTrace(from: p(0, 0), to: p(5, 0), layer: .inner(4), diagonalFirst: false), "not on board")
        XCTAssertTrue(doc.traces.isEmpty)
    }

    func testSetTraceLayerRejoinsOnNewLayer() throws {
        let doc = doc()
        let onBottom = doc.addTrace(from: p(5, 0), to: p(5, 10), layer: .bottom, diagonalFirst: false)
        let moving = doc.addTrace(from: p(0, 5), to: p(10, 5), layer: .top, diagonalFirst: false)
        XCTAssertEqual(doc.connectedRun(of: moving!), [moving!])
        doc.setTraceLayer(moving!, .bottom)
        XCTAssertEqual(try trace(doc, moving).layer, .bottom)
        XCTAssertTrue(has(try trace(doc, onBottom), p(5, 5)))
        XCTAssertEqual(Set(doc.connectedRun(of: moving!)), [moving!, onBottom!])
    }

    func testLayerCountRefusedWithTraceOnLayer3() {
        let doc = doc(layers: 4)
        doc.setLayerType(.inner(3), .wire)
        XCTAssertNotNil(doc.addTrace(from: p(0, 0), to: p(5, 0), layer: .inner(3), diagonalFirst: false))
        XCTAssertEqual(doc.canSetLayerCount(2), "Delete or move the traces on Layer 3 first.")
        doc.setLayerCount(2)
        XCTAssertEqual(doc.board.layers, 4)
        XCTAssertNil(doc.canSetLayerCount(6))
    }

    func testPlaneTypeRefusedWhileLayerHoldsTrace() {
        let doc = doc(layers: 4)
        doc.setLayerType(.inner(3), .wire)
        let id = doc.addTrace(from: p(0, 0), to: p(5, 0), layer: .inner(3), diagonalFirst: false)
        XCTAssertEqual(doc.canSetLayerType(.inner(3), .plane), "Move or delete this layer's traces first.")
        doc.setLayerType(.inner(3), .plane)
        XCTAssertEqual(doc.board.style(of: .inner(3)).type, .wire)
        doc.deleteTrace(id!)
        doc.setLayerType(.inner(3), .plane)
        XCTAssertEqual(doc.board.style(of: .inner(3)).type, .plane)
    }

    func testTraceHitAndWidth() throws {
        let doc = doc()
        let id = try XCTUnwrap(doc.addTrace(from: p(0, 0), to: p(10, 0), layer: .top, diagonalFirst: false))
        XCTAssertEqual(doc.trace(at: p(5, 0.2), tolerance: 0.1)?.id, id)
        XCTAssertNil(doc.trace(at: p(5, 0.5), tolerance: 0.1))
        XCTAssertNil(doc.trace(at: p(5, 0), tolerance: 0.1,
                               visible: BoardVisibility(hidden: [.copper(.top)], copperLayers: doc.board.copperLayers)))
        doc.setTraceWidth(id, 3)
        XCTAssertEqual(try trace(doc, id).width, 5)
        doc.setTraceWidth(id, 250)
        XCTAssertEqual(try trace(doc, id).width, 100)
    }
}
