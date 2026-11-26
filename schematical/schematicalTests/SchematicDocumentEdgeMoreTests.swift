import CoreGraphics
import XCTest
@testable import schematical

/// More `SchematicDocument` lines no other test reached (plan-68-C): undo
/// coalescing, part swaps, wires to empty points, loose ends, buses and the
/// card-edge bus.
@MainActor
final class SchematicDocumentEdgeMoreTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func delete(_ doc: SchematicDocument, _ id: UUID) {
        doc.select(.chip(id))
        XCTAssertTrue(doc.deleteSchematicSelection())
    }

    // MARK: Undo

    /// Two coalescing steps with one key, the second touching an item the
    /// first did not: one undo step takes both back.
    func testACoalescedStepGainsANewItem() {
        let doc = TestSupport.newDocument()
        let a = doc.addNetLabel(at: p(100, 100))
        let b = doc.addNetLabel(at: p(300, 100))
        let names = doc.netLabels.map(\.name)
        doc.recordEdit(.schematic, "Rename", coalescing: "both") { _ = doc.setNetLabelName(a, "AAA") }
        doc.recordEdit(.schematic, "Rename", coalescing: "both") { _ = doc.setNetLabelName(b, "BBB") }
        doc.endCoalescing()
        XCTAssertEqual(doc.netLabels.map(\.name), ["AAA", "BBB"])
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.netLabels.map(\.name), names, "one undo takes back both renames")
    }

    // MARK: Parts

    /// A header made smaller: wires on the pins it loses end loose; a wire
    /// from a kept pin to a label is untouched.
    func testShrinkingAHeaderLoosensTheLostPinsOnly() throws {
        let doc = TestSupport.newDocument()
        let header = try TestSupport.place(doc, "GENERIC-HEADER-1X04", at: p(200, 200))
        let label = doc.addNetLabel(at: p(500, 100))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(header, "1")), .label(label)))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(500, 400))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(header, "4")), .pin(try TestSupport.pin(led, "A"))))
        doc.setHeaderSize(header.id, to: try TestSupport.part("GENERIC-HEADER-1X02"))
        XCTAssertEqual(doc.chips.first { $0.id == header.id }?.part.name, "GENERIC-HEADER-1X02")
        XCTAssertTrue(doc.wires.contains { $0.end == .label(label) && $0.start == .pin(PinRef(chipID: header.id, index: 0)) })
        XCTAssertEqual(doc.junctions.filter(\.isLoose).count, 1, "the pin 4 wire ends loose")
    }

    // MARK: Wires

    /// A line from a point no wire passes through, or from the end of a
    /// wire, adds nothing.
    func testConnectFromAnEmptyPointOrAWireEndAddsNothing() throws {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let one = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: p(500, 400))
        let k = try TestSupport.pin(one, "K")
        XCTAssertNil(doc.connect(.pin(k), .pin(k)), "a line to itself")
        let before = doc.state
        XCTAssertNil(doc.connect(.point(p(2000, 2000)), .pin(k)))
        XCTAssertEqual(doc.state, before)
        XCTAssertNil(doc.connect(.pin(k), .pin(try TestSupport.pin(two, "A"))))
        let wire = try XCTUnwrap(doc.wires.first)
        let end = try XCTUnwrap(doc.path(for: wire).last)
        let wired = doc.state
        XCTAssertNil(doc.connect(.point(end), .pin(try TestSupport.pin(two, "K"))))
        XCTAssertEqual(doc.state, wired, "the end of a wire is a pin, not a point to branch from")
    }

    /// Deleting a part whose pin sits on an existing junction makes that
    /// junction the loose end, instead of adding a second one there.
    func testDeletingAPartOnAJunctionLoosensThatJunction() throws {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: p(800, 600))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "K")), .pin(try TestSupport.pin(two, "A"))))
        let wire = try XCTUnwrap(doc.wires.first)
        let path = doc.path(for: wire)
        let mid = try XCTUnwrap(zip(path, path.dropFirst()).lazy.compactMap { a, b -> CGPoint? in
            let m = SchematicGrid.snap(CGPoint(x: (a.x + b.x) / 2, y: (a.y + b.y) / 2))
            return m != a && m != b ? m : nil
        }.first)
        let three = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 800))
        XCTAssertNil(doc.connect(.point(mid), .pin(try TestSupport.pin(three, "A"))))
        let junction = try XCTUnwrap(doc.junctions.first { $0.point == mid })
        // A fourth part whose K tip lands on the junction, wired to the third.
        let probe = try TestSupport.place(doc, "GENERIC-LED", at: p(1500, 1500))
        let probeTip = try XCTUnwrap(doc.pinTip(try TestSupport.pin(probe, "K")))
        delete(doc, probe.id)
        let four = try TestSupport.place(doc, "GENERIC-LED",
                                         at: p(1500 + mid.x - probeTip.x, 1500 + mid.y - probeTip.y))
        XCTAssertEqual(doc.pinTip(try TestSupport.pin(four, "K")), mid)
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(four, "K")), .pin(try TestSupport.pin(three, "K"))))
        let junctions = doc.junctions.count
        delete(doc, four.id)
        XCTAssertEqual(doc.junctions.count, junctions, "no new junction")
        XCTAssertEqual(doc.junctions.first { $0.id == junction.id }?.isLoose, true)
    }

    /// A loose end dragged onto a bus pin is taken by it.
    func testALooseEndOnABusPinIsAdopted() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 400), to: p(400, 400)))
        XCTAssertNil(doc.addPort(at: p(250, 400)))
        let port = try XCTUnwrap(doc.bus(id)?.ports.first)
        let busWire = try XCTUnwrap(doc.bus(id)?.wires.first)
        XCTAssertNil(doc.setPortWires(port.id, [busWire.id]))
        let ref = BusPinRef(portID: port.id, wireID: busWire.id)
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(250, 700))
        XCTAssertNil(doc.connect(.busPin(ref), .pin(try TestSupport.pin(led, "A"))))
        let tip = try XCTUnwrap(doc.busPinTip(ref))
        // Unticking the pin leaves the wire's end loose where the pin was.
        XCTAssertNil(doc.setPortWires(port.id, []))
        let loose = try XCTUnwrap(doc.junctions.first { $0.isLoose && $0.point == tip })
        XCTAssertNil(doc.setPortWires(port.id, [busWire.id]))
        XCTAssertEqual(doc.busPinTip(ref), tip)
        doc.adoptLooseEnds(junctions: [loose.id])
        XCTAssertFalse(doc.junctions.contains { $0.id == loose.id })
        XCTAssertTrue(doc.wires.contains { $0.start == .busPin(ref) || $0.end == .busPin(ref) })
    }

    // MARK: Buses

    func testBusWireAndPortRefusals() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(500, 100)))
        var card = BusCard()
        card.type = .custom
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertNil(doc.setBusWireNames(id, (1...8).map { "W\($0)" }))
        XCTAssertNil(doc.addPort(at: p(300, 100)))
        let port = try XCTUnwrap(doc.bus(id)?.ports.first)
        let high = try XCTUnwrap(doc.bus(id)?.wires.last)
        XCTAssertNil(doc.setPortWires(port.id, [high.id]))
        XCTAssertEqual(doc.setBusWireNames(id, ["W1", "W2"]),
                       "W8 is used by a port; this bus needs at least 8 wires.")
        // Every wire on one port of a short line: no room.
        let short = try XCTUnwrap(doc.addBus(from: p(100, 600), to: p(160, 600)))
        XCTAssertNil(doc.addPort(at: p(130, 600), tolerance: 1))
        let shortPort = try XCTUnwrap(doc.bus(short)?.ports.first)
        let all = Set(try XCTUnwrap(doc.bus(short)?.wires.map(\.id)))
        let message = doc.setPortWires(shortPort.id, all)
        XCTAssertEqual(message, "Not enough room on this bus line for \(all.count) pins.")
        // Two ports facing each other: one can't turn to the other's side.
        let wide = try XCTUnwrap(doc.addBus(from: p(100, 900), to: p(500, 900)))
        XCTAssertNil(doc.addPort(at: p(300, 900)))
        let first = try XCTUnwrap(doc.bus(wide)?.ports.first)
        let other = BusPort.sides(for: try XCTUnwrap(doc.bus(wide)?.lines.first)).first { $0 != first.side }!
        XCTAssertNil(doc.setPortSide(first.id, other))
        XCTAssertNil(doc.addPort(at: p(300, 900)))
        let second = try XCTUnwrap(doc.bus(wide)?.ports.first { $0.id != first.id })
        XCTAssertNotEqual(second.side, other, "the default side, since the other one is taken")
        XCTAssertEqual(doc.setPortSide(second.id, other), "A port on that side is in the way.")
    }

    /// A bus linked with another of its name keeps the wires they share: a
    /// join that would change them is refused.
    func testJoiningALinkedBusWithOneOfOtherWiresIsRefused() throws {
        let doc = TestSupport.newDocument()
        let first = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        let second = try XCTUnwrap(doc.addBus(from: p(600, 100), to: p(800, 100)))
        let name = try XCTUnwrap(doc.bus(first)?.name)
        XCTAssertNil(doc.setBusName(second, name, link: true))
        let third = try XCTUnwrap(doc.addBus(from: p(100, 300), to: p(300, 300)))
        var card = BusCard()
        card.type = .custom
        XCTAssertNil(doc.setBusCard(third, card))
        XCTAssertNil(doc.setBusWireNames(third, ["X1", "X2", "X3"]))
        // With a port only on the third bus, the join would take its wires.
        XCTAssertNil(doc.addPort(at: p(200, 300)))
        let before = doc.buses
        XCTAssertNil(doc.addBus(from: p(300, 100), to: p(300, 300)))
        XCTAssertEqual(doc.buses, before)
        XCTAssertEqual(doc.editorMessage,
                       "\(name) is linked with another bus of its name, so joining can't change its wires.")
    }

    // MARK: The card-edge bus

    private func edgeBus(_ doc: SchematicDocument) throws -> UUID {
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        var card = BusCard()
        card.type = .arduinoShield
        XCTAssertNil(doc.setBusCard(id, card))
        doc.setBoardType(.arduinoShield)
        doc.setEdgeBus(id)
        XCTAssertEqual(doc.edgeBusID, id)
        return id
    }

    func testTheCardEdgeBusCantBeDeletedFromTheSelection() throws {
        let doc = TestSupport.newDocument()
        let id = try edgeBus(doc)
        doc.select(.bus(id))
        XCTAssertFalse(doc.deleteSchematicSelection())
        XCTAssertNotNil(doc.bus(id))
        XCTAssertEqual(doc.editorMessage,
                       "\(try XCTUnwrap(doc.bus(id)).name) is in use as the card edge, so it can't be deleted. Set the PCB's Bus to None first.")
    }

    /// The card edge can't link with a bus of another wire count.
    func testTheCardEdgeBusLinksOnlyWithItsWireCount() throws {
        let doc = TestSupport.newDocument()
        let id = try edgeBus(doc)
        let other = try XCTUnwrap(doc.addBus(from: p(100, 500), to: p(300, 500)))
        let name = try XCTUnwrap(doc.bus(other)?.name)
        let count = try XCTUnwrap(doc.bus(id)?.wires.count)
        XCTAssertNotEqual(doc.bus(other)?.wires.count, count)
        XCTAssertEqual(doc.setBusName(id, name, link: true),
                       "\(try XCTUnwrap(doc.bus(id)).name) is in use as the card edge; it can only link with a bus of \(count) wires.")
    }

    /// A bus line asked about for deletion, then made the card edge before
    /// the answer: the confirmed delete keeps it.
    func testConfirmingTheDeleteOfABusThatBecameTheCardEdgeKeepsIt() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        var card = BusCard()
        card.type = .arduinoShield
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertNil(doc.addPort(at: p(200, 100)))
        let port = try XCTUnwrap(doc.bus(id)?.ports.first)
        let wire = try XCTUnwrap(doc.bus(id)?.wires.first)
        XCTAssertNil(doc.setPortWires(port.id, [wire.id]))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 400))
        XCTAssertNil(doc.connect(.busPin(BusPinRef(portID: port.id, wireID: wire.id)), .pin(try TestSupport.pin(led, "A"))))
        doc.erase(at: p(150, 100))
        XCTAssertNotNil(doc.pendingRemoval)
        doc.setBoardType(.arduinoShield)
        doc.setEdgeBus(id)
        doc.confirmPendingRemoval()
        XCTAssertEqual(doc.bus(id)?.lines.count, 1, "the card edge's last line stays")
        XCTAssertTrue(try XCTUnwrap(doc.editorMessage).contains("in use as the card edge"))
    }
}
