import CoreGraphics
import Foundation
import Observation

// MARK: - Grid

enum SchematicGrid {
    /// Every placed block corner, pin tip and wire bend snaps to this spacing.
    static let spacing: CGFloat = 10

    static func snap(_ value: CGFloat) -> CGFloat {
        (value / spacing).rounded() * spacing
    }

    static func snap(_ point: CGPoint) -> CGPoint {
        CGPoint(x: snap(point.x), y: snap(point.y))
    }
}

// MARK: - Tools

enum SchematicTool: String, CaseIterable, Identifiable {
    case pointer
    case eraser
    case line
    case flip
    case vcc
    case ground

    var id: String { rawValue }

    var title: String {
        switch self {
        case .pointer: "Pointer"
        case .eraser: "Eraser"
        case .line: "Line"
        case .flip: "Flip"
        case .vcc: "VCC"
        case .ground: "GND"
        }
    }

    var symbol: String {
        switch self {
        case .pointer: "cursorarrow"
        case .eraser: "eraser"
        case .line: "line.diagonal"
        case .flip: "arrow.left.and.right"
        case .vcc: "bolt"
        case .ground: "arrow.down.to.line"
        }
    }

    /// The supply placed by this tool, if it places one.
    var supplyName: String? {
        switch self {
        case .vcc: "VCC"
        case .ground: "GND"
        default: nil
        }
    }
}

// MARK: - Block geometry

/// How a placed part is drawn. Logic chips are boxes; oscillators are boxes with
/// a sine wave in a circle in place of the function name; LEDs use the diode
/// symbol; VCC and GND use their own supply symbols; DIP switches are boxes
/// with one lever per position.
nonisolated enum PartSymbol: Hashable {
    case block
    case oscillator
    case diode
    case vcc
    case ground
    case dipSwitch

    /// Supplies have their pin on the top or bottom edge, so their view needs
    /// room above and below the body for the pin stub.
    var hasVerticalPins: Bool {
        self == .vcc || self == .ground
    }

    /// Flip turns logic chips and LEDs left to right. VCC and GND are fixed
    /// in place, so flipping them does nothing.
    var canFlip: Bool {
        !hasVerticalPins
    }
}

/// Schematic block geometry, derived from the backend's pin roles.
///
/// Left column: power pins at the top, a one-row gap, then inputs, then any
/// unconnected pins.
/// Right column: outputs and in/outs at the top, and ground pins at the bottom,
/// with a gap before them.
nonisolated struct ChipLayout: Hashable {
    enum Side: Hashable {
        case left
        case right
        case top
        case bottom

        var opposite: Side {
            switch self {
            case .left: .right
            case .right: .left
            case .top: .bottom
            case .bottom: .top
            }
        }
    }

    struct PinSlot: Identifiable, Hashable {
        let pin: ChipPin
        let side: Side
        /// Vertical centre of a left or right pin inside the body, in body coordinates.
        let centerY: CGFloat
        /// Horizontal centre of a top or bottom pin inside the body.
        var centerX: CGFloat = 0

        var id: Int { pin.number }
    }

    /// Height of one pin row. Two grid cells, so every pin lands on the grid.
    static let pitch: CGFloat = 20
    static let headerHeight: CGFloat = 30
    static let bodyWidth: CGFloat = 120
    static let stubLength: CGFloat = 10
    /// Room on each side of the body for pin numbers.
    static let labelMargin: CGFloat = 30

    /// Diode symbol: the anode and cathode leads sit on this row, and the body
    /// is 40 wide. The extra height under the symbol holds its label.
    static let diodeCentreY: CGFloat = 20
    static let diodeSize = CGSize(width: 40, height: 50)

    let size: CGSize
    let slots: [PinSlot]

    /// Supply symbols are 40 by 20. The single pin sits on the middle column:
    /// VCC's on the bottom edge, GND's on the top edge. Each symbol's bar sits
    /// on its pin's edge, so the connector from bar to pin is one stub (10px).
    static let supplyPinX: CGFloat = 20
    static let supplySize = CGSize(width: 40, height: 20)

    /// DIP switch body width, and the size of each position's lever, which is
    /// centred on its row.
    static let switchBodyWidth: CGFloat = 100
    static let switchLeverSize = CGSize(width: 36, height: 14)

    /// Pins in the hidden group (positions with no lead) are left out of every
    /// symbol, so they are never drawn and can't take a wire.
    init(chip: ChipDefinition, symbol: PartSymbol) {
        var chip = chip
        chip.pins = chip.pins.filter { !$0.isHidden }
        switch symbol {
        case .block, .oscillator:
            self = ChipLayout(chip: chip)
        case .diode:
            self = Self.diode(chip)
        case .vcc:
            self = Self.supply(chip, side: .bottom)
        case .ground:
            self = Self.supply(chip, side: .top)
        case .dipSwitch:
            self = Self.dipSwitch(chip)
        }
    }

    /// One row per switch position, top to bottom. Position k joins pin k
    /// (left) and pin 2N + 1 - k (right), as in switch.h, so both of a
    /// position's pins sit on its row.
    private static func dipSwitch(_ chip: ChipDefinition) -> ChipLayout {
        let positions = chip.pins.count / 2
        let byNumber = Dictionary(chip.pins.map { ($0.number, $0) }, uniquingKeysWith: { first, _ in first })
        var slots: [PinSlot] = []
        for position in 1...max(positions, 1) {
            let y = centerY(forRow: position - 1)
            if let left = byNumber[position] {
                slots.append(PinSlot(pin: left, side: .left, centerY: y))
            }
            if let right = byNumber[2 * positions + 1 - position] {
                slots.append(PinSlot(pin: right, side: .right, centerY: y))
            }
        }
        return ChipLayout(
            size: CGSize(width: switchBodyWidth, height: headerHeight + pitch * CGFloat(max(positions, 1))),
            slots: slots
        )
    }

    /// The lever of a switch position (1-based), in body coordinates.
    static func switchLeverRect(position: Int, bodyWidth: CGFloat) -> CGRect {
        let y = centerY(forRow: position - 1)
        return CGRect(
            x: (bodyWidth - switchLeverSize.width) / 2,
            y: y - switchLeverSize.height / 2,
            width: switchLeverSize.width,
            height: switchLeverSize.height
        )
    }

    private static func supply(_ chip: ChipDefinition, side: Side) -> ChipLayout {
        guard let pin = chip.pins.first else {
            return ChipLayout(chip: chip)
        }
        return ChipLayout(
            size: supplySize,
            slots: [PinSlot(pin: pin, side: side, centerY: 0, centerX: supplyPinX)]
        )
    }

    private init(size: CGSize, slots: [PinSlot]) {
        self.size = size
        self.slots = slots
    }

    /// The same body with every pin moved to the opposite side. Pin rows stay
    /// where they are.
    func flipped() -> ChipLayout {
        ChipLayout(
            size: size,
            slots: slots.map { slot in
                PinSlot(
                    pin: slot.pin,
                    side: slot.side.opposite,
                    centerY: slot.centerY,
                    centerX: slot.centerX
                )
            }
        )
    }

    /// Anode (pin 2, "A") on the left and cathode (pin 1, "K") on the right.
    /// Other pins, such as no-connects on a 4-DIP, are not drawn. Falls back to
    /// the block layout if either terminal is missing.
    private static func diode(_ chip: ChipDefinition) -> ChipLayout {
        guard let anode = chip.pins.first(where: { $0.name == "A" }),
              let cathode = chip.pins.first(where: { $0.name == "K" }) else {
            return ChipLayout(chip: chip)
        }
        return ChipLayout(
            size: diodeSize,
            slots: [
                PinSlot(pin: anode, side: .left, centerY: diodeCentreY),
                PinSlot(pin: cathode, side: .right, centerY: diodeCentreY),
            ]
        )
    }

    init(chip: ChipDefinition) {
        func byNumber(_ a: ChipPin, _ b: ChipPin) -> Bool { a.number < b.number }
        func pins(_ roles: PinRole...) -> [ChipPin] {
            chip.pins.filter { roles.contains($0.role) }.sorted(by: byNumber)
        }

        let power = pins(.power)
        let inputs = pins(.input)
        let unused = pins(.noConnect)
        let outputs = pins(.output, .inOut)
        let ground = pins(.ground)

        let left = Self.stack(Self.grouped(power) + Self.grouped(inputs) + Self.grouped(unused))
        let outputRows = Self.stack(Self.grouped(outputs))
        let rightTop = outputRows.count + (ground.isEmpty ? 0 : 1 + ground.count)
        let rowCount = max(left.count, rightTop, 1)

        let leftRows: [ChipPin?] = left + [ChipPin?](repeating: nil, count: rowCount - left.count)
        var rightRows = [ChipPin?](repeating: nil, count: rowCount)
        for (index, pin) in outputRows.enumerated() {
            rightRows[index] = pin
        }
        for (index, pin) in ground.enumerated() {
            rightRows[rowCount - ground.count + index] = pin
        }

        var slots: [PinSlot] = []
        for (row, pin) in leftRows.enumerated() {
            if let pin {
                slots.append(PinSlot(pin: pin, side: .left, centerY: Self.centerY(forRow: row)))
            }
        }
        for (row, pin) in rightRows.enumerated() {
            if let pin {
                slots.append(PinSlot(pin: pin, side: .right, centerY: Self.centerY(forRow: row)))
            }
        }

        self.slots = slots
        self.size = CGSize(
            width: Self.bodyWidth,
            height: Self.headerHeight + Self.pitch * CGFloat(rowCount)
        )
    }

    /// Splits pins (already in pin-number order) by pin group: the default
    /// group first, in pin-number order, then each named group in the order its
    /// first pin appears. A named group is a bus such as A0..A18, so its pins
    /// are in name order (A2 before A10). With only default pins this is the
    /// input unchanged, so blocks without groups lay out as before.
    private static func grouped(_ pins: [ChipPin]) -> [[ChipPin]] {
        var groups: [[ChipPin]] = [pins.filter(\.isInDefaultGroup)]
        var names: [String] = []
        for pin in pins where !pin.isInDefaultGroup && !names.contains(pin.group) {
            names.append(pin.group)
        }
        for name in names {
            groups.append(
                pins.filter { $0.group == name }
                    .sorted { $0.name.localizedStandardCompare($1.name) == .orderedAscending }
            )
        }
        return groups
    }

    /// Joins non-empty groups top to bottom, with one empty row between groups.
    private static func stack(_ groups: [[ChipPin]]) -> [ChipPin?] {
        var rows: [ChipPin?] = []
        for group in groups where !group.isEmpty {
            if !rows.isEmpty {
                rows.append(nil)
            }
            rows.append(contentsOf: group.map { Optional($0) })
        }
        return rows
    }

    private static func centerY(forRow row: Int) -> CGFloat {
        headerHeight + pitch * CGFloat(row) + pitch / 2
    }
}

// MARK: - Placed items

nonisolated struct PlacedChip: Identifiable, Hashable {
    let id: UUID
    let reference: String
    let partNumber: String
    let chip: ChipDefinition
    let symbol: PartSymbol
    /// Top-left corner of the body. Always on the grid.
    var origin: CGPoint
    /// Flipped parts have their pins on the opposite sides. For an LED the
    /// whole symbol is also mirrored; see `DiodeSymbolView`.
    var isFlipped = false
    /// For a DIP switch, whether each position is ON, position 1 first. Empty
    /// for every other part.
    var switchesOn: [Bool] = []

    private let baseLayout: ChipLayout

    init(
        id: UUID = UUID(),
        reference: String,
        partNumber: String,
        chip: ChipDefinition,
        symbol: PartSymbol,
        origin: CGPoint,
        isFlipped: Bool = false,
        switchesOn: [Bool] = []
    ) {
        self.id = id
        self.reference = reference
        self.partNumber = partNumber
        self.chip = chip
        self.symbol = symbol
        self.baseLayout = ChipLayout(chip: chip, symbol: symbol)
        self.origin = origin
        self.isFlipped = isFlipped
        // A switch has one state per position, OFF unless given; others none.
        let positions = symbol == .dipSwitch ? chip.pins.count / 2 : 0
        self.switchesOn = (0..<positions).map { $0 < switchesOn.count && switchesOn[$0] }
    }

    /// The switch position (1-based) whose lever is under `point`, if any.
    func switchPosition(at point: CGPoint) -> Int? {
        guard symbol == .dipSwitch else { return nil }
        let local = CGPoint(x: point.x - origin.x, y: point.y - origin.y)
        return switchesOn.indices.map { $0 + 1 }.first { position in
            ChipLayout.switchLeverRect(position: position, bodyWidth: layout.size.width)
                .insetBy(dx: -4, dy: -3)
                .contains(local)
        }
    }

    var layout: ChipLayout {
        isFlipped ? baseLayout.flipped() : baseLayout
    }

    /// Where a pin's wire leaves the body: the end of its stub.
    func tip(of slot: ChipLayout.PinSlot) -> CGPoint {
        let stub = ChipLayout.stubLength
        let size = layout.size
        switch slot.side {
        case .left:
            return CGPoint(x: origin.x - stub, y: origin.y + slot.centerY)
        case .right:
            return CGPoint(x: origin.x + size.width + stub, y: origin.y + slot.centerY)
        case .top:
            return CGPoint(x: origin.x + slot.centerX, y: origin.y - stub)
        case .bottom:
            return CGPoint(x: origin.x + slot.centerX, y: origin.y + size.height + stub)
        }
    }

    /// Top-left corner of the view. Supplies also need room above and below
    /// the body, so their view starts one stub higher.
    var viewOrigin: CGPoint {
        CGPoint(
            x: origin.x - ChipLayout.labelMargin,
            y: origin.y - (symbol.hasVerticalPins ? ChipLayout.stubLength : 0)
        )
    }

    var bodyRect: CGRect {
        CGRect(origin: origin, size: layout.size)
    }

    /// The body plus its pin stubs, used for hit testing.
    /// The body plus its pin stubs, used for hit testing and hover. Vertical
    /// supplies also have stubs above and below, so those are included too.
    var hitRect: CGRect {
        let stub = ChipLayout.stubLength
        let verticalStub = symbol.hasVerticalPins ? stub : 0
        return bodyRect.insetBy(dx: -stub, dy: -verticalStub)
    }
}

/// A pin on a placed chip.
struct PinRef: Hashable {
    let chipID: UUID
    let number: Int
}

/// One end of a wire: a pin on a placed part, or a junction where wires meet.
enum WireEnd: Hashable {
    case pin(PinRef)
    case junction(UUID)
}

/// A grid point where wires meet. Wires that branch from a point on another
/// wire both end here, so the point is fixed in place.
struct Junction: Identifiable, Hashable {
    let id: UUID
    let point: CGPoint

    init(id: UUID = UUID(), point: CGPoint) {
        self.id = id
        self.point = point
    }
}

/// Where a line can start or end in Line mode: a pin, or a grid point that an
/// existing wire passes through.
enum LineAnchor: Hashable {
    case pin(PinRef)
    case point(CGPoint)
}

/// A wire joins two ends. Its route is the start tip, then the bends in
/// `interior`, then the end tip. Every segment is horizontal or vertical.
struct Wire: Identifiable, Hashable {
    let id: UUID
    var start: WireEnd
    var end: WireEnd
    var interior: [CGPoint]

    init(id: UUID = UUID(), start: WireEnd, end: WireEnd, interior: [CGPoint]) {
        self.id = id
        self.start = start
        self.end = end
        self.interior = interior
    }
}

// MARK: - Orthogonal routing

/// Right-angle routing helpers. Paths are arrays of grid points where
/// consecutive points share an x or a y.
enum OrthogonalRoute {
    /// Default route between two tips: out horizontally, a vertical run at the
    /// midpoint, then horizontally into the target.
    static func route(from a: CGPoint, to b: CGPoint) -> [CGPoint] {
        if a.x == b.x || a.y == b.y {
            return normalize([a, b])
        }
        let midX = SchematicGrid.snap((a.x + b.x) / 2)
        return normalize([a, CGPoint(x: midX, y: a.y), CGPoint(x: midX, y: b.y), b])
    }

    /// Merges points that overlap, adds a corner wherever two points are
    /// diagonal, and merges again in case the corners landed on existing points.
    /// Bends that sit on a straight run are kept, so a node the user added by
    /// double-clicking survives.
    static func normalize(_ input: [CGPoint]) -> [CGPoint] {
        let merged = mergeCoincident(input)

        var cornered: [CGPoint] = []
        for point in merged {
            if let previous = cornered.last, previous.x != point.x, previous.y != point.y {
                cornered.append(CGPoint(x: point.x, y: previous.y))
            }
            cornered.append(point)
        }
        return mergeCoincident(cornered)
    }

    /// Collapses points that land on the same position. When a point returns to
    /// an earlier position, the loop between the two is removed.
    static func mergeCoincident(_ points: [CGPoint]) -> [CGPoint] {
        var result: [CGPoint] = []
        for point in points {
            if let index = result.lastIndex(of: point) {
                if index + 1 < result.count {
                    result.removeSubrange((index + 1)..<result.count)
                }
            } else {
                result.append(point)
            }
        }
        return result
    }

    /// Re-fits a wire's stored vertices after one of its pins moved. Segments
    /// keep their angles: the segments leaving and entering a pin stay horizontal.
    ///
    /// - With vertices already present, none are added. The vertex next to each
    ///   pin is moved onto the pin's row.
    /// - With no vertices and a straight wire, one vertex goes at the midpoint.
    /// - With no vertices and a diagonal wire, one corner is added.
    static func settled(start: CGPoint, interior: [CGPoint], end: CGPoint) -> [CGPoint] {
        var points = [start] + interior + [end]

        if interior.isEmpty {
            if start.x == end.x || start.y == end.y {
                let middle = SchematicGrid.snap(CGPoint(x: (start.x + end.x) / 2, y: (start.y + end.y) / 2))
                points.insert(middle, at: 1)
            } else {
                points.insert(CGPoint(x: end.x, y: start.y), at: 1)
            }
        } else if interior.count == 1 {
            if points[1].x != start.x {
                points[1].y = start.y
            }
        } else {
            if points[1].x != start.x {
                points[1].y = start.y
            }
            let last = points.count - 2
            if points[last].x != end.x {
                points[last].y = end.y
            }
        }

        return Array(mergeCoincident(points).dropFirst().dropLast())
    }

    /// Moves only the bend at `index`. The neighbouring bends stay where they are,
    /// so the segments on either side may be at any angle.
    static func moveNode(_ path: [CGPoint], at index: Int, to point: CGPoint) -> [CGPoint] {
        guard index > 0, index < path.count - 1 else { return path }
        var moved = path
        moved[index] = point
        return mergeCoincident(moved)
    }

    /// Moves the two vertices at the ends of the segment at `index` by `offset`.
    /// Pin tips never move, so a segment touching a pin moves only its other end.
    static func moveSegment(_ path: [CGPoint], at index: Int, by offset: CGSize) -> [CGPoint] {
        guard index >= 0, index + 1 < path.count else { return path }

        var moved = path
        if index > 0 {
            moved[index] = CGPoint(x: path[index].x + offset.width, y: path[index].y + offset.height)
        }
        if index + 1 < path.count - 1 {
            moved[index + 1] = CGPoint(x: path[index + 1].x + offset.width, y: path[index + 1].y + offset.height)
        }
        return mergeCoincident(moved)
    }

    /// Adds a bend on the segment at `index`, at the grid point nearest `point`.
    static func insertNode(_ path: [CGPoint], segment index: Int, at point: CGPoint) -> [CGPoint] {
        guard index >= 0, index + 1 < path.count else { return path }

        // Project the clicked point onto the segment, then snap it to the grid.
        let a = path[index]
        let b = path[index + 1]
        let dx = b.x - a.x
        let dy = b.y - a.y
        let lengthSquared = dx * dx + dy * dy
        let t = lengthSquared > 0
            ? min(max(((point.x - a.x) * dx + (point.y - a.y) * dy) / lengthSquared, 0), 1)
            : 0
        let node = SchematicGrid.snap(CGPoint(x: a.x + t * dx, y: a.y + t * dy))

        var result = path
        result.insert(node, at: index + 1)
        return mergeCoincident(result)
    }
}

// MARK: - Document

/// The schematic being edited: placed blocks, wires and the selection.
@MainActor
@Observable
final class SchematicDocument {
    private(set) var chips: [PlacedChip] = []
    private(set) var wires: [Wire] = []
    private(set) var junctions: [Junction] = []
    var selectedChipID: UUID?

    /// Number for the next part's reference ("U" plus this number).
    private(set) var nextReference = 1

    /// Replaces everything on the canvas, for example with a file that was
    /// just opened. Clears the selection.
    func replaceContents(chips: [PlacedChip], wires: [Wire], junctions: [Junction], nextReference: Int) {
        self.chips = chips
        self.wires = wires
        self.junctions = junctions
        self.nextReference = max(nextReference, 1)
        selectedChipID = nil
    }

    // MARK: Chips

    /// Places a block with its centre under `point`, snapped to the grid.
    func placeChip(partNumber: String, chip: ChipDefinition, symbol: PartSymbol, centeredAt point: CGPoint) {
        let layout = ChipLayout(chip: chip, symbol: symbol)
        let origin = SchematicGrid.snap(
            CGPoint(x: point.x - layout.size.width / 2, y: point.y - layout.size.height / 2)
        )
        let placed = PlacedChip(
            reference: "U\(nextReference)",
            partNumber: partNumber,
            chip: chip,
            symbol: symbol,
            origin: origin
        )
        nextReference += 1
        chips.append(placed)
        selectedChipID = placed.id
    }

    func moveChip(_ id: UUID, to origin: CGPoint) {
        guard let index = chips.firstIndex(where: { $0.id == id }) else { return }
        chips[index].origin = SchematicGrid.snap(origin)
    }

    /// Swaps the side of every pin on a placed part, and re-fits the wires
    /// attached to it.
    func flipChip(_ id: UUID) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              chips[index].symbol.canFlip else { return }
        chips[index].isFlipped.toggle()
        settleWires(attachedTo: id)
    }

    func chip(at point: CGPoint) -> PlacedChip? {
        chips.last { $0.hitRect.contains(point) }
    }

    /// Flips a switch position (1-based) between ON and OFF. Returns the new
    /// state, or nil when the part is not a switch or has no such position.
    @discardableResult
    func toggleSwitch(_ id: UUID, position: Int) -> Bool? {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              chips[index].switchesOn.indices.contains(position - 1) else { return nil }
        chips[index].switchesOn[position - 1].toggle()
        return chips[index].switchesOn[position - 1]
    }

    // MARK: Pins

    /// Where a wire leaves or enters a pin: the end of its pin stub.
    func pinTip(_ ref: PinRef) -> CGPoint? {
        guard let chip = chips.first(where: { $0.id == ref.chipID }),
              let slot = chip.layout.slots.first(where: { $0.pin.number == ref.number }) else {
            return nil
        }
        return chip.tip(of: slot)
    }

    /// Tip position of every pin on every placed chip.
    var pinTips: [CGPoint] {
        chips.flatMap { chip in
            chip.layout.slots.compactMap { slot in
                pinTip(PinRef(chipID: chip.id, number: slot.pin.number))
            }
        }
    }

    /// The pin whose tip is within `radius` of `point`.
    func pin(at point: CGPoint, radius: CGFloat = 8) -> PinRef? {
        for chip in chips.reversed() {
            for slot in chip.layout.slots {
                let ref = PinRef(chipID: chip.id, number: slot.pin.number)
                if let tip = pinTip(ref), Self.distance(from: point, to: tip) <= radius {
                    return ref
                }
            }
        }
        return nil
    }

    /// Where a wire leaves or enters one of its ends.
    func endTip(_ end: WireEnd) -> CGPoint? {
        switch end {
        case let .pin(ref):
            return pinTip(ref)
        case let .junction(id):
            return junctions.first { $0.id == id }?.point
        }
    }

    /// The point a line would start or end at.
    func anchorTip(_ anchor: LineAnchor) -> CGPoint? {
        switch anchor {
        case let .pin(ref):
            return pinTip(ref)
        case let .point(point):
            return point
        }
    }

    /// What a line can start or end on near `point`: a pin if one is within
    /// `pinRadius`, otherwise the grid point nearest `point` when an existing
    /// wire passes through it and it is within `gridRadius`.
    func lineAnchor(at point: CGPoint, pinRadius: CGFloat, gridRadius: CGFloat) -> LineAnchor? {
        if let ref = pin(at: point, radius: pinRadius) {
            return .pin(ref)
        }
        let grid = SchematicGrid.snap(point)
        guard Self.distance(from: point, to: grid) <= gridRadius,
              segmentHit(at: grid, tolerance: 0.5) != nil else {
            return nil
        }
        return .point(grid)
    }

    // MARK: Wires

    /// Full route of a wire, from one end's tip to the other.
    func path(forWire id: UUID) -> [CGPoint] {
        guard let wire = wires.first(where: { $0.id == id }),
              let start = endTip(wire.start),
              let end = endTip(wire.end) else {
            return []
        }
        // Same rules as settleWires, applied on the fly, so the wire looks the
        // same while dragging as it does after the drop.
        let interior = OrthogonalRoute.settled(start: start, interior: wire.interior, end: end)
        return [start] + interior + [end]
    }

    var wirePaths: [[CGPoint]] {
        wires.map { path(forWire: $0.id) }
    }

    /// Connects two anchors with an orthogonal route. A grid point on an existing
    /// wire becomes a junction: that wire is split there and the new wire branches
    /// from it.
    func connect(_ start: LineAnchor, _ end: LineAnchor) {
        guard start != end,
              let a = anchorTip(start), let b = anchorTip(end), a != b else {
            return
        }

        // Resolve the start first, so a split for it is in place before the end is looked up.
        guard let from = endpoint(for: start), let to = endpoint(for: end),
              let fromTip = endTip(from), let toTip = endTip(to) else {
            return
        }
        let route = OrthogonalRoute.route(from: fromTip, to: toTip)
        wires.append(Wire(start: from, end: to, interior: Array(route.dropFirst().dropLast())))
    }

    /// The wire end for an anchor. Creates a junction when the anchor is a point on a wire.
    private func endpoint(for anchor: LineAnchor) -> WireEnd? {
        switch anchor {
        case let .pin(ref):
            return .pin(ref)
        case let .point(point):
            return junction(at: point).map { WireEnd.junction($0) }
        }
    }

    /// The junction at `point`, creating one if an existing wire passes through it.
    /// The wire is split into two wires that meet at the junction.
    private func junction(at point: CGPoint) -> UUID? {
        if let existing = junctions.first(where: { $0.point == point }) {
            return existing.id
        }
        guard let hit = segmentHit(at: point, tolerance: 0.5),
              let index = wires.firstIndex(where: { $0.id == hit.wire }) else {
            return nil
        }

        var path = self.path(forWire: hit.wire)
        if !path.contains(point) {
            path.insert(point, at: hit.index + 1)
        }
        guard let k = path.firstIndex(of: point), k > 0, k < path.count - 1 else {
            return nil
        }

        let created = Junction(point: point)
        junctions.append(created)

        let original = wires[index]
        wires[index].end = .junction(created.id)
        wires[index].interior = Array(path[1..<k])
        wires.insert(
            Wire(
                start: .junction(created.id),
                end: original.end,
                interior: Array(path[(k + 1)..<(path.count - 1)])
            ),
            at: index + 1
        )
        return created.id
    }

    /// Whether a wire end sits on a pin of the given part.
    private func attaches(_ end: WireEnd, to chipID: UUID) -> Bool {
        if case let .pin(ref) = end {
            return ref.chipID == chipID
        }
        return false
    }

    /// Drops junctions that no wire ends at any more.
    private func pruneJunctions() {
        junctions.removeAll { junction in
            !wires.contains { $0.start == .junction(junction.id) || $0.end == .junction(junction.id) }
        }
    }

    /// Called when a block is released. Re-fits the wires attached to it and
    /// stores the result, so the change stays rather than being recomputed per frame.
    func settleWires(attachedTo chipID: UUID) {
        for index in wires.indices where attaches(wires[index].start, to: chipID) || attaches(wires[index].end, to: chipID) {
            guard let start = endTip(wires[index].start), let end = endTip(wires[index].end) else { continue }
            wires[index].interior = OrthogonalRoute.settled(
                start: start,
                interior: wires[index].interior,
                end: end
            )
        }
    }

    /// Stores a new route for a wire. The pin tips stay where they are.
    func setWirePath(_ id: UUID, _ path: [CGPoint]) {
        guard let index = wires.firstIndex(where: { $0.id == id }), path.count >= 2 else { return }
        wires[index].interior = Array(path.dropFirst().dropLast())
    }

    /// A bend point on a wire, if `point` is on one.
    func nodeHit(at point: CGPoint, tolerance: CGFloat = 6) -> (wire: UUID, index: Int)? {
        for wire in wires.reversed() {
            let path = path(forWire: wire.id)
            guard path.count > 2 else { continue }
            for index in 1..<(path.count - 1)
                where Self.distance(from: point, to: path[index]) <= tolerance {
                return (wire.id, index)
            }
        }
        return nil
    }

    /// A segment of a wire, if `point` is on one. `index` is the segment's
    /// start point within the wire's path.
    func segmentHit(at point: CGPoint, tolerance: CGFloat = 6) -> (wire: UUID, index: Int)? {
        for wire in wires.reversed() {
            let path = path(forWire: wire.id)
            for index in 0..<max(path.count - 1, 0)
                where Self.distance(from: point, toSegment: path[index], path[index + 1]) <= tolerance {
                return (wire.id, index)
            }
        }
        return nil
    }

    /// Double-click on a wire: adds a bend at that point.
    func insertNode(at point: CGPoint, tolerance: CGFloat = 6) {
        guard let hit = segmentHit(at: point, tolerance: tolerance) else { return }
        let path = path(forWire: hit.wire)
        setWirePath(hit.wire, OrthogonalRoute.insertNode(path, segment: hit.index, at: point))
    }

    // MARK: Erasing

    /// Removes whatever is under `point`: a block (and its wires) first, then a wire.
    func erase(at point: CGPoint, tolerance: CGFloat = 6) {
        if let hit = chip(at: point) {
            chips.removeAll { $0.id == hit.id }
            wires.removeAll { attaches($0.start, to: hit.id) || attaches($0.end, to: hit.id) }
            if selectedChipID == hit.id {
                selectedChipID = nil
            }
        } else if let hit = segmentHit(at: point, tolerance: tolerance) {
            wires.removeAll { $0.id == hit.wire }
        } else {
            return
        }
        pruneJunctions()
    }

    // MARK: Geometry

    static func distance(from a: CGPoint, to b: CGPoint) -> CGFloat {
        hypot(a.x - b.x, a.y - b.y)
    }

    static func distance(from point: CGPoint, toSegment a: CGPoint, _ b: CGPoint) -> CGFloat {
        let dx = b.x - a.x
        let dy = b.y - a.y
        let lengthSquared = dx * dx + dy * dy

        guard lengthSquared > 0 else {
            return distance(from: point, to: a)
        }

        let t = min(max(((point.x - a.x) * dx + (point.y - a.y) * dy) / lengthSquared, 0), 1)
        return distance(from: point, to: CGPoint(x: a.x + t * dx, y: a.y + t * dy))
    }
}
