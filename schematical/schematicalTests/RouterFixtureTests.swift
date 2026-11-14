import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The router on the example boards (plan-38-E Phase 4 onward).
@MainActor
final class RouterFixtureTests: XCTestCase {
    private func problem(_ name: String, _ change: (inout RoutingOptions) -> Void = { _ in }) throws -> RoutingProblem {
        var options = RoutingOptions.standard
        change(&options)
        return try Fixture.open(name).routingProblem(options: options, rules: .customDefault)
    }

    private func report(_ name: String, _ result: RoutingResult) {
        let s = result.stats
        print(String(format: "ROUTER %@ %d/%d routed, %d vias, %.0f mm, %.2f s, %d expansions",
                     name, s.routed, s.connections, s.vias, s.wireLength, s.seconds, s.expansions))
    }

    private func checkShapes(_ result: RoutingResult, _ name: String) {
        for trace in result.traces {
            XCTAssertGreaterThanOrEqual(trace.points.count, 2, name)
            XCTAssertNil(TraceRoute.problem(trace.points), name)
        }
    }

    func testSinglePassOnTheFixtures() throws {
        for name in Fixture.names {
            let problem = try problem(name)
            let result = Router.route(problem)
            report(name, result)
            checkShapes(result, name)
            XCTAssertEqual(RoutingCheck.violations(result, problem).count, 0, name)
            if name == "AFU" {
                XCTAssertGreaterThanOrEqual(result.stats.routed, 85)
                XCTAssertLessThan(result.stats.seconds, 30)
            }
        }
    }
}
