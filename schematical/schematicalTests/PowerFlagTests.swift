import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-63-A: the Power Flag net label type (KiCad's `PWR_FLAG`): placed like a
/// No Connect, drawn as a flag on a stem, saved, joins only its own wire.
@MainActor
final class PowerFlagTests: XCTestCase {
    private func newDocument() -> SchematicDocument { TestSupport.newDocument() }

    private func reopened(_ doc: SchematicDocument) throws -> SchematicDocument {
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let opened = newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        return opened
    }

    /// Two LEDs wired K to A, and a flag wired to that wire's A end... the flag
    /// is a label on its own wire to K of the first.
    private func ledPair(_ doc: SchematicDocument) throws -> (k: PinRef, a: PinRef) {
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 500, y: 300))
        let k = try TestSupport.pin(one, "K"), a = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(k), .pin(a)))
        return (k, a)
    }

    private func netlist(_ doc: SchematicDocument) -> SchematicNetlist {
        NetlistBuilder.build(chips: doc.chips, wires: doc.wires, buses: doc.buses, labels: doc.netLabels)
    }

    func testAFlagIsPlacedDrawnSelectedAndNamesNothing() throws {
        let doc = newDocument()
        let id = doc.addNetLabel(at: CGPoint(x: 204, y: 197), style: .powerFlag)
        let flag = try XCTUnwrap(doc.netLabels.first { $0.id == id })
        XCTAssertEqual(flag.origin, CGPoint(x: 200, y: 200))
        XCTAssertTrue(flag.isPowerFlag)
        XCTAssertFalse(flag.isNet)
        XCTAssertFalse(flag.isNoConnect)
        XCTAssertEqual(flag.style.title, "Power Flag")
        XCTAssertEqual(doc.selectedNetLabelID, id)
        // A press on the pennant, and on the point, hits it.
        XCTAssertEqual(doc.netLabel(at: CGPoint(x: 206, y: 185))?.id, id)
        XCTAssertEqual(doc.netLabel(at: CGPoint(x: 200, y: 203))?.id, id)
        // The drawing has no name, and it is in the export.
        let svg = String(decoding: try XCTUnwrap(SchematicExport.svg(SchematicScene(doc))), as: UTF8.self)
        XCTAssertFalse(svg.contains(">N1</text>"))
        // The sidebar's selection list names it.
        XCTAssertEqual(SelectionRows.schematic(doc).first { $0.item == .label(id) }?.title, "Power Flag")
    }

    func testMoveDeleteCopyPasteUndoRedo() throws {
        let doc = newDocument()
        let id = doc.addNetLabel(at: CGPoint(x: 200, y: 200), style: .powerFlag)
        doc.select(.label(id))
        let base = doc.schematicGroupBase()
        doc.moveSchematicGroup(base, by: CGSize(width: 40, height: 20))
        doc.settleSchematicGroup(base)
        XCTAssertEqual(doc.netLabels.first?.origin, CGPoint(x: 240, y: 220))

        let copy = try XCTUnwrap(doc.schematicCopy())
        doc.endCoalescing()
        XCTAssertTrue(doc.pasteSchematic(copy, offset: CGSize(width: 100, height: 0)))
        XCTAssertEqual(doc.netLabels.filter(\.isPowerFlag).count, 2)

        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.netLabels.count, 1)
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.netLabels.count, 2)

        doc.deleteNetLabel(id)
        XCTAssertEqual(doc.netLabels.count, 1)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.netLabels.count, 2)
    }

    func testFileRoundTripAndTheErrorNamesAllSixTypes() throws {
        let doc = newDocument()
        _ = doc.addNetLabel(at: CGPoint(x: 200, y: 200), style: .powerFlag)
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        XCTAssertTrue(String(decoding: data, as: UTF8.self).contains("\"powerFlag\""))
        let opened = try reopened(doc)
        XCTAssertEqual(opened.netLabels, doc.netLabels)
        XCTAssertEqual(opened.netLabels.first?.style, .powerFlag)

        let id = UUID().uuidString
        let json = """
        { "format": "\(SchematicFile.formatName)", "version": 4, "nextReference": 1,
          "parts": [], "junctions": [], "wires": [],
          "labels": [{ "id": "\(id)", "name": "A", "x": 0, "y": 0, "type": "x" }] }
        """
        XCTAssertThrowsError(try SchematicFile.decode(Data(json.utf8))
            .apply(to: newDocument(), catalog: TestSupport.catalog, fileURL: nil)) { error in
            guard case let SchematicFileError.badLabel(message) = error else { return XCTFail("\(error)") }
            for word in ["input", "output", "inout", "passive", "noConnect", "powerFlag"] {
                XCTAssertTrue(message.contains(word), "\(word) in \(message)")
            }
        }
    }

    func testAFlagOnAWireChangesNoNet() throws {
        let doc = newDocument()
        let (k, a) = try ledPair(doc)
        let before = netlist(doc)

        // A flag on a wire of its own to K: one more wire end on K's net.
        let flag = doc.addNetLabel(at: CGPoint(x: 100, y: 400), style: .powerFlag)
        XCTAssertNil(doc.connect(.pin(k), .label(flag)))
        let after = netlist(doc)
        XCTAssertEqual(after.netCount, before.netCount)
        XCTAssertEqual(after.connections.map { "\($0.part)/\($0.pin)/\($0.net)" },
                       before.connections.map { "\($0.part)/\($0.pin)/\($0.net)" })
        XCTAssertEqual(after.connections.first?.net, after.connections.last?.net, "K and A are one net")
        XCTAssertNotNil(a)
    }

    /// A flag as a joint: two wires ending on it join their pins, as a junction
    /// does; as a No Connect they join nothing.
    func testAFlagJoinsItsWiresAndANoConnectDoesNot() throws {
        let doc = newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 600, y: 300))
        let k = try TestSupport.pin(one, "K"), a = try TestSupport.pin(two, "A")
        let flag = doc.addNetLabel(at: CGPoint(x: 350, y: 200), style: .powerFlag)
        XCTAssertNil(doc.connect(.pin(k), .label(flag)))
        XCTAssertNil(doc.connect(.pin(a), .label(flag)))
        func net(_ ref: PinRef) -> Int? {
            let built = netlist(doc)
            let part = doc.chips.firstIndex { $0.id == ref.chipID }!
            let designator = doc.chips[part].part.pins[ref.index].designator
            return built.connections.first { $0.part == part && $0.pin == designator }?.net
        }
        XCTAssertNotNil(net(k))
        XCTAssertEqual(net(k), net(a))
        doc.setNetLabelStyle(flag, .noConnect)
        XCTAssertNotEqual(net(k), net(a))
        doc.setNetLabelStyle(flag, .powerFlag)
        XCTAssertEqual(net(k), net(a))
    }

    func testADocumentWithNoFlagWritesNoPowerFlagText() throws {
        let doc = newDocument()
        _ = doc.addNetLabel(at: CGPoint(x: 200, y: 200), style: .noConnect)
        _ = doc.addNetLabel(at: CGPoint(x: 300, y: 200))
        let text = String(decoding: try SchematicFile(document: doc, fileURL: nil).encoded(), as: UTF8.self)
        XCTAssertFalse(text.contains("powerFlag"))
    }
}
