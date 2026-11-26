//
//  MemoryWindow.swift
//  schematical
//
//  A window showing the contents of one memory part (SRAM, EEPROM) while
//  the simulation runs. It shows one page of words at a time and reads the
//  page again after every simulation step. For an EEPROM it also holds the
//  "Load file on start" setting and, while stopped, previews the contents
//  the chip will start with.
//

import AppKit
import SwiftUI

/// Which memory window to open: a part in one schematic window.
nonisolated struct MemoryWindowTarget: Codable, Hashable, Sendable {
    let documentID: UUID
    let chipID: UUID
}

/// Root of a memory window.
struct MemoryWindowView: View {
    let target: MemoryWindowTarget

    var body: some View {
        if let session = SchematicEditorModel.session(target.documentID) {
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

/// One page of a memory part, with controls to pick the page
/// (`MemoryInspectorModel` holds what it shows).
struct MemoryInspector: View {
    @State private var model: MemoryInspectorModel
    @State private var window: NSWindow?
    /// The simulation, observed for its steps and its run.
    private let simulation: MemorySource

    init(chipID: UUID, document: SchematicDocument, simulation: MemorySource) {
        _model = State(initialValue: MemoryInspectorModel(chipID: chipID, document: document, source: simulation))
        self.simulation = simulation
    }

    /// A test's model (its panel and beep answered).
    init(model: MemoryInspectorModel) {
        _model = State(initialValue: model)
        simulation = model.source
    }

    var body: some View {
        Group {
            if let chip = model.chip {
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
        .background(WindowReader { window = $0 })
        .onAppear { model.appeared() }
        .onChange(of: simulation.stateVersion) { model.stepped() }
        .onChange(of: simulation.isRunning) { model.runningChanged() }
        .onChange(of: model.chip?.memoryFile) { model.fileChanged() }
        .onChange(of: model.page) { model.pageChanged() }
    }

    @ViewBuilder
    private func content(for chip: PlacedChip) -> some View {
        if model.isEEPROM {
            VStack(spacing: 0) {
                LoadFileBar(
                    file: chip.memoryFile,
                    isRunning: simulation.isRunning,
                    choose: { Task { await model.chooseLoadFile(window: window) } },
                    clear: { model.clearLoadFile() }
                )
                Divider()
                if model.showsStartContents, let loadError = model.loadError {
                    ContentUnavailableView(
                        "Can't Load File",
                        systemImage: "exclamationmark.triangle",
                        description: Text(loadError + " Choose the file again.")
                    )
                    .frame(maxHeight: .infinity)
                } else {
                    pageContent(for: chip)
                }
            }
        } else {
            pageContent(for: chip)
        }
    }

    @ViewBuilder
    private func pageContent(for chip: PlacedChip) -> some View {
        if let size = model.size {
            VStack(spacing: 0) {
                pageBar(size: size)
                Divider()
                ScrollView([.vertical, .horizontal]) {
                    MemoryPageGrid(start: model.start, words: model.words, size: size, highlighted: model.highlighted)
                        .equatable()
                        .padding(12)
                        .frame(maxWidth: .infinity, alignment: .topLeading)
                }
                Divider()
                let footer = model.footer(size, function: chip.part.function)
                HStack {
                    Text(footer.left)
                    Spacer()
                    Text(footer.right)
                }
                .font(.caption)
                .foregroundStyle(.secondary)
                .padding(.horizontal, 12)
                .padding(.vertical, 6)
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

    private func pageBar(size: MemorySize) -> some View {
        let count = MemoryInspectorModel.pageCount(size)
        return HStack(spacing: 10) {
            Button { model.previousPage() } label: {
                Label("Previous Page", systemImage: "chevron.left")
            }
            .labelStyle(.iconOnly)
            .disabled(model.page == 0)
            .help("Previous page")

            Button { model.nextPage(size) } label: {
                Label("Next Page", systemImage: "chevron.right")
            }
            .labelStyle(.iconOnly)
            .disabled(model.page >= count - 1)
            .help("Next page")

            Text("Page")
            TextField("Page", value: $model.page, format: .number.grouping(.never))
                .textFieldStyle(.roundedBorder)
                .multilineTextAlignment(.trailing)
                .frame(width: 64)
                .help("Type a page number (0 to \(count - 1)) and press Return")

            Text("of \(count)")
                .foregroundStyle(.secondary)

            Text(model.rangeText(size))
                .font(.system(.body, design: .monospaced))
                .foregroundStyle(.secondary)

            Spacer()

            TextField("Go to address (hex)", text: $model.addressText)
                .textFieldStyle(.roundedBorder)
                .font(.system(.body, design: .monospaced))
                .frame(width: 170)
                .onSubmit { model.goToAddress() }
                .help("Type a word address in hex and press Return")
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
    }
}

/// The words of one page: an address column, the words in hex (X for any
/// unknown bit in a digit), and for 8-bit memories the bytes as text.
private struct MemoryPageGrid: View, Equatable {
    let start: Int
    let words: [MemoryWord]
    let size: MemorySize
    let highlighted: Set<Int>

    var body: some View {
        let perRow = MemoryFormat.wordsPerRow(bits: size.bits)
        let rows = (words.count + perRow - 1) / perRow
        let digits = MemoryFormat.addressDigits(size)

        Grid(alignment: .leading, horizontalSpacing: 6, verticalSpacing: 3) {
            ForEach(0..<rows, id: \.self) { row in
                let indices = row * perRow..<min((row + 1) * perRow, words.count)
                GridRow {
                    Text(MemoryFormat.hex(start + row * perRow, digits: digits))
                        .foregroundStyle(.secondary)
                        .padding(.trailing, 8)
                    ForEach(indices, id: \.self) { index in wordCell(index, digits: digits) }
                    if size.bits <= 8 {
                        Text(MemoryFormat.ascii(words[indices]))
                            .foregroundStyle(.secondary)
                            .padding(.leading, 10)
                    }
                }
            }
        }
        .font(.system(size: 12, design: .monospaced))
        .textSelection(.enabled)
    }

    private func wordCell(_ index: Int, digits: Int) -> some View {
        let word = words[index]
        let defined = word.known == MemoryFormat.wordMask(size.bits)
        return Text(MemoryFormat.wordText(word, bits: size.bits))
            .foregroundStyle(defined ? Color.primary : Color.secondary.opacity(0.7))
            .padding(.horizontal, 2)
            .background(highlighted.contains(index) ? Color.accentColor.opacity(0.3) : Color.clear,
                        in: RoundedRectangle(cornerRadius: 3))
            .help("\(MemoryFormat.hex(start + index, digits: digits)): \(MemoryFormat.binary(word, bits: size.bits))")
    }
}
