// plan-29-M: diodes end to end on the app's real catalog, document,
// simulation and views. Checks the drawer leaves, the kind name, facts and
// symbol of five diode shapes (a Vishay rectifier, a Schottky, a Zener, an
// array, a bridge), runs a diode-OR (two SN74HC04N outputs → two 1N4148
// anodes → common cathode on PULLDOWN and GENERIC-LED → GND) for each
// inverter input pair 00 01 10 11, and renders three placed diodes. Prints
// one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p29-h && APP=/tmp/p29w-dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/diode-app.swift /tmp/p29-h && \
//     /tmp/p29-h/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p29-symbols.png (1N4148, a Schottky, a Zener),
// p29-sidebar.png (the Zener selected).
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runDiodeChecks() }

@MainActor func runDiodeChecks() {
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

    // (1) The drawer: Discrete → Diodes → the six leaves.
    let groups = Harness.catalog.groups(matching: DrawerFilter())
    let diodes = groups.first { $0.category == "Discrete" }?.subcategories.first { $0.subcategory == "Diodes" }
    let leaves = (diodes?.functions ?? []).map(\.function)
    check("1 drawer: Discrete → Diodes leaves \(leaves)") {
        let want = ["Bridge", "Fast recovery", "Rectifier", "Schottky", "Switching", "Zener"]
        return Set(leaves) == Set(want) ? nil : "want \(want)"
    }
    let all = (diodes?.functions ?? []).flatMap(\.parts)
    print("     diode parts in the drawer: \(all.count)")

    // (2) Kind name, facts and symbol for five shapes.
    let doc = Harness.newDocument()
    func place(_ part: CatalogPart, at point: CGPoint) -> PlacedChip {
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return doc.chips.last!
    }
    func named(_ name: String) -> CatalogPart? { Harness.catalog.part(named: name) }
    func diodeType(_ part: CatalogPart) -> String? {
        if case let .diode(type, _, _, _, _, _) = part.details { return type }
        return nil
    }
    func diodeCount(_ part: CatalogPart) -> Int {
        if case let .diode(_, n, _, _, _, _) = part.details { return n }
        return 0
    }
    let rectifier = named("1N4001-E3/54") ?? all.first { $0.manufacturer == "Vishay" && diodeType($0) == "rectifier" }
    let schottky = all.first { diodeType($0) == "schottky" && diodeCount($0) == 1 }
    let zener = all.first { diodeType($0) == "zener" && diodeCount($0) == 1 }
    let array = all.first { diodeType($0) != "bridge" && diodeCount($0) == 2 }
    let bridge = all.first { diodeType($0) == "bridge" }
    let shapes: [(String, CatalogPart?, PartSymbol, [String])] = [
        ("rectifier", rectifier, .plainDiode, ["Type: Rectifier", "Reverse voltage: 50 V", "Forward current: 1 A"]),
        ("schottky", schottky, .plainDiode, ["Type: Schottky"]),
        ("zener", zener, .plainDiode, ["Type: Zener", "Zener voltage:", "Power:"]),
        ("array", array, .block, ["Diodes: 2"]),
        ("bridge", bridge, .block, ["Type: Bridge", "Diodes: 4"]),
    ]
    var placed: [String: PlacedChip] = [:]
    for (index, (label, part, wantSymbol, wantFacts)) in shapes.enumerated() {
        guard let part else {
            print("FAIL 2 \(label): no such part in the catalog")
            failures += 1
            continue
        }
        let chip = place(part, at: CGPoint(x: 120 + 200 * index, y: 900))
        placed[label] = chip
        let rows = PartFacts.rows(for: chip).map { "\($0.label): \($0.value)" }
        print("     facts \(part.name): \(rows.joined(separator: "; "))")
        check("2 \(label) \(part.name): kind Diode, symbol \(wantSymbol), facts \(wantFacts)") {
            if PartFacts.kindName(part) != "Diode" { return "kind \(PartFacts.kindName(part))" }
            if symbol(for: part) != wantSymbol { return "symbol \(symbol(for: part))" }
            let missing = wantFacts.filter { want in !rows.contains { $0.hasPrefix(want) } }
            if label == "rectifier" && part.name != "1N4001-E3/54" { return "1N4001-E3/54 is not in the catalog" }
            return missing.isEmpty ? nil : "missing \(missing)"
        }
    }

    // (3) Diode-OR: LED lit while either inverter output is 1.
    func diodeOR(_ a: Bool, _ b: Bool) -> Bool? {
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
        let inv = put("SN74HC04N", 200, 200)
        let vcc = put("5V", 60, 60)
        let gnd = put("GND", 60, 400)
        connect(pin(vcc, "VCC"), pin(inv, "VCC"))
        connect(pin(gnd, "GND"), pin(inv, "GND"))
        connect(pin(a ? vcc : gnd, a ? "VCC" : "GND"), pin(inv, "1A"))
        connect(pin(b ? vcc : gnd, b ? "VCC" : "GND"), pin(inv, "2A"))
        let d1 = put("1N4148", 420, 160)
        let d2 = put("1N4148", 420, 260)
        connect(pin(inv, "1Y"), pin(d1, "A"))
        connect(pin(inv, "2Y"), pin(d2, "A"))
        connect(pin(d1, "K"), pin(d2, "K"))
        let pd = put("PULLDOWN", 560, 320)
        let led = put("GENERIC-LED", 620, 200)
        let ledGND = put("GND", 720, 300)
        connect(pin(d1, "K"), pin(pd, pd.layout.slots.first!.pin.name))
        connect(pin(d1, "K"), pin(led, "A"))
        connect(pin(led, "K"), pin(ledGND, "GND"))
        let sim = Harness.simulation
        sim.start(document: d)
        RunLoop.main.run(until: Date().addingTimeInterval(0.6))
        let on = (sim.lights[led.id]?.first ?? 0) > 0.5
        let stopped = sim.errorMessage
        sim.stop()
        if let stopped { print("     run \(a ? 1 : 0)\(b ? 1 : 0) stopped: \(stopped)"); return nil }
        return on
    }
    for (a, b) in [(false, false), (false, true), (true, false), (true, true)] {
        let want = !(a && b)  // the outputs are NOT a, NOT b
        let got = diodeOR(a, b)
        print("     inputs \(a ? 1 : 0)\(b ? 1 : 0): LED \(got.map { $0 ? "lit" : "dark" } ?? "run stopped")")
        check("3 diode-OR inputs \(a ? 1 : 0)\(b ? 1 : 0): LED \(want ? "lit" : "dark")") {
            got == want ? nil : "LED \(got.map { $0 ? "lit" : "dark" } ?? "run stopped")"
        }
    }

    // (4) Pictures: 1N4148, a Schottky and a Zener side by side, and the sidebar.
    let picture = Harness.newDocument()
    var zenerID: UUID?
    for (index, part) in [named("1N4148"), schottky, zener].compactMap({ $0 }).enumerated() {
        picture.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100 + 160 * index, y: 100))
        if part.name == zener?.name { zenerID = picture.chips.last!.id }
    }
    let symbolsPath = "\(images)/p29-symbols.png"
    Harness.renderCanvas(picture, to: symbolsPath, size: CGSize(width: 560, height: 220))
    if let zenerID { picture.select(.chip(zenerID)) }
    let sidebarPath = "\(images)/p29-sidebar.png"
    Harness.renderSidebar(picture, to: sidebarPath, size: CGSize(width: 320, height: 640))
    check("4 images written") {
        let missing = [symbolsPath, sidebarPath].filter { !FileManager.default.fileExists(atPath: $0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
