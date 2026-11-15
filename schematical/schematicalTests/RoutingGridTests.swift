import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The routing grid's stamping (plan-38-E Phase 1): exact bloats, own-net
/// passability, the edge and the outside.
final class RoutingGridTests: XCTestCase {
    /// Net 2's trace profile: 0.25 mm wide, 0.125 clearance, so a trace
    /// centre keeps 0.125 + 0.125 = 0.25 mm from foreign copper; the numbers
    /// are exact in binary.
    private func board() -> RoutingTestBoard {
        var board = RoutingTestBoard()
        board.options.grid = 0.05
        board.width[1] = 0.25
        board.width[2] = 0.25
        board.clearance[1] = 0.125
        board.clearance[2] = 0.125
        return board
    }

    private func cell(_ grid: RoutingGrid, _ x: Double, _ y: Double) -> Int {
        grid.nearestCell(CGPoint(x: x, y: y))
    }

    func testExactBloatIsFreeAndAHundredthCloserIsBlocked() {
        var board = board()
        // Net 1's pad ends at x = 4.75; net 2's own pad is far away.
        board.pad(4.5, 10, 0.5, 1, net: 1)
        board.pad(15, 15, 1, 1, net: 2)
        var grid = RoutingGrid(problem: board.problem())
        let p2 = grid.profile(of: 2)
        XCTAssertTrue(grid.isPassable(cell(grid, 5.0, 10), layer: 0, net: 2, profile: p2), "exactly 0.25 mm away")
        XCTAssertFalse(grid.isPassable(cell(grid, 4.95, 10), layer: 0, net: 2, profile: p2))

        board.pads[0].rect.origin.x += 0.01
        grid = RoutingGrid(problem: board.problem())
        XCTAssertFalse(grid.isPassable(cell(grid, 5.0, 10), layer: 0, net: 2, profile: p2), "0.24 mm away")
        XCTAssertTrue(grid.isPassable(cell(grid, 5.05, 10), layer: 0, net: 2, profile: p2))
        XCTAssertTrue(grid.isPassable(cell(grid, 5.0, 10), layer: 1, net: 2, profile: p2), "a Top pad leaves Bottom")
    }

    func testOwnPadIsPassableForItsNetOnly() {
        var board = board()
        board.pad(10, 10, 1, 1, net: 1)
        board.pad(15, 15, 1, 1, net: 2)
        let grid = RoutingGrid(problem: board.problem())
        let next = cell(grid, 10.6, 10)
        XCTAssertTrue(grid.isPassable(next, layer: 0, net: 1, profile: grid.profile(of: 1)))
        XCTAssertFalse(grid.isPassable(next, layer: 0, net: 2, profile: grid.profile(of: 2)))
        XCTAssertTrue(grid.isPassable(cell(grid, 10, 10), layer: 0, net: 1, profile: grid.profile(of: 1)), "inside")
    }

    func testOutsideAndNearTheEdgeAreBlocked() {
        var board = board()
        board.pad(10, 10, 1, 1, net: 2)
        let grid = RoutingGrid(problem: board.problem())
        let p2 = grid.profile(of: 2)
        // Edge 0.25 + half width 0.125: centres closer than 0.375 are blocked.
        XCTAssertEqual(board.rules.minCopperToEdge, 0.25)
        XCTAssertFalse(grid.isPassable(cell(grid, 0, 10), layer: 0, net: 2, profile: p2), "on the edge")
        XCTAssertFalse(grid.isPassable(cell(grid, 0.35, 10), layer: 0, net: 2, profile: p2))
        XCTAssertTrue(grid.isPassable(cell(grid, 0.4, 10), layer: 0, net: 2, profile: p2))
        XCTAssertFalse(grid.isPassable(cell(grid, 10, 19.65), layer: 1, net: 2, profile: p2))
        // A via (radius 0.4) keeps 0.25 + 0.4 from the edge.
        XCTAssertFalse(grid.viaFits(cell(grid, 0.6, 10), net: 2, profile: p2))
        XCTAssertTrue(grid.viaFits(cell(grid, 0.7, 10), net: 2, profile: p2))

        var notch = RoutingTestBoard()
        notch.options.grid = 0.05
        notch.pad(5, 5, 1, 1, net: 1)
        var problem = notch.problem()
        // An L-shaped board: the top-right quarter is outside.
        problem.outline = [CGPoint(x: 0, y: 0), CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10),
                           CGPoint(x: 20, y: 20), CGPoint(x: 0, y: 20)]
        let shaped = RoutingGrid(problem: problem)
        let p1 = shaped.profile(of: 1)
        XCTAssertFalse(shaped.isPassable(shaped.nearestCell(CGPoint(x: 15, y: 5)), layer: 0, net: 1, profile: p1))
        XCTAssertTrue(shaped.isPassable(shaped.nearestCell(CGPoint(x: 15, y: 15)), layer: 0, net: 1, profile: p1))
        XCTAssertTrue(shaped.isPassable(shaped.nearestCell(CGPoint(x: 5, y: 15)), layer: 0, net: 1, profile: p1))
    }

    /// A finger at the bottom edge: its net reaches it through its inner
    /// part; the part of it inside the edge band stays blocked, because the
    /// DRC flags trace copper there (only the finger itself is exempt).
    func testFingerAtTheEdgeIsReachedFromInside() {
        var board = board()
        board.pad(10, 19, 1, 2, net: 2, exempt: true)
        let grid = RoutingGrid(problem: board.problem())
        let p2 = grid.profile(of: 2)
        XCTAssertTrue(grid.isPassable(cell(grid, 10, 18.5), layer: 0, net: 2, profile: p2), "inner part")
        XCTAssertFalse(grid.isPassable(cell(grid, 10, 19.8), layer: 0, net: 2, profile: p2), "in the edge band")
        XCTAssertFalse(grid.isPassable(cell(grid, 10, 18.5), layer: 0, net: 3, profile: p2), "another net")
    }

    func testTwoNetsOverlappingBloatsBlock() {
        var board = board()
        board.pad(9.6, 10, 0.5, 1, net: 1)
        board.pad(10.4, 10, 0.5, 1, net: 3)
        board.pad(15, 15, 1, 1, net: 2)
        let grid = RoutingGrid(problem: board.problem())
        let middle = cell(grid, 10, 10)
        XCTAssertEqual(grid.traceValue(middle, layer: 0, profile: grid.profile(of: 2)), RoutingGrid.blocked)
        XCTAssertFalse(grid.isPassable(middle, layer: 0, net: 1, profile: grid.profile(of: 1)))
        XCTAssertFalse(grid.isPassable(middle, layer: 0, net: 3, profile: grid.profile(of: 3)))
    }

    func testHolesBlockEveryLayerForEveryNet() {
        var board = board()
        board.pad(15, 15, 1, 1, net: 2)
        board.hole(5, 5, drill: 1)
        let grid = RoutingGrid(problem: board.problem())
        let p2 = grid.profile(of: 2)
        for layer in 0..<2 {
            XCTAssertFalse(grid.isPassable(cell(grid, 5.7, 5), layer: layer, net: 2, profile: p2))
            XCTAssertTrue(grid.isPassable(cell(grid, 5.8, 5), layer: layer, net: 2, profile: p2))
        }
    }
}
