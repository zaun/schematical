import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The router's own rule check (plan-38-H Phase 1), each rule held to the
/// DRC: the same copper applied to a document is flagged by the DRC with
/// the matching kind on the same pieces.
@MainActor
final class RoutingVerifierTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func centre(_ doc: SchematicDocument, _ chip: PlacedChip, _ pin: String) throws -> CGPoint {
        let current = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
        let designator = try XCTUnwrap(current.part.pins.first { $0.name == pin }).designator
        let pad = try XCTUnwrap(current.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// Two LEDs wired K to A, and a third LED on no net, well inside the
    /// board.
    private struct Board {
        var doc: SchematicDocument
        var k: CGPoint
        var a: CGPoint
        /// A pad of the third LED (no net).
        var loose: CGPoint
        var looseRect: CGRect
        var net: Int
    }

    private func board() throws -> Board {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: p(400, 200))
        let three = try TestSupport.place(doc, "GENERIC-LED", at: p(700, 200))
        doc.moveFootprint(one.id, to: p(30, 40))
        doc.moveFootprint(two.id, to: p(60, 40))
        doc.moveFootprint(three.id, to: p(45, 60))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "K")), .pin(try TestSupport.pin(two, "A"))))
        let loose = try centre(doc, three, "K")
        let chip = try XCTUnwrap(doc.chips.first { $0.id == three.id })
        let rect = try XCTUnwrap(chip.boardPads.first { CGPoint(x: $0.rect.midX, y: $0.rect.midY) == loose }).rect
        let problem = doc.routingProblem(options: .standard, rules: .customDefault)
        return Board(doc: doc, k: try centre(doc, one, "K"), a: try centre(doc, two, "A"), loose: loose,
                     looseRect: rect, net: try XCTUnwrap(problem.nets.first).id)
    }

    /// Verifies `traces` / `vias` on `board`, then applies them and runs the
    /// DRC; returns the verifier's kinds and the DRC's issues on the new
    /// copper.
    private func check(_ board: Board, traces: [RoutedTrace], vias: [RoutedVia], rules: DesignRules = .customDefault)
        -> (verifier: [RoutingVerifier.Violation], drc: [DesignRuleIssue]) {
        let problem = board.doc.routingProblem(options: .standard, rules: rules)
        let violations = RoutingVerifier.verify(problem, traces: traces, vias: vias)
        var result = RoutingResult()
        result.traces = traces
        result.vias = vias
        board.doc.applyRouting(result, problem: problem)
        let added = Set(board.doc.traces.filter(\.auto).map { BoardItem.trace($0.id) }
            + board.doc.vias.filter(\.auto).map { BoardItem.via($0.id) })
        let issues = board.doc.designRuleCheck(rules: rules).filter { !$0.items.isDisjoint(with: added) }
        return (violations, issues)
    }

    private func trace(_ points: [CGPoint], _ board: Board, width: Double = 0.254, layer: CopperLayer = .top) -> RoutedTrace {
        RoutedTrace(layer: layer, width: width, points: points, net: board.net)
    }

    private func via(_ centre: CGPoint, _ board: Board, drill: Double = 0.4, diameter: Double = 0.8) -> RoutedVia {
        RoutedVia(centre: centre, drill: drill, diameter: diameter, net: board.net)
    }

    private func kinds(_ violations: [RoutingVerifier.Violation]) -> [RoutingVerifier.Kind] { violations.map(\.kind) }

    /// The DRC kinds the router can cause.
    private static let routerKinds: Set<DesignRuleIssue.Kind> = [.short, .unconnected, .layer, .layerRule, .clearance,
                                                                  .edge, .drill, .annularRing, .holeToHole, .via,
                                                                  .traceWidth]

    func testACleanResultHasNoViolations() throws {
        let board = try board()
        let y = board.k.y + 6
        let route = [board.k, p(board.k.x, y), p(board.a.x, y), board.a]
        let (verifier, drc) = check(board, traces: [trace(route, board)], vias: [])
        XCTAssertEqual(verifier, [])
        XCTAssertEqual(drc.filter { Self.routerKinds.contains($0.kind) }.map(\.issue), [])
    }

    /// The net's route, with an extra piece of copper; the verifier must
    /// flag exactly one violation of `kind`, and the DRC its `drcKind` on
    /// the same piece.
    private func expect(_ kind: RoutingVerifier.Kind, _ drcKind: DesignRuleIssue.Kind, rules: DesignRules = .customDefault,
                        file: StaticString = #filePath, line: UInt = #line,
                        _ extra: (Board) -> (traces: [RoutedTrace], vias: [RoutedVia])) throws {
        let board = try board()
        let y = board.k.y + 6
        let route = trace([board.k, p(board.k.x, y), p(board.a.x, y), board.a], board)
        let more = extra(board)
        let (verifier, drc) = check(board, traces: [route] + more.traces, vias: more.vias, rules: rules)
        XCTAssertEqual(verifier.filter { $0.kind == kind }.count, 1, "\(verifier)", file: file, line: line)
        XCTAssertTrue(drc.contains { $0.kind == drcKind }, "DRC: \(drc.map { "\($0.kind) \($0.issue)" })",
                      file: file, line: line)
    }

    func testClearanceAHundredthShort() throws {
        try expect(.clearance, .clearance) { board in
            // A spur of the net passing 0.09 mm from the loose pad's edge.
            let x = board.looseRect.minX - 0.09 - 0.127
            let y = board.k.y + 6
            return ([self.trace([self.p(x, y), self.p(x, board.looseRect.midY)], board)], [])
        }
    }

    func testCopperNearTheEdge() throws {
        try expect(.edge, .edge) { board in
            let y = board.k.y + 6
            return ([self.trace([self.p(board.k.x, y), self.p(board.k.x, 0.3)], board)], [])
        }
    }

    func testThinTrace() throws {
        var rules = DesignRules.customDefault
        rules.minTraceWidth = 0.15
        try expect(.traceWidth, .traceWidth, rules: rules) { board in
            let y = board.k.y + 6
            return ([self.trace([self.p(board.k.x, y), self.p(board.k.x + 2, y)], board, width: 0.127)], [])
        }
    }

    func testSmallViaDrill() throws {
        try expect(.via, .via) { board in
            let y = board.k.y + 6
            let at = self.p(board.k.x + 2, y)
            return ([self.trace([at, self.p(at.x, y + 2)], board, layer: .bottom)], [self.via(at, board, drill: 0.15)])
        }
    }

    func testThinViaRing() throws {
        try expect(.via, .via) { board in
            let y = board.k.y + 6
            let at = self.p(board.k.x + 2, y)
            return ([self.trace([at, self.p(at.x, y + 2)], board, layer: .bottom)],
                    [self.via(at, board, drill: 0.4, diameter: 0.48)])
        }
    }

    func testHoleToHole() throws {
        try expect(.holeToHole, .holeToHole) { board in
            let y = board.k.y + 6
            let a = self.p(board.k.x + 2, y), b = self.p(board.k.x + 2.7, y)
            return ([self.trace([a, self.p(a.x, y + 2), self.p(b.x, y + 2), b], board, layer: .bottom)],
                    [self.via(a, board), self.via(b, board)])
        }
    }

    func testATraceEndInFreeSpace() throws {
        try expect(.unconnected, .unconnected) { board in
            let y = board.k.y + 6
            return ([self.trace([self.p(board.k.x + 2, y), self.p(board.k.x + 2, y + 3)], board)], [])
        }
    }

    func testAOneSidedVia() throws {
        try expect(.unconnected, .unconnected) { board in
            let y = board.k.y + 6
            return ([], [self.via(self.p(board.k.x + 4, y), board)])
        }
    }
}
