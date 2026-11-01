import SwiftUI

/// Draws one placed 7-segment display: a box with the part number in the
/// header, segment pins on the left and common pins on the right (names
/// inside the box, designators outside), and the digits on a dark face in the
/// middle. Each LED the part has is drawn dim, or in the display's color while
/// it glows in play mode. Colon dots (L1, L2) and the raised dot (L3) sit
/// between digits, where the datasheet puts them.
///
/// Like `ChipBlockView`, the frame includes `ChipLayout.labelMargin` on each
/// side, and the body's top-left corner sits at `chip.origin`.
struct SevenSegmentSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool
    /// Per LED, in `DisplayInfo.leds` order: brightness 0…1 (its share of the
    /// last frame spent on). Empty outside play mode.
    var levels: [Double] = []

    var body: some View {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Color = highlighted ? .accentColor : .primary
        let light = DiodeSymbolView.lightColor(for: chip)

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

            if let display = chip.part.display {
                Self.drawFace(&context, display: display, box: box, levels: levels, light: light)
            }
        }
        .frame(width: layout.size.width + 2 * margin, height: layout.size.height)
    }

    // MARK: Face

    private static func drawFace(_ context: inout GraphicsContext, display: DisplayInfo,
                                 box: CGRect, levels: [Double], light: Color) {
        let digit = ChipLayout.sevenSegmentDigitSize
        let gap = ChipLayout.sevenSegmentDigitGap
        let faceWidth = ChipLayout.sevenSegmentFaceWidth(display)
        let faceHeight = digit.height + 16
        let bodyTop = ChipLayout.headerHeight
        let face = CGRect(
            x: box.midX - faceWidth / 2,
            y: bodyTop + (box.maxY - bodyTop - faceHeight) / 2,
            width: faceWidth,
            height: faceHeight
        )
        context.fill(Path(roundedRect: face, cornerRadius: 3), with: .color(Color(white: 0.12)))

        let top = face.minY + 8

        /// Left edge of a digit (1-based). Digits after the colon move right by its width.
        func digitX(_ index: Int) -> CGFloat {
            let colon = display.colonAfter > 0 && index > display.colonAfter ? ChipLayout.sevenSegmentColonWidth : 0
            return face.minX + gap + CGFloat(index - 1) * (digit.width + gap) + colon
        }

        /// Middle of the space after a digit.
        func gapCentre(after index: Int) -> CGFloat {
            let right = digitX(index) + digit.width
            return (right + digitX(index + 1)) / 2
        }

        for (index, led) in display.leds.enumerated() {
            // The unlit segment, then the light over it at its brightness.
            let level = index < levels.count ? levels[index] : 0
            let colors = level > 0 ? [Color.white.opacity(0.1), light.opacity(level)]
                                   : [Color.white.opacity(0.1)]

            if led.digit == 0 {
                // Colon and raised dot.
                let centre: CGPoint
                switch led.segment {
                case "L1": centre = CGPoint(x: gapCentre(after: display.colonAfter), y: top + digit.height * 0.3)
                case "L2": centre = CGPoint(x: gapCentre(after: display.colonAfter), y: top + digit.height * 0.7)
                default: centre = CGPoint(x: gapCentre(after: display.dotAfter), y: top + 1)
                }
                for color in colors {
                    context.fill(Path(ellipseIn: dot(at: centre)), with: .color(color))
                }
                continue
            }

            let x = digitX(led.digit)
            if let path = segmentPath(led.segment, x: x, y: top, size: digit) {
                for color in colors {
                    context.stroke(path, with: .color(color), style: StrokeStyle(lineWidth: 3, lineCap: .round))
                }
            } else if led.segment == "DP" || led.segment == "LDP" {
                let centreX = led.segment == "DP" ? x + digit.width + 4 : x - 4
                for color in colors {
                    context.fill(Path(ellipseIn: dot(at: CGPoint(x: centreX, y: top + digit.height))), with: .color(color))
                }
            }
        }
    }

    private static func dot(at centre: CGPoint) -> CGRect {
        CGRect(x: centre.x - 1.8, y: centre.y - 1.8, width: 3.6, height: 3.6)
    }

    /// The bar for one segment of a digit whose top-left corner is (x, y).
    /// HJ is the vertical bar through the middle that makes G a plus sign.
    private static func segmentPath(_ segment: String, x: CGFloat, y: CGFloat, size: CGSize) -> Path? {
        let w = size.width
        let h = size.height
        let inset: CGFloat = 2.5
        let ends: (CGPoint, CGPoint)
        switch segment {
        case "A": ends = (CGPoint(x: x + inset, y: y), CGPoint(x: x + w - inset, y: y))
        case "B": ends = (CGPoint(x: x + w, y: y + inset), CGPoint(x: x + w, y: y + h / 2 - inset))
        case "C": ends = (CGPoint(x: x + w, y: y + h / 2 + inset), CGPoint(x: x + w, y: y + h - inset))
        case "D": ends = (CGPoint(x: x + inset, y: y + h), CGPoint(x: x + w - inset, y: y + h))
        case "E": ends = (CGPoint(x: x, y: y + h / 2 + inset), CGPoint(x: x, y: y + h - inset))
        case "F": ends = (CGPoint(x: x, y: y + inset), CGPoint(x: x, y: y + h / 2 - inset))
        case "G": ends = (CGPoint(x: x + inset, y: y + h / 2), CGPoint(x: x + w - inset, y: y + h / 2))
        case "HJ": ends = (CGPoint(x: x + w / 2, y: y + h / 2 - 7), CGPoint(x: x + w / 2, y: y + h / 2 + 7))
        default: return nil
        }
        var path = Path()
        path.move(to: ends.0)
        path.addLine(to: ends.1)
        return path
    }
}
