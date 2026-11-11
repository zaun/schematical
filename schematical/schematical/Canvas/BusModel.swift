import CoreGraphics
import Foundation

// MARK: - Bus values

/// One wire carried by a bus. Ports and wire ends refer to it by `id`; the
/// schematic shows `name`.
struct BusWire: Identifiable, Hashable {
    let id: UUID
    /// Display name, 1–4 characters, unique in its bus (case-insensitive).
    var name: String

    init(id: UUID = UUID(), name: String) {
        self.id = id
        self.name = name
    }
}

/// One end of a bus line.
enum BusEnd: String, Hashable, CaseIterable {
    case a
    case b

    var other: BusEnd { self == .a ? .b : .a }
}

/// A named end of a named line.
struct BusLineEnd: Hashable {
    let line: UUID
    let end: BusEnd
}

/// One straight piece of a bus. Horizontal or vertical, on the grid,
/// length > 0. Nothing lies between its two ends.
struct BusLine: Identifiable, Hashable {
    let id: UUID
    var a: CGPoint
    var b: CGPoint

    init(id: UUID = UUID(), a: CGPoint, b: CGPoint) {
        self.id = id
        self.a = a
        self.b = b
    }

    func point(_ end: BusEnd) -> CGPoint {
        end == .a ? a : b
    }

    mutating func set(_ end: BusEnd, to point: CGPoint) {
        if end == .a { a = point } else { b = point }
    }

    var isHorizontal: Bool { a.y == b.y && a.x != b.x }
    var isVertical: Bool { a.x == b.x && a.y != b.y }
    var isStraight: Bool { isHorizontal || isVertical }

    /// Whether both ends are on the schematic grid.
    var isOnGrid: Bool { SchematicGrid.snap(a) == a && SchematicGrid.snap(b) == b }

    /// Lowest and highest coordinate along the line (x if horizontal, else y).
    var alongRange: ClosedRange<CGFloat> {
        isHorizontal ? min(a.x, b.x)...max(a.x, b.x) : min(a.y, b.y)...max(a.y, b.y)
    }

    /// The coordinate across the line (y if horizontal, else x).
    var across: CGFloat { isHorizontal ? a.y : a.x }

    /// Whether `point` lies on the line, its ends included.
    func contains(_ point: CGPoint) -> Bool {
        if a == b { return point == a }
        if isHorizontal { return point.y == a.y && alongRange.contains(point.x) }
        if isVertical { return point.x == a.x && alongRange.contains(point.y) }
        return false
    }

    /// Whether `point` lies on the line strictly between its ends.
    func containsInside(_ point: CGPoint) -> Bool {
        contains(point) && point != a && point != b
    }

    /// Whether the two lines are at a right angle.
    func isPerpendicular(to other: BusLine) -> Bool {
        (isHorizontal && other.isVertical) || (isVertical && other.isHorizontal)
    }

    /// Which way `b` lies from `a`: +1 or −1 along the line, 0 for length 0.
    var direction: CGFloat {
        let d = isHorizontal ? b.x - a.x : b.y - a.y
        return d > 0 ? 1 : (d < 0 ? -1 : 0)
    }
}

/// Where two lines of one bus meet. Joints are never stored: they are worked
/// out from where line ends touch (`Bus.joints`).
enum BusJoint: Hashable {
    /// An L: exactly two line ends meet at a right angle.
    case corner(BusLineEnd, BusLineEnd, at: CGPoint)
    /// A T: one line's end lies strictly inside another line, at a right angle.
    case tee(end: BusLineEnd, on: UUID, at: CGPoint)

    /// A corner with its two ends in a fixed order, so equal corners compare equal.
    static func makeCorner(_ first: BusLineEnd, _ second: BusLineEnd, at point: CGPoint) -> BusJoint {
        first.line.uuidString <= second.line.uuidString
            ? .corner(first, second, at: point)
            : .corner(second, first, at: point)
    }

    var point: CGPoint {
        switch self {
        case let .corner(_, _, at): at
        case let .tee(_, _, at): at
        }
    }

    /// The joint without its point, so joints compare across a drag.
    var shape: BusJoint {
        switch self {
        case let .corner(first, second, _): .corner(first, second, at: .zero)
        case let .tee(end, on, _): .tee(end: end, on: on, at: .zero)
        }
    }

    /// The line ends that meet here (one for a T).
    var ends: [BusLineEnd] {
        switch self {
        case let .corner(first, second, _): [first, second]
        case let .tee(end, _, _): [end]
        }
    }

    /// The joint with `old` renamed to `new`, its ends mapped by `end`: for
    /// when two lines are joined end to end into one.
    func replacingLine(_ old: UUID, with new: UUID, end: (BusEnd) -> BusEnd) -> BusJoint {
        func map(_ e: BusLineEnd) -> BusLineEnd {
            e.line == old ? BusLineEnd(line: new, end: end(e.end)) : e
        }
        switch self {
        case let .corner(first, second, at): return .makeCorner(map(first), map(second), at: at)
        case let .tee(e, on, at): return .tee(end: map(e), on: on == old ? new : on, at: at)
        }
    }

    func involves(_ line: UUID) -> Bool {
        switch self {
        case let .corner(first, second, _): first.line == line || second.line == line
        case let .tee(end, on, _): end.line == line || on == line
        }
    }
}

/// Why a bus is not valid (`Bus.problem`).
enum BusProblem: Equatable {
    case noLines(bus: UUID)
    case notStraight(line: UUID)
    case overlap(UUID, UUID)
    case badJoint(bus: UUID, at: CGPoint)
    case notOnePiece(bus: UUID)
}

/// A thick line that carries 2–999 named wires. It is made of straight lines
/// that meet only at an L or a T.
/// What a bus carries: free wires (`.custom`), or the edge of one of the
/// PCB's card types, whose pins become its wires (`pinNames`), count and
/// names both, until the type is Custom again. The type's settings are the
/// board's (`CardOptions`, `CustomCard`).
struct BusCard: Hashable {
    var type: BoardType = .custom
    var cardOptions = CardOptions()
    var customCard = CustomCard()

    var isCustom: Bool { type == .custom }

    /// The card's pin names, each once, in natural order ("A1", "A2", …,
    /// "A10", "B1"; numbers before letters). Nil for Custom.
    var pinNames: [String]? {
        guard type != .custom else { return nil }
        var board = BoardSettings()
        board.type = type
        board.cardOptions = cardOptions
        board.customCard = customCard
        return board.pinNames
    }
}

struct Bus: Identifiable, Hashable {
    let id: UUID
    /// Shown in the bus editor; "BUS 1", "BUS 2", … when drawn. Unique
    /// (ignoring case); empty only until the document names it.
    var name = ""
    /// Custom, or a card type whose pins set the wires.
    var card = BusCard()
    /// ≥ 1 line, all joined into one piece by T and L joints.
    var lines: [BusLine]
    /// The bus's wires in list order. 2…999 of them. Every line carries all
    /// of them.
    var wires: [BusWire]
    /// Ports on the bus's lines (plan-13-B).
    var ports: [BusPort]

    static let wireCountRange = 2...999
    static let defaultWireCount = 8
    static let nameLengthRange = 1...4
    /// Wider than a wire's 1.5, so a bus stands out.
    static let lineWidth: CGFloat = 4

    init(id: UUID = UUID(), name: String = "", card: BusCard = BusCard(), lines: [BusLine], wires: [BusWire],
         ports: [BusPort] = []) {
        self.id = id
        self.name = name
        self.card = card
        self.lines = lines
        self.wires = wires
        self.ports = ports
    }

    /// "BUS <n>" for the lowest n from 1 that no name in `taken` has
    /// (ignoring case).
    static func defaultBusName(taken: [String]) -> String {
        let used = Set(taken.map { $0.lowercased() })
        var n = 1
        while used.contains("bus \(n)") { n += 1 }
        return "BUS \(n)"
    }

    /// `B<index>`, or else the first free `B<n>` (case-insensitive).
    static func defaultName(for index: Int, taken: Set<String>) -> String {
        let used = Set(taken.map { $0.lowercased() })
        let name = "B\(index)"
        if !used.contains(name.lowercased()) { return name }
        var n = 0
        while used.contains("b\(n)") { n += 1 }
        return "B\(n)"
    }

    /// `count` wires named `B0`, `B1`, ….
    static func defaultWires(count: Int) -> [BusWire] {
        var wires: [BusWire] = []
        for index in 0..<count {
            wires.append(BusWire(name: defaultName(for: index, taken: Set(wires.map(\.name)))))
        }
        return wires
    }

    func line(_ id: UUID) -> BusLine? {
        lines.first { $0.id == id }
    }

    // MARK: Joints

    /// Every joint, and every contact point that is not a T or an L.
    func contacts() -> (joints: Set<BusJoint>, bad: [CGPoint]) {
        var points: [CGPoint] = []
        for line in lines {
            for point in [line.a, line.b] where !points.contains(point) {
                points.append(point)
            }
        }

        var joints = Set<BusJoint>()
        var bad: [CGPoint] = []
        for point in points {
            var ends: [(BusLineEnd, BusLine)] = []
            for line in lines {
                for end in BusEnd.allCases where line.point(end) == point {
                    ends.append((BusLineEnd(line: line.id, end: end), line))
                }
            }
            let inside = lines.filter { $0.containsInside(point) }

            if ends.count == 1, inside.isEmpty {
                continue
            }
            if ends.count == 2, inside.isEmpty, ends[0].1.id != ends[1].1.id,
               ends[0].1.isPerpendicular(to: ends[1].1) {
                joints.insert(.makeCorner(ends[0].0, ends[1].0, at: point))
            } else if ends.count == 1, inside.count == 1, ends[0].1.isPerpendicular(to: inside[0]) {
                joints.insert(.tee(end: ends[0].0, on: inside[0].id, at: point))
            } else {
                bad.append(point)
            }
        }
        return (joints, bad)
    }

    var joints: Set<BusJoint> { contacts().joints }

    /// The joints shapes (joints without their points).
    var jointShapes: Set<BusJoint> { Set(joints.map(\.shape)) }

    /// The joints a line takes part in.
    func joints(of lineID: UUID) -> [BusJoint] {
        joints.filter { $0.involves(lineID) }
    }

    /// The joint a line end is in, or nil for a free end.
    func joint(at end: BusLineEnd) -> BusJoint? {
        joints.first { $0.ends.contains(end) }
    }

    // MARK: Validity

    /// The first reason the bus is not valid, or nil.
    var problem: BusProblem? {
        if lines.isEmpty { return .noLines(bus: id) }
        for line in lines where !line.isStraight || !line.isOnGrid {
            return .notStraight(line: line.id)
        }
        for i in lines.indices {
            for j in lines.indices where j > i {
                let first = lines[i], second = lines[j]
                guard first.isHorizontal == second.isHorizontal, first.across == second.across else { continue }
                let lo = max(first.alongRange.lowerBound, second.alongRange.lowerBound)
                let hi = min(first.alongRange.upperBound, second.alongRange.upperBound)
                if lo < hi { return .overlap(first.id, second.id) }
            }
        }
        if let point = contacts().bad.first { return .badJoint(bus: id, at: point) }
        if pieces().count != 1 { return .notOnePiece(bus: id) }
        return nil
    }

    var isValid: Bool { problem == nil }

    /// Lines grouped by the joints between them, in line order.
    func pieces() -> [[BusLine]] {
        var parent = Dictionary(uniqueKeysWithValues: lines.map { ($0.id, $0.id) })
        func root(_ id: UUID) -> UUID {
            var id = id
            while let next = parent[id], next != id { id = next }
            return id
        }
        for joint in joints {
            let ids: [UUID]
            switch joint {
            case let .corner(first, second, _): ids = [first.line, second.line]
            case let .tee(end, on, _): ids = [end.line, on]
            }
            parent[root(ids[1])] = root(ids[0])
        }
        var order: [UUID] = []
        var groups: [UUID: [BusLine]] = [:]
        for line in lines {
            let r = root(line.id)
            if groups[r] == nil { order.append(r) }
            groups[r, default: []].append(line)
        }
        return order.map { groups[$0] ?? [] }
    }

    /// The first place a line end of one bus touches a line of another:
    /// (that bus, the other bus, the point). Nil when all buses are apart.
    static func firstTouch(_ buses: [Bus]) -> (UUID, UUID, CGPoint)? {
        for first in buses {
            for second in buses where second.id != first.id {
                for line in first.lines {
                    for point in [line.a, line.b] where second.lines.contains(where: { $0.contains(point) }) {
                        return (first.id, second.id, point)
                    }
                }
            }
        }
        return nil
    }
}

// MARK: - Ports

/// A box on a bus line with one pin per ticked bus wire. Wires connect to
/// its pins; every pin for the same bus wire, on any port of the bus, is one
/// net.
struct BusPort: Identifiable, Hashable {
    let id: UUID
    /// The bus line it sits on.
    let line: UUID
    /// Along-axis coordinate of its first pin slot: x on a horizontal line,
    /// y on a vertical one. On the grid.
    var start: CGFloat
    /// Which side its pins come out: top/bottom on a horizontal line,
    /// left/right on a vertical one.
    var side: ChipLayout.Side
    /// The ticked bus wire ids. Pin order is always the bus's wire order.
    var wireIDs: Set<UUID>

    init(id: UUID = UUID(), line: UUID, start: CGFloat, side: ChipLayout.Side, wireIDs: Set<UUID> = []) {
        self.id = id
        self.line = line
        self.start = start
        self.side = side
        self.wireIDs = wireIDs
    }

    /// How deep the body is, out from the bus line.
    static let depth: CGFloat = 40
    static let pitch = ChipLayout.pitch
    static let stub = ChipLayout.stubLength

    /// The default side for a line: bottom on a horizontal line, right on a
    /// vertical one.
    static func defaultSide(for line: BusLine) -> ChipLayout.Side {
        line.isHorizontal ? .bottom : .right
    }

    /// The sides a port on `line` may use.
    static func sides(for line: BusLine) -> [ChipLayout.Side] {
        line.isHorizontal ? [.top, .bottom] : [.left, .right]
    }

    /// +1 when pins point to growing y (bottom) or x (right), else −1.
    var outward: CGFloat {
        side == .bottom || side == .right ? 1 : -1
    }

    /// Along-axis range a port with `slots` slots takes from `start`.
    static func extent(start: CGFloat, slots: Int) -> ClosedRange<CGFloat> {
        let lo = start - pitch / 2
        return lo...(lo + CGFloat(max(slots, 1)) * pitch)
    }
}

extension Bus {
    func port(_ id: UUID) -> BusPort? {
        ports.first { $0.id == id }
    }

    /// The port's pins, in bus wire order.
    func pinWires(of port: BusPort) -> [BusWire] {
        wires.filter { port.wireIDs.contains($0.id) }
    }

    /// Slots a port takes along its line: one per pin, at least one.
    func slots(of port: BusPort) -> Int {
        max(port.wireIDs.count, 1)
    }

    func extent(of port: BusPort) -> ClosedRange<CGFloat> {
        BusPort.extent(start: port.start, slots: slots(of: port))
    }

    /// A point `along` the port's line and `out` from it toward the pins.
    func point(on port: BusPort, along: CGFloat, out: CGFloat) -> CGPoint? {
        guard let line = line(port.line) else { return nil }
        let across = line.across + out * port.outward
        return line.isHorizontal ? CGPoint(x: along, y: across) : CGPoint(x: across, y: along)
    }

    /// The port's body: its extent along the line, `BusPort.depth` deep on
    /// the pin side. The bus line runs along its inner edge.
    func body(of port: BusPort) -> CGRect? {
        let extent = extent(of: port)
        guard let a = point(on: port, along: extent.lowerBound, out: 0),
              let b = point(on: port, along: extent.upperBound, out: BusPort.depth) else { return nil }
        return CGRect(x: min(a.x, b.x), y: min(a.y, b.y), width: abs(b.x - a.x), height: abs(b.y - a.y))
    }

    /// Along-axis position of pin `index` (0-based, in bus order).
    func pinAlong(_ port: BusPort, index: Int) -> CGFloat {
        port.start + CGFloat(index) * BusPort.pitch
    }

    /// Where a wire meets the port's pin for `wireID`: the end of its stub,
    /// 50 out from the line. Nil when the wire isn't ticked on the port.
    func pinTip(_ port: BusPort, wire wireID: UUID) -> CGPoint? {
        guard let index = pinWires(of: port).firstIndex(where: { $0.id == wireID }) else { return nil }
        return point(on: port, along: pinAlong(port, index: index), out: BusPort.depth + BusPort.stub)
    }

    /// Along-axis positions of the Ts on a line's middle (where another line
    /// of the bus ends on it).
    func teeAlongs(on lineID: UUID) -> [CGFloat] {
        guard let line = line(lineID) else { return [] }
        return joints.compactMap { joint in
            guard case let .tee(_, on, at) = joint, on == lineID else { return nil }
            return line.isHorizontal ? at.x : at.y
        }
    }

    /// The free intervals of a port's line, along its axis: the line's ends,
    /// minus the extents of the other ports on that line and side, split at
    /// every T on its middle. Ports on opposite sides never block each
    /// other. Intervals may touch an extent or a T.
    func freeIntervals(around port: BusPort) -> [ClosedRange<CGFloat>] {
        guard let line = line(port.line) else { return [] }
        var cuts: [ClosedRange<CGFloat>] = ports
            .filter { $0.id != port.id && $0.line == port.line && $0.side == port.side }
            .map { extent(of: $0) }
        cuts += teeAlongs(on: line.id).map { $0...$0 }

        var free: [ClosedRange<CGFloat>] = [line.alongRange]
        for cut in cuts {
            free = free.flatMap { interval -> [ClosedRange<CGFloat>] in
                guard cut.lowerBound <= interval.upperBound, cut.upperBound >= interval.lowerBound else {
                    return [interval]
                }
                var pieces: [ClosedRange<CGFloat>] = []
                if interval.lowerBound <= cut.lowerBound { pieces.append(interval.lowerBound...cut.lowerBound) }
                if cut.upperBound <= interval.upperBound { pieces.append(cut.upperBound...interval.upperBound) }
                return pieces
            }
        }
        return free
    }

    /// Where a port can slide: the starts that keep its extent inside the
    /// free interval it is in now (between the line's ends, the neighbouring
    /// ports and the Ts). Nil if it doesn't fit where it is.
    func slideRange(of port: BusPort) -> ClosedRange<CGFloat>? {
        let current = extent(of: port)
        let length = current.upperBound - current.lowerBound
        guard let interval = freeIntervals(around: port).first(where: {
            $0.lowerBound <= current.lowerBound && current.upperBound <= $0.upperBound
        }) else { return nil }
        let lo = interval.lowerBound + BusPort.pitch / 2
        let hi = interval.upperBound - length + BusPort.pitch / 2
        return lo <= hi ? lo...hi : nil
    }

    /// Whether every port still fits where it is: its line exists, its side
    /// matches the line, and the fit rule keeps its start.
    var portsFit: Bool {
        ports.allSatisfy { port in
            guard let line = line(port.line), BusPort.sides(for: line).contains(port.side) else { return false }
            return fit(port: port, slots: slots(of: port), preferredStart: port.start) == port.start
        }
    }

    /// The fit rule. Where a port with `slots` slots can start on its line,
    /// given that it would like `preferredStart`: its extent must lie in one
    /// free interval of the line (the line's ends, minus the other ports'
    /// extents, split at every T). It keeps `preferredStart` if it fits there;
    /// else it moves left (up) just enough to end at that interval's end;
    /// else nil.
    func fit(port: BusPort, slots: Int, preferredStart: CGFloat) -> CGFloat? {
        let length = CGFloat(max(slots, 1)) * BusPort.pitch
        let wanted = BusPort.extent(start: preferredStart, slots: slots)
        for interval in freeIntervals(around: port) where interval.contains(preferredStart) {
            if interval.lowerBound <= wanted.lowerBound, wanted.upperBound <= interval.upperBound {
                return preferredStart
            }
            if wanted.upperBound > interval.upperBound {
                let start = interval.upperBound - length + BusPort.pitch / 2
                if start - BusPort.pitch / 2 >= interval.lowerBound {
                    return start
                }
            }
        }
        return nil
    }
}

// MARK: - Dragging

/// Bus drags that keep every line straight and every joint in place. Each
/// function takes the bus as it was when the drag started and returns the new
/// lines, or nil when the step means nothing. `SchematicDocument.setBusLines`
/// then refuses any result whose joints changed.
enum BusEdit {
    /// Moves a line sideways by the part of `offset` across it (snapped). An L
    /// partner's end and a T line ending on it move with it, so those lines
    /// stretch. A T end of this line slides along the line it sits on.
    static func moveLine(_ bus: Bus, line: UUID, by offset: CGSize) -> [BusLine]? {
        guard let moved = bus.line(line) else { return nil }
        let across = SchematicGrid.snap(moved.isHorizontal ? offset.height : offset.width)
        return shift(bus.lines, joints: bus.joints, line: line, by: across)
    }

    /// Drags a line end to `point`. A free end moves along its line (the line
    /// gets longer or shorter). A corner goes to `point`, moving both lines of
    /// the L sideways. A T end slides along the line it sits on.
    static func moveEnd(_ bus: Bus, line: UUID, end: BusEnd, to point: CGPoint) -> [BusLine]? {
        guard let dragged = bus.line(line) else { return nil }
        let target = SchematicGrid.snap(point)
        let here = dragged.point(end)
        let joints = bus.joints
        let ref = BusLineEnd(line: line, end: end)

        switch bus.joint(at: ref) {
        case nil:
            var lines = bus.lines
            guard let index = lines.firstIndex(where: { $0.id == line }) else { return nil }
            let moved = dragged.isHorizontal
                ? CGPoint(x: target.x, y: here.y)
                : CGPoint(x: here.x, y: target.y)
            lines[index].set(end, to: moved)
            return lines

        case let .corner(first, second, _)?:
            let other = first == ref ? second : first
            guard let partner = bus.line(other.line) else { return nil }
            let mine = dragged.isHorizontal ? target.y - here.y : target.x - here.x
            let theirs = partner.isHorizontal ? target.y - here.y : target.x - here.x
            guard let step = shift(bus.lines, joints: joints, line: line, by: mine) else { return nil }
            return shift(step, joints: joints, line: other.line, by: theirs)

        case .tee?:
            let along = dragged.isHorizontal ? target.y - here.y : target.x - here.x
            return shift(bus.lines, joints: joints, line: line, by: along)
        }
    }

    /// Moves `line` across itself by `amount`; the lines joined to it keep up.
    /// `joints` are the drag-start joints, so they stay meaningful between steps.
    private static func shift(_ lines: [BusLine], joints: Set<BusJoint>, line: UUID, by amount: CGFloat) -> [BusLine]? {
        var lines = lines
        guard let index = lines.firstIndex(where: { $0.id == line }) else { return nil }
        guard amount != 0 else { return lines }
        let horizontal = lines[index].isHorizontal
        func moved(_ point: CGPoint) -> CGPoint {
            horizontal ? CGPoint(x: point.x, y: point.y + amount) : CGPoint(x: point.x + amount, y: point.y)
        }
        lines[index].a = moved(lines[index].a)
        lines[index].b = moved(lines[index].b)

        for joint in joints where joint.involves(line) {
            switch joint {
            case let .corner(first, second, _):
                let mine = first.line == line ? first : second
                let other = first.line == line ? second : first
                guard let o = lines.firstIndex(where: { $0.id == other.line }) else { return nil }
                lines[o].set(other.end, to: lines[index].point(mine.end))
            case let .tee(end, on, _):
                // This line's own T end slides along `on`; nothing else moves.
                guard on == line, let o = lines.firstIndex(where: { $0.id == end.line }) else { continue }
                lines[o].set(end.end, to: moved(lines[o].point(end.end)))
            }
        }
        return lines
    }
}

// MARK: - Joining (the Bus tool)

/// Why the Bus tool refused a drag.
enum BusJoinRefusal: Error, Equatable {
    case samePoint
    case badJoint
    case tooManyWires

    /// The toolbar message, or nil for a drag that didn't go anywhere.
    var message: String? {
        switch self {
        case .samePoint: nil
        case .badJoint: "A bus line can only join another at a T or an L."
        case .tooManyWires: "Joined, these buses would carry more than \(Bus.wireCountRange.upperBound) wires."
        }
    }
}

/// What a Bus tool drag would make.
struct BusDraft {
    /// Index in the document's buses of the bus it grows, or nil for a new bus.
    let busIndex: Int?
    /// The bus after the drag.
    let bus: Bus
    /// The route as drawn, P to Q (two or more points), for the preview.
    let path: [CGPoint]
    /// Index of a second bus the drag joined into `bus`, which goes away.
    var absorbedIndex: Int? = nil
    /// The absorbed bus's wire ids that became another id in `bus` (wires
    /// with the same name, ignoring case). Wires not listed keep their id.
    var wireMap: [UUID: UUID] = [:]
}

enum BusJoin {
    /// What a drag end touches.
    enum Touch {
        case nothing
        /// Strictly inside one line, with nothing else there.
        case middle(bus: Int, line: UUID)
        /// A line end with nothing else there.
        case freeEnd(bus: Int, BusLineEnd)
        /// An L: two line ends of one bus, with nothing else there.
        case corner(bus: Int, BusLineEnd, BusLineEnd)
        /// A T, a crossing, or anything else.
        case other

        var busIndex: Int? {
            switch self {
            case let .middle(bus, _): bus
            case let .freeEnd(bus, _): bus
            case let .corner(bus, _, _): bus
            default: nil
            }
        }
    }

    static func touch(_ point: CGPoint, in buses: [Bus]) -> Touch {
        var hits: [Touch] = []
        for (index, bus) in buses.enumerated() {
            for line in bus.lines {
                for end in BusEnd.allCases where line.point(end) == point {
                    hits.append(.freeEnd(bus: index, BusLineEnd(line: line.id, end: end)))
                }
                if line.containsInside(point) {
                    hits.append(.middle(bus: index, line: line.id))
                }
            }
        }
        if hits.isEmpty { return .nothing }
        if hits.count == 1 { return hits[0] }
        if hits.count == 2, case let .freeEnd(b1, e1) = hits[0], case let .freeEnd(b2, e2) = hits[1],
           b1 == b2, buses[b1].joint(at: e1) == .makeCorner(e1, e2, at: point) {
            return .corner(bus: b1, e1, e2)
        }
        return .other
    }

    /// The bus a drag from `p` to `q` (both on the grid) would make or grow:
    /// one line or an L, joined at a T or an L where it starts or ends on a
    /// bus, or merged into a line it continues straight out of (a corner it
    /// leaves straight out of one of its lines becomes a T). A drag from one
    /// bus to another joins them into the bus it starts on.
    static func join(_ buses: [Bus], from p: CGPoint, to q: CGPoint) -> Result<BusDraft, BusJoinRefusal> {
        let candidates: [[CGPoint]] = p.x == q.x || p.y == q.y
            ? [[p, q]]
            : [[p, CGPoint(x: q.x, y: p.y), q], [p, CGPoint(x: p.x, y: q.y), q]]
        return join(buses, candidates: candidates)
    }

    /// The bus a drawn route would make or grow, by the same rules as
    /// `join(_:from:to:)`: the first of `candidates` that works. Every
    /// candidate runs between the same two points; each bend in it becomes
    /// an L.
    static func join(_ buses: [Bus], candidates raw: [[CGPoint]]) -> Result<BusDraft, BusJoinRefusal> {
        let candidates = raw.map(OrthogonalRoute.simplified).filter { $0.count >= 2 }
        guard let p = candidates.first?.first, let q = candidates.first?.last, p != q else {
            return .failure(.samePoint)
        }
        let atP = touch(p, in: buses)
        let atQ = touch(q, in: buses)
        if case .other = atP { return .failure(.badJoint) }
        if case .other = atQ { return .failure(.badJoint) }

        var base: Bus
        var target = atP.busIndex ?? atQ.busIndex
        var absorbed: Int?
        var wireMap: [UUID: UUID] = [:]
        if let bp = atP.busIndex, let bq = atQ.busIndex, bp != bq {
            guard let merged = merge(buses[bp], absorbing: buses[bq]) else { return .failure(.tooManyWires) }
            base = merged.bus
            wireMap = merged.wireMap
            target = bp
            absorbed = bq
        } else {
            base = target.map { buses[$0] } ?? Bus(lines: [], wires: Bus.defaultWires(count: Bus.defaultWireCount))
        }
        let others = buses.indices.filter { $0 != target && $0 != absorbed }.map { buses[$0] }

        for path in candidates {
            if let bus = build(base, others: others, path: path, atP: atP, atQ: atQ) {
                return .success(BusDraft(busIndex: target, bus: bus, path: path, absorbedIndex: absorbed, wireMap: wireMap))
            }
        }
        return .failure(.badJoint)
    }

    /// `first` with `second`'s lines and ports added.
    ///
    /// When only one of them has ports, the joined bus carries that one's
    /// wires as they are (`first`'s when neither has). When both do, a wire
    /// of `second` becomes `first`'s wire of the same name (ignoring case)
    /// and the rest are added; wires a port uses come first and unused ones
    /// last, so the bus can be shrunk again. Nil when that is too many wires.
    static func merge(_ first: Bus, absorbing second: Bus) -> (bus: Bus, wireMap: [UUID: UUID])? {
        let lines = first.lines + second.lines
        let ports = first.ports + second.ports
        if first.ports.isEmpty || second.ports.isEmpty {
            // The wires (and the type that set them) of the one with ports.
            let keep = first.ports.isEmpty && !second.ports.isEmpty ? second : first
            return (Bus(id: first.id, name: first.name, card: keep.card, lines: lines, wires: keep.wires,
                        ports: ports), [:])
        }

        var wires = first.wires
        var wireMap: [UUID: UUID] = [:]
        for wire in second.wires {
            if let same = first.wires.first(where: { $0.name.lowercased() == wire.name.lowercased() }) {
                wireMap[wire.id] = same.id
            } else {
                wires.append(wire)
            }
        }
        guard wires.count <= Bus.wireCountRange.upperBound else { return nil }
        let moved = second.ports.map { port in
            BusPort(id: port.id, line: port.line, start: port.start, side: port.side,
                    wireIDs: Set(port.wireIDs.map { wireMap[$0] ?? $0 }))
        }
        let allPorts = first.ports + moved
        let used = allPorts.reduce(into: Set<UUID>()) { $0.formUnion($1.wireIDs) }
        let ordered = wires.filter { used.contains($0.id) } + wires.filter { !used.contains($0.id) }
        // The type holds only while the wires are still exactly its pins.
        let card = ordered.map(\.name) == first.card.pinNames ? first.card : BusCard()
        return (Bus(id: first.id, name: first.name, card: card, lines: lines, wires: ordered, ports: allPorts), wireMap)
    }

    /// A corner touch, for a drag that leaves it along `segment` (from or to
    /// the corner): the end of the corner's line it goes straight out of, and
    /// the corner's shape and the T it becomes. Nil when it leaves along neither line.
    private static func leaving(_ touch: Touch, along segment: BusLine, lines: [BusLine]) -> (Touch, from: BusJoint, to: BusJoint)? {
        guard case let .corner(bus, e1, e2) = touch else { return nil }
        for (end, other) in [(e1, e2), (e2, e1)] {
            guard let line = lines.first(where: { $0.id == end.line }),
                  line.isHorizontal == segment.isHorizontal else { continue }
            let point = line.point(end.end)
            let far = segment.a == point ? segment.b : segment.a
            guard !line.contains(far) else { continue }
            return (.freeEnd(bus: bus, end),
                    from: .makeCorner(e1, e2, at: point).shape,
                    to: BusJoint.tee(end: other, on: end.line, at: point).shape)
        }
        return nil
    }

    private static func build(_ base: Bus, others: [Bus], path: [CGPoint], atP: Touch, atQ: Touch) -> Bus? {
        let drawn = zip(path, path.dropFirst()).map { BusLine(a: $0, b: $1) }
        var added = drawn
        var lines = base.lines
        var baseShapes = base.jointShapes
        /// New line ends that were merged into an existing line's end.
        var alias: [BusLineEnd: BusLineEnd] = [:]
        func resolve(_ end: BusLineEnd) -> BusLineEnd { alias[end] ?? end }

        // A corner the drag leaves straight out of one of its lines: that
        // line grows, and the corner becomes a T on it.
        var atP = atP, atQ = atQ
        if case .corner = atP {
            guard let (touch, from, to) = leaving(atP, along: drawn[0], lines: lines) else { return nil }
            atP = touch
            baseShapes.remove(from)
            baseShapes.insert(to)
        }
        if case .corner = atQ, let last = drawn.last {
            guard let (touch, from, to) = leaving(atQ, along: last, lines: lines) else { return nil }
            atQ = touch
            baseShapes.remove(from)
            baseShapes.insert(to)
        }

        // A new line that continues straight out of a free end extends that line.
        var mergedP = false
        if case let .freeEnd(_, end) = atP, let first = added.first,
           let index = lines.firstIndex(where: { $0.id == end.line }),
           lines[index].isHorizontal == first.isHorizontal, !lines[index].contains(first.b) {
            lines[index].set(end.end, to: first.b)
            alias[BusLineEnd(line: first.id, end: .b)] = end
            added.removeFirst()
            mergedP = true
        }
        var mergedQ = false
        if case let .freeEnd(_, end) = atQ, let last = added.last,
           let index = lines.firstIndex(where: { $0.id == end.line }),
           lines[index].isHorizontal == last.isHorizontal, !lines[index].contains(last.a) {
            lines[index].set(end.end, to: last.a)
            alias[BusLineEnd(line: last.id, end: .a)] = end
            added.removeLast()
            mergedQ = true
        }

        // End to end: the straight drag ran from a free end of one line to a
        // free end of another line in line with it. The three become one line;
        // the joints and ports of the line that goes away move to it.
        var ports = base.ports
        if mergedP, added.isEmpty, case let .freeEnd(_, startEnd) = atP, case let .freeEnd(_, endEnd) = atQ,
           startEnd.line != endEnd.line,
           let x = lines.firstIndex(where: { $0.id == startEnd.line }),
           let y = lines.firstIndex(where: { $0.id == endEnd.line }),
           lines[x].isHorizontal == lines[y].isHorizontal, lines[x].across == lines[y].across,
           !lines[y].contains(path[0]) {
            let gone = lines[y]
            let farEnd = endEnd.end.other
            lines[x].set(startEnd.end, to: gone.point(farEnd))
            lines.remove(at: y)
            baseShapes = Set(baseShapes.map {
                $0.replacingLine(gone.id, with: startEnd.line) { $0 == farEnd ? startEnd.end : $0 }
            })
            ports = ports.map { port in
                port.line == gone.id
                    ? BusPort(id: port.id, line: startEnd.line, start: port.start, side: port.side, wireIDs: port.wireIDs)
                    : port
            }
            mergedQ = true
        }

        // The joints the drag is meant to make.
        var expected = Set<BusJoint>()
        func expect(_ touch: Touch, _ newEnd: BusLineEnd, at point: CGPoint) {
            switch touch {
            case let .middle(_, line): expected.insert(.tee(end: resolve(newEnd), on: line, at: point))
            case let .freeEnd(_, end): expected.insert(.makeCorner(resolve(newEnd), end, at: point))
            default: break
            }
        }
        if !mergedP { expect(atP, BusLineEnd(line: drawn[0].id, end: .a), at: path[0]) }
        if !mergedQ, let last = drawn.last { expect(atQ, BusLineEnd(line: last.id, end: .b), at: path[path.count - 1]) }
        for index in 0..<(drawn.count - 1) {
            expected.insert(.makeCorner(resolve(BusLineEnd(line: drawn[index].id, end: .b)),
                                        resolve(BusLineEnd(line: drawn[index + 1].id, end: .a)),
                                        at: path[index + 1]))
        }

        let result = Bus(id: base.id, name: base.name, card: base.card, lines: lines + added, wires: base.wires,
                         ports: ports)
        guard result.isValid, result.portsFit else { return nil }
        guard Bus.firstTouch(others + [result]) == nil else { return nil }
        guard result.jointShapes == baseShapes.union(expected.map(\.shape)) else { return nil }
        return result
    }
}
