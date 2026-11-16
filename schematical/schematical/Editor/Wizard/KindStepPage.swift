import SwiftUI

/// Step 1 of the Part Wizard, "What are you making?" (plan-46-F): start from
/// a similar library part, make a part with no simulation, or pick a kind
/// from the grouped list, each in plain words with example parts.
struct KindStepPage: View {
    @Bindable var model: PartEditorModel
    @State private var query = ""
    /// The kind waiting for the "settings will be cleared" answer.
    @State private var pendingKind: String?

    var body: some View {
        Section {
            TextField("Search the library", text: $query, prompt: Text("A part number, such as 74HC00"))
            let matches = model.similarParts(matching: query)
            if !query.trimmingCharacters(in: .whitespaces).isEmpty, matches.isEmpty {
                Text("No library part matches.").foregroundStyle(.secondary)
            }
            ForEach(matches, id: \.name) { part in
                Button {
                    model.chooseSimilar(named: part.name)
                } label: {
                    HStack {
                        Text(Self.similarRow(part)).lineLimit(1)
                        Spacer()
                        Image(systemName: "doc.on.doc").foregroundStyle(.secondary)
                    }
                    .contentShape(Rectangle())
                }
                .buttonStyle(.plain)
                .help("Start from a copy of \(part.name)")
            }
        } header: {
            Text("Start from a similar part")
        } footer: {
            Text("Choosing a part copies it: everything is filled in, and you change what differs.")
        }

        Section("Or make a part that is not simulated") {
            kindRow(PartKinds.info["generic"], title: "Just a symbol and footprint (no simulation)")
        }

        ForEach(KindGroup.allCases.filter { $0 != .other }, id: \.self) { group in
            Section(group.rawValue) {
                ForEach(PartKinds.kinds(in: group), id: \.kind) { info in
                    kindRow(info, title: info.name)
                }
            }
        }
        .alert("Change to \(pendingName)?", isPresented: Binding(
            get: { pendingKind != nil },
            set: { if !$0 { pendingKind = nil } })) {
            Button("Change") {
                if let kind = pendingKind { model.chooseKind(kind) }
                pendingKind = nil
            }
            Button("Cancel", role: .cancel) { pendingKind = nil }
        } message: {
            Text(Self.clearMessage(from: model.draft.kind))
        }
    }

    private var pendingName: String {
        pendingKind.map { PartDraft.displayName(of: $0) } ?? ""
    }

    @ViewBuilder private func kindRow(_ info: KindInfo?, title: String) -> some View {
        if let info {
            Button {
                if model.kindChangeClearsSettings(to: info.kind) {
                    pendingKind = info.kind
                } else {
                    model.chooseKind(info.kind)
                }
            } label: {
                HStack(alignment: .top) {
                    VStack(alignment: .leading, spacing: 2) {
                        Text(title).font(.headline)
                        Text(info.blurb).font(.callout).foregroundStyle(.secondary)
                            .fixedSize(horizontal: false, vertical: true)
                        if !info.examples.isEmpty {
                            Text("In the library: " + info.examples.joined(separator: ", "))
                                .font(.caption).foregroundStyle(.tertiary)
                        }
                    }
                    Spacer()
                    if model.kindChosen, model.draft.kind == info.kind {
                        Image(systemName: "checkmark").foregroundStyle(.tint)
                            .accessibilityLabel("Chosen")
                    }
                }
                .contentShape(Rectangle())
            }
            .buttonStyle(.plain)
            .accessibilityIdentifier("kind-\(info.kind)")
        }
    }

    /// "SN74HC00N — NAND, DIP, Texas Instruments".
    static func similarRow(_ part: CatalogPart) -> String {
        let facts = [part.function, part.package, part.manufacturer].filter { !$0.isEmpty }
        return facts.isEmpty ? part.name : "\(part.name) — \(facts.joined(separator: ", "))"
    }

    /// "The logic gate's own settings will be cleared. …"
    static func clearMessage(from kind: String) -> String {
        "The \(PartDraft.displayName(of: kind).lowercased())'s own settings will be cleared. "
            + "Pins, name, datasheet and footprint are kept."
    }
}
