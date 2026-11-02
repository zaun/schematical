import CoreGraphics
import Foundation

// MARK: - Board choices

/// The colour of the board's solder mask. Raw values are the file values.
nonisolated enum BoardColor: String, CaseIterable, Hashable, Sendable {
    case green, black, red, blue, yellow, white, purple

    var title: String {
        switch self {
        case .green: "Green"
        case .black: "Black"
        case .red: "Red"
        case .blue: "Blue"
        case .yellow: "Yellow"
        case .white: "White"
        case .purple: "Purple"
        }
    }
}

/// The units the PCB sidebar shows lengths in. The file always stores mm.
nonisolated enum BoardUnits: String, CaseIterable, Hashable, Sendable {
    case mm
    case inch = "in"

    var title: String { rawValue }
}

/// Where a footprint mark is drawn: not at all, on the document layer (gray,
/// not printed), or on the silkscreen.
nonisolated enum MarkLayer: String, CaseIterable, Hashable, Sendable {
    case hidden, document, silkscreen

    var title: String {
        switch self {
        case .hidden: "Hidden"
        case .document: "Document"
        case .silkscreen: "Silkscreen"
        }
    }
}

/// Which of a part's two footprints is on the board (the PCB Style option,
/// for parts with a connector footprint). Raw values are the file values.
nonisolated enum FootprintStyle: String, CaseIterable, Hashable, Sendable {
    case module, connector

    var title: String {
        switch self {
        case .module: "Full Module"
        case .connector: "Connector Only"
        }
    }
}

/// The layer a board text is on.
nonisolated enum TextLayer: String, CaseIterable, Hashable, Sendable {
    case document, silkscreen

    var title: String {
        switch self {
        case .document: "Document"
        case .silkscreen: "Silkscreen"
        }
    }
}

/// The point of a footprint that snaps to the grid, that it turns about, and
/// whose X/Y the sidebar shows. The box points are of the footprint's
/// bounding box (body plus pads) as turned.
nonisolated enum FootprintOrigin: String, CaseIterable, Hashable, Sendable {
    case center, topLeft, centerLeft, bottomLeft, pad1Center

    var title: String {
        switch self {
        case .center: "Center"
        case .topLeft: "Top Left"
        case .centerLeft: "Center Left"
        case .bottomLeft: "Bottom Left"
        case .pad1Center: "Pad 1 Center"
        }
    }
}

// MARK: - Board

/// The board: a rectangle from (0, 0), its copper layer count and colour, and
/// the units the sidebar uses.
nonisolated struct BoardSettings: Hashable, Sendable {
    /// mm, within `sizeRange`.
    var width: Double = 100
    var height: Double = 80
    /// Copper layers, one of `layerCounts`. Stored only; the top side is drawn.
    var layers: Int = 2
    var color: BoardColor = .green
    var units: BoardUnits = .mm

    static let layerCounts = [1, 2, 4, 6]
    static let sizeRange: ClosedRange<Double> = 1...1000
}

/// Free text on the board.
nonisolated struct BoardText: Identifiable, Hashable, Sendable {
    let id: UUID
    /// May hold line breaks; never empty.
    var text: String
    /// mm: the top-left of the text at rotation 0. On the grid.
    var anchor: CGPoint
    /// Font size in mil, within `sizeRange`.
    var size: Int
    var layer: TextLayer = .silkscreen
    /// 0, 90, 180 or 270, clockwise about `anchor`.
    var rotation: Int = 0

    static let sizeRange = 10...1000
    static let defaultSize = 50

    /// The font size in mm (1 mil = 0.0254 mm).
    var sizeMM: Double { Double(size) * 0.0254 }
}

// MARK: - References

/// A part reference split into a letter prefix and a number: "U12" is "U" and
/// 12. A reference not of that shape keeps the whole string as the prefix and
/// has no number.
nonisolated struct ReferenceParts: Hashable, Sendable {
    var prefix: String
    var number: Int?

    init(_ reference: String) {
        let letters = reference.prefix { $0.isASCII && $0.isLetter }
        let digits = reference.dropFirst(letters.count)
        if !letters.isEmpty, !digits.isEmpty, digits.allSatisfy({ $0.isASCII && $0.isNumber }),
           let number = Int(digits) {
            prefix = String(letters)
            self.number = number
        } else {
            prefix = reference
            number = nil
        }
    }

    /// Why the prefix and number don't make a reference, or nil when they do.
    static func validate(prefix: String, number: Int?) -> String? {
        guard (1...8).contains(prefix.count), prefix.allSatisfy({ $0.isASCII && $0.isLetter }) else {
            return "The prefix must be 1 to 8 letters."
        }
        guard let number, number >= 1 else {
            return "The number must be 1 or more."
        }
        return nil
    }

    /// The reference made from a valid prefix and number: "r", 3 → "R3".
    static func reference(prefix: String, number: Int) -> String {
        prefix.uppercased() + String(number)
    }
}

extension CatalogPart {
    /// The designator of the part's first pin: the pin the pin-1 mark and the
    /// "Pad 1 Center" origin use. Empty for a part with no pins.
    nonisolated var firstPinDesignator: String {
        pins.first?.designator ?? ""
    }
}
