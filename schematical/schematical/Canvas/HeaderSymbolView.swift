import SwiftUI

/// A generic pin header (plan-23): the body, one stub per pin with a male
/// pin (a filled square) or a female socket (an open U facing out) at its
/// end, the pin numbers inside the body, and "1×8 male" under it.
///
/// Like `ChipBlockView`, the frame includes `ChipLayout.labelMargin` on each
/// side, and the body's top-left corner sits at `chip.origin`. The frame is
/// `labelHeight` taller than the body for the size label.
struct HeaderSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool

    static let labelHeight: CGFloat = 16

    /// "1×8 male": the size and gender under the body.
    static func label(for chip: PlacedChip) -> String {
        // Plain ifs: Xcode 26.3's optimizer crashes ("Found ownership error")
        // on the `??` chain this used to be, failing the Release build.
        var words: [String] = []
        if let size = chip.part.headerSize {
            words.append("\(size.rows)×\(size.perRow)")
        }
        if let gender = chip.gender {
            words.append(gender.lowercased())
        } else if let gender = chip.part.genders.first {
            words.append(gender.lowercased())
        }
        return words.filter { !$0.isEmpty }.joined(separator: " ")
    }

    var body: some View {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered)
        }
        .frame(width: layout.size.width + 2 * margin, height: layout.size.height + Self.labelHeight)
    }

    /// Draws the header, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool) {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let width = layout.size.width
        let height = layout.size.height
        let female = chip.gender == "Female"

        let body = CGRect(x: margin, y: 0, width: width, height: height)
        painter.stroke(Path(body), with: outline, lineWidth: lineWidth)

        for slot in layout.slots {
            let left = slot.side == .left
            let y = slot.centerY
            // The body edge, and the stub's outer end (the wire's tip).
            let edge = left ? margin : margin + width
            let tip = left ? margin - stub : margin + width + stub
            let out: CGFloat = left ? -1 : 1
            var lead = Path()
            lead.move(to: CGPoint(x: edge, y: y))
            lead.addLine(to: CGPoint(x: tip, y: y))
            painter.stroke(lead, with: outline, lineWidth: 1)
            if female {
                // An open U, 8 wide, opening away from the body.
                let base = tip - out * 4
                var socket = Path()
                socket.move(to: CGPoint(x: tip, y: y - 4))
                socket.addLine(to: CGPoint(x: base, y: y - 4))
                socket.addLine(to: CGPoint(x: base, y: y + 4))
                socket.addLine(to: CGPoint(x: tip, y: y + 4))
                painter.stroke(socket, with: outline, lineWidth: 1.5)
            } else {
                // A filled 6×6 pin at the stub's end.
                let x = tip - out * 3
                painter.fill(Path(CGRect(x: x - 3, y: y - 3, width: 6, height: 6)), with: outline)
            }
            painter.draw(PaintedText(slot.pin.designator, size: 9, monospaced: true),
                         at: CGPoint(x: left ? edge + 3 : edge - 3, y: y),
                         anchor: left ? .leading : .trailing)
        }

        painter.draw(PaintedText(label(for: chip), size: 10, monospaced: true, ink: .secondary),
                     at: CGPoint(x: margin + width / 2, y: height + labelHeight / 2),
                     anchor: .center)
    }
}
