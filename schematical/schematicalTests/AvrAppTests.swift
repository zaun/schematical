import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-62-H: a simulated microcontroller in the app. Its firmware is the
/// part's "load file on start", set and saved like an EEPROM's and loaded
/// into its flash at every start; its Processor window shows the AVR's
/// registers, the SREG flags and the last instruction.
@MainActor
final class AvrAppTests: XCTestCase {
    /// Kept for the life of the test process, like `TestSupport`'s documents:
    /// freeing one inside a test method aborts the host.
    private static var keptSimulations: [SimulationController] = []

    /// `sbi DDRB,5; sbi PORTB,5; loop: rjmp loop`: PB5 high for good
    /// (`oracle.js asm`, backend/tools/avr8js-oracle).
    private static let ledOnHex = ":06000000259A2D9AFFCFA6\n:00000001FF\n"
    /// `loop: rjmp loop`: no pin is driven.
    private static let ledOffHex = ":02000000FFCF30\n:00000001FF\n"

    private func firmware(_ name: String, _ hex: String) throws -> MemoryFile {
        let folder = FileManager.default.temporaryDirectory
            .appendingPathComponent("AvrAppTests-\(ProcessInfo.processInfo.processIdentifier)")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        let url = folder.appendingPathComponent(name)
        try Data(hex.utf8).write(to: url)
        return MemoryFile(url: url, bookmark: nil)
    }

    private func wire(_ doc: SchematicDocument, _ a: PlacedChip, _ an: String, _ b: PlacedChip, _ bn: String,
                      file: StaticString = #filePath, line: UInt = #line) throws {
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(a, an)), .pin(try TestSupport.pin(b, bn))),
                     "\(an) to \(bn)", file: file, line: line)
    }

    /// 5V and GND on an ATMEGA328P-PU, and an LED from PB5 to ground.
    private func board() throws -> (doc: SchematicDocument, mcu: PlacedChip, led: PlacedChip) {
        let doc = TestSupport.newDocument()
        let supply = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 600))
        let mcu = try TestSupport.place(doc, "ATMEGA328P-PU", at: CGPoint(x: 400, y: 300))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 800, y: 300))
        try wire(doc, supply, "VCC", mcu, "VCC")
        try wire(doc, supply, "VCC", mcu, "AVCC")
        try wire(doc, ground, "GND", mcu, "GND")
        try wire(doc, mcu, "PB5", led, "A")
        try wire(doc, led, "K", ground, "GND")
        return (doc, mcu, led)
    }

    private func simulation() -> SimulationController {
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        return simulation
    }

    private func lit(_ simulation: SimulationController, _ id: UUID) -> Bool {
        (simulation.lights[id]?.first ?? 0) > 0.5
    }

    private func settle() { RunLoop.main.run(until: Date().addingTimeInterval(0.8)) }

    // MARK: - The load file

    func testPartCapabilities() throws {
        let mega = try TestSupport.part("ATMEGA328P-PU")
        XCTAssertTrue(mega.isSimulatedMcu)
        XCTAssertTrue(mega.hasLoadFile)
        XCTAssertTrue(mega.hasProcessorWindow)
        XCTAssertEqual(mega.loadFileSize, MemorySize(words: 32768, bits: 8), "the flash, in bytes")
        XCTAssertFalse(mega.isMemory, "no memory window for a microcontroller")

        let tiny = try TestSupport.part("ATTINY85-20PU")
        XCTAssertFalse(tiny.isSimulatedMcu)
        XCTAssertFalse(tiny.hasLoadFile)
        XCTAssertFalse(tiny.hasProcessorWindow)
        XCTAssertNil(tiny.loadFileSize)

        let cpu = try TestSupport.part("W65C02S6TPG-14")
        XCTAssertTrue(cpu.hasProcessorWindow)
        XCTAssertFalse(cpu.hasLoadFile)

        let eeprom = try TestSupport.part("AT28C256-15PU")
        XCTAssertTrue(eeprom.hasLoadFile)
        XCTAssertEqual(eeprom.loadFileSize, eeprom.memorySize)
        XCTAssertFalse(eeprom.hasProcessorWindow)

        XCTAssertFalse(try TestSupport.part("SN74HC00N").hasLoadFile)
    }

    func testFirmwareRuns() throws {
        let (doc, mcu, led) = try board()
        doc.setMemoryFile(mcu.id, try firmware("led_on.hex", Self.ledOnHex))
        let simulation = simulation()
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        settle()
        XCTAssertTrue(lit(simulation, led.id), "led_on drives PB5 high: the LED is lit")
        simulation.stop()

        doc.setMemoryFile(mcu.id, try firmware("led_off.hex", Self.ledOffHex))
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        settle()
        XCTAssertFalse(lit(simulation, led.id), "led_off drives nothing: the LED is dark")
        simulation.stop()
    }

    func testNoFirmwareStillRuns() throws {
        let (doc, _, led) = try board()
        let simulation = simulation()
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        settle()
        XCTAssertFalse(lit(simulation, led.id), "an erased chip drives nothing")
        XCTAssertNil(simulation.errorMessage)
        simulation.stop()
    }

    func testUnreadableFirmwareStopsTheStart() throws {
        let (doc, mcu, _) = try board()
        let missing = FileManager.default.temporaryDirectory
            .appendingPathComponent("AvrAppTests-no-such-file-\(UUID().uuidString).hex")
        doc.setMemoryFile(mcu.id, MemoryFile(url: missing, bookmark: nil))
        let simulation = simulation()
        simulation.start(document: doc)
        XCTAssertFalse(simulation.isRunning)
        let message = try XCTUnwrap(simulation.errorMessage)
        XCTAssertTrue(message.hasPrefix("\(mcu.reference): "), message)
    }

    func testFirmwareTooBigStopsTheStart() throws {
        let (doc, mcu, _) = try board()
        // One data byte at 0x8000, the first address past 32 KB of flash
        // (an extended linear address record is not needed below 64 KB).
        doc.setMemoryFile(mcu.id, try firmware("too_big.hex", ":01800000007F\n:00000001FF\n"))
        let simulation = simulation()
        simulation.start(document: doc)
        XCTAssertFalse(simulation.isRunning)
        XCTAssertTrue((simulation.errorMessage ?? "").hasPrefix("\(mcu.reference): "), simulation.errorMessage ?? "")
    }

    func testLoadFileUndo() throws {
        let (doc, mcu, _) = try board()
        let file = MemoryFile(url: URL(fileURLWithPath: "/tmp/firmware.hex"), bookmark: nil)
        doc.setMemoryFile(mcu.id, file)
        XCTAssertEqual(doc.chips.first { $0.id == mcu.id }?.memoryFile, file)
        XCTAssertEqual(doc.undoName, "Change Load File")
        XCTAssertTrue(doc.undo())
        XCTAssertNil(doc.chips.first { $0.id == mcu.id }?.memoryFile, "undo clears it")
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.chips.first { $0.id == mcu.id }?.memoryFile, file, "redo sets it again")

        // A microcontroller with no simulation model takes no load file.
        let tiny = try TestSupport.place(doc, "ATTINY85-20PU", at: CGPoint(x: 400, y: 700))
        doc.setMemoryFile(tiny.id, file)
        XCTAssertNil(doc.chips.first { $0.id == tiny.id }?.memoryFile)
    }

    func testLoadFileRoundTrip() throws {
        let (doc, mcu, _) = try board()
        doc.setMemoryFile(mcu.id, MemoryFile(url: URL(fileURLWithPath: "/tmp/firmware/blink.hex"), bookmark: nil))
        let opened = try BoardLayersTests.roundTrip(doc)
        let reopened = try XCTUnwrap(opened.chips.first { $0.partNumber == "ATMEGA328P-PU" })
        XCTAssertEqual(reopened.memoryFile?.url.path, "/tmp/firmware/blink.hex")
    }

    // MARK: - The processor window

    func testProcessorState() throws {
        let (doc, mcu, _) = try board()
        doc.setMemoryFile(mcu.id, try firmware("led_on.hex", Self.ledOnHex))
        let simulation = simulation()
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        settle()
        let state = try XCTUnwrap(simulation.cpuState(of: mcu.id))
        XCTAssertEqual(state.status, .running)
        XCTAssertGreaterThan(state.cycles, 0)
        XCTAssertEqual(state.registers.count, 35)
        XCTAssertEqual(state.registers.prefix(4).map(\.name), ["PC", "SP", "SREG", "R0"])
        XCTAssertEqual(state.registers.last?.name, "R31")
        XCTAssertEqual(state.flags.map(\.label), ["I", "T", "H", "S", "V", "N", "Z", "C"])
        XCTAssertEqual(state.flags.first?.set, false, "led_on never enables interrupts")
        XCTAssertTrue(state.instructionText.hasSuffix("RJMP .-2"), state.instructionText)
        XCTAssertTrue(state.instructionText.hasPrefix("0004  FF CF"), state.instructionText)
        simulation.stop()
        XCTAssertNil(simulation.cpuState(of: mcu.id), "no state while stopped")
    }

    /// The processor window shows a firmware problem before Run does.
    func testLoadProblem() throws {
        let (doc, mcu, _) = try board()
        func problem() -> String? { CpuInspector.loadProblem(of: doc.chips.first { $0.id == mcu.id }) }
        XCTAssertNil(problem(), "no file chosen")
        doc.setMemoryFile(mcu.id, try firmware("led_on.hex", Self.ledOnHex))
        XCTAssertNil(problem(), "a file that reads and fits")
        doc.setMemoryFile(mcu.id, try firmware("too_big.hex", ":01800000007F\n:00000001FF\n"))
        XCTAssertNotNil(problem(), "a byte past the end of the flash")
        doc.setMemoryFile(mcu.id, MemoryFile(url: URL(fileURLWithPath: "/tmp/AvrAppTests-missing.hex"), bookmark: nil))
        XCTAssertNotNil(problem(), "a file that is not there")
        XCTAssertNil(CpuInspector.loadProblem(of: nil))
    }

    func testSregFlags() {
        // I, H and C set: 0b1010_0001.
        let snapshot = CpuSnapshot(
            status: .running, cycles: 1,
            registers: [.init(name: "PC", value: 0, bits: 16), .init(name: "SREG", value: 0xA1, bits: 8)],
            instruction: nil)
        XCTAssertEqual(snapshot.flags.map(\.label), ["I", "T", "H", "S", "V", "N", "Z", "C"])
        XCTAssertEqual(snapshot.flags.map(\.set), [true, false, true, false, false, false, false, true])
        // The other processors keep their own flags.
        let p = CpuSnapshot(status: .running, cycles: 1,
                            registers: [.init(name: "P", value: 0x80, bits: 8)], instruction: nil)
        XCTAssertEqual(p.flags.first?.label, "N")
    }
}
