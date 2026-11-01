import SwiftUI

/// Draws one placed VCC or GND source, in the usual vertical orientation.
///
/// - VCC is a T: a short bar across the top, with the stem running down to the
///   pin on the bottom edge. The label sits above the bar.
/// - GND points down: the stem comes in from the pin on the top edge and ends
///   at three horizontal bars of shrinking width. The label sits below them.
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
        let highlighted = isSelected || isHovered
        let outline: Color = highlighted ? .accentColor : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let height = layout.size.height
        let flipped = chip.isFlipped
        let isGround = chip.symbol == .ground

        Canvas { context, _ in
            // Body coordinates, mirrored top to bottom when flipped, then shifted
            // by the margin and the top padding.
            func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint {
                CGPoint(x: x + margin, y: (flipped ? height - y : y) + pad)
            }

            // Each connector runs exactly one stub (`pad`) from its bar to the
            // pin tip, which sits `pad` past the body edge.
            var strokes = Path()
            if isGround {
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
            context.stroke(strokes, with: .color(outline), lineWidth: lineWidth)

            // VCC's label sits inside the body above its bar; GND's sits below its bars.
            context.draw(
                Text(chip.part.function)
                    .font(.system(size: 10, weight: .semibold, design: .monospaced)),
                at: point(20, isGround ? 18 : 8),
                anchor: .center
            )
        }
        .frame(width: layout.size.width + 2 * margin, height: height + 2 * pad)
    }
}
