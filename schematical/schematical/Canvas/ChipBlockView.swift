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
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Color = highlighted ? .accentColor : .primary

        Canvas { context, _ in
            let box = CGRect(x: margin, y: 0, width: layout.size.width, height: layout.size.height)

            context.fill(Path(box), with: .color(Color(nsColor: .controlBackgroundColor)))
            if isHovered {
                context.fill(Path(box), with: .color(Color.accentColor.opacity(0.12)))
            }
            context.stroke(
                Path(box),
                with: .color(outline),
                lineWidth: highlighted ? 2 : 1.5
            )

            var headerRule = Path()
            headerRule.move(to: CGPoint(x: box.minX, y: ChipLayout.headerHeight))
            headerRule.addLine(to: CGPoint(x: box.maxX, y: ChipLayout.headerHeight))
            context.stroke(headerRule, with: .color(outline.opacity(0.5)), lineWidth: 1)

            context.draw(
                Text(chip.partNumber)
                    .font(.system(size: 11, weight: .semibold, design: .monospaced)),
                at: CGPoint(x: box.midX, y: ChipLayout.headerHeight / 2),
                anchor: .center
            )

            let bodyCentre = CGPoint(x: box.midX, y: (ChipLayout.headerHeight + box.maxY) / 2)
            if chip.symbol == .oscillator {
                let radius = min(14, (box.maxY - ChipLayout.headerHeight) / 2 - 4)
                Self.drawSineGlyph(&context, centre: bodyCentre, radius: radius, color: outline)
            } else {
                context.draw(
                    Text(chip.part.function)
                        .font(.system(size: 13, weight: .medium))
                        .foregroundStyle(.secondary),
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
                context.stroke(pinStub, with: .color(outline), lineWidth: 1.5)

                context.draw(
                    Text(slot.pin.designator)
                        .font(.system(size: 9))
                        .foregroundStyle(.secondary),
                    at: CGPoint(x: isLeft ? tipX - 3 : tipX + 3, y: y),
                    anchor: isLeft ? .trailing : .leading
                )

                context.draw(
                    Text(slot.pin.name)
                        .font(.system(size: 10, weight: .medium, design: .monospaced))
                        .foregroundStyle(Self.nameColor(for: slot.pin.role)),
                    at: CGPoint(x: isLeft ? edgeX + 5 : edgeX - 5, y: y),
                    anchor: isLeft ? .leading : .trailing
                )
            }
        }
        .frame(width: layout.size.width + 2 * margin, height: layout.size.height)
    }

    /// One cycle of a sine wave inside a circle: the oscillator's symbol.
    private static func drawSineGlyph(_ context: inout GraphicsContext, centre: CGPoint, radius: CGFloat, color: Color) {
        let circle = CGRect(x: centre.x - radius, y: centre.y - radius, width: radius * 2, height: radius * 2)
        context.stroke(Path(ellipseIn: circle), with: .color(color.opacity(0.6)), lineWidth: 1)

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
        context.stroke(wave, with: .color(color), style: StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round))
    }

    private static func nameColor(for role: ChipPinRole) -> Color {
        switch role {
        case .power: .red
        case .ground: .blue
        default: .primary
        }
    }
}
