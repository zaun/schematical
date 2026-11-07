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
