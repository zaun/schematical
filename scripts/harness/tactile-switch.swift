// plan-27-K: tactile switches end to end on the app's real catalog,
// document, simulation and views. Picks the first Normal Open two-terminal,
// Normal Open four-terminal and Normal Close part, wires each as
// 5V → switch → GENERIC-LED → GND, and checks the drawer, the press, the
// joined pins, the file and the pictures. Prints one ok/FAIL line per check;
// exits 1 on any FAIL.
//
//   rm -rf /tmp/p27-h && scripts/harness/build.sh scripts/harness/tactile-switch.swift /tmp/p27-h && \
//     /tmp/p27-h/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p27-symbols.png (the three released symbols),
// p27-pressed.png (the NO switch held), p27-sidebar.png (NO selected).
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runTactileSwitchChecks() }

@MainActor func runTactileSwitchChecks() {
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

    // (1) The drawer, and how long building it takes.
    let started = Date()
    let groups = Harness.catalog.groups(matching: DrawerFilter())
    let seconds = Date().timeIntervalSince(started)
    let tactile = groups.first { $0.category == "Switch" }?.subcategories.first { $0.subcategory == "Tactile" }
    check(String(format: "1 drawer: Switch → Tactile is grouped by body size (groups(matching:) %.3f s)", seconds)) {
        guard let tactile else { return "no Switch → Tactile" }
        let leaves = tactile.functions.map(\.function)
        return leaves.contains("6 × 6 mm") && leaves.allSatisfy { $0.hasSuffix(" mm") } ? nil : "leaves \(leaves)"
    }
    let parts = (tactile?.functions ?? []).flatMap(\.parts).sorted { $0.name < $1.name }
    func terminals(_ part: CatalogPart) -> Int { part.pins.filter { !$0.isHidden }.count }
    guard let noTwo = parts.first(where: { !$0.isNormallyClosed && terminals($0) == 2 }),
          let noFour = parts.first(where: { !$0.isNormallyClosed && terminals($0) == 4 }),
          let nc = parts.first(where: { $0.isNormallyClosed }) else {
        print("FAIL catalog: need a NO 2-terminal, a NO 4-terminal and an NC tactile part")
        exit(1)
    }
    print("     parts: NO2 \(noTwo.name), NO4 \(noFour.name), NC \(nc.name)")

    let doc = Harness.newDocument()
    func place(_ part: CatalogPart, at point: CGPoint) -> PlacedChip {
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return doc.chips.last!
    }
    func placeNamed(_ name: String, at point: CGPoint) -> PlacedChip {
        place(Harness.catalog.part(named: name)!, at: point)
    }
    func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
        let slot = chip.layout.slots.first { $0.pin.name == name }!
        return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
    }
    func connect(_ a: LineAnchor, _ b: LineAnchor) {
        if let message = doc.connect(a, b) { print("FAIL wiring: \(message)"); failures += 1 }
    }
    /// 5V → `from` … `to` → LED → GND, in a row at `y`.
    func row(_ part: CatalogPart, y: CGFloat, from: String, to: String) -> (UUID, UUID) {
        let supply = placeNamed("5V", at: CGPoint(x: 40, y: y - 40))
        let sw = place(part, at: CGPoint(x: 200, y: y))
        let led = placeNamed("GENERIC-LED", at: CGPoint(x: 420, y: y))
        let ground = placeNamed("GND", at: CGPoint(x: 520, y: y + 60))
        connect(pin(supply, "VCC"), pin(sw, from))
        connect(pin(sw, to), pin(led, "A"))
        connect(pin(led, "K"), pin(ground, "GND"))
        return (sw.id, led.id)
    }
    let (noTwoID, noTwoLED) = row(noTwo, y: 100, from: "A", to: "B")
    let (noFourID, noFourLED) = row(noFour, y: 300, from: "A1", to: "B2")
    let (ncID, ncLED) = row(nc, y: 500, from: terminals(nc) == 4 ? "A1" : "A", to: terminals(nc) == 4 ? "B2" : "B")
    // (4) the four-terminal part's A2 drives a second LED: lit at rest (A1–A2 joined).
    let noFourChip = doc.chips.first { $0.id == noFourID }!
    let joinedLED = placeNamed("GENERIC-LED", at: CGPoint(x: 200, y: 420))
    let joinedGND = placeNamed("GND", at: CGPoint(x: 300, y: 460))
    connect(pin(noFourChip, "A2"), pin(joinedLED, "A"))
    connect(pin(joinedLED, "K"), pin(joinedGND, "GND"))

    let sim = Harness.simulation
    sim.start(document: doc)
    func settle() { RunLoop.main.run(until: Date().addingTimeInterval(0.6)) }
    func lit(_ led: UUID) -> Bool { (sim.lights[led]?.first ?? 0) > 0.5 }
    func press(_ id: UUID, _ pressed: Bool) {
        guard let isPressed = doc.setButton(id, pressed: pressed),
              let chip = doc.chips.first(where: { $0.id == id }) else { return }
        sim.setSwitch(id, position: 1, on: isPressed != chip.part.isNormallyClosed)
        settle()
    }
    settle()

    check("2 NO \(noTwo.name): LED dark at rest, lit pressed, dark released") {
        if lit(noTwoLED) { return "lit at rest" }
        press(noTwoID, true)
        if !lit(noTwoLED) { return "dark while pressed" }
        press(noTwoID, false)
        return lit(noTwoLED) ? "lit after release" : nil
    }
    check("3 NC \(nc.name): LED lit at rest, dark pressed, lit released") {
        if !lit(ncLED) { return "dark at rest" }
        press(ncID, true)
        if lit(ncLED) { return "lit while pressed" }
        press(ncID, false)
        return lit(ncLED) ? nil : "dark after release"
    }
    check("4 NO \(noFour.name): A2 joined to A1 (second LED lit at rest); B2 through B1 only pressed") {
        if !lit(joinedLED.id) { return "A2's LED dark at rest" }
        if lit(noFourLED) { return "B2's LED lit at rest" }
        press(noFourID, true)
        if !lit(noFourLED) { return "B2's LED dark while pressed (B1 unwired)" }
        press(noFourID, false)
        return nil
    }

    // (6) Pictures: the three released symbols, then the NO pressed, then the sidebar.
    let symbolsPath = "\(images)/p27-symbols.png"
    Harness.renderCanvas(doc, to: symbolsPath, size: CGSize(width: 640, height: 640))
    _ = doc.setButton(noTwoID, pressed: true)
    let pressedPath = "\(images)/p27-pressed.png"
    Harness.renderCanvas(doc, to: pressedPath, size: CGSize(width: 640, height: 240))
    _ = doc.setButton(noTwoID, pressed: false)
    doc.select(.chip(noTwoID))
    let sidebarPath = "\(images)/p27-sidebar.png"
    Harness.renderSidebar(doc, to: sidebarPath, size: CGSize(width: 320, height: 700))
    check("6 images written") {
        let missing = [symbolsPath, pressedPath, sidebarPath].filter { !FileManager.default.fileExists(atPath: $0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }
    sim.stop()

    // (5) Save pressed, reopen: released, and no `switches` for it.
    check("5 save and reopen: released, no switches key") {
        _ = doc.setButton(noTwoID, pressed: true)
        let path = "\(images)/p27-file.schematical"
        try Harness.save(doc, to: path)
        let text = try String(contentsOfFile: path, encoding: .utf8)
        let file = try SchematicFile.decode(Data(text.utf8))
        guard let saved = file.parts.first(where: { $0.partNumber == noTwo.name }) else { return "part not saved" }
        if saved.switches != nil { return "switches saved: \(saved.switches!)" }
        let opened = try Harness.open(path)
        guard let chip = opened.chips.first(where: { $0.partNumber == noTwo.name }) else { return "part not reopened" }
        return chip.isPressed ? "reopened pressed" : nil
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
