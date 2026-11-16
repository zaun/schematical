import SwiftUI

/// Draws one placed nixie tube: a box with the part number in the header,
/// cathode pins on the left and the anode on the right (names inside the
/// box, designators outside), and the tube on a dark face in the middle.
/// The cathodes sit one behind the other, as in the tube, so every glyph is
/// drawn at the same place: the first one dim while the tube is dark, and
/// each glowing one in the tube's color at its brightness. Decimal points are dots at the bottom
/// corners.
///
/// Like `ChipBlockView`, the frame includes `ChipLayout.labelMargin` on each
/// side, and the body's top-left corner sits at `chip.origin`.
struct NixieSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool
    /// Pins (`ChipPin.index`) with a wire on them: their numbers are not drawn.
    var connectedPins: Set<Int> = []
    /// Per cathode, in `NixieInfo.glyphs` order: brightness 0…1 (its share of
    /// the last frame spent on). Empty outside play mode.
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

        if let nixie = chip.part.nixie {
            drawFace(&painter, nixie: nixie, box: box, levels: levels, light: light)
        }
    }

    // MARK: Face

    private static func drawFace<P: SchematicPainter>(_ painter: inout P, nixie: NixieInfo,
                                                      box: CGRect, levels: [Double], light: Ink) {
        let size = ChipLayout.nixieFaceSize
        let bodyTop = ChipLayout.headerHeight
        let face = CGRect(
            x: box.midX - size.width / 2,
            y: bodyTop + (box.maxY - bodyTop - size.height) / 2,
            width: size.width,
            height: size.height
        )
        // The envelope: a dome at the top, as a side-view tube stands.
        let facePath = Path(roundedRect: face, cornerRadius: size.width / 2 - 2)
        painter.fill(facePath, with: .displayFace)
        // Outlines the face where the fill is not drawn (the export).
        painter.stroke(facePath, with: .faceEdge, lineWidth: 1)

        let centre = CGPoint(x: face.midX, y: face.midY - 1)
        let dotY = face.maxY - 11
        // The front cathode is drawn dark only while no character glows.
        let anyLit = nixie.glyphs.enumerated().contains { index, glyph in
            glyph != "DP" && glyph != "LDP" && index < levels.count && levels[index] > 0
        }
        var drewUnlit = anyLit

        for (index, glyph) in nixie.glyphs.enumerated() {
            let level = index < levels.count ? levels[index] : 0

            if glyph == "DP" || glyph == "LDP" {
                // A dim dot, then the light over it at its brightness.
                let x = glyph == "DP" ? face.maxX - 8 : face.minX + 8
                let dot = Path(ellipseIn: CGRect(x: x - 1.8, y: dotY - 1.8, width: 3.6, height: 3.6))
                painter.fill(dot, with: .unlit)
                if level > 0 {
                    painter.fill(dot, with: light.opacity(level))
                }
                continue
            }
            // Long symbols ("Hz") are drawn smaller, to stay inside the glass.
            let points: CGFloat = glyph.count > 1 ? 15 : 26
            if !drewUnlit {
                // The front cathode, dark.
                painter.draw(PaintedText(glyph, size: points, weight: .light, ink: .unlit),
                             at: centre, anchor: .center)
                drewUnlit = true
            }
            if level > 0 {
                painter.draw(PaintedText(glyph, size: points, weight: .light, ink: light.opacity(level)),
                             at: centre, anchor: .center)
            }
        }
    }
}
