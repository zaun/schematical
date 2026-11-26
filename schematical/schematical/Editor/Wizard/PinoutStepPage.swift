import AppKit
import SwiftUI

/// Step 4 of the Part Wizard, "Pins" (plan-46-H): paste the datasheet's pin
/// table, check which column is which, and apply it; the pin table below is
/// the form's own, for review and for parts with no table to copy.
struct PinoutStepPage: View {
    @Bindable var model: PartEditorModel
    /// Where Paste reads from; tests give their own.
    let pasteboard: NSPasteboard
    @State private var pasted: PastedPinTable
    @State private var confirmReplace: Bool
    @State private var pinCount: Int

    // The state's first values are set here: xccov never counts a `@State` default (plan-68-AF).
    init(model: PartEditorModel, pasteboard: NSPasteboard = .general) {
        self.model = model
        self.pasteboard = pasteboard
        _pasted = State(initialValue: PastedPinTable())
        _confirmReplace = State(initialValue: false)
        _pinCount = State(initialValue: 8)
    }

    var body: some View {
        Section {
            TextEditor(text: Binding(get: { pasted.text }, set: { pasted.paste($0) }))
                .font(.system(.body, design: .monospaced))
                .frame(minHeight: 90, maxHeight: 140)
                .accessibilityLabel("Pasted pin table")
            HStack {
                Button("Paste") {
                    if let text = pasteboard.string(forType: .string) { pasted.paste(text) }
                }
                Button("Clear") { pasted.paste("") }
                    .disabled(pasted.text.isEmpty)
            }
        } header: {
            Text("Paste the pin table from the datasheet")
        } footer: {
            Text("Select the rows of the pin table in the datasheet (in the pane on the right, or any PDF viewer), "
                + "copy them and paste them here. One row per pin: its number or ball, its name and, if the table "
                + "has one, its type (I, O, I/O, power …).")
        }

        if !pasted.table.rows.isEmpty {
            Section("Check the columns") {
                let result = pasted.result
                ScrollView(.horizontal) {
                    Grid(alignment: .leading, horizontalSpacing: 10, verticalSpacing: 4) {
                        GridRow {
                            ForEach(pasted.columns.indices, id: \.self) { index in
                                Picker("Column \(index + 1)", selection: $pasted.columns[index]) {
                                    ForEach(PinColumn.allCases, id: \.self) { Text($0.title).tag($0) }
                                }
                                .labelsHidden()
                                .frame(minWidth: 110)
                            }
                        }
                        ForEach(Array(pasted.table.rows.prefix(PastedPinTable.previewRows).enumerated()),
                                id: \.offset) { _, row in
                            GridRow {
                                ForEach(pasted.columns.indices, id: \.self) { index in
                                    Text(row.indices.contains(index) ? row[index] : "")
                                        .lineLimit(1)
                                        .foregroundStyle(pasted.columns[index] == .ignore ? .secondary : .primary)
                                }
                            }
                        }
                    }
                }
                if pasted.hiddenRows > 0 {
                    Text("… and \(pasted.hiddenRows) more rows").font(.caption).foregroundStyle(.secondary)
                }
                HStack {
                    Text(Self.summary(pins: result.pins.count, skipped: result.skipped))
                    Spacer()
                    Button("Apply") {
                        if model.draft.pinCount > 0 { confirmReplace = true } else { apply() }
                    }
                    .disabled(result.pins.isEmpty)
                }
                if result.pins.isEmpty {
                    Text("Choose which column holds the pin number (or ball) and which the name.")
                        .font(.callout).foregroundStyle(.orange)
                } else {
                    Text("Roles and active-low marks are guessed from the names and the type column. "
                        + "Check them in the table below after applying.")
                        .font(.caption).foregroundStyle(.secondary)
                }
            }
            .alert("Replace the \(model.draft.pinCount) pins the part has?", isPresented: $confirmReplace) {
                Button("Replace") { apply() }
                Button("Cancel", role: .cancel) {}
            } message: {
                Text("The pasted table's pins take their place.")
            }
        }

        if model.draft.pinCount == 0 {
            Section("No table to copy?") {
                HStack {
                    Stepper("Start with \(pinCount) numbered pins", value: $pinCount, in: 1...400)
                    Button("Start") { model.startWithNumberedPins(pinCount) }
                }
            }
        }

        PinoutsSection(pinouts: $model.draft.pinouts, partPinout: $model.draft.pinout,
                       commitName: { pinout, pin, name in model.commitPinName(pinout: pinout, pin: pin, to: name) },
                       status: model.renameStatus)
    }

    private func apply() {
        model.applyPins(pasted.result.pins, style: pasted.style)
    }

    /// "14 pins, 1 row skipped".
    static func summary(pins: Int, skipped: Int) -> String {
        "\(pins) \(pins == 1 ? "pin" : "pins"), \(skipped) \(skipped == 1 ? "row" : "rows") skipped"
    }
}

/// The Pins step's pasted text and what is read from it (plan-68-AF): the
/// table, the guessed column of each cell (the user can change them), and
/// the pins they give.
struct PastedPinTable: Equatable {
    /// How many rows of the pasted table the grid shows.
    static let previewRows = 10

    private(set) var text = ""
    private(set) var table = PinTable(rows: [])
    var columns: [PinColumn] = []

    /// New text: parsed again, its columns guessed again. The same text
    /// keeps the columns the user chose.
    mutating func paste(_ text: String) {
        guard text != self.text else { return }
        self.text = text
        table = PinTableParser.parse(text)
        columns = PinTableParser.guessColumns(table)
    }

    var result: (pins: [PinDraft], skipped: Int) { PinTableParser.pins(table, columns: columns) }
    var style: PinoutDraft.Style { PinTableParser.style(columns) }
    /// The rows past the grid's.
    var hiddenRows: Int { max(0, table.rows.count - Self.previewRows) }
}

/// A pin's name in the pin table. The edit is committed on Return and when
/// the field loses focus, never per keystroke: a rename is carried into the
/// kind's settings (plan-46-D § 3.3), and "A" → "AB" typed letter by letter
/// must not capture another pin's references on the way.
struct PinNameField: View {
    let name: String
    let commit: (String) -> Void
    @State private var text: String
    @FocusState private var focused: Bool

    init(name: String, commit: @escaping (String) -> Void) {
        self.name = name
        self.commit = commit
        _text = State(initialValue: "")
    }

    var body: some View {
        TextField("", text: $text)
            .focused($focused)
            .onSubmit { commitIfChanged() }
            .onChange(of: focused) { _, now in if !now { commitIfChanged() } }
            .onChange(of: name) { _, new in if new != text { text = new } }
            .onAppear { text = name }
            .onDisappear { commitIfChanged() }
    }

    private func commitIfChanged() {
        if text != name { commit(text) }
    }
}

extension PartEditorModel {
    /// Commits a pin's new name: when the pin is in the part's pinout, the
    /// kind's settings that name it are renamed with it (`renamePin`), then
    /// the pin table is. Returns how many settings were rewritten.
    @discardableResult
    func commitPinName(pinout pinoutID: UUID, pin pinID: UUID, to new: String) -> Int {
        guard let p = draft.pinouts.firstIndex(where: { $0.id == pinoutID }),
              let i = draft.pinouts[p].pins.firstIndex(where: { $0.id == pinID }) else { return 0 }
        let old = draft.pinouts[p].pins[i].name
        guard old != new else { return 0 }
        var next = draft
        let count = next.pinouts[p].name == next.pinout ? next.renamePin(from: old, to: new) : 0
        next.pinouts[p].pins[i].name = new
        draft = next
        renameStatus = count == 0 ? nil : "Renamed in \(count) \(count == 1 ? "setting" : "settings")"
        return count
    }

    /// Replaces the pins of the part's pinout with a pasted table's.
    func applyPins(_ pins: [PinDraft], style: PinoutDraft.Style) {
        var next = draft
        if let index = next.pinouts.firstIndex(where: { $0.name == next.pinout }) {
            next.pinouts[index].pins = pins
            next.pinouts[index].style = style
        } else {
            let name = next.pinout.isEmpty ? "default" : next.pinout
            next.pinouts.append(PinoutDraft(name: name, style: style, pins: pins))
            next.pinout = name
        }
        draft = next
        renameStatus = nil
    }

    /// N numbered pins with no names yet, for a part with no table to copy.
    func startWithNumberedPins(_ count: Int) {
        applyPins((1...max(1, count)).map { PinDraft(designator: String($0), name: "", role: "input") },
                  style: .numbered)
    }
}
