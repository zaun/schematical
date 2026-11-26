import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Improving what is routed (`RoutingPolish`), the runs `Router.route`
/// chooses between, and what neither may touch: a coupled pair's copper and
/// a length-match group's outcome.
final class RoutingPolishTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func strategy(polish: Int) -> Router.Strategy {
        var strategy = Router.Strategy(rounds: 30, viaCost: 12, historyStep: 1, maxRips: 12)
        strategy.polishPasses = polish
        return strategy
    }

    private func routed(_ board: RoutingTestBoard, _ strategy: Router.Strategy) -> (result: RoutingResult, problem: RoutingProblem) {
        let problem = board.problem()
        var run = Router.Run(problem: problem, strategy: strategy, seed: problem.options.seed)
        run.routeAll(progress: { _ in }, isCancelled: { false })
        let copper = run.finishedCopper()
        return (run.result(traces: copper.traces, vias: copper.vias), problem)
    }

    /// Net 1 runs straight across; net 2, routed after it, has to cross its
    /// line. Round net 1's pad is a long way, so the first routing hops
    /// under on the Bottom.
    private func wall() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20)
        board.options.grid = 0.127
        board.pad(5, 10, 1, 1, net: 1)
        board.pad(15, 10, 1, 1, net: 1)
        board.pad(10, 3, 1, 1, net: 2)
        board.pad(10, 17, 1, 1, net: 2)
        return board
    }

    func testANeedlessViaPairIsRoutedOut() {
        let (first, problem) = routed(wall(), strategy(polish: 0))
        XCTAssertEqual(first.stats.routed, 2)
        XCTAssertEqual(first.stats.vias, 2, "the first routing takes the short way under")
        XCTAssertEqual(RoutingCheck.violations(first, problem).count, 0)

        let (polished, _) = routed(wall(), strategy(polish: 5))
        XCTAssertEqual(polished.stats.routed, 2)
        XCTAssertEqual(polished.stats.vias, 0, "one layer is enough once a via costs what it should")
        XCTAssertEqual(RoutingCheck.violations(polished, problem).count, 0)
        XCTAssertGreaterThan(polished.stats.polishKept, 0)
        XCTAssertGreaterThanOrEqual(polished.stats.polishTried, polished.stats.polishKept)
        XCTAssertTrue(polished.traces.allSatisfy { $0.layer == .top })
    }

    func testPolishNeverRoutesLessNorScoresWorseAndStaysLegal() {
        let plain = strategy(polish: 0), better = strategy(polish: 10)
        let (first, problem) = routed(RoutingTestBoard.crossing(), plain)
        let (polished, _) = routed(RoutingTestBoard.crossing(), better)
        XCTAssertEqual(RoutingCheck.violations(polished, problem).count, 0)
        XCTAssertGreaterThanOrEqual(polished.stats.routed, first.stats.routed)
        XCTAssertEqual(polished.stats.routed, 8)
        XCTAssertLessThanOrEqual(polished.stats.vias, first.stats.vias)
        var a = first, b = polished
        a.stats.bends = Router.bends(a.traces)
        b.stats.bends = Router.bends(b.traces)
        XCTAssertFalse(Router.better(a, than: b, strategy: better, pitch: 0.127), "what the passes kept never scores worse")
    }

    func testAStoppedPolishLeavesALegalResult() {
        let problem = RoutingTestBoard.crossing().problem()
        // How often a whole run asks, and how often before the improvement starts.
        var whole = 0, first = 0
        var full = Router.Run(problem: problem, strategy: strategy(polish: 10), seed: 1)
        full.routeAll(progress: { _ in }, isCancelled: { whole += 1; return false })
        var plain = Router.Run(problem: problem, strategy: strategy(polish: 0), seed: 1)
        plain.routeAll(progress: { _ in }, isCancelled: { first += 1; return false })
        XCTAssertGreaterThan(whole, first + 2)
        // Stops half way through the improvement passes.
        var run = Router.Run(problem: problem, strategy: strategy(polish: 10), seed: 1)
        var polls = 0
        run.routeAll(progress: { _ in }, isCancelled: { polls += 1; return polls > (whole + first) / 2 })
        XCTAssertTrue(run.stopped)
        XCTAssertEqual(run.routedCount, 8, "a stop keeps every connection the last kept move had")
        let copper = run.finishedCopper()
        let result = run.result(traces: copper.traces, vias: copper.vias)
        XCTAssertEqual(RoutingCheck.violations(result, problem).count, 0)
    }

    // MARK: Pairs and groups

    private let pairID = UUID()

    /// A coupled pair across the middle, and two nets that have to cross it.
    private func pairBoard() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 30, height: 20)
        board.options.grid = 0.127
        board.pad(5, 9.5, 0.4, 0.4, net: 1)
        board.pad(25, 9.5, 0.4, 0.4, net: 1)
        board.pad(5, 10.5, 0.4, 0.4, net: 2)
        board.pad(25, 10.5, 0.4, 0.4, net: 2)
        board.groupOf[1] = pairID
        board.groupOf[2] = pairID
        board.groups = [RoutingGroup(id: pairID, name: "USB", mode: .diffPair, nets: [1, 2], tolerance: 0.25,
                                     gap: 0.2, maxUncoupled: 5, maxMeander: 2)]
        board.pad(12, 4, 1, 1, net: 3)
        board.pad(12, 16, 1, 1, net: 3)
        board.pad(18, 4, 1, 1, net: 4)
        board.pad(18, 16, 1, 1, net: 4)
        return board
    }

    func testACoupledPairIsNeverTakenUp() {
        let problem = pairBoard().problem()
        var plain = Router.Run(problem: problem, strategy: strategy(polish: 0), seed: 1)
        plain.routeAll(progress: { _ in }, isCancelled: { false })
        var polished = Router.Run(problem: problem, strategy: strategy(polish: 10), seed: 1)
        polished.routeAll(progress: { _ in }, isCancelled: { false })
        XCTAssertEqual(polished.pairNets, [1, 2])
        guard case .coupled = polished.pairStatus[pairID] else { return XCTFail("the pair stays coupled") }
        for net in [1, 2] {
            XCTAssertFalse(polished.polishable(net))
            XCTAssertEqual(polished.state.ledger(of: net).map(\.traces), plain.state.ledger(of: net).map(\.traces),
                           "the pair's copper is as it was first routed")
            XCTAssertEqual(polished.state.ledger(of: net).map(\.vias), plain.state.ledger(of: net).map(\.vias))
        }
        XCTAssertEqual(polished.routedCount, 4)
        let copper = polished.finishedCopper()
        XCTAssertEqual(RoutingCheck.violations(polished.result(traces: copper.traces, vias: copper.vias), problem).count, 0)
    }

    func testALengthGroupIsStillMatchedByTheWholeRoute() throws {
        let group = UUID()
        var board = RoutingTestBoard(width: 40, height: 20)
        board.options.grid = 0.127
        board.pad(5, 6, 1, 1, net: 1)
        board.pad(35, 6, 1, 1, net: 1)
        board.pad(10, 12, 1, 1, net: 2)
        board.pad(30, 12, 1, 1, net: 2)
        board.groupOf[1] = group
        board.groupOf[2] = group
        board.groups = [RoutingGroup(id: group, name: "Bus", mode: .lengthMatch, nets: [1, 2], tolerance: 0.5,
                                     gap: 0.2, maxUncoupled: 5, maxMeander: 2)]
        let problem = board.problem()
        XCTAssertEqual(problem.options.strategy, .balanced)
        let result = Router.route(problem)
        XCTAssertEqual(result.stats.routed, 2)
        let outcome = try XCTUnwrap(result.groups.first)
        XCTAssertEqual(outcome.status, .matched)
        XCTAssertLessThanOrEqual(outcome.spread, 0.5 + 1e-6)
        XCTAssertEqual(RoutingCheck.violations(result, problem).count, 0)
    }

    // MARK: The runs `route` chooses between

    func testBalancedAndThoroughMakeSeveralRunsAndFastOne() {
        XCTAssertTrue(Router.Strategy.of(.fast).viaCosts.isEmpty)
        XCTAssertEqual(Router.Strategy.of(.fast).polishPasses, 0)
        XCTAssertGreaterThan(Router.Strategy.of(.balanced).viaCosts.count, 1)
        // Thorough makes every run Balanced does, the same way, and more.
        let balanced = Router.Strategy.of(.balanced), thorough = Router.Strategy.of(.thorough)
        XCTAssertGreaterThan(thorough.viaCosts.count, balanced.viaCosts.count)
        XCTAssertEqual(Array(thorough.viaCosts.prefix(balanced.viaCosts.count)), balanced.viaCosts)
        XCTAssertEqual(thorough.planPasses, balanced.planPasses)
        XCTAssertEqual(thorough.polishPasses, balanced.polishPasses)
        XCTAssertEqual(thorough.rounds, balanced.rounds)
        XCTAssertGreaterThan(Router.Strategy.of(.balanced).polishPasses, 0)
        XCTAssertGreaterThan(Router.Strategy.of(.balanced).planPasses, 0)
    }

    func testTheSameBoardGivesTheSameCopperEveryTime() {
        let problem = RoutingTestBoard.crossing().problem()
        let a = Router.route(problem), b = Router.route(problem)
        XCTAssertEqual(a.traces, b.traces)
        XCTAssertEqual(a.vias, b.vias)
        XCTAssertEqual(a.stats.routed, 8)
        XCTAssertEqual(a.stats.bends, Router.bends(a.traces))
        XCTAssertEqual(RoutingCheck.violations(a, problem).count, 0)
    }

    func testTheRouteKeepsTheBestOfItsRuns() {
        let problem = RoutingTestBoard.crossing().problem()
        let chosen = Router.route(problem)
        let balanced = Router.Strategy.of(.balanced)
        for cost in balanced.viaCosts {
            var own = balanced
            own.viaCost = cost
            var run = Router.Run(problem: problem, strategy: own, seed: problem.options.seed)
            run.routeAll(progress: { _ in }, isCancelled: { false })
            let copper = run.finishedCopper()
            var result = run.result(traces: copper.traces, vias: copper.vias)
            result.stats.bends = Router.bends(result.traces)
            XCTAssertFalse(Router.better(result, than: chosen, strategy: balanced, pitch: problem.options.grid),
                           "no run at via cost \(cost) beats the one kept")
        }
    }

    func testBetterPrefersMoreRoutedThenFewerViasAndBends() {
        let strategy = Router.Strategy.of(.balanced)
        var a = RoutingResult(), b = RoutingResult()
        a.stats.routed = 5; a.stats.wireLength = 500; a.stats.vias = 40
        b.stats.routed = 4; b.stats.wireLength = 100
        XCTAssertTrue(Router.better(a, than: b, strategy: strategy, pitch: 0.127))
        XCTAssertFalse(Router.better(b, than: a, strategy: strategy, pitch: 0.127))
        b.stats.routed = 5; b.stats.wireLength = 500; b.stats.vias = 39
        XCTAssertTrue(Router.better(b, than: a, strategy: strategy, pitch: 0.127))
        a.stats.vias = 39; a.stats.bends = 3; b.stats.bends = 4
        XCTAssertTrue(Router.better(a, than: b, strategy: strategy, pitch: 0.127))
        b.stats.bends = 3
        XCTAssertFalse(Router.better(a, than: b, strategy: strategy, pitch: 0.127))
        XCTAssertFalse(Router.better(b, than: a, strategy: strategy, pitch: 0.127))
    }

    func testProgressComesFromOneRunAtATimeAndInOrder() {
        let problem = RoutingTestBoard.crossing().problem()
        var totals: [Int] = []
        _ = Router.route(problem, progress: { totals.append($0.routed) })
        XCTAssertFalse(totals.isEmpty)
        XCTAssertEqual(totals.max(), 8)
    }
}
