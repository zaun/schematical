import SwiftUI

/// How the PCB page colours the board and its layers.
extension BoardColor {
    /// The solder mask colour filling the board rectangle.
    var fill: Color {
        switch self {
        case .green: Color(red: 0.07, green: 0.36, blue: 0.17)
        case .black: Color(red: 0.08, green: 0.08, blue: 0.08)
        case .red: Color(red: 0.62, green: 0.10, blue: 0.12)
        case .blue: Color(red: 0.08, green: 0.22, blue: 0.55)
        case .yellow: Color(red: 0.85, green: 0.72, blue: 0.12)
        case .white: Color(red: 0.93, green: 0.93, blue: 0.91)
        case .purple: Color(red: 0.36, green: 0.16, blue: 0.50)
        }
    }

    /// The silkscreen ink that reads on this board: black on the light
    /// boards (yellow, white), white on the rest.
    var silkscreen: Color {
        switch self {
        case .yellow, .white: Self.blackInk
        default: Self.whiteInk
        }
    }

    static let whiteInk = Color(white: 0.97)
    static let blackInk = Color(white: 0.05)
    /// Document-layer marks: shown on screen, not printed.
    static let document = Color(white: 0.6)
}

extension MarkLayer {
    /// The mark's colour on a board of this colour; nil when hidden.
    func color(on board: BoardColor) -> Color? {
        switch self {
        case .hidden: nil
        case .document: BoardColor.document
        case .silkscreen: board.silkscreen
        }
    }
}

extension TextLayer {
    func color(on board: BoardColor) -> Color {
        switch self {
        case .document: BoardColor.document
        case .silkscreen: board.silkscreen
        }
    }
}

extension LayerColor {
    /// The colour for drawing.
    var color: Color {
        Color(.sRGB, red: Double(red) / 255, green: Double(green) / 255, blue: Double(blue) / 255)
    }

    /// A colour from the colour well, in sRGB, rounded to bytes.
    init(_ color: Color) {
        let rgb = NSColor(color).usingColorSpace(.sRGB) ?? NSColor.black
        func byte(_ value: CGFloat) -> UInt8 { UInt8(min(max((value * 255).rounded(), 0), 255)) }
        self.init(red: byte(rgb.redComponent), green: byte(rgb.greenComponent), blue: byte(rgb.blueComponent))
    }
}
