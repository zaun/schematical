import CoreGraphics
import XCTest
@testable import schematical

/// The `SchematicDocument` lines no other test reached (plan-68-C): the
/// refusals, hit tests and edge cases of the edit API, each checked for what
/// it does and, for edits, that undo puts the document back.
@MainActor
final class SchematicDocumentEdgeTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    // MARK: Box select

    func testBoxSelectTakesJunctionsLabelsAndBoardTexts() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(300, 300))
        let label = doc.addNetLabel(at: p(100, 100))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "A")), .label(label)))
        // A junction where a second wire branches off the first.
        let wire = try XCTUnwrap(doc.wires.first)
        let path = doc.path(for: wire)
        let mid = SchematicGrid.snap(CGPoint(x: (path[0].x + path[1].x) / 2, y: (path[0].y + path[1].y) / 2))
        XCTAssertNil(doc.connect(.point(mid), .pin(try TestSupport.pin(led, "K"))))
        let junction = try XCTUnwrap(doc.junctions.first)
        let items = doc.schematicItems(in: CGRect(x: -1000, y: -1000, width: 3000, height: 3000))
        XCTAssertTrue(items.contains(.junction(junction.id)))
        XCTAssertTrue(items.contains(.label(label)))
        let none = doc.schematicItems(in: CGRect(x: 5000, y: 5000, width: 10, height: 10))
        XCTAssertTrue(none.isEmpty)

        let text = doc.addText(at: p(10, 10))
        let board = doc.boardItems(in: CGRect(x: -500, y: -500, width: 1000, height: 1000))
        XCTAssertTrue(board.contains(.text(text)))
    }

    // MARK: References

    func testRenameChipRefusals() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let b = try TestSupport.place(doc, "SN74HC00N", at: p(500, 200))
        let invalid = doc.renameChip(a.id, prefix: "", number: 1, page: .schematic)
        XCTAssertNotNil(invalid, "an empty prefix is refused")
        XCTAssertEqual(doc.chips.first { $0.id == a.id }?.reference, a.reference)
        let taken = doc.renameChip(a.id, prefix: "U", number: Int(b.reference.dropFirst()), page: .schematic)
        XCTAssertEqual(taken, "\(b.reference) is already used by another part.")
        XCTAssertNil(doc.renameChip(a.id, prefix: "U", number: 9, page: .schematic))
        XCTAssertEqual(doc.chips.first { $0.id == a.id }?.reference, "U9")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.chips.first { $0.id == a.id }?.reference, a.reference)
    }

    /// Two parts at the same place are numbered in their references' order.
    func testResetReferencesBreaksTiesByReference() throws {
        let doc = TestSupport.newDocument()
        let name = "SN74HC00N"
        let first = try TestSupport.place(doc, name, at: p(200, 200))
        let second = try TestSupport.place(doc, name, at: p(200, 200))
        XCTAssertNil(doc.renameChip(first.id, prefix: "U", number: 7, page: .schematic))
        XCTAssertNil(doc.renameChip(second.id, prefix: "U", number: 3, page: .schematic))
        XCTAssertTrue(doc.resetReferences(page: .schematic))
        XCTAssertEqual(doc.chips.first { $0.id == second.id }?.reference, "U1", "U3 sorts before U7")
        XCTAssertEqual(doc.chips.first { $0.id == first.id }?.reference, "U2")
    }

    // MARK: Pins and bus pins

    private func busWithPort(_ doc: SchematicDocument) throws -> (bus: UUID, port: BusPort) {
        let id = try XCTUnwrap(doc.addBus(from: p(100, 400), to: p(400, 400)))
        XCTAssertNil(doc.addPort(at: p(250, 400)))
        let port = try XCTUnwrap(doc.bus(id)?.ports.first)
        let wire = try XCTUnwrap(doc.bus(id)?.wires.first)
        XCTAssertNil(doc.setPortWires(port.id, [wire.id]))
        return (id, try XCTUnwrap(doc.bus(id)?.ports.first))
    }

    func testPinTipsBusPinsAndLineAnchors() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(300, 200))
        let (bus, port) = try busWithPort(doc)
        let refs = doc.busPinRefs
        XCTAssertEqual(refs.count, port.wireIDs.count)
        let ref = try XCTUnwrap(refs.first)
        let tip = try XCTUnwrap(doc.busPinTip(ref))
        XCTAssertEqual(doc.busPin(at: tip), ref)
        XCTAssertNil(doc.busPin(at: p(tip.x + 100, tip.y + 100)))
        let anode = try TestSupport.pin(led, "A")
        let anodeTip = try XCTUnwrap(doc.pinTip(anode))
        XCTAssertTrue(doc.pinTips.contains(anodeTip))
        XCTAssertTrue(doc.pinTips.contains(tip))
        XCTAssertEqual(doc.lineAnchor(at: anodeTip, pinRadius: 4, gridRadius: 4), .pin(anode))
        XCTAssertEqual(doc.lineAnchor(at: tip, pinRadius: 4, gridRadius: 4), .busPin(ref))
        // A grid point on a wire, and one off every wire.
        XCTAssertNil(doc.connect(.pin(anode), .busPin(ref)))
        let wire = try XCTUnwrap(doc.wires.first)
        let path = doc.path(for: wire)
        let onWire = try XCTUnwrap(zip(path, path.dropFirst()).lazy.compactMap { a, b -> CGPoint? in
            let m = SchematicGrid.snap(CGPoint(x: (a.x + b.x) / 2, y: (a.y + b.y) / 2))
            return m != a && m != b && (m.x == a.x && m.x == b.x || m.y == a.y && m.y == b.y) ? m : nil
        }.first)
        XCTAssertEqual(doc.lineAnchor(at: onWire, pinRadius: 1, gridRadius: 4), .point(onWire))
        XCTAssertNil(doc.lineAnchor(at: p(5000, 5000), pinRadius: 1, gridRadius: 4))
        XCTAssertNil(doc.lineAnchor(at: CGPoint(x: onWire.x + 4.5, y: onWire.y + 4.5), pinRadius: 1, gridRadius: 1),
                     "too far from the grid point")
        _ = bus
    }

    /// The bends of a wire are nodes; its ends are not.
    func testNodeHit() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(300, 200))
        let other = try TestSupport.place(doc, "GENERIC-LED", at: p(600, 500))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(try TestSupport.pin(other, "A"))))
        let wire = try XCTUnwrap(doc.wires.first)
        let path = doc.path(for: wire)
        XCTAssertGreaterThan(path.count, 2)
        let hit = try XCTUnwrap(doc.nodeHit(at: path[1]))
        XCTAssertEqual(hit.wire, wire.id)
        XCTAssertEqual(hit.index, 1)
        XCTAssertNil(doc.nodeHit(at: path[0]))
        XCTAssertNil(doc.nodeHit(at: p(5000, 5000)))
    }

    // MARK: Buses

    func testBusEndHit() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        let line = try XCTUnwrap(doc.bus(id)?.lines.first)
        let hit = try XCTUnwrap(doc.busEndHit(at: p(101, 99)))
        XCTAssertEqual(hit.bus, id)
        XCTAssertEqual(hit.line, line.id)
        XCTAssertEqual(line.point(hit.end), p(100, 100))
        XCTAssertNil(doc.busEndHit(at: p(200, 100)), "the middle of a line is not an end")
    }

    func testSetBusLines() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        let line = try XCTUnwrap(doc.bus(id)?.lines.first)
        var moved = line
        moved.a = p(100, 200)
        moved.b = p(300, 200)
        XCTAssertTrue(doc.setBusLines(id, [moved]))
        XCTAssertEqual(doc.bus(id)?.lines.first?.a, p(100, 200))
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.bus(id)?.lines.first?.a, p(100, 100))
        // Onto another bus: refused.
        _ = try XCTUnwrap(doc.addBus(from: p(100, 300), to: p(300, 300)))
        var onto = line
        onto.a = p(100, 300)
        onto.b = p(300, 300)
        XCTAssertFalse(doc.setBusLines(id, [onto]))
        // A line of zero length is not a valid bus.
        var dot = line
        dot.b = dot.a
        XCTAssertFalse(doc.setBusLines(id, [dot]))
    }

    func testAddPortRefusals() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        XCTAssertEqual(doc.addPort(at: p(100, 100)), "A port can't go on the end of a bus line or on a joint.")
        XCTAssertEqual(doc.editorMessage, "A port can't go on the end of a bus line or on a joint.")
        // A port on each side at one spot: a third has nowhere to go.
        XCTAssertNil(doc.addPort(at: p(200, 100)))
        XCTAssertNil(doc.addPort(at: p(200, 100)), "the other side")
        XCTAssertEqual(doc.bus(id)?.ports.count, 2)
        XCTAssertEqual(doc.addPort(at: p(200, 100)), "Not enough room on this bus line for a port.")
        XCTAssertEqual(doc.bus(id)?.ports.count, 2)
    }

    /// Deleting the middle line of a bus splits it in two; each half keeps
    /// its ports and the wires on their pins.
    func testErasingAMiddleBusLineSplitsTheBus() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        XCTAssertEqual(doc.addBus(from: p(300, 100), to: p(300, 300)), id)
        XCTAssertEqual(doc.addBus(from: p(300, 300), to: p(500, 300)), id)
        XCTAssertEqual(doc.bus(id)?.lines.count, 3)
        XCTAssertNil(doc.addPort(at: p(450, 300)))
        let port = try XCTUnwrap(doc.bus(id)?.ports.first)
        let wire = try XCTUnwrap(doc.bus(id)?.wires.first)
        XCTAssertNil(doc.setPortWires(port.id, [wire.id]))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(450, 600))
        let ref = BusPinRef(portID: port.id, wireID: wire.id)
        XCTAssertNil(doc.connect(.busPin(ref), .pin(try TestSupport.pin(led, "A"))))
        let busCount = doc.buses.count
        doc.erase(at: p(300, 200))
        XCTAssertEqual(doc.buses.count, busCount + 1)
        let split = try XCTUnwrap(doc.buses.first { $0.ports.contains { $0.id == port.id } })
        XCTAssertNotEqual(split.id, id)
        let newWire = try XCTUnwrap(split.wires.first { $0.name == wire.name })
        XCTAssertEqual(split.ports.first?.wireIDs, [newWire.id])
        XCTAssertTrue(doc.wires.contains { $0.start == .busPin(BusPinRef(portID: port.id, wireID: newWire.id))
            || $0.end == .busPin(BusPinRef(portID: port.id, wireID: newWire.id)) })
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.buses.count, busCount)
        XCTAssertEqual(doc.bus(id)?.lines.count, 3)
    }

    func testErasingAPortWithoutWiresAndErasingNothing() throws {
        let doc = TestSupport.newDocument()
        let (bus, port) = try busWithPort(doc)
        let before = doc.state
        doc.erase(at: p(5000, 5000))
        XCTAssertEqual(doc.state, before, "nothing there: nothing changes")
        // On the port's body, between the bus line and the pin's tip.
        let tip = try XCTUnwrap(doc.busPinRefs.first.flatMap { doc.busPinTip($0) })
        doc.erase(at: CGPoint(x: tip.x, y: (tip.y + 400) / 2))
        XCTAssertTrue(doc.bus(bus)?.ports.isEmpty ?? false)
        XCTAssertNil(doc.pendingRemoval, "no wire on its pins: no question")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.bus(bus)?.ports.first?.id, port.id)
        // The same through the selection.
        doc.selectedPortID = port.id
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertTrue(doc.bus(bus)?.ports.isEmpty ?? false)
    }

    // MARK: PCB

    func testLayerChangesRefusedForPours() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let inner = try XCTUnwrap(doc.board.copperLayers.first { $0.isInner })
        // A pour goes on a wire layer; inner layers start as planes.
        doc.setLayerType(inner, .wire)
        XCTAssertNotNil(doc.addPour(rect: CGRect(x: 0, y: 0, width: 10, height: 10), layer: inner))
        XCTAssertEqual(doc.canSetLayerCount(2), "Delete or move the pours on \(inner.title) first.")
        XCTAssertEqual(doc.canSetLayerType(inner, .plane), "Move or delete this layer's pours first.")
    }

    func testMoveKeepoutSnapsToTheTraceGrid() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addKeepout(rect: CGRect(x: 0, y: 0, width: 5, height: 5))
        let before = try XCTUnwrap(doc.keepouts.first { $0.id == id }?.rect)
        doc.moveKeepout(id, to: CGPoint(x: 3.3, y: 4.4))
        let rect = try XCTUnwrap(doc.keepouts.first { $0.id == id }?.rect)
        XCTAssertEqual(rect.origin, BoardGrid.snap(CGPoint(x: 3.3, y: 4.4), spacing: doc.board.traceGrid))
        XCTAssertEqual(rect.size, before.size, "only the place changes")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.keepouts.first { $0.id == id }?.rect, before)
    }

    /// A diagonal segment moves by a whole number of trace-grid steps along
    /// its normal.
    func testDraggingADiagonalTraceSegment() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 0, y: 0), CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10)],
                                            layer: .top))
        let base = try XCTUnwrap(doc.traceSegmentBase(id, at: CGPoint(x: 5, y: 5)))
        XCTAssertEqual(base.segment, 0)
        doc.dragTraceSegment(base, by: CGSize(width: -3, height: 3))
        let points = try XCTUnwrap(doc.traces.first { $0.id == id }?.points)
        XCTAssertNotEqual(points, base.points)
        let spacing = doc.board.traceGrid
        let a = base.points[0], b = base.points[1]
        let length = hypot(b.x - a.x, b.y - a.y)
        let normal = CGPoint(x: -(b.y - a.y) / length, y: (b.x - a.x) / length)
        let expected = TraceRoute.slide(base.points, segment: 0,
                                        by: BoardGrid.snap(-3 * normal.x + 3 * normal.y, spacing: spacing),
                                        anchored: base.anchored)
        XCTAssertEqual(points, expected)
    }
}
