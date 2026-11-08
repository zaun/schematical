import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The Bus tool joining at an L corner and joining two buses.
@MainActor
final class BusJoinTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// An L: across from (100, 100) to (200, 100), then down to (200, 200).
    private func corner(_ doc: SchematicDocument) throws -> UUID {
        try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(200, 200)))
    }

    func testLeavingCornerStraightMakesTee() throws {
        let doc = TestSupport.newDocument()
        let id = try corner(doc)
        // Straight on from the horizontal line: it grows, the corner becomes a T.
        XCTAssertEqual(doc.addBus(from: p(200, 100), to: p(300, 100)), id)
        let bus = try XCTUnwrap(doc.bus(id))
        XCTAssertEqual(doc.buses.count, 1)
        XCTAssertEqual(bus.lines.count, 2)
        XCTAssertTrue(bus.isValid)
        XCTAssertEqual(bus.joints.count, 1)
        guard case .tee = try XCTUnwrap(bus.joints.first) else { return XCTFail("expected a T") }
    }

    func testEndingOnCornerStraightMakesTee() throws {
        let doc = TestSupport.newDocument()
        let id = try corner(doc)
        // Up into the corner, in line with the vertical line.
        XCTAssertEqual(doc.addBus(from: p(200, 0), to: p(200, 100)), id)
        let bus = try XCTUnwrap(doc.bus(id))
        XCTAssertEqual(bus.lines.count, 2)
        XCTAssertTrue(bus.isValid)
        guard case .tee = try XCTUnwrap(bus.joints.first) else { return XCTFail("expected a T") }
    }

    func testCornerBackAlongItsLineIsRefused() throws {
        let doc = TestSupport.newDocument()
        _ = try corner(doc)
        XCTAssertNil(doc.addBus(from: p(200, 100), to: p(150, 100)))
        XCTAssertEqual(doc.buses.first?.lines.count, 2)
    }

    func testJoiningTwoBusesMergesThem() throws {
        let doc = TestSupport.newDocument()
        let first = try corner(doc)
        let second = try XCTUnwrap(doc.addBus(from: p(210, 100), to: p(400, 100)))
        XCTAssertEqual(doc.buses.count, 2)
        // From the corner to the other bus's free end: one bus, one long line and a T.
        XCTAssertEqual(doc.addBus(from: p(200, 100), to: p(210, 100)), first)
        XCTAssertEqual(doc.buses.count, 1)
        XCTAssertNil(doc.bus(second))
        let bus = try XCTUnwrap(doc.bus(first))
        XCTAssertTrue(bus.isValid)
        XCTAssertEqual(bus.lines.count, 2)
        XCTAssertEqual(bus.wires.count, Bus.defaultWireCount)
        XCTAssertTrue(bus.lines.contains { $0.alongRange == 100...400 && $0.isHorizontal })
        XCTAssertEqual(doc.selectedBusID, first)
    }

    func testMergeMatchesWiresByNameAndPutsUnusedLast() throws {
        let a = BusWire(name: "A"), b = BusWire(name: "B")
        let first = Bus(lines: [BusLine(a: p(0, 0), b: p(100, 0))], wires: [a, b],
                        ports: [BusPort(line: UUID(), start: 0, side: .bottom, wireIDs: [b.id])])
        let x = BusWire(name: "b"), y = BusWire(name: "C"), z = BusWire(name: "D")
        let second = Bus(lines: [BusLine(a: p(200, 0), b: p(300, 0))], wires: [x, y, z],
                         ports: [BusPort(line: UUID(), start: 0, side: .bottom, wireIDs: [x.id, y.id])])
        let merged = try XCTUnwrap(BusJoin.merge(first, absorbing: second))
        XCTAssertEqual(merged.bus.id, first.id)
        XCTAssertEqual(merged.bus.wires.map(\.name), ["B", "C", "A", "D"])
        XCTAssertEqual(merged.wireMap, [x.id: b.id])
        XCTAssertEqual(merged.bus.ports.last?.wireIDs, [b.id, y.id])
    }

    func testMergeKeepsWiresOfTheOnlyBusWithPorts() throws {
        let a = BusWire(name: "A0"), d = BusWire(name: "D0")
        let plain = Bus(lines: [BusLine(a: p(0, 0), b: p(100, 0))], wires: [d, BusWire(name: "D1")])
        let ported = Bus(lines: [BusLine(a: p(200, 0), b: p(300, 0))], wires: [a, BusWire(name: "A1")],
                         ports: [BusPort(line: UUID(), start: 0, side: .bottom, wireIDs: [a.id])])
        let one = try XCTUnwrap(BusJoin.merge(plain, absorbing: ported))
        XCTAssertEqual(one.bus.id, plain.id)
        XCTAssertEqual(one.bus.wires, ported.wires)
        XCTAssertEqual(one.bus.ports.count, 1)
        let two = try XCTUnwrap(BusJoin.merge(ported, absorbing: plain))
        XCTAssertEqual(two.bus.wires, ported.wires)
        // Neither has ports: the bus the drag starts on keeps its wires.
        let three = try XCTUnwrap(BusJoin.merge(plain, absorbing: Bus(lines: [], wires: ported.wires)))
        XCTAssertEqual(three.bus.wires, plain.wires)
    }

    func testJoiningMovesWiresOnAbsorbedPortPins() throws {
        let doc = TestSupport.newDocument()
        let first = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        let second = try XCTUnwrap(doc.addBus(from: p(400, 100), to: p(600, 100)))
        XCTAssertNil(doc.addPort(at: p(200, 100)))
        XCTAssertNil(doc.addPort(at: p(500, 100)))
        let near = try XCTUnwrap(doc.bus(first)?.ports.first)
        let port = try XCTUnwrap(doc.bus(second)?.ports.first)
        let nearB0 = try XCTUnwrap(doc.bus(first)?.wires.first)
        let b0 = try XCTUnwrap(doc.bus(second)?.wires.first)
        XCTAssertNil(doc.setPortWires(near.id, [nearB0.id]))
        XCTAssertNil(doc.setPortWires(port.id, [b0.id]))
        let pin = BusPinRef(portID: port.id, wireID: b0.id)
        XCTAssertNil(doc.connect(.busPin(pin), .busPin(BusPinRef(portID: near.id, wireID: nearB0.id))))
        XCTAssertEqual(doc.wires.count, 1)

        XCTAssertEqual(doc.addBus(from: p(300, 100), to: p(400, 100)), first)
        let bus = try XCTUnwrap(doc.bus(first))
        let firstB0 = bus.wires[0].id
        XCTAssertEqual(bus.port(port.id)?.wireIDs, [firstB0])
        guard case let .busPin(ref) = doc.wires[0].start else { return XCTFail("wire left the pin") }
        XCTAssertEqual(ref, BusPinRef(portID: port.id, wireID: firstB0))
    }

    // MARK: plan-25-A: routes with more than one bend

    private func hasLine(_ bus: Bus, _ a: CGPoint, _ b: CGPoint) -> Bool {
        bus.lines.contains { ($0.a == a && $0.b == b) || ($0.a == b && $0.b == a) }
    }

    func testAUMakesThreeLinesAndTwoCorners() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(candidates: [[p(100, 100), p(300, 100), p(300, 200), p(100, 200)]]))
        let bus = try XCTUnwrap(doc.bus(id))
        XCTAssertEqual(bus.lines.count, 3)
        XCTAssertTrue(bus.isValid)
        XCTAssertEqual(bus.joints.count, 2)
        XCTAssertTrue(bus.joints.allSatisfy { if case .corner = $0 { true } else { false } })
        XCTAssertEqual(Set(bus.joints.map(\.point)), [p(300, 100), p(300, 200)])
    }

    func testAUFromAFreeEndExtendsThatLine() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(0, 100), to: p(100, 100)))
        XCTAssertEqual(doc.addBus(candidates: [[p(100, 100), p(300, 100), p(300, 200), p(100, 200)]]), id)
        let bus = try XCTUnwrap(doc.bus(id))
        XCTAssertEqual(doc.buses.count, 1)
        XCTAssertEqual(bus.lines.count, 3)
        XCTAssertTrue(hasLine(bus, p(0, 100), p(300, 100)))
        XCTAssertTrue(hasLine(bus, p(300, 100), p(300, 200)))
        XCTAssertTrue(hasLine(bus, p(300, 200), p(100, 200)))
        XCTAssertEqual(bus.joints.count, 2)
    }

    /// Lines only meet at their ends, so a crossing is not a joint: the same
    /// as two buses crossing.
    func testARouteThatCrossesItselfIsAPlainCrossing() throws {
        let doc = TestSupport.newDocument()
        let route = [p(100, 100), p(300, 100), p(300, 200), p(200, 200), p(200, 0)]
        let id = try XCTUnwrap(doc.addBus(candidates: [route]))
        let bus = try XCTUnwrap(doc.bus(id))
        XCTAssertEqual(bus.lines.count, 4)
        XCTAssertEqual(Set(bus.joints.map(\.point)), [p(300, 100), p(300, 200), p(200, 200)])
    }

    /// Ending on its own first line would make a T the route was not drawn
    /// to make: refused, as any unexpected joint is.
    func testARouteEndingOnItselfIsRefused() {
        let doc = TestSupport.newDocument()
        let route = [p(100, 100), p(300, 100), p(300, 200), p(200, 200), p(200, 100)]
        guard case let .failure(refusal) = doc.busDraft(candidates: [route]) else {
            return XCTFail("a route ending on itself was accepted")
        }
        XCTAssertEqual(refusal, .badJoint)
        XCTAssertNil(doc.addBus(candidates: [route]))
        XCTAssertTrue(doc.buses.isEmpty)
    }

    func testCollinearPointsBecomeOneLine() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(candidates: [[p(100, 100), p(200, 100), p(300, 100)]]))
        let bus = try XCTUnwrap(doc.bus(id))
        XCTAssertEqual(bus.lines.count, 1)
        XCTAssertTrue(hasLine(bus, p(100, 100), p(300, 100)))
    }
}
