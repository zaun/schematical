import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-47-M: inductors, fuses, PTC resettable fuses, relays and buzzers in
/// the app. The kind names, facts rows, drawer labels and references read the
/// real catalog's facts (one real part per kind and type: the most stocked
/// `add` row of plan-47's pick); inductors and fuses draw as the coil and the
/// rectangle with pin 1 on the left and pin 2 on the right; the drawer lists
/// them by value; and in play mode a click blows a fuse and a second restores
/// it. The part counts are never hard-coded: the part files are regenerated.
@MainActor
final class InductorFuseTests: XCTestCase {
    /// Kept for the life of the test process, like `TestSupport`'s documents.
    private static var keptSimulations: [SimulationController] = []

    private func value(_ chip: PlacedChip, _ label: String) -> String? {
        PartFacts.rows(for: chip).first { $0.label == label }?.value
    }

    private func place(_ name: String, in doc: SchematicDocument, at x: CGFloat = 100) throws -> PlacedChip {
        try TestSupport.place(doc, name, at: CGPoint(x: x, y: 100))
    }

    private var everything: DrawerFilter {
        var filter = DrawerFilter()
        filter.visibility = PartVisibility(showObsolete: true, showMilitary: true, showAutomotive: true)
        return filter
    }

    private func parts(_ kind: PartKind) -> [CatalogPart] {
        TestSupport.catalog.allParts.filter { $0.kind == kind }
    }

    // MARK: - Phase 1: facts, kind names, references

    func testInductorValueText() {
        XCTAssertEqual(InductorValue.text(pH: 100), "100 pH")
        XCTAssertEqual(InductorValue.text(pH: 10_000), "10 nH")
        XCTAssertEqual(InductorValue.text(pH: 1_800), "1.8 nH")
        XCTAssertEqual(InductorValue.text(pH: 4_700_000), "4.7 µH")
        XCTAssertEqual(InductorValue.text(pH: 1_000_000_000), "1 mH")
        XCTAssertEqual(InductorValue.text(pH: 22_000_000_000_000), "22 H")
        XCTAssertEqual(PartFacts.amps(mA: 500), "500 mA")
        XCTAssertEqual(PartFacts.amps(mA: 1600), "1.6 A")
        XCTAssertEqual(PartFacts.volts(mV: 250_000), "250 V")
        XCTAssertEqual(PartFacts.volts(mV: 500), "0.5 V")
        XCTAssertEqual(PartFacts.microohmsText(29_000), "29 mΩ")
        XCTAssertEqual(PartFacts.microohmsText(1_500_000), "1.5 Ω")
    }

    /// LQG15HN18NJ02D (Murata, 18 nH): the inductor's facts, row by row.
    func testInductorFacts() throws {
        let doc = TestSupport.newDocument()
        let chip = try place("LQG15HN18NJ02D", in: doc)
        XCTAssertEqual(PartFacts.kindName(chip.part), "Inductor")
        XCTAssertEqual(PartFacts.kindName(PART_INDUCTOR), "Inductor")
        XCTAssertEqual(value(chip, "Kind"), "Inductor")
        XCTAssertEqual(value(chip, "Inductance"), "18 nH")
        XCTAssertEqual(value(chip, "Tolerance"), "±5%")
        XCTAssertEqual(value(chip, "Rated current"), "350 mA")
        XCTAssertNil(value(chip, "Saturation current"))
        XCTAssertEqual(value(chip, "DC resistance"), "510 mΩ")
        XCTAssertEqual(value(chip, "Shielding"), "Unshielded")
        XCTAssertEqual(chip.part.trailingLabel, "18 nH")
        XCTAssertEqual(chip.part.detailLabels, ["350 mA", "Unshielded"])
        XCTAssertEqual(ReferenceCategory.of(chip.part), .inductor)
        XCTAssertEqual(chip.reference, "L1")
    }

    /// SS-5F-10A-AP (Eaton): a fast fuse rated for AC only; 0ZCM0010FF2G (Bel
    /// Fuse): a PTC with its hold, trip, voltage and fault current.
    func testFuseAndPTCFacts() throws {
        let doc = TestSupport.newDocument()
        let fuse = try place("SS-5F-10A-AP", in: doc)
        XCTAssertEqual(PartFacts.kindName(fuse.part), "Fuse")
        XCTAssertEqual(PartFacts.kindName(PART_FUSE), "Fuse")
        XCTAssertEqual(value(fuse, "Type"), "Fuse")
        XCTAssertEqual(value(fuse, "Rated current"), "10 A")
        XCTAssertEqual(value(fuse, "Speed"), "Fast")
        XCTAssertEqual(value(fuse, "Voltage (AC)"), "125 V")
        XCTAssertNil(value(fuse, "Voltage (DC)"))
        XCTAssertEqual(value(fuse, "Breaking capacity"), "50 A")
        XCTAssertEqual(fuse.part.trailingLabel, "10 A")
        XCTAssertEqual(fuse.part.detailLabels, ["Fast", "125 V"])
        XCTAssertEqual(ReferenceCategory.of(fuse.part), .fuse)
        XCTAssertEqual(fuse.reference, "F1")

        let ptc = try place("0ZCM0010FF2G", in: doc, at: 300)
        XCTAssertEqual(PartFacts.kindName(ptc.part), "Resettable fuse")
        XCTAssertEqual(value(ptc, "Kind"), "Resettable fuse")
        XCTAssertEqual(value(ptc, "Type"), "Resettable fuse (PTC)")
        XCTAssertEqual(value(ptc, "Hold current"), "100 mA")
        XCTAssertEqual(value(ptc, "Trip current"), "250 mA")
        XCTAssertEqual(value(ptc, "Maximum voltage"), "15 V")
        XCTAssertEqual(value(ptc, "Maximum current"), "40 A")
        XCTAssertNil(value(ptc, "Speed"))
        XCTAssertEqual(ptc.part.trailingLabel, "100 mA")
        XCTAssertEqual(ptc.part.detailLabels, ["250 mA trip", "15 V"])
        XCTAssertEqual(ReferenceCategory.of(ptc.part), .fuse)
        XCTAssertEqual(ptc.reference, "F2")
    }

    /// An electromechanical, a reed and a solid-state relay, and a latching one.
    func testRelayFacts() throws {
        let doc = TestSupport.newDocument()
        let power = try place("RZ03-1A3-D005-R", in: doc)
        XCTAssertEqual(PartFacts.kindName(power.part), "Relay")
        XCTAssertEqual(PartFacts.kindName(PART_RELAY), "Relay")
        XCTAssertEqual(value(power, "Type"), "Electromechanical")
        XCTAssertEqual(value(power, "Contacts"), power.part.function)
        XCTAssertEqual(value(power, "Coil voltage"), "5 V")
        XCTAssertEqual(value(power, "Coil resistance"), "60 Ω")
        XCTAssertEqual(value(power, "Contact current"), "16 A")
        XCTAssertEqual(value(power, "Operate time"), "8 ms")
        XCTAssertEqual(value(power, "Release time"), "6 ms")
        XCTAssertNil(value(power, "Latching"))
        XCTAssertEqual(power.part.trailingLabel, "5 V")
        XCTAssertEqual(power.part.detailLabels, ["16 A"])
        XCTAssertEqual(ReferenceCategory.of(power.part), .relay)
        XCTAssertEqual(power.reference, "K1")

        let reed = try place("JWD-172-1", in: doc, at: 300)
        XCTAssertEqual(PartFacts.kindName(reed.part), "Reed relay")
        XCTAssertEqual(value(reed, "Type"), "Reed")
        XCTAssertEqual(value(reed, "Contact current"), "500 mA")
        XCTAssertEqual(value(reed, "Switching voltage"), "28 V")
        XCTAssertEqual(value(reed, "Operate time"), "1.5 ms")
        XCTAssertEqual(reed.part.trailingLabel, "5 V")

        let ssr = try place("AQV210EHAX", in: doc, at: 500)
        XCTAssertEqual(PartFacts.kindName(ssr.part), "Solid-state relay")
        XCTAssertEqual(value(ssr, "Type"), "Solid-state")
        XCTAssertNil(value(ssr, "Coil voltage"))
        XCTAssertEqual(value(ssr, "Contact current"), "130 mA")
        XCTAssertEqual(value(ssr, "Switching voltage"), "350 V")
        XCTAssertEqual(value(ssr, "On-resistance"), "35 Ω")
        XCTAssertEqual(ssr.part.trailingLabel, "130 mA")

        let latching = try XCTUnwrap(parts(PART_RELAY).first { $0.relay?.latching == true })
        let latchingChip = try place(latching.name, in: doc, at: 700)
        XCTAssertEqual(value(latchingChip, "Latching"), "Yes")
        XCTAssertTrue(latching.detailLabels.contains("latching"))
    }

    /// SMT-0825-S-4-R (PUI Audio): a transducer with four pads.
    func testBuzzerFacts() throws {
        let doc = TestSupport.newDocument()
        let chip = try place("SMT-0825-S-4-R", in: doc)
        let info = try XCTUnwrap(chip.part.buzzer)
        XCTAssertEqual(PartFacts.kindName(chip.part), "Buzzer")
        XCTAssertEqual(PartFacts.kindName(PART_BUZZER), "Buzzer")
        XCTAssertEqual(value(chip, "Type"), "Transducer (external drive)")
        XCTAssertEqual(value(chip, "Technology"), PartFacts.capitalized(info.technology))
        XCTAssertEqual(value(chip, "Frequency"), "2.5 kHz")
        XCTAssertEqual(value(chip, "Rated voltage"), "3.6 V")
        XCTAssertEqual(value(chip, "Voltage range"), "2.5–4.5 V")
        XCTAssertEqual(value(chip, "Current"), "90 mA")
        XCTAssertEqual(value(chip, "Sound level"), "85 dB")
        XCTAssertEqual(chip.part.trailingLabel, "2.5 kHz")
        XCTAssertEqual(chip.part.detailLabels, ["3.6 V", PartFacts.capitalized(info.technology)])
        XCTAssertEqual(ReferenceCategory.of(chip.part), .speaker)
        XCTAssertEqual(chip.reference, "LS1")

        let indicator = try XCTUnwrap(parts(PART_BUZZER).first { $0.buzzer?.driver == "indicator" })
        let indicatorChip = try place(indicator.name, in: doc, at: 300)
        XCTAssertEqual(value(indicatorChip, "Type"), "Indicator (built-in driver)")
    }

    /// The lowest free number of each prefix, kinds mixed in any order.
    func testReferences() throws {
        let doc = TestSupport.newDocument()
        let names = ["LQG15HN18NJ02D", "SS-5F-10A-AP", "RZ03-1A3-D005-R", "SMT-0825-S-4-R", "SN74HC04N",
                     "LQG15HN18NJ02D", "0ZCM0010FF2G", "JWD-172-1", "SMT-0825-S-4-R"]
        var references: [String] = []
        for (index, name) in names.enumerated() {
            references.append(try place(name, in: doc, at: CGFloat(100 + 150 * index)).reference)
        }
        XCTAssertEqual(references, ["L1", "F1", "K1", "LS1", "U1", "L2", "F2", "K2", "LS2"])
    }

    /// Every part of the four kinds has its kind named, a facts row for its
    /// kind and a drawer label: nothing falls to the empty default.
    func testEveryPartReadsAsItsKind() throws {
        let kinds: [PartKind] = [PART_INDUCTOR, PART_FUSE, PART_RELAY, PART_BUZZER]
        for kind in kinds {
            let all = parts(kind)
            XCTAssertFalse(all.isEmpty, "no part of kind \(kind.rawValue)")
            for part in all {
                XCTAssertFalse(PartFacts.kindName(part).isEmpty, part.name)
                XCTAssertNotNil(part.trailingLabel, part.name)
                XCTAssertFalse(part.detailLabels.isEmpty, part.name)
                XCTAssertNotEqual(ReferenceCategory.of(part), .integratedCircuit, part.name)
            }
        }
    }

    // MARK: - Phase 2: the symbols

    func testSymbolFor() throws {
        XCTAssertEqual(symbol(for: try TestSupport.part("LQG15HN18NJ02D")), .inductor)
        XCTAssertEqual(symbol(for: try TestSupport.part("SS-5F-10A-AP")), .fuse)
        XCTAssertEqual(symbol(for: try TestSupport.part("0ZCM0010FF2G")), .fuse)
        let cartridge = try XCTUnwrap(parts(PART_FUSE).first { $0.drawer.last == "Cartridge" })
        XCTAssertEqual(symbol(for: cartridge), .fuse, cartridge.name)
        // Relays and buzzers draw as blocks until plan-47-N.
        XCTAssertEqual(symbol(for: try TestSupport.part("RZ03-1A3-D005-R")), .block)
        XCTAssertEqual(symbol(for: try TestSupport.part("SMT-0825-S-4-R")), .block)
    }

    /// Every inductor and fuse part draws as such: pin 1 left, pin 2 right, no
    /// pad of another name drawn (a four-pad inductor's NC pads).
    func testEveryInductorAndFuseGetsItsSymbol() throws {
        for (kind, want) in [(PART_INDUCTOR, PartSymbol.inductor), (PART_FUSE, .fuse)] {
            let all = parts(kind)
            XCTAssertFalse(all.isEmpty)
            for part in all {
                XCTAssertEqual(symbol(for: part), want, part.name)
                let layout = ChipLayout(part: part, symbol: want)
                XCTAssertEqual(layout.slots.map(\.pin.name).sorted(), ["1", "2"], part.name)
                XCTAssertEqual(layout.slots.first { $0.pin.name == "1" }?.side, .left, part.name)
                XCTAssertEqual(layout.slots.first { $0.pin.name == "2" }?.side, .right, part.name)
                XCTAssertEqual(layout.size, ChipLayout.diodeSize, part.name)
            }
        }
        let four = try XCTUnwrap(parts(PART_INDUCTOR).first { $0.pins.count == 4 })
        XCTAssertEqual(symbol(for: four), .inductor)
        XCTAssertEqual(ChipLayout(part: four, symbol: .inductor).slots.count, 2)
    }

    /// A part whose pins are not named 1 and 2 stays a block.
    func testOtherPinNamesStayABlock() throws {
        let gate = try TestSupport.part("SN74HC04N")
        let layout = ChipLayout(part: gate, symbol: .inductor)
        XCTAssertEqual(layout.slots.count, gate.pins.filter { !$0.isHidden }.count)
    }

    /// A placed inductor, fuse, PTC, cartridge fuse, four-pad inductor and a
    /// blown fuse put ink on the export scene (and a blown one more than an
    /// intact one has strokes, its own colour not counting in the export).
    func testRendering() throws {
        let doc = TestSupport.newDocument()
        let four = try XCTUnwrap(parts(PART_INDUCTOR).first { $0.pins.count == 4 })
        let cartridge = try XCTUnwrap(parts(PART_FUSE).first { $0.drawer.last == "Cartridge" })
        let names = ["LQG15HN18NJ02D", "SS-5F-10A-AP", "0ZCM0010FF2G", cartridge.name, four.name]
        for (index, name) in names.enumerated() {
            let chip = try place(name, in: doc, at: CGFloat(100 + 200 * index))
            var painter = BoundsPainter()
            var scene = SchematicScene(doc)
            scene.chips = [chip]
            scene.paint(into: &painter)
            let rect = try XCTUnwrap(painter.box.rect, name)
            XCTAssertGreaterThan(rect.width, 40, name)
            XCTAssertGreaterThan(rect.height, 14, name)
        }
        var blown = BoundsPainter()
        FuseSymbolView.paint(&blown, chip: try XCTUnwrap(doc.chips.first { $0.partNumber == "SS-5F-10A-AP" }),
                             isSelected: false, isHovered: false, isBlown: true)
        XCTAssertNotNil(blown.box.rect)
    }

    /// The value text above each symbol.
    func testSymbolLabels() throws {
        let doc = TestSupport.newDocument()
        XCTAssertEqual(InductorSymbolView.label(for: try place("LQG15HN18NJ02D", in: doc)), "18 nH")
        XCTAssertEqual(FuseSymbolView.label(for: try place("SS-5F-10A-AP", in: doc, at: 300)), "10 A")
        XCTAssertEqual(FuseSymbolView.label(for: try place("0ZCM0010FF2G", in: doc, at: 500)), "100 mA")
    }

    // MARK: - Phase 3: the drawer

    /// Discrete → Inductors / Fuses / Resettable fuses, relays and buzzers
    /// under the paths the part files give.
    func testPaths() throws {
        let groups = TestSupport.catalog.groups(matching: everything)
        let discrete = try XCTUnwrap(groups.first { $0.category == "Discrete" })
        let names = Set(discrete.subcategories.map(\.subcategory))
        XCTAssertTrue(names.isSuperset(of: ["Inductors", "Fuses", "Resettable fuses"]), "\(names)")
        for part in parts(PART_INDUCTOR) { XCTAssertEqual(Array(part.drawer.prefix(2)), ["Discrete", "Inductors"], part.name) }
        for part in parts(PART_FUSE) {
            let want = part.fuse?.isPTC == true ? "Resettable fuses" : "Fuses"
            XCTAssertEqual(Array(part.drawer.prefix(2)), ["Discrete", want], part.name)
        }
        for part in parts(PART_RELAY) {
            XCTAssertEqual(Array(part.drawer.prefix(2)), ["Switches & Relays", "Relay"], part.name)
        }
        for part in parts(PART_BUZZER) { XCTAssertEqual(part.drawer, ["Displays & Indicators", "Buzzers"], part.name) }
    }

    /// The inductor, fuse and PTC leaves go by the smallest value in each,
    /// and a leaf's parts by value, then name.
    func testValueOrder() throws {
        let groups = TestSupport.catalog.groups(matching: everything)
        let discrete = try XCTUnwrap(groups.first { $0.category == "Discrete" })
        for name in ["Inductors", "Fuses", "Resettable fuses"] {
            let subcategory = try XCTUnwrap(discrete.subcategories.first { $0.subcategory == name })
            XCTAssertFalse(subcategory.subgroups.isEmpty, name)
            for group in subcategory.subgroups {
                let values = group.functions.map { leaf in leaf.parts.compactMap(\.drawerValue).min() ?? 0 }
                XCTAssertEqual(values, values.sorted(), "\(name) / \(group.subcategory) leaves \(group.functions.map(\.function))")
                for leaf in group.functions {
                    let parts = leaf.parts
                    let want = parts.sorted {
                        ($0.drawerValue ?? 0) != ($1.drawerValue ?? 0)
                            ? ($0.drawerValue ?? 0) < ($1.drawerValue ?? 0)
                            : $0.name.localizedStandardCompare($1.name) == .orderedAscending
                    }.map(\.name)
                    XCTAssertEqual(parts.map(\.name), want, "\(name) / \(leaf.function)")
                }
            }
        }
    }

    /// Relay leaves keep the name order; a leaf's parts go by coil voltage, then name.
    func testRelayAndBuzzerOrder() throws {
        let groups = TestSupport.catalog.groups(matching: everything)
        let relay = try XCTUnwrap(groups.first { $0.category == "Switches & Relays" }?
            .subcategories.first { $0.subcategory == "Relay" })
        for group in relay.subgroups {
            let leaves = group.functions.map(\.function)
            XCTAssertEqual(leaves, leaves.sorted { $0.localizedStandardCompare($1) == .orderedAscending }, group.subcategory)
            for leaf in group.functions {
                let want = leaf.parts.sorted {
                    ($0.relay?.coilMV ?? 0) != ($1.relay?.coilMV ?? 0)
                        ? ($0.relay?.coilMV ?? 0) < ($1.relay?.coilMV ?? 0)
                        : $0.name.localizedStandardCompare($1.name) == .orderedAscending
                }.map(\.name)
                XCTAssertEqual(leaf.parts.map(\.name), want, "\(group.subcategory) / \(leaf.function)")
            }
        }
        let buzzers = try XCTUnwrap(groups.first { $0.category == "Displays & Indicators" }?
            .subcategories.first { $0.subcategory == "Buzzers" })
        XCTAssertFalse(buzzers.functions.isEmpty)
        for leaf in buzzers.functions {
            let hz = leaf.parts.compactMap { $0.buzzer?.hz }
            XCTAssertEqual(hz, hz.sorted(), leaf.function)
        }
    }

    // MARK: - Phase 3: blowing a fuse

    /// 5V → GND-input inverter's output → inductor → fuse → resistor → LED → GND.
    private struct Circuit {
        let doc: SchematicDocument
        let fuse: PlacedChip
        let led: PlacedChip
        let inductor: PlacedChip
    }

    private func buildCircuit() throws -> Circuit {
        let doc = TestSupport.newDocument()
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 500))
        let gnd2 = try TestSupport.place(doc, "GND", at: CGPoint(x: 900, y: 500))
        let gate = try TestSupport.place(doc, "SN74HC04N", at: CGPoint(x: 240, y: 200))
        let inductor = try TestSupport.place(doc, "LQG15HN18NJ02D", at: CGPoint(x: 460, y: 160))
        let fuse = try TestSupport.place(doc, "SS-5F-10A-AP", at: CGPoint(x: 600, y: 160))
        let resistor = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 740, y: 160))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 860, y: 300))
        func connect(_ a: PlacedChip, _ x: String, _ b: PlacedChip, _ y: String) throws {
            let message = doc.connect(.pin(try TestSupport.pin(a, x)), .pin(try TestSupport.pin(b, y)))
            XCTAssertNil(message, "\(x) to \(y)")
        }
        try connect(vcc, "VCC", gate, "VCC")
        try connect(gnd, "GND", gate, "GND")
        try connect(gnd, "GND", gate, "1A")
        try connect(gate, "1Y", inductor, "1")
        try connect(inductor, "2", fuse, "1")
        try connect(fuse, "2", resistor, "1")
        try connect(resistor, "2", led, "A")
        try connect(led, "K", gnd2, "GND")
        return Circuit(doc: doc, fuse: fuse, led: led, inductor: inductor)
    }

    private func lit(_ simulation: SimulationController, _ led: PlacedChip, for seconds: TimeInterval = 0.3) -> Bool {
        RunLoop.main.run(until: Date().addingTimeInterval(seconds))
        return (simulation.lights[led.id]?.first ?? 0) > 0.5
    }

    /// A fuse has one position (its contact); an inductor and a gate none.
    func testPositionCounts() throws {
        let circuit = try buildCircuit()
        let netlist = NetlistBuilder.build(chips: circuit.doc.chips, wires: circuit.doc.wires,
                                           buses: circuit.doc.buses, labels: circuit.doc.netLabels)
        for (index, chip) in circuit.doc.chips.enumerated() {
            XCTAssertEqual(netlist.positionCounts[index], chip.part.kind == PART_FUSE ? 1 : 0, chip.partNumber)
        }
        XCTAssertEqual(netlist.fuseParts, circuit.doc.chips.indices.filter { circuit.doc.chips[$0].id == circuit.fuse.id })
    }

    /// The LED behind an inductor and a fuse is lit; a click on the fuse
    /// blows it and the LED goes dark; a second click restores it; stopping
    /// clears the state and the next play starts with the fuse intact.
    func testBlowingAFuse() throws {
        let circuit = try buildCircuit()
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)

        // Not running: nothing to blow.
        XCTAssertNil(simulation.toggleFuse(circuit.fuse.id))
        XCTAssertFalse(simulation.setFuse(circuit.fuse.id, blown: true))
        XCTAssertTrue(simulation.blownFuses.isEmpty)

        simulation.start(document: circuit.doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        XCTAssertTrue(lit(simulation, circuit.led), "lit through the inductor and the fuse")
        XCTAssertTrue(simulation.blownFuses.isEmpty)

        // Neither an inductor nor an LED can be blown.
        XCTAssertNil(simulation.toggleFuse(circuit.inductor.id))
        XCTAssertNil(simulation.toggleFuse(circuit.led.id))
        XCTAssertTrue(simulation.blownFuses.isEmpty)

        XCTAssertEqual(simulation.toggleFuse(circuit.fuse.id), true)
        XCTAssertEqual(simulation.blownFuses, [circuit.fuse.id])
        XCTAssertFalse(lit(simulation, circuit.led), "dark with the fuse blown")

        XCTAssertEqual(simulation.toggleFuse(circuit.fuse.id), false)
        XCTAssertTrue(simulation.blownFuses.isEmpty)
        XCTAssertTrue(lit(simulation, circuit.led), "lit again once restored")

        // Blown at the end of the run: stop clears it; the next play is intact.
        XCTAssertTrue(simulation.setFuse(circuit.fuse.id, blown: true))
        simulation.stop()
        XCTAssertTrue(simulation.blownFuses.isEmpty)
        simulation.start(document: circuit.doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        XCTAssertTrue(simulation.blownFuses.isEmpty)
        XCTAssertTrue(lit(simulation, circuit.led), "every play starts with every fuse intact")
        simulation.stop()
    }

    /// The fuse's whole body takes the click, so a click there never reaches a switch lever.
    func testAFuseIsHitAnywhereOnItsBody() throws {
        let circuit = try buildCircuit()
        let body = circuit.fuse.bodyRect
        for point in [CGPoint(x: body.midX, y: body.midY), CGPoint(x: body.minX + 2, y: body.minY + 2),
                      CGPoint(x: body.maxX - 2, y: body.maxY - 2)] {
            XCTAssertEqual(circuit.doc.chip(at: point)?.id, circuit.fuse.id)
        }
    }
}
