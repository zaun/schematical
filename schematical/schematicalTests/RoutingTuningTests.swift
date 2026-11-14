import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Length-match groups in the engine (plan-38-I Phase 3).
final class RoutingTuningTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    private let groupID = UUID()

    /// Three two-pin nets on Top, 20, 24 and 30 mm between pad centres, in
    /// one group.
    private func board(tolerance: Double = 0.25, lengths: [Double] = [20, 24, 30]) -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 50, height: 40, layers: [.top])
        board.options.grid = 0.127
        board.options.vias = .none
        for (n, length) in lengths.enumerated() {
            let y = 8 + Double(n) * 12
            board.pad(5, y, 1, 1, net: n + 1)
            board.pad(5 + length, y, 1, 1, net: n + 1)
            board.groupOf[n + 1] = groupID
        }
        board.groups = [RoutingGroup(id: groupID, name: "Bus", mode: .lengthMatch, nets: Array(1...lengths.count),
                                     tolerance: tolerance, gap: 0.2, maxUncoupled: 5, maxMeander: 3)]
        return board
    }

    private func outcome(_ result: RoutingResult) throws -> GroupOutcome {
        try XCTUnwrap(result.groups.first)
    }

    private func clean(_ result: RoutingResult, _ problem: RoutingProblem) {
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: result.traces, vias: result.vias), [])
    }

    func testThreeNetsAreMatchedToTheLongest() throws {
        let problem = board().problem()
        let result = Router.route(problem)
        let group = try outcome(result)
        XCTAssertEqual(group.status, .matched, "\(group)")
        XCTAssertEqual(group.target, try XCTUnwrap(group.lengths[3]), accuracy: 1e-9)
        for net in 1...3 {
            XCTAssertLessThanOrEqual(group.target - (group.lengths[net] ?? 0), 0.25 + 1e-6, "net \(net)")
            XCTAssertEqual(group.lengths[net] ?? 0, RoutingTuning.length(of: net, traces: result.traces, problem: problem),
                           accuracy: 1e-9)
        }
        clean(result, problem)
    }

    func testTheLongestNetIsNeverTouched() throws {
        var untuned = board()
        untuned.options.tuneGroups = false
        let before = Router.route(untuned.problem()).traces.filter { $0.net == 3 }
        let after = Router.route(board().problem()).traces.filter { $0.net == 3 }
        XCTAssertEqual(before, after)
    }

    func testAWideToleranceNeedsNoBump() throws {
        var untuned = board(tolerance: 7)
        untuned.options.tuneGroups = false
        let plain = Router.route(untuned.problem())
        let result = Router.route(board(tolerance: 7).problem())
        // 24 is within 7 mm of 30: its copper is as routed; 20 is not.
        XCTAssertEqual(result.traces.filter { $0.net == 2 }, plain.traces.filter { $0.net == 2 })
        XCTAssertNotEqual(result.traces.filter { $0.net == 1 }, plain.traces.filter { $0.net == 1 }, "20 is not")
        XCTAssertEqual(try outcome(result).status, .matched)
    }

    func testABoxedInNetIsResidualAndLegal() throws {
        var board = board(lengths: [20, 30])
        // Net 9's copper close above and below net 1's run: no room to bump.
        board.trace([p(7, 7.3), p(24, 7.3)], net: 9)
        board.trace([p(7, 8.7), p(24, 8.7)], net: 9)
        let problem = board.problem()
        let result = Router.route(problem)
        let group = try outcome(result)
        guard case let .residual(spread, _) = group.status else { return XCTFail("\(group.status)") }
        XCTAssertEqual(spread, group.spread, accuracy: 1e-9)
        XCTAssertGreaterThan(spread, 0.25)
        clean(result, problem)
    }

    func testAThreePinMemberIsIneligible() throws {
        var board = board()
        board.pad(15, 8, 1, 1, net: 1)
        let result = Router.route(board.problem())
        XCTAssertEqual(try outcome(result).status, .ineligible(net: 1, reason: RoutingTuning.multiDrop))
    }

    func testAMemberWithOnlyKeptCopperIsResidual() throws {
        var board = board(lengths: [20, 30])
        // Net 1 drawn by hand: kept copper, not routed this run.
        board.unrouted = [1]
        board.trace([p(5, 8), p(25, 8)], net: 1)
        let result = Router.route(board.problem())
        let group = try outcome(result)
        XCTAssertEqual(group.lengths[1] ?? 0, 20, accuracy: 1e-9)
        XCTAssertEqual(group.status, .residual(spread: group.spread, reason: RoutingTuning.noRoutedCopper))
    }

    func testTwoRunsAreEqual() throws {
        let problem = board().problem()
        XCTAssertEqual(Router.route(problem).traces, Router.route(problem).traces)
    }
}
