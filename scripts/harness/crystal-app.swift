// plan-44-G: crystals and ceramic resonators end to end on the app's real
// catalog, document, simulation and views. Checks the drawer (Crystal → kHz /
// MHz / Ceramic resonator with Generic beside them, the leaves in frequency
// order), the kind name, facts and symbol of GENERIC-CRYSTAL-8M, the
// resonator and a catalog 32.768 kHz crystal, that a placed GENERIC-CRYSTAL-8M
// is Y1 on HC-49/US, and runs two LED circuits in the real
// SimulationController (each its own 0.6 s document run): GENERIC-CRYSTAL-32768's
// terminal 1 on an SN74HC04N 1A (VCC on 14, GND on 7), 1Y → GENERIC-RESISTOR →
// GENERIC-LED → GND: the LED lights; the same circuit with 1A on PULLUP-5V
// instead of the crystal (a constant 1, so 1Y is 0): the LED stays dark.
// Renders a placed crystal, a resonator and a four-pad part. Prints one
// ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p44-harness && APP=/tmp/p44-dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/crystal-app.swift /tmp/p44-harness && \
//     /tmp/p44-harness/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p44-symbols.png (GENERIC-CRYSTAL-8M,
// GENERIC-RESONATOR-8M, a four-pad catalog crystal), p44-sidebar.png (the
// resonator selected).
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runCrystalChecks() }

@MainActor func runCrystalChecks() {
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

    // (1) The drawer: Crystal → kHz / MHz / Ceramic resonator, Generic beside them.
    let groups = Harness.catalog.groups(matching: DrawerFilter())
    let crystal = groups.first { $0.category == "Crystal" }
    func leaves(_ sub: String) -> [String] {
        (crystal?.subcategories.first { $0.subcategory == sub }?.functions ?? []).map(\.function)
    }
    let khz = leaves("kHz"), mhz = leaves("MHz"), resonator = leaves("Ceramic resonator")
    let generic = (crystal?.functions ?? []).map(\.function)
    check("1 drawer: Crystal → kHz \(khz); MHz \(mhz); Ceramic resonator \(resonator); Generic \(generic)") {
        if crystal == nil { return "no Crystal category" }
        if generic != ["Generic"] { return "Crystal leaves \(generic)" }
        if khz != ["32 kHz", "32.768 kHz"] { return "kHz leaves \(khz)" }
        let wantMHz = ["Under 4 MHz", "4 MHz to 8 MHz", "8 MHz to 16 MHz", "16 MHz to 32 MHz", "32 MHz and over"]
        if mhz != wantMHz { return "MHz leaves \(mhz)" }
        return resonator == ["Under 4 MHz", "4 MHz to 8 MHz", "8 MHz to 16 MHz", "16 MHz and over"]
            ? nil : "resonator leaves \(resonator)"
    }
    let listed = (crystal?.subcategories ?? []).flatMap(\.functions).flatMap(\.parts).count
        + (crystal?.functions ?? []).flatMap(\.parts).count
    print("     crystal parts in the drawer: \(listed)")
    check("1 each leaf's parts are in frequency order") {
        for sub in crystal?.subcategories ?? [] {
            for leaf in sub.functions {
                let hz = leaf.parts.compactMap(\.crystalHz)
                if hz != hz.sorted() { return "\(sub.subcategory) / \(leaf.function): \(hz.prefix(6))" }
            }
        }
        return nil
    }

    // (2) Kind name, facts and symbol.
    let doc = Harness.newDocument()
    func place(_ part: CatalogPart, at point: CGPoint) -> PlacedChip {
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return doc.chips.last!
    }
    let shapes: [(String, String, String, [String])] = [
        ("crystal", "GENERIC-CRYSTAL-8M", "Crystal", ["Type: Crystal", "Frequency: 8 MHz"]),
        ("resonator", "GENERIC-RESONATOR-8M", "Ceramic resonator",
         ["Type: Ceramic resonator", "Frequency: 8 MHz", "Built-in capacitors: Yes"]),
        ("32.768 kHz", "ABS07-32.768KHZ-T", "Crystal",
         ["Frequency: 32.768 kHz", "Load capacitance: 12.5 pF", "Tolerance: ±20 ppm", "ESR: 70 kΩ"]),
    ]
    for (index, (label, name, kind, wantFacts)) in shapes.enumerated() {
        guard let part = named(name) else {
            print("FAIL 2 \(label): \(name) is not in the catalog")
            failures += 1
            continue
        }
        let chip = place(part, at: CGPoint(x: 120 + 200 * index, y: 900))
        let rows = PartFacts.rows(for: chip).map { "\($0.label): \($0.value)" }
        print("     facts \(name): \(rows.joined(separator: "; "))")
        check("2 \(label) \(name): kind \(kind), symbol crystal, facts \(wantFacts)") {
            if PartFacts.kindName(part) != kind { return "kind \(PartFacts.kindName(part))" }
            if symbol(for: part) != .crystal { return "symbol \(symbol(for: part))" }
            let missing = wantFacts.filter { want in !rows.contains(want) }
            return missing.isEmpty ? nil : "missing \(missing)"
        }
    }

    // (3) GENERIC-CRYSTAL-8M placed: Y1 on HC-49/US.
    let y = Harness.newDocument()
    if let part = named("GENERIC-CRYSTAL-8M") {
        y.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100))
        let chip = y.chips.last!
        print("     GENERIC-CRYSTAL-8M: \(chip.reference) on \(chip.part.package ?? "-") (\(chip.footprintID ?? "-"))")
        check("3 GENERIC-CRYSTAL-8M placed: Y1 on HC-49/US") {
            chip.reference == "Y1" && chip.part.package == "HC-49/US" ? nil : "\(chip.reference) \(chip.part.package ?? "-")"
        }
    } else {
        print("FAIL 3: GENERIC-CRYSTAL-8M is not in the catalog")
        failures += 1
    }

    // (4) The LED circuits in the real simulation controller.
    func run(clock: String) -> Double? {
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
        let gnd = put("GND", 60, 600)
        let gnd2 = put("GND", 700, 600)
        let inverter = put("SN74HC04N", 300, 300)
        let source = put(clock, 100, 300)
        let r = put("GENERIC-RESISTOR", 520, 200)
        let led = put("GENERIC-LED", 640, 300)
        connect(pin(vcc, "VCC"), pin(inverter, "VCC"))
        connect(pin(gnd, "GND"), pin(inverter, "GND"))
        let drive = source.layout.slots.first { $0.pin.name == "X1" || $0.pin.name == "VCC" }
            ?? source.layout.slots.first!
        connect(pin(source, drive.pin.name), pin(inverter, "1A"))
        connect(pin(inverter, "1Y"), pin(r, "1"))
        connect(pin(r, "2"), pin(led, "A"))
        connect(pin(led, "K"), pin(gnd2, "GND"))
        let sim = Harness.simulation
        sim.start(document: d)
        RunLoop.main.run(until: Date().addingTimeInterval(0.6))
        let level = sim.lights[led.id]?.first
        let stopped = sim.errorMessage
        sim.stop()
        if let stopped { print("     run stopped: \(stopped)"); return nil }
        return Double(level ?? 0)
    }
    let toggling = run(clock: "GENERIC-CRYSTAL-32768")
    print("     GENERIC-CRYSTAL-32768 → SN74HC04N → R → LED: light \(toggling.map { String($0) } ?? "run stopped")")
    check("4 GENERIC-CRYSTAL-32768 on SN74HC04N 1A: the LED lights") {
        (toggling ?? 0) > 0.05 ? nil : "light \(toggling.map { String($0) } ?? "run stopped")"
    }
    let constant = run(clock: "PULLUP-5V")
    print("     PULLUP-5V → SN74HC04N → R → LED: light \(constant.map { String($0) } ?? "run stopped")")
    check("4 PULLUP-5V on SN74HC04N 1A instead: the LED stays dark") {
        constant == 0 ? nil : "light \(constant.map { String($0) } ?? "run stopped")"
    }

    // (5) Pictures: a crystal, a resonator and a four-pad part, and the sidebar.
    let picture = Harness.newDocument()
    var resonatorID: UUID?
    let four = Harness.catalog.groups(matching: DrawerFilter()).flatMap(\.allParts).first {
        $0.kind == PART_CRYSTAL && $0.pins.count == 4 && $0.pins.contains { $0.name == "NC" }
    }
    for (index, name) in ["GENERIC-CRYSTAL-8M", "GENERIC-RESONATOR-8M", four?.name ?? ""].enumerated() {
        guard let part = named(name) else { continue }
        picture.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 110 + 200 * index, y: 120))
        if name == "GENERIC-RESONATOR-8M" { resonatorID = picture.chips.last!.id }
    }
    let symbolsPath = "\(images)/p44-symbols.png"
    Harness.renderCanvas(picture, to: symbolsPath, size: CGSize(width: 660, height: 260))
    if let resonatorID { picture.select(.chip(resonatorID)) }
    let sidebarPath = "\(images)/p44-sidebar.png"
    Harness.renderSidebar(picture, to: sidebarPath, size: CGSize(width: 320, height: 640))
    check("5 images written") {
        let missing = [symbolsPath, sidebarPath].filter { !FileManager.default.fileExists(atPath: $0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
