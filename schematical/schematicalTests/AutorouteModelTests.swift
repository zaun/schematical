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
        model.options.scope = .nets([.ground])
        let again = AutorouteModel(document: TestSupport.newDocument(), defaults: store)
        var expected = model.options
        expected.scope = .all
        XCTAssertEqual(again.options, expected)
        XCTAssertEqual(again.options.scope, .all)
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
}
