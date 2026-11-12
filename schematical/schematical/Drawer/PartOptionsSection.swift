import AppKit
import SwiftUI

/// The Schematic sidebar's Options for a placed part that has a choice of
/// color, resistance or footprint (the Generic LED, pull resistors): a Color
/// picker, an Ohms field and a Footprint picker. A header (plan-23) also
/// gets Rows, Pins per row and Gender. A change takes effect at once on the
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

    /// Resizes a header to `rows` × `perRow`, when that size is a part.
    private func resize(_ id: UUID, rows: Int, perRow: Int) {
        guard let part = catalog.part(named: HeaderFamily.name(rows: rows, perRow: perRow)) else { return }
        document.setHeaderSize(id, to: part)
    }

    /// Whether `part` has anything for this section to show.
    static func applies(to part: CatalogPart) -> Bool {
        hasColorChoice(part) || !part.footprintChoices.isEmpty
    }

    /// An LED with more than one color.
    static func hasColorChoice(_ part: CatalogPart) -> Bool {
        part.kind == PART_LED && part.colors.count > 1
    }

    var body: some View {
        // Read the chip again, so the pickers follow every change.
        let current = document.chips.first { $0.id == chip.id } ?? chip
        Section("Options") {
            if let size = current.part.headerSize {
                Picker("Rows", selection: Binding(
                    get: { size.rows },
                    set: { resize(chip.id, rows: $0, perRow: size.perRow) }
                )) {
                    Text("1").tag(1)
                    Text("2").tag(2)
                }
                .pickerStyle(.segmented)
                let wired = Self.wiredPins(of: chip.id, in: document)
                Picker("Pins per row", selection: Binding(
                    get: { size.perRow },
                    set: { resize(chip.id, rows: size.rows, perRow: $0) }
                )) {
                    ForEach(1...HeaderFamily.maxPerRow, id: \.self) { n in
                        Text(Self.pinsPerRowLabel(n, rows: size.rows, wiredPins: wired)).tag(n)
                    }
                }
                .pickerStyle(.menu)
                if current.part.genders.count > 1 {
                    Picker("Gender", selection: Binding(
                        get: { current.gender ?? current.part.genders[0] },
                        set: { document.setPartGender(chip.id, $0) }
                    )) {
                        ForEach(current.part.genders, id: \.self) { Text($0).tag($0) }
                    }
                    .pickerStyle(.segmented)
                }
            }
            if Self.hasColorChoice(current.part) {
                ColorChoicePicker(colors: current.part.colors, selection: Binding(
                    get: { current.color ?? current.part.colors[0] },
                    set: { document.setPartColor(chip.id, $0) }
                ))
            }
            if current.part.isPullResistor {
                OhmsField(ohms: current.ohms ?? ResistorValue.defaultOhms) { ohms in
                    document.setPartOhms(chip.id, ohms)
                }
            }
            if current.part.isGenericCapacitor {
                CapacitanceField(farads: current.farads ?? CapacitorValue.defaultFarads) { farads in
                    document.setPartFarads(chip.id, farads)
                }
            }
            if !current.part.footprintChoices.isEmpty {
                FootprintChoicePicker(choices: current.part.footprintChoices, selection: Binding(
                    get: { current.footprintID ?? current.part.footprint?.id ?? "" },
                    set: { document.setPartFootprint(chip.id, $0) }
                ))
            }
        }
    }
}

/// A pull resistor's resistance: a number field and a unit menu (Ω, kΩ,
/// MΩ), showing `ohms` in the largest unit with a number of at least 1. The
/// number commits on Return or when the field loses focus; picking a unit
/// commits the shown number in that unit. Input that doesn't parse, or gives
/// a value that isn't `ResistorValue.isValid`, puts the row back. Also used
/// by Settings ▸ New Pull Resistor.
struct OhmsField: View {
    let ohms: Double
    let commit: (Double) -> Void

    @State private var text = ""
    /// What the field last showed of the stored value; when `text` differs,
    /// the user is typing and outside changes don't overwrite it.
    @State private var shown = ""
    @State private var unit: ResistorValue.Unit = .kilo
    @FocusState private var isFocused: Bool

    var body: some View {
        LabeledContent("Ohms") {
            HStack(spacing: 6) {
                TextField("Ohms", text: $text)
                    .labelsHidden()
                    .focused($isFocused)
                    .multilineTextAlignment(.trailing)
                    .frame(minWidth: 60)
                    .onSubmit(apply)
                    .onChange(of: isFocused) { _, focused in
                        if !focused { apply() }
                    }
                Picker("Unit", selection: Binding(get: { unit }, set: pick)) {
                    ForEach(ResistorValue.Unit.allCases, id: \.self) { unit in
                        Text(unit.symbol).tag(unit)
                    }
                }
                .pickerStyle(.menu)
                .labelsHidden()
                .fixedSize()
            }
        }
        .onChange(of: ohms) { refresh() }
        .onAppear { show(ohms) }
    }

    /// Shows the stored value, unless the user is typing a new one.
    private func refresh() {
        if !isFocused || text == shown {
            show(ohms)
        } else {
            unit = ResistorValue.split(ohms).unit
        }
    }

    private func show(_ value: Double) {
        let split = ResistorValue.split(value)
        shown = ResistorValue.numberText(split.number)
        text = shown
        unit = split.unit
    }

    private func apply() {
        guard text != shown else { return }
        guard let number = ResistorValue.parse(text),
              let value = ResistorValue.ohms(number: number, unit: unit) else {
            show(ohms)
            return
        }
        commit(value)
        show(value)
    }

    private func pick(_ newUnit: ResistorValue.Unit) {
        guard newUnit != unit else { return }
        let number = ResistorValue.parse(text) ?? ResistorValue.split(ohms).number
        guard let value = ResistorValue.ohms(number: number, unit: newUnit) else {
            show(ohms)
            return
        }
        commit(value)
        show(value)
    }
}

/// The generic capacitor's capacitance (plan-33-H): a number field and a
/// unit menu (pF, nF, µF), showing `farads` in the largest unit with a
/// number of at least 1. Commits like `OhmsField`: on Return, on losing
/// focus, or on picking a unit; input that doesn't parse, or isn't
/// `CapacitorValue.isValid`, puts the row back.
struct CapacitanceField: View {
    let farads: Double
    let commit: (Double) -> Void

    @State private var text = ""
    @State private var shown = ""
    @State private var unit: CapacitorValue.Unit = .nano
    @FocusState private var isFocused: Bool

    var body: some View {
        LabeledContent("Capacitance") {
            HStack(spacing: 6) {
                TextField("Capacitance", text: $text)
                    .labelsHidden()
                    .focused($isFocused)
                    .multilineTextAlignment(.trailing)
                    .frame(minWidth: 60)
                    .onSubmit(apply)
                    .onChange(of: isFocused) { _, focused in
                        if !focused { apply() }
                    }
                Picker("Unit", selection: Binding(get: { unit }, set: pick)) {
                    ForEach(CapacitorValue.Unit.allCases, id: \.self) { unit in
                        Text(unit.symbol).tag(unit)
                    }
                }
                .pickerStyle(.menu)
                .labelsHidden()
                .fixedSize()
            }
        }
        .onChange(of: farads) { refresh() }
        .onAppear { show(farads) }
    }

    private func refresh() {
        if !isFocused || text == shown {
            show(farads)
        } else {
            unit = CapacitorValue.split(farads).unit
        }
    }

    private func show(_ value: Double) {
        let split = CapacitorValue.split(value)
        shown = CapacitorValue.numberText(split.number)
        text = shown
        unit = split.unit
    }

    private func apply() {
        guard text != shown else { return }
        guard let number = CapacitorValue.parse(text),
              let value = CapacitorValue.farads(number: number, unit: unit) else {
            show(farads)
            return
        }
        commit(value)
        show(value)
    }

    private func pick(_ newUnit: CapacitorValue.Unit) {
        guard newUnit != unit else { return }
        let number = CapacitorValue.parse(text) ?? CapacitorValue.split(farads).number
        guard let value = CapacitorValue.farads(number: number, unit: newUnit) else {
            show(farads)
            return
        }
        commit(value)
        show(value)
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
