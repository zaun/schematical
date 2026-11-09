import SwiftUI

/// Draws one placed tactile (push-button) switch, plan-27-J: a box with the
/// part number in the header, side A's pins on the left and side B's on the
/// right. On a four-terminal switch a bar joins the two pins of each side
/// (they are connected inside the part). In the middle, two contacts and a
/// plunger: when the contact is closed (pressed on a Normal Open part,
/// released on a Normal Close one) the bar touches both contacts and is
/// green; open, there is a gap. Pressed, the cap is drawn pushed down. Hold
/// the mouse on the button in play mode to press it.
///
/// Like `DipSwitchSymbolView`, the frame includes `ChipLayout.labelMargin`
/// on each side, and the body's top-left corner sits at `chip.origin`.
struct TactileSwitchSymbolView: View {
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

    /// Draws the switch, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let box = CGRect(x: margin, y: 0, width: layout.size.width, height: layout.size.height)
        let closed = chip.tactileContactClosed
        let pressed = chip.isPressed

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

        // Pin stubs and designators outside the box, as on a block.
        for slot in layout.slots {
            let isLeft = slot.side == .left
            let edgeX = isLeft ? box.minX : box.maxX
            let tipX = isLeft ? box.minX - stub : box.maxX + stub

            var pinStub = Path()
            pinStub.move(to: CGPoint(x: tipX, y: slot.centerY))
            pinStub.addLine(to: CGPoint(x: edgeX, y: slot.centerY))
            painter.stroke(pinStub, with: outline, lineWidth: 1.5)

            painter.draw(
                PaintedText(slot.pin.designator, size: 9, ink: .secondary),
                at: CGPoint(x: isLeft ? tipX - 3 : tipX + 3, y: slot.centerY),
                anchor: isLeft ? .trailing : .leading
            )
        }

        // The button area and the contact line through its middle.
        let button = ChipLayout.tactileButtonRect(bodySize: layout.size).offsetBy(dx: margin, dy: 0)
        let contactY = button.midY + 4
        let contactA = CGPoint(x: button.minX + 3, y: contactY)
        let contactB = CGPoint(x: button.maxX - 3, y: contactY)

        // Each side's pins: leads into the box, joined by a bar when the side
        // has two pins, then one lead to its contact.
        for side in [ChipLayout.Side.left, .right] {
            let pins = layout.slots.filter { $0.side == side }
            guard !pins.isEmpty else { continue }
            let isLeft = side == .left
            let edgeX = isLeft ? box.minX : box.maxX
            let railX = isLeft ? box.minX + 10 : box.maxX - 10
            let contact = isLeft ? contactA : contactB
            var leads = Path()
            for slot in pins {
                leads.move(to: CGPoint(x: edgeX, y: slot.centerY))
                leads.addLine(to: CGPoint(x: railX, y: slot.centerY))
            }
            let ys = pins.map(\.centerY)
            if let top = ys.min(), let bottom = ys.max(), top != bottom {
                leads.move(to: CGPoint(x: railX, y: top))
                leads.addLine(to: CGPoint(x: railX, y: bottom))
            }
            let start = CGPoint(x: railX, y: min(max(contactY, ys.min() ?? contactY), ys.max() ?? contactY))
            leads.move(to: start)
            if start.y != contactY {
                leads.addLine(to: CGPoint(x: railX, y: contactY))
            }
            leads.addLine(to: contact)
            painter.stroke(leads, with: outline.opacity(0.6), lineWidth: 1)
        }

        // The two contacts.
        for point in [contactA, contactB] {
            painter.fill(Path(ellipseIn: CGRect(x: point.x - 2.5, y: point.y - 2.5, width: 5, height: 5)),
                         with: outline)
        }

        // The bar: touching both contacts when closed, lifted when open.
        let barY = closed ? contactY - 3 : contactY - 9
        var bar = Path()
        bar.move(to: CGPoint(x: contactA.x - 1, y: barY))
        bar.addLine(to: CGPoint(x: contactB.x + 1, y: barY))
        painter.stroke(bar, with: closed ? .switchOn : outline, lineWidth: closed ? 3 : 2)

        // The plunger and cap above the bar; pressed, pushed down.
        let capTop = button.minY + (pressed ? 5 : 2)
        var plunger = Path()
        plunger.move(to: CGPoint(x: button.midX, y: capTop + 3))
        plunger.addLine(to: CGPoint(x: button.midX, y: barY))
        painter.stroke(plunger, with: outline, lineWidth: 1.25)
        let cap = CGRect(x: button.midX - 8, y: capTop, width: 16, height: 3)
        painter.fill(Path(roundedRect: cap, cornerRadius: 1), with: pressed ? .switchOn : outline)
    }
}
