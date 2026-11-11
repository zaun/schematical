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
        XCTAssertEqual(pasted.map(\.reference), ["U3", "U4"])
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
        XCTAssertEqual(doc.undoName(on: .schematic), "Paste")
        XCTAssertTrue(doc.undo(on: .schematic))
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
        XCTAssertEqual(doc.chips.map(\.reference), ["U1", "U2", "U3", "U4"])
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
}
