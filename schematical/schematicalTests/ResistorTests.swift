import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-34-G: resistor-kind parts in the app. GENERIC-RESISTOR has the pulls'
/// Ohms field, footprint choices and file `ohms`; every resistor-kind part
/// takes the lowest free R<n>; the drawer lists resistor leaves in ohms order.
@MainActor
final class ResistorTests: XCTestCase {
    /// A placed GENERIC-RESISTOR is R1, 1 kΩ, Axial (RES-AXIAL-10.16), drawn
    /// as a resistor with "1kΩ" above it.
    func testPlacingTheGenericResistor() throws {
        let doc = TestSupport.newDocument()
        let r = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 100, y: 100))
        XCTAssertTrue(r.part.isGenericResistor)
        XCTAssertTrue(r.part.hasOhmsField)
        XCTAssertEqual(r.reference, "R1")
        XCTAssertEqual(r.ohms, 1000)
        XCTAssertEqual(r.footprintID, "RES-AXIAL-10.16")
        XCTAssertEqual(r.symbol, .resistor)
        XCTAssertEqual(ResistorSymbolView.label(for: r), "1kΩ")
        XCTAssertEqual(PartFacts.kindName(r.part), "Resistor")
    }

    /// A maker resistor and a pull take R2 and R3; a chip after them is U1.
    /// A maker part has no Ohms field: its value is its part's.
    func testReferences() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 100, y: 100))
        let maker = try TestSupport.place(doc, "RC0603FR-074K7L", at: CGPoint(x: 300, y: 100))
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 500, y: 100))
        let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 700, y: 100))
        XCTAssertEqual(maker.reference, "R2")
        XCTAssertEqual(pull.reference, "R3")
        XCTAssertEqual(chip.reference, "U1")
        XCTAssertFalse(maker.part.hasOhmsField)
        XCTAssertNil(maker.ohms)
        XCTAssertEqual(maker.part.resistorOhms, 4700)
        XCTAssertEqual(ResistorSymbolView.label(for: maker), "4.7kΩ")
        XCTAssertEqual(ReferenceCategory.of(maker.part), .resistor)
    }

    /// setPartOhms 4700 then undo restores 1 kΩ; a maker part ignores it.
    func testSetOhmsAndUndo() throws {
        let doc = TestSupport.newDocument()
        let r = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 100, y: 100))
        let maker = try TestSupport.place(doc, "RC0603FR-074K7L", at: CGPoint(x: 300, y: 100))
        doc.setPartOhms(r.id, 4700)
        XCTAssertEqual(doc.chips[0].ohms, 4700)
        XCTAssertEqual(ResistorSymbolView.label(for: doc.chips[0]), "4.7kΩ")
        XCTAssertEqual(doc.undoName, "Change Resistance")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.chips[0].ohms, 1000)
        doc.setPartOhms(maker.id, 1000)
        XCTAssertNil(doc.chips[1].ohms)
    }

    /// Save writes "ohms": 4700 for the generic; open restores it.
    func testRoundTrip() throws {
        let doc = TestSupport.newDocument()
        let r = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 100, y: 100))
        try TestSupport.place(doc, "RC0603FR-074K7L", at: CGPoint(x: 300, y: 100))
        doc.setPartOhms(r.id, 4700)
        let file = try SchematicFile.decode(SchematicFile(document: doc, fileURL: nil).encoded())
        XCTAssertEqual(file.parts[0].ohms, 4700)
        XCTAssertNil(file.parts[1].ohms)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.chips[0].ohms, 4700)
        XCTAssertEqual(opened.chips[0].reference, "R1")
        XCTAssertEqual(opened.chips[1].reference, "R2")
    }

    /// A file's GENERIC-RESISTOR with "ohms": 0 fails to open (badOhms).
    func testBadOhmsFails() {
        let json = """
        { "format": "\(SchematicFile.formatName)", "version": 4, "nextReference": 1,
          "parts": [{ "id": "\(UUID().uuidString)", "reference": "R1", "partNumber": "GENERIC-RESISTOR",
                      "x": 100, "y": 100, "flipped": false, "ohms": 0 }],
          "junctions": [], "wires": [] }
        """
        let opened = TestSupport.newDocument()
        XCTAssertThrowsError(try SchematicFile.decode(Data(json.utf8))
            .apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)) { error in
            guard case let SchematicFileError.badOhms(reference, ohms) = error else {
                return XCTFail("unexpected \(error)")
            }
            XCTAssertEqual(reference, "R1")
            XCTAssertEqual(ohms, 0)
        }
    }

    /// A generic 4.7 kΩ Axial and a pull 4.7 kΩ Axial share one RESISTOR
    /// line; a maker part's line has its part number and value 4.7kΩ.
    func testBillOfMaterials() throws {
        let doc = TestSupport.newDocument()
        let r = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 100, y: 100))
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 300, y: 100))
        try TestSupport.place(doc, "RC0603FR-074K7L", at: CGPoint(x: 500, y: 100))
        doc.setPartOhms(r.id, 4700)
        doc.setPartOhms(pull.id, 4700)
        doc.setPartFootprint(pull.id, "RES-AXIAL-10.16")
        let lines = BillOfMaterials.lines(for: doc.chips)
        let generic = try XCTUnwrap(lines.first { $0.partNumber == BillOfMaterials.resistorPartNumber })
        XCTAssertEqual(generic.references, ["R1", "R2"])
        XCTAssertEqual(generic.value, "4.7kΩ")
        XCTAssertEqual(generic.package, "Axial")
        let maker = try XCTUnwrap(lines.first { $0.partNumber == "RC0603FR-074K7L" })
        XCTAssertEqual(maker.value, "4.7kΩ")
        XCTAssertEqual(maker.references, ["R3"])
    }

    /// Discrete → Resistors → Generic beside Array / Chip / Through-hole (a
    /// third drawer level); each of those groups' leaves in increasing ohms.
    func testDrawerOrder() throws {
        let groups = TestSupport.catalog.groups(matching: DrawerFilter())
        let discrete = try XCTUnwrap(groups.first { $0.category == "Discrete" })
        XCTAssertNil(groups.first { $0.category == "Resistors" })
        let resistors = try XCTUnwrap(discrete.subcategories.first { $0.subcategory == "Resistors" })
        XCTAssertEqual(resistors.path, ["Discrete", "Resistors"])
        XCTAssertEqual(resistors.functions.map(\.function), ["Generic"])
        XCTAssertEqual(Set(resistors.subgroups.map(\.subcategory)), ["Array", "Chip", "Through-hole"])
        XCTAssertEqual(resistors.subgroups.first { $0.subcategory == "Chip" }?.path, ["Discrete", "Resistors", "Chip"])
        XCTAssertTrue(resistors.subgroups.allSatisfy(\.subgroups.isEmpty))
        XCTAssertTrue(TestSupport.catalog.kinds(inGroup: ["Discrete", "Resistors", "Chip"]) == [PART_RESISTOR.rawValue])
        let order = ["< 1 kΩ", "1 kΩ – 5 kΩ", "5 kΩ – 10 kΩ", "10 kΩ – 50 kΩ", "50 kΩ – 100 kΩ",
                     "100 kΩ – 500 kΩ", "500 kΩ – 1 MΩ", "≥ 1 MΩ"]
        for sub in resistors.subgroups {
            let leaves = sub.functions.map(\.function)
            XCTAssertEqual(leaves, order.filter(leaves.contains), sub.subcategory)
        }
        let chip = try XCTUnwrap(resistors.subgroups.first { $0.subcategory == "Chip" })
        XCTAssertEqual(chip.functions.map(\.function), order)
    }

    /// An array is a block named "Resistor array" with a Resistors fact.
    func testArray() throws {
        let array = try XCTUnwrap(TestSupport.allParts.first {
            if case let .resistor(_, _, _, n) = $0.details { return n > 1 }
            return false
        })
        XCTAssertEqual(symbol(for: array), .block)
        XCTAssertEqual(PartFacts.kindName(array), "Resistor array")
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, array.name, at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(chip.reference, "R1")
        XCTAssertTrue(PartFacts.rows(for: chip).contains { $0.label == "Resistors" })
    }
}
