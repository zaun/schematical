import SwiftUI

/// The selected net label: its net name (1 to 8 characters, no spaces;
/// labels with the same name are one net) and its type. The name applies
/// as it is typed; emptied, or refused for voltage, the label keeps its
/// last name. A No Connect has no name to edit.
struct NetLabelPanel: View {
    let document: SchematicDocument
    let label: NetLabel

    @State private var name: String
    @FocusState private var isEditing: Bool

    init(document: SchematicDocument, label: NetLabel) {
        self.document = document
        self.label = label
        _name = State(initialValue: "")
    }

    private var editing: NetLabelEditing { NetLabelEditing(document: document, label: label) }

    var body: some View {
        Section {
            Text(label.isNet ? "Net Label" : label.title)
                .font(.title2.bold())
            if label.isNet {
                nameField
            }
            Picker("Type", selection: Binding { label.style } set: { document.setNetLabelStyle(label.id, $0) }) {
                ForEach(NetLabel.Style.allCases) { Text($0.title).tag($0) }
            }
        } footer: {
            Text(editing.footer)
                .foregroundStyle(.secondary)
        }
        .onAppear { name = label.name }
        .onChange(of: isEditing) { if !$1 { name = editing.nameAfterEditing(name) } }
        // Undo and redo change the name from outside the field.
        .onChange(of: label.name) { name = NetLabelEditing.nameAfterRename(field: name, to: $1) }
    }

    private var nameField: some View {
        TextField("Net", text: $name)
            .font(.body.monospaced())
            .focused($isEditing)
            .accessibilityHint("Up to \(NetLabel.maxNameLength) characters. Labels with the same name are connected.")
            .onChange(of: name) { if let shown = editing.nameAfterTyping($1) { name = shown } }
            .onSubmit { isEditing = false }
    }
}

/// What the net label panel's name field does (plan-68-AB: out of
/// `NetLabelPanel`, so `NetLabelPanelTests` drive it without a view).
struct NetLabelEditing {
    let document: SchematicDocument
    let label: NetLabel

    /// The other labels on this net.
    var others: Int {
        document.netLabels.filter { $0.id != label.id && $0.isNet && $0.netKey == label.netKey }.count
    }

    var footer: String {
        if label.isPowerFlag {
            return "Marks the net it is on as driven, for the electrical rules check. It names nothing and joins nothing else."
        }
        if !label.isNet {
            return "Marks a pin as left unconnected on purpose. It joins nothing, and the PCB shows no line for it."
        }
        let others = others
        return others == 0
            ? "No other label is named \(label.name)."
            : "Joined to \(others) other \(others == 1 ? "label" : "labels") named \(label.name)."
    }

    /// The field's text once `field` was typed: at most eight characters, no
    /// spaces (uppercased while Settings says uppercase only). A cleaned name
    /// is applied to the label; nil when the field stays as typed.
    func nameAfterTyping(_ field: String) -> String? {
        let cleaned = NetLabel.cleanName(NamePreferences.typed(field)) ?? ""
        if cleaned != field { return cleaned }
        guard !cleaned.isEmpty else { return nil }
        return document.setNetLabelName(label.id, cleaned) ? nil : label.name
    }

    /// Editing ended with `field` in the field: left empty, it shows the
    /// name the label kept. Ends the typing's undo step.
    func nameAfterEditing(_ field: String) -> String {
        document.endCoalescing()
        return field.isEmpty ? label.name : field
    }

    /// The label was renamed to `name` from outside the field (undo, redo):
    /// the field follows, unless it is empty while being typed in.
    static func nameAfterRename(field: String, to name: String) -> String {
        field.isEmpty ? field : name
    }
}
