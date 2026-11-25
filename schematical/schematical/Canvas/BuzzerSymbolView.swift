import SwiftUI

/// Draws one placed buzzer (plan-47-N): a half disc on a base line between the
/// leads, `+` on the left and `-` on the right, a small `+` by the positive
/// lead and the rated frequency under it ("2.5 kHz"). In play mode, while the
/// buzzer is on, three sound-wave arcs show above the dome.
///
/// Like `ResistorSymbolView`, the frame includes `ChipLayout.labelMargin` on
/// each side, and the body's top-left corner sits at `chip.origin`.
struct BuzzerSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool
    /// Play mode: the share of the last frame the buzzer was on (0…1).
    var level: Double = 0

    var body: some View {
        let frame = chip.symbolFrameSize

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered, level: level)
        }
        .frame(width: frame.width, height: frame.height)
    }

    /// The text under the symbol: the rated frequency, else the part's function.
    static func label(for chip: PlacedChip) -> String {
        guard let info = chip.part.buzzer, info.hz > 0 else { return chip.part.function }
        return PartFacts.hertz(UInt64(info.hz))
    }

    /// Draws the buzzer, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool,
                                           isHovered: Bool, level: Double = 0) {
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let mid = ChipLayout.diodeCentreY
        let width = chip.unturnedSize.width
        let centre = width / 2
        let radius: CGFloat = 9

        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint { chip.symbolPoint(x, y) }
        /// A circular arc of the symbol's own coordinates, from `from` to
        /// `to` degrees (0 = right, 90 = down).
        func arc(_ r: CGFloat, from: Double, to: Double) -> Path {
            var path = Path()
            let steps = 16
            for step in 0...steps {
                let angle = (from + (to - from) * Double(step) / Double(steps)) * .pi / 180
                let p = point(centre + r * CGFloat(cos(angle)), mid + r * CGFloat(sin(angle)))
                if step == 0 { path.move(to: p) } else { path.addLine(to: p) }
            }
            return path
        }

        // The leads, with the base line between them under the dome.
        var leads = Path()
        leads.move(to: point(-stub, mid))
        leads.addLine(to: point(width + stub, mid))
        painter.stroke(leads, with: outline, lineWidth: lineWidth)

        // The half disc: the arc over the base line.
        var dome = arc(radius, from: 180, to: 360)
        dome.closeSubpath()
        if level > 0 {
            painter.fill(dome, with: Ink.switchOn.opacity(min(1, 0.4 + 0.6 * level)))
        } else {
            painter.fill(dome, with: Ink.secondary.opacity(0.12))
        }
        painter.stroke(dome, with: outline, lineWidth: lineWidth)

        // The `+` by the positive lead.
        painter.draw(
            PaintedText("+", size: 10, weight: .semibold, ink: .secondary),
            at: point(4, mid - 7),
            anchor: .center
        )

        // The sound waves, while it is on.
        if level > 0 {
            for r: CGFloat in [12.5, 16, 19.5] {
                painter.stroke(arc(r, from: 225, to: 315), with: outline.opacity(0.85), lineWidth: 1.25)
            }
        }

        let value = chip.symbolLabel(y: chip.unturnedSize.height - 8)
        painter.draw(
            PaintedText(label(for: chip), size: 10, weight: .medium, ink: .secondary),
            at: value.point,
            anchor: value.anchor
        )
    }
}
