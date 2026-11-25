// plan-55-J: power switches end to end on the app's real catalog, document,
// voltage check and simulation. Checks the drawer (Power → Power Switches →
// Load Switches / Current-Limited / Ideal Diodes and their leaves), the kind
// name, facts and symbol of a load switch, a current-limited, a dual and an
// ideal diode, that a placed TPS22918DBVR is U1 on SOT23-6, and runs LED
// circuits in the real SimulationController (each its own 0.6 s document run):
// 5 V → TPS22918DBVR (ON on 5 V, active high) → resistor → LED (lit), the same
// with ON on GND (dark), and 5 V → LM66200DRLR's VIN1 (ON on GND, active low)
// → resistor → LED (lit). It also refuses 12 V on the TPS22918's VIN with
// "takes at most 5.5 V in", and a 5 V switch output on the 3V3 rail with the
// lower voltage first. Prints one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p55-harness && APP=/tmp/p55-dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/powerswitch-app.swift /tmp/p55-harness && \
//     /tmp/p55-harness/H.app/Contents/MacOS/harness [image dir]
//
// Image (default dir /tmp): p55-sidebar.png (the dual current-limited part selected).
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runPowerSwitchChecks() }

@MainActor func runPowerSwitchChecks() {
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

    // (1) The drawer: Power → Power Switches → Load Switches / Current-Limited / Ideal Diodes.
    // The default drawer hides the automotive-flagged parts: count with every flag shown, and the default.
    let everything = DrawerFilter(visibility: PartVisibility(showObsolete: true, showMilitary: true, showAutomotive: true))
    let groups = Harness.catalog.groups(matching: everything)
    let switches = groups.first { $0.category == "Power" }?.subcategories.first { $0.subcategory == "Power Switches" }
    let leaves = (switches?.subgroups ?? []).map(\.subcategory).sorted()
    check("1 drawer: Power → Power Switches → \(leaves)") {
        leaves == ["Current-Limited", "Ideal Diodes", "Load Switches"] ? nil : "leaves \(leaves)"
    }
    func functions(_ leaf: String) -> [String] {
        (switches?.subgroups.first { $0.subcategory == leaf }?.functions ?? []).map(\.function)
    }
    print("     Load Switches: \(functions("Load Switches")); Current-Limited: \(functions("Current-Limited")); Ideal Diodes: \(functions("Ideal Diodes"))")
    check("1 drawer: the leaves' functions are channel / input counts") {
        let allowed: Set<String> = ["Single channel", "Dual channel", "Triple channel", "Quad channel", "Single input", "Dual input"]
        let all = Set(["Load Switches", "Current-Limited", "Ideal Diodes"].flatMap(functions))
        return all.isSubset(of: allowed) && !all.isEmpty ? nil : "functions \(all)"
    }
    let count = (switches?.subgroups ?? []).flatMap(\.functions).flatMap(\.parts).count
    print("     power switch parts in the drawer: \(count)")
    check("1 drawer: 817 power switch parts with every flag shown") { count == 817 ? nil : "\(count)" }
    let shown = Harness.catalog.groups(matching: DrawerFilter()).first { $0.category == "Power" }?
        .subcategories.first { $0.subcategory == "Power Switches" }?.subgroups.flatMap(\.functions).flatMap(\.parts).count ?? 0
    print("     power switch parts in the default drawer: \(shown)")
    check("1 drawer: the default drawer hides only the automotive-flagged parts") {
        let flagged = (switches?.subgroups ?? []).flatMap(\.functions).flatMap(\.parts).filter { $0.flags.automotive }.count
        return shown == count - flagged && flagged > 0 ? nil : "\(shown) shown, \(flagged) automotive of \(count)"
    }

    // (2) Kind name, facts and symbol.
    let doc = Harness.newDocument()
    func place(_ part: CatalogPart, at point: CGPoint) -> PlacedChip {
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return doc.chips.last!
    }
    let dualLimitedName = Harness.catalog.groups(matching: everything).flatMap(\.allParts)
        .filter { $0.powerswitch?.type == "limited" && $0.powerswitch?.switches == 2 }.map(\.name).sorted().first ?? "none"
    let shapes: [(String, String, String, [String])] = [
        ("load switch", "TPS22918DBVR", "Load switch", ["Type: Load switch", "Input: 1–5.5 V", "Current: 2 A"]),
        ("current-limited", "TPS2051BDBVR", "Current-limited switch",
         ["Type: Current-limited switch", "Input: 2.7–5.5 V", "Current limit: 1 A"]),
        ("dual current-limited", dualLimitedName, "Dual current-limited switch", ["Channels: 2"]),
        ("dual ideal diode", "LM66200DRLR", "Dual ideal diode", ["Type: Ideal diode", "Inputs: 2", "Input: 1.6–5.5 V"]),
    ]
    var dualID: UUID?
    for (index, (label, name, kind, wantFacts)) in shapes.enumerated() {
        guard let part = named(name) else {
            print("FAIL 2 \(label): \(name) is not in the catalog")
            failures += 1
            continue
        }
        let chip = place(part, at: CGPoint(x: 120 + 200 * index, y: 900))
        if label == "dual current-limited" { dualID = chip.id }
        let rows = PartFacts.rows(for: chip).map { "\($0.label): \($0.value)" }
        print("     facts \(name): \(rows.joined(separator: "; "))")
        check("2 \(label) \(name): kind \(kind), symbol block, facts \(wantFacts)") {
            if PartFacts.kindName(part) != kind { return "kind \(PartFacts.kindName(part))" }
            if symbol(for: part) != .block { return "symbol \(symbol(for: part))" }
            let missing = wantFacts.filter { want in !rows.contains(want) }
            return missing.isEmpty ? nil : "missing \(missing)"
        }
    }

    // (3) TPS22918DBVR placed: U1 on SOT23-6.
    let u = Harness.newDocument()
    if let part = named("TPS22918DBVR") {
        u.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100))
        let chip = u.chips.last!
        let footprint = chip.footprintID ?? chip.part.footprint?.id
        print("     TPS22918DBVR: \(chip.reference) on \(chip.part.package ?? "-") (\(footprint ?? "-"))")
        check("3 TPS22918DBVR placed: U1 on SOT23-6") {
            chip.reference == "U1" && footprint == "SOT23-6" ? nil : "\(chip.reference) \(footprint ?? "-")"
        }
    } else {
        print("FAIL 3: TPS22918DBVR is not in the catalog")
        failures += 1
    }

    // (4) LED circuits in the real simulation controller.
    func run(_ build: (SchematicDocument, (String, CGFloat, CGFloat) -> PlacedChip,
                       (PlacedChip, String) -> LineAnchor, (LineAnchor, LineAnchor) -> Void) -> PlacedChip) -> Bool? {
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
        let led = build(d, put, pin, connect)
        let sim = Harness.simulation
        sim.start(document: d)
        RunLoop.main.run(until: Date().addingTimeInterval(0.6))
        let on = (sim.lights[led.id]?.first ?? 0) > 0.5
        let stopped = sim.errorMessage
        sim.stop()
        if let stopped { print("     run stopped: \(stopped)"); return nil }
        return on
    }
    func state(_ lit: Bool?) -> String { lit.map { $0 ? "lit" : "dark" } ?? "run stopped" }

    /// 5 V → TPS22918DBVR (ON on `enable`) → resistor → LED → GND.
    func loadCircuit(enableOn: Bool) -> Bool? {
        run { _, put, pin, connect in
            let rail = put("5V", 60, 60)
            let gnd = put("GND", 60, 500)
            let sw = put("TPS22918DBVR", 240, 200)
            let r = put("GENERIC-RESISTOR", 440, 160)
            let led = put("GENERIC-LED", 580, 260)
            connect(pin(rail, "VCC"), pin(sw, "VIN"))
            connect(pin(sw, "GND"), pin(gnd, "GND"))
            connect(enableOn ? pin(rail, "VCC") : pin(gnd, "GND"), pin(sw, "ON"))
            connect(pin(sw, "VOUT"), pin(r, "1"))
            connect(pin(r, "2"), pin(led, "A"))
            connect(pin(led, "K"), pin(gnd, "GND"))
            return led
        }
    }
    for (on, want) in [(true, true), (false, false)] {
        let got = loadCircuit(enableOn: on)
        print("     5 V → TPS22918DBVR (ON \(on ? "on 5 V" : "on GND")) → resistor → LED: \(state(got))")
        check("4 TPS22918DBVR, ON \(on ? "high" : "low"): LED \(want ? "lit" : "dark")") {
            got == want ? nil : "LED \(state(got))"
        }
    }
    /// 5 V → LM66200DRLR's VIN1 (ON on GND: active low) → resistor → LED → GND.
    let diode = run { _, put, pin, connect in
        let rail = put("5V", 60, 60)
        let gnd = put("GND", 60, 500)
        let sw = put("LM66200DRLR", 240, 200)
        let r = put("GENERIC-RESISTOR", 440, 160)
        let led = put("GENERIC-LED", 580, 260)
        connect(pin(rail, "VCC"), pin(sw, "VIN1"))
        connect(pin(sw, "GND"), pin(gnd, "GND"))
        connect(pin(gnd, "GND"), pin(sw, "ON"))
        connect(pin(sw, "VOUT"), pin(r, "1"))
        connect(pin(r, "2"), pin(led, "A"))
        connect(pin(led, "K"), pin(gnd, "GND"))
        return led
    }
    print("     5 V → LM66200DRLR VIN1 (ON on GND) → resistor → LED: \(state(diode))")
    check("4 LM66200DRLR passing 5 V (ON low): LED lit") { diode == true ? nil : "LED \(state(diode))" }

    // (5) The messages: 12 V on the input is refused with the limit; a 5 V output on 3V3 is a clash.
    func wire(_ d: SchematicDocument, _ a: PlacedChip, _ an: String, _ b: PlacedChip, _ bn: String) -> String? {
        func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
            let slot = chip.layout.slots.first { $0.pin.name == name }!
            return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
        }
        return d.connect(pin(a, an), pin(b, bn))
    }
    func put(_ d: SchematicDocument, _ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
        let part = Harness.catalog.part(named: name)!
        d.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: x, y: y))
        return d.chips.last!
    }
    let over = Harness.newDocument()
    let overSwitch = put(over, "TPS22918DBVR", 300, 300)
    let overRail = put(over, "12V", 100, 300)
    let overText = wire(over, overRail, "VCC", overSwitch, "VIN")
    print("     12 V on TPS22918DBVR's VIN: \(overText ?? "accepted")")
    check("5 12 V on VIN: refused, naming the part and 5.5 V") {
        overText?.hasPrefix("Can't connect: U1 pin 1 (VIN) gets 12 V;") == true
            && overText?.hasSuffix("TPS22918DBVR takes at most 5.5 V in.") == true ? nil : overText ?? "accepted"
    }
    let clash = Harness.newDocument()
    let clashSwitch = put(clash, "TPS22918DBVR", 400, 300)
    let five = put(clash, "5V", 100, 300)
    let three = put(clash, "3V3", 700, 300)
    _ = wire(clash, five, "VCC", clashSwitch, "VIN")
    let clashText = wire(clash, three, "VCC", clashSwitch, "VOUT")
    print("     5 V switch output on 3V3: \(clashText ?? "accepted")")
    check("5 a 5 V switch output on the 3V3 rail: the lower voltage first") {
        clashText == "Can't connect: 3.3 V (\(three.reference)) and 5 V (\(clashSwitch.reference)) supplies on one net."
            ? nil : clashText ?? "accepted"
    }

    // (6) The sidebar picture (the dual current-limited part selected).
    if let dualID { doc.select(.chip(dualID)) }
    let sidebarPath = "\(images)/p55-sidebar.png"
    Harness.renderSidebar(doc, to: sidebarPath, size: CGSize(width: 320, height: 640))
    check("6 image written") {
        FileManager.default.fileExists(atPath: sidebarPath) ? nil : "missing \(sidebarPath)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
