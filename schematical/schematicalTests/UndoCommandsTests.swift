import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-31-D: where ⌘Z / ⇧⌘Z go, and the Edit menu's titles.
@MainActor
final class UndoCommandsTests: XCTestCase {
    private func target(_ page: EditorPage) -> (SchematicDocument, UndoTarget) {
        let doc = TestSupport.newDocument()
        return (doc, UndoTarget(document: doc, page: page))
    }

    func testTextEditingGoesToTheResponderChain() {
        let (_, target) = target(.schematic)
        XCTAssertEqual(UndoRouting.route(textIsEditing: true, target: target), .responderChain)
    }

    func testNoSchematicWindowGoesToTheResponderChain() {
        XCTAssertEqual(UndoRouting.route(textIsEditing: false, target: nil), .responderChain)
    }

    func testASchematicWindowGoesToItsPage() {
        let (_, pcb) = target(.pcb)
        XCTAssertEqual(UndoRouting.route(textIsEditing: false, target: pcb), .page(.pcb))
        let (_, schematic) = target(.schematic)
        XCTAssertEqual(UndoRouting.route(textIsEditing: false, target: schematic), .page(.schematic))
    }

    func testUndoTitles() throws {
        let (doc, target) = target(.schematic)
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo")
        XCTAssertEqual(UndoRouting.undoTitle(nil), "Undo")
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        doc.flipChip(chip.id)
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo Flip")
        doc.isPlaying = true
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo")
        doc.isPlaying = false
    }

    /// The title follows the target's page: a Schematic-only step does not
    /// title the PCB, a two-page step titles both.
    func testUndoTitleFollowsThePage() throws {
        let doc = TestSupport.newDocument()
        let schematic = UndoTarget(document: doc, page: .schematic), pcb = UndoTarget(document: doc, page: .pcb)
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(UndoRouting.undoTitle(pcb), "Undo Add Part")
        doc.flipChip(chip.id)
        XCTAssertEqual(UndoRouting.undoTitle(schematic), "Undo Flip")
        XCTAssertEqual(UndoRouting.undoTitle(pcb), "Undo Add Part")
    }

    func testRedoTitleAfterUndo() throws {
        let (doc, target) = target(.schematic)
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        doc.flipChip(chip.id)
        XCTAssertEqual(UndoRouting.redoTitle(target), "Redo")
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(UndoRouting.redoTitle(target), "Redo Flip")
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo Add Part")
    }
}
