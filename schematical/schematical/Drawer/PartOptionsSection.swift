import AppKit
import SwiftUI

/// The Schematic sidebar's Options for a placed part that has a choice of
/// color or footprint (the Generic LED): a Color picker and a Footprint
/// picker. A header (plan-23) also gets Rows, Pins per row and Gender. A
/// pick takes effect at once on the canvas, the PCB and the file.
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
            if !current.part.footprintChoices.isEmpty {
                FootprintChoicePicker(choices: current.part.footprintChoices, selection: Binding(
                    get: { current.footprintID ?? current.part.footprint?.id ?? "" },
                    set: { document.setPartFootprint(chip.id, $0) }
                ))
            }
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
/// full width under its label. Also used by Settings ▸ New Generic LED.
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
