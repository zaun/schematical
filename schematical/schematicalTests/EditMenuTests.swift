import AppKit
import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The Edit menu's Select All, Cut and Reset References.
@MainActor
final class EditMenuTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// Selected text that can't be edited (a name in the sidebar) is what
    /// Copy copies; the other items, and Copy with no text selected, stay
    /// the schematic's. A field being edited gets every item.
    func testCopyGoesToSelectedReadOnlyText() {
        let text = NSTextView(frame: NSRect(x: 0, y: 0, width: 100, height: 20))
        text.string = "SN74HC00N"
        text.isEditable = false
        text.isSelectable = true
        XCTAssertFalse(EditMenuItem.copy.goesToText(text))
        text.setSelectedRange(NSRange(location: 0, length: 4))
        XCTAssertTrue(EditMenuItem.copy.goesToText(text))
        for item in EditMenuItem.allCases where item != .copy {
            XCTAssertFalse(item.goesToText(text), "\(item)")
        }
        text.isEditable = true
        text.setSelectedRange(NSRange(location: 0, length: 0))
        for item in EditMenuItem.allCases {
            XCTAssertTrue(item.goesToText(text), "\(item)")
        }
        XCTAssertFalse(EditMenuItem.copy.goesToText(nil))
        XCTAssertFalse(EditMenuItem.copy.goesToText(NSView()))
    }

    func testSelectAllSchematicTakesEverything() throws {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: p(300, 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "K")), .pin(try TestSupport.pin(two, "A"))))
        let label = doc.addNetLabel(at: p(100, 300))
        let note = doc.addNote(at: p(0, 400))

        doc.selectAllSchematic()
        let selected = doc.schematicSelection.items
        XCTAssertTrue(selected.contains(.chip(one.id)))
        XCTAssertTrue(selected.contains(.chip(two.id)))
        XCTAssertTrue(selected.contains(.label(label)))
        XCTAssertTrue(selected.contains(.note(note)))
        XCTAssertTrue(doc.wires.allSatisfy { selected.contains(.wire($0.id)) })
        XCTAssertEqual(doc.wires.count, 1)
        // The parts' footprints are selected on the PCB too.
        XCTAssertTrue(doc.isSelected(.footprint(one.id)))
    }

    func testSelectAllBoardTakesWhatIsShown() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let top = try XCTUnwrap(doc.addTrace(points: [p(5, 5), p(15, 5)], layer: .top))
        let bottom = try XCTUnwrap(doc.addTrace(points: [p(5, 10), p(15, 10)], layer: .bottom))

        doc.selectAllBoard()
        XCTAssertTrue(doc.isSelected(.footprint(led.id)))
        XCTAssertTrue(doc.isSelected(.trace(top)))
        XCTAssertTrue(doc.isSelected(.trace(bottom)))

        doc.clearBoardSelection()
        doc.selectAllBoard(visible: BoardVisibility(hidden: [.copper(.bottom)]))
        XCTAssertTrue(doc.isSelected(.trace(top)))
        XCTAssertFalse(doc.isSelected(.trace(bottom)))
    }

    func testCutBoardTracesDeletesOnlyTraces() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let trace = try XCTUnwrap(doc.addTrace(points: [p(5, 5), p(15, 5)], layer: .top))
        doc.endCoalescing()
        XCTAssertFalse(doc.cutBoardSelection(), "nothing selected")

        doc.selectAllBoard()
        XCTAssertTrue(doc.cutBoardSelection())
        XCTAssertFalse(doc.traces.contains { $0.id == trace })
        XCTAssertTrue(doc.chips.contains { $0.id == led.id })
        XCTAssertEqual(doc.undoName, "Cut")
        XCTAssertTrue(doc.undo())
        XCTAssertTrue(doc.traces.contains { $0.id == trace })
    }

    func testCopyPasteBoardText() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(2)
        let id = doc.addText(at: p(20, 20))
        doc.setText(id, "Hello")
        doc.setTextSize(id, 80)
        doc.setTextRotation(id, 90)
        doc.setTextSide(id, .bottom)
        doc.setLocked(.text(id), true)
        doc.endCoalescing()

        let copied = doc.selectedTexts()
        XCTAssertEqual(copied.map(\.id), [id])
        let offset = BoardClipboard.offset(centring: [], texts: copied, on: p(60, 40), grid: doc.board.traceGrid)
        let ids = doc.pasteBoard(traces: [], texts: copied, offset: offset)
        let pasted = try XCTUnwrap(doc.texts.first { $0.id == ids.first })
        XCTAssertEqual(doc.texts.count, 2)
        XCTAssertNotEqual(pasted.id, id)
        XCTAssertEqual([pasted.text, "\(pasted.size)", "\(pasted.rotation)"], ["Hello", "80", "90"])
        XCTAssertEqual(pasted.side, .bottom)
        XCTAssertFalse(pasted.locked, "a copy is not locked")
        XCTAssertEqual(pasted.boardRect.midX, 60, accuracy: doc.board.grid, "centred on the pointer")
        XCTAssertEqual(pasted.boardRect.midY, 40, accuracy: doc.board.grid)
        XCTAssertEqual(doc.boardSelection.items, [.text(pasted.id)], "the copy is the selection")
        XCTAssertEqual(doc.undoName, "Paste")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.texts.map(\.id), [id])
    }

    func testCopyPasteVia() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(2)
        let id = try XCTUnwrap(doc.addVia(at: p(20, 20)))
        doc.setViaDiameter(id, 1.2)
        doc.setViaDrill(id, 0.6)
        doc.setLocked(.via(id), true)
        doc.endCoalescing()

        let copied = doc.selectedVias()
        XCTAssertEqual(copied.map(\.id), [id])
        let offset = BoardClipboard.offset(centring: [], vias: copied, on: p(60, 40), grid: doc.board.traceGrid)
        let ids = doc.pasteBoard(traces: [], texts: [], vias: copied, offset: offset)
        let pasted = try XCTUnwrap(doc.vias.first { $0.id == ids.first })
        XCTAssertEqual(doc.vias.count, 2)
        XCTAssertNotEqual(pasted.id, id)
        XCTAssertEqual(pasted.drill, 0.6)
        XCTAssertEqual(pasted.diameter, 1.2)
        XCTAssertFalse(pasted.locked, "a copy is not locked")
        XCTAssertEqual(pasted.centre.x, 60, accuracy: doc.board.traceGrid)
        XCTAssertEqual(pasted.centre.y, 40, accuracy: doc.board.traceGrid)
        XCTAssertEqual(doc.boardSelection.items, [.via(pasted.id)])
        XCTAssertEqual(doc.undoName, "Paste")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.vias.map(\.id), [id])
    }

    func testPasteViaRefusedOnOneLayerBoard() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        let copied = [BoardVia(id: UUID(), centre: p(20, 20))]
        XCTAssertEqual(doc.pasteBoard(traces: [], texts: [], vias: copied, offset: .zero), [])
        XCTAssertTrue(doc.vias.isEmpty)
    }

    func testCutBoardSelectionTakesTextsWithTraces() throws {
        let doc = TestSupport.newDocument()
        let text = doc.addText(at: p(20, 20))
        let trace = try XCTUnwrap(doc.addTrace(points: [p(5, 5), p(15, 5)], layer: .top))
        doc.endCoalescing()
        doc.setLayerCount(2)
        let via = try XCTUnwrap(doc.addVia(at: p(30, 30)))
        doc.endCoalescing()
        doc.selectAllBoard()
        XCTAssertTrue(doc.cutBoardSelection())
        XCTAssertFalse(doc.vias.contains { $0.id == via })
        XCTAssertFalse(doc.texts.contains { $0.id == text })
        XCTAssertFalse(doc.traces.contains { $0.id == trace })
        XCTAssertEqual(doc.undoName, "Cut")
    }

    func testCutSchematicIsNamedCut() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        doc.endCoalescing()
        doc.select(.chip(led.id))
        XCTAssertNotNil(doc.schematicCopy())
        XCTAssertTrue(doc.deleteSchematicSelection(named: "Cut"))
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertEqual(doc.undoName, "Cut")
    }

    func testResetReferencesRenumbersInReadingOrder() throws {
        let doc = TestSupport.newDocument()
        // Placed out of reading order, and renamed.
        let lower = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 400))
        let upperRight = try TestSupport.place(doc, "GENERIC-LED", at: p(500, 100))
        let upperLeft = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: p(700, 400))
        let cap = try TestSupport.place(doc, "GENERIC-CAPACITOR", at: p(700, 100))
        XCTAssertNil(doc.renameChip(lower.id, prefix: "D", number: 7, page: .schematic))
        XCTAssertNil(doc.renameChip(upperLeft.id, prefix: "U", number: 4, page: .schematic))
        XCTAssertNil(doc.renameChip(pull.id, prefix: "R", number: 9, page: .schematic))
        doc.endCoalescing()

        XCTAssertTrue(doc.resetReferences(page: .schematic))
        func reference(_ chip: PlacedChip) -> String? { doc.chips.first { $0.id == chip.id }?.reference }
        // LEDs take the standard D prefix (Settings ▸ Reference Defaults).
        XCTAssertEqual(reference(upperLeft), "D1")
        XCTAssertEqual(reference(upperRight), "D2")
        XCTAssertEqual(reference(lower), "D3")
        XCTAssertEqual(reference(pull), "R1")
        XCTAssertEqual(reference(cap), "C1")
        // No part has the Integrated Circuit prefix, so U numbers start over.
        XCTAssertEqual(doc.nextReference, 1)
        XCTAssertEqual(doc.undoName, "Reset References")

        // Again changes nothing, and records nothing.
        XCTAssertFalse(doc.resetReferences(page: .schematic))

        XCTAssertTrue(doc.undo())
        XCTAssertEqual(reference(lower), "D7")
        XCTAssertEqual(reference(pull), "R9")
    }
}

/// ⇧⌘R turns a footprint or board text the other way from ⌘R.
@MainActor
final class RotateBackTests: XCTestCase {
    func testCounterclockwiseUndoesClockwise() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        func rotation() -> Int? { doc.chips.first { $0.id == led.id }?.board?.rotation }
        let start = try XCTUnwrap(rotation())
        doc.rotateFootprint(led.id, counterclockwise: true)
        XCTAssertEqual(rotation(), (start + 270) % 360)
        doc.rotateFootprint(led.id)
        XCTAssertEqual(rotation(), start)
    }
}
