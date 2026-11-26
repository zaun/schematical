import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// `Wiring` on hand-built wires and junctions, with no document (plan-68-D):
/// tips, routes, hits, splitting, joining, loose ends and re-fitting.
@MainActor
final class WiringTests: XCTestCase {
    private let chip = UUID()
    private var a: PinRef { PinRef(chipID: chip, index: 0) }
    private var b: PinRef { PinRef(chipID: chip, index: 1) }
    private var c: PinRef { PinRef(chipID: chip, index: 2) }

    /// Pins a at (0, 0), b at (100, 0), c at (100, 100).
    private var tips: PinTips {
        PinTips(pins: [a: CGPoint(x: 0, y: 0), b: CGPoint(x: 100, y: 0), c: CGPoint(x: 100, y: 100)])
    }

    private func wiring(_ wires: [Wire] = [], _ junctions: [Junction] = []) -> Wiring {
        Wiring(wires: wires, junctions: junctions)
    }

    func testTipsResolveEveryKindOfEnd() {
        let port = BusPinRef(portID: UUID(), wireID: UUID())
        let label = UUID()
        let junction = Junction(point: CGPoint(x: 40, y: 20))
        let tips = PinTips(pins: [a: .zero], busPins: [port: CGPoint(x: 5, y: 6)], labels: [label: CGPoint(x: 7, y: 8)])
        let value = wiring([], [junction])
        XCTAssertEqual(value.endTip(.pin(a), tips), .zero)
        XCTAssertEqual(value.endTip(.junction(junction.id), tips), CGPoint(x: 40, y: 20))
        XCTAssertEqual(value.endTip(.busPin(port), tips), CGPoint(x: 5, y: 6))
        XCTAssertEqual(value.endTip(.label(label), tips), CGPoint(x: 7, y: 8))
        XCTAssertNil(value.endTip(.junction(UUID()), tips))
        XCTAssertEqual(value.anchorTip(.pin(a), tips), .zero)
        XCTAssertEqual(value.anchorTip(.point(CGPoint(x: 3, y: 4)), tips), CGPoint(x: 3, y: 4))
        XCTAssertEqual(value.anchorTip(.busPin(port), tips), CGPoint(x: 5, y: 6))
        XCTAssertEqual(value.anchorTip(.label(label), tips), CGPoint(x: 7, y: 8))
    }

    func testPathRunsTipToTipAndIsKept() {
        let wire = Wire(start: .pin(a), end: .pin(b), interior: [])
        let value = wiring([wire])
        let path = value.path(for: wire, tips)
        XCTAssertEqual(path.first, CGPoint(x: 0, y: 0))
        XCTAssertEqual(path.last, CGPoint(x: 100, y: 0))
        // The kept route comes back for the same wire and tips.
        XCTAssertEqual(value.path(forWire: wire.id, tips), path)
        XCTAssertEqual(value.path(forWire: UUID(), tips), [])
        let dangling = Wire(start: .pin(a), end: .junction(UUID()), interior: [])
        XCTAssertEqual(value.path(for: dangling, tips), [])
    }

    func testRouteCacheIsDroppedWhenItOutgrowsTheWires() {
        let routes = WireRoutes()
        let wires = (0..<20).map { _ in Wire(start: .pin(a), end: .pin(b), interior: []) }
        for wire in wires {
            XCTAssertEqual(routes.path(for: wire, start: .zero, end: CGPoint(x: 10, y: 0), wireCount: 0).count, 2)
        }
        // Still right after the drop.
        XCTAssertEqual(routes.path(for: wires[0], start: .zero, end: CGPoint(x: 10, y: 0), wireCount: 0).count, 2)
    }

    func testConnectedPinsLooseEndsAndDots() {
        let loose = Junction(point: CGPoint(x: 50, y: 50), isLoose: true)
        let plain = Junction(point: CGPoint(x: 60, y: 60))
        let one = Wire(start: .pin(a), end: .junction(loose.id), interior: [])
        let two = Wire(start: .junction(plain.id), end: .pin(b), interior: [])
        var value = wiring([one, two], [loose, plain])
        XCTAssertEqual(value.connectedPins, [chip: [0, 1]])
        XCTAssertTrue(value.hasLooseEnd(one))
        XCTAssertFalse(value.hasLooseEnd(two))
        XCTAssertTrue(value.showsDot(plain))
        XCTAssertFalse(value.showsDot(loose), "one end on a loose junction is no connection")
        value.wires += [Wire(start: .junction(loose.id), end: .pin(c), interior: []),
                        Wire(start: .junction(loose.id), end: .junction(loose.id), interior: [])]
        XCTAssertTrue(value.showsDot(loose))
    }

    func testHitsFindBendsAndSegments() {
        let bent = Wire(start: .pin(a), end: .pin(c), interior: [CGPoint(x: 100, y: 0)])
        let value = wiring([bent])
        XCTAssertEqual(value.nodeHit(at: CGPoint(x: 99, y: 2), tolerance: 6, tips)?.index, 1)
        XCTAssertNil(value.nodeHit(at: CGPoint(x: 50, y: 0), tolerance: 6, tips))
        let straight = wiring([Wire(start: .pin(a), end: .pin(b), interior: [])])
        XCTAssertNil(straight.nodeHit(at: CGPoint(x: 25, y: 0), tolerance: 6, tips), "only its shown midpoint is a node")
        XCTAssertEqual(value.segmentHit(at: CGPoint(x: 100, y: 50), tolerance: 6, tips)?.index, 1)
        XCTAssertNil(value.segmentHit(at: CGPoint(x: 50, y: 50), tolerance: 6, tips))
        XCTAssertEqual(value.wireHit(at: CGPoint(x: 40, y: 0), tips)?.wire.id, bent.id)
        XCTAssertNil(value.wireHit(at: CGPoint(x: 40, y: 3), tips))
    }

    func testStandInsAndEndpoints() {
        let port = BusPinRef(portID: UUID(), wireID: UUID())
        let label = UUID()
        let junction = Junction(point: CGPoint(x: 0, y: 60))
        let wire = Wire(start: .pin(a), end: .pin(b), interior: [])
        var value = wiring([wire], [junction])
        XCTAssertEqual(value.standIn(for: .pin(a), tips), .pin(a))
        XCTAssertEqual(value.standIn(for: .point(CGPoint(x: 0, y: 60)), tips), .junction(junction.id))
        XCTAssertEqual(value.standIn(for: .point(CGPoint(x: 40, y: 0)), tips), .pin(a), "a point on a wire stands in as its start")
        if case .junction? = value.standIn(for: .point(CGPoint(x: 500, y: 500)), tips) {} else { XCTFail("a fresh junction") }
        XCTAssertEqual(value.standIn(for: .busPin(port), tips), .busPin(port))
        XCTAssertEqual(value.standIn(for: .label(label), tips), .label(label))
        XCTAssertEqual(value.endpoint(for: .pin(a), tips), .pin(a))
        XCTAssertEqual(value.endpoint(for: .busPin(port), tips), .busPin(port))
        XCTAssertEqual(value.endpoint(for: .label(label), tips), .label(label))
        XCTAssertNil(value.endpoint(for: .point(CGPoint(x: 500, y: 500)), tips), "no wire there")
        guard case let .junction(id)? = value.endpoint(for: .point(CGPoint(x: 40, y: 0)), tips) else {
            return XCTFail("the wire splits")
        }
        XCTAssertEqual(value.wires.count, 2)
        XCTAssertEqual(value.wires[0].end, .junction(id))
        XCTAssertEqual(value.wires[1].start, .junction(id))
    }

    func testSplittingKeepsBendsAndRefusesAnEnd() {
        // A bent wire keeps its bends in the half they are on.
        let bent = Wire(start: .pin(a), end: .pin(c), interior: [CGPoint(x: 100, y: 0)])
        var value = wiring([bent])
        let id = value.junction(at: CGPoint(x: 100, y: 40), tips)
        XCTAssertNotNil(id)
        XCTAssertEqual(value.wires[0].interior, [CGPoint(x: 100, y: 0)])
        XCTAssertEqual(value.wires[1].interior, [])
        // An existing junction there is the answer.
        XCTAssertEqual(value.junction(at: CGPoint(x: 100, y: 40), tips), id)
        // A wire's own end is no place to split it.
        var ends = wiring([Wire(start: .pin(a), end: .pin(b), interior: [])])
        XCTAssertNil(ends.junction(at: CGPoint(x: 0, y: 0), tips))
        XCTAssertEqual(ends.wires.count, 1)
        XCTAssertNil(ends.junction(at: CGPoint(x: 0, y: 300), tips))
    }

    func testConnectAddsAWireAndFailsCleanly() {
        var value = wiring()
        XCTAssertTrue(value.connect(.pin(a), .pin(c), path: nil, first: .horizontal, last: .vertical, tips))
        XCTAssertEqual(value.wires.count, 1)
        XCTAssertEqual(value.path(for: value.wires[0], tips).first, CGPoint(x: 0, y: 0))
        // A drawn path gives the bends.
        XCTAssertTrue(value.connect(.pin(a), .pin(b), path: [.zero, CGPoint(x: 0, y: -40), CGPoint(x: 100, y: -40),
                                                         CGPoint(x: 100, y: 0)],
                                    first: nil, last: nil, tips))
        XCTAssertEqual(value.wires[1].interior, [CGPoint(x: 0, y: -40), CGPoint(x: 100, y: -40)])
        // An end that does not resolve adds nothing.
        XCTAssertFalse(value.connect(.pin(a), .point(CGPoint(x: 900, y: 900)), path: nil, first: nil, last: nil, tips))
        XCTAssertFalse(value.connect(.pin(a), .busPin(BusPinRef(portID: UUID(), wireID: UUID())), path: nil,
                                     first: nil, last: nil, tips))
        XCTAssertEqual(value.wires.count, 2)
    }

    func testConnectFromALooseEndJoinsTheWires() {
        let loose = Junction(point: CGPoint(x: 50, y: 0), isLoose: true)
        var value = wiring([Wire(start: .pin(a), end: .junction(loose.id), interior: [])], [loose])
        XCTAssertTrue(value.connect(.point(CGPoint(x: 50, y: 0)), .pin(b), path: nil, first: nil, last: .horizontal, tips))
        XCTAssertEqual(value.wires.count, 1, "the two wires through the spot became one")
        XCTAssertEqual(value.wires[0].start, .pin(a))
        XCTAssertEqual(value.wires[0].end, .pin(b))
        XCTAssertTrue(value.junctions.isEmpty)
    }

    func testPruneDropsUnusedJunctionsAndJoinsThrough() {
        let used = Junction(point: CGPoint(x: 50, y: 0))
        let unused = Junction(point: CGPoint(x: 70, y: 70))
        // Wires both ways round the junction: the join turns them to run on.
        var value = wiring([Wire(start: .junction(used.id), end: .pin(a), interior: []),
                            Wire(start: .pin(b), end: .junction(used.id), interior: [])], [used, unused])
        value.pruneJunctions(tips)
        XCTAssertEqual(value.wires.count, 1)
        XCTAssertEqual(Set([value.wires[0].start, value.wires[0].end]), [.pin(a), .pin(b)])
        XCTAssertTrue(value.junctions.isEmpty)
    }

    func testJoinLeavesLoopsAndUnresolvedEnds() {
        let spot = Junction(point: CGPoint(x: 50, y: 0))
        // Two wires between the same two ends would join into a loop.
        var loop = wiring([Wire(start: .pin(a), end: .junction(spot.id), interior: []),
                           Wire(start: .junction(spot.id), end: .pin(a), interior: [])], [spot])
        loop.joinThroughJunctions(tips)
        XCTAssertEqual(loop.wires.count, 2)
        // A wire on the junction at both ends, with one more: three ends, no join.
        var both = wiring([Wire(start: .junction(spot.id), end: .junction(spot.id), interior: [CGPoint(x: 50, y: 50)]),
                           Wire(start: .pin(a), end: .junction(spot.id), interior: [])], [spot])
        both.joinThroughJunctions(tips)
        XCTAssertEqual(both.wires.count, 2)
        // An end whose tip is unknown is not joined.
        var unknown = wiring([Wire(start: .pin(PinRef(chipID: UUID(), index: 0)), end: .junction(spot.id), interior: []),
                              Wire(start: .junction(spot.id), end: .pin(b), interior: [])], [spot])
        unknown.joinThroughJunctions(tips)
        XCTAssertEqual(unknown.wires.count, 2)
        XCTAssertEqual(Wiring.reversed(Wire(start: .pin(a), end: .pin(b), interior: [.zero, CGPoint(x: 1, y: 1)])).interior,
                       [CGPoint(x: 1, y: 1), .zero])
    }

    func testLoosenMakesSharedLooseJunctions() {
        let there = Junction(point: CGPoint(x: 100, y: 100))
        let gone = PinRef(chipID: UUID(), index: 0)
        var value = wiring([Wire(start: .pin(a), end: .pin(b), interior: []),
                            Wire(start: .pin(a), end: .pin(c), interior: []),
                            Wire(start: .pin(gone), end: .pin(b), interior: [])], [there])
        value.loosen(where: { $0 == .pin(a) || $0 == .pin(c) || $0 == .pin(gone) }, tips)
        // Both ends on a share one new junction; c's end reuses the junction at its tip.
        XCTAssertEqual(value.wires[0].start, value.wires[1].start)
        XCTAssertEqual(value.wires[1].end, .junction(there.id))
        XCTAssertTrue(value.junctions.allSatisfy(\.isLoose))
        XCTAssertEqual(value.junctions.count, 2)
        XCTAssertEqual(value.wires[2].start, .pin(gone), "an end with no tip is left alone")
    }

    func testAdoptLooseEnds() {
        let onB = Junction(point: CGPoint(x: 100, y: 0), isLoose: true)
        let onC = Junction(point: CGPoint(x: 100, y: 100), isLoose: true)
        let nowhere = Junction(point: CGPoint(x: 300, y: 300), isLoose: true)
        let plain = Junction(point: CGPoint(x: 400, y: 400))
        let base = [Wire(start: .pin(a), end: .junction(onB.id), interior: []),
                    Wire(start: .pin(c), end: .junction(onC.id), interior: []),
                    Wire(start: .junction(nowhere.id), end: .pin(a), interior: [])]
        let dropped: [CGPoint: WireEnd] = [CGPoint(x: 100, y: 0): .pin(b), CGPoint(x: 100, y: 100): .pin(c)]
        var value = wiring(base, [onB, onC, nowhere, plain])
        var asked = 0
        let adopted = value.adoptLooseEnds(dropped: dropped, every: [:], draggedJunctions: []) { _, _ in
            asked += 1
            return true
        }
        XCTAssertTrue(adopted)
        XCTAssertEqual(asked, 1, "the end already on c would end on itself")
        XCTAssertEqual(value.wires[0].end, .pin(b))
        XCTAssertFalse(value.junctions.contains { $0.id == onB.id })
        // A refused candidate stays loose; a dragged junction looks in `every`.
        var refused = wiring(base, [onB, onC, nowhere])
        XCTAssertFalse(refused.adoptLooseEnds(dropped: dropped, every: [:], draggedJunctions: []) { _, _ in false })
        XCTAssertEqual(refused.wires, base)
        var dragged = wiring(base, [onB, onC, nowhere])
        XCTAssertTrue(dragged.adoptLooseEnds(dropped: [:], every: [CGPoint(x: 300, y: 300): .pin(b)],
                                             draggedJunctions: [nowhere.id]) { _, _ in true })
        XCTAssertEqual(dragged.wires[2].start, .pin(b))
    }

    func testSettleSetPathInsertNodeAndRemap() {
        let wire = Wire(start: .pin(a), end: .pin(c), interior: [CGPoint(x: 30, y: 30)])
        let other = Wire(start: .pin(b), end: .junction(UUID()), interior: [CGPoint(x: 1, y: 1)])
        var value = wiring([wire, other])
        value.settle(where: { $0 == .pin(a) || $0 == .pin(b) }, tips)
        XCTAssertEqual(value.wires[0].interior, OrthogonalRoute.settled(start: .zero, interior: [CGPoint(x: 30, y: 30)],
                                                                        end: CGPoint(x: 100, y: 100)))
        XCTAssertEqual(value.wires[1].interior, [CGPoint(x: 1, y: 1)], "an end that does not resolve is skipped")
        value.setPath(wire.id, [.zero, CGPoint(x: 0, y: 100), CGPoint(x: 100, y: 100)])
        XCTAssertEqual(value.wires[0].interior, [CGPoint(x: 0, y: 100)])
        value.setPath(wire.id, [.zero])
        value.setPath(UUID(), [.zero, .zero, .zero])
        XCTAssertEqual(value.wires[0].interior, [CGPoint(x: 0, y: 100)], "a one-point path or an unknown wire changes nothing")
        value.insertNode(at: CGPoint(x: 0, y: 50), tolerance: 6, tips)
        XCTAssertEqual(value.wires[0].interior.count, 2)
        value.insertNode(at: CGPoint(x: 500, y: 500), tolerance: 6, tips)
        XCTAssertEqual(value.wires[0].interior.count, 2)
        value.remapEnds { $0 == .pin(a) ? .pin(b) : $0 }
        XCTAssertEqual(value.wires[0].start, .pin(b))
        XCTAssertEqual(value.wires[0].end, .pin(c))
    }

    func testGroupMoveRefitAndSettle() {
        let spot = Junction(point: CGPoint(x: 50, y: 50))
        let still = Junction(point: CGPoint(x: 60, y: 60))
        let wire = Wire(start: .pin(a), end: .junction(spot.id), interior: [CGPoint(x: 50, y: 0)])
        let other = Wire(start: .pin(b), end: .pin(c), interior: [])
        var value = wiring([wire, other], [spot, still])
        func shift(_ p: CGPoint) -> CGPoint { CGPoint(x: p.x + 20, y: p.y) }
        value.moveGroup(junctions: [spot.id: spot.point, still.id: still.point], bends: [wire.id: wire.interior,
                                                                                          other.id: []], by: { $0 == still.point ? $0 : shift($0) })
        XCTAssertEqual(value.junctions[0].point, CGPoint(x: 70, y: 50))
        XCTAssertEqual(value.junctions[1].point, CGPoint(x: 60, y: 60))
        XCTAssertEqual(value.wires[0].interior, [CGPoint(x: 70, y: 0)])
        // Re-fit to the moved ends; an unknown wire, or one with no tips, is skipped.
        let route = [CGPoint(x: 0, y: 0), CGPoint(x: 50, y: 0), CGPoint(x: 50, y: 50)]
        value.refit([wire.id: route, UUID(): route], tips)
        XCTAssertTrue(OrthogonalRoute.isOrthogonal(value.path(for: value.wires[0], tips)))
        var dangling = wiring([Wire(start: .pin(a), end: .junction(UUID()), interior: [CGPoint(x: 5, y: 5)])])
        dangling.refit([dangling.wires[0].id: route], tips)
        dangling.settleGroup(affected: [dangling.wires[0].id], orthogonal: [], tips)
        XCTAssertEqual(dangling.wires[0].interior, [CGPoint(x: 5, y: 5)])
        // Settling: an orthogonal wire gets corners for a slanted segment.
        var slanted = wiring([Wire(start: .pin(a), end: .pin(c), interior: [CGPoint(x: 40, y: 10)]),
                              Wire(start: .pin(a), end: .pin(b), interior: [CGPoint(x: 40, y: 10)])])
        slanted.settleGroup(affected: Set(slanted.wires.map(\.id)), orthogonal: [slanted.wires[0].id], tips)
        XCTAssertTrue(OrthogonalRoute.isOrthogonal(slanted.path(for: slanted.wires[0], tips)))
    }
}
