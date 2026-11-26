import AppKit
import SwiftUI

/// The Schematic sidebar's Options for a placed part that has a choice of
/// color, resistance or footprint (the Generic LED, pull resistors): a Color
/// picker, an Ohms field and a Footprint picker. A header (plan-23) also
/// gets Rows, Pins per row and Gender; a test point (plan-57) gets Size and
/// Through hole in place of the Footprint picker. A change takes effect at once on the
/// canvas, the PCB and the file.
struct PartOptionsSection: View {
    /// Looks up a header's new size (`HeaderFamily.name(rows:perRow:)`).
    let catalog: PartCatalogStore
    let document: SchematicDocument
    let chip: PlacedChip

    /// A Pins per row menu item: `"8"`, or `"4 — removes 2 wires"` when
    /// that size would remove wires. `wiredPins` has one pin number per wire
    /// end on the header (a pin with two wires is listed twice); an end on a
    /// pin above `rows × n` is removed.
    static func pinsPerRowLabel(_ n: Int, rows: Int, wiredPins: [Int]) -> String {
        let removed = wiredPins.filter { $0 > rows * n }.count
        if removed == 0 { return "\(n)" }
        return "\(n) — removes \(removed) \(removed == 1 ? "wire" : "wires")"
    }

    /// The pin number (1-based) of every wire end on a chip.
    static func wiredPins(of id: UUID, in document: SchematicDocument) -> [Int] {
        document.wires.flatMap { [$0.start, $0.end] }.compactMap { end in
            if case let .pin(ref) = end, ref.chipID == id { return ref.index + 1 }
            return nil
        }
    }

    /// Whether `part` has anything for this section to show.
    static func applies(to part: CatalogPart) -> Bool {
        hasColorChoice(part) || !part.footprintChoices.isEmpty || !part.units.isEmpty
    }

    /// Whether the section has anything to show for a placed chip: what `applies(to: part)` says, or
    /// a socket that fits the chip (plan-51-I).
    static func applies(to chip: PlacedChip, in catalog: PartCatalogStore) -> Bool {
        applies(to: chip.part) || isSocketable(chip.part, in: catalog)
    }

    /// Whether a socket fits `part`: it has no footprint choices, and the catalog has a socket that holds its footprint.
    static func isSocketable(_ part: CatalogPart, in catalog: PartCatalogStore) -> Bool {
        guard part.footprintChoices.isEmpty, let base = part.footprint else { return false }
        return !catalog.sockets(fitting: base).isEmpty
    }

    /// The Mounting control's three choices.
    enum MountingChoice: String, CaseIterable, Identifiable {
        case ic, withSocket, socketOnly

        var id: String { rawValue }
        var title: String {
            switch self {
            case .ic: "IC"
            case .withSocket: "IC + Socket"
            case .socketOnly: "Socket Only"
            }
        }

        init(_ socket: ChipSocket?) {
            switch socket?.mode {
            case nil: self = .ic
            case .withSocket?: self = .withSocket
            case .socketOnly?: self = .socketOnly
            }
        }

        var mode: ChipSocket.Mode? {
            switch self {
            case .ic: nil
            case .withSocket: .withSocket
            case .socketOnly: .socketOnly
            }
        }
    }

    /// Switches a chip to a mounting choice: IC takes the socket off; from IC a socket mode starts
    /// with the generic socket, and between the two socket modes the chip keeps its socket.
    static func choose(_ choice: MountingChoice, for chip: PlacedChip, in document: SchematicDocument,
                       catalog: PartCatalogStore) {
        guard let mode = choice.mode else {
            document.setPartSocket(chip.id, mode: nil, socket: nil)
            return
        }
        let socket = chip.socket?.part ?? chip.part.footprint.flatMap { catalog.genericSocket(fitting: $0) }
        document.setPartSocket(chip.id, mode: mode, socket: socket)
    }

    @State private var choosingSocket: Bool

    init(catalog: PartCatalogStore, document: SchematicDocument, chip: PlacedChip) {
        self.catalog = catalog
        self.document = document
        self.chip = chip
        _choosingSocket = State(initialValue: false)
    }

    /// "Mill-Max Manufacturing Corp. · DIP socket, 0.3 in, through hole, gold".
    static func socketCaption(_ part: CatalogPart) -> String {
        var pieces = [SocketPickerSheet.mountingText(part) == "surface-mount" ? "surface mount" : "through hole"]
        let spacing = SocketPickerSheet.rowSpacingText(part)
        if !spacing.isEmpty { pieces.insert(spacing, at: 0) }
        let finish = SocketPickerSheet.finishText(part)
        if !finish.isEmpty { pieces.append(finish) }
        let title = "\(part.package), " + pieces.joined(separator: ", ")
        return part.manufacturer == "Generic" ? title : "\(part.manufacturer) · \(title)"
    }

    /// An LED with more than one color.
    static func hasColorChoice(_ part: CatalogPart) -> Bool {
        part.kind == PART_LED && part.colors.count > 1
    }

    var body: some View {
        // The model reads the chip again, so the pickers follow every change.
        let model = PartOptionsModel(catalog: catalog, document: document, chip: chip)
        Section("Options") {
            // A part with units (a gate IC): one block, or each unit its own symbol (plan-67).
            if let unitView = model.unitViewName {
                Picker("View", selection: Binding(get: { model.isSplit }, set: model.setSplit)) {
                    Text("Block").tag(false)
                    Text(unitView).tag(true)
                }
                .pickerStyle(.segmented)
                .help("Block: one box with every pin. \(unitView): each \(unitView.lowercased()) as its own symbol, placed, "
                    + "flipped and turned on its own, plus a block for the power pins.")
            }
            if let size = model.headerSize {
                Picker("Rows", selection: Binding(get: { size.rows }, set: model.setRows)) {
                    Text("1").tag(1)
                    Text("2").tag(2)
                }
                .pickerStyle(.segmented)
                Picker("Pins per row", selection: Binding(get: { size.perRow }, set: model.setPerRow)) {
                    ForEach(model.perRowItems, id: \.n) { Text($0.label).tag($0.n) }
                }
                .pickerStyle(.menu)
            }
            if !model.genders.isEmpty {
                Picker("Gender", selection: Binding(get: { model.gender }, set: model.setGender)) {
                    ForEach(model.genders, id: \.self) { Text($0).tag($0) }
                }
                .pickerStyle(.segmented)
            }
            if !model.colors.isEmpty {
                ColorChoicePicker(colors: model.colors, selection: Binding(get: { model.color }, set: model.setColor))
            }
            if let ohms = model.ohms {
                OhmsField(ohms: ohms, commit: model.setOhms)
            }
            if let farads = model.farads {
                CapacitanceField(farads: farads, commit: model.setFarads)
            }
            if let mounting = model.testPoint {
                Picker("Size", selection: Binding(get: { mounting.size }, set: model.setTestPointSize)) {
                    ForEach(TestPointFamily.sizes, id: \.self) { Text(TestPointFamily.label($0)).tag($0) }
                }
                .pickerStyle(.menu)
                Toggle("Through hole", isOn: Binding(get: { mounting.throughHole }, set: model.setThroughHole))
            }
            if !model.footprintChoices.isEmpty {
                FootprintChoicePicker(choices: model.footprintChoices,
                                      selection: Binding(get: { model.footprintID }, set: model.setFootprint))
            }
            if model.isSocketable {
                Picker("Mounting", selection: Binding(get: { model.mounting }, set: model.setMounting)) {
                    ForEach(MountingChoice.allCases) { Text($0.title).tag($0) }
                }
                .pickerStyle(.segmented)
                if let socket = model.current.socket {
                    LabeledContent("Socket", value: socket.part.name)
                    Text(Self.socketCaption(socket.part))
                        .font(.caption).foregroundStyle(.secondary)
                    Button("Choose Socket…") { choosingSocket = true }
                }
            }
        }
        .sheet(isPresented: $choosingSocket) {
            SocketPickerSheet(catalog: catalog, document: document, chipID: chip.id, onUse: model.useSocket)
        }
    }
}

/// A menu of LED colors; each row a filled circle in the LED's glow color,
/// then the name. Also used by Settings ▸ New Generic LED.
struct ColorChoicePicker: View {
    let colors: [String]
    @Binding var selection: String

    var body: some View {
        Picker("Color", selection: $selection) {
            ForEach(colors, id: \.self) { color in
                Label {
                    Text(color)
                } icon: {
                    Image(nsImage: Self.swatch(DiodeSymbolView.lightColor(named: color)))
                }
                .tag(color)
            }
        }
        .pickerStyle(.menu)
    }

    /// A non-template image, so a menu keeps its color.
    static func swatch(_ color: Color) -> NSImage {
        let image = NSImage(size: CGSize(width: 12, height: 12), flipped: false) { rect in
            NSColor(color).setFill()
            NSBezierPath(ovalIn: rect.insetBy(dx: 1, dy: 1)).fill()
            return true
        }
        image.isTemplate = false
        return image
    }
}

/// A menu of a part's footprint choices by footprint description, selected
/// by footprint id. The descriptions are long, so the menu gets the row's
/// full width under its label. Also used by Settings ▸ New Generic LED and
/// New Pull Resistor.
struct FootprintChoicePicker: View {
    let choices: [CatalogPart.FootprintChoice]
    @Binding var selection: String

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Footprint")
            Picker("Footprint", selection: $selection) {
                ForEach(choices, id: \.footprint.id) { choice in
                    Text(choice.footprint.description).tag(choice.footprint.id)
                }
            }
            .pickerStyle(.menu)
            .labelsHidden()
            .frame(maxWidth: .infinity, alignment: .leading)
        }
    }
}
