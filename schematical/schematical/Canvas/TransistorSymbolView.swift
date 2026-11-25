import SwiftUI

/// Draws one placed single transistor (plan-40-H) with the standard
/// schematic symbol. A BJT: a circle, the base lead to the base bar, the
/// collector and emitter leads, the arrow on the emitter lead pointing out
/// for NPN and in for PNP. A MOSFET: the gate plate, the broken channel
/// line, the drain and source leads, the body arrow on the source pointing
/// in for N-channel and out for P-channel. The polarity comes from the
/// part (`TransistorInfo`); Darlington and pre-biased parts draw as the
/// plain BJT. The part name sits to the right of the symbol, between the two
/// right-hand leads; on a part turned on its side it stands beside the
/// symbol (`nameLabel`). No glow and nothing in play mode. Transistor arrays
/// are drawn as blocks.
///
/// Like `ResistorSymbolView`, the frame is `PlacedChip.symbolFrameSize`, and
/// the body's top-left corner sits at `chip.origin`.
struct TransistorSymbolView: View {
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

    /// Where the part name goes, in the view's frame, and which point of it
    /// sits there. Flat (0 or 180), it starts between the collector and
    /// emitter leads and reads away from the symbol. On its side those leads
    /// run down or up and the gap between them is too narrow, so it stands
    /// right of the symbol, level with it.
    static func nameLabel(for chip: PlacedChip) -> (point: CGPoint, anchor: UnitPoint) {
        let mid = ChipLayout.transistorMidY
        guard chip.rotation == 90 || chip.rotation == 270 else { return chip.symbolText(44, mid, toward: .right) }
        let right = chip.symbolRect(CGRect(origin: .zero, size: chip.unturnedSize)).maxX
        return (CGPoint(x: right - 4, y: chip.symbolPoint(24, mid).y), .leading)
    }

    /// Draws the transistor, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let width = chip.unturnedSize.width
        let info = chip.part.transistor
        let mosfet = info?.mosfet ?? false
        let pType = info?.pType ?? false
        let top = ChipLayout.transistorTopY
        let mid = ChipLayout.transistorMidY
        let bottom = ChipLayout.transistorBottomY

        // The symbol's own coordinates, flipped and turned with the part.
        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint { chip.symbolPoint(x, y) }

        var strokes = Path()
        var arrow = Path()
        // The x where the collector / drain and emitter / source leads turn
        // to run right to their pins.
        let turn: CGFloat = 30
        if mosfet {
            let plate: CGFloat = 14, channel: CGFloat = 20
            // Gate lead and plate.
            strokes.move(to: point(-stub, mid))
            strokes.addLine(to: point(plate, mid))
            strokes.move(to: point(plate, mid - 12))
            strokes.addLine(to: point(plate, mid + 12))
            // The broken channel: drain, body and source segments.
            for y in [top + 9, mid, bottom - 9] {
                strokes.move(to: point(channel, y - 4))
                strokes.addLine(to: point(channel, y + 4))
            }
            // Drain: from the channel's top segment up to its pin.
            strokes.move(to: point(channel, top + 9))
            strokes.addLine(to: point(turn, top + 9))
            strokes.addLine(to: point(turn, top))
            strokes.addLine(to: point(width + stub, top))
            // Source: from the bottom segment down to its pin, with the body
            // (the middle segment) tied to it.
            strokes.move(to: point(channel, bottom - 9))
            strokes.addLine(to: point(turn, bottom - 9))
            strokes.addLine(to: point(turn, bottom))
            strokes.addLine(to: point(width + stub, bottom))
            strokes.move(to: point(channel, mid))
            strokes.addLine(to: point(turn, mid))
            strokes.addLine(to: point(turn, bottom - 9))
            // The body arrow: into the channel for N, out of it for P.
            if pType {
                addArrowhead(to: &arrow, tip: point(turn - 2, mid), from: point(channel, mid))
            } else {
                addArrowhead(to: &arrow, tip: point(channel + 1, mid), from: point(turn, mid))
            }
        } else {
            let bar: CGFloat = 16
            let centre = point(24, mid)
            painter.stroke(Path(ellipseIn: CGRect(x: centre.x - 16, y: centre.y - 16, width: 32, height: 32)),
                           with: outline, lineWidth: lineWidth)
            // Base lead and bar.
            strokes.move(to: point(-stub, mid))
            strokes.addLine(to: point(bar, mid))
            strokes.move(to: point(bar, mid - 9))
            strokes.addLine(to: point(bar, mid + 9))
            // Collector: a diagonal off the bar, then up and right to its pin.
            strokes.move(to: point(bar, mid - 5))
            strokes.addLine(to: point(turn, mid - 13))
            strokes.addLine(to: point(turn, top))
            strokes.addLine(to: point(width + stub, top))
            // Emitter: the same, down and right.
            let emitterStart = point(bar, mid + 5), emitterEnd = point(turn, mid + 13)
            strokes.move(to: emitterStart)
            strokes.addLine(to: emitterEnd)
            strokes.addLine(to: point(turn, bottom))
            strokes.addLine(to: point(width + stub, bottom))
            // The emitter arrow: out of the base for NPN, into it for PNP.
            if pType {
                addArrowhead(to: &arrow, tip: interpolate(emitterStart, emitterEnd, 0.25), from: emitterEnd)
            } else {
                addArrowhead(to: &arrow, tip: interpolate(emitterStart, emitterEnd, 0.95), from: emitterStart)
            }
        }
        painter.stroke(strokes, with: outline, lineWidth: lineWidth)
        painter.fill(arrow, with: outline)

        let name = nameLabel(for: chip)

        painter.draw(
            PaintedText(chip.part.name, size: 10, weight: .medium, ink: .secondary),
            at: name.point,
            anchor: name.anchor
        )
    }

    /// The point `t` of the way from `a` to `b`.
    private static func interpolate(_ a: CGPoint, _ b: CGPoint, _ t: CGFloat) -> CGPoint {
        CGPoint(x: a.x + (b.x - a.x) * t, y: a.y + (b.y - a.y) * t)
    }

    /// A filled arrowhead with its point at `tip`, pointing away from `from`.
    private static func addArrowhead(to path: inout Path, tip: CGPoint, from: CGPoint) {
        let dx = tip.x - from.x, dy = tip.y - from.y
        let length = max((dx * dx + dy * dy).squareRoot(), 0.001)
        let ux = dx / length, uy = dy / length
        let back: CGFloat = 6, half: CGFloat = 3
        let base = CGPoint(x: tip.x - ux * back, y: tip.y - uy * back)
        path.move(to: tip)
        path.addLine(to: CGPoint(x: base.x - uy * half, y: base.y + ux * half))
        path.addLine(to: CGPoint(x: base.x + uy * half, y: base.y - ux * half))
        path.closeSubpath()
    }
}
