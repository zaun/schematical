// plan-33-H: capacitors end to end on the app's real catalog, document and
// views. Checks the drawer (Capacitors → Generic, Ceramic / Electrolytic /
// Tantalum, every leaf in increasing capacitance), the kind name, facts and
// symbol of a real ceramic part, the generic capacitor (C1, 0.1 µF on the
// disc footprint; set to 4.7 nF, saved and reopened; undo), and the voltage
// check refusing a 6.3 V tantalum on a 12 V supply. Renders a ceramic, an
// electrolytic and a tantalum, and the sidebar of the generic capacitor.
// Prints one ok/FAIL line per check, then `all checks ok`; exits 1 on any FAIL.
//
//   APP=/tmp/p33-dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/capacitor-app.swift /tmp/p33-h && \
//     /tmp/p33-h/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p33-symbols.png, p33-sidebar.png.
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runCapacitorChecks() }

@MainActor func runCapacitorChecks() {
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
    let caps = groups.first { $0.category == "Capacitors" }
    let subs = (caps?.subcategories ?? []).map(\.subcategory)
    let functions = (caps?.functions ?? []).map(\.function)
    check("1 drawer: Capacitors → \(functions) and \(subs)") {
        if functions != ["Generic"] { return "functions \(functions)" }
        return Set(subs) == ["Ceramic", "Electrolytic", "Tantalum"] ? nil : "subcategories \(subs)"
    }
    for sub in caps?.subcategories ?? [] {
        let leaves = sub.functions.map { group in
            (group.function, group.parts.compactMap(\.capacitorFarads).min() ?? -1)
        }
        let increasing = zip(leaves, leaves.dropFirst()).allSatisfy { $0.1 < $1.1 }
        let parts = sub.functions.reduce(0) { $0 + $1.parts.count }
        print("     \(sub.subcategory): \(leaves.count) leaves, \(parts) parts, "
              + "\(leaves.first?.0 ?? "-") … \(leaves.last?.0 ?? "-")")
        check("1 \(sub.subcategory) leaves in strictly increasing capacitance") {
            increasing ? nil : "order \(leaves.map(\.0))"
        }
    }

    // (2) A real ceramic part: kind, facts, symbol.
    let doc = Harness.newDocument()
    func place(_ part: CatalogPart, at point: CGPoint) -> PlacedChip {
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return doc.chips.last!
    }
    let ceramics = caps?.subcategories.first { $0.subcategory == "Ceramic" }?.functions.flatMap(\.parts) ?? []
    let ceramic = ceramics.first { part in
        if case let .capacitor(_, _, _, _, d, t) = part.details { return d != nil && t != nil }
        return false
    }
    check("2 ceramic \(ceramic?.name ?? "-"): kind Capacitor, symbol capacitor, facts") {
        guard let ceramic else { return "no ceramic part" }
        let chip = place(ceramic, at: CGPoint(x: 100, y: 100))
        let rows = PartFacts.rows(for: chip).map(\.label)
        print("     facts: \(PartFacts.rows(for: chip).map { "\($0.label): \($0.value)" }.joined(separator: "; "))")
        if PartFacts.kindName(ceramic) != "Capacitor" { return "kind \(PartFacts.kindName(ceramic))" }
        if symbol(for: ceramic) != .capacitor { return "symbol \(symbol(for: ceramic))" }
        let want = ["Type", "Capacitance", "Rated voltage", "Dielectric", "Tolerance"]
        let missing = want.filter { !rows.contains($0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }

    // (3) The generic capacitor.
    let gdoc = Harness.newDocument()
    let generic = Harness.catalog.part(named: "GENERIC-CAPACITOR")!
    gdoc.placeChip(part: generic, symbol: symbol(for: generic), centeredAt: CGPoint(x: 100, y: 100))
    let placed = gdoc.chips.last!
    check("3 GENERIC-CAPACITOR: C1, \(CapacitorValue.text(placed.farads ?? 0)), \(placed.footprintID ?? "-")") {
        if placed.reference != "C1" { return "reference \(placed.reference)" }
        if placed.farads != 1e-7 || CapacitorValue.text(1e-7) != "100 nF" { return "value \(placed.farads ?? 0)" }
        return placed.footprintID == "CAP-DISC-5.08" ? nil : "footprint \(placed.footprintID ?? "-")"
    }
    gdoc.setPartFarads(placed.id, 4.7e-9)
    let path = NSTemporaryDirectory() + "p33-capacitor.schematical"
    check("3 set 4.7 nF, save, reopen: 4.7 nF") {
        try Harness.save(gdoc, to: path)
        let reopened = try Harness.open(path)
        let value = reopened.chips.first?.farads ?? 0
        return value == 4.7e-9 ? nil : "reopened \(CapacitorValue.text(value))"
    }
    check("3 undo: 0.1 µF") {
        guard gdoc.undo(on: .schematic) else { return "nothing to undo" }
        return gdoc.chips.first?.farads == 1e-7 ? nil : "after undo \(gdoc.chips.first?.farads ?? 0)"
    }

    // (4) A 6.3 V tantalum wired to a 12 V supply is refused.
    let tantalums = caps?.subcategories.first { $0.subcategory == "Tantalum" }?.functions.flatMap(\.parts) ?? []
    let low = tantalums.first { part in
        if case let .capacitor(type, _, _, mv, _, _) = part.details { return type == "tantalum" && mv == 6300 }
        return false
    }
    check("4 \(low?.name ?? "-") (6.3 V) + pin on 12 V is refused") {
        guard let low else { return "no 6.3 V tantalum" }
        let d = Harness.newDocument()
        let supply = Harness.catalog.part(named: "12V")!
        d.placeChip(part: supply, symbol: symbol(for: supply), centeredAt: CGPoint(x: 60, y: 60))
        let rail = d.chips.last!
        d.placeChip(part: low, symbol: symbol(for: low), centeredAt: CGPoint(x: 200, y: 200))
        let cap = d.chips.last!
        func anchor(_ chip: PlacedChip, _ name: String) -> LineAnchor {
            let slot = chip.layout.slots.first { $0.pin.name == name }!
            return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
        }
        let message = d.connect(anchor(rail, rail.layout.slots.first!.pin.name), anchor(cap, "+"))
        print("     refusal: \(message ?? "none")")
        guard let message else { return "the wire was accepted" }
        return message.contains("is rated 6.3 V") && message.contains("12 V") ? nil : "message \(message)"
    }

    // (5) Pictures: a ceramic, an electrolytic and a tantalum; the generic's sidebar.
    let picture = Harness.newDocument()
    let electrolytic = caps?.subcategories.first { $0.subcategory == "Electrolytic" }?.functions.first?.parts.first
    for (index, part) in [ceramic, electrolytic, low].compactMap({ $0 }).enumerated() {
        picture.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100 + 160 * index, y: 100))
    }
    let symbolsPath = "\(images)/p33-symbols.png"
    Harness.renderCanvas(picture, to: symbolsPath, size: CGSize(width: 560, height: 220))
    let side = Harness.newDocument()
    side.placeChip(part: generic, symbol: symbol(for: generic), centeredAt: CGPoint(x: 100, y: 100))
    side.select(.chip(side.chips.last!.id))
    let sidebarPath = "\(images)/p33-sidebar.png"
    Harness.renderSidebar(side, to: sidebarPath, size: CGSize(width: 320, height: 720))
    check("5 images written") {
        let missing = [symbolsPath, sidebarPath].filter { !FileManager.default.fileExists(atPath: $0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
