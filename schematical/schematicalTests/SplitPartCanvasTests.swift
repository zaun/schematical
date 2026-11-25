import CoreGraphics
import XCTest
@testable import schematical

/// A part drawn as its units on the canvas (plan-67-E): what is drawn, what
/// a press picks, what Rotate and Flip act on, the sidebar's View option, and
/// the question before such a part is deleted.
@MainActor
final class SplitPartCanvasTests: XCTestCase {
    private func split() throws -> (doc: SchematicDocument, gate: PlacedChip) {
        let doc = TestSupport.newDocument()
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 300, y: 300))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 700, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(gate, "1Y")), .pin(try TestSupport.pin(led, "A"))))
        doc.setPartSplit(gate.id, true)
        doc.endCoalescing()
        doc.clearSchematicSelection()
        return (doc, try XCTUnwrap(doc.chips.first { $0.id == gate.id }))
    }

    private func centre(_ chip: PlacedChip, _ key: String) throws -> CGPoint {
        let rect = try XCTUnwrap(chip.piece(forUnit: key)).bodyRect
        return CGPoint(x: rect.midX, y: rect.midY)
    }

    private func now(_ doc: SchematicDocument, _ chip: PlacedChip) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.id == chip.id })
    }

    // MARK: The View option and drawing

    func testViewOptionApplies() throws {
        XCTAssertTrue(PartOptionsSection.applies(to: try TestSupport.part("SN74HC00N")))
        XCTAssertFalse(PartOptionsSection.applies(to: try TestSupport.part("SN74HC74N")))
        XCTAssertEqual(try TestSupport.part("SN74HC00N").unitViewName, "Gate")
    }

    func testPiecesAreDrawnOnce() throws {
        let (doc, gate) = try split()
        let drawn = doc.chips.flatMap(\.pieces)
        XCTAssertEqual(drawn.count, 6)
        XCTAssertEqual(Set(drawn.map(\.drawID)).count, 6)
        XCTAssertEqual(drawn.filter { $0.id == gate.id }.map(\.drawnReference), ["U1-A", "U1-B", "U1-C", "U1-D", "U1-P"])
        // Each piece has a label of its own.
        XCTAssertEqual(drawn.compactMap { ReferenceLabel.anchor(for: $0) }.count, 6)
    }

    func testNewPartsAreBlocks() throws {
        let doc = TestSupport.newDocument()
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 300, y: 300))
        XCTAssertFalse(gate.isSplit)
        XCTAssertEqual(doc.chips.flatMap(\.pieces), doc.chips)
    }

    // MARK: Picking

    func testPressPicksThePiece() throws {
        let (doc, gate) = try split()
        let piece = try XCTUnwrap(doc.piece(at: try centre(gate, "C")))
        XCTAssertEqual(piece.unitKey, "C")
        XCTAssertEqual(doc.chip(at: try centre(gate, "C"))?.id, gate.id)
        let press = doc.pressPiece(piece, shift: false)
        doc.releaseSelection(press, wasClick: true)
        XCTAssertEqual(doc.selectedChipID, gate.id)
        XCTAssertEqual(doc.picked(gate), ["C"])
        XCTAssertEqual(doc.chips[0].pieces.map { doc.isPicked($0) }, [false, false, true, false, false])
        XCTAssertEqual(doc.chips[0].pieces.map { doc.isSibling($0) }, [true, true, false, true, true])
        // Another piece: it alone.
        let other = try XCTUnwrap(doc.piece(at: try centre(gate, "A")))
        doc.releaseSelection(doc.pressPiece(other, shift: false), wasClick: true)
        XCTAssertEqual(doc.picked(gate), ["A"])
        // The drag would move only it.
        XCTAssertEqual(Set(doc.schematicGroupBase().units.keys), [UnitRef(chipID: gate.id, key: "A")])
    }

    func testShiftPressAddsAPiece() throws {
        let (doc, gate) = try split()
        func press(_ key: String, shift: Bool, click: Bool = true) throws {
            let piece = try XCTUnwrap(doc.piece(at: try centre(gate, key)))
            doc.releaseSelection(doc.pressPiece(piece, shift: shift), wasClick: click)
        }
        try press("A", shift: false)
        try press("B", shift: true)
        XCTAssertEqual(doc.picked(gate), ["A", "B"])
        // A plain press on one of the two: the drag moves both…
        let a = try XCTUnwrap(doc.piece(at: try centre(gate, "A")))
        let held = doc.pressPiece(a, shift: false)
        XCTAssertTrue(held.dragsGroup)
        XCTAssertEqual(Set(doc.schematicGroupBase().units.keys.map(\.key)), ["A", "B"])
        doc.releaseSelection(held, wasClick: false)
        XCTAssertEqual(doc.picked(gate), ["A", "B"], "a drag keeps the picks")
        // …and a click keeps only it.
        try press("A", shift: false)
        XCTAssertEqual(doc.picked(gate), ["A"])
        // Shift-click the last picked piece: the part leaves the selection.
        try press("A", shift: true)
        XCTAssertNil(doc.selectedChipID)
        XCTAssertTrue(doc.pickedUnits.isEmpty)
        // Shift-press a piece of an unselected part: the part joins, that piece picked.
        let led = try XCTUnwrap(doc.chips.last)
        doc.select(.chip(led.id))
        try press("D", shift: true)
        XCTAssertTrue(doc.isSelected(.chip(led.id)))
        XCTAssertEqual(doc.picked(gate), ["D"])
        try press("C", shift: true)
        try press("D", shift: true)
        XCTAssertEqual(doc.picked(gate), ["C"])
    }

    func testMarqueePicksTouchedPieces() throws {
        let (doc, gate) = try split()
        let b = try XCTUnwrap(gate.piece(forUnit: "B")).bodyRect, c = try XCTUnwrap(gate.piece(forUnit: "C")).bodyRect
        doc.boxSelectSchematic(b.union(c).insetBy(dx: 5, dy: 5), base: [])
        XCTAssertTrue(doc.isSelected(.chip(gate.id)))
        XCTAssertEqual(doc.picked(gate), ["B", "C"])
        // The box grows: the picks follow.
        let d = try XCTUnwrap(gate.piece(forUnit: "D")).bodyRect
        doc.boxSelectSchematic(b.union(d).insetBy(dx: 5, dy: 5), base: [])
        XCTAssertEqual(doc.picked(gate), ["B", "C", "D"])
    }

    func testSelectAllPicksEverything() throws {
        let (doc, gate) = try split()
        doc.selectAllSchematic()
        XCTAssertEqual(doc.picked(gate).count, 5)
        XCTAssertTrue(doc.chips[0].pieces.allSatisfy { doc.isPicked($0) })
        XCTAssertFalse(doc.chips[0].pieces.contains { doc.isSibling($0) })
    }

    func testUnsplitPressIsUnchanged() throws {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 400, y: 300))
        let piece = try XCTUnwrap(doc.piece(at: CGPoint(x: one.bodyRect.midX, y: one.bodyRect.midY)))
        XCTAssertEqual(piece, one)
        XCTAssertEqual(doc.pressPiece(piece, shift: false), SelectionPress(dragsGroup: false, onClick: .none))
        XCTAssertEqual(doc.selectedChipID, one.id)
        XCTAssertTrue(doc.pickedUnits.isEmpty)
        XCTAssertTrue(doc.isPicked(one))
        XCTAssertFalse(doc.isSibling(one))
        XCTAssertEqual(doc.schematicGroupBase().chips, [one.id: one.origin])
        XCTAssertEqual(doc.pressPiece(two, shift: true), SelectionPress(dragsGroup: true, onClick: .none))
        XCTAssertEqual(doc.pressPiece(two, shift: true), SelectionPress(dragsGroup: true, onClick: .remove(.chip(two.id))))
        XCTAssertTrue(doc.pickedUnits.isEmpty)
    }

    // MARK: Rotate and Flip

    func testRotateAndFlipTargets() throws {
        let (doc, gate) = try split()
        XCTAssertNil(doc.rotateTarget)
        doc.select(.chip(gate.id))
        XCTAssertNil(doc.rotateTarget, "every unit is picked: nothing to turn")
        XCTAssertNil(doc.flipTarget)
        doc.pick(UnitRef(chipID: gate.id, key: "B"), extend: false)
        XCTAssertEqual(doc.rotateTarget, .unit(UnitRef(chipID: gate.id, key: "B")))
        XCTAssertEqual(doc.flipTarget, .unit(UnitRef(chipID: gate.id, key: "B")))
        doc.pick(UnitRef(chipID: gate.id, key: "P"), extend: false)
        XCTAssertNil(doc.rotateTarget, "the block of leftover pins does not turn")
        XCTAssertEqual(doc.flipTarget, .unit(UnitRef(chipID: gate.id, key: "P")))
        doc.pick(UnitRef(chipID: gate.id, key: "A"), extend: true)
        XCTAssertNil(doc.flipTarget, "two picked")
        // An unsplit part, as before.
        let led = try XCTUnwrap(doc.chips.last)
        doc.select(.chip(led.id))
        XCTAssertEqual(doc.rotateTarget, .chip(led.id))
        XCTAssertEqual(doc.flipTarget, .chip(led.id))
        doc.setPartSplit(gate.id, false)
        doc.select(.chip(gate.id))
        XCTAssertNil(doc.rotateTarget, "a block does not turn")
        XCTAssertEqual(doc.flipTarget, .chip(gate.id))
    }

    // MARK: The delete confirm

    func testDeleteAsksForASplitPart() throws {
        let (doc, gate) = try split()
        doc.select(.chip(gate.id))
        doc.pick(UnitRef(chipID: gate.id, key: "B"), extend: false)
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertEqual(doc.pendingRemoval?.message, "Delete U1 and all 5 of its symbols?")
        XCTAssertEqual(doc.chips.count, 2, "nothing goes before the answer")
        // A second delete while the question is up does nothing.
        XCTAssertFalse(doc.deleteSchematicSelection())
        doc.confirmPendingRemoval()
        XCTAssertNil(doc.pendingRemoval)
        XCTAssertEqual(doc.chips.map(\.partNumber), ["GENERIC-LED"])
        XCTAssertEqual(doc.undoName, "Delete")
        // The wire stays, with a loose end where the pin was.
        XCTAssertEqual(doc.wires.count, 1)
        XCTAssertTrue(doc.junctions.contains { $0.isLoose })
        XCTAssertTrue(doc.undo())
        let back = try now(doc, gate)
        XCTAssertTrue(back.isSplit)
        XCTAssertEqual(back.unitPlacements, gate.unitPlacements)
    }

    func testCancelKeepsThePart() throws {
        let (doc, gate) = try split()
        doc.select(.chip(gate.id))
        XCTAssertTrue(doc.deleteSchematicSelection())
        doc.cancelPendingRemoval()
        XCTAssertNil(doc.pendingRemoval)
        XCTAssertEqual(try now(doc, gate), gate)
        XCTAssertTrue(doc.isSelected(.chip(gate.id)))
    }

    func testBlockPartDeletesAtOnce() throws {
        let doc = TestSupport.newDocument()
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 300, y: 300))
        let resistor = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 100, y: 100))
        doc.select(.chip(gate.id))
        doc.toggleSelection(.chip(resistor.id))
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertNil(doc.pendingRemoval)
        XCTAssertTrue(doc.chips.isEmpty)
    }

    func testMixedSelectionMessage() throws {
        let (doc, gate) = try split()
        let led = try XCTUnwrap(doc.chips.last)
        let other = try TestSupport.place(doc, "SN74HC04N", at: CGPoint(x: 300, y: 800))
        doc.select(.chip(gate.id))
        doc.toggleSelection(.chip(led.id))
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertEqual(doc.pendingRemoval?.message,
                       "Delete the 2 selected parts? U1 is drawn as several symbols; all of them go.")
        doc.cancelPendingRemoval()
        doc.setPartSplit(other.id, true)
        doc.selectAllSchematic()
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertEqual(doc.pendingRemoval?.message,
                       "Delete the 3 selected parts? U1 and U2 are drawn as several symbols; all of them go.")
        doc.confirmPendingRemoval()
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertTrue(doc.wires.isEmpty)
    }

    func testCutAsksAndKeepsTheUndoName() throws {
        let (doc, gate) = try split()
        doc.select(.chip(gate.id))
        XCTAssertNotNil(doc.schematicCopy())
        XCTAssertTrue(doc.deleteSchematicSelection(named: "Cut", includingTitleBlock: false))
        XCTAssertNotNil(doc.pendingRemoval)
        doc.confirmPendingRemoval()
        XCTAssertEqual(doc.chips.count, 1)
        XCTAssertEqual(doc.undoName, "Cut")
    }

    func testEraserAsks() throws {
        let (doc, gate) = try split()
        doc.erase(at: try centre(gate, "B"))
        XCTAssertEqual(doc.pendingRemoval?.message, "Delete U1 and all 5 of its symbols?")
        XCTAssertEqual(doc.chips.count, 2)
        // The sweep goes on over another symbol: nothing more happens.
        doc.erase(at: try centre(gate, "C"))
        XCTAssertEqual(doc.chips.count, 2)
        doc.confirmPendingRemoval()
        XCTAssertEqual(doc.chips.map(\.partNumber), ["GENERIC-LED"])
        // An unsplit part goes at once.
        let led = try XCTUnwrap(doc.chips.first)
        doc.erase(at: CGPoint(x: led.bodyRect.midX, y: led.bodyRect.midY))
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertNil(doc.pendingRemoval)
    }

    func testAgentCanDeleteWithoutTheQuestion() throws {
        let (doc, gate) = try split()
        doc.select(.chip(gate.id))
        XCTAssertTrue(doc.deleteSchematicSelection(confirmed: true))
        XCTAssertNil(doc.pendingRemoval)
        XCTAssertEqual(doc.chips.count, 1)
    }
}
