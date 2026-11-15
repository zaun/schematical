import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Putting a routing result on the board (plan-38-G): one undo step,
/// clearing, unroute, the scope from the selection.
@MainActor
final class RoutingApplyTests: XCTestCase {
    private func centre(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
        let designator = chip.part.pins[ref.index].designator
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// Two LEDs wired K to A, their net in a 20 mil Power class.
    private func powerPair() throws -> (doc: SchematicDocument, k: CGPoint, a: CGPoint, key: PlaneNet) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        let kRef = try TestSupport.pin(one, "K"), aRef = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(kRef), .pin(aRef)))
        let key = PlaneNet.pin(chip: one.id, designator: one.part.pins[kRef.index].designator)
        let power = doc.addNetClass(named: "Power")
        doc.setNetClassWidth(power, 20)
        doc.assignNet(key, to: power)
        return (doc, try centre(doc, kRef), try centre(doc, aRef), key)
    }

    private func problem(_ doc: SchematicDocument, _ change: (inout RoutingOptions) -> Void = { _ in }) -> RoutingProblem {
        var options = RoutingOptions.standard
        change(&options)
        return doc.routingProblem(options: options, rules: .customDefault)
    }

    /// A hand-made result: down from K, across, up to A, with a via on the way.
    private func handResult(_ problem: RoutingProblem, k: CGPoint, a: CGPoint) throws -> RoutingResult {
        let net = try XCTUnwrap(problem.nets.first)
        var result = RoutingResult()
        let y = k.y + 6
        result.traces = [RoutedTrace(layer: .top, width: net.width, points: [k, CGPoint(x: k.x, y: y), CGPoint(x: a.x, y: y), a],
                                     net: net.id)]
        result.vias = [RoutedVia(centre: CGPoint(x: (k.x + a.x) / 2, y: y), drill: 0.4, diameter: 0.8, net: net.id)]
        result.nets = [NetOutcome(net: net.id, name: net.name, status: .complete, unroutedConnections: 0)]
        return result
    }

    // MARK: Phase 1

    func testResultLandsAutoAtTheClassWidthAsOneUndoStep() throws {
        let (doc, k, a, _) = try powerPair()
        let problem = problem(doc)
        XCTAssertEqual(problem.nets.first?.width ?? 0, 0.508, accuracy: 1e-9)
        let before = doc.state
        let summary = doc.applyRouting(try handResult(problem, k: k, a: a), problem: problem)
        XCTAssertEqual(summary.addedTraces, 1)
        XCTAssertEqual(summary.addedVias, 1)
        XCTAssertEqual(doc.undoName(on: .pcb), "Autoroute")
        let trace = try XCTUnwrap(doc.traces.first)
        XCTAssertTrue(trace.auto)
        XCTAssertEqual(trace.width, 20)
        XCTAssertTrue(try XCTUnwrap(doc.vias.first).auto)
        XCTAssertEqual(doc.boardSelection.items, [.trace(trace.id), .via(doc.vias[0].id)])
        XCTAssertTrue(doc.ratsnest().isEmpty)
        let after = doc.state
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.state, before)
        XCTAssertTrue(doc.redo(on: .pcb))
        XCTAssertEqual(doc.state, after)
    }

    func testClearedCopperGoesAndLockedCopperStays() throws {
        let (doc, k, a, _) = try powerPair()
        let gone = try XCTUnwrap(doc.addTrace(points: [k, CGPoint(x: k.x, y: k.y - 4)], layer: .top))
        let kept = try XCTUnwrap(doc.addTrace(points: [a, CGPoint(x: a.x, y: a.y - 4)], layer: .top))
        doc.setLocked(.trace(kept), true)
        var problem = problem(doc) { $0.existing = .clearUnlocked }
        XCTAssertEqual(problem.removedTraces, [gone])
        problem.removedTraces.insert(kept)  // even if a stale problem names it
        let summary = doc.applyRouting(RoutingResult(), problem: problem)
        XCTAssertEqual(summary.removedTraces, 1)
        XCTAssertFalse(doc.traces.contains { $0.id == gone })
        XCTAssertTrue(doc.traces.contains { $0.id == kept }, "locked copper is never deleted")
    }

    func testLockedCopperIsNeverInRemovedTraces() throws {
        let (doc, k, a, _) = try powerPair()
        let gone = try XCTUnwrap(doc.addTrace(points: [k, CGPoint(x: k.x, y: k.y - 4)], layer: .top))
        let kept = try XCTUnwrap(doc.addTrace(points: [a, CGPoint(x: a.x, y: a.y - 4)], layer: .top))
        doc.setLocked(.trace(kept), true)
        let problem = problem(doc) { $0.existing = .clearUnlocked }
        doc.applyRouting(RoutingResult(), problem: problem)
        XCTAssertFalse(doc.traces.contains { $0.id == gone })
        XCTAssertTrue(doc.traces.contains { $0.id == kept && $0.locked })
    }

    func testATraceOnAPlaneIsSkippedAndReported() throws {
        let (doc, k, a, _) = try powerPair()
        doc.setLayerCount(4)
        let problem = problem(doc)
        var result = try handResult(problem, k: k, a: a)
        result.traces.append(RoutedTrace(layer: .inner(2), width: 0.254, points: [k, a], net: problem.nets[0].id))
        let summary = doc.applyRouting(result, problem: problem)
        XCTAssertEqual(summary.skipped, 1)
        XCTAssertEqual(summary.addedTraces, 1)
        XCTAssertFalse(doc.traces.contains { $0.layer == .inner(2) })
    }

    func testAnEmptyResultRecordsNothing() throws {
        let (doc, _, _, _) = try powerPair()
        let name = doc.undoName(on: .pcb)
        let summary = doc.applyRouting(RoutingResult(), problem: problem(doc))
        XCTAssertEqual(summary.addedTraces, 0)
        XCTAssertEqual(doc.undoName(on: .pcb), name)
    }

    /// AFU, routed (Balanced) and applied once.
    private func routedAFU() throws -> (doc: SchematicDocument, problem: RoutingProblem, result: RoutingResult) {
        let doc = try Fixture.open("AFU")
        let problem = doc.routingProblem(options: .fixture, rules: .customDefault)
        let result = Router.route(problem)
        doc.applyRouting(result, problem: problem)
        return (doc, problem, result)
    }

    func testAFUAppliedHasNoUnroutedAndNoShorts() throws {
        let (doc, _, result) = try routedAFU()
        XCTAssertEqual(result.stats.routed, 115)
        XCTAssertEqual(doc.ratsnest().count, 0)
        let issues = doc.designRuleCheck(rules: .customDefault)
        XCTAssertEqual(issues.filter { $0.kind == .unrouted }.count, 0)
        XCTAssertEqual(issues.filter { $0.kind == .short }.map(\.issue), [])

        // Phase 2: unroute one net's autorouted copper.
        let copper = doc.connectivity()
        let target = try XCTUnwrap(result.nets.first { outcome in
            (doc.netRows(in: copper).first { $0.net == outcome.net }?.pads ?? 0) == 2
        })
        let removed = doc.unroute(nets: [target.net], onlyAuto: true)
        XCTAssertGreaterThan(removed, 0)
        XCTAssertEqual(doc.undoName(on: .pcb), "Unroute")
        XCTAssertEqual(doc.ratsnest().count, 1, "a two-pad net's one connection")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.ratsnest().count, 0)

        let total = doc.traces.count
        XCTAssertGreaterThan(total, 0)
        doc.unrouteAll(onlyAuto: true)
        XCTAssertEqual(doc.traces.count, 0, "AFU had no traces before routing")
        XCTAssertEqual(doc.vias.count, 0)
        XCTAssertEqual(doc.ratsnest().count, 115)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.traces.count, total)
    }

    // MARK: Phase 2

    func testOnlyAutoKeepsHandCopperAndLockedAutoStays() throws {
        let (doc, k, a, key) = try powerPair()
        let problem = problem(doc)
        doc.applyRouting(try handResult(problem, k: k, a: a), problem: problem)
        let hand = try XCTUnwrap(doc.addTrace(points: [k, CGPoint(x: k.x - 3, y: k.y)], layer: .bottom))
        let auto = try XCTUnwrap(doc.traces.first { $0.auto })
        let net = try XCTUnwrap(doc.connectivity().net(of: key))
        // A second auto trace, locked.
        doc.setLocked(.trace(auto.id), true)
        XCTAssertEqual(doc.unroute(nets: [net], onlyAuto: true), 1, "only the via: the auto trace is locked")
        XCTAssertTrue(doc.traces.contains { $0.id == hand })
        XCTAssertTrue(doc.traces.contains { $0.id == auto.id })
        XCTAssertEqual(doc.unroute(nets: [net], onlyAuto: false), 1, "the hand trace")
        XCTAssertFalse(doc.traces.contains { $0.id == hand })
        XCTAssertTrue(doc.traces.contains { $0.id == auto.id }, "locked copper survives every mode")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.traces.contains { $0.id == hand })
    }

    func testNothingIsUnroutedWhileTheSimulationRuns() throws {
        let (doc, k, a, key) = try powerPair()
        let problem = problem(doc)
        doc.applyRouting(try handResult(problem, k: k, a: a), problem: problem)
        let net = try XCTUnwrap(doc.connectivity().net(of: key))
        doc.isPlaying = true
        XCTAssertEqual(doc.unroute(nets: [net], onlyAuto: false), 0)
        XCTAssertEqual(doc.unrouteAll(onlyAuto: false), 0)
        doc.isPlaying = false
        XCTAssertEqual(doc.traces.count, 1)
    }

    func testScopeFromSelection() throws {
        let (doc, k, a, key) = try powerPair()
        XCTAssertEqual(RoutingScope.fromSelection(doc), .all)
        let chip = try XCTUnwrap(doc.chips.first)
        doc.select(.footprint(chip.id))
        XCTAssertEqual(RoutingScope.fromSelection(doc), .footprints([chip.id]))
        let trace = try XCTUnwrap(doc.addTrace(points: [k, CGPoint(x: k.x, y: k.y + 3)], layer: .top))
        doc.select(.trace(trace))
        XCTAssertEqual(RoutingScope.fromSelection(doc), .nets([key]))
        _ = a
    }
}
