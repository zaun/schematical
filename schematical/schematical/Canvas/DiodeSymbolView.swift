import SwiftUI

/// Draws one placed LED with the standard diode symbol: a filled triangle
/// pointing from the anode (left) to the cathode (right), a bar across the
/// cathode, and two arrows for the emitted light. The function label sits below.
///
/// Like `ChipBlockView`, the frame includes `ChipLayout.labelMargin` on each
/// side, and the body's top-left corner sits at `chip.origin`.
struct DiodeSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool
    /// Play mode: brightness 0…1, the share of the last frame the LED was on
    /// (anode high, cathode low). A lit LED glows in its own color.
    var level: Double = 0

    /// The LED's color: the placed LED's own color (Red, Green, Blue, Amber,
    /// White), or a 7-segment display's (also Yellow and Orange).
    static func lightColor(for chip: PlacedChip) -> Color {
        lightColor(named: chip.color ?? chip.part.colors.first ?? "")
    }

    /// The glow color of a color name; `.secondary` for an unknown one.
    nonisolated static func lightColor(named family: String) -> Color {
        switch family {
        case "Red": return .red
        case "Green": return .green
        case "Blue": return .blue
        case "Amber": return Color(red: 1, green: 0.7, blue: 0)
        case "Yellow": return .yellow
        case "Orange": return .orange
        case "White": return Color(white: 0.7)
        default: return .secondary
        }
    }

    var body: some View {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Color = highlighted ? .accentColor : .primary
        let light = Self.lightColor(for: chip)
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let mid = ChipLayout.diodeCentreY

        let width = layout.size.width
        let flipped = chip.isFlipped

        Canvas { context, _ in
            // Body coordinates, mirrored left to right when flipped, then shifted
            // by the label margin. Mirroring every point keeps the arrowheads
            // and leads consistent.
            func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint {
                CGPoint(x: (flipped ? width - x : x) + margin, y: y)
            }

            // Leads: from each pin tip into the symbol.
            var leads = Path()
            leads.move(to: point(-stub, mid))
            leads.addLine(to: point(12, mid))
            leads.move(to: point(26, mid))
            leads.addLine(to: point(width + stub, mid))
            context.stroke(leads, with: .color(outline), lineWidth: lineWidth)

            // Diode body: anode at the base, pointing right toward the cathode.
            if level > 0 {
                let glow = CGRect(x: point(19, mid).x - 16, y: point(19, mid).y - 16, width: 32, height: 32)
                context.fill(Path(ellipseIn: glow), with: .color(light.opacity(0.3 * level)))
            }
            var triangle = Path()
            triangle.move(to: point(12, mid - 6))
            triangle.addLine(to: point(12, mid + 6))
            triangle.addLine(to: point(26, mid))
            triangle.closeSubpath()
            context.fill(triangle, with: .color(outline))
            if level > 0 {
                context.fill(triangle, with: .color(light.opacity(level)))
            }

            var cathodeBar = Path()
            cathodeBar.move(to: point(26, mid - 6))
            cathodeBar.addLine(to: point(26, mid + 6))
            context.stroke(cathodeBar, with: .color(outline), lineWidth: lineWidth)

            // Light emitted up and to the right, with open arrowheads.
            var arrows = Path()
            Self.addArrow(to: &arrows, from: point(22, mid - 8), to: point(30, mid - 16))
            Self.addArrow(to: &arrows, from: point(28, mid - 4), to: point(36, mid - 12))
            context.stroke(arrows, with: .color(outline), lineWidth: 1.25)
            if level > 0 {
                context.stroke(arrows, with: .color(light.opacity(level)), lineWidth: 1.25)
            }

            context.draw(
                Text(chip.part.function)
                    .font(.system(size: 10, weight: .medium))
                    .foregroundStyle(.secondary),
                at: point(width / 2, layout.size.height - 8),
                anchor: .center
            )
        }
        .frame(width: layout.size.width + 2 * margin, height: layout.size.height)
    }

    /// A line with an open arrowhead at `end`. The arms run back along the
    /// line's direction, which here is up and to the right.
    private static func addArrow(to path: inout Path, from start: CGPoint, to end: CGPoint) {
        path.move(to: start)
        path.addLine(to: end)
        path.move(to: end)
        path.addLine(to: CGPoint(x: end.x - 4, y: end.y))
        path.move(to: end)
        path.addLine(to: CGPoint(x: end.x, y: end.y + 4))
    }
}
