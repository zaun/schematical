import SwiftUI

/// The inspector for a selected bus: its name, its type (Custom, or one of
/// the PCB's card types, whose pins set the wires), how many wires it
/// carries, and each wire's display name. A card type sets the wires, so
/// their count and names are locked until the type is Custom again. The
/// PCB's card edge using the bus ("In use as card edge") locks the count
/// only; the names can still change.
/// Fields commit on Return or when focus leaves them; a refused value shows
/// the reason in red and puts the field back (the `ReferenceSection`
/// pattern). Nothing links a bus to the board.
struct BusPanel: View {
    let document: SchematicDocument
    let busID: UUID

    @State private var countText = ""
    @State private var countError: String?
    @FocusState private var countFocused: Bool
    @State private var nameText = ""
    @State private var nameError: String?
    @FocusState private var nameFocused: Bool
    @State private var typeError: String?

    private var bus: Bus? { document.bus(busID) }

    var body: some View {
        if let bus {
            let inUse = document.edgeBusID == busID
            let locked = !bus.card.isCustom || inUse
            Form {
                Section {
                    VStack(alignment: .leading, spacing: 4) {
                        Text(bus.name.isEmpty ? "Bus" : bus.name)
                            .font(.title2.bold())
                        Text("\(bus.lines.count) \(bus.lines.count == 1 ? "line" : "lines")")
                            .font(.callout)
                            .foregroundStyle(.secondary)
                        if inUse {
                            Label("In use as card edge", systemImage: "info.circle")
                                .font(.callout)
                                .foregroundStyle(.secondary)
                                .help("The PCB's card edge uses this bus: its wire count is locked and it can't be deleted. Its wire names can still change.")
                        }
                    }
                }
                Section("Bus") {
                    TextField("Name", text: $nameText)
                        .focused($nameFocused)
                        .onSubmit(commitName)
                        .onChange(of: nameFocused) { old, new in
                            if old, !new { commitName() }
                        }
                    if let nameError {
                        Text(nameError)
                            .font(.caption)
                            .foregroundStyle(.red)
                    }
                }
                Section("Details") {
                    Picker("Type", selection: Binding(get: { bus.card.type }, set: { type in
                        var card = bus.card
                        card.type = type
                        setCard(card)
                    })) {
                        ForEach(BoardType.allCases, id: \.self) { type in
                            Text(type.title).tag(type)
                        }
                    }
                    .pickerStyle(.menu)
                    .help("Custom wires are free; a card type sets the wires to the card's pins")
                    if bus.card.type == .customCard {
                        LabeledContent("Pads per side") {
                            Stepper("\(bus.card.customCard.pads)", value: Binding(
                                get: { bus.card.customCard.pads },
                                set: { pads in
                                    var card = bus.card
                                    card.customCard.pads = pads
                                    card.customCard = card.customCard.clamped()
                                    setCard(card)
                                }
                            ), in: CustomCard.padRange)
                        }
                        Toggle("Both sides", isOn: Binding(get: { bus.card.customCard.bothSides }, set: { on in
                            var card = bus.card
                            card.customCard.bothSides = on
                            setCard(card)
                        }))
                    }
                    if let typeError {
                        Text(typeError)
                            .font(.caption)
                            .foregroundStyle(.red)
                    }
                }
                if bus.card.type.hasOptions {
                    CardOptionsSection(type: bus.card.type, options: { document.bus(busID)?.card.cardOptions ?? CardOptions() },
                                       set: { options in
                                           guard var card = document.bus(busID)?.card else { return }
                                           card.cardOptions = options
                                           setCard(card)
                                       })
                }
                Section("Wires") {
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
                    .disabled(locked)
                    .help(inUse ? "Locked: in use as the card edge" : locked ? "Set by the bus type" : "")
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
                                .disabled(!bus.card.isCustom)
                                .help(bus.card.isCustom ? "" : "Set by the bus type; set Type to Custom to change it")
                        }
                    }
                }
            }
            .formStyle(.grouped)
            .onAppear {
                resetCount()
                nameText = bus.name
            }
            .onChange(of: bus.wires.count) { _, _ in resetCount() }
            // Undo, redo and other edits change the name from outside the field.
            .onChange(of: bus.name) { _, new in nameText = new }
        }
    }

    private func commitName() {
        nameError = document.setBusName(busID, nameText)
        nameText = document.bus(busID)?.name ?? nameText
    }

    private func setCard(_ card: BusCard) {
        typeError = document.setBusCard(busID, card)
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

/// The inspector for a selected bus port: which side its pins come out of,
/// and a checkbox per bus wire (ticked wires are pins, in bus order).
struct BusPortPanel: View {
    let document: SchematicDocument
    let portID: UUID

    @State private var message: String?

    var body: some View {
        if let found = document.port(portID), let line = found.bus.line(found.port.line) {
            let bus = found.bus
            let port = found.port
            Form {
                Section {
                    VStack(alignment: .leading, spacing: 4) {
                        Text("Bus Port")
                            .font(.title2.bold())
                        Text("\(port.wireIDs.count) of \(bus.wires.count) wires")
                            .font(.callout)
                            .foregroundStyle(.secondary)
                    }
                }
                Section("Pins") {
                    Picker("Side", selection: sideBinding(port.side)) {
                        ForEach(BusPort.sides(for: line), id: \.self) { side in
                            Text(Self.title(side)).tag(side)
                        }
                    }
                    HStack {
                        Button("Select All") { set(Set(bus.wires.map(\.id))) }
                        Button("Deselect All") { set([]) }
                    }
                    if let message {
                        Text(message)
                            .font(.caption)
                            .foregroundStyle(.red)
                    }
                    LazyVStack(alignment: .leading, spacing: 4) {
                        ForEach(bus.wires) { wire in
                            Toggle(wire.name, isOn: Binding(
                                get: { port.wireIDs.contains(wire.id) },
                                set: { on in
                                    var ids = port.wireIDs
                                    if on { ids.insert(wire.id) } else { ids.remove(wire.id) }
                                    set(ids)
                                }
                            ))
                            .toggleStyle(.checkbox)
                        }
                    }
                }
            }
            .formStyle(.grouped)
        }
    }

    private func sideBinding(_ side: ChipLayout.Side) -> Binding<ChipLayout.Side> {
        Binding(get: { side }, set: { message = document.setPortSide(portID, $0) })
    }

    /// Stores the ticked wires; a refusal shows its message. Unticking a pin
    /// with wires on it asks first (the window's confirm dialog).
    private func set(_ ids: Set<UUID>) {
        message = document.requestPortWires(portID, ids)
    }

    private static func title(_ side: ChipLayout.Side) -> String {
        switch side {
        case .top: "Top"
        case .bottom: "Bottom"
        case .left: "Left"
        case .right: "Right"
        }
    }
}
