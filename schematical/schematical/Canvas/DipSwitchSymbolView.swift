import SwiftUI

/// Draws one placed DIP switch: a box with the part number in the header and
/// one row per position. Each row has the position number, a lead from each
/// pin to a lever in the middle, and the lever itself: knob to the right and
/// filled when the position is ON (its two pins joined), knob to the left when
/// OFF. Click a lever to flip it.
///
/// Like `ChipBlockView`, the frame includes `ChipLayout.labelMargin` on each
/// side, and the body's top-left corner sits at `chip.origin`.
struct DipSwitchSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool

    var body: some View {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Color = highlighted ? .accentColor : .primary
        let switchesOn = chip.switchesOn

        Canvas { context, _ in
            let box = CGRect(x: margin, y: 0, width: layout.size.width, height: layout.size.height)

            context.fill(Path(box), with: .color(Color(nsColor: .controlBackgroundColor)))
            if isHovered {
                context.fill(Path(box), with: .color(Color.accentColor.opacity(0.12)))
            }
            context.stroke(Path(box), with: .color(outline), lineWidth: highlighted ? 2 : 1.5)

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

            // Pin stubs and pin numbers outside the box, as on a block.
            for slot in layout.slots {
                let isLeft = slot.side == .left
                let edgeX = isLeft ? box.minX : box.maxX
                let tipX = isLeft ? box.minX - stub : box.maxX + stub

                var pinStub = Path()
                pinStub.move(to: CGPoint(x: tipX, y: slot.centerY))
                pinStub.addLine(to: CGPoint(x: edgeX, y: slot.centerY))
                context.stroke(pinStub, with: .color(outline), lineWidth: 1.5)

                context.draw(
                    Text("\(slot.pin.number)")
                        .font(.system(size: 9))
                        .foregroundStyle(.secondary),
                    at: CGPoint(x: isLeft ? tipX - 3 : tipX + 3, y: slot.centerY),
                    anchor: isLeft ? .trailing : .leading
                )
            }

            for (index, isOn) in switchesOn.enumerated() {
                let position = index + 1
                let lever = ChipLayout.switchLeverRect(position: position, bodyWidth: layout.size.width)
                    .offsetBy(dx: margin, dy: 0)
                let y = lever.midY

                // Leads from each side of the box to the lever.
                var leads = Path()
                leads.move(to: CGPoint(x: box.minX, y: y))
                leads.addLine(to: CGPoint(x: lever.minX, y: y))
                leads.move(to: CGPoint(x: lever.maxX, y: y))
                leads.addLine(to: CGPoint(x: box.maxX, y: y))
                context.stroke(leads, with: .color(outline.opacity(0.6)), lineWidth: 1)

                let track = Path(roundedRect: lever, cornerRadius: lever.height / 2)
                context.fill(track, with: .color(isOn ? Color.green.opacity(0.85) : Color(nsColor: .controlBackgroundColor)))
                context.stroke(track, with: .color(outline), lineWidth: 1.25)

                let knobSize = lever.height - 4
                let knob = CGRect(
                    x: isOn ? lever.maxX - 2 - knobSize : lever.minX + 2,
                    y: lever.minY + 2,
                    width: knobSize,
                    height: knobSize
                )
                context.fill(Path(ellipseIn: knob), with: .color(isOn ? .white : outline))

                // Position number, just inside the box on the left of the lever.
                context.draw(
                    Text("\(position)")
                        .font(.system(size: 9, weight: .medium, design: .monospaced))
                        .foregroundStyle(.secondary),
                    at: CGPoint(x: lever.minX - 6, y: y - 7),
                    anchor: .trailing
                )
            }
        }
        .frame(width: layout.size.width + 2 * margin, height: layout.size.height)
    }
}
