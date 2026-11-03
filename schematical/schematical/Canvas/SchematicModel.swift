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
    case pullUp
    case pullDown

    var id: String { rawValue }

    var title: String {
        switch self {
        case .pointer: "Pointer"
        case .eraser: "Eraser"
        case .line: "Line"
        case .flip: "Flip"
        case .vcc: "VCC"
        case .ground: "GND"
        case .pullUp: "Pull-up"
        case .pullDown: "Pull-down"
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
        case .pullUp: "arrow.up.circle"
        case .pullDown: "arrow.down.circle"
        }
    }

    /// The supply placed by this tool, if it places one: `voltage` (a
    /// `SupplyChoice` name) for VCC, GND for ground, `PULLUP-<voltage>` for
    /// Pull-up and PULLDOWN for Pull-down.
    func supplyName(voltage: String) -> String? {
        switch self {
        case .vcc: voltage
        case .ground: "GND"
        case .pullUp: "PULLUP-" + voltage
        case .pullDown: "PULLDOWN"
        default: nil
        }
    }
}

/// The supplies the VCC tool can place, in menu order (library part names).
enum SupplyChoice {
    static let names = ["5V", "3V3", "2V5", "1V8", "1V2", "12V"]
    static let defaultName = "5V"
}

// MARK: - Block geometry

/// How a placed part is drawn. Logic chips are boxes; oscillators are boxes with
/// a sine wave in a circle in place of the function name; LEDs use the diode
/// symbol; VCC and GND use their own supply symbols, and pull-ups and
/// pull-downs a resistor to a rail or to ground; DIP switches are boxes
/// with one lever per position; 7-segment displays are boxes that show their
/// digits.
nonisolated enum PartSymbol: Hashable {
    case block
    case oscillator
    case diode
    case vcc
    case ground
    case pullUp
    case pullDown
    case dipSwitch
    case sevenSegment
    case characterLCD

    /// Supplies have their pin on the top or bottom edge, so their view needs
    /// room above and below the body for the pin stub.
    var hasVerticalPins: Bool {
        self == .vcc || self == .ground || self == .pullUp || self == .pullDown
    }

    /// Flip turns logic chips and LEDs left to right. VCC, GND and the pulls
    /// are fixed in place, so flipping them does nothing.
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

        var id: Int { pin.index }
    }

    /// Height of one pin row. Two grid cells, so every pin lands on the grid.
    static let pitch: CGFloat = 20
    static let headerHeight: CGFloat = 30
    static let bodyWidth: CGFloat = 120
    static let stubLength: CGFloat = 10
    /// Room on each side of the body for pin labels.
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
    /// Pull-up and pull-down symbols are 40 by 80: the 60-tall resistor (6
    /// grid steps) plus the rail bar and label (pull-up) or the ground bars
    /// (pull-down). The pin is on the bottom (pull-up) or top (pull-down) edge.
    static let pullSize = CGSize(width: 40, height: 80)
    static let pullResistorHeight: CGFloat = 60

    /// DIP switch body width, and the size of each position's lever, which is
    /// centred on its row.
    static let switchBodyWidth: CGFloat = 100
    static let switchLeverSize = CGSize(width: 36, height: 14)

    /// Absent pins (numbered positions with no lead) are left out of every
    /// symbol, so they are never drawn and can't take a wire.
    init(part: CatalogPart, symbol: PartSymbol) {
        let pins = part.pins.filter { !$0.isHidden }
        switch symbol {
        case .block, .oscillator:
            self = ChipLayout(pins: pins)
        case .diode:
            self = Self.diode(pins)
        case .vcc:
            self = Self.supply(pins, side: .bottom)
        case .ground:
            self = Self.supply(pins, side: .top)
        case .pullUp:
            self = Self.supply(pins, side: .bottom, size: Self.pullSize)
        case .pullDown:
            self = Self.supply(pins, side: .top, size: Self.pullSize)
        case .dipSwitch:
            self = Self.dipSwitch(pins)
        case .sevenSegment:
            self = Self.sevenSegment(pins, display: part.display)
        case .characterLCD:
            self = Self.characterLCD(pins, info: part.characterLCD)
        }
    }

    /// Character LCD face: cells of 5 x 8 dots, each dot `lcdDot` square on
    /// an `lcdDotPitch` pitch; one empty dot column between cells and two
    /// empty dot rows between lines; `lcdFaceMargin` all round. A name column
    /// on each side of the face, as for 7-segment displays.
    static let lcdDot: CGFloat = 2
    static let lcdDotPitch: CGFloat = 2.5
    static let lcdFaceMargin: CGFloat = 6
    static let lcdCellSize = CGSize(width: 4 * lcdDotPitch + lcdDot, height: 7 * lcdDotPitch + lcdDot)
    static let lcdCellPitch = CGSize(width: 6 * lcdDotPitch, height: 10 * lcdDotPitch)

    /// The face's size: 249 x 56.5 for 16 x 2.
    static func lcdFaceSize(_ info: CharacterLCDInfo) -> CGSize {
        let columns = CGFloat(max(info.columns, 1)), rows = CGFloat(max(info.rows, 1))
        return CGSize(
            width: (columns - 1) * lcdCellPitch.width + lcdCellSize.width + 2 * lcdFaceMargin,
            height: (rows - 1) * lcdCellPitch.height + lcdCellSize.height + 2 * lcdFaceMargin
        )
    }

    /// Bus pins on the left (RS, R/W, E, a one-row gap, DB0-DB7); every
    /// other drawn pin on the right, in pin order (VSS, VDD, V0, LED+, LED-).
    private static func characterLCD(_ pins: [ChipPin], info: CharacterLCDInfo?) -> ChipLayout {
        guard let info else { return ChipLayout(pins: pins) }
        let byIndex = Dictionary(pins.map { ($0.index, $0) }, uniquingKeysWith: { first, _ in first })
        let bus = info.busPins.compactMap { byIndex[$0] }
        let left = stack([Array(bus.prefix(3)), Array(bus.dropFirst(3))])
        let busSet = Set(info.busPins)
        let right = pins.filter { !busSet.contains($0.index) }.sorted { $0.index < $1.index }
        let face = lcdFaceSize(info)
        let faceRows = Int(((face.height + 16) / pitch).rounded(.up))
        let rows = max(left.count, right.count, faceRows)

        var slots: [PinSlot] = []
        for (row, pin) in left.enumerated() {
            if let pin {
                slots.append(PinSlot(pin: pin, side: .left, centerY: centerY(forRow: row)))
            }
        }
        for (row, pin) in right.enumerated() {
            slots.append(PinSlot(pin: pin, side: .right, centerY: centerY(forRow: row)))
        }
        let width = SchematicGrid.snap(face.width + 2 * sevenSegmentNameColumn + 5)
        return ChipLayout(
            size: CGSize(width: max(width, bodyWidth), height: headerHeight + pitch * CGFloat(rows)),
            slots: slots
        )
    }

    /// 7-segment display: digit size, gap between digits, and room for the
    /// colon. The digits sit in the middle of the body, with a column for pin
    /// names on each side (`sevenSegmentNameColumn`).
    static let sevenSegmentDigitSize = CGSize(width: 18, height: 32)
    static let sevenSegmentDigitGap: CGFloat = 10
    static let sevenSegmentColonWidth: CGFloat = 8
    static let sevenSegmentNameColumn: CGFloat = 36

    /// Width of the face that shows the digits, colon included.
    static func sevenSegmentFaceWidth(_ display: DisplayInfo) -> CGFloat {
        let digits = CGFloat(max(display.digits, 1))
        let colon = display.colonAfter > 0 ? sevenSegmentColonWidth : 0
        return digits * sevenSegmentDigitSize.width + (digits + 1) * sevenSegmentDigitGap + colon
    }

    /// Segment pins on the left and common pins on the right, in the order
    /// the backend gives. No-connect pins are not drawn.
    private static func sevenSegment(_ pins: [ChipPin], display: DisplayInfo?) -> ChipLayout {
        guard let display else { return ChipLayout(pins: pins) }
        let byIndex = Dictionary(pins.map { ($0.index, $0) }, uniquingKeysWith: { first, _ in first })
        // Segment pins, then (after a gap) any no-connect leads; commons on the right.
        let unused = pins.filter { $0.role == .noConnect }.sorted { $0.index < $1.index }
        let left = stack([display.segmentPins.compactMap { byIndex[$0] }, unused])
        let right = display.commonPins.compactMap { byIndex[$0] }
        // At least enough rows for the digit face.
        let faceRows = Int(((sevenSegmentDigitSize.height + 16) / pitch).rounded(.up))
        let rows = max(left.count, right.count, faceRows)

        var slots: [PinSlot] = []
        for (row, pin) in left.enumerated() {
            if let pin {
                slots.append(PinSlot(pin: pin, side: .left, centerY: centerY(forRow: row)))
            }
        }
        for (row, pin) in right.enumerated() {
            slots.append(PinSlot(pin: pin, side: .right, centerY: centerY(forRow: row)))
        }
        let width = SchematicGrid.snap(sevenSegmentFaceWidth(display) + 2 * sevenSegmentNameColumn + 5)
        return ChipLayout(
            size: CGSize(width: max(width, bodyWidth), height: headerHeight + pitch * CGFloat(rows)),
            slots: slots
        )
    }

    /// One row per switch position, top to bottom. Position k joins the pins
    /// at index k - 1 (left) and 2N - k (right), as in switch.h, so both of a
    /// position's pins sit on its row.
    private static func dipSwitch(_ pins: [ChipPin]) -> ChipLayout {
        let positions = pins.count / 2
        let byIndex = Dictionary(pins.map { ($0.index, $0) }, uniquingKeysWith: { first, _ in first })
        var slots: [PinSlot] = []
        for position in 1...max(positions, 1) {
            let y = centerY(forRow: position - 1)
            if let left = byIndex[position - 1] {
                slots.append(PinSlot(pin: left, side: .left, centerY: y))
            }
            if let right = byIndex[2 * positions - position] {
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

    private static func supply(_ pins: [ChipPin], side: Side, size: CGSize = supplySize) -> ChipLayout {
        guard let pin = pins.first else {
            return ChipLayout(pins: pins)
        }
        return ChipLayout(
            size: size,
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
    private static func diode(_ pins: [ChipPin]) -> ChipLayout {
        guard let anode = pins.first(where: { $0.name == "A" }),
              let cathode = pins.first(where: { $0.name == "K" }) else {
            return ChipLayout(pins: pins)
        }
        return ChipLayout(
            size: diodeSize,
            slots: [
                PinSlot(pin: anode, side: .left, centerY: diodeCentreY),
                PinSlot(pin: cathode, side: .right, centerY: diodeCentreY),
            ]
        )
    }

    init(pins allPins: [ChipPin]) {
        func byIndex(_ a: ChipPin, _ b: ChipPin) -> Bool { a.index < b.index }
        func pins(_ roles: ChipPinRole...) -> [ChipPin] {
            allPins.filter { roles.contains($0.role) }.sorted(by: byIndex)
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

    /// Splits pins (already in pinout order) by pin group: the default
    /// group first, in pinout order, then each named group in the order its
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

/// An EEPROM's load file: where it is, and the app-scoped security bookmark
/// that lets the sandboxed app read it after a restart.
nonisolated struct MemoryFile: Hashable, Sendable {
    var url: URL
    var bookmark: Data?
    var name: String { url.lastPathComponent }
}

nonisolated struct PlacedChip: Identifiable, Hashable {
    let id: UUID
    /// "U1", "R3": letters then a number. Changed through
    /// `SchematicDocument.renameChip`, which keeps references unique.
    var reference: String
    /// The library part, copied from the backend at launch.
    let part: CatalogPart
    let symbol: PartSymbol
    /// Top-left corner of the body. Always on the grid.
    var origin: CGPoint
    /// Flipped parts have their pins on the opposite sides. For an LED the
    /// whole symbol is also mirrored; see `DiodeSymbolView`.
    var isFlipped = false
    /// For a DIP switch, whether each position is ON, position 1 first. Empty
    /// for every other part.
    var switchesOn: [Bool] = []
    /// Where the footprint sits on the PCB. Set exactly when the part has a
    /// footprint (every part but VCC and GND).
    var board: BoardPlacement?
    /// For an EEPROM, the file loaded into it when the simulation starts.
    /// Nil for every other part, and for an EEPROM that starts erased.
    var memoryFile: MemoryFile?

    private let baseLayout: ChipLayout

    init(
        id: UUID = UUID(),
        reference: String,
        part: CatalogPart,
        symbol: PartSymbol,
        origin: CGPoint,
        isFlipped: Bool = false,
        switchesOn: [Bool] = [],
        board: BoardPlacement? = nil,
        memoryFile: MemoryFile? = nil
    ) {
        self.id = id
        self.reference = reference
        self.part = part
        self.symbol = symbol
        self.baseLayout = ChipLayout(part: part, symbol: symbol)
        self.origin = origin
        self.isFlipped = isFlipped
        self.board = part.footprint == nil ? nil : board
        self.memoryFile = part.kind == PART_EEPROM ? memoryFile : nil
        // A switch has one state per position, OFF unless given; others none.
        var positions = 0
        if case let .dipSwitch(count, _, _) = part.details {
            positions = count
        }
        self.switchesOn = (0..<positions).map { $0 < switchesOn.count && switchesOn[$0] }
    }

    /// The orderable part number, as saved in a file.
    var partNumber: String { part.name }

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
    /// The pin's position in its part's pinout (`ChipPin.index`).
    let index: Int
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
    /// The selected part. Selecting one clears the selected text.
    var selectedChipID: UUID? {
        didSet {
            if selectedChipID != nil, selectedTextID != nil { selectedTextID = nil }
        }
    }

    /// The PCB's size, layers, colour and units.
    var board = BoardSettings()
    /// Free texts on the PCB, in drawing order.
    private(set) var texts: [BoardText] = []
    /// The selected board text. Selecting one clears the selected part.
    var selectedTextID: UUID? {
        didSet {
            if selectedTextID != nil, selectedChipID != nil { selectedChipID = nil }
        }
    }

    /// Lowest number tried for the next part's reference ("U" plus a number).
    private(set) var nextReference = 1

    /// Why the last wire or switch edit was refused (a voltage out of range),
    /// or nil. Cleared by the next accepted one; the window clears it after a
    /// few seconds.
    var editorMessage: String?

    /// The part library, for the voltage check. With none, no edit is
    /// refused.
    private let library: PartLibraryHandle?

    init(library: PartLibraryHandle? = nil) {
        self.library = library
    }

    /// Replaces everything on the canvas, for example with a file that was
    /// just opened. Clears the selection.
    func replaceContents(chips: [PlacedChip], wires: [Wire], junctions: [Junction], nextReference: Int,
                         board: BoardSettings, texts: [BoardText]) {
        self.chips = chips
        self.wires = wires
        self.junctions = junctions
        self.nextReference = max(nextReference, 1)
        self.board = board
        self.texts = texts
        selectedChipID = nil
        selectedTextID = nil
        editorMessage = nil
    }

    // MARK: Chips

    /// Places a block with its centre under `point`, snapped to the grid.
    /// The new part is not selected, so the parts library stays open for
    /// the next drop.
    func placeChip(part: CatalogPart, symbol: PartSymbol, centeredAt point: CGPoint) {
        let layout = ChipLayout(part: part, symbol: symbol)
        let origin = SchematicGrid.snap(
            CGPoint(x: point.x - layout.size.width / 2, y: point.y - layout.size.height / 2)
        )
        // The first U<n> from nextReference on that no part uses, since a
        // part may have been renamed to one.
        let used = Set(chips.map(\.reference))
        var number = nextReference
        while used.contains("U\(number)") {
            number += 1
        }
        let placed = PlacedChip(
            reference: "U\(number)",
            part: part,
            symbol: symbol,
            origin: origin,
            board: part.footprint.map { BoardLayout.nextSpot(for: $0, among: chips) }
        )
        nextReference = number + 1
        chips.append(placed)
    }

    /// Renames a part to `prefix` (uppercased) plus `number`. Returns why it
    /// can't (a bad prefix or number, or a reference another part has), or
    /// nil when it was renamed.
    @discardableResult
    func renameChip(_ id: UUID, prefix: String, number: Int?) -> String? {
        if let message = ReferenceParts.validate(prefix: prefix, number: number) {
            return message
        }
        guard let number, let index = chips.firstIndex(where: { $0.id == id }) else { return nil }
        let reference = ReferenceParts.reference(prefix: prefix, number: number)
        if chips.contains(where: { $0.id != id && $0.reference == reference }) {
            return "\(reference) is already used by another part."
        }
        chips[index].reference = reference
        return nil
    }

    func moveChip(_ id: UUID, to origin: CGPoint) {
        guard let index = chips.firstIndex(where: { $0.id == id }) else {
            return
        }

        let snapped = SchematicGrid.snap(origin)
        guard chips[index].origin != snapped else {
            return
        }

        chips[index].origin = snapped
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

    // MARK: Footprints (PCB)

    /// Moves a footprint's centre (board mm), then snaps its origin point to
    /// the board grid.
    func moveFootprint(_ id: UUID, to centre: CGPoint) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              let board = chips[index].board,
              let footprint = chips[index].footprint else { return }
        var moved = board
        moved.centre = centre
        chips[index].board = BoardLayout.snapped(moved, footprint: footprint,
                                                 pad1: chips[index].part.firstPinDesignator)
    }

    /// Moves a footprint so its origin point lands on `point` (board mm),
    /// snapped to the grid.
    func moveFootprintOrigin(_ id: UUID, to point: CGPoint) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              let board = chips[index].board,
              let footprint = chips[index].footprint else { return }
        let pad1 = chips[index].part.firstPinDesignator
        let current = BoardLayout.originPoint(of: board, footprint: footprint, pad1: pad1)
        var moved = board
        moved.centre.x += point.x - current.x
        moved.centre.y += point.y - current.y
        chips[index].board = BoardLayout.snapped(moved, footprint: footprint, pad1: pad1)
    }

    /// Turns a footprint 90° clockwise about its origin point, which stays
    /// where it was.
    func rotateFootprint(_ id: UUID) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              let board = chips[index].board,
              let footprint = chips[index].footprint else { return }
        let pad1 = chips[index].part.firstPinDesignator
        let before = BoardLayout.originPoint(of: board, footprint: footprint, pad1: pad1)
        var turned = board
        turned.rotation = (board.rotation + 90) % 360
        let after = BoardLayout.originPoint(of: turned, footprint: footprint, pad1: pad1)
        turned.centre.x += before.x - after.x
        turned.centre.y += before.y - after.y
        chips[index].board = BoardLayout.snapped(turned, footprint: footprint, pad1: pad1)
    }

    /// Changes which point of a footprint is its origin. The footprint stays
    /// where it is and is not re-snapped.
    func setFootprintOrigin(_ id: UUID, _ origin: FootprintOrigin) {
        guard let index = chips.firstIndex(where: { $0.id == id }), chips[index].board != nil else { return }
        chips[index].board?.origin = origin
    }

    /// Switches a part between its footprint and its connector footprint
    /// (the PCB Style option). Pin 1's first pad stays exactly where it was
    /// (not re-snapped: like an origin change, this may leave the origin
    /// point off the grid); rotation, origin and marks are kept. Does nothing for a part without a connector
    /// footprint or when the style does not change.
    func setFootprintStyle(_ id: UUID, _ style: FootprintStyle) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              chips[index].part.connectorFootprint != nil,
              let board = chips[index].board, board.style != style,
              let before = chips[index].footprint else { return }
        let pad1 = chips[index].part.firstPinDesignator
        var changed = board
        changed.style = style
        chips[index].board = changed
        guard let after = chips[index].footprint else { return }
        if let old = before.pads.first(where: { $0.designator == pad1 }),
           let new = after.pads.first(where: { $0.designator == pad1 }) {
            let anchor = board.board(old.centre)
            let landed = changed.board(new.centre)
            changed.centre.x += anchor.x - landed.x
            changed.centre.y += anchor.y - landed.y
        }
        chips[index].board = changed
    }

    /// Sets where a footprint's marks are drawn. A nil layer is left as it is.
    func setFootprintMarks(_ id: UUID, outline: MarkLayer? = nil, pin1: MarkLayer? = nil,
                           reference: MarkLayer? = nil) {
        guard let index = chips.firstIndex(where: { $0.id == id }), chips[index].board != nil else { return }
        if let outline { chips[index].board?.outline = outline }
        if let pin1 { chips[index].board?.pin1 = pin1 }
        if let reference { chips[index].board?.reference = reference }
    }

    // MARK: Board

    /// Sets the board's size in mm, each side clamped to `BoardSettings.sizeRange`.
    func setBoardSize(width: Double, height: Double) {
        let range = BoardSettings.sizeRange
        board.width = min(max(width, range.lowerBound), range.upperBound)
        board.height = min(max(height, range.lowerBound), range.upperBound)
    }

    // MARK: Board texts

    /// Adds a "Text" on the silkscreen with its top-left at `point` (snapped)
    /// and selects it.
    @discardableResult
    func addText(at point: CGPoint) -> UUID {
        let text = BoardText(id: UUID(), text: "Text", anchor: BoardGrid.snap(point), size: BoardText.defaultSize)
        texts.append(text)
        selectedTextID = text.id
        return text.id
    }

    /// Changes a text's words. An empty string is refused and the text kept.
    func setText(_ id: UUID, _ value: String) {
        guard !value.isEmpty, let index = texts.firstIndex(where: { $0.id == id }) else { return }
        texts[index].text = value
    }

    /// Sets a text's font size in mil, clamped to `BoardText.sizeRange`.
    func setTextSize(_ id: UUID, _ size: Int) {
        guard let index = texts.firstIndex(where: { $0.id == id }) else { return }
        texts[index].size = min(max(size, BoardText.sizeRange.lowerBound), BoardText.sizeRange.upperBound)
    }

    func setTextLayer(_ id: UUID, _ layer: TextLayer) {
        guard let index = texts.firstIndex(where: { $0.id == id }) else { return }
        texts[index].layer = layer
    }

    /// Moves a text's anchor to `point`, snapped to the grid.
    func moveText(_ id: UUID, to point: CGPoint) {
        guard let index = texts.firstIndex(where: { $0.id == id }) else { return }
        texts[index].anchor = BoardGrid.snap(point)
    }

    /// Sets a text's rotation: 0, 90, 180 or 270. Other values are ignored.
    func setTextRotation(_ id: UUID, _ rotation: Int) {
        guard BoardPlacement.rotations.contains(rotation),
              let index = texts.firstIndex(where: { $0.id == id }) else { return }
        texts[index].rotation = rotation
    }

    /// Turns a text 90° clockwise about its anchor.
    func rotateText(_ id: UUID) {
        guard let index = texts.firstIndex(where: { $0.id == id }) else { return }
        texts[index].rotation = (texts[index].rotation + 90) % 360
    }

    func deleteText(_ id: UUID) {
        texts.removeAll { $0.id == id }
        if selectedTextID == id {
            selectedTextID = nil
        }
    }

    /// The text under a board point (mm): the last one drawn there.
    func text(at point: CGPoint) -> BoardText? {
        texts.last { $0.boardRect.contains(point) }
    }

    /// The footprint under a board point (mm): the last one drawn there.
    func footprint(at point: CGPoint) -> PlacedChip? {
        chips.last { $0.boardBounds?.contains(point) ?? false }
    }

    /// Sets or clears an EEPROM's load file. Does nothing for other parts.
    /// The memory window calls it only while the simulation is stopped.
    func setMemoryFile(_ id: UUID, _ file: MemoryFile?) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              chips[index].part.kind == PART_EEPROM else { return }
        chips[index].memoryFile = file
    }

    /// Flips a switch position (1-based) between ON and OFF. Returns the new
    /// state, or nil when the part is not a switch or has no such position,
    /// or when turning it ON would put a voltage out of range on a part (the
    /// switch stays OFF and `editorMessage` says why).
    @discardableResult
    func toggleSwitch(_ id: UUID, position: Int) -> Bool? {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              chips[index].switchesOn.indices.contains(position - 1) else { return nil }
        if !chips[index].switchesOn[position - 1], let library {
            var candidate = chips
            candidate[index].switchesOn[position - 1] = true
            if let issue = VoltageCheck.firstNew(before: chips, wires, after: candidate, wires, library: library) {
                editorMessage = VoltageCheck.message(issue, chips: chips, edit: .turnOn)
                return nil
            }
        }
        editorMessage = nil
        chips[index].switchesOn[position - 1].toggle()
        return chips[index].switchesOn[position - 1]
    }

    // MARK: Pins

    /// Where a wire leaves or enters a pin: the end of its pin stub.
    func pinTip(_ ref: PinRef) -> CGPoint? {
        guard let chip = chips.first(where: { $0.id == ref.chipID }),
              let slot = chip.layout.slots.first(where: { $0.pin.index == ref.index }) else {
            return nil
        }
        return chip.tip(of: slot)
    }

    /// Tip position of every pin on every placed chip.
    var pinTips: [CGPoint] {
        chips.flatMap { chip in
            chip.layout.slots.compactMap { slot in
                pinTip(PinRef(chipID: chip.id, index: slot.pin.index))
            }
        }
    }

    /// The pin whose tip is within `radius` of `point`.
    func pin(at point: CGPoint, radius: CGFloat = 8) -> PinRef? {
        for chip in chips.reversed() {
            for slot in chip.layout.slots {
                let ref = PinRef(chipID: chip.id, index: slot.pin.index)
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
    ///
    /// Refused, with nothing changed, when the wire would put a voltage out of
    /// range on a part (a problem the drawing doesn't already have). Returns
    /// why, which is also `editorMessage`; nil otherwise.
    func connect(_ start: LineAnchor, _ end: LineAnchor) -> String? {
        guard start != end,
              let a = anchorTip(start), let b = anchorTip(end), a != b else {
            return nil
        }

        // The check runs on stand-in ends before anything changes: resolving a
        // point on a wire for real splits that wire.
        if let library, let standInStart = standIn(for: start), let standInEnd = standIn(for: end) {
            let candidate = wires + [Wire(start: standInStart, end: standInEnd, interior: [])]
            if let issue = VoltageCheck.firstNew(before: chips, wires, after: chips, candidate, library: library) {
                editorMessage = VoltageCheck.message(issue, chips: chips, edit: .connect)
                return editorMessage
            }
        }
        editorMessage = nil

        // Resolve the start first, so a split for it is in place before the end is looked up.
        guard let from = endpoint(for: start), let to = endpoint(for: end),
              let fromTip = endTip(from), let toTip = endTip(to) else {
            return nil
        }
        let route = OrthogonalRoute.route(from: fromTip, to: toTip)
        wires.append(Wire(start: from, end: to, interior: Array(route.dropFirst().dropLast())))
        return nil
    }

    /// A wire end in the same net as the end `anchor` would get, without
    /// changing anything: a point on a wire stands in as that wire's start.
    private func standIn(for anchor: LineAnchor) -> WireEnd? {
        switch anchor {
        case let .pin(ref):
            return .pin(ref)
        case let .point(point):
            if let existing = junctions.first(where: { $0.point == point }) {
                return .junction(existing.id)
            }
            if let wire = wire(at: point) {
                return wire.start
            }
            return .junction(UUID())
        }
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

    /// The wire passing through `point`, if any. Changes nothing.
    func wire(at point: CGPoint) -> Wire? {
        wireHit(at: point)?.wire
    }

    /// The wire passing through `point`, and the segment it's on.
    private func wireHit(at point: CGPoint) -> (wire: Wire, index: Int)? {
        guard let hit = segmentHit(at: point, tolerance: 0.5),
              let wire = wires.first(where: { $0.id == hit.wire }) else {
            return nil
        }
        return (wire, hit.index)
    }

    /// The junction at `point`, creating one if an existing wire passes through it.
    /// The wire is split into two wires that meet at the junction.
    private func junction(at point: CGPoint) -> UUID? {
        if let existing = junctions.first(where: { $0.point == point }) {
            return existing.id
        }
        guard let hit = wireHit(at: point),
              let index = wires.firstIndex(where: { $0.id == hit.wire.id }) else {
            return nil
        }

        var path = self.path(forWire: hit.wire.id)
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
