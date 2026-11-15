import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-40-H: transistor-kind parts in the app. Every transistor takes the
/// lowest free Q<n>; a single BJT or MOSFET draws as the transistor symbol,
/// an array as a block; the sidebar names the kind and the ratings; the
/// generics carry five footprint choices; the drawer lists the BJT and
/// MOSFET leaves.
@MainActor
final class TransistorTests: XCTestCase {
    private func info(_ part: CatalogPart) -> TransistorInfo? {
        if case let .transistor(info) = part.details { return info }
        return nil
    }

    /// The first catalog transistor matching `test`, most specific first.
    private func firstTransistor(_ what: String, _ test: (CatalogPart, TransistorInfo) -> Bool) throws -> CatalogPart {
        try XCTUnwrap(TestSupport.allParts.first { part in
            guard let i = info(part), !part.name.hasPrefix("GENERIC-") else { return false }
            return test(part, i)
        }, what)
    }

    private func row(_ chip: PlacedChip, _ label: String) -> String? {
        PartFacts.rows(for: chip).first { $0.label == label }?.value
    }

    /// A placed GENERIC-NPN is Q1 on TO-92 with five footprint choices,
    /// drawn as a transistor, named "Transistor".
    func testPlacingTheGenericNPN() throws {
        let doc = TestSupport.newDocument()
        let q = try TestSupport.place(doc, "GENERIC-NPN", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(q.reference, "Q1")
        XCTAssertEqual(q.part.package, "TO-92")
        XCTAssertEqual(q.footprintID, q.part.footprintChoices.first?.footprint.id)
        XCTAssertEqual(q.part.footprintChoices.map(\.package), ["TO-92", "SOT-23", "SOT-223", "TO-220", "DPAK"])
        XCTAssertEqual(q.symbol, .transistor)
        XCTAssertEqual(PartFacts.kindName(q.part), "Transistor")
        XCTAssertEqual(row(q, "Type"), "NPN")
        XCTAssertNil(row(q, "Vceo"), "a generic has no ratings")
        for name in ["GENERIC-PNP", "GENERIC-NMOS", "GENERIC-PMOS"] {
            let part = try TestSupport.part(name)
            XCTAssertEqual(symbol(for: part), .transistor, name)
            XCTAssertEqual(part.footprintChoices.count, 5, name)
        }
    }

    /// Every transistor-kind part takes the next Q; a chip after them is U1.
    func testReferences() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "GENERIC-NPN", at: CGPoint(x: 100, y: 100))
        let second = try TestSupport.place(doc, "GENERIC-PMOS", at: CGPoint(x: 300, y: 100))
        let third = try TestSupport.place(doc, "2N7000", at: CGPoint(x: 500, y: 100))
        let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 700, y: 100))
        XCTAssertEqual(second.reference, "Q2")
        XCTAssertEqual(third.reference, "Q3")
        XCTAssertEqual(chip.reference, "U1")
        XCTAssertEqual(ReferenceCategory.of(third.part), .transistor)
    }

    /// 2N3904BU: Transistor, Type NPN, Vceo 40 V, Ic 0.2 A, Power, hFE 100.
    func testBJTFacts() throws {
        let doc = TestSupport.newDocument()
        let q = try TestSupport.place(doc, "2N3904BU", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(PartFacts.kindName(q.part), "Transistor")
        XCTAssertEqual(row(q, "Type"), "NPN")
        XCTAssertEqual(row(q, "Vceo"), "40 V")
        XCTAssertEqual(row(q, "Ic"), "0.2 A")
        XCTAssertEqual(row(q, "Power"), "0.625 W")
        XCTAssertEqual(row(q, "hFE"), "100")
        XCTAssertEqual(q.symbol, .transistor)
        XCTAssertEqual(q.part.detailLabels, ["40 V", "0.2 A"])
    }

    /// A logic-level MOSFET: Vds, Id, Rds(on), Vgs(th) "≤", Vgs "±".
    func testMOSFETFacts() throws {
        let doc = TestSupport.newDocument()
        let q = try TestSupport.place(doc, "IRLML6244TRPBF", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(row(q, "Type"), "N-channel logic-level")
        XCTAssertEqual(row(q, "Vds"), "20 V")
        XCTAssertEqual(row(q, "Id"), "6.3 A")
        XCTAssertEqual(row(q, "Rds(on)"), "21 mΩ")
        XCTAssertEqual(row(q, "Vgs(th)"), "≤ 1.1 V")
        XCTAssertEqual(row(q, "Vgs"), "±12 V")
        XCTAssertEqual(q.symbol, .transistor)
    }

    /// A pre-biased part shows R1 and R2; a Darlington is named so.
    func testPreBiasedAndDarlington() throws {
        let pre = try firstTransistor("a pre-biased part with R1 and R2") { _, i in
            i.elements == 1 && i.r1Milliohm > 0 && i.r2Milliohm > 0
        }
        let doc = TestSupport.newDocument()
        let q = try TestSupport.place(doc, pre.name, at: CGPoint(x: 100, y: 100))
        XCTAssertTrue(row(q, "Type")?.hasSuffix("pre-biased") ?? false)
        XCTAssertNotNil(row(q, "R1"))
        XCTAssertNotNil(row(q, "R2"))
        XCTAssertEqual(symbol(for: pre), .transistor)
        let darlington = try firstTransistor("a Darlington") { _, i in i.darlington && i.elements == 1 }
        let d = try TestSupport.place(doc, darlington.name, at: CGPoint(x: 300, y: 100))
        XCTAssertTrue(row(d, "Type")?.hasSuffix("Darlington") ?? false)
        XCTAssertEqual(symbol(for: darlington), .transistor)
    }

    /// A dual array is a block named "Transistor array" with an Elements row;
    /// a complementary pair says so.
    func testArrays() throws {
        let dual = try TestSupport.part("BC846SZ")
        XCTAssertEqual(PartFacts.kindName(dual), "Transistor array")
        XCTAssertEqual(symbol(for: dual), .block)
        let doc = TestSupport.newDocument()
        let q = try TestSupport.place(doc, dual.name, at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(q.reference, "Q1")
        XCTAssertEqual(row(q, "Elements"), "2")
        let pair = try firstTransistor("a complementary BJT pair") { _, i in i.mixed && !i.mosfet }
        let p = try TestSupport.place(doc, pair.name, at: CGPoint(x: 300, y: 100))
        XCTAssertEqual(row(p, "Type"), "NPN + PNP")
    }

    /// Every pin name of every placed kind has a layout slot; a tab that
    /// repeats a name (G,D,S,D) gives three slots.
    func testEveryPinHasASlot() throws {
        let tab = try firstTransistor("a tab part, G,D,S,D") { part, i in
            i.elements == 1 && part.pins.map(\.name) == ["G", "D", "S", "D"]
        }
        let doc = TestSupport.newDocument()
        var picks: [CatalogPart] = [tab]
        for (what, test) in [
            ("an NPN", { (_: CatalogPart, i: TransistorInfo) in i.type == "npn" && i.elements == 1 }),
            ("a PNP", { (_: CatalogPart, i: TransistorInfo) in i.type == "pnp" && i.elements == 1 }),
            ("an N-channel", { (_: CatalogPart, i: TransistorInfo) in i.type == "nmos" && i.elements == 1 }),
            ("a P-channel", { (_: CatalogPart, i: TransistorInfo) in i.type == "pmos" && i.elements == 1 }),
            ("an array", { (_: CatalogPart, i: TransistorInfo) in i.elements > 1 }),
        ] as [(String, (CatalogPart, TransistorInfo) -> Bool)] {
            picks.append(try firstTransistor(what, test))
        }
        picks += try ["GENERIC-NPN", "GENERIC-NMOS"].map { try TestSupport.part($0) }
        for (k, part) in picks.enumerated() {
            let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 100 + 200 * k, y: 100))
            let slotted = Set(chip.layout.slots.map(\.pin.name))
            let names = Set(part.pins.filter { !$0.isHidden }.map(\.name))
            XCTAssertEqual(slotted, names, part.name)
        }
        let tabChip = try TestSupport.place(doc, tab.name, at: CGPoint(x: 100, y: 400))
        XCTAssertEqual(tabChip.symbol, .transistor)
        XCTAssertEqual(tabChip.layout.slots.count, 3)
    }

    /// Discrete → Transistors: the Generic leaf beside BJT and MOSFET, whose
    /// leaves sort as the drawer shows them.
    func testDrawer() throws {
        let groups = TestSupport.catalog.groups(matching: DrawerFilter())
        let discrete = try XCTUnwrap(groups.first { $0.category == "Discrete" })
        let transistors = try XCTUnwrap(discrete.subcategories.first { $0.subcategory == "Transistors" })
        XCTAssertEqual(transistors.functions.map(\.function), ["Generic"])
        XCTAssertEqual(Set(transistors.subgroups.map(\.subcategory)), ["BJT", "MOSFET"])
        let bjt = try XCTUnwrap(transistors.subgroups.first { $0.subcategory == "BJT" })
        XCTAssertEqual(bjt.functions.map(\.function),
                       ["Array", "NPN", "NPN Darlington", "NPN pre-biased", "PNP", "PNP Darlington", "PNP pre-biased"])
        let mosfet = try XCTUnwrap(transistors.subgroups.first { $0.subcategory == "MOSFET" })
        XCTAssertEqual(mosfet.functions.map(\.function),
                       ["Array", "N-channel", "N-channel logic-level", "P-channel", "P-channel logic-level"])
        XCTAssertEqual(TestSupport.catalog.kinds(inGroup: ["Discrete", "Transistors", "BJT"]), [PART_TRANSISTOR.rawValue])
    }

    /// Two GENERIC-NPN on TO-92 share a line; one on SOT-23 is its own; no value.
    func testBillOfMaterials() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "GENERIC-NPN", at: CGPoint(x: 100, y: 100))
        try TestSupport.place(doc, "GENERIC-NPN", at: CGPoint(x: 300, y: 100))
        let third = try TestSupport.place(doc, "GENERIC-NPN", at: CGPoint(x: 500, y: 100))
        let sot23 = try XCTUnwrap(third.part.footprintChoices.first { $0.package == "SOT-23" })
        doc.setPartFootprint(third.id, sot23.footprint.id)
        let lines = BillOfMaterials.lines(for: doc.chips).filter { $0.partNumber == "GENERIC-NPN" }
        XCTAssertEqual(lines.count, 2)
        let to92 = try XCTUnwrap(lines.first { $0.package == "TO-92" })
        XCTAssertEqual(to92.references, ["Q1", "Q2"])
        XCTAssertEqual(to92.value, "")
        let own = try XCTUnwrap(lines.first { $0.package == "SOT-23" })
        XCTAssertEqual(own.references, ["Q3"])
    }

    /// Choosing SOT-23, saving and reopening keeps the choice.
    func testFootprintChoiceRoundTrip() throws {
        let doc = TestSupport.newDocument()
        let q = try TestSupport.place(doc, "GENERIC-NPN", at: CGPoint(x: 100, y: 100))
        let sot23 = try XCTUnwrap(q.part.footprintChoices.first { $0.package == "SOT-23" })
        doc.setPartFootprint(q.id, sot23.footprint.id)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.chips[0].footprintID, sot23.footprint.id)
        XCTAssertEqual(opened.chips[0].reference, "Q1")
    }
}
