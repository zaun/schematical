import SwiftUI

/// The reference ("U3", "D1") the Schematic draws above a part: centred
/// just outside the top of its frame, over a diode's, LED's or capacitor's
/// symbol, over a resistor's value, over a transistor's symbol, or over a pull resistor's resistance,
/// at three quarters of the block's part number size. Not drawn for power
/// symbols (VCC, GND), or when the part's Display Reference is off.
enum ReferenceLabel {
    /// Three quarters of the block header's part number (11 pt).
    static let fontSize: CGFloat = 11 * 0.75
    /// Room above the anchor for the text, for the view that draws it.
    static let height: CGFloat = 12

    /// Where the label's bottom centre goes, in the chip view's frame
    /// (`PlacedChip.viewOrigin` is (0, 0)); nil when none is drawn.
    static func anchor(for chip: PlacedChip) -> CGPoint? {
        guard chip.showsReference, !chip.reference.isEmpty else { return nil }
        let x = ChipLayout.labelMargin + chip.layout.size.width / 2
        switch chip.symbol {
        case .diode, .plainDiode:
            // Above the LED's light arrows (their tips at y = 4); diodes use
            // the same line, so the two kinds line up side by side.
            return CGPoint(x: x, y: 2)
        case .capacitor:
            // Above the polarized "+" (its top at y = 9) and the plates.
            return CGPoint(x: x, y: 6)
        case .resistor:
            // Above the value (`ResistorSymbolView`: 10 pt, centred on y = 6).
            return CGPoint(x: x, y: -1)
        case .transistor:
            // Above the symbol: over the collector or drain lead (y = 10)
            // and the BJT's circle (its top at y = 14), centred on the body.
            return CGPoint(x: x, y: 6)
        case .pullUp, .pullDown:
            // Over the resistance, left-aligned with it (`textAnchor`): the
            // value starts at body x = 30 and is centred on y = 50 (pull-up)
            // or 30 (pull-down), below the top pad.
            let valueY: CGFloat = chip.symbol == .pullUp ? 50 : 30
            return CGPoint(x: ChipLayout.labelMargin + 30, y: valueY + ChipLayout.stubLength - 6)
        case .vcc, .ground:
            // Power symbols have none: their label says what they are.
            return nil
        case .block, .oscillator, .barrelJack, .header, .dipSwitch, .tactileSwitch, .sevenSegment,
             .nixie, .characterLCD:
            // Just outside the frame's top edge.
            return CGPoint(x: x, y: -3)
        }
    }

    /// Which point of the text sits on `anchor`: its bottom centre, or for
    /// a pull resistor its bottom left, over the resistance.
    static func textAnchor(for chip: PlacedChip) -> UnitPoint {
        chip.symbol == .pullUp || chip.symbol == .pullDown ? .bottomLeading : .bottom
    }

    /// Draws the label, the chip view's frame top-left at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isHighlighted: Bool) {
        guard let anchor = anchor(for: chip) else { return }
        painter.draw(
            PaintedText(chip.reference, size: fontSize, weight: .semibold, monospaced: true,
                        ink: isHighlighted ? .accent : .primary),
            at: anchor,
            anchor: textAnchor(for: chip)
        )
    }
}

/// The live canvas's reference label for one chip: a strip `height` tall
/// whose bottom is the label's anchor. The parent offsets it to
/// `chip.viewOrigin` plus `offsetY(for:)`.
struct ReferenceLabelView: View {
    let chip: PlacedChip
    let isHighlighted: Bool

    /// The strip's top, relative to the chip view's frame.
    static func offsetY(for chip: PlacedChip) -> CGFloat? {
        ReferenceLabel.anchor(for: chip).map { $0.y - ReferenceLabel.height }
    }

    var body: some View {
        let width = chip.layout.size.width + 2 * ChipLayout.labelMargin
        Canvas { context, _ in
            var painter = ScreenPainter(context)
            painter.translateBy(x: 0, y: -(Self.offsetY(for: chip) ?? 0))
            ReferenceLabel.paint(&painter, chip: chip, isHighlighted: isHighlighted)
        }
        .frame(width: width, height: ReferenceLabel.height)
        .allowsHitTesting(false)
    }
}
