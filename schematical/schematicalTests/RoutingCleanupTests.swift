import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Cleaning routed copper (plan-38-H Phases 2–3): straightening, chamfers,
/// via hops; never longer, never more vias, never illegal.
final class RoutingCleanupTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func trace(_ points: [CGPoint], layer: CopperLayer = .top, net: Int = 1) -> RoutedTrace {
        RoutedTrace(layer: layer, width: 0.254, points: points, net: net)
    }

    private func cleaned(_ board: RoutingTestBoard, _ traces: [RoutedTrace], vias: [RoutedVia] = [])
        -> (traces: [RoutedTrace], vias: [RoutedVia], problem: RoutingProblem) {
        let problem = board.problem()
        var cleanup = RoutingCleanup(problem: problem, traces: traces, vias: vias,
                                     traceEntry: Array(traces.indices), viaEntry: Array(vias.indices))
        cleanup.run()
        let result = cleanup.result
        // Never longer, never more vias, always legal.
        XCTAssertLessThanOrEqual(result.traces.reduce(0) { $0 + RoutingCleanup.length($1.points) },
                                 traces.reduce(0) { $0 + RoutingCleanup.length($1.points) } + 1e-9)
        XCTAssertLessThanOrEqual(result.vias.count, vias.count)
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: result.traces, vias: result.vias)
            .filter { $0.kind != .unconnected }, [])
        return (result.traces, result.vias, problem)
    }

    private func pads(_ a: CGPoint, _ b: CGPoint, layers: Set<CopperLayer>? = [.top]) -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20)
        board.options.grid = 0.1
        board.pad(a.x, a.y, 1, 1, net: 1, layers: layers)
        board.pad(b.x, b.y, 1, 1, net: 1, layers: layers)
        return board
    }

    private func angles(_ points: [CGPoint]) -> Set<Int> {
        Set(zip(points, points.dropFirst()).map { a, b in
            Int((atan2(Double(b.y - a.y), Double(b.x - a.x)) * 180 / .pi).rounded())
        })
    }

    // MARK: Phase 2

    func testAStaircaseBecomesOne45DegreeSegment() {
        let start = p(5, 5)
        var stair = [start]
        for i in 0..<10 {
            stair.append(p(5 + Double(i + 1) * 0.1, 5 + Double(i) * 0.1))
            stair.append(p(5 + Double(i + 1) * 0.1, 5 + Double(i + 1) * 0.1))
        }
        let board = pads(start, stair.last!)
        let result = cleaned(board, [trace(stair)])
        XCTAssertEqual(result.traces.count, 1)
        XCTAssertEqual(result.traces[0].points.count, 2, "\(result.traces[0].points)")
        XCTAssertEqual(angles(result.traces[0].points), [45])
    }

    func testAnLAroundAnObstacleStaysClearAndIsChamferedWhereLegal() {
        var board = pads(p(3, 3), p(10, 10))
        // Net 2's pad sits in the corner's way: the L must keep round it.
        board.pad(5, 7, 3, 3, net: 2)
        board.unrouted = [2]
        let raw = [p(3, 3), p(3, 10), p(10, 10)]
        let result = cleaned(board, [trace(raw)])
        let points = result.traces[0].points
        XCTAssertEqual(points.first, p(3, 3))
        XCTAssertEqual(points.last, p(10, 10))
        XCTAssertTrue(angles(points).contains(45) || angles(points).contains(-45) || angles(points).contains(135),
                      "the corner got a chamfer: \(points)")
    }

    func testOrthogonalHasNo45DegreeSegment() {
        let start = p(5, 5)
        var stair = [start]
        for i in 0..<10 {
            stair.append(p(5 + Double(i + 1) * 0.5, 5 + Double(i) * 0.5))
            stair.append(p(5 + Double(i + 1) * 0.5, 5 + Double(i + 1) * 0.5))
        }
        var board = pads(start, stair.last!)
        board.options.angle = .orthogonal
        let result = cleaned(board, [trace(stair)])
        for t in result.traces {
            XCTAssertTrue(angles(t.points).isSubset(of: [0, 90, -90, 180, -180]), "\(t.points)")
        }
    }

    func testAShortcutTooCloseToCopperIsRefused() {
        // The straight way from (3, 3) to (10, 10) passes 0.01 mm inside
        // net 2's clearance; the raw route goes round.
        // Net 2's 1 mm pad below the diagonal y = x, its top-left corner
        // (cx − 0.5, cy + 0.5) at 0.127 + 0.09 mm from the centreline: the
        // diagonal's copper would pass 0.09 mm from it, 0.01 inside the
        // 0.1 mm clearance. 0.02 mm further away it would be legal.
        func board(_ shift: Double) -> RoutingTestBoard {
            var board = pads(p(3, 3), p(10, 10))
            board.unrouted = [2]
            let cy = 6.5, cx = cy + 1 + (0.127 + 0.09 + shift) * 2.squareRoot()
            board.pad(cx, cy, 1, 1, net: 2)
            return board
        }
        let diagonal = [p(3, 3), p(10, 10)]
        let tight = board(0).problem(), loose = board(0.02).problem()
        let probe = { (problem: RoutingProblem) in
            RoutingCleanup(problem: problem, traces: [], vias: [], traceEntry: [], viaEntry: [])
                .isLegal(diagonal, layer: .top, width: 0.254, net: 1)
        }
        XCTAssertFalse(probe(tight))
        XCTAssertTrue(probe(loose))
        let raw = [p(3, 3), p(3, 10), p(10, 10)]
        XCTAssertNotEqual(cleaned(board(0), [trace(raw)]).traces[0].points, diagonal, "the shortcut is refused")
        XCTAssertEqual(cleaned(board(0.02), [trace(raw)]).traces[0].points, diagonal, "and taken when legal")
    }

    // MARK: Phase 3

    private func hop(blocked: Bool) -> (traces: [RoutedTrace], vias: [RoutedVia], problem: RoutingProblem) {
        var board = pads(p(3, 10), p(17, 10))
        if blocked {
            board.trace([p(9, 5), p(9, 15)], net: 2)
            board.unrouted = [2]
        }
        let a = p(8, 10), b = p(10, 10)
        let traces = [trace([p(3, 10), a]), trace([a, b], layer: .bottom), trace([b, p(17, 10)])]
        let vias = [RoutedVia(centre: a, drill: 0.4, diameter: 0.8, net: 1), RoutedVia(centre: b, drill: 0.4, diameter: 0.8, net: 1)]
        return cleaned(board, traces, vias: vias)
    }

    func testAHopOverOpenBoardGoesBackOnTop() {
        let result = hop(blocked: false)
        XCTAssertEqual(result.vias, [])
        XCTAssertTrue(result.traces.allSatisfy { $0.layer == .top })
    }

    func testAHopUnderAForeignTraceStays() {
        let result = hop(blocked: true)
        XCTAssertEqual(result.vias.count, 2)
        XCTAssertTrue(result.traces.contains { $0.layer == .bottom })
    }
}
