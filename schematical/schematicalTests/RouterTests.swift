import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Copper checks shared by the engine tests.
enum RoutingCheck {
    /// Every routed piece of copper as shapes, with its net.
    static func shapes(_ result: RoutingResult) -> [(shape: CopperShape, net: Int)] {
        var shapes: [(CopperShape, Int)] = []
        for (t, trace) in result.traces.enumerated() {
            for (a, b) in zip(trace.points, trace.points.dropFirst()) {
                shapes.append((CopperShape(core: .segment(a, b), radius: trace.width / 2, layers: [trace.layer], node: t),
                               trace.net ?? -1))
            }
        }
        for (v, via) in result.vias.enumerated() {
            shapes.append((CopperShape(core: .segment(via.centre, via.centre), radius: via.diameter / 2, layers: nil,
                                       node: 100_000 + v), via.net ?? -1))
        }
        return shapes
    }

    /// The pairs of copper of different nets (routed against routed, and
    /// routed against the problem's obstacles) closer than the larger of
    /// their nets' clearances, less the DRC's 1e-4 mm tolerance.
    static func violations(_ result: RoutingResult, _ problem: RoutingProblem) -> [String] {
        let clearance = Dictionary(uniqueKeysWithValues: problem.nets.map { ($0.id, $0.clearance) })
        func c(_ net: Int?) -> Double { net.flatMap { clearance[$0] } ?? problem.rules.minClearance }
        let routed = shapes(result)
        var found: [String] = []
        for i in routed.indices {
            for j in routed.indices where j > i && routed[i].net != routed[j].net && routed[i].shape.shares(routed[j].shape) {
                let gap = routed[i].shape.gap(to: routed[j].shape)
                if gap < max(c(routed[i].net), c(routed[j].net)) - 1e-4 {
                    found.append("net \(routed[i].net) vs net \(routed[j].net): gap \(gap)")
                }
            }
            for obstacle in problem.obstacles where obstacle.shape.node != -1 && obstacle.net != routed[i].net
                && obstacle.shape.shares(routed[i].shape) {
                let gap = routed[i].shape.gap(to: obstacle.shape)
                if gap < max(c(routed[i].net), c(obstacle.net)) - 1e-4 {
                    found.append("net \(routed[i].net) vs obstacle of net \(obstacle.net.map(String.init) ?? "none"): gap \(gap)")
                }
            }
        }
        return found
    }
}

/// The router's single pass (plan-38-E Phase 3): trees, failures, layers,
/// classes, plane fan-out, statuses, determinism.
final class RouterTests: XCTestCase {
    private func board(layers: [CopperLayer] = [.top, .bottom]) -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20, layers: layers)
        board.options.grid = 0.1
        return board
    }

    private func outcome(_ result: RoutingResult, _ net: Int) throws -> NetOutcome {
        try XCTUnwrap(result.nets.first { $0.net == net })
    }

    func testThreePadNetIsATree() throws {
        var board = board()
        board.pad(5, 10, 1, 1, net: 1)
        board.pad(15, 10, 1, 1, net: 1)
        board.pad(10, 15, 1, 1, net: 1)
        let problem = board.problem()
        let result = Router.route(problem)
        XCTAssertEqual(try outcome(result, 1).status, .complete)
        XCTAssertEqual(result.stats.routed, 2)
        XCTAssertEqual(result.stats.connections, 2)
        XCTAssertGreaterThanOrEqual(result.traces.count, 2)
        XCTAssertTrue(result.traces.allSatisfy { $0.points.count >= 2 && $0.net == 1 })
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
    }

    func testAFailedConnectionKeepsTheOthers() throws {
        var board = board()
        board.options.vias = .none
        board.pad(5, 10, 1, 1, net: 1)
        board.pad(10, 4, 1, 1, net: 1)
        board.pad(15, 10, 1, 1, net: 1)
        // A closed ring of net 2 round the third pad.
        board.trace([CGPoint(x: 13, y: 8), CGPoint(x: 17, y: 8), CGPoint(x: 17, y: 12), CGPoint(x: 13, y: 12),
                     CGPoint(x: 13, y: 8)], net: 2)
        let result = Router.route(board.problem())
        let net = try outcome(result, 1)
        XCTAssertEqual(net.status, .partial(unrouted: 1))
        XCTAssertEqual(net.unroutedConnections, 1)
        XCTAssertEqual(net.unrouted.count, 1)
        XCTAssertTrue(net.unrouted[0].hasSuffix("P3"), net.unrouted[0])
        XCTAssertEqual(result.stats.routed, 1)
    }

    func testCrossingNetsBothCompleteWithAVia() throws {
        var board = board()
        board.pad(4, 10, 1, 1, net: 1)
        board.pad(16, 10, 1, 1, net: 1)
        board.pad(10, 4, 1, 1, net: 2)
        board.pad(10, 16, 1, 1, net: 2)
        // Fast takes the first legal way: the second net hops under the first.
        board.options.strategy = .fast
        var problem = board.problem()
        var result = Router.route(problem)
        XCTAssertEqual(try outcome(result, 1).status, .complete)
        XCTAssertEqual(try outcome(result, 2).status, .complete)
        XCTAssertGreaterThanOrEqual(result.vias.count, 2, "one net crosses the other on Bottom")
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
        // Balanced improves on it: round the other net's pad, on one layer.
        board.options.strategy = .balanced
        problem = board.problem()
        result = Router.route(problem)
        XCTAssertEqual(try outcome(result, 1).status, .complete)
        XCTAssertEqual(try outcome(result, 2).status, .complete)
        XCTAssertEqual(result.vias.count, 0)
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
    }

    func testEachClassKeepsItsOwnClearance() throws {
        var board = board()
        board.options.vias = .none
        // A 0.5 mm Power net keeping 0.3 mm, between two Default nets.
        board.pad(2, 10, 1, 1, net: 1)
        board.pad(18, 10, 1, 1, net: 1)
        board.width[1] = 0.5
        board.clearance[1] = 0.3
        board.pad(2, 8.6, 0.6, 0.6, net: 2)
        board.pad(18, 8.6, 0.6, 0.6, net: 2)
        board.pad(2, 7.9, 0.4, 0.4, net: 3)
        board.pad(18, 7.9, 0.4, 0.4, net: 3)
        let problem = board.problem()
        let result = Router.route(problem)
        for net in 1...3 { XCTAssertEqual(try outcome(result, net).status, .complete, "net \(net)") }
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
        let shapes = RoutingCheck.shapes(result)
        func gap(_ a: Int, _ b: Int) -> Double {
            var best = Double.infinity
            for x in shapes where x.net == a { for y in shapes where y.net == b { best = min(best, x.shape.gap(to: y.shape)) } }
            return best
        }
        XCTAssertGreaterThanOrEqual(gap(1, 2), 0.3 - 1e-4, "the Power net's clearance")
        XCTAssertGreaterThanOrEqual(gap(2, 3), 0.1 - 1e-4)
        // The grid holds a Default net to the Default spacing from another
        // Default net's copper, and to the Power spacing from the Power net's:
        // 0.4 mm from net 3's pad edge is free for net 2 (it needs 0.1 +
        // 0.127), 0.4 mm from net 1's is not (0.3 + 0.127).
        let grid = RoutingGrid(problem: problem)
        let p2 = grid.profile(of: 2)
        let net3Pad = try XCTUnwrap(board.pads.first { $0.net == 3 }).rect
        let net1Pad = try XCTUnwrap(board.pads.first { $0.net == 1 }).rect
        XCTAssertTrue(grid.isPassable(grid.nearestCell(CGPoint(x: net3Pad.midX, y: net3Pad.minY - 0.4)), layer: 0,
                                      net: 2, profile: p2))
        XCTAssertFalse(grid.isPassable(grid.nearestCell(CGPoint(x: net1Pad.maxX + 0.4, y: net1Pad.midY)), layer: 0,
                                       net: 2, profile: p2))
    }

    func testPlaneNetFansOutByVias() throws {
        var board = board(layers: [.top, .inner(2), .inner(3), .bottom])
        board.options.layers = [.inner(2): .init(enabled: false)]
        board.planes = [.inner(2): 1]
        board.planeLayer[1] = .inner(2)
        board.pad(4, 4, 1, 1, net: 1)
        board.pad(16, 4, 1, 1, net: 1)
        board.pad(10, 16, 1, 1, net: 1)
        let problem = board.problem()
        XCTAssertEqual(problem.routable, [.top, .inner(3), .bottom])
        let result = Router.route(problem)
        let net = try outcome(result, 1)
        XCTAssertEqual(net.status, .complete)
        XCTAssertEqual(result.vias.count, 3, "one per surface-only component")
        for trace in result.traces {
            let length = zip(trace.points, trace.points.dropFirst()).reduce(0) { $0 + hypot($1.1.x - $1.0.x, $1.1.y - $1.0.y) }
            XCTAssertLessThan(length, 1.5, "only a short tie from each pad to its via")
        }
        // Each via meets its pad's copper, and a trace touches it: the DRC's
        // "connected" rule for a via.
        for via in result.vias {
            XCTAssertTrue(board.pads.contains { CopperGeometry.pointDistance(via.centre, $0.rect) <= via.diameter / 2 })
            XCTAssertTrue(result.traces.contains { $0.points.contains { p in hypot(p.x - via.centre.x, p.y - via.centre.y) < 1e-6 } })
        }
        XCTAssertEqual(RoutingCheck.violations(result, problem), [])
    }

    func testImpossibleNetIsReportedNotSearched() throws {
        var board = board()
        board.pad(5, 10, 1, 1, net: 1)
        board.pad(15, 10, 1, 1, net: 1)
        board.impossible[1] = "Pad P2 is on Bottom; N1 is limited to Top"
        let result = Router.route(board.problem())
        XCTAssertEqual(try outcome(result, 1).status, .impossible("Pad P2 is on Bottom; N1 is limited to Top"))
        XCTAssertTrue(result.traces.isEmpty)
        XCTAssertEqual(result.stats.expansions, 0)
    }

    private func busyBoard() -> RoutingTestBoard {
        var board = board()
        for i in 0..<6 {
            board.pad(2, 3 + Double(i) * 2.5, 1, 1, net: i + 1)
            board.pad(18, 3 + Double(5 - i) * 2.5, 1, 1, net: i + 1)
        }
        board.pad(10, 10, 1, 1, net: 7)
        board.pad(10, 2, 1, 1, net: 7)
        board.pad(10, 18, 1, 1, net: 7)
        return board
    }

    func testSameProblemSameResultInAnyNetOrder() throws {
        let problem = busyBoard().problem()
        let once = Router.route(problem)
        var twice = Router.route(problem)
        XCTAssertEqual(once.traces, twice.traces)
        XCTAssertEqual(once.vias, twice.vias)
        XCTAssertEqual(once.nets, twice.nets)
        var reversed = problem
        reversed.nets.reverse()
        twice = Router.route(reversed)
        XCTAssertEqual(once.traces, twice.traces)
        XCTAssertEqual(once.vias, twice.vias)
        XCTAssertEqual(once.nets, twice.nets)
        XCTAssertGreaterThan(once.stats.routed, 0)
        XCTAssertEqual(RoutingCheck.violations(once, problem), [])
    }
}
