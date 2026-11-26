import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-68-W: a run's rarer turns, driven on `Router.Run` directly — chains
/// as the tuning's obstacles, tuning faulted back, the seeded tie key, a
/// pair whose pads give no terminal, the ledger's vias as obstacles,
/// existing copper a detour may not rip, planning that is cancelled or
/// finds no way, an unrouted net the polish can now join, and the repair
/// of finished copper.
final class RouterRunEdgeTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func board(_ width: Double = 20, _ height: Double = 20, layers: [CopperLayer] = [.top, .bottom]) -> RoutingTestBoard {
        var board = RoutingTestBoard(width: width, height: height, layers: layers)
        board.options.grid = 0.1
        return board
    }

    private func fast(rounds: Int = 6) -> Router.Strategy {
        var strategy = Router.Strategy.of(.fast)
        strategy.rounds = rounds
        return strategy
    }

    // MARK: Router

    func testChainsBecomeObstaclesForTheTuning() {
        var board = board()
        board.chain([p(5, 5), p(15, 5)], net: 7)
        let problem = board.problem()
        XCTAssertEqual(problem.movable.count, 1)
        let before = problem.obstacles.count
        let trace = RoutedTrace(layer: .top, width: 0.3, points: [p(5, 5), p(10, 6), p(15, 5)], net: 7)
        let via = RoutedVia(centre: p(15, 5), drill: 0.3, diameter: 0.6, net: 7)
        let copper = FinishedCopper(traces: [], vias: [], chainTraces: [trace], chainVias: [via])
        let result = Router.withChainsAsObstacles(problem, copper)
        XCTAssertEqual(result.movable, [])
        let added = Array(result.obstacles.dropFirst(before))
        XCTAssertEqual(added.map(\.kind), [.trace, .trace, .via])
        XCTAssertTrue(added.allSatisfy { $0.net == 7 })
        XCTAssertEqual(added[0].shape.radius, 0.15)
        XCTAssertEqual(added[0].shape.layers, [.top])
        XCTAssertEqual(added[2].shape.radius, 0.3)
        XCTAssertEqual(added[2].drill?.radius, 0.15)
        XCTAssertNil(added[2].shape.layers, "a via is on every layer")
        XCTAssertEqual(Router.withChainsAsObstacles(board.problem(), FinishedCopper(traces: [], vias: [])).obstacles.count,
                       before, "the chains themselves are not added")
        var plain = RoutingTestBoard()
        plain.pad(1, 1, 1, 1, net: 1)
        let free = plain.problem()
        XCTAssertEqual(Router.withChainsAsObstacles(free, copper), free, "no chains: the problem as it was")
    }

    /// A trace the verifier faults goes back as it was, and the group is
    /// measured again on what was kept.
    func testAFaultedTraceIsKeptAsRoutedAndTheGroupMeasuredAgain() throws {
        let group = UUID()
        var board = board()
        board.options.tuneGroups = false
        board.pad(2, 4, 0.5, 0.5, net: 1)
        board.pad(18, 4, 0.5, 0.5, net: 1)
        board.pad(2, 14, 0.5, 0.5, net: 2)
        board.pad(18, 14, 0.5, 0.5, net: 2)
        board.pad(10, 4, 1, 1, net: 3)
        board.groups = [RoutingGroup(id: group, name: "BUS", mode: .lengthMatch, nets: [1, 2], tolerance: 0.25, gap: 0,
                                     maxUncoupled: 0, maxMeander: 2)]
        let problem = board.problem()
        var result = RoutingResult()
        // Net 1's trace runs over net 3's pad: illegal.
        result.traces = [RoutedTrace(layer: .top, width: 0.254, points: [p(2, 4), p(18, 4)], net: 1),
                         RoutedTrace(layer: .top, width: 0.254, points: [p(2, 14), p(10, 16), p(18, 14)], net: 2)]
        result.nets = [NetOutcome(net: 1, name: "N1", status: .complete, unroutedConnections: 0),
                       NetOutcome(net: 2, name: "N2", status: .complete, unroutedConnections: 0)]
        let tuned = Router.tuned(result, problem: problem)
        XCTAssertEqual(tuned.traces, result.traces)
        let outcome = try XCTUnwrap(tuned.groups.first)
        let one = try XCTUnwrap(outcome.lengths[1]), two = try XCTUnwrap(outcome.lengths[2])
        XCTAssertEqual(one, 16, accuracy: 1e-9)
        XCTAssertEqual(two, 2 * hypot(8, 2), accuracy: 1e-9)
        XCTAssertEqual(outcome.spread, abs(two - one), accuracy: 1e-9)
    }

    func testTheTieKeyMixesInARunsOwnSeed() {
        var board = board()
        board.pad(2, 2, 0.5, 0.5, net: 1)
        let problem = board.problem()
        let own = Router.Run(problem: problem, strategy: fast(), seed: problem.options.seed)
        XCTAssertEqual(own.tieKey(5), 5, "the problem's seed: the net id")
        let other = Router.Run(problem: problem, strategy: fast(), seed: 99)
        let a = other.tieKey(5), b = other.tieKey(6)
        XCTAssertNotEqual(a, 5)
        XCTAssertNotEqual(a, b)
        XCTAssertEqual(Router.Run(problem: problem, strategy: fast(), seed: 99).tieKey(5), a, "deterministic")
    }

    /// A pair the pair router couples whose pads give its run no terminal
    /// is routed as two nets, as a pair that isn't two two-pin nets is.
    func testAPairWithoutTerminalsIsUncoupled() throws {
        let pair = UUID()
        var board = RoutingTestBoard(width: 30, height: 20)
        board.options.grid = 0.127
        board.pad(5, 9.5, 0.4, 0.4, net: 1)
        board.pad(25, 9.5, 0.4, 0.4, net: 1)
        board.pad(5, 10.5, 0.4, 0.4, net: 2)
        board.pad(25, 10.5, 0.4, 0.4, net: 2)
        board.groupOf = [1: pair, 2: pair]
        board.groups = [RoutingGroup(id: pair, name: "USB", mode: .diffPair, nets: [1, 2], tolerance: 0.25, gap: 0.2,
                                     maxUncoupled: 5, maxMeander: 2)]
        let problem = board.problem()
        guard case .success = RoutingPair.route(problem.groups[0], problem: problem) else { return XCTFail("the pair routes") }

        var coupled = Router.Run(problem: problem, strategy: fast())
        coupled.routePairs(isCancelled: { false })
        guard case .coupled = coupled.pairStatus[pair] else { return XCTFail("\(String(describing: coupled.pairStatus[pair]))") }

        var stripped = problem
        for i in stripped.nets.indices { stripped.nets[i].components = stripped.nets[i].components.map { _ in RoutingComponent() } }
        var run = Router.Run(problem: problem, strategy: fast())
        run.state = RoutingState(problem: stripped)
        run.routePairs(isCancelled: { false })
        XCTAssertEqual(run.pairStatus[pair], .uncoupled(reason: PairFailure.notTwoPin.reason))
        XCTAssertFalse(run.pairNets.contains(1))
        XCTAssertEqual(run.state.ledger.filter { [1, 2].contains($0.net) }, [])
    }

    /// Nets 1 and 2: a pair across a 1-layer board, walls to the edges at its
    /// ends; net 3 must cross it.
    private func crossedPair(_ pair: UUID) -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 30, height: 20, layers: [.top])
        board.options.grid = 0.127
        board.options.routePairs = true
        board.pad(5, 9.4, 0.4, 0.4, net: 1)
        board.pad(25, 9.4, 0.4, 0.4, net: 1)
        board.pad(5, 10.6, 0.4, 0.4, net: 2)
        board.pad(25, 10.6, 0.4, 0.4, net: 2)
        // Net 3's pads at the edges: no way round them either.
        board.pad(15, 0.7, 0.6, 0.6, net: 3)
        board.pad(15, 19.3, 0.6, 0.6, net: 3)
        // Walls from the edges in between each end's two pads: no way round the pair.
        board.trace([p(0, 10), p(4.9, 10)], width: 0.3, net: 9)
        board.trace([p(25.1, 10), p(30, 10)], width: 0.3, net: 9)
        board.groupOf = [1: pair, 2: pair]
        board.groups = [RoutingGroup(id: pair, name: "USB", mode: .diffPair, nets: [1, 2], tolerance: 0.25, gap: 0.2,
                                     maxUncoupled: 5, maxMeander: 2)]
        return board
    }

    /// A pair ripped up in the last round is routed once more after the
    /// rounds: coupled if it can be, else as two nets.
    func testAPairRippedInTheLastRoundIsRoutedAgain() throws {
        let pair = UUID()
        var run = Router.Run(problem: crossedPair(pair).problem(), strategy: fast(rounds: 1))
        run.routeAll(progress: { _ in }, isCancelled: { false })
        let three = try XCTUnwrap(run.state.netsByID[3])
        XCTAssertEqual(run.unrouted(three), 0, "net 3 crossed where the pair was")
        guard case .uncoupled = run.pairStatus[pair] else {
            let copper = run.state.ledger.map { "net \($0.net): \($0.traces.map { $0.points.map { "(\($0.x), \($0.y))" } })" }
            return XCTFail("the pair can't be coupled across net 3: \(String(describing: run.pairStatus[pair])) \(copper)")
        }
        XCTAssertFalse(run.pairNets.contains(1))
        XCTAssertTrue(run.rippedPairs.isEmpty)
    }

    /// A pair whose vias are smaller than the rules allow is refused whole.
    func testAPairThatBreaksARuleIsIllegal() throws {
        let pair = UUID()
        var board = RoutingTestBoard(width: 30, height: 20)
        board.options.grid = 0.127
        board.pad(5, 9.5, 0.4, 0.4, net: 1)
        board.pad(25, 9.5, 0.4, 0.4, net: 1)
        board.pad(5, 10.5, 0.4, 0.4, net: 2)
        board.pad(25, 10.5, 0.4, 0.4, net: 2)
        // A wall on Top: the pair changes layer twice.
        board.trace([p(15, 0.5), p(15, 19.5)], width: 0.3, layer: .top, net: 9)
        board.rules.minViaDiameter = 1.0
        board.groupOf = [1: pair, 2: pair]
        board.groups = [RoutingGroup(id: pair, name: "USB", mode: .diffPair, nets: [1, 2], tolerance: 0.25, gap: 0.2,
                                     maxUncoupled: 20, maxMeander: 2)]
        let problem = board.problem()
        guard case let .failure(failure) = RoutingPair.route(problem.groups[0], problem: problem) else {
            return XCTFail("vias of 0.8 mm under a 1 mm minimum")
        }
        XCTAssertEqual(failure, .illegal)
    }

    func testTheLedgersViasAreObstaclesWithTheirDrill() {
        var board = board()
        board.pad(2, 2, 0.5, 0.5, net: 1)
        board.pad(18, 2, 0.5, 0.5, net: 5)
        var run = Router.Run(problem: board.problem(), strategy: fast())
        let node = run.grid.nearestCell(p(10, 10))
        run.state.add(net: 5, path: [node], traces: [], vias: [RoutedVia(centre: p(10, 10), drill: 0.3, diameter: 0.6, net: 5)],
                      allowed: [true, true])
        let obstacles = run.ledgerObstacles(excluding: [])
        XCTAssertEqual(obstacles.map(\.kind), [.via])
        XCTAssertEqual(obstacles.first?.net, 5)
        XCTAssertEqual(obstacles.first?.shape.radius, 0.3)
        XCTAssertEqual(obstacles.first?.drill?.radius, 0.15)
        XCTAssertEqual(run.ledgerObstacles(excluding: [5]), [])
    }

    /// A relaxed detour through existing copper (a chain) is no way: the
    /// chain is never ripped, and the connection stays unrouted.
    func testADetourThroughExistingCopperRipsNothing() throws {
        var board = board(layers: [.top])
        board.pad(4, 10, 0.6, 0.6, net: 1)
        board.pad(16, 10, 0.6, 0.6, net: 1)
        board.chain([p(10, 0.3), p(10, 19.7)], width: 0.4, net: 7)
        let problem = board.problem()
        var run = Router.Run(problem: problem, strategy: fast())
        let net = try XCTUnwrap(run.state.netsByID[1])
        let chainEntries = run.state.ledger.filter { $0.chain != nil }
        XCTAssertEqual(chainEntries.count, 1)
        let lost = run.join(net, present: 50, round: 1, progress: { _ in }, isCancelled: { false })
        XCTAssertEqual(lost, [])
        XCTAssertEqual(run.state.ledger.filter { $0.chain != nil }, chainEntries)
        XCTAssertEqual(run.unrouted(net), 1)
        XCTAssertEqual(run.rips, 0)
    }

    // MARK: Plan

    func testPlanningSkipsAGroupNoPathReachesAndStopsWhenCancelled() throws {
        var board = board(layers: [.top])
        board.pad(4, 10, 0.6, 0.6, net: 1)
        board.pad(15, 10, 0.6, 0.6, net: 1)
        // A ring of another net's kept copper round the second pad.
        board.trace([p(13, 8), p(17, 8), p(17, 12), p(13, 12), p(13, 8)], width: 0.3, net: 9)
        var run = Router.Run(problem: board.problem(), strategy: fast())
        let net = try XCTUnwrap(run.state.netsByID[1])
        let costs = RoutingPlan(grid: run.grid).costs(lambda: 6)
        XCTAssertEqual(run.planNet(net, rules: run.rules(net), plan: costs, isCancelled: { false }), [])
        XCTAssertFalse(run.stopped)

        var wide = RoutingTestBoard(width: 60, height: 60, layers: [.top])
        wide.options.grid = 0.1
        wide.pad(3, 3, 0.6, 0.6, net: 1)
        wide.pad(57, 3, 0.6, 0.6, net: 1)
        // A wall between them, open only at the far end: the search floods the near side first.
        wide.trace([CGPoint(x: 30, y: 0.5), CGPoint(x: 30, y: 55)], width: 0.4, net: 9)
        var far = Router.Run(problem: wide.problem(), strategy: fast())
        let farNet = try XCTUnwrap(far.state.netsByID[1])
        let farCosts = RoutingPlan(grid: far.grid).costs(lambda: 6)
        XCTAssertEqual(far.planNet(farNet, rules: far.rules(farNet), plan: farCosts, isCancelled: { true }), [])
        XCTAssertTrue(far.stopped)
    }

    // MARK: Polish

    /// A net left with nothing routed is joined afresh by the polish when
    /// there is room, and the move is kept.
    func testThePolishJoinsANetLeftUnrouted() throws {
        var board = board()
        board.pad(4, 10, 0.6, 0.6, net: 1)
        board.pad(16, 10, 0.6, 0.6, net: 1)
        var strategy = fast()
        strategy.polishPasses = 1
        var run = Router.Run(problem: board.problem(), strategy: strategy)
        let net = try XCTUnwrap(run.state.netsByID[1])
        XCTAssertEqual(run.unrouted(net), 1)
        run.polish(progress: { _ in }, isCancelled: { false })
        XCTAssertEqual(run.unrouted(net), 0)
        XCTAssertFalse(run.state.ledger(of: 1).isEmpty)
    }

    // MARK: Repair

    /// Net 1 straight across net 2's pad: illegal copper in the ledger.
    private func illegalRun(layers: [CopperLayer] = [.top, .bottom], wall: Bool = false) throws -> (Router.Run, Int) {
        var board = board(layers: layers)
        board.pad(2, 10, 0.6, 0.6, net: 1)
        board.pad(18, 10, 0.6, 0.6, net: 1)
        board.pad(10, 10, 1, 1, net: 2)
        board.pad(10, 2, 0.6, 0.6, net: 2)
        if wall { board.trace([p(13, 0.5), p(13, 19.5)], width: 0.3, layer: .top, net: 9) }
        var run = Router.Run(problem: board.problem(), strategy: fast())
        let ends = try XCTUnwrap(run.state.terminals[1])
        let path = [try XCTUnwrap(ends[0].first?.node), try XCTUnwrap(ends[1].first?.node)]
        let id = run.state.add(net: 1, path: path,
                               traces: [RoutedTrace(layer: .top, width: 0.254, points: [p(2, 10), p(18, 10)], net: 1)],
                               vias: [], allowed: [true, true])
        return (run, id)
    }

    func testAStoppedRunRipsIllegalCopperAtOnce() throws {
        var (run, _) = try illegalRun()
        run.stopped = true
        let copper = run.finishedCopper()
        XCTAssertEqual(copper.traces, [])
        XCTAssertEqual(run.illegalNets, [1])
        XCTAssertEqual(run.state.ledger(of: 1), [])
        XCTAssertEqual(run.rips, 0, "not routed again")
    }

    /// Routed again with a margin, a connection that needs a via gets one
    /// of the net's real size.
    func testARerouteKeepsTheNetsRealViaSize() throws {
        var (run, id) = try illegalRun(wall: true)
        let real = try XCTUnwrap(run.state.netsByID[1])
        run.reroute([id], margin: 0.05)
        let entries = run.state.ledger(of: 1)
        XCTAssertFalse(entries.isEmpty)
        let vias = entries.flatMap(\.vias)
        XCTAssertFalse(vias.isEmpty, "the wall on Top is crossed on Bottom")
        XCTAssertTrue(vias.allSatisfy { $0.diameter == real.viaDiameter && $0.drill == real.viaDrill })
        XCTAssertTrue(entries.flatMap(\.traces).allSatisfy { $0.width == real.width })
        XCTAssertFalse(run.illegalNets.contains(1))
    }

    /// A pushed chain the cleanup would change the shape of goes back to
    /// the push's copper; illegal even so, it goes back to the problem's:
    /// its runs and its via as they were, untouched.
    func testAPushedChainTheCleanupBreaksIsRestored() throws {
        var board = board()
        board.pad(2, 2, 0.6, 0.6, net: 1)
        let run0 = MovableChain.Run(trace: UUID(), layer: .top, first: 0, last: 1, reversed: false, points: [p(5, 10), p(8, 10)])
        let run1 = MovableChain.Run(trace: UUID(), layer: .bottom, first: 0, last: 1, reversed: false, points: [p(8, 10), p(11, 10)])
        func end(_ point: CGPoint, _ layer: CopperLayer) -> MovableChain.End {
            var anchor = RoutingComponent()
            anchor.pads = [CopperShape(core: .segment(point, point), radius: 0.1, layers: [layer], node: -2)]
            return MovableChain.End(point: point, layer: layer, anchor: anchor)
        }
        board.chains = [MovableChain(id: 0, net: 7, width: 0.254, viaDrill: 0.3, viaDiameter: 0.6, runs: [run0, run1],
                                     vias: [MovableChain.Via(id: UUID(), centre: p(8, 10))],
                                     ends: [end(p(5, 10), .top), end(p(11, 10), .bottom)])]
        board.unrouted.insert(7)
        var run = Router.Run(problem: board.problem(), strategy: fast())
        let chain = try XCTUnwrap(run.state.ledger.first { $0.chain == 0 })
        // The push moved it: its via and its runs' inner ends 1 mm up.
        run.state.replace(chain.id,
                          traces: [RoutedTrace(layer: .top, width: 0.254, points: [p(5, 10), p(8, 11)], net: 7),
                                   RoutedTrace(layer: .bottom, width: 0.254, points: [p(8, 11), p(11, 10)], net: 7)],
                          vias: [RoutedVia(centre: p(8, 11), drill: 0.3, diameter: 0.6, net: 7)], touched: true)
        let copper = run.finishedCopper()
        XCTAssertEqual(copper.moved, [], "back where it was: nothing moved")
        XCTAssertEqual(copper.chainVias, [RoutedVia(centre: p(8, 10), drill: 0.3, diameter: 0.6, net: 7)])
        XCTAssertEqual(Set(copper.chainTraces.map(\.points)), [[p(5, 10), p(8, 10)], [p(8, 10), p(11, 10)]])
        let restored = try XCTUnwrap(run.state.entries[chain.id])
        XCTAssertFalse(restored.touched)
        XCTAssertEqual(run.illegalNets, [])
    }
}
