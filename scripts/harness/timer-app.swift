// plan-48-H: 555 timers and op-amps end to end on the app's real catalog,
// document, simulation and views. Checks the drawer (Analog → Timers,
// Analog → Op-amps), the kind names and facts, an op-amp leaving Run on, an
// astable 555 circuit run for 0.6 s from the placed values (the OUT net
// toggles, the LED has on-time, the Timing rows read Astable / 687 Hz /
// 52.4 % high) and the same circuit mis-wired (OUT stays 0, the LED is dark,
// the timer's warning mark says why). Renders the mis-wired circuit running
// (the mark and the sidebar's Timing section) and a placed op-amp.
// Prints one ok/FAIL line per check, then `all checks ok`; exits 1 on any FAIL.
//
//   APP=/tmp/p48-dd2/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/timer-app.swift /tmp/p48-harness && \
//     /tmp/p48-harness/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p48-symbols.png, p48-timing.png.
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runTimerChecks() }

@MainActor func runTimerChecks() {
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
    func place(_ part: CatalogPart, in d: SchematicDocument, at point: CGPoint) -> PlacedChip {
        d.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return d.chips.last!
    }
    func put(_ d: SchematicDocument, _ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
        place(Harness.catalog.part(named: name)!, in: d, at: CGPoint(x: x, y: y))
    }
    func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
        let slot = chip.layout.slots.first { $0.pin.name == name }!
        return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
    }

    // (1) The drawer.
    let groups = Harness.catalog.groups(matching: DrawerFilter())
    let analog = groups.first { $0.category == "Analog" }
    let timers = analog?.subcategories.first { $0.subcategory == "Timers" }
    let opamps = analog?.subcategories.first { $0.subcategory == "Op-amps" }
    check("1 drawer: Analog → Timers \(timers?.functions.map(\.function) ?? [])") {
        let functions = Set((timers?.functions ?? []).map(\.function))
        return functions == ["Generic", "Single (555)", "Dual (556)"] ? nil : "functions \(functions)"
    }
    check("1 drawer: Analog → Op-amps \(opamps?.functions.map(\.function) ?? []) and its groups") {
        guard let opamps else { return "no Op-amps group" }
        if opamps.functions.map(\.function) != ["Generic"] { return "functions \(opamps.functions.map(\.function))" }
        if Set(opamps.subgroups.map(\.subcategory)) != ["Single", "Dual", "Quad"] {
            return "subgroups \(opamps.subgroups.map(\.subcategory))"
        }
        for sub in opamps.subgroups where Set(sub.functions.map(\.function)) != ["General purpose", "JFET input", "CMOS"] {
            return "\(sub.subcategory): \(sub.functions.map(\.function))"
        }
        return nil
    }

    // (2) Kind names and facts.
    let doc = Harness.newDocument()
    let timer = put(doc, "GENERIC-555", 100, 100)
    check("2 GENERIC-555: U1, Timer, a block of eight pins") {
        if timer.reference != "U1" { return "reference \(timer.reference)" }
        if PartFacts.kindName(timer.part) != "Timer" { return "kind \(PartFacts.kindName(timer.part))" }
        if symbol(for: timer.part) != .block { return "symbol \(symbol(for: timer.part))" }
        let rows = PartFacts.rows(for: timer).map { "\($0.label): \($0.value)" }
        print("     facts: \(rows.joined(separator: "; "))")
        return timer.part.pins.count == 8 ? nil : "\(timer.part.pins.count) pins"
    }
    let dual = Harness.catalog.groups(matching: DrawerFilter()).flatMap(\.allParts).first { $0.timer?.timers == 2 }
    check("2 dual timer \(dual?.name ?? "-"): Dual timer") {
        guard let dual else { return "no 556" }
        return PartFacts.kindName(dual) == "Dual timer" ? nil : PartFacts.kindName(dual)
    }
    let opamp = put(doc, "GENERIC-OPAMP", 400, 100)
    check("2 GENERIC-OPAMP: U2, Dual op-amp, Amplifiers 2, the Play mode row") {
        if opamp.reference != "U2" { return "reference \(opamp.reference)" }
        if PartFacts.kindName(opamp.part) != "Dual op-amp" { return "kind \(PartFacts.kindName(opamp.part))" }
        let rows = PartFacts.rows(for: opamp)
        print("     facts: \(rows.map { "\($0.label): \($0.value)" }.joined(separator: "; "))")
        let have = Dictionary(rows.map { ($0.label, $0.value) }, uniquingKeysWith: { first, _ in first })
        if have["Amplifiers"] != "2" { return "Amplifiers \(have["Amplifiers"] ?? "-")" }
        return have["Play mode"] == "Not simulated: drives nothing" ? nil : "Play mode \(have["Play mode"] ?? "-")"
    }
    check("2 a placed op-amp leaves Run on") {
        SimulationAvailability.canStart(doc) ? nil : (SimulationAvailability.reason(doc.chips) ?? "cannot start")
    }
    let all = Harness.catalog.groups(matching: DrawerFilter()).flatMap(\.allParts)
    check("2 quad and single op-amp names") {
        guard let quad = all.first(where: { $0.opamp?.amplifiers == 4 }),
              let single = all.first(where: { $0.opamp?.amplifiers == 1 }) else { return "no quad or single op-amp" }
        let names = [PartFacts.kindName(quad), PartFacts.kindName(single)]
        return names == ["Quad op-amp", "Op-amp"] ? nil : "\(names)"
    }

    // (3) The astable circuit, then the same with the 10 kΩ resistor's DISCH end on 5V.
    struct Circuit {
        let doc: SchematicDocument
        let timer: PlacedChip
        let led: PlacedChip
    }
    func build(misWired: Bool) -> Circuit {
        let d = Harness.newDocument()
        let vcc = put(d, "5V", 60, 60)
        let gnd = put(d, "GND", 60, 700)
        let t = put(d, "GENERIC-555", 400, 300)
        let ra = put(d, "GENERIC-RESISTOR", 200, 150)
        let rb = put(d, "GENERIC-RESISTOR", 200, 300)
        let cap = put(d, "GENERIC-CAPACITOR", 200, 450)
        let ro = put(d, "GENERIC-RESISTOR", 700, 300)
        let led = put(d, "GENERIC-LED", 800, 450)
        d.setPartOhms(ra.id, 1000)
        d.setPartOhms(rb.id, 10_000)
        func connect(_ x: LineAnchor, _ y: LineAnchor) {
            if let message = d.connect(x, y) { print("FAIL wiring: \(message)"); failures += 1 }
        }
        connect(pin(vcc, "VCC"), pin(t, "VCC"))
        connect(pin(gnd, "GND"), pin(t, "GND"))
        connect(pin(vcc, "VCC"), pin(t, "RESET"))
        connect(pin(vcc, "VCC"), pin(ra, "1"))
        connect(pin(ra, "2"), pin(t, "DISCH"))
        if misWired {
            connect(pin(vcc, "VCC"), pin(rb, "1"))
        } else {
            connect(pin(t, "DISCH"), pin(rb, "1"))
        }
        connect(pin(rb, "2"), pin(t, "THRES"))
        connect(pin(t, "THRES"), pin(t, "TRIG"))
        connect(pin(t, "THRES"), pin(cap, "1"))
        connect(pin(cap, "2"), pin(gnd, "GND"))
        connect(pin(t, "OUT"), pin(ro, "1"))
        connect(pin(ro, "2"), pin(led, "A"))
        connect(pin(led, "K"), pin(gnd, "GND"))
        return Circuit(doc: d, timer: t, led: led)
    }
    func outNet(_ c: Circuit) -> Int? {
        let netlist = NetlistBuilder.build(chips: c.doc.chips, wires: c.doc.wires, buses: c.doc.buses,
                                           labels: c.doc.netLabels)
        guard let part = netlist.chipIDs.firstIndex(of: c.timer.id) else { return nil }
        let designator = c.timer.part.pins.first { $0.name == "OUT" }?.designator
        return netlist.connections.first { $0.part == part && $0.pin == designator }?.net
    }

    let good = build(misWired: false)
    let sim = Harness.simulation
    sim.start(document: good.doc)
    RunLoop.main.run(until: Date().addingTimeInterval(0.6))
    let goodSections = sim.timerSections[good.timer.id] ?? []
    let goodToggling = outNet(good).map { sim.togglingNets.contains($0) } ?? false
    let goodLED = sim.lights[good.led.id]?.first ?? 0
    let goodError = sim.errorMessage
    sim.stop()
    check("3 astable run: Astable, 687 Hz, 52.4 % high; the OUT net toggles; the LED has on-time") {
        if let goodError { return "stopped: \(goodError)" }
        guard let section = goodSections.first else { return "no timer section" }
        let rows = [TimerText.mode(section), TimerText.frequency(section), TimerText.duty(section)]
        print("     Timing rows: \(rows.joined(separator: ", ")); high \(TimerText.duration(section.highPs)), low \(TimerText.duration(section.lowPs))")
        if rows != ["Astable", "687 Hz", "52.4 % high"] { return "rows \(rows)" }
        if !goodToggling { return "the OUT net did not toggle" }
        return goodLED > 0 ? nil : "the LED has no on-time"
    }

    let bad = build(misWired: true)
    sim.start(document: bad.doc)
    RunLoop.main.run(until: Date().addingTimeInterval(0.6))
    let badSections = sim.timerSections[bad.timer.id] ?? []
    let badToggling = outNet(bad).map { sim.togglingNets.contains($0) } ?? true
    let badLED = sim.lights[bad.led.id]?.first ?? 1
    let badError = sim.errorMessage
    let warning = TimerText.warning(badSections)
    // The picture while running: the block's mark and the sidebar's Timing section.
    bad.doc.select(.chip(bad.timer.id))
    Harness.renderCanvas(bad.doc, to: "\(images)/p48-timing.png", size: CGSize(width: 900, height: 760))
    Harness.renderSidebar(bad.doc, to: "\(images)/p48-sidebar.png", size: CGSize(width: 320, height: 1100))
    sim.stop()
    check("3 mis-wired run: Not timing with the reason; OUT stays 0; the LED is dark; the mark's label") {
        if let badError { return "stopped: \(badError)" }
        guard let section = badSections.first else { return "no timer section" }
        if TimerText.mode(section) != "Not timing" || section.reason != TIMER_SHAPE {
            return "\(TimerText.mode(section)) \(section.reason)"
        }
        if badToggling { return "the OUT net toggled" }
        if badLED > 0.01 { return "the LED is lit" }
        let label = warning.map { "Not timing: \($0)" }
        print("     warning mark: \(label ?? "-")")
        return label == "Not timing: " + TimerText.reason(TIMER_SHAPE) ? nil : "label \(label ?? "-")"
    }

    // (4) Picture: a placed timer and an op-amp.
    let picture = Harness.newDocument()
    _ = put(picture, "GENERIC-555", 100, 120)
    _ = put(picture, "GENERIC-OPAMP", 320, 120)
    if let quad = all.first(where: { $0.opamp?.amplifiers == 4 }) {
        _ = place(quad, in: picture, at: CGPoint(x: 560, y: 120))
    }
    let symbolsPath = "\(images)/p48-symbols.png"
    Harness.renderCanvas(picture, to: symbolsPath, size: CGSize(width: 760, height: 280))
    check("4 images written") {
        let missing = [symbolsPath, "\(images)/p48-timing.png", "\(images)/p48-sidebar.png"].filter {
            !FileManager.default.fileExists(atPath: $0)
        }
        return missing.isEmpty ? nil : "missing \(missing)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
