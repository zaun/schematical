import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-61-A: one `UndoManager` per document. Every recorded step is one undo
/// group on `SchematicDocument.undoManager`, whichever page made it.
@MainActor
final class DocumentUndoTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// Schematic, PCB, Schematic edits undo in strict reverse order and redo
    /// in order, with the step's name on each side.
    func testUndoAndRedoFollowDocumentOrder() throws {
        let doc = TestSupport.newDocument()
        let s0 = doc.state
        let label = doc.addNetLabel(at: p(100, 100))
        let s1 = doc.state
        doc.setBoardSize(width: 120, height: 90)
        let s2 = doc.state
        XCTAssertTrue(doc.setNetLabelName(label, "CLK"))
        let s3 = doc.state
        XCTAssertEqual(doc.undoName, "Rename Net Label")
        XCTAssertNil(doc.redoName)

        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.state, s2)
        XCTAssertEqual(doc.undoName, "Change Board Size")
        XCTAssertEqual(doc.redoName, "Rename Net Label")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.state, s1)
        XCTAssertEqual(doc.undoName, "Add Net Label")
        XCTAssertEqual(doc.redoName, "Change Board Size")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.state, s0)
        XCTAssertNil(doc.undoName)
        XCTAssertEqual(doc.redoName, "Add Net Label")
        XCTAssertFalse(doc.undo())

        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.state, s1)
        XCTAssertEqual(doc.undoName, "Add Net Label")
        XCTAssertEqual(doc.redoName, "Change Board Size")
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.state, s2)
        XCTAssertEqual(doc.undoName, "Change Board Size")
        XCTAssertEqual(doc.redoName, "Rename Net Label")
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.state, s3)
        XCTAssertEqual(doc.undoName, "Rename Net Label")
        XCTAssertNil(doc.redoName)
        XCTAssertFalse(doc.redo())
    }

    /// Typing merges into the step already registered.
    func testTypingIsOneUndo() throws {
        let doc = TestSupport.newDocument()
        let label = doc.addNetLabel(at: p(100, 100))
        let added = doc.state
        var name = ""
        for character in "CLOCK1" {
            name.append(character)
            XCTAssertTrue(doc.setNetLabelName(label, name))
        }
        XCTAssertEqual(doc.netLabels.first?.name, "CLOCK1")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.state, added)
        XCTAssertEqual(doc.undoName, "Add Net Label")
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.netLabels.first?.name, "CLOCK1")
    }

    /// A drag of five frames is one step.
    func testAGestureIsOneUndo() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let placed = doc.state
        doc.beginGesture(.pcb)
        for frame in 1...5 {
            doc.moveFootprint(chip.id, to: p(CGFloat(10 * frame), 30))
        }
        doc.endGesture()
        XCTAssertNotEqual(doc.state, placed)
        XCTAssertEqual(doc.undoName, "Move")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.state, placed)
        XCTAssertEqual(doc.undoName, "Add Part")
    }

    /// While the simulation runs nothing is undone and the manager says so.
    func testNothingIsUndoneWhilePlaying() throws {
        let doc = TestSupport.newDocument()
        _ = doc.addNetLabel(at: p(100, 100))
        let before = doc.state
        doc.isPlaying = true
        XCTAssertFalse(doc.undoManager.canUndo)
        XCTAssertFalse(doc.undo())
        doc.undoManager.undo()
        XCTAssertEqual(doc.state, before)
        XCTAssertNil(doc.undoName)
        doc.isPlaying = false
        XCTAssertTrue(doc.undoManager.canUndo)
        XCTAssertEqual(doc.undoName, "Add Net Label")
        XCTAssertTrue(doc.undo())
        doc.isPlaying = true
        XCTAssertFalse(doc.undoManager.canRedo)
        XCTAssertFalse(doc.redo())
        doc.isPlaying = false
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.state, before)
    }

    /// A part deleted with no step recorded (while playing): the step that
    /// moved its footprint changes nothing any more, so one undo skips it
    /// and undoes the step under it, and the skipped step leaves the stacks.
    func testAStepWhoseItemIsGoneIsSkipped() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 120, height: 90)
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        doc.setBoardColor(.red)
        doc.moveFootprint(chip.id, to: p(60, 40))
        doc.isPlaying = true
        doc.select(.chip(chip.id))
        _ = doc.deleteSchematicSelection()
        doc.isPlaying = false
        XCTAssertTrue(doc.chips.isEmpty, "the part is deleted with no step")
        let color = doc.board.color
        XCTAssertEqual(doc.undoName, "Move")
        XCTAssertTrue(doc.undo())
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertNotEqual(doc.board.color, color, "the board color, under the skipped move, is undone")
        XCTAssertEqual(doc.undoName, "Add Part")
        XCTAssertEqual(doc.redoName, "Change Board Color", "the skipped move left the stacks")
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.board.color, color)
        XCTAssertFalse(doc.redo())
    }

    /// A save between two typed characters ends the step.
    func testWillSaveEndsATypingStep() throws {
        let doc = TestSupport.newDocument()
        let label = doc.addNetLabel(at: p(100, 100))
        XCTAssertTrue(doc.setNetLabelName(label, "C"))
        doc.willSave()
        XCTAssertTrue(doc.setNetLabelName(label, "CL"))
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.netLabels.first?.name, "C")
        XCTAssertTrue(doc.undo())
        XCTAssertNotEqual(doc.netLabels.first?.name, "C")
        XCTAssertEqual(doc.undoName, "Add Net Label")
    }

    /// Edit › Undo's title and action follow the document, not the page.
    func testMenuTitlesFollowTheDocument() throws {
        let doc = TestSupport.newDocument()
        let target = UndoTarget(document: doc)
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        doc.moveFootprint(chip.id, to: p(60, 40))
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo Move")
        XCTAssertEqual(UndoRouting.route(textIsEditing: false, target: target), .document)
        doc.isPlaying = true
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo")
        doc.isPlaying = false
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(UndoRouting.undoTitle(target), "Undo Add Part")
        XCTAssertEqual(UndoRouting.redoTitle(target), "Redo Move")
    }

    /// A file applied to the document ends its undo history.
    func testApplyingAFileEmptiesTheManager() throws {
        let source = TestSupport.newDocument()
        try TestSupport.place(source, "SN74HC00N", at: p(200, 200))
        let file = SchematicFile(document: source, fileURL: nil)

        let doc = TestSupport.newDocument()
        _ = doc.addNetLabel(at: p(100, 100))
        doc.setBoardSize(width: 120, height: 90)
        XCTAssertTrue(doc.undo())
        XCTAssertNotNil(doc.undoName)
        XCTAssertNotNil(doc.redoName)
        _ = try file.apply(to: doc, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(doc.chips.count, 1)
        XCTAssertFalse(doc.undoManager.canUndo)
        XCTAssertFalse(doc.undoManager.canRedo)
        XCTAssertNil(doc.undoName)
        XCTAssertNil(doc.redoName)
        XCTAssertFalse(doc.undo())
    }
}
