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

/// The side of the board a part or text is on. The canvas always looks
/// from the top; a bottom item is drawn mirrored, under the top ones.
nonisolated enum BoardSide: String, CaseIterable, Hashable, Sendable {
    case top, bottom

    var title: String {
        switch self {
        case .top: "Top"
        case .bottom: "Bottom"
        }
    }

    /// The copper layer on this side.
    var copperLayer: CopperLayer {
        switch self {
        case .top: .top
        case .bottom: .bottom
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

// MARK: - Copper layers

/// One copper layer of the board, by name: Top, an inner layer by its place
/// in the stack (Layer 2 to Layer 5), or Bottom. Bottom is Bottom on every
/// board, so a bottom-side part keeps its layer when the count changes.
nonisolated enum CopperLayer: Hashable, Sendable {
    case top
    /// 2...5, its place in the stack.
    case inner(Int)
    case bottom

    /// The file value: "top", "inner2" to "inner5", or "bottom".
    var rawValue: String {
        switch self {
        case .top: "top"
        case let .inner(n): "inner\(n)"
        case .bottom: "bottom"
        }
    }

    init?(rawValue: String) {
        switch rawValue {
        case "top": self = .top
        case "bottom": self = .bottom
        default:
            guard rawValue.hasPrefix("inner"), let n = Int(rawValue.dropFirst(5)),
                  (2...5).contains(n), rawValue == "inner\(n)" else { return nil }
            self = .inner(n)
        }
    }

    var title: String {
        switch self {
        case .top: "Top"
        case let .inner(n): "Layer \(n)"
        case .bottom: "Bottom"
        }
    }

    var isInner: Bool {
        if case .inner = self { return true }
        return false
    }

    /// The layers of a board with `count` copper layers, Top first and
    /// Bottom last. A 1-layer board has only Top.
    static func allCases(for count: Int) -> [CopperLayer] {
        guard count > 1 else { return [.top] }
        return [.top] + (2..<count).map { .inner($0) } + [.bottom]
    }
}

/// What a copper layer is for: Top and Bottom carry components; an inner
/// layer is a plane (ground, power) or carries wires.
nonisolated enum LayerType: String, CaseIterable, Hashable, Sendable {
    case component, plane, wire

    var title: String {
        switch self {
        case .component: "Component"
        case .plane: "Plane"
        case .wire: "Wire"
        }
    }
}

/// An sRGB colour, saved as "#RRGGBB".
nonisolated struct LayerColor: Hashable, Sendable {
    var red: UInt8
    var green: UInt8
    var blue: UInt8

    init(red: UInt8, green: UInt8, blue: UInt8) {
        self.red = red
        self.green = green
        self.blue = blue
    }

    /// "#RRGGBB", either case; nil for anything else.
    init?(hex: String) {
        guard hex.count == 7, hex.first == "#" else { return nil }
        let digits = hex.dropFirst()
        guard digits.allSatisfy(\.isHexDigit), let value = UInt32(digits, radix: 16) else { return nil }
        red = UInt8((value >> 16) & 0xFF)
        green = UInt8((value >> 8) & 0xFF)
        blue = UInt8(value & 0xFF)
    }

    /// "#RRGGBB", upper case.
    var hex: String {
        String(format: "#%02X%02X%02X", red, green, blue)
    }
}

/// The schematic net a Plane layer carries: a supply rail (ground is the
/// 0 mV rail), or a signal net, named by one of its pins (a part's id and
/// the pin's designator), which outlasts any one wire of it.
nonisolated enum PlaneNet: Hashable, Sendable {
    case rail(mv: UInt32)
    case pin(chip: UUID, designator: String)

    static let ground = PlaneNet.rail(mv: 0)

    /// As the file stores it: "ground", "power:<mV>" or
    /// "pin:<part UUID>/<designator>".
    var fileValue: String {
        switch self {
        case .rail(0): "ground"
        case let .rail(mv): "power:\(mv)"
        case let .pin(chip, designator): "pin:\(chip.uuidString)/\(designator)"
        }
    }

    init?(fileValue: String) {
        if fileValue == "ground" {
            self = .ground
        } else if fileValue.hasPrefix("power:"), let mv = UInt32(fileValue.dropFirst(6)), mv > 0 {
            self = .rail(mv: mv)
        } else if fileValue.hasPrefix("pin:"), let slash = fileValue.firstIndex(of: "/"),
                  let chip = UUID(uuidString: String(fileValue[fileValue.index(fileValue.startIndex, offsetBy: 4)..<slash])),
                  fileValue.index(after: slash) < fileValue.endIndex {
            self = .pin(chip: chip, designator: String(fileValue[fileValue.index(after: slash)...]))
        } else {
            return nil
        }
    }
}

/// How a copper layer is drawn and what it is for.
nonisolated struct LayerStyle: Hashable, Sendable {
    var color: LayerColor
    var type: LayerType
    /// A Plane's net; nil is copper on no net. Kept while the layer is a
    /// Wire layer, so it comes back with the Plane.
    var net: PlaneNet? = nil

    /// Red Top, blue Bottom, and gold, green, purple and cyan inner layers.
    /// Top and Bottom are Component; inner layers start as Plane.
    static func defaultStyle(of layer: CopperLayer) -> LayerStyle {
        switch layer {
        case .top: LayerStyle(color: LayerColor(hex: "#C83737")!, type: .component)
        case .bottom: LayerStyle(color: LayerColor(hex: "#3A6FD8")!, type: .component)
        case let .inner(n):
            LayerStyle(color: LayerColor(hex: defaultInnerColors[(n - 2) % 4])!, type: .plane)
        }
    }

    private static let defaultInnerColors = ["#D8B23A", "#3AAE5C", "#B04AD0", "#3AB8C8"]
}

// MARK: - Board

/// The board: a rectangle from (0, 0), its copper layers and colour, and
/// the units the sidebar uses.
nonisolated struct BoardSettings: Hashable, Sendable {
    /// What the board is: a free rectangle, or a standard add-in card with
    /// its edge connector (`BoardType`).
    var type: BoardType = .custom
    /// A custom card's edge connector; used only when `type` is
    /// `.customCard`, but kept so it comes back.
    var customCard = CustomCard()
    /// The settings of the card types that have them (PCI Express, M.2,
    /// Mini PCIe, SO-DIMM, DIMM); kept across type changes.
    var cardOptions = CardOptions()
    /// The schematic bus chosen as the card's edge: its wires are the
    /// card's pins, in order. Set back to nil as soon as it stops counting
    /// (`SchematicDocument.edgeBusID`): the board isn't a card, the card's
    /// pin count changed, or the bus is gone.
    var edgeBus: UUID?
    /// mm, within `sizeRange`. For a card, the bounding box of the card
    /// with its edge tab: `width` is the card's length (at least the card's
    /// least), `height` the standard's.
    var width: Double = 100
    var height: Double = 80
    /// The copper layer count, one of `layerCounts`: Top first, then the
    /// inner layers, Bottom last (see `copperLayers`).
    var layers: Int = 2
    var color: BoardColor = .green
    var units: BoardUnits = .mm
    /// The component grid in mm, within `gridRange`: footprints and texts
    /// snap to it, and the canvas dots it. Changing it moves nothing.
    var grid: Double = Double(BoardGrid.spacing)
    /// Styles the user has set. A layer without one uses its default. A
    /// style is kept when its layer leaves the board, so it comes back with
    /// the layer.
    var styles: [CopperLayer: LayerStyle] = [:]

    static let layerCounts = [1, 2, 4, 6]
    static let sizeRange: ClosedRange<Double> = 1...1000
    static let gridRange: ClosedRange<Double> = 0.01...10

    /// The trace grid in mm: the component grid with a point between each
    /// two of its points. Trace ends snap to it. Not set on its own.
    var traceGrid: Double { grid / 2 }

    /// The board's copper layers in stack order.
    var copperLayers: [CopperLayer] { CopperLayer.allCases(for: layers) }

    /// The board's outline, clockwise from the top-left (mm): the
    /// rectangle, or a card's shape with its edge tab and key notches.
    var outline: [CGPoint] {
        card?.outline(length: width) ?? [CGPoint(x: 0, y: 0), CGPoint(x: width, y: 0),
                                         CGPoint(x: width, y: height), CGPoint(x: 0, y: height)]
    }

    /// The connector's pins: a card's gold fingers, or a HAT's or
    /// shield's header pads; none on a custom board.
    var fingers: [EdgeFinger] { card?.fingers ?? [] }

    /// The standard's mounting holes; none on a custom board or a card.
    var holes: [MountingHole] { card?.holes ?? [] }

    /// The standard's header pads that no bus carries (an Arduino shield's
    /// ICSP header, when chosen).
    var extraPads: [EdgeFinger] { card?.extraPads ?? [] }

    /// What the connector's pins are labelled in lists: "Edge" or "Header".
    var connectorName: String { card?.connectorName ?? "Edge" }

    /// Whether the board needs a Bottom layer: its connector has fingers
    /// (a header's pads go through every layer).
    var needsTwoLayers: Bool { fingers.contains { !$0.isThroughHole } }

    /// How many pins the card has: its distinct finger names (a finger on
    /// each side with one name is one pin). 0 on a custom board.
    var pinCount: Int { pinNames.count }

    /// The card's pin names, each once, in natural order ("A1", "A2", …,
    /// "A10", "B1"; numbers before letters), or the standard's own order
    /// (an Arduino shield's), as a bus of its type names its wires. Pin `i`
    /// is the edge bus's wire `i`.
    var pinNames: [String] {
        if let order = card?.pinOrder { return order }
        var seen = Set<String>()
        return fingers.map(\.name).filter { seen.insert($0).inserted }
            .sorted { $0.localizedStandardCompare($1) == .orderedAscending }
    }

    /// The card the board is: a standard's, or the custom card built from
    /// `customCard` at the board's height. Nil for a plain board.
    var card: CardSpec? {
        type == .customCard ? customCard.spec(height: height) : type.card(cardOptions)
    }

    /// The layers a trace can go on: every board layer that isn't a Plane.
    var traceLayers: [CopperLayer] { copperLayers.filter { style(of: $0).type != .plane } }

    /// The layer's stored style, or its default.
    func style(of layer: CopperLayer) -> LayerStyle {
        styles[layer] ?? LayerStyle.defaultStyle(of: layer)
    }
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
    /// On Bottom the text is drawn mirrored inside its own box.
    var side: BoardSide = .top
    /// Locked: dragging, the sidebar's X and Y, and Rotate leave it where
    /// it is.
    var locked = false

    static let sizeRange = 10...1000
    static let defaultSize = 50

    /// The font size in mm (1 mil = 0.0254 mm).
    var sizeMM: Double { Double(size) * 0.0254 }
}

// MARK: - Layer visibility

/// A layer the PCB page's View popover can show or hide: a copper layer,
/// the board's own fill (its solder-mask colour; the outline always
/// shows), the silkscreen, the document layer, or the ratsnest (the
/// unrouted connections' lines).
nonisolated enum BoardViewLayer: Hashable, Sendable {
    case copper(CopperLayer)
    case board
    case silkscreen
    case document
    case ratsnest

    var title: String {
        switch self {
        case let .copper(layer): layer.title
        case .board: "PCB"
        case .silkscreen: "Silkscreen"
        case .document: "Document"
        case .ratsnest: "Ratsnest"
        }
    }
}

/// Which layers the PCB canvas draws, and so which items a press can hit.
/// Holds the hidden set, so layers added later start visible. Per window;
/// never saved.
///
/// A part's marks and texts belong to its side: they show only while that
/// side's copper layer shows too. A through-hole pad goes through every
/// layer, so it shows while any copper layer shows.
nonisolated struct BoardVisibility: Equatable, Sendable {
    var hidden: Set<BoardViewLayer> = []
    /// The board's copper layers, for `showsAnyCopper`.
    var copperLayers: [CopperLayer] = CopperLayer.allCases(for: 6)

    func shows(_ layer: BoardViewLayer) -> Bool { !hidden.contains(layer) }

    var showsAnyCopper: Bool { copperLayers.contains { shows(.copper($0)) } }

    /// A pad: surface mount (`drill == 0`) on its side's layer, or through
    /// hole.
    func showsPad(drill: Double, side: BoardSide) -> Bool {
        drill > 0 ? showsAnyCopper : shows(.copper(side.copperLayer))
    }

    /// A footprint mark (outline, pin 1, reference) on `layer`, on `side`.
    func showsMark(_ layer: MarkLayer, side: BoardSide) -> Bool {
        let view: BoardViewLayer
        switch layer {
        case .hidden: return false
        case .document: view = .document
        case .silkscreen: view = .silkscreen
        }
        return shows(view) && shows(.copper(side.copperLayer))
    }

    func showsText(_ text: BoardText) -> Bool {
        let view: BoardViewLayer = text.layer == .silkscreen ? .silkscreen : .document
        return shows(view) && shows(.copper(text.side.copperLayer))
    }

    /// A Plane layer's tint over the board.
    func showsPlane(_ layer: CopperLayer, style: LayerStyle) -> Bool {
        style.type == .plane && shows(.copper(layer))
    }

    /// A trace on `layer`.
    func showsTrace(on layer: CopperLayer) -> Bool { shows(.copper(layer)) }

    /// Whether anything of the footprint is drawn, so it can be hit.
    func showsFootprint(_ chip: PlacedChip) -> Bool {
        guard let placement = chip.board else { return false }
        if [placement.outline, placement.reference].contains(where: { showsMark($0, side: placement.side) }) {
            return true
        }
        if chip.pin1Mark != nil, showsMark(placement.pin1, side: placement.side) { return true }
        return chip.boardPads.contains { showsPad(drill: $0.drill, side: placement.side) }
    }
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
