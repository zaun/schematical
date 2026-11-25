import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The repair loop (plan-38-H Phase 4): a connection whose copper breaks a
/// rule is reverted, or ripped up and routed again with a margin, or left
/// unrouted with the reason; never DRC-dirty.
final class RoutingRepairTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// Net 1 from (3, 10) to (17, 10), net 2's pad in the middle at (10, 10).
    private func board(walled: Bool = false) -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20)
        board.options.grid = 0.127
        board.pad(3, 10, 1, 1, net: 1)
        board.pad(17, 10, 1, 1, net: 1)
        board.pad(10, 10, 1, 1, net: 2)
        board.unrouted = [2]
        if walled {
            board.options.vias = .none
            board.trace([p(10, 0), p(10, 9.2)], net: 9)
            board.trace([p(10, 10.8), p(10, 20)], net: 9)
        }
        return board
    }

    /// A run whose ledger holds one illegal connection: straight through
    /// net 2's pad.
    private func illegalRun(_ board: RoutingTestBoard) throws -> Router.Run {
        var run = Router.Run(problem: board.problem())
        let terminals = try XCTUnwrap(run.state.terminals[1])
        let start = try XCTUnwrap(terminals[0].first), end = try XCTUnwrap(terminals[1].first)
        let trace = RoutedTrace(layer: .top, width: 0.254, points: [p(3, 10), p(17, 10)], net: 1)
        run.state.add(net: 1, path: [start.node, end.node], traces: [trace], vias: [], allowed: [true, true])
        XCTAssertEqual(run.state.groups(of: 1).components, [[0, 1]])
        return run
    }

    func testAnIllegalConnectionIsRoutedAgainWithAMargin() throws {
        let board = board()
        var run = try illegalRun(board)
        let problem = run.problem
        XCTAssertFalse(RoutingVerifier.verify(problem, traces: run.result.traces, vias: run.result.vias).isEmpty)
        let copper = run.finishedCopper()
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: copper.traces, vias: copper.vias), [])
        let result = run.result(traces: copper.traces, vias: copper.vias)
        XCTAssertEqual(result.stats.routed, 1, "routed again, round net 2's pad")
        XCTAssertGreaterThan(run.rips, 0)
        XCTAssertNil(result.nets.first { $0.net == 1 }?.reason)
    }

    /// plan-66-C: a repair that routes a net again (`reroute`) copies the ledger into a run of its own;
    /// the chains in it must not be seeded twice, and keep what a push made of them.
    func testRepairRerouteKeepsChainEntries() throws {
        var board = board()
        board.pad(2, 18, 1, 1, net: 7)
        board.pad(6, 18, 1, 1, net: 7)
        board.chain([p(2, 18), p(6, 18)], net: 7, from: 3, to: 4)
        var run = try illegalRun(board)
        let chain = try XCTUnwrap(run.state.ledger.first { $0.chain != nil })
        let moved = [RoutedTrace(layer: .top, width: 0.254, points: [p(2, 18), p(4, 18.5), p(6, 18)], net: 7)]
        run.state.replace(chain.id, traces: moved, vias: [], touched: true)
        let copper = run.finishedCopper()
        XCTAssertGreaterThan(run.rips, 0, "the illegal connection was routed again")
        let entries = run.state.ledger.filter { $0.chain != nil }
        XCTAssertEqual(entries.count, 1, "each chain entry once")
        XCTAssertEqual(entries[0].chain, 0)
        XCTAssertEqual(entries[0].id, chain.id)
        XCTAssertTrue(entries[0].touched, "its flag survives")
        XCTAssertEqual(entries[0].traces, moved, "and its copper")
        XCTAssertEqual(RoutingVerifier.verify(run.problem, traces: copper.traces + copper.chainTraces,
                                              vias: copper.vias + copper.chainVias), [])
    }

    func testAConnectionThatCantBeMadeLegalIsReportedWithTheReason() throws {
        let board = board(walled: true)
        var run = try illegalRun(board)
        let problem = run.problem
        let copper = run.finishedCopper()
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: copper.traces, vias: copper.vias), [], "never dirty")
        let result = run.result(traces: copper.traces, vias: copper.vias)
        let outcome = try XCTUnwrap(result.nets.first { $0.net == 1 })
        XCTAssertEqual(outcome.unroutedConnections, 1)
        XCTAssertEqual(outcome.reason, Router.illegalReason)
    }

    func testRevertingSwapsAnEntrysPiecesForItsRawOnes() {
        let raw = LedgerEntry(id: 7, net: 1, path: [], traces: [RoutedTrace(layer: .top, width: 0.254,
                                                                              points: [p(0, 0), p(1, 0), p(1, 1)], net: 1)],
                              vias: [], box: .zero, nodes: [])
        let cleaned: (traces: [RoutedTrace], vias: [RoutedVia], traceEntry: [Int], viaEntry: [Int]) =
            ([RoutedTrace(layer: .top, width: 0.254, points: [p(0, 0), p(1, 1)], net: 1),
              RoutedTrace(layer: .top, width: 0.254, points: [p(5, 5), p(6, 6)], net: 2)], [], [7, 8], [])
        let reverted = Router.Run.reverting(cleaned, entries: [7], ledger: [7: raw])
        XCTAssertEqual(reverted.traces.map(\.points), [[p(5, 5), p(6, 6)], [p(0, 0), p(1, 0), p(1, 1)]])
        XCTAssertEqual(reverted.traceEntry, [8, 7])
    }

    /// A channel whose legal band for a trace centre is 0.12 mm wide, under
    /// one cell, between two pads of other nets: the route is legal through
    /// it, or the connection is reported; the copper is never dirty.
    func testANarrowChannelIsNeverDirty() throws {
        var board = RoutingTestBoard(width: 20, height: 20, layers: [.top])
        board.options.grid = 0.127
        board.options.vias = .none
        board.pad(3, 10, 1, 1, net: 1)
        board.pad(17, 10, 1, 1, net: 1)
        // Walls of net 9 above and below a gap at x = 10 whose free band for
        // a 0.254 mm trace keeping 0.1 mm is 0.12 mm tall.
        let half = 0.06 + 0.1 + 0.127 + 0.127
        board.trace([p(10, 0), p(10, 10 - half)], net: 9)
        board.trace([p(10, 10 + half), p(10, 20)], net: 9)
        let problem = board.problem()
        let result = Router.route(problem)
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: result.traces, vias: result.vias), [])
        let outcome = try XCTUnwrap(result.nets.first { $0.net == 1 })
        XCTAssertTrue(outcome.status == .complete || outcome.unroutedConnections == 1, "\(outcome)")
    }
}
