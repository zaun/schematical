import SwiftUI

/// Draws one placed fuse or PTC resettable fuse (plan-47-M): a rectangle
/// between the leads with the lead drawn through it, pin 1 on the left, and
/// the rated current (a PTC's hold current) centred above ("500 mA"). A PTC
/// adds the thermistor stroke: a diagonal across the rectangle with a flat
/// foot.
///
/// In play mode a click blows the fuse and a second restores it
/// (`SimulationController.toggleFuse`): a blown fuse draws with a gap in the
/// middle of its lead and its body in red.
///
/// Like `ResistorSymbolView`, the frame includes `ChipLayout.labelMargin` on
/// each side, and the body's top-left corner sits at `chip.origin`.
struct FuseSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool
    /// Play mode: the fuse has been blown with a click.
    var isBlown = false

    var body: some View {
        let frame = chip.symbolFrameSize

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered, isBlown: isBlown)
        }
        .frame(width: frame.width, height: frame.height)
    }

    /// The text above the symbol: the rated (hold) current.
    static func label(for chip: PlacedChip) -> String {
        guard let info = chip.part.fuse, info.ratedMA > 0 else { return chip.part.function }
        return PartFacts.amps(mA: info.ratedMA)
    }

    /// Draws the fuse, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool,
                                           isHovered: Bool, isBlown: Bool) {
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let bodyInk: Ink = isBlown ? .color(.red) : outline
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let mid = ChipLayout.diodeCentreY
        let width = chip.unturnedSize.width
        let centre = width / 2
        let rectLeft: CGFloat = 4
        let rectRight = width - 4
        let half: CGFloat = 5

        // The symbol's own coordinates, flipped with the part.
        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint { chip.symbolPoint(x, y) }

        // The lead through the body; a blown fuse's is open in the middle.
        var leads = Path()
        if isBlown {
            leads.move(to: point(-stub, mid))
            leads.addLine(to: point(centre - 4, mid))
            leads.move(to: point(centre + 4, mid))
            leads.addLine(to: point(width + stub, mid))
        } else {
            leads.move(to: point(-stub, mid))
            leads.addLine(to: point(width + stub, mid))
        }
        painter.stroke(leads, with: outline, lineWidth: lineWidth)

        var body = Path()
        body.move(to: point(rectLeft, mid - half))
        body.addLine(to: point(rectRight, mid - half))
        body.addLine(to: point(rectRight, mid + half))
        body.addLine(to: point(rectLeft, mid + half))
        body.closeSubpath()
        painter.stroke(body, with: bodyInk, lineWidth: lineWidth)

        if chip.part.fuse?.isPTC == true {
            // The thermistor stroke: a diagonal with a flat foot, longer
            // than the rectangle is tall.
            var stroke = Path()
            stroke.move(to: point(centre - 12, mid + 8))
            stroke.addLine(to: point(centre - 6, mid + 8))
            stroke.addLine(to: point(centre + 7, mid - 8))
            painter.stroke(stroke, with: bodyInk, lineWidth: lineWidth)
        }

        let value = chip.symbolLabel(y: mid - 14)
        painter.draw(
            PaintedText(label(for: chip), size: 10, weight: .medium, ink: .secondary),
            at: value.point,
            anchor: value.anchor
        )
    }
}
