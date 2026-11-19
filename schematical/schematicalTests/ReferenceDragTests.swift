import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-41-B: Cmd-drag on a reference moves only the reference. The tests run
/// `ReferenceDragging`'s steps in the order the PCB canvas does.
@MainActor
final class ReferenceDragTests: XCTestCase {
    /// Screen points per mm at 100 % zoom.
    private let scale = BoardScale.pointsPerMM

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func chip(_ doc: SchematicDocument, _ id: UUID) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.id == id })
    }

    private func centre(_ doc: SchematicDocument, _ id: UUID) throws -> CGPoint {
        try XCTUnwrap(try chip(doc, id).boardReferenceCentre)
    }

    /// Two parts well apart, on a board big enough for both.
    private func twoParts() throws -> (doc: SchematicDocument, a: UUID, b: UUID) {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 200, height: 100)
        let a = try TestSupport.place(doc, "SN74HC00N", at: p(100, 100))
        let b = try TestSupport.place(doc, "SN74HC00N", at: p(300, 100))
        doc.moveFootprint(a.id, to: p(30, 40))
        doc.moveFootprint(b.id, to: p(120, 40))
        return (doc, a.id, b.id)
    }

    /// A whole press: `press`, then one `drag` frame to `offset`, then release.
    @discardableResult
    private func cmdDrag(_ doc: SchematicDocument, at point: CGPoint, by offset: CGSize,
                         visible: BoardVisibility = BoardVisibility()) -> UUID? {
        doc.beginGesture(.pcb)
        defer { doc.endGesture() }
        guard let press = ReferenceDragging.press(doc, at: point, scale: scale, visible: visible) else { return nil }
        ReferenceDragging.drag(doc, id: press.id, from: press.from, by: offset)
        return press.id
    }

    func testPressHitsTheReferenceNotThePadsOrEmptyBoard() throws {
        let c = try twoParts()
        let at = try centre(c.doc, c.a)
        XCTAssertEqual(c.doc.referenceHit(at: at, tolerance: 0.3), c.a)
        XCTAssertEqual(c.doc.referenceHit(at: try centre(c.doc, c.b), tolerance: 0.3), c.b)
        let pad = try XCTUnwrap(try chip(c.doc, c.a).boardPads.first)
        XCTAssertNil(c.doc.referenceHit(at: p(pad.rect.midX, pad.rect.midY), tolerance: 0.3))
        XCTAssertNil(c.doc.referenceHit(at: p(180, 90), tolerance: 0.3))
        XCTAssertNil(ReferenceDragging.press(c.doc, at: p(180, 90), scale: scale, visible: BoardVisibility()))
    }

    func testTopReferenceIsHitBeforeABottomOneUnderIt() throws {
        let doc = TestSupport.newDocument()
        let top = try TestSupport.place(doc, "SN74HC00N", at: p(100, 100))
        let bottom = try TestSupport.place(doc, "SN74HC00N", at: p(300, 100))
        doc.moveFootprint(top.id, to: p(30, 40))
        doc.moveFootprint(bottom.id, to: p(30, 40))
        doc.setFootprintSide(bottom.id, .bottom)
        // The bottom part's reference is last in the list, yet Top wins.
        XCTAssertEqual(doc.chips.last?.id, bottom.id)
        XCTAssertEqual(try chip(doc, bottom.id).board?.side, .bottom)
        XCTAssertEqual(doc.referenceHit(at: try centre(doc, top.id), tolerance: 0.3), top.id)
    }

    func testHiddenOrLayerOffReferencesCantBeHit() throws {
        let c = try twoParts()
        let at = try centre(c.doc, c.a)
        c.doc.setFootprintMarks(c.a, reference: .hidden)
        XCTAssertNil(c.doc.referenceHit(at: at, tolerance: 0.3), "reference hidden")
        c.doc.setFootprintMarks(c.a, reference: .silkscreen)
        var off = BoardVisibility()
        off.hidden = [.silkscreen]
        XCTAssertNil(c.doc.referenceHit(at: at, tolerance: 0.3, visible: off), "silkscreen layer off")
        XCTAssertEqual(c.doc.referenceHit(at: at, tolerance: 0.3), c.a)
    }

    func testDragMovesOnlyTheReference() throws {
        let c = try twoParts()
        _ = c.doc.pressSelection(.footprint(c.a), shift: false)
        _ = c.doc.pressSelection(.footprint(c.b), shift: true)
        let beforeA = try XCTUnwrap(try chip(c.doc, c.a).board)
        let beforeB = try XCTUnwrap(try chip(c.doc, c.b).board)
        let from = try centre(c.doc, c.a)
        XCTAssertEqual(cmdDrag(c.doc, at: from, by: CGSize(width: 4.2, height: -6.3)), c.a)
        let afterA = try XCTUnwrap(try chip(c.doc, c.a).board)
        XCTAssertEqual(afterA.centre, beforeA.centre)
        XCTAssertEqual(afterA.rotation, beforeA.rotation)
        XCTAssertEqual(afterA.side, beforeA.side)
        XCTAssertNotEqual(afterA.referenceOffset, .zero)
        XCTAssertEqual(try chip(c.doc, c.b).board, beforeB, "the other selected part didn't move")
        let to = try centre(c.doc, c.a)
        let grid = c.doc.board.traceGrid
        XCTAssertEqual(to.x, from.x + 4.2, accuracy: grid / 2 + 1e-3)
        XCTAssertEqual(to.y, from.y - 6.3, accuracy: grid / 2 + 1e-3)
        XCTAssertEqual((to.x / grid).rounded() * grid, to.x, accuracy: 1e-3, "on the trace grid")
        XCTAssertEqual((to.y / grid).rounded() * grid, to.y, accuracy: 1e-3)
    }

    func testAPressWithNoMovementLeavesTheReference() throws {
        let c = try twoParts()
        let setup = c.doc.undoName(on: .pcb)
        XCTAssertEqual(cmdDrag(c.doc, at: try centre(c.doc, c.a), by: .zero), c.a)
        XCTAssertEqual(try chip(c.doc, c.a).board?.referenceOffset, .zero)
        XCTAssertEqual(c.doc.undoName(on: .pcb), setup, "no step recorded")
    }

    func testPressSelectsThePartAndKeepsAMultiSelection() throws {
        let c = try twoParts()
        c.doc.select(.footprint(c.b))
        XCTAssertNotNil(ReferenceDragging.press(c.doc, at: try centre(c.doc, c.a), scale: scale, visible: BoardVisibility()))
        XCTAssertEqual(c.doc.boardSelection.items, [.footprint(c.a)])

        _ = c.doc.pressSelection(.footprint(c.b), shift: true)
        XCTAssertEqual(c.doc.boardSelection.items, [.footprint(c.a), .footprint(c.b)])
        XCTAssertNotNil(ReferenceDragging.press(c.doc, at: try centre(c.doc, c.a), scale: scale, visible: BoardVisibility()))
        XCTAssertEqual(c.doc.boardSelection.items, [.footprint(c.a), .footprint(c.b)])
    }

    func testTheWholePressIsOneUndoStep() throws {
        let c = try twoParts()
        let from = try centre(c.doc, c.a)
        let setup = c.doc.undoName(on: .pcb)
        // Several frames of one press.
        c.doc.beginGesture(.pcb)
        let press = try XCTUnwrap(ReferenceDragging.press(c.doc, at: from, scale: scale, visible: BoardVisibility()))
        for step in 1...5 {
            ReferenceDragging.drag(c.doc, id: press.id, from: press.from, by: CGSize(width: Double(step), height: Double(step) / 2))
        }
        c.doc.endGesture()
        XCTAssertEqual(c.doc.undoName(on: .pcb), "Move Reference")
        XCTAssertTrue(c.doc.undo(on: .pcb))
        XCTAssertEqual(try chip(c.doc, c.a).board?.referenceOffset, .zero)
        XCTAssertEqual(c.doc.undoName(on: .pcb), setup, "one step, not five")
    }

    func testALockedPartsReferenceMoves() throws {
        let c = try twoParts()
        c.doc.setLocked(.footprint(c.a), true)
        let before = try XCTUnwrap(try chip(c.doc, c.a).board)
        cmdDrag(c.doc, at: try centre(c.doc, c.a), by: CGSize(width: 3, height: 3))
        let after = try XCTUnwrap(try chip(c.doc, c.a).board)
        XCTAssertNotEqual(after.referenceOffset, .zero)
        XCTAssertEqual(after.centre, before.centre)
    }

    func testTheReferenceFollowsALaterMoveAndTurn() throws {
        let c = try twoParts()
        cmdDrag(c.doc, at: try centre(c.doc, c.a), by: CGSize(width: 5, height: -4))
        let offset = try XCTUnwrap(try chip(c.doc, c.a).board?.referenceOffset)
        XCTAssertNotEqual(offset, .zero)
        let before = try centre(c.doc, c.a)
        let centreBefore = try XCTUnwrap(try chip(c.doc, c.a).board?.centre)
        c.doc.moveFootprint(c.a, to: p(centreBefore.x + 10, centreBefore.y + 5))
        let moved = try XCTUnwrap(try chip(c.doc, c.a).board)
        let delta = p(moved.centre.x - centreBefore.x, moved.centre.y - centreBefore.y)
        let after = try centre(c.doc, c.a)
        XCTAssertEqual(after.x - before.x, delta.x, accuracy: 1e-9)
        XCTAssertEqual(after.y - before.y, delta.y, accuracy: 1e-9)
        XCTAssertEqual(moved.referenceOffset, offset)

        c.doc.rotateFootprint(c.a)
        let turned = try XCTUnwrap(try chip(c.doc, c.a).board)
        XCTAssertEqual(turned.rotation, 90)
        XCTAssertEqual(turned.referenceOffset, offset)
        let expected = turned.board(offset)
        let now = try centre(c.doc, c.a)
        XCTAssertEqual(now.x, expected.x, accuracy: 1e-9)
        XCTAssertEqual(now.y, expected.y, accuracy: 1e-9)
    }
}
