import SwiftUI

/// Writes what it is asked to draw as SVG elements: black line art at the
/// export palette's alpha, every shape a `<path>` with absolute coordinates,
/// every label a live `<text>`. Ink the export drops writes nothing
/// (plan-28-C § 3.1).
nonisolated struct SVGPainter: SchematicPainter {
    /// The elements in paint order. Shared by copies, so a turned copy
    /// writes into the same document. Nonisolated: a main-actor class's
    /// isolated deinit aborts on macOS 15.8 (see `TestSupport.keptDocuments`).
    nonisolated final class Output {
        var elements: [String] = []
    }

    let output = Output()
    var transform = CGAffineTransform.identity

    init() {}

    mutating func fill(_ path: Path, with ink: Ink) {
        guard let alpha = ExportPalette.alpha(for: ink) else { return }
        var element = "<path d=\"\(Self.d(for: path.applying(transform)))\" fill=\"#000\""
        if alpha != 1 { element += " fill-opacity=\"\(Self.number(alpha))\"" }
        output.elements.append(element + "/>")
    }

    mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle) {
        guard let alpha = ExportPalette.alpha(for: ink) else { return }
        var element = "<path d=\"\(Self.d(for: path.applying(transform)))\" fill=\"none\" stroke=\"#000\""
        if alpha != 1 { element += " stroke-opacity=\"\(Self.number(alpha))\"" }
        element += " stroke-width=\"\(Self.number(style.lineWidth))\""
        switch style.lineCap {
        case .round: element += " stroke-linecap=\"round\""
        case .square: element += " stroke-linecap=\"square\""
        default: break
        }
        switch style.lineJoin {
        case .round: element += " stroke-linejoin=\"round\""
        case .bevel: element += " stroke-linejoin=\"bevel\""
        default: break
        }
        if style.miterLimit != 4 { element += " stroke-miterlimit=\"\(Self.number(style.miterLimit))\"" }
        if !style.dash.isEmpty {
            element += " stroke-dasharray=\"\(style.dash.map(Self.number).joined(separator: " "))\""
            if style.dashPhase != 0 { element += " stroke-dashoffset=\"\(Self.number(style.dashPhase))\"" }
        }
        output.elements.append(element + "/>")
    }

    mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint) {
        guard let alpha = ExportPalette.alpha(for: text.ink) else { return }
        // A shift alone moves the point; a turn needs the matrix.
        var position = point
        var element = "<text"
        if transform.a == 1, transform.b == 0, transform.c == 0, transform.d == 1 {
            position = point.applying(transform)
        } else {
            let t = transform
            element += " transform=\"matrix(\([t.a, t.b, t.c, t.d, t.tx, t.ty].map(Self.number).joined(separator: " ")))\""
        }
        element += " x=\"\(Self.number(position.x))\" y=\"\(Self.number(position.y))\""
        element += " font-family=\"\(text.monospaced ? Self.monospacedFamily : Self.family)\""
        element += " font-size=\"\(Self.number(text.size))\" font-weight=\"\(Self.weight(text.weight))\""
        element += " text-anchor=\"\(anchor.x <= 0 ? "start" : anchor.x >= 1 ? "end" : "middle")\""
        element += " dominant-baseline=\"\(anchor.y <= 0 ? "text-before-edge" : anchor.y >= 1 ? "text-after-edge" : "central")\""
        element += " fill=\"#000\""
        if alpha != 1 { element += " fill-opacity=\"\(Self.number(alpha))\"" }
        output.elements.append(element + ">\(Self.escaped(text.string))</text>")
    }

    mutating func translateBy(x: CGFloat, y: CGFloat) {
        transform = CGAffineTransform(translationX: x, y: y).concatenating(transform)
    }

    mutating func rotate(by angle: Angle) {
        transform = CGAffineTransform(rotationAngle: angle.radians).concatenating(transform)
    }

    static let family = "'Helvetica Neue', Helvetica, Arial, sans-serif"
    static let monospacedFamily = "Menlo, 'SF Mono', Consolas, 'DejaVu Sans Mono', monospace"

    static func weight(_ weight: Font.Weight) -> Int {
        switch weight {
        case .ultraLight: 100
        case .thin: 200
        case .light: 300
        case .medium: 500
        case .semibold: 600
        case .bold: 700
        case .heavy: 800
        case .black: 900
        default: 400
        }
    }

    /// The path's `d` attribute: `M`, `L`, `Q`, `C`, `Z`, space separated.
    static func d(for path: Path) -> String {
        var parts: [String] = []
        func pt(_ p: CGPoint) -> String { "\(number(p.x)) \(number(p.y))" }
        path.forEach { element in
            switch element {
            case .move(let to): parts.append("M" + pt(to))
            case .line(let to): parts.append("L" + pt(to))
            case .quadCurve(let to, let control): parts.append("Q" + pt(control) + " " + pt(to))
            case .curve(let to, let control1, let control2):
                parts.append("C" + pt(control1) + " " + pt(control2) + " " + pt(to))
            case .closeSubpath: parts.append("Z")
            }
        }
        return parts.joined(separator: " ")
    }

    /// At most 2 decimals, trailing zeros and the dot trimmed, `-0` as `0`,
    /// `.` as the separator in every locale.
    static func number(_ value: CGFloat) -> String {
        var text = String(format: "%.2f", Double(value))
        if text.contains(".") {
            while text.hasSuffix("0") { text.removeLast() }
            if text.hasSuffix(".") { text.removeLast() }
        }
        return text == "-0" ? "0" : text
    }

    /// XML text: `&`, `<`, `>`, `"` and `'` escaped.
    static func escaped(_ text: String) -> String {
        var out = ""
        for character in text {
            switch character {
            case "&": out += "&amp;"
            case "<": out += "&lt;"
            case ">": out += "&gt;"
            case "\"": out += "&quot;"
            case "'": out += "&apos;"
            default: out.append(character)
            }
        }
        return out
    }
}
