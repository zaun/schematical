import SwiftUI

/// Draws one placed slide switch (plan-58-G): a box with the part number in the
/// header, the common terminals on the left and the throws on the right. Each
/// pole is a blade that starts at its common terminal (SPST and DPST: its `A`
/// terminal) and rests on the throw the knob position connects, green; in an
/// Off position it points half way, touching nothing. A dashed line joins the
/// blades of a two-pole switch. Under the poles, a slot has a dot for each knob
/// position and the knob (filled green when a contact is closed) at the
/// current one. A click on the body moves the knob to the next position.
///
/// Like `DipSwitchSymbolView`, the frame includes `ChipLayout.labelMargin` on
/// each side, and the body's top-left corner sits at `chip.origin`.
struct SlideSwitchSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool
    /// Pins (`ChipPin.index`) with a wire on them: their numbers are not drawn.
    var connectedPins: Set<Int> = []

    var body: some View {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered,
                       connectedPins: connectedPins)
        }
        .frame(width: layout.size.width + 2 * margin, height: layout.size.height)
    }

    /// Draws the switch, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool,
                                           connectedPins: Set<Int> = []) {
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

        // The part number, smaller when it is long (some are 20 characters).
        painter.draw(
            PaintedText(chip.partNumber, size: chip.partNumber.count > 14 ? 8 : 11, weight: .semibold, monospaced: true),
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

            if !connectedPins.contains(slot.pin.index) {
                painter.draw(
                    PaintedText(slot.pin.designator, size: 9, ink: .secondary),
                    at: CGPoint(x: isLeft ? tipX - 3 : tipX + 3, y: slot.centerY),
                    anchor: isLeft ? .trailing : .leading
                )
            }
        }

        guard let slide = chip.part.slideStates else { return }
        let circuit = slide.circuit
        let poles = circuit.hasPrefix("D") ? 2 : 1
        let throwing = circuit.hasSuffix("DT")
        let left = box.minX + 16, right = box.maxX - 16
        func slot(_ base: String, _ pole: Int) -> ChipLayout.PinSlot? {
            let name = poles == 1 ? base : "\(base)\(pole + 1)"
            return layout.slots.first { $0.pin.name == name }
        }
        let on = chip.switchesOn
        /// How far along each blade the dashed linkage joins them.
        let linkFraction: CGFloat = 0.55

        var linkPoints: [CGPoint] = []
        for pole in 0..<poles {
            // The pivot (the common terminal, or `A` on a one-throw pole) and its throws.
            let pivotSlot = throwing ? slot("C", pole) : slot("A", pole)
            let throwSlots = throwing ? [slot("A", pole), slot("B", pole)] : [slot("B", pole)]
            guard let pivotSlot, throwSlots.allSatisfy({ $0 != nil }) else { continue }
            let pivot = CGPoint(x: left, y: pivotSlot.centerY)
            let contacts = throwSlots.compactMap { $0 }.map { CGPoint(x: right, y: $0.centerY) }
            let firstContact = throwing ? 2 * pole : pole
            let closed = contacts.indices.first { index in
                on.indices.contains(firstContact + index) && on[firstContact + index]
            }

            // Leads from the pins to the pivot and the throws, and the dots.
            var leads = Path()
            leads.move(to: CGPoint(x: box.minX, y: pivot.y))
            leads.addLine(to: pivot)
            for contact in contacts {
                leads.move(to: contact)
                leads.addLine(to: CGPoint(x: box.maxX, y: contact.y))
            }
            painter.stroke(leads, with: outline.opacity(0.6), lineWidth: 1)
            for point in [pivot] + contacts {
                painter.fill(Path(ellipseIn: CGRect(x: point.x - 2.5, y: point.y - 2.5, width: 5, height: 5)),
                             with: outline)
            }

            // The blade: onto the connected throw; else half way (a one-throw
            // pole lifts it off its terminal).
            let tip: CGPoint
            if let closed {
                tip = contacts[closed]
            } else if throwing {
                tip = CGPoint(x: pivot.x + (right - left) * 0.6, y: pivot.y)
            } else {
                tip = CGPoint(x: right - 8, y: pivot.y - 9)
            }
            var blade = Path()
            blade.move(to: pivot)
            blade.addLine(to: tip)
            if closed != nil {
                painter.stroke(blade, with: .switchOn, lineWidth: 5)
            }
            painter.stroke(blade, with: outline, lineWidth: 2)
            linkPoints.append(CGPoint(x: pivot.x + (tip.x - pivot.x) * linkFraction,
                                      y: pivot.y + (tip.y - pivot.y) * linkFraction))
        }

        // The mechanical link between the two poles' blades.
        if linkPoints.count == 2 {
            var link = Path()
            link.move(to: linkPoints[0])
            link.addLine(to: linkPoints[1])
            painter.stroke(link, with: outline.opacity(0.6),
                           style: StrokeStyle(lineWidth: 1, dash: [3, 2]))
        }

        // The knob's slot, a dot for each position, and the knob at the current one.
        let slotY = box.maxY - ChipLayout.slideKnobStrip / 2
        let track = CGRect(x: left, y: slotY - 4, width: right - left, height: 8)
        painter.fill(Path(roundedRect: track, cornerRadius: 4), with: .paper)
        painter.stroke(Path(roundedRect: track, cornerRadius: 4), with: outline.opacity(0.6), lineWidth: 1)
        let knobSize = CGSize(width: 16, height: 12)
        func knobX(_ position: Int) -> CGFloat {
            let steps = max(slide.positions - 1, 1)
            return left + knobSize.width / 2 + (right - left - knobSize.width) * CGFloat(position - 1) / CGFloat(steps)
        }
        for position in 1...slide.positions {
            painter.fill(Path(ellipseIn: CGRect(x: knobX(position) - 1.5, y: slotY - 1.5, width: 3, height: 3)),
                         with: outline.opacity(0.6))
        }
        if let position = chip.slidePosition {
            let knob = CGRect(x: knobX(position) - knobSize.width / 2, y: slotY - knobSize.height / 2,
                              width: knobSize.width, height: knobSize.height)
            let connected = on.contains(true)
            painter.fill(Path(roundedRect: knob, cornerRadius: 2), with: connected ? .knobOn : outline)
            painter.stroke(Path(roundedRect: knob, cornerRadius: 2), with: outline, lineWidth: 1)
        }
    }
}
