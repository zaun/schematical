import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Movable chains in the engine (plan-66-C): held as entries of bystander
/// nets, crossable only at a push price, never ripped, frozen unless touched.
final class RoutingChainEngineTests: XCTestCase {
    /// One layer, no vias, a wall of net 9 across the whole board, and net 1 from the left to the right.
    private func wall() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20, layers: [.top])
        board.options.grid = 0.1
        board.options.vias = .none
        board.pad(3, 10, 1, 1, net: 1)
        board.pad(17, 10, 1, 1, net: 1)
        board.trace([CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 20)], net: 9)
        return board
    }

    private func routed(_ board: RoutingTestBoard, _ strategy: RoutingOptions.Strategy) -> RoutingResult {
        var board = board
        board.options.strategy = strategy
        return Router.route(board.problem())
    }

    // MARK: The grid

    func testTheGridWithChainsEqualsTheGridWithObstacles() throws {
        let kept = RoutingTestBoard.twoGaps()
        let chained = kept.tracesAsChains()
        XCTAssertEqual(chained.chains.count, 3, "the wall's three pieces")
        let state = RoutingState(problem: chained.problem())
        let obstacles = RoutingGrid(problem: kept.problem())
        XCTAssertEqual(state.ledger.filter { $0.chain != nil }.count, 3)
        XCTAssertEqual(state.grid.traceMaps.count, obstacles.traceMaps.count)
        XCTAssertTrue(state.grid.traceMaps == obstacles.traceMaps, "trace maps equal cell for cell")
        XCTAssertTrue(state.grid.viaMaps == obstacles.viaMaps, "via maps equal cell for cell")
        XCTAssertTrue(state.grid.fixedTraceMaps != obstacles.fixedTraceMaps, "the chains are not in the fixed maps")
        // The mask marks exactly where the wall stamped.
        XCTAssertEqual(state.grid.traceMask.count, state.grid.traceMaps.count)
        XCTAssertTrue(state.grid.traceMask.contains(1))
    }

    func testTakingAChainOutRestoresTheGridWithoutIt() throws {
        let chained = RoutingTestBoard.twoGaps().tracesAsChains()
        var state = RoutingState(problem: chained.problem())
        let id = try XCTUnwrap(state.ledger.first { $0.chain == 0 }?.id)
        state.remove([id])
        var rest = RoutingTestBoard.twoGaps()
        rest.obstacles.removeFirst()
        let without = rest.tracesAsChains()
        let fresh = RoutingState(problem: without.problem())
        XCTAssertTrue(state.grid.traceMaps == fresh.grid.traceMaps)
        XCTAssertTrue(state.grid.viaMaps == fresh.grid.viaMaps)
        XCTAssertTrue(state.grid.traceMask == fresh.grid.traceMask, "the mask forgets it too")
    }

    func testReplacingAChainsCopperStampsTheNewCopperExactly() throws {
        let chained = RoutingTestBoard.twoGaps().tracesAsChains()
        var state = RoutingState(problem: chained.problem())
        let entry = try XCTUnwrap(state.ledger.first { $0.chain == 0 })
        var moved = entry.traces
        moved[0].points = moved[0].points.map { CGPoint(x: $0.x + 0.5, y: $0.y) }
        state.replace(entry.id, traces: moved, vias: [], touched: true)
        XCTAssertEqual(state.entries[entry.id]?.touched, true)
        XCTAssertEqual(state.entries[entry.id]?.traces, moved)
        XCTAssertEqual(state.entries[entry.id]?.chain, 0)
        // The grid equals a state built with the entry's copper there from the start.
        var fresh = RoutingState(problem: chained.problem())
        fresh.replace(entry.id, traces: moved, vias: [], touched: true)
        XCTAssertTrue(state.grid.traceMaps == fresh.grid.traceMaps)
        // And putting the old copper back restores the first grid.
        state.replace(entry.id, traces: entry.traces, vias: [], touched: false)
        XCTAssertTrue(state.grid.traceMaps == RoutingState(problem: chained.problem()).grid.traceMaps)
        XCTAssertTrue(state.grid.traceMask == RoutingState(problem: chained.problem()).grid.traceMask)
    }

    // MARK: The search

    func testARelaxedSearchDoesNotCrossAChainUnlessPriced() throws {
        let chained = wall().tracesAsChains()
        let problem = chained.problem()
        let state = RoutingState(problem: problem)
        let net = try XCTUnwrap(problem.nets.first)
        let sources = try XCTUnwrap(state.terminals[1])[0].map(\.node), targets = try XCTUnwrap(state.terminals[1])[1].map(\.node)
        var search = RoutingSearch(nodes: state.grid.cellCount * state.grid.layers.count)
        var rules = SearchRules(net: net, grid: state.grid, problem: problem)
        func find(_ rules: SearchRules) -> Result<[Int], SearchFailure> {
            search.search(state.grid, rules: rules, sources: sources, targets: targets, goal: state.grid.cellBox(targets),
                          history: rules.present > 0 ? state.history : nil, isCancelled: { false })
        }
        XCTAssertEqual(find(rules), .failure(.noPath), "hard")
        rules.present = 2
        XCTAssertEqual(find(rules), .failure(.noPath), "relaxed, no push price: a chain is fixed copper")
        rules.pushPrice = 5
        let path = try find(rules).get()
        XCTAssertGreaterThan(path.count, 2, "relaxed with a push price crosses it")
        // The obstacle form never crosses it, at any price.
        let kept = RoutingState(problem: RoutingTestBoard.chainAsObstacle(chained).problem())
        var fixedSearch = RoutingSearch(nodes: kept.grid.cellCount * kept.grid.layers.count)
        let fixedRules = rules
        XCTAssertEqual(fixedSearch.search(kept.grid, rules: fixedRules, sources: sources, targets: targets,
                                          goal: kept.grid.cellBox(targets), history: kept.history, isCancelled: { false }),
                       .failure(.noPath))
    }

    // MARK: Cleanup and the result

    /// One layer, a chain of net 7 between its pads at (5, 5) and (15, 9), and a lone pad of net 3 at `pad`.
    private func open(pad: CGPoint? = nil) -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20, layers: [.top])
        board.options.grid = 0.1
        board.options.vias = .none
        board.pad(5, 5, 1, 1, net: 7)
        board.pad(15, 9, 1, 1, net: 7)
        board.chain([CGPoint(x: 5, y: 5), CGPoint(x: 15, y: 9)], net: 7, from: 0, to: 1)
        if let pad { board.pad(pad.x, pad.y, 1, 1, net: 3) }
        return board
    }

    private func finish(_ run: inout Router.Run, _ problem: RoutingProblem) -> (FinishedCopper, RoutingResult) {
        let copper = run.finishedCopper()
        var result = run.result(traces: copper.traces, vias: copper.vias)
        result.moved = copper.moved
        return (copper, result)
    }

    func testAnUntouchedChainIsFrozenAndUnlisted() throws {
        let board = RoutingTestBoard.twoGaps().tracesAsChains()
        let problem = board.problem()
        var run = Router.Run(problem: problem)
        run.routeAll(progress: { _ in }, isCancelled: { false })
        let (copper, result) = finish(&run, problem)
        XCTAssertTrue(copper.moved.isEmpty)
        XCTAssertTrue(result.moved.isEmpty)
        XCTAssertEqual(copper.chainTraces.map(\.points), problem.movable.flatMap { $0.runs.map(\.points) }, "the wall as it was")
        XCTAssertFalse(result.traces.contains { $0.net == 9 }, "the new copper has none of it")
        XCTAssertEqual(result.stats.routed, result.stats.connections)
        XCTAssertEqual(result.stats.movedTraces + result.stats.movedVias, 0)
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
    }

    func testATouchedChainIsCleanedAndListed() throws {
        let problem = open().problem()
        var run = Router.Run(problem: problem)
        let id = try XCTUnwrap(run.state.ledger.first { $0.chain == 0 }?.id)
        let staircase = [CGPoint(x: 5, y: 5), CGPoint(x: 7, y: 5), CGPoint(x: 7, y: 6), CGPoint(x: 9, y: 6),
                         CGPoint(x: 9, y: 7), CGPoint(x: 15, y: 7), CGPoint(x: 15, y: 9)]
        run.state.replace(id, traces: [RoutedTrace(layer: .top, width: 0.254, points: staircase, net: 7)], vias: [], touched: true)
        let (copper, result) = finish(&run, problem)
        let moved = try XCTUnwrap(copper.moved.first)
        XCTAssertEqual(copper.moved.count, 1)
        XCTAssertEqual(moved.chain, 0)
        XCTAssertEqual(moved.runs.count, 1)
        XCTAssertLessThan(moved.runs[0].count, staircase.count, "cleaned")
        XCTAssertEqual(moved.runs[0].first, CGPoint(x: 5, y: 5), "its ends stay")
        XCTAssertEqual(moved.runs[0].last, CGPoint(x: 15, y: 9))
        XCTAssertNotEqual(moved.runs[0], [CGPoint(x: 5, y: 5), CGPoint(x: 15, y: 9)])
        XCTAssertEqual(copper.chainTraces.map(\.points), moved.runs)
        XCTAssertTrue(copper.traces.isEmpty, "the chain is not new copper")
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
    }

    func testAChainThatStaysIllegalGoesBack() throws {
        // A chain from (5, 5) to (15, 5) that goes round a big pad of net 3 above it...
        var board = RoutingTestBoard(width: 20, height: 20, layers: [.top])
        board.options.grid = 0.1
        board.options.vias = .none
        board.pad(5, 5, 1, 1, net: 7)
        board.pad(15, 5, 1, 1, net: 7)
        board.pad(10, 5, 2, 2, net: 3)
        board.chain([CGPoint(x: 5, y: 5), CGPoint(x: 8, y: 3), CGPoint(x: 12, y: 3), CGPoint(x: 15, y: 5)], net: 7, from: 0, to: 1)
        let problem = board.problem()
        var run = Router.Run(problem: problem)
        let id = try XCTUnwrap(run.state.ledger.first { $0.chain == 0 }?.id)
        let original = try XCTUnwrap(run.state.entries[id]).traces
        // ... pushed straight over it: no cleaning can make that legal.
        let over = [CGPoint(x: 5, y: 5), CGPoint(x: 10, y: 5), CGPoint(x: 15, y: 5)]
        run.state.replace(id, traces: [RoutedTrace(layer: .top, width: 0.254, points: over, net: 7)], vias: [], touched: true)
        let (copper, result) = finish(&run, problem)
        XCTAssertTrue(copper.moved.isEmpty, "nothing moved")
        XCTAssertEqual(run.state.entries[id]?.traces, original, "the original copper is back")
        XCTAssertEqual(run.state.entries[id]?.touched, false)
        XCTAssertEqual(copper.chainTraces.map(\.points), original.map(\.points))
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
    }

    func testAnIllegalUntouchedChainIsNotRepaired() throws {
        // The user's own chain already runs through net 3's pad.
        let problem = open(pad: CGPoint(x: 10, y: 7)).problem()
        var run = Router.Run(problem: problem)
        XCTAssertFalse(RoutingVerifier.verify(problem, traces: run.state.ledger.flatMap(\.traces), vias: []).isEmpty,
                       "the board is already illegal")
        let (copper, result) = finish(&run, problem)
        XCTAssertTrue(copper.moved.isEmpty)
        XCTAssertEqual(run.state.ledger.filter { $0.chain != nil }.count, 1, "not ripped")
        XCTAssertTrue(run.illegalNets.isEmpty)
        XCTAssertEqual(run.rips, 0)
        XCTAssertTrue(result.traces.isEmpty)
    }

    // MARK: The router

    private func compare(_ a: RoutingResult, _ b: RoutingResult, _ message: String, file: StaticString = #filePath, line: UInt = #line) {
        XCTAssertEqual(a.traces, b.traces, message, file: file, line: line)
        XCTAssertEqual(a.vias, b.vias, message, file: file, line: line)
        XCTAssertEqual(a.nets, b.nets, message, file: file, line: line)
        for (name, x, y) in [("routed", a.stats.routed, b.stats.routed), ("connections", a.stats.connections, b.stats.connections),
                             ("vias", a.stats.vias, b.stats.vias), ("bends", a.stats.bends, b.stats.bends),
                             ("rounds", a.stats.rounds, b.stats.rounds), ("rips", a.stats.rips, b.stats.rips),
                             ("planPasses", a.stats.planPasses, b.stats.planPasses),
                             ("polishTried", a.stats.polishTried, b.stats.polishTried),
                             ("polishKept", a.stats.polishKept, b.stats.polishKept)] {
            XCTAssertEqual(x, y, "\(message): \(name)", file: file, line: line)
        }
        XCTAssertEqual(a.stats.wireLength, b.stats.wireLength, accuracy: 1e-9, message, file: file, line: line)
        XCTAssertEqual(a.stats.pushTried, 0, message, file: file, line: line)
        XCTAssertEqual(a.stats.movedTraces, 0, message, file: file, line: line)
    }

    func testRoutingWithChainsEqualsRoutingWithObstacles() throws {
        var parallel = RoutingTestBoard.parallelRows()
        // A net-5 chain in the corner between two pads of its own.
        parallel.pad(2, 2, 1, 1, net: 5)
        parallel.pad(5, 2, 1, 1, net: 5)
        parallel.chain([CGPoint(x: 2, y: 2), CGPoint(x: 5, y: 2)], net: 5, from: 6, to: 7)
        XCTAssertEqual(parallel.chains.count, 1)
        // The chain's pads are fixed copper in both forms: the same pads, a chain or a kept trace between.
        let boards: [(String, RoutingTestBoard)] = [
            ("twoGaps", RoutingTestBoard.twoGaps().tracesAsChains()),
            ("reversedChannel", RoutingTestBoard.reversedChannel().tracesAsChains()),
            ("parallelRows", parallel),
        ]
        for (name, chained) in boards {
            XCTAssertFalse(chained.chains.isEmpty, name)
            let kept = RoutingTestBoard.chainAsObstacle(chained)
            for strategy in [RoutingOptions.Strategy.fast, .balanced] {
                let on = routed(chained, strategy), off = routed(kept, strategy)
                compare(on, off, "\(name) \(strategy)")
                XCTAssertEqual(on.stats.routed, on.stats.connections, "\(name) \(strategy) routes everything")
                let problem = chained.problem()
                XCTAssertEqual(RoutingCheck.violations(on, problem), [], "\(name) \(strategy)")
            }
        }
    }

    func testAChainEntryIsNeverRipped() throws {
        let chained = wall().tracesAsChains()
        var run = Router.Run(problem: chained.problem(), strategy: Router.Strategy(rounds: 6, viaCost: 12, historyStep: 0.5, maxRips: 8))
        run.routeAll(progress: { _ in }, isCancelled: { false })
        XCTAssertEqual(run.rips, 0, "nothing was ripped")
        XCTAssertEqual(run.state.groups(of: 1).components.count, 2, "the connection stays unrouted")
        let entry = try XCTUnwrap(run.state.ledger.first { $0.chain != nil })
        XCTAssertFalse(entry.touched)
        XCTAssertEqual(run.state.ledger.count, 1, "only the chain is in the ledger")
        // The same board with the wall as a kept trace: the same failure.
        let fixed = Router.route(RoutingTestBoard.chainAsObstacle(chained).problem())
        let held = Router.route(chained.problem())
        XCTAssertEqual(held.stats.routed, 0)
        XCTAssertEqual(fixed.stats.routed, 0)
        // And a rip budget does not matter: ripping is refused outright.
        var state = run.state
        XCTAssertEqual(state.rip([entry.id]), [])
        XCTAssertEqual(state.ledger.count, 1)
    }
}
