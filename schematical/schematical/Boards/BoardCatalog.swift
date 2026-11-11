import CoreGraphics
import Foundation

/// The standard boards, read from the bundled `boards.json` (format:
/// `docs/boards-format.md`). Adding a board is an edit to that file; no
/// code knows any one standard.
nonisolated final class BoardCatalog: @unchecked Sendable {
    static let fileName = "boards.json"

    /// The bundled catalog. Empty, with `loadError` set, when the file
    /// can't be read (a test checks that it can).
    static let shared: BoardCatalog = {
        guard let url = Bundle.main.url(forResource: "boards", withExtension: "json") else {
            return BoardCatalog(boards: [], loadError: "\(fileName) is missing from the app")
        }
        do {
            return try BoardCatalog(data: Data(contentsOf: url))
        } catch {
            return BoardCatalog(boards: [], loadError: "\(fileName): \(BoardCatalog.describe(error))")
        }
    }()

    let boards: [BoardDefinition]
    let loadError: String?
    private let byID: [String: BoardDefinition]
    private let lock = NSLock()
    private var cache: [CacheKey: CardSpec] = [:]

    private struct CacheKey: Hashable {
        let id: String
        let options: [String: BoardValue]
        let width: Double?
    }

    private struct File: Decodable {
        let boards: [BoardDefinition]
    }

    init(data: Data) throws {
        let file = try JSONDecoder().decode(File.self, from: data)
        var byID: [String: BoardDefinition] = [:]
        for board in file.boards {
            guard board.id != BoardType.custom.rawValue, board.id != BoardType.customCard.rawValue else {
                throw BoardExpression.Failure(description: "board id \"\(board.id)\" is built in")
            }
            guard byID[board.id] == nil else {
                throw BoardExpression.Failure(description: "board id \"\(board.id)\" is used twice")
            }
            byID[board.id] = board
        }
        self.boards = file.boards
        self.byID = byID
        self.loadError = nil
    }

    private init(boards: [BoardDefinition], loadError: String?) {
        self.boards = boards
        self.byID = [:]
        self.loadError = loadError
    }

    func board(_ id: String) -> BoardDefinition? { byID[id] }

    /// The board `id` built with `options`, `width` long (its default
    /// length when nil; clamped to its range). Nil when the board is
    /// unknown or its definition fails (a test builds every board).
    func spec(_ id: String, options: CardOptions, width: Double?) -> CardSpec? {
        guard let board = byID[id] else { return nil }
        let resolved = board.resolved(options)
        let key = CacheKey(id: id, options: resolved, width: width)
        lock.lock()
        if let spec = cache[key] {
            lock.unlock()
            return spec
        }
        lock.unlock()
        guard let spec = try? board.build(resolved, width: width) else { return nil }
        lock.lock()
        if cache.count > 512 { cache.removeAll() }
        cache[key] = spec
        lock.unlock()
        return spec
    }

    /// A decoding or evaluation error as one line.
    static func describe(_ error: Error) -> String {
        switch error {
        case let failure as BoardExpression.Failure:
            return failure.description
        case DecodingError.dataCorrupted(let context):
            return "\(path(context)): \(context.debugDescription)"
        case DecodingError.keyNotFound(let key, let context):
            return "\(path(context)): \(key.stringValue) is missing"
        case DecodingError.typeMismatch(_, let context), DecodingError.valueNotFound(_, let context):
            return "\(path(context)): \(context.debugDescription)"
        default:
            return error.localizedDescription
        }
    }

    private static func path(_ context: DecodingError.Context) -> String {
        context.codingPath.map { $0.intValue.map { "[\($0)]" } ?? ".\($0.stringValue)" }.joined()
    }
}

// MARK: - Definition

/// One standard board as `boards.json` describes it. Every number is a
/// `BoardFormula`, so it can depend on the board's options and variables.
nonisolated struct BoardDefinition: Decodable, Sendable {
    let id: String
    let title: String
    /// The type menu's section ("PC Cards", "Cartridges", …).
    let group: String
    /// Where the numbers come from; not used by the app.
    let notes: String?
    /// "edge" (gold fingers) or "header" (through-hole pads); names the
    /// pins in lists ("Edge.A1", "Header.40").
    let connector: Connector
    /// The sidebar's heading over the options.
    let optionsTitle: String
    let options: [BoardOption]
    let vars: [String: BoardFormula]
    /// The board's size: `height` always; `width` when fixed, or `length`
    /// (default, min, max) when the user sets it.
    let height: BoardFormula
    let width: BoardFormula?
    let length: LengthRange?
    /// Coordinates measured up from the board's bottom edge instead of
    /// down from its top.
    let yUp: Bool
    let outline: [PathItem]
    let pins: [PinGroup]
    let holes: [HoleItem]
    /// The bus's wire order: natural order of the names (nil), the order
    /// the pins are listed in ("listed"), or the names given.
    let pinOrder: PinOrder?

    enum Connector: String, Decodable, Sendable {
        case edge, header

        var title: String { self == .edge ? "Edge" : "Header" }
    }

    struct LengthRange: Decodable, Sendable {
        let `default`: BoardFormula
        let min: BoardFormula
        let max: BoardFormula
    }

    enum PinOrder: Decodable, Sendable {
        case listed
        case names([String])

        init(from decoder: Decoder) throws {
            let container = try decoder.singleValueContainer()
            if let names = try? container.decode([String].self) {
                self = .names(names)
                return
            }
            guard try container.decode(String.self) == "listed" else {
                throw DecodingError.dataCorruptedError(in: container,
                                                       debugDescription: "pinOrder must be \"listed\" or a list of names")
            }
            self = .listed
        }
    }

    private enum Keys: String, CodingKey {
        case id, title, group, notes, connector, optionsTitle, options, vars, height, width, length, yUp, outline, pins,
             holes, pinOrder
    }

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: Keys.self)
        id = try c.decode(String.self, forKey: .id)
        title = try c.decode(String.self, forKey: .title)
        group = try c.decodeIfPresent(String.self, forKey: .group) ?? "Other"
        notes = try c.decodeIfPresent(String.self, forKey: .notes)
        connector = try c.decodeIfPresent(Connector.self, forKey: .connector) ?? .edge
        optionsTitle = try c.decodeIfPresent(String.self, forKey: .optionsTitle) ?? "Card"
        options = try c.decodeIfPresent([BoardOption].self, forKey: .options) ?? []
        vars = try c.decodeIfPresent([String: BoardFormula].self, forKey: .vars) ?? [:]
        height = try c.decode(BoardFormula.self, forKey: .height)
        width = try c.decodeIfPresent(BoardFormula.self, forKey: .width)
        length = try c.decodeIfPresent(LengthRange.self, forKey: .length)
        guard (width == nil) != (length == nil) else {
            throw DecodingError.dataCorruptedError(forKey: .width, in: c,
                                                   debugDescription: "board \(id) needs width or length, not both")
        }
        yUp = try c.decodeIfPresent(Bool.self, forKey: .yUp) ?? false
        outline = try c.decode([PathItem].self, forKey: .outline)
        pins = try c.decode([PinGroup].self, forKey: .pins)
        holes = try c.decodeIfPresent([HoleItem].self, forKey: .holes) ?? []
        pinOrder = try c.decodeIfPresent(PinOrder.self, forKey: .pinOrder)
    }

    /// Each option's value: the one in `options` when it's one of its
    /// choices, else its default.
    func resolved(_ options: CardOptions) -> [String: BoardValue] {
        var values: [String: BoardValue] = [:]
        for option in self.options {
            values[option.id] = options[option.id].flatMap { option.choice(for: $0) }?.value ?? option.default
        }
        return values
    }

    /// Whether `option` applies with these option values (its `when`).
    func shows(_ option: BoardOption, with options: CardOptions) -> Bool {
        guard let when = option.when else { return true }
        let scope = Scope(board: self, options: resolved(options), width: nil)
        return (try? when.flag(scope.lookup)) ?? true
    }

    /// The board built with resolved option values, `width` long.
    func build(_ options: [String: BoardValue], width requested: Double?) throws -> CardSpec {
        let sizing = Scope(board: self, options: options, width: nil)
        let height = try self.height.number(sizing.lookup)
        let length: Double, minimum: Double, maximum: Double
        if let width {
            length = try width.number(sizing.lookup)
            (minimum, maximum) = (length, length)
        } else {
            let range = self.length!
            length = try range.default.number(sizing.lookup)
            minimum = try range.min.number(sizing.lookup)
            maximum = try range.max.number(sizing.lookup)
        }
        let actual = min(max(requested ?? length, minimum), maximum)
        let scope = Scope(board: self, options: options, width: actual, height: height)
        func y(_ value: Double) -> Double { yUp ? height - value : value }

        var outline: [CGPoint] = []
        try PathItem.trace(self.outline, scope: scope, into: &outline)
        outline = outline.map { CGPoint(x: $0.x, y: y($0.y)) }

        var fingers: [EdgeFinger] = [], extra: [EdgeFinger] = []
        for group in pins {
            for (pin, onBus) in try group.pins(scope: scope, flip: y) {
                if onBus { fingers.append(pin) } else { extra.append(pin) }
            }
        }
        var holes: [MountingHole] = []
        for hole in self.holes where try hole.when?.flag(scope.lookup) ?? true {
            holes.append(MountingHole(centre: CGPoint(x: try hole.x.number(scope.lookup), y: y(try hole.y.number(scope.lookup))),
                                      drill: try hole.drill.number(scope.lookup)))
        }
        let order: [String]?
        switch pinOrder {
        case nil: order = nil
        case .listed:
            var seen = Set<String>()
            order = fingers.map(\.name).filter { seen.insert($0).inserted }
        case .names(let names): order = names
        }
        return CardSpec(height: height, length: length, minimumLength: minimum, maximumLength: maximum,
                        outline: outline, fingers: fingers, holes: holes, extraPads: extra, pinOrder: order,
                        connectorName: connector.title)
    }

    /// Names for expressions: loop counters and a pin's `let`s first,
    /// then `width` and `height`, the options, and the board's `vars`
    /// (each worked out once, on first use).
    final class Scope {
        let board: BoardDefinition
        let options: [String: BoardValue]
        let width: Double?
        let height: Double?
        /// The pin being built: its counters, and its `let`s once worked out.
        private var locals: [String: BoardValue] = [:]
        private var lets: [String: BoardFormula] = [:]
        private var values: [String: BoardValue] = [:]
        private var working: Set<String> = []

        func beginPin(_ counters: [String: BoardValue], lets: [String: BoardFormula]) {
            locals = counters
            self.lets = lets
        }

        func endPin() {
            locals = [:]
            lets = [:]
        }

        init(board: BoardDefinition, options: [String: BoardValue], width: Double?, height: Double? = nil) {
            self.board = board
            self.options = options
            self.width = width
            self.height = height
        }

        func lookup(_ name: String) throws -> BoardValue? {
            if let value = locals[name] { return value }
            if let formula = lets[name] {
                guard working.insert("let " + name).inserted else {
                    throw BoardExpression.Failure(description: "\"\(name)\" depends on itself")
                }
                defer { working.remove("let " + name) }
                let value = try formula.evaluate(lookup)
                locals[name] = value
                return value
            }
            if name == "width", let width { return .number(width) }
            if name == "height", let height { return .number(height) }
            if let value = options[name] { return value }
            for option in board.options {
                if let value = options[option.id], let vars = option.choice(for: value)?.vars, let formula = vars[name] {
                    return try formula.evaluate(lookup)
                }
            }
            if let value = values[name] { return value }
            guard let formula = board.vars[name] else {
                if name == "width" || name == "height" {
                    throw BoardExpression.Failure(description: "\(name) can't be used in the board's size")
                }
                return nil
            }
            guard working.insert(name).inserted else {
                throw BoardExpression.Failure(description: "variable \"\(name)\" depends on itself")
            }
            defer { working.remove(name) }
            // A variable doesn't see a pin's counters or lets.
            let saved = (locals, lets)
            (locals, lets) = ([:], [:])
            defer { (locals, lets) = saved }
            let value = try formula.evaluate(lookup)
            values[name] = value
            return value
        }
    }
}

/// A setting of a board (PCI Express lanes, M.2 key, …), stored in the
/// file's `cardOptions` under `id`.
nonisolated struct BoardOption: Decodable, Sendable {
    let id: String
    let title: String
    let style: Style
    let `default`: BoardValue
    /// Whether it changes the pins, so a bus of the board's type stores it.
    let onBus: Bool
    /// Shown only when this holds.
    let when: BoardFormula?
    let help: String?
    let choices: [Choice]

    enum Style: String, Decodable, Sendable {
        case segmented, menu, toggle
    }

    struct Choice: Decodable, Sendable {
        let value: BoardValue
        let title: String
        /// Variables the choice sets.
        let vars: [String: BoardFormula]?

        private enum Keys: String, CodingKey { case value, title, vars }

        init(value: BoardValue, title: String, vars: [String: BoardFormula]? = nil) {
            self.value = value
            self.title = title
            self.vars = vars
        }

        init(from decoder: Decoder) throws {
            let c = try decoder.container(keyedBy: Keys.self)
            value = try c.decode(BoardValue.self, forKey: .value)
            title = try c.decodeIfPresent(String.self, forKey: .title) ?? value.description
            vars = try c.decodeIfPresent([String: BoardFormula].self, forKey: .vars)
        }
    }

    private enum Keys: String, CodingKey { case id, title, style, `default`, onBus, when, help, choices }

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: Keys.self)
        id = try c.decode(String.self, forKey: .id)
        title = try c.decode(String.self, forKey: .title)
        style = try c.decodeIfPresent(Style.self, forKey: .style) ?? .segmented
        onBus = try c.decodeIfPresent(Bool.self, forKey: .onBus) ?? true
        when = try c.decodeIfPresent(BoardFormula.self, forKey: .when)
        help = try c.decodeIfPresent(String.self, forKey: .help)
        if style == .toggle {
            choices = try c.decodeIfPresent([Choice].self, forKey: .choices)
                ?? [Choice(value: .bool(false), title: "Off"), Choice(value: .bool(true), title: "On")]
        } else {
            choices = try c.decode([Choice].self, forKey: .choices)
        }
        `default` = try c.decodeIfPresent(BoardValue.self, forKey: .default) ?? choices[0].value
        guard choices.contains(where: { $0.value == `default` }) else {
            throw DecodingError.dataCorruptedError(forKey: .default, in: c,
                                                   debugDescription: "option \(id)'s default isn't one of its choices")
        }
    }

    func choice(for value: BoardValue) -> Choice? { choices.first { $0.value == value } }

    /// The choices as a message lists them: 1, 4, 8 or 16.
    var choiceList: String {
        let names = choices.map(\.value.quoted)
        return names.count == 1 ? names[0] : names.dropLast().joined(separator: ", ") + " or " + names.last!
    }
}

// MARK: - Outline

/// One step of an outline, clockwise from the top-left: a point
/// `[x, y]`, an arc, or a group of steps used only `when` a condition
/// holds.
nonisolated enum PathItem: Decodable, Sendable {
    case point(BoardFormula, BoardFormula)
    /// Points round `centre` from `from` to `to` degrees (y down, so 90°
    /// points down; y up, 90° points up), `steps` segments; `skipFirst`
    /// leaves out its first point.
    case arc(centre: (BoardFormula, BoardFormula), radius: BoardFormula, from: BoardFormula, to: BoardFormula,
             steps: Int, skipFirst: Bool)
    case when(BoardFormula, [PathItem])

    private enum Keys: String, CodingKey { case arc, radius, from, to, steps, skipFirst, when, then }

    init(from decoder: Decoder) throws {
        if var point = try? decoder.unkeyedContainer() {
            let x = try point.decode(BoardFormula.self), y = try point.decode(BoardFormula.self)
            guard point.isAtEnd else {
                throw DecodingError.dataCorruptedError(in: point, debugDescription: "a point is [x, y]")
            }
            self = .point(x, y)
            return
        }
        let c = try decoder.container(keyedBy: Keys.self)
        if let when = try c.decodeIfPresent(BoardFormula.self, forKey: .when) {
            self = .when(when, try c.decode([PathItem].self, forKey: .then))
            return
        }
        var centre = try c.nestedUnkeyedContainer(forKey: .arc)
        self = .arc(centre: (try centre.decode(BoardFormula.self), try centre.decode(BoardFormula.self)),
                    radius: try c.decode(BoardFormula.self, forKey: .radius),
                    from: try c.decode(BoardFormula.self, forKey: .from), to: try c.decode(BoardFormula.self, forKey: .to),
                    steps: try c.decodeIfPresent(Int.self, forKey: .steps) ?? 6,
                    skipFirst: try c.decodeIfPresent(Bool.self, forKey: .skipFirst) ?? false)
    }

    static func trace(_ items: [PathItem], scope: BoardDefinition.Scope, into points: inout [CGPoint]) throws {
        for item in items {
            switch item {
            case .point(let x, let y):
                points.append(CGPoint(x: try x.number(scope.lookup), y: try y.number(scope.lookup)))
            case .arc(let centre, let radius, let from, let to, let steps, let skipFirst):
                let cx = try centre.0.number(scope.lookup), cy = try centre.1.number(scope.lookup)
                let r = try radius.number(scope.lookup)
                let start = try from.number(scope.lookup), end = try to.number(scope.lookup)
                for step in (skipFirst ? 1 : 0)...steps {
                    let angle = (start + (end - start) * Double(step) / Double(steps)) * .pi / 180
                    points.append(CGPoint(x: cx + r * cos(angle), y: cy + r * sin(angle)))
                }
            case .when(let condition, let items):
                if try condition.flag(scope.lookup) { try trace(items, scope: scope, into: &points) }
            }
        }
    }
}

// MARK: - Pins

/// A run of pins: one per value of `n` (`each`, or `from` … `to` by
/// `step`; `i` counts them from 0), kept `where` it holds, each named by
/// `name` ("A{n}": braces hold expressions) and shaped as a `finger`
/// (an edge contact on one side) or a `pad` (a through-hole header pad).
/// `let` works out per-pin values first. `onBus: false` pins are on the
/// board only (an Arduino shield's ICSP header).
nonisolated struct PinGroup: Decodable, Sendable {
    let when: BoardFormula?
    let each: [BoardValue]?
    let from: BoardFormula?
    let to: BoardFormula?
    let step: BoardFormula?
    let filter: BoardFormula?
    let name: [NamePart]
    let lets: [String: BoardFormula]
    let finger: Finger?
    let pad: Pad?
    let onBus: Bool

    enum NamePart: Sendable {
        case text(String)
        case expression(BoardExpression)
    }

    struct Finger: Decodable, Sendable {
        /// The finger's centre along the edge, its width, and its top and
        /// bottom (as drawn, the edge at the bottom).
        let x: BoardFormula
        let width: BoardFormula
        let top: BoardFormula
        let bottom: BoardFormula
        let side: BoardSide
    }

    struct Pad: Decodable, Sendable {
        let x: BoardFormula
        let y: BoardFormula
        let size: BoardFormula?
        let drill: BoardFormula?
        let square: BoardFormula?
    }

    private enum Keys: String, CodingKey {
        case when, each, from, to, step, `where`, name, `let`, finger, pad, onBus
    }

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: Keys.self)
        when = try c.decodeIfPresent(BoardFormula.self, forKey: .when)
        each = try c.decodeIfPresent([BoardValue].self, forKey: .each)
        from = try c.decodeIfPresent(BoardFormula.self, forKey: .from)
        to = try c.decodeIfPresent(BoardFormula.self, forKey: .to)
        step = try c.decodeIfPresent(BoardFormula.self, forKey: .step)
        filter = try c.decodeIfPresent(BoardFormula.self, forKey: .where)
        guard (each == nil) != (from == nil && to == nil) else {
            throw DecodingError.dataCorruptedError(forKey: .each, in: c, debugDescription: "pins need each, or from and to")
        }
        let template = try c.decodeIfPresent(String.self, forKey: .name) ?? "{n}"
        do {
            name = try Self.parse(template)
        } catch let failure as BoardExpression.Failure {
            throw DecodingError.dataCorruptedError(forKey: .name, in: c, debugDescription: failure.description)
        }
        lets = try c.decodeIfPresent([String: BoardFormula].self, forKey: .let) ?? [:]
        finger = try c.decodeIfPresent(Finger.self, forKey: .finger)
        pad = try c.decodeIfPresent(Pad.self, forKey: .pad)
        guard (finger == nil) != (pad == nil) else {
            throw DecodingError.dataCorruptedError(forKey: .finger, in: c, debugDescription: "pins need a finger or a pad")
        }
        onBus = try c.decodeIfPresent(Bool.self, forKey: .onBus) ?? true
    }

    private static func parse(_ template: String) throws -> [NamePart] {
        var parts: [NamePart] = []
        var rest = Substring(template)
        while let open = rest.firstIndex(of: "{") {
            if open > rest.startIndex { parts.append(.text(String(rest[..<open]))) }
            guard let close = rest[open...].firstIndex(of: "}") else {
                throw BoardExpression.Failure(description: "unclosed { in pin name \"\(template)\"")
            }
            parts.append(.expression(try BoardExpression.parse(String(rest[rest.index(after: open)..<close]))))
            rest = rest[rest.index(after: close)...]
        }
        if !rest.isEmpty { parts.append(.text(String(rest))) }
        return parts
    }

    /// The group's pins, with whether each is on the bus. `flip` turns a
    /// board y into a y down from the top.
    func pins(scope: BoardDefinition.Scope, flip: (Double) -> Double) throws -> [(EdgeFinger, Bool)] {
        let lookup = scope.lookup
        if let when, try !when.flag(lookup) { return [] }
        let values: [BoardValue]
        if let each {
            values = each
        } else {
            let first = try from!.number(lookup), last = try to!.number(lookup), by = try step?.number(lookup) ?? 1
            guard by != 0 else { throw BoardExpression.Failure(description: "a pin step of 0") }
            values = Array(stride(from: first, through: last, by: by)).map { .number($0) }
        }
        defer { scope.endPin() }
        var result: [(EdgeFinger, Bool)] = []
        for (index, value) in values.enumerated() {
            scope.beginPin(["n": value, "i": .number(Double(index))], lets: lets)
            if let filter, try !filter.flag(lookup) { continue }
            let name = try self.name.map { part -> String in
                switch part {
                case .text(let text): text
                case .expression(let expression): try expression.evaluate(lookup).description
                }
            }.joined()
            if let finger {
                let x = try finger.x.number(lookup), width = try finger.width.number(lookup)
                let top = flip(try finger.top.number(lookup)), bottom = flip(try finger.bottom.number(lookup))
                let rect = CGRect(x: x - width / 2, y: min(top, bottom), width: width, height: abs(bottom - top))
                result.append((EdgeFinger(name: name, rect: rect, side: finger.side), onBus))
            } else if let pad {
                let centre = CGPoint(x: try pad.x.number(lookup), y: flip(try pad.y.number(lookup)))
                let pin = EdgeFinger.header(name, centre: centre, size: try pad.size?.number(lookup) ?? 1.8,
                                            drill: try pad.drill?.number(lookup) ?? 1.1,
                                            round: !(try pad.square?.flag(lookup) ?? false))
                result.append((pin, onBus))
            }
        }
        return result
    }
}

/// A non-plated mounting hole, used `when` a condition holds.
nonisolated struct HoleItem: Decodable, Sendable {
    let x: BoardFormula
    let y: BoardFormula
    let drill: BoardFormula
    let when: BoardFormula?
}

/// A finger's side, as `boards.json` names it.
extension BoardSide: Decodable {}
