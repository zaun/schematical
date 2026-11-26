import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The push primitive (plan-66-D): a connection with no legal way takes the copper in its
/// way up, lays itself, and lays that copper again within reach, all or nothing.
final class RoutingPushTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func run(_ board: RoutingTestBoard, _ change: (inout Router.Strategy) -> Void = { _ in }) -> Router.Run {
        var strategy = Router.Strategy.of(.fast)
        change(&strategy)
        return Router.Run(problem: board.problem(), strategy: strategy)
    }

    /// `tryPush` for the net's first two components.
    private func push(_ run: inout Router.Run, net: Int, isCancelled: () -> Bool = { false }) throws -> Bool {
        let routing = try XCTUnwrap(run.state.netsByID[net])
        let sources = run.state.nodes(components: [0], entries: [], net: net)
        let targets = run.state.nodes(components: [1], entries: [], net: net)
        return run.tryPush(routing, sources: sources, targets: targets, mainComponents: [0], targetComponents: [1],
                           isCancelled: isCancelled)
    }

    private func same(_ a: RoutingState, _ b: RoutingState) -> Bool {
        a.ledger == b.ledger && a.grid.traceMaps == b.grid.traceMaps && a.grid.viaMaps == b.grid.viaMaps
            && a.grid.traceMask == b.grid.traceMask && a.grid.viaMask == b.grid.viaMask && a.history == b.history
    }

    /// Net 5's two pads joined by a straight trace at y = 10, as an entry the run laid.
    @discardableResult
    private func lay(_ run: inout Router.Run, at y: CGFloat = 10) throws -> Int {
        let terminals = try XCTUnwrap(run.state.terminals[5])
        let a = try XCTUnwrap(terminals[0].first), b = try XCTUnwrap(terminals[1].first)
        let trace = RoutedTrace(layer: .top, width: 0.254, points: [p(3, y), p(17, y)], net: 5)
        let id = run.state.add(net: 5, path: [a.node, b.node], traces: [trace], vias: [], allowed: [true])
        XCTAssertEqual(run.state.groups(of: 5).components.count, 1)
        return id
    }

    private func finish(_ run: inout Router.Run) -> (FinishedCopper, RoutingResult) {
        let copper = run.finishedCopper()
        var result = run.result(traces: copper.traces, vias: copper.vias)
        result.moved = copper.moved
        return (copper, result)
    }

    // MARK: Pushing aside

    func testAChainInAGapIsPushedAside() throws {
        let board = RoutingTestBoard.shoveGap()
        var run = run(board)
        XCTAssertFalse(run.find({ var r = run.rules(run.state.netsByID[2]!); r.present = 0; return r }(),
                                sources: run.state.nodes(components: [0], entries: [], net: 2),
                                targets: run.state.nodes(components: [1], entries: [], net: 2), isCancelled: { false }) != nil,
                       "the gap is full: no legal way")
        XCTAssertTrue(try push(&run, net: 2))
        XCTAssertEqual(run.state.groups(of: 2).components.count, 1, "net 2 is joined")
        let chain = try XCTUnwrap(run.state.ledger.first { $0.chain != nil })
        XCTAssertTrue(chain.touched)
        XCTAssertEqual(chain.traces.count, 1)
        let points = chain.traces[0].points
        XCTAssertEqual(points.first, p(3, 10))
        XCTAssertEqual(points.last, p(17, 10))
        XCTAssertEqual(chain.traces[0].layer, .top)
        XCTAssertTrue(points.allSatisfy { abs($0.y - 10) <= run.strategy.pushReach + 1e-9 }, "within reach of where it was")
        XCTAssertNotEqual(points, [p(3, 10), p(17, 10)], "it moved")
        XCTAssertEqual(run.pushTried, 1)
        XCTAssertEqual(run.pushKept, 1)
        let (copper, result) = finish(&run)
        XCTAssertEqual(copper.moved.count, 1)
        XCTAssertEqual(RoutingCheck.violations(result, run.problem), [])
    }

    func testALedgerConnectionInAGapIsPushedAside() throws {
        let board = RoutingTestBoard.ledgerShoveGap()
        var run = run(board)
        try lay(&run)
        let old = try XCTUnwrap(run.state.ledger(of: 5).first)
        XCTAssertTrue(try push(&run, net: 2))
        XCTAssertEqual(run.state.groups(of: 2).components.count, 1)
        XCTAssertEqual(run.state.groups(of: 5).components.count, 1, "net 5 is still joined")
        let new = try XCTUnwrap(run.state.ledger(of: 5).first)
        XCTAssertNotEqual(new.traces, old.traces, "net 5 was laid again")
        XCTAssertEqual([new.path.first, new.path.last], [old.path.first, old.path.last], "between the same terminals")
        let (_, result) = finish(&run)
        XCTAssertEqual(result.stats.routed, result.stats.connections)
        XCTAssertEqual(RoutingCheck.violations(result, run.problem), [])
    }

    // MARK: Refusals leave the state as it was

    func testALockedTraceIsNotPushed() throws {
        let board = RoutingTestBoard.chainAsObstacle(RoutingTestBoard.shoveGap())
        var run = run(board)
        let before = run.state
        XCTAssertFalse(try push(&run, net: 2))
        XCTAssertTrue(same(run.state, before))
        XCTAssertEqual(run.pushTried, 1)
        XCTAssertEqual(run.pushKept, 0)
    }

    func testAPushBeyondReachFails() throws {
        var run = run(RoutingTestBoard.shoveGap()) { $0.pushReach = 0.05 }
        let before = run.state
        XCTAssertFalse(try push(&run, net: 2), "the chain may move only half a cell")
        XCTAssertTrue(same(run.state, before))
    }

    func testAVictimThatWouldGainAViaIsRefused() throws {
        // Two layers; net 2 may not use a via; the gap holds one trace's room and the chain would need
        // the bottom layer (two vias) to give way.
        var board = RoutingTestBoard.shoveGap(gap: 9.6...10.4)
        board.layers = [.top, .bottom]
        board.options.vias = .allowed
        board.viaDrill[2] = 0.1
        board.obstacles = board.obstacles.map { $0 }
        var run = run(board)
        let before = run.state
        XCTAssertFalse(try push(&run, net: 2))
        XCTAssertTrue(same(run.state, before))
        XCTAssertEqual(run.state.ledger.filter { $0.chain != nil }.map(\.vias.count), [0], "the chain gained no via")
    }

    /// Net 2 (pads above both) through a wide gap (y 9.2 to 10.8: trace centres between 9.554 and 10.446) in
    /// which the chains of nets 5 and 6 sit at y = 9.7 and 10.1. Three traces fit (at 9.6, 10.0, 10.4); net 2
    /// can only get there if net 5 gives way to it, and net 5 only if net 6 gives way to net 5.
    private func cascade() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20, layers: [.top])
        board.options.grid = 0.1
        board.options.vias = .none
        board.pad(3, 6, 1, 1, net: 2)
        board.pad(17, 6, 1, 1, net: 2)
        // Pads of 0.2 mm: the chains are 0.4 mm apart.
        for (i, y) in [9.7, 10.1].enumerated() {
            board.pad(3, y, 0.2, 0.2, net: 5 + i)
            board.pad(17, y, 0.2, 0.2, net: 5 + i)
            board.chain([p(3, y), p(17, y)], net: 5 + i, from: 2 + 2 * i, to: 3 + 2 * i)
        }
        board.trace([p(10, 0), p(10, 9.2)], net: 9)
        board.trace([p(10, 10.8), p(10, 20)], net: 9)
        return board
    }

    func testACascadeNeedsDepthTwo() throws {
        var shallow = run(cascade()) { $0.pushDepth = 1 }
        let before = shallow.state
        XCTAssertFalse(try push(&shallow, net: 2), "net 5 would have to push net 6, one level too deep")
        XCTAssertTrue(same(shallow.state, before))
        var deep = run(cascade()) { $0.pushDepth = 2 }
        XCTAssertTrue(try push(&deep, net: 2))
        XCTAssertEqual(deep.state.groups(of: 2).components.count, 1)
        let chains = deep.state.ledger.filter { $0.chain != nil }
        XCTAssertEqual(chains.count, 2)
        XCTAssertTrue(chains.allSatisfy(\.touched), "both moved")
        let (copper, result) = finish(&deep)
        XCTAssertEqual(copper.moved.map(\.chain), [0, 1])
        XCTAssertEqual(RoutingCheck.violations(result, deep.problem), [])
    }

    func testABranchOfAMultiPinNetIsNotPushed() throws {
        let board = RoutingTestBoard.ledgerShoveGap()
        var board3 = board
        board3.pad(12, 14, 1, 1, net: 5)
        var run = run(board3)
        // Net 5 as a tree by hand: one connection pad to pad through the gap (through a middle node), and a
        // branch from the third pad that ends on that connection's middle node.
        let terminals = try XCTUnwrap(run.state.terminals[5])
        let a = try XCTUnwrap(terminals[0].first), b = try XCTUnwrap(terminals[1].first), c = try XCTUnwrap(terminals[2].first)
        let middle = run.state.grid.nearestCell(p(10, 10)) + (a.node / run.state.grid.cellCount) * run.state.grid.cellCount
        let trunk = RoutedTrace(layer: .top, width: 0.254, points: [p(3, 10), p(17, 10)], net: 5)
        let branch = RoutedTrace(layer: .top, width: 0.254, points: [p(12, 14), p(10, 10)], net: 5)
        run.state.add(net: 5, path: [a.node, middle, b.node], traces: [trunk], vias: [], allowed: [true])
        run.state.add(net: 5, path: [c.node, middle], traces: [branch], vias: [], allowed: [true])
        XCTAssertEqual(run.state.groups(of: 5).components.count, 1)
        let before = run.state
        XCTAssertFalse(try push(&run, net: 2), "the trunk has a branch on it, and the branch ends on the trunk")
        XCTAssertTrue(same(run.state, before))
    }

    func testACoupledPairIsNeverPushed() throws {
        var run = run(RoutingTestBoard.ledgerShoveGap())
        try lay(&run)
        run.pairNets.insert(5)
        let before = run.state
        XCTAssertFalse(try push(&run, net: 2))
        XCTAssertTrue(same(run.state, before))
    }

    func testACancelledPushIsRestored() throws {
        var run = run(RoutingTestBoard.shoveGap())
        let before = run.state
        var polls = 0
        XCTAssertFalse(try push(&run, net: 2, isCancelled: { polls += 1; return polls > 2 }))
        XCTAssertGreaterThan(polls, 2, "the search was asked")
        XCTAssertTrue(same(run.state, before))
        XCTAssertTrue(run.stopped)
    }

    func testPushIsDeterministic() throws {
        var first = run(RoutingTestBoard.shoveGap()), second = run(RoutingTestBoard.shoveGap())
        XCTAssertTrue(try push(&first, net: 2))
        XCTAssertTrue(try push(&second, net: 2))
        XCTAssertEqual(first.state.ledger, second.state.ledger)
        XCTAssertTrue(same(first.state, second.state))
    }
}
