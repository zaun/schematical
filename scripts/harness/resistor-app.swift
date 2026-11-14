// plan-34-G: resistors end to end on the app's real catalog, document,
// simulation and views. Checks the drawer (Resistors → Generic, Array /
// Chip / Through-hole, every leaf in increasing resistance), the kind name,
// facts and symbol of a chip resistor and of an array, the generic resistor
// (R1, 1kΩ on RES-AXIAL-10.16; set to 4.7 kΩ, saved and reopened; undo),
// and an SN74HC04N output lighting a GENERIC-LED through GENERIC-RESISTOR
// while the output is 1. Renders a generic, a chip resistor and an array.
// Prints one ok/FAIL line per check, then `all checks ok`; exits 1 on any FAIL.
//
//   APP=/tmp/p34-dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/resistor-app.swift /tmp/p34-harness && \
//     /tmp/p34-harness/H.app/Contents/MacOS/harness [image dir]
//
// Image (default dir /tmp): p34-symbols.png.
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runResistorChecks() }

@MainActor func runResistorChecks() {
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

    // (1) The drawer.
    let groups = Harness.catalog.groups(matching: DrawerFilter())
    let resistors = groups.first { $0.category == "Resistors" }
    let subs = (resistors?.subcategories ?? []).map(\.subcategory)
    let functions = (resistors?.functions ?? []).map(\.function)
    check("1 drawer: Resistors → \(functions) and \(subs)") {
        if functions != ["Generic"] { return "functions \(functions)" }
        return Set(subs) == ["Array", "Chip", "Through-hole"] ? nil : "subcategories \(subs)"
    }
    let order = ["< 1 kΩ", "1 kΩ – 5 kΩ", "5 kΩ – 10 kΩ", "10 kΩ – 50 kΩ", "50 kΩ – 100 kΩ",
                 "100 kΩ – 500 kΩ", "500 kΩ – 1 MΩ", "≥ 1 MΩ"]
    for sub in resistors?.subcategories ?? [] {
        let leaves = sub.functions.map(\.function)
        let ranks = leaves.map { order.firstIndex(of: $0) ?? -1 }
        let parts = sub.functions.reduce(0) { $0 + $1.parts.count }
        print("     \(sub.subcategory): \(leaves.count) leaves, \(parts) parts: \(leaves.joined(separator: ", "))")
        check("1 \(sub.subcategory) leaves in increasing ohms") {
            !ranks.contains(-1) && zip(ranks, ranks.dropFirst()).allSatisfy { $0 < $1 } ? nil : "order \(leaves)"
        }
    }

    // (2) A chip resistor and an array: kind, facts, symbol.
    let doc = Harness.newDocument()
    func place(_ part: CatalogPart, in d: SchematicDocument, at point: CGPoint) -> PlacedChip {
        d.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return d.chips.last!
    }
    func parts(_ sub: String) -> [CatalogPart] {
        resistors?.subcategories.first { $0.subcategory == sub }?.functions.flatMap(\.parts) ?? []
    }
    let chipPart = parts("Chip").first { part in
        if case let .resistor(_, tol, power, _) = part.details { return tol != nil && power != nil }
        return false
    }
    check("2 chip \(chipPart?.name ?? "-"): kind Resistor, symbol resistor, facts") {
        guard let chipPart else { return "no chip resistor" }
        let chip = place(chipPart, in: doc, at: CGPoint(x: 100, y: 100))
        print("     facts: \(PartFacts.rows(for: chip).map { "\($0.label): \($0.value)" }.joined(separator: "; "))")
        let rows = PartFacts.rows(for: chip).map(\.label)
        if PartFacts.kindName(chipPart) != "Resistor" { return "kind \(PartFacts.kindName(chipPart))" }
        if symbol(for: chipPart) != .resistor { return "symbol \(symbol(for: chipPart))" }
        if chip.reference != "R1" { return "reference \(chip.reference)" }
        let missing = ["Resistance", "Tolerance", "Power"].filter { !rows.contains($0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }
    let arrayPart = parts("Array").first
    check("2 array \(arrayPart?.name ?? "-"): kind Resistor array, symbol block, Resistors row") {
        guard let arrayPart else { return "no array" }
        let chip = place(arrayPart, in: doc, at: CGPoint(x: 400, y: 100))
        print("     facts: \(PartFacts.rows(for: chip).map { "\($0.label): \($0.value)" }.joined(separator: "; "))")
        if PartFacts.kindName(arrayPart) != "Resistor array" { return "kind \(PartFacts.kindName(arrayPart))" }
        if symbol(for: arrayPart) != .block { return "symbol \(symbol(for: arrayPart))" }
        if chip.reference != "R2" { return "reference \(chip.reference)" }
        return PartFacts.rows(for: chip).contains { $0.label == "Resistors" } ? nil : "no Resistors row"
    }

    // (3) The generic resistor.
    let gdoc = Harness.newDocument()
    let generic = Harness.catalog.part(named: "GENERIC-RESISTOR")!
    let placed = place(generic, in: gdoc, at: CGPoint(x: 100, y: 100))
    check("3 GENERIC-RESISTOR: R1, \(ResistorValue.text(placed.ohms ?? 0)), \(placed.footprintID ?? "-")") {
        if placed.reference != "R1" { return "reference \(placed.reference)" }
        if ResistorSymbolView.label(for: placed) != "1kΩ" { return "label \(ResistorSymbolView.label(for: placed))" }
        return placed.footprintID == "RES-AXIAL-10.16" ? nil : "footprint \(placed.footprintID ?? "-")"
    }
    gdoc.setPartOhms(placed.id, 4700)
    let path = NSTemporaryDirectory() + "p34-resistor.schematical"
    check("3 set 4.7 kΩ: shows 4.7kΩ, saves \"ohms\": 4700, reopens 4.7 kΩ") {
        if ResistorSymbolView.label(for: gdoc.chips[0]) != "4.7kΩ" { return "label \(ResistorSymbolView.label(for: gdoc.chips[0]))" }
        try Harness.save(gdoc, to: path)
        let text = try String(contentsOfFile: path, encoding: .utf8)
        if !text.contains("\"ohms\" : 4700") && !text.contains("\"ohms\":4700") && !text.contains("\"ohms\": 4700") {
            return "no ohms 4700 in the file"
        }
        let reopened = try Harness.open(path)
        return reopened.chips.first?.ohms == 4700 ? nil : "reopened \(reopened.chips.first?.ohms ?? 0)"
    }
    check("3 undo: 1 kΩ") {
        guard gdoc.undo(on: .schematic) else { return "nothing to undo" }
        return gdoc.chips.first?.ohms == 1000 ? nil : "after undo \(gdoc.chips.first?.ohms ?? 0)"
    }

    // (4) SN74HC04N 1Y - GENERIC-RESISTOR - GENERIC-LED - GND: lit while 1Y is 1.
    func ledRun(input high: Bool) -> Bool? {
        let d = Harness.newDocument()
        func put(_ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
            place(Harness.catalog.part(named: name)!, in: d, at: CGPoint(x: x, y: y))
        }
        func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
            let slot = chip.layout.slots.first { $0.pin.name == name }!
            return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
        }
        func connect(_ x: LineAnchor, _ y: LineAnchor) {
            if let message = d.connect(x, y) { print("FAIL wiring: \(message)"); failures += 1 }
        }
        let inv = put("SN74HC04N", 200, 200)
        let vcc = put("5V", 60, 60)
        let gnd = put("GND", 60, 400)
        connect(pin(vcc, "VCC"), pin(inv, "VCC"))
        connect(pin(gnd, "GND"), pin(inv, "GND"))
        connect(pin(high ? vcc : gnd, high ? "VCC" : "GND"), pin(inv, "1A"))
        let r = put("GENERIC-RESISTOR", 420, 160)
        let led = put("GENERIC-LED", 620, 200)
        let ledGND = put("GND", 720, 300)
        connect(pin(inv, "1Y"), pin(r, "1"))
        connect(pin(r, "2"), pin(led, "A"))
        connect(pin(led, "K"), pin(ledGND, "GND"))
        let sim = Harness.simulation
        sim.start(document: d)
        RunLoop.main.run(until: Date().addingTimeInterval(0.6))
        let on = (sim.lights[led.id]?.first ?? 0) > 0.5
        let stopped = sim.errorMessage
        sim.stop()
        if let stopped { print("     run \(high ? 1 : 0) stopped: \(stopped)"); return nil }
        return on
    }
    for high in [false, true] {
        let got = ledRun(input: high)
        check("4 inverter input \(high ? 1 : 0) → LED through the resistor \(high ? "dark" : "lit")") {
            guard let got else { return "the run stopped" }
            return got == !high ? nil : "LED \(got ? "lit" : "dark")"
        }
    }

    // (5) Picture: the generic, a chip resistor and an array.
    let picture = Harness.newDocument()
    for (index, part) in [generic, chipPart, arrayPart].compactMap({ $0 }).enumerated() {
        _ = place(part, in: picture, at: CGPoint(x: 100 + 180 * index, y: 120))
    }
    let symbolsPath = "\(images)/p34-symbols.png"
    Harness.renderCanvas(picture, to: symbolsPath, size: CGSize(width: 620, height: 260))
    check("5 image written") {
        FileManager.default.fileExists(atPath: symbolsPath) ? nil : "missing \(symbolsPath)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
