import SwiftUI

/// Draws one placed crystal or ceramic resonator (plan-44-G): two plates
/// 16 pt apart between the leads, a rectangle between them, `X1` on the left
/// and `X2` on the right. A three-pin resonator's middle pin (`GND`) has a
/// lead down from the rectangle's centre to the bottom edge. The frequency
/// (`PartFacts.hertz`) sits below; nothing glows in play mode (the part
/// drives its clock, the wires and the chips it feeds show it).
///
/// Like `CapacitorSymbolView`, the frame includes `ChipLayout.labelMargin` on
/// each side, and the body's top-left corner sits at `chip.origin`; a part
/// with a `GND` pin has stub room above and below (`PlacedChip.hasVerticalPins`).
struct CrystalSymbolView: View {
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

    /// The text under the symbol: the frequency.
    static func label(for chip: PlacedChip) -> String {
        chip.part.crystalHz.map(PartFacts.hertz) ?? chip.part.function
    }

    /// Whether the part has a `GND` (common) pin drawn at the bottom.
    static func hasGround(_ part: CatalogPart) -> Bool {
        part.pins.contains { $0.name == "GND" && !$0.isHidden }
    }

    /// Draws the crystal, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let mid = ChipLayout.diodeCentreY
        let width = chip.unturnedSize.width
        let height = chip.unturnedSize.height
        let centre = width / 2
        let plateLeft = centre - 8
        let plateRight = centre + 8
        let rectLeft = centre - 4
        let rectRight = centre + 4

        // The symbol's own coordinates, flipped with the part.
        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint { chip.symbolPoint(x, y) }

        var leads = Path()
        leads.move(to: point(-stub, mid))
        leads.addLine(to: point(plateLeft, mid))
        leads.move(to: point(plateRight, mid))
        leads.addLine(to: point(width + stub, mid))
        if hasGround(chip.part) {
            leads.move(to: point(centre, mid + 7))
            leads.addLine(to: point(centre, height + stub))
        }
        painter.stroke(leads, with: outline, lineWidth: lineWidth)

        var plates = Path()
        plates.move(to: point(plateLeft, mid - 7))
        plates.addLine(to: point(plateLeft, mid + 7))
        plates.move(to: point(plateRight, mid - 7))
        plates.addLine(to: point(plateRight, mid + 7))
        painter.stroke(plates, with: outline, lineWidth: lineWidth)

        var body = Path()
        body.move(to: point(rectLeft, mid - 7))
        body.addLine(to: point(rectRight, mid - 7))
        body.addLine(to: point(rectRight, mid + 7))
        body.addLine(to: point(rectLeft, mid + 7))
        body.closeSubpath()
        painter.stroke(body, with: outline, lineWidth: lineWidth)

        let text = PaintedText(label(for: chip), size: 10, weight: .medium, ink: .secondary)
        if hasGround(chip.part) {
            // The ground lead runs down the middle: the frequency goes beside it.
            painter.draw(text, at: point(centre + 5, height - 8), anchor: .leading)
        } else {
            let value = chip.symbolLabel(y: height - 8)
            painter.draw(text, at: value.point, anchor: value.anchor)
        }
    }
}
