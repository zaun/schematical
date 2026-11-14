import SwiftUI

/// Draws one placed two-terminal resistor (plan-34-G): the pulls' zig-zag
/// (six half-waves, `SupplySymbolView`) laid horizontally between the
/// leads, pin 1 on the left, and its value centred above: the generic
/// resistor's own (`PlacedChip.ohms`), else the part's resistance. No glow
/// and nothing in play mode. Resistor arrays are drawn as blocks.
///
/// Like `CapacitorSymbolView`, the frame includes `ChipLayout.labelMargin`
/// on each side, and the body's top-left corner sits at `chip.origin`.
struct ResistorSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool

    var body: some View {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered)
        }
        .frame(width: layout.size.width + 2 * margin, height: layout.size.height)
    }

    /// The text above the symbol: "1kΩ", "4.7kΩ", "0Ω".
    static func label(for chip: PlacedChip) -> String {
        if let ohms = chip.ohms { return ResistorValue.text(ohms) }
        return chip.part.resistorOhms.map(ResistorValue.text) ?? chip.part.function
    }

    /// Draws the resistor, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let mid = ChipLayout.diodeCentreY
        let width = layout.size.width
        let flipped = chip.isFlipped
        let lead: CGFloat = 2

        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint {
            CGPoint(x: (flipped ? width - x : x) + margin, y: y)
        }

        // Leads from each pin tip to the body edge, then the zig-zag across
        // the body: a lead-in, six half-waves, a lead-out.
        var strokes = Path()
        strokes.move(to: point(-stub, mid))
        strokes.addLine(to: point(lead, mid))
        let step = (width - 2 * lead) / 6
        for i in 0..<6 {
            let y: CGFloat = i % 2 == 0 ? mid - 6 : mid + 6
            strokes.addLine(to: point(lead + step * (CGFloat(i) + 0.5), y))
        }
        strokes.addLine(to: point(width - lead, mid))
        strokes.addLine(to: point(width + stub, mid))
        painter.stroke(strokes, with: outline, lineWidth: lineWidth)

        painter.draw(
            PaintedText(label(for: chip), size: 10, weight: .medium, ink: .secondary),
            at: point(width / 2, mid - 14),
            anchor: .center
        )
    }
}
