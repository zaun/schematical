import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Negotiated rip-up and reroute (plan-38-F Phases 2–3).
final class RoutingNegotiationTests: XCTestCase {
    /// One layer, no vias: a wall of net 9 down the middle with a 1 mm gap
    /// at y = 10, room for one trace. Net 1 runs straight through the gap;
    /// net 2 crosses from the lower left to the upper right and needs it too.
    private func channel(gap: Bool = true) -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20, layers: [.top])
        board.options.grid = 0.1
        board.options.vias = .none
        board.pad(3, 10, 1, 1, net: 1)
        board.pad(17, 10, 1, 1, net: 1)
        board.pad(3, 6, 1, 1, net: 2)
        board.pad(17, 14, 1, 1, net: 2)
        if gap {
            board.trace([CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 9.5)], net: 9)
            board.trace([CGPoint(x: 10, y: 10.5), CGPoint(x: 10, y: 20)], net: 9)
        } else {
            board.trace([CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 20)], net: 9)
        }
        return board
    }

    private func search(_ state: RoutingState, net: Int, present: Double) -> (Result<[Int], SearchFailure>, Float) {
        let routing = state.problem.nets.first { $0.id == net }!
        var rules = SearchRules(net: routing, grid: state.grid, problem: state.problem)
        rules.present = present
        let terminals = state.terminals[net]!
        var search = RoutingSearch(nodes: state.grid.cellCount * state.grid.layers.count)
        let result = search.search(state.grid, rules: rules, sources: terminals[0].map(\.node),
                                   targets: terminals[1].map(\.node), goal: state.grid.cellBox(terminals[1].map(\.node)),
                                   history: present > 0 ? state.history : nil, isCancelled: { false })
        return (result, search.lastCost)
    }

    /// Net 1 routed through the gap.
    private func firstRouted(_ board: RoutingTestBoard) throws -> (RoutingState, Int) {
        var state = RoutingState(problem: board.problem())
        let (result, _) = search(state, net: 1, present: 0)
        let path = try result.get()
        let routing = state.problem.nets[0]
        let terminals = state.terminals[1]!
        let copper = Router.copper(path, startTie: terminals[0].first { $0.node == path[0] }?.tie,
                                   endTie: terminals[1].first { $0.node == path.last }?.tie, net: routing, grid: state.grid)
        let id = state.add(net: 1, path: path, traces: copper.traces, vias: copper.vias, allowed: [true])
        return (state, id)
    }

    // MARK: Phase 2

    func testRelaxedSearchNamesTheBlockingConnection() throws {
        let (state, first) = try firstRouted(channel())
        XCTAssertEqual(search(state, net: 2, present: 0).0, .failure(.noPath), "the gap is taken")
        let (relaxed, _) = search(state, net: 2, present: 2)
        let path = try relaxed.get()
        XCTAssertEqual(state.victims(of: path, net: 2), [first])
    }

    func testRelaxedSearchNeverCrossesFixedCopper() throws {
        let state = RoutingState(problem: channel(gap: false).problem())
        XCTAssertEqual(search(state, net: 2, present: 2).0, .failure(.noPath))
        XCTAssertEqual(search(state, net: 2, present: 1000).0, .failure(.noPath))
    }

    func testHistoryMakesAContestedPathDearer() throws {
        var (state, _) = try firstRouted(channel())
        let (relaxed, before) = search(state, net: 2, present: 2)
        let path = try relaxed.get()
        state.addHistory(path, step: 1)
        let (again, after) = search(state, net: 2, present: 2)
        XCTAssertNoThrow(try again.get())
        XCTAssertGreaterThan(after, before)
    }

    // MARK: Phase 3

    /// Two gaps in a wall of net 9, one layer, no vias: A (y 9.4–10.6) fits
    /// either net, B (y 3.6–4.4) only net 1, since net 2 keeps a 0.2 mm
    /// clearance. The single pass routes net 1 first (shorter), through A;
    /// net 2 can then only be routed by moving net 1 to B.
    private func twoGaps() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20, layers: [.top])
        board.options.grid = 0.1
        board.options.vias = .none
        board.pad(3, 10, 1, 1, net: 1)
        board.pad(17, 10, 1, 1, net: 1)
        board.pad(3, 13, 1, 1, net: 2)
        board.pad(17, 16, 1, 1, net: 2)
        board.clearance[2] = 0.2
        board.trace([CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 3.6)], net: 9)
        board.trace([CGPoint(x: 10, y: 4.4), CGPoint(x: 10, y: 9.4)], net: 9)
        board.trace([CGPoint(x: 10, y: 10.6), CGPoint(x: 10, y: 20)], net: 9)
        return board
    }

    private func complete(_ result: RoutingResult) -> Bool {
        result.stats.routed == result.stats.connections
    }

    func testEveryStrategyCompletesWhereTheSinglePassFails() throws {
        var board = twoGaps()
        board.options.strategy = .fast
        var single = Router.Run(problem: board.problem(), strategy: Router.Strategy(rounds: 0, viaCost: 12,
                                                                                  historyStep: 1, maxRips: 12))
        single.routeAll(progress: { _ in }, isCancelled: { false })
        XCTAssertEqual(single.result.stats.routed, 1, "the single pass leaves net 2 unrouted")
        for strategy in RoutingOptions.Strategy.allCases {
            board.options.strategy = strategy
            let problem = board.problem()
            let result = Router.route(problem)
            XCTAssertTrue(complete(result), "\(strategy): \(result.nets)")
            // Fast has one run and no plan: it gets there only by ripping net 1 up.
            if strategy == .fast { XCTAssertGreaterThan(result.stats.rips, 0, "\(strategy) ripped net 1 up") }
            XCTAssertEqual(RoutingCheck.violations(result, problem), [], "\(strategy)")
        }
    }

    /// Six nets from the left to the right in reversed order, two layers.
    private func reversedChannel() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20)
        board.options.grid = 0.1
        for i in 0..<6 {
            board.pad(2, 4 + Double(i) * 2.4, 1, 1, net: i + 1)
            board.pad(18, 4 + Double(5 - i) * 2.4, 1, 1, net: i + 1)
        }
        // Walls above and below the channel.
        board.trace([CGPoint(x: 5, y: 2), CGPoint(x: 15, y: 2)], width: 0.5, net: 9)
        board.trace([CGPoint(x: 5, y: 18), CGPoint(x: 15, y: 18)], width: 0.5, net: 9)
        return board
    }

    func testReversedChannelCompletesUnderBalanced() throws {
        let problem = reversedChannel().problem()
        let result = Router.route(problem)
        XCTAssertTrue(complete(result), "\(result.nets.filter { $0.status != .complete })")
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
    }

    func testACancelAfterTheFirstRoundIsLegal() throws {
        let problem = twoGaps().problem()
        var round = 0
        let result = Router.route(problem, progress: { round = max(round, $0.round) }, isCancelled: { round >= 1 })
        XCTAssertTrue(result.stats.stopped || complete(result))
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])

        let busy = reversedChannel().problem()
        var calls = 0
        let early = Router.route(busy, isCancelled: { calls += 1; return calls > 2 })
        XCTAssertTrue(early.stats.stopped)
        XCTAssertEqual(RoutingCheck.violations(early, busy), [])
    }

    func testATinyTimeLimitReturnsPromptlyAndLegal() throws {
        var board = reversedChannel()
        board.options.timeLimit = 0.001
        let problem = board.problem()
        let started = Date()
        let result = Router.route(problem)
        XCTAssertLessThan(Date().timeIntervalSince(started), 5)
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
    }

    func testSameResultTwiceAndInAnyNetOrder() throws {
        let problem = reversedChannel().problem()
        let once = Router.route(problem)
        XCTAssertEqual(once.traces, Router.route(problem).traces)
        var reversed = problem
        reversed.nets.reverse()
        let other = Router.route(reversed)
        XCTAssertEqual(once.traces, other.traces)
        XCTAssertEqual(once.vias, other.vias)
        XCTAssertEqual(once.nets, other.nets)
    }

    func testThoroughSeedsAreEachLegal() throws {
        var board = reversedChannel()
        board.options.strategy = .thorough
        for seed in [UInt64(1), 2] {
            board.options.seed = seed
            let problem = board.problem()
            let result = Router.route(problem)
            XCTAssertTrue(complete(result), "seed \(seed)")
            XCTAssertEqual(RoutingCheck.violations(result, problem), [], "seed \(seed)")
        }
    }

    func testANetBlockedByFixedCopperStallsTheRunEarly() throws {
        var board = channel(gap: false)
        board.pads.removeAll { $0.net == 1 }
        let result = Router.route(board.problem())
        XCTAssertEqual(result.nets.first { $0.net == 2 }?.status, .failed)
        XCTAssertLessThanOrEqual(result.stats.rounds, 1 + Router.Strategy.of(.balanced).stall,
                                 "no change for 3 rounds ends the run, not 30")
    }

    func testRipTakesHangingEntriesWithIt() throws {
        var (state, first) = try firstRouted(channel())
        XCTAssertEqual(state.rip([first]), [1])
        XCTAssertTrue(state.ledger.isEmpty)
        XCTAssertTrue(state.grid.traceMaps == RoutingState(problem: channel().problem()).grid.traceMaps)
    }
}
