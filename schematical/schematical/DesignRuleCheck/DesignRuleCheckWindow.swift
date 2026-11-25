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
import SwiftUI

/// Which schematic window's check to open.
nonisolated struct DesignRuleCheckTarget: Codable, Hashable, Sendable {
    let documentID: UUID
}

// MARK: - Menu command

extension FocusedValues {
    /// Opens the focused window's Design Rule Check; nil off the PCB page.
    @Entry var openDesignRuleCheck: (() -> Void)?
}

/// Window › Design Rule Check (⇧⌘D), enabled on the PCB page only.
struct DesignRuleCheckCommands: Commands {
    @FocusedValue(\.openDesignRuleCheck) private var open

    var body: some Commands {
        CommandGroup(before: .windowList) {
            Button("Design Rule Check") { open?() }
                .keyboardShortcut("d", modifiers: [.command, .shift])
                .disabled(open == nil)
        }
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
    @State private var showsResults = false

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
                Picker("Manufacturer", selection: Binding(
                    get: { board.rulesManufacturer },
                    // A manufacturer's rules replace the board's; Custom keeps them.
                    set: { document.setDesignRules($0.fixedRules ?? document.board.rules, manufacturer: $0) }
                )) {
                    ForEach(DRCManufacturer.allCases, id: \.self) { Text($0.title).tag($0) }
                }
                .pickerStyle(.menu)
            } footer: {
                Text(footer(board))
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            DesignRuleFields(rules: board.rules, editable: true) { field, typed in
                var next = document.board.rules
                next[keyPath: field] = max(0, typed)
                document.setDesignRules(next, manufacturer: document.board.rulesManufacturer)
                return document.board.rules[keyPath: field]
            }
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
private struct DesignRuleCheckInput: Hashable {
    let chips: [PlacedChip]
    let wires: [Wire]
    let buses: [Bus]
    let board: BoardSettings
    let texts: [BoardText]
    let traces: [BoardTrace]
    let vias: [BoardVia]
    let holes: [BoardHole]
}

struct DesignRuleCheckTable: View {
    let document: SchematicDocument
    /// Goes back to the rules.
    let showRules: () -> Void
    @State private var issues: [DesignRuleIssue] = []
    @State private var hasRun = false
    @State private var selection: Set<DesignRuleIssue.ID> = []

    private var input: DesignRuleCheckInput {
        DesignRuleCheckInput(chips: document.chips, wires: document.wires, buses: document.buses,
                             board: document.board, texts: document.texts, traces: document.traces,
                             vias: document.vias, holes: document.holes)
    }

    var body: some View {
        Group {
            if !hasRun {
                ProgressView()
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
            } else if issues.isEmpty {
                ContentUnavailableView(
                    "No Problems",
                    systemImage: "checkmark.seal",
                    description: Text("The board passes every \(document.board.rulesTitle) design rule, "
                                      + "and every connection is routed.")
                )
            } else {
                Table(issues, selection: $selection) {
                    TableColumn("#") { Text("\($0.index)").monospacedDigit().foregroundStyle(.secondary) }
                        .width(min: 28, ideal: 36, max: 60)
                    TableColumn("Reference", value: \.reference)
                        .width(min: 80, ideal: 170)
                    TableColumn("Issue", value: \.issue)
                        .width(min: 200, ideal: 460)
                }
                .onCopyCommand {
                    let chosen = issues.filter { selection.contains($0.id) }
                    let text = (chosen.isEmpty ? issues : chosen)
                        .map { "\($0.index)\t\($0.reference)\t\($0.issue)" }
                        .joined(separator: "\n")
                    return [NSItemProvider(object: text as NSString)]
                }
            }
        }
        .navigationSubtitle(summary)
        .toolbar {
            ToolbarItem(placement: .navigation) {
                Button(action: showRules) {
                    Label("Rules", systemImage: "chevron.left")
                }
                .help("Back to step 1: this board's design rules")
            }
        }
        .task(id: input) {
            // Wait for a drag or a run of typing to settle.
            if hasRun { try? await Task.sleep(for: .milliseconds(150)) }
            guard !Task.isCancelled else { return }
            let found = document.designRuleCheck(rules: document.board.rules)
            // Line numbers move when the list changes.
            if found != issues { selection = [] }
            issues = found
            hasRun = true
        }
        .onChange(of: selection) { _, chosen in
            let items = issues.filter { chosen.contains($0.id) }.reduce(into: Set<BoardItem>()) { $0.formUnion($1.items) }
            if !items.isEmpty { document.selectBoardItems(items) }
        }
    }

    private var summary: String {
        let count = issues.count
        return "Step 2 of 2 · \(count == 0 ? "No" : String(count)) issue\(count == 1 ? "" : "s") · "
            + "\(document.board.rulesTitle) rules"
    }
}
