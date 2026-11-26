import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-68-W: the document side of routing — a chain skipped for a locked
/// via or a move that leaves no trace, the scope of footprints and copper
/// selected together, what the problem clears and keeps out, a connection
/// that ends at a trace vertex, a T junction's chain, and the Route menu's
/// selection commands.
@MainActor
final class RoutingDocumentEdgeTests: XCTestCase {
    private func centre(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
        let designator = chip.part.pins[ref.index].designator
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    /// Two LEDs wired K to A, 30 mm apart on the board.
    private func pair() throws -> (doc: SchematicDocument, k: CGPoint, a: CGPoint, key: PlaneNet, chip: UUID) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        let kRef = try TestSupport.pin(one, "K"), aRef = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(kRef), .pin(aRef)))
        let key = PlaneNet.pin(chip: one.id, designator: one.part.pins[kRef.index].designator)
        return (doc, try centre(doc, kRef), try centre(doc, aRef), key, one.id)
    }

    private func problem(_ doc: SchematicDocument, _ change: (inout RoutingOptions) -> Void = { _ in }) -> RoutingProblem {
        var options = RoutingOptions.standard
        change(&options)
        return doc.routingProblem(options: options, rules: .customDefault)
    }

    private func moving(_ chain: MovableChain, runs: [[CGPoint]], vias: [CGPoint]? = nil) -> RoutingResult {
        var result = RoutingResult()
        result.moved = [MovedChain(chain: chain.id, runs: runs, vias: vias ?? chain.vias.map(\.centre))]
        return result
    }

    // MARK: Applying moved chains

    func testAChainWhoseViaIsLockedIsSkipped() throws {
        let (doc, k, a, _, _) = try pair()
        let p = CGPoint(x: k.x, y: k.y + 6)
        XCTAssertNotNil(doc.addTrace(points: [k, p], layer: .top, width: 20))
        XCTAssertNotNil(doc.addTrace(points: [p, CGPoint(x: a.x, y: p.y), a], layer: .bottom, width: 20))
        let via = try XCTUnwrap(doc.addVia(at: p))
        let problem = problem(doc) { $0.pushAndShove = true }
        let chain = try XCTUnwrap(problem.movable.first)
        doc.setLocked(.via(via), true)
        let before = (doc.traces, doc.vias)
        let moved = CGPoint(x: p.x + 0.4, y: p.y)
        let summary = doc.applyRouting(moving(chain, runs: [[k, moved], [moved, CGPoint(x: a.x, y: p.y), a]], vias: [moved]),
                                       problem: problem)
        XCTAssertEqual(summary.skipped, 1)
        XCTAssertEqual(summary.movedVias, 0)
        XCTAssertEqual(doc.traces, before.0)
        XCTAssertEqual(doc.vias, before.1)
    }

    /// A move that leaves a trace no length is not made: the trace stays.
    func testAMoveThatCollapsesATraceIsSkipped() throws {
        let (doc, k, a, _, _) = try pair()
        let y = k.y + 6
        let points = [k, CGPoint(x: k.x, y: y), CGPoint(x: a.x, y: y), a]
        XCTAssertNotNil(doc.addTrace(points: points, layer: .top, width: 20))
        let branch = try XCTUnwrap(doc.addTrace(points: [points[2], CGPoint(x: points[2].x, y: y + 4)], layer: .top, width: 20))
        let problem = problem(doc) { $0.pushAndShove = true }
        let chain = try XCTUnwrap(problem.movable.first { $0.runs[0].trace == branch })
        let before = doc.traces
        let summary = doc.applyRouting(moving(chain, runs: [[points[2], points[2]]]), problem: problem)
        XCTAssertEqual(summary.skipped, 1)
        XCTAssertEqual(summary.movedTraces, 0)
        XCTAssertEqual(doc.traces, before)
    }

    // MARK: Scope

    func testAFootprintAndATraceSelectedScopeTheirNets() throws {
        let (doc, k, _, key, chip) = try pair()
        let trace = try XCTUnwrap(doc.addTrace(points: [k, CGPoint(x: k.x, y: k.y + 3)], layer: .top))
        doc.selectBoardItems([.footprint(chip)])
        XCTAssertEqual(RoutingScope.fromSelection(doc), .footprints([chip]))
        doc.selectBoardItems([.footprint(chip), .trace(trace)])
        guard case let .nets(keys) = RoutingScope.fromSelection(doc) else {
            return XCTFail("\(RoutingScope.fromSelection(doc))")
        }
        XCTAssertTrue(keys.contains(key))
        let copper = doc.connectivity()
        let index = try XCTUnwrap(doc.chips.firstIndex { $0.id == chip })
        let padNets = Set(copper.pads.indices.filter { copper.pads[$0].chip == index }
            .compactMap { copper.netOfPad[$0].flatMap(copper.planeNet(ofNet:)) })
        XCTAssertEqual(Set(keys), padNets.union([key]), "every net on the footprint's pads, and the trace's")
    }

    // MARK: The problem

    func testCopperOnNoNetIsClearedOnlyWithEverythingUnlocked() throws {
        let (doc, _, _, key, _) = try pair()
        let floating = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 40, y: 60), CGPoint(x: 50, y: 60)], layer: .top))
        XCTAssertTrue(problem(doc) { $0.existing = .clearUnlocked }.removedTraces.contains(floating))
        XCTAssertFalse(problem(doc) { $0.existing = .clearUnlocked; $0.scope = .nets([key]) }.removedTraces.contains(floating))
        XCTAssertFalse(problem(doc) { $0.existing = .clearAuto }.removedTraces.contains(floating))
    }

    func testANonPlatedHoleIsAHoleObstacle() throws {
        let doc = TestSupport.newDocument()
        let usb = try TestSupport.place(doc, "10133476-10001LF", at: CGPoint(x: 100, y: 100))
        doc.moveFootprint(usb.id, to: CGPoint(x: 30, y: 30))
        let holes = problem(doc).obstacles.filter { $0.kind == .hole }
        XCTAssertEqual(holes.count, 2, "its two non-plated holes")
        XCTAssertTrue(holes.allSatisfy { $0.net == nil && $0.drill != nil })
        XCTAssertTrue(holes.contains { $0.shape.radius == 0.325 })
    }

    /// One unrouted line from a pad to a vertex of the net's own trace.
    func testAConnectionMayEndAtATraceVertex() throws {
        let (doc, k, a, _, _) = try pair()
        let end = CGPoint(x: a.x - 10, y: a.y)
        XCTAssertNotNil(doc.addTrace(points: [a, end], layer: .top))
        let routed = problem(doc) { $0.scope = .connection(from: k, to: end) }
        XCTAssertEqual(routed.nets.count, 1)
        XCTAssertEqual(problem(doc) { $0.scope = .connection(from: k, to: CGPoint(x: 5, y: 5)) }.nets, [],
                       "an end on no copper names nothing")
    }

    /// A trace ending on the middle of another's segment touches it away
    /// from its anchors: that segment's chain stays put.
    func testATJunctionKeepsTheSegmentItLandsOnOutOfTheChains() throws {
        let (doc, k, a, _, _) = try pair()
        let y = k.y + 6
        let points = [k, CGPoint(x: k.x, y: y), CGPoint(x: a.x, y: y), a]
        let trunk = try XCTUnwrap(doc.addTrace(points: points, layer: .top, width: 20))
        let middle = CGPoint(x: (points[1].x + points[2].x) / 2, y: y)
        XCTAssertNotNil(doc.addTrace(points: [middle, CGPoint(x: middle.x, y: y + 4)], layer: .top, width: 20))
        let chains = problem(doc) { $0.pushAndShove = true }.movable
        let onTrunk = chains.flatMap(\.runs).filter { $0.trace == trunk }
        XCTAssertFalse(onTrunk.contains { run in zip(run.points, run.points.dropFirst()).contains { $0 == points[1] && $1 == points[2] } },
                       "\(onTrunk)")
    }

    /// A trace whose own vertex lands on the middle of one of its segments
    /// touches itself there: that segment stays put too.
    func testATraceTouchingItselfKeepsThatSegmentOutOfTheChains() throws {
        let (doc, k, a, _, _) = try pair()
        let y = k.y + 6
        let fold = [k, CGPoint(x: k.x, y: y), CGPoint(x: k.x + 10, y: y), CGPoint(x: k.x + 10, y: y + 5),
                    CGPoint(x: k.x + 5, y: y)]
        let trace = try XCTUnwrap(doc.addTrace(points: fold, layer: .top, width: 20))
        XCTAssertEqual(doc.traces.first { $0.id == trace }?.points.count, fold.count, "kept as drawn")
        let runs = problem(doc) { $0.pushAndShove = true }.movable.flatMap(\.runs).filter { $0.trace == trace }
        XCTAssertFalse(runs.contains { run in zip(run.points, run.points.dropFirst()).contains { $0 == fold[1] && $1 == fold[2] } },
                       "\(runs)")
        _ = a
    }

    // MARK: Route menu

    func testSelectedNetsAndSelectAutorouted() throws {
        let (doc, k, a, key, chip) = try pair()
        let hand = try XCTUnwrap(doc.addTrace(points: [k, CGPoint(x: k.x, y: k.y + 3)], layer: .top))
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: a.x, y: a.y)))
        let copper = doc.connectivity()
        let net = try XCTUnwrap(copper.net(of: key))
        doc.selectBoardItems([.trace(hand)])
        XCTAssertEqual(doc.selectedNets(), [net])
        doc.selectBoardItems([.via(via)])
        XCTAssertEqual(doc.selectedNets(), [net])
        doc.selectBoardItems([.footprint(chip)])
        XCTAssertTrue(doc.selectedNets().contains(net))
        doc.selectBoardItems([.footprint(UUID())])
        XCTAssertEqual(doc.selectedNets(), [], "a footprint no longer there")

        let routing = problem(doc)
        var result = RoutingResult()
        let id = try XCTUnwrap(routing.nets.first?.id)
        result.traces = [RoutedTrace(layer: .bottom, width: 0.254, points: [k, CGPoint(x: k.x, y: k.y + 8), CGPoint(x: a.x, y: k.y + 8), a], net: id)]
        result.nets = [NetOutcome(net: id, name: "", status: .complete, unroutedConnections: 0)]
        doc.applyRouting(result, problem: routing)
        let auto = doc.traces.filter(\.auto).map { BoardItem.trace($0.id) } + doc.vias.filter(\.auto).map { BoardItem.via($0.id) }
        XCTAssertFalse(auto.isEmpty)
        doc.selectAutorouted()
        XCTAssertEqual(doc.boardSelection.items, Set(auto))
        XCTAssertFalse(doc.boardSelection.items.contains(.trace(hand)))
    }
}
