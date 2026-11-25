// plan-58-G: slide switches end to end on the app's real catalog, document,
// simulation and views. Checks the drawer leaves (Switches & Relays → Slide →
// SPDT On-On …), wires 5 V → switch → GENERIC-LEDs for an SPDT On-On, an SPDT
// On-Off-On and a DPDT On-On part, clicks the switches (the canvas's click:
// `nextSlidePosition`, `setSlideState`, `setSlide`) and checks which LEDs are
// lit, the click order, the refused move, play mode's restore, and the file
// round trip. Prints one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p58-h && APP=/tmp/p58-dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/slide-switch.swift /tmp/p58-h && \
//     /tmp/p58-h/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p58g-gallery-1.png (SPST, SPDT On-On, SPDT On-Off-On
// in every position), p58g-gallery-2.png (DPST, DPDT On-On, DPDT On-Off-On),
// p58g-circuit.png (the wired SPDT, DPDT and center-off circuits),
// p58g-sidebar.png (the SPDT On-On selected).
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runSlideSwitchChecks() }

@MainActor func runSlideSwitchChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp"
    var failures = 0
    func check(_ name: String, _ body: @MainActor () throws -> String?) {
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
    func part(_ name: String) -> CatalogPart { Harness.catalog.part(named: name)! }

    // (1) The drawer.
    let groups = Harness.catalog.groups(matching: DrawerFilter())
    let slide = groups.first { $0.category == "Switches & Relays" }?.subcategories.first { $0.subcategory == "Slide" }
    check("1 drawer: Switches & Relays → Slide has a leaf per circuit and action") {
        guard let slide else { return "no Switches & Relays → Slide" }
        let leaves = Set(slide.functions.map(\.function))
        let wanted: Set = ["SPST On-Off", "SPDT On-On", "SPDT On-Off-On", "DPST On-Off", "DPDT On-On", "DPDT On-Off-On"]
        if leaves != wanted { return "leaves \(leaves.sorted())" }
        if !slide.parts.isEmpty { return "parts listed directly" }
        let spdt = slide.functions.first { $0.function == "SPDT On-On" }?.parts.first { $0.name == "1101M2S3CQE2" }
        guard let spdt else { return "1101M2S3CQE2 not under SPDT On-On" }
        let row = ([spdt.package] + spdt.detailLabels + [spdt.voltageLabel].compactMap { $0 }).joined(separator: " · ")
        print("     row: \(spdt.name)  \(row)")
        return spdt.count == nil && spdt.trailingLabel == nil ? nil : "has a (n) count"
    }

    let doc = Harness.newDocument()
    func place(_ name: String, at point: CGPoint) -> PlacedChip {
        let p = part(name)
        doc.placeChip(part: p, symbol: symbol(for: p), centeredAt: point)
        return doc.chips.last!
    }
    func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
        let slot = chip.layout.slots.first { $0.pin.name == name }!
        return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
    }
    func connect(_ a: LineAnchor, _ b: LineAnchor) {
        if let message = doc.connect(a, b) { print("FAIL wiring: \(message)"); failures += 1 }
    }
    /// 5 V → `common`; each throw → its LED → GND. Returns the LED chip ids in `throwsPins` order.
    func wire(_ sw: PlacedChip, common: String, throwsPins: [String], x: CGFloat, y: CGFloat,
              supplyAt: CGPoint) -> [UUID] {
        let supply = place("5V", at: supplyAt)
        connect(pin(supply, "VCC"), pin(sw, common))
        var leds: [UUID] = []
        for (index, name) in throwsPins.enumerated() {
            let led = place("GENERIC-LED", at: CGPoint(x: x + 220, y: y - 40 + CGFloat(index) * 80))
            let ground = place("GND", at: CGPoint(x: x + 320, y: y + CGFloat(index) * 80 + 20))
            connect(pin(sw, name), pin(led, "A"))
            connect(pin(led, "K"), pin(ground, "GND"))
            leds.append(led.id)
        }
        return leds
    }

    let spdt = place("1101M2S3CQE2", at: CGPoint(x: 200, y: 100))
    let spdtLEDs = wire(spdt, common: "C", throwsPins: ["A", "B"], x: 200, y: 100, supplyAt: CGPoint(x: 60, y: 60))
    let center = place("1103M2S3CBE2", at: CGPoint(x: 200, y: 400))
    let centerLEDs = wire(center, common: "C", throwsPins: ["A", "B"], x: 200, y: 400, supplyAt: CGPoint(x: 60, y: 360))
    let dpdt = place("1201M2S3CQE2", at: CGPoint(x: 200, y: 700))
    let dpdtLEDs = wire(dpdt, common: "C1", throwsPins: ["A1", "B1"], x: 200, y: 640, supplyAt: CGPoint(x: 60, y: 640))
        + wire(dpdt, common: "C2", throwsPins: ["A2", "B2"], x: 200, y: 780, supplyAt: CGPoint(x: 60, y: 780))

    func position(_ id: UUID) -> Int? { doc.chips.first { $0.id == id }?.slidePosition }
    let sim = Harness.simulation
    func settle() { RunLoop.main.run(until: Date().addingTimeInterval(0.6)) }
    func lit(_ led: UUID) -> Bool { (sim.lights[led]?.first ?? 0) > 0.5 }
    /// The canvas's click on a slide switch's body.
    @discardableResult
    func click(_ id: UUID) -> Int? {
        guard let chip = doc.chips.first(where: { $0.id == id }), let next = chip.nextSlidePosition,
              let state = doc.setSlideState(id, to: next) else { return nil }
        sim.setSlide(id, state: state)
        settle()
        return state
    }
    func litSet(_ leds: [UUID]) -> [Bool] { leds.map(lit) }

    sim.start(document: doc)
    settle()
    check("2 SPDT On-On: LED 1 lit at start; click: LED 2 lit, LED 1 dark; click: back") {
        if !sim.isRunning { return "not running: \(sim.errorMessage ?? "")" }
        if litSet(spdtLEDs) != [true, false] { return "start \(litSet(spdtLEDs))" }
        if click(spdt.id) != 2 || litSet(spdtLEDs) != [false, true] { return "after click 1: \(litSet(spdtLEDs))" }
        if click(spdt.id) != 1 || litSet(spdtLEDs) != [true, false] { return "after click 2: \(litSet(spdtLEDs))" }
        return nil
    }
    check("3 SPDT On-Off-On: starts centre (both dark); positions 2, 3, 2, 1 with LEDs") {
        if position(center.id) != 2 || litSet(centerLEDs) != [false, false] { return "start \(position(center.id) ?? 0) \(litSet(centerLEDs))" }
        // The first click from the middle goes up; reach position 1 first, then walk the order 1 → 2 → 3 → 2 → 1.
        doc.setSlideState(center.id, to: 1)
        sim.setSlide(center.id, state: 1)
        settle()
        var seen: [Int] = [1]
        var lights: [[Bool]] = [litSet(centerLEDs)]
        for _ in 0..<4 {
            guard let state = click(center.id) else { return "a click was refused" }
            seen.append(state)
            lights.append(litSet(centerLEDs))
        }
        if seen != [1, 2, 3, 2, 1] { return "order \(seen)" }
        let wanted = [[true, false], [false, false], [false, true], [false, false], [true, false]]
        return lights == wanted ? nil : "LEDs \(lights)"
    }
    check("4 DPDT On-On: both poles move together (LED pairs swap)") {
        if litSet(dpdtLEDs) != [true, false, true, false] { return "start \(litSet(dpdtLEDs))" }
        click(dpdt.id)
        if litSet(dpdtLEDs) != [false, true, false, true] { return "after click \(litSet(dpdtLEDs))" }
        click(dpdt.id)
        return litSet(dpdtLEDs) == [true, false, true, false] ? nil : "back \(litSet(dpdtLEDs))"
    }
    check("5 play mode: moves made while running are put back by Stop") {
        click(spdt.id)
        if position(spdt.id) != 2 { return "not moved" }
        sim.stop()
        return position(spdt.id) == 1 ? nil : "position after stop \(position(spdt.id) ?? 0)"
    }

    // (6) A move the voltage check refuses: 12 V → a 28 V switch → the 5 V-rated JS102000SAQN's B pin.
    check("6 refused move: joining 12 V to a 5 V-rated switch changes nothing and says why") {
        let probe = Harness.newDocument()
        @MainActor
        func put(_ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
            let p = part(name)
            probe.placeChip(part: p, symbol: symbol(for: p), centeredAt: CGPoint(x: x, y: y))
            return probe.chips.last!
        }
        @MainActor
        func link(_ a: PlacedChip, _ an: String, _ b: PlacedChip, _ bn: String) {
            let slot: (PlacedChip, String) -> LineAnchor = { chip, name in
                .pin(PinRef(chipID: chip.id, index: chip.layout.slots.first { $0.pin.name == name }!.pin.index))
            }
            _ = probe.connect(slot(a, an), slot(b, bn))
        }
        let rail = put("12V", 60, 60)
        let driver = put("1101M2S3CQE2", 200, 100)
        let low = put("JS102000SAQN", 420, 100)
        link(rail, "VCC", driver, "C")
        link(driver, "B", low, "B")
        if probe.wires.count != 2 { return "wiring failed (\(probe.wires.count) wires)" }
        let refused = probe.setSlideState(driver.id, to: 2)
        let after = probe.chips.first { $0.id == driver.id }!
        if refused != nil { return "the move was allowed" }
        guard let message = probe.editorMessage, message.contains("rated 5 V") else { return "message: \(probe.editorMessage ?? "none")" }
        print("     message: \(message)")
        return after.slidePosition == 1 ? nil : "position changed to \(after.slidePosition ?? 0)"
    }

    // (7) The file round trip: park the center switch in position 3, save, reopen.
    check("7 file: position 3 saved as switches [false, true], reopens in position 3") {
        doc.setSlideState(center.id, to: 3)
        let path = "\(images)/p58g-file.schematical"
        try Harness.save(doc, to: path)
        let file = try SchematicFile.decode(Data(contentsOf: URL(fileURLWithPath: path)))
        guard let saved = file.parts.first(where: { $0.partNumber == "1103M2S3CBE2" }) else { return "part not saved" }
        if saved.switches != [false, true] { return "saved \(String(describing: saved.switches))" }
        let opened = try Harness.open(path)
        guard let chip = opened.chips.first(where: { $0.partNumber == "1103M2S3CBE2" }) else { return "part not reopened" }
        return chip.slidePosition == 3 ? nil : "reopened in position \(chip.slidePosition ?? 0)"
    }

    // (8) Pictures. The circuits (SPDT in 1, centre in 3) and the sidebar.
    let circuitPath = "\(images)/p58g-circuit.png"
    Harness.renderCanvas(doc, to: circuitPath, size: CGSize(width: 640, height: 900))
    doc.select(.chip(spdt.id))
    let sidebarPath = "\(images)/p58g-sidebar.png"
    Harness.renderSidebar(doc, to: sidebarPath, size: CGSize(width: 320, height: 700))

    // Every circuit in every position: one column per part, one row per position.
    func gallery(_ names: [String], to path: String, height: CGFloat) {
        let board = Harness.newDocument()
        for (column, name) in names.enumerated() {
            let p = part(name)
            for row in 0..<(p.slideStates?.positions ?? 0) {
                board.placeChip(part: p, symbol: symbol(for: p),
                                centeredAt: CGPoint(x: 100 + 220 * CGFloat(column), y: 100 + 160 * CGFloat(row)))
                board.setSlideState(board.chips.last!.id, to: row + 1)
            }
        }
        Harness.renderCanvas(board, to: path, size: CGSize(width: 220 * CGFloat(names.count) + 20, height: height))
    }
    func firstNamed(_ function: String) -> String {
        Harness.catalog.groups(matching: DrawerFilter()).flatMap(\.allParts)
            .filter { $0.function == function }.map(\.name).sorted().first!
    }
    let gallery1 = "\(images)/p58g-gallery-1.png", gallery2 = "\(images)/p58g-gallery-2.png"
    gallery([firstNamed("SPST On-Off"), "1101M2S3CQE2", "1103M2S3CBE2"], to: gallery1, height: 520)
    gallery([firstNamed("DPST On-Off"), "1201M2S3CQE2", firstNamed("DPDT On-Off-On")], to: gallery2, height: 640)
    check("8 images written") {
        let missing = [circuitPath, sidebarPath, gallery1, gallery2].filter { !FileManager.default.fileExists(atPath: $0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
