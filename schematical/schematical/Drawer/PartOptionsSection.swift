import AppKit
import SwiftUI

/// The Schematic sidebar's Options for a placed part that has a choice of
/// color or footprint (the Generic LED): a Color picker and a Footprint
/// picker. A pick takes effect at once on the canvas, the PCB and the file.
struct PartOptionsSection: View {
    let document: SchematicDocument
    let chip: PlacedChip

    /// Whether `part` has anything for this section to show.
    static func applies(to part: CatalogPart) -> Bool {
        (part.kind == PART_LED && part.colors.count > 1) || !part.footprintChoices.isEmpty
    }

    var body: some View {
        // Read the chip again, so the pickers follow every change.
        let current = document.chips.first { $0.id == chip.id } ?? chip
        Section("Options") {
            if current.part.kind == PART_LED, current.part.colors.count > 1 {
                Picker("Color", selection: Binding(
                    get: { current.color ?? current.part.colors[0] },
                    set: { document.setPartColor(chip.id, $0) }
                )) {
                    ForEach(current.part.colors, id: \.self) { color in
                        ColorChoiceRow(name: color).tag(color)
                    }
                }
            }
            if !current.part.footprintChoices.isEmpty {
                // The descriptions are long, so the menu gets the row's full
                // width under its label.
                VStack(alignment: .leading, spacing: 6) {
                    Text("Footprint")
                    Picker("Footprint", selection: Binding(
                        get: { current.footprintID ?? current.part.footprint?.id ?? "" },
                        set: { document.setPartFootprint(chip.id, $0) }
                    )) {
                        ForEach(current.part.footprintChoices, id: \.footprint.id) { choice in
                            Text(choice.footprint.description).tag(choice.footprint.id)
                        }
                    }
                    .labelsHidden()
                    .frame(maxWidth: .infinity, alignment: .leading)
                }
            }
        }
        .pickerStyle(.menu)
    }
}

/// A color's picker row: a filled circle in the LED's glow color, then the
/// name. The circle is a non-template image, so a menu keeps its color.
struct ColorChoiceRow: View {
    let name: String

    var body: some View {
        Label {
            Text(name)
        } icon: {
            Image(nsImage: Self.swatch(DiodeSymbolView.lightColor(named: name)))
        }
    }

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
