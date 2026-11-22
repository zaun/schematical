import SwiftUI

/// Where a two-terminal symbol's drawing goes once the part is flipped and
/// turned (LEDs, diodes, resistors and capacitors; `PartSymbol.canRotate`).
///
/// Each symbol is drawn once, in its own coordinates: left to right, pin 1
/// on the left, the leads on the row `ChipLayout.diodeCentreY`, in a body
/// `PlacedChip.unturnedSize` big. `symbolPoint` carries a point of that
/// drawing to the view's frame, so the flip and the turn move every line,
/// arrowhead and mark together. Text is placed with it but never turned.
extension PlacedChip {
    /// The frame's top-left corner sits at (-label margin, -stub) from the
    /// body's when the pins are on the top and bottom, else at
    /// (-label margin, 0). Matches `viewOrigin`.
    private var frameInset: CGPoint {
        CGPoint(x: ChipLayout.labelMargin, y: hasVerticalPins ? ChipLayout.stubLength : 0)
    }

    /// A point of the unturned symbol, in the view's frame.
    func symbolPoint(_ x: CGFloat, _ y: CGFloat) -> CGPoint {
        let size = unturnedSize
        var point = CGPoint(x: isFlipped ? size.width - x : x, y: y)
        var w = size.width, h = size.height
        for _ in 0..<(rotation / 90) {
            point = CGPoint(x: h - point.y, y: point.x)
            swap(&w, &h)
        }
        return CGPoint(x: point.x + frameInset.x, y: point.y + frameInset.y)
    }

    /// Where a text at `y` on the unturned symbol's centre line goes, and which
    /// point of it sits there. Text is never turned. On its side the part is
    /// narrow and the text would touch it, so the text stands clear of the
    /// pin line, on the side it turned to, its near edge on the anchor.
    func symbolLabel(y: CGFloat) -> (point: CGPoint, anchor: UnitPoint) {
        let size = unturnedSize
        let spot = symbolPoint(size.width / 2, y)
        guard rotation == 90 || rotation == 270 else { return (spot, .center) }
        let axis = symbolPoint(size.width / 2, ChipLayout.diodeCentreY).x
        let gap: CGFloat = 12
        return spot.x < axis ? (CGPoint(x: axis - gap, y: spot.y), .trailing)
                             : (CGPoint(x: axis + gap, y: spot.y), .leading)
    }

    /// The view's frame: the body, its label margin each side, and the stub
    /// room above and below when the pins are on the top and bottom.
    var symbolFrameSize: CGSize {
        let size = layout.size
        return CGSize(width: size.width + 2 * ChipLayout.labelMargin,
                      height: size.height + (hasVerticalPins ? 2 * ChipLayout.stubLength : 0))
    }
}
