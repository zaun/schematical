import SwiftUI

/// Standard barrel-jack schematic: the part number is above the jack, not
/// in a package box. Three-terminal jacks show the normally-closed sleeve
/// contact found in their datasheet schematic. The jack flips and turns with
/// the part (`PlacedChip.symbolPoint`); its texts move with it and stay
/// upright.
struct BarrelJackSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool

    var body: some View {
        let frame = chip.symbolFrameSize

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered)
        }
        .frame(width: frame.width, height: frame.height)
    }

    /// Where the part number goes, in the view's frame, and which point of
    /// it sits there: centred on the band above the jack, or, with the part
    /// on its side, starting on that band and reading away from the jack.
    static func nameLabel(for chip: PlacedChip) -> (point: CGPoint, anchor: UnitPoint) {
        let spot = chip.symbolPoint(chip.unturnedSize.width / 2, 9)
        switch chip.turnedSide(.top) {
        case .left: return (spot, .trailing)
        case .right: return (spot, .leading)
        case .top, .bottom: return (spot, .center)
        }
    }

    /// Draws the jack, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let stub = ChipLayout.stubLength
        let width = chip.unturnedSize.width
        let color: Ink = isSelected || isHovered ? .accent : .primary
        let hasSwitch = chip.part.pins.contains { $0.name == "Switch" }
        // The rows of the unturned jack's pins (`ChipLayout.barrelJack`).
        let centreY: CGFloat = 30, switchY: CGFloat = 50, sleeveY: CGFloat = 75

        // The symbol's own coordinates, flipped and turned with the part.
        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint { chip.symbolPoint(x, y) }
        func circle(_ x: CGFloat, _ y: CGFloat) -> Path {
            let centre = point(x, y)
            return Path(ellipseIn: CGRect(x: centre.x - 4, y: centre.y - 4, width: 8, height: 8))
        }
        // Part number above the schematic, centred on its image.
        let name = nameLabel(for: chip)
        painter.draw(PaintedText(chip.partNumber, size: 11, weight: .semibold, monospaced: true),
                     at: name.point, anchor: name.anchor)

        let tube = chip.symbolRect(CGRect(x: 8, y: centreY - 8, width: 76, height: 16))
        painter.stroke(Path(roundedRect: tube, cornerRadius: 8), with: color, lineWidth: 1.5)
        let collar = chip.symbolRect(CGRect(x: 84, y: centreY - 13, width: 12, height: 26))
        painter.stroke(Path(collar), with: color, lineWidth: 1.5)

        var lines = Path()
        lines.move(to: point(96, centreY)); lines.addLine(to: point(110, centreY))
        lines.move(to: point(118, centreY)); lines.addLine(to: point(width + stub, centreY))
        if hasSwitch {
            lines.move(to: point(96, switchY)); lines.addLine(to: point(110, switchY))
            lines.move(to: point(118, switchY)); lines.addLine(to: point(width + stub, switchY))
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

        // Each pin's number, past the end of its lead. On its side the lead
        // runs up or down, where the wire goes, so the number stands beside it.
        let rows = ["Centre": centreY, "Switch": switchY, "Sleeve": sleeveY]
        let onItsSide = chip.rotation == 90 || chip.rotation == 270
        for slot in chip.layout.slots {
            guard let y = rows[slot.pin.name] else { continue }
            let text = PaintedText(slot.pin.designator, size: 10)
            if onItsSide {
                let spot = point(width + stub - 4, y)
                painter.draw(text, at: CGPoint(x: spot.x + 5, y: spot.y), anchor: .leading)
            } else {
                let label = chip.symbolText(width + stub + 5, y, toward: .right)
                painter.draw(text, at: label.point, anchor: label.anchor)
            }
        }
    }
}
