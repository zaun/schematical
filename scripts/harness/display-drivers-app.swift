// plan-56-G: the display driver kinds end to end in the app's real catalog, document and simulation.
//
//   1. lcddriver + lcd glass: a PCF1179C (bus-less LCD clock, MODE high: 12-hour, 1:00 AM) wired pin
//      for pin to GENERIC-LCD-CLOCK-1:2 runs in the SimulationController; the glass lights the 1
//      (b, c), the three 0s and AM, and the AM/PM-aware symbol is rendered to <out>/lcd-clock.png.
//   2. leddriver + 7seg: an ICM7228C (parallel, hex) wired to a four-digit common-anode LTC-5623G
//      takes digit 1 = 5 through a tactile switch on WRITE (a pull-up and the switch to ground:
//      the write is the rising edge when the switch is released).
//
//   scripts/harness/build.sh scripts/harness/display-drivers-app.swift /tmp/p56-h && \
//     /tmp/p56-h/H.app/Contents/MacOS/harness /tmp/p56-out
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runDisplayDrivers() }

@MainActor func runDisplayDrivers() {
    let out = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p56-out"
    try? FileManager.default.createDirectory(atPath: out, withIntermediateDirectories: true)
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print("\(ok ? "PASS" : "FAIL"): \(what)")
        if !ok { failures += 1 }
    }
    let catalog = Harness.catalog

    func place(_ doc: SchematicDocument, _ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
        guard let part = catalog.part(named: name) else {
            print("FAIL: no part \(name)")
            exit(1)
        }
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: x, y: y))
        return doc.chips.last!
    }
    func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
        guard let slot = chip.layout.slots.first(where: { $0.pin.name == name }) else {
            print("FAIL: \(chip.part.name) has no pin \(name)")
            exit(1)
        }
        return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
    }
    func connect(_ doc: SchematicDocument, _ a: LineAnchor, _ b: LineAnchor) {
        if let message = doc.connect(a, b) {
            print("FAIL wiring: \(message)")
            failures += 1
        }
    }
    func settle(_ seconds: TimeInterval = 0.6) { RunLoop.main.run(until: Date().addingTimeInterval(seconds)) }

    // ---- 1. PCF1179C on GENERIC-LCD-CLOCK-1:2 ----------------------------------------------------
    do {
        let doc = Harness.newDocument()
        let clock = place(doc, "PCF1179CT,118", 300, 300)
        let glass = place(doc, "GENERIC-LCD-CLOCK-1:2", 700, 300)
        let vcc = place(doc, "5V", 60, 60)
        let gnd = place(doc, "GND", 60, 560)
        expect(symbol(for: clock.part) == .block && PartFacts.kindName(clock.part) == "LCD driver",
               "PCF1179C is an LCD driver drawn as a block")
        expect(catalog.part(named: "GENERIC-LCD-CLOCK-1:2")?.display?.leds.contains { $0.segment == "AM" } == true,
               "the glass has an AM mark")
        for name in clock.part.pins.map(\.name) where !["S1", "S2", "DATA", "OSC_IN", "OSC_OUT", "VPP", "SEL", "VSS",
                                                          "VDD", "MODE", "TS", "ENABLE", "FLASH"].contains(name) {
            if glass.layout.slots.contains(where: { $0.pin.name == name }) {
                connect(doc, pin(clock, name), pin(glass, name))
            }
        }
        connect(doc, pin(vcc, "VCC"), pin(clock, "VDD"))
        connect(doc, pin(vcc, "VCC"), pin(clock, "MODE"))
        connect(doc, pin(vcc, "VCC"), pin(clock, "ENABLE"))
        connect(doc, pin(vcc, "VCC"), pin(clock, "TS"))
        connect(doc, pin(gnd, "GND"), pin(clock, "VSS"))
        connect(doc, pin(gnd, "GND"), pin(clock, "FLASH"))
        let sim = Harness.simulation
        sim.start(document: doc)
        settle()
        let levels = sim.lights[glass.id] ?? []
        let cells = glass.part.display?.leds ?? []
        let stopped = sim.errorMessage
        sim.stop()
        if let stopped { print("     run stopped: \(stopped)") }
        func on(_ digit: Int, _ segment: String) -> Bool {
            guard let i = cells.firstIndex(where: { $0.digit == digit && $0.segment == segment }), i < levels.count else { return false }
            return levels[i] > 0.05
        }
        let summary = cells.indices.filter { $0 < levels.count && levels[$0] > 0.05 }.map { "\(cells[$0].digit)\(cells[$0].segment)" }
        print("     lit cells: \(summary.joined(separator: " "))")
        expect(stopped == nil && !levels.isEmpty, "the simulation runs and reports the glass's lights")
        expect(on(2, "B") && on(2, "C") && !on(2, "A") && !on(2, "G"), "digit 2 (hours) shows 1 (b, c)")
        expect(!on(1, "B") && !on(1, "C"), "the hour tens is blank (1:00, not 11:00)")
        expect(["A", "B", "C", "D", "E", "F"].allSatisfy { on(3, $0) } && !on(3, "G"), "minutes tens shows 0")
        expect(["A", "B", "C", "D", "E", "F"].allSatisfy { on(4, $0) } && !on(4, "G"), "minutes units shows 0")
        expect(on(0, "L1") && on(0, "L2"), "FLASH low: the colon is on")
        expect(on(0, "AM") && !on(0, "PM"), "MODE high: 12-hour, AM lit")
        let view = ZStack(alignment: .topLeading) {
            SevenSegmentSymbolView(chip: glass, isSelected: false, isHovered: false, levels: levels)
                .offset(x: 10, y: 10)
        }
        .frame(width: 520, height: 1180, alignment: .topLeading)
        Harness.render(view, size: CGSize(width: 520, height: 1180), to: "\(out)/lcd-clock.png")
    }

    // ---- 2. ICM7228C on a LTC-5623G through a tactile switch ---------------------------------------
    do {
        let doc = Harness.newDocument()
        let driver = place(doc, "ICM7228CIPIZ", 300, 320)
        let display = place(doc, "LTC-5623G", 700, 320)
        let vcc = place(doc, "5V", 60, 60)
        let gnd = place(doc, "GND", 60, 600)
        let pull = place(doc, "PULLUP-5V", 120, 420)
        let button = place(doc, "EVP-ADK04K", 120, 500)
        let gnd2 = place(doc, "GND", 120, 600)
        expect(PartFacts.kindName(driver.part) == "LED driver", "ICM7228C is an LED driver")
        for (chip, names) in [(driver, ["SEG a", "SEG b", "SEG c", "SEG d", "SEG e", "SEG f", "SEG g", "DP"])] {
            for (n, display_pin) in zip(names, ["A", "B", "C", "D", "E", "F", "G", "DP"]) {
                connect(doc, pin(chip, n), pin(display, display_pin))
            }
        }
        for k in 1...4 { connect(doc, pin(driver, "DIGIT \(k)"), pin(display, "CA\(k)")) }
        connect(doc, pin(vcc, "VCC"), pin(driver, "VDD"))
        connect(doc, pin(gnd, "GND"), pin(driver, "VSS"))
        for p in ["DA0", "DA1", "DA2", "ID1", "ID3"] { connect(doc, pin(gnd, "GND"), pin(driver, p)) }
        for p in ["ID7 (INPUT DP)", "ID0", "ID2", "HEXA/CODE B/SHUTDOWN"] { connect(doc, pin(vcc, "VCC"), pin(driver, p)) }
        connect(doc, pin(pull, pull.layout.slots.first!.pin.name), pin(driver, "WRITE"))
        connect(doc, pin(button, "A"), pin(driver, "WRITE"))
        connect(doc, pin(button, "B"), pin(gnd2, "GND"))
        let sim = Harness.simulation
        sim.start(document: doc)
        settle()
        if let isPressed = doc.setButton(button.id, pressed: true) {
            sim.setSwitch(button.id, position: 1, on: isPressed != button.part.isNormallyClosed)
        }
        settle()
        if let isPressed = doc.setButton(button.id, pressed: false) {
            sim.setSwitch(button.id, position: 1, on: isPressed != button.part.isNormallyClosed)
        }
        settle()
        let levels = sim.lights[display.id] ?? []
        let cells = display.part.display?.leds ?? []
        let stopped = sim.errorMessage
        sim.stop()
        if let stopped { print("     run stopped: \(stopped)") }
        func on(_ digit: Int, _ segment: String) -> Bool {
            guard let i = cells.firstIndex(where: { $0.digit == digit && $0.segment == segment }), i < levels.count else { return false }
            return levels[i] > 0.01
        }
        let summary = cells.indices.filter { $0 < levels.count && levels[$0] > 0.01 }.map { "\(cells[$0].digit)\(cells[$0].segment)" }
        print("     lit segments: \(summary.joined(separator: " "))")
        expect(stopped == nil && !levels.isEmpty, "the simulation runs and reports the display's lights")
        let five = ["A", "C", "D", "F", "G"]
        expect(five.allSatisfy { on(1, $0) } && !on(1, "B") && !on(1, "E"), "digit 1 shows 5 after the write (a, c, d, f, g)")
    }
    print(failures == 0 ? "display-drivers-app: all checks passed" : "FAIL: \(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
