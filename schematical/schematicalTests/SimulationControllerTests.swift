import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-W: the simulation controller's play-mode views of the circuit —
/// each wire style and pin level, a start the backend refuses (a part it
/// doesn't know, a pin it can't connect, two supplies on one net), the
/// serial reads outside play mode, and a frame whose circuit time stood still.
@MainActor
final class SimulationControllerTests: XCTestCase {
    private static var kept: [SimulationController] = []

    private func controller() -> SimulationController {
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.kept.append(simulation)
        return simulation
    }

    private func spin(_ seconds: TimeInterval = 0.3) { RunLoop.main.run(until: Date().addingTimeInterval(seconds)) }

    /// Wires `a`.`x` to `b`.`y`; the new wire's id.
    @discardableResult
    private func wire(_ doc: SchematicDocument, _ a: PlacedChip, _ x: String, _ b: PlacedChip, _ y: String) throws -> UUID {
        let before = Set(doc.wires.map(\.id))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(a, x)), .pin(try TestSupport.pin(b, y))), "\(x)–\(y)")
        return try XCTUnwrap(doc.wires.first { !before.contains($0.id) }?.id)
    }

    func testWireStylesAndPinLevels() throws {
        let doc = TestSupport.newDocument()
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 600))
        let gate = try TestSupport.place(doc, "SN74HC04N", at: CGPoint(x: 300, y: 300))
        let three = try TestSupport.place(doc, "3V3", at: CGPoint(x: 700, y: 60))
        let clock = try TestSupport.place(doc, "OSC-SIT1534-2012", at: CGPoint(x: 700, y: 300))
        try wire(doc, vcc, "VCC", gate, "VCC")
        try wire(doc, gnd, "GND", gate, "GND")
        let low = try wire(doc, gnd, "GND", gate, "1A")
        let high = try wire(doc, vcc, "VCC", gate, "2A")
        let floating = try wire(doc, gate, "3A", gate, "4A")
        try wire(doc, three, "VCC", clock, "VDD")
        try wire(doc, gnd, "GND", clock, "GND")
        let ticking = try wire(doc, clock, "CLK OUT", gate, "5A")

        let simulation = controller()
        XCTAssertEqual(simulation.wireStyle(low), .normal, "not playing")
        XCTAssertNil(simulation.level(ofPin: "1", on: gate.id))
        simulation.start(document: doc)
        defer { simulation.stop() }
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        spin()
        func pin(_ name: String) throws -> String { gate.part.pins[try TestSupport.pin(gate, name).index].designator }
        XCTAssertEqual(simulation.level(ofPin: try pin("1A"), on: gate.id), "0")
        XCTAssertEqual(simulation.level(ofPin: try pin("2A"), on: gate.id), "1")
        XCTAssertEqual(simulation.level(ofPin: try pin("3A"), on: gate.id), "Z")
        XCTAssertNil(simulation.level(ofPin: try pin("6Y"), on: gate.id), "a pin on no net")
        XCTAssertNil(simulation.level(ofPin: "1", on: UUID()))
        XCTAssertEqual(simulation.serialChannels(of: gate.id), 0, "a gate has no serial channel")
        XCTAssertEqual(simulation.wireStyle(low), .normal)
        XCTAssertEqual(simulation.wireStyle(high), .value(.green))
        XCTAssertEqual(simulation.wireStyle(floating), .floating)
        XCTAssertEqual(simulation.wireStyle(ticking), .toggling)
        XCTAssertEqual(simulation.wireStyle(UUID()), .normal, "no such wire")
    }

    /// An X net (a floating input's output) draws orange.
    func testAnUnknownNetIsOrange() throws {
        let doc = TestSupport.newDocument()
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 600))
        let gate = try TestSupport.place(doc, "SN74HC04N", at: CGPoint(x: 300, y: 300))
        let other = try TestSupport.place(doc, "SN74HC04N", at: CGPoint(x: 600, y: 300))
        try wire(doc, vcc, "VCC", gate, "VCC")
        try wire(doc, gnd, "GND", gate, "GND")
        try wire(doc, vcc, "VCC", other, "VCC")
        try wire(doc, gnd, "GND", other, "GND")
        let unknown = try wire(doc, gate, "1Y", other, "1A")
        let simulation = controller()
        simulation.start(document: doc)
        defer { simulation.stop() }
        spin()
        XCTAssertEqual(simulation.wireStyle(unknown), .value(.orange))
        let input = other.part.pins[try TestSupport.pin(other, "1A").index].designator
        XCTAssertEqual(simulation.level(ofPin: input, on: other.id), "X", "a floating input's output is unknown")
    }

    func testStartsTheBackendRefuses() throws {
        let simulation = controller()
        simulation.start(netlist: SchematicNetlist(partNames: ["NO-SUCH-PART"], netCount: 0, connections: [], switchesOn: [],
                                                   chipIDs: [UUID()], netOfWire: [:]))
        XCTAssertFalse(simulation.isRunning)
        XCTAssertNotNil(simulation.errorMessage)
        simulation.start(netlist: SchematicNetlist(partNames: ["5V"], netCount: 1, connections: [(part: 0, pin: "ZZ", net: 0)],
                                                   switchesOn: [], chipIDs: [UUID()], netOfWire: [:]))
        XCTAssertFalse(simulation.isRunning)
        XCTAssertNotNil(simulation.errorMessage, "a pin the part doesn't have")

        // Two supplies on one net: refused for voltage, the net drawn as a short.
        let doc = TestSupport.newDocument()
        let five = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let three = try TestSupport.place(doc, "3V3", at: CGPoint(x: 300, y: 60))
        let clash = try wire(doc, five, "VCC", three, "VCC")
        simulation.start(document: doc)
        XCTAssertFalse(simulation.isRunning)
        XCTAssertNotNil(simulation.errorMessage)
        XCTAssertEqual(simulation.wireStyle(clash), .shorted)
        simulation.stop()
        XCTAssertEqual(simulation.wireStyle(clash), .normal, "Stop clears it")
    }

    func testSerialReadsOutsidePlayMode() {
        let simulation = controller()
        let chip = UUID()
        XCTAssertEqual(simulation.serialChannels(of: chip), 0)
        XCTAssertEqual(simulation.serialSentCount(of: chip, channel: 0), 0)
        XCTAssertEqual(simulation.serialSent(of: chip, channel: 0, from: 0, count: 4), [])
        XCTAssertEqual(simulation.serialSend(to: chip, channel: 0, bytes: [65]), "The simulation is not running.")
    }

    /// A frame whose circuit time did not move keeps the lights as they were;
    /// its LCD frames and its error are taken as they come.
    func testAFrameWhoseTimeStoodStill() throws {
        let doc = TestSupport.newDocument()
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 600))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 300))
        let resistor = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 300, y: 100))
        try wire(doc, vcc, "VCC", resistor, "1")
        try wire(doc, resistor, "2", led, "A")
        try wire(doc, led, "K", gnd, "GND")
        let simulation = controller()
        simulation.start(document: doc)
        defer { simulation.stop() }
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        spin()
        let index = try XCTUnwrap(doc.chips.firstIndex { $0.id == led.id })
        func frame(_ time: UInt64, on: UInt64, error: String? = nil) -> SimulationSnapshot {
            SimulationSnapshot(wallTime: ProcessInfo.processInfo.systemUptime, circuitTime: time, netValues: [],
                               netChanges: [], lightTimes: [index: [on]],
                               lcdFrames: [index: LCDFrame(LcdState())], shortedNet: nil, error: error)
        }
        let time = simulation.circuitTime + 1_000_000
        simulation.apply(frame(time, on: 0), generation: simulation.generation)
        simulation.apply(frame(time + 1_000_000, on: 500_000), generation: simulation.generation)
        XCTAssertEqual(simulation.lights[led.id], [0.5])
        simulation.apply(frame(time + 1_000_000, on: 900_000, error: "stopped"), generation: simulation.generation)
        XCTAssertEqual(simulation.lights[led.id], [0.5], "no time passed: the last brightness stays")
        XCTAssertEqual(simulation.lcdFrames[led.id], LCDFrame(LcdState()))
        XCTAssertEqual(simulation.errorMessage, "stopped")
        let version = simulation.stateVersion
        simulation.apply(frame(time, on: 0), generation: simulation.generation + 1)
        XCTAssertEqual(simulation.stateVersion, version, "a stopped engine's frame is ignored")
    }
}
