import SwiftUI

/// Draws one placed chip: a box with the part number in the header, the
/// readable function in the middle, pin stubs and pin names on the left
/// (inputs, power) and right (outputs, ground), and pin designators outside the box.
///
/// The view's frame includes `ChipLayout.labelMargin` on each side. The body's
/// top-left corner sits at `chip.origin`, so the parent offsets by that margin.
struct ChipBlockView: View {
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

    /// Draws the chip, its top-left corner at the view's (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let box = CGRect(x: margin, y: 0, width: layout.size.width, height: layout.size.height)

        painter.fill(Path(box), with: .paper)
        if isHovered {
            painter.fill(Path(box), with: Ink.accent.opacity(0.12))
        }
        painter.stroke(Path(box), with: outline, lineWidth: highlighted ? 2 : 1.5)

        var headerRule = Path()
        headerRule.move(to: CGPoint(x: box.minX, y: ChipLayout.headerHeight))
        headerRule.addLine(to: CGPoint(x: box.maxX, y: ChipLayout.headerHeight))
        painter.stroke(headerRule, with: outline.opacity(0.5), lineWidth: 1)

        painter.draw(
            PaintedText(chip.partNumber, size: 11, weight: .semibold, monospaced: true),
            at: CGPoint(x: box.midX, y: ChipLayout.headerHeight / 2),
            anchor: .center
        )

        let bodyCentre = CGPoint(x: box.midX, y: (ChipLayout.headerHeight + box.maxY) / 2)
        if chip.symbol == .oscillator {
            let radius = min(14, (box.maxY - ChipLayout.headerHeight) / 2 - 4)
            drawSineGlyph(&painter, centre: bodyCentre, radius: radius, ink: outline)
        } else if chip.symbol == .barrelJack {
            let radius = min(14, (box.maxY - ChipLayout.headerHeight) / 2 - 4)
            drawBarrelGlyph(&painter, centre: bodyCentre, radius: radius, ink: outline)
        } else {
            painter.draw(
                PaintedText(chip.part.function, size: 13, weight: .medium, ink: .secondary),
                at: bodyCentre,
                anchor: .center
            )
        }

        for slot in layout.slots {
            let isLeft = slot.side == .left
            let y = slot.centerY
            let edgeX = isLeft ? box.minX : box.maxX
            let tipX = isLeft ? box.minX - stub : box.maxX + stub

            var pinStub = Path()
            pinStub.move(to: CGPoint(x: tipX, y: y))
            pinStub.addLine(to: CGPoint(x: edgeX, y: y))
            painter.stroke(pinStub, with: outline, lineWidth: 1.5)

            painter.draw(
                PaintedText(slot.pin.designator, size: 9, ink: .secondary),
                at: CGPoint(x: isLeft ? tipX - 3 : tipX + 3, y: y),
                anchor: isLeft ? .trailing : .leading
            )

            painter.draw(
                PaintedText(slot.pin.name, size: 10, weight: .medium, monospaced: true,
                            ink: nameInk(for: slot.pin.role)),
                at: CGPoint(x: isLeft ? edgeX + 5 : edgeX - 5, y: y),
                anchor: isLeft ? .leading : .trailing
            )
        }
    }

    /// One cycle of a sine wave inside a circle: the oscillator's symbol.
    private static func drawSineGlyph<P: SchematicPainter>(_ painter: inout P, centre: CGPoint, radius: CGFloat, ink: Ink) {
        let circle = CGRect(x: centre.x - radius, y: centre.y - radius, width: radius * 2, height: radius * 2)
        painter.stroke(Path(ellipseIn: circle), with: ink.opacity(0.6), lineWidth: 1)

        let amplitude = radius * 0.45
        let halfWidth = radius * 0.8
        let steps = 40
        var wave = Path()
        for step in 0...steps {
            let t = CGFloat(step) / CGFloat(steps)
            let point = CGPoint(
                x: centre.x - halfWidth + 2 * halfWidth * t,
                y: centre.y - amplitude * sin(2 * .pi * t)
            )
            if step == 0 {
                wave.move(to: point)
            } else {
                wave.addLine(to: point)
            }
        }
        painter.stroke(wave, with: ink, style: StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round))
    }

    /// Front view of a barrel jack: outer sleeve ring and centre pin.
    private static func drawBarrelGlyph<P: SchematicPainter>(_ painter: inout P, centre: CGPoint, radius: CGFloat, ink: Ink) {
        let outer = CGRect(x: centre.x - radius, y: centre.y - radius, width: radius * 2, height: radius * 2)
        let innerRadius = radius * 0.35
        let inner = CGRect(x: centre.x - innerRadius, y: centre.y - innerRadius,
                           width: innerRadius * 2, height: innerRadius * 2)
        painter.stroke(Path(ellipseIn: outer), with: ink.opacity(0.7), lineWidth: 2)
        painter.fill(Path(ellipseIn: inner), with: ink.opacity(0.7))
    }

    private static func nameInk(for role: ChipPinRole) -> Ink {
        switch role {
        case .power: .power
        case .ground: .ground
        default: .primary
        }
    }
}
