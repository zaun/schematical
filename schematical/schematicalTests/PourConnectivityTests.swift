import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// A pour joins the nodes of its net (plan-52-D): the ratsnest, the kept
/// fills, gestures, and what the router and the design rule check read.
@MainActor
final class PourConnectivityTests: XCTestCase {
    /// Two LEDs on a 4-layer board, each cathode wired to a GND symbol of
    /// its own, and anode A of one wired to anode A of the other.
    private func board() throws -> (doc: SchematicDocument, one: PlacedChip, two: PlacedChip) {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        var leds: [PlacedChip] = []
        for n in 0..<2 {
            let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100 + 300 * n, y: 100))
            let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 100 + 300 * n, y: 400))
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
            doc.moveFootprint(led.id, to: CGPoint(x: 30 + 30 * CGFloat(n), y: 40))
            leds.append(led)
        }
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(leds[0], "A")), .pin(try TestSupport.pin(leds[1], "A"))))
        return (doc, leds[0], leds[1])
    }

    private var wholeBoard: (SchematicDocument) -> CGRect {
        { CGRect(x: 0, y: 0, width: $0.board.width, height: $0.board.height) }
    }

    private func cathodePads(_ doc: SchematicDocument, _ chip: PlacedChip) throws -> Int {
        let copper = doc.connectivity()
        let placed = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
        let designator = placed.part.pins[try TestSupport.pin(chip, "K").index].designator
        let chipIndex = try XCTUnwrap(doc.chips.firstIndex { $0.id == chip.id })
        return try XCTUnwrap(copper.pads.indices.first { copper.pads[$0].chip == chipIndex && copper.pads[$0].pad.designator == designator })
    }

    private func requireThroughHole(_ doc: SchematicDocument) throws {
        guard doc.chips.first?.boardPads.allSatisfy({ $0.drill > 0 }) == true else {
            throw XCTSkip("the LED footprint is not through-hole")
        }
    }

    // MARK: Phase 1

    func testGroundPourOnTopJoinsTheGroundPads() throws {
        let (doc, one, two) = try board()
        try requireThroughHole(doc)
        XCTAssertEqual(doc.ratsnest().count, 2, "GND and the anodes are unrouted")
        let before = doc.connectivity().netOfPad
        let id = try XCTUnwrap(doc.addPour(rect: wholeBoard(doc), layer: .top))
        XCTAssertEqual(doc.pours.first?.net, .ground)
        XCTAssertEqual(doc.ratsnest().count, 1, "the pour routes GND; the anodes stay")
        let copper = doc.connectivity()
        let a = try cathodePads(doc, one), b = try cathodePads(doc, two)
        XCTAssertEqual(copper.root[a], copper.root[b])
        XCTAssertFalse(try XCTUnwrap(copper.pourFills[id]).isEmpty)
        XCTAssertEqual(copper.netOfPad, before, "a pour never changes a pad's net")
    }

    func testAPourOnAnotherNetJoinsNoPad() throws {
        let (doc, _, _) = try board()
        try requireThroughHole(doc)
        let id = try XCTUnwrap(doc.addPour(rect: wholeBoard(doc), layer: .top))
        doc.setPourNet(id, .rail(mv: 5000))
        XCTAssertEqual(doc.ratsnest().count, 2)
        doc.setPourNet(id, nil)
        XCTAssertEqual(doc.ratsnest().count, 2, "a pour with no net joins nothing")
        XCTAssertTrue(doc.connectivity().pourFills.isEmpty)
    }

    func testAKeepoutOverACathodeLeavesItUnjoined() throws {
        let (doc, one, _) = try board()
        try requireThroughHole(doc)
        _ = doc.addPour(rect: wholeBoard(doc), layer: .top)
        XCTAssertEqual(doc.ratsnest().count, 1)
        let pad = try XCTUnwrap(doc.connectivity().pads[try cathodePads(doc, one)].pad.rect)
        doc.addKeepout(rect: pad.insetBy(dx: -1, dy: -1), layers: [.top])
        XCTAssertEqual(doc.ratsnest().count, 2, "one cathode is cut off from the pour")
        let keepout = try XCTUnwrap(doc.keepouts.first)
        doc.setKeepoutLayers(keepout.id, [.bottom])
        XCTAssertEqual(doc.ratsnest().count, 1, "a keep-out on Bottom doesn't touch a Top pour")
    }

    func testAPourOnBottomJoinsThroughHolePads() throws {
        let (doc, _, _) = try board()
        try requireThroughHole(doc)
        _ = doc.addPour(rect: wholeBoard(doc), layer: .bottom)
        XCTAssertEqual(doc.ratsnest().count, 1)
    }

    func testUndoOfThePourBringsTheLinesBack() throws {
        let (doc, _, _) = try board()
        try requireThroughHole(doc)
        _ = doc.addPour(rect: wholeBoard(doc), layer: .top)
        XCTAssertEqual(doc.ratsnest().count, 1)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.ratsnest().count, 2)
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.ratsnest().count, 1)
    }

    // MARK: Phase 2

    func testAGestureReusesTheFillsUntilItEnds() throws {
        let (doc, one, _) = try board()
        try requireThroughHole(doc)
        _ = doc.addPour(rect: wholeBoard(doc), layer: .top)
        _ = doc.connectivity()
        let chip = try XCTUnwrap(doc.chips.first { $0.id == one.id })
        let centre = try XCTUnwrap(chip.board?.centre)
        PourFiller.fillCount = 0
        doc.beginGesture(.pcb)
        doc.moveFootprint(chip.id, to: CGPoint(x: centre.x + 1.27, y: centre.y))
        _ = doc.connectivity()
        doc.moveFootprint(chip.id, to: CGPoint(x: centre.x + 2.54, y: centre.y))
        _ = doc.connectivity()
        XCTAssertEqual(PourFiller.fillCount, 0, "no refill while a drag moves what the fill reads")
        doc.endGesture()
        _ = doc.connectivity()
        XCTAssertEqual(PourFiller.fillCount, 1, "one refill after the release")
        _ = doc.connectivity()
        XCTAssertEqual(PourFiller.fillCount, 1, "kept")
    }

    func testResizingAPourInAGestureRefills() throws {
        let (doc, _, _) = try board()
        try requireThroughHole(doc)
        let id = try XCTUnwrap(doc.addPour(rect: wholeBoard(doc), layer: .top))
        _ = doc.connectivity()
        PourFiller.fillCount = 0
        doc.beginGesture(.pcb)
        doc.setPourRect(id, CGRect(x: 0, y: 0, width: 30, height: 30))
        _ = doc.connectivity()
        XCTAssertEqual(PourFiller.fillCount, 1)
        doc.endGesture()
    }

    func testBuildsThatLeaveCopperOutUseTheSameFills() throws {
        let (doc, _, _) = try board()
        try requireThroughHole(doc)
        _ = doc.addPour(rect: wholeBoard(doc), layer: .top)
        let kept = doc.connectivity()
        PourFiller.fillCount = 0
        let fresh = doc.connectivity(excludingTraces: [UUID()])
        XCTAssertEqual(PourFiller.fillCount, 0, "the kept fills serve it")
        XCTAssertEqual(kept.root, fresh.root)
        XCTAssertEqual(kept.pourFills.mapValues(\.pieces.count), fresh.pourFills.mapValues(\.pieces.count))
    }

    func testADocumentWithNoPourNeverFills() throws {
        let (doc, _, _) = try board()
        PourFiller.fillCount = 0
        _ = doc.connectivity()
        _ = doc.connectivity(excludingTraces: [UUID()])
        _ = doc.addVia(at: CGPoint(x: 20, y: 20))
        _ = doc.connectivity()
        XCTAssertEqual(PourFiller.fillCount, 0)
        XCTAssertTrue(doc.connectivity().pourFills.isEmpty)
    }

    // MARK: Phase 3

    func testTheRouterSeesOneComponentForAJoinedNet() throws {
        let (doc, _, _) = try board()
        try requireThroughHole(doc)
        func ground() -> RoutingNet? {
            doc.routingProblem(options: RoutingOptions(), rules: .customDefault).nets.first { $0.key == .ground }
        }
        XCTAssertEqual(ground()?.components.count, 2)
        _ = doc.addPour(rect: wholeBoard(doc), layer: .top)
        XCTAssertNil(ground(), "one component needs no route: the net is left out")
    }

    func testTheDesignRuleCheckCountsThePourAsCopper() throws {
        let (doc, _, _) = try board()
        try requireThroughHole(doc)
        func ground(_ kind: DesignRuleIssue.Kind) -> Int {
            doc.designRuleCheck(rules: doc.board.rules).filter { $0.kind == kind }.count
        }
        let unrouted = ground(.unrouted)
        _ = doc.addPour(rect: wholeBoard(doc), layer: .top)
        XCTAssertEqual(ground(.unrouted), unrouted - 1)
        XCTAssertEqual(ground(.short), 0)
    }

    /// plan-68-AJ: what a pour fills around: a bare hole as a hole, every
    /// trace and via with its net; and a build that is not plain reuses the
    /// fills a plain one kept.
    func testPourCopperHasHolesTracesAndVias() throws {
        let (doc, _, _) = try board()
        XCTAssertNotNil(doc.addPour(rect: wholeBoard(doc), layer: .top))
        let hole = doc.addHole(at: CGPoint(x: 10, y: 10))
        XCTAssertFalse(try XCTUnwrap(doc.holes.first { $0.id == hole }).plated)
        let trace = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 5, y: 30), CGPoint(x: 20, y: 30)], layer: .top))
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: 25, y: 30)))
        let copper = doc.connectivity()
        let items = doc.pourCopper(for: copper)
        XCTAssertTrue(items.contains { $0.isHole }, "the bare hole")
        XCTAssertTrue(items.contains { $0.node == .trace(trace) })
        XCTAssertTrue(items.contains { $0.node == .via(via) })
        // A keep-out added since the kept fills: a build that is not plain
        // fills again and keeps nothing.
        doc.addKeepout(rect: CGRect(x: 1, y: 1, width: 2, height: 2))
        let first = doc.pourFills(for: copper, plain: false)
        XCTAssertEqual(doc.keptPourFills?.keepouts.count, 0, "the kept fills are still the old ones")
        let plain = doc.pourFills(for: copper, plain: true)
        XCTAssertEqual(doc.keptPourFills?.keepouts.count, 1)
        XCTAssertEqual(Set(first.keys), Set(plain.keys))
        let reused = doc.pourFills(for: copper, plain: false)
        XCTAssertEqual(Set(reused.keys), Set(plain.keys))
        XCTAssertEqual(reused.values.map(\.pieces.count).sorted(), plain.values.map(\.pieces.count).sorted())
    }
}
