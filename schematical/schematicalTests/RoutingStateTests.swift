import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The routing ledger (plan-38-F Phase 1): exact un-stamping, joins, the
/// tile index.
final class RoutingStateTests: XCTestCase {
    private func board() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20)
        board.options.grid = 0.1
        return board
    }

    /// Routes net `net` from component `from` to component `to` on `state`
    /// and records it; returns the entry id.
    @discardableResult
    private func connect(_ state: inout RoutingState, net: Int, from: Int, to: Int) throws -> Int {
        let routing = try XCTUnwrap(state.problem.nets.first { $0.id == net })
        let rules = SearchRules(net: routing, grid: state.grid, problem: state.problem)
        let terminals = try XCTUnwrap(state.terminals[net])
        var search = RoutingSearch(nodes: state.grid.cellCount * state.grid.layers.count)
        let path = try search.search(state.grid, rules: rules, sources: terminals[from].map(\.node),
                                     targets: terminals[to].map(\.node), goal: state.grid.cellBox(terminals[to].map(\.node)),
                                     isCancelled: { false }).get()
        let copper = Router.copper(path, startTie: terminals[from].first { $0.node == path[0] }?.tie,
                                   endTie: terminals[to].first { $0.node == path.last }?.tie, net: routing, grid: state.grid)
        return state.add(net: net, path: path, traces: copper.traces, vias: copper.vias, allowed: rules.allowed)
    }

    func testUnstampingEqualsAStateBuiltWithoutIt() throws {
        var board = board()
        board.pad(3, 5, 1, 1, net: 1); board.pad(17, 5, 1, 1, net: 1)
        board.pad(3, 6.2, 1, 1, net: 2); board.pad(17, 6.2, 1, 1, net: 2)
        board.pad(3, 7.4, 1, 1, net: 3); board.pad(17, 7.4, 1, 1, net: 3)
        var state = RoutingState(problem: board.problem())
        try connect(&state, net: 1, from: 0, to: 1)
        let middle = try connect(&state, net: 2, from: 0, to: 1)
        try connect(&state, net: 3, from: 0, to: 1)
        let kept = state.ledger.filter { $0.id != middle }
        state.remove([middle])

        var fresh = RoutingState(problem: board.problem())
        for entry in kept {
            let allowed = fresh.grid.layers.map { _ in true }
            fresh.add(net: entry.net, path: entry.path, traces: entry.traces, vias: entry.vias, allowed: allowed)
        }
        XCTAssertEqual(state.grid.traceMaps.count, fresh.grid.traceMaps.count)
        XCTAssertTrue(state.grid.traceMaps == fresh.grid.traceMaps, "trace maps equal cell for cell")
        XCTAssertTrue(state.grid.viaMaps == fresh.grid.viaMaps, "via maps equal cell for cell")
        XCTAssertFalse(state.grid.traceMaps == RoutingState(problem: board.problem()).grid.traceMaps,
                       "the other two are still stamped")
    }

    func testUnstampingTheMiddleOfATreeSplitsTheNet() throws {
        var board = board()
        board.pad(5, 10, 1, 1, net: 1)
        board.pad(10, 10, 1, 1, net: 1)
        board.pad(15, 10, 1, 1, net: 1)
        var state = RoutingState(problem: board.problem())
        let first = try connect(&state, net: 1, from: 0, to: 1)
        try connect(&state, net: 1, from: 1, to: 2)
        XCTAssertEqual(state.groups(of: 1).components, [[0, 1, 2]])
        state.remove([first])
        XCTAssertEqual(state.groups(of: 1).components, [[0], [1, 2]])
        XCTAssertEqual(state.dangling(of: 1), [])
    }

    func testADanglingEntryIsFound() throws {
        var board = board()
        board.pad(5, 10, 1, 1, net: 1)
        board.pad(15, 10, 1, 1, net: 1)
        board.pad(10, 15, 1, 1, net: 1)
        var state = RoutingState(problem: board.problem())
        let trunk = try connect(&state, net: 1, from: 0, to: 1)
        // A branch from the middle of the trunk down to the third pad.
        let entry = try XCTUnwrap(state.entries[trunk])
        let routing = state.problem.nets[0]
        let rules = SearchRules(net: routing, grid: state.grid, problem: state.problem)
        let mid = entry.path[entry.path.count / 2]
        let targets = state.terminals[1]![2].map(\.node)
        var search = RoutingSearch(nodes: state.grid.cellCount * 2)
        let path = try search.search(state.grid, rules: rules, sources: [mid], targets: targets,
                                     goal: state.grid.cellBox(targets), isCancelled: { false }).get()
        let copper = Router.copper(path, startTie: nil, endTie: nil, net: routing, grid: state.grid)
        let branch = state.add(net: 1, path: path, traces: copper.traces, vias: copper.vias, allowed: rules.allowed)
        XCTAssertEqual(state.groups(of: 1).components, [[0, 1, 2]])
        state.remove([trunk])
        XCTAssertEqual(state.dangling(of: 1), [branch])
        XCTAssertEqual(state.groups(of: 1).components.count, 3)
    }

    func testTileIndexFindsExactlyTheEntriesMeetingABox() throws {
        var board = board()
        board.boardWidth = 40
        for i in 0..<5 {
            let y = 3 + Double(i) * 3
            board.pad(3 + Double(i) * 6, y, 1, 1, net: i + 1)
            board.pad(8 + Double(i) * 6, y + 1, 1, 1, net: i + 1)
        }
        var state = RoutingState(problem: board.problem())
        for net in 1...5 { try connect(&state, net: net, from: 0, to: 1) }
        let boxes = [CGRect(x: 0, y: 0, width: 10, height: 10), CGRect(x: 14, y: 8, width: 3, height: 3),
                     CGRect(x: 30, y: 0, width: 10, height: 40), CGRect(x: 100, y: 100, width: 1, height: 1),
                     CGRect(x: 0, y: 0, width: 40, height: 20)]
        for box in boxes {
            let found = state.entries(near: box).map(\.id)
            let expected = state.ledger.filter { $0.box.insetBy(dx: -1e-9, dy: -1e-9).intersects(box) }.map(\.id)
            XCTAssertEqual(found, expected, "\(box)")
        }
        XCTAssertEqual(state.entries(near: CGRect(x: 0, y: 0, width: 40, height: 20)).count, 5)
    }
}
