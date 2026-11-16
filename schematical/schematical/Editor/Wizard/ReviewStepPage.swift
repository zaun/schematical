import SwiftUI

/// The Part Wizard's last step, "Review" (plan-46-K): what is still to do,
/// the backend's verdict with a way to the step that has the problem, the
/// part as the schematic and the PCB will draw it, every pin with the pad it
/// lands on, and Save Part… / Add to Library.
struct ReviewStepPage: View {
    @Bindable var model: PartEditorModel

    var body: some View {
        let todo = Self.todo(model)
        if !todo.isEmpty {
            Section("Still to do") {
                ForEach(todo, id: \.self) { item in
                    HStack {
                        Label(item.issue, systemImage: "exclamationmark.circle").foregroundStyle(.orange)
                        Spacer()
                        Button("Go to \(item.title)") { model.go(to: item.stepID) }
                    }
                }
            }
        }

        Section {
            switch model.validation {
            case nil:
                Text("Checking…").foregroundStyle(.secondary)
            case let .success(part)?:
                Label("The part loads: \(part.pins.count) pins", systemImage: "checkmark.circle")
                    .foregroundStyle(.green)
            case let .failure(failure)?:
                Label(failure.message, systemImage: "exclamationmark.triangle")
                    .foregroundStyle(.red)
                    .textSelection(.enabled)
                if let target = Self.target(of: failure.message, in: model) {
                    Button("Go to \(target.title)") { model.go(to: target.id) }
                        .help("The step that holds what this message is about")
                }
            }
            if let status = model.status {
                Text(status).font(.caption).foregroundStyle(.secondary)
            }
            HStack {
                Button("Save Part…") { Task { await model.save() } }
                    .disabled(!model.isValid)
                Button("Add to Library") { Task { await model.addToLibraryFromMenu() } }
                    .disabled(!model.isValid)
                if !model.isValid {
                    Text("Both are available once the part loads.").font(.caption).foregroundStyle(.secondary)
                }
            }
        } header: {
            Text("The part")
        }

        Section("Preview") {
            if let part = model.lastValidPart {
                HStack(alignment: .top, spacing: 12) {
                    VStack(alignment: .leading) {
                        Text("Block").font(.caption).foregroundStyle(.secondary)
                        BlockPreviewView(chip: PartPreview.chip(for: part), revision: model.previewRevision)
                            .frame(minHeight: 220)
                    }
                    VStack(alignment: .leading) {
                        Text("Footprint").font(.caption).foregroundStyle(.secondary)
                        FootprintPreviewView(part: part, revision: model.previewRevision)
                            .frame(minHeight: 220)
                    }
                }
                if !model.isValid {
                    Text("Showing the part as it last loaded.").font(.caption).foregroundStyle(.secondary)
                }
            } else {
                Text("The previews show the part once it loads.").foregroundStyle(.secondary)
            }
        }

        let rows = Self.pinRows(draft: model.draft, part: model.isValid ? model.lastValidPart : nil)
        Section("Pins and pads") {
            if rows.isEmpty {
                Text("The part has no pins yet.").foregroundStyle(.secondary)
            } else {
                Grid(alignment: .leading, horizontalSpacing: 16, verticalSpacing: 3) {
                    GridRow {
                        Text("Pin").font(.caption).foregroundStyle(.secondary)
                        Text("Name").font(.caption).foregroundStyle(.secondary)
                        Text("Role").font(.caption).foregroundStyle(.secondary)
                        Text("Pad").font(.caption).foregroundStyle(.secondary)
                    }
                    ForEach(rows, id: \.self) { row in
                        GridRow {
                            Text(row.designator).monospaced()
                            Text(row.name)
                            Text(row.role).foregroundStyle(.secondary)
                            switch row.pad {
                            case .found: Label("Pad \(row.designator)", systemImage: "checkmark").foregroundStyle(.green)
                            case .missing: Label("No pad", systemImage: "xmark").foregroundStyle(.red)
                            case .notNeeded: Text("No lead").foregroundStyle(.secondary)
                            case .unknown: Text("—").foregroundStyle(.secondary)
                            }
                        }
                        .accessibilityElement(children: .combine)
                    }
                }
            }
        }
    }

    // MARK: Still to do

    struct Todo: Hashable {
        let stepID: String
        let title: String
        let issue: String
    }

    /// Every step's issues, in step order.
    static func todo(_ model: PartEditorModel) -> [Todo] {
        model.steps.flatMap { step in
            model.issues(of: step).map { Todo(stepID: step.id, title: step.title, issue: $0) }
        }
    }

    /// The step a loader message points at, with its title for the button.
    static func target(of message: String, in model: PartEditorModel) -> (id: String, title: String)? {
        let steps = model.steps
        guard let id = WizardIssueLocator.step(for: message, steps: steps, draft: model.draft),
              let step = steps.first(where: { $0.id == id }) else { return nil }
        return (id, step.title)
    }

    // MARK: Pins and pads

    struct PinRow: Hashable {
        enum Pad: Hashable {
            /// The footprint has a pad for the pin.
            case found
            /// It has none.
            case missing
            /// An absent pin has no lead, so it needs no pad.
            case notNeeded
            /// The part does not load, so its footprint is not known.
            case unknown
        }

        let designator: String
        let name: String
        let role: String
        let pad: Pad
    }

    /// One row per pin of the part's pinout. `part` is the part as the
    /// backend loaded the current draft (nil while it is rejected).
    static func pinRows(draft: PartDraft, part: CatalogPart?) -> [PinRow] {
        let pins = draft.pinouts.first { $0.name == draft.pinout }?.pins ?? []
        let pads = part?.footprint.map { Set($0.pads.compactMap(\.designator)) }
        return pins.map { pin in
            let pad: PinRow.Pad
            if pin.role == "absent" {
                pad = .notNeeded
            } else if let pads {
                pad = pads.contains(pin.designator) ? .found : .missing
            } else {
                pad = .unknown
            }
            return PinRow(designator: pin.designator, name: pin.name, role: pin.role, pad: pad)
        }
    }
}
