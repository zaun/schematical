import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Coupled differential pairs (plan-38-J Phases 2–3).
final class RoutingPairTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    private let pairID = UUID()

    /// Nets 1 (P) and 2 (N): small Top pads 1 mm apart at x = 5 and x = 25.
    private func board(width: Double = 0.254, gap: Double = 0.2) -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 30, height: 20)
        board.options.grid = 0.127
        board.pad(5, 9.5, 0.4, 0.4, net: 1)
        board.pad(25, 9.5, 0.4, 0.4, net: 1)
        board.pad(5, 10.5, 0.4, 0.4, net: 2)
        board.pad(25, 10.5, 0.4, 0.4, net: 2)
        board.width[1] = width
        board.width[2] = width
        board.groupOf[1] = pairID
        board.groupOf[2] = pairID
        board.groups = [RoutingGroup(id: pairID, name: "USB", mode: .diffPair, nets: [1, 2], tolerance: 0.25,
                                     gap: gap, maxUncoupled: 5, maxMeander: 2)]
        return board
    }

    private func pair(_ board: RoutingTestBoard) -> (Result<PairRoute, PairFailure>, RoutingProblem) {
        let problem = board.problem()
        return (RoutingPair.route(problem.groups[0], problem: problem), problem)
    }

    /// The edge-to-edge gap from points of `a` to the nearest of `b`, every
    /// 0.05 mm, where both are on one layer, for x within `xs`.
    private func gaps(_ a: [RoutedTrace], _ b: [RoutedTrace], xs: ClosedRange<CGFloat>) -> [Double] {
        var result: [Double] = []
        for trace in a {
            for (u, v) in zip(trace.points, trace.points.dropFirst()) {
                let length = hypot(v.x - u.x, v.y - u.y)
                let n = max(1, Int(length / 0.05))
                for s in 0...n {
                    let t = Double(s) / Double(n)
                    let point = CGPoint(x: u.x + (v.x - u.x) * t, y: u.y + (v.y - u.y) * t)
                    guard xs.contains(point.x) else { continue }
                    let others = b.filter { $0.layer == trace.layer }.flatMap { o in zip(o.points, o.points.dropFirst()).map { (o.width, $0.0, $0.1) } }
                    guard let nearest = others.map({ SchematicDocument.distance(from: point, toSegment: $0.1, $0.2) - $0.0 / 2 - trace.width / 2 }).min()
                    else { continue }
                    result.append(nearest)
                }
            }
        }
        return result
    }

    func testAnOpenPairRunsParallelAtItsGap() throws {
        let (result, problem) = pair(board())
        let route = try result.get()
        XCTAssertTrue(route.vias.isEmpty)
        let g = gaps(route.p, route.n, xs: 9...21)
        XCTAssertFalse(g.isEmpty)
        XCTAssertEqual(g.min()!, 0.2, accuracy: 0.001)
        XCTAssertEqual(g.max()!, 0.2, accuracy: 0.001)
        XCTAssertLessThanOrEqual(route.uncoupled, 5)
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: route.p + route.n, vias: route.vias), [])
    }

    func testABlockingTraceMakesAFlaredViaPair() throws {
        var board = board()
        board.trace([p(15, 0.6), p(15, 19.4)], net: 9)
        let (result, problem) = pair(board)
        let route = try result.get()
        XCTAssertEqual(route.vias.count, 4, "two layer changes, two vias each")
        for k in stride(from: 0, to: route.vias.count, by: 2) {
            let a = route.vias[k], b = route.vias[k + 1]
            XCTAssertNotEqual(a.net, b.net)
            XCTAssertGreaterThanOrEqual(hypot(a.centre.x - b.centre.x, a.centre.y - b.centre.y) - a.diameter / 2 - b.diameter / 2,
                                        problem.rules.minClearance - 1e-6, "the two vias clear each other")
        }
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: route.p + route.n, vias: route.vias), [])
    }

    func testAWidePairKeepsItsOwnClearanceFromANeighbour() throws {
        var board = board(width: 0.5)
        board.clearance[1] = 0.3
        board.clearance[2] = 0.3
        board.pad(5, 13, 0.4, 0.4, net: 3)
        board.pad(25, 13, 0.4, 0.4, net: 3)
        let problem = board.problem()
        let result = Router.route(problem)
        let pairCopper = result.traces.filter { $0.net == 1 || $0.net == 2 }
        let neighbour = result.traces.filter { $0.net == 3 }
        XCTAssertFalse(pairCopper.isEmpty)
        XCTAssertFalse(neighbour.isEmpty)
        let shapes = RoutingCheck.shapes(result)
        let gap = shapes.filter { $0.net == 3 }.flatMap { x in shapes.filter { $0.net != 3 }.map { x.shape.gap(to: $0.shape) } }.min()!
        XCTAssertGreaterThanOrEqual(gap, 0.3 - 1e-4)
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: result.traces, vias: result.vias), [])
    }

    func testAnOnlyTopPairNeverUsesAVia() throws {
        var board = board()
        board.allowed[1] = [.top]
        board.allowed[2] = [.top]
        let route = try pair(board).0.get()
        XCTAssertTrue(route.vias.isEmpty)
        XCTAssertTrue((route.p + route.n).allSatisfy { $0.layer == .top })
        board.trace([p(15, 0.6), p(15, 19.4)], net: 9)
        guard case .failure = pair(board).0 else { return XCTFail("walled off on Top: no coupled route") }
    }

    func testAChannelTooNarrowForThePairIsUncoupled() throws {
        var board = board()
        board.options.vias = .none
        // A 0.5 mm gap in a wall across Top: one trace fits, not 2w + g = 0.708.
        let half = 0.25 + 0.1 + 0.127
        board.trace([p(15, 0.6), p(15, 10 - half)], net: 9)
        board.trace([p(15, 10 + half), p(15, 19.4)], net: 9)
        guard case let .failure(failure) = pair(board).0 else { return XCTFail("the pair can't fit") }
        XCTAssertFalse(failure.reason.isEmpty)
    }

    // MARK: Phase 3

    private func group(_ result: RoutingResult) throws -> GroupOutcome {
        try XCTUnwrap(result.groups.first { $0.group == pairID })
    }

    func testSkewIsTunedOnTheOuterSide() throws {
        var board = board()
        // N's far pad 2 mm further out sideways: N's breakout, and N, are longer.
        board.pads[3].rect.origin.y += 2
        let problem = board.problem()
        let result = Router.route(problem)
        let outcome = try group(result)
        guard case .coupled = outcome.status else { return XCTFail("\(outcome.status)") }
        XCTAssertLessThanOrEqual(outcome.spread, 0.25 + 1e-6, "\(outcome.lengths)")
        // Over the coupled middle (x 6–23), P (below) never goes above its
        // lane and N (above) never below its: a bump only ever stands away
        // from the partner. And one of them was bumped.
        func lane(_ net: Int) -> CGFloat {
            var y: CGFloat = 0, longest: CGFloat = 0
            for trace in result.traces where trace.net == net {
                for (u, v) in zip(trace.points, trace.points.dropFirst()) where hypot(v.x - u.x, v.y - u.y) > longest {
                    longest = hypot(v.x - u.x, v.y - u.y)
                    y = u.y
                }
            }
            return y
        }
        let pLane = lane(1), nLane = lane(2)
        XCTAssertLessThan(pLane, nLane)
        let middle = { (net: Int) in
            result.traces.filter { $0.net == net }.flatMap(\.points).filter { (6...23).contains($0.x) }
        }
        XCTAssertTrue(middle(1).allSatisfy { $0.y <= pLane + 1e-6 }, "no P bump toward N")
        XCTAssertTrue(middle(2).allSatisfy { $0.y >= nLane - 1e-6 }, "no N bump toward P")
        XCTAssertTrue(middle(1).contains { $0.y < pLane - 0.1 } || middle(2).contains { $0.y > nLane + 0.1 }, "tuned")
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: result.traces, vias: result.vias), [])
    }

    func testARippedPairIsRoutedAgainCoupled() throws {
        var board = board()
        // Net 3 crosses the pair's straight way, on Top only: it must rip the
        // pair, which then crosses it on Bottom.
        board.pad(15, 3, 0.4, 0.4, net: 3)
        board.pad(15, 17, 0.4, 0.4, net: 3)
        board.allowed[3] = [.top]
        // Walls of net 9 on Top from each edge to the pair's pads, so net 3
        // can't go round the pair's ends.
        board.trace([p(0.6, 10), p(4.6, 10)], net: 9)
        board.trace([p(25.4, 10), p(29.4, 10)], net: 9)
        board.options.strategy = .balanced
        let problem = board.problem()
        let result = Router.route(problem)
        XCTAssertEqual(result.stats.routed, result.stats.connections, "\(result.nets)")
        XCTAssertGreaterThan(result.stats.rips, 0)
        guard case .coupled = try group(result).status else { return XCTFail("\(try group(result).status)") }
        XCTAssertEqual(result.vias.filter { $0.net == 1 || $0.net == 2 }.count, 4)
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: result.traces, vias: result.vias), [])
    }

    func testAPairAndTenNetsAllComplete() throws {
        var board = board()
        board.boardHeight = 40
        for i in 0..<10 {
            let y = 14 + Double(i) * 2.5
            board.pad(3, y, 0.6, 0.6, net: 10 + i)
            board.pad(27, 14 + 36.5 - y, 0.6, 0.6, net: 10 + i)
        }
        let problem = board.problem()
        let result = Router.route(problem)
        XCTAssertEqual(result.stats.routed, result.stats.connections, "\(result.nets.filter { $0.status != .complete })")
        XCTAssertEqual(RoutingVerifier.verify(problem, traces: result.traces, vias: result.vias), [])
    }
}
