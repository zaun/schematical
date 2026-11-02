import SwiftUI

/// Draws one placed character LCD module: a box with the part number in the
/// header, the bus pins on the left and the power pins on the right (names
/// inside the box, designators outside), and the display face in the middle.
/// In play mode the face shows the dots the controller displays (the
/// backend's `LcdState` image), lit by the backlight; outside it, every dot
/// is unlit with the backlight off.
///
/// Like `ChipBlockView`, the frame includes `ChipLayout.labelMargin` on each
/// side, and the body's top-left corner sits at `chip.origin`.
struct CharacterLCDSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool
    /// The dot image of the last frame; nil outside play mode.
    var frame: LCDFrame?
    /// Backlight level 0…1 (its share of the last frame spent on).
    var backlight: Double = 0

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

            for slot in layout.slots {
                let isLeft = slot.side == .left
                let edgeX = isLeft ? box.minX : box.maxX
                let tipX = isLeft ? box.minX - stub : box.maxX + stub

                var pinStub = Path()
                pinStub.move(to: CGPoint(x: tipX, y: slot.centerY))
                pinStub.addLine(to: CGPoint(x: edgeX, y: slot.centerY))
                context.stroke(pinStub, with: .color(outline), lineWidth: 1.5)

                context.draw(
                    Text(slot.pin.designator)
                        .font(.system(size: 9))
                        .foregroundStyle(.secondary),
                    at: CGPoint(x: isLeft ? tipX - 3 : tipX + 3, y: slot.centerY),
                    anchor: isLeft ? .trailing : .leading
                )
                context.draw(
                    Text(slot.pin.name)
                        .font(.system(size: 9, weight: .medium, design: .monospaced)),
                    at: CGPoint(x: isLeft ? edgeX + 4 : edgeX - 4, y: slot.centerY),
                    anchor: isLeft ? .leading : .trailing
                )
            }

            if let info = chip.part.characterLCD {
                Self.drawFace(&context, info: info, box: box, frame: frame,
                              backlight: frame == nil ? 0 : backlight)
            }
        }
        .frame(width: layout.size.width + 2 * margin, height: layout.size.height)
    }

    // MARK: Face

    /// An RGB color, for blending.
    private struct RGB {
        var r, g, b: Double

        init(_ hex: UInt32) {
            r = Double((hex >> 16) & 0xFF) / 255
            g = Double((hex >> 8) & 0xFF) / 255
            b = Double(hex & 0xFF) / 255
        }

        init(r: Double, g: Double, b: Double) {
            self.r = r
            self.g = g
            self.b = b
        }

        func mix(_ other: RGB, _ t: Double) -> RGB {
            RGB(r: r + (other.r - r) * t, g: g + (other.g - g) * t, b: b + (other.b - b) * t)
        }

        func darkened(_ fraction: Double) -> RGB {
            RGB(r: r * (1 - fraction), g: g * (1 - fraction), b: b * (1 - fraction))
        }

        var color: Color { Color(red: r, green: g, blue: b) }
    }

    /// The face's colors for a panel at a backlight level (plan-11-D §4.2).
    private static func palette(_ panel: LCDPanel, backlight: Double)
        -> (background: RGB, lit: RGB, unlit: RGB, unknown: RGB) {
        let t = min(1, max(0, backlight))
        let background: RGB, lit: RGB
        switch panel {
        case .yellowGreen:
            background = RGB(0x7A8A3A).mix(RGB(0xA9C53C), t)
            lit = RGB(0x1E2810)
        case .blueNegative:
            background = RGB(0x0E1D5C).mix(RGB(0x1F45D6), t)
            lit = RGB(0x34467F).mix(RGB(0xEAF2FF), t)
        }
        let unlit = background.darkened(0.06)
        return (background, lit, unlit, lit.mix(unlit, 0.5))
    }

    private static func drawFace(_ context: inout GraphicsContext, info: CharacterLCDInfo, box: CGRect,
                                 frame: LCDFrame?, backlight: Double) {
        let size = ChipLayout.lcdFaceSize(info)
        let bodyTop = ChipLayout.headerHeight
        let face = CGRect(
            x: box.midX - size.width / 2,
            y: bodyTop + (box.maxY - bodyTop - size.height) / 2,
            width: size.width,
            height: size.height
        )
        let colors = palette(info.panel, backlight: backlight)
        context.fill(Path(roundedRect: face, cornerRadius: 3), with: .color(colors.background.color))

        let lit = colors.lit.color, unlit = colors.unlit.color, unknown = colors.unknown.color
        let pitch = ChipLayout.lcdDotPitch, dotSize = ChipLayout.lcdDot
        for row in 0..<info.rows {
            for column in 0..<info.columns {
                let cellX = face.minX + ChipLayout.lcdFaceMargin + CGFloat(column) * ChipLayout.lcdCellPitch.width
                let cellY = face.minY + ChipLayout.lcdFaceMargin + CGFloat(row) * ChipLayout.lcdCellPitch.height
                for y in 0..<Int(LCD_STATE_CELL_HEIGHT) {
                    for x in 0..<Int(LCD_STATE_CELL_WIDTH) {
                        let value = frame?.dot(x: column * Int(LCD_STATE_CELL_WIDTH) + x,
                                               y: row * Int(LCD_STATE_CELL_HEIGHT) + y)
                            ?? UInt8(LCD_DOT_OFF)
                        let color = value == UInt8(LCD_DOT_ON) ? lit
                            : value == UInt8(LCD_DOT_X) ? unknown : unlit
                        let rect = CGRect(x: cellX + CGFloat(x) * pitch, y: cellY + CGFloat(y) * pitch,
                                          width: dotSize, height: dotSize)
                        context.fill(Path(rect), with: .color(color))
                    }
                }
            }
        }
    }
}
