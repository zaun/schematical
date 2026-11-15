import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The PCB Route tool and the Nets rows' Route Net (plan-38-K Phase 3).
@MainActor
final class RouteToolTests: XCTestCase {
    private func centre(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
        let designator = chip.part.pins[ref.index].designator
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// Two LEDs wired K to A, apart on the board.
    private func pair() throws -> (doc: SchematicDocument, k: CGPoint, a: CGPoint, key: PlaneNet) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        let kRef = try TestSupport.pin(one, "K"), aRef = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(kRef), .pin(aRef)))
        return (doc, try centre(doc, kRef), try centre(doc, aRef),
                .pin(chip: one.id, designator: one.part.pins[kRef.index].designator))
    }

    func testAPadIsItsNet() throws {
        let (doc, k, _, key) = try pair()
        XCTAssertEqual(RouteTool.scope(doc, at: k, tolerance: 0.1), .nets([key]))
    }

    func testAnUnroutedLineIsThatConnection() throws {
        let (doc, _, _, _) = try pair()
        let line = try XCTUnwrap(doc.ratsnest().first)
        let middle = CGPoint(x: (line.0.x + line.1.x) / 2, y: (line.0.y + line.1.y) / 2)
        XCTAssertEqual(RouteTool.scope(doc, at: middle, tolerance: 0.1), .connection(from: line.0, to: line.1))
        var hidden = BoardVisibility()
        hidden.hidden = [.ratsnest]
        XCTAssertNil(RouteTool.scope(doc, at: middle, tolerance: 0.1, visible: hidden), "a hidden line can't be hit")
    }

    func testEmptySpaceIsNothing() throws {
        let (doc, _, _, _) = try pair()
        XCTAssertNil(RouteTool.scope(doc, at: CGPoint(x: 5, y: 75), tolerance: 0.1))
    }

    func testTheRouteToolIsATool() {
        XCTAssertTrue(PCBTool.allCases.contains(.route))
        XCTAssertEqual(PCBTool.route.title, "Route")
        XCTAssertTrue(PCBTool.route.tooltip.hasPrefix("Route — click a pad"))
    }

    /// Unroute Selected with a part selected: every net on its pads loses
    /// its unlocked copper (plan-38-M's real-app item 10 selected a trace).
    func testUnrouteSelectedOnAPart() async throws {
        let (doc, _, _, key) = try pair()
        let model = AutorouteModel(document: doc, defaults: UserDefaults(suiteName: "RouteToolTests-\(UUID())")!)
        XCTAssertTrue(model.route(rules: .customDefault))
        await model.session.wait()
        XCTAssertFalse(doc.traces.isEmpty)
        let chip = try XCTUnwrap(doc.chips.first)
        doc.select(.footprint(chip.id))
        let copper = doc.connectivity()
        XCTAssertEqual(doc.selectedNets(), [try XCTUnwrap(copper.net(of: key))])
        XCTAssertGreaterThan(doc.unroute(nets: doc.selectedNets(), onlyAuto: false), 0)
        XCTAssertTrue(doc.traces.isEmpty)
        XCTAssertEqual(doc.ratsnest().count, 1)
    }

    func testRouteNetRoutesTheRowsNet() async throws {
        let (doc, _, _, key) = try pair()
        let row = try XCTUnwrap(doc.netRows().first)
        let scope = NetsSection.routeScope(for: row)
        XCTAssertEqual(scope, .nets([key]))
        // As the window does it: the scope into the model, and a run.
        let model = AutorouteModel(document: doc, defaults: UserDefaults(suiteName: "RouteToolTests-\(UUID())")!)
        model.scopeChoice = .scope(scope)
        XCTAssertEqual(model.runOptions.scope, .nets([key]))
        XCTAssertTrue(model.route(rules: .customDefault))
        await model.session.wait()
        guard case .finished = model.session.state else { return XCTFail("\(model.session.state)") }
        XCTAssertTrue(doc.ratsnest().isEmpty)
    }
}
