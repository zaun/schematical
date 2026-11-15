import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The quality of the router's results on the example boards: how many
/// vias, corners and jogs, and how much wire, `Router.route` leaves at the
/// default options (Balanced) under the design rules' default net class.
///
/// Balanced and Thorough plan, route and then improve the board several
/// times over, which an unoptimised build takes many minutes to do, so
/// these tests run only in an optimised one:
///
///     xcodebuild … -configuration Release ENABLE_TESTABILITY=YES test \
///         -only-testing:schematicalTests/RouterQualityFixtureTests
///
/// The numbers they print are the ones in `docs/routing.md` § Results.
@MainActor
final class RouterQualityFixtureTests: XCTestCase {
    struct Measure {
        var routed: Int, connections: Int, vias: Int, bends: Int, oddBends: Int, wire: Double, seconds: Double
        /// Pieces shorter than 0.6 mm between a turn one way and a turn the other: the jogs that make a trace look wiggly.
        var jogs = 0
    }

    static func measure(_ result: RoutingResult) -> Measure {
        var bends = 0, odd = 0, jogs = 0
        for trace in result.traces {
            let p = trace.points
            guard p.count >= 3 else { continue }
            func turn(_ i: Int) -> Double {
                (p[i].x - p[i - 1].x) * (p[i + 1].y - p[i].y) - (p[i].y - p[i - 1].y) * (p[i + 1].x - p[i].x)
            }
            for i in 1..<(p.count - 1) {
                let u = CGPoint(x: p[i].x - p[i - 1].x, y: p[i].y - p[i - 1].y)
                let v = CGPoint(x: p[i + 1].x - p[i].x, y: p[i + 1].y - p[i].y)
                let angle = abs(atan2(u.x * v.y - u.y * v.x, u.x * v.x + u.y * v.y)) * 180 / .pi
                bends += 1
                if abs(angle - 90) >= 1 && abs(angle - 45) >= 1 { odd += 1 }
                if i + 2 < p.count, hypot(p[i + 1].x - p[i].x, p[i + 1].y - p[i].y) < 0.6, turn(i) * turn(i + 1) < 0 { jogs += 1 }
            }
        }
        let s = result.stats
        return Measure(routed: s.routed, connections: s.connections, vias: s.vias, bends: bends, oddBends: odd,
                       wire: s.wireLength, seconds: s.seconds, jogs: jogs)
    }

    /// The DRC kinds the router can cause.
    static let routerKinds = RouterFixtureTests.routerKinds

    private func run(_ name: String, strategy: RoutingOptions.Strategy = .balanced) throws -> Measure {
        #if DEBUG
        throw XCTSkip("Balanced and Thorough on the fixtures need an optimised build (see the type's comment).")
        #else
        var options = RoutingOptions.standard
        options.strategy = strategy
        let doc = try Fixture.open(name)
        let problem = doc.routingProblem(options: options, rules: .customDefault)
        let result = Router.route(problem)
        XCTAssertEqual(RoutingCheck.violations(result, problem).count, 0, name)
        let m = Self.measure(result)
        XCTAssertEqual(result.stats.bends, m.bends, name)
        print(String(format: "QUALITY %@ %@: routed %d/%d, %d vias, %d bends (%d off 45°/90°, %d jogs), %.0f mm, %.1f s",
                     name, "\(strategy)", m.routed, m.connections, m.vias, m.bends, m.oddBends, m.jogs, m.wire, m.seconds))
        // Applied, the DRC finds nothing the router caused, and an unrouted line for each connection left.
        doc.applyRouting(result, problem: problem)
        let issues = doc.designRuleCheck(rules: .customDefault)
        let caused = issues.filter { Self.routerKinds.contains($0.kind) }
        XCTAssertEqual(caused.map { "\($0.kind) \($0.reference): \($0.issue)" }, [], name)
        XCTAssertEqual(issues.filter { $0.kind == .unrouted }.count, m.connections - m.routed, name)
        return m
        #endif
    }

    /// Bounds a little over what was measured (`docs/routing.md` § Results),
    /// so a change that makes the boards worse fails here.
    func testAFU() throws {
        let m = try run("AFU")
        XCTAssertEqual(m.routed, 115)
        XCTAssertLessThanOrEqual(m.vias, 80)
        XCTAssertLessThanOrEqual(m.oddBends, 3)
        XCTAssertLessThanOrEqual(m.wire, 1500)
    }

    func testRegFile() throws {
        let m = try run("RegFile")
        XCTAssertEqual(m.routed, 24)
        XCTAssertLessThanOrEqual(m.vias, 18)
        XCTAssertLessThanOrEqual(m.oddBends, 1)
        XCTAssertLessThanOrEqual(m.wire, 320)
    }

    func testLFU() throws {
        let m = try run("LFU")
        XCTAssertGreaterThanOrEqual(m.routed, 186)
        XCTAssertLessThanOrEqual(m.vias, 320)
        XCTAssertLessThanOrEqual(m.oddBends, 5)
    }

    func testThorough() throws {
        let afu = try run("AFU", strategy: .thorough)
        XCTAssertEqual(afu.routed, 115)
        XCTAssertLessThanOrEqual(afu.vias, 80)
        let reg = try run("RegFile", strategy: .thorough)
        XCTAssertEqual(reg.routed, 24)
        XCTAssertLessThanOrEqual(reg.vias, 18)
        let lfu = try run("LFU", strategy: .thorough)
        XCTAssertGreaterThanOrEqual(lfu.routed, 186)
        XCTAssertLessThanOrEqual(lfu.vias, 320)
    }
}
