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
        PartEditorForm(model: model)
            .frame(minWidth: 620, minHeight: 480)
            .navigationTitle(model.title)
            .background(WindowReader { model.window = $0 })
            .focusedSceneValue(\.partEditor, model)
            .toolbar {
                ToolbarItemGroup {
                    Button("Open Part…") { Task { await model.open() } }
                    Button("Save Part…") { Task { await model.save() } }
                        .disabled(!model.isValid)
                    Button("Add to Library") { Task { await model.addToLibraryFromMenu() } }
                        .disabled(!model.isValid)
                }
            }
    }
}

/// The form: Identity (and the backend's verdict), Pinouts, Footprint,
/// Supply, and the kind's own section.
struct PartEditorForm: View {
    @Bindable var model: PartEditorModel

    var body: some View {
        Form {
            if let openError = model.openError {
                Section { Text(openError).foregroundStyle(.red) }
            }
            identity
            verdict
            PinoutsSection(pinouts: $model.draft.pinouts, partPinout: $model.draft.pinout)
            footprint
            if let schema = model.draft.schema {
                let supply = schema.part.filter { Self.supplyKeys.contains($0.key) }
                if !supply.isEmpty {
                    Section("Supply") {
                        FieldsEditor(fields: supply, object: $model.draft.kindPart, pinNames: model.draft.pinNames)
                    }
                }
                Section(PartDraft.displayName(of: model.draft.kind)) {
                    FieldsEditor(fields: schema.deviceFields(variant: model.draft.variant),
                                 object: $model.draft.kindDevice, pinNames: model.draft.pinNames)
                    FieldsEditor(fields: schema.part.filter { !Self.supplyKeys.contains($0.key) },
                                 object: $model.draft.kindPart, pinNames: model.draft.pinNames)
                }
            } else {
                Section { Text("The \(model.draft.kind) kind has no form yet.").foregroundStyle(.secondary) }
            }
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

    private var identity: some View {
        Section("Identity") {
            TextField("Name", text: $model.draft.name)
            if !model.draft.name.isEmpty, model.libraryName != model.draft.name {
                Text("A part named \(model.draft.name) is loaded; Add to Library adds this one as \(model.libraryName).")
                    .font(.caption).foregroundStyle(.secondary)
            }
            TextField("Manufacturer", text: Binding(
                get: { model.draft.manufacturer ?? "" },
                set: { model.draft.manufacturer = $0.isEmpty ? nil : $0 }))
            TextField("Package", text: $model.draft.package)
            TextField("Function", text: $model.draft.function)
            TextField("Description", text: $model.draft.description)
            TextField("Drawer category", text: drawerLevel(0))
            TextField("Drawer group", text: drawerLevel(1))
            TextField("Device id", text: $model.draft.deviceID)
            TextField("Note", text: Binding(
                get: { model.draft.note ?? "" },
                set: { model.draft.note = $0.isEmpty ? nil : $0 }), axis: .vertical)
                .lineLimit(1...4)
            LabeledContent("Flags") {
                HStack {
                    ForEach(PartDraft.flagNames, id: \.self) { flag in
                        Toggle(flag.capitalized, isOn: flagBinding(flag))
                    }
                }
            }
            DatasheetField(datasheet: $model.draft.datasheet, ids: model.validator.datasheetIDs)
        }
    }

    @ViewBuilder private var verdict: some View {
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
            }
            if let status = model.status {
                Text(status).font(.caption).foregroundStyle(.secondary)
            }
        }
    }

    private var footprint: some View {
        Section("Footprint") {
            let pins = model.draft.pinCount
            let choices = model.validator.footprints.filter { Int($0.pins) == pins }
            Picker("Footprint", selection: $model.draft.footprint) {
                Text("None").tag(String?.none)
                ForEach(choices, id: \.id) { footprint in
                    Text("\(footprint.id) — \(footprint.description)").tag(String?.some(footprint.id))
                }
                if let current = model.draft.footprint, !choices.contains(where: { $0.id == current }) {
                    Text(current).tag(String?.some(current))
                }
            }
            Text("Bundled footprints with \(pins) pads (the part's pinout has \(pins) pins).")
                .font(.caption).foregroundStyle(.secondary)
        }
    }

    private func drawerLevel(_ level: Int) -> Binding<String> {
        Binding(
            get: { model.draft.drawer.indices.contains(level) ? model.draft.drawer[level] : "" },
            set: { text in
                var drawer = model.draft.drawer
                while drawer.count <= level { drawer.append("") }
                drawer[level] = text
                while let last = drawer.last, last.isEmpty { drawer.removeLast() }
                model.draft.drawer = drawer
            }
        )
    }

    private func flagBinding(_ flag: String) -> Binding<Bool> {
        Binding(
            get: { model.draft.flags.contains(flag) },
            set: { on in
                if on {
                    if !model.draft.flags.contains(flag) {
                        model.draft.flags = PartDraft.flagNames.filter { $0 == flag || model.draft.flags.contains($0) }
                    }
                } else {
                    model.draft.flags.removeAll { $0 == flag }
                }
            }
        )
    }
}

extension Footprint {
    /// Its designated pad count, as `footprints.json` names it (`pins`).
    var pins: Int { Set(pads.compactMap(\.designator)).count }
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
