import SwiftUI

/// The PCB page's inspector. It follows the selection: the board's settings
/// with nothing selected (with a Layers section, one row per copper layer), the footprint's with only a footprint selected,
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
        case selection
    }

    static func content(for document: SchematicDocument) -> Content {
        if let id = document.selectedFootprintID, document.chips.contains(where: { $0.id == id && $0.board != nil }) {
            return .footprint(id)
        }
        if let id = document.selectedTextID, document.texts.contains(where: { $0.id == id }) {
            return .text(id)
        }
        return document.boardSelection.isEmpty ? .board : .selection
    }

    var body: some View {
        Form {
            Section {
                Picker("Units", selection: $document.board.units) {
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
            case .selection:
                SelectionListSection(
                    rows: SelectionRows.board(document),
                    onSelect: { document.select($0) },
                    onClear: { document.clearBoardSelection() }
                )
            case .board:
                BoardPanel(document: document)
                LayersSection(document: document)
            }
        }
        .formStyle(.grouped)
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
            document.board.color = colour
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
/// and rotation, and Delete.
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

        Section {
            // ⌘⌫, not ⌫ alone, so it can't fire while a field is being edited.
            Button("Delete Text", role: .destructive) {
                document.deleteText(text.id)
            }
            .keyboardShortcut(.delete, modifiers: .command)
        }
        .onAppear {
            words = text.text
            showSize()
        }
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

        ReferenceSection(document: document, chip: chip)

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
