import SwiftUI

/// A test point (plan-57): a small open circle at the end of a lead, with the
/// pin's stub beyond it. The reference is drawn above the frame by
/// `ReferenceLabel`; there is no body rectangle and no pin text.
///
/// Like `ResistorSymbolView`, the frame is `PlacedChip.symbolFrameSize` and
/// the body's top-left corner sits at `chip.origin`; the lead and the circle
/// flip and turn with the part.
struct TestPointSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool

    /// The circle's diameter.
    static let circleDiameter: CGFloat = 10

    var body: some View {
        let frame = chip.symbolFrameSize

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered)
        }
        .frame(width: frame.width, height: frame.height)
    }

    /// The circle's centre, in the view's frame: the middle of the body.
    static func circleCentre(of chip: PlacedChip) -> CGPoint {
        chip.symbolPoint(chip.unturnedSize.width / 2, chip.unturnedSize.height / 2)
    }

    /// Draws the lead and the circle, the frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        guard !chip.layout.slots.isEmpty else { return }
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let size = chip.unturnedSize

        // Unturned, the pin is on the left: the lead runs from the stub's
        // outer end (the wire's tip) to the circle, in the middle of the body.
        let centre = circleCentre(of: chip)
        var lead = Path()
        lead.move(to: chip.symbolPoint(-stub, size.height / 2))
        lead.addLine(to: chip.symbolPoint(size.width / 2 - circleDiameter / 2, size.height / 2))
        painter.stroke(lead, with: outline, lineWidth: 1.5)
        let circle = Path(ellipseIn: CGRect(x: centre.x - circleDiameter / 2, y: centre.y - circleDiameter / 2,
                                            width: circleDiameter, height: circleDiameter))
        painter.fill(circle, with: .paper)
        painter.stroke(circle, with: outline, lineWidth: lineWidth)
    }
}
