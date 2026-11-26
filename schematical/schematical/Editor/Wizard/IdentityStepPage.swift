import SwiftUI

/// Step 2 of the Part Wizard, "Part number" (plan-46-F): the part number,
/// who makes it, its package and a one-line description. The keys the
/// library files a part by (device id, function, drawer path) are filled in
/// from these and can be changed under More options.
struct IdentityStepPage: View {
    @Bindable var model: PartEditorModel
    @State private var showMore: Bool

    // The state's first value is set here: xccov never counts a `@State` default (plan-68-AF).
    init(model: PartEditorModel) {
        self.model = model
        _showMore = State(initialValue: false)
    }

    var body: some View {
        Section {
            TextField("Part number", text: $model.draft.name, prompt: Text("As printed on the part: SN74HC00N"))
            if !model.draft.name.isEmpty, model.libraryName != model.draft.name {
                Text("A part named \(model.draft.name) is loaded; Add to Library adds this one as \(model.libraryName).")
                    .font(.caption).foregroundStyle(.secondary)
            }
            HStack {
                TextField("Manufacturer", text: Binding(
                    get: { model.draft.manufacturer ?? "" },
                    set: { model.draft.manufacturer = $0.isEmpty ? nil : $0 }),
                    prompt: Text("Texas Instruments"))
                Menu {
                    Button("My own design / unknown") { model.draft.manufacturer = "Generic" }
                    Divider()
                    // One line: a menu builds its items only as it opens (plan-68-AF).
                    ForEach(model.manufacturers, id: \.self) { name in Button(name) { model.draft.manufacturer = name } }
                } label: {
                    Image(systemName: "chevron.down")
                }
                .menuStyle(.borderlessButton)
                .fixedSize()
                .accessibilityLabel("Manufacturers")
            }
            if model.draft.manufacturer == "Generic" {
                Text("“Generic” means your own design or an unknown maker. A Generic part has no datasheet.")
                    .font(.caption).foregroundStyle(.secondary)
            }
            HStack {
                TextField("Package", text: $model.draft.package, prompt: Text("SOIC, DIP, TSSOP …"))
                Menu {
                    // One line: a menu builds its items only as it opens (plan-68-AF).
                    ForEach(packageChoices, id: \.self) { name in Button(name) { model.draft.package = name } }
                } label: {
                    Image(systemName: "chevron.down")
                }
                .menuStyle(.borderlessButton)
                .fixedSize()
                .accessibilityLabel("Packages")
            }
            TextField("Description", text: $model.draft.description, prompt: Text("Quad 2-input NAND gate"))
        } footer: {
            Text("The part number is the name the part has in the drawer and on the schematic.")
        }

        Section {
            DisclosureGroup("More options", isExpanded: $showMore) {
                TextField("Device id", text: $model.draft.deviceID)
                TextField("Function", text: $model.draft.function)
                TextField("Drawer category", text: model.drawerLevel(0))
                TextField("Drawer group", text: model.drawerLevel(1))
                TextField("Drawer sub-group", text: model.drawerLevel(2))
                if !model.autoKeys.isEmpty {
                    Text("Filled in from the part number and the kind until you change them.")
                        .font(.caption).foregroundStyle(.secondary)
                }
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
            }
        }
    }

    private var packageChoices: [String] { Self.packages(model.catalog.packages, matching: model.draft.package) }

    /// The catalog's packages that contain the typed text (all of them for
    /// no text), at most 40 for the menu.
    static func packages(_ all: [String], matching text: String) -> [String] {
        let needle = text.trimmingCharacters(in: .whitespaces).lowercased()
        return Array(all.filter { needle.isEmpty || $0.lowercased().contains(needle) }.prefix(40))
    }
}
