import SwiftUI

/// What a diode symbol's cathode end shows: an LED (plain bar, two light
/// arrows, a glow in play mode), or a diode part's bar (plan-29): plain,
/// Schottky (both ends hooked back, the S) or Zener (the ends bent opposite
/// ways, the Z). Diode parts have no arrows and no glow.
nonisolated enum DiodeMark: Hashable {
    case led
    case plain
    case schottky
    case zener

    /// The mark of a placed part: a diode part's by its type, else `.led`.
    init(part: CatalogPart) {
        guard case let .diode(type, _, _, _, _, _) = part.details else {
            self = .led
            return
        }
        switch type {
        case "schottky": self = .schottky
        case "zener": self = .zener
        default: self = .plain
        }
    }
}

/// Draws one placed LED with the standard diode symbol: a filled triangle
/// pointing from the anode (left) to the cathode (right), a bar across the
/// cathode, and two arrows for the emitted light. The function label sits below.
///
/// Like `ChipBlockView`, the frame includes `ChipLayout.labelMargin` on each
/// side, and the body's top-left corner sits at `chip.origin`.
struct DiodeSymbolView: View {
    let chip: PlacedChip
    let isSelected: Bool
    let isHovered: Bool
    /// Play mode: brightness 0…1, the share of the last frame the LED was on
    /// (anode high, cathode low). A lit LED glows in its own color.
    var level: Double = 0
    /// `.led` for an LED; a diode part's mark otherwise.
    var mark: DiodeMark = .led

    /// The glow color of a color name (an LED's own color, or a 7-segment
    /// display's); `.secondary` for an unknown one.
    nonisolated static func lightColor(named family: String) -> Color {
        switch family {
        case "Red": return .red
        case "Green": return .green
        case "Blue": return .blue
        case "Amber": return Color(red: 1, green: 0.7, blue: 0)
        case "Yellow": return .yellow
        case "Orange": return .orange
        case "White": return Color(white: 0.7)
        case "Warm white": return Color(red: 1, green: 0.82, blue: 0.5)
        case "Blue-green": return Color(red: 0.25, green: 0.9, blue: 0.8)
        default: return .secondary
        }
    }

    /// The LED's color name: the placed LED's own, else the part's first.
    static func lightName(for chip: PlacedChip) -> String {
        chip.color ?? chip.part.colors.first ?? ""
    }

    var body: some View {
        let frame = chip.symbolFrameSize

        Canvas { context, _ in
            var painter = ScreenPainter(context)
            Self.paint(&painter, chip: chip, isSelected: isSelected, isHovered: isHovered, level: level,
                       mark: mark)
        }
        .frame(width: frame.width, height: frame.height)
    }

    /// Draws the LED, its frame's top-left corner at (0, 0).
    static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, isSelected: Bool, isHovered: Bool,
                                           level: Double, mark: DiodeMark = .led) {
        let level = mark == .led ? level : 0
        let stub = ChipLayout.stubLength
        let highlighted = isSelected || isHovered
        let outline: Ink = highlighted ? .accent : .primary
        let light = Ink.lamp(lightName(for: chip))
        let lineWidth: CGFloat = highlighted ? 2 : 1.5
        let mid = ChipLayout.diodeCentreY

        let width = chip.unturnedSize.width
        let height = chip.unturnedSize.height

        // Body coordinates, mirrored left to right when flipped and then
        // turned, then shifted into the frame. Mapping every point keeps the
        // arrowheads and leads consistent.
        func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint { chip.symbolPoint(x, y) }

        // Leads: from each pin tip into the symbol.
        var leads = Path()
        leads.move(to: point(-stub, mid))
        leads.addLine(to: point(12, mid))
        leads.move(to: point(26, mid))
        leads.addLine(to: point(width + stub, mid))
        painter.stroke(leads, with: outline, lineWidth: lineWidth)

        // Diode body: anode at the base, pointing right toward the cathode.
        if level > 0 {
            let glow = CGRect(x: point(19, mid).x - 16, y: point(19, mid).y - 16, width: 32, height: 32)
            painter.fill(Path(ellipseIn: glow), with: light.opacity(0.3 * level))
        }
        var triangle = Path()
        triangle.move(to: point(12, mid - 6))
        triangle.addLine(to: point(12, mid + 6))
        triangle.addLine(to: point(26, mid))
        triangle.closeSubpath()
        painter.fill(triangle, with: outline)
        if level > 0 {
            painter.fill(triangle, with: light.opacity(level))
        }

        var cathodeBar = Path()
        switch mark {
        case .led, .plain:
            cathodeBar.move(to: point(26, mid - 6))
            cathodeBar.addLine(to: point(26, mid + 6))
        case .schottky:
            // The S: each end hooked back 3 pt, top end toward the anode,
            // bottom end away from it.
            cathodeBar.move(to: point(23, mid - 3))
            cathodeBar.addLine(to: point(23, mid - 6))
            cathodeBar.addLine(to: point(26, mid - 6))
            cathodeBar.addLine(to: point(26, mid + 6))
            cathodeBar.addLine(to: point(29, mid + 6))
            cathodeBar.addLine(to: point(29, mid + 3))
        case .zener:
            // The Z: the top end bent toward the anode, the bottom end away.
            cathodeBar.move(to: point(23, mid - 8))
            cathodeBar.addLine(to: point(26, mid - 6))
            cathodeBar.addLine(to: point(26, mid + 6))
            cathodeBar.addLine(to: point(29, mid + 8))
        }
        painter.stroke(cathodeBar, with: outline, lineWidth: lineWidth)

        guard mark == .led else {
            let label = chip.symbolLabel(y: height - 8)
            painter.draw(
                PaintedText(chip.part.function, size: 10, weight: .medium, ink: .secondary),
                at: label.point,
                anchor: label.anchor
            )
            return
        }

        // Light emitted up and to the right, with open arrowheads.
        var arrows = Path()
        addArrow(to: &arrows, from: (22, mid - 8), to: (30, mid - 16), through: point)
        addArrow(to: &arrows, from: (28, mid - 4), to: (36, mid - 12), through: point)
        painter.stroke(arrows, with: outline, lineWidth: 1.25)
        if level > 0 {
            painter.stroke(arrows, with: light.opacity(level), lineWidth: 1.25)
        }

        let label = chip.symbolLabel(y: height - 8)
        painter.draw(
            PaintedText(chip.part.function, size: 10, weight: .medium, ink: .secondary),
            at: label.point,
            anchor: label.anchor
        )
    }

    /// A line with an open arrowhead at `end`, in the unturned symbol's
    /// coordinates, where it runs up and to the right: the arms run back from
    /// `end` to its left and below it. `point` carries each point to the
    /// frame, so a flipped or turned arrow keeps its head on its tip.
    private static func addArrow(to path: inout Path, from start: (CGFloat, CGFloat), to end: (CGFloat, CGFloat),
                                 through point: (CGFloat, CGFloat) -> CGPoint) {
        path.move(to: point(start.0, start.1))
        path.addLine(to: point(end.0, end.1))
        path.move(to: point(end.0, end.1))
        path.addLine(to: point(end.0 - 4, end.1))
        path.move(to: point(end.0, end.1))
        path.addLine(to: point(end.0, end.1 + 4))
    }
}
