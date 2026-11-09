//
//  RegisterWindow.swift
//  schematical
//
//  A read-only window showing one converter (ADC, DAC) or RTC part while
//  the simulation runs (plan-24-I): every register's current value and, for
//  an RTC, the time its clock counts. It reads the state again after every
//  simulation step, as the processor window does.
//

import AppKit
import SwiftUI

/// Which register window to open: a part in one schematic window.
nonisolated struct RegisterWindowTarget: Codable, Hashable, Sendable {
    let sessionID: UUID
    let chipID: UUID
}

/// A converter's or RTC's registers, copied out of the backend's
/// `ConverterRegister` list, and an RTC's clock (`RtcTime`).
nonisolated struct RegisterSnapshot: Equatable, Sendable {
    struct Register: Equatable, Sendable {
        let name: String
        /// The bus address; nil for an internal register.
        let address: Int?
        let space: Int
        let bits: Int
        let value: UInt32
    }

    struct Clock: Equatable, Sendable {
        let year, month, day, weekday, hour, minute, second, hundredth: Int
        /// The part counts hundredths of a second.
        let hundredths: Bool
        let running: Bool
    }

    let registers: [Register]
    /// An RTC's counters; nil for a converter.
    let clock: Clock?

    init(registers: [Register], clock: Clock?) {
        self.registers = registers
        self.clock = clock
    }

    init(_ registers: [ConverterRegister], clock: RtcTime?) {
        self.registers = registers.map { register in
            Register(name: register.name.map { String(cString: $0) } ?? "",
                     address: register.address < 0 ? nil : Int(register.address),
                     space: Int(register.space), bits: Int(register.width), value: register.value)
        }
        self.clock = clock.map { time in
            Clock(year: Int(time.year), month: Int(time.month), day: Int(time.day),
                  weekday: Int(time.weekday), hour: Int(time.hour), minute: Int(time.minute),
                  second: Int(time.second), hundredth: Int(time.hundredth),
                  hundredths: time.hundredths != 0, running: time.running != 0)
        }
    }

    /// Whether any register is outside space 0 (then the Space column shows).
    var hasSpaces: Bool { registers.contains { $0.space != 0 } }

    /// A value in hex, `bits / 4` digits (rounded up); a 1-bit register as 0 or 1.
    static func hex(_ value: UInt32, bits: Int) -> String {
        if bits <= 1 { return String(value & 1) }
        let digits = (bits + 3) / 4
        let text = String(value, radix: 16, uppercase: true)
        return String(repeating: "0", count: max(0, digits - text.count)) + text
    }

    /// A register's address in hex ("—" for an internal register).
    static func address(_ register: Register) -> String {
        guard let address = register.address else { return "—" }
        return "0x" + hex(UInt32(address), bits: address > 0xFF ? 16 : 8)
    }

    /// "2000-01-01 00:00:05 Saturday", with ".25" hundredths when counted.
    static func clockText(_ clock: Clock) -> String {
        let days = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
        let day = days.indices.contains(clock.weekday) ? days[clock.weekday] : ""
        var text = String(format: "%04d-%02d-%02d %02d:%02d:%02d", clock.year, clock.month, clock.day,
                          clock.hour, clock.minute, clock.second)
        if clock.hundredths { text += String(format: ".%02d", clock.hundredth) }
        return day.isEmpty ? text : "\(text) \(day)"
    }
}

struct RegisterWindowView: View {
    let target: RegisterWindowTarget

    var body: some View {
        if let session = SchematicSessions.shared.session(target.sessionID) {
            RegisterInspector(chipID: target.chipID, document: session.document,
                              simulation: session.simulation)
        } else {
            ContentUnavailableView(
                "Schematic Closed",
                systemImage: "list.bullet.rectangle",
                description: Text("The schematic this part belongs to is no longer open.")
            )
            .frame(minWidth: 420, minHeight: 240)
        }
    }
}

/// One converter's or RTC's registers, read again on every simulation step.
struct RegisterInspector: View {
    let chipID: UUID
    let document: SchematicDocument
    let simulation: SimulationController

    private var chip: PlacedChip? {
        document.chips.first { $0.id == chipID }
    }

    var body: some View {
        Group {
            if let chip {
                content(chip)
                    .navigationTitle("\(chip.reference) — \(chip.partNumber)")
            } else {
                ContentUnavailableView(
                    "Part Removed",
                    systemImage: "list.bullet.rectangle",
                    description: Text("This part is no longer in the schematic.")
                )
                .navigationTitle("Registers")
            }
        }
        .frame(minWidth: 420, minHeight: 320)
    }

    /// The state now: read in the body, so every new `stateVersion` redraws.
    private func snapshot(_ chip: PlacedChip) -> RegisterSnapshot? {
        _ = simulation.stateVersion
        guard simulation.isRunning else { return nil }
        return simulation.registerState(of: chip)
    }

    @ViewBuilder private func content(_ chip: PlacedChip) -> some View {
        let snapshot = snapshot(chip)
        VStack(alignment: .leading, spacing: 14) {
            Text(PartFacts.kindName(chip.part))
                .font(.title3.weight(.semibold))
            if let snapshot {
                if let clock = snapshot.clock {
                    section("Time") {
                        Text(RegisterSnapshot.clockText(clock))
                            .font(.system(.body, design: .monospaced))
                            .textSelection(.enabled)
                        if !clock.running {
                            Text("Stopped")
                                .foregroundStyle(.secondary)
                        }
                    }
                }
                section("Registers") { registerTable(snapshot) }
            } else {
                Text("Run the simulation to see the part's registers.")
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

    private func registerTable(_ snapshot: RegisterSnapshot) -> some View {
        let spaces = snapshot.hasSpaces
        return ScrollView {
            Grid(alignment: .leading, horizontalSpacing: 16, verticalSpacing: 3) {
                GridRow {
                    Text("Register")
                    Text("Address")
                    if spaces { Text("Space") }
                    Text("Value")
                }
                .foregroundStyle(.secondary)
                ForEach(Array(snapshot.registers.enumerated()), id: \.offset) { _, register in
                    GridRow {
                        Text(register.name)
                        Text(RegisterSnapshot.address(register))
                        if spaces { Text(String(register.space)) }
                        Text(RegisterSnapshot.hex(register.value, bits: register.bits))
                    }
                }
            }
            .font(.system(.body, design: .monospaced))
            .textSelection(.enabled)
            .frame(maxWidth: .infinity, alignment: .leading)
        }
    }
}
