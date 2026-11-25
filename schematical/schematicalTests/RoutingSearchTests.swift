import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// One connection's search (plan-38-E Phase 2): optimality, detours, vias,
/// layer rules and preferences, angles, via fitting and cancellation.
final class RoutingSearchTests: XCTestCase {
    struct Run {
        var grid: RoutingGrid
        var result: Result<[Int], SearchFailure>
        var expansions: Int
        var cost: Float = 0
        var path: [Int] { (try? result.get()) ?? [] }
    }

    /// Searches net `net`'s first component to its second.
    private func run(_ board: RoutingTestBoard, net: Int = 1, change: (inout SearchRules) -> Void = { _ in },
                     corridor: ((RoutingGrid) -> [Float])? = nil, isCancelled: () -> Bool = { false }) throws -> Run {
        let problem = board.problem()
        let grid = RoutingGrid(problem: problem)
        let routing = try XCTUnwrap(problem.nets.first { $0.id == net })
        var rules = SearchRules(net: routing, grid: grid, problem: problem)
        change(&rules)
        let from = grid.terminals(of: routing.components[0], net: net, profile: rules.profile, allowed: rules.allowed)
        let to = grid.terminals(of: routing.components[1], net: net, profile: rules.profile, allowed: rules.allowed)
        XCTAssertFalse(from.isEmpty)
        XCTAssertFalse(to.isEmpty)
        var search = RoutingSearch(nodes: grid.cellCount * grid.layers.count)
        let result = search.search(grid, rules: rules, sources: from.map(\.node), targets: to.map(\.node),
                                   goal: grid.cellBox(to.map(\.node)), corridor: corridor?(grid), isCancelled: isCancelled)
        return Run(grid: grid, result: result, expansions: search.expansions, cost: search.lastCost)
    }

    private func layer(_ run: Run, _ node: Int) -> Int { node / run.grid.cellCount }
    private func xy(_ run: Run, _ node: Int) -> (Int, Int) {
        let c = node % run.grid.cellCount
        return (c % run.grid.nx, c / run.grid.nx)
    }

    /// The path's length in cells (1 a side step, √2 a diagonal; vias 0).
    private func length(_ run: Run) -> Double {
        zip(run.path, run.path.dropFirst()).reduce(0) { sum, pair in
            let (a, b) = (xy(run, pair.0), xy(run, pair.1))
            return sum + hypot(Double(a.0 - b.0), Double(a.1 - b.1))
        }
    }

    private func viaCount(_ run: Run) -> Int {
        zip(run.path, run.path.dropFirst()).filter { layer(run, $0.0) != layer(run, $0.1) }.count
    }

    /// The shortest octile length on one layer from `run`'s first to last
    /// node by plain Dijkstra over the same passability: the reference.
    private func referenceLength(_ run: Run, layer: Int, net: Int, from: [Int], to: Set<Int>) -> Double {
        let grid = run.grid
        var best = [Double](repeating: .infinity, count: grid.cellCount)
        var open: [(Double, Int)] = from.map { (0, $0 % grid.cellCount) }
        for (_, c) in open { best[c] = 0 }
        let profile = grid.profile(of: net)
        while !open.isEmpty {
            open.sort { $0.0 > $1.0 }
            let (d, c) = open.removeLast()
            if d > best[c] { continue }
            if to.contains(c) { return d }
            let x = c % grid.nx, y = c / grid.nx
            for (dx, dy) in [(1, 0), (-1, 0), (0, 1), (0, -1), (1, 1), (1, -1), (-1, 1), (-1, -1)] {
                let jx = x + dx, jy = y + dy
                guard jx >= 0, jy >= 0, jx < grid.nx, jy < grid.ny else { continue }
                let n = grid.cell(jx, jy)
                guard grid.isPassable(n, layer: layer, net: net, profile: profile) else { continue }
                if dx != 0, dy != 0 {
                    guard grid.isPassable(grid.cell(jx, y), layer: layer, net: net, profile: profile),
                          grid.isPassable(grid.cell(x, jy), layer: layer, net: net, profile: profile) else { continue }
                }
                let nd = d + hypot(Double(dx), Double(dy))
                if nd < best[n] - 1e-9 { best[n] = nd; open.append((nd, n)) }
            }
        }
        return .infinity
    }

    private func board(_ width: Double = 20, _ height: Double = 20, layers: [CopperLayer] = [.top, .bottom])
        -> RoutingTestBoard {
        var board = RoutingTestBoard(width: width, height: height, layers: layers)
        board.options.grid = 0.1
        return board
    }

    func testStraightRunIsStraight() throws {
        var board = board()
        board.pad(5, 10, 1, 1, net: 1)
        board.pad(15, 10, 1, 1, net: 1)
        let run = try run(board)
        XCTAssertFalse(run.path.isEmpty)
        XCTAssertEqual(Set(run.path.map { xy(run, $0).1 }).count, 1, "one row")
        XCTAssertEqual(length(run), Double(run.path.count - 1), accuracy: 1e-9, "no diagonal")
        XCTAssertEqual(viaCount(run), 0)
    }

    func testDetourIsTheShortestWayAround() throws {
        var board = board()
        board.pad(5, 10, 1, 1, net: 1)
        board.pad(15, 10, 1, 1, net: 1)
        board.pad(10, 10, 1, 6, net: 2)
        board.unrouted = [2]
        board.allowed[1] = [.top]
        let run = try run(board) { $0.bendCost = 0 }
        XCTAssertFalse(run.path.isEmpty)
        // Clear of net 2's copper by its bloat all the way.
        let wall = CGRect(x: 9.5, y: 7, width: 1, height: 6)
        for node in run.path {
            let p = run.grid.point(node % run.grid.cellCount)
            XCTAssertGreaterThanOrEqual(CopperGeometry.pointDistance(p, wall), 0.1 + 0.127 - 1e-9)
        }
        let problem = board.problem()
        let rules = SearchRules(net: problem.nets[0], grid: run.grid, problem: problem)
        let from = run.grid.terminals(of: problem.nets[0].components[0], net: 1, profile: rules.profile, allowed: rules.allowed)
        let to = run.grid.terminals(of: problem.nets[0].components[1], net: 1, profile: rules.profile, allowed: rules.allowed)
        let reference = referenceLength(run, layer: 0, net: 1, from: from.map(\.node), to: Set(to.map { $0.node }))
        XCTAssertEqual(length(run), reference, accuracy: 1e-3)
        XCTAssertGreaterThan(length(run), 90, "it goes round the 6 mm wall, not through it")
    }

    /// A wall of net 2 across the whole board on Top.
    private func walled(layers: [CopperLayer] = [.top, .bottom]) -> RoutingTestBoard {
        var board = board(layers: layers)
        board.pad(3, 10, 1, 1, net: 1)
        board.pad(17, 10, 1, 1, net: 1)
        board.trace([CGPoint(x: 10, y: 0), CGPoint(x: 10, y: 20)], layer: .top, net: 2)
        return board
    }

    func testAWallOnTopForcesAViaAndAReturn() throws {
        let run = try run(walled())
        XCTAssertFalse(run.path.isEmpty)
        XCTAssertEqual(viaCount(run), 2)
        XCTAssertEqual(layer(run, run.path.first!), 0)
        XCTAssertEqual(layer(run, run.path.last!), 0)
        XCTAssertTrue(run.path.contains { layer(run, $0) == 1 })
    }

    func testNoViasFails() throws {
        var board = walled()
        board.options.vias = .none
        XCTAssertEqual(try run(board).result, .failure(.noPath))
    }

    func testOnlyTopNeverTouchesBottom() throws {
        var board = walled()
        board.allowed[1] = [.top]
        let run = try run(board)
        XCTAssertEqual(run.result, .failure(.noPath))
        var open = board
        open.obstacles = []
        let free = try self.run(open)
        XCTAssertFalse(free.path.isEmpty)
        XCTAssertTrue(free.path.allSatisfy { layer(free, $0) == 0 })
    }

    func testPreferBottomTakesBottom() throws {
        var board = board()
        board.pad(5, 10, 1, 1, net: 1, layers: nil)
        board.pad(15, 10, 1, 1, net: 1, layers: nil)
        board.preferred[1] = [.bottom]
        let run = try run(board)
        XCTAssertFalse(run.path.isEmpty)
        XCTAssertTrue(run.path.allSatisfy { layer(run, $0) == 1 }, "the through-hole pads reach Bottom")
    }

    func testLayerDirectionsSteerAVerticalRun() throws {
        var board = board()
        board.pad(10, 4, 1, 1, net: 1, layers: nil)
        board.pad(10, 16, 1, 1, net: 1, layers: nil)
        board.options.layers = [.top: .init(enabled: true, direction: .horizontal),
                                .bottom: .init(enabled: true, direction: .vertical)]
        let run = try run(board)
        XCTAssertTrue(run.path.allSatisfy { layer(run, $0) == 1 }, "vertical runs belong on Bottom")
        board.options.layers = [.top: .init(enabled: true, direction: .vertical),
                                .bottom: .init(enabled: true, direction: .horizontal)]
        let flipped = try self.run(board)
        XCTAssertTrue(flipped.path.allSatisfy { layer(flipped, $0) == 0 })
    }

    func testOrthogonalHasNoDiagonalStep() throws {
        var board = board()
        board.pad(5, 5, 1, 1, net: 1)
        board.pad(15, 12, 1, 1, net: 1)
        board.options.angle = .orthogonal
        let run = try run(board)
        XCTAssertFalse(run.path.isEmpty)
        for (a, b) in zip(run.path, run.path.dropFirst()) where layer(run, a) == layer(run, b) {
            let (p, q) = (xy(run, a), xy(run, b))
            XCTAssertTrue(p.0 == q.0 || p.1 == q.1)
        }
        board.options.angle = .octilinear
        let diagonal = try self.run(board)
        XCTAssertTrue(zip(diagonal.path, diagonal.path.dropFirst()).contains { pair in
            let (p, q) = (xy(diagonal, pair.0), xy(diagonal, pair.1))
            return p.0 != q.0 && p.1 != q.1
        })
    }

    func testAViaNeedsRoomOnEveryLayer() throws {
        let layers: [CopperLayer] = [.top, .inner(2), .inner(3), .bottom]
        let open = walled(layers: layers)
        XCTAssertFalse(try run(open).path.isEmpty)
        var blocked = open
        // Net 2's copper over the left half of Layer 2: no via fits there.
        blocked.pad(4.75, 10, 9.5, 20, net: 2, layers: [.inner(2)])
        blocked.unrouted = [2]
        XCTAssertEqual(try run(blocked).result, .failure(.noPath))
    }

    func testCancelStopsWithin4096Expansions() throws {
        var board = walled()
        board.options.vias = .none
        board.options.grid = 0.05
        var asked = 0
        let run = try run(board) { _ in } isCancelled: { asked += 1; return true }
        XCTAssertEqual(run.result, .failure(.cancelled))
        XCTAssertEqual(asked, 1)
        XCTAssertLessThanOrEqual(run.expansions, 4096)
    }

    // MARK: Corridors (plan-66-D)

    /// One layer, no vias, net 1 from (3, 10) to (17, 12).
    private func corridorBoard() -> RoutingTestBoard {
        var board = RoutingTestBoard(width: 20, height: 20, layers: [.top])
        board.options.grid = 0.1
        board.options.vias = .none
        board.pad(3, 10, 1, 1, net: 1)
        board.pad(17, 12, 1, 1, net: 1)
        return board
    }

    func testNoCorridorIsTheSameSearch() throws {
        let board = corridorBoard()
        let plain = try run(board)
        let zeros = try run(board, corridor: { Array(repeating: 0, count: $0.cellCount * $0.layers.count) })
        XCTAssertFalse(plain.path.isEmpty)
        XCTAssertEqual(zeros.path, plain.path)
        XCTAssertEqual(zeros.cost, plain.cost)
        XCTAssertEqual(zeros.expansions, plain.expansions)
    }

    func testACorridorKeepsAPathInsideIt() throws {
        // Net 1 from (3, 10) to (17, 10); a corridor one cell wide along that row.
        var board = corridorBoard()
        board.pads.removeLast()
        board.pad(17, 10, 1, 1, net: 1)
        func line(_ grid: RoutingGrid, until: Int? = nil) -> [Float] {
            var values = [Float](repeating: -1, count: grid.cellCount * grid.layers.count)
            let row = Int(((10 - grid.origin.y) / grid.pitch).rounded())
            for x in 0..<(until ?? grid.nx) { values[grid.cell(x, row)] = 0 }
            return values
        }
        let inside = try run(board, corridor: { line($0) })
        XCTAssertFalse(inside.path.isEmpty)
        let row = Int(((10 - inside.grid.origin.y) / inside.grid.pitch).rounded())
        XCTAssertTrue(inside.path.allSatisfy { xy(inside, $0).1 == row }, "every node on the row")
        // A corridor that stops short of the target finds nothing.
        let short = try run(board, corridor: { line($0, until: $0.nx / 2) })
        XCTAssertEqual(short.result, .failure(.noPath))
    }

    func testADeviationPriceKeepsAPathNearTheOldLine() throws {
        // From (3, 10) to (17, 12): free to go diagonal first or last; the old line is the row y = 10.
        let board = corridorBoard()
        func around(_ deviation: Double) -> (RoutingGrid) -> [Float] {
            { grid in
                var values = [Float](repeating: -1, count: grid.cellCount * grid.layers.count)
                for y in 0..<grid.ny {
                    let d = abs(Double(y) * grid.pitch + grid.origin.y - 10)
                    guard d <= 3 else { continue }
                    for x in 0..<grid.nx { values[grid.cell(x, y)] = Float(deviation * d / grid.pitch) }
                }
                return values
            }
        }
        let near = try run(board, corridor: around(1))
        XCTAssertFalse(near.path.isEmpty)
        let row = Int(((10 - near.grid.origin.y) / near.grid.pitch).rounded())
        let first = near.path.prefix(near.path.count / 2).map { xy(near, $0).1 }
        XCTAssertTrue(first.allSatisfy { $0 == row }, "stays on the old line until it must leave: \(first)")
        let deviation = { (run: Run) in run.path.reduce(0) { $0 + abs(self.xy(run, $1).1 - row) } }
        let free = try run(board, corridor: around(0))
        XCTAssertLessThanOrEqual(deviation(near), deviation(free))
    }
}
