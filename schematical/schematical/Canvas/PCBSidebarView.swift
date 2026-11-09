import SwiftUI

/// The PCB page's inspector. It follows the selection: the board's settings
/// with nothing selected (a Grid section, then Board, then Layers with one
/// row per copper layer), the via's with only a via selected, the trace's with
/// only a trace selected, the footprint's with only a footprint selected,
/// the text's with only a board text selected, and a read-only
/// `Selection (N)` list for several items (plan-15). A mm | in switch at the
/// top sets the units of every length field.
struct PCBSidebarView: View {
    @Bindable var document: SchematicDocument

    /// What the inspector shows.
    enum Content: Equatable {
        case board
        case footprint(UUID)
        case text(UUID)
        case trace(UUID)
        case via(UUID)
        case selection
    }

    static func content(for document: SchematicDocument) -> Content {
        if let id = document.selectedFootprintID, document.chips.contains(where: { $0.id == id && $0.board != nil }) {
            return .footprint(id)
        }
        if let id = document.selectedTextID, document.texts.contains(where: { $0.id == id }) {
            return .text(id)
        }
        if let id = document.selectedTraceID, document.traces.contains(where: { $0.id == id }) {
            return .trace(id)
        }
        if let id = document.selectedViaID, document.vias.contains(where: { $0.id == id }) {
            return .via(id)
        }
        return document.boardSelection.isEmpty ? .board : .selection
    }

    var body: some View {
        Form {
            Section {
                Picker("Units", selection: Binding(get: { document.board.units }, set: { document.setBoardUnits($0) })) {
                    ForEach(BoardUnits.allCases, id: \.self) { units in
                        Text(units.title).tag(units)
                    }
                }
                .pickerStyle(.segmented)
            }

            switch Self.content(for: document) {
            case let .footprint(id):
                if let chip = document.chips.first(where: { $0.id == id }) {
                    FootprintPanel(document: document, chip: chip)
                        // Fresh field state for each part.
                        .id(chip.id)
                }
            case let .text(id):
                if let text = document.texts.first(where: { $0.id == id }) {
                    TextPanel(document: document, text: text)
                        .id(text.id)
                }
            case let .trace(id):
                if let trace = document.traces.first(where: { $0.id == id }) {
                    TracePanel(document: document, trace: trace)
                        .id(trace.id)
                }
            case let .via(id):
                if let via = document.vias.first(where: { $0.id == id }) {
                    ViaPanel(document: document, via: via)
                        .id(via.id)
                }
            case .selection:
                SelectionListSection(
                    rows: SelectionRows.board(document),
                    onSelect: { document.select($0) },
                    onClear: { document.clearBoardSelection() }
                )
            case .board:
                GridSection(document: document)
                BoardPanel(document: document)
                LayersSection(document: document)
            }
        }
        .formStyle(.grouped)
    }
}

// MARK: - Grid

/// The snap grids: the component grid, which footprints and texts snap to
/// and the canvas dots, and the trace grid trace ends snap to, which has a
/// point between each two component-grid points. Only the component grid
/// is set; the trace grid follows it.
private struct GridSection: View {
    @Bindable var document: SchematicDocument

    var body: some View {
        Section("Grid") {
            LengthField(title: "Components", millimetres: document.board.grid, units: document.board.units,
                        precise: true) {
                document.setBoardGrid($0)
                return document.board.grid
            }
            LabeledContent("Traces") {
                Text(LengthField.format(document.board.traceGrid, units: document.board.units, precise: true)
                     + " " + document.board.units.title)
            }
            .help("Half the component grid: a point between each two component-grid points")
        }
    }
}

// MARK: - Board

/// Size, copper layers and colour of the board.
private struct BoardPanel: View {
    @Bindable var document: SchematicDocument

    var body: some View {
        Section("Board") {
            LengthField(title: "Width", millimetres: document.board.width, units: document.board.units) {
                document.setBoardSize(width: $0, height: document.board.height)
                return document.board.width
            }
            LengthField(title: "Height", millimetres: document.board.height, units: document.board.units) {
                document.setBoardSize(width: document.board.width, height: $0)
                return document.board.height
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
/// Wire.
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

// MARK: - Text

/// The selected board text: its words, side, layer, font size, position
/// and rotation. ⌘⌫ or the Delete tool deletes it (`ContentView`).
private struct TextPanel: View {
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
private struct ViaPanel: View {
    let document: SchematicDocument
    let via: BoardVia

    var body: some View {
        let units = document.board.units
        Section("Via") {
            LengthField(title: "X", millimetres: via.centre.x, units: units, precise: true) {
                document.setViaCentre(via.id, CGPoint(x: $0, y: via.centre.y))
                return current.centre.x
            }
            LengthField(title: "Y", millimetres: via.centre.y, units: units, precise: true) {
                document.setViaCentre(via.id, CGPoint(x: via.centre.x, y: $0))
                return current.centre.y
            }
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

// MARK: - Trace

/// The selected trace: width, layer and the pads its joined run touches.
/// ⌘⌫ or the Delete tool deletes it (`ContentView`). Width is in mil
/// whatever the board units, as text size is.
struct TracePanel: View {
    let document: SchematicDocument
    let trace: BoardTrace

    @State private var width = ""
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

// MARK: - Footprint

/// The selected part: name, reference, side (Top or Bottom), origin and
/// position, and marks.
private struct FootprintPanel: View {
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
            if let point = chip.boardOrigin {
                LengthField(title: "X", millimetres: point.x, units: units) {
                    move(to: CGPoint(x: $0, y: point.y)).x
                }
                LengthField(title: "Y", millimetres: point.y, units: units) {
                    move(to: CGPoint(x: point.x, y: $0)).y
                }
            }
            LabeledContent("Rotation", value: "\(placement.rotation)°")
        }

        Section("Marks") {
            markPicker("Bounding Box", placement.outline) { document.setFootprintMarks(chip.id, outline: $0) }
            markPicker("Pin 1", placement.pin1) { document.setFootprintMarks(chip.id, pin1: $0) }
            markPicker("Reference", placement.reference) { document.setFootprintMarks(chip.id, reference: $0) }
        }
    }

    private func markPicker(_ title: String, _ layer: MarkLayer, set: @escaping (MarkLayer) -> Void) -> some View {
        Picker(title, selection: Binding(get: { layer }, set: set)) {
            ForEach(MarkLayer.allCases, id: \.self) { layer in
                Text(layer.title).tag(layer)
            }
        }
        .pickerStyle(.segmented)
    }

    /// Moves the origin point (snapped) and returns where it ended up.
    private func move(to point: CGPoint) -> CGPoint {
        document.moveFootprintOrigin(chip.id, to: point)
        return document.chips.first { $0.id == chip.id }?.boardOrigin ?? point
    }
}
