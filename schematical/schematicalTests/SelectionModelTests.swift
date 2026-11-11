import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-15 phase 1: the selection sets and group movement, on the document
/// alone (no views).
@MainActor
final class SelectionModelTests: XCTestCase {
    static var catalog: PartCatalogStore { TestSupport.catalog }

    private func newDocument() -> SchematicDocument { TestSupport.newDocument() }

    @discardableResult
    private func place(_ doc: SchematicDocument, _ name: String, at point: CGPoint) throws -> PlacedChip {
        try TestSupport.place(doc, name, at: point)
    }

    private func pin(_ chip: PlacedChip, _ name: String) throws -> PinRef {
        try TestSupport.pin(chip, name)
    }

    /// Two LEDs joined K to A, and a third LED's A branching from the middle
    /// of that wire (so there is a junction and three wires).
    private struct Circuit {
        let doc: SchematicDocument
        let left: PlacedChip, right: PlacedChip, below: PlacedChip
        let junction: Junction
    }

    private func circuit() throws -> Circuit {
        let doc = newDocument()
        let left = try place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let right = try place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 100))
        let below = try place(doc, "GENERIC-LED", at: CGPoint(x: 250, y: 300))
        XCTAssertNil(doc.connect(.pin(try pin(left, "K")), .pin(try pin(right, "A"))))
        let path = doc.path(forWire: try XCTUnwrap(doc.wires.first).id)
        let middle = CGPoint(x: SchematicGrid.snap((path.first!.x + path.last!.x) / 2), y: path.first!.y)
        XCTAssertNil(doc.connect(.pin(try pin(below, "A")), .point(middle)))
        XCTAssertEqual(doc.wires.count, 3)
        let junction = try XCTUnwrap(doc.junctions.first)
        return Circuit(doc: doc, left: left, right: right, below: below, junction: junction)
    }

    private func chip(_ doc: SchematicDocument, _ id: UUID) -> PlacedChip { doc.chips.first { $0.id == id }! }

    // MARK: Set algebra

    func testReplaceToggleClear() {
        var selection = ItemSelection<Int>()
        XCTAssertTrue(selection.isEmpty)
        XCTAssertNil(selection.sole)
        selection.replace(with: 1)
        XCTAssertEqual(selection.sole, 1)
        selection.toggle(2)
        XCTAssertEqual(selection.items, [1, 2])
        XCTAssertNil(selection.sole)
        selection.toggle(1)
        XCTAssertEqual(selection.items, [2])
        selection.replace(with: 3)
        XCTAssertEqual(selection.items, [3])
        selection.insert(4)
        selection.filter { $0 != 3 }
        XCTAssertEqual(selection.items, [4])
        selection.clear()
        XCTAssertTrue(selection.isEmpty)
    }

    func testPressAndRelease() {
        var selection = ItemSelection<Int>()
        // Plain press: the item alone, a single-item drag.
        var press = selection.press(1, shift: false)
        XCTAssertEqual(press, SelectionPress(dragsGroup: false, onClick: .none))
        XCTAssertEqual(selection.items, [1])
        // Shift press on a new item adds it and drags the group.
        press = selection.press(2, shift: true)
        XCTAssertEqual(press, SelectionPress(dragsGroup: true, onClick: .none))
        selection.release(press, wasClick: true)
        XCTAssertEqual(selection.items, [1, 2])
        // Shift-click on a selected item removes it; a Shift-drag keeps it.
        press = selection.press(2, shift: true)
        selection.release(press, wasClick: false)
        XCTAssertEqual(selection.items, [1, 2])
        press = selection.press(2, shift: true)
        selection.release(press, wasClick: true)
        XCTAssertEqual(selection.items, [1])
        // Plain press on one item of several drags the group; a click keeps only it.
        selection.insert(3)
        press = selection.press(3, shift: false)
        XCTAssertTrue(press.dragsGroup)
        XCTAssertEqual(selection.items, [1, 3])
        selection.release(press, wasClick: true)
        XCTAssertEqual(selection.items, [3])
        // Plain press on something unselected replaces the selection.
        selection.insert(1)
        _ = selection.press(5, shift: false)
        XCTAssertEqual(selection.items, [5])
    }

    func testSchematicItemsAndBusExcludeEachOther() throws {
        let doc = newDocument()
        let led = try place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let busID = try XCTUnwrap(doc.addBus(from: CGPoint(x: 200, y: 300), to: CGPoint(x: 400, y: 300)))
        XCTAssertEqual(doc.selectedBusID, busID)
        doc.select(.chip(led.id))
        XCTAssertNil(doc.selectedBusID)
        XCTAssertEqual(doc.selectedChipID, led.id)
        doc.selectedBusID = busID
        XCTAssertTrue(doc.schematicSelection.isEmpty)
        XCTAssertNil(doc.selectedChipID)
    }

    func testPartsAreOneSelectionAcrossPages() throws {
        let doc = newDocument()
        let led = try place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let other = try place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 100))
        doc.clearAllSelections()

        doc.select(.chip(led.id))
        XCTAssertEqual(doc.boardSelection.items, [.footprint(led.id)], "a part selects its footprint")
        doc.toggleSelection(.chip(other.id))
        XCTAssertEqual(doc.boardSelection.items, [.footprint(led.id), .footprint(other.id)])

        doc.select(.footprint(other.id))
        XCTAssertEqual(doc.schematicSelection.items, [.chip(other.id)], "a footprint selects its part")
        doc.toggleSelection(.footprint(led.id))
        XCTAssertEqual(doc.schematicSelection.items, [.chip(led.id), .chip(other.id)])

        doc.clearBoardSelection()
        XCTAssertTrue(doc.schematicSelection.isEmpty)
        doc.select(.chip(led.id))
        doc.clearSchematicSelection()
        XCTAssertTrue(doc.boardSelection.isEmpty)
    }

    func testNonPartsDoNotCrossPages() throws {
        let c = try circuit()
        let doc = c.doc
        let text = doc.addText(at: CGPoint(x: 5, y: 5))
        doc.select(.chip(c.left.id))
        doc.toggleSelection(.wire(try XCTUnwrap(doc.wires.first).id))
        XCTAssertEqual(doc.boardSelection.items, [.footprint(c.left.id)], "a wire has no footprint")

        doc.select(.text(text))
        XCTAssertTrue(doc.schematicSelection.isEmpty, "a text has no part")
        // A wire has no footprint; the PCB's text selection is untouched.
        doc.select(.wire(try XCTUnwrap(doc.wires.first).id))
        XCTAssertEqual(doc.boardSelection.items, [.text(text)])

        // A footprint plus a text: the part is selected, the text stays PCB-only.
        doc.select(.footprint(c.left.id))
        doc.toggleSelection(.text(text))
        XCTAssertEqual(doc.schematicSelection.items, [.chip(c.left.id)])
        XCTAssertEqual(doc.boardSelection.count, 2)
    }

    // MARK: Group movement

    func testMixedGroupGetsOneSnappedDelta() throws {
        let c = try circuit()
        let doc = c.doc
        let belowA = WireEnd.pin(try pin(c.below, "A"))
        let wire = try XCTUnwrap(doc.wires.first { $0.start == belowA || $0.end == belowA })
        doc.select(.chip(c.left.id))
        doc.toggleSelection(.chip(c.below.id))
        doc.toggleSelection(.junction(c.junction.id))
        doc.toggleSelection(.wire(wire.id))
        let base = doc.schematicGroupBase()
        XCTAssertEqual(base.chips.count, 2)
        XCTAssertEqual(base.junctions.count, 1)
        XCTAssertEqual(base.wires.count, 1)
        // Every wire touches a moved part or the junction.
        XCTAssertEqual(base.affectedWires, Set(doc.wires.map(\.id)))

        // 13, 27 snaps to 10, 30.
        doc.moveSchematicGroup(base, by: CGSize(width: 13, height: 27))
        let delta = CGSize(width: 10, height: 30)
        for (id, origin) in base.chips {
            XCTAssertEqual(chip(doc, id).origin, CGPoint(x: origin.x + delta.width, y: origin.y + delta.height))
        }
        XCTAssertEqual(doc.junctions.first { $0.id == c.junction.id }?.point,
                       CGPoint(x: c.junction.point.x + 10, y: c.junction.point.y + 30))
        let bends = try XCTUnwrap(base.wires[wire.id])
        XCTAssertEqual(doc.wires.first { $0.id == wire.id }?.interior,
                       bends.map { CGPoint(x: $0.x + 10, y: $0.y + 30) })
        // The unselected part stays.
        XCTAssertEqual(chip(doc, c.right.id).origin, c.right.origin)

        // Frames are from the press, not cumulative.
        doc.moveSchematicGroup(base, by: CGSize(width: 13, height: 27))
        XCTAssertEqual(chip(doc, c.left.id).origin, CGPoint(x: c.left.origin.x + 10, y: c.left.origin.y + 30))

        doc.settleSchematicGroup(base)
        for wire in doc.wires {
            let path = doc.path(forWire: wire.id)
            XCTAssertTrue(OrthogonalRoute.isOrthogonal(path), "wire \(path) has a slanted segment")
            XCTAssertEqual(path.first, doc.endTip(wire.start))
            XCTAssertEqual(path.last, doc.endTip(wire.end))
        }
    }

    func testSelectedWireKeepsAttachedEnds() throws {
        let c = try circuit()
        let doc = c.doc
        let belowA = WireEnd.pin(try pin(c.below, "A"))
        let wire = try XCTUnwrap(doc.wires.first { $0.start == belowA })
        let ends = (wire.start, wire.end)
        let tips = (doc.endTip(wire.start), doc.endTip(wire.end))
        doc.select(.wire(wire.id))
        let base = doc.schematicGroupBase()
        XCTAssertTrue(base.chips.isEmpty)
        XCTAssertTrue(base.junctions.isEmpty)
        doc.moveSchematicGroup(base, by: CGSize(width: 40, height: -20))
        doc.settleSchematicGroup(base)
        let moved = try XCTUnwrap(doc.wires.first { $0.id == wire.id })
        XCTAssertEqual(moved.start, ends.0)
        XCTAssertEqual(moved.end, ends.1)
        let path = doc.path(forWire: wire.id)
        XCTAssertEqual(path.first, tips.0)
        XCTAssertEqual(path.last, tips.1)
        XCTAssertTrue(OrthogonalRoute.isOrthogonal(path), "\(path)")
        // Nothing else moved.
        XCTAssertEqual(chip(doc, c.below.id).origin, c.below.origin)
        XCTAssertEqual(doc.junctions.first?.point, c.junction.point)
    }

    func testWireNotSelectedDoesNotMovePart() throws {
        let c = try circuit()
        let doc = c.doc
        for wire in doc.wires { doc.toggleSelection(.wire(wire.id)) }
        let base = doc.schematicGroupBase()
        doc.moveSchematicGroup(base, by: CGSize(width: 50, height: 50))
        for placed in [c.left, c.right, c.below] {
            XCTAssertEqual(chip(doc, placed.id).origin, placed.origin)
        }
    }

    func testBoardGroupGetsOneSnappedDelta() throws {
        let doc = newDocument()
        let a = try place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 100))
        let text = doc.addText(at: CGPoint(x: 10, y: 10))
        doc.select(.footprint(a.id))
        doc.toggleSelection(.footprint(b.id))
        doc.toggleSelection(.text(text))
        let base = doc.boardGroupBase()
        XCTAssertEqual(base.footprints.count, 2)
        XCTAssertEqual(base.texts.count, 1)
        // 0.8 mm snaps to 0.635 mm (one step); 2.0 mm to 1.905 mm (three).
        doc.moveBoardGroup(base, by: CGSize(width: 0.8, height: 2.0))
        let dx = BoardGrid.snap(0.8), dy = BoardGrid.snap(2.0)
        XCTAssertEqual(dx, 0.635, accuracy: 1e-9)
        XCTAssertEqual(dy, 1.905, accuracy: 1e-9)
        for (id, centre) in base.footprints {
            let now = try XCTUnwrap(chip(doc, id).board?.centre)
            XCTAssertEqual(now.x - centre.x, dx, accuracy: 1e-9)
            XCTAssertEqual(now.y - centre.y, dy, accuracy: 1e-9)
        }
        let anchor = try XCTUnwrap(doc.texts.first?.anchor)
        let start = try XCTUnwrap(base.texts[text])
        XCTAssertEqual(anchor.x - start.x, dx, accuracy: 1e-9)
        XCTAssertEqual(anchor.y - start.y, dy, accuracy: 1e-9)
    }

    // MARK: Cleanup

    func testEraseDropsStaleSelection() throws {
        let c = try circuit()
        let doc = c.doc
        doc.select(.chip(c.below.id))
        for wire in doc.wires { doc.toggleSelection(.wire(wire.id)) }
        doc.toggleSelection(.junction(c.junction.id))
        doc.toggleSelection(.chip(c.left.id))
        XCTAssertEqual(doc.boardSelection.items, [.footprint(c.below.id), .footprint(c.left.id)])
        // Erasing the lower LED leaves its wire with a loose end; the
        // junction stays, since its wires still end on it.
        let belowPoint = CGPoint(x: chip(doc, c.below.id).bodyRect.midX, y: chip(doc, c.below.id).bodyRect.midY)
        doc.erase(at: belowPoint)
        let wireIDs = Set(doc.wires.map(\.id))
        for item in doc.schematicSelection.items {
            switch item {
            case let .chip(id): XCTAssertTrue(doc.chips.contains { $0.id == id })
            case let .wire(id): XCTAssertTrue(wireIDs.contains(id))
            case let .junction(id): XCTAssertTrue(doc.junctions.contains { $0.id == id })
            case let .note(id): XCTAssertTrue(doc.notes.contains { $0.id == id })
            case let .label(id): XCTAssertTrue(doc.netLabels.contains { $0.id == id })
            }
        }
        XCTAssertFalse(doc.isSelected(.chip(c.below.id)))
        XCTAssertTrue(doc.isSelected(.chip(c.left.id)))
        XCTAssertEqual(doc.boardSelection.items, [.footprint(c.left.id)],
                       "the erased part's footprint left the PCB selection")
    }

    func testDeleteTextDropsIt() {
        let doc = newDocument()
        let a = doc.addText(at: .zero)
        let b = doc.addText(at: CGPoint(x: 5, y: 5))
        doc.toggleSelection(.text(a))
        XCTAssertEqual(doc.boardSelection.count, 2)
        doc.deleteText(a)
        XCTAssertEqual(doc.boardSelection.items, [.text(b)])
    }

    func testReplaceContentsClearsBothSelections() throws {
        let c = try circuit()
        let doc = c.doc
        doc.select(.chip(c.left.id))
        doc.select(.footprint(c.left.id))
        doc.replaceContents(chips: doc.chips, wires: doc.wires, junctions: doc.junctions, buses: [],
                            nextReference: 4, board: doc.board, texts: [], traces: [])
        XCTAssertTrue(doc.schematicSelection.isEmpty)
        XCTAssertTrue(doc.boardSelection.isEmpty)
    }

    func testSelectionIsNotSaved() throws {
        let c = try circuit()
        let doc = c.doc
        doc.select(.chip(c.left.id))
        doc.toggleSelection(.junction(c.junction.id))
        doc.toggleSelection(.wire(try XCTUnwrap(doc.wires.first).id))
        doc.select(.footprint(c.right.id))
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let unselected = newDocument()
        let plain = try SchematicFile(document: { () -> SchematicDocument in
            let copy = newDocument()
            try? SchematicFile.decode(data).apply(to: copy, catalog: Self.catalog, fileURL: nil)
            return copy
        }(), fileURL: nil).encoded()
        XCTAssertEqual(data, plain, "a saved selection would change the bytes")
        XCTAssertFalse(String(decoding: data, as: UTF8.self).lowercased().contains("select"))

        try SchematicFile.decode(data).apply(to: unselected, catalog: Self.catalog, fileURL: nil)
        XCTAssertEqual(unselected.chips.count, 3)
        XCTAssertTrue(unselected.schematicSelection.isEmpty)
        XCTAssertTrue(unselected.boardSelection.isEmpty)
    }
}
