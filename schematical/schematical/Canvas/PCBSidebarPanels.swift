import SwiftUI

/// Type, size, copper layers and colour of the board.
struct BoardPanel: View {
    @Bindable var document: SchematicDocument

    var body: some View {
        Section("Board") {
            Picker("Type", selection: Binding(get: { document.board.type }, set: { document.setBoardType($0) })) {
                BoardTypeMenuItems()
            }
            .pickerStyle(.menu)
            .help("Custom is a free rectangle; a card type sets a standard card's size, edge connector and fingers; a HAT or shield its size, header and holes")
            if document.board.type.isCard {
                // The schematic bus whose wires are the card's pins, in
                // order. Only buses with as many wires as the card has pins.
                Picker(selection: Binding(get: { document.edgeBusID }, set: { document.setEdgeBus($0) })) {
                    Text("None").tag(UUID?.none)
                    ForEach(document.edgeBusChoices) { bus in
                        Text(bus.name).tag(Optional(bus.id))
                    }
                } label: {
                    // The wire count the bus needs sits between the label and the menu.
                    HStack {
                        Text("Bus")
                        Spacer()
                        Text("(\(document.board.pinCount) Wires)")
                            .foregroundStyle(.secondary)
                    }
                }
                .pickerStyle(.menu)
                .help("Buses with \(document.board.pinCount) wires, one per pin of this card")
            }
            if document.board.isFreeForm {
                // A free-form outline fixes the size: it is the outline's bounds (plan-65-F).
                LabeledContent("Width") {
                    Text(LengthField.format(document.board.width, units: document.board.units)
                         + " " + document.board.units.title)
                }
                .help("Fixed by the board outline")
            } else if document.board.card?.isFixedSize == true {
                // A cartridge's size is fixed by its shell, a HAT's or shield's by its standard.
                LabeledContent("Width") {
                    Text(LengthField.format(document.board.width, units: document.board.units)
                         + " " + document.board.units.title)
                }
                .help(document.board.connectorName == "Header" ? "Fixed by the board standard" : "Fixed by the cartridge")
            } else {
                LengthField(title: "Width", millimetres: document.board.width, units: document.board.units) {
                    document.setBoardSize(width: $0, height: document.board.height)
                    return document.board.width
                }
            }
            if document.board.isFreeForm {
                LabeledContent("Height") {
                    Text(LengthField.format(document.board.height, units: document.board.units)
                         + " " + document.board.units.title)
                }
                .help("Fixed by the board outline")
                Button("Reset to Rectangle") { document.resetBoardOutline() }
                    .help("Drop the free-form outline: the board becomes a rectangle of the same width and height")
            } else if document.board.type == .custom || document.board.type == .customCard {
                LengthField(title: "Height", millimetres: document.board.height, units: document.board.units) {
                    document.setBoardSize(width: document.board.width, height: $0)
                    return document.board.height
                }
            } else {
                // A card's height is fixed by its standard.
                LabeledContent("Height") {
                    Text(LengthField.format(document.board.height, units: document.board.units)
                         + " " + document.board.units.title)
                }
                .help("Fixed by the card standard")
            }
            Picker("Layers", selection: Binding(
                get: { document.board.layers },
                set: { document.setLayerCount($0) }
            )) {
                ForEach(BoardSettings.layerCounts, id: \.self) { count in
                    // A count that would drop a layer holding something is
                    // greyed, and says why.
                    let refusal = count == document.board.layers ? nil : document.canSetLayerCount(count)
                    Text(count == 1 ? "1 layer" : "\(count) layers")
                        .tag(count)
                        .selectionDisabled(refusal != nil)
                        .help(refusal ?? "")
                }
            }
            .pickerStyle(.menu)
            LabeledContent("Color") {
                VStack(alignment: .trailing, spacing: 6) {
                    HStack(spacing: 6) {
                        ForEach(BoardColor.allCases, id: \.self) { colour in
                            swatch(colour)
                        }
                    }
                    Text(document.board.color.title)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
        }
    }

    private func swatch(_ colour: BoardColor) -> some View {
        Button {
            document.setBoardColor(colour)
        } label: {
            Circle()
                .fill(colour.fill)
                .overlay(Circle().strokeBorder(.white.opacity(0.25), lineWidth: 0.5))
                .frame(width: 20, height: 20)
                .padding(3)
                .overlay {
                    if document.board.color == colour {
                        Circle().strokeBorder(Color.accentColor, lineWidth: 2)
                    }
                }
        }
        .buttonStyle(.plain)
        .help(colour.title)
        .accessibilityLabel(colour.title)
    }
}

// MARK: - Layers

/// One row per copper layer, Top first: a colour well, the name and the
/// type. Top and Bottom are always Component; an inner layer is Plane or
/// Wire. A Plane's net and clearance are in `PlanesSection`.
struct LayersSection: View {
    let document: SchematicDocument

    var body: some View {
        Section("Layers") {
            ForEach(document.board.copperLayers, id: \.self) { layer in
                row(layer)
            }
        }
    }

    private func row(_ layer: CopperLayer) -> some View {
        let style = document.board.style(of: layer)
        return HStack(spacing: 8) {
            ColorPicker("\(layer.title) colour", selection: Binding(
                get: { style.color.color },
                set: { document.setLayerColor(layer, LayerColor($0)) }
            ), supportsOpacity: false)
            .labelsHidden()
            Text(layer.title)
            Spacer()
            if layer.isInner {
                Picker("\(layer.title) type", selection: Binding(
                    get: { style.type },
                    set: { document.setLayerType(layer, $0) }
                )) {
                    ForEach([LayerType.plane, .wire], id: \.self) { type in
                        let refusal = type == style.type ? nil : document.canSetLayerType(layer, type)
                        Text(type.title)
                            .tag(type)
                            .selectionDisabled(refusal != nil)
                            .help(refusal ?? "")
                    }
                }
                .pickerStyle(.menu)
                .labelsHidden()
                .fixedSize()
            } else {
                Text(LayerType.component.title)
                    .foregroundStyle(.secondary)
            }
        }
    }
}

// MARK: - Planes

/// Each Plane layer, in stack order, a row each: its name, the net it carries (None,
/// GND, a power rail or a signal net) and its clearance, how far its copper
/// keeps from pads and vias off its net (the DRC's min clearance until set;
/// a high-voltage plane wants more). Shown only when the board has a Plane.
struct PlanesSection: View {
    let document: SchematicDocument

    static func planes(of board: BoardSettings) -> [CopperLayer] {
        board.copperLayers.filter { $0.isInner && board.style(of: $0).type == .plane }
    }

    var body: some View {
        let planes = Self.planes(of: document.board)
        if !planes.isEmpty {
            let choices = document.planeNetChoices()
            let rules = document.board.rules
            Section("Planes") {
                ForEach(planes, id: \.self) { layer in
                    row(layer, choices: choices, rules: rules)
                }
            }
        }
    }

    /// The layer's colour and name, then its net and its clearance, with a
    /// button back to the DRC's minimum once the clearance is set.
    private func row(_ layer: CopperLayer, choices: [(net: PlaneNet, title: String)], rules: DesignRules) -> some View {
        let style = document.board.style(of: layer)
        let units = document.board.units
        return HStack(spacing: 8) {
            RoundedRectangle(cornerRadius: 3)
                .fill(style.color.color)
                .frame(width: 14, height: 14)
            Text(layer.title)
            Spacer()
            netPicker(layer, choices: choices)
            HStack(spacing: 4) {
                Text("Clearance").foregroundStyle(.secondary)
                LengthField(title: "\(layer.title) clearance", millimetres: style.clearance(rules: rules), units: units,
                            precise: true) {
                    document.setLayerClearance(layer, $0)
                    return document.board.style(of: layer).clearance(rules: rules)
                }
                .labelsHidden()
                .frame(width: 64)
                Text(units.title).foregroundStyle(.secondary)
                Button {
                    document.setLayerClearance(layer, nil)
                } label: {
                    Image(systemName: "arrow.uturn.backward")
                }
                .buttonStyle(.borderless)
                // Keeps its place when hidden, so the rows line up.
                .opacity(style.clearance == nil ? 0 : 1)
                .disabled(style.clearance == nil)
                .help("Use the DRC's min clearance (\(LengthField.format(rules.minClearance, units: units, precise: true)) \(units.title))")
            }
            .padding(.leading, 12)
            .help("How far the Plane's copper keeps from pads and vias not on its net; "
                  + (style.clearance == nil ? "the DRC's min clearance until set" : "set for this Plane"))
        }
    }

    /// The Plane's net. A net the schematic no longer has stays listed, as
    /// "Missing net", until another is chosen.
    private func netPicker(_ layer: CopperLayer, choices: [(net: PlaneNet, title: String)]) -> some View {
        let current = document.board.style(of: layer).net
        let missing = current.map { net in !choices.contains { $0.net == net } } ?? false
        return Picker("\(layer.title) net", selection: Binding(
            get: { current },
            set: { document.setLayerNet(layer, $0) }
        )) {
            Text("None").tag(PlaneNet?.none)
            if missing, let current {
                Text("Missing net").tag(Optional(current))
            }
            ForEach(choices, id: \.net) { choice in
                Text(choice.title).tag(Optional(choice.net))
            }
        }
        .pickerStyle(.menu)
        .labelsHidden()
        // Shrinks (a long signal net truncates) before the name does.
        .layoutPriority(-1)
    }
}

// MARK: - Text

/// The selected board text: its words, side, layer, font size, position
/// and rotation. ⌘⌫ or the Delete tool deletes it (`ContentView`).
struct TextPanel: View {
    let document: SchematicDocument
    let text: BoardText

    @State private var words = ""
    @State private var size = ""
    @FocusState private var focus: Field?

    private enum Field { case words, size }

    private var units: BoardUnits { document.board.units }

    var body: some View {
        Section {
            Text("Text")
                .font(.title2.bold())
            TextField("Text", text: $words, axis: .vertical)
                .font(.body.monospaced())
                .lineLimit(1...6)
                .focused($focus, equals: .words)
                .onSubmit(commitWords)
        }

        Section {
            Picker("Side", selection: Binding(
                get: { text.side },
                set: { document.setTextSide(text.id, $0) }
            )) {
                ForEach(BoardSide.allCases, id: \.self) { side in
                    Text(side.title).tag(side)
                }
            }
            .pickerStyle(.segmented)
            .disabled(document.board.layers == 1)
            Picker("Layer", selection: Binding(
                get: { text.layer },
                set: { document.setTextLayer(text.id, $0) }
            )) {
                ForEach(TextLayer.allCases, id: \.self) { layer in
                    Text(layer.title).tag(layer)
                }
            }
            .pickerStyle(.segmented)
            LabeledContent("Font Size") {
                HStack(spacing: 4) {
                    TextField("Font Size", text: $size)
                        .labelsHidden()
                        .multilineTextAlignment(.trailing)
                        .focused($focus, equals: .size)
                        .onSubmit(commitSize)
                    Text("mil")
                        .foregroundStyle(.secondary)
                    Stepper("Font Size", value: Binding(
                        get: { text.size },
                        set: { document.setTextSize(text.id, $0); showSize() }
                    ), in: BoardText.sizeRange, step: 5)
                    .labelsHidden()
                }
            }
        }

        Section("Position") {
            LockToggle(document: document, item: .text(text.id))
            Group {
                LengthField(title: "X", millimetres: text.anchor.x, units: units) {
                    move(to: CGPoint(x: $0, y: text.anchor.y)).x
                }
                LengthField(title: "Y", millimetres: text.anchor.y, units: units) {
                    move(to: CGPoint(x: text.anchor.x, y: $0)).y
                }
                Picker("Rotation", selection: Binding(
                    get: { text.rotation },
                    set: { document.setTextRotation(text.id, $0) }
                )) {
                    ForEach(BoardPlacement.rotations, id: \.self) { rotation in
                        Text("\(rotation)°").tag(rotation)
                    }
                }
                .pickerStyle(.segmented)
            }
            .disabled(text.locked)
        }

        .onAppear {
            words = text.text
            showSize()
        }
        // Undo and redo change the text from outside the fields (plan-31).
        .onChange(of: text.text) { _, value in words = value }
        .onChange(of: text.size) { _, _ in showSize() }
        .onChange(of: focus) { old, _ in
            switch old {
            case .words?: commitWords()
            case .size?: commitSize()
            case nil: break
            }
        }
    }

    private func current() -> BoardText? {
        document.texts.first { $0.id == text.id }
    }

    /// An empty value is refused by the document; the field shows the kept text.
    private func commitWords() {
        document.setText(text.id, words)
        words = current()?.text ?? words
    }

    private func commitSize() {
        if let value = Int(size.trimmingCharacters(in: .whitespaces)) {
            document.setTextSize(text.id, value)
        }
        showSize()
    }

    private func showSize() {
        size = String(current()?.size ?? text.size)
    }

    /// Moves the anchor (snapped) and returns where it ended up.
    private func move(to point: CGPoint) -> CGPoint {
        document.moveText(text.id, to: point)
        return current()?.anchor ?? point
    }
}

// MARK: - Via

/// The selected via: where it is, its hole and its metal, the ring of metal
/// those leave, and the traces it joins. ⌘⌫ or the Delete tool deletes it.
struct ViaPanel: View {
    let document: SchematicDocument
    let via: BoardVia

    var body: some View {
        let units = document.board.units
        Section("Via") {
            LockToggle(document: document, item: .via(via.id))
            Group {
                LengthField(title: "X", millimetres: via.centre.x, units: units, precise: true) {
                    document.setViaCentre(via.id, CGPoint(x: $0, y: via.centre.y))
                    return current.centre.x
                }
                LengthField(title: "Y", millimetres: via.centre.y, units: units, precise: true) {
                    document.setViaCentre(via.id, CGPoint(x: via.centre.x, y: $0))
                    return current.centre.y
                }
            }
            .disabled(via.locked)
            LengthField(title: "Hole", millimetres: via.drill, units: units, precise: true) {
                document.setViaDrill(via.id, $0)
                return current.drill
            }
            .help("The drill diameter")
            LengthField(title: "Metal", millimetres: via.diameter, units: units, precise: true) {
                document.setViaDiameter(via.id, $0)
                return current.diameter
            }
            .help("The copper's outer diameter, at least 0.1 mm more than the hole")
            LabeledContent("Ring") {
                Text(LengthField.format(via.ring, units: units, precise: true) + " " + units.title)
            }
            .help("The copper left round the hole, each side")
            LabeledContent("Layers", value: "Through, all \(document.board.layers)")
            Toggle("Tented on Top", isOn: Binding(get: { via.tentedTop },
                                                  set: { document.setViaTenting(via.id, .top, $0) }))
                .help("Covered by solder mask. Off leaves the via open on that side, for probing or soldering")
            Toggle("Tented on Bottom", isOn: Binding(get: { via.tentedBottom },
                                                     set: { document.setViaTenting(via.id, .bottom, $0) }))
                .help("Covered by solder mask. Off leaves the via open on that side, for probing or soldering")
            NetAndClassRows(document: document) { document.net(ofVia: via.id, in: $0) }
        }
        Section("Connections") {
            let joined = document.traces(touching: via)
            if joined.isEmpty {
                Text("No traces").foregroundStyle(.secondary)
            } else {
                let layers = document.board.copperLayers.filter { layer in joined.contains { $0.layer == layer } }
                Text("\(joined.count) \(joined.count == 1 ? "trace" : "traces") on "
                     + layers.map(\.title).joined(separator: ", "))
            }
        }
    }

    /// The via as the document now has it, after an edit clamped it.
    private var current: BoardVia {
        document.vias.first { $0.id == via.id } ?? via
    }
}

// MARK: - Hole

/// The selected drilled hole: where it is, its size (a screw size or
/// Custom), whether it is plated, and a plated hole's land, net and the
/// traces it joins. ⌘⌫ or the Delete tool deletes it.
struct HolePanel: View {
    let document: SchematicDocument
    let hole: BoardHole

    var body: some View {
        let units = document.board.units
        Section("Hole") {
            LockToggle(document: document, item: .hole(hole.id))
            Group {
                LengthField(title: "X", millimetres: hole.centre.x, units: units, precise: true) {
                    document.setHoleCentre(hole.id, CGPoint(x: $0, y: hole.centre.y))
                    return current.centre.x
                }
                LengthField(title: "Y", millimetres: hole.centre.y, units: units, precise: true) {
                    document.setHoleCentre(hole.id, CGPoint(x: hole.centre.x, y: $0))
                    return current.centre.y
                }
            }
            .disabled(hole.locked)
            Picker("Size", selection: Binding(get: { hole.size }, set: { document.setHoleSize(hole.id, $0) })) {
                ForEach(HoleSize.allCases, id: \.self) { size in
                    Text(size.title).tag(size)
                }
            }
            .help("A screw size sets the hole and its land; Custom sets them by hand")
            LengthField(title: "Drill", millimetres: hole.drill, units: units, precise: true) {
                document.setHoleDrill(hole.id, $0)
                return current.drill
            }
            .help("The hole's diameter")
            Toggle("Plated", isOn: Binding(get: { hole.plated }, set: { document.setHolePlated(hole.id, $0) }))
                .help("A plated hole has a copper land on every layer that traces join and a net")
        }
        if hole.plated {
            Section("Copper") {
                LengthField(title: "Land", millimetres: hole.diameter, units: units, precise: true) {
                    document.setHoleDiameter(hole.id, $0)
                    return current.diameter
                }
                .help("The copper's outer diameter, at least 0.2 mm more than the hole")
                LabeledContent("Ring") {
                    Text(LengthField.format(hole.ring, units: units, precise: true) + " " + units.title)
                }
                .help("The copper left round the hole, each side")
                netPicker
                let joined = document.traces(touching: hole)
                LabeledContent("Traces") {
                    Text(joined.isEmpty ? "None" : "\(joined.count)")
                }
                .help("Traces join the land at four points: right, below, left and above the hole")
            }
        }
    }

    /// The net: None, GND, a power rail or a signal net, as a Plane's. A
    /// net the schematic no longer has stays listed, as "Missing net",
    /// until another is chosen.
    private var netPicker: some View {
        let choices = document.planeNetChoices()
        let missing = hole.net.map { net in !choices.contains { $0.net == net } } ?? false
        return Picker("Net", selection: Binding(get: { hole.net }, set: { document.setHoleNet(hole.id, $0) })) {
            Text("None").tag(PlaneNet?.none)
            if missing, let net = hole.net {
                Text("Missing net").tag(Optional(net))
            }
            ForEach(choices, id: \.net) { choice in
                Text(choice.title).tag(Optional(choice.net))
            }
        }
        .help("The net the hole's copper is on")
    }

    /// The hole as the document now has it, after an edit clamped it.
    private var current: BoardHole {
        document.holes.first { $0.id == hole.id } ?? hole
    }
}

// MARK: - Trace

/// The selected trace: width, layer and the pads its joined run touches.
/// ⌘⌫ or the Delete tool deletes it (`ContentView`). Width is in mil
/// whatever the board units, as text size is.
struct TracePanel: View {
    let document: SchematicDocument
    let trace: BoardTrace

    @State private var width = ""
    @State private var calculating = false
    @FocusState private var isWidthFocused: Bool

    /// The layers the Layer picker offers: never a Plane.
    static func layers(for board: BoardSettings) -> [CopperLayer] { board.traceLayers }

    var body: some View {
        Section {
            Text("Trace")
                .font(.title2.bold())
        }

        Section {
            LabeledContent("Width") {
                HStack(spacing: 4) {
                    TextField("Width", text: $width)
                        .labelsHidden()
                        .multilineTextAlignment(.trailing)
                        .focused($isWidthFocused)
                        .onSubmit(commitWidth)
                    Text("mil")
                        .foregroundStyle(.secondary)
                    Stepper("Width", value: Binding(
                        get: { trace.width },
                        set: { document.setTraceWidth(trace.id, $0); showWidth() }
                    ), in: BoardTrace.widthRange, step: 5)
                    .labelsHidden()
                }
            }
            Picker("Layer", selection: Binding(
                get: { trace.layer },
                set: { document.setTraceLayer(trace.id, $0) }
            )) {
                ForEach(Self.layers(for: document.board), id: \.self) { layer in
                    Text(layer.title).tag(layer)
                }
            }
            .pickerStyle(.menu)
            LockToggle(document: document, item: .trace(trace.id))
            NetAndClassRows(document: document) { document.net(ofTrace: trace.id, in: $0) }
        }

        Section("Estimates") {
            let copper = document.connectivity()
            let net = document.net(ofTrace: trace.id, in: copper)
            let gap = EstimateText.pairGapMM(board: document.board, net: net.flatMap { copper.planeNet(ofNet: $0) })
            ForEach(EstimateText.rows(board: document.board, layer: trace.layer, widthMil: trace.width,
                                      pairGapMM: gap)) { row in
                LabeledContent(row.title, value: row.value).help(row.help)
            }
            Button("Calculate Width…") { calculating = true }
                .help("The width that gives a target impedance or carries a current")
                .sheet(isPresented: $calculating) {
                    WidthCalculatorSheet(document: document, layers: document.board.traceLayers, layer: trace.layer,
                                         gapMM: gap ?? NetGroup.defaultGap) { document.setTraceWidth(trace.id, $0) }
                }
        }

        Section("Connects") {
            let run = document.connectedRun(of: trace.id)
            let pads = document.pads(touching: run)
            Text(pads.isEmpty ? "Nothing" : pads.joined(separator: ", "))
                .textSelection(.enabled)
            if run.count > 1 {
                Text("Joined to \(run.count - 1) other \(run.count == 2 ? "trace" : "traces")")
                    .foregroundStyle(.secondary)
            }
        }

        .onAppear(perform: showWidth)
        // Undo and redo change the width from outside the field (plan-31).
        .onChange(of: trace.width) { _, _ in showWidth() }
        .onChange(of: isWidthFocused) { wasFocused, _ in
            if wasFocused { commitWidth() }
        }
    }

    /// A typed width is clamped by the document; the field shows the result.
    private func commitWidth() {
        if let value = Int(width.trimmingCharacters(in: .whitespaces)) {
            document.setTraceWidth(trace.id, value)
        }
        showWidth()
    }

    private func showWidth() {
        width = String(document.traces.first { $0.id == trace.id }?.width ?? trace.width)
    }
}

/// A selected trace's or via's net (read-only) and that net's class, which
/// a menu sets. The class belongs to the net, so choosing one moves every
/// trace, via and pad of the net into it, and the net's traces and vias take
/// the class's width and via size.
struct NetAndClassRows: View {
    let document: SchematicDocument
    let net: (BoardConnectivity) -> Int?

    var body: some View {
        let copper = document.connectivity()
        let netID = net(copper)
        let names = document.netAndClassNames(ofNet: netID, in: copper)
        let key = netID.flatMap { copper.netKeys[$0] }
        LabeledContent("Net", value: names.net)
        Picker("Class", selection: Binding<UUID?>(
            get: { netID.flatMap { document.netClass(ofNet: $0, in: copper) }?.id },
            set: { if let key { document.assignNet(key, to: $0) } }
        )) {
            Text("Default").tag(UUID?.none)
            ForEach(document.board.netClasses) { netClass in
                Text(netClass.name).tag(Optional(netClass.id))
            }
        }
        .pickerStyle(.menu)
        .disabled(key == nil)
        .help("The net's class: its trace width, clearance and via. Every trace and via on the net changes to match.")
    }
}

// MARK: - Footprint

/// The selected part: name, reference, side (Top or Bottom), origin and
/// position, and marks.
struct FootprintPanel: View {
    let document: SchematicDocument
    let chip: PlacedChip

    private var placement: BoardPlacement { chip.board! }
    private var units: BoardUnits { document.board.units }

    var body: some View {
        Section {
            VStack(alignment: .leading, spacing: 4) {
                PartHeader(chip: chip)
                if let footprint = chip.footprint {
                    Text("\(footprint.id) — \(footprint.description)")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            .textSelection(.enabled)
        }

        ReferenceSection(document: document, chip: chip, page: .pcb)

        if chip.part.connectorFootprint != nil {
            Section("Footprint") {
                Picker("Style", selection: Binding(
                    get: { placement.style },
                    set: { document.setFootprintStyle(chip.id, $0) }
                )) {
                    ForEach(FootprintStyle.allCases, id: \.self) { style in
                        Text(style.title).tag(style)
                    }
                }
                .pickerStyle(.segmented)
            }
        }

        Section("Position") {
            Picker("Layer", selection: Binding(
                get: { placement.side },
                set: { document.setFootprintSide(chip.id, $0) }
            )) {
                ForEach(BoardSide.allCases, id: \.self) { side in
                    Text(side.title).tag(side)
                }
            }
            .pickerStyle(.segmented)
            .disabled(document.board.layers == 1)
            Picker("Origin", selection: Binding(
                get: { placement.origin },
                set: { document.setFootprintOrigin(chip.id, $0) }
            )) {
                ForEach(FootprintOrigin.allCases, id: \.self) { origin in
                    Text(origin.title).tag(origin)
                }
            }
            .pickerStyle(.menu)
            LockToggle(document: document, item: .footprint(chip.id))
            if let point = chip.boardOrigin {
                Group {
                    LengthField(title: "X", millimetres: point.x, units: units) {
                        move(to: CGPoint(x: $0, y: point.y)).x
                    }
                    LengthField(title: "Y", millimetres: point.y, units: units) {
                        move(to: CGPoint(x: point.x, y: $0)).y
                    }
                }
                .disabled(placement.locked)
            }
            LabeledContent("Rotation", value: "\(placement.rotation)°")
        }

        MarksSection(document: document, ids: [chip.id])
    }

    /// Moves the origin point (snapped) and returns where it ended up.
    private func move(to point: CGPoint) -> CGPoint {
        document.moveFootprintOrigin(chip.id, to: point)
        return document.chips.first { $0.id == chip.id }?.boardOrigin ?? point
    }
}
