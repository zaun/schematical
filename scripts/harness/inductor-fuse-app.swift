// plan-47-M: inductors, fuses, PTC resettable fuses, relays and buzzers in the
// app, end to end on the app's real catalog, document, simulation and views.
// Checks the drawer paths and the value order of the leaves (Discrete →
// Inductors / Fuses / Resettable fuses; Relay; Buzzers); the kind name, facts
// rows and drawer labels of one real part per kind and type; that every
// two-terminal inductor and fuse part draws as such (`symbol(for:)`); the
// references L1, F1, K1, LS1; and the LED circuit 5V/GND → SN74HC04N (input on
// GND) → 1Y → inductor → fuse → GENERIC-RESISTOR → GENERIC-LED → GND in the
// real SimulationController: lit, dark after a click on the fuse (blown),
// lit again after a second click, and intact again at the next play. Renders a
// placed inductor, fuse, PTC and a blown fuse. Prints one ok/FAIL line per
// check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p47-harness && APP=/tmp/p47-dd-m/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/inductor-fuse-app.swift /tmp/p47-harness && \
//     /tmp/p47-harness/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p47-symbols.png (an inductor, a fuse, a PTC),
// p47-blown.png (the circuit with the fuse blown), p47-sidebar.png (the PTC
// selected).
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runInductorFuseChecks() }

@MainActor func runInductorFuseChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp"
    var failures = 0
    func check(_ name: String, _ body: () throws -> String?) {
        do {
            if let problem = try body() {
                print("FAIL \(name): \(problem)")
                failures += 1
            } else {
                print("ok   \(name)")
            }
        } catch {
            print("FAIL \(name): \(error)")
            failures += 1
        }
    }
    func named(_ name: String) -> CatalogPart? { Harness.catalog.part(named: name) }

    var everything = DrawerFilter()
    everything.visibility = PartVisibility(showObsolete: true, showMilitary: true, showAutomotive: true)
    let groups = Harness.catalog.groups(matching: everything)
    func category(_ name: String) -> PartCatalogStore.CategoryGroup? { groups.first { $0.category == name } }
    func subcategory(_ top: String, _ name: String) -> PartCatalogStore.SubcategoryGroup? {
        category(top)?.subcategories.first { $0.subcategory == name }
    }

    // (1) The drawer: paths and the value order of the leaves.
    let discrete = category("Discrete")
    let discreteNames = (discrete?.subcategories ?? []).map(\.subcategory)
    check("1 drawer: Discrete has Inductors, Fuses, Resettable fuses (\(discreteNames))") {
        let missing = ["Inductors", "Fuses", "Resettable fuses"].filter { !discreteNames.contains($0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }
    for name in ["Inductors", "Fuses", "Resettable fuses"] {
        guard let sub = subcategory("Discrete", name) else { continue }
        let leaves = sub.subgroups.flatMap(\.functions)
        let parts = leaves.flatMap(\.parts).count
        print("     Discrete → \(name): \(sub.subgroups.map(\.subcategory)), \(leaves.count) leaves, \(parts) parts")
        check("1 drawer: \(name) leaves and parts are in value order") {
            for group in sub.subgroups {
                let mins = group.functions.map { $0.parts.compactMap(\.drawerValue).min() ?? 0 }
                if mins != mins.sorted() { return "\(group.subcategory) leaves \(group.functions.map(\.function))" }
                for leaf in group.functions {
                    let values = leaf.parts.map { $0.drawerValue ?? 0 }
                    if values != values.sorted() { return "\(leaf.function) parts not in value order" }
                }
            }
            return nil
        }
    }
    let relay = subcategory("Switches & Relays", "Relay")
    print("     Switches & Relays → Relay: \((relay?.subgroups ?? []).map(\.subcategory))")
    check("1 drawer: Relay has its leaves, parts by coil voltage") {
        guard let relay, !relay.subgroups.isEmpty else { return "no Switches & Relays → Relay" }
        for group in relay.subgroups {
            for leaf in group.functions {
                let volts = leaf.parts.map { $0.relay?.coilMV ?? 0 }
                if volts != volts.sorted() { return "\(group.subcategory) / \(leaf.function) coil voltages \(volts)" }
            }
        }
        return nil
    }
    let buzzers = subcategory("Displays & Indicators", "Buzzers")
    check("1 drawer: Displays & Indicators → Buzzers, parts by frequency") {
        guard let buzzers, !buzzers.functions.isEmpty else { return "no Displays & Indicators → Buzzers" }
        for leaf in buzzers.functions {
            let hz = leaf.parts.compactMap { $0.buzzer?.hz }
            if hz != hz.sorted() { return "\(leaf.function) frequencies \(hz)" }
        }
        return nil
    }

    // (2) Kind name, facts, drawer labels and reference prefix of one real part per kind and type.
    let doc = Harness.newDocument()
    func place(_ part: CatalogPart, at point: CGPoint) -> PlacedChip {
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return doc.chips.last!
    }
    let shapes: [(String, String, String, String, [String], String, String)] = [
        ("inductor", "LQG15HN18NJ02D", "Inductor", "L1",
         ["Inductance: 18 nH", "Tolerance: ±5%", "Rated current: 350 mA", "Shielding: Unshielded"], "18 nH", "350 mA, Unshielded"),
        ("fuse", "SS-5F-10A-AP", "Fuse", "F1",
         ["Type: Fuse", "Rated current: 10 A", "Speed: Fast", "Voltage (AC): 125 V", "Breaking capacity: 50 A"], "10 A", "Fast, 125 V"),
        ("PTC", "0ZCM0010FF2G", "Resettable fuse", "F2",
         ["Type: Resettable fuse (PTC)", "Hold current: 100 mA", "Trip current: 250 mA", "Maximum voltage: 15 V"], "100 mA", "250 mA trip, 15 V"),
        ("relay", "RZ03-1A3-D005-R", "Relay", "K1",
         ["Type: Electromechanical", "Coil voltage: 5 V", "Contact current: 16 A", "Operate time: 8 ms"], "5 V", "16 A"),
        ("reed relay", "JWD-172-1", "Reed relay", "K2",
         ["Type: Reed", "Contact current: 500 mA", "Switching voltage: 28 V"], "5 V", "500 mA"),
        ("solid-state relay", "AQV210EHAX", "Solid-state relay", "K3",
         ["Type: Solid-state", "Contact current: 130 mA", "On-resistance: 35 Ω"], "130 mA", "130 mA"),
        ("buzzer", "SMT-0825-S-4-R", "Buzzer", "LS1",
         ["Type: Transducer (external drive)", "Frequency: 2.5 kHz", "Rated voltage: 3.6 V", "Sound level: 85 dB"], "2.5 kHz", ""),
    ]
    for (index, (label, name, kind, reference, wantFacts, trailing, details)) in shapes.enumerated() {
        guard let part = named(name) else {
            print("FAIL 2 \(label): \(name) is not in the catalog")
            failures += 1
            continue
        }
        let chip = place(part, at: CGPoint(x: 120 + 160 * index, y: 900))
        let rows = PartFacts.rows(for: chip).map { "\($0.label): \($0.value)" }
        print("     facts \(name): \(rows.joined(separator: "; "))")
        print("     drawer row \(name): \(part.trailingLabel ?? "-") | \(part.detailLabels.joined(separator: ", ")) | \(part.drawer.joined(separator: " → "))")
        check("2 \(label) \(name): kind \(kind), reference \(reference), facts, drawer labels") {
            if PartFacts.kindName(part) != kind { return "kind \(PartFacts.kindName(part))" }
            if chip.reference != reference { return "reference \(chip.reference)" }
            if part.trailingLabel != trailing { return "trailing label \(part.trailingLabel ?? "-")" }
            if !details.isEmpty, part.detailLabels.joined(separator: ", ") != details {
                return "detail labels \(part.detailLabels)"
            }
            let missing = wantFacts.filter { want in !rows.contains(want) }
            return missing.isEmpty ? nil : "missing \(missing)"
        }
    }
    check("2 a buzzer placed after the other kinds is LS1") {
        let ls = doc.chips.filter { $0.reference.hasPrefix("LS") }.map(\.reference)
        return ls == ["LS1"] ? nil : "\(ls)"
    }

    // (3) symbol(for:) on every inductor and fuse part.
    check("3 every inductor and fuse draws as a coil or a rectangle, 1 left and 2 right") {
        var counts: [String: Int] = [:]
        for part in Harness.catalog.allParts where part.kind == PART_INDUCTOR || part.kind == PART_FUSE {
            let want: PartSymbol = part.kind == PART_INDUCTOR ? .inductor : .fuse
            if symbol(for: part) != want { return "\(part.name) is \(symbol(for: part))" }
            let layout = ChipLayout(part: part, symbol: want)
            if layout.slots.map(\.pin.name).sorted() != ["1", "2"] { return "\(part.name) draws pins \(layout.slots.map(\.pin.name))" }
            if layout.slots.first(where: { $0.pin.name == "1" })?.side != .left { return "\(part.name) pin 1 not left" }
            if layout.slots.first(where: { $0.pin.name == "2" })?.side != .right { return "\(part.name) pin 2 not right" }
            counts["\(want)", default: 0] += 1
        }
        print("     symbols: \(counts)")
        return counts.count == 2 ? nil : "\(counts)"
    }

    // (4) The LED circuit in the real simulation controller.
    let d = Harness.newDocument()
    func put(_ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
        let part = Harness.catalog.part(named: name)!
        d.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: x, y: y))
        return d.chips.last!
    }
    func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
        let slot = chip.layout.slots.first { $0.pin.name == name }!
        return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
    }
    func connect(_ x: LineAnchor, _ y: LineAnchor) {
        if let message = d.connect(x, y) { print("FAIL wiring: \(message)"); failures += 1 }
    }
    let vcc = put("5V", 60, 60)
    let gnd = put("GND", 60, 500)
    let gnd2 = put("GND", 940, 500)
    let gate = put("SN74HC04N", 240, 200)
    let inductor = put("LQG15HN18NJ02D", 460, 160)
    let fuse = put("SS-5F-10A-AP", 600, 160)
    let resistor = put("GENERIC-RESISTOR", 740, 160)
    let led = put("GENERIC-LED", 880, 300)
    connect(pin(vcc, "VCC"), pin(gate, "VCC"))
    connect(pin(gnd, "GND"), pin(gate, "GND"))
    connect(pin(gnd, "GND"), pin(gate, "1A"))
    connect(pin(gate, "1Y"), pin(inductor, "1"))
    connect(pin(inductor, "2"), pin(fuse, "1"))
    connect(pin(fuse, "2"), pin(resistor, "1"))
    connect(pin(resistor, "2"), pin(led, "A"))
    connect(pin(led, "K"), pin(gnd2, "GND"))

    let sim = Harness.simulation
    func lit() -> Bool {
        RunLoop.main.run(until: Date().addingTimeInterval(0.3))
        return (sim.lights[led.id]?.first ?? 0) > 0.5
    }
    func word(_ on: Bool) -> String { on ? "lit" : "dark" }
    sim.start(document: d)
    var states: [Bool] = []
    var blownAfterClicks: [Bool] = []
    if sim.isRunning {
        states.append(lit())
        sim.toggleFuse(fuse.id)
        blownAfterClicks.append(sim.blownFuses.contains(fuse.id))
        states.append(lit())
        // The picture of the blown fuse, taken while it is blown.
        Harness.renderCanvas(d, to: "\(images)/p47-blown.png", size: CGSize(width: 1000, height: 360),
                             origin: CGPoint(x: 20, y: 40))
        sim.toggleFuse(fuse.id)
        blownAfterClicks.append(sim.blownFuses.contains(fuse.id))
        states.append(lit())
        sim.stop()
    }
    let stopError = sim.errorMessage
    print("     LED: \(states.map(word)); blown after the clicks: \(blownAfterClicks)")
    check("4 LED behind the inductor and the fuse: lit, blown: dark, restored: lit") {
        if sim.blownFuses.count != 0 { return "blown fuses left after stop: \(sim.blownFuses)" }
        if states.count != 3 { return "the run did not start: \(stopError ?? "-")" }
        return states == [true, false, true] && blownAfterClicks == [true, false] ? nil
            : "LED \(states.map(word)), blown \(blownAfterClicks)"
    }
    sim.start(document: d)
    let again = sim.isRunning ? lit() : false
    let blownAtStart = sim.blownFuses.count
    sim.stop()
    check("4 the next play starts with the fuse intact: lit, nothing blown") {
        again && blownAtStart == 0 ? nil : "LED \(word(again)), blown \(blownAtStart)"
    }

    // (5) Pictures: an inductor, a fuse and a PTC; the PTC's sidebar.
    let picture = Harness.newDocument()
    var ptcID: UUID?
    for (index, name) in ["LQG15HN18NJ02D", "SS-5F-10A-AP", "0ZCM0010FF2G"].enumerated() {
        guard let part = named(name) else { continue }
        picture.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 110 + 200 * index, y: 120))
        if name == "0ZCM0010FF2G" { ptcID = picture.chips.last!.id }
    }
    let symbolsPath = "\(images)/p47-symbols.png"
    Harness.renderCanvas(picture, to: symbolsPath, size: CGSize(width: 660, height: 260))
    if let ptcID { picture.select(.chip(ptcID)) }
    let sidebarPath = "\(images)/p47-sidebar.png"
    Harness.renderSidebar(picture, to: sidebarPath, size: CGSize(width: 320, height: 640))
    check("5 images written") {
        let missing = ["\(images)/p47-blown.png", symbolsPath, sidebarPath]
            .filter { !FileManager.default.fileExists(atPath: $0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
