import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Push and shove in the router (plan-66-E): `join` pushes before it rips up, every strategy
/// uses it, and a tie between runs goes to the one that moved less.
final class RoutingPushIntegrationTests: XCTestCase {
    private func route(_ board: RoutingTestBoard, _ strategy: RoutingOptions.Strategy, push: Bool,
                       isCancelled: () -> Bool = { false }) -> (result: RoutingResult, problem: RoutingProblem) {
        var board = board
        board.options.strategy = strategy
        board.options.pushAndShove = push
        let problem = board.problem()
        return (Router.route(problem, isCancelled: isCancelled), problem)
    }

    func testPushRoutesWhatOffCannot() throws {
        let board = RoutingTestBoard.shoveGap()
        for strategy in RoutingOptions.Strategy.allCases {
            let off = route(board, strategy, push: false)
            XCTAssertEqual(off.result.stats.routed, 0, "\(strategy) off")
            XCTAssertEqual(off.result.stats.connections, 1)
            XCTAssertTrue(off.result.moved.isEmpty, "\(strategy) off")
            XCTAssertEqual(off.result.stats.pushTried, 0, "\(strategy) off")
            let (result, problem) = route(board, strategy, push: true)
            XCTAssertEqual(result.stats.routed, 1, "\(strategy) on")
            XCTAssertEqual(result.moved.count, 1, "\(strategy) on")
            XCTAssertGreaterThanOrEqual(result.stats.pushKept, 1, "\(strategy) on")
            XCTAssertEqual(result.stats.movedTraces, 1)
            XCTAssertEqual(RoutingCheck.violations(result, problem), [], "\(strategy) on")
            let moved = try XCTUnwrap(result.moved.first)
            XCTAssertEqual(moved.runs.first?.first, CGPoint(x: 3, y: 10), "\(strategy): its ends are the pads' centres")
            XCTAssertEqual(moved.runs.first?.last, CGPoint(x: 17, y: 10), "\(strategy)")
        }
    }

    func testALockedTraceIsNeverMoved() throws {
        let board = RoutingTestBoard.chainAsObstacle(RoutingTestBoard.shoveGap())
        let (result, problem) = route(board, .fast, push: true)
        XCTAssertEqual(result.stats.routed, 0)
        XCTAssertTrue(result.moved.isEmpty)
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
    }

    func testNothingBlockedGivesTheOffResult() throws {
        var board = RoutingTestBoard.parallelRows()
        board.pad(2, 2, 1, 1, net: 5)
        board.pad(5, 2, 1, 1, net: 5)
        board.chain([CGPoint(x: 2, y: 2), CGPoint(x: 5, y: 2)], net: 5, from: 6, to: 7)
        for strategy in [RoutingOptions.Strategy.fast, .balanced] {
            let off = route(board, strategy, push: false).result, on = route(board, strategy, push: true).result
            XCTAssertEqual(on.traces, off.traces, "\(strategy)")
            XCTAssertEqual(on.vias, off.vias, "\(strategy)")
            XCTAssertEqual(on.stats.routed, off.stats.routed, "\(strategy)")
            XCTAssertEqual(on.stats.rips, 0)
            XCTAssertEqual(off.stats.rips, 0)
            XCTAssertEqual(on.stats.pushTried, 0, "no hard search failed, so nothing was pushed")
            XCTAssertTrue(on.moved.isEmpty)
        }
    }

    func testPushOnIsDeterministic() throws {
        let board = RoutingTestBoard.shoveGap()
        let first = route(board, .balanced, push: true).result
        for _ in 0..<2 {
            let again = route(board, .balanced, push: true).result
            XCTAssertEqual(again.traces, first.traces)
            XCTAssertEqual(again.vias, first.vias)
            XCTAssertEqual(again.moved, first.moved)
            XCTAssertEqual(again.stats.routed, first.stats.routed)
            XCTAssertEqual(again.stats.pushKept, first.stats.pushKept)
        }
    }

    func testAStoppedRunWithPushIsLegal() throws {
        let board = RoutingTestBoard.shoveGap()
        var polls = 0
        let (result, problem) = route(board, .balanced, push: true) { polls += 1; return polls > 6 }
        XCTAssertGreaterThan(polls, 6, "the run was stopped")
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
        // Whatever the run moved, it left the board legal: the chain is either as it was or moved with the connection.
        XCTAssertEqual(result.moved.count, result.stats.routed)
    }

    func testBetterPrefersLessMovedCopperOnATie() throws {
        var a = RoutingResult(), b = RoutingResult()
        a.stats.routed = 5; b.stats.routed = 5
        a.stats.wireLength = 100; b.stats.wireLength = 100
        a.stats.movedTraces = 3
        let strategy = Router.Strategy.of(.fast)
        XCTAssertTrue(Router.better(b, than: a, strategy: strategy, pitch: 0.127), "the one that moved nothing")
        XCTAssertFalse(Router.better(a, than: b, strategy: strategy, pitch: 0.127))
        // Equal moved copper: neither is better.
        b.stats.movedTraces = 3
        XCTAssertFalse(Router.better(a, than: b, strategy: strategy, pitch: 0.127))
        XCTAssertFalse(Router.better(b, than: a, strategy: strategy, pitch: 0.127))
        // A lower score still wins over less moved copper.
        a.stats.wireLength = 90
        a.stats.movedTraces = 9
        b.stats.movedTraces = 0
        XCTAssertTrue(Router.better(a, than: b, strategy: strategy, pitch: 0.127))
    }
}
