import CoreGraphics
import XCTest
@testable import schematical

/// Editing a part drawn as its units (plan-67-C): the view switch, one
/// unit's move, flip and turn, paste, and which units a group drag moves.
@MainActor
final class SplitPartEditTests: XCTestCase {
    /// A 7400 with `1Y` wired to an LED and `VCC` to a 5 V supply.
    private func wired() throws -> (doc: SchematicDocument, gate: PlacedChip, wires: [Wire]) {
        let doc = TestSupport.newDocument()
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 300, y: 300))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 600, y: 200))
        let supply = try TestSupport.place(doc, "5V", at: CGPoint(x: 100, y: 100))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(gate, "1Y")), .pin(try TestSupport.pin(led, "A"))))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(gate, "VCC")), .pin(try TestSupport.pin(supply, "VCC"))))
        doc.endCoalescing()
        return (doc, gate, doc.wires)
    }

    private func chip(_ doc: SchematicDocument, _ id: UUID) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.id == id })
    }

    /// Every wire's drawn path starts and ends on its ends' tips.
    private func assertWiresOnTheirPins(_ doc: SchematicDocument, file: StaticString = #filePath, line: UInt = #line) {
        for wire in doc.wires {
            let path = doc.path(for: wire)
            XCTAssertEqual(path.first, doc.endTip(wire.start), file: file, line: line)
            XCTAssertEqual(path.last, doc.endTip(wire.end), file: file, line: line)
        }
    }

    func testWiresFollowTheSplit() throws {
        let (doc, gate, wires) = try wired()
        let output = try TestSupport.pin(gate, "1Y"), power = try TestSupport.pin(gate, "VCC")
        let blockTip = try XCTUnwrap(doc.pinTip(output))

        doc.setPartSplit(gate.id, true)
        let split = try chip(doc, gate.id)
        XCTAssertTrue(split.isSplit)
        XCTAssertEqual(doc.wires.map(\.id), wires.map(\.id))
        XCTAssertEqual(doc.wires.map(\.start), wires.map(\.start))
        XCTAssertEqual(doc.wires.map(\.end), wires.map(\.end))
        let a = try XCTUnwrap(split.piece(forUnit: "A")), p = try XCTUnwrap(split.piece(forUnit: "P"))
        let onA = try XCTUnwrap(doc.pinTip(output)), onP = try XCTUnwrap(doc.pinTip(power))
        XCTAssertTrue(a.hitRect.insetBy(dx: -1, dy: -1).contains(onA))
        XCTAssertTrue(p.hitRect.insetBy(dx: -1, dy: -1).contains(onP))
        XCTAssertNotEqual(onA, blockTip)
        assertWiresOnTheirPins(doc)
        // Every pin can still be found under the pointer.
        XCTAssertEqual(doc.pin(at: onA), output)
        XCTAssertEqual(doc.connectedPins[gate.id], [output.index, power.index])

        doc.setPartSplit(gate.id, false)
        XCTAssertFalse(try chip(doc, gate.id).isSplit)
        XCTAssertEqual(doc.pinTip(output), blockTip)
        assertWiresOnTheirPins(doc)
    }

    func testUndoAndRedo() throws {
        let (doc, gate, _) = try wired()
        let before = try chip(doc, gate.id)
        doc.setPartSplit(gate.id, true)
        doc.endCoalescing()
        XCTAssertEqual(doc.undoName, "Change View")
        let split = try chip(doc, gate.id)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(try chip(doc, gate.id), before)
        assertWiresOnTheirPins(doc)
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(try chip(doc, gate.id), split)
        assertWiresOnTheirPins(doc)
    }

    func testMoveFlipRotateOneUnit() throws {
        let (doc, gate, _) = try wired()
        doc.setPartSplit(gate.id, true)
        doc.endCoalescing()
        let b = UnitRef(chipID: gate.id, key: "B")
        let start = try chip(doc, gate.id)
        let aTips = try XCTUnwrap(start.piece(forUnit: "A")).layout.slots.map { start.tip(of: $0) }
        let bPiece = try XCTUnwrap(start.piece(forUnit: "B"))
        let bTips = bPiece.layout.slots.map { bPiece.tip(of: $0) }

        doc.moveUnit(b, to: CGPoint(x: bPiece.origin.x + 203, y: bPiece.origin.y + 38))
        XCTAssertEqual(doc.undoName, "Move")
        doc.endCoalescing()
        var now = try chip(doc, gate.id)
        var moved = try XCTUnwrap(now.piece(forUnit: "B"))
        XCTAssertEqual(moved.origin, CGPoint(x: bPiece.origin.x + 200, y: bPiece.origin.y + 40), "snapped to the grid")
        XCTAssertEqual(moved.layout.slots.map { moved.tip(of: $0) }, bTips.map { CGPoint(x: $0.x + 200, y: $0.y + 40) })
        XCTAssertEqual(try XCTUnwrap(now.piece(forUnit: "A")).layout.slots.map { now.tip(of: $0) }, aTips)

        doc.flipUnit(b)
        XCTAssertEqual(doc.undoName, "Flip")
        doc.endCoalescing()
        now = try chip(doc, gate.id)
        moved = try XCTUnwrap(now.piece(forUnit: "B"))
        XCTAssertEqual(moved.layout.slots.map(\.side), [.right, .right, .left])

        doc.rotateUnit(b)
        XCTAssertEqual(doc.undoName, "Rotate")
        doc.endCoalescing()
        now = try chip(doc, gate.id)
        moved = try XCTUnwrap(now.piece(forUnit: "B"))
        XCTAssertEqual(moved.rotation, 90)
        XCTAssertEqual(Set(moved.layout.slots.map(\.side)), [.top, .bottom])
        doc.rotateUnit(b, counterclockwise: true)
        XCTAssertEqual(try XCTUnwrap(try chip(doc, gate.id).piece(forUnit: "B")).rotation, 0)
        doc.endCoalescing()

        // The block of leftover pins flips but does not turn.
        let before = try chip(doc, gate.id)
        doc.rotateUnit(UnitRef(chipID: gate.id, key: "P"))
        XCTAssertEqual(try chip(doc, gate.id), before)
        doc.flipUnit(UnitRef(chipID: gate.id, key: "P"))
        XCTAssertEqual(try XCTUnwrap(try chip(doc, gate.id).unitPlacements.last).isFlipped, true)
        // A unit the part does not have: nothing.
        let flipped = try chip(doc, gate.id)
        doc.flipUnit(UnitRef(chipID: gate.id, key: "Z"))
        XCTAssertEqual(try chip(doc, gate.id), flipped)
        assertWiresOnTheirPins(doc)
    }

    func testFlipChipIgnoresASplitPart() throws {
        let (doc, gate, _) = try wired()
        doc.setPartSplit(gate.id, true)
        doc.endCoalescing()
        let before = try chip(doc, gate.id)
        doc.flipChip(gate.id)
        doc.rotateChip(gate.id)
        XCTAssertEqual(try chip(doc, gate.id), before)
    }

    func testPasteKeepsTheArrangement() throws {
        let (doc, gate, _) = try wired()
        doc.setPartSplit(gate.id, true)
        doc.flipUnit(UnitRef(chipID: gate.id, key: "C"))
        doc.rotateUnit(UnitRef(chipID: gate.id, key: "D"))
        doc.endCoalescing()
        let original = try chip(doc, gate.id)
        doc.select(.chip(gate.id))
        let copy = try XCTUnwrap(doc.schematicCopy())
        XCTAssertTrue(doc.pasteSchematic(copy, offset: CGSize(width: 403, height: 97)))
        let pasted = try XCTUnwrap(doc.chips.last)
        XCTAssertNotEqual(pasted.id, gate.id)
        XCTAssertNotEqual(pasted.reference, original.reference)
        XCTAssertTrue(pasted.isSplit)
        XCTAssertEqual(pasted.unitPlacements.map(\.key), original.unitPlacements.map(\.key))
        XCTAssertEqual(pasted.unitPlacements.map(\.origin),
                       original.unitPlacements.map { CGPoint(x: $0.origin.x + 400, y: $0.origin.y + 100) })
        XCTAssertEqual(pasted.unitPlacements.map(\.isFlipped), original.unitPlacements.map(\.isFlipped))
        XCTAssertEqual(pasted.unitPlacements.map(\.rotation), original.unitPlacements.map(\.rotation))
        XCTAssertEqual(pasted.pieces.first?.drawnReference, "\(pasted.reference)-A")
    }

    func testPartsWithoutUnitsRefuse() throws {
        let doc = TestSupport.newDocument()
        let resistor = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 100, y: 100))
        let flipFlop = try TestSupport.place(doc, "SN74HC74N", at: CGPoint(x: 300, y: 300))
        doc.endCoalescing()
        let name = doc.undoName
        doc.setPartSplit(resistor.id, true)
        doc.setPartSplit(flipFlop.id, true)
        XCTAssertEqual(doc.chips.map(\.isSplit), [false, false])
        XCTAssertEqual(doc.undoName, name)
        // Joining a part that is a block is nothing too.
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 600, y: 300))
        doc.endCoalescing()
        let placed = doc.undoName
        doc.setPartSplit(gate.id, false)
        XCTAssertEqual(doc.undoName, placed)
    }

    // MARK: Picks and group drags

    func testPicksDefaultToEveryUnit() throws {
        let (doc, gate, _) = try wired()
        doc.setPartSplit(gate.id, true)
        let all: Set<String> = ["A", "B", "C", "D", "P"]
        XCTAssertEqual(doc.picked(try chip(doc, gate.id)), all, "not selected")
        doc.select(.chip(gate.id))
        XCTAssertEqual(doc.picked(try chip(doc, gate.id)), all)
        doc.pick(UnitRef(chipID: gate.id, key: "A"), extend: false)
        XCTAssertEqual(doc.picked(try chip(doc, gate.id)), ["A"])
        doc.pick(UnitRef(chipID: gate.id, key: "B"), extend: true)
        XCTAssertEqual(doc.picked(try chip(doc, gate.id)), ["A", "B"])
        doc.pick(UnitRef(chipID: gate.id, key: "A"), extend: true)
        XCTAssertEqual(doc.picked(try chip(doc, gate.id)), ["B"])
        doc.pick(UnitRef(chipID: gate.id, key: "C"), extend: false)
        XCTAssertEqual(doc.picked(try chip(doc, gate.id)), ["C"])
        // Deselected and selected again: every unit.
        doc.clearSchematicSelection()
        XCTAssertTrue(doc.pickedUnits.isEmpty)
        doc.select(.chip(gate.id))
        XCTAssertEqual(doc.picked(try chip(doc, gate.id)), all)
        doc.pickUnits([UnitRef(chipID: gate.id, key: "B"), UnitRef(chipID: gate.id, key: "D")])
        XCTAssertEqual(doc.picked(try chip(doc, gate.id)), ["B", "D"])
        // Select All picks everything.
        doc.selectAllSchematic()
        XCTAssertEqual(doc.picked(try chip(doc, gate.id)), ["B", "D"], "still selected: the picks stay")
    }

    func testGroupDragMovesOnlyPickedUnits() throws {
        let (doc, gate, _) = try wired()
        doc.setPartSplit(gate.id, true)
        doc.endCoalescing()
        let before = try chip(doc, gate.id)
        doc.select(.chip(gate.id))
        doc.pick(UnitRef(chipID: gate.id, key: "A"), extend: false)
        let base = doc.schematicGroupBase()
        XCTAssertTrue(base.chips.isEmpty)
        XCTAssertEqual(Set(base.units.keys), [UnitRef(chipID: gate.id, key: "A")])
        // The wire on unit A's output moves with it; the one on VCC (unit P) does not.
        XCTAssertEqual(base.affectedWires.count, 1)
        doc.moveSchematicGroup(base, by: CGSize(width: 43, height: -18))
        doc.settleSchematicGroup(base)
        let after = try chip(doc, gate.id)
        XCTAssertEqual(after.unitPlacements[0].origin,
                       CGPoint(x: before.unitPlacements[0].origin.x + 40, y: before.unitPlacements[0].origin.y - 20))
        XCTAssertEqual(Array(after.unitPlacements.dropFirst()), Array(before.unitPlacements.dropFirst()))
        assertWiresOnTheirPins(doc)
        XCTAssertEqual(doc.undoName, "Move")
    }

    func testGroupDragOfAWholeSplitPart() throws {
        let (doc, gate, _) = try wired()
        doc.setPartSplit(gate.id, true)
        doc.endCoalescing()
        let before = try chip(doc, gate.id)
        doc.select(.chip(gate.id))
        let base = doc.schematicGroupBase()
        XCTAssertEqual(base.units.count, 5)
        XCTAssertEqual(base.affectedWires.count, 2)
        doc.moveSchematicGroup(base, by: CGSize(width: 100, height: 50))
        doc.settleSchematicGroup(base)
        let after = try chip(doc, gate.id)
        XCTAssertEqual(after.unitPlacements.map(\.origin),
                       before.unitPlacements.map { CGPoint(x: $0.origin.x + 100, y: $0.origin.y + 50) })
        assertWiresOnTheirPins(doc)
    }

    func testMixedGroupDrag() throws {
        let (doc, gate, _) = try wired()
        doc.setPartSplit(gate.id, true)
        doc.endCoalescing()
        let led = try XCTUnwrap(doc.chips.first { $0.partNumber == "GENERIC-LED" })
        let before = try chip(doc, gate.id)
        doc.select(.chip(gate.id))
        doc.pick(UnitRef(chipID: gate.id, key: "B"), extend: false)
        doc.toggleSelection(.chip(led.id))
        XCTAssertEqual(doc.picked(try chip(doc, gate.id)), ["B"])
        let base = doc.schematicGroupBase()
        XCTAssertEqual(Set(base.chips.keys), [led.id])
        doc.moveSchematicGroup(base, by: CGSize(width: 20, height: 30))
        doc.settleSchematicGroup(base)
        XCTAssertEqual(try chip(doc, led.id).origin, CGPoint(x: led.origin.x + 20, y: led.origin.y + 30))
        let after = try chip(doc, gate.id)
        XCTAssertEqual(after.unitPlacements[1].origin,
                       CGPoint(x: before.unitPlacements[1].origin.x + 20, y: before.unitPlacements[1].origin.y + 30))
        XCTAssertEqual(after.unitPlacements[0], before.unitPlacements[0])
        assertWiresOnTheirPins(doc)
    }
}
