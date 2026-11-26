import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-W: the Autoroute sheet drawn in each of its states (options with
/// a summary, a run under way, its results, a discarded run), its buttons,
/// and the model's run controls and group lines.
@MainActor
final class AutorouteSheetTests: XCTestCase {
    override func setUp() {
        super.setUp()
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    private func defaults() -> UserDefaults {
        let name = "AutorouteSheetTests-\(UUID().uuidString)"
        let defaults = UserDefaults(suiteName: name)!
        defaults.removePersistentDomain(forName: name)
        return defaults
    }

    /// Two LEDs wired K to A on a 4-layer board whose inner 3 is a Plane;
    /// the net is in a length-match group.
    private func document() throws -> (SchematicDocument, PlaneNet) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        let k = try TestSupport.pin(one, "K"), a = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(k), .pin(a)))
        doc.setLayerCount(4)
        doc.setLayerType(.inner(3), .plane)
        let key = PlaneNet.pin(chip: one.id, designator: one.part.pins[k.index].designator)
        let group = doc.addNetGroup(named: "BUS", mode: .lengthMatch)
        XCTAssertTrue(doc.assignNet(key, toGroup: group))
        return (doc, key)
    }

    /// Every wire layer off: the net has no layer, so no run can route it.
    private func noLayers(_ model: AutorouteModel) {
        for row in model.layerRows where !row.isPlane { model.setLayer(row.layer, enabled: false) }
    }

    func testTheOptionsShowTheSummaryAndThePlane() throws {
        let (doc, _) = try document()
        let model = AutorouteModel(document: doc, defaults: defaults())
        noLayers(model)
        model.options.existing = .clearAuto
        var closed = 0
        XCTAssertNil(model.summary)
        let probe = ViewProbe(AutorouteSheet(model: model, onClose: { closed += 1 }), size: NSSize(width: 520, height: 640))
        defer { probe.close() }
        probe.settle(0.6)
        let summary = try XCTUnwrap(model.summary, "the sheet works out the summary after a pause")
        XCTAssertEqual(summary.impossible.count, 1)
        XCTAssertTrue(probe.has("1 net, 1 connection to route"), "\(probe.labels())")
        XCTAssertTrue(probe.has("has no layer it may be routed on"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Removes 0 traces and 0 vias"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Plane (fan-out only)"), "\(probe.labels())")

        try probe.press("Alternate Directions")
        XCTAssertEqual(model.layerRows.filter { !$0.isPlane }.map(\.direction), [.horizontal, .vertical], "the inner layers are Planes")
        try probe.press("Close")
        XCTAssertEqual(closed, 1)
    }

    func testARunsResultsAreListedAndActOnTheBoard() async throws {
        let (doc, key) = try document()
        let model = AutorouteModel(document: doc, defaults: defaults())
        noLayers(model)
        model.refreshSummary(rules: .customDefault)
        var highlighted: [PlaneNet] = []
        var openedCheck = 0
        let probe = ViewProbe(AutorouteSheet(model: model, onClose: {}, onHighlight: { highlighted.append($0) },
                                             onOpenDesignRuleCheck: { openedCheck += 1 }),
                              size: NSSize(width: 520, height: 640))
        defer { probe.close() }
        try probe.press("Route")
        await model.session.wait()
        probe.settle()
        guard case .finished = model.session.state else { return XCTFail("\(model.session.state)") }
        XCTAssertTrue(probe.has("Routed 0 of 1 connections"), "\(probe.labels())")
        let row = try XCTUnwrap(model.results.first)
        XCTAssertTrue(probe.has("1 unrouted"), "\(probe.labels())")
        XCTAssertTrue(probe.has("BUS: not matched: \(RoutingTuning.notRouted)"), "\(probe.labels())")
        try probe.press(row.name)
        XCTAssertEqual(highlighted, [key])
        try probe.press("Design Rule Check")
        XCTAssertEqual(openedCheck, 1)
    }

    func testACancelledRunSaysSo() async throws {
        let (doc, _) = try document()
        let model = AutorouteModel(document: doc, defaults: defaults())
        XCTAssertTrue(model.route(rules: .customDefault))
        model.cancel()
        await model.session.wait()
        XCTAssertEqual(model.session.state, .discarded(RoutingSession.cancelledReason))
        let probe = ViewProbe(AutorouteSheet(model: model, onClose: {}), size: NSSize(width: 520, height: 640))
        defer { probe.close() }
        XCTAssertTrue(probe.has(RoutingSession.cancelledReason), "\(probe.labels())")
    }

    func testAStoppedRunKeepsWhatIsRouted() async throws {
        let (doc, _) = try document()
        let model = AutorouteModel(document: doc, defaults: defaults())
        model.options.strategy = .fast
        XCTAssertTrue(model.route(rules: .customDefault))
        let probe = ViewProbe(AutorouteSheet(model: model, onClose: {}), size: NSSize(width: 520, height: 640))
        defer { probe.close() }
        if model.session.isRunning { XCTAssertTrue(probe.has("Routing…"), "\(probe.labels())") }
        model.stop()
        await model.session.wait()
        guard case .finished = model.session.state else { return XCTFail("Stop keeps the run: \(model.session.state)") }
    }

    func testTheProgressViewShowsTheRoundAndItsButtonsAct() throws {
        var stops = 0, cancels = 0
        var progress = RoutingProgress(routed: 3, total: 10, round: 2, net: "CLK")
        progress.current = 1
        let probe = ViewProbe(RoutingProgressView(progress: progress, stop: { stops += 1 }, cancel: { cancels += 1 }))
        defer { probe.close() }
        XCTAssertTrue(probe.has("3 of 10 connections routed, round 2"), "\(probe.labels())")
        XCTAssertTrue(probe.has("1 routed right now, while rerouting"), "\(probe.labels())")
        XCTAssertTrue(probe.has("CLK"))
        try probe.press("Stop")
        try probe.press("Cancel")
        XCTAssertEqual([stops, cancels], [1, 1])

        let first = ViewProbe(RoutingProgressView(progress: RoutingProgress(routed: 0, total: 0, round: 0, net: ""),
                                                  stop: {}, cancel: {}))
        defer { first.close() }
        XCTAssertTrue(first.has("0 of 0 connections routed"), "\(first.labels())")
        XCTAssertFalse(first.has("round"))
        XCTAssertFalse(first.has("right now"))
    }

    func testGroupLinesAndResultRowIDs() {
        func line(_ status: GroupOutcome.Status) -> String {
            AutorouteModel.groupLine(GroupOutcome(group: UUID(), name: "G", target: 0, lengths: [:], spread: 0, status: status))
        }
        XCTAssertEqual(line(.matched), "G: matched")
        XCTAssertEqual(line(.residual(spread: 0.4, reason: nil)), "G: 0.40 mm apart")
        XCTAssertEqual(line(.residual(spread: 0.4, reason: "no room")), "G: 0.40 mm apart (no room)")
        XCTAssertEqual(line(.ineligible(net: 1, reason: "why")), "G: not matched: why")
        XCTAssertEqual(line(.coupled(uncoupled: 1.25)), "G: coupled, 1.25 mm apart")
        XCTAssertEqual(line(.uncoupled(reason: "why")), "G: routed uncoupled: why")
        XCTAssertEqual(AutorouteModel.ResultRow(key: nil, name: "CLK", count: 1, reason: "").id, "CLK")
    }
}
