import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-33-H: capacitors take `C<n>` references; `GENERIC-CAPACITOR` keeps a
/// capacitance per placed part (`farads`), saved in the file, undoable, and
/// on its Bill of Materials line.
@MainActor
final class CapacitorTests: XCTestCase {
    /// A placed generic capacitor is C1, 0.1 µF, on the ceramic disc footprint.
    func testPlacingTheGenericCapacitor() throws {
        let doc = TestSupport.newDocument()
        let cap = try TestSupport.place(doc, "GENERIC-CAPACITOR", at: CGPoint(x: 100, y: 100))
        XCTAssertTrue(cap.part.isGenericCapacitor)
        XCTAssertEqual(cap.reference, "C1")
        XCTAssertEqual(cap.farads, 1e-7)
        XCTAssertEqual(cap.footprintID, "CAP-DISC-5.08")
        XCTAssertEqual(cap.symbol, .capacitor)
        XCTAssertEqual(PartFacts.kindName(cap.part), "Capacitor")
    }

    /// Capacitors count C1, C2; a gate after them is still U1; a real
    /// capacitor gets a C reference and no per-part value.
    func testReferences() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "GENERIC-CAPACITOR", at: CGPoint(x: 100, y: 100))
        let second = try TestSupport.place(doc, "GENERIC-CAPACITOR", at: CGPoint(x: 300, y: 100))
        let gate = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 500, y: 100))
        let real = try TestSupport.place(doc, "GMC21X7R472K50NT", at: CGPoint(x: 700, y: 100))
        XCTAssertEqual(second.reference, "C2")
        XCTAssertEqual(gate.reference, "U1")
        XCTAssertNil(gate.farads)
        XCTAssertEqual(real.reference, "C3")
        XCTAssertNil(real.farads)
        XCTAssertEqual(real.part.capacitorFarads ?? 0, 4.7e-9, accuracy: 1e-15)
    }

    /// `setPartFarads` changes the value, undo restores it; invalid values
    /// and other parts are ignored.
    func testSetPartFaradsAndUndo() throws {
        let doc = TestSupport.newDocument()
        let cap = try TestSupport.place(doc, "GENERIC-CAPACITOR", at: CGPoint(x: 100, y: 100))
        doc.setPartFarads(cap.id, 4.7e-9)
        XCTAssertEqual(doc.chips[0].farads, 4.7e-9)
        XCTAssertEqual(doc.undoName(on: .schematic), "Change Capacitance")
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(doc.chips[0].farads, 1e-7)
        for bad in [0, -1, Double.infinity, 2, 1e-14] {
            doc.setPartFarads(cap.id, bad)
            XCTAssertEqual(doc.chips[0].farads, 1e-7, "\(bad)")
        }
        let real = try TestSupport.place(doc, "GMC21X7R472K50NT", at: CGPoint(x: 400, y: 100))
        doc.setPartFarads(real.id, 1e-6)
        XCTAssertNil(doc.chips[1].farads)
    }

    /// Save and open keeps the value and the footprint.
    func testRoundTrip() throws {
        let doc = TestSupport.newDocument()
        let cap = try TestSupport.place(doc, "GENERIC-CAPACITOR", at: CGPoint(x: 100, y: 100))
        doc.setPartFarads(cap.id, 4.7e-9)
        doc.setPartFootprint(cap.id, "CAP-0603")
        let file = try SchematicFile.decode(SchematicFile(document: doc, fileURL: nil).encoded())
        XCTAssertEqual(file.parts[0].farads, 4.7e-9)
        XCTAssertEqual(file.parts[0].footprint, "CAP-0603")
        let opened = try BoardLayersTests.roundTrip(doc)
        let chip = try XCTUnwrap(opened.chips.first)
        XCTAssertEqual(chip.reference, "C1")
        XCTAssertEqual(chip.farads, 4.7e-9)
        XCTAssertEqual(chip.footprintID, "CAP-0603")
    }

    /// A file part with these extra keys (JSON, leading comma) as a version 4 file.
    private static func open(capacitor extra: String) throws -> SchematicDocument {
        let json = """
        { "format": "\(SchematicFile.formatName)", "version": 4, "nextReference": 1,
          "parts": [{ "id": "\(UUID().uuidString)", "reference": "C3", "partNumber": "GENERIC-CAPACITOR",
                      "x": 100, "y": 100, "flipped": false\(extra) }],
          "junctions": [], "wires": [] }
        """
        let opened = TestSupport.newDocument()
        try SchematicFile.decode(Data(json.utf8)).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        return opened
    }

    /// A file without `farads` opens with 0.1 µF.
    func testOldFileOpensWithTheDefault() throws {
        let chip = try XCTUnwrap(try Self.open(capacitor: "").chips.first)
        XCTAssertEqual(chip.farads, 1e-7)
        XCTAssertEqual(chip.footprintID, "CAP-DISC-5.08")
    }

    func testBadFaradsFails() {
        XCTAssertThrowsError(try Self.open(capacitor: ", \"farads\": 0")) { error in
            guard case let SchematicFileError.badFarads(reference, farads) = error else {
                return XCTFail("unexpected \(error)")
            }
            XCTAssertEqual(reference, "C3")
            XCTAssertEqual(farads, 0)
            XCTAssertEqual(error.localizedDescription,
                           "C3 has capacitance 0 F; it must be more than 0 and at most 1 F.")
        }
        XCTAssertNoThrow(try Self.open(capacitor: ", \"farads\": 1"))
    }

    func testValueText() {
        let cases: [(Double, String)] = [(1e-13, "0.1 pF"), (1e-12, "1 pF"), (4.7e-9, "4.7 nF"), (1e-7, "100 nF"),
                                         (4.7e-6, "4.7 µF"), (4.7e-3, "4700 µF")]
        for (farads, text) in cases {
            XCTAssertEqual(CapacitorValue.text(farads), text)
        }
        XCTAssertEqual(CapacitorValue.farads(number: 4.7, unit: .nano) ?? 0, 4.7e-9, accuracy: 1e-18)
        XCTAssertNil(CapacitorValue.farads(number: 0, unit: .pico))
        XCTAssertEqual(CapacitorValue.parse(" 4.7 "), 4.7)
    }

    /// The generic capacitor is one CAPACITOR line per value; a real one is
    /// its own part number with its capacitance as the value.
    func testBillOfMaterials() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "GENERIC-CAPACITOR", at: CGPoint(x: 100, y: 100))
        try TestSupport.place(doc, "GENERIC-CAPACITOR", at: CGPoint(x: 300, y: 100))
        let other = try TestSupport.place(doc, "GENERIC-CAPACITOR", at: CGPoint(x: 500, y: 100))
        doc.setPartFarads(other.id, 4.7e-9)
        try TestSupport.place(doc, "GMC21X7R472K50NT", at: CGPoint(x: 700, y: 100))
        let lines = BillOfMaterials.lines(for: doc.chips)
        let generic = lines.filter { $0.partNumber == "CAPACITOR" }
        XCTAssertEqual(generic.map(\.value).sorted(), ["100 nF", "4.7 nF"])
        XCTAssertEqual(generic.first { $0.value == "100 nF" }?.references, ["C1", "C2"])
        XCTAssertEqual(generic.first?.description, "Capacitor")
        let real = try XCTUnwrap(lines.first { $0.partNumber == "GMC21X7R472K50NT" })
        XCTAssertEqual(real.value, "4.7 nF")
        XCTAssertEqual(real.references, ["C4"])
    }
}
