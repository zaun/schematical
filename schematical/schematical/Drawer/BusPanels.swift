import SwiftUI

/// The inspector for a selected bus: how many wires it carries, and each
/// wire's display name. Fields commit on Return or when focus leaves them; a
/// refused value shows the reason in red and puts the field back (the
/// `ReferenceSection` pattern).
struct BusPanel: View {
    let document: SchematicDocument
    let busID: UUID

    @State private var countText = ""
    @State private var countError: String?
    @FocusState private var countFocused: Bool

    private var bus: Bus? { document.bus(busID) }

    var body: some View {
        if let bus {
            Form {
                Section {
                    VStack(alignment: .leading, spacing: 4) {
                        Text("Bus")
                            .font(.title2.bold())
                        Text("\(bus.lines.count) \(bus.lines.count == 1 ? "line" : "lines")")
                            .font(.callout)
                            .foregroundStyle(.secondary)
                    }
                }
                Section("Bus") {
                    LabeledContent("Wires") {
                        HStack {
                            TextField("Wires", value: countBinding, format: .number)
                                .labelsHidden()
                                .frame(width: 60)
                                .multilineTextAlignment(.trailing)
                                .focused($countFocused)
                                .onSubmit(commitCount)
                            Stepper("Wires", value: stepperBinding, in: Bus.wireCountRange)
                                .labelsHidden()
                        }
                    }
                    .onChange(of: countFocused) { old, new in
                        if old, !new { commitCount() }
                    }
                    if let countError {
                        Text(countError)
                            .font(.caption)
                            .foregroundStyle(.red)
                    }
                }
                Section("Wire Names") {
                    LazyVStack(alignment: .leading, spacing: 6) {
                        ForEach(Array(bus.wires.enumerated()), id: \.element.id) { index, wire in
                            BusWireNameRow(document: document, busID: busID, index: index, wire: wire)
                        }
                    }
                }
            }
            .formStyle(.grouped)
            .onAppear(perform: resetCount)
            .onChange(of: bus.wires.count) { _, _ in resetCount() }
        }
    }

    /// The count field edits `countText`; it is read back on commit.
    private var countBinding: Binding<Int?> {
        Binding(
            get: { Int(countText) },
            set: { countText = $0.map(String.init) ?? "" }
        )
    }

    /// The stepper sets the count straight away.
    private var stepperBinding: Binding<Int> {
        Binding(
            get: { bus?.wires.count ?? Bus.defaultWireCount },
            set: { value in
                countError = document.setBusWireCount(busID, value)
                resetCount()
            }
        )
    }

    private func commitCount() {
        if let value = Int(countText.trimmingCharacters(in: .whitespaces)) {
            countError = document.setBusWireCount(busID, value)
        }
        resetCount()
    }

    private func resetCount() {
        countText = String(bus?.wires.count ?? 0)
    }
}

/// One wire's index and its editable name.
private struct BusWireNameRow: View {
    let document: SchematicDocument
    let busID: UUID
    let index: Int
    let wire: BusWire

    @State private var name = ""
    @State private var error: String?
    @FocusState private var focused: Bool

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text("\(index)")
                    .monospacedDigit()
                    .foregroundStyle(.secondary)
                    .frame(width: 36, alignment: .trailing)
                TextField("Name", text: $name)
                    .labelsHidden()
                    .focused($focused)
                    .onSubmit(commit)
            }
            if let error {
                Text(error)
                    .font(.caption)
                    .foregroundStyle(.red)
            }
        }
        .onAppear { name = wire.name }
        .onChange(of: wire.name) { _, new in name = new }
        .onChange(of: focused) { old, new in
            if old, !new { commit() }
        }
    }

    private func commit() {
        error = document.renameBusWire(bus: busID, wire: wire.id, to: name)
        name = document.bus(busID)?.wires.first { $0.id == wire.id }?.name ?? wire.name
    }
}
