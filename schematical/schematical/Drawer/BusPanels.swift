import CoreTransferable
import SwiftUI
import UniformTypeIdentifiers

/// The inspector for a selected bus: its name, its type (Custom, or one of
/// the PCB's card types, whose pins set the wires), how many wires it
/// carries, and each wire's display name. A card type sets the wires, so
/// their count and names are locked until the type is Custom again. The
/// PCB's card edge using the bus ("In use as card edge") locks the count
/// only; the names can still change.
/// Fields commit on Return or when focus leaves them; a refused value shows
/// the reason in red and puts the field back (the `ReferenceSection`
/// pattern). Nothing links a bus to the board.
/// Naming a bus after another bus links the two, after a confirmation:
/// this bus takes the other's type and wires, and from then on they are
/// one bus electrically and edits to either change both.
struct BusPanel: View {
    @State private var model: BusPanelModel
    @FocusState private var countFocused: Bool
    @FocusState private var nameFocused: Bool

    init(document: SchematicDocument, busID: UUID) {
        _model = State(initialValue: BusPanelModel(document: document, busID: busID))
    }

    var body: some View {
        @Bindable var model = model
        if let bus = model.bus {
            Form {
                Section {
                    VStack(alignment: .leading, spacing: 4) {
                        Text(model.title)
                            .font(.title2.bold())
                        Text(model.lineCountText)
                            .font(.callout)
                            .foregroundStyle(.secondary)
                        if model.isInUse {
                            Label("In use as card edge", systemImage: "info.circle")
                                .font(.callout)
                                .foregroundStyle(.secondary)
                                .help("The PCB's card edge uses this bus: its wire count is locked and it can't be deleted. Its wire names can still change.")
                        }
                    }
                }
                Section("Details") {
                    TextField("Name", text: $model.nameText)
                        .focused($nameFocused)
                        .onSubmit(model.commitName)
                        .onChange(of: nameFocused) { model.nameFocusChanged(from: $0, to: $1) }
                    ErrorText(model.nameError)
                    Picker("Type", selection: $model.type) {
                        BoardTypeMenuItems()
                    }
                    .pickerStyle(.menu)
                    .help("Custom wires are free; a card type sets the wires to the card's pins")
                    if bus.card.type == .customCard {
                        LabeledContent("Pads per side") {
                            Stepper("\(model.pads)", value: $model.pads, in: CustomCard.padRange)
                        }
                        Toggle("Both sides", isOn: $model.bothSides)
                    }
                    ErrorText(model.typeError)
                    LabeledContent("Wires") {
                        HStack {
                            TextField("Wires", value: $model.countValue, format: .number)
                                .labelsHidden()
                                .frame(width: 60)
                                .multilineTextAlignment(.trailing)
                                .focused($countFocused)
                                .onSubmit(model.commitCount)
                            Stepper("Wires", value: $model.stepperValue, in: Bus.wireCountRange)
                                .labelsHidden()
                        }
                    }
                    .disabled(model.isCountLocked)
                    .help(model.countHelp)
                    .onChange(of: countFocused) { model.countFocusChanged(from: $0, to: $1) }
                    ErrorText(model.countError)
                }
                if bus.card.type.hasBusOptions {
                    CardOptionsSection(type: bus.card.type, bus: true, options: { model.cardOptions },
                                       set: model.setCardOptions)
                }
                Section {
                    let uses = model.wireUses
                    LazyVStack(alignment: .leading, spacing: 6) {
                        ForEach(Array(bus.wires.enumerated()), id: \.element.id) { index, wire in
                            BusWireNameRow(model: model, index: index, count: bus.wires.count,
                                           wire: wire, uses: uses[wire.name] ?? 1)
                                .disabled(!bus.card.isCustom)
                                .help(model.wireHelp)
                        }
                    }
                } header: {
                    HStack {
                        Text("Wire Names")
                        Spacer()
                        Button("Import…") { Task { await model.importWires() } }
                            .disabled(!bus.card.isCustom)
                            .help(model.importHelp)
                        Button("Export…") { Task { await model.exportWires() } }
                            .help("Save the wire names as a CSV file")
                    }
                } footer: {
                    if let footer = model.wireFooter {
                        Text(footer.text)
                            .font(.caption)
                            .foregroundStyle(footer.isError ? .red : .secondary)
                    }
                }
            }
            .formStyle(.grouped)
            .onAppear(perform: model.appear)
            .confirmationDialog(model.linkTitle, isPresented: $model.isAskingToLink,
                                presenting: model.linkTarget) { target in
                Button("Rename and Link") { model.link(to: target) }
                Button("Cancel", role: .cancel, action: model.cancelLink)
            } message: { target in
                Text(BusPanelModel.linkMessage(target))
            }
            .onChange(of: bus.wires.count) { model.resetCount() }
            // Undo, redo and other edits change the name from outside the field.
            .onChange(of: bus.name) { model.busNameChanged($1) }
        }
    }
}

/// A refusal's reason, in red under the field it refused; nothing when nil.
private struct ErrorText: View {
    let message: String?

    init(_ message: String?) {
        self.message = message
    }

    var body: some View {
        if let message {
            Text(message)
                .font(.caption)
                .foregroundStyle(.red)
        }
    }
}

/// One wire's index and its editable name. The handle drags the wire to
/// another row (dropped on a row, it takes that row's place); the context
/// menu moves it one place or to an end.
private struct BusWireNameRow: View {
    let model: BusPanelModel
    let index: Int
    let count: Int
    let wire: BusWire
    /// How many wires of the bus have this wire's name; more than 1 warns.
    let uses: Int

    @State private var name: String
    @State private var error: String?
    @State private var targeted: Bool
    @FocusState private var focused: Bool

    init(model: BusPanelModel, index: Int, count: Int, wire: BusWire, uses: Int) {
        (self.model, self.index, self.count, self.wire, self.uses) = (model, index, count, wire, uses)
        _name = State(initialValue: "")
        _targeted = State(initialValue: false)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Image(systemName: "line.3.horizontal")
                    .foregroundStyle(.secondary)
                    .frame(width: 16)
                    .contentShape(Rectangle())
                    .draggable(BusWireDragItem(bus: model.busID, wire: wire.id)) { Text(wire.name).padding(4) }
                    .help("Drag to reorder")
                // A name shared with other wires of the bus: allowed, but warned.
                Image(systemName: "exclamationmark.triangle.fill")
                    .foregroundStyle(.yellow)
                    .frame(width: 16)
                    .opacity(uses > 1 ? 1 : 0)
                    .help(BusPanelModel.sharedNameHelp(wire.name, uses: uses))
                    .accessibilityHidden(uses <= 1)
                Text("\(index)")
                    .monospacedDigit()
                    .foregroundStyle(.secondary)
                    .frame(width: 36, alignment: .trailing)
                TextField("Name", text: $name)
                    .labelsHidden()
                    .focused($focused)
                    .onSubmit(commit)
                    // Uppercased as typed while Settings says uppercase only.
                    .onChange(of: name) { name = NamePreferences.typed($1) }
            }
            ErrorText(error)
        }
        // The row a dragged wire would take the place of.
        .background(RoundedRectangle(cornerRadius: 4).fill(Color.accentColor.opacity(targeted ? 0.2 : 0)).padding(-2))
        .dropDestination(for: BusWireDragItem.self) { items, _ in model.drop(items, on: wire.id, at: index) } isTargeted: { targeted = $0 }
        .contextMenu {
            let moves = BusPanelModel.moves(index: index, count: count)
            moveButton(moves[0])
            moveButton(moves[1])
            Divider()
            moveButton(moves[2])
            moveButton(moves[3])
        }
        .onAppear { name = wire.name }
        .onChange(of: wire.name) { name = $1 }
        .onChange(of: focused) { if $0, !$1 { commit() } }
    }

    private func moveButton(_ move: BusPanelModel.WireMove) -> some View {
        Button(move.title) { model.moveWire(wire.id, to: move.place) }
            .disabled(!move.isEnabled)
    }

    private func commit() {
        (error, name) = model.renameWire(wire, to: name)
    }
}

/// A bus wire being dragged to another place in the bus's wire list.
nonisolated struct BusWireDragItem: Codable, Hashable, Sendable, Transferable {
    let bus: UUID
    let wire: UUID

    static var transferRepresentation: some TransferRepresentation {
        CodableRepresentation(contentType: .json)
    }
}

/// The inspector for a selected bus port: which side its pins come out of,
/// and a checkbox per bus wire (ticked wires are pins, in bus order).
struct BusPortPanel: View {
    @State private var model: BusPortPanelModel

    init(document: SchematicDocument, portID: UUID) {
        _model = State(initialValue: BusPortPanelModel(document: document, portID: portID))
    }

    var body: some View {
        @Bindable var model = model
        if let found = model.document.port(model.portID), let line = found.bus.line(found.port.line) {
            let bus = found.bus
            Form {
                Section {
                    VStack(alignment: .leading, spacing: 4) {
                        Text("Bus Port")
                            .font(.title2.bold())
                        Text("\(found.port.wireIDs.count) of \(bus.wires.count) wires")
                            .font(.callout)
                            .foregroundStyle(.secondary)
                    }
                }
                Section("Pins") {
                    Picker("Side", selection: $model.side) {
                        ForEach(BusPort.sides(for: line), id: \.self) { Text(BusPortPanelModel.title($0)).tag($0) }
                    }
                    HStack {
                        Button("Select All", action: model.selectAll)
                        Button("Deselect All", action: model.deselectAll)
                    }
                    ErrorText(model.message)
                    LazyVStack(alignment: .leading, spacing: 4) {
                        ForEach(bus.wires) { wire in
                            Toggle(wire.name, isOn: Binding { model.isPin(wire.id) } set: { model.setPin(wire.id, $0) })
                                .toggleStyle(.checkbox)
                        }
                    }
                }
            }
            .formStyle(.grouped)
        }
    }
}
