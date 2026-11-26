import CoreGraphics
import XCTest
@testable import schematical

/// Edits refused because they would put 12 V on a 74HC00's supply pin
/// (plan-68-C): renaming or restyling a net label, pasting, pressing a
/// button, closing a DIP switch. Each leaves the document as it was and says
/// why in `editorMessage`.
@MainActor
final class SchematicVoltageRefusalTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func newDocument() -> SchematicDocument {
        TestSupport.newDocument(library: TestSupport.catalog.library)
    }

    /// A 12 V supply and a 74HC00 (VCC at most 6 V), not connected.
    private func supplyAndGate(_ doc: SchematicDocument) throws -> (supply: PinRef, vcc: PinRef) {
        let supply = try TestSupport.part("12V")
        doc.placeChip(part: supply, symbol: symbol(for: supply), centeredAt: p(100, 300))
        let source = try XCTUnwrap(doc.chips.last)
        let gate = try TestSupport.place(doc, "SN74HC00N", at: p(500, 300))
        return (try TestSupport.pin(source, "VCC"), try TestSupport.pin(gate, "VCC"))
    }

    private func label(_ doc: SchematicDocument, on pin: PinRef, at point: CGPoint, name: String,
                       style: NetLabel.Style = .input) throws -> UUID {
        let id = doc.addNetLabel(at: point, style: style)
        XCTAssertTrue(doc.setNetLabelName(id, name))
        XCTAssertNil(doc.connect(.pin(pin), .label(id)))
        return id
    }

    func testRenamingALabelOntoAnOverVoltageNetIsRefused() throws {
        let doc = newDocument()
        let (supply, vcc) = try supplyAndGate(doc)
        _ = try label(doc, on: supply, at: p(100, 100), name: "PWR")
        let other = try label(doc, on: vcc, at: p(500, 100), name: "LOGIC")
        XCTAssertFalse(doc.setNetLabelName(other, "PWR"))
        XCTAssertEqual(doc.netLabels.first { $0.id == other }?.name, "LOGIC")
        let message = try XCTUnwrap(doc.editorMessage)
        XCTAssertTrue(message.contains("12 V"), message)
    }

    func testLeavingNoConnectOntoAnOverVoltageNetIsRefused() throws {
        let doc = newDocument()
        let (supply, vcc) = try supplyAndGate(doc)
        _ = try label(doc, on: supply, at: p(100, 100), name: "PWR")
        let other = try label(doc, on: vcc, at: p(500, 100), name: "PWR", style: .noConnect)
        doc.setNetLabelStyle(other, .input)
        XCTAssertEqual(doc.netLabels.first { $0.id == other }?.style, .noConnect)
        XCTAssertTrue(try XCTUnwrap(doc.editorMessage).contains("12 V"))
    }

    func testPastingALabelOntoAnOverVoltageNetIsRefused() throws {
        let source = newDocument()
        let gate = try TestSupport.place(source, "SN74HC00N", at: p(500, 300))
        let id = try label(source, on: try TestSupport.pin(gate, "VCC"), at: p(500, 100), name: "PWR")
        source.select(.chip(gate.id))
        source.toggleSelection(.label(id))
        let copy = try XCTUnwrap(source.schematicCopy())

        let doc = newDocument()
        let supply = try TestSupport.part("12V")
        doc.placeChip(part: supply, symbol: symbol(for: supply), centeredAt: p(100, 300))
        _ = try label(doc, on: try TestSupport.pin(try XCTUnwrap(doc.chips.last), "VCC"), at: p(100, 100), name: "PWR")
        let before = doc.state
        XCTAssertFalse(doc.pasteSchematic(copy, offset: CGSize(width: 0, height: 400)))
        XCTAssertEqual(doc.state, before, "nothing pasted, and the next reference is put back")
        XCTAssertTrue(try XCTUnwrap(doc.editorMessage).contains("12 V"))
    }

    func testPressingAButtonOntoAnOverVoltageIsRefused() throws {
        let doc = newDocument()
        let (supply, vcc) = try supplyAndGate(doc)
        let part = try XCTUnwrap(TestSupport.allParts.first { part in
            part.isTactileSwitch && !part.isNormallyClosed && Set(part.pins.map(\.name)) == ["A", "B"]
        })
        let button = try TestSupport.place(doc, part.name, at: p(300, 500))
        XCTAssertNil(doc.connect(.pin(supply), .pin(try TestSupport.pin(button, "A"))))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(button, "B")), .pin(vcc)))
        XCTAssertNil(doc.setButton(button.id, pressed: true))
        XCTAssertEqual(doc.chips.first { $0.id == button.id }?.switchesOn, [false])
        XCTAssertTrue(try XCTUnwrap(doc.editorMessage).contains("12 V"))
    }

    func testClosingADipSwitchOntoAnOverVoltageIsRefused() throws {
        let doc = newDocument()
        let (supply, vcc) = try supplyAndGate(doc)
        // TE GDS, 2 positions: position 1 joins 1A and 1B (Parts/switch.json "contacts").
        let dip = try TestSupport.place(doc, "TE-GDS0204", at: p(300, 600))
        let a = try TestSupport.pin(dip, "1A")
        let b = try TestSupport.pin(dip, "1B")
        XCTAssertNil(doc.connect(.pin(supply), .pin(a)))
        XCTAssertNil(doc.connect(.pin(b), .pin(vcc)))
        XCTAssertNil(doc.toggleSwitch(dip.id, position: 1))
        XCTAssertEqual(doc.chips.first { $0.id == dip.id }?.switchesOn.first, false)
        XCTAssertTrue(try XCTUnwrap(doc.editorMessage).contains("12 V"))
    }
}
