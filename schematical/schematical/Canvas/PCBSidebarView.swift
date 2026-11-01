import SwiftUI

/// The PCB page's inspector. It follows the selection: the board's settings
/// with nothing selected, the footprint's with a part selected, the text's
/// with a board text selected. A mm | in switch at the top sets the units of
/// every length field.
struct PCBSidebarView: View {
    @Bindable var document: SchematicDocument

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

            if let id = document.selectedChipID,
               let chip = document.chips.first(where: { $0.id == id }), chip.board != nil {
                FootprintPanel(document: document, chip: chip)
                    // Fresh field state for each part.
                    .id(chip.id)
            } else if let id = document.selectedTextID,
                      let text = document.texts.first(where: { $0.id == id }) {
                TextPanel(document: document, text: text)
                    .id(text.id)
            } else {
                BoardPanel(document: document)
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
            Picker("Layers", selection: $document.board.layers) {
                ForEach(BoardSettings.layerCounts, id: \.self) { count in
                    Text(count == 1 ? "1 layer" : "\(count) layers").tag(count)
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

// MARK: - Text

/// The selected board text: its words, layer, font size, position and
/// rotation, and Delete.
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

/// The selected part: name, reference, origin and position, and marks.
private struct FootprintPanel: View {
    let document: SchematicDocument
    let chip: PlacedChip

    @State private var prefix = ""
    @State private var number = ""
    @State private var referenceError: String?
    @FocusState private var referenceFocus: ReferenceField?

    private enum ReferenceField { case prefix, number }

    private var placement: BoardPlacement { chip.board! }
    private var units: BoardUnits { document.board.units }

    var body: some View {
        Section {
            VStack(alignment: .leading, spacing: 4) {
                Text(chip.reference)
                    .font(.title2.bold())
                Text(chip.part.name)
                    .font(.headline)
                if !chip.part.description.isEmpty {
                    Text(chip.part.description)
                        .font(.callout)
                        .foregroundStyle(.secondary)
                }
                if let footprint = chip.part.footprint {
                    Text("\(footprint.id) — \(footprint.description)")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            .textSelection(.enabled)
        }

        Section("Reference") {
            HStack {
                TextField("Prefix", text: $prefix)
                    .focused($referenceFocus, equals: .prefix)
                TextField("Number", text: $number)
                    .focused($referenceFocus, equals: .number)
            }
            .labelsHidden()
            .onSubmit(commitReference)
            .onChange(of: referenceFocus) { old, new in
                // Leaving the pair of fields commits; moving between them doesn't.
                if old != nil, new == nil { commitReference() }
            }
            if let referenceError {
                Text(referenceError)
                    .font(.caption)
                    .foregroundStyle(.red)
            }
        }
        .onAppear(perform: resetReference)

        Section("Position") {
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

    private func commitReference() {
        let trimmed = number.trimmingCharacters(in: .whitespaces)
        referenceError = document.renameChip(chip.id, prefix: prefix.trimmingCharacters(in: .whitespaces),
                                             number: Int(trimmed))
        resetReference()
    }

    /// Puts the fields back to the part's reference as stored.
    private func resetReference() {
        let reference = document.chips.first { $0.id == chip.id }?.reference ?? chip.reference
        let parts = ReferenceParts(reference)
        prefix = parts.prefix
        number = parts.number.map(String.init) ?? ""
    }
}
