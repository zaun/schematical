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

    /// A step holding the diff from `a` to `b`, on the pages its keys own
    /// plus `page`.
    private func step(_ name: String, _ a: DocumentState, _ b: DocumentState, page: EditorPage = .schematic,
                      coalescing: String? = nil) -> UndoStep {
        let changes = UndoDiff.changes(from: a, to: b)
        let pages = changes.keys.reduce(into: Set([page])) { $0.formUnion($1.pages) }
        return UndoStep(name: name, coalescing: coalescing, changes: changes, pages: pages)
    }

    /// A one-key step on `pages` (for the stack cases).
    private func bare(_ name: String, pages: Set<EditorPage>, coalescing: String? = nil,
                      before: Int = 1, after: Int = 2) -> UndoStep {
        UndoStep(name: name, coalescing: coalescing,
                 changes: [.nextReference: UndoChange(before: .nextReference(before), after: .nextReference(after))],
                 pages: pages)
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

    /// The array shrank since (another page's edit): the item goes on the end.
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

    // MARK: Phase 1 — stacks

    func testTwoPageStepUndoneFromOnePageLeavesBoth() {
        var history = UndoHistory()
        let shared = bare("Delete", pages: [.schematic, .pcb])
        history.push(shared)
        XCTAssertTrue(history.undoTop(on: .pcb) === shared)
        let popped = history.popUndo(on: .schematic)
        XCTAssertTrue(popped === shared)
        history.pushRedo(shared)
        XCTAssertNil(history.undoTop(on: .schematic))
        XCTAssertNil(history.undoTop(on: .pcb))
        XCTAssertTrue(history.redoTop(on: .schematic) === shared)
        XCTAssertTrue(history.redoTop(on: .pcb) === shared)
        // Redo from the other page puts it back on both.
        XCTAssertTrue(history.popRedo(on: .pcb) === shared)
        history.pushUndo(shared)
        XCTAssertNil(history.redoTop(on: .schematic))
        XCTAssertTrue(history.undoTop(on: .schematic) === shared)
        XCTAssertTrue(history.undoTop(on: .pcb) === shared)
    }

    func testNewSchematicStepClearsOnlySchematicRedo() {
        var history = UndoHistory()
        let s = bare("Move", pages: [.schematic]), b = bare("Move", pages: [.pcb])
        history.push(s)
        history.push(b)
        history.pushRedo(history.popUndo(on: .schematic)!)
        history.pushRedo(history.popUndo(on: .pcb)!)
        history.push(bare("Flip", pages: [.schematic]))
        XCTAssertNil(history.redoTop(on: .schematic))
        XCTAssertTrue(history.redoTop(on: .pcb) === b)
    }

    func testNewSchematicStepClearsASharedRedoFromBothPages() {
        var history = UndoHistory()
        let pcbOnly = bare("Move", pages: [.pcb]), shared = bare("Delete", pages: [.schematic, .pcb])
        history.push(pcbOnly)
        history.push(shared)
        history.pushRedo(history.popUndo(on: .pcb)!)   // shared
        history.pushRedo(history.popUndo(on: .pcb)!)   // pcbOnly
        history.push(bare("Flip", pages: [.schematic]))
        XCTAssertEqual(history.redo[.pcb]?.map(\.name), ["Move"])
        XCTAssertTrue(history.redoTop(on: .pcb) === pcbOnly)
        XCTAssertNil(history.redoTop(on: .schematic))
    }

    func testCoalescingMergesUntilEnded() {
        var history = UndoHistory()
        history.push(bare("Edit Text", pages: [.schematic], coalescing: "k", before: 1, after: 2))
        history.push(bare("Edit Text", pages: [.schematic], coalescing: "k", before: 2, after: 3))
        XCTAssertEqual(history.undo[.schematic]?.count, 1)
        XCTAssertEqual(history.undoTop(on: .schematic)?.changes[.nextReference],
                       UndoChange(before: .nextReference(1), after: .nextReference(3)))
        history.endCoalescing()
        history.push(bare("Edit Text", pages: [.schematic], coalescing: "k", before: 3, after: 4))
        XCTAssertEqual(history.undo[.schematic]?.count, 2)
        // Another key stops it too.
        history.push(bare("Move", pages: [.schematic], before: 4, after: 5))
        history.push(bare("Edit Text", pages: [.schematic], coalescing: "k", before: 5, after: 6))
        XCTAssertEqual(history.undo[.schematic]?.count, 4)
        // A different coalescing key is its own step.
        history.push(bare("Edit Text", pages: [.schematic], coalescing: "other", before: 6, after: 7))
        XCTAssertEqual(history.undo[.schematic]?.count, 5)
    }

    func testCoalescingBackToTheStartDropsTheStep() {
        var history = UndoHistory()
        history.push(bare("Edit Text", pages: [.schematic], coalescing: "k", before: 1, after: 2))
        history.push(bare("Edit Text", pages: [.schematic], coalescing: "k", before: 2, after: 1))
        XCTAssertNil(history.undoTop(on: .schematic))
    }

    func testThe501stPushDropsTheOldest() {
        var history = UndoHistory()
        let first = bare("First", pages: [.schematic, .pcb])
        history.push(first)
        for n in 0..<UndoHistory.limit {
            history.push(bare("S\(n)", pages: [.schematic]))
        }
        XCTAssertEqual(history.undo[.schematic]?.count, UndoHistory.limit)
        XCTAssertEqual(history.undo[.schematic]?.first?.name, "S0")
        // Still on the PCB list, now that page's alone.
        XCTAssertTrue(history.undoTop(on: .pcb) === first)
        XCTAssertEqual(first.pages, [.pcb])
    }

    func testClearEmptiesEverything() {
        var history = UndoHistory()
        history.push(bare("A", pages: [.schematic, .pcb]))
        history.pushRedo(history.popUndo(on: .pcb)!)
        history.push(bare("B", pages: [.pcb]))
        history.clear()
        for page in EditorPage.allCases {
            XCTAssertNil(history.undoTop(on: page))
            XCTAssertNil(history.redoTop(on: page))
        }
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
        XCTAssertEqual(doc.undoName(on: .schematic), "Flip")
        XCTAssertNil(doc.undoName(on: .pcb))
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(doc.state, before)
        XCTAssertEqual(file(doc), beforeFile)
        XCTAssertNil(doc.undoName(on: .schematic))
        XCTAssertEqual(doc.redoName(on: .schematic), "Flip")
        XCTAssertTrue(doc.redo(on: .schematic))
        XCTAssertEqual(doc.state, after)
        XCTAssertFalse(doc.redo(on: .schematic))
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
        XCTAssertNil(doc.undoName(on: .schematic), "nothing is pushed while the gesture is open")
        doc.endGesture()
        XCTAssertEqual(doc.undoName(on: .schematic), "Flip Switch")
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(doc.state, before)
        XCTAssertNil(doc.undoName(on: .schematic))
    }

    func testEditThatChangesNothingRecordsNothing() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        doc.replaceContents(chips: doc.chips, wires: [], junctions: [], buses: [], nextReference: 2,
                            board: doc.board, texts: [], traces: [])
        doc.recordEdit(.schematic, "Change Color") { doc.setPartColor(chip.id, "red") }
        doc.beginGesture(.schematic)
        doc.endGesture()
        XCTAssertNil(doc.undoName(on: .schematic))
        XCTAssertFalse(doc.undo(on: .schematic))
    }

    func testPlayingRecordsNothingAndUndoIsOff() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        doc.recordEdit(.schematic, "Flip") { doc.flipChip(chip.id) }
        doc.isPlaying = true
        _ = doc.renameChip(chip.id, prefix: "U", number: 9, page: .schematic)
        let playing = doc.state
        XCTAssertFalse(doc.undo(on: .schematic))
        XCTAssertFalse(doc.redo(on: .schematic))
        XCTAssertEqual(doc.state, playing)
        doc.isPlaying = false
        // Only the flip is on the list.
        XCTAssertEqual(doc.undoName(on: .schematic), "Flip")
        XCTAssertTrue(doc.undo(on: .schematic))
        // Under it, the drop (recorded by placeChip since plan-31-B).
        XCTAssertEqual(doc.undoName(on: .schematic), "Add Part")
    }

    func testReplaceContentsEmptiesBothLists() throws {
        let doc = TestSupport.newDocument()
        try doc.recordEdit(.schematic, "Add Part") { try TestSupport.place(doc, "SN74HC00N", at: p(200, 200)) }
        XCTAssertEqual(doc.undoName(on: .schematic), "Add Part")
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Part", "adding a part is on both lists")
        doc.replaceContents(chips: [], wires: [], junctions: [], buses: [], nextReference: 1,
                            board: BoardSettings(), texts: [], traces: [])
        for page in EditorPage.allCases {
            XCTAssertNil(doc.undoName(on: page))
            XCTAssertNil(doc.redoName(on: page))
        }
    }

    /// Undo back to the saved file clears "unsaved": the file compares equal.
    func testUndoRestoresTheSavedFile() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let saved = file(doc)
        _ = doc.renameChip(chip.id, prefix: "U", number: 42, page: .schematic)
        XCTAssertNotEqual(file(doc), saved)
        doc.undo(on: .pcb)
        XCTAssertEqual(file(doc), saved)
    }

    func testUndoPrunesASelectionOfAPartThatIsGone() throws {
        let doc = TestSupport.newDocument()
        let chip = try doc.recordEdit(.schematic, "Add Part") { try TestSupport.place(doc, "SN74HC00N", at: p(200, 200)) }
        doc.select(.chip(chip.id))
        XCTAssertTrue(doc.isSelected(.chip(chip.id)))
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertFalse(doc.isSelected(.chip(chip.id)))
        XCTAssertFalse(doc.isSelected(.footprint(chip.id)))
        XCTAssertNil(doc.selectedChipID)
    }

    /// A step whose items are gone is skipped, and the one under it undone.
    func testUndoSkipsAStepThatChangesNothing() throws {
        let doc = TestSupport.newDocument()
        let chip = try doc.recordEdit(.schematic, "Add Part") { try TestSupport.place(doc, "SN74HC00N", at: p(200, 200)) }
        doc.recordEdit(.pcb, "Rotate") { doc.rotateFootprint(chip.id) }
        doc.recordEdit(.pcb, "Change Board Size") { doc.setBoardSize(width: 120, height: 90) }
        // Undo the board size, then the add from the Schematic: the PCB's
        // rotate now has no part.
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.undoName(on: .schematic), "Add Part")
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(doc.undoName(on: .pcb), "Rotate")
        XCTAssertFalse(doc.undo(on: .pcb))
        XCTAssertNil(doc.undoName(on: .pcb))
    }
}
