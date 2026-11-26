import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-68-W: the routing engine's less travelled paths — kept-trace
/// terminals, mark generations wrapping, a search that runs out of open
/// nodes with a best target, options that fail to decode, a problem's
/// defaults, copper by a non-plated hole, an unrouted length-match member.
final class RoutingEngineEdgeTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    // MARK: Terminals

    /// A kept trace of the component gives terminals along its line, on its
    /// own layer only, each tied to the nearest point of the segment.
    func testAKeptTracesTerminalsTieOnItsLine() {
        var board = RoutingTestBoard(width: 10, height: 10)
        board.options.grid = 0.1
        board.pad(2, 5, 0.6, 0.6, net: 1)
        let grid = RoutingGrid(problem: board.problem())
        let trace = RoutedTrace(layer: .top, width: 0.254, points: [p(3, 5.02), p(6, 5.02)])
        let component = RoutingComponent(traces: [trace])
        let profile = grid.profile(of: 1)

        let found = grid.terminals(of: component, net: 1, profile: profile, allowed: [true, true])
        XCTAssertFalse(found.isEmpty)
        for terminal in found {
            XCTAssertLessThan(terminal.node, grid.cellCount, "on Top, the trace's layer")
            let tie = try? XCTUnwrap(terminal.tie)
            XCTAssertEqual(tie?.y ?? 0, 5.02, accuracy: 1e-9, "on the segment's line")
            XCTAssertTrue((3...6).contains(tie?.x ?? -1))
            let cell = grid.point(terminal.node % grid.cellCount)
            XCTAssertEqual(tie?.x ?? 0, min(max(cell.x, 3), 6), accuracy: 1e-9, "the nearest point of the segment")
        }
        XCTAssertEqual(grid.terminals(of: component, net: 1, profile: profile, allowed: [false, true]), [],
                       "a trace on a layer the net may not use gives none")
    }

    func testProjectClampsToTheSegment() {
        XCTAssertEqual(RoutingGrid.project(p(5, 1), p(0, 0), p(10, 0)), p(5, 0))
        XCTAssertEqual(RoutingGrid.project(p(-3, 2), p(0, 0), p(10, 0)), p(0, 0))
        XCTAssertEqual(RoutingGrid.project(p(20, -1), p(0, 0), p(10, 0)), p(10, 0))
        XCTAssertEqual(RoutingGrid.project(p(4, 4), p(1, 1), p(1, 1)), p(1, 1), "a point segment")
    }

    // MARK: Generations

    private func twoPads() -> (RoutingProblem, RoutingGrid, SearchRules, [Int], [Int]) {
        var board = RoutingTestBoard(width: 10, height: 10)
        board.options.grid = 0.25
        board.pad(2, 5, 0.6, 0.6, net: 1)
        board.pad(8, 5, 0.6, 0.6, net: 1)
        let problem = board.problem()
        let grid = RoutingGrid(problem: problem)
        let rules = SearchRules(net: problem.nets[0], grid: grid, problem: problem)
        let from = grid.terminals(of: problem.nets[0].components[0], net: 1, profile: rules.profile, allowed: rules.allowed)
        let to = grid.terminals(of: problem.nets[0].components[1], net: 1, profile: rules.profile, allowed: rules.allowed)
        return (problem, grid, rules, from.map(\.node), to.map(\.node))
    }

    /// After `UInt32.max` searches the target marks start again from 1:
    /// old marks never read as targets, and the searches still find the pads.
    func testSearchMarksSurviveTheGenerationWrapping() throws {
        let (_, grid, rules, from, to) = twoPads()
        var search = RoutingSearch(nodes: grid.cellCount * grid.layers.count)
        let first = try search.search(grid, rules: rules, sources: from, targets: to, goal: grid.cellBox(to),
                                      isCancelled: { false }).get()
        search.generation = .max
        // Searched the other way: the first search's targets are now sources.
        let back = try search.search(grid, rules: rules, sources: to, targets: from, goal: grid.cellBox(from),
                                     isCancelled: { false }).get()
        XCTAssertEqual(search.generation, 1)
        XCTAssertTrue(from.contains(back.last!))
        XCTAssertEqual(back.count, first.count)
        let again = try search.search(grid, rules: rules, sources: from, targets: to, goal: grid.cellBox(to),
                                      isCancelled: { false }).get()
        XCTAssertEqual(again, first)
        XCTAssertEqual(search.generation, 2)
    }

    func testDemandSurvivesTheGenerationWrapping() {
        let (_, grid, _, from, _) = twoPads()
        var demand = RoutingDemand(grid: grid)
        demand.generation = .max
        demand.add([[from[0]]], profile: 0)
        XCTAssertEqual(demand.generation, 1)
        XCTAssertEqual(demand.count(node: from[0], profile: 0), 1)
        demand.remove([[from[0]]], profile: 0)
        XCTAssertEqual(demand.count(node: from[0], profile: 0), 0)
        XCTAssertEqual(demand.generation, 2)
    }

    /// With target costs, a best target found while nothing open could beat
    /// it until the board runs out of nodes is still the answer.
    func testATargetCostSearchEndsWithTheBestWhenNothingIsLeftOpen() throws {
        var board = RoutingTestBoard(width: 2, height: 2, layers: [.top])
        board.options.grid = 0.25
        board.pad(1, 1, 0.3, 0.3, net: 1)
        let problem = board.problem()
        let grid = RoutingGrid(problem: problem)
        let rules = SearchRules(net: problem.nets[0], grid: grid, problem: problem)
        let nodes = grid.terminals(of: problem.nets[0].components[0], net: 1, profile: rules.profile,
                                   allowed: rules.allowed).map(\.node)
        let node = try XCTUnwrap(nodes.first)
        var search = RoutingSearch(nodes: grid.cellCount)
        let path = try search.search(grid, rules: rules, sources: [node], targets: [node], goal: nil,
                                     targetCosts: [node: 1000], isCancelled: { false }).get()
        XCTAssertEqual(path, [node])
        XCTAssertEqual(search.lastCost, 1000)
        XCTAssertGreaterThan(search.expansions, 1, "it looked at the rest of the board first")
    }

    // MARK: Options and the problem

    func testOptionsWithAnUnknownLayerOrNetDoNotDecode() throws {
        var options = RoutingOptions()
        options.layers[.top] = RoutingOptions.LayerOption(enabled: false, direction: .vertical)
        let good = try JSONEncoder().encode(options)
        XCTAssertEqual(try JSONDecoder().decode(RoutingOptions.self, from: good).layers[.top]?.enabled, false)
        let text = try XCTUnwrap(String(data: good, encoding: .utf8))
        let key = "\"\(CopperLayer.top.rawValue)\":"
        XCTAssertTrue(text.contains(key), text)
        let badLayer = Data(text.replacingOccurrences(of: key, with: "\"no such layer\":").utf8)
        XCTAssertThrowsError(try JSONDecoder().decode(RoutingOptions.self, from: badLayer)) { error in
            XCTAssertTrue("\(error)".contains("no such layer"), "\(error)")
        }
        XCTAssertThrowsError(try JSONDecoder().decode([PlaneNet].self, from: Data(#"["not a net!"]"#.utf8))) { error in
            XCTAssertTrue("\(error)".contains("not a net!"), "\(error)")
        }
        let nets = try? JSONDecoder().decode([PlaneNet].self, from: JSONEncoder().encode([PlaneNet.ground]))
        XCTAssertEqual(nets, [.ground])
    }

    func testAProblemStartsWithNoCutoutsChainsOrBystanders() {
        let problem = RoutingProblem(outline: [p(0, 0), p(1, 0), p(1, 1)], layers: [.top], routable: [.top], planes: [:],
                                     rules: .customDefault, options: RoutingOptions(), nets: [], obstacles: [], groups: [],
                                     removedTraces: [], removedVias: [])
        XCTAssertEqual(problem.cutouts, [])
        XCTAssertEqual(problem.movable, [])
        XCTAssertEqual(problem.bystanders, [])
        XCTAssertEqual(problem.connections, 0)
    }

    // MARK: Verifier and tuning

    /// New copper keeps the copper-to-edge distance from a non-plated hole.
    func testCopperTooNearANonPlatedHoleIsAClearanceViolation() {
        var board = RoutingTestBoard()
        board.hole(10, 10, drill: 1)
        let problem = board.problem()
        let near = RoutedTrace(layer: .top, width: 0.254, points: [p(5, 10.6), p(15, 10.6)], net: 1)
        let found = RoutingVerifier.verify(problem, traces: [near], vias: []).filter { $0.detail.contains("non-plated hole") }
        XCTAssertEqual(found.map(\.kind), [.clearance])
        XCTAssertEqual(found.first?.traces, [0])
        let far = RoutedTrace(layer: .top, width: 0.254, points: [p(5, 11.2), p(15, 11.2)], net: 1)
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: [far], vias: []).filter { $0.detail.contains("hole") }, [])
    }

    /// A length-match member the run left unrouted makes the group not matched, naming it.
    func testAnUnroutedMemberMakesTheGroupIneligible() {
        let group = UUID()
        var board = RoutingTestBoard()
        board.pad(2, 2, 0.5, 0.5, net: 1)
        board.pad(18, 2, 0.5, 0.5, net: 1)
        board.pad(2, 8, 0.5, 0.5, net: 2)
        board.pad(18, 8, 0.5, 0.5, net: 2)
        board.groups = [RoutingGroup(id: group, name: "BUS", mode: .lengthMatch, nets: [1, 2], tolerance: 0.25, gap: 0,
                                     maxUncoupled: 0, maxMeander: 2)]
        let problem = board.problem()
        var traces = [RoutedTrace(layer: .top, width: 0.254, points: [p(2, 2), p(18, 2)], net: 1)]
        let outcomes = [NetOutcome(net: 1, name: "N1", status: .complete, unroutedConnections: 0),
                        NetOutcome(net: 2, name: "N2", status: .failed, unroutedConnections: 1)]
        let result = RoutingTuning.tune(problem, traces: &traces, vias: [], outcomes: outcomes)
        XCTAssertEqual(result.map(\.status), [.ineligible(net: 2, reason: RoutingTuning.notRouted)])
        XCTAssertEqual(result.first?.lengths.keys.sorted(), [1])
    }
}
