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
        XCTAssertEqual(doc.undoName, "Autoroute")
        let trace = try XCTUnwrap(doc.traces.first)
        XCTAssertTrue(trace.auto)
        XCTAssertEqual(trace.width, 20)
        XCTAssertTrue(try XCTUnwrap(doc.vias.first).auto)
        XCTAssertEqual(doc.boardSelection.items, [.trace(trace.id), .via(doc.vias[0].id)])
        XCTAssertTrue(doc.ratsnest().isEmpty)
        let after = doc.state
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.state, before)
        XCTAssertTrue(doc.redo())
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
        let name = doc.undoName
        let summary = doc.applyRouting(RoutingResult(), problem: problem(doc))
        XCTAssertEqual(summary.addedTraces, 0)
        XCTAssertEqual(doc.undoName, name)
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
        XCTAssertEqual(doc.undoName, "Unroute")
        XCTAssertEqual(doc.ratsnest().count, 1, "a two-pad net's one connection")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.ratsnest().count, 0)

        let total = doc.traces.count
        XCTAssertGreaterThan(total, 0)
        doc.unrouteAll(onlyAuto: true)
        XCTAssertEqual(doc.traces.count, 0, "AFU had no traces before routing")
        XCTAssertEqual(doc.vias.count, 0)
        XCTAssertEqual(doc.ratsnest().count, 115)
        XCTAssertTrue(doc.undo())
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
        XCTAssertTrue(doc.undo())
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

    // MARK: Phase 3 (plan-66-C): moving existing chains

    /// The pads' groups by label, whatever the roots are called: what the copper joins.
    private func padGroups(_ doc: SchematicDocument) -> Set<Set<String>> {
        let copper = doc.connectivity()
        var groups: [Int: Set<String>] = [:]
        for (p, pad) in copper.pads.enumerated() where copper.netOfPad[p] != nil {
            groups[copper.root[p], default: []].insert(pad.label)
        }
        return Set(groups.values)
    }

    /// The pair with a zig-zag trace K, B1, B2, A (10 mil) and a branch ending free at B2; the problem has
    /// the option on and three chains: [K, B1, B2], [B2, A] of the trunk and the branch.
    private func trunk(_ change: (inout RoutingOptions) -> Void = { _ in }) throws
        -> (doc: SchematicDocument, trunk: UUID, branch: UUID, points: [CGPoint], problem: RoutingProblem) {
        let (doc, k, a, _) = try powerPair()
        let y = k.y + 6
        let points = [k, CGPoint(x: k.x, y: y), CGPoint(x: a.x, y: y), a]
        let trunk = try XCTUnwrap(doc.addTrace(points: points, layer: .top, width: 20))
        let branch = try XCTUnwrap(doc.addTrace(points: [points[2], CGPoint(x: points[2].x, y: y + 4)], layer: .top, width: 20))
        let problem = problem(doc) { $0.pushAndShove = true; change(&$0) }
        XCTAssertEqual(problem.movable.count, 3)
        return (doc, trunk, branch, points, problem)
    }

    private func moving(_ chain: MovableChain, runs: [[CGPoint]]? = nil, vias: [CGPoint]? = nil) -> RoutingResult {
        var result = RoutingResult()
        result.moved = [MovedChain(chain: chain.id, runs: runs ?? chain.runs.map(\.points), vias: vias ?? chain.vias.map(\.centre))]
        return result
    }

    func testAMovedChainReplacesItsPointsAndKeepsTheRest() throws {
        let t = try trunk()
        let chain = try XCTUnwrap(t.problem.movable.first { $0.runs[0].trace == t.trunk && $0.runs[0].first == 0 })
        XCTAssertEqual(chain.runs[0].points, Array(t.points[0...2]))
        var shifted = chain.runs[0].points
        shifted[1].x += 0.2
        shifted[1].y += 0.2
        let before = t.doc.traces
        let selected = t.doc.boardSelection.items
        let summary = t.doc.applyRouting(moving(chain, runs: [shifted]), problem: t.problem)
        XCTAssertEqual(summary.movedTraces, 1)
        XCTAssertEqual(summary.movedVias, 0)
        XCTAssertEqual(summary.skipped, 0)
        let trunk = try XCTUnwrap(t.doc.traces.first { $0.id == t.trunk })
        let old = try XCTUnwrap(before.first { $0.id == t.trunk })
        XCTAssertEqual(trunk.points, [shifted[0], shifted[1], shifted[2], t.points[3]])
        XCTAssertEqual(trunk.points[2], t.points[2], "its ends stay")
        XCTAssertEqual(trunk.points[3], old.points[3], "what lay outside the chain is as it was")
        XCTAssertEqual([trunk.layer, trunk.width, trunk.locked, trunk.auto] as [AnyHashable],
                       [old.layer, old.width, old.locked, old.auto] as [AnyHashable])
        XCTAssertEqual(t.doc.traces.first { $0.id == t.branch }, before.first { $0.id == t.branch }, "other traces are untouched")
        XCTAssertEqual(t.doc.traces.count, before.count)
        XCTAssertEqual(t.doc.boardSelection.items, selected, "moved copper is not selected")
    }

    func testAMovedChainWithAViaMovesTheVia() throws {
        let (doc, k, a, _) = try powerPair()
        let p = CGPoint(x: k.x, y: k.y + 6)
        XCTAssertNotNil(doc.addTrace(points: [k, p], layer: .top, width: 20))
        XCTAssertNotNil(doc.addTrace(points: [p, CGPoint(x: a.x, y: p.y), a], layer: .bottom, width: 20))
        let viaID = try XCTUnwrap(doc.addVia(at: p))
        let problem = problem(doc) { $0.pushAndShove = true }
        let chain = try XCTUnwrap(problem.movable.first)
        XCTAssertEqual(chain.vias.map(\.id), [viaID])
        let before = try XCTUnwrap(doc.vias.first { $0.id == viaID })
        let moved = CGPoint(x: p.x + 0.4, y: p.y)
        let summary = doc.applyRouting(moving(chain, runs: [[k, moved], [moved, CGPoint(x: a.x, y: p.y), a]], vias: [moved]), problem: problem)
        XCTAssertEqual(summary.movedVias, 1)
        XCTAssertEqual(summary.movedTraces, 2)
        let via = try XCTUnwrap(doc.vias.first { $0.id == viaID })
        XCTAssertEqual(via.centre, moved)
        XCTAssertEqual([via.drill, via.diameter] as [Double], [before.drill, before.diameter] as [Double])
        XCTAssertEqual([via.tentedTop, via.tentedBottom, via.locked, via.auto], [before.tentedTop, before.tentedBottom, before.locked, before.auto])
        XCTAssertEqual(doc.traces.count, 2)
        XCTAssertTrue(doc.traces.allSatisfy { $0.points.contains(moved) }, "each trace ends at the via")
    }

    func testTheBoardsConnectivityIsUnchangedByAMove() throws {
        let t = try trunk()
        let groups = padGroups(t.doc)
        XCTAssertGreaterThan(groups.count, 0)
        for chain in t.problem.movable {
            var runs = chain.runs.map(\.points)
            // Every inner vertex 0.2 mm sideways.
            for r in runs.indices where runs[r].count > 2 {
                for i in 1..<(runs[r].count - 1) { runs[r][i].y += 0.2 }
            }
            XCTAssertEqual(t.doc.applyRouting(moving(chain, runs: runs), problem: t.problem).skipped, 0)
        }
        XCTAssertEqual(padGroups(t.doc), groups)
        XCTAssertTrue(t.doc.ratsnest().isEmpty)
    }

    func testAMoveIsOneUndoStepAndUndoRestoresThePoints() throws {
        let t = try trunk()
        let chain = try XCTUnwrap(t.problem.movable.first { $0.runs[0].trace == t.trunk && $0.runs[0].first == 0 })
        let before = t.doc.traces
        let state = t.doc.state
        var shifted = chain.runs[0].points
        shifted[1].y += 0.3
        t.doc.applyRouting(moving(chain, runs: [shifted]), problem: t.problem)
        XCTAssertNotEqual(t.doc.traces, before)
        XCTAssertEqual(t.doc.undoName, "Autoroute")
        XCTAssertTrue(t.doc.undo())
        XCTAssertEqual(t.doc.traces, before, "every trace's points bit for bit")
        XCTAssertEqual(t.doc.state, state)
    }

    func testAStaleChainIsSkipped() throws {
        let t = try trunk()
        let chain = try XCTUnwrap(t.problem.movable.first { $0.runs[0].trace == t.trunk && $0.runs[0].first == 0 })
        // The user edits the trace between the problem and the apply.
        var edited = t.points
        edited[1].x += 0.5
        XCTAssertTrue(t.doc.setTracePoints(t.trunk, edited))
        let before = t.doc.traces
        var shifted = chain.runs[0].points
        shifted[1].y += 0.3
        let summary = t.doc.applyRouting(moving(chain, runs: [shifted]), problem: t.problem)
        XCTAssertEqual(summary.skipped, 1)
        XCTAssertEqual(summary.movedTraces, 0)
        XCTAssertEqual(t.doc.traces, before, "the document is unchanged")
        // A locked trace is skipped the same way.
        t.doc.setLocked(.trace(t.branch), true)
        let branch = try XCTUnwrap(t.problem.movable.first { $0.runs[0].trace == t.branch })
        var bent = branch.runs[0].points
        bent.insert(CGPoint(x: bent[0].x + 0.5, y: (bent[0].y + bent[1].y) / 2), at: 1)
        XCTAssertEqual(t.doc.applyRouting(moving(branch, runs: [bent]), problem: t.problem).skipped, 1)
    }

    func testNoSegmentIsZeroLengthAfterASplice() throws {
        let t = try trunk()
        let chain = try XCTUnwrap(t.problem.movable.first { $0.runs[0].trace == t.trunk && $0.runs[0].first == 0 })
        // A run that repeats its last point: the splice drops the repeat.
        let run = chain.runs[0].points
        let summary = t.doc.applyRouting(moving(chain, runs: [[run[0], run[1], run[2], run[2]]]), problem: t.problem)
        XCTAssertEqual(summary.skipped, 0)
        for trace in t.doc.traces { XCTAssertTrue(TraceRoute.isLegal(trace.points), "\(trace.points)") }
        XCTAssertEqual(try XCTUnwrap(t.doc.traces.first { $0.id == t.trunk }).points, t.points)
    }

    func testAChainEndMovedWithinItsPadStaysConnected() throws {
        let (doc, k, a, _) = try powerPair()
        // Hand-drawn from a point inside K's pad, off its centre.
        let inside = CGPoint(x: k.x + 0.2, y: k.y)
        let y = k.y + 6
        XCTAssertNotNil(doc.addTrace(points: [inside, CGPoint(x: inside.x, y: y), CGPoint(x: a.x, y: y), a], layer: .top, width: 20))
        let problem = problem(doc) { $0.pushAndShove = true }
        let chain = try XCTUnwrap(problem.movable.first)
        XCTAssertEqual(chain.runs[0].points.first, inside)
        let groups = padGroups(doc)
        // Re-laid, it starts at the pad's centre.
        var runs = chain.runs[0].points
        runs[0] = k
        runs[1] = CGPoint(x: k.x, y: y)
        let summary = doc.applyRouting(moving(chain, runs: [runs]), problem: problem)
        XCTAssertEqual(summary.movedTraces, 1)
        XCTAssertEqual(doc.traces.first?.points.first, k)
        XCTAssertEqual(padGroups(doc), groups)
        XCTAssertTrue(doc.ratsnest().isEmpty)
    }

    /// The routed RegFile: every chain of the board's existing copper nudged 0.01 mm at its inner
    /// vertices, as one result; the pads' groups are as they were and one undo restores it all.
    func testAMoveOnARoutedBoardKeepsConnectivity() throws {
        let doc = try Fixture.open("RegFile")
        let first = doc.routingProblem(options: .fixture, rules: .customDefault)
        doc.applyRouting(Router.route(first), problem: first)
        var options = RoutingOptions.fixture
        options.pushAndShove = true
        options.scope = .nets([try XCTUnwrap(first.nets.first?.key)])
        let problem = doc.routingProblem(options: options, rules: .customDefault)
        XCTAssertFalse(problem.movable.isEmpty)
        let groups = padGroups(doc), traces = doc.traces
        var result = RoutingResult()
        for chain in problem.movable {
            var runs = chain.runs.map(\.points)
            for r in runs.indices where runs[r].count > 2 {
                for i in 1..<(runs[r].count - 1) { runs[r][i].y += 0.01 }
            }
            result.moved.append(MovedChain(chain: chain.id, runs: runs, vias: chain.vias.map(\.centre)))
        }
        let summary = doc.applyRouting(result, problem: problem)
        XCTAssertEqual(summary.skipped, 0)
        XCTAssertGreaterThan(summary.movedTraces, 0)
        XCTAssertNotEqual(doc.traces, traces, "something moved")
        XCTAssertEqual(padGroups(doc), groups)
        for trace in doc.traces { XCTAssertTrue(TraceRoute.isLegal(trace.points)) }
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.traces, traces)
    }
}
