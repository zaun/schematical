import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The schematic Net Label tool: naming, joining nets by name, shape,
/// moving, deleting, undo, saving and exporting.
@MainActor
final class NetLabelTests: XCTestCase {
    private func newDocument() -> SchematicDocument { TestSupport.newDocument() }

    private func label(_ doc: SchematicDocument, _ id: UUID) throws -> NetLabel {
        try XCTUnwrap(doc.netLabels.first { $0.id == id })
    }

    private func reopened(_ doc: SchematicDocument) throws -> SchematicDocument {
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let opened = newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        return opened
    }

    /// Two LEDs far apart: K of the first wired to one label, A of the
    /// second to another. Nothing else joins them.
    private func labelledPair(_ doc: SchematicDocument) throws -> (left: UUID, right: UUID, k: PinRef, a: PinRef) {
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 600, y: 300))
        let k = try TestSupport.pin(one, "K"), a = try TestSupport.pin(two, "A")
        let left = doc.addNetLabel(at: CGPoint(x: 250, y: 100))
        let right = doc.addNetLabel(at: CGPoint(x: 450, y: 300))
        XCTAssertNil(doc.connect(.pin(k), .label(left)))
        XCTAssertNil(doc.connect(.label(right), .pin(a)))
        return (left, right, k, a)
    }

    private func netlist(_ doc: SchematicDocument) -> SchematicNetlist {
        NetlistBuilder.build(chips: doc.chips, wires: doc.wires, buses: doc.buses, labels: doc.netLabels)
    }

    private func net(_ doc: SchematicDocument, _ ref: PinRef) -> Int? {
        let chips = doc.chips
        let built = netlist(doc)
        guard let part = chips.firstIndex(where: { $0.id == ref.chipID }) else { return nil }
        let designator = chips[part].part.pins[ref.index].designator
        return built.connections.first { $0.part == part && $0.pin == designator }?.net
    }

    func testAddSnapsSelectsAndNames() throws {
        let doc = newDocument()
        let first = doc.addNetLabel(at: CGPoint(x: 104, y: 57))
        let added = try label(doc, first)
        XCTAssertEqual(added.origin, CGPoint(x: 100, y: 60))
        XCTAssertEqual(added.name, "N1")
        XCTAssertEqual(added.style, .input)
        XCTAssertEqual(doc.selectedNetLabelID, first)
        XCTAssertEqual(SchematicSidebarView.content(for: doc), .label(first))
        let second = doc.addNetLabel(at: CGPoint(x: 300, y: 60))
        XCTAssertEqual(try label(doc, second).name, "N2")
        // A free name is reused, ignoring case.
        XCTAssertTrue(doc.setNetLabelName(first, "n2"))
        XCTAssertEqual(doc.nextNetLabelName(), "N1")
    }

    func testNamesAreUpToFourCharactersWithNoSpaces() throws {
        XCTAssertEqual(NetLabel.cleanName(" a b cdef "), "abcd")
        XCTAssertEqual(NetLabel.cleanName("CLK"), "CLK")
        XCTAssertNil(NetLabel.cleanName("   "))
        let doc = newDocument()
        let id = doc.addNetLabel(at: .zero)
        XCTAssertTrue(doc.setNetLabelName(id, "RESET"))
        XCTAssertEqual(try label(doc, id).name, "RESE")
        XCTAssertFalse(doc.setNetLabelName(id, ""))
        XCTAssertEqual(try label(doc, id).name, "RESE")
    }

    func testLabelsOfOneNameAreOneNet() throws {
        let doc = newDocument()
        let (left, right, k, a) = try labelledPair(doc)
        // N1 and N2: two nets.
        XCTAssertNotEqual(net(doc, k), net(doc, a))
        XCTAssertTrue(doc.setNetLabelName(left, "clk"))
        XCTAssertTrue(doc.setNetLabelName(right, "CLK"))
        XCTAssertNotNil(net(doc, k))
        XCTAssertEqual(net(doc, k), net(doc, a))
        // The PCB shows the connection as an unrouted line.
        XCTAssertEqual(doc.ratsnest().count, 1)
        XCTAssertTrue(doc.setNetLabelName(right, "DATA"))
        XCTAssertNotEqual(net(doc, k), net(doc, a))
        XCTAssertTrue(doc.ratsnest().isEmpty)
    }

    func testShapeFollowsType() throws {
        var label = NetLabel(id: UUID(), name: "A", origin: CGPoint(x: 100, y: 50))
        // Input: flat at the wire, a point away from it.
        XCTAssertEqual(label.outline.count, 5)
        XCTAssertTrue(label.outline.contains(CGPoint(x: 160, y: 50)))
        XCTAssertFalse(label.outline.contains(CGPoint(x: 100, y: 50)))
        // Output: a point at the wire, flat away from it.
        label.style = .output
        XCTAssertEqual(label.outline.count, 5)
        XCTAssertTrue(label.outline.contains(CGPoint(x: 100, y: 50)))
        XCTAssertFalse(label.outline.contains(CGPoint(x: 160, y: 50)))
        // In/out: both.
        label.style = .inout
        XCTAssertEqual(label.outline.count, 6)
        XCTAssertTrue(label.outline.contains(CGPoint(x: 100, y: 50)))
        XCTAssertTrue(label.outline.contains(CGPoint(x: 160, y: 50)))
        XCTAssertEqual(label.rect, CGRect(x: 100, y: 40, width: 60, height: 20))
        // Flipped, it runs left of its point.
        label.isFlipped = true
        XCTAssertEqual(label.rect, CGRect(x: 40, y: 40, width: 60, height: 20))
        XCTAssertTrue(label.outline.contains(CGPoint(x: 40, y: 50)))
        XCTAssertEqual(label.tip, CGPoint(x: 100, y: 50))
    }

    func testPassiveIsABox() throws {
        let label = NetLabel(id: UUID(), name: "A", style: .passive, origin: CGPoint(x: 100, y: 50))
        XCTAssertEqual(label.outline, [CGPoint(x: 100, y: 40), CGPoint(x: 160, y: 40),
                                       CGPoint(x: 160, y: 60), CGPoint(x: 100, y: 60)])
        XCTAssertTrue(label.isNet)
    }

    func testNoConnectIsAnXThatJoinsNothing() throws {
        let doc = newDocument()
        let (left, right, k, a) = try labelledPair(doc)
        // Even with one name, two No Connects never join.
        XCTAssertTrue(doc.setNetLabelName(right, "N1"))
        XCTAssertEqual(net(doc, k), net(doc, a))
        doc.setNetLabelStyle(left, .noConnect)
        doc.setNetLabelStyle(right, .noConnect)
        XCTAssertNotNil(net(doc, k))
        XCTAssertNotNil(net(doc, a))
        XCTAssertNotEqual(net(doc, k), net(doc, a))
        XCTAssertTrue(doc.ratsnest().isEmpty)

        // Two pins wired to the same No Connect stay apart too.
        XCTAssertNil(doc.connect(.pin(a), .label(left)))
        XCTAssertNotEqual(net(doc, k), net(doc, a))
        XCTAssertTrue(doc.ratsnest().isEmpty)

        // An X on the point, no name; a press round the point hits it.
        let cross = try label(doc, left)
        XCTAssertFalse(cross.isNet)
        XCTAssertEqual(cross.rect, CGRect(x: cross.tip.x - 10, y: cross.tip.y - 10, width: 20, height: 20))
        XCTAssertEqual(doc.netLabel(at: CGPoint(x: cross.tip.x + 4, y: cross.tip.y - 4))?.id, left)
        let svg = String(decoding: try XCTUnwrap(SchematicExport.svg(SchematicScene(doc))), as: UTF8.self)
        XCTAssertFalse(svg.contains(">N1</text>"))

        // Saved and opened as it was.
        let opened = try reopened(doc)
        XCTAssertEqual(opened.netLabels, doc.netLabels)
        XCTAssertEqual(opened.netLabels.first?.style, .noConnect)
    }

    func testWiresFollowAMoveAndAFlip() throws {
        let doc = newDocument()
        let (left, _, _, _) = try labelledPair(doc)
        let wire = try XCTUnwrap(doc.wires.first { $0.end == .label(left) || $0.start == .label(left) })
        doc.select(.label(left))
        let base = doc.schematicGroupBase()
        XCTAssertTrue(base.affectedWires.contains(wire.id))
        doc.moveSchematicGroup(base, by: CGSize(width: 40, height: 20))
        doc.settleSchematicGroup(base)
        XCTAssertEqual(try label(doc, left).origin, CGPoint(x: 290, y: 120))
        let path = doc.path(forWire: wire.id)
        XCTAssertTrue(path.first == CGPoint(x: 290, y: 120) || path.last == CGPoint(x: 290, y: 120))
        doc.flipNetLabel(left)
        XCTAssertTrue(try label(doc, left).isFlipped)
        XCTAssertEqual(try label(doc, left).tip, CGPoint(x: 290, y: 120))
    }

    func testLineToolAnchorsOnTheLabelPoint() throws {
        let doc = newDocument()
        let id = doc.addNetLabel(at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(doc.lineAnchor(at: CGPoint(x: 203, y: 198), pinRadius: 8, gridRadius: 6), .label(id))
        XCTAssertEqual(doc.anchorAxis(.label(id)), .horizontal)
        XCTAssertTrue(doc.pinTips.contains(CGPoint(x: 200, y: 200)))
    }

    func testDeleteLeavesLooseEndsAndUndoBringsItBack() throws {
        let doc = newDocument()
        let (left, _, k, _) = try labelledPair(doc)
        let wire = try XCTUnwrap(doc.wires.first { [$0.start, $0.end].contains(.label(left)) })
        let rect = try label(doc, left).rect
        doc.erase(at: CGPoint(x: rect.midX, y: rect.midY))
        XCTAssertFalse(doc.netLabels.contains { $0.id == left })
        let kept = try XCTUnwrap(doc.wires.first { $0.id == wire.id })
        XCTAssertTrue(doc.hasLooseEnd(kept))
        XCTAssertTrue([kept.start, kept.end].contains(.pin(k)))

        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertTrue(doc.netLabels.contains { $0.id == left })
        XCTAssertEqual(doc.wires.first { $0.id == wire.id }, wire)

        // ⌘⌫ on a selected label does the same.
        doc.select(.label(left))
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertFalse(doc.netLabels.contains { $0.id == left })
        XCTAssertTrue(doc.schematicSelection.isEmpty)
    }

    func testDroppingALabelOnALooseEndConnectsIt() throws {
        let doc = newDocument()
        let (left, _, _, _) = try labelledPair(doc)
        let point = try label(doc, left).tip
        doc.deleteNetLabel(left)
        let added = doc.addNetLabel(at: point)
        XCTAssertTrue(doc.wires.contains { [$0.start, $0.end].contains(.label(added)) })
        XCTAssertFalse(doc.junctions.contains(where: \.isLoose))
    }

    func testRenameAndTypeAreUndoable() throws {
        let doc = newDocument()
        let id = doc.addNetLabel(at: .zero)
        doc.endCoalescing()
        doc.setNetLabelStyle(id, .inout)
        XCTAssertEqual(doc.undoName(on: .schematic), "Change Net Label Type")
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(try label(doc, id).style, .input)
        XCTAssertTrue(doc.setNetLabelName(id, "A"))
        XCTAssertTrue(doc.setNetLabelName(id, "AB"))
        doc.endCoalescing()
        XCTAssertEqual(doc.undoName(on: .schematic), "Rename Net Label")
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(try label(doc, id).name, "N1")
    }

    func testRoundTrip() throws {
        let doc = newDocument()
        let (left, right, _, _) = try labelledPair(doc)
        XCTAssertTrue(doc.setNetLabelName(left, "CLK"))
        doc.setNetLabelStyle(right, .output)
        doc.flipNetLabel(right)
        let opened = try reopened(doc)
        XCTAssertEqual(opened.netLabels, doc.netLabels)
        XCTAssertEqual(opened.wires, doc.wires)
    }

    func testFilesWithoutLabelsDoNotWriteThem() throws {
        let doc = newDocument()
        let json = String(decoding: try SchematicFile(document: doc, fileURL: nil).encoded(), as: UTF8.self)
        XCTAssertFalse(json.contains("\"labels\""))
    }

    func testBadLabelsAreRefused() throws {
        func open(labels: String, wires: String = "") throws {
            let json = """
            { "format": "\(SchematicFile.formatName)", "version": 4, "nextReference": 1,
              "parts": [], "junctions": [], "wires": [\(wires)], "labels": [\(labels)] }
            """
            try SchematicFile.decode(Data(json.utf8)).apply(to: newDocument(), catalog: TestSupport.catalog,
                                                            fileURL: nil)
        }
        let id = UUID().uuidString
        XCTAssertThrowsError(try open(labels: #"{ "id": "\#(id)", "name": "", "x": 0, "y": 0 }"#)) { error in
            guard case SchematicFileError.badLabel = error else { return XCTFail("\(error)") }
        }
        XCTAssertThrowsError(try open(labels: #"{ "id": "\#(id)", "name": "TOOLONG", "x": 0, "y": 0 }"#)) { error in
            guard case SchematicFileError.badLabel = error else { return XCTFail("\(error)") }
        }
        XCTAssertThrowsError(try open(labels: #"{ "id": "\#(id)", "name": "A", "x": 0, "y": 0, "type": "x" }"#)) { error in
            guard case SchematicFileError.badLabel = error else { return XCTFail("\(error)") }
        }
        let junction = UUID().uuidString, wire = UUID().uuidString
        XCTAssertThrowsError(try open(
            labels: "",
            wires: #"{ "id": "\#(wire)", "from": { "label": "\#(id)" }, "to": { "junction": "\#(junction)" }, "bends": [] }"#
        )) { error in
            guard case SchematicFileError.missingLabel = error else { return XCTFail("\(error)") }
        }
    }

    func testExportDrawsTheName() throws {
        let doc = newDocument()
        let id = doc.addNetLabel(at: .zero)
        XCTAssertTrue(doc.setNetLabelName(id, "A&B"))
        let scene = SchematicScene(doc)
        XCTAssertFalse(scene.isEmpty)
        let svg = String(decoding: try XCTUnwrap(SchematicExport.svg(scene)), as: UTF8.self)
        XCTAssertTrue(svg.contains(">\(SVGPainter.escaped("A&B"))</text>"))
    }
}
