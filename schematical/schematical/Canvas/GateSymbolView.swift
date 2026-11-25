import SwiftUI

/// Draws one gate of a part shown in Gate view (plan-67) with its standard
/// logic symbol: the AND, OR and XOR bodies and the buffer triangle, a bubble
/// on an inverted output, a second output where the gate has one (CD4068B:
/// AND and NAND), a wide input edge when the inputs outnumber the body, the
/// hysteresis mark for Schmitt-trigger inputs and the diamond for open-drain
/// outputs. Each pin's number sits by its lead. No pin names and no part
/// number: those are on the part's `P` block.
///
/// The chip is a piece (`PlacedChip.piece`). Like `TransistorSymbolView`, the
/// frame is `PlacedChip.symbolFrameSize`, the body's top-left corner sits at
/// `chip.origin`, and every point goes through `symbolPoint`, so the flip and
/// the turn move the whole drawing.
struct GateSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool
    /// Another unit of the same selected part is picked: drawn faintly marked.
    var isSibling = false

    var body: some View {
        let frame = chip.symbolFrameSize

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered, isSibling: isSibling)
        }
        .frame(width: frame.width, height: frame.height)
    }

    /// The gate's drawing, in the unturned body's coordinates (top-left 0, 0).
    struct Geometry {
        let shape: GateShape
        /// The layout's height: 20 per input, at least 40.
        let height: CGFloat
        static let width = ChipLayout.gateWidth

        init(shape: GateShape) {
            self.shape = shape
            height = ChipLayout.gateHeight(inputs: shape.inputs.count)
        }

        var mid: CGFloat { height / 2 }
        /// The body is at most 60 tall; more inputs than fit meet a wider edge.
        var bodyHeight: CGFloat { min(height, 60) }
        var top: CGFloat { mid - bodyHeight / 2 }
        var bottom: CGFloat { mid + bodyHeight / 2 }
        var bodyBox: CGRect { CGRect(x: 0, y: top, width: Self.width, height: bodyHeight) }
        /// How far right an XOR's body starts, behind its extra curve.
        var bodyInset: CGFloat { shape.body == .xor ? 6 : 0 }

        /// Where an input lead at `y` meets the symbol: the straight back, the
        /// wide edge, or the curved back of an OR (an XOR's outer curve).
        func backX(_ y: CGFloat) -> CGFloat {
            guard shape.body == .or || shape.body == .xor, y > top, y < bottom else { return 0 }
            let t = (y - top) / bodyHeight
            return 30 * t * (1 - t)
        }

        /// Where the body's front edge is at `y`: its tip at the middle.
        func frontX(_ y: CGFloat) -> CGFloat {
            let half = bodyHeight / 2
            let off = min(abs(y - mid), half)
            switch shape.body {
            case .and:
                return Self.width - half + (half * half - off * off).squareRoot()
            case .buffer:
                return Self.width * (1 - off / half)
            case .or, .xor:
                // The top edge: from (inset, top) by (40, top) to (width, mid).
                let t = ((half - off) / half).squareRoot()
                return (1 - t) * (1 - t) * bodyInset + 2 * t * (1 - t) * 40 + t * t * Self.width
            }
        }

        func inputY(_ index: Int) -> CGFloat {
            ChipLayout.gateInputY(index, of: shape.inputs.count, height: height)
        }

        func outputY(_ index: Int) -> CGFloat {
            ChipLayout.gateOutputY(index, of: shape.outputs.count, height: height)
        }
    }

    /// The geometry of the gate a piece stands for; nil for any other chip.
    static func geometry(of chip: PlacedChip) -> Geometry? {
        guard case let .gate(shape)? = chip.unit?.shape else { return nil }
        return Geometry(shape: shape)
    }

    /// Where the reference goes, in the view's frame, and which point of it
    /// sits there: above the body, or right of it when the gate is on its
    /// side and its leads run up and down.
    static func referenceLabel(for chip: PlacedChip) -> (point: CGPoint, anchor: UnitPoint) {
        let box = geometry(of: chip)?.bodyBox ?? CGRect(origin: .zero, size: chip.unturnedSize)
        let rect = chip.symbolRect(box)
        if chip.rotation == 90 || chip.rotation == 270 {
            return (CGPoint(x: rect.maxX + 6, y: rect.midY + ReferenceLabel.height / 2), .bottomLeading)
        }
        return (CGPoint(x: rect.midX, y: rect.minY - 3), .bottom)
    }

    /// Draws the gate, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool,
                                           isSibling: Bool = false) {
        guard let g = geometry(of: chip) else { return }
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : isSibling ? Ink.accent.opacity(0.5) : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let stub = ChipLayout.stubLength
        let width = Geometry.width
        let mid = g.mid, top = g.top, bottom = g.bottom, half = g.bodyHeight / 2

        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint { chip.symbolPoint(x, y) }

        // The body.
        var body = Path()
        switch g.shape.body {
        case .and:
            let k = 0.5523 * half, flat = width - half
            body.move(to: point(0, top))
            body.addLine(to: point(flat, top))
            body.addCurve(to: point(width, mid), control1: point(flat + k, top), control2: point(width, mid - k))
            body.addCurve(to: point(flat, bottom), control1: point(width, mid + k), control2: point(flat + k, bottom))
            body.addLine(to: point(0, bottom))
            body.closeSubpath()
        case .or, .xor:
            let inset = g.bodyInset
            body.move(to: point(inset, top))
            body.addQuadCurve(to: point(width, mid), control: point(40, top))
            body.addQuadCurve(to: point(inset, bottom), control: point(40, bottom))
            body.addQuadCurve(to: point(inset, top), control: point(inset + 15, mid))
            body.closeSubpath()
        case .buffer:
            body.move(to: point(0, top))
            body.addLine(to: point(width, mid))
            body.addLine(to: point(0, bottom))
            body.closeSubpath()
        }
        painter.fill(body, with: .paper)
        if isHovered { painter.fill(body, with: Ink.accent.opacity(0.12)) }
        painter.stroke(body, with: outline, lineWidth: lineWidth)

        var strokes = Path()
        if g.shape.body == .xor {
            // The second curve behind the body.
            strokes.move(to: point(0, top))
            strokes.addQuadCurve(to: point(0, bottom), control: point(15, mid))
        }
        // More inputs than the body is tall: the wide input edge.
        if g.height > g.bodyHeight {
            if g.shape.body == .and || g.shape.body == .buffer {
                strokes.move(to: point(0, 0))
                strokes.addLine(to: point(0, g.height))
            } else {
                strokes.move(to: point(0, 0))
                strokes.addLine(to: point(0, top))
                strokes.move(to: point(0, bottom))
                strokes.addLine(to: point(0, g.height))
            }
        }

        // Input leads, from each pin's tip to the symbol.
        for index in g.shape.inputs.indices {
            let y = g.inputY(index)
            strokes.move(to: point(-stub, y))
            strokes.addLine(to: point(g.backX(y), y))
        }

        // Output leads, with a bubble on an inverted one.
        var bubbles = Path()
        for (index, output) in g.shape.outputs.enumerated() {
            let y = g.outputY(index)
            let front = g.frontX(y)
            if output.inverted {
                let centre = point(front + 4, y)
                bubbles.addEllipse(in: CGRect(x: centre.x - 4, y: centre.y - 4, width: 8, height: 8))
                if front + 8 < width + stub {
                    strokes.move(to: point(front + 8, y))
                    strokes.addLine(to: point(width + stub, y))
                }
            } else {
                strokes.move(to: point(front, y))
                strokes.addLine(to: point(width + stub, y))
            }
        }
        painter.stroke(strokes, with: outline, lineWidth: 1.5)
        painter.fill(bubbles, with: .paper)
        painter.stroke(bubbles, with: outline, lineWidth: 1.5)

        // Marks inside the body.
        var marks = Path()
        if g.shape.schmitt {
            marks.move(to: point(14, mid + 4))
            marks.addLine(to: point(20, mid + 4))
            marks.addLine(to: point(24, mid - 4))
            marks.addLine(to: point(26, mid - 4))
            marks.move(to: point(14, mid + 4))
            marks.addLine(to: point(16, mid + 4))
            marks.addLine(to: point(20, mid - 4))
            marks.addLine(to: point(26, mid - 4))
        }
        if g.shape.openDrain {
            let x: CGFloat = g.shape.body == .buffer ? 36 : width - 16
            marks.move(to: point(x - 3, mid))
            marks.addLine(to: point(x, mid - 3))
            marks.addLine(to: point(x + 3, mid))
            marks.addLine(to: point(x, mid + 3))
            marks.closeSubpath()
            marks.move(to: point(x - 3, mid + 5))
            marks.addLine(to: point(x + 3, mid + 5))
        }
        painter.stroke(marks, with: outline, lineWidth: 1)

        // Pin numbers, beside each lead where it leaves the symbol.
        func number(_ pin: Int, x: CGFloat, y: CGFloat, side: ChipLayout.Side) {
            guard let designator = chip.part.pins.first(where: { $0.index == pin })?.designator else { return }
            let spot = point(x, y)
            let place: (CGPoint, UnitPoint)
            switch chip.turnedSide(side) {
            case .left: place = (CGPoint(x: spot.x - 2, y: spot.y - 1), .bottomTrailing)
            case .right: place = (CGPoint(x: spot.x + 2, y: spot.y - 1), .bottomLeading)
            case .top: place = (CGPoint(x: spot.x + 2, y: spot.y - 2), .bottomLeading)
            case .bottom: place = (CGPoint(x: spot.x + 2, y: spot.y + 2), .topLeading)
            }
            painter.draw(PaintedText(designator, size: 8, ink: .secondary), at: place.0, anchor: place.1)
        }
        for (index, pin) in g.shape.inputs.enumerated() {
            let y = g.inputY(index)
            number(pin, x: 0, y: y, side: .left)
        }
        for (index, output) in g.shape.outputs.enumerated() {
            let y = g.outputY(index)
            number(output.pin, x: max(width, g.frontX(y) + (output.inverted ? 8 : 0)), y: y, side: .right)
        }
    }
}
