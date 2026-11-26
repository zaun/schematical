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

    /// The DRC kinds the router can cause (plan-38-H § 1).
    static let routerKinds: Set<DesignRuleIssue.Kind> = [.short, .unconnected, .layer, .layerRule, .clearance, .edge,
                                                          .drill, .annularRing, .holeToHole, .via, .traceWidth]

    /// Balanced on each fixture, as `Router.route` runs it (route, then
    /// clean, verify and repair), checked against plan-38-F's floors and
    /// plan-38-H's guarantee: applied, the DRC finds nothing the router
    /// caused; unrouted lines equal the connections not routed; cleaning
    /// made no net longer and added no via.
    func testNegotiationOnTheFixtures() throws {
        try negotiate(Fixture.names, push: false)
    }

    /// plan-66-E: the same with push and shove on (AFU and RegFile; LFU is the Release test's): the same
    /// floors, and nothing the router caused in the DRC. The fixtures hold no copper, so what is pushed
    /// is what the run itself has laid.
    func testPushOnTheFixtures() throws {
        try negotiate(["AFU", "RegFile"], push: true)
    }

    private func negotiate(_ names: [String], push: Bool) throws {
        let floors = ["AFU": 115, "LFU": 127, "RegFile": 12]
        let silkscreen = ["AFU": 16, "LFU": 0, "RegFile": 8]
        for name in names {
            let doc = try Fixture.open(name)
            var options = RoutingOptions.standard
            options.pushAndShove = push
            let problem = doc.routingProblem(options: options, rules: .customDefault)
            let started = Date()
            // Negotiation alone in an unoptimised build (see `RoutingOptions.fixture`).
            #if DEBUG
            let strategy = Router.Strategy(rounds: 30, viaCost: 12, historyStep: 1, maxRips: 12)
            #else
            let strategy = Router.Strategy.of(.balanced)
            #endif
            var run = Router.Run(problem: problem, strategy: strategy, seed: problem.options.seed)
            run.routeAll(progress: { _ in }, isCancelled: { false })
            let raw = run.result
            let copper = run.finishedCopper()
            var result = run.result(traces: copper.traces, vias: copper.vias)
            result.stats.seconds = Date().timeIntervalSince(started)
            let vertices = { (traces: [RoutedTrace]) in traces.reduce(0) { $0 + $1.points.count } }
            print(String(format: "ROUTER cleanup %@ wire %.0f -> %.0f mm, vias %d -> %d, vertices %d -> %d",
                         name, raw.stats.wireLength, result.stats.wireLength, raw.stats.vias, result.stats.vias,
                         vertices(raw.traces), vertices(result.traces)))
            for net in problem.nets {
                let before = raw.traces.filter { $0.net == net.id }, after = result.traces.filter { $0.net == net.id }
                // Longer only by what leaving a pad at an aligned angle takes, at each trace end.
                XCTAssertLessThanOrEqual(after.reduce(0) { $0 + RoutingCleanup.length($1.points) },
                                         before.reduce(0) { $0 + RoutingCleanup.length($1.points) }
                                             + 2 * RoutingCleanup.endGrowth * Double(before.count) + 1e-6, "\(name) \(net.name)")
                XCTAssertLessThanOrEqual(result.vias.filter { $0.net == net.id }.count,
                                         raw.vias.filter { $0.net == net.id }.count, "\(name) \(net.name)")
            }
            doc.applyRouting(result, problem: problem)
            let issues = doc.designRuleCheck(rules: .customDefault)
            let caused = issues.filter { Self.routerKinds.contains($0.kind) }
            XCTAssertEqual(caused.map { "\($0.kind) \($0.reference): \($0.issue)" }, [], name)
            XCTAssertEqual(issues.filter { $0.kind == .unrouted }.count, result.stats.connections - result.stats.routed, name)
            XCTAssertEqual(issues.filter { $0.kind == .silkscreen }.count, silkscreen[name], name)
            report("balanced \(name)", result)
            print("ROUTER rounds \(result.stats.rounds) rips \(result.stats.rips)")
            if push {
                print("PUSH \(name) routed \(result.stats.routed)/\(result.stats.connections), pushTried \(result.stats.pushTried), "
                      + "pushKept \(result.stats.pushKept), rips \(result.stats.rips), vias \(result.stats.vias), "
                      + String(format: "wire %.1f mm", result.stats.wireLength))
            }
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

    /// plan-38-I Phase 4: AFU's first 4 two-pin nets (by net index) as one
    /// length-match group, 0.5 mm: matched or residual, the engine's spread
    /// equal to the applied document's, and nothing the router caused in the
    /// DRC.
    func testLengthGroupOnAFU() throws {
        let doc = try Fixture.open("AFU")
        let copper = doc.connectivity()
        let members = Set(copper.netOfPad.values).filter { copper.pinCount(ofNet: $0) == 2 }.sorted().prefix(4)
        XCTAssertEqual(members.count, 4)
        let group = doc.addNetGroup(named: "Tuned", mode: .lengthMatch)
        doc.setNetGroupTolerance(group, 0.5)
        for net in members { XCTAssertTrue(doc.assignNet(try XCTUnwrap(copper.planeNet(ofNet: net)), toGroup: group)) }

        var plain = RoutingOptions.fixture
        plain.tuneGroups = false
        let before = Router.route(doc.routingProblem(options: plain, rules: .customDefault))
        let problem = doc.routingProblem(options: .fixture, rules: .customDefault)
        let result = Router.route(problem)
        let outcome = try XCTUnwrap(result.groups.first)
        let lengthsBefore = members.map { RoutingTuning.length(of: $0, traces: before.traces, problem: problem) }
        print("ROUTER group before \(lengthsBefore.map { String(format: "%.2f", $0) }) after "
              + "\(members.map { String(format: "%.2f", outcome.lengths[$0] ?? -1) }) status \(outcome.status)")
        switch outcome.status {
        case .matched, .residual: break
        case .ineligible, .coupled, .uncoupled: XCTFail("\(outcome.status)")
        }
        doc.applyRouting(result, problem: problem)
        let applied = doc.connectivity()
        let lengths = members.compactMap { applied.net(of: try! XCTUnwrap(copper.planeNet(ofNet: $0))) }
            .compactMap { doc.netLength(ofNet: $0, in: applied) }
        XCTAssertEqual(lengths.count, 4)
        XCTAssertEqual(outcome.spread, (lengths.max() ?? 0) - (lengths.min() ?? 0), accuracy: 1e-6)
        let caused = doc.designRuleCheck(rules: .customDefault).filter { Self.routerKinds.contains($0.kind) }
        XCTAssertEqual(caused.map { "\($0.kind) \($0.reference): \($0.issue)" }, [])
    }

    /// plan-38-J Phase 4: AFU's first two two-pin nets as a pair (gap 0.2),
    /// with everything else routed: nothing the router caused in the DRC,
    /// and a coupled pair's uncoupled length as measured on the applied copper.
    func testDiffPairOnAFU() throws {
        let doc = try Fixture.open("AFU")
        let copper = doc.connectivity()
        let members = Array(Set(copper.netOfPad.values).filter { copper.pinCount(ofNet: $0) == 2 }.sorted().prefix(2))
        let pair = doc.addNetGroup(named: "Pair", mode: .diffPair)
        doc.setNetGroupGap(pair, 0.2)
        let keys = try members.map { try XCTUnwrap(copper.planeNet(ofNet: $0)) }
        for key in keys { XCTAssertTrue(doc.assignNet(key, toGroup: pair)) }
        let problem = doc.routingProblem(options: .fixture, rules: .customDefault)
        let result = Router.route(problem)
        let outcome = try XCTUnwrap(result.groups.first { $0.group == pair })
        doc.applyRouting(result, problem: problem)
        let applied = doc.connectivity()
        let nets = keys.compactMap { applied.net(of: $0) }
        let netOfRoot = applied.netOfRoot()
        func traces(_ net: Int) -> [RoutedTrace] {
            applied.traces.indices.filter { netOfRoot[applied.root[applied.traceNode($0)]] == net }.map {
                RoutedTrace(layer: applied.traces[$0].layer, width: applied.traces[$0].widthMM, points: applied.traces[$0].points)
            }
        }
        let measured = PairGeometry.uncoupledLength(p: traces(nets[0]), n: traces(nets[1]), gap: 0.2)
        print(String(format: "ROUTER pair %@, uncoupled %.2f mm (measured %.2f), skew %.2f mm, routed %d/%d",
                     "\(outcome.status)", { () -> Double in if case let .coupled(mm) = outcome.status { return mm } else { return -1 } }(),
                     measured, outcome.spread, result.stats.routed, result.stats.connections))
        switch outcome.status {
        case let .coupled(uncoupled): XCTAssertEqual(uncoupled, measured, accuracy: 0.05)
        case .uncoupled: break
        default: XCTFail("\(outcome.status)")
        }
        let caused = doc.designRuleCheck(rules: .customDefault).filter { Self.routerKinds.contains($0.kind) }
        XCTAssertEqual(caused.map { "\($0.kind) \($0.reference): \($0.issue)" }, [])
    }

    /// Fast and Thorough on the fixtures, for the record (plan-38-F Phase 4);
    /// each result is legal.
    func testFastAndThoroughOnTheFixtures() throws {
        // Thorough only in an optimised build (see `RoutingOptions.fixture`).
        #if DEBUG
        let strategies = [RoutingOptions.Strategy.fast]
        #else
        let strategies = [RoutingOptions.Strategy.fast, .thorough]
        #endif
        for strategy in strategies {
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
                                                                              maxRips: 12))
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
