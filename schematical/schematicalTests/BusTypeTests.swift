import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// A bus's name ("BUS #") and type (Custom, or a card whose pins set its
/// wires).
@MainActor
final class BusTypeTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func bus(_ doc: SchematicDocument, _ id: UUID) throws -> Bus {
        try XCTUnwrap(doc.bus(id))
    }

    func testNewBusesAreNumbered() throws {
        let doc = TestSupport.newDocument()
        let first = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        let second = try XCTUnwrap(doc.addBus(from: p(100, 400), to: p(300, 400)))
        XCTAssertEqual(try bus(doc, first).name, "BUS 1")
        XCTAssertEqual(try bus(doc, second).name, "BUS 2")
        // Extending a bus keeps its name.
        XCTAssertEqual(doc.addBus(from: p(300, 100), to: p(500, 100)), first)
        XCTAssertEqual(try bus(doc, first).name, "BUS 1")
        // The lowest free number is reused.
        XCTAssertNil(doc.setBusName(first, "Data"))
        let third = try XCTUnwrap(doc.addBus(from: p(100, 700), to: p(300, 700)))
        XCTAssertEqual(try bus(doc, third).name, "BUS 1")
    }

    func testRenaming() throws {
        let doc = TestSupport.newDocument()
        let first = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        let second = try XCTUnwrap(doc.addBus(from: p(100, 400), to: p(300, 400)))
        XCTAssertNil(doc.setBusName(first, "  ISA slot  "))
        XCTAssertEqual(try bus(doc, first).name, "ISA slot")
        XCTAssertEqual(doc.undoName(on: .schematic), "Rename Bus")
        XCTAssertEqual(doc.setBusName(second, "isa SLOT"), "isa SLOT is already used by another bus.")
        XCTAssertEqual(doc.setBusName(second, " "), "A bus name can't be empty.")
        XCTAssertEqual(try bus(doc, second).name, "BUS 2")
    }

    func testCardTypeSetsTheWires() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        let before = try bus(doc, id).wires
        XCTAssertEqual(before.count, 8)

        var card = BusCard()
        card.type = .isa8
        XCTAssertNil(doc.setBusCard(id, card))
        let isa = try bus(doc, id)
        XCTAssertEqual(isa.wires.count, 62)
        XCTAssertEqual(Array(isa.wires.map(\.name).prefix(3)), ["A1", "A2", "A3"])
        XCTAssertEqual(isa.wires[30].name, "A31")
        XCTAssertEqual(isa.wires[31].name, "B1")
        XCTAssertEqual(isa.wires.last?.name, "B31")
        XCTAssertEqual(Array(isa.wires.prefix(8).map(\.id)), before.map(\.id), "existing wires keep their ids")
        XCTAssertEqual(doc.undoName(on: .schematic), "Change Bus Type")

        // The type sets the wires: no count or name edits.
        XCTAssertNotNil(doc.setBusWireCount(id, 10))
        XCTAssertNotNil(doc.renameBusWire(bus: id, wire: isa.wires[0].id, to: "X"))
        XCTAssertEqual(try bus(doc, id).wires.count, 62)
        XCTAssertEqual(try bus(doc, id).wires[0].name, "A1")

        // Options: a PCI Express x4 card has 32 pins a side.
        card.type = .pcie
        card.cardOptions.pcieLanes = 4
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertEqual(try bus(doc, id).wires.count, 64)

        // Numbered cartridges: numbers in order.
        card.type = .nes
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertEqual(try bus(doc, id).wires.map(\.name), (1...72).map(String.init))

        // The Atari 8-bit: numbers, then letters.
        card.type = .atari8bit
        XCTAssertNil(doc.setBusCard(id, card))
        let atari = try bus(doc, id).wires.map(\.name)
        XCTAssertEqual(Array(atari.prefix(15)), (1...15).map(String.init))
        XCTAssertEqual(Array(atari.suffix(15)), ["A", "B", "C", "D", "E", "F", "H", "J", "K", "L", "M", "N", "P", "R", "S"])

        // Back to Custom: the wires stay, and can be edited again.
        card.type = .custom
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertEqual(try bus(doc, id).wires.count, 30)
        XCTAssertNil(doc.setBusWireCount(id, 12))
        XCTAssertEqual(try bus(doc, id).wires.count, 12)

        // Undo goes back a step at a time.
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(try bus(doc, id).card.type, .atari8bit)
    }

    func testCustomCardType() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        var card = BusCard()
        card.type = .customCard
        card.customCard.pads = 10
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertEqual(try bus(doc, id).wires.count, 20)
        card.customCard.bothSides = false
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertEqual(try bus(doc, id).wires.map(\.name), (1...10).map { "A\($0)" })
        card.customCard.pads = 1
        XCTAssertEqual(doc.setBusCard(id, card), "A bus needs 2 to 999 wires; this card has 1 pins.")
        XCTAssertEqual(try bus(doc, id).wires.count, 10)
    }

    /// A card with fewer pins than a port's highest wire is refused.
    func testPortsKeepTheirWires() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(600, 100)))
        XCTAssertNil(doc.setBusWireCount(id, 50))
        XCTAssertNil(doc.addPort(at: p(300, 100)))
        let port = try XCTUnwrap(try bus(doc, id).ports.first)
        let wire40 = try bus(doc, id).wires[40]
        XCTAssertNil(doc.setPortWires(port.id, [wire40.id]))

        var card = BusCard()
        card.type = .atari2600
        XCTAssertEqual(doc.setBusCard(id, card), "\(wire40.name) is used by a port; this card has only 24 pins.")
        XCTAssertEqual(try bus(doc, id).card.type, .custom)
        card.type = .isa8
        XCTAssertNil(doc.setBusCard(id, card))
        let after = try bus(doc, id)
        XCTAssertEqual(after.wires[40].id, wire40.id, "the port keeps its wire, renamed")
        XCTAssertEqual(after.wires[40].name, "B10")
    }

    func testFile() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        XCTAssertNil(doc.setBusName(id, "Slot 1"))
        var card = BusCard()
        card.type = .m2
        card.cardOptions.m2Key = .e
        XCTAssertNil(doc.setBusCard(id, card))
        let plain = try XCTUnwrap(doc.addBus(from: p(100, 400), to: p(300, 400)))

        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let opened = TestSupport.newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(opened.bus(id)?.name, "Slot 1")
        XCTAssertEqual(opened.bus(id)?.card, doc.bus(id)?.card)
        XCTAssertEqual(opened.bus(id)?.wires, doc.bus(id)?.wires)
        XCTAssertEqual(opened.bus(plain)?.name, "BUS 1")
        XCTAssertEqual(opened.bus(plain)?.card.type, .custom)

        // Older files have no names: numbered in order. A type whose pins
        // don't match the wires is dropped.
        var file = try SchematicFile.decode(data)
        file.buses?[0].name = nil
        file.buses?[1].name = nil
        file.buses?[1].type = "isa8"
        let older = TestSupport.newDocument()
        try SchematicFile.decode(try file.encoded()).apply(to: older, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(older.buses.map(\.name), ["BUS 1", "BUS 2"])
        XCTAssertEqual(older.bus(plain)?.card.type, .custom)

        // Two buses with one name are refused.
        file.buses?[0].name = "A"
        file.buses?[1].name = "a"
        XCTAssertThrowsError(try SchematicFile.decode(try file.encoded())
            .apply(to: TestSupport.newDocument(), catalog: TestSupport.catalog, fileURL: nil))
    }

    // MARK: The card edge's bus

    func testEdgeBusChoicesLockAndDelete() throws {
        let doc = TestSupport.newDocument()
        let isaBus = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        XCTAssertNil(doc.setBusWireCount(isaBus, 62))
        let other = try XCTUnwrap(doc.addBus(from: p(100, 400), to: p(300, 400)))

        // A plain board offers nothing.
        XCTAssertTrue(doc.edgeBusChoices.isEmpty)
        doc.setBoardType(.isa8)
        XCTAssertEqual(doc.board.pinCount, 62)
        XCTAssertEqual(doc.edgeBusChoices.map(\.id), [isaBus], "only the count matters")
        doc.setEdgeBus(other)
        XCTAssertNil(doc.edgeBusID, "the wrong count is ignored")
        doc.setEdgeBus(isaBus)
        XCTAssertEqual(doc.edgeBusID, isaBus)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Card Bus")

        // Its count is locked; its names aren't.
        XCTAssertEqual(doc.setBusWireCount(isaBus, 64), "BUS 1 is in use as the card edge; its wire count is locked.")
        let wire = try bus(doc, isaBus).wires[0]
        XCTAssertNil(doc.renameBusWire(bus: isaBus, wire: wire.id, to: "GND"))
        var card = BusCard()
        card.type = .isa16
        XCTAssertNotNil(doc.setBusCard(isaBus, card), "a type with another count")
        card.type = .isa8
        XCTAssertNil(doc.setBusCard(isaBus, card), "a type with the same count")
        XCTAssertNotNil(doc.renameBusWire(bus: isaBus, wire: wire.id, to: "X"), "the type locks the names")
        card.type = .custom
        XCTAssertNil(doc.setBusCard(isaBus, card))
        XCTAssertNil(doc.renameBusWire(bus: isaBus, wire: wire.id, to: "X"), "Custom frees them again")
        XCTAssertNotNil(doc.setBusWireCount(isaBus, 64), "the count stays locked by the card edge")

        // It can't be erased.
        doc.erase(at: p(200, 100))
        XCTAssertNotNil(doc.bus(isaBus))
        XCTAssertEqual(doc.editorMessage,
                       "BUS 1 is in use as the card edge, so it can't be deleted. Set the PCB's Bus to None first.")
        // Another can.
        doc.erase(at: p(200, 400))
        XCTAssertNil(doc.bus(other))

        // A different card count sets the link back to None, for good.
        doc.setBoardType(.pci)
        XCTAssertNil(doc.edgeBusID)
        XCTAssertNil(doc.board.edgeBus, "no hidden link")
        doc.setBoardType(.isa8)
        XCTAssertNil(doc.edgeBusID, "not brought back")
        // Undo brings back the card and its link together.
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.edgeBusID, isaBus)
        doc.setEdgeBus(nil)
        XCTAssertNil(doc.board.edgeBus)
        XCTAssertNil(doc.setBusWireCount(isaBus, 10))
        doc.erase(at: p(200, 100))
        XCTAssertNil(doc.bus(isaBus))
    }

    func testEdgeBusFile() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        XCTAssertNil(doc.setBusWireCount(id, 62))
        doc.setBoardType(.isa8)
        doc.setEdgeBus(id)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.edgeBusID, id)
        // Not written while it doesn't count.
        doc.setBoardType(.custom)
        XCTAssertNil(try SchematicFile.decode(SchematicFile(document: doc, fileURL: nil).encoded()).board?.edgeBus)
    }

    /// The card edge is a hidden port on its bus: a finger whose bus wire is
    /// wired to a part's pin gets a red line to that pin's pad, until a
    /// trace joins them.
    func testFingersGetRatsnestLines() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(600, 600))
        let k = try TestSupport.pin(led, "K")
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        var card = BusCard()
        card.type = .isa8
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertNil(doc.addPort(at: p(200, 100)))
        let port = try XCTUnwrap(doc.bus(id)?.ports.first)
        let b3 = try XCTUnwrap(doc.bus(id)?.wires.first { $0.name == "B3" })
        XCTAssertNil(doc.setPortWires(port.id, [b3.id]))
        XCTAssertNil(doc.connect(.busPin(BusPinRef(portID: port.id, wireID: b3.id)), .pin(k)))
        XCTAssertTrue(doc.ratsnest().isEmpty, "no card edge yet")

        doc.setBoardType(.isa8)
        doc.setEdgeBus(id)
        let finger = try XCTUnwrap(doc.board.fingers.first { $0.name == "B3" })
        XCTAssertEqual(finger.side, .bottom)
        let placed = try XCTUnwrap(doc.chips.first { $0.id == led.id })
        let designator = placed.part.pins[k.index].designator
        let pad = try XCTUnwrap(placed.boardPads.first { $0.designator == designator })
        let lines = doc.ratsnest()
        XCTAssertEqual(lines.count, 1)
        let line = try XCTUnwrap(lines.first)
        let ends = [line.0, line.1]
        XCTAssertTrue(ends.contains { abs($0.x - finger.rect.midX) < 1e-6 && abs($0.y - finger.rect.midY) < 1e-6 },
                      "\(lines)")
        XCTAssertTrue(ends.contains { abs($0.x - pad.rect.midX) < 1e-6 && abs($0.y - pad.rect.midY) < 1e-6 })

        // Wire names don't matter, only the order: renaming keeps the line.
        card.type = .custom
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertNil(doc.renameBusWire(bus: id, wire: b3.id, to: "IRQ"))
        XCTAssertEqual(doc.ratsnest().count, 1)

        // A Bottom trace from the finger to the pad routes it (the LED's
        // pads go through, so Bottom reaches them).
        guard pad.drill > 0 else { return }
        XCTAssertNotNil(doc.addTrace(points: [finger.contact, CGPoint(x: finger.contact.x, y: pad.rect.midY),
                                              CGPoint(x: pad.rect.midX, y: pad.rect.midY)], layer: .bottom))
        XCTAssertTrue(doc.ratsnest().isEmpty)
    }
}
