import SwiftUI

// The Part Editor form's sections, each a view of its own (plan-46-E): the
// full form ("All fields") stacks them, and the Part Wizard's steps
// ("Guided") host the same views one at a time, so the two cannot drift
// apart. `PinoutsSection` (PartEditorWindow.swift) is the sixth.

/// Identity: the part's name and maker, where it files in the drawer, its
/// flags and its datasheet.
struct IdentitySection: View {
    @Bindable var model: PartEditorModel

    var body: some View {
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
            TextField("Drawer category", text: model.drawerLevel(0))
            TextField("Drawer group", text: model.drawerLevel(1))
            TextField("Drawer sub-group", text: model.drawerLevel(2))
            TextField("Device id", text: $model.draft.deviceID)
            TextField("Note", text: Binding(
                get: { model.draft.note ?? "" },
                set: { model.draft.note = $0.isEmpty ? nil : $0 }), axis: .vertical)
                .lineLimit(1...4)
            LabeledContent("Flags") {
                HStack {
                    ForEach(PartDraft.flagNames, id: \.self) { flag in
                        Toggle(flag.capitalized, isOn: model.flagBinding(flag))
                    }
                }
            }
            DatasheetField(datasheet: $model.draft.datasheet, ids: model.validator.datasheetIDs)
        }
    }
}

extension PartEditorModel {
    /// One level of the drawer path (category, group, sub-group), for the
    /// form and the wizard's More options.
    func drawerLevel(_ level: Int) -> Binding<String> {
        Binding(
            get: { self.draft.drawer.indices.contains(level) ? self.draft.drawer[level] : "" },
            set: { text in
                var drawer = self.draft.drawer
                while drawer.count <= level { drawer.append("") }
                drawer[level] = text
                while let last = drawer.last, last.isEmpty { drawer.removeLast() }
                self.draft.drawer = drawer
            }
        )
    }

    func flagBinding(_ flag: String) -> Binding<Bool> {
        Binding(
            get: { self.draft.flags.contains(flag) },
            set: { on in
                if on {
                    if !self.draft.flags.contains(flag) {
                        self.draft.flags = PartDraft.flagNames.filter { $0 == flag || self.draft.flags.contains($0) }
                    }
                } else {
                    self.draft.flags.removeAll { $0 == flag }
                }
            }
        )
    }
}

/// The backend's verdict on the draft, and the last Save or Add's result.
struct VerdictSection: View {
    @Bindable var model: PartEditorModel

    var body: some View {
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
}

/// The footprint: the part's custom footprint when it has one, otherwise the
/// bundled footprint picker.
struct FootprintSection: View {
    @Bindable var model: PartEditorModel

    var body: some View {
        if model.draft.customFootprint != nil {
            Section("Custom Footprint") {
                CustomFootprintEditor(
                    footprint: Binding(get: { model.draft.customFootprint! },
                                       set: { model.draft.customFootprint = $0 }),
                    pinNumbers: model.draft.pinNumbers,
                    onRemove: { model.draft.customFootprint = nil })
            }
        } else {
            bundledFootprint
        }
        if model.draft.kind == "socket" {
            FitsEditor(model: model)
        }
    }

    private var bundledFootprint: some View {
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
            Picker("Connector footprint", selection: $model.draft.connectorFootprint) {
                Text("None").tag(String?.none)
                ForEach(choices, id: \.id) { Text($0.id).tag(String?.some($0.id)) }
                if let current = model.draft.connectorFootprint, !choices.contains(where: { $0.id == current }) {
                    Text(current).tag(String?.some(current))
                }
            }
            FootprintChoicesEditor(choices: $model.draft.footprintChoices,
                                   ids: model.validator.footprints.map(\.id))
            Button("Custom Footprint…") {
                var custom = CustomFootprintDraft.starter(id: CustomFootprintDraft.id(forPart: model.draft.uuid),
                                                          pins: pins)
                // Number the starter's pads with the pinout's own pin numbers.
                let numbers = model.draft.pinNumbers
                for index in custom.pads.indices where index < numbers.count {
                    custom.pads[index].pin = numbers[index]
                }
                model.draft.customFootprint = custom
            }
        }
    }
}

/// A socket's `fits` (plan-51-G): the chip footprints it holds, a checkbox per bundled
/// footprint with the part's pad count. The part's own footprint and the socket footprints
/// (`SKT-…`) are not offered; an id the file names that is not offered stays listed, so it
/// can be unchecked. With nothing checked the key is absent, and the loader says `missing "fits"`.
struct FitsEditor: View {
    @Bindable var model: PartEditorModel

    /// The footprints offered for a part of `pins` pads whose own footprint is `own`.
    static func choices(_ footprints: [Footprint], pins: Int, own: String?) -> [Footprint] {
        footprints.filter { Int($0.pins) == pins && $0.id != own && !$0.id.hasPrefix("SKT-") }
    }

    /// `fits` with `id` turned on (appended) or off; nil when nothing is left.
    static func toggled(_ fits: [String]?, _ id: String, on: Bool) -> [String]? {
        var list = fits ?? []
        if on {
            if !list.contains(id) { list.append(id) }
        } else {
            list.removeAll { $0 == id }
        }
        return list.isEmpty ? nil : list
    }

    var body: some View {
        let pins = model.draft.pinCount
        let offered = Self.choices(model.validator.footprints, pins: pins, own: model.draft.footprint)
        let offeredIDs = Set(offered.map(\.id))
        Section("Fits") {
            ForEach(offered, id: \.id) { footprint in
                Toggle("\(footprint.id) — \(footprint.description)", isOn: binding(footprint.id))
            }
            ForEach((model.draft.fits ?? []).filter { !offeredIDs.contains($0) }, id: \.self) { id in
                Toggle(id, isOn: binding(id))
            }
            Text("The chip footprints this socket holds: bundled footprints with \(pins) pads. "
                + "Pin n of the chip sits on the socket's pad n.")
                .font(.caption).foregroundStyle(.secondary)
        }
    }

    private func binding(_ id: String) -> Binding<Bool> {
        Binding(get: { (model.draft.fits ?? []).contains(id) },
                set: { model.draft.fits = Self.toggled(model.draft.fits, id, on: $0) })
    }
}

/// Supply: the part's `vcc` and `inputMax`, for a kind whose parts have them.
struct SupplySection: View {
    @Bindable var model: PartEditorModel

    var body: some View {
        if let schema = model.draft.schema {
            let supply = schema.part.filter { PartEditorForm.supplyKeys.contains($0.key) }
            if !supply.isEmpty {
                Section("Supply") {
                    FieldsEditor(fields: supply, object: $model.draft.kindPart, pinNames: model.draft.pinNames)
                }
            }
        }
    }
}

/// The kind's own keys. With `keys` nil it is the form's kind section: every
/// device key of the variant, then the part's keys but the Supply section's.
/// With `keys` (side-qualified top-level keys, `"device.timing"`: a wizard
/// step's) it shows just those, device keys first.
struct KindFieldsSection: View {
    @Bindable var model: PartEditorModel
    var keys: [String]? = nil
    /// The section's header; the kind's name when nil. "" for no header.
    var title: String? = nil
    /// A wizard step's fields: keys that belong to another choice are hidden
    /// and each field shows its key (plan-46-I). All fields never sets it.
    var guided = false

    var body: some View {
        if let schema = model.draft.schema {
            let fields = Self.fields(schema: schema, variant: model.draft.variant, keys: keys)
            let header = title ?? PartDraft.displayName(of: model.draft.kind)
            if header.isEmpty {
                Section { editors(fields) }
            } else {
                Section(header) { editors(fields) }
            }
        } else {
            Section { Text("The \(model.draft.kind) kind has no form yet.").foregroundStyle(.secondary) }
        }
    }

    @ViewBuilder private func editors(_ fields: (device: [Field], part: [Field])) -> some View {
        FieldsEditor(fields: fields.device, object: $model.draft.kindDevice, pinNames: model.draft.pinNames)
            .environment(\.guidedFields, guided ? GuidedFields(kind: model.draft.kind, path: "device") : nil)
            .environment(\.datasheetFind, guided ? { model.findInDatasheet($0) } : nil)
        FieldsEditor(fields: fields.part, object: $model.draft.kindPart, pinNames: model.draft.pinNames)
            .environment(\.guidedFields, guided ? GuidedFields(kind: model.draft.kind, path: "part") : nil)
            .environment(\.datasheetFind, guided ? { model.findInDatasheet($0) } : nil)
    }

    /// The fields the section shows, by side (pure, for the tests).
    static func fields(schema: KindSchema, variant: String?, keys: [String]?) -> (device: [Field], part: [Field]) {
        let device = schema.deviceFields(variant: variant)
        let part = schema.partFields(variant: variant)
        guard let keys else {
            return (device, part.filter { !PartEditorForm.supplyKeys.contains($0.key) })
        }
        let wanted = keys.compactMap { KindKey(qualified: $0) }
        func pick(_ side: KindKey.Side, _ fields: [Field]) -> [Field] {
            wanted.filter { $0.side == side }.compactMap { key in fields.first { $0.key == key.key } }
        }
        return (pick(.device, device), pick(.part, part))
    }
}
