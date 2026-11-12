import SwiftUI

/// Draws one placed capacitor (plan-33-H): two 12 pt plates 4 pt apart
/// between the leads, pin 1 on the left. A polarized capacitor's right
/// (negative) plate is curved and a `+` sits above-left of its left
/// (positive) plate. Its value sits below: the generic capacitor's own
/// (`PlacedChip.farads`), else the part's function (its capacitance). No
/// glow and nothing in play mode.
///
/// Like `DiodeSymbolView`, the frame includes `ChipLayout.labelMargin` on
/// each side, and the body's top-left corner sits at `chip.origin`.
struct CapacitorSymbolView: View {
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

    /// The text under the symbol.
    static func label(for chip: PlacedChip) -> String {
        if let farads = chip.farads { return CapacitorValue.text(farads) }
        return chip.part.function
    }

    /// Whether the part has a + and a - pin.
    static func isPolarized(_ part: CatalogPart) -> Bool {
        if case let .capacitor(_, polarized, _, _, _, _) = part.details { return polarized }
        return false
    }

    /// Draws the capacitor, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let mid = ChipLayout.diodeCentreY
        let width = layout.size.width
        let flipped = chip.isFlipped
        let left = width / 2 - 2
        let right = width / 2 + 2

        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint {
            CGPoint(x: (flipped ? width - x : x) + margin, y: y)
        }

        var leads = Path()
        leads.move(to: point(-stub, mid))
        leads.addLine(to: point(left, mid))
        leads.move(to: point(right, mid))
        leads.addLine(to: point(width + stub, mid))
        painter.stroke(leads, with: outline, lineWidth: lineWidth)

        var plates = Path()
        plates.move(to: point(left, mid - 6))
        plates.addLine(to: point(left, mid + 6))
        if isPolarized(chip.part) {
            // The negative plate: an arc bowed toward the positive one.
            plates.move(to: point(right + 2, mid - 6))
            plates.addQuadCurve(to: point(right + 2, mid + 6), control: point(right - 2, mid))
        } else {
            plates.move(to: point(right, mid - 6))
            plates.addLine(to: point(right, mid + 6))
        }
        painter.stroke(plates, with: outline, lineWidth: lineWidth)

        if isPolarized(chip.part) {
            var plus = Path()
            plus.move(to: point(left - 7, mid - 9))
            plus.addLine(to: point(left - 3, mid - 9))
            plus.move(to: point(left - 5, mid - 11))
            plus.addLine(to: point(left - 5, mid - 7))
            painter.stroke(plus, with: outline, lineWidth: 1.25)
        }

        painter.draw(
            PaintedText(label(for: chip), size: 10, weight: .medium, ink: .secondary),
            at: point(width / 2, layout.size.height - 8),
            anchor: .center
        )
    }
}
