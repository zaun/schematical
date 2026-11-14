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
    let document: SchematicDocument
    let busID: UUID

    @State private var countText = ""
    @State private var countError: String?
    @FocusState private var countFocused: Bool
    @State private var nameText = ""
    @State private var nameError: String?
    @FocusState private var nameFocused: Bool
    @State private var typeError: String?
    @State private var wireOrderError: String?
    /// The bus a name being committed would link with, while asking.
    @State private var linkTarget: Bus?

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
                Section("Details") {
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
                    Picker("Type", selection: Binding(get: { bus.card.type }, set: { type in
                        var card = bus.card
                        card.type = type
                        setCard(card)
                    })) {
                        BoardTypeMenuItems()
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
                if bus.card.type.hasBusOptions {
                    CardOptionsSection(type: bus.card.type, bus: true, options: { document.bus(busID)?.card.cardOptions ?? CardOptions() },
                                       set: { options in
                                           guard var card = document.bus(busID)?.card else { return }
                                           card.cardOptions = options
                                           setCard(card)
                                       })
                }
                Section {
                    let uses = bus.wires.reduce(into: [String: Int]()) { $0[$1.name, default: 0] += 1 }
                    LazyVStack(alignment: .leading, spacing: 6) {
                        ForEach(Array(bus.wires.enumerated()), id: \.element.id) { index, wire in
                            BusWireNameRow(document: document, busID: busID, index: index, count: bus.wires.count,
                                           wire: wire, uses: uses[wire.name] ?? 1, message: $wireOrderError)
                                .disabled(!bus.card.isCustom)
                                .help(bus.card.isCustom ? "" : "Set by the bus type; set Type to Custom to change it")
                        }
                    }
                } header: {
                    HStack {
                        Text("Wire Names")
                        Spacer()
                        Button("Import…") { importWires() }
                            .disabled(!bus.card.isCustom)
                            .help(bus.card.isCustom ? "Load the wire names from a CSV or text file" : "Set by the bus type; set Type to Custom to change it")
                        Button("Export…") { exportWires(bus) }
                            .help("Save the wire names as a CSV file")
                    }
                } footer: {
                    if let wireOrderError {
                        Text(wireOrderError)
                            .font(.caption)
                            .foregroundStyle(.red)
                    } else if bus.card.isCustom {
                        Text("Drag a wire by its handle to reorder; a wire keeps its net and port pins.")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }
                }
            }
            .formStyle(.grouped)
            .onAppear {
                resetCount()
                nameText = bus.name
            }
            .confirmationDialog(linkTarget.map { "Link with \($0.name)?" } ?? "", isPresented: Binding(
                get: { linkTarget != nil },
                set: { if !$0 { cancelLink() } }
            ), presenting: linkTarget) { target in
                Button("Rename and Link") { link(to: target) }
                Button("Cancel", role: .cancel) { cancelLink() }
            } message: { target in
                Text("A bus named \(target.name) already exists. Renaming this bus will change its settings (type, wire count and wire names) to match \(target.name)'s, and the two buses will be linked: each wire is one net on both, and changing either bus's settings changes both.")
            }
            .onChange(of: bus.wires.count) { _, _ in resetCount() }
            // Undo, redo and other edits change the name from outside the field.
            .onChange(of: bus.name) { _, new in nameText = new }
        }
    }

    private func commitName() {
        guard linkTarget == nil else { return }
        if let target = document.busLinkTarget(busID, named: nameText) {
            nameError = nil
            linkTarget = target
            return
        }
        nameError = document.setBusName(busID, nameText)
        nameText = document.bus(busID)?.name ?? nameText
    }

    private func link(to target: Bus) {
        linkTarget = nil
        nameError = document.setBusName(busID, target.name, link: true)
        nameText = document.bus(busID)?.name ?? nameText
    }

    private func cancelLink() {
        linkTarget = nil
        nameText = document.bus(busID)?.name ?? nameText
    }

    /// Saves the bus's wire names (see `BusWireList`).
    private func exportWires(_ bus: Bus) {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.commaSeparatedText]
        panel.nameFieldStringValue = "\(bus.name.isEmpty ? "Bus" : bus.name) wires.csv"
        guard panel.runModal() == .OK, let url = panel.url else { return }
        do {
            try BusWireList.text(bus.wires.map(\.name)).write(to: url, atomically: true, encoding: .utf8)
            wireOrderError = nil
        } catch {
            NSAlert(error: error).runModal()
        }
    }

    /// Loads wire names from a file onto this bus (see `BusWireList`).
    private func importWires() {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [.commaSeparatedText, .plainText]
        panel.allowsMultipleSelection = false
        guard panel.runModal() == .OK, let url = panel.url else { return }
        do {
            let text = try String(contentsOf: url, encoding: .utf8)
            switch BusWireList.names(in: text) {
            case let .success(names): wireOrderError = document.setBusWireNames(busID, names)
            case let .failure(error): wireOrderError = error.message
            }
        } catch {
            NSAlert(error: error).runModal()
        }
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

/// One wire's index and its editable name. The handle drags the wire to
/// another row (dropped on a row, it takes that row's place); the context
/// menu moves it one place or to an end.
private struct BusWireNameRow: View {
    let document: SchematicDocument
    let busID: UUID
    let index: Int
    let count: Int
    let wire: BusWire
    /// How many wires of the bus have this wire's name; more than 1 warns.
    let uses: Int
    /// The panel's reorder message: why a move was refused.
    @Binding var message: String?

    @State private var name = ""
    @State private var error: String?
    @State private var targeted = false
    @FocusState private var focused: Bool

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Image(systemName: "line.3.horizontal")
                    .foregroundStyle(.secondary)
                    .frame(width: 16)
                    .contentShape(Rectangle())
                    .draggable(BusWireDragItem(bus: busID, wire: wire.id)) {
                        Text(wire.name)
                            .padding(4)
                    }
                    .help("Drag to reorder")
                // A name shared with other wires of the bus: allowed, but warned.
                Image(systemName: "exclamationmark.triangle.fill")
                    .foregroundStyle(.yellow)
                    .frame(width: 16)
                    .opacity(uses > 1 ? 1 : 0)
                    .help(uses > 1 ? "\(wire.name) is used \(uses) times in this bus. Each wire is still its own net." : "")
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
                    .onChange(of: name) { _, value in
                        let typed = NamePreferences.typed(value)
                        if typed != value { name = typed }
                    }
            }
            if let error {
                Text(error)
                    .font(.caption)
                    .foregroundStyle(.red)
            }
        }
        // The row a dragged wire would take the place of.
        .background(
            RoundedRectangle(cornerRadius: 4)
                .fill(Color.accentColor.opacity(targeted ? 0.2 : 0))
                .padding(-2)
        )
        .dropDestination(for: BusWireDragItem.self) { items, _ in
            guard let item = items.first, item.bus == busID, item.wire != wire.id else { return false }
            move(item.wire, to: index)
            return true
        } isTargeted: { targeted = $0 }
        .contextMenu {
            Button("Move Up") { move(wire.id, to: index - 1) }
                .disabled(index == 0)
            Button("Move Down") { move(wire.id, to: index + 1) }
                .disabled(index == count - 1)
            Divider()
            Button("Move to Top") { move(wire.id, to: 0) }
                .disabled(index == 0)
            Button("Move to Bottom") { move(wire.id, to: count - 1) }
                .disabled(index == count - 1)
        }
        .onAppear { name = wire.name }
        .onChange(of: wire.name) { _, new in name = new }
        .onChange(of: focused) { old, new in
            if old, !new { commit() }
        }
    }

    private func move(_ id: UUID, to place: Int) {
        message = document.moveBusWire(bus: busID, wire: id, to: place)
    }

    private func commit() {
        error = document.renameBusWire(bus: busID, wire: wire.id, to: name)
        name = document.bus(busID)?.wires.first { $0.id == wire.id }?.name ?? wire.name
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
