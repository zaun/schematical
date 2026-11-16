import SwiftUI

/// A small drawing of a footprint (plan-46-J): its pads to scale, the body
/// outline, drill holes, and pin 1 in the accent color. Top view, as the
/// PCB draws it.
struct FootprintThumbnail: View {
    let footprint: Footprint

    var body: some View {
        Canvas { context, size in
            Self.draw(footprint, in: &context, size: size)
        }
        .accessibilityLabel("Drawing of \(footprint.id)")
    }

    /// Everything the drawing covers, in millimetres.
    static func bounds(of footprint: Footprint) -> CGRect {
        var rect = CGRect(x: -footprint.body.width / 2, y: -footprint.body.height / 2,
                          width: footprint.body.width, height: footprint.body.height)
        for pad in footprint.pads {
            rect = rect.union(CGRect(x: pad.centre.x - pad.size.width / 2, y: pad.centre.y - pad.size.height / 2,
                                     width: pad.size.width, height: pad.size.height))
        }
        return rect
    }

    /// The pad that marks pin 1: the pad of pin "1", else the first pad.
    static func pin1Index(of footprint: Footprint) -> Int? {
        footprint.pads.firstIndex { $0.designator == "1" } ?? (footprint.pads.isEmpty ? nil : 0)
    }

    static func draw(_ footprint: Footprint, in context: inout GraphicsContext, size: CGSize) {
        let bounds = bounds(of: footprint)
        guard bounds.width > 0, bounds.height > 0, size.width > 0, size.height > 0 else { return }
        let scale = min(size.width / (bounds.width * 1.15), size.height / (bounds.height * 1.15))
        context.translateBy(x: size.width / 2, y: size.height / 2)
        context.scaleBy(x: scale, y: scale)
        context.translateBy(x: -bounds.midX, y: -bounds.midY)

        let bodyRect = CGRect(x: -footprint.body.width / 2, y: -footprint.body.height / 2,
                              width: footprint.body.width, height: footprint.body.height)
        let outline = footprint.bodyRound ? Path(ellipseIn: bodyRect) : Path(bodyRect)
        context.stroke(outline, with: .color(.secondary), lineWidth: 1 / scale)

        let first = pin1Index(of: footprint)
        for (index, pad) in footprint.pads.enumerated() {
            let rect = CGRect(x: pad.centre.x - pad.size.width / 2, y: pad.centre.y - pad.size.height / 2,
                              width: pad.size.width, height: pad.size.height)
            let shape = pad.shape == .round ? Path(ellipseIn: rect)
                : Path(roundedRect: rect, cornerRadius: pad.radius)
            if pad.plated {
                context.fill(shape, with: .color(index == first ? Color.accentColor : PCBCanvasView.copper))
            } else {
                context.stroke(shape, with: .color(.secondary), lineWidth: 1 / scale)
            }
            if pad.drill > 0, pad.plated {
                let hole = CGRect(x: pad.centre.x - pad.drill / 2, y: pad.centre.y - pad.drill / 2,
                                  width: pad.drill, height: pad.drill)
                context.fill(Path(ellipseIn: hole), with: .color(PCBCanvasView.background))
            }
        }
    }
}

/// A labeled sketch of one parametric footprint style (plan-46-J): what each
/// number of the "Make one" form measures on the datasheet's drawing.
struct FootprintDiagram: View {
    let style: ParametricFootprintDraft.Style

    var body: some View {
        Canvas { context, size in
            draw(in: &context, size: size)
        }
        .frame(height: 170)
        .accessibilityLabel("Sketch of a \(style.title.lowercased()) footprint with its dimensions")
    }

    /// The dimensions the sketch names, for the form's labels and the tests.
    static func dimensions(_ style: ParametricFootprintDraft.Style) -> [String] {
        switch style {
        case .dual: ["Pitch", "Row spacing", "Pad length", "Pad width"]
        case .quad: ["Pitch", "Spacing across", "Spacing down", "Pad length", "Pad width"]
        case .grid: ["Pitch", "Ball pad"]
        case .header: ["Pitch", "Row pitch", "Pad", "Drill"]
        }
    }

    private func draw(in context: inout GraphicsContext, size: CGSize) {
        let copper = PCBCanvasView.copper
        let ink = Color.primary
        func pad(_ rect: CGRect, round: Bool = false, first: Bool = false) {
            let path = round ? Path(ellipseIn: rect) : Path(rect)
            context.fill(path, with: .color(first ? Color.accentColor : copper))
        }
        func label(_ text: String, at point: CGPoint, anchor: UnitPoint = .center) {
            context.draw(Text(text).font(.caption2).foregroundColor(ink), at: point, anchor: anchor)
        }
        /// A dimension line with end ticks and its name beside it.
        func dimension(_ a: CGPoint, _ b: CGPoint, _ text: String, offset: CGSize = .zero, anchor: UnitPoint = .center) {
            var path = Path()
            path.move(to: a)
            path.addLine(to: b)
            let vertical = abs(a.x - b.x) < 0.5
            for end in [a, b] {
                path.move(to: CGPoint(x: end.x - (vertical ? 3 : 0), y: end.y - (vertical ? 0 : 3)))
                path.addLine(to: CGPoint(x: end.x + (vertical ? 3 : 0), y: end.y + (vertical ? 0 : 3)))
            }
            context.stroke(path, with: .color(ink.opacity(0.7)), lineWidth: 1)
            label(text, at: CGPoint(x: (a.x + b.x) / 2 + offset.width, y: (a.y + b.y) / 2 + offset.height),
                  anchor: anchor)
        }
        let cx = size.width / 2, cy = size.height / 2 + 4
        switch style {
        case .dual:
            let span: CGFloat = 110, pitch: CGFloat = 26, padW: CGFloat = 34, padH: CGFloat = 12
            let body = CGRect(x: cx - 36, y: cy - 58, width: 72, height: 116)
            context.stroke(Path(body), with: .color(.secondary), lineWidth: 1)
            for row in 0 ..< 4 {
                let y = cy + (CGFloat(row) - 1.5) * pitch
                pad(CGRect(x: cx - span / 2 - padW / 2, y: y - padH / 2, width: padW, height: padH), first: row == 0)
                pad(CGRect(x: cx + span / 2 - padW / 2, y: y - padH / 2, width: padW, height: padH))
            }
            label("1", at: CGPoint(x: cx - span / 2 - padW / 2 - 8, y: cy - 1.5 * pitch))
            dimension(CGPoint(x: cx - span / 2, y: cy + 66), CGPoint(x: cx + span / 2, y: cy + 66), "Row spacing",
                      offset: CGSize(width: 0, height: 9))
            dimension(CGPoint(x: cx + span / 2 + padW / 2 + 10, y: cy - 1.5 * pitch),
                      CGPoint(x: cx + span / 2 + padW / 2 + 10, y: cy - 0.5 * pitch), "Pitch",
                      offset: CGSize(width: 6, height: 0), anchor: .leading)
            dimension(CGPoint(x: cx + span / 2 - padW / 2, y: cy + 1.5 * pitch + 14),
                      CGPoint(x: cx + span / 2 + padW / 2, y: cy + 1.5 * pitch + 14), "Pad length",
                      offset: CGSize(width: 44, height: 0), anchor: .leading)
            dimension(CGPoint(x: cx - span / 2 - padW / 2 - 10, y: cy + 1.5 * pitch - padH / 2),
                      CGPoint(x: cx - span / 2 - padW / 2 - 10, y: cy + 1.5 * pitch + padH / 2), "Pad width",
                      offset: CGSize(width: -6, height: 0), anchor: .trailing)
        case .quad:
            let span: CGFloat = 104, pitch: CGFloat = 20, padL: CGFloat = 20, padW: CGFloat = 9
            context.stroke(Path(CGRect(x: cx - 46, y: cy - 46, width: 92, height: 92)), with: .color(.secondary),
                           lineWidth: 1)
            for i in 0 ..< 4 {
                let d = (CGFloat(i) - 1.5) * pitch
                pad(CGRect(x: cx - span / 2 - padL / 2, y: cy + d - padW / 2, width: padL, height: padW), first: i == 0)
                pad(CGRect(x: cx + span / 2 - padL / 2, y: cy + d - padW / 2, width: padL, height: padW))
                pad(CGRect(x: cx + d - padW / 2, y: cy - span / 2 - padL / 2, width: padW, height: padL))
                pad(CGRect(x: cx + d - padW / 2, y: cy + span / 2 - padL / 2, width: padW, height: padL))
            }
            label("1", at: CGPoint(x: cx - span / 2 - padL / 2 - 8, y: cy - 1.5 * pitch))
            dimension(CGPoint(x: cx - span / 2, y: cy + span / 2 + 20), CGPoint(x: cx + span / 2, y: cy + span / 2 + 20),
                      "Spacing across", offset: CGSize(width: 0, height: 9))
            dimension(CGPoint(x: cx + span / 2 + 22, y: cy - span / 2), CGPoint(x: cx + span / 2 + 22, y: cy + span / 2),
                      "Spacing down", offset: CGSize(width: 6, height: 0), anchor: .leading)
            dimension(CGPoint(x: cx - span / 2 - 22, y: cy - 1.5 * pitch),
                      CGPoint(x: cx - span / 2 - 22, y: cy - 0.5 * pitch), "Pitch",
                      offset: CGSize(width: -6, height: 0), anchor: .trailing)
        case .grid:
            let pitch: CGFloat = 30, ball: CGFloat = 14
            context.stroke(Path(CGRect(x: cx - 66, y: cy - 52, width: 132, height: 104)), with: .color(.secondary),
                           lineWidth: 1)
            for row in 0 ..< 3 {
                for column in 0 ..< 4 {
                    let x = cx + (CGFloat(column) - 1.5) * pitch, y = cy + (CGFloat(row) - 1) * pitch
                    pad(CGRect(x: x - ball / 2, y: y - ball / 2, width: ball, height: ball), round: true,
                        first: row == 0 && column == 0)
                }
            }
            label("A1", at: CGPoint(x: cx - 1.5 * pitch, y: cy - pitch - 16))
            label("A", at: CGPoint(x: cx - 82, y: cy - pitch))
            label("B", at: CGPoint(x: cx - 82, y: cy))
            label("C", at: CGPoint(x: cx - 82, y: cy + pitch))
            dimension(CGPoint(x: cx - 0.5 * pitch, y: cy + pitch + 18), CGPoint(x: cx + 0.5 * pitch, y: cy + pitch + 18),
                      "Pitch", offset: CGSize(width: 0, height: 9))
            dimension(CGPoint(x: cx + 1.5 * pitch - ball / 2, y: cy - pitch - 14),
                      CGPoint(x: cx + 1.5 * pitch + ball / 2, y: cy - pitch - 14), "Ball pad",
                      offset: CGSize(width: 34, height: 0), anchor: .leading)
        case .header:
            let pitch: CGFloat = 34, rowPitch: CGFloat = 34, padD: CGFloat = 20, hole: CGFloat = 9
            context.stroke(Path(CGRect(x: cx - 76, y: cy - 38, width: 152, height: 76)), with: .color(.secondary),
                           lineWidth: 1)
            var number = 1
            for column in 0 ..< 4 {
                for row in 0 ..< 2 {
                    let x = cx + (CGFloat(column) - 1.5) * pitch, y = cy + (CGFloat(row) - 0.5) * rowPitch
                    pad(CGRect(x: x - padD / 2, y: y - padD / 2, width: padD, height: padD), round: number != 1,
                        first: number == 1)
                    context.fill(Path(ellipseIn: CGRect(x: x - hole / 2, y: y - hole / 2, width: hole, height: hole)),
                                 with: .color(PCBCanvasView.background))
                    number += 1
                }
            }
            label("1", at: CGPoint(x: cx - 1.5 * pitch - 18, y: cy - 0.5 * rowPitch))
            label("2", at: CGPoint(x: cx - 1.5 * pitch - 18, y: cy + 0.5 * rowPitch))
            dimension(CGPoint(x: cx - 0.5 * pitch, y: cy + 52), CGPoint(x: cx + 0.5 * pitch, y: cy + 52), "Pitch",
                      offset: CGSize(width: 0, height: 9))
            dimension(CGPoint(x: cx + 1.5 * pitch + 24, y: cy - 0.5 * rowPitch),
                      CGPoint(x: cx + 1.5 * pitch + 24, y: cy + 0.5 * rowPitch), "Row pitch",
                      offset: CGSize(width: 6, height: 0), anchor: .leading)
        }
    }
}
