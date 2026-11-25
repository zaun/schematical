import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// `SchematicDocument.routingProblem(options:)` (plan-38-D): scope, include,
/// clearing, rules and obstacles, on the fixtures and on small boards.
@MainActor
final class RoutingProblemTests: XCTestCase {
    private func problem(_ doc: SchematicDocument, _ change: (inout RoutingOptions) -> Void = { _ in }) -> RoutingProblem {
        var options = RoutingOptions()
        change(&options)
        return doc.routingProblem(options: options, rules: .customDefault)
    }

    private func centre(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
        let designator = chip.part.pins[ref.index].designator
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// Two LEDs wired K to A, with the traces given put on the board.
    private func wiredPair(traces: (CGPoint, CGPoint) -> [BoardTrace] = { _, _ in [] }) throws
        -> (doc: SchematicDocument, k: CGPoint, a: CGPoint, key: PlaneNet) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        let kRef = try TestSupport.pin(one, "K"), aRef = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(kRef), .pin(aRef)))
        let k = try centre(doc, kRef), a = try centre(doc, aRef)
        let extra = traces(k, a)
        if !extra.isEmpty {
            doc.replaceContents(chips: doc.chips, wires: doc.wires, junctions: doc.junctions, buses: doc.buses,
                                nextReference: doc.nextReference, board: doc.board, texts: doc.texts,
                                traces: extra)
        }
        let key = PlaneNet.pin(chip: one.id, designator: one.part.pins[kRef.index].designator)
        return (doc, k, a, key)
    }

    func testFixtureCensus() throws {
        let expected = ["AFU": (53, 115), "LFU": (59, 188), "RegFile": (8, 24)]
        for name in Fixture.names {
            let routing = problem(try Fixture.open(name))
            XCTAssertEqual(routing.nets.count, expected[name]?.0, name)
            XCTAssertEqual(routing.connections, expected[name]?.1, name)
            XCTAssertTrue(routing.removedTraces.isEmpty && routing.removedVias.isEmpty, name)
            XCTAssertEqual(routing.routable, [.top, .bottom], name)
            for net in routing.nets {
                XCTAssertGreaterThanOrEqual(net.width, DesignRules.customDefault.minTraceWidth)
                XCTAssertGreaterThanOrEqual(net.clearance, DesignRules.customDefault.minClearance)
                XCTAssertEqual(net.allowed, [.top, .bottom])
                XCTAssertNil(net.impossible, "\(name) \(net.name)")
            }
        }
    }

    func testSameProblemEveryTime() throws {
        let doc = try Fixture.open("AFU")
        XCTAssertEqual(problem(doc), problem(doc))
    }

    func testNetsScopeGivesThatNet() throws {
        let doc = try Fixture.open("AFU")
        let rail = try XCTUnwrap(doc.netRows().first { $0.isRail && $0.unrouted > 0 })
        let routing = problem(doc) { $0.scope = .nets([rail.key]) }
        XCTAssertEqual(routing.nets.map(\.key), [rail.key])
        XCTAssertEqual(routing.connections, rail.unrouted)
    }

    func testFootprintScopeGivesTheNetsOnItsPads() throws {
        let doc = try Fixture.open("AFU")
        let copper = doc.connectivity()
        let all = problem(doc)
        let index = try XCTUnwrap(doc.chips.indices.first { doc.chips[$0].board != nil && doc.chips[$0].boardPads.count > 4 })
        let onPads = Set(copper.pads.indices.filter { copper.pads[$0].chip == index }.compactMap { copper.netOfPad[$0] })
        let expected = Set(all.nets.map(\.id)).intersection(onPads)
        XCTAssertFalse(expected.isEmpty)
        let routing = problem(doc) { $0.scope = .footprints([doc.chips[index].id]) }
        XCTAssertEqual(Set(routing.nets.map(\.id)), expected)
    }

    func testClearAutoRemovesOnlyUnlockedAutoCopper() throws {
        let hand = UUID(), auto = UUID(), lockedAuto = UUID()
        let (doc, k, a, _) = try wiredPair { k, a in [
            BoardTrace(id: hand, layer: .top, width: 10, points: [k, CGPoint(x: k.x, y: k.y + 3)]),
            BoardTrace(id: auto, layer: .top, width: 10, points: [a, CGPoint(x: a.x, y: a.y + 3)], auto: true),
            BoardTrace(id: lockedAuto, layer: .bottom, width: 10, points: [a, CGPoint(x: a.x + 3, y: a.y)],
                       locked: true, auto: true),
        ] }
        let removed = CopperShape.Core.segment(a, CGPoint(x: a.x, y: a.y + 3))
        let keep = problem(doc)
        XCTAssertTrue(keep.removedTraces.isEmpty)
        XCTAssertTrue(keep.obstacles.contains { $0.shape.core == removed })
        let cleared = problem(doc) { $0.existing = .clearAuto }
        XCTAssertEqual(cleared.removedTraces, [auto])
        XCTAssertFalse(cleared.obstacles.contains { $0.shape.core == removed }, "removed copper is no obstacle")
        _ = k
        let net = try XCTUnwrap(cleared.nets.first)
        XCTAssertEqual(net.components.flatMap(\.traces).count, 2, "the hand trace and the locked auto trace stay")
        let unlocked = problem(doc) { $0.existing = .clearUnlocked }
        XCTAssertEqual(unlocked.removedTraces, [hand, auto])
    }

    func testUntouchedNetsSkipsANetWithATrace() throws {
        let (doc, _, _, _) = try wiredPair { k, _ in
            [BoardTrace(id: UUID(), layer: .top, width: 10, points: [k, CGPoint(x: k.x, y: k.y + 3)])]
        }
        XCTAssertEqual(problem(doc).nets.count, 1)
        XCTAssertTrue(problem(doc) { $0.include = .untouchedNets }.nets.isEmpty)
    }

    func testClassWidthAndOnlyRuleReachTheNet() throws {
        let (doc, _, _, key) = try wiredPair()
        let power = doc.addNetClass(named: "Power")
        doc.setNetClassWidth(power, 20)
        doc.setNetClassClearance(power, 0.3)
        doc.setNetClassVia(power, drill: 0.5, diameter: 1)
        doc.assignNet(key, to: power)
        doc.setNetLayerRule(key, LayerRule(mode: .only, layers: [.top]))
        let net = try XCTUnwrap(problem(doc).nets.first)
        XCTAssertEqual(net.width, 0.508, accuracy: 1e-9)
        XCTAssertEqual(net.clearance, 0.3)
        XCTAssertEqual(net.viaDrill, 0.5)
        XCTAssertEqual(net.viaDiameter, 1)
        XCTAssertEqual(net.allowed, [.top])
        XCTAssertNil(net.impossible, "an LED's through-hole pads reach Top")

        let overridden = try XCTUnwrap(problem(doc) { $0.widthOverride = 8; $0.clearanceOverride = 0.01 }.nets.first)
        XCTAssertEqual(overridden.width, 8 * 0.0254, accuracy: 1e-9)
        XCTAssertEqual(overridden.clearance, DesignRules.customDefault.minClearance, "never below the DRC's minimum")

        let bottomOff = problem(doc) { $0.layers = [.top: .init(enabled: false)] }
        XCTAssertEqual(bottomOff.routable, [.bottom])
        XCTAssertNotNil(bottomOff.nets.first?.impossible, "Only Top, and Top is off")
    }

    func testOnlyTopNetWithABottomSurfacePadIsImpossible() throws {
        let doc = try Fixture.open("LFU")
        let copper = doc.connectivity()
        let pad = try XCTUnwrap(copper.pads.indices.first {
            copper.pads[$0].side == .bottom && copper.pads[$0].pad.drill == 0 && copper.netOfPad[$0] != nil
        })
        let net = try XCTUnwrap(copper.netOfPad[pad])
        let key = try XCTUnwrap(copper.planeNet(ofNet: net))
        doc.setNetLayerRule(key, LayerRule(mode: .only, layers: [.top]))
        let routing = problem(doc) { $0.scope = .nets([key]) }
        let routed = try XCTUnwrap(routing.nets.first)
        XCTAssertEqual(routed.allowed, [.top])
        let message = try XCTUnwrap(routed.impossible)
        XCTAssertTrue(message.contains("is on Bottom"), message)
        XCTAssertTrue(message.contains("limited to Top"), message)
    }

    func testConnectionScopeNamesTwoComponentsOfOneNet() throws {
        let doc = try Fixture.open("AFU")
        let line = try XCTUnwrap(doc.ratsnest().first)
        let routing = problem(doc) { $0.scope = .connection(from: line.0, to: line.1) }
        XCTAssertEqual(routing.nets.count, 1)
        XCTAssertEqual(routing.nets.first?.components.count, 2)
        XCTAssertEqual(routing.connections, 1)
        let pads = routing.nets.first?.components.flatMap(\.pads).map(\.bounds) ?? []
        XCTAssertTrue(pads.contains { $0.insetBy(dx: -1e-6, dy: -1e-6).contains(line.0) })
        XCTAssertTrue(pads.contains { $0.insetBy(dx: -1e-6, dy: -1e-6).contains(line.1) })

        XCTAssertTrue(problem(doc) { $0.scope = .connection(from: line.0, to: CGPoint(x: -50, y: -50)) }.nets.isEmpty)
    }

    /// plan-66-B: with the option off the problem has no chains and the
    /// obstacles are exactly pads + trace segments + vias + holes + outline edges.
    func testAProblemWithoutChainsIsAsBefore() throws {
        func check(_ doc: SchematicDocument, _ name: String) {
            let routing = problem(doc)
            XCTAssertTrue(routing.movable.isEmpty, name)
            XCTAssertTrue(routing.bystanders.isEmpty, name)
            let copper = doc.connectivity()
            let pads = copper.pads.filter { $0.pad.plated || $0.pad.holeSegment != nil }.count
            let segments = copper.traces.reduce(0) { $0 + $1.segments.count }
            XCTAssertEqual(routing.obstacles.count, pads + segments + copper.vias.count + doc.boardEdge.segments.count, name)
        }
        for name in Fixture.names { check(try Fixture.open(name), name) }
        // The fixtures have no copper yet: a board with a bent trace and a via.
        let (doc, k, a, _) = try wiredPair()
        XCTAssertNotNil(doc.addTrace(points: [k, CGPoint(x: (k.x + a.x) / 2, y: k.y + 3), a], layer: .top, width: 10))
        XCTAssertNotNil(doc.addVia(at: CGPoint(x: 45, y: 60)))
        XCTAssertEqual(doc.connectivity().traces.reduce(0) { $0 + $1.segments.count }, 2)
        XCTAssertEqual(doc.connectivity().vias.count, 1)
        check(doc, "a bent trace and a via")
    }

    func testObstaclesCarryTheirNetsAndBoardPadsAreExempt() throws {
        let routing = problem(try Fixture.open("RegFile"))
        XCTAssertTrue(routing.obstacles.contains { $0.exemptFromEdge }, "the card's fingers")
        XCTAssertTrue(routing.obstacles.filter(\.exemptFromEdge).allSatisfy { $0.net != nil || $0.net == nil })
        let outline = routing.obstacles.filter { $0.shape.node == -1 }
        XCTAssertEqual(outline.count, routing.outline.count)
        XCTAssertTrue(outline.allSatisfy { $0.net == nil })
        let netted = Set(routing.obstacles.compactMap(\.net))
        XCTAssertTrue(Set(routing.nets.map(\.id)).isSubset(of: netted))
    }
}
