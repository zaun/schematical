//
//  MemoryWindow.swift
//  schematical
//
//  A window showing the contents of one memory part (SRAM) while the
//  simulation runs. It shows one page of words at a time and reads the page
//  again after every simulation step.
//

import AppKit
import SwiftUI

/// Which memory window to open: a part in one schematic window.
nonisolated struct MemoryWindowTarget: Codable, Hashable, Sendable {
    let sessionID: UUID
    let chipID: UUID
}

/// The open schematic windows, so a memory window can find the document and
/// simulation its part belongs to. Held weakly: closing the schematic window
/// frees them.
@MainActor
final class SchematicSessions {
    static let shared = SchematicSessions()

    private struct Entry {
        weak var document: SchematicDocument?
        weak var simulation: SimulationController?
    }

    private var entries: [UUID: Entry] = [:]

    func register(_ id: UUID, document: SchematicDocument, simulation: SimulationController) {
        entries = entries.filter { $0.value.document != nil }
        entries[id] = Entry(document: document, simulation: simulation)
    }

    func session(_ id: UUID) -> (document: SchematicDocument, simulation: SimulationController)? {
        guard let entry = entries[id], let document = entry.document,
              let simulation = entry.simulation else { return nil }
        return (document, simulation)
    }
}

/// Root of a memory window.
struct MemoryWindowView: View {
    let target: MemoryWindowTarget

    var body: some View {
        if let session = SchematicSessions.shared.session(target.sessionID) {
            MemoryInspector(
                chipID: target.chipID,
                document: session.document,
                simulation: session.simulation
            )
        } else {
            ContentUnavailableView(
                "Schematic Closed",
                systemImage: "memorychip",
                description: Text("The schematic this memory belongs to is no longer open.")
            )
            .frame(minWidth: 420, minHeight: 240)
        }
    }
}

/// One page of a memory part, with controls to pick the page.
private struct MemoryInspector: View {
    let chipID: UUID
    let document: SchematicDocument
    let simulation: SimulationController

    /// Words per page.
    static let pageWords = 256

    @State private var page = 0
    @State private var size: MemorySize?
    @State private var words: [MemoryWord] = []
    /// Simulator state version at which each word on the page last changed.
    @State private var changedAt: [Int: Int] = [:]
    @State private var addressText = ""

    /// Simulator steps a changed word stays highlighted (about half a second).
    private static let highlightSteps = 30

    private var chip: PlacedChip? {
        document.chips.first { $0.id == chipID }
    }

    var body: some View {
        Group {
            if let chip {
                content(for: chip)
                    .navigationTitle("\(chip.reference) · \(chip.partNumber)")
            } else {
                ContentUnavailableView(
                    "Part Removed",
                    systemImage: "memorychip",
                    description: Text("This memory is no longer in the schematic.")
                )
                .navigationTitle("Memory")
            }
        }
        .frame(minWidth: 520, minHeight: 360)
        .onAppear(perform: reload)
        .onChange(of: simulation.stateVersion) { reload() }
        .onChange(of: simulation.isRunning) { reload() }
        .onChange(of: page) {
            if let size, page < 0 || page >= pageCount(size) {
                page = min(max(page, 0), pageCount(size) - 1)
                return
            }
            changedAt = [:]
            words = []
            reload()
        }
    }

    @ViewBuilder
    private func content(for chip: PlacedChip) -> some View {
        if let size {
            VStack(spacing: 0) {
                pageBar(size: size)
                Divider()
                ScrollView([.vertical, .horizontal]) {
                    MemoryPageGrid(
                        start: page * Self.pageWords,
                        words: words,
                        size: size,
                        highlighted: highlighted
                    )
                    .equatable()
                    .padding(12)
                    .frame(maxWidth: .infinity, alignment: .topLeading)
                }
                Divider()
                footer(chip: chip, size: size)
            }
        } else {
            ContentUnavailableView {
                Label("Simulation Not Running", systemImage: "memorychip")
            } description: {
                Text("Run the simulation to see the contents of \(chip.reference). "
                     + "SRAM powers up with every bit unknown (X) and loses its contents when the simulation stops.")
            }
        }
    }

    // MARK: Page picker

    private func pageCount(_ size: MemorySize) -> Int {
        max(1, (size.words + Self.pageWords - 1) / Self.pageWords)
    }

    private func pageBar(size: MemorySize) -> some View {
        let count = pageCount(size)
        let first = page * Self.pageWords
        let last = min(first + Self.pageWords, size.words) - 1

        return HStack(spacing: 10) {
            Button {
                page = max(0, page - 1)
            } label: {
                Label("Previous Page", systemImage: "chevron.left")
            }
            .labelStyle(.iconOnly)
            .disabled(page == 0)
            .help("Previous page")

            Button {
                page = min(count - 1, page + 1)
            } label: {
                Label("Next Page", systemImage: "chevron.right")
            }
            .labelStyle(.iconOnly)
            .disabled(page >= count - 1)
            .help("Next page")

            Text("Page")
            TextField("Page", value: $page, format: .number.grouping(.never))
                .textFieldStyle(.roundedBorder)
                .multilineTextAlignment(.trailing)
                .frame(width: 64)
                .help("Type a page number (0 to \(count - 1)) and press Return")

            Text("of \(count)")
                .foregroundStyle(.secondary)

            Text("\(hex(first, digits: addressDigits(size)))–\(hex(last, digits: addressDigits(size)))")
                .font(.system(.body, design: .monospaced))
                .foregroundStyle(.secondary)

            Spacer()

            TextField("Go to address (hex)", text: $addressText)
                .textFieldStyle(.roundedBorder)
                .font(.system(.body, design: .monospaced))
                .frame(width: 170)
                .onSubmit { goToAddress(size: size) }
                .help("Type a word address in hex and press Return")
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
    }

    private func goToAddress(size: MemorySize) {
        var text = addressText.trimmingCharacters(in: .whitespaces)
        if text.lowercased().hasPrefix("0x") {
            text.removeFirst(2)
        }
        guard let address = Int(text, radix: 16), address >= 0, address < size.words else {
            NSSound.beep()
            return
        }
        page = address / Self.pageWords
    }

    private func footer(chip: PlacedChip, size: MemorySize) -> some View {
        let known = words.filter { $0.known == wordMask(size.bits) }.count
        return HStack {
            Text("\(size.words) words × \(size.bits) bits (\(chip.part.function))")
            Spacer()
            Text("\(known) of \(words.count) words on this page defined")
        }
        .font(.caption)
        .foregroundStyle(.secondary)
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
    }

    // MARK: Reading

    private var highlighted: Set<Int> {
        Set(changedAt.compactMap { index, version in
            simulation.stateVersion - version < Self.highlightSteps ? index : nil
        })
    }

    /// Reads the page again. State is only assigned when it changed, so the
    /// grid redraws only when the memory does.
    private func reload() {
        let newSize = simulation.memorySize(of: chipID)
        if newSize != size {
            size = newSize
        }
        guard let newSize else {
            if !words.isEmpty { words = [] }
            if !changedAt.isEmpty { changedAt = [:] }
            return
        }
        if page >= pageCount(newSize) {
            page = 0
        }

        let fresh = simulation.readMemory(of: chipID, start: page * Self.pageWords, count: Self.pageWords)
        guard fresh != words else {
            expireHighlights()
            return
        }
        if words.count == fresh.count {
            var updated = changedAt
            for index in fresh.indices where fresh[index] != words[index] {
                updated[index] = simulation.stateVersion
            }
            changedAt = updated
        }
        words = fresh
        expireHighlights()
    }

    private func expireHighlights() {
        let live = changedAt.filter { simulation.stateVersion - $0.value < Self.highlightSteps }
        if live.count != changedAt.count {
            changedAt = live
        }
    }
}

/// The words of one page: an address column, the words in hex (X for any
/// unknown bit in a digit), and for 8-bit memories the bytes as text.
private struct MemoryPageGrid: View, Equatable {
    let start: Int
    let words: [MemoryWord]
    let size: MemorySize
    let highlighted: Set<Int>

    private var wordsPerRow: Int {
        switch size.bits {
        case ...8: 16
        case ...16: 8
        default: 4
        }
    }

    var body: some View {
        let perRow = wordsPerRow
        let rows = (words.count + perRow - 1) / perRow

        Grid(alignment: .leading, horizontalSpacing: 6, verticalSpacing: 3) {
            ForEach(0..<rows, id: \.self) { row in
                GridRow {
                    Text(hex(start + row * perRow, digits: addressDigits(size)))
                        .foregroundStyle(.secondary)
                        .padding(.trailing, 8)

                    ForEach(row * perRow..<min((row + 1) * perRow, words.count), id: \.self) { index in
                        wordCell(index)
                    }

                    if size.bits <= 8 {
                        Text(ascii(row: row, perRow: perRow))
                            .foregroundStyle(.secondary)
                            .padding(.leading, 10)
                    }
                }
            }
        }
        .font(.system(size: 12, design: .monospaced))
        .textSelection(.enabled)
    }

    private func wordCell(_ index: Int) -> some View {
        let word = words[index]
        let defined = word.known == wordMask(size.bits)

        return Text(wordText(word))
            .foregroundStyle(defined ? Color.primary : Color.secondary.opacity(0.7))
            .padding(.horizontal, 2)
            .background(
                highlighted.contains(index) ? Color.accentColor.opacity(0.3) : Color.clear,
                in: RoundedRectangle(cornerRadius: 3)
            )
            .help("\(hex(start + index, digits: addressDigits(size))): \(binary(word))")
    }

    private func wordText(_ word: MemoryWord) -> String {
        let digits = (size.bits + 3) / 4
        let present = wordMask(size.bits)
        var text = ""
        for nibble in (0..<digits).reversed() {
            let shift = UInt32(nibble * 4)
            let bits: UInt32 = (0xF << shift) & present
            if word.known & bits != bits {
                text += "X"
            } else {
                text += String((word.value >> shift) & 0xF, radix: 16, uppercase: true)
            }
        }
        return text
    }

    private func binary(_ word: MemoryWord) -> String {
        String((0..<size.bits).reversed().map { bit in
            let b = UInt32(1) << UInt32(bit)
            return word.known & b == 0 ? "X" : (word.value & b != 0 ? "1" : "0")
        })
    }

    private func ascii(row: Int, perRow: Int) -> String {
        String((row * perRow..<min((row + 1) * perRow, words.count)).map { index in
            let word = words[index]
            guard word.known & 0xFF == 0xFF, (0x20..<0x7F).contains(word.value & 0xFF),
                  let scalar = Unicode.Scalar(word.value & 0xFF) else { return "." }
            return Character(scalar)
        })
    }
}

// MARK: - Formatting

/// Hex digits needed for any word address of this memory.
private func addressDigits(_ size: MemorySize) -> Int {
    max(1, String(max(size.words - 1, 0), radix: 16).count)
}

private func hex(_ value: Int, digits: Int) -> String {
    let text = String(value, radix: 16, uppercase: true)
    return String(repeating: "0", count: max(0, digits - text.count)) + text
}

/// Bits that exist in a word of the given width.
private func wordMask(_ bits: Int) -> UInt32 {
    bits >= 32 ? .max : (UInt32(1) << UInt32(bits)) - 1
}
