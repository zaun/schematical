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

    func testNegotiationOnTheFixtures() throws {
        let floors = ["AFU": 115, "LFU": 127, "RegFile": 12]
        for name in Fixture.names {
            let problem = try problem(name) { $0.strategy = .balanced }
            let result = Router.route(problem)
            report("balanced \(name)", result)
            print("ROUTER rounds \(result.stats.rounds) rips \(result.stats.rips)")
            checkShapes(result, name)
            XCTAssertEqual(RoutingCheck.violations(result, problem).count, 0, name)
            XCTAssertGreaterThanOrEqual(result.stats.routed, floors[name] ?? 0, name)
            if name == "AFU" { XCTAssertLessThan(result.stats.seconds, 60) }
            // Every unrouted connection is named, under its net's name.
            for net in result.nets where net.unroutedConnections > 0 {
                XCTAssertEqual(net.unrouted.count, net.unroutedConnections, "\(name) \(net.name)")
                XCTAssertEqual(net.name, problem.nets.first { $0.id == net.net }?.name)
            }
            for net in result.nets where net.unroutedConnections > 0 {
                print("ROUTER unrouted \(name) \(net.name): \(net.unrouted.joined(separator: ", "))")
            }
        }
    }

    /// Fast and Thorough on the fixtures, for the record (plan-38-F Phase 4);
    /// each result is legal.
    func testFastAndThoroughOnTheFixtures() throws {
        for strategy in [RoutingOptions.Strategy.fast, .thorough] {
            for name in Fixture.names {
                let problem = try problem(name) { $0.strategy = strategy }
                let result = Router.route(problem)
                report("\(strategy) \(name)", result)
                XCTAssertEqual(RoutingCheck.violations(result, problem).count, 0, "\(strategy) \(name)")
            }
        }
    }

    func testSinglePassOnTheFixtures() throws {
        for name in Fixture.names {
            let problem = try problem(name)
            var run = Router.Run(problem: problem, strategy: Router.Strategy(rounds: 0, viaCost: 12, historyStep: 1,
                                                                              maxRips: 12, runs: 1))
            run.routeAll(progress: { _ in }, isCancelled: { false })
            let result = run.result
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
