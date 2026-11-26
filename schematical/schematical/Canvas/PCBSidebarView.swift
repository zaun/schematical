import SwiftUI

/// The PCB page's inspector. It follows the selection: the board's settings
/// with nothing selected (a Grid section, then Board, then Layers with one
/// row per copper layer, then a Board Details… button for the rest:
/// `BoardDetailsSheet`), the via's with only a via selected, the trace's with
/// only a trace selected, the footprint's with only a footprint selected,
/// the text's with only a board text selected, and a read-only
/// `Selection (N)` list for several items (plan-15). A mm | in switch at the
/// top sets the units of every length field.
struct PCBSidebarView: View {
    @Bindable var document: SchematicDocument
    /// The net the Nets section highlights on the canvas (window state).
    var highlightedNet: Binding<PlaneNet?>
    /// Routes a net from its row (Route Net).
    var onRoute: (RoutingOptions.Scope) -> Void

    /// The Board Details sheet is open.
    @State private var isDetailsOpen: Bool

    init(document: SchematicDocument, highlightedNet: Binding<PlaneNet?> = .constant(nil),
         onRoute: @escaping (RoutingOptions.Scope) -> Void = { _ in }) {
        self.document = document
        self.highlightedNet = highlightedNet
        self.onRoute = onRoute
        _isDetailsOpen = State(initialValue: false)
    }

    /// What the inspector shows.
    enum Content: Equatable {
        case board
        case footprint(UUID)
        case text(UUID)
        case trace(UUID)
        case via(UUID)
        case hole(UUID)
        case pour(UUID)
        case keepout(UUID)
        case cutout(UUID)
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
        if let id = document.selectedHoleID, document.holes.contains(where: { $0.id == id }) {
            return .hole(id)
        }
        if let id = document.selectedPourID, document.pours.contains(where: { $0.id == id }) {
            return .pour(id)
        }
        if let id = document.selectedKeepoutID, document.keepouts.contains(where: { $0.id == id }) {
            return .keepout(id)
        }
        if let id = document.selectedCutoutID, document.cutouts.contains(where: { $0.id == id }) {
            return .cutout(id)
        }
        return document.boardSelection.isEmpty ? .board : .selection
    }

    /// The ids of the selected texts when the selection is only texts (two
    /// or more); empty otherwise.
    static func textIDs(of document: SchematicDocument) -> [UUID] {
        var ids: [UUID] = []
        for item in document.boardSelection.items {
            guard case let .text(id) = item, document.texts.contains(where: { $0.id == id }) else { return [] }
            ids.append(id)
        }
        return ids
    }

    /// The ids of the selected footprints that are on the board, in selection order.
    static func footprintIDs(of document: SchematicDocument) -> [UUID] {
        document.boardSelection.items.compactMap { item -> UUID? in
            if case let .footprint(id) = item, document.chips.contains(where: { $0.id == id && $0.board != nil }) {
                return id
            }
            return nil
        }
    }

    /// The one value every selected item shares, or nil when they differ
    /// (or there are none): a picker then shows no segment selected.
    static func shared<Value: Hashable>(_ values: Set<Value>) -> Value? {
        values.count == 1 ? values.first : nil
    }

    /// A size or width typed in mil: a whole number, spaces round it allowed; nil otherwise.
    static func mil(_ text: String) -> Int? {
        Int(text.trimmingCharacters(in: .whitespaces))
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
            case let .hole(id):
                if let hole = document.holes.first(where: { $0.id == id }) {
                    HolePanel(document: document, hole: hole)
                        .id(hole.id)
                }
            case let .pour(id):
                if let pour = document.pours.first(where: { $0.id == id }) {
                    PourPanel(document: document, pour: pour)
                        .id(pour.id)
                }
            case let .keepout(id):
                if let keepout = document.keepouts.first(where: { $0.id == id }) {
                    KeepoutPanel(document: document, keepout: keepout)
                        .id(keepout.id)
                }
            case let .cutout(id):
                if let cutout = document.cutouts.first(where: { $0.id == id }) {
                    CutoutPanel(document: document, cutout: cutout)
                        .id(cutout.id)
                }
            case .selection:
                let textIDs = Self.textIDs(of: document)
                if !textIDs.isEmpty { MultiTextSection(document: document, ids: textIDs) }
                MultiPositionSection(document: document, showsSide: textIDs.isEmpty)
                let footprints = Self.footprintIDs(of: document)
                if !footprints.isEmpty {
                    MarksSection(document: document, ids: footprints)
                }
                SelectionListSection(
                    rows: SelectionRows.board(document),
                    onSelect: { document.select($0) },
                    onClear: { document.clearBoardSelection() }
                )
            case .board:
                GridSection(document: document)
                BoardPanel(document: document)
                LayersSection(document: document)
                Section {
                    Button("Board Details…") { isDetailsOpen = true }
                        .frame(maxWidth: .infinity)
                        .help("Stackup, Planes, nets, net classes and net groups"
                              + (BoardDetailsSheet.tabs(for: document.board).contains(.card) ? ", and the card's settings" : ""))
                }
            }
        }
        .formStyle(.grouped)
        .sheet(isPresented: $isDetailsOpen) {
            BoardDetailsSheet(document: document, highlightedNet: highlightedNet, onRoute: onRoute,
                              onClose: { isDetailsOpen = false })
        }
    }
}

// MARK: - Position of several items

/// The Position section for a selection of several items: the Layer (Top or
/// Bottom) of its footprints and board texts, and Lock Position for every
/// item. A change applies to all of them as one undo step. When they
/// disagree, the Layer shows no segment selected and Lock Position is off;
/// turning it on locks them all.
struct MultiPositionSection: View {
    let document: SchematicDocument
    /// False when the Text section (all texts) already offers the side.
    let showsSide: Bool

    init(document: SchematicDocument, showsSide: Bool = true) {
        self.document = document
        self.showsSide = showsSide
    }

    var body: some View {
        let items = Array(document.boardSelection.items)
        let sides = Set(items.compactMap { Self.side(of: $0, in: document) })
        Section("Position") {
            if showsSide, !sides.isEmpty {
                Picker("Layer", selection: Binding<BoardSide?>(
                    get: { PCBSidebarView.shared(sides) },
                    set: { if let side = $0 { document.setSide(of: items, side) } }
                )) {
                    ForEach(BoardSide.allCases, id: \.self) { side in
                        Text(side.title).tag(Optional(side))
                    }
                }
                .pickerStyle(.segmented)
                .disabled(document.board.layers == 1)
            }
            Toggle("Lock Position", isOn: Binding(
                get: { !items.isEmpty && items.allSatisfy { document.isLocked($0) } },
                set: { document.setLocked(items, $0) }
            ))
            .help("A locked item can't be moved")
        }
    }

    /// The side a footprint or board text is on; nil for any other item.
    static func side(of item: BoardItem, in document: SchematicDocument) -> BoardSide? {
        switch item {
        case let .footprint(id): document.chips.first { $0.id == id }?.board?.side
        case let .text(id): document.texts.first { $0.id == id }?.side
        default: nil
        }
    }
}

// MARK: - Several texts

/// The Text section for a selection of only board texts: Side, Layer and
/// Font Size, each applied to all of them as one undo step. When they
/// disagree, a Side or Layer shows no segment selected and the Font Size
/// field shows "Mixed".
struct MultiTextSection: View {
    let document: SchematicDocument
    let ids: [UUID]

    @State private var size: String
    @FocusState private var sizeFocused: Bool

    init(document: SchematicDocument, ids: [UUID]) {
        self.document = document
        self.ids = ids
        _size = State(initialValue: "")
    }

    private var texts: [BoardText] { document.texts.filter { ids.contains($0.id) } }

    var body: some View {
        let texts = texts
        let sides = Set(texts.map(\.side))
        let layers = Set(texts.map(\.layer))
        let sizes = Set(texts.map(\.size))
        Section("Text") {
            Picker("Side", selection: Binding<BoardSide?>(
                get: { PCBSidebarView.shared(sides) },
                set: { if let side = $0 { document.setSide(of: ids.map { BoardItem.text($0) }, side) } }
            )) {
                ForEach(BoardSide.allCases, id: \.self) { side in
                    Text(side.title).tag(Optional(side))
                }
            }
            .pickerStyle(.segmented)
            .disabled(document.board.layers == 1)
            Picker("Layer", selection: Binding<TextLayer?>(
                get: { PCBSidebarView.shared(layers) },
                set: { if let layer = $0 { document.setTextLayer(ids, layer) } }
            )) {
                ForEach(TextLayer.allCases, id: \.self) { layer in
                    Text(layer.title).tag(Optional(layer))
                }
            }
            .pickerStyle(.segmented)
            LabeledContent("Font Size") {
                HStack(spacing: 4) {
                    TextField("Font Size", text: $size, prompt: Text("Mixed"))
                        .labelsHidden()
                        .multilineTextAlignment(.trailing)
                        .focused($sizeFocused)
                        .onSubmit(commitSize)
                    Text("mil")
                        .foregroundStyle(.secondary)
                    Stepper("Font Size", value: Binding(
                        get: { PCBSidebarView.shared(sizes) ?? BoardText.defaultSize },
                        set: { document.setTextSize(ids, $0); showSize() }
                    ), in: BoardText.sizeRange, step: 5)
                    .labelsHidden()
                }
            }
        }
        .onAppear { showSize() }
        .onChange(of: sizes) { _, _ in showSize() }
        .onChange(of: sizeFocused) { _, focused in if !focused { commitSize() } }
    }

    private func commitSize() {
        if let value = PCBSidebarView.mil(size) {
            document.setTextSize(ids, value)
        }
        showSize()
    }

    /// The shared size, or empty (the "Mixed" prompt) when they differ.
    private func showSize() {
        size = PCBSidebarView.shared(Set(texts.map(\.size))).map(String.init) ?? ""
    }
}

// MARK: - Marks

/// The Marks section of one or more selected footprints: where the bounding
/// box, pin 1 mark and reference are drawn. A change applies to every
/// footprint in `ids` as one undo step. When they disagree, no segment is
/// shown selected for that mark.
struct MarksSection: View {
    let document: SchematicDocument
    let ids: [UUID]

    private var placements: [BoardPlacement] {
        document.chips.filter { ids.contains($0.id) }.compactMap(\.board)
    }

    var body: some View {
        Section("Marks") {
            markPicker("Bounding Box", \.outline) { document.setFootprintMarks(ids, outline: $0) }
            markPicker("Pin 1", \.pin1) { document.setFootprintMarks(ids, pin1: $0) }
            markPicker("Reference", \.reference) { document.setFootprintMarks(ids, reference: $0) }
            if placements.contains(where: { $0.referenceOffset != .zero }) {
                Button("Reset Reference Position") {
                    document.recordEdit(.pcb, "Reset Reference") {
                        for id in ids { document.resetReference(id) }
                    }
                }
            }
        }
    }

    private func markPicker(_ title: String, _ mark: KeyPath<BoardPlacement, MarkLayer>,
                            set: @escaping (MarkLayer) -> Void) -> some View {
        let layers = Set(placements.map { $0[keyPath: mark] })
        return Picker(title, selection: Binding<MarkLayer?>(
            get: { PCBSidebarView.shared(layers) },
            set: { if let layer = $0 { set(layer) } }
        )) {
            ForEach(MarkLayer.allCases, id: \.self) { layer in
                Text(layer.title).tag(Optional(layer))
            }
        }
        .pickerStyle(.segmented)
    }
}

// MARK: - Lock Position

/// The Lock Position switch of a selected PCB item. A locked item can't be
/// dragged, and its X, Y and rotation can't be changed. Off by default.
struct LockToggle: View {
    let document: SchematicDocument
    let item: BoardItem

    var body: some View {
        Toggle("Lock Position", isOn: Binding(
            get: { document.isLocked(item) },
            set: { document.setLocked(item, $0) }
        ))
        .help("A locked item can't be moved")
    }
}

// MARK: - Grid

/// The snap grids: the component grid, which footprints and texts snap to
/// and the canvas dots, and the trace grid trace ends snap to, which has a
/// point between each two component-grid points. Only the component grid
/// is set; the trace grid follows it.
struct GridSection: View {
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

// MARK: - Card options

/// The type menus' items: Custom and Custom Card, then the standard
/// boards in `boards.json` order, a section per group.
struct BoardTypeMenuItems: View {
    var body: some View {
        let types = BoardType.allCases
        ForEach(types.filter { $0.group == nil }, id: \.self) { Text($0.title).tag($0) }
        let groups = types.compactMap(\.group).reduce(into: [String]()) { if !$0.contains($1) { $0.append($1) } }
        ForEach(groups, id: \.self) { group in
            Section(group) {
                ForEach(types.filter { $0.group == group }, id: \.self) { Text($0.title).tag($0) }
            }
        }
    }
}

/// The settings a board type has in `boards.json` (PCI Express lanes, M.2
/// length and key, …), each a picker or toggle. Used for the board and,
/// with `bus`, for a bus's type (only the settings that change the pins).
/// `options` reads them, `set` stores them.
struct CardOptionsSection: View {
    let type: BoardType
    var bus = false
    let options: () -> CardOptions
    let set: (CardOptions) -> Void

    var body: some View {
        Section(type.optionsTitle) {
            if let definition = type.definition {
                ForEach(definition.options.filter { (!bus || $0.onBus) && definition.shows($0, with: options()) },
                        id: \.id) { option in
                    control(option, of: definition)
                }
            }
        }
    }

    @ViewBuilder
    private func control(_ option: BoardOption, of definition: BoardDefinition) -> some View {
        let value = Binding<BoardValue>(get: { definition.resolved(options())[option.id] ?? option.default },
                                        set: { value in
                                            var next = options()
                                            next[option.id] = value
                                            set(next)
                                        })
        switch option.style {
        case .toggle:
            Toggle(option.title, isOn: Binding(get: { value.wrappedValue == .bool(true) },
                                               set: { value.wrappedValue = .bool($0) }))
                .help(option.help ?? "")
        case .segmented, .menu:
            let picker = Picker(option.title, selection: value) {
                ForEach(option.choices, id: \.value) { Text($0.title).tag($0.value) }
            }
            .help(option.help ?? "")
            if option.style == .menu {
                picker.pickerStyle(.menu)
            } else {
                picker.pickerStyle(.segmented)
            }
        }
    }
}

// MARK: - Custom card

/// A custom card's edge connector: how many fingers, their size and
/// spacing, and the tab they sit on. Each change is clamped to fit
/// (`CustomCard.clamped`) and grows the board to hold the tab.
struct CustomCardSection: View {
    @Bindable var document: SchematicDocument

    var body: some View {
        let card = document.board.customCard
        let units = document.board.units
        Section("Card Edge") {
            LabeledContent("Pads per side") {
                HStack {
                    TextField("Pads per side", value: Binding(
                        get: { card.pads },
                        set: { change { $0.pads = $1 }($0) }
                    ), format: .number)
                    .multilineTextAlignment(.trailing)
                    .labelsHidden()
                    Stepper("", value: Binding(get: { card.pads }, set: { change { $0.pads = $1 }($0) }),
                            in: CustomCard.padRange)
                        .labelsHidden()
                }
            }
            Toggle("Both sides", isOn: Binding(get: { card.bothSides }, set: { value in
                var next = card
                next.bothSides = value
                document.setCustomCard(next)
            }))
            .help("Fingers on Bottom too (B1, B2, …), under the Top ones (A1, A2, …)")
            length("Pitch", \.pitch, help: "Finger centre to centre")
            length("Pad width", \.padWidth)
            length("Pad length", \.padLength)
            length("Pad offset", \.padOffset, help: "From the card's bottom edge up to the fingers")
            length("Depth", \.depth, help: "How far the tab reaches below the card")
            length("Indent", \.indent, help: "From the card's left edge to the tab")
            length("Margin", \.margin, help: "From each end of the tab to its outermost finger's centre")
            length("Chamfer", \.chamfer, help: "The tab's lower corners")
            LabeledContent("Tab length") {
                Text(LengthField.format(card.tabLength, units: units, precise: true) + " " + units.title)
            }
        }
    }

    /// A length setting of the card; the field shows what the document kept.
    private func length(_ title: String, _ field: WritableKeyPath<CustomCard, Double>,
                        help: String = "") -> some View {
        LengthField(title: title, millimetres: document.board.customCard[keyPath: field],
                    units: document.board.units, precise: true) { typed in
            var next = document.board.customCard
            next[keyPath: field] = typed
            document.setCustomCard(next)
            return document.board.customCard[keyPath: field]
        }
        .help(help)
    }

    /// An integer setting of the card.
    private func change(_ set: @escaping (inout CustomCard, Int) -> Void) -> (Int) -> Void {
        { value in
            var next = document.board.customCard
            set(&next, value)
            document.setCustomCard(next)
        }
    }
}

// MARK: - Board
