import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// `BoardCopper` on hand-built traces and vias, with no document
/// (plan-68-E): adding and joining, widths and layers, vias, routing's
/// primitives, segment and group drags, and the trace geometry.
@MainActor
final class BoardCopperTests: XCTestCase {
    private let settings = BoardSettings()

    private func trace(_ points: [CGPoint], layer: CopperLayer = .top) -> BoardTrace {
        BoardTrace(id: UUID(), layer: layer, width: 10, points: points)
    }

    func testAddJoinsWhatItCrosses() throws {
        var value = BoardCopper(traces: [], vias: [])
        XCTAssertNil(value.addTrace(points: [.zero], layer: .top, width: 10, settings: settings), "no length")
        XCTAssertNil(value.addTrace(points: [.zero, CGPoint(x: 10, y: 0)], layer: .inner(2), width: 10, settings: settings))
        let across = try XCTUnwrap(value.addTrace(points: [.zero, CGPoint(x: 10, y: 0)], layer: .top, width: 100_000,
                                                  settings: settings))
        XCTAssertEqual(value.traces[0].width, BoardTrace.widthRange.upperBound)
        let down = try XCTUnwrap(value.addTrace(points: [CGPoint(x: 5, y: -5), CGPoint(x: 5, y: 5)], layer: .top,
                                                width: 10, settings: settings))
        XCTAssertTrue(value.traces[0].points.contains(CGPoint(x: 5, y: 0)), "the crossing is a vertex of both")
        XCTAssertTrue(value.traces[1].points.contains(CGPoint(x: 5, y: 0)))
        XCTAssertEqual(Set(value.connectedRun(of: across)), [across, down])
        XCTAssertEqual(value.connectedRun(of: UUID()), [])
        value.join(UUID())
        // On another layer nothing joins.
        let bottom = try XCTUnwrap(value.addTrace(points: [CGPoint(x: 2, y: -5), CGPoint(x: 2, y: 5)], layer: .bottom,
                                                  width: 10, settings: settings))
        XCTAssertEqual(value.connectedRun(of: bottom), [bottom])
        value.setLayer(bottom, .top, settings: settings)
        XCTAssertEqual(value.traces[2].layer, .top)
        XCTAssertTrue(value.traces[0].points.contains(CGPoint(x: 2, y: 0)))
        value.setLayer(bottom, .top, settings: settings)
        value.setLayer(bottom, .inner(2), settings: settings)
        value.setLayer(UUID(), .bottom, settings: settings)
        XCTAssertEqual(value.traces[2].layer, .top)
    }

    func testPointsWidthsLocksAndMoves() {
        let a = trace([.zero, CGPoint(x: 10, y: 0)])
        var value = BoardCopper(traces: [a], vias: [])
        XCTAssertTrue(value.canSetPoints(a.id, [.zero, CGPoint(x: 0, y: 10)]))
        XCTAssertFalse(value.canSetPoints(a.id, [.zero]))
        XCTAssertFalse(value.canSetPoints(UUID(), [.zero, CGPoint(x: 0, y: 10)]))
        value.setPoints(a.id, [.zero, CGPoint(x: 0, y: 10)])
        value.setPoints(a.id, [.zero, CGPoint(x: 0, y: 10)])
        value.setPoints(UUID(), [])
        XCTAssertEqual(value.traces[0].points, [.zero, CGPoint(x: 0, y: 10)])
        value.setWidth(a.id, -1)
        value.setWidth(UUID(), 1)
        XCTAssertEqual(value.traces[0].width, BoardTrace.widthRange.lowerBound)
        value.moveTraces([a.id], by: CGSize(width: 1, height: 1))
        XCTAssertEqual(value.traces[0].points, [CGPoint(x: 1, y: 1), CGPoint(x: 1, y: 11)])
        value.setLocked(trace: a.id, true)
        value.setLocked(trace: UUID(), true)
        XCTAssertFalse(value.canSetPoints(a.id, [.zero, CGPoint(x: 0, y: 10)]))
        value.moveTraces([a.id], by: CGSize(width: 1, height: 1))
        XCTAssertEqual(value.traces[0].points[0], CGPoint(x: 1, y: 1), "locked")
    }

    func testHitsAndNearestPoints() {
        let a = trace([.zero, CGPoint(x: 10, y: 0)])
        let b = trace([CGPoint(x: 0, y: 20), CGPoint(x: 10, y: 20)], layer: .bottom)
        let value = BoardCopper(traces: [a, b], vias: [])
        XCTAssertEqual(value.trace(at: CGPoint(x: 5, y: 0.1), tolerance: 0.2, layers: [.top, .bottom])?.id, a.id)
        XCTAssertNil(value.trace(at: CGPoint(x: 5, y: 20), tolerance: 0.2, layers: [.top]), "its layer is not shown")
        XCTAssertNil(value.trace(at: CGPoint(x: 5, y: 10), tolerance: 0.2, layers: [.top, .bottom]))
        XCTAssertEqual(value.nearestTracePoint(to: CGPoint(x: 9.5, y: 0.5), layer: .top, tolerance: 1), CGPoint(x: 10, y: 0))
        XCTAssertEqual(value.nearestTracePoint(to: CGPoint(x: 5, y: 0.5), layer: .top, tolerance: 1), CGPoint(x: 5, y: 0))
        XCTAssertEqual(value.nearestTracePoint(to: CGPoint(x: 5, y: 19.5), layer: nil, tolerance: 1), CGPoint(x: 5, y: 20))
        XCTAssertNil(value.nearestTracePoint(to: CGPoint(x: 5, y: 19.5), layer: .top, tolerance: 1))
        XCTAssertEqual(value.traces(touching: CGPoint(x: 5, y: 0.3), radius: 0.2).map(\.id), [a.id])
        XCTAssertTrue(value.traces(touching: CGPoint(x: 5, y: 10), radius: 0.2).isEmpty)
    }

    func testVias() {
        var value = BoardCopper(traces: [], vias: [])
        let rules = NetRules.defaults(for: settings.rules)
        let id = value.addVia(at: CGPoint(x: 5, y: 5), rules: rules)
        XCTAssertEqual(value.via(at: CGPoint(x: 5.1, y: 5))?.id, id)
        XCTAssertNil(value.via(at: CGPoint(x: 50, y: 5)))
        value.setViaCentre(id, CGPoint(x: 6, y: 6))
        value.setViaCentre(id, CGPoint(x: 6, y: 6))
        XCTAssertEqual(value.vias[0].centre, CGPoint(x: 6, y: 6))
        value.setViaDrill(id, 100)
        value.setViaDrill(id, 100)
        value.setViaDrill(UUID(), 1)
        XCTAssertLessThanOrEqual(value.vias[0].drill, value.vias[0].diameter - 2 * BoardVia.minimumRing + 1e-9)
        value.setViaDiameter(id, 0)
        value.setViaDiameter(id, 0)
        value.setViaDiameter(UUID(), 1)
        XCTAssertGreaterThanOrEqual(value.vias[0].diameter, value.vias[0].drill + 2 * BoardVia.minimumRing - 1e-9)
        value.setViaTenting(id, .top, !value.vias[0].tentedTop)
        value.setViaTenting(id, .bottom, !value.vias[0].tentedBottom)
        value.setViaTenting(id, .bottom, value.vias[0].tentedBottom)
        value.setViaTenting(id, .top, value.vias[0].tentedTop)
        value.setViaTenting(UUID(), .top, true)
        value.setLocked(via: id, true)
        value.setLocked(via: UUID(), true)
        value.setViaCentre(id, .zero)
        XCTAssertEqual(value.vias[0].centre, CGPoint(x: 6, y: 6), "locked")
        value.apply(NetRules(width: 33, clearance: nil, viaDrill: 0.4, viaDiameter: 0.9), traces: [], vias: [id])
        XCTAssertEqual(value.vias[0].drill, 0.4)
        value.apply(NetRules(width: 33, clearance: nil, viaDrill: 0.4, viaDiameter: 0.9), traces: [], vias: [id])
    }

    func testRoutingPrimitivesAndPaste() {
        var value = BoardCopper(traces: [], vias: [])
        var one = settings
        one.layers = 1
        XCTAssertFalse(value.append(trace([.zero]), settings: settings))
        XCTAssertFalse(value.append(trace([.zero, CGPoint(x: 1, y: 0)], layer: .inner(3)), settings: settings))
        XCTAssertTrue(value.append(trace([.zero, CGPoint(x: 1, y: 0)]), settings: settings))
        let via = BoardVia(id: UUID(), centre: .zero, drill: 0.3, diameter: 0.6)
        XCTAssertFalse(value.append(via, layers: 1))
        XCTAssertTrue(value.append(via, layers: 2))
        value.apply(NetRules(width: 33, clearance: nil, viaDrill: 0.3, viaDiameter: 0.6), traces: [value.traces[0].id], vias: [])
        XCTAssertEqual(value.traces[0].width, 33)
        let pasted = value.paste(traces: value.traces + [trace([.zero, CGPoint(x: 1, y: 0)], layer: .inner(4))],
                                 vias: value.vias, offset: CGSize(width: 2, height: 0), settings: settings)
        XCTAssertEqual(pasted.traces.count, 1)
        XCTAssertEqual(pasted.vias.count, 1)
        XCTAssertEqual(value.paste(traces: [], vias: value.vias, offset: .zero, settings: one).vias, [])
        value.remove(traces: [], vias: [])
        value.remove(traces: Set(value.traces.map(\.id)), vias: Set(value.vias.map(\.id)))
        XCTAssertTrue(value.traces.isEmpty && value.vias.isEmpty)
    }

    func testSegmentDragsAndGroupMoves() throws {
        let bent = trace([.zero, CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 10)])
        let other = trace([CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10)])
        let via = BoardVia(id: UUID(), centre: .zero, drill: 0.3, diameter: 0.6)
        var value = BoardCopper(traces: [bent, other], vias: [via])
        XCTAssertNil(value.segmentBase(UUID(), at: .zero, pads: []))
        let base = try XCTUnwrap(value.segmentBase(bent.id, at: CGPoint(x: 5, y: 1), pads: [CGRect(x: 9, y: -1, width: 2, height: 2)]))
        XCTAssertEqual(base.segment, 0)
        XCTAssertEqual(base.anchored, [0, 1, 2], "under a via, on a pad, shared with a trace")
        let free = try XCTUnwrap(value.segmentBase(bent.id, at: CGPoint(x: 10, y: 6), pads: []))
        XCTAssertEqual(free.segment, 1)
        value.dragSegment(free, by: CGSize(width: 2.2, height: 0), grid: 1)
        XCTAssertNotEqual(value.traces[0].points, bent.points)
        value.dragSegment(free, by: CGSize(width: 2.2, height: 0), grid: 1)
        let diagonal = trace([.zero, CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10)])
        var slanted = BoardCopper(traces: [diagonal], vias: [])
        let slant = try XCTUnwrap(slanted.segmentBase(diagonal.id, at: CGPoint(x: 5, y: 5), pads: []))
        slanted.dragSegment(slant, by: CGSize(width: 3, height: -3), grid: 1)
        XCTAssertNotEqual(slanted.traces[0].points, diagonal.points)
        slanted.dragSegment(TraceSegmentBase(id: UUID(), segment: 0, points: [], anchored: []), by: .zero, grid: 1)
        slanted.dragSegment(TraceSegmentBase(id: diagonal.id, segment: 0, points: [.zero, .zero], anchored: []),
                            by: CGSize(width: 1, height: 1), grid: 1)
        value.setLocked(trace: bent.id, true)
        XCTAssertNil(value.segmentBase(bent.id, at: .zero, pads: []))

        value.moveGroup(traces: [other.id: other.points, UUID(): []], vias: [via.id: via.centre, UUID(): .zero],
                        by: CGSize(width: 1, height: 0))
        XCTAssertEqual(value.traces[1].points[0], CGPoint(x: 11, y: 10))
        XCTAssertEqual(value.vias[0].centre, CGPoint(x: 1, y: 0))
        value.moveGroup(traces: [other.id: other.points], vias: [via.id: via.centre], by: CGSize(width: 1, height: 0))
    }

    func testGeometry() {
        XCTAssertEqual(BoardCopper.nearest(on: .zero, .zero, to: CGPoint(x: 5, y: 5)), .zero)
        XCTAssertEqual(BoardCopper.nearest(on: .zero, CGPoint(x: 10, y: 0), to: CGPoint(x: 5, y: 5)), CGPoint(x: 5, y: 0))
        XCTAssertEqual(BoardCopper.contacts(.zero, CGPoint(x: 10, y: 0), CGPoint(x: 5, y: -5), CGPoint(x: 5, y: 5)),
                       [CGPoint(x: 5, y: 0)])
        XCTAssertEqual(BoardCopper.contacts(.zero, CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 5)),
                       [CGPoint(x: 10, y: 0)], "an end shared exactly")
        XCTAssertEqual(BoardCopper.contacts(.zero, CGPoint(x: 10, y: 0), CGPoint(x: 20, y: -5), CGPoint(x: 20, y: 5)), [])
        XCTAssertEqual(BoardCopper.contacts(.zero, CGPoint(x: 10, y: 0), CGPoint(x: 5, y: 0), CGPoint(x: 15, y: 0)),
                       [CGPoint(x: 10, y: 0), CGPoint(x: 5, y: 0)], "a collinear overlap's ends")
        XCTAssertEqual(BoardCopper.contacts(.zero, CGPoint(x: 10, y: 0), .zero, CGPoint(x: 10, y: 0)).count, 2)
        XCTAssertEqual(BoardCopper.contacts(.zero, CGPoint(x: 10, y: 0), CGPoint(x: 0, y: 1), CGPoint(x: 10, y: 1)), [])
        let line = [CGPoint.zero, CGPoint(x: 10, y: 0)]
        XCTAssertEqual(BoardCopper.inserting(CGPoint(x: 5, y: 0), into: line).count, 3)
        XCTAssertEqual(BoardCopper.inserting(.zero, into: line), line)
        XCTAssertEqual(BoardCopper.inserting(CGPoint(x: 5, y: 5), into: line), line)
        let rect = CGRect(x: 0, y: 0, width: 10, height: 10)
        XCTAssertTrue(BoardCopper.segment(CGPoint(x: -5, y: 5), CGPoint(x: 15, y: 5), touches: rect))
        XCTAssertFalse(BoardCopper.segment(CGPoint(x: -5, y: 20), CGPoint(x: 15, y: 20), touches: rect))
        XCTAssertFalse(BoardCopper.segment(CGPoint(x: -5, y: -5), CGPoint(x: -1, y: 20), touches: rect))
        XCTAssertFalse(BoardCopper.segment(CGPoint(x: 20, y: 0), CGPoint(x: 20, y: 5), touches: rect))
    }
}
