import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The plan made before any copper is put down (`RoutingPlan`): the soft
/// map of what the nets' plans need, and the negotiated passes.
final class RoutingPlanTests: XCTestCase {
    /// Eight nets from a left column to a right column in the opposite
    /// order: every pair crosses.
    private func crossing() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 30, height: 24)
        board.options.grid = 0.127
        for i in 0..<8 {
            board.pad(4, 5 + 2 * Double(i), 1.2, 0.8, net: i + 1)
            board.pad(26, 19 - 2 * Double(i), 1.2, 0.8, net: i + 1)
        }
        return board
    }

    private func strategy(plan: Int) -> Router.Strategy {
        var strategy = Router.Strategy(rounds: 30, viaCost: 12, historyStep: 1, maxRips: 12)
        strategy.planPasses = plan
        return strategy
    }

    func testDemandCountsEachNetOnceAndRemovesExactly() {
        let grid = RoutingGrid(problem: crossing().problem())
        var demand = RoutingDemand(grid: grid)
        let cells = grid.cellCount
        let row = grid.nearestCell(CGPoint(x: 10, y: 12))
        let path = (0..<20).map { row + $0 }
        // The same net twice over the same cells counts once.
        demand.add([path, path], profile: 0)
        XCTAssertEqual(demand.count(node: path[10], profile: 0), 1)
        XCTAssertEqual(demand.count(node: path[10] + cells, profile: 0), 0, "a trace is on its own layer only")
        XCTAssertTrue(demand.conflicts([path], profile: 0).isEmpty, "a net never conflicts with itself")
        // A second net over the same cells shares every one of them.
        demand.add([path], profile: 0)
        XCTAssertEqual(demand.count(node: path[10], profile: 0), 2)
        XCTAssertEqual(demand.conflicts([path], profile: 0).count, path.count)
        // Far enough away it shares nothing.
        let far = (0..<20).map { grid.nearestCell(CGPoint(x: 10, y: 20)) + $0 }
        XCTAssertEqual(demand.count(node: far[10], profile: 0), 0)
        demand.remove([path], profile: 0)
        demand.remove([path, path], profile: 0)
        XCTAssertTrue(demand.counts.allSatisfy { $0 == 0 })
    }

    func testAPlannedViaIsCountedOnEveryLayer() {
        let grid = RoutingGrid(problem: crossing().problem())
        var demand = RoutingDemand(grid: grid)
        let cells = grid.cellCount
        let cell = grid.nearestCell(CGPoint(x: 15, y: 12))
        // Top, then down to Bottom at `cell`.
        demand.add([[cell - 1, cell, cells + cell, cells + cell + 1]], profile: 0)
        let beside = cell + 3 * grid.nx
        XCTAssertEqual(demand.count(node: beside, profile: 0), 1, "inside the via's keep-out on Top")
        XCTAssertEqual(demand.count(node: cells + beside, profile: 0), 1, "and on Bottom")
    }

    func testPlanningStampsNothingAndIsLeftOutWithoutPasses() {
        let problem = crossing().problem()
        var none = Router.Run(problem: problem, strategy: strategy(plan: 0), seed: 1)
        none.planAll(progress: { _ in }, isCancelled: { false })
        XCTAssertNil(none.plan)

        var run = Router.Run(problem: problem, strategy: strategy(plan: 12), seed: 1)
        let before = run.grid
        var reports: [String] = []
        run.planAll(progress: { reports.append($0.net) }, isCancelled: { false })
        let plan = try? XCTUnwrap(run.plan)
        XCTAssertEqual(run.grid.traceMaps, before.traceMaps, "a plan puts no copper down")
        XCTAssertEqual(run.grid.viaMaps, before.viaMaps)
        XCTAssertTrue(run.state.ledger.isEmpty)
        XCTAssertEqual(plan?.plannedNets, Array(1...8))
        XCTAssertEqual(plan?.iterations, plan?.overuseByPass.count)
        XCTAssertEqual(reports.first, "Planning, pass 1")
        // The plan kept is the one that shared the least.
        XCTAssertEqual(plan?.overuse, plan?.overuseByPass.min())
    }

    func testNegotiationSeparatesThePlans() throws {
        let problem = crossing().problem()
        var run = Router.Run(problem: problem, strategy: strategy(plan: 14), seed: 1)
        run.planAll(progress: { _ in }, isCancelled: { false })
        let plan = try XCTUnwrap(run.plan)
        let first = try XCTUnwrap(plan.overuseByPass.first)
        XCTAssertGreaterThan(first, 0, "every net wants the straight way at first")
        XCTAssertLessThan(plan.overuse, first / 2, "\(plan.overuseByPass)")
    }

    func testARoutedNetLeavesThePlanAndTheRunCompletes() throws {
        let problem = crossing().problem()
        var run = Router.Run(problem: problem, strategy: strategy(plan: 14), seed: 1)
        run.routeAll(progress: { _ in }, isCancelled: { false })
        XCTAssertEqual(run.routedCount, 8)
        XCTAssertEqual(run.plan?.plannedNets, [], "each net's plan is released when its copper goes down")
        let copper = run.finishedCopper()
        let result = run.result(traces: copper.traces, vias: copper.vias)
        XCTAssertGreaterThan(result.stats.planPasses, 0)
        XCTAssertEqual(RoutingCheck.violations(result, problem).count, 0)
    }

    func testAPlanPricesButNeverForbids() {
        // A plan's price is added to a search's cost, never taken as an obstacle:
        // a net with one way through still gets it though another's plan is on it.
        var board = RoutingTestBoard(width: 20, height: 10, layers: [.top])
        board.options.grid = 0.127
        board.pad(3, 5, 1, 1, net: 1)
        board.pad(17, 5, 1, 1, net: 1)
        board.pad(3, 3, 1, 1, net: 2)
        board.pad(17, 3, 1, 1, net: 2)
        let problem = board.problem()
        var run = Router.Run(problem: problem, strategy: strategy(plan: 6), seed: 1)
        run.routeAll(progress: { _ in }, isCancelled: { false })
        XCTAssertEqual(run.routedCount, 2)
    }
}
