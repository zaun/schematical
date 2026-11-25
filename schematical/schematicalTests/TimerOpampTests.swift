import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-48-H: 555-type timers and op-amps in the app, on the real catalog:
/// kind names, facts, references and the drawer (phase 1); placed values to
/// the backend, the timer state and its text (phase 2).
@MainActor
final class TimerOpampTests: XCTestCase {
    private func rows(_ chip: PlacedChip) -> [String: String] {
        Dictionary(PartFacts.rows(for: chip).map { ($0.label, $0.value) }, uniquingKeysWith: { first, _ in first })
    }

    /// A placed GENERIC-555 is U1, a "Timer" drawn as a block of its eight pins; its facts are
    /// the process and the highest astable frequency.
    func testGenericTimer() throws {
        let doc = TestSupport.newDocument()
        let t = try TestSupport.place(doc, "GENERIC-555", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(t.reference, "U1")
        XCTAssertEqual(PartFacts.kindName(t.part), "Timer")
        XCTAssertEqual(t.symbol, .block)
        XCTAssertEqual(t.part.pins.count, 8)
        XCTAssertEqual(ReferenceCategory.of(t.part), .integratedCircuit)
        XCTAssertEqual(t.part.timer?.timers, 1)
        XCTAssertEqual(t.part.timer?.process, "bipolar")
        let r = rows(t)
        XCTAssertEqual(r["Kind"], "Timer")
        XCTAssertEqual(r["Process"], "Bipolar")
        XCTAssertEqual(r["Max frequency"], PartFacts.hertz(500_000))
        XCTAssertNil(r["Timers"])
        XCTAssertEqual(t.part.detailLabels, ["Bipolar"])
        XCTAssertTrue(t.part.isSimulated)
    }

    /// A 556 part is a "Dual timer" with a Timers row; a CMOS one says so.
    func testDualAndCmosTimers() throws {
        let dual = try XCTUnwrap(TestSupport.allParts.first { $0.timer?.timers == 2 })
        XCTAssertEqual(PartFacts.kindName(dual), "Dual timer")
        XCTAssertEqual(dual.function, "Dual (556)")
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, dual.name, at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(rows(chip)["Timers"], "2")
        let cmos = try XCTUnwrap(TestSupport.allParts.first { $0.timer?.process == "cmos" })
        XCTAssertEqual(cmos.detailLabels, ["CMOS"])
    }

    /// GENERIC-OPAMP is a "Dual op-amp"; a quad part a "Quad op-amp" and a single an "Op-amp";
    /// an op-amp is simulated, so a placed one leaves Run on.
    func testOpamps() throws {
        let doc = TestSupport.newDocument()
        let o = try TestSupport.place(doc, "GENERIC-OPAMP", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(o.reference, "U1")
        XCTAssertEqual(PartFacts.kindName(o.part), "Dual op-amp")
        XCTAssertEqual(o.symbol, .block)
        XCTAssertEqual(ReferenceCategory.of(o.part), .integratedCircuit)
        let r = rows(o)
        XCTAssertEqual(r["Kind"], "Dual op-amp")
        XCTAssertEqual(r["Amplifiers"], "2")
        XCTAssertEqual(r["Input"], "General purpose")
        XCTAssertEqual(r["Gain bandwidth"], PartFacts.hertz(1_000_000))
        XCTAssertEqual(r["Play mode"], "Not simulated: drives nothing")
        XCTAssertEqual(o.part.detailLabels, [PartFacts.hertz(1_000_000)])
        XCTAssertTrue(o.part.isSimulated)
        XCTAssertTrue(SimulationAvailability.unsimulated(doc.chips).isEmpty)
        XCTAssertTrue(SimulationAvailability.canStart(doc))
        let quad = try XCTUnwrap(TestSupport.allParts.first { $0.opamp?.amplifiers == 4 })
        XCTAssertEqual(PartFacts.kindName(quad), "Quad op-amp")
        let single = try XCTUnwrap(TestSupport.allParts.first { $0.opamp?.amplifiers == 1 })
        XCTAssertEqual(PartFacts.kindName(single), "Op-amp")
        XCTAssertNil(rows(try TestSupport.place(doc, single.name, at: CGPoint(x: 300, y: 100)))["Amplifiers"])
        let second = try TestSupport.place(doc, "GENERIC-OPAMP", at: CGPoint(x: 500, y: 100))
        XCTAssertEqual(second.reference, "U3")
    }

    /// The drawer: Analog → Timers (Generic, Single (555), Dual (556)) and Analog → Op-amps
    /// (Generic, and the groups Single, Dual and Quad with their types as functions).
    func testDrawer() throws {
        let groups = TestSupport.catalog.groups(matching: DrawerFilter())
        let analog = try XCTUnwrap(groups.first { $0.category == "Analog" })
        let timers = try XCTUnwrap(analog.subcategories.first { $0.subcategory == "Timers" })
        XCTAssertEqual(timers.path, ["Analog", "Timers"])
        XCTAssertEqual(Set(timers.functions.map(\.function)), ["Generic", "Single (555)", "Dual (556)"])
        let opamps = try XCTUnwrap(analog.subcategories.first { $0.subcategory == "Op-amps" })
        XCTAssertEqual(opamps.path, ["Analog", "Op-amps"])
        XCTAssertEqual(opamps.functions.map(\.function), ["Generic"])
        XCTAssertEqual(Set(opamps.subgroups.map(\.subcategory)), ["Single", "Dual", "Quad"])
        for sub in opamps.subgroups {
            XCTAssertEqual(Set(sub.functions.map(\.function)), ["General purpose", "JFET input", "CMOS"], sub.subcategory)
        }
        XCTAssertTrue(TestSupport.catalog.kinds(inGroup: ["Analog", "Timers"]) == [PART_TIMER.rawValue])
        XCTAssertTrue(TestSupport.catalog.kinds(inGroup: ["Analog", "Op-amps", "Dual"]) == [PART_OPAMP.rawValue])
    }

    // MARK: - Phase 2: placed values, the timer state, its text

    private static var keptSimulations: [SimulationController] = []

    private func spin(for seconds: TimeInterval = 0.6) {
        RunLoop.main.run(until: Date().addingTimeInterval(seconds))
    }

    /// The astable circuit of plan-48-E § 6 (k): 555, 1 kΩ from 5V to DISCH, 10 kΩ from DISCH to
    /// THRES and TRIG, 100 nF to GND, RESET on 5V, OUT through 1 kΩ to an LED to GND. With
    /// `misWired` the 10 kΩ resistor's DISCH end is on 5V instead.
    private func astable(misWired: Bool = false) throws
        -> (doc: SchematicDocument, timer: PlacedChip, led: PlacedChip) {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 700))
        let timer = try TestSupport.place(doc, "GENERIC-555", at: CGPoint(x: 400, y: 300))
        let ra = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 200, y: 150))
        let rb = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 200, y: 300))
        let cap = try TestSupport.place(doc, "GENERIC-CAPACITOR", at: CGPoint(x: 200, y: 450))
        let rled = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 700, y: 300))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 800, y: 450))
        doc.setPartOhms(ra.id, 1000)
        doc.setPartOhms(rb.id, 10_000)
        func connect(_ a: PlacedChip, _ x: String, _ b: PlacedChip, _ y: String) throws {
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(a, x)), .pin(try TestSupport.pin(b, y))), "\(x) to \(y)")
        }
        try connect(vcc, "VCC", timer, "VCC")
        try connect(gnd, "GND", timer, "GND")
        try connect(vcc, "VCC", timer, "RESET")
        try connect(vcc, "VCC", ra, "1")
        try connect(ra, "2", timer, "DISCH")
        if misWired {
            try connect(vcc, "VCC", rb, "1")
        } else {
            try connect(timer, "DISCH", rb, "1")
        }
        try connect(rb, "2", timer, "THRES")
        try connect(timer, "THRES", timer, "TRIG")
        try connect(timer, "THRES", cap, "1")
        try connect(cap, "2", gnd, "GND")
        try connect(timer, "OUT", rled, "1")
        try connect(rled, "2", led, "A")
        try connect(led, "K", gnd, "GND")
        return (doc, timer, led)
    }

    /// The timing text for the times of plan-48-E § 6 (k) and (r).
    func testTimerText() {
        let astable = TimerSection(mode: TIMER_ASTABLE, reason: TIMER_OK, firstPs: 1_208_473_518,
                                   highPs: 762_461_899, lowPs: 693_147_181)
        XCTAssertEqual(TimerText.mode(astable), "Astable")
        XCTAssertEqual(TimerText.frequency(astable), "687 Hz")
        XCTAssertEqual(TimerText.duty(astable), "52.4 % high")
        XCTAssertEqual(TimerText.duration(762_461_899), "762 µs")
        XCTAssertEqual(TimerText.duration(693_147_181), "693 µs")
        XCTAssertEqual(TimerText.duration(1_098_612_289), "1.1 ms")
        XCTAssertEqual(TimerText.duration(20_800), "20.8 ns")
        XCTAssertEqual(TimerText.duration(999_600_000), "1 ms")
        XCTAssertEqual(TimerText.duration(2_500_000_000_000), "2.5 s")
        let shot = TimerSection(mode: TIMER_ONE_SHOT, reason: TIMER_OK, firstPs: 0, highPs: 1_098_612_289, lowPs: 0)
        XCTAssertEqual(TimerText.mode(shot), "One-shot")
        XCTAssertEqual(TimerText.frequency(shot), "")
        let latch = TimerSection(mode: TIMER_LATCH, reason: TIMER_OK, firstPs: 0, highPs: 0, lowPs: 0)
        XCTAssertEqual(TimerText.mode(latch), "Latch")
        XCTAssertNil(TimerText.warning([astable, shot, latch]))
        let stopped = TimerSection(mode: TIMER_STOPPED, reason: TIMER_SHAPE, firstPs: 0, highPs: 0, lowPs: 0)
        XCTAssertEqual(TimerText.mode(stopped), "Not timing")
        XCTAssertEqual(TimerText.reason(TIMER_SHAPE),
                       "The resistors and capacitors on TRIG, THRES and DISCH do not form an astable or a one-shot circuit")
        XCTAssertEqual(TimerText.warning([astable, stopped]), TimerText.reason(TIMER_SHAPE))
    }

    /// The netlist sends every generic resistor's, pull resistor's and generic capacitor's value.
    func testNetlistValues() throws {
        let doc = TestSupport.newDocument()
        let r = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 100, y: 100))
        try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 300, y: 100))
        try TestSupport.place(doc, "GENERIC-CAPACITOR", at: CGPoint(x: 500, y: 100))
        try TestSupport.place(doc, "RC0603FR-074K7L", at: CGPoint(x: 700, y: 100))
        doc.setPartOhms(r.id, 4700)
        let netlist = NetlistBuilder.build(chips: doc.chips, wires: doc.wires, buses: doc.buses, labels: doc.netLabels)
        let byPart = Dictionary(uniqueKeysWithValues: netlist.values.map { ($0.part, $0.value) })
        XCTAssertEqual(byPart[0], 4_700_000)
        XCTAssertEqual(byPart[1], 1_000_000)
        XCTAssertEqual(byPart[2], 100_000_000)
        XCTAssertNil(byPart[3], "a maker resistor keeps its own value")
        XCTAssertEqual(netlist.values.count, 3)
    }

    /// The astable circuit starts, the 555 reports its times, and OUT blinks the LED.
    func testAstableCircuit() throws {
        let (doc, timer, led) = try astable()
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        let sections = try XCTUnwrap(simulation.timerSections[timer.id])
        XCTAssertEqual(sections.count, 1)
        XCTAssertEqual(sections[0].mode, TIMER_ASTABLE)
        XCTAssertEqual(sections[0].reason, TIMER_OK)
        XCTAssertEqual(Double(sections[0].highPs), 762_461_899, accuracy: 1)
        XCTAssertEqual(TimerText.frequency(sections[0]), "687 Hz")
        XCTAssertNil(TimerText.warning(sections))
        spin()
        XCTAssertGreaterThan(simulation.lights[led.id]?.first ?? 0, 0.1, "the LED blinks")
        simulation.stop()
        XCTAssertTrue(simulation.timerSections.isEmpty)
    }

    /// The same circuit with the 10 kΩ resistor's DISCH end on 5V: not timing, with the reason.
    func testMisWiredCircuit() throws {
        let (doc, timer, led) = try astable(misWired: true)
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        let sections = try XCTUnwrap(simulation.timerSections[timer.id])
        XCTAssertEqual(sections[0].mode, TIMER_STOPPED)
        XCTAssertEqual(sections[0].reason, TIMER_SHAPE)
        XCTAssertEqual(TimerText.mode(sections[0]), "Not timing")
        XCTAssertEqual(TimerText.warning(sections), TimerText.reason(TIMER_SHAPE))
        spin()
        XCTAssertLessThan(simulation.lights[led.id]?.first ?? 1, 0.01, "the LED stays dark")
        simulation.stop()
    }
}
