//
//  DesignRuleCheckWindow.swift
//  schematical
//
//  Window › Design Rule Check (⇧⌘D): one schematic window's PCB checked
//  against Settings ▸ DRC Defaults, one line per problem. Only on the PCB
//  page; it rechecks as the board or the rules change, and selecting a line
//  selects what it names on the PCB.
//

import AppKit
import SwiftUI

/// Which schematic window's check to open.
nonisolated struct DesignRuleCheckTarget: Codable, Hashable, Sendable {
    let sessionID: UUID
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
        if let session = SchematicSessions.shared.session(target.sessionID) {
            DesignRuleCheckTable(document: session.document)
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

/// Everything the check reads, so it reruns only when one of them changes.
private struct DesignRuleCheckInput: Hashable {
    let chips: [PlacedChip]
    let wires: [Wire]
    let buses: [Bus]
    let board: BoardSettings
    let texts: [BoardText]
    let traces: [BoardTrace]
    let vias: [BoardVia]
    let rules: DesignRules
}

struct DesignRuleCheckTable: View {
    let document: SchematicDocument
    @AppStorage(PreferenceKey.drcManufacturer) private var manufacturer = DRCManufacturer.custom
    @AppStorage(PreferenceKey.drcCustomRules) private var customData = Data()
    @State private var issues: [DesignRuleIssue] = []
    @State private var hasRun = false
    @State private var selection: Set<DesignRuleIssue.ID> = []

    private var rules: DesignRules { manufacturer.fixedRules ?? DesignRules.decoded(customData) }

    private var input: DesignRuleCheckInput {
        DesignRuleCheckInput(chips: document.chips, wires: document.wires, buses: document.buses,
                             board: document.board, texts: document.texts, traces: document.traces,
                             vias: document.vias, rules: rules)
    }

    var body: some View {
        Group {
            if !hasRun {
                ProgressView()
            } else if issues.isEmpty {
                ContentUnavailableView(
                    "No Problems",
                    systemImage: "checkmark.seal",
                    description: Text("The board passes every \(manufacturer.title) design rule, "
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
        .frame(minWidth: 560, minHeight: 240)
        .navigationTitle("Design Rule Check")
        .navigationSubtitle(summary)
        .toolbar {
            ToolbarItem(placement: .primaryAction) {
                SettingsLink {
                    Label("Design Rules…", systemImage: "slider.horizontal.3")
                }
                .help("Change the rules in Settings ▸ DRC Defaults")
            }
        }
        .task(id: input) {
            // Wait for a drag or a run of typing to settle.
            if hasRun { try? await Task.sleep(for: .milliseconds(150)) }
            guard !Task.isCancelled else { return }
            let found = document.designRuleCheck(rules: rules)
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
        return "\(count == 0 ? "No" : String(count)) issue\(count == 1 ? "" : "s") · \(manufacturer.title) rules"
    }
}
