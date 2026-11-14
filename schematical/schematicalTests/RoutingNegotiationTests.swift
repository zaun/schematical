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

    func testRipTakesHangingEntriesWithIt() throws {
        var (state, first) = try firstRouted(channel())
        XCTAssertEqual(state.rip([first]), [1])
        XCTAssertTrue(state.ledger.isEmpty)
        XCTAssertTrue(state.grid.traceMaps == RoutingState(problem: channel().problem()).grid.traceMaps)
    }
}
