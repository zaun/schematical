import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-31-A: the undo engine. Phase 1 cases work on hand-built states;
/// Phase 2 cases on real documents.
@MainActor
final class UndoHistoryTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func chip(_ name: String = "SN74HC00N", reference: String = "U1", at origin: CGPoint = CGPoint(x: 100, y: 100),
                      board: BoardPlacement? = BoardPlacement(centre: CGPoint(x: 10, y: 10), rotation: 0)) throws -> PlacedChip {
        let part = try TestSupport.part(name)
        return PlacedChip(reference: reference, part: part, symbol: symbol(for: part), origin: origin, board: board)
    }

    private func wire(_ x: CGFloat) -> Wire {
        Wire(start: .junction(UUID()), end: .junction(UUID()), interior: [p(x, 0)])
    }

    private func trace(_ x: CGFloat) -> BoardTrace {
        BoardTrace(id: UUID(), layer: .top, width: 10, points: [p(x, 0), p(x, 10)])
    }

    // MARK: plan-64 — document info

    func testDocumentInfoDiffsAndAppliesAsOneKey() {
        var info = DocumentInfo()
        info.title = "Clock"
        info.titleBlock = p(100, 200)
        let changes = UndoDiff.changes(from: DocumentState(), to: DocumentState(documentInfo: info))
        XCTAssertEqual(Set(changes.keys), [.documentInfo])
        var state = DocumentState(documentInfo: info)
        XCTAssertTrue(UndoDiff.apply(changes, backwards: true, to: &state))
        XCTAssertEqual(state, DocumentState())
        XCTAssertTrue(UndoDiff.apply(changes, backwards: false, to: &state))
        XCTAssertEqual(state.documentInfo, info)
        XCTAssertTrue(UndoDiff.changes(from: DocumentState(documentInfo: info),
                                       to: DocumentState(documentInfo: info)).isEmpty)
    }

    // MARK: Phase 1 — diff

    func testMovedChipIsPartOnly() throws {
        let c = try chip()
        var moved = c
        moved.origin = p(200, 100)
        let changes = UndoDiff.changes(from: DocumentState(chips: [c]), to: DocumentState(chips: [moved]))
        XCTAssertEqual(Set(changes.keys), [.part(c.id)])
    }

    func testMovedFootprintIsFootprintOnly() throws {
        let c = try chip()
        var moved = c
        moved.board?.centre = p(30, 30)
        let changes = UndoDiff.changes(from: DocumentState(chips: [c]), to: DocumentState(chips: [moved]))
        XCTAssertEqual(Set(changes.keys), [.footprint(c.id)])
    }

    func testRenamedChipIsReferenceOnly() throws {
        let c = try chip()
        var renamed = c
        renamed.reference = "U9"
        let changes = UndoDiff.changes(from: DocumentState(chips: [c]), to: DocumentState(chips: [renamed]))
        XCTAssertEqual(Set(changes.keys), [.reference(c.id)])
    }

    func testAddedChipIsAllThree() throws {
        let c = try chip()
        let changes = UndoDiff.changes(from: DocumentState(), to: DocumentState(chips: [c]))
        XCTAssertEqual(Set(changes.keys), [.part(c.id), .reference(c.id), .footprint(c.id)])
    }

    func testDeletingAnEarlierChipDoesNotChangeLaterOnes() throws {
        let a = try chip(reference: "U1"), b = try chip(reference: "U2")
        let changes = UndoDiff.changes(from: DocumentState(chips: [a, b]), to: DocumentState(chips: [b]))
        XCTAssertEqual(Set(changes.keys), [.part(a.id), .reference(a.id), .footprint(a.id)])
    }

    // MARK: Phase 1 — apply

    /// Every key kind changes at once; backwards gives the old state, forwards
    /// the new one.
    func testApplyRoundTripsEveryKeyKind() throws {
        let kept = try chip(reference: "U1"), gone = try chip(reference: "U2", at: p(300, 100))
        let junction = Junction(point: p(10, 10))
        let bus = Bus(lines: [BusLine(a: p(0, 200), b: p(200, 200))], wires: [BusWire(name: "D0"), BusWire(name: "D1")])
        let note = SchematicNote(id: UUID(), text: "hi", origin: p(0, 300))
        let text = BoardText(id: UUID(), text: "T", anchor: p(5, 5), size: 50)
        let w = wire(0), t = trace(0)
        let old = DocumentState(chips: [kept, gone], wires: [w], junctions: [junction], buses: [bus], notes: [note],
                                nextReference: 3, board: BoardSettings(), texts: [text], traces: [t])
        var new = old
        new.chips[0].origin = p(120, 100)
        new.chips[0].reference = "U7"
        new.chips[0].board?.rotation = 90
        new.chips.remove(at: 1)
        let added = try chip(reference: "U3", at: p(500, 100))
        new.chips.append(added)
        new.wires[0].interior = [p(40, 0)]
        new.junctions[0].point = p(20, 20)
        new.buses[0].wires[0].name = "A0"
        new.notes[0].text = "hello"
        new.nextReference = 4
        new.board.width = 150
        new.texts[0].rotation = 90
        new.traces[0].width = 20
        new.wires.append(wire(50))
        new.traces.append(trace(50))

        let changes = UndoDiff.changes(from: old, to: new)
        let kinds = Set(changes.keys.map { key -> String in "\(key)".components(separatedBy: "(").first! })
        XCTAssertEqual(kinds, ["part", "reference", "footprint", "wire", "junction", "bus", "note",
                               "nextReference", "text", "trace", "board"])
        var state = new
        XCTAssertTrue(UndoDiff.apply(changes, backwards: true, to: &state))
        XCTAssertEqual(state, old)
        XCTAssertTrue(UndoDiff.apply(changes, backwards: false, to: &state))
        XCTAssertEqual(state, new)
    }

    func testDeletedItemsComeBackAtTheirIndex() {
        let wires = [wire(0), wire(1), wire(2)]
        let notes = (0..<3).map { SchematicNote(id: UUID(), text: "\($0)", origin: p(0, CGFloat($0))) }
        let traces = [trace(0), trace(1), trace(2)]
        let old = DocumentState(wires: wires, notes: notes, traces: traces)
        var new = old
        new.wires.remove(at: 1)
        new.notes.remove(at: 1)
        new.traces.remove(at: 1)
        let changes = UndoDiff.changes(from: old, to: new)
        var state = new
        UndoDiff.apply(changes, backwards: true, to: &state)
        XCTAssertEqual(state.wires.map(\.id), wires.map(\.id))
        XCTAssertEqual(state.notes.map(\.id), notes.map(\.id))
        XCTAssertEqual(state.traces.map(\.id), traces.map(\.id))
    }

    /// The array shrank since (an edit recorded no step): the item goes on the end.
    func testDeletedItemGoesOnTheEndWhenTheArrayShrank() {
        let wires = [wire(0), wire(1), wire(2)]
        let traces = [trace(0), trace(1), trace(2)]
        var new = DocumentState(wires: wires, traces: traces)
        new.wires.remove(at: 2)
        new.traces.remove(at: 2)
        let changes = UndoDiff.changes(from: DocumentState(wires: wires, traces: traces), to: new)
        var state = DocumentState(wires: [wires[0]], traces: [traces[0]])
        UndoDiff.apply(changes, backwards: true, to: &state)
        XCTAssertEqual(state.wires.map(\.id), [wires[0].id, wires[2].id])
        XCTAssertEqual(state.traces.map(\.id), [traces[0].id, traces[2].id])
    }

    func testFootprintOfAnAbsentPartIsSkipped() throws {
        let c = try chip()
        var moved = c
        moved.board?.centre = p(40, 40)
        let changes = UndoDiff.changes(from: DocumentState(chips: [c]), to: DocumentState(chips: [moved]))
        var state = DocumentState()
        XCTAssertFalse(UndoDiff.apply(changes, backwards: true, to: &state))
        XCTAssertEqual(state, DocumentState())
    }

    /// Undoing a Schematic move keeps the footprint where the PCB has it now.
    func testPartApplyKeepsTodaysFootprintAndReference() throws {
        let c = try chip()
        var moved = c
        moved.origin = p(300, 300)
        let changes = UndoDiff.changes(from: DocumentState(chips: [c]), to: DocumentState(chips: [moved]))
        var state = DocumentState(chips: [moved])
        state.chips[0].board?.centre = p(70, 70)
        state.chips[0].reference = "U5"
        UndoDiff.apply(changes, backwards: true, to: &state)
        XCTAssertEqual(state.chips[0].origin, c.origin)
        XCTAssertEqual(state.chips[0].board?.centre, p(70, 70))
        XCTAssertEqual(state.chips[0].reference, "U5")
    }

    // MARK: The document's one list (plan-61-A)

    /// A new step ends redo, whichever page made the undone ones.
    func testANewStepClearsRedo() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        doc.flipChip(chip.id)
        doc.rotateFootprint(chip.id)
        XCTAssertTrue(doc.undo())
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.redoName, "Flip")
        doc.setBoardSize(width: 120, height: 90)
        XCTAssertNil(doc.redoName)
        XCTAssertFalse(doc.redo())
        XCTAssertEqual(doc.undoName, "Change Board Size")
    }

    /// How many steps `undo()` takes off before it has nothing left.
    private func undoCount(_ doc: SchematicDocument) -> Int {
        var count = 0
        while doc.undo() { count += 1 }
        return count
    }

    func testCoalescingMergesUntilEnded() {
        let doc = TestSupport.newDocument()
        let a = doc.addNote(at: p(100, 100)), b = doc.addNote(at: p(100, 300))
        doc.setNoteText(a, "C")
        doc.setNoteText(a, "Cl")
        XCTAssertEqual(doc.undoName, "Edit Text")
        doc.endCoalescing()
        doc.setNoteText(a, "Clo")          // a second step: editing ended
        // Another step stops it too.
        doc.setNoteSize(a, 24)
        doc.setNoteText(a, "Cloc")         // a third
        // A different coalescing key is its own step.
        doc.setNoteText(b, "D")            // a fourth
        doc.setNoteText(b, "Da")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.notes.first { $0.id == b }?.text, SchematicNote.placeholder)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.notes.first { $0.id == a }?.text, "Clo")
        XCTAssertEqual(doc.undoName, "Change Font Size")
        XCTAssertTrue(doc.undo())
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.notes.first { $0.id == a }?.text, "Cl")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.notes.first { $0.id == a }?.text, SchematicNote.placeholder)
        XCTAssertEqual(undoCount(doc), 2, "the two Add Text steps are left")
    }

    func testCoalescingBackToTheStartDropsTheStep() {
        let doc = TestSupport.newDocument()
        let id = doc.addNote(at: p(100, 100))
        let added = doc.state
        doc.setNoteText(id, "C")
        XCTAssertEqual(doc.undoName, "Edit Text")
        doc.setNoteText(id, SchematicNote.placeholder)
        XCTAssertEqual(doc.state, added)
        XCTAssertEqual(doc.undoName, "Add Text", "the typing step is gone")
        XCTAssertNil(doc.redoName)
        // The next character starts a new step.
        doc.setNoteText(id, "D")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.state, added)
        XCTAssertEqual(doc.undoName, "Add Text")
    }

    func testThe501stStepDropsTheOldest() {
        let doc = TestSupport.newDocument()
        XCTAssertEqual(doc.undoManager.levelsOfUndo, 500)
        for n in 0...500 {
            doc.setBoardSize(width: CGFloat(60 + n), height: 90)
        }
        XCTAssertEqual(undoCount(doc), 500)
        XCTAssertEqual(doc.board.width, 60, "the first step, to 60, is no longer undoable")
    }

    // MARK: Phase 2 — the document API

    private func file(_ doc: SchematicDocument) -> SchematicFile {
        SchematicFile(document: doc, fileURL: nil)
    }

    func testRecordedFlipUndoesAndRedoes() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        doc.replaceContents(chips: doc.chips, wires: [], junctions: [], buses: [], nextReference: 2,
                            board: doc.board, texts: [], traces: [])
        let before = doc.state, beforeFile = file(doc)
        doc.recordEdit(.schematic, "Flip") { doc.flipChip(chip.id) }
        let after = doc.state
        XCTAssertNotEqual(before, after)
        XCTAssertEqual(doc.undoName, "Flip")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.state, before)
        XCTAssertEqual(file(doc), beforeFile)
        XCTAssertNil(doc.undoName)
        XCTAssertEqual(doc.redoName, "Flip")
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.state, after)
        XCTAssertFalse(doc.redo())
    }

    /// A gesture is one step, named by the first inner edit that changed
    /// something: a zero-delta move changes nothing, the switch flip does.
    func testGestureIsOneStepNamedByTheFirstChange() throws {
        let doc = TestSupport.newDocument()
        let dip = try TestSupport.firstDip()
        let chip = try TestSupport.place(doc, dip.name, at: p(200, 200))
        doc.replaceContents(chips: doc.chips, wires: [], junctions: [], buses: [], nextReference: 2,
                            board: doc.board, texts: [], traces: [])
        doc.select(.chip(chip.id))
        let before = doc.state
        doc.beginGesture(.schematic)
        let base = doc.schematicGroupBase()
        doc.recordEdit(.schematic, "Move") { doc.moveSchematicGroup(base, by: .zero) }
        doc.recordEdit(.schematic, "Flip Switch") { _ = doc.toggleSwitch(chip.id, position: 1) }
        doc.recordEdit(.schematic, "Move") { doc.moveSchematicGroup(base, by: CGSize(width: 40, height: 0)) }
        XCTAssertNil(doc.undoName, "nothing is pushed while the gesture is open")
        doc.endGesture()
        XCTAssertEqual(doc.undoName, "Flip Switch")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.state, before)
        XCTAssertNil(doc.undoName)
    }

    func testEditThatChangesNothingRecordsNothing() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        doc.replaceContents(chips: doc.chips, wires: [], junctions: [], buses: [], nextReference: 2,
                            board: doc.board, texts: [], traces: [])
        doc.recordEdit(.schematic, "Change Color") { doc.setPartColor(chip.id, "red") }
        doc.beginGesture(.schematic)
        doc.endGesture()
        XCTAssertNil(doc.undoName)
        XCTAssertFalse(doc.undo())
    }

    func testPlayingRecordsNothingAndUndoIsOff() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        doc.recordEdit(.schematic, "Flip") { doc.flipChip(chip.id) }
        doc.isPlaying = true
        _ = doc.renameChip(chip.id, prefix: "U", number: 9, page: .schematic)
        let playing = doc.state
        XCTAssertFalse(doc.undo())
        XCTAssertFalse(doc.redo())
        XCTAssertEqual(doc.state, playing)
        doc.isPlaying = false
        // Only the flip is on the list.
        XCTAssertEqual(doc.undoName, "Flip")
        XCTAssertTrue(doc.undo())
        // Under it, the drop (recorded by placeChip since plan-31-B).
        XCTAssertEqual(doc.undoName, "Add Part")
    }

    func testReplaceContentsEmptiesTheUndoManager() throws {
        let doc = TestSupport.newDocument()
        try doc.recordEdit(.schematic, "Add Part") { try TestSupport.place(doc, "SN74HC00N", at: p(200, 200)) }
        doc.setBoardSize(width: 120, height: 90)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.undoName, "Add Part")
        XCTAssertEqual(doc.redoName, "Change Board Size")
        doc.replaceContents(chips: [], wires: [], junctions: [], buses: [], nextReference: 1,
                            board: BoardSettings(), texts: [], traces: [])
        XCTAssertNil(doc.undoName)
        XCTAssertNil(doc.redoName)
        XCTAssertFalse(doc.undoManager.canUndo)
        XCTAssertFalse(doc.undoManager.canRedo)
    }

    /// Undo back to the saved file clears "unsaved": the file compares equal.
    func testUndoRestoresTheSavedFile() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let saved = file(doc)
        _ = doc.renameChip(chip.id, prefix: "U", number: 42, page: .schematic)
        XCTAssertNotEqual(file(doc), saved)
        doc.undo()
        XCTAssertEqual(file(doc), saved)
    }

    func testUndoPrunesASelectionOfAPartThatIsGone() throws {
        let doc = TestSupport.newDocument()
        let chip = try doc.recordEdit(.schematic, "Add Part") { try TestSupport.place(doc, "SN74HC00N", at: p(200, 200)) }
        doc.select(.chip(chip.id))
        XCTAssertTrue(doc.isSelected(.chip(chip.id)))
        XCTAssertTrue(doc.undo())
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertFalse(doc.isSelected(.chip(chip.id)))
        XCTAssertFalse(doc.isSelected(.footprint(chip.id)))
        XCTAssertNil(doc.selectedChipID)
    }

    /// A step whose items are gone is skipped, and the one under it undone.
    /// A part is deleted with no step (while playing): the rotate of its
    /// footprint changes nothing any more.
    func testUndoSkipsAStepThatChangesNothing() throws {
        let doc = TestSupport.newDocument()
        doc.recordEdit(.pcb, "Change Board Size") { doc.setBoardSize(width: 120, height: 90) }
        let chip = try doc.recordEdit(.schematic, "Add Part") { try TestSupport.place(doc, "SN74HC00N", at: p(200, 200)) }
        doc.recordEdit(.pcb, "Change Board Color") { doc.setBoardColor(.red) }
        doc.recordEdit(.pcb, "Rotate") { doc.rotateFootprint(chip.id) }
        doc.isPlaying = true
        doc.select(.chip(chip.id))
        _ = doc.deleteSchematicSelection()
        doc.isPlaying = false
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertEqual(doc.undoName, "Rotate")
        XCTAssertTrue(doc.undo())
        XCTAssertNotEqual(doc.board.color, .red, "the color, under the skipped rotate, is undone")
        XCTAssertEqual(doc.undoName, "Add Part")
        XCTAssertEqual(doc.redoName, "Change Board Color", "the skipped rotate is on neither list")
    }
}
