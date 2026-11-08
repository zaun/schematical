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
        let size = chip.part.headerSize.map { "\($0.rows)×\($0.perRow)" } ?? ""
        let gender = (chip.gender ?? chip.part.genders.first ?? "").lowercased()
        return [size, gender].filter { !$0.isEmpty }.joined(separator: " ")
    }

    var body: some View {
        let layout = chip.layout
        let margin = ChipLayout.labelMargin
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Color = highlighted ? .accentColor : .primary
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let width = layout.size.width
        let height = layout.size.height
        let female = chip.gender == "Female"

        Canvas { context, _ in
            let body = CGRect(x: margin, y: 0, width: width, height: height)
            context.stroke(Path(body), with: .color(outline), lineWidth: lineWidth)

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
                context.stroke(lead, with: .color(outline), lineWidth: 1)
                if female {
                    // An open U, 8 wide, opening away from the body.
                    let base = tip - out * 4
                    var socket = Path()
                    socket.move(to: CGPoint(x: tip, y: y - 4))
                    socket.addLine(to: CGPoint(x: base, y: y - 4))
                    socket.addLine(to: CGPoint(x: base, y: y + 4))
                    socket.addLine(to: CGPoint(x: tip, y: y + 4))
                    context.stroke(socket, with: .color(outline), lineWidth: 1.5)
                } else {
                    // A filled 6×6 pin at the stub's end.
                    let x = tip - out * 3
                    context.fill(Path(CGRect(x: x - 3, y: y - 3, width: 6, height: 6)), with: .color(outline))
                }
                context.draw(Text(slot.pin.designator).font(.system(size: 9, design: .monospaced)),
                             at: CGPoint(x: left ? edge + 3 : edge - 3, y: y),
                             anchor: left ? .leading : .trailing)
            }

            context.draw(Text(Self.label(for: chip)).font(.system(size: 10, design: .monospaced))
                            .foregroundStyle(.secondary),
                         at: CGPoint(x: margin + width / 2, y: height + Self.labelHeight / 2),
                         anchor: .center)
        }
        .frame(width: width + 2 * margin, height: height + Self.labelHeight)
    }
}
