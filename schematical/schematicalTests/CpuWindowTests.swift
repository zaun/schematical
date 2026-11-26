import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-X: the processor window (CpuWindow.swift): its snapshot of the
/// backend's `CpuState`, and what each state of the window shows.
@MainActor
final class CpuWindowTests: XCTestCase {
    private static var keptSimulations: [SimulationController] = []
    private var probes: [ViewProbe] = []
    private var files: [URL] = []

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
        files.forEach { try? FileManager.default.removeItem(at: $0) }
        files = []
        try await super.tearDown()
    }

    private func probe<V: View>(_ view: V) -> ViewProbe {
        let probe = ViewProbe(view)
        probes.append(probe)
        return probe
    }

    private func simulation() -> SimulationController {
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        return simulation
    }

    /// A firmware file this test writes (removed in tearDown).
    private func firmware(_ hex: String) throws -> MemoryFile {
        let url = FileManager.default.temporaryDirectory
            .appendingPathComponent("CpuWindowTests-\(UUID().uuidString).hex")
        try Data(hex.utf8).write(to: url)
        files.append(url)
        return MemoryFile(url: url, bookmark: nil)
    }

    /// An ATtiny85 with 5V and GND: a simulated microcontroller with a load file.
    private func tiny() throws -> (doc: SchematicDocument, mcu: PlacedChip) {
        let doc = TestSupport.newDocument()
        let supply = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 600))
        let mcu = try TestSupport.place(doc, "ATTINY85-20PU", at: CGPoint(x: 400, y: 300))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(supply, "VCC")), .pin(try TestSupport.pin(mcu, "VCC"))))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(ground, "GND")), .pin(try TestSupport.pin(mcu, "GND"))))
        return (doc, mcu)
    }

    // MARK: The snapshot

    /// A `CpuState` as the backend fills it.
    private func state(_ status: CpuStatus, registers: [(String, UInt32, UInt32)] = [],
                       instruction: (address: UInt32, bytes: [UInt8], text: String)? = nil) -> CpuState {
        var state = CpuState()
        state.status = status
        state.cycles = 42
        state.register_count = registers.count
        withUnsafeMutableBytes(of: &state.registers) { raw in
            let all = raw.bindMemory(to: CpuRegister.self)
            for (index, register) in registers.enumerated() {
                all[index] = CpuRegister(name: UnsafePointer(strdup(register.0)), value: register.1,
                                         bits: register.2)
            }
        }
        if let instruction {
            state.has_instruction = 1
            state.instruction_address = instruction.address
            state.instruction_length = instruction.bytes.count
            withUnsafeMutableBytes(of: &state.instruction_bytes) { raw in
                for (index, byte) in instruction.bytes.enumerated() { raw[index] = byte }
            }
            withUnsafeMutableBytes(of: &state.instruction_text) { raw in
                for (index, byte) in instruction.text.utf8.enumerated() { raw[index] = byte }
            }
        }
        return state
    }

    func testEveryStatusAndItsLine() {
        let cases: [(CpuStatus, CpuSnapshot.Status, String)] = [
            (CPU_RUNNING, .running, "Running · 42 cycles"),
            (CPU_LOST, .lost, "Unknown — waiting for reset"),
            (CPU_RESET, .reset, "Held in reset"),
            (CPU_WAITING, .waiting, "Waiting for interrupt (WAI)"),
            (CPU_STOPPED, .stopped, "Stopped (STP)"),
            (CPU_HALTED, .halted, "Halted (HALT)"),
            (CPU_JAMMED, .jammed, "Jammed — reset to continue"),
            (CPU_BUS_RELEASED, .busReleased, "Bus released (BUSACK)"),
        ]
        for (raw, status, line) in cases {
            let snapshot = CpuSnapshot(state(raw))
            XCTAssertEqual(snapshot.status, status)
            XCTAssertEqual(snapshot.statusText, line)
            XCTAssertEqual(snapshot.cycles, 42)
            XCTAssertNil(snapshot.instruction)
            XCTAssertEqual(snapshot.instructionText, "—")
        }
    }

    func testRegistersAndTheLastInstruction() {
        let snapshot = CpuSnapshot(state(CPU_RUNNING, registers: [("A", 0x41, 8), ("PC", 0xC004, 16), ("E", 1, 1)],
                                         instruction: (0xC004, [0xA0, 0x07], "LDY #$07")))
        XCTAssertEqual(snapshot.registers, [.init(name: "A", value: 0x41, bits: 8),
                                            .init(name: "PC", value: 0xC004, bits: 16),
                                            .init(name: "E", value: 1, bits: 1)])
        XCTAssertEqual(snapshot.instruction, .init(address: 0xC004, bytes: [0xA0, 0x07], text: "LDY #$07"))
        XCTAssertEqual(snapshot.instructionText, "C004  A0 07  LDY #$07")
        XCTAssertEqual(CpuSnapshot.hex(snapshot.registers[0]), "41")
        XCTAssertEqual(CpuSnapshot.hex(snapshot.registers[2]), "1")
        XCTAssertTrue(snapshot.flags.isEmpty, "no flag register")

        let wide = CpuSnapshot(state(CPU_RUNNING, registers: [("PBR", 0, 8)],
                                     instruction: (0x008029, [0x80, 0xFE], "BRA $8029")))
        XCTAssertEqual(wide.instructionText, "008029  80 FE  BRA $8029", "a program bank shows 6 digits")
    }

    func testFlagLabelsPerProcessor() {
        func flags(_ registers: [CpuSnapshot.Register]) -> [String] {
            CpuSnapshot(status: .running, cycles: 0, registers: registers, instruction: nil)
                .flags.map { "\($0.label)\($0.set ? "1" : "0")" }
        }
        XCTAssertEqual(flags([.init(name: "P", value: 0x81, bits: 8), .init(name: "E", value: 0, bits: 1)]),
                       ["N1", "V0", "M0", "X0", "D0", "I0", "Z0", "C1"], "65816 native")
        XCTAssertEqual(flags([.init(name: "P", value: 0x30, bits: 8), .init(name: "E", value: 1, bits: 1)]),
                       ["N0", "V0", "–1", "B1", "D0", "I0", "Z0", "C0"], "65816 emulation")
        XCTAssertEqual(flags([.init(name: "AF", value: 0x1241, bits: 16)]),
                       ["S0", "Z1", "Y0", "H0", "X0", "P/V0", "N0", "C1"], "Z80: the low byte of AF")
    }

    // MARK: The views

    private static let running = CpuSnapshot(
        status: .running, cycles: 9,
        registers: [.init(name: "PC", value: 0x1234, bits: 16), .init(name: "SREG", value: 0x80, bits: 8)],
        instruction: .init(address: 4, bytes: [0xFF, 0xCF], text: "RJMP .-2"))

    func testStateViewShowsRegistersFlagsAndInstruction() {
        let probe = probe(CpuStateView(snapshot: Self.running))
        XCTAssertTrue(probe.has("Running · 9 cycles"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Registers"))
        XCTAssertTrue(probe.has("1234"))
        XCTAssertTrue(probe.has("Flags"))
        XCTAssertTrue(probe.has("SREG"))
        XCTAssertTrue(probe.has("Last instruction"))
        XCTAssertTrue(probe.has("0004  FF CF  RJMP .-2"))
    }

    func testStateViewWithoutRegistersOrFlags() {
        let lost = probe(CpuStateView(snapshot: CpuSnapshot(status: .lost, cycles: 0, registers: [], instruction: nil)))
        XCTAssertTrue(lost.has("Unknown — waiting for reset"), "\(lost.labels())")
        XCTAssertFalse(lost.has("Registers"))
        XCTAssertTrue(lost.has("Last instruction"))
        let plain = probe(CpuStateView(snapshot: CpuSnapshot(
            status: .running, cycles: 1, registers: [.init(name: "A", value: 1, bits: 8)], instruction: nil)))
        XCTAssertTrue(plain.has("Registers"), "\(plain.labels())")
        XCTAssertFalse(plain.has("Flags"), "no flag register, no Flags row")
        let stopped = probe(CpuStateView(snapshot: nil))
        XCTAssertTrue(stopped.has("Run the simulation to see the processor's state."))
    }

    func testWindowOfAClosedSchematic() {
        let probe = probe(CpuWindowView(target: CpuWindowTarget(documentID: UUID(), chipID: UUID())))
        XCTAssertTrue(probe.has("Schematic Closed"), "\(probe.labels())")
    }

    /// An open schematic: the processor's state while it runs, then the part removed.
    func testWindowFollowsTheRunAndThePart() throws {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let open = TestSupport.openSystemDocument(of: doc, catalog: TestSupport.catalog, until: self)
        let cpu = try TestSupport.place(doc, "W65C02S6TPG-14", at: CGPoint(x: 300, y: 300))
        let probe = probe(CpuWindowView(target: CpuWindowTarget(documentID: open.model.id, chipID: cpu.id)))
        XCTAssertTrue(probe.has("Run the simulation to see the processor's state."), "\(probe.labels())")
        XCTAssertFalse(probe.has("Load file on start:"), "a cpu-kind part has no firmware bar")
        open.model.simulation.start(document: doc)
        XCTAssertTrue(open.model.simulation.isRunning, open.model.simulation.errorMessage ?? "")
        probe.settle()
        XCTAssertTrue(probe.has("Last instruction"), "\(probe.labels())")
        open.model.simulation.stop()
        probe.settle()
        XCTAssertTrue(doc.undo(), "undo the placement")
        probe.settle()
        XCTAssertTrue(probe.has("Part Removed"), "\(probe.labels())")
    }

    /// A microcontroller's firmware bar: Choose… (answered by the test),
    /// a file that won't load, and Clear.
    func testFirmwareBar() throws {
        let (doc, mcu) = try tiny()
        let tooBig = try firmware(":01800000007F\n:00000001FF\n")
        let good = try firmware(":06000000B89AC09AFFCF80\n:00000001FF\n")
        var asked = 0
        var inspector = CpuInspector(chipID: mcu.id, document: doc, simulation: simulation())
        inspector.chooseFile = { _ in
            asked += 1
            return tooBig
        }
        let probe = probe(inspector)
        XCTAssertTrue(probe.has("Load file on start:"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Run the simulation to see the processor's state."))

        try probe.press("Choose…")
        probe.settle()
        XCTAssertEqual(asked, 1)
        XCTAssertEqual(doc.chips.first { $0.id == mcu.id }?.memoryFile, tooBig)
        XCTAssertTrue(probe.has("Can't Load File"), "a file past the flash shows before Run: \(probe.labels())")

        doc.setMemoryFile(mcu.id, good)
        probe.settle()
        XCTAssertFalse(probe.has("Can't Load File"), "\(probe.labels())")

        try probe.press("Clear")
        XCTAssertNil(doc.chips.first { $0.id == mcu.id }?.memoryFile)
    }

    /// The load problem is checked again when a run stops: the file went
    /// bad on disk during the run (nothing else changed).
    func testLoadProblemIsCheckedWhenARunStops() throws {
        let (doc, mcu) = try tiny()
        let file = try firmware(":06000000B89AC09AFFCF80\n:00000001FF\n")
        doc.setMemoryFile(mcu.id, file)
        let simulation = simulation()
        let probe = probe(CpuInspector(chipID: mcu.id, document: doc, simulation: simulation))
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        probe.settle()
        XCTAssertTrue(probe.has("Running"), "\(probe.labels())")
        try Data(":01800000007F\n:00000001FF\n".utf8).write(to: file.url)
        probe.settle()
        XCTAssertFalse(probe.has("Can't Load File"), "not checked while running")
        simulation.stop()
        probe.settle()
        XCTAssertTrue(probe.has("Can't Load File"), "\(probe.labels())")
    }
}
