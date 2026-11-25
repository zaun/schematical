//
//  CpuWindow.swift
//  schematical
//
//  A window showing one processor part while the simulation runs: a cpu
//  kind part, or a simulated microcontroller (plan-62-H). It shows the
//  status, registers, flags, the last instruction completed and the cycle
//  count, and reads the state again after every simulation step, as the
//  memory window does. A microcontroller's window also has its firmware:
//  the "Load file on start" bar, as an EEPROM's memory window has.
//

import AppKit
import SwiftUI

/// Which processor window to open: a part in one schematic window.
nonisolated struct CpuWindowTarget: Codable, Hashable, Sendable {
    let documentID: UUID
    let chipID: UUID
}

/// A processor's state, copied out of the backend's `CpuState`.
nonisolated struct CpuSnapshot: Equatable, Sendable {
    enum Status: Equatable, Sendable {
        case running, lost, reset, waiting, stopped, halted, jammed, busReleased
    }

    struct Register: Equatable, Sendable {
        let name: String
        let value: UInt32
        let bits: Int
    }

    struct Instruction: Equatable, Sendable {
        let address: UInt32
        let bytes: [UInt8]
        let text: String
    }

    let status: Status
    let cycles: UInt64
    /// In the backend's order; empty while lost or in reset.
    let registers: [Register]
    /// The last completed instruction, nil before the first.
    let instruction: Instruction?

    init(status: Status, cycles: UInt64, registers: [Register], instruction: Instruction?) {
        self.status = status
        self.cycles = cycles
        self.registers = registers
        self.instruction = instruction
    }

    init(_ state: CpuState) {
        switch state.status {
        case CPU_RUNNING: status = .running
        case CPU_RESET: status = .reset
        case CPU_WAITING: status = .waiting
        case CPU_STOPPED: status = .stopped
        case CPU_HALTED: status = .halted
        case CPU_JAMMED: status = .jammed
        case CPU_BUS_RELEASED: status = .busReleased
        default: status = .lost
        }
        cycles = state.cycles
        let count = min(Int(state.register_count), Int(CPU_MAX_REGISTERS))
        registers = withUnsafeBytes(of: state.registers) { raw in
            let all = raw.bindMemory(to: CpuRegister.self)
            return (0..<count).map { index in
                let register = all[index]
                return Register(name: register.name.map { String(cString: $0) } ?? "",
                                value: register.value, bits: Int(register.bits))
            }
        }
        if state.has_instruction != 0 {
            let length = min(Int(state.instruction_length), 4)
            let bytes = withUnsafeBytes(of: state.instruction_bytes) { Array($0.prefix(length)) }
            let text = withUnsafeBytes(of: state.instruction_text) { raw -> String in
                let chars = raw.bindMemory(to: CChar.self)
                let end = chars.firstIndex(of: 0) ?? chars.count
                return String(decoding: raw.prefix(end), as: UTF8.self)
            }
            instruction = Instruction(address: state.instruction_address, bytes: bytes, text: text)
        } else {
            instruction = nil
        }
    }

    /// The status line.
    var statusText: String {
        switch status {
        case .running: return "Running · \(cycles) cycles"
        case .lost: return "Unknown — waiting for reset"
        case .reset: return "Held in reset"
        case .waiting: return "Waiting for interrupt (WAI)"
        case .stopped: return "Stopped (STP)"
        case .halted: return "Halted (HALT)"
        case .jammed: return "Jammed — reset to continue"
        case .busReleased: return "Bus released (BUSACK)"
        }
    }

    func register(_ name: String) -> Register? {
        registers.first { $0.name == name }
    }

    /// A register's value in hex, `bits / 4` digits (rounded up); a 1-bit
    /// register as 0 or 1.
    static func hex(_ register: Register) -> String {
        if register.bits <= 1 { return String(register.value & 1) }
        let digits = (register.bits + 3) / 4
        let text = String(register.value, radix: 16, uppercase: true)
        return String(repeating: "0", count: max(0, digits - text.count)) + text
    }

    /// The flag cells: (label, set), most significant bit first. The 6502
    /// family's P, the low byte of the Z80's AF, or an AVR's SREG (`I T H S
    /// V N Z C`); empty otherwise. The 65816 (it has an E register) labels P
    /// by mode: native `N V M X D I Z C`, emulation as the 65C02 (bit 5
    /// reads 1, bit 4 is B).
    var flags: [(label: String, set: Bool)] {
        let labels: [String]
        let value: UInt32
        if let p = register("P") {
            if let e = register("E"), e.value & 1 == 0 {
                labels = ["N", "V", "M", "X", "D", "I", "Z", "C"]
            } else {
                labels = ["N", "V", "–", "B", "D", "I", "Z", "C"]
            }
            value = p.value
        } else if let af = register("AF") {
            labels = ["S", "Z", "Y", "H", "X", "P/V", "N", "C"]
            value = af.value & 0xFF
        } else if let sreg = register("SREG") {
            labels = ["I", "T", "H", "S", "V", "N", "Z", "C"]
            value = sreg.value
        } else {
            return []
        }
        return labels.enumerated().map { index, label in
            (label, value & (1 << UInt32(7 - index)) != 0)
        }
    }

    /// "C004  A0 07  LDY #$07", or "—". A processor with a program bank
    /// (the 65816's PBR) shows 6 digits: "008029  80 FE  BRA $8029".
    var instructionText: String {
        guard let instruction else { return "—" }
        let address = String(format: register("PBR") != nil ? "%06X" : "%04X", instruction.address)
        let bytes = instruction.bytes.map { String(format: "%02X", $0) }.joined(separator: " ")
        return "\(address)  \(bytes)  \(instruction.text)"
    }
}

/// Root of a processor window.
struct CpuWindowView: View {
    let target: CpuWindowTarget

    var body: some View {
        if let session = SchematicEditorModel.session(target.documentID) {
            CpuInspector(chipID: target.chipID, document: session.document,
                         simulation: session.simulation)
        } else {
            ContentUnavailableView(
                "Schematic Closed",
                systemImage: "cpu",
                description: Text("The schematic this processor belongs to is no longer open.")
            )
            .frame(minWidth: 420, minHeight: 240)
        }
    }
}

/// One processor's state, read again on every simulation step.
struct CpuInspector: View {
    let chipID: UUID
    let document: SchematicDocument
    let simulation: SimulationController

    /// The window, for the firmware's Choose… panel.
    @State private var window: NSWindow?
    /// Why a microcontroller's firmware can't be loaded, checked while the
    /// simulation is stopped; nil when it can (or there is none).
    @State private var loadError: String?

    private var chip: PlacedChip? {
        document.chips.first { $0.id == chipID }
    }

    var body: some View {
        Group {
            if let chip {
                VStack(spacing: 0) {
                    if chip.part.hasLoadFile {
                        // A microcontroller's firmware (plan-62-H).
                        LoadFileBar(
                            file: chip.memoryFile,
                            isRunning: simulation.isRunning,
                            choose: {
                                Task {
                                    if let file = await MemoryFileAccess.choose(attachedTo: window) {
                                        document.setMemoryFile(chipID, file)
                                    }
                                }
                            },
                            clear: { document.setMemoryFile(chipID, nil) }
                        )
                        Divider()
                    }
                    if chip.part.hasLoadFile, !simulation.isRunning, let loadError {
                        ContentUnavailableView(
                            "Can't Load File",
                            systemImage: "exclamationmark.triangle",
                            description: Text(loadError + " Choose the file again.")
                        )
                        .frame(maxHeight: .infinity)
                    } else {
                        content
                    }
                }
                .navigationTitle("\(chip.reference) — \(chip.partNumber)")
            } else {
                ContentUnavailableView(
                    "Part Removed",
                    systemImage: "cpu",
                    description: Text("This processor is no longer in the schematic.")
                )
                .navigationTitle("Processor")
            }
        }
        .frame(minWidth: 420, minHeight: 360)
        .background(WindowReader { window = $0 })
        .onAppear { refreshLoadFile() }
        .onChange(of: simulation.isRunning) {
            if !simulation.isRunning {
                refreshLoadFile()
            }
        }
        .onChange(of: chip?.memoryFile) { refreshLoadFile() }
    }

    /// Parses the firmware again (as the start will), to show a problem
    /// before Run does.
    private func refreshLoadFile() {
        loadError = CpuInspector.loadProblem(of: chip)
    }

    /// Why a part's load file can't be loaded, or nil: no such part, a part
    /// with no load file, no file chosen, or a file that reads and fits.
    static func loadProblem(of chip: PlacedChip?) -> String? {
        guard let chip, chip.part.hasLoadFile, let size = chip.part.loadFileSize,
              chip.memoryFile != nil else { return nil }
        do {
            _ = try MemoryFileAccess.image(chip.memoryFile, size: size)
            return nil
        } catch {
            return error.localizedDescription
        }
    }

    /// The state now: read in the body, so every new `stateVersion` redraws.
    private var snapshot: CpuSnapshot? {
        _ = simulation.stateVersion
        guard simulation.isRunning else { return nil }
        return simulation.cpuState(of: chipID)
    }

    @ViewBuilder private var content: some View {
        let snapshot = snapshot
        VStack(alignment: .leading, spacing: 14) {
            if let snapshot {
                Text(snapshot.statusText)
                    .font(.title3.weight(.semibold))
                if !snapshot.registers.isEmpty {
                    section("Registers") { registerGrid(snapshot.registers) }
                    let flags = snapshot.flags
                    if !flags.isEmpty {
                        section("Flags") { flagRow(flags) }
                    }
                }
                section("Last instruction") {
                    Text(snapshot.instructionText)
                        .font(.system(.body, design: .monospaced))
                        .textSelection(.enabled)
                }
            } else {
                Text("Run the simulation to see the processor's state.")
                    .foregroundStyle(.secondary)
            }
            Spacer(minLength: 0)
        }
        .padding(16)
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
    }

    private func section<Content: View>(_ title: String, @ViewBuilder _ body: () -> Content) -> some View {
        VStack(alignment: .leading, spacing: 6) {
            Text(title)
                .font(.headline)
                .foregroundStyle(.secondary)
            body()
        }
    }

    private func registerGrid(_ registers: [CpuSnapshot.Register]) -> some View {
        let columns = [GridItem(.adaptive(minimum: 110), alignment: .leading)]
        return LazyVGrid(columns: columns, alignment: .leading, spacing: 4) {
            ForEach(registers, id: \.name) { register in
                HStack(spacing: 6) {
                    Text(register.name)
                        .foregroundStyle(.secondary)
                        .frame(minWidth: 34, alignment: .trailing)
                    Text(CpuSnapshot.hex(register))
                }
                .font(.system(.body, design: .monospaced))
            }
        }
    }

    private func flagRow(_ flags: [(label: String, set: Bool)]) -> some View {
        HStack(spacing: 4) {
            ForEach(Array(flags.enumerated()), id: \.offset) { _, flag in
                Text(flag.label)
                    .font(.system(.callout, design: .monospaced).weight(.semibold))
                    .frame(minWidth: 26)
                    .padding(.vertical, 3)
                    .padding(.horizontal, 4)
                    .background(
                        RoundedRectangle(cornerRadius: 4)
                            .fill(flag.set ? Color.accentColor : Color.secondary.opacity(0.15))
                    )
                    .foregroundStyle(flag.set ? Color.white : Color.secondary)
            }
        }
    }
}
