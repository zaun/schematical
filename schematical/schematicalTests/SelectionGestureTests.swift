import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-15 phase 2: the press, drag and release steps each canvas runs for a
/// Pointer/Select gesture, called in the order the canvas calls them.
@MainActor
final class SelectionGestureTests: XCTestCase {
    private func doc() -> SchematicDocument { TestSupport.newDocument() }

    /// A press on `item`, a drag of `delta` in frames, and the release, as
    /// `SchematicCanvasView` runs them for a group drag. Returns the press.
    @discardableResult
    private func drag(_ doc: SchematicDocument, _ item: SchematicItem, shift: Bool,
                      by delta: CGSize) -> SelectionPress<SchematicItem> {
        let press = doc.pressSelection(item, shift: shift)
        let base = doc.schematicGroupBase()
        for step in 1...4 {
            let t = CGFloat(step) / 4
            doc.moveSchematicGroup(base, by: CGSize(width: delta.width * t, height: delta.height * t))
        }
        doc.settleSchematicGroup(base)
        doc.releaseSelection(press, wasClick: delta == .zero)
        return press
    }

    private func origin(_ doc: SchematicDocument, _ id: UUID) -> CGPoint { doc.chips.first { $0.id == id }!.origin }

    func testShiftSelectThenDragMovesEveryPart() throws {
        let doc = doc()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 100))
        let c = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 500, y: 100))
        // Click a, Shift-click b: both selected, c not.
        drag(doc, .chip(a.id), shift: false, by: .zero)
        drag(doc, .chip(b.id), shift: true, by: .zero)
        XCTAssertEqual(doc.schematicSelection.items, [.chip(a.id), .chip(b.id)])
        // A plain drag on a selected part moves both by one snapped delta.
        let press = drag(doc, .chip(a.id), shift: false, by: CGSize(width: 34, height: -18))
        XCTAssertTrue(press.dragsGroup)
        XCTAssertEqual(origin(doc, a.id), CGPoint(x: a.origin.x + 30, y: a.origin.y - 20))
        XCTAssertEqual(origin(doc, b.id), CGPoint(x: b.origin.x + 30, y: b.origin.y - 20))
        XCTAssertEqual(origin(doc, c.id), c.origin)
        XCTAssertEqual(doc.schematicSelection.count, 2, "a drag keeps the selection")
        // Shift-drag on c adds it and moves all three.
        drag(doc, .chip(c.id), shift: true, by: CGSize(width: 0, height: 40))
        XCTAssertEqual(doc.schematicSelection.count, 3)
        XCTAssertEqual(origin(doc, c.id), CGPoint(x: c.origin.x, y: c.origin.y + 40))
        XCTAssertEqual(origin(doc, a.id), CGPoint(x: a.origin.x + 30, y: a.origin.y + 20))
        // A plain click on one of them keeps only it.
        drag(doc, .chip(b.id), shift: false, by: .zero)
        XCTAssertEqual(doc.schematicSelection.items, [.chip(b.id)])
        // Shift-click on the only selected one empties the selection.
        drag(doc, .chip(b.id), shift: true, by: .zero)
        XCTAssertTrue(doc.schematicSelection.isEmpty)
    }

    func testEmptyPressPansAndClearsOnlyWithoutShift() throws {
        let doc = doc()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        doc.select(.chip(a.id))
        doc.pressEmptySchematic(shift: true)
        XCTAssertEqual(doc.selectedChipID, a.id)
        doc.pressEmptySchematic(shift: false)
        XCTAssertTrue(doc.schematicSelection.isEmpty)

        let text = doc.addText(at: .zero)
        doc.pressEmptyBoard(shift: true)
        XCTAssertEqual(doc.selectedTextID, text)
        doc.pressEmptyBoard(shift: false)
        XCTAssertTrue(doc.boardSelection.isEmpty)
    }

    func testPlainPressOnWireKeepsRouteEditing() throws {
        let doc = doc()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(a, "K")), .pin(try TestSupport.pin(b, "A"))))
        let wire = try XCTUnwrap(doc.wires.first)
        // Plain press on an unselected wire: it alone is selected and the drag
        // edits its route (not a group drag).
        XCTAssertFalse(doc.pressSelection(.wire(wire.id), shift: false).dragsGroup)
        XCTAssertEqual(doc.schematicSelection.items, [.wire(wire.id)])
        // Shift press on a wire drags the group.
        doc.select(.chip(a.id))
        XCTAssertTrue(doc.pressSelection(.wire(wire.id), shift: true).dragsGroup)
        XCTAssertEqual(doc.schematicSelection.count, 2)
    }

    func testBusPressLeavesItemsAndShiftDoesNotMixThem() throws {
        let doc = doc()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let bus = try XCTUnwrap(doc.addBus(from: CGPoint(x: 200, y: 300), to: CGPoint(x: 400, y: 300)))
        doc.select(.chip(a.id))
        // The canvas sets the bus on any press on it, Shift or not.
        doc.selectedBusID = bus
        XCTAssertTrue(doc.schematicSelection.isEmpty)
        _ = doc.pressSelection(.chip(a.id), shift: true)
        XCTAssertNil(doc.selectedBusID)
        XCTAssertEqual(doc.selectedChipID, a.id)
    }

    func testJunctionDragMovesItAndKeepsItsWires() throws {
        let doc = doc()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 100))
        let c = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 250, y: 300))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(a, "K")), .pin(try TestSupport.pin(b, "A"))))
        let path = doc.path(forWire: try XCTUnwrap(doc.wires.first).id)
        let middle = CGPoint(x: SchematicGrid.snap((path.first!.x + path.last!.x) / 2), y: path.first!.y)
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(c, "A")), .point(middle)))
        let junction = try XCTUnwrap(doc.junctionHit(at: CGPoint(x: middle.x + 2, y: middle.y - 2)))
        drag(doc, .junction(junction.id), shift: false, by: CGSize(width: 0, height: 60))
        XCTAssertEqual(doc.junctions.first?.point, CGPoint(x: middle.x, y: middle.y + 60))
        for wire in doc.wires {
            let route = doc.path(forWire: wire.id)
            XCTAssertTrue(OrthogonalRoute.isOrthogonal(route), "\(route)")
            XCTAssertEqual(route.first, doc.endTip(wire.start))
            XCTAssertEqual(route.last, doc.endTip(wire.end))
        }
        XCTAssertEqual(doc.wires.filter { $0.start == .junction(junction.id) || $0.end == .junction(junction.id) }.count, 3)
    }

    func testBoardShiftSelectAndGroupDrag() throws {
        let doc = doc()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 100))
        let text = doc.addText(at: CGPoint(x: 20, y: 20))
        XCTAssertEqual(doc.boardSelection.items, [.text(text)], "a new text is selected")
        var press = doc.pressSelection(.footprint(a.id), shift: true)
        doc.releaseSelection(press, wasClick: true)
        press = doc.pressSelection(.footprint(b.id), shift: true)
        let base = doc.boardGroupBase()
        XCTAssertEqual(base.footprints.count + base.texts.count, 3)
        doc.moveBoardGroup(base, by: CGSize(width: 3.0, height: -1.4))
        doc.releaseSelection(press, wasClick: false)
        let dx = BoardGrid.snap(3.0), dy = BoardGrid.snap(-1.4)
        for (id, centre) in base.footprints {
            let now = try XCTUnwrap(doc.chips.first { $0.id == id }?.board?.centre)
            XCTAssertEqual(now.x - centre.x, dx, accuracy: 1e-9)
            XCTAssertEqual(now.y - centre.y, dy, accuracy: 1e-9)
        }
        XCTAssertEqual(doc.texts[0].anchor.x - base.texts[text]!.x, dx, accuracy: 1e-9)
        XCTAssertEqual(doc.boardSelection.count, 3)
        // Shift-click the text out again.
        press = doc.pressSelection(.text(text), shift: true)
        doc.releaseSelection(press, wasClick: true)
        XCTAssertEqual(doc.boardSelection.items, [.footprint(a.id), .footprint(b.id)])
        // The Schematic page follows with the same parts (the text has no part).
        XCTAssertEqual(doc.schematicSelection.items, [.chip(a.id), .chip(b.id)])
    }

    func testShiftDragBoxAddsWhatItTouches() throws {
        let doc = doc()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 100))
        let c = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 600, y: 400))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(b, "K")), .pin(try TestSupport.pin(c, "A"))))
        let wire = try XCTUnwrap(doc.wires.first)
        doc.select(.chip(c.id))
        let base = doc.schematicSelection.items
        // A box over a only adds a; c stays selected.
        doc.boxSelectSchematic(a.hitRect.insetBy(dx: 2, dy: 2), base: base)
        XCTAssertEqual(doc.schematicSelection.items, [.chip(a.id), .chip(c.id)])
        // Growing it to b picks up b and the wire leaving b; shrinking drops them.
        let both = a.hitRect.union(b.hitRect)
        doc.boxSelectSchematic(both, base: base)
        XCTAssertEqual(doc.schematicSelection.items, [.chip(a.id), .chip(b.id), .chip(c.id), .wire(wire.id)])
        doc.boxSelectSchematic(a.hitRect, base: base)
        XCTAssertEqual(doc.schematicSelection.items, [.chip(a.id), .chip(c.id)])
        // A box over nothing leaves the selection as it was at the press.
        doc.boxSelectSchematic(CGRect(x: -900, y: -900, width: 10, height: 10), base: base)
        XCTAssertEqual(doc.schematicSelection.items, base)
    }

    func testBoardShiftDragBoxAddsWhatItTouches() throws {
        let doc = doc()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 100))
        let boundsA = try XCTUnwrap(doc.chips.first { $0.id == a.id }?.boardBounds)
        let boundsB = try XCTUnwrap(doc.chips.first { $0.id == b.id }?.boardBounds)
        XCTAssertFalse(boundsA.intersects(boundsB), "test parts must not overlap on the board")
        doc.boxSelectBoard(boundsA, base: [])
        XCTAssertEqual(doc.boardSelection.items, [.footprint(a.id)])
        doc.boxSelectBoard(boundsA.union(boundsB), base: [])
        XCTAssertEqual(doc.boardSelection.items, [.footprint(a.id), .footprint(b.id)])
        XCTAssertEqual(doc.schematicSelection.items, [.chip(a.id), .chip(b.id)], "the parts are selected on the schematic too")
    }

    func testSegmentTouchesRect() {
        let r = CGRect(x: 0, y: 0, width: 10, height: 10)
        XCTAssertTrue(SchematicDocument.segment(CGPoint(x: -5, y: 5), CGPoint(x: 15, y: 5), touches: r))
        XCTAssertTrue(SchematicDocument.segment(CGPoint(x: 2, y: 2), CGPoint(x: 3, y: 3), touches: r))
        XCTAssertFalse(SchematicDocument.segment(CGPoint(x: -5, y: -1), CGPoint(x: 15, y: -1), touches: r))
        XCTAssertFalse(SchematicDocument.segment(CGPoint(x: 11, y: -5), CGPoint(x: 20, y: 5), touches: r))
    }
}
