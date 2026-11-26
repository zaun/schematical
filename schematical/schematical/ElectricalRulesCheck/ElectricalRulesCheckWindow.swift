//
//  ElectricalRulesCheckWindow.swift
//  schematical
//
//  Window › Electrical Rules Check (⌥⌘E): one schematic window's drawing
//  checked by KiCad's rules (plan-63), in two steps like the Design Rule Check.
//  Step 1 shows the rules: a severity for each and the pin-conflict map, saved
//  in the file. Step 2 runs the check, one line per violation; it reruns as
//  the drawing changes, selecting a line selects what it names, and a
//  violation can be excluded and put back. Only on the Schematic page.
//

import AppKit
import SwiftUI

/// Which schematic window's check to open.
nonisolated struct ElectricalRulesCheckTarget: Codable, Hashable, Sendable {
    let documentID: UUID
}

// MARK: - Menu command

/// Window › Design Rule Check and Window › Electrical Rules Check, as one entry of the app's
/// commands (a `CommandsBuilder` takes at most ten).
struct CheckCommands: Commands {
    var body: some Commands {
        DesignRuleCheckCommands()
        ElectricalRulesCheckCommands()
    }
}

/// Window › Electrical Rules Check (⌥⌘E), enabled on the Schematic page only.
struct ElectricalRulesCheckCommands: Commands {
    @FocusedValue(\.schematicEditor) private var editor

    var body: some Commands {
        CommandGroup(before: .windowList) {
            ElectricalRulesCheckMenuItem(editor: editor)
        }
    }
}

/// A view, so it is rendered again when the page it read changes (`EditorMenus`).
private struct ElectricalRulesCheckMenuItem: View {
    let editor: SchematicEditorModel?

    var body: some View {
        let open = editor?.page == .schematic ? editor?.menus.openElectricalRulesCheck : nil
        Button("Electrical Rules Check") { open?() }
            .keyboardShortcut("e", modifiers: [.command, .option])
            .disabled(open == nil)
    }
}

// MARK: - Window

struct ElectricalRulesCheckWindowView: View {
    let target: ElectricalRulesCheckTarget

    var body: some View {
        if let session = SchematicEditorModel.session(target.documentID) {
            ElectricalRulesCheckWizard(document: session.document)
        } else {
            ContentUnavailableView(
                "Schematic Closed",
                systemImage: "checklist",
                description: Text("The schematic this check belongs to is no longer open.")
            )
            .frame(minWidth: 520, minHeight: 240)
        }
    }
}

/// The two steps: the rules, then the results. Opens on the rules every time.
struct ElectricalRulesCheckWizard: View {
    let document: SchematicDocument
    @State private var showsResults = false

    var body: some View {
        Group {
            if showsResults {
                ElectricalRulesCheckTable(document: document) { showsResults = false }
            } else {
                ElectricalRulesCheckRules(document: document) { showsResults = true }
            }
        }
        .frame(minWidth: 620, minHeight: 280)
        .navigationTitle("Electrical Rules Check")
    }
}

// MARK: - Step 1: the rules

/// The severities and the pin map, editable; a change is an undo step of the
/// schematic and is saved with it.
struct ElectricalRulesCheckRules: View {
    let document: SchematicDocument
    let run: () -> Void

    /// The rules by the headings the list groups them under.
    static let groups: [(title: String, rules: [ErcRule])] = [
        ("Pins", [.pinNotConnected, .pinNotDriven, .powerPinNotDriven, .noConnectConnected, .noConnectDangling]),
        ("Wires", [.wireDangling, .unconnectedWireEndpoint, .fourWayJunction]),
        ("Labels and names", [.labelDangling, .isolatedPinLabel, .multipleNetNames, .similarLabels,
                              .similarLabelAndPower, .singleGlobalLabel]),
        ("Parts", [.duplicateReference]),
    ]

    var body: some View {
        Form {
            ForEach(Self.groups, id: \.title) { group in
                Section(group.title) {
                    ForEach(group.rules, id: \.self) { rule in
                        Picker(rule.title, selection: Binding(
                            get: { document.erc.severity(of: rule) },
                            set: { document.setErcSeverity(rule, $0) }
                        )) {
                            ForEach(ErcSeverity.allCases, id: \.self) { Text($0.title).tag($0) }
                        }
                        .pickerStyle(.menu)
                    }
                }
            }
            Section {
                ErcPinMapGrid(document: document)
            } header: {
                Text("Pin conflicts on a net")
            } footer: {
                Text("Where two pins of these types are on one net: green is fine, yellow a warning, red an error. "
                     + "Click a square to change it.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
        }
        .formStyle(.grouped)
        .safeAreaInset(edge: .bottom, spacing: 0) {
            VStack(spacing: 0) {
                Divider()
                HStack {
                    Button("Reset to KiCad Defaults") { document.resetErcSettings() }
                        .disabled(document.erc.severities.isEmpty && document.erc.pinMap == nil)
                        .help("KiCad's severities and pin map; the excluded violations stay")
                    Spacer()
                    Button("Run Check", action: run)
                        .keyboardShortcut(.defaultAction)
                }
                .padding(12)
            }
            .background(.bar)
        }
        .navigationSubtitle("Step 1 of 2 · rules")
    }
}

/// The 12×12 pin map: a square per pair of pin types.
struct ErcPinMapGrid: View {
    let document: SchematicDocument

    private static let short = ["In", "Out", "Bidi", "3-St", "Pass", "Free", "Unsp", "PwrIn", "PwrOut", "OC", "OE", "NC"]

    var body: some View {
        let map = document.erc.effectivePinMap
        let n = ErcPinTypes.count
        Grid(horizontalSpacing: 2, verticalSpacing: 2) {
            GridRow {
                Text("")
                ForEach(0..<n, id: \.self) { column in
                    Text(Self.short[column]).font(.caption2).foregroundStyle(.secondary).frame(width: 34)
                }
            }
            ForEach(0..<n, id: \.self) { row in
                GridRow {
                    Text(ErcPinTypes.titles[row]).font(.caption2).foregroundStyle(.secondary)
                        .frame(maxWidth: .infinity, alignment: .trailing)
                    ForEach(0..<n, id: \.self) { column in
                        cell(row, column, map[row][column])
                    }
                }
            }
        }
    }

    /// The no_connect row and column are drawn and never changed: no pair is checked against it.
    @ViewBuilder
    private func cell(_ row: Int, _ column: Int, _ level: UInt8) -> some View {
        let fixed = row == ErcPinTypes.noConnect || column == ErcPinTypes.noConnect
        Button {
            document.setErcPinMap(row, column, to: (level + 1) % 3)
        } label: {
            RoundedRectangle(cornerRadius: 3)
                .fill(Self.colour(level))
                .frame(width: 34, height: 16)
                .opacity(fixed ? 0.25 : 1)
        }
        .buttonStyle(.plain)
        .disabled(fixed)
        .help("\(ErcPinTypes.titles[row]) and \(ErcPinTypes.titles[column]): \(["fine", "a warning", "an error"][Int(level)])")
        .accessibilityLabel("\(ErcPinTypes.titles[row]) with \(ErcPinTypes.titles[column])")
        .accessibilityValue(["fine", "warning", "error"][Int(level)])
    }

    private static func colour(_ level: UInt8) -> Color {
        switch level {
        case 0: .green.opacity(0.55)
        case 1: .yellow
        default: .red
        }
    }
}

// MARK: - Step 2: the results

/// Everything the check reads, so it reruns only when one of them changes.
private struct ErcInput: Hashable {
    let chips: [PlacedChip]
    let wires: [Wire]
    let junctions: [Junction]
    let labels: [NetLabel]
    let buses: [Bus]
    let erc: ErcSettings
}

struct ElectricalRulesCheckTable: View {
    let document: SchematicDocument
    /// Goes back to the rules.
    let showRules: () -> Void
    @State private var model: ErcResultsModel

    init(document: SchematicDocument, showRules: @escaping () -> Void) {
        self.document = document
        self.showRules = showRules
        _model = State(initialValue: ErcResultsModel(document: document))
    }

    private var input: ErcInput {
        ErcInput(chips: document.chips, wires: document.wires, junctions: document.junctions,
                 labels: document.netLabels, buses: document.buses, erc: document.erc)
    }

    var body: some View {
        @Bindable var model = model
        Group {
            if !model.hasRun {
                ProgressView().frame(maxWidth: .infinity, maxHeight: .infinity)
            } else if model.rows.isEmpty {
                ContentUnavailableView(
                    model.showsExcluded ? "Nothing Excluded" : "No Violations",
                    systemImage: model.showsExcluded ? "eye" : "checkmark.seal",
                    description: Text(model.showsExcluded
                        ? "No violation is excluded."
                        : "The schematic passes every rule that is on.")
                )
            } else {
                Table(model.rows, selection: $model.selection) {
                    TableColumn("#") { Text("\($0.index)").monospacedDigit().foregroundStyle(.secondary) }
                        .width(min: 28, ideal: 36, max: 60)
                    TableColumn("Severity") { violation in
                        Label(violation.severity.title,
                              systemImage: violation.severity == .error ? "xmark.octagon.fill" : "exclamationmark.triangle.fill")
                            .foregroundStyle(violation.severity == .error ? Color.red : Color.yellow)
                    }
                    .width(min: 80, ideal: 90, max: 110)
                    TableColumn("Reference", value: \.reference)
                        .width(min: 80, ideal: 130)
                    TableColumn("Violation", value: \.message)
                        .width(min: 200, ideal: 400)
                }
                .onCopyCommand {
                    let chosen = model.selectedRows
                    let text = (chosen.isEmpty ? model.rows : chosen)
                        .map { "\($0.index)\t\($0.severity.title)\t\($0.reference)\t\($0.message)" }
                        .joined(separator: "\n")
                    return [NSItemProvider(object: text as NSString)]
                }
            }
        }
        .navigationSubtitle(summary)
        .toolbar {
            ToolbarItem(placement: .navigation) {
                Button(action: showRules) { Label("Rules", systemImage: "chevron.left") }
                    .help("Back to step 1: the rules")
            }
            ToolbarItem(placement: .principal) {
                Picker("Show", selection: $model.showsExcluded) {
                    Text("Violations").tag(false)
                    Text("Excluded (\(model.excluded.count))").tag(true)
                }
                .pickerStyle(.segmented)
            }
            ToolbarItem {
                if model.showsExcluded {
                    Button("Include") { model.includeSelected() }
                        .disabled(model.selection.isEmpty)
                        .help("Put the selected violations back")
                } else {
                    Button("Exclude") { model.excludeSelected() }
                        .disabled(model.selection.isEmpty)
                        .help("Hide the selected violations; the file remembers them")
                }
            }
        }
        .task(id: input) {
            // Wait for a drag or a run of typing to settle.
            if model.hasRun { try? await Task.sleep(for: .milliseconds(150)) }
            guard !Task.isCancelled else { return }
            model.run()
        }
        .onChange(of: model.selection) { _, chosen in model.select(chosen) }
        .onDisappear { model.clearMarkers() }
        .onChange(of: model.showsExcluded) { _, _ in model.selection = [] }
    }

    private var summary: String {
        guard model.hasRun else { return "Step 2 of 2" }
        let errors = model.errorCount, warnings = model.warningCount
        if errors + warnings == 0 { return "Step 2 of 2 · no violations" }
        return "Step 2 of 2 · \(errors) error\(errors == 1 ? "" : "s"), \(warnings) warning\(warnings == 1 ? "" : "s")"
    }
}
