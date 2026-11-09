import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-31-C: every PCB edit is one undo step on the PCB list. One test per
/// row of plan-31-C § Current State, plus the independence of the two lists.
@MainActor
final class PCBUndoTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func file(_ doc: SchematicDocument) -> SchematicFile {
        SchematicFile(document: doc, fileURL: nil)
    }

    /// Runs `edit`, which must change the file, then checks it is one PCB
    /// step named `name`, not on the Schematic list: undo gives the file
    /// before, redo the file after.
    private func assertStep(_ doc: SchematicDocument, _ name: String,
                            file: StaticString = #filePath, line: UInt = #line,
                            _ edit: () throws -> Void) rethrows {
        let schematic = doc.undoName(on: .schematic)
        let before = self.file(doc)
        try edit()
        let after = self.file(doc)
        XCTAssertNotEqual(before, after, "the edit changed nothing", file: file, line: line)
        XCTAssertEqual(doc.undoName(on: .pcb), name, file: file, line: line)
        XCTAssertEqual(doc.undoName(on: .schematic), schematic, "a PCB step is not on the Schematic list",
                       file: file, line: line)
        XCTAssertTrue(doc.undo(on: .pcb), file: file, line: line)
        XCTAssertEqual(self.file(doc), before, "undo", file: file, line: line)
        XCTAssertEqual(doc.redoName(on: .pcb), name, file: file, line: line)
        XCTAssertTrue(doc.redo(on: .pcb), file: file, line: line)
        XCTAssertEqual(self.file(doc), after, "redo", file: file, line: line)
    }

    private func withPart() throws -> (doc: SchematicDocument, chip: PlacedChip) {
        let doc = TestSupport.newDocument()
        return (doc, try TestSupport.place(doc, "SN74HC00N", at: p(200, 200)))
    }

    private func board(_ doc: SchematicDocument, _ id: UUID) throws -> BoardPlacement {
        try XCTUnwrap(doc.chips.first { $0.id == id }?.board)
    }

    // MARK: Footprints

    func testMoveFootprint() throws {
        let c = try withPart()
        try assertStep(c.doc, "Move") { c.doc.moveFootprint(c.chip.id, to: p(40, 30)) }
    }

    func testMoveFootprintOrigin() throws {
        let c = try withPart()
        try assertStep(c.doc, "Move") { c.doc.moveFootprintOrigin(c.chip.id, to: p(40, 30)) }
    }

    func testRotateFootprint() throws {
        let c = try withPart()
        try assertStep(c.doc, "Rotate") { c.doc.rotateFootprint(c.chip.id) }
    }

    func testSetFootprintSide() throws {
        let c = try withPart()
        try assertStep(c.doc, "Change Side") { c.doc.setFootprintSide(c.chip.id, .bottom) }
    }

    func testSetFootprintOrigin() throws {
        let c = try withPart()
        let other = try XCTUnwrap(FootprintOrigin.allCases.first { $0 != c.chip.board?.origin })
        try assertStep(c.doc, "Change Origin") { c.doc.setFootprintOrigin(c.chip.id, other) }
    }

    func testSetFootprintStyle() throws {
        let doc = TestSupport.newDocument()
        let part = try XCTUnwrap(TestSupport.allParts.first { $0.connectorFootprint != nil }, "no part with a connector footprint")
        let chip = try TestSupport.place(doc, part.name, at: p(200, 200))
        try assertStep(doc, "Change Style") { doc.setFootprintStyle(chip.id, .connector) }
    }

    func testSetFootprintMarks() throws {
        let c = try withPart()
        let other = try XCTUnwrap(MarkLayer.allCases.first { $0 != c.chip.board?.outline })
        try assertStep(c.doc, "Change Marks") { c.doc.setFootprintMarks(c.chip.id, outline: other) }
    }

    // MARK: Board

    func testSetBoardSize() throws {
        let doc = TestSupport.newDocument()
        try assertStep(doc, "Change Board Size") { doc.setBoardSize(width: 120, height: 90) }
    }

    func testSetLayerCount() throws {
        let doc = TestSupport.newDocument()
        try assertStep(doc, "Change Layers") { doc.setLayerCount(4) }
    }

    func testSetLayerColor() throws {
        let doc = TestSupport.newDocument()
        try assertStep(doc, "Change Layer Color") { doc.setLayerColor(.top, LayerColor(red: 1, green: 2, blue: 3)) }
    }

    func testSetLayerType() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        try assertStep(doc, "Change Layer Type") { doc.setLayerType(.inner(3), .wire) }
    }

    func testSetBoardUnits() throws {
        let doc = TestSupport.newDocument()
        let other = try XCTUnwrap(BoardUnits.allCases.first { $0 != doc.board.units })
        try assertStep(doc, "Change Units") { doc.setBoardUnits(other) }
    }

    func testSetBoardColor() throws {
        let doc = TestSupport.newDocument()
        let other = try XCTUnwrap(BoardColor.allCases.first { $0 != doc.board.color })
        try assertStep(doc, "Change Board Color") { doc.setBoardColor(other) }
    }

    // MARK: Texts

    private func withText() -> (doc: SchematicDocument, text: UUID) {
        let doc = TestSupport.newDocument()
        return (doc, doc.addText(at: p(10, 10)))
    }

    func testAddText() throws {
        let doc = TestSupport.newDocument()
        try assertStep(doc, "Add Text") { doc.addText(at: p(10, 10)) }
    }

    func testSetText() throws {
        let c = withText()
        try assertStep(c.doc, "Edit Text") { c.doc.setText(c.text, "Rev A") }
    }

    func testSetTextSize() throws {
        let c = withText()
        try assertStep(c.doc, "Change Font Size") { c.doc.setTextSize(c.text, 80) }
    }

    func testSetTextLayer() throws {
        let c = withText()
        try assertStep(c.doc, "Change Layer") { c.doc.setTextLayer(c.text, .document) }
    }

    func testSetTextSide() throws {
        let c = withText()
        try assertStep(c.doc, "Change Side") { c.doc.setTextSide(c.text, .bottom) }
    }

    func testMoveText() throws {
        let c = withText()
        try assertStep(c.doc, "Move") { c.doc.moveText(c.text, to: p(20, 20)) }
    }

    func testSetTextRotation() throws {
        let c = withText()
        try assertStep(c.doc, "Rotate") { c.doc.setTextRotation(c.text, 180) }
    }

    func testRotateText() throws {
        let c = withText()
        try assertStep(c.doc, "Rotate") { c.doc.rotateText(c.text) }
    }

    func testDeleteText() throws {
        let c = withText()
        try assertStep(c.doc, "Delete Text") { c.doc.deleteText(c.text) }
    }

    // MARK: Traces

    private func withTrace() throws -> (doc: SchematicDocument, trace: UUID) {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addTrace(from: p(0, 5.08), to: p(20.32, 5.08), layer: .top, diagonalFirst: false))
        return (doc, id)
    }

    func testAddTrace() throws {
        let doc = TestSupport.newDocument()
        try assertStep(doc, "Add Trace") {
            XCTAssertNotNil(doc.addTrace(from: p(0, 0), to: p(10.16, 0), layer: .top, diagonalFirst: false))
        }
    }

    /// Two crossing traces that were never joined (as a file could hold).
    func testJoin() throws {
        let doc = TestSupport.newDocument()
        let a = BoardTrace(id: UUID(), layer: .top, width: 10, points: [p(0, 5.08), p(20.32, 5.08)])
        let b = BoardTrace(id: UUID(), layer: .top, width: 10, points: [p(10.16, 0), p(10.16, 10.16)])
        doc.replaceContents(chips: [], wires: [], junctions: [], buses: [], nextReference: 1,
                            board: BoardSettings(), texts: [], traces: [a, b])
        try assertStep(doc, "Join Traces") { doc.join(a.id) }
    }

    func testSetTraceWidth() throws {
        let c = try withTrace()
        try assertStep(c.doc, "Change Trace Width") { c.doc.setTraceWidth(c.trace, 20) }
    }

    func testSetTraceLayer() throws {
        let c = try withTrace()
        try assertStep(c.doc, "Change Layer") { c.doc.setTraceLayer(c.trace, .bottom) }
    }

    func testDeleteTrace() throws {
        let c = try withTrace()
        try assertStep(c.doc, "Delete Trace") { c.doc.deleteTrace(c.trace) }
    }

    func testMoveTraces() throws {
        let c = try withTrace()
        try assertStep(c.doc, "Move") { c.doc.moveTraces([c.trace], by: CGSize(width: 5.08, height: 0)) }
    }

    // MARK: Group drags

    func testMoveBoardGroup() throws {
        let c = try withPart()
        c.doc.select(.footprint(c.chip.id))
        let base = c.doc.boardGroupBase()
        try assertStep(c.doc, "Move") { c.doc.moveBoardGroup(base, by: CGSize(width: 5, height: 5)) }
    }

    func testSettleBoardGroup() throws {
        let c = try withTrace()
        let other = try XCTUnwrap(c.doc.addTrace(from: p(25.4, 0), to: p(25.4, 10.16), layer: .top, diagonalFirst: false))
        c.doc.select(.trace(other))
        let base = c.doc.boardGroupBase()
        c.doc.moveBoardGroup(base, by: CGSize(width: -12.7, height: 0))
        try assertStep(c.doc, "Move") { c.doc.settleBoardGroup(base) }
    }

    // MARK: Independence of the two lists

    func testEachPageUndoesItsOwnSideOfAPart() throws {
        let c = try withPart()
        let origin = c.chip.origin, centre = try board(c.doc, c.chip.id).centre
        c.doc.select(.chip(c.chip.id))
        c.doc.moveSchematicGroup(c.doc.schematicGroupBase(), by: CGSize(width: 100, height: 0))
        c.doc.moveFootprint(c.chip.id, to: p(60, 40))
        let movedOrigin = try XCTUnwrap(c.doc.chips.first?.origin)
        XCTAssertNotEqual(movedOrigin, origin)

        XCTAssertTrue(c.doc.undo(on: .pcb))
        XCTAssertEqual(try board(c.doc, c.chip.id).centre, centre)
        XCTAssertEqual(c.doc.chips.first?.origin, movedOrigin, "the Schematic move stays")

        XCTAssertTrue(c.doc.undo(on: .schematic))
        XCTAssertEqual(c.doc.chips.first?.origin, origin)
        XCTAssertEqual(try board(c.doc, c.chip.id).centre, centre, "the footprint stays where the PCB undo left it")
    }

    func testPCBStepsAreNeverOnTheSchematicList() throws {
        let c = try withPart()
        c.doc.rotateFootprint(c.chip.id)
        c.doc.setBoardSize(width: 120, height: 90)
        c.doc.addText(at: p(10, 10))
        _ = c.doc.addTrace(from: p(0, 0), to: p(10.16, 0), layer: .top, diagonalFirst: false)
        c.doc.setBoardColor(.red)
        XCTAssertEqual(c.doc.undoName(on: .schematic), "Add Part")
        XCTAssertEqual(c.doc.undoName(on: .pcb), "Change Board Color")
    }

    /// The add undone from the Schematic takes the footprint with it, so the
    /// PCB's move of it changes nothing: it is skipped and the step under it
    /// undone.
    func testAPCBStepOnARemovedPartIsSkipped() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 120, height: 90)
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        doc.moveFootprint(chip.id, to: p(60, 40))
        XCTAssertEqual(doc.undoName(on: .pcb), "Move")
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.board.width, BoardSettings().width, "the board size, under the skipped move, is undone")
        XCTAssertFalse(doc.undo(on: .pcb))
    }

    func testTextToolGestureIsOneAddText() throws {
        let doc = TestSupport.newDocument()
        let before = doc.state
        doc.beginGesture(.pcb)
        doc.addText(at: p(10, 10))
        let base = doc.boardGroupBase()
        for frame in 1...5 {
            doc.moveBoardGroup(base, by: CGSize(width: CGFloat(frame), height: CGFloat(frame)))
        }
        doc.settleBoardGroup(base)
        doc.endGesture()
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Text")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.state, before)
        XCTAssertNil(doc.undoName(on: .pcb))
    }
}
