import SwiftUI

/// What a drawn thing is, not its color. A painter turns it into a color:
/// the screen palette gives the app's colors, the export palette black at
/// some alpha (plan-28-A § 3.1, `docs/export.md`).
nonisolated struct Ink: Hashable {
    enum Role: Hashable {
        /// Semantic colors.
        case primary, secondary, accent, paper
        /// Pin-name colors (red, blue).
        case power, ground
        /// DIP lever ON: track and knob.
        case switchOn, knobOn
        /// 7-segment / LCD face: an unlit segment, the face fill, the face outline.
        case unlit, displayFace, faceEdge
        /// LED / segment light by color name ("Red", "White" …).
        case lamp(String)
        /// LCD face colors, r g b in 0…1.
        case display(Double, Double, Double)
        /// Screen only: simulation wire colors, tool previews.
        case color(Color)
    }

    var role: Role
    var opacity = 1.0

    init(role: Role, opacity: Double = 1) {
        self.role = role
        self.opacity = opacity
    }

    /// Multiplies, as `Color.opacity` does.
    func opacity(_ value: Double) -> Ink {
        Ink(role: role, opacity: opacity * value)
    }

    static let primary = Ink(role: .primary)
    static let secondary = Ink(role: .secondary)
    static let accent = Ink(role: .accent)
    static let paper = Ink(role: .paper)
    static let power = Ink(role: .power)
    static let ground = Ink(role: .ground)
    static let switchOn = Ink(role: .switchOn)
    static let knobOn = Ink(role: .knobOn)
    static let unlit = Ink(role: .unlit)
    static let displayFace = Ink(role: .displayFace)
    static let faceEdge = Ink(role: .faceEdge)
    static func lamp(_ name: String) -> Ink { Ink(role: .lamp(name)) }
    static func display(_ r: Double, _ g: Double, _ b: Double) -> Ink { Ink(role: .display(r, g, b)) }
    static func color(_ color: Color) -> Ink { Ink(role: .color(color)) }
}

/// A label: string, font and ink.
nonisolated struct PaintedText {
    var string: String
    var size: CGFloat
    var weight: Font.Weight = .regular
    var monospaced = false
    var ink: Ink = .primary

    init(_ string: String, size: CGFloat, weight: Font.Weight = .regular, monospaced: Bool = false,
         ink: Ink = .primary) {
        self.string = string
        self.size = size
        self.weight = weight
        self.monospaced = monospaced
        self.ink = ink
    }
}

/// The one interface symbol and layer drawing goes through, so the same
/// code draws the live canvas and writes an export. Painters are values;
/// transform a copy to draw part of a picture turned or shifted.
protocol SchematicPainter {
    mutating func fill(_ path: Path, with ink: Ink)
    mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle)
    mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint)
    mutating func translateBy(x: CGFloat, y: CGFloat)
    mutating func rotate(by angle: Angle)
}

extension SchematicPainter {
    mutating func stroke(_ path: Path, with ink: Ink, lineWidth: CGFloat) {
        stroke(path, with: ink, style: StrokeStyle(lineWidth: lineWidth))
    }
}

/// What every drawn thing looks like in an export: black at an alpha, or
/// not drawn (nil). The final alpha is this times `ink.opacity`.
nonisolated enum ExportPalette {
    static func alpha(for role: Ink.Role) -> Double? {
        switch role {
        case .primary, .accent, .power, .ground, .lamp, .knobOn, .faceEdge: 1
        case .secondary: 0.6
        case .unlit: 0.15
        case .switchOn: 0.25
        case .paper, .displayFace, .display, .color: nil
        }
    }

    /// The ink's final alpha: the role's export alpha times its opacity.
    static func alpha(for ink: Ink) -> Double? {
        alpha(for: ink.role).map { $0 * ink.opacity }
    }
}

/// How `ScreenPainter` resolves ink: the app's colors, or black line art.
nonisolated enum ScreenPalette {
    case screen
    case export

    /// The color an ink draws in; nil when it is not drawn.
    func color(for ink: Ink) -> Color? {
        switch self {
        case .screen:
            guard let base = Self.screenColor(for: ink.role) else { return nil }
            return ink.opacity == 1 ? base : base.opacity(ink.opacity)
        case .export:
            guard let alpha = ExportPalette.alpha(for: ink) else { return nil }
            return alpha == 1 ? Color.black : Color.black.opacity(alpha)
        }
    }

    static func screenColor(for role: Ink.Role) -> Color? {
        switch role {
        case .primary: .primary
        case .secondary: .secondary
        case .accent: .accentColor
        case .paper: Color(nsColor: .controlBackgroundColor)
        case .power: .red
        case .ground: .blue
        case .switchOn: Color.green.opacity(0.85)
        case .knobOn: .white
        case .unlit: Color.white.opacity(0.1)
        case .displayFace: Color(white: 0.12)
        case .faceEdge: nil
        case .lamp(let name): DiodeSymbolView.lightColor(named: name)
        case .display(let r, let g, let b): Color(red: r, green: g, blue: b)
        case .color(let color): color
        }
    }
}

/// Draws into a SwiftUI `GraphicsContext`: the live canvas (`.screen`) and
/// the raster export (`.export`).
nonisolated struct ScreenPainter: SchematicPainter {
    var context: GraphicsContext
    var palette: ScreenPalette

    init(_ context: GraphicsContext, palette: ScreenPalette = .screen) {
        self.context = context
        self.palette = palette
    }

    mutating func fill(_ path: Path, with ink: Ink) {
        guard let color = palette.color(for: ink) else { return }
        context.fill(path, with: .color(color))
    }

    mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle) {
        guard let color = palette.color(for: ink) else { return }
        context.stroke(path, with: .color(color), style: style)
    }

    mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint) {
        guard let resolved = self.text(text) else { return }
        context.draw(resolved, at: point, anchor: anchor)
    }

    mutating func translateBy(x: CGFloat, y: CGFloat) {
        context.translateBy(x: x, y: y)
    }

    mutating func rotate(by angle: Angle) {
        context.rotate(by: angle)
    }

    /// The SwiftUI text, styled as the views styled it before the painter:
    /// secondary is the hierarchical style, primary at full opacity is the
    /// default style, anything else its color.
    private func text(_ text: PaintedText) -> Text? {
        let base = Text(text.string)
            .font(.system(size: text.size, weight: text.weight, design: text.monospaced ? .monospaced : .default))
        if palette == .screen, text.ink.opacity == 1 {
            switch text.ink.role {
            case .primary: return base
            case .secondary: return base.foregroundStyle(.secondary)
            default: break
            }
        }
        guard let color = palette.color(for: text.ink) else { return nil }
        return base.foregroundStyle(color)
    }
}
