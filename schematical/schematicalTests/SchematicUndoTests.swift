import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-31-B: every Schematic edit is one undo step. One test per row of
/// plan-31-B § Current State, plus the plan-30 cases and gestures.
@MainActor
final class SchematicUndoTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func file(_ doc: SchematicDocument) -> SchematicFile {
        SchematicFile(document: doc, fileURL: nil)
    }

    private func centre(_ chip: PlacedChip) -> CGPoint {
        CGPoint(x: chip.hitRect.midX, y: chip.hitRect.midY)
    }

    private func chip(_ doc: SchematicDocument, _ id: UUID) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.id == id })
    }

    /// Runs `edit`, which must change the file, then checks it is one step
    /// named `name` on `page`: undo gives the file before, redo the file after.
    private func assertStep(_ doc: SchematicDocument, _ name: String, page: EditorPage = .schematic,
                            file: StaticString = #filePath, line: UInt = #line,
                            _ edit: () throws -> Void) rethrows {
        let before = self.file(doc)
        try edit()
        let after = self.file(doc)
        XCTAssertNotEqual(before, after, "the edit changed nothing", file: file, line: line)
        XCTAssertEqual(doc.undoName(on: page), name, file: file, line: line)
        XCTAssertTrue(doc.undo(on: page), file: file, line: line)
        XCTAssertEqual(self.file(doc), before, "undo", file: file, line: line)
        XCTAssertEqual(doc.redoName(on: page), name, file: file, line: line)
        XCTAssertTrue(doc.redo(on: page), file: file, line: line)
        XCTAssertEqual(self.file(doc), after, "redo", file: file, line: line)
    }

    /// Two NAND parts with a wire from the left 1Y to the right 1A.
    private func wiredPair() throws -> (doc: SchematicDocument, left: PlacedChip, right: PlacedChip, wire: UUID) {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let right = try TestSupport.place(doc, "SN74HC00N", at: p(500, 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(left, "1Y")), .pin(try TestSupport.pin(right, "1A"))))
        return (doc, left, right, try XCTUnwrap(doc.wires.last?.id))
    }

    /// A bus line from (100, 100) to (300, 100), one port with `B0` ticked,
    /// and a wire from that pin to a part's 1A (as `LooseEndTests`).
    private func busCircuit() throws -> (doc: SchematicDocument, bus: UUID, port: UUID, wire: UUID) {
        let doc = TestSupport.newDocument()
        let bus = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        XCTAssertNil(doc.addPort(at: p(200, 100)))
        let port = try XCTUnwrap(doc.bus(bus)?.ports.first?.id)
        let b0 = try XCTUnwrap(doc.bus(bus)?.wires.first?.id)
        XCTAssertNil(doc.setPortWires(port, [b0]))
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(260, 400))
        XCTAssertNil(doc.connect(.busPin(BusPinRef(portID: port, wireID: b0)), .pin(try TestSupport.pin(chip, "1A"))))
        return (doc, bus, port, try XCTUnwrap(doc.wires.last?.id))
    }

    // MARK: One test per row

    func testPlaceChip() throws {
        let doc = TestSupport.newDocument()
        try assertStep(doc, "Add Part") { try TestSupport.place(doc, "SN74HC00N", at: p(200, 200)) }
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Part")
        // Undone from the PCB, it leaves the Schematic list too.
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertNil(doc.undoName(on: .schematic))
        XCTAssertEqual(doc.redoName(on: .schematic), "Add Part")
    }

    func testSetPartColor() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let other = try XCTUnwrap(led.part.colors.first { $0 != led.color })
        try assertStep(doc, "Change Color") { doc.setPartColor(led.id, other) }
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Part", "a colour is not on the PCB list")
    }

    func testSetPartGender() throws {
        let doc = TestSupport.newDocument()
        let header = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: p(200, 200))
        let other = try XCTUnwrap(header.part.genders.first { $0 != header.gender })
        try assertStep(doc, "Change Gender") { doc.setPartGender(header.id, other) }
    }

    func testSetHeaderSize() throws {
        let doc = TestSupport.newDocument()
        let header = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: p(200, 200))
        let smaller = try TestSupport.part("GENERIC-HEADER-1X04")
        try assertStep(doc, "Change Size") { doc.setHeaderSize(header.id, to: smaller) }
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Size")
    }

    func testSetPartOhms() throws {
        let doc = TestSupport.newDocument()
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: p(100, 100))
        try assertStep(doc, "Change Resistance") { doc.setPartOhms(pull.id, 4700) }
    }

    func testSetPartFootprint() throws {
        let doc = TestSupport.newDocument()
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: p(100, 100))
        try assertStep(doc, "Change Footprint") { doc.setPartFootprint(pull.id, "RES-0805") }
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Footprint")
    }

    func testRenameChip() throws {
        let doc = TestSupport.newDocument()
        let part = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        try assertStep(doc, "Rename") { XCTAssertNil(doc.renameChip(part.id, prefix: "U", number: 7, page: .schematic)) }
        XCTAssertEqual(doc.undoName(on: .pcb), "Rename")
    }

    func testRenameChipFromThePCBIsOnBoth() throws {
        let doc = TestSupport.newDocument()
        let part = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        try assertStep(doc, "Rename", page: .pcb) {
            XCTAssertNil(doc.renameChip(part.id, prefix: "IC", number: 3, page: .pcb))
        }
        XCTAssertEqual(doc.undoName(on: .schematic), "Rename")
    }

    func testFlipChip() throws {
        let doc = TestSupport.newDocument()
        let part = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        try assertStep(doc, "Flip") { doc.flipChip(part.id) }
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Part")
    }

    func testMoveSchematicGroup() throws {
        let c = try wiredPair()
        c.doc.select(.chip(c.right.id))
        let base = c.doc.schematicGroupBase()
        try assertStep(c.doc, "Move") { c.doc.moveSchematicGroup(base, by: CGSize(width: 40, height: 60)) }
    }

    func testSettleSchematicGroup() throws {
        let c = try wiredPair()
        c.doc.select(.chip(c.right.id))
        let base = c.doc.schematicGroupBase()
        c.doc.moveSchematicGroup(base, by: CGSize(width: 40, height: 60))
        try assertStep(c.doc, "Move") { c.doc.settleSchematicGroup(base) }
    }

    func testAddNote() throws {
        let doc = TestSupport.newDocument()
        try assertStep(doc, "Add Text") { doc.addNote(at: p(100, 100)) }
    }

    func testSetNoteText() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNote(at: p(100, 100))
        try assertStep(doc, "Edit Text") { doc.setNoteText(id, "Clock") }
    }

    func testSetNoteSize() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNote(at: p(100, 100))
        try assertStep(doc, "Change Font Size") { doc.setNoteSize(id, 24) }
    }

    func testSetNoteFramed() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNote(at: p(100, 100))
        try assertStep(doc, "Change Frame") { doc.setNoteFramed(id, false) }
    }

    func testDeleteNote() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNote(at: p(100, 100))
        try assertStep(doc, "Delete Text") { doc.deleteNote(id) }
    }

    func testSetMemoryFile() throws {
        let doc = TestSupport.newDocument()
        let eeprom = try TestSupport.place(doc, "AT28C256-15PU", at: p(300, 300))
        let memory = MemoryFile(url: URL(fileURLWithPath: "/tmp/rom.bin"), bookmark: nil)
        try assertStep(doc, "Change Load File") { doc.setMemoryFile(eeprom.id, memory) }
    }

    func testToggleSwitch() throws {
        let doc = TestSupport.newDocument()
        let dip = try TestSupport.place(doc, try TestSupport.firstDip().name, at: p(200, 200))
        try assertStep(doc, "Flip Switch") { XCTAssertEqual(doc.toggleSwitch(dip.id, position: 1), true) }
    }

    func testConnect() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let right = try TestSupport.place(doc, "SN74HC00N", at: p(500, 200))
        try assertStep(doc, "Add Wire") {
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(left, "1Y")), .pin(try TestSupport.pin(right, "1A"))))
        }
    }

    func testSettleWiresAttachedToPart() throws {
        let c = try wiredPair()
        c.doc.select(.chip(c.right.id))
        c.doc.moveSchematicGroup(c.doc.schematicGroupBase(), by: CGSize(width: 40, height: 60))
        try assertStep(c.doc, "Move") { c.doc.settleWires(attachedTo: c.right.id) }
    }

    func testSettleWiresAttachedToPort() throws {
        let c = try busCircuit()
        let start = try XCTUnwrap(c.doc.port(c.port)?.port.start)
        XCTAssertTrue(c.doc.slidePort(c.port, to: start - 50))
        try assertStep(c.doc, "Move") { c.doc.settleWires(attachedToPort: c.port) }
    }

    func testSettleWiresAttachedToBus() throws {
        let c = try busCircuit()
        let start = try XCTUnwrap(c.doc.port(c.port)?.port.start)
        XCTAssertTrue(c.doc.slidePort(c.port, to: start - 50))
        try assertStep(c.doc, "Move") { c.doc.settleWires(attachedToBus: c.bus) }
    }

    func testAdoptLooseEnds() throws {
        let c = try wiredPair()
        c.doc.erase(at: centre(c.right))
        XCTAssertTrue(c.doc.hasLooseEnd(try XCTUnwrap(c.doc.wires.first)))
        // A new part dropped away from the loose end, then moved onto it.
        let new = try TestSupport.place(c.doc, "SN74HC00N", at: p(500, 600))
        c.doc.select(.chip(new.id))
        c.doc.moveSchematicGroup(c.doc.schematicGroupBase(), by: CGSize(width: 0, height: -400))
        try assertStep(c.doc, "Connect") { c.doc.adoptLooseEnds(chips: [new.id]) }
        XCTAssertFalse(c.doc.hasLooseEnd(try XCTUnwrap(c.doc.wires.first)))
    }

    func testSetWirePath() throws {
        let c = try wiredPair()
        let path = c.doc.path(forWire: c.wire)
        let start = try XCTUnwrap(path.first), end = try XCTUnwrap(path.last)
        let detour = [start, p(start.x + 20, start.y), p(start.x + 20, start.y + 80), p(end.x - 20, start.y + 80),
                      p(end.x - 20, end.y), end]
        try assertStep(c.doc, "Move Wire") { c.doc.setWirePath(c.wire, detour) }
    }

    func testInsertNode() throws {
        let c = try wiredPair()
        let path = c.doc.path(forWire: c.wire)
        let a = path[0], b = path[1]
        try assertStep(c.doc, "Add Bend") { c.doc.insertNode(at: p((a.x + b.x) / 2, (a.y + b.y) / 2)) }
    }

    func testAddBusFromTo() throws {
        let doc = TestSupport.newDocument()
        try assertStep(doc, "Add Bus") { XCTAssertNotNil(doc.addBus(from: p(100, 100), to: p(300, 100))) }
    }

    func testAddBusCandidates() throws {
        let doc = TestSupport.newDocument()
        try assertStep(doc, "Add Bus") { XCTAssertNotNil(doc.addBus(candidates: [[p(100, 100), p(300, 100)]])) }
    }

    func testSetBusWireCount() throws {
        let c = try busCircuit()
        let count = try XCTUnwrap(c.doc.bus(c.bus)?.wires.count)
        try assertStep(c.doc, "Change Bus Width") { XCTAssertNil(c.doc.setBusWireCount(c.bus, count + 2)) }
    }

    func testRenameBusWire() throws {
        let c = try busCircuit()
        let wire = try XCTUnwrap(c.doc.bus(c.bus)?.wires.last?.id)
        try assertStep(c.doc, "Rename Bus Wire") { XCTAssertNil(c.doc.renameBusWire(bus: c.bus, wire: wire, to: "X9")) }
    }

    func testAddPort() throws {
        let doc = TestSupport.newDocument()
        XCTAssertNotNil(doc.addBus(from: p(100, 100), to: p(300, 100)))
        try assertStep(doc, "Add Port") { XCTAssertNil(doc.addPort(at: p(200, 100))) }
    }

    func testSetPortWires() throws {
        let c = try busCircuit()
        let wires = try XCTUnwrap(c.doc.bus(c.bus)?.wires.prefix(2).map(\.id))
        try assertStep(c.doc, "Change Port Pins") { XCTAssertNil(c.doc.setPortWires(c.port, Set(wires))) }
    }

    func testRequestPortWires() throws {
        let c = try busCircuit()
        let wires = try XCTUnwrap(c.doc.bus(c.bus)?.wires.prefix(3).map(\.id))
        try assertStep(c.doc, "Change Port Pins") { XCTAssertNil(c.doc.requestPortWires(c.port, Set(wires))) }
    }

    func testSetPortSide() throws {
        let c = try busCircuit()
        let (bus, port) = try XCTUnwrap(c.doc.port(c.port))
        let line = try XCTUnwrap(bus.line(port.line))
        let other = try XCTUnwrap(BusPort.sides(for: line).first { $0 != port.side })
        try assertStep(c.doc, "Change Port Side") { XCTAssertNil(c.doc.setPortSide(c.port, other)) }
    }

    func testSlidePortTo() throws {
        let c = try busCircuit()
        let start = try XCTUnwrap(c.doc.port(c.port)?.port.start)
        try assertStep(c.doc, "Move Port") { XCTAssertTrue(c.doc.slidePort(c.port, to: start - 50)) }
    }

    func testSlidePortBy() throws {
        let c = try busCircuit()
        let start = try XCTUnwrap(c.doc.port(c.port)?.port.start)
        try assertStep(c.doc, "Move Port") { c.doc.slidePort(c.port, from: start, by: -50) }
    }

    private func shifted(_ lines: [BusLine], by offset: CGSize) -> [BusLine] {
        lines.map { line in
            var line = line
            line.a.x += offset.width; line.b.x += offset.width
            line.a.y += offset.height; line.b.y += offset.height
            return line
        }
    }

    func testSetBusLines() throws {
        let doc = TestSupport.newDocument()
        let bus = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        let lines = try XCTUnwrap(doc.bus(bus)?.lines)
        try assertStep(doc, "Move Bus") { XCTAssertTrue(doc.setBusLines(bus, shifted(lines, by: CGSize(width: 0, height: 40)))) }
    }

    func testDragBus() throws {
        let doc = TestSupport.newDocument()
        let bus = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        let lines = try XCTUnwrap(doc.bus(bus)?.lines)
        try assertStep(doc, "Move Bus") {
            let reached = doc.dragBus(bus, from: .zero, to: CGSize(width: 0, height: 40)) { self.shifted(lines, by: $0) }
            XCTAssertEqual(reached, CGSize(width: 0, height: 40))
        }
    }

    func testConfirmPendingRemoval() throws {
        let c = try busCircuit()
        let port = try XCTUnwrap(c.doc.bus(c.bus)?.ports.first)
        let body = try XCTUnwrap(c.doc.bus(c.bus)?.body(of: port))
        c.doc.erase(at: CGPoint(x: body.midX, y: body.midY))
        XCTAssertNotNil(c.doc.pendingRemoval)
        try assertStep(c.doc, "Delete") { c.doc.confirmPendingRemoval() }
        XCTAssertNotEqual(c.doc.undoName(on: .pcb), "Delete", "ports are Schematic only")
    }

    func testEraseAPart() throws {
        let c = try wiredPair()
        try assertStep(c.doc, "Delete") { c.doc.erase(at: centre(c.right)) }
        XCTAssertEqual(c.doc.undoName(on: .pcb), "Delete")
        // Undone from the PCB, it leaves the Schematic list too.
        XCTAssertTrue(c.doc.undo(on: .pcb))
        XCTAssertNotNil(c.doc.chips.first { $0.id == c.right.id })
        XCTAssertNotEqual(c.doc.undoName(on: .schematic), "Delete")
    }

    func testEraseAWireIsSchematicOnly() throws {
        let c = try wiredPair()
        let path = c.doc.path(forWire: c.wire)
        let a = path[0], b = path[1]
        try assertStep(c.doc, "Delete") { c.doc.erase(at: p((a.x + b.x) / 2, (a.y + b.y) / 2)) }
        XCTAssertEqual(c.doc.undoName(on: .pcb), "Add Part")
    }

    // MARK: plan-30 cases

    /// A part dropped so its pin lands on a loose wire end: the move and the
    /// auto-connect are one step, and undo leaves the end loose again.
    func testDropOntoALooseEndIsOneStep() throws {
        let c = try wiredPair()
        c.doc.erase(at: centre(c.right))
        let loose = c.doc.state
        let new = try TestSupport.place(c.doc, "SN74HC00N", at: p(500, 600))
        let placed = c.doc.state
        c.doc.select(.chip(new.id))
        c.doc.beginGesture(.schematic)
        let base = c.doc.schematicGroupBase()
        c.doc.moveSchematicGroup(base, by: CGSize(width: 0, height: -200))
        c.doc.moveSchematicGroup(base, by: CGSize(width: 0, height: -400))
        c.doc.settleSchematicGroup(base)
        c.doc.endGesture()
        XCTAssertFalse(c.doc.hasLooseEnd(try XCTUnwrap(c.doc.wires.first)), "connected on drop")
        XCTAssertEqual(c.doc.undoName(on: .schematic), "Move")
        XCTAssertTrue(c.doc.undo(on: .schematic))
        XCTAssertEqual(c.doc.state, placed)
        XCTAssertTrue(c.doc.hasLooseEnd(try XCTUnwrap(c.doc.wires.first)))
        XCTAssertEqual(try chip(c.doc, new.id).origin, new.origin)
        XCTAssertEqual(c.doc.undoName(on: .schematic), "Add Part")
        XCTAssertTrue(c.doc.undo(on: .schematic))
        XCTAssertEqual(c.doc.state, loose)
    }

    /// A part with two wires on its pins, erased then undone: both wires end
    /// on its pins again and no loose junction is left.
    func testUndoingABlockDeletePutsItsWiresBack() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let right = try TestSupport.place(doc, "SN74HC00N", at: p(500, 200))
        let y = try TestSupport.pin(left, "1Y")
        let a = try TestSupport.pin(right, "1A"), b = try TestSupport.pin(right, "1B")
        XCTAssertNil(doc.connect(.pin(y), .pin(a)))
        XCTAssertNil(doc.connect(.pin(y), .pin(b)))
        let wires = doc.wires
        doc.erase(at: centre(left))
        XCTAssertTrue(doc.junctions.contains(where: \.isLoose))
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(doc.wires, wires)
        XCTAssertTrue(doc.wires.allSatisfy { $0.start == .pin(y) })
        XCTAssertFalse(doc.junctions.contains(where: \.isLoose))
        XCTAssertNotNil(doc.chips.first { $0.id == left.id })
    }

    // MARK: Coalescing and play mode

    func testNoteTypingCoalescesUntilEditingEnds() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNote(at: p(100, 100))
        doc.setNoteText(id, "C")
        doc.setNoteText(id, "Cl")
        doc.setNoteText(id, "Clo")
        doc.endCoalescing()
        doc.setNoteText(id, "Clock")
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(doc.notes.first?.text, "Clo")
        XCTAssertEqual(doc.undoName(on: .schematic), "Edit Text")
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(doc.notes.first?.text, SchematicNote.placeholder)
        XCTAssertEqual(doc.undoName(on: .schematic), "Add Text")
    }

    func testPlayingSwitchFlipIsNotAStep() throws {
        let doc = TestSupport.newDocument()
        let dip = try TestSupport.place(doc, try TestSupport.firstDip().name, at: p(200, 200))
        doc.isPlaying = true
        XCTAssertEqual(doc.toggleSwitch(dip.id, position: 1), true)
        doc.isPlaying = false
        XCTAssertEqual(doc.undoName(on: .schematic), "Add Part")
    }

    /// The simulation sets `isPlaying` for the run and clears it on Stop.
    func testSimulationSetsPlaying() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, try TestSupport.firstDip().name, at: p(200, 200))
        let simulation = SchematicUndoTests.simulation()
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        XCTAssertTrue(doc.isPlaying)
        simulation.stop()
        XCTAssertFalse(doc.isPlaying)
    }

    /// Kept for the life of the test process, as `PlaySwitchResetTests` does.
    private static var keptSimulations: [SimulationController] = []
    private static func simulation() -> SimulationController {
        let simulation = SimulationController(library: TestSupport.catalog.library)
        keptSimulations.append(simulation)
        return simulation
    }
}
