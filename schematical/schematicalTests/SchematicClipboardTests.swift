import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// ⌘C / ⌘V on the Schematic: what a copy takes, and what a paste adds.
@MainActor
final class SchematicClipboardTests: XCTestCase {
    private func newDocument() -> SchematicDocument { TestSupport.newDocument() }

    /// Two LEDs, K of the first wired to A of the second.
    private func wiredPair(_ doc: SchematicDocument) throws -> (one: PlacedChip, two: PlacedChip) {
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "K")), .pin(try TestSupport.pin(two, "A"))))
        return (one, two)
    }

    func testNothingSelectedCopiesNothing() throws {
        let doc = newDocument()
        _ = try wiredPair(doc)
        doc.clearSchematicSelection()
        XCTAssertNil(doc.schematicCopy())
    }

    func testCopyTakesTheWireBetweenCopiedParts() throws {
        let doc = newDocument()
        let (one, two) = try wiredPair(doc)
        doc.select(.chip(one.id))
        doc.toggleSelection(.chip(two.id))
        let copy = try XCTUnwrap(doc.schematicCopy())
        XCTAssertEqual(copy.chips.map(\.id), [one.id, two.id])
        XCTAssertEqual(copy.wires.count, 1)
        XCTAssertTrue(copy.junctions.isEmpty)
        XCTAssertFalse(copy.bounds.isNull)
    }

    func testPasteAddsNewItemsSelectedAndWired() throws {
        let doc = newDocument()
        let (one, two) = try wiredPair(doc)
        let label = doc.addNetLabel(at: CGPoint(x: 100, y: 300))
        XCTAssertTrue(doc.setNetLabelName(label, "CLK"))
        let note = doc.addNote(at: CGPoint(x: 0, y: 400))
        doc.select(.chip(one.id))
        doc.toggleSelection(.chip(two.id))
        doc.toggleSelection(.label(label))
        doc.toggleSelection(.note(note))
        let copy = try XCTUnwrap(doc.schematicCopy())
        doc.endCoalescing()

        XCTAssertTrue(doc.pasteSchematic(copy, offset: CGSize(width: 503, height: 2)))
        XCTAssertEqual(doc.chips.count, 4)
        XCTAssertEqual(doc.wires.count, 2)
        XCTAssertEqual(doc.netLabels.count, 2)
        XCTAssertEqual(doc.notes.count, 2)
        let pasted = Array(doc.chips.suffix(2))
        XCTAssertEqual(Set(doc.chips.map(\.id)).count, 4)
        XCTAssertEqual(pasted.map(\.reference), ["D3", "D4"])
        // Moved by the offset snapped to the grid.
        XCTAssertEqual(pasted[0].origin, CGPoint(x: one.origin.x + 500, y: one.origin.y))
        // Each pasted part has its own spot on the PCB.
        XCTAssertNotNil(pasted[0].board)
        XCTAssertNotEqual(pasted[0].board?.centre, one.board?.centre)
        // The pasted wire joins the pasted parts.
        let wire = try XCTUnwrap(doc.wires.last)
        XCTAssertEqual(wire.start, .pin(try TestSupport.pin(pasted[0], "K")))
        XCTAssertEqual(wire.end, .pin(try TestSupport.pin(pasted[1], "A")))
        // The pasted label keeps its name, so it is on the same net.
        XCTAssertEqual(doc.netLabels.last?.name, "CLK")
        XCTAssertNotEqual(doc.netLabels.last?.id, label)
        // The selection is what was pasted.
        let selected = doc.schematicSelection.items
        XCTAssertTrue(selected.contains(.chip(pasted[0].id)))
        XCTAssertTrue(selected.contains(.wire(wire.id)))
        XCTAssertFalse(selected.contains(.chip(one.id)))

        // One undo step takes it all back.
        XCTAssertEqual(doc.undoName, "Paste")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.chips.count, 2)
        XCTAssertEqual(doc.wires.count, 1)
        XCTAssertEqual(doc.netLabels.count, 1)
        XCTAssertEqual(doc.notes.count, 1)
    }

    func testAWireToAPartNotCopiedPastesWithALooseEnd() throws {
        let doc = newDocument()
        let (one, _) = try wiredPair(doc)
        let wire = try XCTUnwrap(doc.wires.first)
        doc.select(.chip(one.id))
        doc.toggleSelection(.wire(wire.id))
        let copy = try XCTUnwrap(doc.schematicCopy())
        XCTAssertEqual(copy.chips.count, 1)
        XCTAssertEqual(copy.wires.count, 1)
        XCTAssertEqual(copy.junctions.count, 1)
        XCTAssertTrue(copy.junctions[0].isLoose)

        XCTAssertTrue(doc.pasteSchematic(copy, offset: CGSize(width: 0, height: 400)))
        let pasted = try XCTUnwrap(doc.wires.last)
        XCTAssertTrue(doc.hasLooseEnd(pasted))
        // The pasted wire's loose end is selected with it, so the paste drags whole.
        let pastedEnd = try XCTUnwrap(doc.junctions.last)
        XCTAssertTrue(doc.isSelected(.junction(pastedEnd.id)))
        XCTAssertTrue(doc.isSelected(.wire(pasted.id)))
        // The original is untouched.
        XCTAssertEqual(doc.wires.first, wire)
    }

    func testPastingTwiceGivesTwoCopies() throws {
        let doc = newDocument()
        let (one, _) = try wiredPair(doc)
        doc.select(.chip(one.id))
        let copy = try XCTUnwrap(doc.schematicCopy())
        XCTAssertTrue(doc.pasteSchematic(copy, offset: CGSize(width: 0, height: 200)))
        XCTAssertTrue(doc.pasteSchematic(copy, offset: CGSize(width: 0, height: 400)))
        XCTAssertEqual(doc.chips.map(\.reference), ["D1", "D2", "D3", "D4"])
        XCTAssertEqual(Set(doc.chips.map(\.id)).count, 4)
    }

    func testOffsetsCentreOnThePointerOnTheGrid() throws {
        var copy = SchematicCopy()
        copy.bounds = CGRect(x: 100, y: 100, width: 40, height: 20)
        let offset = SchematicClipboard.offset(centring: copy, on: CGPoint(x: 503, y: 297))
        XCTAssertEqual(offset, CGSize(width: 380, height: 190))
        SchematicClipboard.copy(copy)
        XCTAssertEqual(SchematicClipboard.nextOffset(), CGSize(width: 20, height: 20))
        XCTAssertEqual(SchematicClipboard.nextOffset(), CGSize(width: 40, height: 40))
    }

    // MARK: Buses

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// A bus from (100, 100) to (300, 100) with a port carrying B3, whose
    /// pin is wired to an LED's K.
    private func busWiredToLED(_ doc: SchematicDocument) throws -> (bus: UUID, led: PlacedChip) {
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 300))
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        XCTAssertNil(doc.addPort(at: p(200, 100)))
        let port = try XCTUnwrap(doc.bus(id)?.ports.first)
        let b3 = try XCTUnwrap(doc.bus(id)?.wires.first { $0.name == "B3" })
        XCTAssertNil(doc.setPortWires(port.id, [b3.id]))
        XCTAssertNil(doc.connect(.busPin(BusPinRef(portID: port.id, wireID: b3.id)), .pin(try TestSupport.pin(led, "K"))))
        return (id, led)
    }

    func testABusIsASelectionItem() throws {
        let doc = newDocument()
        let (id, led) = try busWiredToLED(doc)
        doc.selectedBusID = id
        XCTAssertEqual(doc.schematicSelection.items, [.bus(id)])
        doc.toggleSelection(.chip(led.id))
        XCTAssertNil(doc.selectedBusID, "not the only thing selected any more")
        XCTAssertTrue(doc.isSelected(.bus(id)))
        doc.selectAllSchematic()
        XCTAssertTrue(doc.isSelected(.bus(id)))
        doc.clearSchematicSelection()
        doc.boxSelectSchematic(CGRect(x: 150, y: 90, width: 10, height: 20), base: [])
        XCTAssertEqual(doc.schematicSelection.items, [.bus(id)])
    }

    func testCopyTakesABusWithItsPortsAndTheWireToACopiedPart() throws {
        let doc = newDocument()
        let (id, led) = try busWiredToLED(doc)
        doc.select(.bus(id))
        doc.toggleSelection(.chip(led.id))
        let copy = try XCTUnwrap(doc.schematicCopy())
        XCTAssertEqual(copy.buses.map(\.id), [id])
        XCTAssertEqual(copy.buses.first?.ports.count, 1)
        XCTAssertEqual(copy.wires.count, 1)
        XCTAssertTrue(copy.junctions.isEmpty, "the wire runs between copied items")
    }

    func testPastedBusIsNewWiredAndSelected() throws {
        let doc = newDocument()
        let (id, led) = try busWiredToLED(doc)
        doc.select(.bus(id))
        doc.toggleSelection(.chip(led.id))
        let copy = try XCTUnwrap(doc.schematicCopy())
        doc.endCoalescing()

        XCTAssertTrue(doc.pasteSchematic(copy, offset: CGSize(width: 0, height: 500)))
        XCTAssertEqual(doc.buses.count, 2)
        let original = try XCTUnwrap(doc.bus(id))
        let pasted = try XCTUnwrap(doc.buses.last)
        XCTAssertNotEqual(pasted.id, id)
        XCTAssertEqual(pasted.lines.first?.a, p(100, 600))
        XCTAssertTrue(Set(pasted.wires.map(\.id)).isDisjoint(with: original.wires.map(\.id)))
        XCTAssertEqual(pasted.wires.map(\.name), original.wires.map(\.name))
        // Same name, same wires: the two link, as labels of one name join.
        XCTAssertEqual(pasted.name, original.name)
        let port = try XCTUnwrap(pasted.ports.first)
        XCTAssertEqual(port.start, try XCTUnwrap(original.ports.first).start, "a horizontal line moved down only")
        XCTAssertEqual(pasted.pinWires(of: port).map(\.name), ["B3"])
        // The copied wire ends on the pasted port's pin.
        let wire = try XCTUnwrap(doc.wires.last)
        XCTAssertTrue([wire.start, wire.end].contains { end in
            if case let .busPin(ref) = end { return ref.portID == port.id }
            return false
        })
        XCTAssertTrue(doc.isSelected(.bus(pasted.id)))
        XCTAssertEqual(doc.schematicSelection.count, 3)

        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.buses.count, 1)
    }

    func testPastedBusInAnotherDocumentKeepsItsName() throws {
        let doc = newDocument()
        let (id, _) = try busWiredToLED(doc)
        XCTAssertNil(doc.setBusName(id, "DATA"))
        doc.select(.bus(id))
        let copy = try XCTUnwrap(doc.schematicCopy())

        let other = newDocument()
        _ = try XCTUnwrap(other.addBus(from: p(0, 0), to: p(0, 200)))
        XCTAssertTrue(other.pasteSchematic(copy, offset: CGSize(width: 100, height: 300)))
        XCTAssertEqual(other.buses.map(\.name), ["BUS 1", "DATA"])
    }

    func testPastedBusWithANameTakenByADifferentBusIsRenamed() throws {
        let doc = newDocument()
        let (id, _) = try busWiredToLED(doc)
        doc.select(.bus(id))
        let copy = try XCTUnwrap(doc.schematicCopy())

        let other = newDocument()
        let theirs = try XCTUnwrap(other.addBus(from: p(0, 0), to: p(0, 200)))
        XCTAssertNil(other.setBusWireCount(theirs, 4))
        XCTAssertTrue(other.pasteSchematic(copy, offset: CGSize(width: 100, height: 300)))
        XCTAssertEqual(other.buses.map(\.name), ["BUS 1", "BUS 2"])
    }

    func testPasteRefusedWhenABusWouldTouchAnother() throws {
        let doc = newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        doc.select(.bus(id))
        let copy = try XCTUnwrap(doc.schematicCopy())
        // Its left end would land on the original's middle.
        XCTAssertFalse(doc.pasteSchematic(copy, offset: CGSize(width: 100, height: 0)))
        XCTAssertEqual(doc.buses.count, 1)
        XCTAssertNotNil(doc.editorMessage)
    }

    func testGroupDragMovesABusWithItsPortAndWire() throws {
        let doc = newDocument()
        let (id, led) = try busWiredToLED(doc)
        doc.select(.bus(id))
        doc.toggleSelection(.chip(led.id))
        let before = try XCTUnwrap(doc.bus(id))
        let base = doc.schematicGroupBase()
        doc.moveSchematicGroup(base, by: CGSize(width: 40, height: 20))
        doc.settleSchematicGroup(base)
        let after = try XCTUnwrap(doc.bus(id))
        XCTAssertEqual(after.lines.first?.a, p(140, 120))
        XCTAssertEqual(after.ports.first?.start, (before.ports.first?.start ?? 0) + 40)
        let port = try XCTUnwrap(after.ports.first)
        let b3 = try XCTUnwrap(after.wires.first { $0.name == "B3" })
        let tip = try XCTUnwrap(after.pinTip(port, wire: b3.id))
        let wire = try XCTUnwrap(doc.wires.first)
        let path = doc.path(forWire: wire.id)
        XCTAssertTrue(path.first == tip || path.last == tip, "the wire still ends on the moved pin")
    }

    func testDeleteRemovesASelectedBusAndLoosensItsWires() throws {
        let doc = newDocument()
        let (id, _) = try busWiredToLED(doc)
        doc.select(.bus(id))
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertTrue(doc.buses.isEmpty)
        XCTAssertEqual(doc.wires.count, 1, "the wire stays")
        XCTAssertTrue(doc.hasLooseEnd(try XCTUnwrap(doc.wires.first)))
    }
}
