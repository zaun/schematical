import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The router and cutouts (plan-65-D): the grid, the verifier, the cleanup
/// and the builder read the board edge.
@MainActor
final class CutoutRoutingTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    private func square(_ x: Double, _ y: Double, _ side: Double) -> [CGPoint] {
        BoardCutout.Shape.rounded(rect: CGRect(x: x, y: y, width: side, height: side), radius: 0).loop
    }

    private func board() -> RoutingTestBoard {
        var board = RoutingTestBoard()
        board.options.grid = 0.1
        board.width[1] = 0.25
        board.clearance[1] = 0.125
        return board
    }

    private func cell(_ grid: RoutingGrid, _ x: Double, _ y: Double) -> Int { grid.nearestCell(CGPoint(x: x, y: y)) }

    func testCellsInsideAndNearACutoutAreBlockedOnEveryLayer() {
        var board = board()
        board.pad(2, 2, 1, 1, net: 1)
        board.pad(18, 18, 1, 1, net: 1)
        board.cutouts = [square(7, 7, 6)]
        let grid = RoutingGrid(problem: board.problem())
        let profile = grid.profile(of: 1)
        // minCopperToEdge (customDefault) + half the 0.25 trace.
        let reach = board.rules.minCopperToEdge + 0.125
        for layer in 0..<2 {
            XCTAssertFalse(grid.isPassable(cell(grid, 10, 10), layer: layer, net: 1, profile: profile), "inside, layer \(layer)")
        }
        XCTAssertFalse(grid.isPassable(cell(grid, 6.8, 10), layer: 0, net: 1, profile: profile), "0.2 mm outside the edge")
        XCTAssertTrue(grid.isPassable(cell(grid, 7 - reach - 0.3, 10), layer: 0, net: 1, profile: profile), "clear of the band")
    }

    func testAClockwiseAndACounterClockwiseCutoutStampTheSameCells() {
        var board = board()
        board.pad(2, 2, 1, 1, net: 1)
        board.pad(18, 18, 1, 1, net: 1)
        board.cutouts = [square(7, 7, 6)]
        let cw = RoutingGrid(problem: board.problem())
        board.cutouts = [square(7, 7, 6).reversed()]
        let ccw = RoutingGrid(problem: board.problem())
        let profile = cw.profile(of: 1)
        for x in stride(from: 1.0, through: 19.0, by: 0.5) {
            for y in stride(from: 1.0, through: 19.0, by: 0.5) {
                XCTAssertEqual(cw.isPassable(cell(cw, x, y), layer: 0, net: 1, profile: profile),
                               ccw.isPassable(cell(ccw, x, y), layer: 0, net: 1, profile: profile), "(\(x), \(y))")
            }
        }
    }

    func testTheRouterGoesRoundACutout() throws {
        var board = board()
        board.pad(2, 10, 1, 1, net: 1, layers: [.top, .bottom])
        board.pad(18, 10, 1, 1, net: 1, layers: [.top, .bottom])
        board.cutouts = [square(7, 7, 6)]
        let problem = board.problem()
        let result = Router.route(problem)
        XCTAssertEqual(result.stats.routed, result.stats.connections, "\(result.nets)")
        XCTAssertGreaterThan(result.stats.connections, 0)
        let edge = problem.edge
        let reach = problem.rules.minCopperToEdge
        for trace in result.traces {
            for point in trace.points { XCTAssertTrue(edge.contains(point), "\(point) is on material") }
            for (a, b) in zip(trace.points, trace.points.dropFirst()) {
                for loop in edge.cutouts {
                    for (c, d) in zip(loop, loop.dropFirst() + loop.prefix(1)) {
                        let shape = CopperShape(core: .segment(a, b), radius: trace.width / 2, layers: nil, node: 0)
                        let edgeShape = CopperShape(core: .segment(c, d), radius: 0, layers: nil, node: -1)
                        XCTAssertGreaterThanOrEqual(shape.gap(to: edgeShape), reach - 1e-4)
                    }
                }
            }
        }
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: result.traces, vias: result.vias).filter { $0.kind == .edge }.count, 0)
    }

    func testTheVerifierFlagsATraceThroughACutout() {
        var board = board()
        board.pad(2, 10, 1, 1, net: 1)
        board.pad(18, 10, 1, 1, net: 1)
        board.cutouts = [square(7, 7, 6)]
        let problem = board.problem()
        let through = RoutedTrace(layer: .top, width: 0.25, points: [p(2, 10), p(18, 10)], net: 1)
        let kinds = RoutingVerifier.verify(problem, traces: [through], vias: []).map(\.kind)
        XCTAssertTrue(kinds.contains(.edge), "\(kinds)")
        let around = RoutedTrace(layer: .top, width: 0.25, points: [p(2, 10), p(2, 3), p(18, 3), p(18, 10)], net: 1)
        XCTAssertFalse(RoutingVerifier.verify(problem, traces: [around], vias: []).map(\.kind).contains(.edge))
    }

    func testTheBuilderCarriesTheCutoutsAndTheirEdges() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: CGRect(x: 20, y: 10, width: 8, height: 6), radius: 0)))
        let problem = doc.routingProblem(options: .standard, rules: .customDefault)
        XCTAssertEqual(problem.cutouts.count, 1)
        XCTAssertEqual(problem.cutouts.first, try XCTUnwrap(doc.cutouts.first { $0.id == id }).shape.loop)
        let outlineObstacles = problem.obstacles.filter { $0.kind == .outline }
        XCTAssertEqual(outlineObstacles.count, 4 + 4)
    }

    func testCleanupDoesNotShortcutOverAThinCutout() {
        var board = board()
        board.pad(3, 10, 1, 1, net: 1)
        board.pad(17, 10, 1, 1, net: 1)
        // 0.8 mm wide and 6 mm high, off the middle of the straight line between the pads: the shortcut's midpoint (10, 10) misses it.
        board.cutouts = [BoardCutout.Shape.rounded(rect: CGRect(x: 6, y: 7, width: 0.8, height: 6), radius: 0).loop]
        let problem = board.problem()
        let bent = RoutedTrace(layer: .top, width: 0.254, points: [p(3, 10), p(3, 4), p(17, 4), p(17, 10)], net: 1)
        var cleanup = RoutingCleanup(problem: problem, traces: [bent], vias: [], traceEntry: [0], viaEntry: [])
        cleanup.run()
        let loop = problem.cutouts[0]
        for trace in cleanup.result.traces {
            for (a, b) in zip(trace.points, trace.points.dropFirst()) {
                for (c, d) in zip(loop, loop.dropFirst() + loop.prefix(1)) {
                    XCTAssertFalse(CopperGeometry.segmentsCross(a, b, c, d), "\(a)-\(b) crosses the cutout")
                }
            }
        }
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: cleanup.result.traces, vias: []).filter { $0.kind == .edge }.count, 0)
    }
}
