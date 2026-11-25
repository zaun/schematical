import SwiftUI

/// Draws one placed fixed inductor (plan-47-M): four half-circle humps
/// between the leads, pin 1 on the left, and the inductance centred above
/// ("4.7 µH"). No glow and nothing in play mode: an inductor conducts like a
/// wire, the wires and the parts it feeds show it.
///
/// Like `ResistorSymbolView`, the frame includes `ChipLayout.labelMargin` on
/// each side, and the body's top-left corner sits at `chip.origin`.
struct InductorSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool

    var body: some View {
        let frame = chip.symbolFrameSize

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered)
        }
        .frame(width: frame.width, height: frame.height)
    }

    /// The text above the symbol: the inductance.
    static func label(for chip: PlacedChip) -> String {
        chip.part.inductor.map { InductorValue.text(pH: $0.pH) } ?? chip.part.function
    }

    /// Draws the inductor, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let mid = ChipLayout.diodeCentreY
        let width = chip.unturnedSize.width

        // The symbol's own coordinates, flipped with the part.
        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint { chip.symbolPoint(x, y) }

        // A lead-in, four humps each 8 wide, a lead-out.
        let humps = 4
        let humpWidth: CGFloat = 8
        let start = (width - CGFloat(humps) * humpWidth) / 2
        var strokes = Path()
        strokes.move(to: point(-stub, mid))
        strokes.addLine(to: point(start, mid))
        for i in 0..<humps {
            let left = start + CGFloat(i) * humpWidth
            let right = left + humpWidth
            strokes.addCurve(to: point(right, mid),
                             control1: point(left, mid - 8),
                             control2: point(right, mid - 8))
        }
        strokes.addLine(to: point(width + stub, mid))
        painter.stroke(strokes, with: outline, lineWidth: lineWidth)

        let value = chip.symbolLabel(y: mid - 14)
        painter.draw(
            PaintedText(label(for: chip), size: 10, weight: .medium, ink: .secondary),
            at: value.point,
            anchor: value.anchor
        )
    }
}
