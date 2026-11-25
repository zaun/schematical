import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-31-D: where ⌘Z / ⇧⌘Z go, and the Edit menu's titles.
@MainActor
final class UndoCommandsTests: XCTestCase {
    private func target() -> (SchematicDocument, UndoTarget) {
        let doc = TestSupport.newDocument()
        return (doc, UndoTarget(document: doc))
    }

    func testTextEditingGoesToTheResponderChain() {
        let (_, target) = target()
        XCTAssertEqual(UndoRouting.route(textIsEditing: true, target: target), .responderChain)
    }

    func testNoSchematicWindowGoesToTheResponderChain() {
        XCTAssertEqual(UndoRouting.route(textIsEditing: false, target: nil), .responderChain)
    }

    func testASchematicWindowGoesToItsDocument() {
        let (_, target) = target()
        XCTAssertEqual(UndoRouting.route(textIsEditing: false, target: target), .document)
    }

    func testUndoTitles() throws {
        let (doc, target) = target()
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo")
        XCTAssertEqual(UndoRouting.undoTitle(nil), "Undo")
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        doc.flipChip(chip.id)
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo Flip")
        doc.isPlaying = true
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo")
        doc.isPlaying = false
    }

    /// The title follows the document's last step, whichever page made it.
    func testUndoTitleFollowsTheDocument() throws {
        let (doc, target) = target()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo Add Part")
        doc.flipChip(chip.id)
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo Flip")
        doc.rotateFootprint(chip.id)
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo Rotate")
    }

    func testRedoTitleAfterUndo() throws {
        let (doc, target) = target()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        doc.flipChip(chip.id)
        XCTAssertEqual(UndoRouting.redoTitle(target), "Redo")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(UndoRouting.redoTitle(target), "Redo Flip")
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo Add Part")
    }
}
