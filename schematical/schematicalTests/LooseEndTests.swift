import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-30: loose wire ends. Removing a pin keeps the wires that were on it,
/// ending on a loose junction at the pin's old tip.
@MainActor
final class LooseEndTests: XCTestCase {
    // MARK: Phase A1 — the flag and the file

    /// A loose junction reopens loose; a plain one reopens plain and writes
    /// no `loose` key.
    func testLooseJunctionRoundTrips() throws {
        let doc = TestSupport.newDocument()
        let loose = Junction(point: CGPoint(x: 100, y: 100), isLoose: true)
        let plain = Junction(point: CGPoint(x: 200, y: 100))
        doc.replaceContents(
            chips: [],
            wires: [Wire(start: .junction(loose.id), end: .junction(plain.id), interior: [])],
            junctions: [loose, plain],
            buses: [],
            nextReference: 1,
            board: BoardSettings(),
            texts: [],
            traces: []
        )
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let json = try XCTUnwrap(JSONSerialization.jsonObject(with: data) as? [String: Any])
        let junctions = try XCTUnwrap(json["junctions"] as? [[String: Any]])
        let byID = Dictionary(uniqueKeysWithValues: junctions.map { ($0["id"] as? String ?? "", $0) })
        XCTAssertEqual(byID[loose.id.uuidString]?["loose"] as? Bool, true)
        XCTAssertNotNil(byID[plain.id.uuidString])
        XCTAssertNil(byID[plain.id.uuidString]?["loose"])

        let opened = TestSupport.newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(opened.junctions.first { $0.id == loose.id }?.isLoose, true)
        XCTAssertEqual(opened.junctions.first { $0.id == plain.id }?.isLoose, false)
    }

    // MARK: Phase A2 — removals keep their wires

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func centre(_ chip: PlacedChip) -> CGPoint {
        CGPoint(x: chip.bodyRect.midX, y: chip.bodyRect.midY)
    }

    private func wire(_ doc: SchematicDocument, _ id: UUID) throws -> Wire {
        try XCTUnwrap(doc.wires.first { $0.id == id }, "wire \(id) gone")
    }

    /// The junction `end` is on, which must be loose and at `point`.
    @discardableResult
    private func assertLoose(_ doc: SchematicDocument, _ end: WireEnd, at point: CGPoint,
                             file: StaticString = #filePath, line: UInt = #line) throws -> Junction {
        guard case let .junction(id) = end else {
            XCTFail("end \(end) is not a junction", file: file, line: line)
            throw XCTSkip("not loose")
        }
        let junction = try XCTUnwrap(doc.junctions.first { $0.id == id }, file: file, line: line)
        XCTAssertTrue(junction.isLoose, file: file, line: line)
        XCTAssertEqual(junction.point, point, file: file, line: line)
        return junction
    }

    /// Connects two anchors and returns the new wire.
    private func connect(_ doc: SchematicDocument, _ a: LineAnchor, _ b: LineAnchor) throws -> UUID {
        XCTAssertNil(doc.connect(a, b))
        return try XCTUnwrap(doc.wires.last?.id)
    }

    func testErasingABlockKeepsItsWires() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let right = try TestSupport.place(doc, "SN74HC00N", at: p(500, 200))
        let y = try TestSupport.pin(left, "1Y"), a = try TestSupport.pin(right, "1A")
        let id = try connect(doc, .pin(y), .pin(a))
        let path = doc.path(forWire: id)
        let tip = try XCTUnwrap(doc.pinTip(a))

        doc.erase(at: centre(right))
        XCTAssertFalse(doc.chips.contains { $0.id == right.id })
        XCTAssertEqual(doc.wires.count, 1)
        XCTAssertEqual(doc.path(forWire: id), path)
        let kept = try wire(doc, id)
        XCTAssertEqual(kept.start, .pin(y))
        try assertLoose(doc, kept.end, at: tip)
    }

    func testWiresOnOnePinShareOneLooseEnd() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let right = try TestSupport.place(doc, "SN74HC00N", at: p(500, 200))
        let y = try TestSupport.pin(left, "1Y")
        let first = try connect(doc, .pin(y), .pin(try TestSupport.pin(right, "1A")))
        let second = try connect(doc, .pin(y), .pin(try TestSupport.pin(right, "1B")))
        let tip = try XCTUnwrap(doc.pinTip(y))
        let junctionsBefore = doc.junctions.count

        doc.erase(at: centre(left))
        XCTAssertEqual(doc.wires.count, 2)
        XCTAssertEqual(doc.junctions.count, junctionsBefore + 1)
        let a = try assertLoose(doc, try wire(doc, first).start, at: tip)
        let b = try assertLoose(doc, try wire(doc, second).start, at: tip)
        XCTAssertEqual(a.id, b.id)
    }

    func testShrinkingAHeaderLoosensTheDroppedPins() throws {
        let doc = TestSupport.newDocument()
        let header = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: p(200, 200))
        let other = try TestSupport.place(doc, "SN74HC00N", at: p(500, 200))
        let slot = try XCTUnwrap(header.layout.slots.first { $0.pin.designator == "8" })
        let pin8 = PinRef(chipID: header.id, index: slot.pin.index)
        let tip = try XCTUnwrap(doc.pinTip(pin8))
        let id = try connect(doc, .pin(pin8), .pin(try TestSupport.pin(other, "1A")))

        doc.setHeaderSize(header.id, to: try TestSupport.part("GENERIC-HEADER-1X04"))
        XCTAssertEqual(doc.chips.first { $0.id == header.id }?.partNumber, "GENERIC-HEADER-1X04")
        XCTAssertEqual(doc.wires.count, 1)
        try assertLoose(doc, try wire(doc, id).start, at: tip)
    }

    /// A bus line from (100, 100) to (300, 100), one port with `B0` ticked,
    /// and a wire from that pin to a part's 1A.
    private func busCircuit() throws -> (doc: SchematicDocument, bus: UUID, port: UUID,
                                         wire: UUID, tip: CGPoint) {
        let doc = TestSupport.newDocument()
        let bus = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        XCTAssertNil(doc.addPort(at: p(200, 100)))
        let port = try XCTUnwrap(doc.bus(bus)?.ports.first?.id)
        let b0 = try XCTUnwrap(doc.bus(bus)?.wires.first?.id)
        XCTAssertNil(doc.setPortWires(port, [b0]))
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(260, 400))
        let pin = BusPinRef(portID: port, wireID: b0)
        let tip = try XCTUnwrap(doc.busPinTip(pin))
        let wire = try connect(doc, .busPin(pin), .pin(try TestSupport.pin(chip, "1A")))
        return (doc, bus, port, wire, tip)
    }

    func testDeletingAPortKeepsItsWires() throws {
        let c = try busCircuit()
        let port = try XCTUnwrap(c.doc.bus(c.bus)?.ports.first)
        let body = try XCTUnwrap(c.doc.bus(c.bus)?.body(of: port))
        c.doc.erase(at: CGPoint(x: body.midX, y: body.midY))
        XCTAssertEqual(c.doc.pendingRemoval?.message, "Delete this port? The 1 wire on its pins stay, with loose ends.")
        c.doc.confirmPendingRemoval()
        XCTAssertNil(c.doc.port(c.port))
        XCTAssertEqual(c.doc.wires.count, 1)
        try assertLoose(c.doc, try wire(c.doc, c.wire).start, at: c.tip)
    }

    func testDeletingABusLineKeepsItsPortsWires() throws {
        let c = try busCircuit()
        c.doc.erase(at: p(110, 100))
        XCTAssertEqual(c.doc.pendingRemoval?.message,
                       "Delete this bus line and its 1 port? The 1 wire on their pins stay, with loose ends.")
        c.doc.confirmPendingRemoval()
        XCTAssertNil(c.doc.bus(c.bus))
        XCTAssertEqual(c.doc.wires.count, 1)
        try assertLoose(c.doc, try wire(c.doc, c.wire).start, at: c.tip)
    }

    func testUntickingAPortPinKeepsItsWire() throws {
        let c = try busCircuit()
        XCTAssertNil(c.doc.requestPortWires(c.port, []))
        XCTAssertEqual(c.doc.pendingRemoval?.message, "Remove 1 pin? The 1 wire on them stay, with loose ends.")
        c.doc.cancelPendingRemoval()
        XCTAssertNil(c.doc.setPortWires(c.port, []))
        XCTAssertEqual(c.doc.port(c.port)?.port.wireIDs, [])
        XCTAssertEqual(c.doc.wires.count, 1)
        try assertLoose(c.doc, try wire(c.doc, c.wire).start, at: c.tip)
    }

    func testErasingAWireStillDeletesIt() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let right = try TestSupport.place(doc, "SN74HC00N", at: p(500, 200))
        let id = try connect(doc, .pin(try TestSupport.pin(left, "1Y")), .pin(try TestSupport.pin(right, "1A")))
        let path = doc.path(forWire: id)
        let mid = CGPoint(x: (path[0].x + path[1].x) / 2, y: (path[0].y + path[1].y) / 2)
        doc.erase(at: mid)
        XCTAssertTrue(doc.wires.isEmpty)
        XCTAssertTrue(doc.junctions.isEmpty)
        XCTAssertEqual(doc.chips.count, 2)
    }
}
