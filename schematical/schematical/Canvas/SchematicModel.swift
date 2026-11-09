import CoreGraphics
import Foundation
import Observation

// MARK: - Grid

nonisolated enum SchematicGrid {
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
    case bus
    case busPort
    case flip
    case text
    case vcc
    case ground
    case pullUp
    case pullDown

    var id: String { rawValue }

    /// The toolbar's first group: editing and drawing tools.
    static let drawingTools: [SchematicTool] = [.pointer, .eraser, .line, .bus, .busPort, .flip, .text]

    /// The toolbar's second group, after a gap: the supply tools. VCC and
    /// Pull-up come last so they sit beside the supply voltage menu they use.
    static let powerTools: [SchematicTool] = [.ground, .pullDown, .vcc, .pullUp]

    var title: String {
        switch self {
        case .pointer: "Pointer"
        case .eraser: "Eraser"
        case .line: "Line"
        case .bus: "Bus"
        case .busPort: "Bus Port"
        case .flip: "Flip"
        case .text: "Text"
        case .vcc: "VCC"
        case .ground: "GND"
        case .pullUp: "Pull-up"
        case .pullDown: "Pull-down"
        }
    }

    /// The toolbar tooltip: the title, plus the drawing keys for Line and Bus.
    var tooltip: String {
        switch self {
        case .line, .bus: "\(title) — Space: lock a corner · /: flip the bend · Esc: undo a corner"
        case .text: "Text — click to add a text block"
        default: title
        }
    }

    var symbol: String {
        switch self {
        case .pointer: "cursorarrow"
        case .eraser: "eraser"
        case .line: "line.diagonal"
        case .bus: "line.3.horizontal"
        case .busPort: "rectangle.connected.to.line.below"
        case .flip: "arrow.left.and.right"
        case .text: "textformat"
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
/// a square wave in a circle in place of the function name; LEDs use the diode
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
    /// A tactile (push-button) switch, plan-27: side A's pins on the left,
    /// side B's on the right, a button in the middle.
    case tactileSwitch
    case sevenSegment
    case characterLCD
    case barrelJack
    /// A generic pin header (plan-23): pins on the left, and with two rows
    /// the even pins on the right.
    case header

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
        case .barrelJack:
            self = Self.barrelJack(pins)
        case .header:
            self = Self.header(pins, rows: part.headerSize?.rows ?? 1)
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
        case .tactileSwitch:
            self = Self.tactileSwitch(pins)
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

    /// Tactile switch: pins named A… (side A) on the left, B… on the right,
    /// in pin order, one row per pair; a two-terminal switch has one row.
    private static func tactileSwitch(_ pins: [ChipPin]) -> ChipLayout {
        let left = pins.filter { $0.name.hasPrefix("A") }
        let right = pins.filter { !$0.name.hasPrefix("A") }
        let rows = max(left.count, right.count, 1)
        var slots: [PinSlot] = []
        for (row, pin) in left.enumerated() {
            slots.append(PinSlot(pin: pin, side: .left, centerY: centerY(forRow: row)))
        }
        for (row, pin) in right.enumerated() {
            slots.append(PinSlot(pin: pin, side: .right, centerY: centerY(forRow: row)))
        }
        return ChipLayout(
            size: CGSize(width: switchBodyWidth, height: headerHeight + pitch * CGFloat(rows)),
            slots: slots
        )
    }

    /// A tactile switch's button: the middle third of the body, below the header.
    static func tactileButtonRect(bodySize: CGSize) -> CGRect {
        CGRect(x: bodySize.width / 3, y: headerHeight,
               width: bodySize.width / 3, height: bodySize.height - headerHeight)
    }

    /// Barrel jack terminals all leave on the right. The drawing follows the
    /// datasheet schematic: centre at top, switched sleeve contact beneath.
    private static func barrelJack(_ pins: [ChipPin]) -> ChipLayout {
        let centre = pins.first(where: { $0.name == "Centre" })
        let sleeve = pins.first(where: { $0.name == "Sleeve" })
        let switched = pins.first(where: { $0.name == "Switch" })
        var slots: [PinSlot] = []
        if let centre { slots.append(PinSlot(pin: centre, side: .right, centerY: 30)) }
        if let switched { slots.append(PinSlot(pin: switched, side: .right, centerY: 50)) }
        if let sleeve { slots.append(PinSlot(pin: sleeve, side: .right, centerY: 75)) }
        return ChipLayout(size: CGSize(width: 120, height: switched == nil ? 90 : 95), slots: slots)
    }

    /// Header body width; its height is one pin row (`pitch`) per column.
    static let headerBodyWidth: CGFloat = 40

    /// A pin header, pins zigzag as on the part (pin 2 across from pin 1):
    /// pin p is in column (p - 1) / rows, one 20-tall row each, centred at
    /// 10 + 20 c. With 1 row every pin is on the left; with 2 rows odd pins
    /// are on the left and even pins on the right, side by side.
    static func header(_ pins: [ChipPin], rows: Int) -> ChipLayout {
        let rows = max(rows, 1)
        let ordered = pins.sorted { $0.index < $1.index }
        let columns = max((ordered.count + rows - 1) / rows, 1)
        let slots = ordered.enumerated().map { position, pin in
            PinSlot(pin: pin, side: rows == 2 && position % 2 == 1 ? .right : .left,
                    centerY: pitch / 2 + pitch * CGFloat(position / rows))
        }
        return ChipLayout(size: CGSize(width: headerBodyWidth, height: pitch * CGFloat(columns)), slots: slots)
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
    /// An LED's color, one of `part.colors`. Nil for every other part.
    var color: String?
    /// For a part with footprint choices, the id of the chosen footprint (one
    /// of `part.footprintChoices`). Nil for every other part.
    var footprintID: String?
    /// A header's gender, one of `part.genders`. Nil for every other part.
    var gender: String?
    /// A pull resistor's resistance in ohms (`ResistorValue.isValid`). Nil
    /// for every other part.
    var ohms: Double?

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
        memoryFile: MemoryFile? = nil,
        color: String? = nil,
        footprintID: String? = nil,
        gender: String? = nil,
        ohms: Double? = nil
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
        // A nil or unknown option is the part's default; parts without the
        // option have none.
        if part.kind == PART_LED, !part.colors.isEmpty {
            self.color = color.flatMap { part.colors.contains($0) ? $0 : nil } ?? part.colors[0]
        } else {
            self.color = nil
        }
        if part.kind == PART_HEADER, !part.genders.isEmpty {
            self.gender = gender.flatMap { part.genders.contains($0) ? $0 : nil } ?? part.genders[0]
        } else {
            self.gender = nil
        }
        if !part.footprintChoices.isEmpty, let base = part.footprint {
            self.footprintID = footprintID.flatMap { id in
                part.footprintChoices.contains { $0.footprint.id == id } ? id : nil
            } ?? base.id
        } else {
            self.footprintID = nil
        }
        if part.isPullResistor {
            self.ohms = ohms.flatMap { ResistorValue.isValid($0) ? $0 : nil } ?? ResistorValue.defaultOhms
        } else {
            self.ohms = nil
        }
        // A switch has one state per position, OFF unless given; others none.
        // A tactile switch has one, "pressed", and always starts released
        // (the file never stores it; plan-27-J).
        var positions = 0
        if case let .dipSwitch(count, _, _) = part.details {
            positions = count
        }
        if part.isTactileSwitch {
            self.switchesOn = [false]
        } else {
            self.switchesOn = (0..<positions).map { $0 < switchesOn.count && switchesOn[$0] }
        }
    }

    /// The orderable part number, as saved in a file.
    var partNumber: String { part.name }

    /// The same chip on another part (a header's new size): id, reference,
    /// symbol, origin, flip, board placement, gender and memory file are
    /// kept. The footprint is the new part's choice with the same package as
    /// the current one (`Straight` stays `Straight`), else its default.
    func resized(to newPart: CatalogPart) -> PlacedChip {
        let package = footprintChoice?.package
        let footprint = newPart.footprintChoices.first { $0.package == package }?.footprint.id
        return PlacedChip(id: id, reference: reference, part: newPart, symbol: symbol, origin: origin,
                          isFlipped: isFlipped, switchesOn: switchesOn, board: board,
                          memoryFile: memoryFile, color: color, footprintID: footprint, gender: gender,
                          ohms: ohms)
    }

    /// The chosen footprint choice, for a part with footprint choices.
    var footprintChoice: CatalogPart.FootprintChoice? {
        guard let footprintID else { return nil }
        return part.footprintChoices.first { $0.footprint.id == footprintID }
    }

    /// A tactile switch is held down (`switchesOn[0]`); false for others.
    var isPressed: Bool {
        part.isTactileSwitch && switchesOn.first == true
    }

    /// A tactile switch's contact is closed: pressed on a Normal Open part,
    /// released on a Normal Close one.
    var tactileContactClosed: Bool {
        isPressed != part.isNormallyClosed
    }

    /// A tactile switch's button, in canvas coordinates.
    var buttonRect: CGRect? {
        guard symbol == .tactileSwitch else { return nil }
        return ChipLayout.tactileButtonRect(bodySize: layout.size).offsetBy(dx: origin.x, dy: origin.y)
    }

    /// Whether `point` is on a tactile switch's button (with the DIP lever's
    /// 4 x 3 pt slop).
    func buttonHit(at point: CGPoint) -> Bool {
        buttonRect?.insetBy(dx: -4, dy: -3).contains(point) ?? false
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
    /// The pin's position in its part's pinout (`ChipPin.index`).
    let index: Int
}

/// A pin on a bus port: the port, and the bus wire the pin is for.
struct BusPinRef: Hashable {
    let portID: UUID
    let wireID: UUID
}

/// One end of a wire: a pin on a placed part, a junction where wires meet,
/// or a pin on a bus port.
enum WireEnd: Hashable {
    case pin(PinRef)
    case junction(UUID)
    case busPin(BusPinRef)
}

/// A grid point where wires meet. Wires that branch from a point on another
/// wire both end here, so the point is fixed in place.
struct Junction: Identifiable, Hashable {
    let id: UUID
    /// Moves only with a group drag (plan-15); always on the grid.
    var point: CGPoint

    init(id: UUID = UUID(), point: CGPoint) {
        self.id = id
        self.point = point
    }
}

/// Where a line can start or end in Line mode: a pin, a bus port pin, or a
/// grid point that an existing wire passes through.
enum LineAnchor: Hashable {
    case pin(PinRef)
    case point(CGPoint)
    case busPin(BusPinRef)
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

/// The direction of a straight run: along a row or along a column.
nonisolated enum RouteAxis: Hashable {
    case horizontal
    case vertical

    var other: RouteAxis { self == .horizontal ? .vertical : .horizontal }
}

/// Right-angle routing helpers. Paths are arrays of grid points where
/// consecutive points share an x or a y.
nonisolated enum OrthogonalRoute {
    /// Pin to pin with both pins facing sideways: out horizontally, a
    /// vertical run at the midpoint, then horizontally into the target.
    static func route(from a: CGPoint, to b: CGPoint) -> [CGPoint] {
        normalize([a] + leg(from: a, to: b, first: .horizontal, last: .horizontal, preferred: .horizontal))
    }

    /// One drawn leg from `p` to `q`: the points after `p`, up to and
    /// including `q`. A straight line when the two are aligned, else an L.
    /// `first` and `last` are the axes a pin forces on the leg's first and
    /// last segment. When both are forced along the same axis no L fits, so
    /// the leg is a Z with its middle run halfway. `preferred` picks the
    /// L's first segment when nothing forces it.
    static func leg(from p: CGPoint, to q: CGPoint,
                    first: RouteAxis?, last: RouteAxis?,
                    preferred: RouteAxis) -> [CGPoint] {
        if p == q { return [] }
        if p.x == q.x || p.y == q.y { return [q] }
        let start: RouteAxis
        switch (first, last) {
        case let (first?, last?) where first == last:
            if first == .horizontal {
                let midX = SchematicGrid.snap((p.x + q.x) / 2)
                return mergeCoincident([p, CGPoint(x: midX, y: p.y), CGPoint(x: midX, y: q.y), q]).filter { $0 != p }
            }
            let midY = SchematicGrid.snap((p.y + q.y) / 2)
            return mergeCoincident([p, CGPoint(x: p.x, y: midY), CGPoint(x: q.x, y: midY), q]).filter { $0 != p }
        case let (first?, _):
            start = first
        case let (nil, last?):
            start = last.other
        case (nil, nil):
            start = preferred
        }
        return start == .horizontal ? [CGPoint(x: q.x, y: p.y), q] : [CGPoint(x: p.x, y: q.y), q]
    }

    /// A drawn route cleaned up: coincident points merged, double-backs
    /// dropped, and any bend in the middle of a straight run removed. The
    /// first and last points stay.
    static func simplified(_ path: [CGPoint]) -> [CGPoint] {
        var result = removeDoubleBacks(mergeCoincident(path))
        var index = 1
        while index + 1 < result.count {
            let a = result[index - 1], b = result[index], c = result[index + 1]
            let straight = (a.y == b.y && b.y == c.y) || (a.x == b.x && b.x == c.x)
            if straight {
                result.remove(at: index)
            } else {
                index += 1
            }
        }
        return result
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

        return Array(removeDoubleBacks(mergeCoincident(points)).dropFirst().dropLast())
    }

    /// Drops a vertex where the route turns straight back on itself (three
    /// points on one line, the middle one outside the other two). That is
    /// what an end moving past a stored vertex leaves behind. The first and
    /// last points never go.
    static func removeDoubleBacks(_ points: [CGPoint]) -> [CGPoint] {
        var result = points
        var index = 1
        while index + 1 < result.count {
            let a = result[index - 1], b = result[index], c = result[index + 1]
            let sameRow = a.y == b.y && b.y == c.y
            let sameColumn = a.x == b.x && b.x == c.x
            let turnsBack = (sameRow && (b.x - a.x) * (c.x - b.x) < 0)
                || (sameColumn && (b.y - a.y) * (c.y - b.y) < 0)
            if turnsBack {
                result.remove(at: index)
                result = mergeCoincident(result)
                index = max(index - 1, 1)
            } else {
                index += 1
            }
        }
        return result
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

/// The schematic being edited: placed blocks, wires, and each page's selection.
@MainActor
@Observable
final class SchematicDocument {
    private(set) var chips: [PlacedChip] = []
    private(set) var wires: [Wire] = []
    private(set) var junctions: [Junction] = []
    /// Buses, in drawing order (plan-13). They carry no current themselves.
    private(set) var buses: [Bus] = []
    /// Text blocks, in drawing order. Drawn under everything else.
    private(set) var notes: [SchematicNote] = []
    /// A text block whose words the sidebar should start editing: one just
    /// placed, or double-clicked. The sidebar clears it. Editor state only.
    var editingNoteID: UUID?
    /// What is selected on the Schematic page: parts, wires and junctions
    /// (plan-15). Editor state only, never saved. Changing it clears the bus
    /// and port selection.
    private(set) var schematicSelection = ItemSelection<SchematicItem>() {
        didSet {
            if !schematicSelection.isEmpty { clearSelection(except: .items) }
            mirrorSchematicSelectionToBoard()
        }
    }
    /// What is selected on the PCB page: footprints, board texts and traces.
    /// A part and its footprint are one selection with the Schematic page
    /// (see `mirrorSchematicSelectionToBoard`); the rest is kept apart.
    private(set) var boardSelection = ItemSelection<BoardItem>() {
        didSet { mirrorBoardSelectionToSchematic() }
    }

    /// Set while one page's selection is being copied to the other, so the
    /// copy does not copy back.
    private var isMirroringSelection = false

    /// The parts selected on the Schematic page.
    private var schematicSelectedParts: Set<UUID> {
        Set(schematicSelection.items.compactMap { if case let .chip(id) = $0 { id } else { nil } })
    }

    /// The parts whose footprint is selected on the PCB page.
    private var boardSelectedParts: Set<UUID> {
        Set(boardSelection.items.compactMap { if case let .footprint(id) = $0 { id } else { nil } })
    }

    /// Selecting a part on the Schematic page selects its footprint on the
    /// PCB page. When the two pages disagree about which parts are selected,
    /// the PCB selection becomes just those footprints (a part without a
    /// footprint has none). Texts and traces are left alone while the parts
    /// agree.
    private func mirrorSchematicSelectionToBoard() {
        guard !isMirroringSelection else { return }
        let parts = schematicSelectedParts
        let withFootprint = Set(chips.lazy.filter { $0.board != nil }.map(\.id))
        let footprints = parts.intersection(withFootprint)
        guard footprints != boardSelectedParts else { return }
        isMirroringSelection = true
        defer { isMirroringSelection = false }
        boardSelection.set(Set(footprints.map(BoardItem.footprint)))
    }

    /// Selecting a footprint on the PCB page selects its part on the
    /// Schematic page, as `mirrorSchematicSelectionToBoard`. A text or trace
    /// has no part, so selecting one leaves no part selected.
    private func mirrorBoardSelectionToSchematic() {
        guard !isMirroringSelection else { return }
        let parts = boardSelectedParts
        guard parts != schematicSelectedParts else { return }
        isMirroringSelection = true
        defer { isMirroringSelection = false }
        schematicSelection.set(Set(parts.map(SchematicItem.chip)))
    }

    /// The selected bus. Selecting one clears the other Schematic selections.
    var selectedBusID: UUID? {
        didSet {
            if selectedBusID != nil { clearSelection(except: .bus) }
        }
    }
    /// The selected bus port. Selecting one clears the other Schematic selections.
    var selectedPortID: UUID? {
        didSet {
            if selectedPortID != nil { clearSelection(except: .port) }
        }
    }

    private enum SelectionKind { case items, bus, port }

    /// On the Schematic page, the item set, a bus and a port exclude each
    /// other: clears every one but `kept`.
    private func clearSelection(except kept: SelectionKind) {
        if kept != .items, !schematicSelection.isEmpty { schematicSelection.clear() }
        if kept != .bus, selectedBusID != nil { selectedBusID = nil }
        if kept != .port, selectedPortID != nil { selectedPortID = nil }
    }

    /// The part, when it is the only thing selected on the Schematic page.
    var selectedChipID: UUID? {
        if case let .chip(id)? = schematicSelection.sole { return id }
        return nil
    }

    /// The part whose footprint is the only thing selected on the PCB page.
    var selectedFootprintID: UUID? {
        if case let .footprint(id)? = boardSelection.sole { return id }
        return nil
    }

    /// The text block, when it is the only thing selected on the Schematic page.
    var selectedNoteID: UUID? {
        if case let .note(id)? = schematicSelection.sole { return id }
        return nil
    }

    /// The board text, when it is the only thing selected on the PCB page.
    var selectedTextID: UUID? {
        if case let .text(id)? = boardSelection.sole { return id }
        return nil
    }

    /// The trace, when it is the only thing selected on the PCB page.
    var selectedTraceID: UUID? {
        if case let .trace(id)? = boardSelection.sole { return id }
        return nil
    }

    // MARK: Selecting

    /// Selects only `item` on the Schematic page.
    func select(_ item: SchematicItem) {
        schematicSelection.replace(with: item)
    }

    /// Selects only `item` on the PCB page.
    func select(_ item: BoardItem) {
        boardSelection.replace(with: item)
    }

    func toggleSelection(_ item: SchematicItem) {
        schematicSelection.toggle(item)
    }

    func toggleSelection(_ item: BoardItem) {
        boardSelection.toggle(item)
    }

    func isSelected(_ item: SchematicItem) -> Bool {
        schematicSelection.contains(item)
    }

    func isSelected(_ item: BoardItem) -> Bool {
        boardSelection.contains(item)
    }

    /// A Pointer press on `item`; see `ItemSelection.press`.
    func pressSelection(_ item: SchematicItem, shift: Bool) -> SelectionPress<SchematicItem> {
        schematicSelection.press(item, shift: shift)
    }

    /// A Select press on `item`; see `ItemSelection.press`.
    func pressSelection(_ item: BoardItem, shift: Bool) -> SelectionPress<BoardItem> {
        boardSelection.press(item, shift: shift)
    }

    func releaseSelection(_ press: SelectionPress<SchematicItem>, wasClick: Bool) {
        schematicSelection.release(press, wasClick: wasClick)
    }

    func releaseSelection(_ press: SelectionPress<BoardItem>, wasClick: Bool) {
        boardSelection.release(press, wasClick: wasClick)
    }

    /// A Pointer press on empty canvas, which pans: it clears the Schematic
    /// selection, unless Shift is down.
    func pressEmptySchematic(shift: Bool) {
        if !shift { clearSchematicSelection() }
    }

    /// A Select press on empty board, which pans: it clears the PCB
    /// selection, unless Shift is down.
    func pressEmptyBoard(shift: Bool) {
        if !shift { clearBoardSelection() }
    }

    // MARK: Box select

    /// The parts, junctions and wires a box (schematic coordinates) touches.
    func schematicItems(in rect: CGRect) -> Set<SchematicItem> {
        var items = Set<SchematicItem>()
        for chip in chips where chip.hitRect.intersects(rect) {
            items.insert(.chip(chip.id))
        }
        for junction in junctions where rect.contains(junction.point) {
            items.insert(.junction(junction.id))
        }
        for note in notes where note.rect.intersects(rect) {
            items.insert(.note(note.id))
        }
        for wire in wires {
            let path = path(forWire: wire.id)
            if zip(path, path.dropFirst()).contains(where: { Self.segment($0, $1, touches: rect) }) {
                items.insert(.wire(wire.id))
            }
        }
        return items
    }

    /// The drawn footprints, texts and traces a box (board mm) touches.
    func boardItems(in rect: CGRect, visible: BoardVisibility = BoardVisibility()) -> Set<BoardItem> {
        var items = Set<BoardItem>()
        for chip in chips where chip.board != nil && visible.showsFootprint(chip) {
            if let bounds = chip.boardBounds, bounds.intersects(rect) { items.insert(.footprint(chip.id)) }
        }
        for text in texts where visible.showsText(text) && text.boardRect.intersects(rect) {
            items.insert(.text(text.id))
        }
        for trace in traces where board.copperLayers.contains(trace.layer) && visible.showsTrace(on: trace.layer) {
            let grown = rect.insetBy(dx: -trace.widthMM / 2, dy: -trace.widthMM / 2)
            if trace.segments.contains(where: { Self.segment($0.0, $0.1, touches: grown) }) {
                items.insert(.trace(trace.id))
            }
        }
        return items
    }

    /// A Shift-drag on empty Schematic canvas: the selection is what it was
    /// at the press (`base`) plus everything the box touches.
    func boxSelectSchematic(_ rect: CGRect, base: Set<SchematicItem>) {
        let items = base.union(schematicItems(in: rect))
        if items != schematicSelection.items { schematicSelection.set(items) }
    }

    /// A Shift-drag on empty board: as `boxSelectSchematic`, on the PCB page.
    func boxSelectBoard(_ rect: CGRect, base: Set<BoardItem>, visible: BoardVisibility = BoardVisibility()) {
        let items = base.union(boardItems(in: rect, visible: visible))
        if items != boardSelection.items { boardSelection.set(items) }
    }

    /// Clears the Schematic page's selection: items, bus and port.
    func clearSchematicSelection() {
        schematicSelection.clear()
        selectedBusID = nil
        selectedPortID = nil
    }

    func clearBoardSelection() {
        boardSelection.clear()
    }

    /// Clears both pages' selections (opening a file, starting a run).
    func clearAllSelections() {
        clearSchematicSelection()
        clearBoardSelection()
    }

    /// Drops selected ids that are no longer in the document.
    private func pruneSelection() {
        let chipIDs = Set(chips.map(\.id)), wireIDs = Set(wires.map(\.id))
        let junctionIDs = Set(junctions.map(\.id)), textIDs = Set(texts.map(\.id))
        let noteIDs = Set(notes.map(\.id))
        schematicSelection.filter { item in
            switch item {
            case let .chip(id): chipIDs.contains(id)
            case let .wire(id): wireIDs.contains(id)
            case let .junction(id): junctionIDs.contains(id)
            case let .note(id): noteIDs.contains(id)
            }
        }
        let footprints = Set(chips.filter { $0.board != nil }.map(\.id))
        let traceIDs = Set(traces.map(\.id))
        boardSelection.filter { item in
            switch item {
            case let .footprint(id): footprints.contains(id)
            case let .text(id): textIDs.contains(id)
            case let .trace(id): traceIDs.contains(id)
            }
        }
    }

    /// The PCB's size, layers, colour and units.
    var board = BoardSettings()
    /// Free texts on the PCB, in drawing order.
    private(set) var texts: [BoardText] = []
    /// Copper traces on the PCB, in drawing order (plan-16).
    private(set) var traces: [BoardTrace] = []

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
    func replaceContents(chips: [PlacedChip], wires: [Wire], junctions: [Junction], buses: [Bus],
                         nextReference: Int, board: BoardSettings, texts: [BoardText],
                         traces: [BoardTrace], notes: [SchematicNote] = []) {
        self.chips = chips
        self.notes = notes
        self.wires = wires
        self.junctions = junctions
        self.buses = buses
        self.nextReference = max(nextReference, 1)
        self.board = board
        self.texts = texts
        self.traces = traces
        editingNoteID = nil
        clearAllSelections()
        editorMessage = nil
    }

    // MARK: Chips

    /// Places a block with its centre under `point`, snapped to the grid.
    /// The new part is not selected, so the parts library stays open for
    /// the next drop.
    /// `color`, `footprintID` and `ohms` are the part's options (LED, parts
    /// with footprint choices, pull resistors); nil (or a value the part
    /// doesn't have) is the part's default.
    func placeChip(part: CatalogPart, symbol: PartSymbol, centeredAt point: CGPoint,
                   color: String? = nil, footprintID: String? = nil, gender: String? = nil,
                   ohms: Double? = nil) {
        let layout = ChipLayout(part: part, symbol: symbol)
        let origin = SchematicGrid.snap(
            CGPoint(x: point.x - layout.size.width / 2, y: point.y - layout.size.height / 2)
        )
        let used = Set(chips.map(\.reference))
        let reference: String
        if part.isPullResistor {
            // A pull resistor is the lowest R<n> no part uses.
            var number = 1
            while used.contains("R\(number)") {
                number += 1
            }
            reference = "R\(number)"
        } else {
            // The first U<n> from nextReference on that no part uses, since a
            // part may have been renamed to one.
            var number = nextReference
            while used.contains("U\(number)") {
                number += 1
            }
            reference = "U\(number)"
            nextReference = number + 1
        }
        var placed = PlacedChip(
            reference: reference,
            part: part,
            symbol: symbol,
            origin: origin,
            color: color,
            footprintID: footprintID,
            gender: gender,
            ohms: ohms
        )
        placed.board = placed.footprint.map { BoardLayout.nextSpot(for: $0, among: chips) }
        chips.append(placed)
    }

    /// Sets an LED's color. Does nothing for a part that isn't an LED, or a
    /// color the part doesn't have.
    func setPartColor(_ id: UUID, _ color: String) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              chips[index].part.kind == PART_LED,
              chips[index].part.colors.contains(color) else { return }
        chips[index].color = color
    }

    /// Sets a header's gender. Does nothing for a part that isn't a header,
    /// or a gender the part doesn't list.
    func setPartGender(_ id: UUID, _ gender: String) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              chips[index].part.kind == PART_HEADER,
              chips[index].part.genders.contains(gender) else { return }
        chips[index].gender = gender
    }

    /// Changes a header's size by swapping its part for `newPart` (the
    /// caller looks up `HeaderFamily.name(rows:perRow:)` in its catalog).
    /// Reference, origin, flip, gender, footprint type and board placement
    /// are kept, and on the PCB pad 1 keeps its board point. Wires on pins
    /// the new size doesn't have are removed; the rest are re-fitted. Does
    /// nothing unless both are headers and `newPart` is another size.
    func setHeaderSize(_ id: UUID, to newPart: CatalogPart) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              chips[index].part.kind == PART_HEADER, newPart.kind == PART_HEADER,
              chips[index].part != newPart else { return }
        let old = chips[index]
        let pad1 = old.part.firstPinDesignator
        let anchor = old.board.flatMap { board in
            old.footprint?.pads.first { $0.designator == pad1 }.map { board.board($0.centre) }
        }
        chips[index] = old.resized(to: newPart)
        let pinCount = newPart.pins.count
        let before = wires.count
        wires.removeAll { wire in
            [wire.start, wire.end].contains { end in
                if case let .pin(ref) = end { return ref.chipID == id && ref.index >= pinCount }
                return false
            }
        }
        if wires.count != before { pruneJunctions() }
        if let anchor, var board = chips[index].board,
           let pad = chips[index].footprint?.pads.first(where: { $0.designator == pad1 }) {
            let landed = board.board(pad.centre)
            board.centre.x += anchor.x - landed.x
            board.centre.y += anchor.y - landed.y
            chips[index].board = board
        }
        settleWires(attachedTo: id)
    }

    /// Sets a pull resistor's resistance. Does nothing for a part that isn't
    /// a pull resistor, or a value that isn't `ResistorValue.isValid`.
    func setPartOhms(_ id: UUID, _ ohms: Double) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              chips[index].part.isPullResistor,
              ResistorValue.isValid(ohms) else { return }
        chips[index].ohms = ohms
    }

    /// Picks one of a part's footprint choices. On the PCB the pad of the
    /// part's first pin keeps its board point (no re-snap, like
    /// `setFootprintStyle`); rotation, origin, marks and style are kept. Does
    /// nothing for an id that isn't one of the part's choices, or the current one.
    func setPartFootprint(_ id: UUID, _ footprintID: String) {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              chips[index].footprintID != footprintID,
              chips[index].part.footprintChoices.contains(where: { $0.footprint.id == footprintID })
        else { return }
        let before = chips[index].footprint
        chips[index].footprintID = footprintID
        guard var changed = chips[index].board, let before, let after = chips[index].footprint else { return }
        let pad1 = chips[index].part.firstPinDesignator
        if let old = before.pads.first(where: { $0.designator == pad1 }),
           let new = after.pads.first(where: { $0.designator == pad1 }) {
            let anchor = changed.board(old.centre)
            let landed = changed.board(new.centre)
            changed.centre.x += anchor.x - landed.x
            changed.centre.y += anchor.y - landed.y
        }
        chips[index].board = changed
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

    /// The junction within `tolerance` of `point`, if any.
    func junctionHit(at point: CGPoint, tolerance: CGFloat = 6) -> Junction? {
        junctions.last { Self.distance(from: point, to: $0.point) <= tolerance }
    }

    // MARK: Group drags (Schematic)

    /// Where every selected part, junction and wire is now, for a group drag.
    func schematicGroupBase() -> SchematicGroupBase {
        var base = SchematicGroupBase()
        for item in schematicSelection.items {
            switch item {
            case let .chip(id):
                if let chip = chips.first(where: { $0.id == id }) { base.chips[id] = chip.origin }
            case let .junction(id):
                if let junction = junctions.first(where: { $0.id == id }) { base.junctions[id] = junction.point }
            case let .wire(id):
                if let wire = wires.first(where: { $0.id == id }) { base.wires[id] = wire.interior }
            case let .note(id):
                if let note = notes.first(where: { $0.id == id }) { base.notes[id] = note.origin }
            }
        }
        for wire in wires where base.wires[wire.id] != nil || moves(wire.start, in: base) || moves(wire.end, in: base) {
            base.affectedWires.insert(wire.id)
            if OrthogonalRoute.isOrthogonal(path(forWire: wire.id)) {
                base.orthogonalWires.insert(wire.id)
            }
        }
        return base
    }

    /// Whether a group drag moves this wire end: a pin of a moved part, or a
    /// moved junction.
    private func moves(_ end: WireEnd, in base: SchematicGroupBase) -> Bool {
        switch end {
        case let .pin(ref): base.chips[ref.chipID] != nil
        case let .junction(id): base.junctions[id] != nil
        case .busPin: false
        }
    }

    /// One frame of a group drag: every part, junction and selected wire bend
    /// in `base` moves by `delta` snapped to the grid, from where it was at
    /// the press. Selected wires keep their ends: only their bends move.
    func moveSchematicGroup(_ base: SchematicGroupBase, by delta: CGSize) {
        let dx = SchematicGrid.snap(delta.width), dy = SchematicGrid.snap(delta.height)
        func shifted(_ point: CGPoint) -> CGPoint { CGPoint(x: point.x + dx, y: point.y + dy) }
        for index in chips.indices {
            guard let origin = base.chips[chips[index].id] else { continue }
            let moved = shifted(origin)
            if chips[index].origin != moved { chips[index].origin = moved }
        }
        for index in junctions.indices {
            guard let point = base.junctions[junctions[index].id] else { continue }
            let moved = shifted(point)
            if junctions[index].point != moved { junctions[index].point = moved }
        }
        for index in wires.indices {
            guard let bends = base.wires[wires[index].id] else { continue }
            let moved = bends.map(shifted)
            if wires[index].interior != moved { wires[index].interior = moved }
        }
        for index in notes.indices {
            guard let origin = base.notes[notes[index].id] else { continue }
            let moved = shifted(origin)
            if notes[index].origin != moved { notes[index].origin = moved }
        }
    }

    /// The end of a group drag: re-fits each affected wire once, as
    /// `settleWires(attachedTo:)` does. A wire that was all right angles at
    /// the press gets corners wherever the move left a slanted segment.
    func settleSchematicGroup(_ base: SchematicGroupBase) {
        for index in wires.indices where base.affectedWires.contains(wires[index].id) {
            guard let start = endTip(wires[index].start), let end = endTip(wires[index].end) else { continue }
            var interior = OrthogonalRoute.settled(start: start, interior: wires[index].interior, end: end)
            if base.orthogonalWires.contains(wires[index].id) {
                interior = Array(OrthogonalRoute.normalize([start] + interior + [end]).dropFirst().dropLast())
            }
            wires[index].interior = interior
        }
    }

    // MARK: Text blocks (Schematic)

    /// Adds a text block with its top-left at `point` (snapped), selects it
    /// and asks the sidebar to edit its words.
    @discardableResult
    func addNote(at point: CGPoint) -> UUID {
        let note = SchematicNote(id: UUID(), text: SchematicNote.placeholder, origin: SchematicGrid.snap(point))
        notes.append(note)
        select(.note(note.id))
        editingNoteID = note.id
        return note.id
    }

    /// Changes a block's words. An empty string is refused and the words kept.
    func setNoteText(_ id: UUID, _ value: String) {
        guard !value.isEmpty, let index = notes.firstIndex(where: { $0.id == id }),
              notes[index].text != value else { return }
        notes[index].text = value
    }

    /// Sets a block's font size in points, clamped to `SchematicNote.sizeRange`.
    func setNoteSize(_ id: UUID, _ size: Int) {
        guard let index = notes.firstIndex(where: { $0.id == id }) else { return }
        notes[index].size = min(max(size, SchematicNote.sizeRange.lowerBound), SchematicNote.sizeRange.upperBound)
    }

    func setNoteFramed(_ id: UUID, _ framed: Bool) {
        guard let index = notes.firstIndex(where: { $0.id == id }) else { return }
        notes[index].framed = framed
    }

    func deleteNote(_ id: UUID) {
        notes.removeAll { $0.id == id }
        if editingNoteID == id { editingNoteID = nil }
        pruneSelection()
    }

    /// The text block under a point: the last one drawn there.
    func note(at point: CGPoint) -> SchematicNote? {
        notes.last { $0.rect.contains(point) }
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

    /// Moves a footprint to the other side of the board, mirrored. Its origin
    /// point stays where it was, then snaps, as in `rotateFootprint`. Does
    /// nothing on a 1-layer board or when the side doesn't change.
    func setFootprintSide(_ id: UUID, _ side: BoardSide) {
        guard board.layers > 1,
              let index = chips.firstIndex(where: { $0.id == id }),
              let placement = chips[index].board, placement.side != side,
              let footprint = chips[index].footprint else { return }
        let pad1 = chips[index].part.firstPinDesignator
        let before = BoardLayout.originPoint(of: placement, footprint: footprint, pad1: pad1)
        var flipped = placement
        flipped.side = side
        let after = BoardLayout.originPoint(of: flipped, footprint: footprint, pad1: pad1)
        flipped.centre.x += before.x - after.x
        flipped.centre.y += before.y - after.y
        chips[index].board = BoardLayout.snapped(flipped, footprint: footprint, pad1: pad1)
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

    // MARK: Board layers

    /// Why the board can't have `count` copper layers, or nil when it can.
    /// A count is refused when it would drop a layer that holds something.
    func canSetLayerCount(_ count: Int) -> String? {
        guard BoardSettings.layerCounts.contains(count) else { return "Boards have 1, 2, 4 or 6 layers." }
        let dropped = Set(board.copperLayers).subtracting(CopperLayer.allCases(for: count))
        if dropped.contains(.bottom),
           chips.contains(where: { $0.board?.side == .bottom }) || texts.contains(where: { $0.side == .bottom }) {
            return "Move every part and text to Top first."
        }
        if let layer = board.copperLayers.first(where: { layer in
            dropped.contains(layer) && traces.contains { $0.layer == layer }
        }) {
            return "Delete or move the traces on \(layer.title) first."
        }
        return nil
    }

    /// Sets the copper layer count, unless `canSetLayerCount` refuses it.
    func setLayerCount(_ count: Int) {
        guard count != board.layers, canSetLayerCount(count) == nil else { return }
        board.layers = count
    }

    func setLayerColor(_ layer: CopperLayer, _ color: LayerColor) {
        guard board.copperLayers.contains(layer) else { return }
        var style = board.style(of: layer)
        guard style.color != color else { return }
        style.color = color
        board.styles[layer] = style
    }

    /// Why `layer` can't become `type`, or nil when it can. Top and Bottom
    /// are always Component; an inner layer is Plane or Wire.
    func canSetLayerType(_ layer: CopperLayer, _ type: LayerType) -> String? {
        guard board.copperLayers.contains(layer) else { return "\(layer.title) is not on this board." }
        guard layer.isInner else { return "\(layer.title) is always a component layer." }
        guard type != .component else { return "Inner layers are planes or wires." }
        if type == .plane, traces.contains(where: { $0.layer == layer }) {
            return "Move or delete this layer's traces first."
        }
        return nil
    }

    func setLayerType(_ layer: CopperLayer, _ type: LayerType) {
        guard canSetLayerType(layer, type) == nil else { return }
        var style = board.style(of: layer)
        guard style.type != type else { return }
        style.type = type
        board.styles[layer] = style
    }

    // MARK: Board texts

    /// Adds a "Text" on the silkscreen with its top-left at `point` (snapped)
    /// and selects it.
    @discardableResult
    func addText(at point: CGPoint) -> UUID {
        let text = BoardText(id: UUID(), text: "Text", anchor: BoardGrid.snap(point), size: BoardText.defaultSize)
        texts.append(text)
        select(.text(text.id))
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

    /// Puts a text on the top or bottom of the board. Its box stays where it
    /// is. Does nothing on a 1-layer board, except to bring a text back to Top.
    func setTextSide(_ id: UUID, _ side: BoardSide) {
        guard side == .top || board.layers > 1,
              let index = texts.firstIndex(where: { $0.id == id }), texts[index].side != side else { return }
        texts[index].side = side
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
        pruneSelection()
    }

    /// The text under a board point (mm): the last one drawn there, a Top
    /// text before a Bottom one. A text that isn't drawn can't be hit.
    func text(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> BoardText? {
        for side in [BoardSide.top, .bottom] {
            if let hit = texts.last(where: {
                $0.side == side && visible.showsText($0) && $0.boardRect.contains(point)
            }) { return hit }
        }
        return nil
    }

    /// The footprint under a board point (mm): the last one drawn there, a
    /// Top footprint before a Bottom one. It is hit by its bounds while
    /// anything of it is drawn.
    func footprint(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> PlacedChip? {
        for side in [BoardSide.top, .bottom] {
            if let hit = chips.last(where: {
                $0.board?.side == side && ($0.boardBounds?.contains(point) ?? false) && visible.showsFootprint($0)
            }) { return hit }
        }
        return nil
    }


    // MARK: Traces (PCB)

    /// The pads on `layer`, with their parts: a surface-mount pad is on its
    /// part's side layer; a through-hole pad is on every layer.
    func pads(on layer: CopperLayer) -> [(chip: PlacedChip, pad: PlacedPad)] {
        chips.flatMap { chip -> [(chip: PlacedChip, pad: PlacedPad)] in
            guard let side = chip.board?.side else { return [] }
            return chip.boardPads
                .filter { $0.drill > 0 || side.copperLayer == layer }
                .map { (chip: chip, pad: $0) }
        }
    }

    /// Where a trace end pressed or released at `point` lands on `layer`:
    /// the centre of a pad there, else a vertex of a trace on that layer
    /// within `tolerance`, else the nearest point of a trace segment within
    /// `tolerance`, else the nearest grid point.
    func traceAnchor(at point: CGPoint, layer: CopperLayer, tolerance: CGFloat) -> CGPoint {
        if let hit = pads(on: layer).last(where: { $0.pad.rect.contains(point) }) {
            return CGPoint(x: hit.pad.rect.midX, y: hit.pad.rect.midY)
        }
        let same = traces.filter { $0.layer == layer }
        var best: (point: CGPoint, distance: CGFloat)?
        for trace in same {
            for vertex in trace.points {
                let d = Self.distance(from: point, to: vertex)
                if d <= tolerance, d < best?.distance ?? .infinity { best = (vertex, d) }
            }
        }
        if let best { return best.point }
        for trace in same {
            for (a, b) in trace.segments {
                let near = Self.nearest(on: a, b, to: point)
                let d = Self.distance(from: point, to: near)
                if d <= tolerance, d < best?.distance ?? .infinity { best = (near, d) }
            }
        }
        return best?.point ?? BoardGrid.snap(point)
    }

    /// Why no trace can go on `layer`, or nil when one can.
    private func traceLayerRefusal(_ layer: CopperLayer) -> String? {
        guard board.copperLayers.contains(layer) else { return "\(layer.title) is not on this board." }
        guard board.style(of: layer).type != .plane else { return "\(layer.title) is a plane." }
        return nil
    }

    /// Adds a 45° trace from `a` to `b` on `layer` (`TraceRoute.route`),
    /// `BoardTrace.defaultWidth` wide, and joins it to every trace it
    /// touches there. Returns its id, or nil when nothing was added: `a`
    /// and `b` the same point, or a layer that is a plane or not on the
    /// board.
    @discardableResult
    func addTrace(from a: CGPoint, to b: CGPoint, layer: CopperLayer, diagonalFirst: Bool) -> UUID? {
        guard !TraceRoute.same(a, b), traceLayerRefusal(layer) == nil else { return nil }
        let trace = BoardTrace(id: UUID(), layer: layer, width: BoardTrace.defaultWidth,
                               points: TraceRoute.route(from: a, to: b, diagonalFirst: diagonalFirst))
        traces.append(trace)
        join(trace.id)
        return trace.id
    }

    /// Makes every point where trace `id` touches or crosses another trace
    /// on its layer a vertex of both. Inserting a vertex on a straight
    /// segment keeps its angle, so both stay legal.
    func join(_ id: UUID) {
        guard let index = traces.firstIndex(where: { $0.id == id }) else { return }
        let layer = traces[index].layer
        for other in traces.indices where other != index && traces[other].layer == layer {
            var contacts: [CGPoint] = []
            for (a, b) in traces[index].segments {
                for (c, d) in traces[other].segments {
                    contacts += Self.contacts(a, b, c, d)
                }
            }
            for point in contacts {
                traces[index].points = Self.inserting(point, into: traces[index].points)
                traces[other].points = Self.inserting(point, into: traces[other].points)
            }
        }
    }

    /// The traces on the same layer reachable from `id` through shared
    /// vertices, `id` first.
    func connectedRun(of id: UUID) -> [UUID] {
        guard let start = traces.first(where: { $0.id == id }) else { return [] }
        let same = traces.filter { $0.layer == start.layer }
        var run = [id], seen: Set<UUID> = [id], queue = [start]
        while !queue.isEmpty {
            let trace = queue.removeFirst()
            for other in same where !seen.contains(other.id) {
                let shares = trace.points.contains { p in other.points.contains { TraceRoute.same($0, p) } }
                if shares {
                    seen.insert(other.id)
                    run.append(other.id)
                    queue.append(other)
                }
            }
        }
        return run
    }

    /// The pads the traces of `run` touch on their layer, as
    /// "<reference>.<designator>" ("U1.3", or "U1.EP" for an unnumbered
    /// centre pad), sorted by reference, then designator.
    func pads(touching run: [UUID]) -> [String] {
        let members = traces.filter { run.contains($0.id) }
        guard let layer = members.first?.layer else { return [] }
        var found: [(reference: String, designator: String)] = []
        for (chip, pad) in pads(on: layer) {
            let touched = members.contains { trace in
                trace.segments.contains { Self.segment($0.0, $0.1, touches: pad.rect) }
            }
            if touched { found.append((chip.reference, pad.designator ?? "EP")) }
        }
        return found
            .sorted {
                $0.reference != $1.reference
                    ? $0.reference.localizedStandardCompare($1.reference) == .orderedAscending
                    : $0.designator.localizedStandardCompare($1.designator) == .orderedAscending
            }
            .map { "\($0.reference).\($0.designator)" }
    }

    /// Sets a trace's width in mil, clamped to `BoardTrace.widthRange`.
    func setTraceWidth(_ id: UUID, _ mil: Int) {
        guard let index = traces.firstIndex(where: { $0.id == id }) else { return }
        traces[index].width = min(max(mil, BoardTrace.widthRange.lowerBound), BoardTrace.widthRange.upperBound)
    }

    /// Moves one trace to another layer and joins it there. Ignored for a
    /// plane or a layer not on the board. Its run on the old layer stays.
    func setTraceLayer(_ id: UUID, _ layer: CopperLayer) {
        guard traceLayerRefusal(layer) == nil,
              let index = traces.firstIndex(where: { $0.id == id }), traces[index].layer != layer else { return }
        traces[index].layer = layer
        join(id)
    }

    func deleteTrace(_ id: UUID) {
        traces.removeAll { $0.id == id }
        pruneSelection()
    }

    /// Moves every vertex of the traces `ids` by `offset` (mm), then joins
    /// each one where it now lands.
    func moveTraces(_ ids: Set<UUID>, by offset: CGSize) {
        for index in traces.indices where ids.contains(traces[index].id) {
            traces[index].points = traces[index].points.map {
                CGPoint(x: $0.x + offset.width, y: $0.y + offset.height)
            }
        }
        for id in ids { join(id) }
    }

    /// The trace under a board point (mm): the last one drawn there (Top's
    /// layer first, down the stack) whose layer is shown, within half its
    /// width plus `tolerance`.
    func trace(at point: CGPoint, tolerance: CGFloat, visible: BoardVisibility = BoardVisibility()) -> BoardTrace? {
        for layer in board.copperLayers where visible.showsTrace(on: layer) {
            if let hit = traces.last(where: { trace in
                trace.layer == layer && trace.segments.contains {
                    Self.distance(from: point, toSegment: $0.0, $0.1) <= trace.widthMM / 2 + tolerance
                }
            }) { return hit }
        }
        return nil
    }

    // MARK: Trace geometry

    /// The point of segment a–b nearest `point`.
    static func nearest(on a: CGPoint, _ b: CGPoint, to point: CGPoint) -> CGPoint {
        let dx = b.x - a.x, dy = b.y - a.y
        let lengthSquared = dx * dx + dy * dy
        guard lengthSquared > 0 else { return a }
        let t = min(max(((point.x - a.x) * dx + (point.y - a.y) * dy) / lengthSquared, 0), 1)
        return CGPoint(x: a.x + t * dx, y: a.y + t * dy)
    }

    /// Where segments a–b and c–d touch: their crossing point, or both ends
    /// of a collinear overlap. Empty when they don't touch.
    static func contacts(_ a: CGPoint, _ b: CGPoint, _ c: CGPoint, _ d: CGPoint) -> [CGPoint] {
        let eps = TraceRoute.epsilon
        let r = CGPoint(x: b.x - a.x, y: b.y - a.y), s = CGPoint(x: d.x - c.x, y: d.y - c.y)
        let denom = r.x * s.y - r.y * s.x
        let lengths = hypot(r.x, r.y) * hypot(s.x, s.y)
        if abs(denom) > 1e-9 * lengths {
            let q = CGPoint(x: c.x - a.x, y: c.y - a.y)
            let t = (q.x * s.y - q.y * s.x) / denom
            let point = CGPoint(x: a.x + t * r.x, y: a.y + t * r.y)
            let onBoth = distance(from: point, toSegment: a, b) < eps && distance(from: point, toSegment: c, d) < eps
            return onBoth ? [snapToEnds(point, [a, b, c, d])] : []
        }
        // Parallel: they touch only when collinear, at the ends of each that
        // lie on the other.
        var points: [CGPoint] = []
        for (end, segment) in [(a, (c, d)), (b, (c, d)), (c, (a, b)), (d, (a, b))]
        where distance(from: end, toSegment: segment.0, segment.1) < eps {
            if !points.contains(where: { TraceRoute.same($0, end) }) { points.append(end) }
        }
        return points
    }

    /// `point`, or the end it is within epsilon of, so a touching end is
    /// shared exactly.
    private static func snapToEnds(_ point: CGPoint, _ ends: [CGPoint]) -> CGPoint {
        ends.first { TraceRoute.same($0, point) } ?? point
    }

    /// `points` with `point` added as a vertex on the segment it lies on.
    /// Unchanged when it is already a vertex or on no segment.
    static func inserting(_ point: CGPoint, into points: [CGPoint]) -> [CGPoint] {
        if points.contains(where: { TraceRoute.same($0, point) }) { return points }
        for index in points.indices.dropLast()
        where distance(from: point, toSegment: points[index], points[index + 1]) < TraceRoute.epsilon {
            var result = points
            result.insert(point, at: index + 1)
            return result
        }
        return points
    }

    /// Whether segment a–b meets `rect` (inside it, or crossing or touching
    /// its edge).
    static func segment(_ a: CGPoint, _ b: CGPoint, touches rect: CGRect) -> Bool {
        // Liang–Barsky: clip the segment's parameter range to the rect.
        var low: CGFloat = 0, high: CGFloat = 1
        let dx = b.x - a.x, dy = b.y - a.y
        for (p, q) in [(-dx, a.x - rect.minX), (dx, rect.maxX - a.x), (-dy, a.y - rect.minY), (dy, rect.maxY - a.y)] {
            if p == 0 {
                if q < 0 { return false }
            } else {
                let t = q / p
                if p < 0 { low = max(low, t) } else { high = min(high, t) }
                if low > high { return false }
            }
        }
        return true
    }

    // MARK: Group drags (PCB)

    /// Where every selected footprint (its centre) and text (its anchor) is
    /// now, for a group drag.
    func boardGroupBase() -> BoardGroupBase {
        var base = BoardGroupBase()
        for item in boardSelection.items {
            switch item {
            case let .footprint(id):
                if let centre = chips.first(where: { $0.id == id })?.board?.centre { base.footprints[id] = centre }
            case let .text(id):
                if let anchor = texts.first(where: { $0.id == id })?.anchor { base.texts[id] = anchor }
            case let .trace(id):
                if let points = traces.first(where: { $0.id == id })?.points { base.traces[id] = points }
            }
        }
        return base
    }

    /// One frame of a group drag on the PCB. One item moves as it always
    /// has, snapped onto the grid. Several all move by the same `delta`
    /// snapped to the board grid, so they keep their places relative to
    /// each other.
    func moveBoardGroup(_ base: BoardGroupBase, by delta: CGSize) {
        func moved(_ point: CGPoint, by offset: CGSize) -> CGPoint {
            CGPoint(x: point.x + offset.width, y: point.y + offset.height)
        }
        let offset = CGSize(width: BoardGrid.snap(delta.width), height: BoardGrid.snap(delta.height))
        // Traces move by the snapped delta from where they were, alone or
        // in a group; joins wait for the release (`settleBoardGroup`).
        for index in traces.indices {
            guard let points = base.traces[traces[index].id] else { continue }
            let target = points.map { moved($0, by: offset) }
            if traces[index].points != target { traces[index].points = target }
        }
        if base.count == 1 {
            if let (id, centre) = base.footprints.first { moveFootprint(id, to: moved(centre, by: delta)) }
            if let (id, anchor) = base.texts.first { moveText(id, to: moved(anchor, by: delta)) }
            return
        }
        for index in chips.indices {
            guard let centre = base.footprints[chips[index].id], chips[index].board != nil else { continue }
            let target = moved(centre, by: offset)
            if chips[index].board?.centre != target { chips[index].board?.centre = target }
        }
        for index in texts.indices {
            guard let anchor = base.texts[texts[index].id] else { continue }
            let target = moved(anchor, by: offset)
            if texts[index].anchor != target { texts[index].anchor = target }
        }
    }

    /// The end of a PCB group drag: each moved trace joins what it now
    /// touches. Traces that weren't selected stay where they are.
    func settleBoardGroup(_ base: BoardGroupBase) {
        for id in base.traces.keys { join(id) }
    }

    /// What a Select press at `point` (board mm) picks: a text, else a
    /// trace (within `tolerance` of its edge), else a footprint; Top before
    /// Bottom within each. Only drawn items count.
    func boardItem(at point: CGPoint, tolerance: CGFloat,
                   visible: BoardVisibility = BoardVisibility()) -> BoardItem? {
        if let text = text(at: point, visible: visible) { return .text(text.id) }
        if let trace = trace(at: point, tolerance: tolerance, visible: visible) { return .trace(trace.id) }
        if let chip = footprint(at: point, visible: visible), chip.board != nil { return .footprint(chip.id) }
        return nil
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
    /// Presses or releases a tactile switch (plan-27-J). Nil for any other
    /// chip, or when a press would close a contact the voltage check refuses
    /// (`editorMessage` says why); otherwise the new pressed state. Releasing
    /// never checks. Not undoable and does not mark the document changed:
    /// the pressed state is never saved.
    func setButton(_ id: UUID, pressed: Bool) -> Bool? {
        guard let index = chips.firstIndex(where: { $0.id == id }), chips[index].part.isTactileSwitch,
              !chips[index].switchesOn.isEmpty else { return nil }
        if pressed, !chips[index].part.isNormallyClosed, !chips[index].isPressed, let library {
            var candidate = chips
            candidate[index].switchesOn[0] = true
            if let issue = VoltageCheck.firstNew(before: chips, wires, after: candidate, wires,
                                                 buses: buses, library: library) {
                editorMessage = VoltageCheck.message(issue, chips: chips, edit: .turnOn)
                return nil
            }
        }
        if pressed { editorMessage = nil }
        chips[index].switchesOn[0] = pressed
        return pressed
    }

    /// Releases every pressed tactile switch (play mode ended).
    func releaseAllButtons() {
        for index in chips.indices where chips[index].isPressed {
            chips[index].switchesOn[0] = false
        }
    }

    func toggleSwitch(_ id: UUID, position: Int) -> Bool? {
        guard let index = chips.firstIndex(where: { $0.id == id }),
              chips[index].switchesOn.indices.contains(position - 1) else { return nil }
        if !chips[index].switchesOn[position - 1], let library {
            var candidate = chips
            candidate[index].switchesOn[position - 1] = true
            if let issue = VoltageCheck.firstNew(before: chips, wires, after: candidate, wires,
                                                 buses: buses, library: library) {
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

    /// Tip position of every pin on every placed chip and bus port.
    var pinTips: [CGPoint] {
        chips.flatMap { chip in
            chip.layout.slots.compactMap { slot in
                pinTip(PinRef(chipID: chip.id, index: slot.pin.index))
            }
        } + busPinRefs.compactMap { busPinTip($0) }
    }

    /// Every pin on every bus port.
    var busPinRefs: [BusPinRef] {
        buses.flatMap { bus in
            bus.ports.flatMap { port in
                bus.pinWires(of: port).map { BusPinRef(portID: port.id, wireID: $0.id) }
            }
        }
    }

    /// Where a wire meets a bus port pin; nil when the port is gone or the
    /// wire isn't ticked on it.
    func busPinTip(_ ref: BusPinRef) -> CGPoint? {
        guard let (bus, port) = port(ref.portID) else { return nil }
        return bus.pinTip(port, wire: ref.wireID)
    }

    /// The bus port pin whose tip is within `radius` of `point`.
    func busPin(at point: CGPoint, radius: CGFloat = 8) -> BusPinRef? {
        busPinRefs.reversed().first { ref in
            busPinTip(ref).map { Self.distance(from: point, to: $0) <= radius } ?? false
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
        case let .busPin(ref):
            return busPinTip(ref)
        }
    }

    /// The point a line would start or end at.
    func anchorTip(_ anchor: LineAnchor) -> CGPoint? {
        switch anchor {
        case let .pin(ref):
            return pinTip(ref)
        case let .point(point):
            return point
        case let .busPin(ref):
            return busPinTip(ref)
        }
    }

    /// The axis a wire must leave or enter `anchor` along: sideways for a
    /// pin on a part's left or right, up/down for one on its top or bottom,
    /// and across the line for a bus port pin. Nil for a point on a wire.
    func anchorAxis(_ anchor: LineAnchor) -> RouteAxis? {
        switch anchor {
        case let .pin(ref):
            guard let chip = chips.first(where: { $0.id == ref.chipID }),
                  let slot = chip.layout.slots.first(where: { $0.pin.index == ref.index }) else { return nil }
            switch slot.side {
            case .left, .right: return .horizontal
            case .top, .bottom: return .vertical
            }
        case let .busPin(ref):
            guard let (bus, port) = port(ref.portID), let line = bus.line(port.line) else { return nil }
            return line.isHorizontal ? .vertical : .horizontal
        case .point:
            return nil
        }
    }

    /// What a line can start or end on near `point`: a part pin if one is
    /// within `pinRadius`, then a bus port pin, otherwise the grid point
    /// nearest `point` when an existing wire passes through it and it is
    /// within `gridRadius`. Never a point on a bus line.
    func lineAnchor(at point: CGPoint, pinRadius: CGFloat, gridRadius: CGFloat) -> LineAnchor? {
        if let ref = pin(at: point, radius: pinRadius) {
            return .pin(ref)
        }
        if let ref = busPin(at: point, radius: pinRadius) {
            return .busPin(ref)
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
    func connect(_ start: LineAnchor, _ end: LineAnchor, path: [CGPoint]? = nil) -> String? {
        guard start != end,
              let a = anchorTip(start), let b = anchorTip(end), a != b else {
            return nil
        }

        // The check runs on stand-in ends before anything changes: resolving a
        // point on a wire for real splits that wire.
        if let library, let standInStart = standIn(for: start), let standInEnd = standIn(for: end) {
            let candidate = wires + [Wire(start: standInStart, end: standInEnd, interior: [])]
            if let issue = VoltageCheck.firstNew(before: chips, wires, after: chips, candidate,
                                                 buses: buses, library: library) {
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
        // The ends are always the tips; a drawn path gives the bends between.
        let bends = path.map { Array($0.dropFirst().dropLast()) }
            ?? Array(OrthogonalRoute.leg(from: fromTip, to: toTip, first: anchorAxis(start),
                                         last: anchorAxis(end), preferred: .horizontal).dropLast())
        let route = OrthogonalRoute.simplified([fromTip] + bends + [toTip])
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
        case let .busPin(ref):
            return .busPin(ref)
        }
    }

    /// The wire end for an anchor. Creates a junction when the anchor is a point on a wire.
    private func endpoint(for anchor: LineAnchor) -> WireEnd? {
        switch anchor {
        case let .pin(ref):
            return .pin(ref)
        case let .point(point):
            return junction(at: point).map { WireEnd.junction($0) }
        case let .busPin(ref):
            return .busPin(ref)
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

    /// Drops junctions that no wire ends at any more. Every removal of
    /// parts or wires ends here, so it also drops what left the selection.
    private func pruneJunctions() {
        junctions.removeAll { junction in
            !wires.contains { $0.start == .junction(junction.id) || $0.end == .junction(junction.id) }
        }
        pruneSelection()
    }

    /// Whether a wire end sits on a pin of the given bus port.
    private func attaches(_ end: WireEnd, toPort portID: UUID) -> Bool {
        if case let .busPin(ref) = end {
            return ref.portID == portID
        }
        return false
    }

    /// Re-fits the wires on a bus port's pins after the port changed (pins
    /// added, removed or moved across), as `settleWires(attachedTo:)` does for
    /// a part.
    func settleWires(attachedToPort portID: UUID) {
        settle { attaches($0, toPort: portID) }
    }

    /// Re-fits the wires on every port of a bus, when a bus drag ends.
    func settleWires(attachedToBus busID: UUID) {
        for port in bus(busID)?.ports ?? [] {
            settleWires(attachedToPort: port.id)
        }
    }

    /// Called when a block is released. Re-fits the wires attached to it and
    /// stores the result, so the change stays rather than being recomputed per frame.
    func settleWires(attachedTo chipID: UUID) {
        settle { attaches($0, to: chipID) }
    }

    private func settle(where touches: (WireEnd) -> Bool) {
        for index in wires.indices where touches(wires[index].start) || touches(wires[index].end) {
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

    // MARK: Buses

    func bus(_ id: UUID) -> Bus? {
        buses.first { $0.id == id }
    }

    /// Whether no line end of one bus touches a line of another.
    static func busesAreApart(_ buses: [Bus]) -> Bool {
        Bus.firstTouch(buses) == nil
    }

    /// What a Bus tool drag from `start` to `end` would make (both snapped).
    /// Changes nothing; the canvas previews it.
    func busDraft(from start: CGPoint, to end: CGPoint) -> Result<BusDraft, BusJoinRefusal> {
        BusJoin.join(buses, from: SchematicGrid.snap(start), to: SchematicGrid.snap(end))
    }

    /// What the Bus tool would make of a drawn route: the first of
    /// `candidates` (grid paths, as `RouteDraft.busCandidates` gives) the
    /// bus rules accept.
    func busDraft(candidates: [[CGPoint]]) -> Result<BusDraft, BusJoinRefusal> {
        BusJoin.join(buses, candidates: candidates)
    }

    /// The Bus tool: draws a line or an L from `start` to `end`, as a new bus
    /// or joined to the bus it starts or ends on (a T, an L, or a longer
    /// line). Selects that bus and returns its id. A refused drag changes
    /// nothing, sets `editorMessage`, and returns nil.
    @discardableResult
    func addBus(from start: CGPoint, to end: CGPoint) -> UUID? {
        applyBusDraft(busDraft(from: start, to: end))
    }

    /// The Bus tool along a drawn route: `addBus(from:to:)` for the first of
    /// `candidates` the bus rules accept.
    @discardableResult
    func addBus(candidates: [[CGPoint]]) -> UUID? {
        applyBusDraft(busDraft(candidates: candidates))
    }

    private func applyBusDraft(_ result: Result<BusDraft, BusJoinRefusal>) -> UUID? {
        switch result {
        case let .failure(refusal):
            if let message = refusal.message { editorMessage = message }
            return nil
        case let .success(draft):
            if let index = draft.busIndex {
                buses[index] = draft.bus
            } else {
                buses.append(draft.bus)
            }
            if let gone = draft.absorbedIndex {
                absorb(buses[gone], wireMap: draft.wireMap)
                buses.remove(at: gone)
            }
            editorMessage = nil
            selectedBusID = draft.bus.id
            return draft.bus.id
        }
    }

    /// After the Bus tool joined `gone` into another bus: wires on `gone`'s port
    /// pins move to the pin for the wire it became, and are re-fitted (the
    /// pins may have moved along the port).
    private func absorb(_ gone: Bus, wireMap: [UUID: UUID]) {
        let portIDs = Set(gone.ports.map(\.id))
        func remap(_ end: WireEnd) -> WireEnd {
            guard case let .busPin(ref) = end, portIDs.contains(ref.portID),
                  let wireID = wireMap[ref.wireID] else { return end }
            return .busPin(BusPinRef(portID: ref.portID, wireID: wireID))
        }
        for index in wires.indices {
            wires[index].start = remap(wires[index].start)
            wires[index].end = remap(wires[index].end)
        }
        for portID in portIDs {
            settleWires(attachedToPort: portID)
        }
    }

    /// Sets how many wires a bus carries, clamped to 2–999. New wires get
    /// fresh ids and default names; removing drops wires from the end. It
    /// can't drop a wire a port uses: that is refused with a message.
    func setBusWireCount(_ id: UUID, _ count: Int) -> String? {
        guard let index = buses.firstIndex(where: { $0.id == id }) else { return nil }
        let bus = buses[index]
        let used = bus.ports.reduce(into: Set<UUID>()) { $0.formUnion($1.wireIDs) }
        if let highest = bus.wires.lastIndex(where: { used.contains($0.id) }), count < highest + 1 {
            return "\(bus.wires[highest].name) is used by a port; this bus needs at least \(highest + 1) wires."
        }
        let wanted = min(max(count, Bus.wireCountRange.lowerBound), Bus.wireCountRange.upperBound)
        var wires = buses[index].wires
        if wanted < wires.count {
            wires.removeLast(wires.count - wanted)
        }
        while wires.count < wanted {
            wires.append(BusWire(name: Bus.defaultName(for: wires.count, taken: Set(wires.map(\.name)))))
        }
        buses[index].wires = wires
        return nil
    }

    /// Renames one wire of a bus. Returns why when the name is refused, and
    /// changes nothing then.
    func renameBusWire(bus id: UUID, wire wireID: UUID, to name: String) -> String? {
        guard let index = buses.firstIndex(where: { $0.id == id }),
              let w = buses[index].wires.firstIndex(where: { $0.id == wireID }) else { return nil }
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        if trimmed.isEmpty { return "A wire name can't be empty." }
        if trimmed.count > Bus.nameLengthRange.upperBound { return "A wire name is at most 4 characters." }
        if trimmed.contains(where: \.isWhitespace) { return "A wire name can't contain spaces." }
        let lowered = trimmed.lowercased()
        if buses[index].wires.contains(where: { $0.id != wireID && $0.name.lowercased() == lowered }) {
            return "\(trimmed) is already used on this bus."
        }
        buses[index].wires[w].name = trimmed
        return nil
    }

    // MARK: Bus ports

    /// The bus and index of a port.
    func portLocation(_ portID: UUID) -> (bus: Int, port: Int)? {
        for (b, bus) in buses.enumerated() {
            if let p = bus.ports.firstIndex(where: { $0.id == portID }) { return (b, p) }
        }
        return nil
    }

    /// A port and the bus it is on.
    func port(_ portID: UUID) -> (bus: Bus, port: BusPort)? {
        portLocation(portID).map { (buses[$0.bus], buses[$0.bus].ports[$0.port]) }
    }

    /// The Bus Port tool: adds an empty port where `point` is on a bus line,
    /// and selects it. Refused (with `editorMessage` set, and returned) on or
    /// near a line end or a T, or when one slot doesn't fit there.
    @discardableResult
    func addPort(at point: CGPoint, tolerance: CGFloat = 6) -> String? {
        guard let hit = busHit(at: point, tolerance: tolerance),
              let b = buses.firstIndex(where: { $0.id == hit.bus }),
              let line = buses[b].line(hit.line) else { return nil }
        let bus = buses[b]
        let along = line.isHorizontal ? point.x : point.y
        let blocked = [line.alongRange.lowerBound, line.alongRange.upperBound] + bus.teeAlongs(on: line.id)
        if blocked.contains(where: { abs($0 - along) <= tolerance }) {
            editorMessage = "A port can't go on the end of a bus line or on a joint."
            return editorMessage
        }
        // The default side first; if a port there is in the way, the other side.
        let defaultSide = BusPort.defaultSide(for: line)
        let sides = [defaultSide] + BusPort.sides(for: line).filter { $0 != defaultSide }
        let fitted = sides.lazy.compactMap { side -> BusPort? in
            var port = BusPort(line: line.id, start: SchematicGrid.snap(along), side: side)
            guard let start = bus.fit(port: port, slots: 1, preferredStart: port.start) else { return nil }
            port.start = start
            return port
        }.first
        guard let placed = fitted else {
            editorMessage = "Not enough room on this bus line for a port."
            return editorMessage
        }
        buses[b].ports.append(placed)
        editorMessage = nil
        selectedPortID = placed.id
        return nil
    }

    /// Sets which bus wires are pins on a port (ticks, Select All, Deselect
    /// All). More pins than fit is refused with a message and changes
    /// nothing; a port that grows may move left (up) to fit. Unticked pins
    /// take the wires on them along.
    func setPortWires(_ portID: UUID, _ ids: Set<UUID>) -> String? {
        guard let at = portLocation(portID) else { return nil }
        let bus = buses[at.bus]
        let port = bus.ports[at.port]
        let wanted = ids.intersection(bus.wires.map(\.id))
        guard let start = bus.fit(port: port, slots: wanted.count, preferredStart: port.start) else {
            return "Not enough room on this bus line for \(wanted.count) pins."
        }
        let removing = port.wireIDs.subtracting(wanted)
        if !removing.isEmpty {
            wires.removeAll {
                endsOn($0.start, port: portID, wireIDs: removing) || endsOn($0.end, port: portID, wireIDs: removing)
            }
            pruneJunctions()
        }
        buses[at.bus].ports[at.port].start = start
        buses[at.bus].ports[at.port].wireIDs = wanted
        // Pins after an added one shift by a pitch; re-fit their wires.
        settleWires(attachedToPort: portID)
        return nil
    }

    /// Sets the side a port's pins come out of, if it suits the line. Ports
    /// on opposite sides may share a place, so a port on the new side that is
    /// in the way refuses the switch with a message.
    @discardableResult
    func setPortSide(_ portID: UUID, _ side: ChipLayout.Side) -> String? {
        guard let at = portLocation(portID),
              let line = buses[at.bus].line(buses[at.bus].ports[at.port].line),
              BusPort.sides(for: line).contains(side) else { return nil }
        let bus = buses[at.bus]
        var moved = bus.ports[at.port]
        moved.side = side
        guard bus.fit(port: moved, slots: bus.slots(of: moved), preferredStart: moved.start) == moved.start else {
            return "A port on that side is in the way."
        }
        buses[at.bus].ports[at.port].side = side
        settleWires(attachedToPort: portID)
        return nil
    }

    /// Moves a port along its line to `start`. Stores it only if the fit
    /// rule keeps it unchanged (on its line, clear of the other ports and the
    /// Ts). Returns whether it did.
    @discardableResult
    func slidePort(_ portID: UUID, to start: CGFloat) -> Bool {
        guard let at = portLocation(portID) else { return false }
        let bus = buses[at.bus]
        let port = bus.ports[at.port]
        guard start != port.start else { return true }
        guard bus.fit(port: port, slots: bus.slots(of: port), preferredStart: start) == start else { return false }
        buses[at.bus].ports[at.port].start = start
        return true
    }

    /// One frame of a port slide: `base` (the start at the press) plus
    /// `along` (the drag along the line), snapped to the grid and clamped to
    /// where the port may go. It never leaves its line or passes a line
    /// end, another port or a T.
    func slidePort(_ portID: UUID, from base: CGFloat, by along: CGFloat) {
        guard let (bus, port) = port(portID), let range = bus.slideRange(of: port) else { return }
        let wanted = SchematicGrid.snap(base + along)
        slidePort(portID, to: min(max(wanted, range.lowerBound), range.upperBound))
    }

    /// The port whose body is under `point` (topmost first).
    func portHit(at point: CGPoint, tolerance: CGFloat = 0) -> UUID? {
        for bus in buses.reversed() {
            for port in bus.ports.reversed() {
                if let body = bus.body(of: port), body.insetBy(dx: -tolerance, dy: -tolerance).contains(point) {
                    return port.id
                }
            }
        }
        return nil
    }

    /// The one way a drag changes a bus. Stores `lines` only if the bus stays
    /// valid, keeps the same joints (points may move), keeps every line's
    /// direction, stays apart from other buses, and every port still fits
    /// where it is (its line long enough, no T slid into it). Ports ride
    /// along with their line. Returns whether it stored the lines.
    @discardableResult
    func setBusLines(_ id: UUID, _ lines: [BusLine]) -> Bool {
        guard let index = buses.firstIndex(where: { $0.id == id }) else { return false }
        let old = buses[index]
        guard lines.map(\.id) == old.lines.map(\.id) else { return false }
        for (before, after) in zip(old.lines, lines)
            where before.isHorizontal != after.isHorizontal || before.direction != after.direction {
            return false
        }
        var edited = old
        edited.lines = lines
        guard edited.isValid, edited.jointShapes == old.jointShapes, edited.portsFit else { return false }
        var all = buses
        all[index] = edited
        guard Self.busesAreApart(all) else { return false }
        buses[index] = edited
        return true
    }

    /// One frame of a bus drag. `step(offset)` gives the lines for a drag of
    /// `offset` from the press (against the press-time bus). The drag walks
    /// from `done` (the last offset stored) toward `target` one grid step at
    /// a time, storing each step through `setBusLines`, and stops at the
    /// first refused one. So a drag never jumps over something it may not
    /// pass, such as a T end over a port. Returns the offset reached.
    @discardableResult
    func dragBus(_ id: UUID, from done: CGSize, to target: CGSize, step: (CGSize) -> [BusLine]?) -> CGSize {
        let dx = target.width - done.width
        let dy = target.height - done.height
        let count = Int((max(abs(dx), abs(dy)) / SchematicGrid.spacing).rounded(.up))
        var reached = done
        guard count > 0 else { return done }
        for k in 1...count {
            let t = CGFloat(k) / CGFloat(count)
            let offset = CGSize(width: SchematicGrid.snap(done.width + dx * t),
                                height: SchematicGrid.snap(done.height + dy * t))
            guard offset != reached else { continue }
            guard let lines = step(offset), setBusLines(id, lines) else { break }
            reached = offset
        }
        return reached
    }

    /// The bus line under `point`, if any (topmost bus first).
    func busHit(at point: CGPoint, tolerance: CGFloat = 6) -> (bus: UUID, line: UUID)? {
        for bus in buses.reversed() {
            for line in bus.lines where Self.distance(from: point, toSegment: line.a, line.b) <= tolerance {
                return (bus.id, line.id)
            }
        }
        return nil
    }

    /// The bus line end under `point`, if any. A corner is two ends at one
    /// point; either is returned, and the drag moves both lines.
    func busEndHit(at point: CGPoint, tolerance: CGFloat = 6) -> (bus: UUID, line: UUID, end: BusEnd)? {
        for bus in buses.reversed() {
            for line in bus.lines {
                for end in BusEnd.allCases where Self.distance(from: point, to: line.point(end)) <= tolerance {
                    return (bus.id, line.id, end)
                }
            }
        }
        return nil
    }

    // MARK: Removing ports and bus lines

    /// A removal waiting for the user to confirm: it would take more than the
    /// thing clicked (wires on port pins, or a line's ports).
    struct PendingRemoval: Identifiable {
        let id = UUID()
        let message: String
        fileprivate let action: Action

        fileprivate enum Action {
            case port(UUID)
            case busLine(bus: UUID, line: UUID)
            case untick(port: UUID, keeping: Set<UUID>)
        }
    }

    /// Set while a confirm dialog is up. Erasing does nothing meanwhile, so
    /// one eraser sweep raises at most one dialog.
    private(set) var pendingRemoval: PendingRemoval?

    func confirmPendingRemoval() {
        guard let pending = pendingRemoval else { return }
        pendingRemoval = nil
        switch pending.action {
        case let .port(id): removePort(id)
        case let .busLine(bus, line): removeBusLine(bus: bus, line: line)
        case let .untick(port, keeping): _ = setPortWires(port, keeping)
        }
    }

    func cancelPendingRemoval() {
        pendingRemoval = nil
    }

    /// Whether a wire end is on one of `wireIDs`' pins of a port (any pin
    /// when `wireIDs` is nil).
    private func endsOn(_ end: WireEnd, port portID: UUID, wireIDs: Set<UUID>? = nil) -> Bool {
        guard case let .busPin(ref) = end, ref.portID == portID else { return false }
        return wireIDs.map { $0.contains(ref.wireID) } ?? true
    }

    /// How many wires end on the pins of a port that unticking `ids` (or,
    /// with nil, deleting the port) would remove.
    func wiresOnPins(port portID: UUID, removing ids: Set<UUID>? = nil) -> Int {
        wires.filter { endsOn($0.start, port: portID, wireIDs: ids) || endsOn($0.end, port: portID, wireIDs: ids) }.count
    }

    /// The port panel's ticks: like `setPortWires`, but unticking a pin that
    /// has wires asks first (through `pendingRemoval`) and returns nil.
    func requestPortWires(_ portID: UUID, _ ids: Set<UUID>) -> String? {
        guard let (_, port) = port(portID) else { return nil }
        let removing = port.wireIDs.subtracting(ids)
        let count = wiresOnPins(port: portID, removing: removing)
        if count > 0 {
            pendingRemoval = PendingRemoval(
                message: "Remove \(Self.counted(removing.count, "pin")) and the \(Self.counted(count, "wire")) connected to them?",
                action: .untick(port: portID, keeping: ids))
            return nil
        }
        return setPortWires(portID, ids)
    }

    /// Removes a port and the wires on its pins.
    private func removePort(_ portID: UUID) {
        guard let at = portLocation(portID) else { return }
        buses[at.bus].ports.remove(at: at.port)
        wires.removeAll { endsOn($0.start, port: portID) || endsOn($0.end, port: portID) }
        pruneJunctions()
        if selectedPortID == portID { selectedPortID = nil }
    }

    /// Removes one bus line, with its ports and the wires on their pins. A bus
    /// with no lines left is removed. If the rest falls into pieces, the piece
    /// holding the first remaining line keeps the bus; each other piece
    /// becomes a bus of its own with the same wire names and fresh wire ids,
    /// and its ports (and the wire ends on them) map to those ids by list
    /// position.
    private func removeBusLine(bus id: UUID, line lineID: UUID) {
        guard let index = buses.firstIndex(where: { $0.id == id }) else { return }
        var bus = buses[index]
        for port in bus.ports where port.line == lineID {
            wires.removeAll { endsOn($0.start, port: port.id) || endsOn($0.end, port: port.id) }
            if selectedPortID == port.id { selectedPortID = nil }
        }
        bus.ports.removeAll { $0.line == lineID }
        bus.lines.removeAll { $0.id == lineID }
        defer { pruneJunctions() }
        if bus.lines.isEmpty {
            buses.remove(at: index)
            if selectedBusID == id { selectedBusID = nil }
            return
        }

        let pieces = bus.pieces()
        func ports(on piece: [BusLine]) -> [BusPort] {
            let ids = Set(piece.map(\.id))
            return bus.ports.filter { ids.contains($0.line) }
        }
        var split: [Bus] = []
        for piece in pieces.dropFirst() {
            let newWires = bus.wires.map { BusWire(name: $0.name) }
            let map = Dictionary(uniqueKeysWithValues: zip(bus.wires.map(\.id), newWires.map(\.id)))
            var moved = ports(on: piece)
            for k in moved.indices {
                moved[k].wireIDs = Set(moved[k].wireIDs.compactMap { map[$0] })
            }
            let movedIDs = Set(moved.map(\.id))
            func remap(_ end: WireEnd) -> WireEnd {
                guard case let .busPin(ref) = end, movedIDs.contains(ref.portID),
                      let wireID = map[ref.wireID] else { return end }
                return .busPin(BusPinRef(portID: ref.portID, wireID: wireID))
            }
            for w in wires.indices {
                wires[w].start = remap(wires[w].start)
                wires[w].end = remap(wires[w].end)
            }
            split.append(Bus(lines: piece, wires: newWires, ports: moved))
        }
        bus.lines = pieces[0]
        bus.ports = ports(on: pieces[0])
        buses[index] = bus
        buses.insert(contentsOf: split, at: index + 1)
    }

    /// "1 wire", "3 wires".
    private static func counted(_ n: Int, _ noun: String) -> String {
        "\(n) \(noun)\(n == 1 ? "" : "s")"
    }

    // MARK: Erasing

    /// Removes whatever is under `point`: a block (and its wires) first, then
    /// a bus port, then a wire, then a bus line, then a text block. A port with wires on its
    /// pins, or a bus line with ports, asks first (`pendingRemoval`); while
    /// that is up, nothing is erased.
    func erase(at point: CGPoint, tolerance: CGFloat = 6) {
        guard pendingRemoval == nil else { return }
        if let hit = chip(at: point) {
            chips.removeAll { $0.id == hit.id }
            wires.removeAll { attaches($0.start, to: hit.id) || attaches($0.end, to: hit.id) }
        } else if let portID = portHit(at: point) {
            let count = wiresOnPins(port: portID)
            if count > 0 {
                pendingRemoval = PendingRemoval(
                    message: "Delete this port and the \(Self.counted(count, "wire")) connected to it?",
                    action: .port(portID))
            } else {
                removePort(portID)
            }
            return
        } else if let hit = segmentHit(at: point, tolerance: tolerance) {
            wires.removeAll { $0.id == hit.wire }
        } else if let hit = busHit(at: point, tolerance: tolerance), let bus = bus(hit.bus) {
            let ports = bus.ports.filter { $0.line == hit.line }
            if ports.isEmpty {
                removeBusLine(bus: hit.bus, line: hit.line)
            } else {
                let count = ports.map { wiresOnPins(port: $0.id) }.reduce(0, +)
                pendingRemoval = PendingRemoval(
                    message: "Delete this bus line, its \(Self.counted(ports.count, "port")) and the \(Self.counted(count, "wire")) connected to them?",
                    action: .busLine(bus: hit.bus, line: hit.line))
            }
            return
        } else if let hit = note(at: point) {
            deleteNote(hit.id)
            return
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
