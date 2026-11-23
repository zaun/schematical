import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-44-G: crystals and ceramic resonators. The kind names, facts rows
/// and drawer labels read the real catalog's `PartCrystal` facts; a crystal
/// takes `Y<n>` references, draws as the plate-rectangle-plate symbol with
/// `X1` on the left and `X2` on the right (a three-pin resonator's `GND` at
/// the bottom centre, a four-pad part's `NC` pads not drawn), and the drawer
/// lists the frequency bands in order.
@MainActor
final class CrystalTests: XCTestCase {
    private func value(_ chip: PlacedChip, _ label: String) -> String? {
        PartFacts.rows(for: chip).first { $0.label == label }?.value
    }

    /// A placed GENERIC-CRYSTAL-8M is Y1, 8 MHz, on its default HC-49/US footprint.
    func testPlacingTheGenericCrystal() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "GENERIC-CRYSTAL-8M", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(chip.reference, "Y1")
        XCTAssertEqual(chip.part.package, "HC-49/US")
        XCTAssertEqual(chip.footprintID, "XTAL-HC49U-2-ABRACON")
        XCTAssertEqual(chip.part.footprintChoices.count, 4)
        XCTAssertEqual(chip.symbol, .crystal)
        XCTAssertEqual(PartFacts.kindName(chip.part), "Crystal")
        XCTAssertEqual(PartFacts.kindName(PART_CRYSTAL), "Crystal")
        XCTAssertEqual(value(chip, "Type"), "Crystal")
        XCTAssertEqual(value(chip, "Frequency"), "8 MHz")
        XCTAssertNil(value(chip, "Load capacitance"))
        XCTAssertNil(value(chip, "Built-in capacitors"))
        XCTAssertEqual(chip.part.trailingLabel, "8 MHz")
        XCTAssertEqual(chip.part.crystalHz, 8_000_000)
        XCTAssertEqual(ReferenceCategory.of(chip.part), .crystal)
    }

    /// References count Y1, Y2; a gate is still U1.
    func testReferences() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "GENERIC-CRYSTAL-32768", at: CGPoint(x: 100, y: 100))
        let second = try TestSupport.place(doc, "GENERIC-CRYSTAL-16M", at: CGPoint(x: 300, y: 100))
        let resonator = try TestSupport.place(doc, "GENERIC-RESONATOR-8M", at: CGPoint(x: 500, y: 100))
        let gate = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 700, y: 100))
        XCTAssertEqual(second.reference, "Y2")
        XCTAssertEqual(resonator.reference, "Y3")
        XCTAssertEqual(gate.reference, "U1")
    }

    /// The resonator is a "Ceramic resonator" with its Built-in capacitors row,
    /// the symbol with a GND lead, and 3 pads on every footprint choice.
    func testTheGenericResonator() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "GENERIC-RESONATOR-8M", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(PartFacts.kindName(chip.part), "Ceramic resonator")
        XCTAssertEqual(value(chip, "Type"), "Ceramic resonator")
        XCTAssertEqual(value(chip, "Built-in capacitors"), "Yes")
        XCTAssertEqual(chip.symbol, .crystal)
        XCTAssertEqual(chip.part.package, "SMD")
        XCTAssertEqual(chip.part.footprintChoices.count, 4)
    }

    /// A catalog crystal's facts: load capacitance, tolerance, stability, ESR,
    /// and the drawer's second line.
    func testCatalogFacts() throws {
        let doc = TestSupport.newDocument()
        let part = try TestSupport.part("ABS07-32.768KHZ-T")
        let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(value(chip, "Frequency"), "32.768 kHz")
        XCTAssertEqual(value(chip, "Load capacitance"), "12.5 pF")
        XCTAssertEqual(value(chip, "Tolerance"), "±20 ppm")
        XCTAssertEqual(value(chip, "ESR"), "70 kΩ")
        XCTAssertEqual(part.trailingLabel, "32.768 kHz")
        XCTAssertEqual(part.detailLabels, ["12.5 pF", "±20 ppm"])
        XCTAssertEqual(part.drawer, ["Clocks", "Crystal", "kHz"])
        XCTAssertEqual(part.function, "32.768 kHz")
        XCTAssertEqual(part.manufacturer, "Abracon")
    }

    /// Every crystal-kind part draws as a crystal, X1 left and X2 right; a
    /// three-pin resonator's GND at the bottom centre; no NC pad is drawn.
    func testLayout() throws {
        let parts = TestSupport.allParts.filter { $0.kind == PART_CRYSTAL }
        XCTAssertEqual(parts.count, 471)   // 604 parts, minus the 133 AEC-Q200 ones the drawer hides by default
        for part in parts {
            XCTAssertEqual(symbol(for: part), .crystal, part.name)
            let layout = ChipLayout(part: part, symbol: .crystal)
            let names = layout.slots.map(\.pin.name)
            XCTAssertTrue(names.contains("X1") && names.contains("X2"), part.name)
            XCTAssertEqual(layout.slots.first { $0.pin.name == "X1" }?.side, .left, part.name)
            XCTAssertEqual(layout.slots.first { $0.pin.name == "X2" }?.side, .right, part.name)
            XCTAssertFalse(names.contains("NC"), part.name)
            if let ground = layout.slots.first(where: { $0.pin.name == "GND" }) {
                XCTAssertEqual(ground.side, .bottom, part.name)
                XCTAssertEqual(ground.centerX, layout.size.width / 2, part.name)
            }
        }
        let resonator = try TestSupport.part("GENERIC-RESONATOR-8M")
        XCTAssertEqual(ChipLayout(part: resonator, symbol: .crystal).slots.map(\.pin.name).sorted(), ["GND", "X1", "X2"])
        let four = try XCTUnwrap(parts.first { $0.pins.count == 4 && $0.pins.contains { $0.name == "NC" } })
        XCTAssertEqual(ChipLayout(part: four, symbol: .crystal).slots.count, 2)
    }

    /// A placed crystal, resonator and four-pad part put ink on the export scene.
    func testRendering() throws {
        let doc = TestSupport.newDocument()
        let four = try XCTUnwrap(TestSupport.allParts.first {
            $0.kind == PART_CRYSTAL && $0.pins.count == 4 && $0.pins.contains { $0.name == "NC" }
        })
        for (index, name) in ["GENERIC-CRYSTAL-8M", "GENERIC-RESONATOR-8M", four.name].enumerated() {
            let chip = try TestSupport.place(doc, name, at: CGPoint(x: 100 + 200 * index, y: 100))
            var painter = BoundsPainter()
            var scene = SchematicScene(doc)
            scene.chips = [chip]
            scene.paint(into: &painter)
            let rect = try XCTUnwrap(painter.box.rect, name)
            XCTAssertGreaterThan(rect.width, 40, name)
            XCTAssertGreaterThan(rect.height, 14, name)
        }
    }

    /// Clocks → Crystal (kHz / MHz, with Generic beside them), Ceramic resonator
    /// (with its Generic), Oscillator, and the RTC leaf; the leaves in
    /// frequency order, and a leaf's parts by frequency.
    func testDrawerOrder() throws {
        let groups = TestSupport.catalog.groups(matching: DrawerFilter())
        let clocks = try XCTUnwrap(groups.first { $0.category == "Clocks" })
        XCTAssertEqual(Set(clocks.subcategories.map(\.subcategory)), ["Crystal", "Ceramic resonator", "Oscillator"])
        XCTAssertEqual(clocks.functions.map(\.function), ["RTC"])
        let crystal = try XCTUnwrap(clocks.subcategories.first { $0.subcategory == "Crystal" })
        let resonator = try XCTUnwrap(clocks.subcategories.first { $0.subcategory == "Ceramic resonator" })
        XCTAssertEqual(Set(crystal.subgroups.map(\.subcategory)), ["kHz", "MHz"])
        XCTAssertEqual(crystal.functions.map(\.function), ["Generic"])
        XCTAssertEqual(Set(crystal.functions[0].parts.map(\.name)),
                       ["GENERIC-CRYSTAL-32768", "GENERIC-CRYSTAL-8M", "GENERIC-CRYSTAL-16M"])
        XCTAssertEqual(Set(resonator.functions.filter { $0.function == "Generic" }.flatMap(\.parts).map(\.name)),
                       ["GENERIC-RESONATOR-8M"])
        func leaves(_ name: String) throws -> [PartCatalogStore.FunctionGroup] {
            if name == "Ceramic resonator" { return resonator.functions.filter { $0.function != "Generic" } }
            return try XCTUnwrap(crystal.subgroups.first { $0.subcategory == name }).functions
        }
        XCTAssertEqual(try leaves("kHz").map(\.function), ["32 kHz", "32.768 kHz"])
        XCTAssertEqual(try leaves("MHz").map(\.function),
                       ["Under 4 MHz", "4 MHz to 8 MHz", "8 MHz to 16 MHz", "16 MHz to 32 MHz", "32 MHz and over"])
        XCTAssertEqual(try leaves("Ceramic resonator").map(\.function),
                       ["Under 4 MHz", "4 MHz to 8 MHz", "8 MHz to 16 MHz", "16 MHz and over"])
        for leaf in try leaves("MHz") + leaves("Ceramic resonator") + leaves("kHz") {
            let hz = leaf.parts.compactMap(\.crystalHz)
            XCTAssertEqual(hz, hz.sorted(), leaf.function)
        }
    }
}
