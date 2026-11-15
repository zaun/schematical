// plan-40-H: transistors end to end on the app's real catalog, document,
// simulation and views. Checks the drawer leaves (Discrete → Transistors →
// Generic, BJT, MOSFET), the kind name, facts and symbol of an NPN, a
// logic-level MOSFET and a dual array, that a placed GENERIC-NPN is Q1 on
// TO-92, and runs three LED circuits in the real SimulationController (each
// its own 0.6 s document run): GENERIC-NPN low side (5V → GENERIC-RESISTOR →
// GENERIC-LED → collector, emitter GND) with the base on PULLUP-5V (lit) and
// on PULLDOWN (dark), and GENERIC-PMOS high side (source 5V, gate GND, drain
// → GENERIC-RESISTOR → GENERIC-LED → GND: lit). Renders a placed NPN, a
// MOSFET and an array. Prints one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p40-harness && APP=/tmp/p40-dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/transistor-app.swift /tmp/p40-harness && \
//     /tmp/p40-harness/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p40-symbols.png (2N3904BU, IRLML6244TRPBF,
// BC846SZ), p40-sidebar.png (the MOSFET selected).
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runTransistorChecks() }

@MainActor func runTransistorChecks() {
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

    // (1) The drawer: Discrete → Transistors → Generic; BJT and MOSFET leaves.
    let groups = Harness.catalog.groups(matching: DrawerFilter())
    let transistors = groups.first { $0.category == "Discrete" }?.subcategories.first { $0.subcategory == "Transistors" }
    let generic = (transistors?.functions ?? []).map(\.function)
    let bjt = (transistors?.subgroups.first { $0.subcategory == "BJT" }?.functions ?? []).map(\.function)
    let mosfet = (transistors?.subgroups.first { $0.subcategory == "MOSFET" }?.functions ?? []).map(\.function)
    check("1 drawer: Transistors → \(generic); BJT \(bjt); MOSFET \(mosfet)") {
        if generic != ["Generic"] { return "Transistors leaves \(generic)" }
        let wantBJT = ["Array", "NPN", "NPN Darlington", "NPN pre-biased", "PNP", "PNP Darlington", "PNP pre-biased"]
        let wantMOS = ["Array", "N-channel", "N-channel logic-level", "P-channel", "P-channel logic-level"]
        if bjt != wantBJT { return "BJT leaves \(bjt)" }
        return mosfet == wantMOS ? nil : "MOSFET leaves \(mosfet)"
    }
    let count = (transistors?.subgroups ?? []).flatMap(\.functions).flatMap(\.parts).count
        + (transistors?.functions ?? []).flatMap(\.parts).count
    print("     transistor parts in the drawer: \(count)")

    // (2) Kind name, facts and symbol.
    let doc = Harness.newDocument()
    func place(_ part: CatalogPart, at point: CGPoint) -> PlacedChip {
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return doc.chips.last!
    }
    let shapes: [(String, String, String, PartSymbol, [String])] = [
        ("NPN", "2N3904BU", "Transistor", .transistor, ["Type: NPN", "Vceo: 40 V", "Ic: 0.2 A", "Power: 0.625 W", "hFE: 100"]),
        ("MOSFET", "IRLML6244TRPBF", "Transistor", .transistor,
         ["Type: N-channel logic-level", "Vds: 20 V", "Id: 6.3 A", "Rds(on): 21 mΩ", "Vgs(th): ≤ 1.1 V"]),
        ("dual array", "BC846SZ", "Transistor array", .block, ["Type: NPN", "Elements: 2"]),
    ]
    for (index, (label, name, kind, wantSymbol, wantFacts)) in shapes.enumerated() {
        guard let part = named(name) else {
            print("FAIL 2 \(label): \(name) is not in the catalog")
            failures += 1
            continue
        }
        let chip = place(part, at: CGPoint(x: 120 + 200 * index, y: 900))
        let rows = PartFacts.rows(for: chip).map { "\($0.label): \($0.value)" }
        print("     facts \(name): \(rows.joined(separator: "; "))")
        check("2 \(label) \(name): kind \(kind), symbol \(wantSymbol), facts \(wantFacts)") {
            if PartFacts.kindName(part) != kind { return "kind \(PartFacts.kindName(part))" }
            if symbol(for: part) != wantSymbol { return "symbol \(symbol(for: part))" }
            let missing = wantFacts.filter { want in !rows.contains(want) }
            return missing.isEmpty ? nil : "missing \(missing)"
        }
    }

    // (3) GENERIC-NPN placed: Q1 on TO-92.
    let q = Harness.newDocument()
    if let npn = named("GENERIC-NPN") {
        q.placeChip(part: npn, symbol: symbol(for: npn), centeredAt: CGPoint(x: 100, y: 100))
        let chip = q.chips.last!
        print("     GENERIC-NPN: \(chip.reference) on \(chip.part.package ?? "-") (\(chip.footprintID ?? "-"))")
        check("3 GENERIC-NPN placed: Q1 on TO-92") {
            chip.reference == "Q1" && chip.part.package == "TO-92" ? nil : "\(chip.reference) \(chip.part.package ?? "-")"
        }
    } else {
        print("FAIL 3: GENERIC-NPN is not in the catalog")
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
    func lowSide(base: String) -> Bool? {
        run { _, put, pin, connect in
            let vcc = put("5V", 60, 60)
            let gnd = put("GND", 60, 500)
            let r = put("GENERIC-RESISTOR", 200, 100)
            let led = put("GENERIC-LED", 340, 160)
            let q = put("GENERIC-NPN", 480, 300)
            let pull = put(base, 300, 360)
            connect(pin(vcc, "VCC"), pin(r, "1"))
            connect(pin(r, "2"), pin(led, "A"))
            connect(pin(led, "K"), pin(q, "C"))
            connect(pin(q, "E"), pin(gnd, "GND"))
            connect(pin(pull, pull.layout.slots.first!.pin.name), pin(q, "B"))
            return led
        }
    }
    for (base, want) in [("PULLUP-5V", true), ("PULLDOWN", false)] {
        let got = lowSide(base: base)
        print("     GENERIC-NPN low side, base on \(base): LED \(got.map { $0 ? "lit" : "dark" } ?? "run stopped")")
        check("4 GENERIC-NPN low side, base on \(base): LED \(want ? "lit" : "dark")") {
            got == want ? nil : "LED \(got.map { $0 ? "lit" : "dark" } ?? "run stopped")"
        }
    }
    let high = run { _, put, pin, connect in
        let vcc = put("5V", 60, 60)
        let gnd = put("GND", 60, 500)
        let gnd2 = put("GND", 620, 500)
        let q = put("GENERIC-PMOS", 200, 200)
        let r = put("GENERIC-RESISTOR", 360, 200)
        let led = put("GENERIC-LED", 500, 300)
        connect(pin(vcc, "VCC"), pin(q, "S"))
        connect(pin(gnd, "GND"), pin(q, "G"))
        connect(pin(q, "D"), pin(r, "1"))
        connect(pin(r, "2"), pin(led, "A"))
        connect(pin(led, "K"), pin(gnd2, "GND"))
        return led
    }
    print("     GENERIC-PMOS high side, gate on GND: LED \(high.map { $0 ? "lit" : "dark" } ?? "run stopped")")
    check("4 GENERIC-PMOS high side, gate on GND: LED lit") {
        high == true ? nil : "LED \(high.map { $0 ? "lit" : "dark" } ?? "run stopped")"
    }

    // (5) Pictures: an NPN, a MOSFET and an array, and the sidebar.
    let picture = Harness.newDocument()
    var mosfetID: UUID?
    for (index, name) in ["2N3904BU", "IRLML6244TRPBF", "BC846SZ"].enumerated() {
        guard let part = named(name) else { continue }
        picture.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 110 + 200 * index, y: 120))
        if name == "IRLML6244TRPBF" { mosfetID = picture.chips.last!.id }
    }
    let symbolsPath = "\(images)/p40-symbols.png"
    Harness.renderCanvas(picture, to: symbolsPath, size: CGSize(width: 660, height: 260))
    if let mosfetID { picture.select(.chip(mosfetID)) }
    let sidebarPath = "\(images)/p40-sidebar.png"
    Harness.renderSidebar(picture, to: sidebarPath, size: CGSize(width: 320, height: 640))
    check("5 images written") {
        let missing = [symbolsPath, sidebarPath].filter { !FileManager.default.fileExists(atPath: $0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
