//
//  TerminalWindow.swift
//  schematical
//
//  The terminal of a serial part (plan-12-K): what each channel transmits
//  on its TX pin, and a field that types into the channel's RX pin. The
//  transcript is the model's sent log (circuit_serial_sent), read again after
//  every simulation step; typing goes through circuit_serial_send, which
//  drives the RX pin as a real serial waveform at the channel's current baud
//  and format.
//
//  A USB-to-UART bridge is the other way round: the terminal is the PC on
//  its USB side. Typing goes out the bridge's TXD pin, the transcript is
//  what the circuit sends into its RXD pin, and the Port menus set the baud,
//  format and flow control, as a PC does (SerialPort).
//

import AppKit
import SwiftUI

/// Which terminal window to open: a serial part in one schematic window.
nonisolated struct TerminalWindowTarget: Codable, Hashable, Sendable {
    let sessionID: UUID
    let chipID: UUID
}

/// Turns sent frames into terminal text.
nonisolated enum TerminalText {
    /// Printable ASCII as text; CR LF, LF and CR alone each as one new line;
    /// any other byte as ⟨hh⟩; a frame wider than 8 bits as ⟨hhh⟩.
    static func render(_ frames: [UInt16]) -> String {
        var text = ""
        var lastWasCR = false

        for frame in frames {
            if frame > 0xFF {
                text += "⟨" + String(format: "%03X", frame) + "⟩"
                lastWasCR = false
                continue
            }
            switch frame {
            case 0x0D:
                text += "\n"
                lastWasCR = true
            case 0x0A:
                if !lastWasCR { text += "\n" }
                lastWasCR = false
            case 0x20...0x7E:
                text.append(Character(UnicodeScalar(UInt8(frame))))
                lastWasCR = false
            default:
                text += "⟨" + String(format: "%02X", frame) + "⟩"
                lastWasCR = false
            }
        }
        return text
    }

    /// The channel names: A, B, C ...
    static func channelName(_ index: Int) -> String {
        String(UnicodeScalar(UInt8(65 + index)))
    }
}

/// The frames read so far, per channel, kept across redraws.
@MainActor final class TerminalLog {
    var frames: [[UInt16]] = []
    /// Frames before this index were cleared from the view (Clear).
    var clearedAt: [Int] = []

    /// Reads the frames a channel sent since the last read.
    func refresh(_ simulation: SimulationController, chipID: UUID, channels: Int) {
        if frames.count != channels {
            frames = Array(repeating: [], count: channels)
            clearedAt = Array(repeating: 0, count: channels)
        }
        for channel in 0..<channels {
            let total = simulation.serialSentCount(of: chipID, channel: channel)
            if total < frames[channel].count {
                frames[channel] = []
                clearedAt[channel] = 0
            }
            let have = frames[channel].count
            if total > have {
                frames[channel] += simulation.serialSent(of: chipID, channel: channel, from: have,
                                                         count: total - have)
            }
        }
    }

    func text(channel: Int) -> String {
        guard channel < frames.count else { return "" }
        return TerminalText.render(Array(frames[channel].dropFirst(clearedAt[channel])))
    }

    func clear(channel: Int) {
        guard channel < frames.count else { return }
        clearedAt[channel] = frames[channel].count
    }
}

/// Root of a terminal window.
struct TerminalWindowView: View {
    let target: TerminalWindowTarget

    var body: some View {
        if let session = SchematicSessions.shared.session(target.sessionID) {
            TerminalInspector(chipID: target.chipID, document: session.document,
                              simulation: session.simulation)
        } else {
            ContentUnavailableView(
                "Schematic Closed",
                systemImage: "terminal",
                description: Text("The schematic this part belongs to is no longer open.")
            )
            .frame(minWidth: 480, minHeight: 240)
        }
    }
}

/// One serial part's terminal.
struct TerminalInspector: View {
    let chipID: UUID
    let document: SchematicDocument
    let simulation: SimulationController

    @State private var log = TerminalLog()
    @State private var channel = 0
    @State private var input = ""
    @State private var error: String?

    private var chip: PlacedChip? {
        document.chips.first { $0.id == chipID }
    }

    /// The part's channels, from its facts (the same while stopped).
    private var channels: Int {
        if case let .serial(_, count, _, _)? = chip?.part.details { return count }
        return 0
    }

    var body: some View {
        Group {
            if let chip {
                content
                    .navigationTitle("\(chip.reference) — \(chip.partNumber)")
            } else {
                ContentUnavailableView(
                    "Part Removed",
                    systemImage: "terminal",
                    description: Text("This part is no longer in the schematic.")
                )
                .navigationTitle("Terminal")
            }
        }
        .frame(minWidth: 480, minHeight: 300)
    }

    /// A USB-to-UART bridge: the PC sets its port.
    private var isUSBBridge: Bool {
        if case let .serial(_, _, _, host)? = chip?.part.details { return host == "usb" }
        return false
    }

    /// The channel's port, read from and written to the simulation.
    private var port: Binding<SerialPort> {
        Binding(
            get: { simulation.serialPort(of: chipID, channel: channel) },
            set: { error = simulation.setSerialPort($0, of: chipID, channel: channel) }
        )
    }

    /// Baud, format and flow control of a bridge's channel.
    @ViewBuilder private var portControls: some View {
        Picker("Baud", selection: port.baud) {
            ForEach(SerialPort.bauds, id: \.self) { Text(String($0)).tag($0) }
        }
        .fixedSize()
        Picker("Format", selection: Binding(
            get: { port.wrappedValue.formatName },
            set: { name in
                if let format = SerialPort.formats.first(where: { $0.formatName == name }) {
                    port.wrappedValue = port.wrappedValue.withFormat(format)
                }
            }
        )) {
            ForEach(SerialPort.formats, id: \.formatName) { Text($0.formatName).tag($0.formatName) }
        }
        .labelsHidden()
        .fixedSize()
        Toggle("RTS/CTS", isOn: port.rtsCts)
            .help("Hardware flow control: the bridge sends only while CTS is low")
    }

    /// The transcript now: refreshed in the body, so every `stateVersion`
    /// (each simulation step) redraws it. A new run's log restarts at 0,
    /// below what was read, which empties the transcript (TerminalLog.refresh).
    private var transcript: String {
        _ = simulation.stateVersion
        if simulation.isRunning {
            log.refresh(simulation, chipID: chipID, channels: channels)
        }
        return log.text(channel: channel)
    }

    @ViewBuilder private var content: some View {
        let text = transcript
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                if channels > 1 {
                    Picker("Channel", selection: $channel) {
                        ForEach(0..<channels, id: \.self) { index in
                            Text(TerminalText.channelName(index)).tag(index)
                        }
                    }
                    .pickerStyle(.segmented)
                    .labelsHidden()
                    .frame(maxWidth: 240)
                }
                if isUSBBridge { portControls }
                Spacer()
                Button("Clear") { log.clear(channel: channel) }
            }
            ScrollViewReader { proxy in
                ScrollView {
                    Text(text.isEmpty ? " " : text)
                        .font(.system(.body, design: .monospaced))
                        .textSelection(.enabled)
                        .frame(maxWidth: .infinity, alignment: .topLeading)
                        .padding(6)
                    Color.clear.frame(height: 1).id("end")
                }
                .background(Color(nsColor: .textBackgroundColor))
                .clipShape(RoundedRectangle(cornerRadius: 4))
                .onChange(of: text) { proxy.scrollTo("end", anchor: .bottom) }
            }
            HStack {
                TextField(simulation.isRunning ? "Type, then Return to send" : "Run the simulation to type",
                          text: $input)
                    .font(.system(.body, design: .monospaced))
                    .disabled(!simulation.isRunning)
                    .onSubmit { send(input, carriageReturn: true) }
            }
            if let error {
                Text(error)
                    .font(.callout)
                    .foregroundStyle(.red)
            }
        }
        .padding(12)
        .onChange(of: simulation.isRunning) { _, running in
            if running { error = nil }
        }
    }

    /// Types `text` (and a CR for Return) into the channel's RX pin (a
    /// bridge: out its TXD pin).
    func send(_ text: String, carriageReturn: Bool) {
        var bytes = Array(text.utf8)
        if carriageReturn { bytes.append(0x0D) }
        error = simulation.serialSend(to: chipID, channel: channel, bytes: bytes)
        if error == nil { input = "" }
    }
}
