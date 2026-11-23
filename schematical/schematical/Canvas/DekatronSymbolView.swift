import SwiftUI

/// Draws one placed dekatron: a box with the part number in the header,
/// cathode pins on the left and the anode and guides on the right (names
/// inside the box, designators outside), and the tube on a dark face in the
/// middle. The ten cathode positions are numerals round a ring, as the
/// glow sees them from the top of the bulb: a dim numeral for each, and the
/// one the glow rests on in the tube's color at its brightness.
///
/// Like `ChipBlockView`, the frame includes `ChipLayout.labelMargin` on each
/// side, and the body's top-left corner sits at `chip.origin`.
struct DekatronSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool
    /// Pins (`ChipPin.index`) with a wire on them: their numbers are not drawn.
    var connectedPins: Set<Int> = []
    /// Per cathode position 0-9: brightness 0…1. Empty outside play mode.
    var levels: [Double] = []

    var body: some View {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered,
                       connectedPins: connectedPins, levels: levels)
        }
        .frame(width: layout.size.width + 2 * margin, height: layout.size.height)
    }

    /// Draws the tube, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool,
                                           connectedPins: Set<Int> = [],
                                           levels: [Double]) {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let light = Ink.lamp(DiodeSymbolView.lightName(for: chip))
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
                slot.pin.paintedName(size: 9, weight: .medium, monospaced: true),
                at: CGPoint(x: isLeft ? edgeX + 4 : edgeX - 4, y: slot.centerY),
                anchor: isLeft ? .leading : .trailing
            )
        }

        drawFace(&painter, box: box, levels: levels, light: light)
    }

    // MARK: Face

    private static func drawFace<P: SchematicPainter>(_ painter: inout P, box: CGRect,
                                                      levels: [Double], light: Ink) {
        let size = ChipLayout.dekatronFaceSize
        let bodyTop = ChipLayout.headerHeight
        let face = CGRect(
            x: box.midX - size.width / 2,
            y: bodyTop + (box.maxY - bodyTop - size.height) / 2,
            width: size.width,
            height: size.height
        )
        let facePath = Path(ellipseIn: face)
        painter.fill(facePath, with: .displayFace)
        // Outlines the face where the fill is not drawn (the export).
        painter.stroke(facePath, with: .faceEdge, lineWidth: 1)

        // Cathode 0 at the top, the others clockwise.
        let centre = CGPoint(x: face.midX, y: face.midY)
        let radius = size.width / 2 - 10
        for position in 0..<10 {
            let angle = CGFloat(position) * 2 * .pi / 10 - .pi / 2
            let at = CGPoint(x: centre.x + radius * cos(angle), y: centre.y + radius * sin(angle))
            let level = position < levels.count ? levels[position] : 0
            let numeral = String(position)

            painter.draw(PaintedText(numeral, size: 9, weight: .regular, ink: .unlit), at: at, anchor: .center)
            if level > 0 {
                let glow = Path(ellipseIn: CGRect(x: at.x - 5.5, y: at.y - 5.5, width: 11, height: 11))
                painter.fill(glow, with: light.opacity(level * 0.35))
                painter.draw(PaintedText(numeral, size: 9, weight: .bold, ink: light.opacity(level)),
                             at: at, anchor: .center)
            }
        }
        // The anode, a dot at the centre of the ring.
        painter.fill(Path(ellipseIn: CGRect(x: centre.x - 1.8, y: centre.y - 1.8, width: 3.6, height: 3.6)),
                     with: .unlit)
    }
}
