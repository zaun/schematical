import AppKit
import SwiftUI

/// The Part Editor window (plan-32-C): a form for one custom part, checked by
/// the backend's own loader as it is edited, saved as a `.schpart` file or
/// added to the parts drawer.
struct PartEditorWindowView: View {
    let target: PartEditorTarget
    @State private var model: PartEditorModel

    init(target: PartEditorTarget) {
        self.target = target
        _model = State(initialValue: PartEditorModel(target: target))
    }

    var body: some View {
        content
            .frame(minWidth: model.mode == .guided ? 1000 : 820, minHeight: 480)
            .navigationTitle(model.title)
            .background(WindowReader { model.window = $0 })
            .focusedSceneValue(\.partEditor, model)
            .toolbar {
                ToolbarItem(placement: .principal) {
                    Picker("Mode", selection: $model.mode) {
                        ForEach(PartEditorModel.EditorMode.allCases, id: \.self) { Text($0.title).tag($0) }
                    }
                    .pickerStyle(.segmented)
                    .help("Guided walks through the part step by step; All fields shows the whole form")
                }
                ToolbarItemGroup {
                    Button("Open Part…") { Task { await model.open() } }
                    Button("Save Part…") { Task { await model.save() } }
                        .disabled(!model.isValid)
                    Button("Add to Library") { Task { await model.addToLibraryFromMenu() } }
                        .disabled(!model.isValid)
                }
            }
    }

    /// Guided: the step list, the current step with Back / Next, the preview.
    /// All fields: the form and the preview. One model, so switching keeps
    /// every value.
    @ViewBuilder private var content: some View {
        switch model.mode {
        case .guided:
            HSplitView {
                WizardStepList(model: model)
                    .frame(minWidth: 180, idealWidth: 200, maxWidth: 220, maxHeight: .infinity)
                VStack(spacing: 0) {
                    if let step = model.currentStep {
                        WizardStepView(model: model, step: step)
                            .id(step.id)
                    }
                    Divider()
                    WizardNavBar(model: model)
                }
                .frame(minWidth: 480, maxWidth: .infinity, maxHeight: .infinity)
                PartPreviewPane(model: model)
                    .frame(minWidth: 260, idealWidth: 340, maxHeight: .infinity, alignment: .top)
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        case .allFields:
            HSplitView {
                PartEditorForm(model: model)
                    .frame(minWidth: 520)
                PartPreviewPane(model: model)
                    .frame(minWidth: 260, idealWidth: 340)
            }
        }
    }
}

/// The preview beside the form: the part as the schematic draws it (Block)
/// or as the PCB does (Footprint), one at a time (plan-32-E). It shows the
/// last part the backend accepted, so it stays while the draft has an error.
struct PartPreviewPane: View {
    @Bindable var model: PartEditorModel

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Picker("Preview", selection: $model.previewPane) {
                ForEach(PartEditorModel.PreviewPane.allCases, id: \.self) { Text($0.title).tag($0) }
            }
            .pickerStyle(.segmented)
            .labelsHidden()
            if let part = model.lastValidPart {
                switch model.previewPane {
                case .block:
                    BlockPreviewView(chip: PartPreview.chip(for: part), revision: model.previewRevision)
                case .footprint:
                    FootprintPreviewView(part: part, revision: model.previewRevision)
                }
            } else {
                ContentUnavailableView("No preview", systemImage: "square.dashed",
                                       description: Text("The preview shows the part once it loads."))
            }
        }
        .padding(8)
    }
}

/// The form ("All fields"): Identity (and the backend's verdict), Pinouts,
/// Footprint, Supply, and the kind's own section. The sections are views of
/// their own (PartEditorSections.swift), shared with the Part Wizard's steps.
struct PartEditorForm: View {
    @Bindable var model: PartEditorModel

    var body: some View {
        Form {
            if let openError = model.openError {
                Section { Text(openError).foregroundStyle(.red) }
            }
            IdentitySection(model: model)
            VerdictSection(model: model)
            PinoutsSection(pinouts: $model.draft.pinouts, partPinout: $model.draft.pinout)
            FootprintSection(model: model)
            SupplySection(model: model)
            KindFieldsSection(model: model)
            if !model.draft.extraKeys.isEmpty {
                Section("Kept as they are") {
                    Text(model.draft.extraKeys.joined(separator: ", "))
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
        }
        .formStyle(.grouped)
    }

    static let supplyKeys: Set<String> = ["vcc", "inputMax"]
}

extension Footprint {
    /// Its designated pad count, as `footprints.json` names it (`pins`).
    var pins: Int { Set(pads.compactMap(\.designator)).count }
}

/// `footprintChoices`: the footprints a placed part may use, each with the
/// package name the PCB sidebar shows; one is the part's own footprint.
struct FootprintChoicesEditor: View {
    @Binding var choices: [FootprintChoiceDraft]?
    let ids: [String]

    var body: some View {
        LabeledContent("Footprint choices") {
            VStack(alignment: .trailing, spacing: 4) {
                ForEach(Binding(get: { choices ?? [] }, set: { choices = $0.isEmpty ? nil : $0 })) { $choice in
                    HStack {
                        TextField("Package", text: $choice.package).frame(width: 120)
                        TextField("Footprint id", text: $choice.footprint).frame(width: 160)
                        Button { choices?.removeAll { $0.id == choice.id }; if choices?.isEmpty == true { choices = nil } }
                            label: { Image(systemName: "minus.circle") }
                            .buttonStyle(.borderless)
                    }
                }
                Button { choices = (choices ?? []) + [FootprintChoiceDraft(package: "", footprint: "")] } label: {
                    Label("Add Choice", systemImage: "plus.circle")
                }
                .buttonStyle(.borderless)
            }
        }
    }
}

/// The datasheet id: a text field and a menu of the bundled ids that match it.
struct DatasheetField: View {
    @Binding var datasheet: String?
    let ids: [String]

    var body: some View {
        HStack {
            TextField("Datasheet", text: Binding(
                get: { datasheet ?? "" },
                set: { datasheet = $0.isEmpty ? nil : $0 }))
            Menu {
                Button("None") { datasheet = nil }
                ForEach(matches, id: \.self) { id in
                    Button(id) { datasheet = id }
                }
            } label: {
                Image(systemName: "chevron.down")
            }
            .menuStyle(.borderlessButton)
            .fixedSize()
        }
    }

    private var matches: [String] {
        let needle = (datasheet ?? "").lowercased()
        return Array(ids.filter { needle.isEmpty || $0.lowercased().contains(needle) }.sorted().prefix(40))
    }
}

/// The device's pinouts, each a pin table.
struct PinoutsSection: View {
    @Binding var pinouts: [PinoutDraft]
    @Binding var partPinout: String

    var body: some View {
        Section("Pinouts") {
            Picker("The part's pinout", selection: $partPinout) {
                ForEach(pinouts) { Text($0.name).tag($0.name) }
                if !pinouts.contains(where: { $0.name == partPinout }) { Text(partPinout).tag(partPinout) }
            }
            ForEach($pinouts) { $pinout in
                PinoutEditor(pinout: $pinout, onRemove: pinouts.count > 1 ? { remove(pinout.id) } : nil,
                             onRename: { old, new in if partPinout == old { partPinout = new } })
            }
            HStack {
                Button("Add Numbered Pinout") { add(.numbered) }
                Button("Add Ball Pinout") { add(.ball) }
            }
        }
    }

    private func add(_ style: PinoutDraft.Style) {
        var n = pinouts.count + 1
        while pinouts.contains(where: { $0.name == "p\(n)" }) { n += 1 }
        pinouts.append(PinoutDraft(name: "p\(n)", style: style, pins: []))
    }

    private func remove(_ id: UUID) {
        pinouts.removeAll { $0.id == id }
        if !pinouts.contains(where: { $0.name == partPinout }), let first = pinouts.first {
            partPinout = first.name
        }
    }
}

struct PinoutEditor: View {
    @Binding var pinout: PinoutDraft
    let onRemove: (() -> Void)?
    let onRename: (String, String) -> Void

    var body: some View {
        GroupBox {
            VStack(alignment: .leading, spacing: 6) {
                HStack {
                    TextField("Pinout name", text: Binding(
                        get: { pinout.name },
                        set: { new in
                            let old = pinout.name
                            pinout.name = new
                            onRename(old, new)
                        }))
                    Text(pinout.style == .ball ? "Ball" : "Numbered").foregroundStyle(.secondary)
                    if let onRemove {
                        Button(action: onRemove) { Image(systemName: "trash") }.buttonStyle(.borderless)
                    }
                }
                Grid(alignment: .leading, horizontalSpacing: 6, verticalSpacing: 4) {
                    GridRow {
                        Text(pinout.style == .ball ? "Ball" : "Pin").font(.caption)
                        Text("Name").font(.caption)
                        Text("Role").font(.caption)
                        Text("Group").font(.caption)
                        Text("Side").font(.caption)
                        Text("Active low").font(.caption)
                        Text("")
                    }
                    ForEach($pinout.pins) { $pin in
                        GridRow {
                            TextField("", text: $pin.designator).frame(width: 50)
                            TextField("", text: $pin.name).frame(minWidth: 70)
                            Picker("", selection: $pin.role) {
                                ForEach(PinDraft.roles, id: \.self) { Text($0).tag($0) }
                            }
                            .labelsHidden().frame(width: 90)
                            TextField("", text: $pin.group).frame(minWidth: 60)
                            Picker("", selection: $pin.side) {
                                Text("auto").tag("")
                                Text("in").tag("in")
                                Text("out").tag("out")
                            }
                            .labelsHidden().frame(width: 70)
                            ActiveLowMenu(pin: $pin)
                            HStack(spacing: 2) {
                                Button { move(pin.id, by: -1) } label: { Image(systemName: "arrow.up") }
                                Button { move(pin.id, by: 1) } label: { Image(systemName: "arrow.down") }
                                Button { pinout.pins.removeAll { $0.id == pin.id } } label: {
                                    Image(systemName: "minus.circle")
                                }
                            }
                            .buttonStyle(.borderless)
                        }
                    }
                }
                Button { addPin() } label: { Label("Add Pin", systemImage: "plus.circle") }
                    .buttonStyle(.borderless)
            }
            .padding(4)
        }
    }

    private func addPin() {
        let designator: String
        if pinout.style == .numbered {
            designator = String((pinout.pins.compactMap { Int($0.designator) }.max() ?? 0) + 1)
        } else {
            designator = "A\(pinout.pins.count + 1)"
        }
        pinout.pins.append(PinDraft(designator: designator, name: "", role: "input"))
    }

    private func move(_ id: UUID, by offset: Int) {
        guard let index = pinout.pins.firstIndex(where: { $0.id == id }) else { return }
        let target = index + offset
        guard pinout.pins.indices.contains(target) else { return }
        pinout.pins.swapAt(index, target)
    }
}

/// A pin's "Active low" control (plan-39-B): one checkable item per
/// alternative of the name; disabled for power, ground, nc and absent pins.
struct ActiveLowMenu: View {
    @Binding var pin: PinDraft

    var body: some View {
        Menu {
            ForEach(pin.menuAlternatives, id: \.self) { alternative in
                Toggle(alternative, isOn: Binding(
                    get: { pin.activeLow.contains(alternative) },
                    set: { on in
                        if on { pin.activeLow.insert(alternative) } else { pin.activeLow.remove(alternative) }
                    }))
            }
        } label: {
            Text(label)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
        .frame(width: 80, alignment: .leading)
        .disabled(!PinDraft.signalRoles.contains(pin.role))
        .help("Mark the alternatives of the pin's name that are active low")
    }

    private var label: String {
        guard PinDraft.signalRoles.contains(pin.role) else { return "—" }
        let flagged = pin.menuAlternatives.filter { pin.activeLow.contains($0) }
        if flagged.isEmpty { return "Low" }
        if flagged.count == pin.menuAlternatives.count { return "Low ✓" }
        return "Low: " + flagged.joined(separator: ",")
    }
}
