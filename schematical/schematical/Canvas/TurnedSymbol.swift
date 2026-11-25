import SwiftUI

/// Where a symbol's drawing goes once the part is flipped and turned (every
/// part drawn as its own symbol; `PartSymbol.canRotate`).
///
/// Each symbol is drawn once, in its own coordinates, as it stands unturned
/// (a two-terminal part left to right, pin 1 on the left, the leads on the
/// row `ChipLayout.diodeCentreY`), in a body `PlacedChip.unturnedSize` big. `symbolPoint` carries a point of that
/// drawing to the view's frame, so the flip and the turn move every line,
/// arrowhead and mark together. Text is placed with it but never turned.
extension PlacedChip {
    /// The frame's top-left corner sits at (-label margin, -stub) from the
    /// body's when the pins are on the top and bottom, else at
    /// (-label margin, 0). Matches `viewOrigin`.
    private var frameInset: CGPoint {
        CGPoint(x: sideMargin, y: hasVerticalPins ? ChipLayout.stubLength : 0)
    }

    /// Room on each side of the body, inside the view's frame:
    /// `ChipLayout.labelMargin`, and more for a transistor or barrel jack on
    /// its side, whose part name stands beside the symbol.
    var sideMargin: CGFloat {
        let onItsSide = rotation == 90 || rotation == 270
        return onItsSide && (symbol == .transistor || symbol == .barrelJack) ? 90 : ChipLayout.labelMargin
    }

    /// The side of the view a side of the unturned symbol faces.
    func turnedSide(_ side: ChipLayout.Side) -> ChipLayout.Side {
        var side = isFlipped && (side == .left || side == .right) ? side.opposite : side
        for _ in 0..<(rotation / 90) {
            switch side {
            case .left: side = .top
            case .top: side = .right
            case .right: side = .bottom
            case .bottom: side = .left
            }
        }
        return side
    }

    /// A rectangle of the unturned symbol, in the view's frame.
    func symbolRect(_ rect: CGRect) -> CGRect {
        let a = symbolPoint(rect.minX, rect.minY), b = symbolPoint(rect.maxX, rect.maxY)
        return CGRect(x: min(a.x, b.x), y: min(a.y, b.y), width: abs(a.x - b.x), height: abs(a.y - b.y))
    }

    /// A point of the unturned symbol, in the view's frame.
    func symbolPoint(_ x: CGFloat, _ y: CGFloat) -> CGPoint {
        var point = CGPoint(x: isFlipped ? unturnedSize.width - x : x, y: y)
        // Turned in the same box as the pins (`ChipLayout.rotated`).
        let size = ChipLayout.turnBox(unturnedSize)
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

    /// Where a text at (`x`, `y`) of the unturned symbol goes when it reads
    /// away from the symbol toward its `side` (a transistor's name, right of
    /// the symbol): its near edge on the point. Text is never turned, so
    /// toward the top or bottom it is centred there.
    func symbolText(_ x: CGFloat, _ y: CGFloat, toward side: ChipLayout.Side) -> (point: CGPoint, anchor: UnitPoint) {
        let anchor: UnitPoint
        switch turnedSide(side) {
        case .left: anchor = .trailing
        case .right: anchor = .leading
        case .top: anchor = .bottom
        case .bottom: anchor = .top
        }
        return (symbolPoint(x, y), anchor)
    }

    /// The view's frame: the body, its label margin each side, and the stub
    /// room above and below when the pins are on the top and bottom.
    var symbolFrameSize: CGSize {
        let size = layout.size
        return CGSize(width: size.width + 2 * sideMargin,
                      height: size.height + (hasVerticalPins ? 2 * ChipLayout.stubLength : 0))
    }
}
