//
//  DesignRuleCheckWindow.swift
//  schematical
//
//  Window › Design Rule Check (⇧⌘D): one schematic window's PCB checked
//  against its own design rules (`BoardSettings.rules`, saved in the file),
//  in two steps. Step 1 shows the rules and changes them; step 2 runs the
//  check, one line per problem. Only on the PCB page; step 2 rechecks as the
//  board changes, and selecting a line selects what it names on the PCB.
//

import AppKit
import Observation
import SwiftUI

/// Which schematic window's check to open.
nonisolated struct DesignRuleCheckTarget: Codable, Hashable, Sendable {
    let documentID: UUID
}

// MARK: - Menu command

/// Window › Design Rule Check (⇧⌘D), enabled on the PCB page only.
struct DesignRuleCheckCommands: Commands {
    @FocusedValue(\.schematicEditor) private var editor

    var body: some Commands {
        CommandGroup(before: .windowList) {
            DesignRuleCheckMenuItem(editor: editor)
        }
    }
}

/// A view, so it is rendered again when the page it read changes
/// (`EditorMenus`).
struct DesignRuleCheckMenuItem: View {
    let editor: SchematicEditorModel?

    var body: some View {
        // The check is of the PCB, so it opens from the PCB page only.
        let open = editor?.page == .pcb ? editor?.menus.openDesignRuleCheck : nil
        Button("Design Rule Check") { open?() }
            .keyboardShortcut("d", modifiers: [.command, .shift])
            .disabled(open == nil)
    }
}

// MARK: - Window

struct DesignRuleCheckWindowView: View {
    let target: DesignRuleCheckTarget

    var body: some View {
        if let session = SchematicEditorModel.session(target.documentID) {
            DesignRuleCheckWizard(document: session.document)
        } else {
            ContentUnavailableView(
                "Schematic Closed",
                systemImage: "checklist",
                description: Text("The board this check belongs to is no longer open.")
            )
            .frame(minWidth: 520, minHeight: 240)
        }
    }
}

/// The two steps: the board's rules, then the check's results. Opens on
/// the rules every time.
struct DesignRuleCheckWizard: View {
    let document: SchematicDocument
    @State private var showsResults: Bool

    init(document: SchematicDocument) {
        self.document = document
        _showsResults = State(initialValue: false)
    }

    var body: some View {
        Group {
            if showsResults {
                DesignRuleCheckTable(document: document) { showsResults = false }
            } else {
                DesignRuleCheckRules(document: document) { showsResults = true }
            }
        }
        .frame(minWidth: 560, minHeight: 240)
        .navigationTitle("Design Rule Check")
    }
}

// MARK: - Step 1: the rules

/// The board's design rules, editable. A change is an undo step of the
/// board's and is saved with it.
struct DesignRuleCheckRules: View {
    let document: SchematicDocument
    let run: () -> Void
    // Settings ▸ DRC Defaults, for Reset to App Defaults.
    @AppStorage(PreferenceKey.drcManufacturer) private var appManufacturer = DRCManufacturer.custom
    @AppStorage(PreferenceKey.drcCustomRules) private var appCustomData = Data()

    private var appRules: DesignRules { appManufacturer.fixedRules ?? DesignRules.decoded(appCustomData) }

    var body: some View {
        let board = document.board
        Form {
            Section {
                // The binding only forwards to `choose`, which the tests call.
                Picker("Manufacturer", selection: Binding(get: { board.rulesManufacturer }, set: { Self.choose($0, for: document) })) {
                    ForEach(DRCManufacturer.allCases, id: \.self) { Text($0.title).tag($0) }
                }
                .pickerStyle(.menu)
            } footer: {
                Text(footer(board))
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            DesignRuleFields(rules: board.rules, editable: true) { Self.set($0, to: $1, in: document) }
        }
        .formStyle(.grouped)
        .safeAreaInset(edge: .bottom, spacing: 0) {
            VStack(spacing: 0) {
                Divider()
                HStack {
                    Button("Reset to App Defaults") {
                        document.setDesignRules(appRules, manufacturer: appManufacturer)
                    }
                    .disabled(board.rules == appRules && board.rulesManufacturer == appManufacturer)
                    .help("Use the rules in Settings ▸ DRC Defaults")
                    Spacer()
                    Button("Run Check", action: run)
                        .keyboardShortcut(.defaultAction)
                }
                .padding(12)
            }
            .background(.bar)
        }
        .navigationSubtitle("Step 1 of 2 · \(board.rulesTitle) rules")
    }

    /// A manufacturer's rules replace the board's; Custom keeps them.
    static func choose(_ manufacturer: DRCManufacturer, for document: SchematicDocument) {
        document.setDesignRules(manufacturer.fixedRules ?? document.board.rules, manufacturer: manufacturer)
    }

    /// A typed rule (never below 0) on the board; the value the board then has.
    static func set(_ field: WritableKeyPath<DesignRules, Double>, to typed: Double, in document: SchematicDocument) -> Double {
        var next = document.board.rules
        next[keyPath: field] = max(0, typed)
        document.setDesignRules(next, manufacturer: document.board.rulesManufacturer)
        return document.board.rules[keyPath: field]
    }

    private func footer(_ board: BoardSettings) -> String {
        let changed = board.rulesManufacturer.fixedRules.map { $0 != board.rules } ?? false
        return (changed ? "\(board.rulesManufacturer.title)'s limits, with changes. " : "")
            + "All lengths are in mm. These rules are saved with this board; "
            + "new boards start from Settings ▸ DRC Defaults."
    }
}

// MARK: - Step 2: the results

/// Everything the check reads (the rules are in `board`), so it reruns only
/// when one of them changes.
struct DesignRuleCheckInput: Hashable {
    let chips: [PlacedChip]
    let wires: [Wire]
    let buses: [Bus]
    let board: BoardSettings
    let texts: [BoardText]
    let traces: [BoardTrace]
    let vias: [BoardVia]
    let holes: [BoardHole]
}

/// What step 2 shows and does, apart from its layout (plan-68-W): the
/// issues of the last check, the selected lines (selecting what they name
/// on the PCB), the copy and the subtitle.
@MainActor
@Observable
final class DesignRuleCheckResults {
    let document: SchematicDocument
    private(set) var issues: [DesignRuleIssue] = []
    private(set) var hasRun = false
    var selection: Set<DesignRuleIssue.ID> = []
    /// How long a change waits before the check runs again (a drag, a run of typing).
    @ObservationIgnored var settleDelay: Duration = .milliseconds(150)

    /// As `SchematicDocument`'s: an isolated deinit aborts the runtime.
    nonisolated deinit {}

    init(document: SchematicDocument) {
        self.document = document
    }

    var input: DesignRuleCheckInput {
        DesignRuleCheckInput(chips: document.chips, wires: document.wires, buses: document.buses,
                             board: document.board, texts: document.texts, traces: document.traces,
                             vias: document.vias, holes: document.holes)
    }

    /// The board changed (or the step opened): checks it, after the settle
    /// delay once it has run. A newer change cancels the wait.
    func inputChanged() async {
        if hasRun { try? await Task.sleep(for: settleDelay) }
        guard !Task.isCancelled else { return }
        check()
    }

    func check() {
        let found = document.designRuleCheck(rules: document.board.rules)
        // Line numbers move when the list changes.
        if found != issues { selection = [] }
        issues = found
        hasRun = true
    }

    /// Selecting lines selects what they name on the PCB.
    func selectionChanged() {
        let items = issues.filter { selection.contains($0.id) }.reduce(into: Set<BoardItem>()) { $0.formUnion($1.items) }
        if !items.isEmpty { document.selectBoardItems(items) }
    }

    /// Edit › Copy: the selected lines, else every line, one per row, tab separated.
    var copyText: String {
        let chosen = issues.filter { selection.contains($0.id) }
        return (chosen.isEmpty ? issues : chosen).map { "\($0.index)\t\($0.reference)\t\($0.issue)" }.joined(separator: "\n")
    }

    func copyItems() -> [NSItemProvider] { [NSItemProvider(object: copyText as NSString)] }

    var summary: String {
        let count = issues.count
        return "Step 2 of 2 · \(count == 0 ? "No" : String(count)) issue\(count == 1 ? "" : "s") · "
            + "\(document.board.rulesTitle) rules"
    }
}

struct DesignRuleCheckTable: View {
    @State private var model: DesignRuleCheckResults
    /// Goes back to the rules.
    let showRules: () -> Void

    init(document: SchematicDocument, showRules: @escaping () -> Void) {
        _model = State(initialValue: DesignRuleCheckResults(document: document))
        self.showRules = showRules
    }

    var body: some View {
        Group {
            if !model.hasRun {
                ProgressView()
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
            } else if model.issues.isEmpty {
                ContentUnavailableView(
                    "No Problems",
                    systemImage: "checkmark.seal",
                    description: Text("The board passes every \(model.document.board.rulesTitle) design rule, "
                                      + "and every connection is routed.")
                )
            } else {
                Table(model.issues, selection: $model.selection) {
                    TableColumn("#") { Text("\($0.index)").monospacedDigit().foregroundStyle(.secondary) }
                        .width(min: 28, ideal: 36, max: 60)
                    TableColumn("Reference", value: \.reference)
                        .width(min: 80, ideal: 170)
                    TableColumn("Issue", value: \.issue)
                        .width(min: 200, ideal: 460)
                }
                .onCopyCommand(perform: model.copyItems)
            }
        }
        .navigationSubtitle(model.summary)
        .toolbar {
            ToolbarItem(placement: .navigation) {
                Button(action: showRules) {
                    Label("Rules", systemImage: "chevron.left")
                }
                .help("Back to step 1: this board's design rules")
            }
        }
        .task(id: model.input) { await model.inputChanged() }
        .onChange(of: model.selection) { model.selectionChanged() }
    }
}
