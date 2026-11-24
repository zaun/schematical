import SwiftUI

/// A test point (plan-57): a small open circle at the end of a lead, with the
/// pin's stub beyond it. The reference is drawn above the frame by
/// `ReferenceLabel`; there is no body rectangle and no pin text.
///
/// Like `HeaderSymbolView`, the frame includes `ChipLayout.labelMargin` on
/// each side and the body's top-left corner sits at `chip.origin`.
struct TestPointSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool

    /// The circle's diameter.
    static let circleDiameter: CGFloat = 10

    var body: some View {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered)
        }
        .frame(width: layout.size.width + 2 * margin, height: layout.size.height)
    }

    /// Draws the lead and the circle, the frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let width = layout.size.width

        for slot in layout.slots {
            let left = slot.side == .left
            let y = slot.centerY
            // The stub's outer end (the wire's tip) and the circle's centre,
            // which sits in the middle of the body.
            let tip = left ? margin - stub : margin + width + stub
            let centre = CGPoint(x: margin + width / 2, y: y)
            let inner = centre.x + (left ? -1 : 1) * circleDiameter / 2
            var lead = Path()
            lead.move(to: CGPoint(x: tip, y: y))
            lead.addLine(to: CGPoint(x: inner, y: y))
            painter.stroke(lead, with: outline, lineWidth: 1.5)
            let circle = Path(ellipseIn: CGRect(x: centre.x - circleDiameter / 2, y: y - circleDiameter / 2,
                                                width: circleDiameter, height: circleDiameter))
            painter.fill(circle, with: .paper)
            painter.stroke(circle, with: outline, lineWidth: lineWidth)
        }
    }
}
