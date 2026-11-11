import CoreGraphics

/// What a board is: a free rectangle (`custom`), a card whose edge the user
/// builds (`customCard`), or one of the standard boards in `boards.json`
/// (a card with gold fingers, a cartridge, or a board on a header), named
/// by its id. Raw values are the file values.
nonisolated struct BoardType: RawRepresentable, CaseIterable, Hashable, Sendable {
    let rawValue: String

    static let custom = BoardType(id: "custom")
    static let customCard = BoardType(id: "customCard")

    private init(id: String) { rawValue = id }

    /// The type `rawValue` names; nil for an id no board has.
    init?(rawValue: String) {
        guard rawValue == Self.custom.rawValue || rawValue == Self.customCard.rawValue
            || BoardCatalog.shared.board(rawValue) != nil else { return nil }
        self.rawValue = rawValue
    }

    /// Custom, Custom Card, then the catalog's boards in file order.
    static var allCases: [BoardType] {
        [.custom, .customCard] + BoardCatalog.shared.boards.map { BoardType(id: $0.id) }
    }

    /// The standard's definition; nil for Custom and Custom Card.
    var definition: BoardDefinition? { BoardCatalog.shared.board(rawValue) }

    var title: String {
        switch self {
        case .custom: "Custom"
        case .customCard: "Custom Card"
        default: definition?.title ?? rawValue
        }
    }

    /// The type menu's section: none for Custom and Custom Card.
    var group: String? { definition?.group }

    /// The sidebar's heading over the type's settings.
    var optionsTitle: String { definition?.optionsTitle ?? "Card" }

    /// Whether the type has settings of its own (`CardOptions`), shown in
    /// the sidebar.
    var hasOptions: Bool { !(definition?.options.isEmpty ?? true) }

    /// Whether any of its settings change the pins a bus of this type
    /// carries (and so are stored with the bus).
    var hasBusOptions: Bool { definition?.options.contains(where: \.onBus) ?? false }

    /// The standard built with `options`, `width` long (its default length
    /// when nil). Nil for Custom and Custom Card.
    func card(_ options: CardOptions, width: Double? = nil) -> CardSpec? {
        BoardCatalog.shared.spec(rawValue, options: options, width: width)
    }

    /// Whether the board has a connector whose pins a bus can carry: a
    /// card's edge tab with fingers, or a header.
    var isCard: Bool { self != .custom }
}

/// The values of the board's settings, by option id (`boards.json`): PCI
/// Express `lanes`, M.2 `key`, …. Kept across type changes; a value that
/// isn't one of the type's choices counts as its default.
nonisolated struct CardOptions: Hashable, Sendable {
    var values: [String: BoardValue] = [:]

    subscript(id: String) -> BoardValue? {
        get { values[id] }
        set { values[id] = newValue }
    }
}

/// A card edge the user builds: one tab of evenly spaced fingers, on Top
/// (A1, A2, …) and, when `bothSides`, on Bottom (B1, B2, …), numbered from
/// the left. Lengths in mm. `clamped()` keeps the parts fitting together.
nonisolated struct CustomCard: Hashable, Sendable {
    /// Fingers on each side.
    var pads = 20
    /// Finger centre to centre.
    var pitch = 2.54
    var padWidth = 1.524
    var padLength = 7.62
    /// From the card's bottom edge up to the fingers.
    var padOffset = 0.0
    /// How far the tab reaches below the card's body.
    var depth = 7.62
    /// From the card's left edge to the tab.
    var indent = 10.0
    /// From each end of the tab to the centre of its outermost finger.
    var margin = 2.54
    /// The tab's lower corners.
    var chamfer = 0.5
    var bothSides = true

    static let padRange = 1...200
    static let pitchRange: ClosedRange<Double> = 0.5...10
    static let depthRange: ClosedRange<Double> = 1...50
    static let indentRange: ClosedRange<Double> = 0...500

    /// The tab's length along the card.
    var tabLength: Double { 2 * margin + Double(pads - 1) * pitch }

    /// Every setting in range: a finger narrower than its pitch, fingers
    /// on the tab, margins at least half a finger, a chamfer that fits.
    func clamped() -> CustomCard {
        var card = self
        card.pads = min(max(pads, Self.padRange.lowerBound), Self.padRange.upperBound)
        card.pitch = min(max(pitch, Self.pitchRange.lowerBound), Self.pitchRange.upperBound)
        card.padWidth = min(max(padWidth, 0.1), card.pitch - 0.05)
        card.depth = min(max(depth, Self.depthRange.lowerBound), Self.depthRange.upperBound)
        card.padOffset = min(max(padOffset, 0), card.depth - 0.1)
        card.padLength = min(max(padLength, 0.1), card.depth - card.padOffset)
        card.indent = min(max(indent, Self.indentRange.lowerBound), Self.indentRange.upperBound)
        card.margin = min(max(margin, card.padWidth / 2), 100)
        card.chamfer = min(max(chamfer, 0), min(card.depth, card.margin - card.padWidth / 2))
        return card
    }

    /// The card `height` high and `length` long (at least long enough for
    /// its tab): clockwise from the top-left, down the front end, along the
    /// bottom edge to the tab, round it, and up the bracket edge.
    func spec(height: Double, length requested: Double) -> CardSpec {
        let card = clamped()
        let bottom = height, edge = height - card.depth
        var fingers: [EdgeFinger] = []
        for n in 1...card.pads {
            let centre = card.indent + card.margin + Double(n - 1) * card.pitch
            let rect = CGRect(x: centre - card.padWidth / 2, y: bottom - card.padOffset - card.padLength,
                              width: card.padWidth, height: card.padLength)
            fingers.append(EdgeFinger(name: "A\(n)", rect: rect, side: .top))
            if card.bothSides { fingers.append(EdgeFinger(name: "B\(n)", rect: rect, side: .bottom)) }
        }
        let start = card.indent, end = card.indent + card.tabLength
        let length = min(max(requested, end), 1000)
        let left = start > 0 ? card.chamfer : 0, right = card.chamfer
        var outline = [CGPoint(x: 0, y: 0), CGPoint(x: length, y: 0), CGPoint(x: length, y: edge),
                       CGPoint(x: end, y: edge), CGPoint(x: end, y: bottom - right)]
        if right > 0 { outline.append(CGPoint(x: end - right, y: bottom)) }
        if left > 0 { outline.append(CGPoint(x: start + left, y: bottom)) }
        outline.append(CGPoint(x: start, y: bottom - left))
        if start > 0 { outline += [CGPoint(x: start, y: edge), CGPoint(x: 0, y: edge)] }
        return CardSpec(height: height, length: max(100, (end + 10).rounded(.up)), minimumLength: end,
                        maximumLength: 1000, outline: outline, fingers: fingers)
    }
}

/// One pin of the board's connector, named as the bus names it ("A1",
/// "B31", "40", "D13"): a card's gold finger, a copper pad on Top
/// (component side) or Bottom (solder side); or a plated through-hole pad
/// of a header (`drill` > 0), on every layer, round or (pin 1) square.
nonisolated struct EdgeFinger: Hashable, Sendable {
    let name: String
    /// mm on the board.
    let rect: CGRect
    /// A finger's side; a header pad is on both.
    let side: BoardSide
    /// mm; 0 for a finger.
    var drill: Double = 0
    /// A round header pad; false is square.
    var round = false

    /// A header pad through every layer.
    var isThroughHole: Bool { drill > 0 }

    /// Whether its copper is on `layer`.
    func isOn(_ layer: CopperLayer) -> Bool { isThroughHole || side.copperLayer == layer }

    /// Where a trace meets it: the middle of a finger's inner end (away
    /// from the card edge), or a header pad's centre.
    var contact: CGPoint {
        isThroughHole ? CGPoint(x: rect.midX, y: rect.midY) : CGPoint(x: rect.midX, y: rect.minY)
    }

    /// As a part's pad, for connectivity and drawing.
    var pad: PlacedPad {
        PlacedPad(designator: name, rect: rect, drill: drill, shape: round ? .round : .rect, radius: 0,
                  chamfer: 0, corners: [])
    }

    /// A header pad `size` square (or round) about `centre`.
    static func header(_ name: String, centre: CGPoint, size: Double = 1.8, drill: Double = 1.1,
                       round: Bool = true) -> EdgeFinger {
        EdgeFinger(name: name, rect: CGRect(x: centre.x - size / 2, y: centre.y - size / 2, width: size, height: size),
                   side: .top, drill: drill, round: round)
    }
}

/// A non-plated mounting hole of a board standard.
nonisolated struct MountingHole: Hashable, Sendable {
    /// mm on the board.
    let centre: CGPoint
    let drill: Double

    /// As a non-plated pad, for connectivity (a Plane is cut round it) and
    /// drawing.
    var pad: PlacedPad {
        PlacedPad(designator: nil, rect: CGRect(x: centre.x - drill / 2, y: centre.y - drill / 2, width: drill,
                                                height: drill),
                  drill: drill, shape: .round, radius: 0, chamfer: 0, corners: [], plated: false)
    }
}

/// A board standard built for one set of options and one length, in board
/// mm with the top edge at y = 0, seen from the component side (Top). A
/// card's edge tab is along the bottom, its bracket end at x = 0.
nonisolated struct CardSpec: Hashable, Sendable {
    let height: Double
    /// The length a new board of this type gets.
    let length: Double
    /// The shortest board that still clears its connector.
    let minimumLength: Double
    /// The longest (a full-length card).
    let maximumLength: Double
    /// Clockwise from the top-left.
    let outline: [CGPoint]
    let fingers: [EdgeFinger]
    /// The standard's mounting holes.
    var holes: [MountingHole] = []
    /// Header pads that no bus carries (an Arduino shield's ICSP header).
    var extraPads: [EdgeFinger] = []
    /// The pin names in the connector's own order, when natural order
    /// isn't it (an Arduino shield's D0…D13, A0…A5, power pins).
    var pinOrder: [String]? = nil
    /// What the connector's pins are labelled in lists ("Edge.A1",
    /// "Header.40").
    var connectorName = "Edge"

    /// Whether the board's length can't change (a cartridge, a HAT).
    var isFixedSize: Bool { minimumLength == maximumLength }
}
