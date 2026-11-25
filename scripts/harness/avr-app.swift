// plan-62-H: a simulated microcontroller in the app's real catalog, document, simulation and
// Processor window.
//
//   An ATMEGA328P-PU with 5V and GND and a GENERIC-LED from PB5 to ground. Its Processor window
//   (CpuInspector) is rendered three times:
//     <out>/avr-stopped.png   stopped, no firmware: the "Load file on start" bar says None
//     <out>/avr-file.png      stopped, led_on.hex chosen: the bar names the file
//     <out>/avr-running.png   running: the status, PC, SP, SREG, R0-R31, the flag row
//                             I T H S V N Z C and the last instruction
//   and the run is checked: the LED is lit, the state has 35 registers.
//
//   scripts/harness/build.sh scripts/harness/avr-app.swift /tmp/p62-h && \
//     /tmp/p62-h/H.app/Contents/MacOS/harness /tmp/p62-h
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runAvr() }

@MainActor func runAvr() {
    let out = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p62-h"
    try? FileManager.default.createDirectory(atPath: out, withIntermediateDirectories: true)
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print("\(ok ? "ok" : "FAIL"): \(what)")
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
    func settle(_ seconds: TimeInterval = 0.8) { RunLoop.main.run(until: Date().addingTimeInterval(seconds)) }

    let doc = Harness.newDocument()
    let supply = place(doc, "5V", 60, 60)
    let ground = place(doc, "GND", 60, 600)
    let mcu = place(doc, "ATMEGA328P-PU", 400, 300)
    let led = place(doc, "GENERIC-LED", 800, 300)
    connect(doc, pin(supply, "VCC"), pin(mcu, "VCC"))
    connect(doc, pin(supply, "VCC"), pin(mcu, "AVCC"))
    connect(doc, pin(ground, "GND"), pin(mcu, "GND"))
    connect(doc, pin(mcu, "PB5"), pin(led, "A"))
    connect(doc, pin(led, "K"), pin(ground, "GND"))

    expect(mcu.part.isSimulated && mcu.part.hasProcessorWindow && mcu.part.hasLoadFile,
           "ATMEGA328P-PU is simulated, has a processor window and takes a load file")

    let sim = Harness.simulation
    let size = CGSize(width: 520, height: 520)
    func window() -> some View {
        CpuInspector(chipID: mcu.id, document: doc, simulation: sim)
    }

    // Stopped, no firmware.
    Harness.renderHosted(window(), size: size, to: "\(out)/avr-stopped.png")
    expect(doc.chips.first { $0.id == mcu.id }?.memoryFile == nil, "no firmware chosen: the bar says None")

    // led_on: sbi DDRB,5; sbi PORTB,5; loop: rjmp loop (oracle.js asm).
    let firmware = URL(fileURLWithPath: "\(out)/led_on.hex")
    try? Data(":06000000259A2D9AFFCFA6\n:00000001FF\n".utf8).write(to: firmware)
    doc.setMemoryFile(mcu.id, MemoryFile(url: firmware, bookmark: nil))
    expect(doc.chips.first { $0.id == mcu.id }?.memoryFile?.name == "led_on.hex",
           "the bar names the file: led_on.hex")
    expect(CpuInspector.loadProblem(of: doc.chips.first { $0.id == mcu.id }) == nil, "the file reads and fits the flash")
    Harness.renderHosted(window(), size: size, to: "\(out)/avr-file.png")

    // Running.
    sim.start(document: doc)
    expect(sim.isRunning, "the simulation starts\(sim.errorMessage.map { ": \($0)" } ?? "")")
    settle()
    expect((sim.lights[led.id]?.first ?? 0) > 0.5, "the LED on PB5 is lit")
    let state = sim.cpuState(of: mcu.id)
    expect(state?.status == .running && state?.registers.count == 35, "the processor state: running, 35 registers")
    expect(state?.flags.map(\.label) == ["I", "T", "H", "S", "V", "N", "Z", "C"], "the flag row is I T H S V N Z C")
    expect(state?.instructionText.hasSuffix("RJMP .-2") == true,
           "the last instruction: \(state?.instructionText ?? "none")")
    Harness.renderHosted(window(), size: size, to: "\(out)/avr-running.png")
    sim.stop()

    exit(failures == 0 ? 0 : 1)
}
