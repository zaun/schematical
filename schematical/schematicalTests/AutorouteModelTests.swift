import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The Autoroute sheet's model (plan-38-K).
@MainActor
final class AutorouteModelTests: XCTestCase {
    private func defaults() -> UserDefaults {
        let name = "AutorouteModelTests-\(UUID().uuidString)"
        let defaults = UserDefaults(suiteName: name)!
        defaults.removePersistentDomain(forName: name)
        return defaults
    }

    func testOptionsAreRememberedButNotTheScope() throws {
        let store = defaults()
        let model = AutorouteModel(document: TestSupport.newDocument(), defaults: store)
        model.options.strategy = .thorough
        model.options.grid = 0.254
        model.options.existing = .clearAuto
        model.options.pushAndShove = true
        model.options.scope = .nets([.ground])
        let again = AutorouteModel(document: TestSupport.newDocument(), defaults: store)
        var expected = model.options
        expected.scope = .all
        XCTAssertEqual(again.options, expected)
        XCTAssertEqual(again.options.scope, .all)
        XCTAssertTrue(again.options.pushAndShove)
    }

    func testASelectedFootprintIsTheSelectionScope() throws {
        let doc = try Fixture.open("AFU")
        let chip = try XCTUnwrap(doc.chips.first { $0.board != nil })
        doc.select(.footprint(chip.id))
        let model = AutorouteModel(document: doc, defaults: defaults())
        model.scopeChoice = .selection
        XCTAssertEqual(model.scope, .footprints([chip.id]))
        XCTAssertEqual(model.runOptions.scope, .footprints([chip.id]))
    }

    func testTheSummaryOnAFU() async throws {
        let doc = try Fixture.open("AFU")
        let model = AutorouteModel(document: doc, defaults: defaults())
        model.refreshSummary(rules: .customDefault)
        XCTAssertEqual(model.summary?.nets, 53)
        XCTAssertEqual(model.summary?.connections, 115)
        XCTAssertEqual(model.summary?.removedTraces, 0)
        XCTAssertTrue(model.route(rules: .customDefault))
        await model.session.wait()
        guard case .finished = model.session.state else { return XCTFail("\(model.session.state)") }
        XCTAssertEqual(model.results, [])
        model.options.include = .untouchedNets
        model.refreshSummary(rules: .customDefault)
        XCTAssertEqual(model.summary?.nets, 0)
        XCTAssertEqual(model.summary?.connections, 0)

        model.options.include = .incompleteNets
        model.options.existing = .clearAuto
        model.refreshSummary(rules: .customDefault)
        XCTAssertEqual(model.summary?.removedTraces, doc.traces.filter(\.auto).count)
        XCTAssertEqual(model.summary?.removedVias, doc.vias.filter(\.auto).count)
        XCTAssertGreaterThan(model.summary?.removedTraces ?? 0, 0)
    }

    func testAOneLayerBoardHasNoVias() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        let model = AutorouteModel(document: doc, defaults: defaults())
        XCTAssertFalse(model.viasAvailable)
        XCTAssertEqual(model.runOptions.vias, .none)
        let two = AutorouteModel(document: TestSupport.newDocument(), defaults: defaults())
        XCTAssertTrue(two.viasAvailable)
        XCTAssertEqual(two.runOptions.vias, .allowed)
    }

    func testAlternatingDirections() throws {
        let doc = TestSupport.newDocument()
        let model = AutorouteModel(document: doc, defaults: defaults())
        model.useAlternatingDirections()
        XCTAssertEqual(model.layerRows.map(\.direction), [.horizontal, .vertical])
        doc.setLayerCount(4)
        doc.setLayerType(.inner(2), .wire)
        doc.setLayerType(.inner(3), .wire)
        model.useAlternatingDirections()
        XCTAssertEqual(model.layerRows.map(\.direction), [.horizontal, .vertical, .horizontal, .vertical])
        doc.setLayerType(.inner(3), .plane)
        XCTAssertEqual(model.layerRows.map(\.isPlane), [false, false, true, false])
    }

    func testRouteIsRefusedWhileTheSimulationPlays() throws {
        let doc = TestSupport.newDocument()
        doc.isPlaying = true
        let model = AutorouteModel(document: doc, defaults: defaults())
        XCTAssertFalse(model.canRoute)
        XCTAssertFalse(model.route(rules: .customDefault))
        XCTAssertNotNil(doc.editorMessage)
    }

    // MARK: Phase 2

    func testTheResultsNameEachUnroutedNetOnce() async throws {
        let doc = try Fixture.open("LFU")
        let model = AutorouteModel(document: doc, defaults: defaults())
        model.options.strategy = .fast
        model.options.timeLimit = 20
        XCTAssertTrue(model.route(rules: .customDefault))
        await model.session.wait()
        guard case .finished = model.session.state else { return XCTFail("\(model.session.state)") }
        let result = try XCTUnwrap(model.session.result)
        let rows = model.results
        XCTAssertEqual(rows.count, result.nets.filter { $0.unroutedConnections > 0 }.count)
        XCTAssertEqual(Set(rows.map(\.name)).count, rows.count, "each net once")
        XCTAssertEqual(rows.reduce(0) { $0 + $1.count }, result.stats.connections - result.stats.routed)
        XCTAssertTrue(rows.allSatisfy { !$0.reason.isEmpty && $0.count > 0 && $0.key != nil })
        XCTAssertFalse(rows.isEmpty, "LFU in 20 s leaves some unrouted")
    }

    /// A net a stopped run never reached says so, not "no legal way found"
    /// (seen in the real-app Stop run, plan-38-K Phase 4).
    func testAStoppedRunSaysSo() {
        let failed = NetOutcome(net: 0, name: "GND", status: .failed, unroutedConnections: 3)
        XCTAssertEqual(AutorouteModel.reason(for: failed, stopped: true), AutorouteModel.stoppedReason)
        XCTAssertEqual(AutorouteModel.reason(for: failed, stopped: false), AutorouteModel.notFoundReason)
        var illegal = failed
        illegal.reason = "can't rejoin"
        XCTAssertEqual(AutorouteModel.reason(for: illegal, stopped: true), "can't rejoin")
        let impossible = NetOutcome(net: 1, name: "CLK", status: .impossible("no layer"), unroutedConnections: 1)
        XCTAssertEqual(AutorouteModel.reason(for: impossible, stopped: true), "no layer")
    }

    func testTheRouteMenuIsOnlyOnThePCBPage() {
        XCTAssertTrue(RouteActions.available(page: .pcb, isPlaying: false))
        XCTAssertFalse(RouteActions.available(page: .schematic, isPlaying: false))
        XCTAssertFalse(RouteActions.available(page: .pcb, isPlaying: true))
    }

    /// plan-66-F: what a run moved of the board's existing copper, in the sheet's words.
    func testTheMovedLine() {
        XCTAssertNil(AutorouteModel.movedLine(traces: 0, vias: 0))
        XCTAssertEqual(AutorouteModel.movedLine(traces: 1, vias: 0), "Moved 1 trace aside")
        XCTAssertEqual(AutorouteModel.movedLine(traces: 3, vias: 1), "Moved 3 traces and 1 via aside")
        XCTAssertEqual(AutorouteModel.movedLine(traces: 0, vias: 2), "Moved 2 vias aside")
        // And from a run: none moved, so no line.
        let model = AutorouteModel(document: TestSupport.newDocument(), defaults: defaults())
        XCTAssertNil(model.movedLine)
    }
}
