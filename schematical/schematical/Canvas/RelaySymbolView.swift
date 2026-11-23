import SwiftUI

/// Draws one placed relay (plan-47-N): a box with the part number in the
/// header, the coil pins on the left and each pole's `NO`, `COM` and `NC`
/// pins on the right (`RelayShape` has the rows).
///
/// Inside the box a coil (two rectangles for a dual-coil latching relay,
/// labelled S and R; an LED for a solid-state relay's input) with the coil
/// voltage under it, a dashed line from the coil to the poles, and per pole
/// an arm pivoting at `COM`: at rest it touches `NC` (a Form A pole's arm
/// rests open, short of `NO`); operated it touches `NO` (a Form B pole's arm
/// opens). In play mode the state comes from the simulation: the relay is
/// operated while its channel's light is above one half
/// (`SimulationController.lights`, one entry per coil, 0 for a latching
/// relay). Outside play mode every arm is at rest.
///
/// Like `TactileSwitchSymbolView`, the frame includes `ChipLayout.labelMargin`
/// on each side, and the body's top-left corner sits at `chip.origin`.
struct RelaySymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool
    /// Pins (`ChipPin.index`) with a wire on them: their numbers are not drawn.
    var connectedPins: Set<Int> = []
    /// Play mode: per channel, the share of the last frame the relay was
    /// operated (0…1); empty outside play mode.
    var levels: [Double] = []

    var body: some View {
        let frame = chip.symbolFrameSize

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered,
                       connectedPins: connectedPins, levels: levels)
        }
        .frame(width: frame.width, height: frame.height)
    }

    /// Whether the pole on `channel` is operated, from the part's lights.
    static func isOperated(channel: Int, levels: [Double]) -> Bool {
        levels.indices.contains(channel) && levels[channel] > 0.5
    }

    /// Draws the relay, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool,
                                           connectedPins: Set<Int> = [], levels: [Double] = []) {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let box = CGRect(x: margin, y: 0, width: layout.size.width, height: layout.size.height)
        let width = layout.size.width

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
            PaintedText(chip.partNumber,
                        size: PaintedText.monospacedSize(11, characters: chip.partNumber.count, fitting: width - 10),
                        weight: .semibold, monospaced: true),
            at: CGPoint(x: box.midX, y: ChipLayout.headerHeight / 2),
            anchor: .center
        )

        // Pin stubs, the pins' designators outside the box and their names
        // inside it, as on a block.
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
            painter.draw(
                PaintedText(slot.pin.name, size: 8, ink: .secondary),
                at: CGPoint(x: isLeft ? edgeX + 3 : edgeX - 3, y: slot.centerY - 6),
                anchor: isLeft ? .leading : .trailing
            )
        }

        guard let shape = RelayShape(part: chip.part) else { return }

        // The symbol's own coordinates (body x from the left edge, y from the
        // top), flipped with the part.
        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint { chip.symbolPoint(x, y) }
        func rowY(_ row: Int) -> CGFloat { ChipLayout.headerHeight + ChipLayout.pitch * CGFloat(row) + ChipLayout.pitch / 2 }
        let inner: Ink = outline.opacity(0.8)

        // The coils: a rectangle between two pins, or the LED of a
        // solid-state relay's input.
        let coilX: CGFloat = 36
        let coilHalfWidth: CGFloat = 7
        let reach: CGFloat = 8
        for (index, coil) in shape.coils.enumerated() {
            let top = rowY(coil.topRow)
            let bottom = rowY(coil.bottomRow)
            let operated = isOperated(channel: index, levels: levels)

            var leads = Path()
            leads.move(to: point(0, top))
            leads.addLine(to: point(coilX, top))
            leads.addLine(to: point(coilX, top + reach))
            leads.move(to: point(0, bottom))
            leads.addLine(to: point(coilX, bottom))
            leads.addLine(to: point(coilX, bottom - reach))
            painter.stroke(leads, with: outline, lineWidth: 1.5)

            if shape.isSolidState {
                // The input LED: anode at the top, a triangle pointing down
                // to the bar, two arrows toward the output.
                let mid = (top + bottom) / 2
                let lit: Ink = operated ? .lamp("Red") : outline
                var triangle = Path()
                triangle.move(to: point(coilX - 6, mid - 5))
                triangle.addLine(to: point(coilX + 6, mid - 5))
                triangle.addLine(to: point(coilX, mid + 5))
                triangle.closeSubpath()
                painter.fill(triangle, with: lit)
                var bar = Path()
                bar.move(to: point(coilX - 6, mid + 5))
                bar.addLine(to: point(coilX + 6, mid + 5))
                painter.stroke(bar, with: outline, lineWidth: 1.5)
                var arrows = Path()
                for dy: CGFloat in [-6, 0] {
                    arrows.move(to: point(coilX + 9, mid + dy))
                    arrows.addLine(to: point(coilX + 16, mid + dy - 5))
                }
                painter.stroke(arrows, with: inner, lineWidth: 1)
            } else {
                let rect = CGRect(x: coilX - coilHalfWidth, y: top + reach, width: 2 * coilHalfWidth,
                                  height: bottom - top - 2 * reach)
                var path = Path()
                path.move(to: point(rect.minX, rect.minY))
                path.addLine(to: point(rect.maxX, rect.minY))
                path.addLine(to: point(rect.maxX, rect.maxY))
                path.addLine(to: point(rect.minX, rect.maxY))
                path.closeSubpath()
                // A coil that is energised glows like a closed contact; a
                // latching relay's two coils never do.
                if operated, !shape.isLatching {
                    painter.fill(path, with: Ink.switchOn)
                }
                painter.stroke(path, with: outline, lineWidth: 1.5)
                if let label = coil.label {
                    painter.draw(
                        PaintedText(label, size: 9, weight: .medium, ink: .secondary),
                        at: point(coilX + coilHalfWidth + 4, (top + bottom) / 2),
                        anchor: chip.isFlipped ? .trailing : .leading
                    )
                }
            }
        }

        // The coil voltage, under the lowest coil pin.
        if let info = chip.part.relay, info.coilMV > 0, let lowest = shape.coils.map(\.bottomRow).max() {
            painter.draw(
                PaintedText(PartFacts.volts(mV: info.coilMV), size: 9, weight: .medium, ink: .secondary),
                at: point(coilX, rowY(lowest) + 13),
                anchor: .center
            )
        }

        // The mechanical link: dashed from each coil to the poles it moves.
        let tipX = width - 28
        let pivotX = width - 56
        let linkX = width - 66
        var link = Path()
        for (index, coil) in shape.coils.enumerated() {
            let mid = (rowY(coil.topRow) + rowY(coil.bottomRow)) / 2
            let linkStart = shape.isSolidState ? coilX + 18 : coilX + coilHalfWidth + (coil.label == nil ? 3 : 14)
            let column = linkX - 4 * CGFloat(index % 3)
            let poles = shape.poles.filter { shape.isLatching || $0.channel == index }
            guard !poles.isEmpty else { continue }
            link.move(to: point(linkStart, mid))
            link.addLine(to: point(column, mid))
            let ys = poles.map { rowY($0.commonRow) } + [mid]
            link.move(to: point(column, ys.min() ?? mid))
            link.addLine(to: point(column, ys.max() ?? mid))
            for pole in poles {
                link.move(to: point(column, rowY(pole.commonRow)))
                link.addLine(to: point(pivotX - 3, rowY(pole.commonRow)))
            }
        }
        painter.stroke(link, with: inner, style: StrokeStyle(lineWidth: 1, dash: [3, 2]))

        // The poles: a lead to each terminal, the arm pivoting at `COM`.
        for pole in shape.poles {
            let operated = isOperated(channel: pole.channel, levels: levels)
            let comY = rowY(pole.commonRow)
            var leads = Path()
            leads.move(to: point(width, comY))
            leads.addLine(to: point(pivotX, comY))
            var dots: [CGPoint] = [point(pivotX, comY)]
            if let row = pole.noRow {
                leads.move(to: point(width, rowY(row)))
                leads.addLine(to: point(tipX, rowY(row)))
                dots.append(point(tipX, rowY(row)))
            }
            if let row = pole.ncRow {
                leads.move(to: point(width, rowY(row)))
                leads.addLine(to: point(tipX, rowY(row)))
                dots.append(point(tipX, rowY(row)))
            }
            painter.stroke(leads, with: outline, lineWidth: 1.5)
            for dot in dots {
                painter.fill(Path(ellipseIn: CGRect(x: dot.x - 2.5, y: dot.y - 2.5, width: 5, height: 5)), with: outline)
            }

            // Where the arm ends: on the terminal it makes, or open.
            let tip: (x: CGFloat, y: CGFloat)
            let closed: Bool
            if operated {
                if let row = pole.noRow {
                    tip = (tipX - 2.5, rowY(row)); closed = true
                } else {
                    tip = (tipX - 3, comY + 9); closed = false
                }
            } else {
                if let row = pole.ncRow {
                    tip = (tipX - 2.5, rowY(row)); closed = true
                } else {
                    tip = (tipX - 3, comY - 9); closed = false
                }
            }
            var arm = Path()
            arm.move(to: point(pivotX, comY))
            arm.addLine(to: point(tip.x, tip.y))
            painter.stroke(arm, with: closed ? Ink.switchOn : outline, lineWidth: closed ? 2.5 : 2)
        }
    }
}
