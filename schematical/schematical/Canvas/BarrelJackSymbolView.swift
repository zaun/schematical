import SwiftUI

/// Standard barrel-jack schematic: the part number is above the jack, not
/// in a package box. Three-terminal jacks show the normally-closed sleeve
/// contact found in their datasheet schematic.
struct BarrelJackSymbolView: View {
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

    /// Draws the jack, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let color: Ink = isSelected || isHovered ? .accent : .primary
        let hasSwitch = chip.part.pins.contains { $0.name == "Switch" }
        let byName = Dictionary(layout.slots.map { ($0.pin.name, $0) }, uniquingKeysWith: { first, _ in first })
        let centreY = byName["Centre"]?.centerY ?? 30
        let switchY = byName["Switch"]?.centerY ?? 50
        let sleeveY = byName["Sleeve"]?.centerY ?? 75

        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint {
            CGPoint(x: chip.isFlipped ? layout.size.width - x + margin : x + margin, y: y)
        }
        func circle(_ x: CGFloat, _ y: CGFloat) -> Path {
            Path(ellipseIn: CGRect(x: point(x, y).x - 4, y: y - 4, width: 8, height: 8))
        }
        // Part number above the schematic, centred on its image.
        painter.draw(PaintedText(chip.partNumber, size: 11, weight: .semibold, monospaced: true),
                     at: point(layout.size.width / 2, 9), anchor: .center)

        let tube = CGRect(x: point(8, centreY - 8).x, y: centreY - 8,
                          width: 76, height: 16)
        painter.stroke(Path(roundedRect: tube, cornerRadius: 8), with: color, lineWidth: 1.5)
        let collar = CGRect(x: point(84, centreY - 13).x, y: centreY - 13, width: 12, height: 26)
        painter.stroke(Path(collar), with: color, lineWidth: 1.5)

        var lines = Path()
        lines.move(to: point(96, centreY)); lines.addLine(to: point(110, centreY))
        lines.move(to: point(118, centreY)); lines.addLine(to: point(layout.size.width + stub, centreY))
        if hasSwitch {
            lines.move(to: point(96, switchY)); lines.addLine(to: point(110, switchY))
            lines.move(to: point(118, switchY)); lines.addLine(to: point(layout.size.width + stub, switchY))
            lines.move(to: point(66, sleeveY)); lines.addLine(to: point(74, sleeveY - 10))
            lines.addLine(to: point(84, sleeveY)); lines.addLine(to: point(110, sleeveY))
        } else {
            lines.move(to: point(62, sleeveY)); lines.addLine(to: point(73, sleeveY - 11))
            lines.addLine(to: point(84, sleeveY)); lines.addLine(to: point(110, sleeveY))
        }
        painter.stroke(lines, with: color, lineWidth: 1.5)
        painter.fill(circle(114, centreY), with: .paper)
        painter.stroke(circle(114, centreY), with: color, lineWidth: 1.5)
        if hasSwitch {
            painter.fill(circle(114, switchY), with: .paper)
            painter.stroke(circle(114, switchY), with: color, lineWidth: 1.5)
        }
        painter.fill(circle(114, sleeveY), with: .paper)
        painter.stroke(circle(114, sleeveY), with: color, lineWidth: 1.5)

        for slot in layout.slots {
            painter.draw(PaintedText(slot.pin.designator, size: 10),
                         at: point(layout.size.width + stub + 5, slot.centerY), anchor: .leading)
        }
    }
}
