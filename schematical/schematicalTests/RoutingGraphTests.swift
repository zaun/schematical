import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The cleanup's net graph (plan-38-H Phase 1): anchors, chains, stubs,
/// ties, kept copper.
final class RoutingGraphTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func board() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20)
        board.options.grid = 0.1
        board.pad(2, 10, 1, 1, net: 1)
        board.pad(18, 10, 1, 1, net: 1)
        board.pad(10, 16, 1, 1, net: 1)
        return board
    }

    private func trace(_ points: [CGPoint], net: Int = 1, layer: CopperLayer = .top) -> RoutedTrace {
        RoutedTrace(layer: layer, width: 0.254, points: points, net: net)
    }

    func testATJunctionInsertsOneAnchoredVertex() {
        let problem = board().problem()
        var cleanup = RoutingCleanup(problem: problem, traces: [trace([p(2, 10), p(18, 10)]), trace([p(10, 10), p(10, 16)])],
                                     vias: [], traceEntry: [0, 1], viaEntry: [])
        let anchors = cleanup.anchors(of: 0)
        XCTAssertEqual(cleanup.traces[0].points, [p(2, 10), p(10, 10), p(18, 10)])
        XCTAssertEqual(anchors, [0, 1, 2])
        XCTAssertEqual(cleanup.anchors(of: 1), [0, 1], "the branch's own ends")
    }

    func testChainsStopAtAnchors() {
        let problem = board().problem()
        // A staircase from the left pad to the right one, with the branch
        // hanging on its middle vertex (10, 11).
        let stair = [p(2, 10), p(4, 10), p(5, 11), p(10, 11), p(15, 11), p(16, 10), p(18, 10)]
        var cleanup = RoutingCleanup(problem: problem, traces: [trace(stair), trace([p(10, 11), p(10, 16)])],
                                     vias: [], traceEntry: [0, 1], viaEntry: [])
        cleanup.run()
        let main = cleanup.result.traces[0].points
        XCTAssertTrue(main.contains(p(10, 11)), "the junction never moves: \(main)")
        XCTAssertEqual(main.first, p(2, 10))
        XCTAssertEqual(main.last, p(18, 10))
        XCTAssertLessThanOrEqual(RoutingCleanup.length(main), RoutingCleanup.length(stair) + 1e-9)
    }

    func testAStubIsRemoved() {
        let problem = board().problem()
        // A trace from the left pad into open board, joining nothing there.
        var cleanup = RoutingCleanup(problem: problem, traces: [trace([p(2, 10), p(18, 10)]), trace([p(2, 10), p(2, 5)])],
                                     vias: [], traceEntry: [0, 1], viaEntry: [])
        cleanup.run()
        XCTAssertEqual(cleanup.result.traces.count, 1)
        XCTAssertEqual(cleanup.result.traces[0].points, [p(2, 10), p(18, 10)])
    }

    func testTiesLandOnThePadCentre() throws {
        var board = board()
        board.pads.removeLast()
        let problem = board.problem()
        let result = Router.route(problem)
        XCTAssertEqual(result.stats.routed, 1)
        let ends = result.traces.flatMap { [$0.points.first!, $0.points.last!] }
        XCTAssertTrue(ends.contains(p(2, 10)))
        XCTAssertTrue(ends.contains(p(18, 10)))
    }

    func testKeptTracesAreUnchanged() throws {
        var board = board()
        board.pads.removeLast()
        board.trace([p(5, 5), p(15, 5)], net: 2)
        board.unrouted = [2]
        let problem = board.problem()
        let kept = problem.obstacles.filter { $0.kind == .trace }
        let result = Router.route(problem)
        XCTAssertEqual(problem.obstacles.filter { $0.kind == .trace }, kept)
        XCTAssertFalse(result.traces.contains { $0.net == 2 }, "kept copper is not in the result")
    }
}
