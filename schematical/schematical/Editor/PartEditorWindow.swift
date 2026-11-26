import AppKit
import SwiftUI

/// The Part Editor window (plan-32-C): a form for one custom part, checked by
/// the backend's own loader as it is edited, saved as a `.schpart` file or
/// added to the User Library.
/// The Part Editor scene's root (plan-61-C). A window opened from the app
/// has its target. A `.schpart` file sent to the app (a double-click in
/// Finder, a drop on the Dock icon, `open -a`) arrives here as a URL, in a
/// window SwiftUI made for it with no target, or in a Part Editor window
/// already open: the first becomes an editor on that file, the second
/// opens another window for it and keeps its own part.
struct PartEditorSceneRoot: View {
    @Binding var target: PartEditorTarget?
    @Environment(\.openWindow) private var openWindow

    var body: some View {
        Group {
            if let target {
                PartEditorWindowView(target: target)
                    .id(target.id)
            } else {
                Color.clear
                    .frame(minWidth: 820, minHeight: 480)
            }
        }
        .onOpenURL { url in Self.open(url, in: &target).map { openWindow(id: PartEditorScene.id, value: $0) } }
    }

    /// A `.schpart` file sent to the scene: a window with no target becomes
    /// its editor (nil returned); one with a part keeps it, and the file's
    /// target is returned for a window of its own.
    static func open(_ url: URL, in target: inout PartEditorTarget?) -> PartEditorTarget? {
        let opened = PartEditorTarget(source: .file(url))
        if target == nil {
            target = opened
            return nil
        }
        return opened
    }
}

struct PartEditorWindowView: View {
    let target: PartEditorTarget
    @State private var model: PartEditorModel

    init(target: PartEditorTarget) {
        self.init(model: PartEditorModel(target: target), target: target)
    }

    /// A window on `model` (a test's, with its presenter answered).
    init(model: PartEditorModel, target: PartEditorTarget) {
        self.target = target
        _model = State(initialValue: model)
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
                    if model.libraryUUID == nil {
                        Button("Add to Library") { Task { await model.addToLibraryFromMenu() } }
                            .disabled(!model.isValid)
                    }
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
                GuidedPreviewPane(model: model)
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

/// The pane beside the wizard (plan-46-G): the datasheet, or the part as the
/// schematic (Block) or the PCB (Footprint) draws it. The datasheet stays
/// loaded while the user moves between steps and tabs.
struct GuidedPreviewPane: View {
    @Bindable var model: PartEditorModel

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Picker("Pane", selection: $model.guidedPane) {
                ForEach(PartEditorModel.GuidedPane.allCases, id: \.self) { Text($0.title).tag($0) }
            }
            .pickerStyle(.segmented)
            .labelsHidden()
            switch model.guidedPane {
            case .datasheet:
                DatasheetPane(model: model)
            case .block, .footprint:
                if let part = model.lastValidPart {
                    if model.guidedPane == .block {
                        BlockPreviewView(chip: PartPreview.chip(for: part), revision: model.previewRevision)
                    } else {
                        FootprintPreviewView(part: part, revision: model.previewRevision)
                    }
                } else {
                    ContentUnavailableView("No preview", systemImage: "square.dashed",
                                           description: Text("The preview shows the part once it loads."))
                }
            }
        }
        .padding(8)
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
            if model.isNewPart {
                Section {
                    LabeledContent("Kind") {
                        HStack {
                            Text(model.kindChosen ? PartDraft.displayName(of: model.draft.kind) : "Not chosen yet")
                            Button("Change…") { model.changeKind() }
                        }
                    }
                }
            }
            IdentitySection(model: model)
            VerdictSection(validation: model.validation, status: model.status)
            PinoutsSection(pinouts: $model.draft.pinouts, partPinout: $model.draft.pinout,
                           commitName: { model.commitPinName(pinout: $0, pin: $1, to: $2) },
                           status: model.renameStatus)
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
                        Button { choices = Self.removing(choice.id, from: choices) } label: {
                            Label("Remove Choice", systemImage: "minus.circle").labelStyle(.iconOnly)
                        }
                        .buttonStyle(.borderless)
                    }
                }
                Button { choices = Self.adding(to: choices) } label: {
                    Label("Add Choice", systemImage: "plus.circle")
                }
                .buttonStyle(.borderless)
            }
        }
    }

    /// `choices` with a blank choice added at the end.
    static func adding(to choices: [FootprintChoiceDraft]?) -> [FootprintChoiceDraft] {
        (choices ?? []) + [FootprintChoiceDraft(package: "", footprint: "")]
    }

    /// `choices` without the one of `id`; nil when none is left (the key is then absent).
    static func removing(_ id: UUID, from choices: [FootprintChoiceDraft]?) -> [FootprintChoiceDraft]? {
        let left = (choices ?? []).filter { $0.id != id }
        return left.isEmpty ? nil : left
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
                ForEach(Self.matches(ids, typed: datasheet), id: \.self) { id in Button(id) { datasheet = id } }
            } label: {
                Image(systemName: "chevron.down")
            }
            .menuStyle(.borderlessButton)
            .fixedSize()
        }
    }

    /// The menu's ids: the first 40, sorted, that contain what is typed (any case).
    static func matches(_ ids: [String], typed datasheet: String?) -> [String] {
        let needle = (datasheet ?? "").lowercased()
        return Array(ids.filter { needle.isEmpty || $0.lowercased().contains(needle) }.sorted().prefix(40))
    }
}

/// The device's pinouts, each a pin table.
struct PinoutsSection: View {
    @Binding var pinouts: [PinoutDraft]
    @Binding var partPinout: String
    /// Commits a pin's new name (pinout id, pin id, name), so the kind's
    /// settings that name the pin follow (plan-46-H); nil sets it directly.
    var commitName: ((UUID, UUID, String) -> Void)? = nil
    /// What the last rename carried over: "Renamed in 2 settings".
    var status: String? = nil

    var body: some View {
        Section("Pinouts") {
            Picker("The part's pinout", selection: $partPinout) {
                ForEach(pinouts) { Text($0.name).tag($0.name) }
                if !pinouts.contains(where: { $0.name == partPinout }) { Text(partPinout).tag(partPinout) }
            }
            ForEach($pinouts) { $pinout in
                PinoutEditor(pinout: $pinout, onRemove: pinouts.count > 1 ? { Self.remove(pinout.id, from: &pinouts, partPinout: &partPinout) } : nil,
                             onRename: { old, new in if partPinout == old { partPinout = new } },
                             commitName: commitName.map { commit in { pin, name in commit(pinout.id, pin, name) } })
            }
            if let status {
                Text(status).font(.caption).foregroundStyle(.secondary)
            }
            HStack {
                Button("Add Numbered Pinout") { Self.add(.numbered, to: &pinouts) }
                Button("Add Ball Pinout") { Self.add(.ball, to: &pinouts) }
            }
        }
    }

    /// Adds an empty pinout of `style`, named "p<n>" with the first free n from the count up.
    static func add(_ style: PinoutDraft.Style, to pinouts: inout [PinoutDraft]) {
        var n = pinouts.count + 1
        while pinouts.contains(where: { $0.name == "p\(n)" }) { n += 1 }
        pinouts.append(PinoutDraft(name: "p\(n)", style: style, pins: []))
    }

    /// Removes a pinout; when it was the part's, the first one left becomes the part's.
    static func remove(_ id: UUID, from pinouts: inout [PinoutDraft], partPinout: inout String) {
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
    /// Commits a pin's new name (pin id, name); nil sets it directly.
    var commitName: ((UUID, String) -> Void)? = nil

    var body: some View {
        GroupBox {
            VStack(alignment: .leading, spacing: 6) {
                HStack {
                    TextField("Pinout name", text: Binding(get: { pinout.name }, set: { rename(to: $0) }))
                    Text(pinout.style == .ball ? "Ball" : "Numbered").foregroundStyle(.secondary)
                    if let onRemove {
                        Button(action: onRemove) { Label("Remove Pinout", systemImage: "trash").labelStyle(.iconOnly) }
                            .buttonStyle(.borderless)
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
                            PinNameField(name: pin.name) { commitPinName(pin.id, $0) }
                            .frame(minWidth: 70)
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
                                Button { Self.move(pin.id, by: -1, in: &pinout) } label: {
                                    Label("Move Up", systemImage: "arrow.up").labelStyle(.iconOnly)
                                }
                                Button { Self.move(pin.id, by: 1, in: &pinout) } label: {
                                    Label("Move Down", systemImage: "arrow.down").labelStyle(.iconOnly)
                                }
                                Button { pinout.pins.removeAll { $0.id == pin.id } } label: {
                                    Label("Remove Pin", systemImage: "minus.circle").labelStyle(.iconOnly)
                                }
                            }
                            .buttonStyle(.borderless)
                        }
                    }
                }
                Button { Self.addPin(to: &pinout) } label: { Label("Add Pin", systemImage: "plus.circle") }
                    .buttonStyle(.borderless)
            }
            .padding(4)
        }
    }

    /// The name field's edit: the pinout takes the name, and `onRename` hears old and new.
    func rename(to new: String) {
        let old = pinout.name
        pinout.name = new
        onRename(old, new)
    }

    /// A pin name field's commit: through `commitName` when given, else straight into the pin.
    func commitPinName(_ id: UUID, _ name: String) {
        if let commitName {
            commitName(id, name)
        } else if let index = pinout.pins.firstIndex(where: { $0.id == id }) {
            pinout.pins[index].name = name
        }
    }

    /// Adds an input pin numbered one past the highest number (a ball pinout: "A<count + 1>").
    static func addPin(to pinout: inout PinoutDraft) {
        let designator: String
        if pinout.style == .numbered {
            designator = String((pinout.pins.compactMap { Int($0.designator) }.max() ?? 0) + 1)
        } else {
            designator = "A\(pinout.pins.count + 1)"
        }
        pinout.pins.append(PinDraft(designator: designator, name: "", role: "input"))
    }

    /// Moves a pin up (-1) or down (1); nothing at either end.
    static func move(_ id: UUID, by offset: Int, in pinout: inout PinoutDraft) {
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
                Toggle(alternative, isOn: Binding(get: { pin.activeLow.contains(alternative) },
                                                  set: { Self.set(alternative, activeLow: $0, on: &pin) }))
            }
        } label: {
            Text(Self.label(for: pin))
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
        .frame(width: 80, alignment: .leading)
        .disabled(!PinDraft.signalRoles.contains(pin.role))
        .help("Mark the alternatives of the pin's name that are active low")
    }

    /// Flags or clears one alternative.
    static func set(_ alternative: String, activeLow on: Bool, on pin: inout PinDraft) {
        if on { pin.activeLow.insert(alternative) } else { pin.activeLow.remove(alternative) }
    }

    /// The menu's title: "—" (no flag for the role), "Low" (none flagged), "Low ✓" (all), or the flagged ones.
    static func label(for pin: PinDraft) -> String {
        guard PinDraft.signalRoles.contains(pin.role) else { return "—" }
        let flagged = pin.menuAlternatives.filter { pin.activeLow.contains($0) }
        if flagged.isEmpty { return "Low" }
        if flagged.count == pin.menuAlternatives.count { return "Low ✓" }
        return "Low: " + flagged.joined(separator: ",")
    }
}
