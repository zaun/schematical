import SwiftUI

/// The selected net label: its net name (1 to 4 characters, no spaces;
/// labels with the same name are one net) and its type. The name applies
/// as it is typed; emptied, or refused for voltage, the label keeps its
/// last name. A No Connect has no name to edit.
struct NetLabelPanel: View {
    let document: SchematicDocument
    let label: NetLabel

    @State private var name = ""
    @FocusState private var isEditing: Bool

    /// The other labels on this net.
    private var others: Int {
        document.netLabels.filter { $0.id != label.id && $0.isNet && $0.netKey == label.netKey }.count
    }

    var body: some View {
        Section {
            Text(label.isNet ? "Net Label" : "No Connect")
                .font(.title2.bold())
            if label.isNet {
                nameField
            }
            Picker("Type", selection: Binding(
                get: { label.style },
                set: { document.setNetLabelStyle(label.id, $0) }
            )) {
                ForEach(NetLabel.Style.allCases) { style in
                    Text(style.title).tag(style)
                }
            }
        } footer: {
            Text(footer)
                .foregroundStyle(.secondary)
        }
        .onAppear { name = label.name }
        .onChange(of: isEditing) { _, editing in
            // Left empty: show the name the label kept.
            if !editing, name.isEmpty { name = label.name }
            if !editing { document.endCoalescing() }
        }
        // Undo and redo change the name from outside the field.
        .onChange(of: label.name) { _, value in
            if !name.isEmpty, name != value { name = value }
        }
    }

    private var footer: String {
        if !label.isNet {
            return "Marks a pin as left unconnected on purpose. It joins nothing, and the PCB shows no line for it."
        }
        return others == 0
            ? "No other label is named \(label.name)."
            : "Joined to \(others) other \(others == 1 ? "label" : "labels") named \(label.name)."
    }

    private var nameField: some View {
        TextField("Net", text: $name)
            .font(.body.monospaced())
            .focused($isEditing)
            .accessibilityHint("Up to \(NetLabel.maxNameLength) characters. Labels with the same name are connected.")
            .onChange(of: name) { _, value in
                // At most four characters, no spaces, as typed.
                let cleaned = NetLabel.cleanName(value) ?? ""
                if cleaned != value { name = cleaned; return }
                guard !cleaned.isEmpty else { return }
                if !document.setNetLabelName(label.id, cleaned) { name = label.name }
            }
            .onSubmit { isEditing = false }
    }
}
