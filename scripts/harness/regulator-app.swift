// plan-42-I: voltage regulators end to end on the app's real catalog,
// document, voltage check and simulation. Checks the drawer (Power →
// Regulators → Linear / Switching and their leaves), the kind name, facts and
// symbol of a fixed, an adjustable, a switching and a dual part, that a
// placed GENERIC-REG-3V3 is U1 on TO-220-3, and runs three LED circuits in
// the real SimulationController (each its own 0.6 s document run): 12 V →
// GENERIC-REG-3V3 → the VCC of an SN74LVC1G04DBVR whose input is on GND (the
// LED on its output lit), the same with IN unwired (dark), and 12 V →
// GENERIC-REG-ADJ with 240 Ω / 720 Ω GENERIC-RESISTORs (5 V on OUT) → the
// same gate (lit). It also refuses an SN74AUC1G04DBVR (0.8–2.7 V) on that
// divider's OUT with "5 V" and, after the 720 Ω is changed to 390 Ω, with
// "3.28 V". Prints one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p42-harness && APP=/tmp/p42-dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/regulator-app.swift /tmp/p42-harness && \
//     /tmp/p42-harness/H.app/Contents/MacOS/harness [image dir]
//
// Image (default dir /tmp): p42-sidebar.png (the dual part selected).
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runRegulatorChecks() }

@MainActor func runRegulatorChecks() {
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

    // (1) The drawer: Power → Regulators → Linear / Switching, leaves by function.
    let groups = Harness.catalog.groups(matching: DrawerFilter())
    let regulators = groups.first { $0.category == "Power" }?.subcategories.first { $0.subcategory == "Regulators" }
    let linear = (regulators?.subgroups.first { $0.subcategory == "Linear" }?.functions ?? []).map(\.function)
    let switching = (regulators?.subgroups.first { $0.subcategory == "Switching" }?.functions ?? []).map(\.function)
    check("1 drawer: Power → Regulators → Linear \(linear); Switching \(switching)") {
        if linear != ["Adjustable", "Fixed", "Generic", "Multi-output"] { return "Linear leaves \(linear)" }
        let want = ["Boost", "Buck", "Buck-boost", "Charge pump", "Multi-output"]
        return switching == want || switching == want + ["SEPIC"] ? nil : "Switching leaves \(switching)"
    }
    let count = (regulators?.subgroups ?? []).flatMap(\.functions).flatMap(\.parts).count
    print("     regulator parts in the drawer: \(count)")
    check("1 drawer: 603 regulator parts") { count == 603 ? nil : "\(count)" }

    // (2) Kind name, facts and symbol.
    let doc = Harness.newDocument()
    func place(_ part: CatalogPart, at point: CGPoint) -> PlacedChip {
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return doc.chips.last!
    }
    let shapes: [(String, String, String, [String])] = [
        ("fixed", "LM1117MPX-3.3/NOPB", "Voltage regulator",
         ["Type: Linear", "Output: 3.3 V", "Input: up to 15 V", "Dropout: 1.2 V", "Current: 0.8 A"]),
        ("adjustable", "GENERIC-REG-ADJ", "Voltage regulator", ["Type: Linear", "Output: Adjustable, Vref 1.25 V"]),
        ("switching", "LM2576HVS-3.3", "Voltage regulator",
         ["Type: Buck", "Output: 3.3 V", "Input: 6–60 V", "Current: 3 A", "Switching: 52 kHz"]),
        ("dual", "ADP222ACPZ-1825-R7", "Dual voltage regulator",
         ["Regulators: 2", "Output: 1.8 V / 2.5 V", "Input: 2.5–5.5 V"]),
    ]
    var dualID: UUID?
    for (index, (label, name, kind, wantFacts)) in shapes.enumerated() {
        guard let part = named(name) else {
            print("FAIL 2 \(label): \(name) is not in the catalog")
            failures += 1
            continue
        }
        let chip = place(part, at: CGPoint(x: 120 + 200 * index, y: 900))
        if label == "dual" { dualID = chip.id }
        let rows = PartFacts.rows(for: chip).map { "\($0.label): \($0.value)" }
        print("     facts \(name): \(rows.joined(separator: "; "))")
        check("2 \(label) \(name): kind \(kind), symbol block, facts \(wantFacts)") {
            if PartFacts.kindName(part) != kind { return "kind \(PartFacts.kindName(part))" }
            if symbol(for: part) != .block { return "symbol \(symbol(for: part))" }
            let missing = wantFacts.filter { want in !rows.contains(want) }
            return missing.isEmpty ? nil : "missing \(missing)"
        }
    }

    // (3) GENERIC-REG-3V3 placed: U1 on TO-220-3.
    let u = Harness.newDocument()
    if let part = named("GENERIC-REG-3V3") {
        u.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100))
        let chip = u.chips.last!
        print("     GENERIC-REG-3V3: \(chip.reference) on \(chip.part.package ?? "-") (\(chip.footprintID ?? "-"))")
        check("3 GENERIC-REG-3V3 placed: U1 on TO-220-3") {
            chip.reference == "U1" && chip.footprintID == "TO-220-3" ? nil : "\(chip.reference) \(chip.footprintID ?? "-")"
        }
    } else {
        print("FAIL 3: GENERIC-REG-3V3 is not in the catalog")
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

    /// The gate (input on GND) on `reg`'s OUT, its output through a resistor
    /// to an LED: lit when the regulator powers the gate.
    func gateCircuit(regulator: String, wireInput: Bool) -> Bool? {
        run { _, put, pin, connect in
            let rail = put("12V", 60, 60)
            let gnd = put("GND", 60, 500)
            let reg = put(regulator, 240, 200)
            let gate = put("SN74LVC1G04DBVR", 440, 200)
            let r = put("GENERIC-RESISTOR", 580, 160)
            let led = put("GENERIC-LED", 700, 260)
            if wireInput { connect(pin(rail, "VCC"), pin(reg, "IN")) }
            connect(pin(reg, "GND"), pin(gnd, "GND"))
            connect(pin(reg, "OUT"), pin(gate, "VCC"))
            connect(pin(gate, "A"), pin(gnd, "GND"))
            connect(pin(gate, "GND"), pin(gnd, "GND"))
            connect(pin(gate, "Y"), pin(r, "1"))
            connect(pin(r, "2"), pin(led, "A"))
            connect(pin(led, "K"), pin(gnd, "GND"))
            return led
        }
    }
    let gated = gateCircuit(regulator: "GENERIC-REG-3V3", wireInput: true)
    print("     12 V → GENERIC-REG-3V3 → the VCC of an SN74LVC1G04DBVR (input low) → LED: \(state(gated))")
    check("4 GENERIC-REG-3V3 powering a gate, IN on 12 V: the LED on its output lit") {
        gated == true ? nil : "LED \(state(gated))"
    }
    /// OUT → resistor → LED → GND: the regulator's own output drives the LED.
    /// (A logic gate runs with its VCC unwired, so the dark case needs the
    /// regulator itself on the LED.)
    func direct(wireInput: Bool) -> Bool? {
        run { _, put, pin, connect in
            let rail = put("12V", 60, 60)
            let gnd = put("GND", 60, 500)
            let reg = put("GENERIC-REG-3V3", 240, 200)
            let r = put("GENERIC-RESISTOR", 440, 160)
            let led = put("GENERIC-LED", 580, 260)
            if wireInput { connect(pin(rail, "VCC"), pin(reg, "IN")) }
            connect(pin(reg, "GND"), pin(gnd, "GND"))
            connect(pin(reg, "OUT"), pin(r, "1"))
            connect(pin(r, "2"), pin(led, "A"))
            connect(pin(led, "K"), pin(gnd, "GND"))
            return led
        }
    }
    for (wired, want) in [(true, true), (false, false)] {
        let got = direct(wireInput: wired)
        print("     12 V → GENERIC-REG-3V3\(wired ? "" : " (IN unwired)") → resistor → LED: \(state(got))")
        check("4 GENERIC-REG-3V3 on an LED, \(wired ? "IN on 12 V" : "IN unwired"): LED \(want ? "lit" : "dark")") {
            got == want ? nil : "LED \(state(got))"
        }
    }

    // The adjustable divider: 240 Ω from OUT to ADJ, 720 Ω from ADJ to GND.
    func divider(_ d: SchematicDocument, r2Ohms: Double) -> (reg: PlacedChip, gnd: PlacedChip) {
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
        let rail = put("12V", 60, 60)
        let gnd = put("GND", 60, 500)
        let reg = put("GENERIC-REG-ADJ", 240, 200)
        let r1 = put("GENERIC-RESISTOR", 400, 120)
        let r2 = put("GENERIC-RESISTOR", 400, 360)
        d.setPartOhms(r1.id, 240)
        d.setPartOhms(r2.id, r2Ohms)
        connect(pin(rail, "VCC"), pin(reg, "IN"))
        connect(pin(reg, "OUT"), pin(r1, "1"))
        connect(pin(reg, "ADJ"), pin(r1, "2"))
        connect(pin(reg, "ADJ"), pin(r2, "1"))
        connect(pin(r2, "2"), pin(gnd, "GND"))
        return (reg, gnd)
    }
    func refusal(_ d: SchematicDocument, regID: UUID, part name: String) -> String? {
        let part = Harness.catalog.part(named: name)!
        d.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 700, y: 200))
        let load = d.chips.last!
        let reg = d.chips.first { $0.id == regID }!
        func pin(_ chip: PlacedChip, _ pinName: String) -> LineAnchor {
            let slot = chip.layout.slots.first { $0.pin.name == pinName }!
            return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
        }
        return d.connect(pin(load, "VCC"), pin(reg, "OUT"))
    }
    let five = Harness.newDocument()
    let fiveParts = divider(five, r2Ohms: 720)
    let fiveText = refusal(five, regID: fiveParts.reg.id, part: "SN74AUC1G04DBVR")
    print("     240 Ω / 720 Ω, an SN74AUC1G04DBVR on OUT: \(fiveText ?? "accepted")")
    check("4 240 Ω / 720 Ω: OUT is 5 V, a 0.8–2.7 V part is refused with 5 V") {
        fiveText?.contains("5 V on") == true ? nil : fiveText ?? "accepted"
    }
    let lower = Harness.newDocument()
    let lowerParts = divider(lower, r2Ohms: 390)
    let lowerText = refusal(lower, regID: lowerParts.reg.id, part: "SN74AUC1G04DBVR")
    print("     240 Ω / 390 Ω, an SN74AUC1G04DBVR on OUT: \(lowerText ?? "accepted")")
    check("4 240 Ω / 390 Ω: OUT is 3.28 V, the refusal names it") {
        lowerText?.contains("3.28 V on") == true ? nil : lowerText ?? "accepted"
    }
    // The 5 V divider output powers the gate in play mode (the same gate
    // wired onto `five`, whose OUT now carries the resistors' 5 V).
    let live = Harness.newDocument()
    let liveParts = divider(live, r2Ohms: 720)
    func liveLoad() -> PlacedChip? {
        func put(_ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
            let part = Harness.catalog.part(named: name)!
            live.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: x, y: y))
            return live.chips.last!
        }
        func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
            let slot = chip.layout.slots.first { $0.pin.name == name }!
            return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
        }
        func connect(_ x: LineAnchor, _ y: LineAnchor) {
            if let message = live.connect(x, y) { print("FAIL wiring: \(message)"); failures += 1 }
        }
        let reg = live.chips.first { $0.id == liveParts.reg.id }!
        let gate = put("SN74LVC1G04DBVR", 700, 200)
        let r = put("GENERIC-RESISTOR", 840, 160)
        let led = put("GENERIC-LED", 960, 260)
        connect(pin(reg, "OUT"), pin(gate, "VCC"))
        connect(pin(gate, "A"), pin(liveParts.gnd, "GND"))
        connect(pin(gate, "GND"), pin(liveParts.gnd, "GND"))
        connect(pin(gate, "Y"), pin(r, "1"))
        connect(pin(r, "2"), pin(led, "A"))
        connect(pin(led, "K"), pin(liveParts.gnd, "GND"))
        return led
    }
    var dividerLit: Bool?
    if let led = liveLoad() {
        let sim = Harness.simulation
        sim.start(document: live)
        RunLoop.main.run(until: Date().addingTimeInterval(0.6))
        dividerLit = (sim.lights[led.id]?.first ?? 0) > 0.5
        if let stopped = sim.errorMessage { print("     run stopped: \(stopped)"); dividerLit = nil }
        sim.stop()
    }
    print("     12 V → GENERIC-REG-ADJ (240 Ω / 720 Ω) → gate: LED \(state(dividerLit))")
    check("4 GENERIC-REG-ADJ with 240 Ω / 720 Ω: LED lit") { dividerLit == true ? nil : "LED \(state(dividerLit))" }

    // (5) The sidebar picture (the dual part selected).
    if let dualID { doc.select(.chip(dualID)) }
    let sidebarPath = "\(images)/p42-sidebar.png"
    Harness.renderSidebar(doc, to: sidebarPath, size: CGSize(width: 320, height: 640))
    check("5 image written") {
        FileManager.default.fileExists(atPath: sidebarPath) ? nil : "missing \(sidebarPath)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
