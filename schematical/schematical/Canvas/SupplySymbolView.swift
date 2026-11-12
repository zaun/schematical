import SwiftUI

/// Draws one placed VCC or GND source, or a pull-up or pull-down, in the
/// usual vertical orientation.
///
/// - VCC is a T: a short bar across the top, with the stem running down to the
///   pin on the bottom edge. The label sits above the bar.
/// - GND points down: the stem comes in from the pin on the top edge and ends
///   at three horizontal bars of shrinking width. The label sits below them.
/// - A pull-up is VCC with a resistor in its stem: the rail label and bar at
///   the top, a zig-zag resistor below, the pin on the bottom edge.
/// - A pull-down is GND with a resistor in its stem: the pin on the top edge,
///   the resistor, then the ground bars.
///
/// A pull-up or pull-down also shows its resistance (`1kΩ`) to the right of
/// the resistor, in the label margin.
///
/// Flipping mirrors the symbol top to bottom, which moves the pin to the other edge.
///
/// The view is `ChipLayout.stubLength` taller than the body at the top and the
/// bottom, so the pin stubs stay inside the frame. `PlacedChip.viewOrigin`
/// accounts for that.
struct SupplySymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool

    var body: some View {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let pad = ChipLayout.stubLength

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered)
        }
        .frame(width: layout.size.width + 2 * margin, height: layout.size.height + 2 * pad)
    }

    /// Draws the supply, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let pad = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let height = layout.size.height
        let flipped = chip.isFlipped
        let isGround = chip.symbol == .ground
        let symbol = chip.symbol

        // Body coordinates, mirrored top to bottom when flipped, then shifted
        // by the margin and the top padding.
        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint {
            CGPoint(x: x + margin, y: (flipped ? height - y : y) + pad)
        }

        // Each connector runs exactly one stub (`pad`) from its bar to the
        // pin tip, which sits `pad` past the body edge.
        var strokes = Path()
        // A vertical zig-zag on x = 20 from y0 down to y1: a lead-in, six
        // half-waves, a lead-out.
        func resistor(_ y0: CGFloat, _ y1: CGFloat) {
            let lead: CGFloat = 6
            let step = (y1 - y0 - 2 * lead) / 6
            strokes.move(to: point(20, y0))
            strokes.addLine(to: point(20, y0 + lead))
            for i in 0..<6 {
                let x: CGFloat = i % 2 == 0 ? 26 : 14
                strokes.addLine(to: point(x, y0 + lead + step * (CGFloat(i) + 0.5)))
            }
            strokes.addLine(to: point(20, y1 - lead))
            strokes.addLine(to: point(20, y1))
        }
        if symbol == .pullUp {
            // Label above the bar, bar on the grid row at y = 20 (like VCC's),
            // the 60-tall resistor down to the bottom edge, pin tip `pad`
            // below it.
            let bar = height - ChipLayout.pullResistorHeight
            strokes.move(to: point(10, bar))
            strokes.addLine(to: point(30, bar))
            resistor(bar, height)
            strokes.move(to: point(20, height))
            strokes.addLine(to: point(20, height + pad))
        } else if symbol == .pullDown {
            // Pin tip at y = -pad, resistor from the top edge, ground bars below.
            strokes.move(to: point(20, -pad))
            strokes.addLine(to: point(20, 0))
            let bottom = ChipLayout.pullResistorHeight
            resistor(0, bottom)
            let bars: [(y: CGFloat, half: CGFloat)] = [(bottom, 8), (bottom + 4, 5), (bottom + 8, 2)]
            for (y, half) in bars {
                strokes.move(to: point(20 - half, y))
                strokes.addLine(to: point(20 + half, y))
            }
        } else if isGround {
            // Pin tip at y = -pad, bars on the top edge (y = 0) and below it.
            strokes.move(to: point(20, -pad))
            strokes.addLine(to: point(20, 0))
            let bars: [(y: CGFloat, half: CGFloat)] = [(0, 8), (4, 5), (8, 2)]
            for (y, half) in bars {
                strokes.move(to: point(20 - half, y))
                strokes.addLine(to: point(20 + half, y))
            }
        } else {
            // Bar on the bottom edge (y = height), pin tip `pad` below it.
            strokes.move(to: point(10, height))
            strokes.addLine(to: point(30, height))
            strokes.move(to: point(20, height))
            strokes.addLine(to: point(20, height + pad))
        }
        painter.stroke(strokes, with: outline, lineWidth: lineWidth)

        // VCC's label sits inside the body above its bar; GND's sits below
        // its bars. A pull-up shows its rail (PULLUP-3V3 → 3V3) above its
        // bar; a pull-down has no label (its ground bars say it).
        if symbol != .pullDown {
            let label = symbol == .pullUp
                ? String(chip.part.name.dropFirst("PULLUP-".count)) : chip.part.function
            painter.draw(
                PaintedText(label, size: 10, weight: .semibold, monospaced: true),
                at: point(20, isGround ? 18 : 8),
                anchor: .center
            )
        }
        // The resistance, right of the zig-zag (which ends at x = 26),
        // halfway along it. 9 pt keeps the longest value ("1.234kΩ")
        // inside the margin.
        if symbol == .pullUp || symbol == .pullDown {
            let mid: CGFloat = symbol == .pullUp ? 50 : 30
            painter.draw(
                PaintedText(ResistorValue.text(chip.ohms ?? ResistorValue.defaultOhms), size: 9, monospaced: true),
                at: point(30, mid),
                anchor: .leading
            )
        }
    }
}
