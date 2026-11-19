import Foundation

/// A footprint made from a few numbers (plan-46-J): one of the parametric
/// styles of docs/parts-format.md § Footprints (`dual`, `quad`, `grid`,
/// `header`), which the backend expands into pads. A custom one lives in the
/// part file's `footprints`, like an explicit custom footprint.
///
/// `init?(json:)` and `encoded()` are inverses for an entry of these styles:
/// keys the form has no control for (`note`, `centre`, `slots`, a pad's
/// corner keys) are kept and written back.
struct ParametricFootprintDraft: Equatable, Sendable {
    enum Style: String, CaseIterable, Sendable {
        case dual, quad, grid, header

        /// The style in a hobbyist's words.
        var title: String {
            switch self {
            case .dual: "Two rows"
            case .quad: "Four sides"
            case .grid: "Ball grid"
            case .header: "Pin header"
            }
        }

        var examples: String {
            switch self {
            case .dual: "DIP, SOIC, TSSOP, SOT-23, chip parts"
            case .quad: "QFN, QFP, LCC"
            case .grid: "BGA, WLCSP"
            case .header: "Through-hole pin headers and sockets"
            }
        }
    }

    var id: String
    var description: String
    var style: Style
    /// The number of designated pads; the pinout's pin count.
    var pins: Int
    var bodyWidth: Double
    var bodyHeight: Double
    var bodyRound = false
    var padWidth: Double
    var padHeight: Double
    /// "rect" or "round".
    var padShape = "rect"
    /// A through-hole diameter; nil = surface mount (the key is left out).
    var drill: Double? = nil
    /// Pin 1's pad shape, when it differs (through-hole parts mark pin 1
    /// with a square pad); `dual`, `quad` and `header` only.
    var pin1Shape: String? = nil

    /// Pad-centre distance along a row or column (every style).
    var pitch: Double

    // dual
    /// Centre-to-centre distance between the two columns.
    var span: Double = 0
    /// Pads per column; nil = `pins / 2`.
    var perSide: Int? = nil

    // quad
    /// Slot counts `[left, bottom, right, top]`.
    var sides: [Int] = [0, 0, 0, 0]
    var spanX: Double = 0
    var spanY: Double = 0

    // grid and header
    var rows = 1
    // grid
    var columns = 1
    /// The populated ball names: the pinout's designators.
    var balls: [String] = []

    // header
    /// Row 2's y minus row 1's; nil with one row.
    var rowPitch: Double? = nil
    /// y of row 1 from the body centre.
    var firstRowY: Double = 0

    /// Top-level keys with no control (`note`, `centre`, `slots`), kept.
    var extras: [String: JSONValue] = [:]
    /// The pad's keys with no control (`radius`, `chamfer`, `corners`), kept.
    var padExtras: [String: JSONValue] = [:]

    // MARK: Writing

    /// The `footprints.json` entry: exactly the style's keys.
    func encoded() -> JSONValue {
        var body: [String: JSONValue] = ["width": .number(bodyWidth), "height": .number(bodyHeight)]
        if bodyRound { body["round"] = .bool(true) }
        var pad = padExtras
        pad["width"] = .number(padWidth)
        pad["height"] = .number(padHeight)
        pad["shape"] = .string(padShape)
        var keys = extras
        keys["id"] = .string(id)
        keys["description"] = .string(description)
        keys["style"] = .string(style.rawValue)
        keys["pins"] = .number(Double(pins))
        keys["body"] = .object(body)
        keys["pad"] = .object(pad)
        keys["pitch"] = .number(pitch)
        if let drill { keys["drill"] = .number(drill) }
        if let pin1Shape, style != .grid { keys["pin1Shape"] = .string(pin1Shape) }
        switch style {
        case .dual:
            keys["span"] = .number(span)
            if let perSide { keys["perSide"] = .number(Double(perSide)) }
        case .quad:
            keys["sides"] = .array(sides.map { .number(Double($0)) })
            keys["spanX"] = .number(spanX)
            keys["spanY"] = .number(spanY)
        case .grid:
            keys["rows"] = .number(Double(rows))
            keys["columns"] = .number(Double(columns))
            keys["balls"] = .array(balls.map { .string($0) })
        case .header:
            keys["rows"] = .number(Double(rows))
            keys["firstRowY"] = .number(firstRowY)
            if let rowPitch { keys["rowPitch"] = .number(rowPitch) }
        }
        return .object(keys)
    }

    // MARK: Reading

    /// The keys each style's form controls; the rest of an entry is `extras`.
    private static let common: Set<String> = ["id", "description", "style", "pins", "body", "pad", "pitch", "drill",
                                              "pin1Shape"]
    private static let own: [Style: Set<String>] = [
        .dual: ["span", "perSide"],
        .quad: ["sides", "spanX", "spanY"],
        .grid: ["rows", "columns", "balls"],
        .header: ["rows", "firstRowY", "rowPitch"],
    ]

    /// An entry of one of the four styles; nil for an `explicit` entry or
    /// one whose keys are not in the expected shape.
    init?(json: JSONValue) {
        guard case let .object(keys) = json, case let .string(styleName)? = keys["style"],
              let style = Style(rawValue: styleName),
              case let .string(id)? = keys["id"], case let .string(description)? = keys["description"],
              case let .number(pins)? = keys["pins"], pins == pins.rounded(),
              case let .object(body)? = keys["body"], case let .number(bodyWidth)? = body["width"],
              case let .number(bodyHeight)? = body["height"], Set(body.keys).isSubset(of: ["width", "height", "round"]),
              case var .object(pad)? = keys["pad"], case let .number(padWidth)? = pad["width"],
              case let .number(padHeight)? = pad["height"], case let .string(padShape)? = pad["shape"],
              case let .number(pitch)? = keys["pitch"] else { return nil }
        self.id = id
        self.description = description
        self.style = style
        self.pins = Int(pins)
        self.bodyWidth = bodyWidth
        self.bodyHeight = bodyHeight
        if let round = body["round"] {
            guard round == .bool(true) else { return nil }
            bodyRound = true
        }
        self.padWidth = padWidth
        self.padHeight = padHeight
        self.padShape = padShape
        for key in ["width", "height", "shape"] { pad.removeValue(forKey: key) }
        padExtras = pad
        self.pitch = pitch
        if let value = keys["drill"] {
            guard case let .number(drill) = value else { return nil }
            self.drill = drill
        }
        if let value = keys["pin1Shape"] {
            guard case let .string(shape) = value, style != .grid else { return nil }
            pin1Shape = shape
        }
        func whole(_ value: JSONValue?) -> Int? {
            guard case let .number(number)? = value, number == number.rounded() else { return nil }
            return Int(number)
        }
        switch style {
        case .dual:
            guard case let .number(span)? = keys["span"] else { return nil }
            self.span = span
            if keys["perSide"] != nil {
                guard let perSide = whole(keys["perSide"]) else { return nil }
                self.perSide = perSide
            }
        case .quad:
            guard case let .array(items)? = keys["sides"], items.count == 4,
                  case let .number(spanX)? = keys["spanX"], case let .number(spanY)? = keys["spanY"] else { return nil }
            let sides = items.compactMap { whole($0) }
            guard sides.count == 4 else { return nil }
            self.sides = sides
            self.spanX = spanX
            self.spanY = spanY
        case .grid:
            guard let rows = whole(keys["rows"]), let columns = whole(keys["columns"]),
                  case let .array(items)? = keys["balls"] else { return nil }
            let balls = items.compactMap { item -> String? in
                if case let .string(ball) = item { return ball }
                return nil
            }
            guard balls.count == items.count else { return nil }
            self.rows = rows
            self.columns = columns
            self.balls = balls
        case .header:
            guard let rows = whole(keys["rows"]), case let .number(firstRowY)? = keys["firstRowY"] else { return nil }
            self.rows = rows
            self.firstRowY = firstRowY
            if let value = keys["rowPitch"] {
                guard case let .number(rowPitch) = value else { return nil }
                self.rowPitch = rowPitch
            }
        }
        let controlled = Self.common.union(Self.own[style] ?? [])
        extras = keys.filter { !controlled.contains($0.key) }
    }

    // MARK: A new one

    /// A first footprint of a style for a pinout: sensible numbers to edit
    /// from the datasheet's drawing (a 1.27 mm SOIC, a 0.5 mm QFN, a 0.5 mm
    /// ball grid, a 2.54 mm header).
    static func starter(style: Style, id: String, pins: Int, balls: [String] = []) -> ParametricFootprintDraft {
        let count = max(pins, 1)
        switch style {
        case .dual:
            let perSide = (count + 1) / 2
            var draft = ParametricFootprintDraft(id: id, description: "\(count)-pin, two rows", style: .dual, pins: count,
                                                 bodyWidth: 3.9, bodyHeight: max(Double(perSide) * 1.27, 1.27),
                                                 padWidth: 1.55, padHeight: 0.6, pitch: 1.27)
            draft.span = 5.4
            if count % 2 == 1 { draft.perSide = perSide }
            return draft
        case .quad:
            let base = count / 4, extra = count % 4
            // Left, bottom, right, top; any remainder goes to the first sides.
            let sides = (0 ..< 4).map { base + ($0 < extra ? 1 : 0) }
            let side = Double(max(sides.max() ?? 1, 1)) * 0.5 + 1.0
            var draft = ParametricFootprintDraft(id: id, description: "\(count)-pin, four sides", style: .quad,
                                                 pins: count, bodyWidth: side, bodyHeight: side,
                                                 padWidth: 0.6, padHeight: 0.25, pitch: 0.5)
            draft.sides = sides
            draft.spanX = side - 0.2
            draft.spanY = side - 0.2
            return draft
        case .grid:
            let size = gridSize(balls: balls, count: count)
            var draft = ParametricFootprintDraft(id: id, description: "\(count)-ball grid", style: .grid, pins: count,
                                                 bodyWidth: Double(size.columns) * 0.5 + 0.3,
                                                 bodyHeight: Double(size.rows) * 0.5 + 0.3,
                                                 padWidth: 0.25, padHeight: 0.25, padShape: "round", pitch: 0.5)
            draft.rows = size.rows
            draft.columns = size.columns
            draft.balls = balls
            return draft
        case .header:
            var draft = ParametricFootprintDraft(id: id, description: "1x\(count) pin header", style: .header,
                                                 pins: count, bodyWidth: Double(count) * 2.54, bodyHeight: 2.54,
                                                 padWidth: 1.8, padHeight: 1.8, padShape: "round", drill: 1.1,
                                                 pin1Shape: "rect", pitch: 2.54)
            draft.rows = 1
            return draft
        }
    }

    init(id: String, description: String, style: Style, pins: Int, bodyWidth: Double, bodyHeight: Double,
         padWidth: Double, padHeight: Double, padShape: String = "rect", drill: Double? = nil,
         pin1Shape: String? = nil, pitch: Double) {
        self.id = id
        self.description = description
        self.style = style
        self.pins = pins
        self.bodyWidth = bodyWidth
        self.bodyHeight = bodyHeight
        self.padWidth = padWidth
        self.padHeight = padHeight
        self.padShape = padShape
        self.drill = drill
        self.pin1Shape = pin1Shape
        self.pitch = pitch
    }

    // MARK: Ball grids

    /// The row letters of a ball grid: A, B, C, … without I, O, Q, S, X, Z.
    static let rowLetters: [Character] = Array("ABCDEFGHJKLMNPRTUVWY")

    /// A ball name's row index (A = 0) and column (1-based); nil when it is
    /// not a one-letter-row ball name.
    static func position(ofBall ball: String) -> (row: Int, column: Int)? {
        guard let letter = ball.first, let row = rowLetters.firstIndex(of: letter),
              let column = Int(ball.dropFirst()), column >= 1 else { return nil }
        return (row, column)
    }

    /// The smallest grid that holds these balls: rows up to the highest row
    /// letter, columns up to the highest column number. With no usable
    /// names, a square that holds `count`.
    static func gridSize(balls: [String], count: Int) -> (rows: Int, columns: Int) {
        let positions = balls.compactMap { position(ofBall: $0) }
        guard !positions.isEmpty else {
            let side = max(Int(Double(max(count, 1)).squareRoot().rounded(.up)), 1)
            return (side, side)
        }
        return ((positions.map(\.row).max() ?? 0) + 1, positions.map(\.column).max() ?? 1)
    }
}

/// The one inline footprint a part names: placed pad by pad, or made from
/// numbers.
enum InlineFootprint: Equatable, Sendable {
    case explicit(CustomFootprintDraft)
    case parametric(ParametricFootprintDraft)

    var id: String {
        switch self {
        case let .explicit(draft): draft.id
        case let .parametric(draft): draft.id
        }
    }

    func encoded() -> JSONValue {
        switch self {
        case let .explicit(draft): draft.encoded()
        case let .parametric(draft): draft.encoded()
        }
    }
}

extension PartDraft {
    /// The part's inline footprint (plan-46-J): the file's entry its
    /// `footprint` names, of any style. Setting one makes it the file's only
    /// inline footprint and the part's footprint; nil removes the
    /// `footprints` key and leaves the part with no footprint.
    /// `customFootprint` is the same for an `explicit` entry only.
    var inlineFootprint: InlineFootprint? {
        get {
            guard let footprint, let entries = footprints else { return nil }
            for entry in entries {
                guard case let .object(keys) = entry, keys["id"] == .string(footprint) else { continue }
                if let explicit = CustomFootprintDraft(json: entry) { return .explicit(explicit) }
                if let parametric = ParametricFootprintDraft(json: entry) { return .parametric(parametric) }
            }
            return nil
        }
        set {
            if let newValue {
                footprints = [newValue.encoded()]
                footprint = newValue.id
            } else {
                if let current = inlineFootprint, footprint == current.id { footprint = nil }
                footprints = nil
            }
        }
    }

    /// The designators of the part's pinout, in pinout order.
    var pinDesignators: [String] {
        (pinouts.first { $0.name == pinout }?.pins ?? []).map(\.designator)
    }

    /// Whether the part's pinout names its pins like balls.
    var isBallPinout: Bool {
        pinouts.first { $0.name == pinout }?.style == .ball
    }
}

/// Orders the library's footprints for a package name (plan-46-J): the ones
/// whose id or description shares the package's words first.
enum FootprintRanking {
    /// Upper-case words: runs of letters and digits.
    static func tokens(_ text: String) -> [String] {
        text.uppercased().split { !$0.isLetter && !$0.isNumber }.map(String.init)
    }

    /// 2 per package word a footprint has exactly, 1 per word one of its
    /// words starts with.
    static func score(_ footprint: Footprint, package: [String]) -> Int {
        let words = Set(tokens(footprint.id) + tokens(footprint.description))
        return package.reduce(0) { total, token in
            if words.contains(token) { return total + 2 }
            return total + (words.contains { $0.hasPrefix(token) } ? 1 : 0)
        }
    }

    /// Best match first, then by id.
    static func rank(_ footprints: [Footprint], package: String) -> [Footprint] {
        let wanted = tokens(package)
        return footprints.map { ($0, score($0, package: wanted)) }
            .sorted { a, b in a.1 != b.1 ? a.1 > b.1 : a.0.id < b.0.id }
            .map(\.0)
    }

    /// The footprints whose id or description holds every word of a search.
    static func search(_ footprints: [Footprint], text: String) -> [Footprint] {
        let wanted = tokens(text)
        guard !wanted.isEmpty else { return footprints }
        return footprints.filter { footprint in
            let haystack = (footprint.id + " " + footprint.description).uppercased()
            return wanted.allSatisfy { haystack.contains($0) }
        }
    }
}

/// The bundled footprints as `footprints.json` writes them (plan-46-J): the
/// parameters "Start from this one" copies.
enum BundledFootprints {
    @MainActor private static var entries: [String: JSONValue]?

    /// A bundled footprint's entry, by id.
    @MainActor
    static func entry(id: String) -> JSONValue? {
        if entries == nil {
            var loaded: [String: JSONValue] = [:]
            if let url = Bundle.main.url(forResource: "footprints", withExtension: "json"),
               let data = try? Data(contentsOf: url),
               case let .object(root)? = try? JSONDecoder().decode(JSONValue.self, from: data),
               case let .array(items)? = root["footprints"] {
                for item in items {
                    if case let .object(keys) = item, case let .string(id)? = keys["id"] { loaded[id] = item }
                }
            }
            entries = loaded
        }
        return entries?[id]
    }
}
