import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-15 phase 3: what each sidebar shows for zero, one and several
/// selected items, the selection rows, and clearing.
@MainActor
final class SelectionSidebarTests: XCTestCase {
    private struct Circuit {
        let doc: SchematicDocument
        let left: PlacedChip, right: PlacedChip
        let wire: Wire
        let junction: Junction
    }

    private func circuit() throws -> Circuit {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let right = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 100))
        let below = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 250, y: 300))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(left, "K")), .pin(try TestSupport.pin(right, "A"))))
        let path = doc.path(forWire: try XCTUnwrap(doc.wires.first).id)
        let middle = CGPoint(x: SchematicGrid.snap((path.first!.x + path.last!.x) / 2), y: path.first!.y)
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(below, "A")), .point(middle)))
        let wire = try XCTUnwrap(doc.wires.first { $0.start == .pin(PinRef(chipID: left.id, index: try TestSupport.pin(left, "K").index)) })
        return Circuit(doc: doc, left: left, right: right, wire: wire, junction: try XCTUnwrap(doc.junctions.first))
    }

    func testSchematicSidebarStates() throws {
        let c = try circuit()
        let doc = c.doc
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .library)
        doc.select(.chip(c.left.id))
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .chip(c.left.id), "a lone part keeps its inspector")
        doc.toggleSelection(.chip(c.right.id))
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .selection)
        // A lone wire or junction has no inspector: it is listed.
        doc.select(.wire(c.wire.id))
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .selection)
        doc.select(.junction(c.junction.id))
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .selection)
        doc.clearSchematicSelection()
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .library)
    }

    func testSchematicRowsListEverySelectedItem() throws {
        let c = try circuit()
        let doc = c.doc
        doc.select(.wire(c.wire.id))
        doc.toggleSelection(.junction(c.junction.id))
        doc.toggleSelection(.chip(c.right.id))
        doc.toggleSelection(.chip(c.left.id))
        let rows = SelectionRows.schematic(doc)
        XCTAssertEqual(rows.count, doc.schematicSelection.count)
        XCTAssertEqual(Set(rows.map(\.item)), doc.schematicSelection.items)
        // Parts in drawing order, then junctions, then wires.
        XCTAssertEqual(rows.map(\.title), [c.left.reference, c.right.reference, "Junction", "Wire"])
        XCTAssertEqual(rows[0].detail, "GENERIC-LED")
        XCTAssertEqual(rows[2].detail, "(\(Int(c.junction.point.x)), \(Int(c.junction.point.y)))")
        XCTAssertEqual(rows[3].detail, "\(c.left.reference) K – junction \(rows[2].detail)")
        XCTAssertEqual(rows[3].accessibilityLabel, "Wire, \(rows[3].detail)")
    }

    func testRowSelectReducesAndClearEmpties() throws {
        let c = try circuit()
        let doc = c.doc
        doc.select(.chip(c.left.id))
        doc.toggleSelection(.chip(c.right.id))
        doc.toggleSelection(.wire(c.wire.id))
        // What a row's button does: select only that item.
        let row = try XCTUnwrap(SelectionRows.schematic(doc).first { $0.item == .chip(c.right.id) })
        doc.select(row.item)
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .chip(c.right.id), "the row opened the part's inspector")
        doc.toggleSelection(.chip(c.left.id))
        // What Clear does.
        doc.clearSchematicSelection()
        XCTAssertTrue(doc.schematicSelection.isEmpty)
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .library)
    }

    func testBusAndPortPanelsStillShow() throws {
        let doc = TestSupport.newDocument()
        let bus = try XCTUnwrap(doc.addBus(from: CGPoint(x: 200, y: 300), to: CGPoint(x: 400, y: 300)))
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .bus(bus))
        XCTAssertNil(doc.addPort(at: CGPoint(x: 300, y: 300)))
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .port(try XCTUnwrap(doc.selectedPortID)))
    }

    func testPCBSidebarStatesAndRows() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 100))
        XCTAssertEqual(PCBSidebarView.content(for: doc), .board)
        let text = doc.addText(at: .zero)
        doc.setText(text, "Line one is quite long indeed\nline two")
        XCTAssertEqual(PCBSidebarView.content(for: doc), .text(text))
        doc.select(.footprint(a.id))
        XCTAssertEqual(PCBSidebarView.content(for: doc), .footprint(a.id))
        doc.toggleSelection(.footprint(b.id))
        doc.toggleSelection(.text(text))
        XCTAssertEqual(PCBSidebarView.content(for: doc), .selection)
        let rows = SelectionRows.board(doc)
        XCTAssertEqual(rows.map(\.title), [a.reference, b.reference, "Text"])
        XCTAssertEqual(rows[2].detail, "Line one is quite long …")
        XCTAssertTrue(rows[0].detail.hasPrefix("GENERIC-LED — "), rows[0].detail)
        doc.select(rows[2].item)
        XCTAssertEqual(PCBSidebarView.content(for: doc), .text(text))
        doc.toggleSelection(.footprint(a.id))
        doc.clearBoardSelection()
        XCTAssertEqual(PCBSidebarView.content(for: doc), .board)
    }

    /// ContentView's Run button calls `clearAllSelections()` before starting.
    func testRunClearsEverySelection() throws {
        let c = try circuit()
        let doc = c.doc
        doc.select(.chip(c.left.id))
        doc.toggleSelection(.wire(c.wire.id))
        doc.select(.footprint(c.right.id))
        doc.toggleSelection(.text(doc.addText(at: .zero)))
        doc.clearAllSelections()
        XCTAssertTrue(doc.schematicSelection.isEmpty)
        XCTAssertTrue(doc.boardSelection.isEmpty)
        let bus = try XCTUnwrap(doc.addBus(from: CGPoint(x: 200, y: 400), to: CGPoint(x: 400, y: 400)))
        XCTAssertEqual(doc.selectedBusID, bus)
        doc.clearAllSelections()
        XCTAssertNil(doc.selectedBusID)
        XCTAssertNil(doc.selectedPortID)
    }
}
