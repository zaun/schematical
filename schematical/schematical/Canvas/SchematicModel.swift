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
    case text
    case netLabel
    case vcc
    case ground
    case pullUp
    case pullDown

    var id: String { rawValue }

    /// The toolbar's first group: editing and drawing tools.
    static let drawingTools: [SchematicTool] = [.pointer, .eraser, .line, .bus, .busPort, .text, .netLabel]

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
        case .text: "Text"
        case .netLabel: "Net Label"
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
        case .netLabel: "Net Label — click to add a label; labels with the same name are one net"
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
        case .text: "textformat"
        case .netLabel: "tag"
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

/// The supplies the VCC tool can place, in menu order (library part names),
/// and the names of the rails of any other voltage (the menu's Custom…).
nonisolated enum SupplyChoice {
    static let names = ["5V", "3V3", "2V5", "1V8", "1V2", "12V"]
    static let defaultName = "5V"
    /// The highest rail the backend loads (`supply.c`).
    static let maxVolts = 300.0

    /// The rail name of a voltage, the decimal point written as the `V`:
    /// 170 is "170V", 3.3 "3V3", 1.25 "1V25". Millivolt steps; nil outside
    /// 0 < volts <= `maxVolts`.
    static func name(volts: Double) -> String? {
        guard volts.isFinite else { return nil }
        let mv = Int((volts * 1000).rounded())
        guard mv > 0, mv <= Int(maxVolts * 1000) else { return nil }
        var fraction = String(format: "%03d", mv % 1000)
        while fraction.hasSuffix("0") { fraction.removeLast() }
        return "\(mv / 1000)V\(fraction)"
    }

    /// The voltage a rail name says ("170V", "3V3"), in millivolts; nil for
    /// any other text, and for a name `name(volts:)` would not write.
    static func millivolts(name: String) -> Int? {
        let halves = name.split(separator: "V", maxSplits: 1, omittingEmptySubsequences: false)
        guard halves.count == 2, let whole = Int(halves[0]), halves[0].allSatisfy(\.isNumber),
              halves[1].count <= 3, halves[1].allSatisfy(\.isNumber) else { return nil }
        let fraction = Int(halves[1].padding(toLength: 3, withPad: "0", startingAt: 0)) ?? 0
        let mv = whole * 1000 + fraction
        return SupplyChoice.name(volts: Double(mv) / 1000) == name ? mv : nil
    }

    /// What someone types for a voltage ("170", "3.3", "3,3 V", "170V",
    /// "3V3") as a rail name; nil when it is no voltage in range.
    static func name(typed text: String) -> String? {
        var text = text.trimmingCharacters(in: .whitespaces).uppercased()
        if let mv = millivolts(name: text) { return name(volts: Double(mv) / 1000) }
        if text.hasSuffix("V") { text.removeLast() }
        text = text.trimmingCharacters(in: .whitespaces).replacingOccurrences(of: ",", with: ".")
        guard let volts = Double(text), text.allSatisfy({ $0.isNumber || $0 == "." }) else { return nil }
        return name(volts: volts)
    }
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
    /// A diode part (plan-29): the diode symbol with no light; its cathode
    /// bar follows the part's type (`DiodeMark`).
    case plainDiode
    /// A capacitor (plan-33): two plates; a polarized one has a curved
    /// negative plate and a `+` by its positive pin (`CapacitorSymbolView`).
    case capacitor
    /// A two-terminal resistor (plan-34): a horizontal zig-zag between pin 1
    /// (left) and pin 2 (right), its value above (`ResistorSymbolView`).
    /// Resistor arrays are blocks.
    case resistor
    /// A single transistor (plan-40-H): the BJT or MOSFET symbol, the base
    /// or gate on the left, the collector or drain at the top right and the
    /// emitter or source at the bottom right (`TransistorSymbolView`).
    /// Transistor arrays are blocks.
    case transistor
    case vcc
    case ground
    case pullUp
    case pullDown
    case dipSwitch
    /// A tactile (push-button) switch, plan-27: side A's pins on the left,
    /// side B's on the right, a button in the middle.
    case tactileSwitch
    case sevenSegment
    /// A nixie tube: cathodes on the left, the anode on the right, the tube
    /// in the middle (`NixieSymbolView`).
    case nixie
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

    /// Rotate turns LEDs, diodes, resistors and capacitors in quarter turns
    /// (`PlacedChip.rotation`), which puts their pins on the top and bottom.
    /// Nothing else on the Schematic rotates.
    var canRotate: Bool {
        switch self {
        case .diode, .plainDiode, .capacitor, .resistor: true
        default: false
        }
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

    /// Transistor symbol (plan-40-H): the base or gate on `transistorMidY`,
    /// the collector or drain on `transistorTopY`, the emitter or source on
    /// `transistorBottomY`. The body is wide enough for the part name to the
    /// right of the symbol, between the two right-hand leads.
    static let transistorTopY: CGFloat = 10
    static let transistorMidY: CGFloat = 30
    static let transistorBottomY: CGFloat = 50
    static let transistorSize = CGSize(width: 80, height: 60)

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
        case .diode, .plainDiode:
            self = Self.diode(pins)
        case .capacitor, .resistor:
            self = Self.capacitor(pins)
        case .transistor:
            self = Self.transistor(pins)
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
        case .nixie:
            self = Self.nixie(pins, nixie: part.nixie)
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

    /// Bus pins on the left (RS, R/W, E — or E1, E2 on a two-controller
    /// module — a one-row gap, DB0-DB7); every other drawn pin on the right,
    /// in pin order (VSS, VDD, V0, LED+, LED-).
    private static func characterLCD(_ pins: [ChipPin], info: CharacterLCDInfo?) -> ChipLayout {
        guard let info else { return ChipLayout(pins: pins) }
        let byIndex = Dictionary(pins.map { ($0.index, $0) }, uniquingKeysWith: { first, _ in first })
        let bus = info.busPins.compactMap { byIndex[$0] }
        let control = 2 + max(info.enables, 1)
        let left = stack([Array(bus.prefix(control)), Array(bus.dropFirst(control))])
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
        // Commons, then (after a gap) any other lead: a VFD's filament.
        let drawn = Set(display.segmentPins + display.commonPins)
        let others = pins.filter { $0.role != .noConnect && !$0.isHidden && !drawn.contains($0.index) }
            .sorted { $0.index < $1.index }
        let right = stack([display.commonPins.compactMap { byIndex[$0] }, others])
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
            if let pin {
                slots.append(PinSlot(pin: pin, side: .right, centerY: centerY(forRow: row)))
            }
        }
        let width = SchematicGrid.snap(sevenSegmentFaceWidth(display) + 2 * sevenSegmentNameColumn + 5)
        return ChipLayout(
            size: CGSize(width: max(width, bodyWidth), height: headerHeight + pitch * CGFloat(rows)),
            slots: slots
        )
    }

    /// Nixie tube: the glass in the middle of the body, with a column for pin
    /// names on each side (`sevenSegmentNameColumn`).
    static let nixieFaceSize = CGSize(width: 40, height: 64)

    /// Cathode pins on the left, in glyph order; the anode on the right,
    /// then (after a gap) any no-connect pins.
    private static func nixie(_ pins: [ChipPin], nixie: NixieInfo?) -> ChipLayout {
        guard let nixie else { return ChipLayout(pins: pins) }
        let byIndex = Dictionary(pins.map { ($0.index, $0) }, uniquingKeysWith: { first, _ in first })
        let left = nixie.cathodePins.compactMap { byIndex[$0] }
        let used = Set(nixie.cathodePins + nixie.anodePins)
        let unused = pins.filter { !used.contains($0.index) }.sorted { $0.index < $1.index }
        let right = stack([nixie.anodePins.compactMap { byIndex[$0] }, unused])
        // At least enough rows for the glass.
        let faceRows = Int(((nixieFaceSize.height + 16) / pitch).rounded(.up))
        let rows = max(left.count, right.count, faceRows)

        var slots: [PinSlot] = []
        for (row, pin) in left.enumerated() {
            slots.append(PinSlot(pin: pin, side: .left, centerY: centerY(forRow: row)))
        }
        for (row, pin) in right.enumerated() {
            if let pin {
                slots.append(PinSlot(pin: pin, side: .right, centerY: centerY(forRow: row)))
            }
        }
        let width = SchematicGrid.snap(nixieFaceSize.width + 2 * sevenSegmentNameColumn + 5)
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

    /// The same body turned clockwise by `degrees` (0, 90, 180 or 270): the
    /// size swaps for a quarter turn and every pin moves to the side and
    /// position its point on the body edge turns to.
    func rotated(by degrees: Int) -> ChipLayout {
        let turns = ((degrees / 90) % 4 + 4) % 4
        guard turns != 0 else { return self }
        let width = size.width, height = size.height
        let newSize = turns == 2 ? size : CGSize(width: height, height: width)
        return ChipLayout(
            size: newSize,
            slots: slots.map { slot in
                // The pin's point on the body edge, then where it turns to.
                let edge: CGPoint
                switch slot.side {
                case .left: edge = CGPoint(x: 0, y: slot.centerY)
                case .right: edge = CGPoint(x: width, y: slot.centerY)
                case .top: edge = CGPoint(x: slot.centerX, y: 0)
                case .bottom: edge = CGPoint(x: slot.centerX, y: height)
                }
                var side = slot.side
                var point = edge
                var w = width, h = height
                for _ in 0..<turns {
                    point = CGPoint(x: h - point.y, y: point.x)
                    swap(&w, &h)
                    switch side {
                    case .left: side = .top
                    case .top: side = .right
                    case .right: side = .bottom
                    case .bottom: side = .left
                    }
                }
                return PinSlot(pin: slot.pin, side: side,
                               centerY: side == .left || side == .right ? point.y : 0,
                               centerX: side == .top || side == .bottom ? point.x : 0)
            }
        )
    }

    /// Anode (pin 2, "A") on the left and cathode (pin 1, "K") on the right.
    /// Other pins, such as no-connects on a 4-DIP, are not drawn. Falls back to
    /// the block layout if either terminal is missing.
    /// A capacitor (plan-33-H): pin 1 (`1`, or `+` on a polarized part) on
    /// the left, pin 2 (`2` or `-`) on the right, the diode's box.
    private static func capacitor(_ pins: [ChipPin]) -> ChipLayout {
        guard pins.count == 2,
              let first = pins.first(where: { $0.name == "+" || $0.name == "1" }),
              let second = pins.first(where: { $0.name == "-" || $0.name == "2" }) else {
            return ChipLayout(pins: pins)
        }
        return ChipLayout(
            size: diodeSize,
            slots: [
                PinSlot(pin: first, side: .left, centerY: diodeCentreY),
                PinSlot(pin: second, side: .right, centerY: diodeCentreY),
            ]
        )
    }

    /// A transistor (plan-40-H): the base (`B`) or gate (`G`) on the left
    /// at the middle, the collector (`C`) or drain (`D`) at the top right,
    /// the emitter (`E`) or source (`S`) at the bottom right. As the diode,
    /// the first pin of each name is placed; a tab of the same name (G, D,
    /// S, D) is joined to it inside the part and is not drawn. Falls back
    /// to the block layout if a terminal is missing.
    private static func transistor(_ pins: [ChipPin]) -> ChipLayout {
        func first(_ names: String...) -> ChipPin? { pins.first { names.contains($0.name) } }
        guard let control = first("B", "G"), let top = first("C", "D"), let bottom = first("E", "S") else {
            return ChipLayout(pins: pins)
        }
        return ChipLayout(
            size: transistorSize,
            slots: [
                PinSlot(pin: control, side: .left, centerY: transistorMidY),
                PinSlot(pin: top, side: .right, centerY: transistorTopY),
                PinSlot(pin: bottom, side: .right, centerY: transistorBottomY),
            ]
        )
    }

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
        func pins(_ roles: ChipPinRole...) -> [ChipPin] {
            allPins.filter { roles.contains($0.role) }.sorted(by: Self.byName)
        }
        // Signal pins: left or right by role, unless the part file picks a side.
        func signals(_ side: ChipPinSide, roles: ChipPinRole...) -> [ChipPin] {
            allPins.filter { $0.side == side || ($0.side == .auto && roles.contains($0.role)) }
                .sorted(by: Self.byName)
        }

        let power = pins(.power)
        let inputs = signals(.input, roles: .input)
        let unused = pins(.noConnect)
        let outputs = signals(.output, roles: .output, .inOut)
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

    /// Pin name order, numbers compared as numbers (A2 before A10); pins
    /// with the same name (GND, NC) in pinout order.
    private static func byName(_ a: ChipPin, _ b: ChipPin) -> Bool {
        switch a.name.localizedStandardCompare(b.name) {
        case .orderedAscending: return true
        case .orderedDescending: return false
        case .orderedSame: return a.index < b.index
        }
    }

    /// Splits pins (already in name order) by pin group: the default group
    /// first, then each named group (a bus such as A0..A18, or one gate's
    /// inputs) in the order of its first pin by name. Pins keep name order
    /// within each group. With only default pins this is the input unchanged.
    private static func grouped(_ pins: [ChipPin]) -> [[ChipPin]] {
        var groups: [[ChipPin]] = [pins.filter(\.isInDefaultGroup)]
        var names: [String] = []
        for pin in pins where !pin.isInDefaultGroup && !names.contains(pin.group) {
            names.append(pin.group)
        }
        for name in names {
            groups.append(pins.filter { $0.group == name })
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
    /// How far an LED, diode, resistor or capacitor is turned clockwise on
    /// the Schematic: 0, 90, 180 or 270 (`PartSymbol.canRotate`; 0 for every
    /// other part). The flip is applied first, then the turn.
    var rotation = 0
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
    /// The resistance in ohms (`ResistorValue.isValid`) of a pull resistor or
    /// `GENERIC-RESISTOR` (`CatalogPart.hasOhmsField`, plan-34-G). Nil for
    /// every other part, maker resistors included (their value is their part's).
    var ohms: Double?
    /// The generic capacitor's capacitance in farads (`CapacitorValue.isValid`,
    /// plan-33-H). Nil for every other part, real capacitors included (their
    /// value is their part's).
    var farads: Double?
    /// Whether the Schematic draws the reference above the part; the
    /// sidebar's Display Reference. On unless turned off.
    var showsReference = true

    private let baseLayout: ChipLayout

    init(
        id: UUID = UUID(),
        reference: String,
        part: CatalogPart,
        symbol: PartSymbol,
        origin: CGPoint,
        isFlipped: Bool = false,
        rotation: Int = 0,
        switchesOn: [Bool] = [],
        board: BoardPlacement? = nil,
        memoryFile: MemoryFile? = nil,
        color: String? = nil,
        footprintID: String? = nil,
        gender: String? = nil,
        ohms: Double? = nil,
        farads: Double? = nil
    ) {
        self.id = id
        self.reference = reference
        self.part = part
        self.symbol = symbol
        self.baseLayout = ChipLayout(part: part, symbol: symbol)
        self.origin = origin
        self.isFlipped = isFlipped
        self.rotation = symbol.canRotate ? PlacedChip.normalized(rotation) : 0
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
        if part.hasOhmsField {
            self.ohms = ohms.flatMap { ResistorValue.isValid($0) ? $0 : nil } ?? ResistorValue.defaultOhms
        } else {
            self.ohms = nil
        }
        if part.isGenericCapacitor {
            self.farads = farads.flatMap { CapacitorValue.isValid($0) ? $0 : nil } ?? CapacitorValue.defaultFarads
        } else {
            self.farads = nil
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
    /// symbol, origin, flip, board placement, gender, memory file and
    /// Display Reference are kept. The footprint is the new part's choice with the same package as
    /// the current one (`Straight` stays `Straight`), else its default.
    func resized(to newPart: CatalogPart) -> PlacedChip {
        let package = footprintChoice?.package
        let footprint = newPart.footprintChoices.first { $0.package == package }?.footprint.id
        var chip = PlacedChip(id: id, reference: reference, part: newPart, symbol: symbol, origin: origin,
                              isFlipped: isFlipped, rotation: rotation, switchesOn: switchesOn, board: board,
                              memoryFile: memoryFile, color: color, footprintID: footprint, gender: gender,
                              ohms: ohms, farads: farads)
        chip.showsReference = showsReference
        return chip
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
        let flipped = isFlipped ? baseLayout.flipped() : baseLayout
        return rotation == 0 ? flipped : flipped.rotated(by: rotation)
    }

    /// 0, 90, 180 or 270 for any whole number of degrees.
    static func normalized(_ degrees: Int) -> Int {
        ((degrees % 360) + 360) % 360 / 90 * 90
    }

    /// Whether the pins are on the top and bottom: a supply, or a part
    /// turned a quarter turn. Such a part's view has room for the stubs above
    /// and below its body.
    var hasVerticalPins: Bool {
        symbol.hasVerticalPins || rotation == 90 || rotation == 270
    }

    /// The size of the part before it is flipped or turned.
    var unturnedSize: CGSize { baseLayout.size }

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
            y: origin.y - (hasVerticalPins ? ChipLayout.stubLength : 0)
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
        let verticalStub = hasVerticalPins ? stub : 0
        return bodyRect.insetBy(dx: -stub, dy: -verticalStub)
    }
}

/// A pin on a placed chip.
nonisolated struct PinRef: Hashable {
    let chipID: UUID
    /// The pin's position in its part's pinout (`ChipPin.index`).
    let index: Int
}

/// A pin on a bus port: the port, and the bus wire the pin is for.
nonisolated struct BusPinRef: Hashable {
    let portID: UUID
    let wireID: UUID
}

/// One end of a wire: a pin on a placed part, a junction where wires meet,
/// or a pin on a bus port.
nonisolated enum WireEnd: Hashable {
    case pin(PinRef)
    case junction(UUID)
    case busPin(BusPinRef)
    /// A net label's point (`NetLabel`).
    case label(UUID)
}

/// A grid point where wires meet. Wires that branch from a point on another
/// wire both end here, so the point is fixed in place.
nonisolated struct Junction: Identifiable, Hashable {
    let id: UUID
    /// Moves only with a group drag (plan-15); always on the grid.
    var point: CGPoint
    /// A loose end: the spot where a deleted pin was. The wires on it kept
    /// their routes, and dropping a pin here connects them again (plan-30).
    var isLoose: Bool

    init(id: UUID = UUID(), point: CGPoint, isLoose: Bool = false) {
        self.id = id
        self.point = point
        self.isLoose = isLoose
    }
}

/// Where a line can start or end in Line mode: a pin, a bus port pin, or a
/// grid point that an existing wire passes through.
enum LineAnchor: Hashable {
    case pin(PinRef)
    case point(CGPoint)
    case busPin(BusPinRef)
    case label(UUID)
}

/// A wire joins two ends. Its route is the start tip, then the bends in
/// `interior`, then the end tip. Every segment is horizontal or vertical.
nonisolated struct Wire: Identifiable, Hashable {
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

    /// `path` (all right angles) with its ends moved to `start` and `end`.
    /// Each end's segment keeps its direction: a wire that left its pin
    /// sideways still does, and one that left it up or down still does. A
    /// slanted segment left over gets a corner; at an end, the corner keeps
    /// that end's direction.
    static func refit(_ path: [CGPoint], start: CGPoint, end: CGPoint) -> [CGPoint] {
        guard path.count >= 2 else { return path }
        func axis(_ a: CGPoint, _ b: CGPoint) -> RouteAxis? {
            a == b ? nil : a.y == b.y ? .horizontal : a.x == b.x ? .vertical : nil
        }
        let last = path.count - 1
        let startAxis = axis(path[0], path[1])
        let endAxis = axis(path[last - 1], path[last])
        if path.count == 2 {
            return [start] + leg(from: start, to: end, first: startAxis, last: endAxis, preferred: .horizontal)
        }

        var points = path
        points[0] = start
        points[last] = end
        if start != path[0], let startAxis {
            if startAxis == .horizontal { points[1].y = start.y } else { points[1].x = start.x }
        }
        if end != path[last], let endAxis {
            if endAxis == .horizontal { points[last - 1].y = end.y } else { points[last - 1].x = end.x }
        }

        var result = [points[0]]
        for index in 1...last {
            let p = result[result.count - 1], q = points[index]
            if p.x != q.x, p.y != q.y {
                // Leave the start along its axis, reach the end along its
                // axis; elsewhere go across first.
                let across: Bool
                if index == 1, let startAxis {
                    across = startAxis == .horizontal
                } else if index == last, let endAxis {
                    across = endAxis == .vertical
                } else {
                    across = true
                }
                result.append(across ? CGPoint(x: q.x, y: p.y) : CGPoint(x: p.x, y: q.y))
            }
            result.append(q)
        }
        return removeDoubleBacks(mergeCoincident(result))
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
    private(set) var chips: [PlacedChip] = [] {
        didSet {
            chipsRevision &+= 1
            copperRevision &+= 1
        }
    }
    private(set) var wires: [Wire] = [] { didSet { copperRevision &+= 1 } }
    private(set) var junctions: [Junction] = []
    /// Buses, in drawing order (plan-13). They carry no current themselves.
    private(set) var buses: [Bus] = [] { didSet { copperRevision &+= 1 } }

    /// Counts every change to what `connectivity()` reads (parts, wires,
    /// buses, net labels, traces, vias, holes, board settings), so the
    /// connectivity and ratsnest are built once per change, not once per
    /// redraw. Observed: a view that reads only the kept result still
    /// redraws when it goes stale.
    private(set) var copperRevision = 0
    /// Counts every change to `chips`, for `chipPads`.
    @ObservationIgnored private var chipsRevision = 0
    /// The last `connectivity()` and `ratsnest()`, and the `copperRevision`
    /// they were built at (see Ratsnest.swift).
    @ObservationIgnored var keptConnectivity: (revision: Int, copper: BoardConnectivity)?
    /// The last pour fills, with the pours and keep-outs they were made for (plan-52-D).
    @ObservationIgnored var keptPourFills: (revision: Int, pours: [BoardPour], keepouts: [BoardKeepout],
                                            fills: [UUID: PourFill])?
    @ObservationIgnored var keptRatsnest: (revision: Int, lines: [(CGPoint, CGPoint)])?
    @ObservationIgnored private var keptChipPads: (revision: Int, pads: [[PlacedPad]])?
    @ObservationIgnored private var keptVoltageIssues: (revision: Int, issues: Set<VoltageIssue>)?
    @ObservationIgnored private var keptChipIndex: (revision: Int, index: [UUID: Int])?
    /// Every wire's last route, with the wire and the two tips it was worked
    /// out from (see `path(for:)`): a wire is routed again only when it or
    /// the place of one of its ends changes.
    @ObservationIgnored private var keptPaths: [UUID: (wire: Wire, start: CGPoint, end: CGPoint, path: [CGPoint])] = [:]

    /// The voltage problems the drawing has now, kept until anything in it
    /// changes: an edit's check only needs to build the netlist of the
    /// drawing it would make.
    func currentVoltageIssues(_ library: PartLibraryHandle) -> Set<VoltageIssue> {
        let revision = copperRevision
        if let kept = keptVoltageIssues, kept.revision == revision { return kept.issues }
        let found = VoltageCheck.issues(chips: chips, wires: wires, buses: buses, labels: netLabels, library: library)
        keptVoltageIssues = (revision, found)
        return found
    }

    /// Each part's place in `chips`, by id, built once per change to the
    /// parts: a wire's end finds its part here, not by searching `chips`.
    private var chipIndex: [UUID: Int] {
        if let kept = keptChipIndex, kept.revision == chipsRevision { return kept.index }
        var index: [UUID: Int] = [:]
        index.reserveCapacity(chips.count)
        for (position, chip) in chips.enumerated() where index[chip.id] == nil { index[chip.id] = position }
        keptChipIndex = (chipsRevision, index)
        return index
    }

    /// Each part's pads on the board (`PlacedChip.boardPads`), in `chips`
    /// order, built once per change to the parts.
    var chipPads: [[PlacedPad]] {
        let chips = chips
        if let kept = keptChipPads, kept.revision == chipsRevision { return kept.pads }
        let pads = chips.map(\.boardPads)
        keptChipPads = (chipsRevision, pads)
        return pads
    }
    /// Text blocks, in drawing order. Drawn under everything else.
    private(set) var notes: [SchematicNote] = []
    /// Net labels, in drawing order. Labels with the same name are one net.
    private(set) var netLabels: [NetLabel] = [] { didSet { copperRevision &+= 1 } }
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

    /// The bus, when it is the only thing selected on the Schematic page
    /// (buses are items of `schematicSelection`). Setting one makes it the
    /// whole selection; setting nil clears a lone selected bus.
    var selectedBusID: UUID? {
        get {
            if case let .bus(id)? = schematicSelection.sole { return id }
            return nil
        }
        set {
            if let newValue {
                if schematicSelection.items != [.bus(newValue)] { schematicSelection.set([.bus(newValue)]) }
            } else if selectedBusID != nil {
                schematicSelection.clear()
            }
        }
    }
    /// The selected bus port. Selecting one clears the other Schematic selections.
    var selectedPortID: UUID? {
        didSet {
            if selectedPortID != nil { clearSelection(except: .port) }
        }
    }

    private enum SelectionKind { case items, port }

    /// On the Schematic page, the item set (buses included) and a port
    /// exclude each other: clears the one that isn't `kept`.
    private func clearSelection(except kept: SelectionKind) {
        if kept != .items, !schematicSelection.isEmpty { schematicSelection.clear() }
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

    /// The net label, when it is the only thing selected on the Schematic page.
    var selectedNetLabelID: UUID? {
        if case let .label(id)? = schematicSelection.sole { return id }
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

    /// The via, when it is the only thing selected on the PCB page.
    var selectedViaID: UUID? {
        if case let .via(id)? = boardSelection.sole { return id }
        return nil
    }

    /// The hole, when it is the only thing selected on the PCB page.
    var selectedHoleID: UUID? {
        if case let .hole(id)? = boardSelection.sole { return id }
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

    /// The parts, junctions, wires, text blocks, net labels and buses a box
    /// (schematic coordinates) touches. A bus is in when any of its lines
    /// is.
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
        for label in netLabels where label.rect.intersects(rect) {
            items.insert(.label(label.id))
        }
        for wire in wires {
            let path = path(for: wire)
            if zip(path, path.dropFirst()).contains(where: { Self.segment($0, $1, touches: rect) }) {
                items.insert(.wire(wire.id))
            }
        }
        for bus in buses where bus.lines.contains(where: { Self.segment($0.a, $0.b, touches: rect) }) {
            items.insert(.bus(bus.id))
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
        if visible.showsAnyCopper {
            for via in vias where via.rect.intersects(rect) { items.insert(.via(via.id)) }
            for hole in holes where hole.rect.intersects(rect) { items.insert(.hole(hole.id)) }
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

    /// Selects exactly `items` on the PCB page (a Design Rule Check line).
    func selectBoardItems(_ items: Set<BoardItem>) {
        if items != boardSelection.items { boardSelection.set(items) }
    }

    /// Edit › Select All on the Schematic: every part, wire, junction, text
    /// block, net label and bus.
    func selectAllSchematic() {
        var items = Set<SchematicItem>()
        items.formUnion(chips.map { .chip($0.id) })
        items.formUnion(wires.map { .wire($0.id) })
        items.formUnion(junctions.map { .junction($0.id) })
        items.formUnion(notes.map { .note($0.id) })
        items.formUnion(netLabels.map { .label($0.id) })
        items.formUnion(buses.map { .bus($0.id) })
        if items != schematicSelection.items { schematicSelection.set(items) }
    }

    /// Edit › Select All on the PCB: every footprint, text, trace, via and
    /// hole drawn with `visible` (as a box select would take them).
    func selectAllBoard(visible: BoardVisibility = BoardVisibility()) {
        var items = Set<BoardItem>()
        for chip in chips where chip.board != nil && visible.showsFootprint(chip) {
            items.insert(.footprint(chip.id))
        }
        for text in texts where visible.showsText(text) { items.insert(.text(text.id)) }
        for trace in traces where board.copperLayers.contains(trace.layer) && visible.showsTrace(on: trace.layer) {
            items.insert(.trace(trace.id))
        }
        if visible.showsAnyCopper {
            items.formUnion(vias.map { .via($0.id) })
            items.formUnion(holes.map { .hole($0.id) })
        }
        if items != boardSelection.items { boardSelection.set(items) }
    }

    /// Clears the Schematic page's selection: items (buses included) and
    /// port.
    func clearSchematicSelection() {
        schematicSelection.clear()
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
        let noteIDs = Set(notes.map(\.id)), labelIDs = Set(netLabels.map(\.id))
        let busIDs = Set(buses.map(\.id))
        schematicSelection.filter { item in
            switch item {
            case let .chip(id): chipIDs.contains(id)
            case let .wire(id): wireIDs.contains(id)
            case let .junction(id): junctionIDs.contains(id)
            case let .note(id): noteIDs.contains(id)
            case let .label(id): labelIDs.contains(id)
            case let .bus(id): busIDs.contains(id)
            }
        }
        let footprints = Set(chips.filter { $0.board != nil }.map(\.id))
        let traceIDs = Set(traces.map(\.id)), viaIDs = Set(vias.map(\.id)), holeIDs = Set(holes.map(\.id))
        let pourIDs = Set(pours.map(\.id)), keepoutIDs = Set(keepouts.map(\.id))
        boardSelection.filter { item in
            switch item {
            case let .footprint(id): footprints.contains(id)
            case let .text(id): textIDs.contains(id)
            case let .trace(id): traceIDs.contains(id)
            case let .via(id): viaIDs.contains(id)
            case let .hole(id): holeIDs.contains(id)
            case let .pour(id): pourIDs.contains(id)
            case let .keepout(id): keepoutIDs.contains(id)
            }
        }
    }

    /// The PCB's size, layers, colour and units. Changed through the edit
    /// functions, so every change is an undo step.
    private(set) var board = BoardSettings() { didSet { copperRevision &+= 1 } }
    /// Free texts on the PCB, in drawing order.
    private(set) var texts: [BoardText] = []
    /// Copper traces on the PCB, in drawing order (plan-16).
    private(set) var traces: [BoardTrace] = [] { didSet { copperRevision &+= 1 } }
    /// Vias on the PCB, in drawing order.
    private(set) var vias: [BoardVia] = [] { didSet { copperRevision &+= 1 } }
    /// Drilled holes on the PCB (mounting holes and others), in drawing
    /// order.
    private(set) var holes: [BoardHole] = [] { didSet { copperRevision &+= 1 } }
    /// Copper pours (plan-52), in drawing order: a pour fills before the
    /// ones after it, and gives way to the ones before it.
    private(set) var pours: [BoardPour] = [] { didSet { copperRevision &+= 1 } }
    /// Keep-outs (plan-52): rectangles the pours stay out of.
    private(set) var keepouts: [BoardKeepout] = [] { didSet { copperRevision &+= 1 } }

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
        // A new board starts from Settings ▸ DRC Defaults; from then on its
        // rules are its own, saved in its file.
        board.rules = DesignRules.appDefaults()
        board.rulesManufacturer = DRCManufacturer.appDefault()
    }

    /// Gives a new, blank schematic the standard net classes
    /// (`NetClass.standardClasses`), outside the undo history. Does nothing
    /// once the board has any class.
    func addStandardNetClasses() {
        guard board.netClasses.isEmpty else { return }
        board.netClasses = NetClass.standardClasses(rules: board.rules)
    }

    /// Replaces everything on the canvas, for example with a file that was
    /// just opened. Clears the selection.
    func replaceContents(chips: [PlacedChip], wires: [Wire], junctions: [Junction], buses: [Bus],
                         nextReference: Int, board: BoardSettings, texts: [BoardText],
                         traces: [BoardTrace], vias: [BoardVia] = [], holes: [BoardHole] = [],
                         pours: [BoardPour] = [], keepouts: [BoardKeepout] = [],
                         notes: [SchematicNote] = [], netLabels: [NetLabel] = []) {
        self.chips = chips
        self.notes = notes
        self.netLabels = netLabels
        self.wires = wires
        self.junctions = junctions
        // Buses from older files have no name: number them in order.
        var named: [Bus] = []
        for var bus in buses {
            if bus.name.isEmpty { bus.name = Bus.defaultBusName(taken: (buses + named).map(\.name)) }
            named.append(bus)
        }
        self.buses = named
        self.nextReference = max(nextReference, 1)
        self.board = board
        self.texts = texts
        defer { if self.board.edgeBus != nil, edgeBusID == nil { self.board.edgeBus = nil } }
        self.traces = traces
        self.vias = vias
        self.holes = holes
        self.pours = pours
        self.keepouts = keepouts
        editingNoteID = nil
        clearAllSelections()
        editorMessage = nil
        history.clear()
        recordingDepth = 0
        recordingStart = nil
        recordingName = nil
        recordingCoalescing = nil
        isGestureOpen = false
    }

    // MARK: Undo

    /// Each page's undo and redo lists (plan-31). Observed, so the Edit
    /// menu's titles follow it.
    private var history = UndoHistory()
    /// How many `recordEdit` calls (and an open gesture) are running. Only
    /// the outermost records.
    private var recordingDepth = 0
    private var recordingStart: DocumentState?
    private var recordingPage: EditorPage = .schematic
    /// The name of the step: of the outermost edit that changed something,
    /// the first one (a gesture's edits are siblings, the first names it).
    private var recordingName: String?
    /// The depth of the edit that gave `recordingName`.
    private var recordingNameDepth = 0
    private var recordingCoalescing: String?
    /// The outermost call's coalescing key, which wins when it gave one.
    private var outerCoalescing: String?
    /// True while the simulation runs: edits are not recorded and undo/redo
    /// do nothing. Set by `SimulationController`.
    var isPlaying = false
    /// A mouse drag is being recorded as one step.
    private(set) var isGestureOpen = false

    /// The saved part of the document, as a value.
    var state: DocumentState {
        DocumentState(chips: chips, wires: wires, junctions: junctions, buses: buses, notes: notes,
                      netLabels: netLabels, nextReference: nextReference, board: board, texts: texts, traces: traces, vias: vias, holes: holes,
                      pours: pours, keepouts: keepouts)
    }

    /// Writes a state back (undo, redo), then drops selections and editor
    /// state that point at items no longer there.
    private func setState(_ s: DocumentState) {
        if chips != s.chips { chips = s.chips }
        if wires != s.wires { wires = s.wires }
        if junctions != s.junctions { junctions = s.junctions }
        if buses != s.buses { buses = s.buses }
        if notes != s.notes { notes = s.notes }
        if netLabels != s.netLabels { netLabels = s.netLabels }
        if nextReference != s.nextReference { nextReference = s.nextReference }
        if board != s.board { board = s.board }
        if texts != s.texts { texts = s.texts }
        if traces != s.traces { traces = s.traces }
        if vias != s.vias { vias = s.vias }
        if holes != s.holes { holes = s.holes }
        if pours != s.pours { pours = s.pours }
        if keepouts != s.keepouts { keepouts = s.keepouts }
        pruneSelection()
        if let id = editingNoteID, !notes.contains(where: { $0.id == id }) { editingNoteID = nil }
        if let id = selectedBusID, bus(id) == nil { selectedBusID = nil }
        if let id = selectedPortID, portLocation(id) == nil { selectedPortID = nil }
    }

    /// Runs `body` as one undo step named `name` on `page` (and on every page
    /// that shows what it changed). Nested calls are part of the outermost
    /// one. The step is named by the outermost call that changed something
    /// (the first such, among a gesture's calls). An edit
    /// that changes nothing records nothing. Pushes with the same
    /// `coalescing` key merge until `endCoalescing()`.
    @discardableResult
    func recordEdit<T>(_ page: EditorPage, _ name: String, coalescing: String? = nil,
                       _ body: () throws -> T) rethrows -> T {
        guard !isPlaying else { return try body() }
        if recordingDepth == 0 { startRecording(page, coalescing: coalescing) }
        recordingDepth += 1
        let before = state
        defer {
            if recordingName == nil || recordingDepth < recordingNameDepth, state != before {
                recordingName = name
                recordingNameDepth = recordingDepth
                recordingCoalescing = coalescing
            }
            recordingDepth -= 1
            if recordingDepth == 0 { finishRecording() }
        }
        return try body()
    }

    private func startRecording(_ page: EditorPage, coalescing: String?) {
        recordingStart = state
        recordingPage = page
        recordingName = nil
        recordingCoalescing = nil
        outerCoalescing = coalescing
    }

    private func finishRecording() {
        defer {
            recordingStart = nil
            recordingName = nil
            recordingCoalescing = nil
            outerCoalescing = nil
        }
        guard let start = recordingStart else { return }
        // A card-edge bus link that no longer counts (the card's pin count
        // changed, the board stopped being a card, or the bus is gone) is
        // set back to None, in the same step, so no hidden link stays.
        if board.edgeBus != nil, edgeBusID == nil { board.edgeBus = nil }
        let changes = UndoDiff.changes(from: start, to: state)
        guard !changes.isEmpty else { return }
        let pages = changes.keys.reduce(into: Set([recordingPage])) { $0.formUnion($1.pages) }
        history.push(UndoStep(name: recordingName ?? "Edit", coalescing: outerCoalescing ?? recordingCoalescing,
                              changes: changes, pages: pages))
    }

    /// Starts recording a whole mouse drag on `page` as one step. A gesture
    /// still open (one SwiftUI ended without `onEnded`) is closed first.
    func beginGesture(_ page: EditorPage) {
        guard !isPlaying else { return }
        if isGestureOpen { endGesture() }
        isGestureOpen = true
        if recordingDepth == 0 { startRecording(page, coalescing: nil) }
        recordingDepth += 1
    }

    /// Ends the drag `beginGesture` started and records what it changed.
    func endGesture() {
        guard isGestureOpen else { return }
        isGestureOpen = false
        // The pours were not refilled while the drag moved what they read.
        if !pours.isEmpty { copperRevision &+= 1 }
        recordingDepth -= 1
        if recordingDepth == 0 { finishRecording() }
    }

    /// The next edit starts a new step even with the same coalescing key
    /// (a note editor lost focus).
    func endCoalescing() {
        history.endCoalescing()
    }

    /// Undoes the last step on `page`. A step that changes nothing any more
    /// (its items are gone) is dropped and the next one undone. False when
    /// there is nothing to undo, or while playing.
    @discardableResult
    func undo(on page: EditorPage) -> Bool {
        if isGestureOpen { endGesture() }
        guard !isPlaying else { return false }
        while let step = history.popUndo(on: page) {
            var s = state
            if UndoDiff.apply(step.changes, backwards: true, to: &s) {
                setState(s)
                history.pushRedo(step)
                return true
            }
        }
        return false
    }

    /// Redoes the last undone step on `page`, as `undo(on:)`.
    @discardableResult
    func redo(on page: EditorPage) -> Bool {
        if isGestureOpen { endGesture() }
        guard !isPlaying else { return false }
        while let step = history.popRedo(on: page) {
            var s = state
            if UndoDiff.apply(step.changes, backwards: false, to: &s) {
                setState(s)
                history.pushUndo(step)
                return true
            }
        }
        return false
    }

    /// The name of the step `undo(on:)` would undo ("Move"), or nil.
    func undoName(on page: EditorPage) -> String? {
        history.undoTop(on: page)?.name
    }

    /// The name of the step `redo(on:)` would redo, or nil.
    func redoName(on page: EditorPage) -> String? {
        history.redoTop(on: page)?.name
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
                   ohms: Double? = nil, farads: Double? = nil) {
        recordEdit(.schematic, "Add Part") {
            let layout = ChipLayout(part: part, symbol: symbol)
            let origin = SchematicGrid.snap(
                CGPoint(x: point.x - layout.size.width / 2, y: point.y - layout.size.height / 2)
            )
            let reference = takeReference(for: part, gender: gender)
            var placed = PlacedChip(
                reference: reference,
                part: part,
                symbol: symbol,
                origin: origin,
                color: color,
                footprintID: footprintID,
                gender: gender,
                ohms: ohms,
                farads: farads
            )
            placed.board = placed.footprint.map {
                BoardLayout.nextSpot(for: $0, among: chips, pad1: placed.part.firstPinDesignator,
                                     hasPinOneMark: placed.part.hasPinOneMark, grid: self.board.grid)
            }
            chips.append(placed)
            adoptLooseEnds(chips: [placed.id])
        }
    }

    /// The reference for a new part, taken, with its prefix from Settings ▸
    /// Reference Defaults (`ReferencePrefixes`). A part with the Integrated Circuit
    /// prefix gets the first number from `nextReference` on that no part
    /// uses (a part may have been renamed to one), which moves
    /// `nextReference` past it; any other part the lowest number no part
    /// uses. `alsoUsed` are references taken but not yet in `chips` (a
    /// paste in progress); `gender` is a header's.
    private func takeReference(for part: CatalogPart, gender: String?,
                               alsoUsed: [String] = []) -> String {
        let used = Set(chips.map(\.reference) + alsoUsed)
        let prefix = ReferencePrefixes.prefix(for: part, gender: gender)
        let counted = prefix == ReferencePrefixes.prefix(for: .integratedCircuit)
        var number = counted ? nextReference : 1
        while used.contains(ReferenceParts.reference(prefix: prefix, number: number)) {
            number += 1
        }
        if counted { nextReference = number + 1 }
        return ReferenceParts.reference(prefix: prefix, number: number)
    }

    /// Sets an LED's color. Does nothing for a part that isn't an LED, or a
    /// color the part doesn't have.
    func setPartColor(_ id: UUID, _ color: String) {
        recordEdit(.schematic, "Change Color") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.kind == PART_LED,
                  chips[index].part.colors.contains(color) else { return }
            chips[index].color = color
        }
    }

    /// Turns the reference above a part on the Schematic on or off.
    func setPartShowsReference(_ id: UUID, _ shows: Bool) {
        recordEdit(.schematic, shows ? "Show Reference" : "Hide Reference") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].showsReference != shows else { return }
            chips[index].showsReference = shows
        }
    }

    /// Sets a header's gender. Does nothing for a part that isn't a header,
    /// or a gender the part doesn't list.
    func setPartGender(_ id: UUID, _ gender: String) {
        recordEdit(.schematic, "Change Gender") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.kind == PART_HEADER,
                  chips[index].part.genders.contains(gender) else { return }
            chips[index].gender = gender
        }
    }

    /// Changes a header's size by swapping its part for `newPart` (the
    /// caller looks up `HeaderFamily.name(rows:perRow:)` in its catalog).
    /// Reference, origin, flip, gender, footprint type and board placement
    /// are kept, and on the PCB pad 1 keeps its board point. Wires on pins
    /// the new size doesn't have stay, with loose ends at those pins' old
    /// tips; the rest are re-fitted. Does
    /// nothing unless both are headers and `newPart` is another size.
    func setHeaderSize(_ id: UUID, to newPart: CatalogPart) {
        recordEdit(.schematic, "Change Size") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.kind == PART_HEADER, newPart.kind == PART_HEADER,
                  chips[index].part != newPart else { return }
            let old = chips[index]
            let pad1 = old.part.firstPinDesignator
            let anchor = old.board.flatMap { board in
                old.footprint?.pads.first { $0.designator == pad1 }.map { board.board($0.centre) }
            }
            let pinCount = newPart.pins.count
            loosen { end in
                if case let .pin(ref) = end { return ref.chipID == id && ref.index >= pinCount }
                return false
            }
            chips[index] = old.resized(to: newPart)
            // The reference was placed for the old body.
            chips[index].board?.referenceOffset = .zero
            if let anchor, var board = chips[index].board,
               let pad = chips[index].footprint?.pads.first(where: { $0.designator == pad1 }) {
                let landed = board.board(pad.centre)
                board.centre.x += anchor.x - landed.x
                board.centre.y += anchor.y - landed.y
                chips[index].board = board
            }
            settleWires(attachedTo: id)
        }
    }

    /// Sets the resistance of a pull resistor or `GENERIC-RESISTOR`
    /// (`hasOhmsField`). Does nothing for any other part, or a value that
    /// isn't `ResistorValue.isValid`.
    func setPartOhms(_ id: UUID, _ ohms: Double) {
        recordEdit(.schematic, "Change Resistance") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.hasOhmsField,
                  ResistorValue.isValid(ohms) else { return }
            chips[index].ohms = ohms
        }
    }

    /// Sets the generic capacitor's capacitance (plan-33-H). Does nothing for
    /// any other part, or a value that isn't `CapacitorValue.isValid`.
    func setPartFarads(_ id: UUID, _ farads: Double) {
        recordEdit(.schematic, "Change Capacitance") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.isGenericCapacitor,
                  CapacitorValue.isValid(farads) else { return }
            chips[index].farads = farads
        }
    }

    /// Picks one of a part's footprint choices. On the PCB the pad of the
    /// part's first pin keeps its board point (no re-snap, like
    /// `setFootprintStyle`); rotation, origin, marks and style are kept. Does
    /// nothing for an id that isn't one of the part's choices, or the current one.
    func setPartFootprint(_ id: UUID, _ footprintID: String) {
        recordEdit(.schematic, "Change Footprint") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].footprintID != footprintID,
                  chips[index].part.footprintChoices.contains(where: { $0.footprint.id == footprintID })
            else { return }
            let before = chips[index].footprint
            chips[index].footprintID = footprintID
            guard var changed = chips[index].board, let before, let after = chips[index].footprint else { return }
            // The reference was placed for the old body.
            changed.referenceOffset = .zero
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
    }

    /// Renames a part to `prefix` (uppercased) plus `number`. Returns why it
    /// can't (a bad prefix or number, or a reference another part has), or
    /// nil when it was renamed. `page` is the page the rename was made on;
    /// the step is on both pages' lists either way.
    @discardableResult
    func renameChip(_ id: UUID, prefix: String, number: Int?, page: EditorPage) -> String? {
        return recordEdit(page, "Rename") { () -> String? in
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
    }

    /// Edit › Reset References: every part gets its prefix from Settings ▸
    /// Reference Defaults back (`ReferencePrefixes`) and is numbered again from 1
    /// within its prefix, in reading order on the Schematic (top to bottom,
    /// then left to right), as one step. The next new part continues the
    /// Integrated Circuit prefix's numbers.
    /// Returns whether any reference changed.
    @discardableResult
    func resetReferences(page: EditorPage) -> Bool {
        let order = chips.indices.sorted { a, b in
            let ca = chips[a], cb = chips[b]
            if ca.origin.y != cb.origin.y { return ca.origin.y < cb.origin.y }
            if ca.origin.x != cb.origin.x { return ca.origin.x < cb.origin.x }
            return ca.reference.localizedStandardCompare(cb.reference) == .orderedAscending
        }
        var counts: [String: Int] = [:]
        var renamed = chips
        for index in order {
            let prefix = ReferencePrefixes.prefix(for: chips[index].part, gender: chips[index].gender)
            let number = (counts[prefix] ?? 0) + 1
            counts[prefix] = number
            renamed[index].reference = ReferenceParts.reference(prefix: prefix, number: number)
        }
        let next = (counts[ReferencePrefixes.prefix(for: .integratedCircuit)] ?? 0) + 1
        guard renamed != chips || next != nextReference else { return false }
        recordEdit(page, "Reset References") {
            chips = renamed
            nextReference = next
        }
        return true
    }

    /// Swaps the side of every pin on a placed part, and re-fits the wires
    /// attached to it.
    func flipChip(_ id: UUID) {
        recordEdit(.schematic, "Flip") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].symbol.canFlip else { return }
            chips[index].isFlipped.toggle()
            settleWires(attachedTo: id)
            adoptLooseEnds(chips: [id])
        }
    }

    /// Turns an LED, diode, resistor or capacitor a quarter turn, clockwise
    /// or not, and re-fits the wires attached to it.
    func rotateChip(_ id: UUID, counterclockwise: Bool = false) {
        recordEdit(.schematic, "Rotate") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].symbol.canRotate else { return }
            chips[index].rotation = PlacedChip.normalized(chips[index].rotation + (counterclockwise ? 270 : 90))
            settleWires(attachedTo: id)
            adoptLooseEnds(chips: [id])
        }
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
            case let .label(id):
                if let label = netLabels.first(where: { $0.id == id }) { base.labels[id] = label.origin }
            case let .bus(id):
                if let bus = bus(id) {
                    base.buses[id] = bus
                    base.ports.formUnion(bus.ports.map(\.id))
                }
            }
        }
        for wire in wires where base.wires[wire.id] != nil || moves(wire.start, in: base) || moves(wire.end, in: base) {
            base.affectedWires.insert(wire.id)
            let route = path(for: wire)
            if OrthogonalRoute.isOrthogonal(route) {
                base.orthogonalWires.insert(wire.id)
                if base.wires[wire.id] == nil { base.paths[wire.id] = route }
            }
        }
        return base
    }

    /// Whether a group drag moves this wire end: a pin of a moved part, a
    /// moved junction or a moved net label.
    private func moves(_ end: WireEnd, in base: SchematicGroupBase) -> Bool {
        switch end {
        case let .pin(ref): base.chips[ref.chipID] != nil
        case let .junction(id): base.junctions[id] != nil
        case let .busPin(ref): base.ports.contains(ref.portID)
        case let .label(id): base.labels[id] != nil
        }
    }

    /// `bus` moved by (`dx`, `dy`): its lines, and its ports along with
    /// them (a port's start is along its line).
    static func shifted(_ bus: Bus, dx: CGFloat, dy: CGFloat) -> Bus {
        var moved = bus
        var horizontal: [UUID: Bool] = [:]
        for index in moved.lines.indices {
            horizontal[moved.lines[index].id] = moved.lines[index].isHorizontal
            moved.lines[index].a.x += dx
            moved.lines[index].a.y += dy
            moved.lines[index].b.x += dx
            moved.lines[index].b.y += dy
        }
        for index in moved.ports.indices {
            moved.ports[index].start += horizontal[moved.ports[index].line] == true ? dx : dy
        }
        return moved
    }

    /// One frame of a group drag: every part, junction, selected wire bend
    /// and bus in `base` moves by `delta` snapped to the grid, from where it
    /// was at the press. Selected wires keep their ends: only their bends
    /// move. A frame that would make a moved bus touch another bus changes
    /// nothing, so the group stays at the last place it could go.
    func moveSchematicGroup(_ base: SchematicGroupBase, by delta: CGSize) {
        recordEdit(.schematic, "Move") {
            let dx = SchematicGrid.snap(delta.width), dy = SchematicGrid.snap(delta.height)
            func shifted(_ point: CGPoint) -> CGPoint { CGPoint(x: point.x + dx, y: point.y + dy) }
            if !base.buses.isEmpty {
                let moved = buses.map { bus in base.buses[bus.id].map { Self.shifted($0, dx: dx, dy: dy) } ?? bus }
                guard Self.busesAreApart(moved) else { return }
                if moved != buses { buses = moved }
            }
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
            for index in netLabels.indices {
                guard let origin = base.labels[netLabels[index].id] else { continue }
                let moved = shifted(origin)
                if netLabels[index].origin != moved { netLabels[index].origin = moved }
            }
            for index in wires.indices {
                guard let route = base.paths[wires[index].id],
                      let start = endTip(wires[index].start), let end = endTip(wires[index].end) else { continue }
                let fitted = Array(OrthogonalRoute.refit(route, start: start, end: end).dropFirst().dropLast())
                if wires[index].interior != fitted { wires[index].interior = fitted }
            }
        }
    }

    /// The end of a group drag: re-fits each affected wire once, as
    /// `settleWires(attachedTo:)` does. A wire that was all right angles at
    /// the press gets corners wherever the move left a slanted segment.
    func settleSchematicGroup(_ base: SchematicGroupBase) {
        recordEdit(.schematic, "Move") {
            for index in wires.indices where base.affectedWires.contains(wires[index].id) {
                guard let start = endTip(wires[index].start), let end = endTip(wires[index].end) else { continue }
                var interior = OrthogonalRoute.settled(start: start, interior: wires[index].interior, end: end)
                if base.orthogonalWires.contains(wires[index].id) {
                    interior = Array(OrthogonalRoute.normalize([start] + interior + [end]).dropFirst().dropLast())
                }
                wires[index].interior = interior
            }
            adoptLooseEnds(chips: Set(base.chips.keys), ports: base.ports, junctions: Set(base.junctions.keys),
                           labels: Set(base.labels.keys))
        }
    }

    // MARK: Copy and paste (Schematic)

    /// Adds a copy (`SchematicCopy`) moved by `offset` (snapped to the
    /// grid), as one step, and makes it the selection. Every item gets a new
    /// id; parts get new references (as a placed part does) and a footprint
    /// spot to the right of the board's parts. Net labels keep their names,
    /// so they join the nets of those names. A bus keeps its name when no
    /// bus has it, or when the bus that has it carries the same wires and
    /// type (so the two link, as labels join); else it takes the next free
    /// "BUS n". Loose ends that land on a pin connect. Refused, with nothing
    /// changed, when a pasted bus would touch another bus, or when it would
    /// put a voltage out of range on a part (`editorMessage` says why).
    /// Everything pasted is selected, wire ends (junctions) included, so the
    /// whole paste drags as one; a loose end that joins a pin is no longer a
    /// junction and drops out of the selection. Returns whether anything was
    /// pasted.
    @discardableResult
    func pasteSchematic(_ copy: SchematicCopy, offset: CGSize) -> Bool {
        guard !copy.isEmpty else { return false }
        return recordEdit(.schematic, "Paste") { () -> Bool in
            let dx = SchematicGrid.snap(offset.width), dy = SchematicGrid.snap(offset.height)
            func shifted(_ point: CGPoint) -> CGPoint { CGPoint(x: point.x + dx, y: point.y + dy) }
            var newID: [UUID: UUID] = [:]
            func renamed(_ id: UUID) -> UUID {
                if let id = newID[id] { return id }
                let fresh = UUID()
                newID[id] = fresh
                return fresh
            }

            let savedNextReference = nextReference
            var pastedChips: [PlacedChip] = []
            for chip in copy.chips {
                var placed = PlacedChip(
                    id: renamed(chip.id), reference: takeReference(for: chip.part, gender: chip.gender,
                                                                   alsoUsed: pastedChips.map(\.reference)),
                    part: chip.part,
                    symbol: chip.symbol, origin: shifted(chip.origin), isFlipped: chip.isFlipped,
                    rotation: chip.rotation, switchesOn: chip.switchesOn, memoryFile: chip.memoryFile, color: chip.color,
                    footprintID: chip.footprintID, gender: chip.gender, ohms: chip.ohms, farads: chip.farads
                )
                placed.showsReference = chip.showsReference
                placed.board = placed.footprint.map {
                    BoardLayout.nextSpot(for: $0, among: chips + pastedChips, pad1: placed.part.firstPinDesignator,
                                         hasPinOneMark: placed.part.hasPinOneMark, grid: self.board.grid)
                }
                pastedChips.append(placed)
            }
            let pastedJunctions = copy.junctions.map {
                Junction(id: renamed($0.id), point: shifted($0.point), isLoose: $0.isLoose)
            }
            let pastedLabels = copy.labels.map {
                NetLabel(id: renamed($0.id), name: $0.name, style: $0.style, origin: shifted($0.origin),
                         isFlipped: $0.isFlipped)
            }
            let pastedNotes = copy.notes.map {
                SchematicNote(id: renamed($0.id), text: $0.text, origin: shifted($0.origin), size: $0.size,
                              framed: $0.framed)
            }
            var pastedBuses: [Bus] = []
            for bus in copy.buses {
                let lines = bus.lines.map {
                    BusLine(id: renamed($0.id), a: shifted($0.a), b: shifted($0.b))
                }
                let busWires = bus.wires.map { BusWire(id: renamed($0.id), name: $0.name) }
                var horizontal: [UUID: Bool] = [:]
                for line in bus.lines { horizontal[line.id] = line.isHorizontal }
                let ports = bus.ports.map { port in
                    BusPort(id: renamed(port.id), line: renamed(port.line),
                            start: port.start + (horizontal[port.line] == true ? dx : dy), side: port.side,
                            wireIDs: Set(port.wireIDs.map(renamed)))
                }
                let lowered = bus.name.lowercased()
                let others = buses + pastedBuses
                let named = others.filter { $0.name.lowercased() == lowered }
                let links = named.allSatisfy { $0.wires.map(\.name) == bus.wires.map(\.name) && $0.card == bus.card }
                let name = !bus.name.isEmpty && links ? (named.first?.name ?? bus.name)
                    : Bus.defaultBusName(taken: others.map(\.name))
                pastedBuses.append(Bus(id: renamed(bus.id), name: name, card: bus.card, lines: lines,
                                       wires: busWires, ports: ports))
            }
            guard Self.busesAreApart(buses + pastedBuses) else {
                nextReference = savedNextReference
                editorMessage = "Can't paste there: a pasted bus would touch another bus."
                return false
            }
            func end(_ end: WireEnd) -> WireEnd {
                switch end {
                case let .pin(ref): .pin(PinRef(chipID: renamed(ref.chipID), index: ref.index))
                case let .junction(id): .junction(renamed(id))
                case let .label(id): .label(renamed(id))
                // Only pins of copied buses (`schematicCopy` loosens the rest).
                case let .busPin(ref): .busPin(BusPinRef(portID: renamed(ref.portID), wireID: renamed(ref.wireID)))
                }
            }
            let pastedWires = copy.wires.map {
                Wire(id: renamed($0.id), start: end($0.start), end: end($0.end), interior: $0.interior.map(shifted))
            }

            // Labels join existing nets by name, which can put a voltage
            // out of range.
            if let library,
               let issue = VoltageCheck.firstNew(before: chips, wires, after: chips + pastedChips,
                                                 wires + pastedWires, buses: buses + pastedBuses, labels: netLabels,
                                                 afterLabels: netLabels + pastedLabels, library: library) {
                nextReference = savedNextReference
                editorMessage = VoltageCheck.message(issue, chips: chips + pastedChips, edit: .connect)
                return false
            }
            editorMessage = nil
            chips += pastedChips
            junctions += pastedJunctions
            netLabels += pastedLabels
            notes += pastedNotes
            buses += pastedBuses
            wires += pastedWires

            var selection = Set<SchematicItem>()
            selection.formUnion(pastedChips.map { .chip($0.id) })
            selection.formUnion(pastedJunctions.map { .junction($0.id) })
            selection.formUnion(pastedLabels.map { .label($0.id) })
            selection.formUnion(pastedNotes.map { .note($0.id) })
            selection.formUnion(pastedWires.map { .wire($0.id) })
            selection.formUnion(pastedBuses.map { .bus($0.id) })
            clearSchematicSelection()
            for item in selection { toggleSelection(item) }
            pruneJunctions()
            adoptLooseEnds(chips: Set(pastedChips.map(\.id)),
                           ports: Set(pastedBuses.flatMap { $0.ports.map(\.id) }),
                           junctions: Set(pastedJunctions.filter(\.isLoose).map(\.id)),
                           labels: Set(pastedLabels.map(\.id)))
            return true
        }
    }

    // MARK: Net labels (Schematic)

    /// The lowest "N<n>" no label is named, for a new label.
    func nextNetLabelName() -> String {
        let taken = Set(netLabels.map(\.netKey))
        var n = 1
        while taken.contains("N\(n)") { n += 1 }
        return "N\(n)"
    }

    /// Adds a net label with its point at `point` (snapped), named
    /// `nextNetLabelName()`, and selects it. A loose wire end at that point
    /// joins it.
    @discardableResult
    func addNetLabel(at point: CGPoint, style: NetLabel.Style = .input) -> UUID {
        return recordEdit(.schematic, "Add Net Label") { () -> UUID in
            let label = NetLabel(id: UUID(), name: nextNetLabelName(), style: style, origin: SchematicGrid.snap(point))
            netLabels.append(label)
            select(.label(label.id))
            adoptLooseEnds(labels: [label.id])
            return label.id
        }
    }

    /// Renames a label (`NetLabel.cleanName`). Refused, with the name
    /// kept, when nothing is left of `value`, or when joining the nets would
    /// put a voltage out of range on a part (`editorMessage` says why).
    /// Returns whether the label now has the cleaned name. Typing one name
    /// is one undo step, until `endCoalescing()`.
    @discardableResult
    func setNetLabelName(_ id: UUID, _ value: String) -> Bool {
        recordEdit(.schematic, "Rename Net Label", coalescing: "label-name:\(id)") { () -> Bool in
            guard let name = NetLabel.cleanName(value),
                  let index = netLabels.firstIndex(where: { $0.id == id }) else { return false }
            guard netLabels[index].name != name else { return true }
            var renamed = netLabels
            renamed[index].name = name
            // Joining two nets can put a voltage out of range on a part; a name
            // no other label has joins nothing (and leaving a net only
            // removes connections), so there is nothing to check.
            let joins = netLabels.contains { $0.id != id && $0.isNet && $0.netKey == renamed[index].netKey }
            if joins, let library,
               let issue = VoltageCheck.firstNew(before: chips, wires, after: chips, wires, buses: buses,
                                                 labels: netLabels, afterLabels: renamed,
                                                 known: currentVoltageIssues(library), library: library) {
                editorMessage = VoltageCheck.message(issue, chips: chips, edit: .connect)
                return false
            }
            netLabels = renamed
            return true
        }
    }

    /// Sets a label's type. Refused, with the type kept, when the change
    /// would join nets that put a voltage out of range on a part (a No
    /// Connect turning back into a named label); `editorMessage` says why.
    func setNetLabelStyle(_ id: UUID, _ style: NetLabel.Style) {
        recordEdit(.schematic, "Change Net Label Type") {
            guard let index = netLabels.firstIndex(where: { $0.id == id }),
                  netLabels[index].style != style else { return }
            var changed = netLabels
            changed[index].style = style
            // Only becoming or leaving No Connect changes what joins what.
            if netLabels[index].isNet != style.isNet, let library,
               let issue = VoltageCheck.firstNew(before: chips, wires, after: chips, wires, buses: buses,
                                                 labels: netLabels, afterLabels: changed,
                                                 known: currentVoltageIssues(library), library: library) {
                editorMessage = VoltageCheck.message(issue, chips: chips, edit: .connect)
                return
            }
            netLabels = changed
        }
    }

    /// Turns a label to run the other way from its point. Its wires stay on
    /// the point.
    func flipNetLabel(_ id: UUID) {
        recordEdit(.schematic, "Flip") {
            guard let index = netLabels.firstIndex(where: { $0.id == id }) else { return }
            netLabels[index].isFlipped.toggle()
            settle { $0 == .label(id) }
        }
    }

    /// Deletes a label; its wires stay, with loose ends at its point.
    func deleteNetLabel(_ id: UUID) {
        recordEdit(.schematic, "Delete") {
            loosen { $0 == .label(id) }
            netLabels.removeAll { $0.id == id }
            pruneSelection()
        }
    }

    /// The label under a point: the last one drawn there.
    func netLabel(at point: CGPoint) -> NetLabel? {
        netLabels.last { $0.rect.contains(point) }
    }

    /// The label whose point is within `radius` of `point`.
    func netLabelTip(at point: CGPoint, radius: CGFloat) -> UUID? {
        netLabels.last { Self.distance(from: point, to: $0.tip) <= radius }?.id
    }

    func netLabelTip(_ id: UUID) -> CGPoint? {
        netLabels.first { $0.id == id }?.tip
    }

    // MARK: Text blocks (Schematic)

    /// Adds a text block with its top-left at `point` (snapped), selects it
    /// and asks the sidebar to edit its words.
    @discardableResult
    func addNote(at point: CGPoint) -> UUID {
        return recordEdit(.schematic, "Add Text") { () -> UUID in
            let note = SchematicNote(id: UUID(), text: SchematicNote.placeholder, origin: SchematicGrid.snap(point))
            notes.append(note)
            select(.note(note.id))
            editingNoteID = note.id
            return note.id
        }
    }

    /// Changes a block's words. An empty string is refused and the words kept.
    func setNoteText(_ id: UUID, _ value: String) {
        recordEdit(.schematic, "Edit Text", coalescing: "note-text:\(id)") {
            guard !value.isEmpty, let index = notes.firstIndex(where: { $0.id == id }),
                  notes[index].text != value else { return }
            notes[index].text = value
        }
    }

    /// Sets a block's font size in points, clamped to `SchematicNote.sizeRange`.
    func setNoteSize(_ id: UUID, _ size: Int) {
        recordEdit(.schematic, "Change Font Size") {
            guard let index = notes.firstIndex(where: { $0.id == id }) else { return }
            notes[index].size = min(max(size, SchematicNote.sizeRange.lowerBound), SchematicNote.sizeRange.upperBound)
        }
    }

    func setNoteFramed(_ id: UUID, _ framed: Bool) {
        recordEdit(.schematic, "Change Frame") {
            guard let index = notes.firstIndex(where: { $0.id == id }) else { return }
            notes[index].framed = framed
        }
    }

    func deleteNote(_ id: UUID) {
        recordEdit(.schematic, "Delete Text") {
            notes.removeAll { $0.id == id }
            if editingNoteID == id { editingNoteID = nil }
            pruneSelection()
        }
    }

    /// The text block under a point: the last one drawn there.
    func note(at point: CGPoint) -> SchematicNote? {
        notes.last { $0.rect.contains(point) }
    }

    // MARK: Footprints (PCB)

    /// Moves a footprint's centre (board mm), then snaps its origin point to
    /// the board grid.
    func moveFootprint(_ id: UUID, to centre: CGPoint) {
        recordEdit(.pcb, "Move") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  let board = chips[index].board, !board.locked,
                  let footprint = chips[index].footprint else { return }
            var moved = board
            moved.centre = centre
            chips[index].board = BoardLayout.snapped(moved, footprint: footprint,
                                                     pad1: chips[index].part.firstPinDesignator, grid: self.board.grid)
        }
    }

    /// Moves a footprint so its origin point lands on `point` (board mm),
    /// snapped to the grid.
    func moveFootprintOrigin(_ id: UUID, to point: CGPoint) {
        recordEdit(.pcb, "Move") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  let board = chips[index].board, !board.locked,
                  let footprint = chips[index].footprint else { return }
            let pad1 = chips[index].part.firstPinDesignator
            let current = BoardLayout.originPoint(of: board, footprint: footprint, pad1: pad1)
            var moved = board
            moved.centre.x += point.x - current.x
            moved.centre.y += point.y - current.y
            chips[index].board = BoardLayout.snapped(moved, footprint: footprint, pad1: pad1, grid: self.board.grid)
        }
    }

    /// Turns a footprint 90° clockwise about its origin point, which stays
    /// where it was.
    func rotateFootprint(_ id: UUID, counterclockwise: Bool = false) {
        recordEdit(.pcb, "Rotate") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  let board = chips[index].board, !board.locked,
                  let footprint = chips[index].footprint else { return }
            let pad1 = chips[index].part.firstPinDesignator
            let before = BoardLayout.originPoint(of: board, footprint: footprint, pad1: pad1)
            var turned = board
            turned.rotation = (board.rotation + (counterclockwise ? 270 : 90)) % 360
            let after = BoardLayout.originPoint(of: turned, footprint: footprint, pad1: pad1)
            turned.centre.x += before.x - after.x
            turned.centre.y += before.y - after.y
            chips[index].board = BoardLayout.snapped(turned, footprint: footprint, pad1: pad1, grid: self.board.grid)
        }
    }

    /// Moves a footprint to the other side of the board, mirrored left to
    /// right as seen from above. Its origin point stays where it was, then
    /// snaps, as in `rotateFootprint`. Does nothing on a 1-layer board or when
    /// the side doesn't change.
    func setFootprintSide(_ id: UUID, _ side: BoardSide) {
        recordEdit(.pcb, "Change Side") {
            guard board.layers > 1,
                  let index = chips.firstIndex(where: { $0.id == id }),
                  let placement = chips[index].board, placement.side != side,
                  let footprint = chips[index].footprint else { return }
            let pad1 = chips[index].part.firstPinDesignator
            let before = BoardLayout.originPoint(of: placement, footprint: footprint, pad1: pad1)
            var flipped = placement
            flipped.side = side
            // A placement mirrors before it turns, so a board-wide left-right
            // mirror of a turned footprint is the mirror plus the opposite
            // turn. Without this, 90° and 270° parts come out mirrored top to
            // bottom and their pad rows swap.
            flipped.rotation = (360 - placement.rotation) % 360
            let after = BoardLayout.originPoint(of: flipped, footprint: footprint, pad1: pad1)
            flipped.centre.x += before.x - after.x
            flipped.centre.y += before.y - after.y
            chips[index].board = BoardLayout.snapped(flipped, footprint: footprint, pad1: pad1, grid: self.board.grid)
        }
    }

    /// Changes which point of a footprint is its origin. The footprint stays
    /// where it is and is not re-snapped.
    func setFootprintOrigin(_ id: UUID, _ origin: FootprintOrigin) {
        recordEdit(.pcb, "Change Origin") {
            guard let index = chips.firstIndex(where: { $0.id == id }), chips[index].board != nil else { return }
            chips[index].board?.origin = origin
        }
    }

    /// Switches a part between its footprint and its connector footprint
    /// (the PCB Style option). Pin 1's first pad stays exactly where it was
    /// (not re-snapped: like an origin change, this may leave the origin
    /// point off the grid); rotation, origin and marks are kept. Does nothing for a part without a connector
    /// footprint or when the style does not change.
    func setFootprintStyle(_ id: UUID, _ style: FootprintStyle) {
        recordEdit(.pcb, "Change Style") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.connectorFootprint != nil,
                  let board = chips[index].board, board.style != style,
                  let before = chips[index].footprint else { return }
            let pad1 = chips[index].part.firstPinDesignator
            var changed = board
            changed.style = style
            // The reference was placed for the old body.
            changed.referenceOffset = .zero
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
    }

    /// Sets where a footprint's marks are drawn. A nil layer is left as it is.
    func setFootprintMarks(_ id: UUID, outline: MarkLayer? = nil, pin1: MarkLayer? = nil,
                           reference: MarkLayer? = nil) {
        recordEdit(.pcb, "Change Marks") {
            guard let index = chips.firstIndex(where: { $0.id == id }), chips[index].board != nil else { return }
            if let outline { chips[index].board?.outline = outline }
            if let pin1 { chips[index].board?.pin1 = pin1 }
            if let reference { chips[index].board?.reference = reference }
        }
    }

    /// Moves a footprint's reference so its text is centred at `point` (board
    /// mm), snapped to the trace grid. The reference is stored relative to the
    /// part (`BoardPlacement.referenceOffset`), so it follows the part. A
    /// locked footprint's reference can still be moved.
    func moveReference(_ id: UUID, toCentre point: CGPoint) {
        recordEdit(.pcb, "Move Reference") {
            guard let index = chips.firstIndex(where: { $0.id == id }), let board = chips[index].board else { return }
            let grid = CGFloat(self.board.traceGrid)
            let snapped = BoardGrid.snap(point, spacing: grid)
            let local = board.local(snapped)
            // Whole 0.0001 mm, so the file carries no float noise.
            let offset = CGPoint(x: (local.x * 10000).rounded() / 10000 + 0, y: (local.y * 10000).rounded() / 10000 + 0)
            if board.referenceOffset != offset { chips[index].board?.referenceOffset = offset }
        }
    }

    /// Puts a footprint's reference back on the body centre.
    func resetReference(_ id: UUID) {
        recordEdit(.pcb, "Reset Reference") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].board?.referenceOffset != .zero else { return }
            chips[index].board?.referenceOffset = .zero
        }
    }

    // MARK: Board

    /// Sets the board's size in mm, each side clamped to `BoardSettings.sizeRange`.
    func setBoardSize(width: Double, height: Double) {
        recordEdit(.pcb, "Change Board Size") {
            let range = BoardSettings.sizeRange
            if board.type == .customCard {
                // A custom card is as high as you like above its tab, and at
                // least long enough for it.
                let card = board.customCard.clamped()
                board.width = min(max(width, card.indent + card.tabLength), range.upperBound)
                board.height = min(max(height, card.depth + 1), range.upperBound)
                return
            }
            if let card = board.card {
                // A card's height is the standard's; only its length changes,
                // between the shortest that clears its connector and full length.
                board.width = min(max(width, card.minimumLength), card.maximumLength)
                board.height = card.height
                return
            }
            board.width = min(max(width, range.lowerBound), range.upperBound)
            board.height = min(max(height, range.lowerBound), range.upperBound)
        }
    }

    /// Sets the card settings (lanes, length, key, size, memory) and fits
    /// the board to them: the standard's height, and its length unless the
    /// card's length is free (PCI Express), when it stays within range.
    func setCardOptions(_ options: CardOptions) {
        recordEdit(.pcb, "Change Card") {
            guard board.cardOptions != options else { return }
            board.cardOptions = options
            guard board.type.hasOptions, let card = board.card else { return }
            board.height = card.height
            board.width = card.isFixedSize ? card.length : min(max(board.width, card.minimumLength), card.maximumLength)
        }
    }

    /// Sets the custom card's edge connector (clamped to fit), and grows
    /// the board to hold its tab.
    func setCustomCard(_ card: CustomCard) {
        recordEdit(.pcb, "Change Card Edge") {
            let value = card.clamped()
            if board.customCard != value { board.customCard = value }
            guard board.type == .customCard else { return }
            board.width = max(board.width, value.indent + value.tabLength)
            board.height = max(board.height, value.depth + 1)
        }
    }

    /// Makes the board a custom rectangle, a custom card (keeping its size,
    /// grown to fit the tab) or a standard card. A card takes
    /// its standard's height and length (the length stays editable), and
    /// at least 2 layers when it has fingers, which are on both sides. Back to
    /// Custom keeps the size. Nothing on the board moves.
    func setBoardType(_ type: BoardType) {
        recordEdit(.pcb, "Change Board Type") {
            guard board.type != type else { return }
            board.type = type
            if type != .customCard, let card = board.card {
                board.width = card.length
                board.height = card.height
            }
            if type == .customCard {
                // Keep the size, grown to fit the tab.
                let card = board.customCard.clamped()
                board.width = max(board.width, card.indent + card.tabLength)
                board.height = max(board.height, card.depth + 1)
            }
            if board.needsTwoLayers, board.layers < 2 { board.layers = 2 }
        }
    }

    /// Sets the component grid (mm), clamped to `BoardSettings.gridRange`.
    /// The trace grid follows it. Nothing on the board moves.
    func setBoardGrid(_ mm: Double) {
        recordEdit(.pcb, "Change Grid") {
            let range = BoardSettings.gridRange
            let value = min(max(mm, range.lowerBound), range.upperBound)
            if board.grid != value { board.grid = value }
        }
    }

    /// The units the PCB sidebar shows sizes in.
    func setBoardUnits(_ units: BoardUnits) {
        recordEdit(.pcb, "Change Units") {
            if board.units != units { board.units = units }
        }
    }

    /// The board's solder-mask colour.
    func setBoardColor(_ color: BoardColor) {
        recordEdit(.pcb, "Change Board Color") {
            if board.color != color { board.color = color }
        }
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
        if let layer = board.copperLayers.first(where: { layer in
            dropped.contains(layer) && pours.contains { $0.layer == layer }
        }) {
            return "Delete or move the pours on \(layer.title) first."
        }
        if count == 1, !vias.isEmpty {
            return "Delete the vias first."
        }
        if count == 1, board.needsTwoLayers {
            return "A card has fingers on both sides."
        }
        return nil
    }

    /// Sets the copper layer count, unless `canSetLayerCount` refuses it.
    func setLayerCount(_ count: Int) {
        recordEdit(.pcb, "Change Layers") {
            guard count != board.layers, canSetLayerCount(count) == nil else { return }
            board.layers = count
        }
    }

    // MARK: Stackup (PCB)

    func setStackupFinish(_ finish: SurfaceFinish) {
        recordEdit(.pcb, "Change Stackup") {
            if board.stackup.finish != finish { board.stackup.finish = finish }
        }
    }

    /// Sets the temperature rise the current estimate allows, clamped to
    /// `BoardStackup.temperatureRiseRange`.
    func setStackupTemperatureRise(_ degrees: Double) {
        recordEdit(.pcb, "Change Stackup") {
            let value = min(max(degrees, BoardStackup.temperatureRiseRange.lowerBound),
                            BoardStackup.temperatureRiseRange.upperBound)
            if board.stackup.temperatureRise != value { board.stackup.temperatureRise = value }
        }
    }

    /// Sets the thickness (mm) of the copper layer at `index` (Top is 0),
    /// clamped to `BoardStackup.copperRange`.
    func setCopperThickness(_ mm: Double, at index: Int) {
        recordEdit(.pcb, "Change Stackup") {
            guard board.stackup.copper.indices.contains(index) else { return }
            let value = min(max(mm, BoardStackup.copperRange.lowerBound), BoardStackup.copperRange.upperBound)
            if board.stackup.copper[index] != value { board.stackup.copper[index] = value }
        }
    }

    /// Changes the dielectric at `index` (the one below copper layer
    /// `index`): any of its kind, thickness (mm) and Er; values are clamped
    /// to `BoardStackup.thicknessRange` and `erRange`.
    func setDielectric(at index: Int, kind: DielectricKind? = nil, thickness: Double? = nil, er: Double? = nil) {
        recordEdit(.pcb, "Change Stackup") {
            guard board.stackup.dielectrics.indices.contains(index) else { return }
            var dielectric = board.stackup.dielectrics[index]
            if let kind { dielectric.kind = kind }
            if let thickness {
                dielectric.thickness = min(max(thickness, BoardStackup.thicknessRange.lowerBound),
                                           BoardStackup.thicknessRange.upperBound)
            }
            if let er { dielectric.er = min(max(er, BoardStackup.erRange.lowerBound), BoardStackup.erRange.upperBound) }
            if board.stackup.dielectrics[index] != dielectric { board.stackup.dielectrics[index] = dielectric }
        }
    }

    /// Puts the board's standard stackup for its layer count back, keeping
    /// nothing of the old copper and dielectrics, the finish or the rise.
    func resetStackup() {
        recordEdit(.pcb, "Reset Stackup") {
            let standard = BoardStackup.standard(layers: board.layers)
            if board.stackup != standard { board.stackup = standard }
        }
    }

    func setLayerColor(_ layer: CopperLayer, _ color: LayerColor) {
        recordEdit(.pcb, "Change Layer Color") {
            guard board.copperLayers.contains(layer) else { return }
            var style = board.style(of: layer)
            guard style.color != color else { return }
            style.color = color
            board.styles[layer] = style
        }
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
        if type == .plane, pours.contains(where: { $0.layer == layer }) {
            return "Move or delete this layer's pours first."
        }
        return nil
    }

    func setLayerType(_ layer: CopperLayer, _ type: LayerType) {
        recordEdit(.pcb, "Change Layer Type") {
            guard canSetLayerType(layer, type) == nil else { return }
            var style = board.style(of: layer)
            guard style.type != type else { return }
            style.type = type
            board.styles[layer] = style
        }
    }

    /// Sets a Plane layer's net; nil leaves it on no net.
    func setLayerNet(_ layer: CopperLayer, _ net: PlaneNet?) {
        recordEdit(.pcb, "Change Plane Net") {
            guard board.copperLayers.contains(layer), layer.isInner else { return }
            var style = board.style(of: layer)
            guard style.net != net else { return }
            style.net = net
            board.styles[layer] = style
        }
    }

    /// Sets a Plane layer's clearance (mm), clamped to
    /// `LayerStyle.clearanceRange`; nil goes back to the DRC's minimum.
    func setLayerClearance(_ layer: CopperLayer, _ mm: Double?) {
        recordEdit(.pcb, "Change Plane Clearance") {
            guard board.copperLayers.contains(layer), layer.isInner else { return }
            var style = board.style(of: layer)
            let value = mm.map { min(max($0, LayerStyle.clearanceRange.lowerBound), LayerStyle.clearanceRange.upperBound) }
            guard style.clearance != value else { return }
            style.clearance = value
            board.styles[layer] = style
        }
    }

    /// The nets a Plane can carry, as its Net picker lists them: ground,
    /// then each power rail from low to high (every supply symbol and pull
    /// resistor of one voltage is one rail), then each signal net the
    /// schematic wires, named by up to three of its pins ("U1.3, R2.1, …")
    /// and stored by its first.
    func planeNetChoices() -> [(net: PlaneNet, title: String)] {
        let netlist = NetlistBuilder.build(chips: chips, wires: wires, buses: buses, labels: netLabels)
        var rails = Set<UInt32>()
        var railNets = Set<Int>()
        for chip in chips {
            if case let .supply(_, mv, _) = chip.part.details { rails.insert(mv) }
        }
        for connection in netlist.connections {
            if case .supply = chips[connection.part].part.details { railNets.insert(connection.net) }
        }
        var choices: [(net: PlaneNet, title: String)] = rails.sorted().map { mv in
            (PlaneNet.rail(mv: mv), Self.planeNetTitle(rail: mv))
        }
        var pinsOfNet: [Int: [(chip: PlacedChip, designator: String)]] = [:]
        for connection in netlist.connections where !railNets.contains(connection.net) {
            let chip = chips[connection.part]
            pinsOfNet[connection.net, default: []].append((chip, connection.pin))
        }
        typealias NetPin = (chip: PlacedChip, designator: String)
        func before(_ a: NetPin, _ b: NetPin) -> Bool {
            let byPart = a.chip.reference.localizedStandardCompare(b.chip.reference)
            if byPart != .orderedSame { return byPart == .orderedAscending }
            return a.designator.localizedStandardCompare(b.designator) == .orderedAscending
        }
        let signals = pinsOfNet.values.map { $0.sorted(by: before) }
            .filter { $0.count > 1 }
            .sorted { before($0[0], $1[0]) }
        for pins in signals {
            var title = pins.prefix(3).map { "\($0.chip.reference).\($0.designator)" }.joined(separator: ", ")
            if pins.count > 3 { title += ", …" }
            choices.append((.pin(chip: pins[0].chip.id, designator: pins[0].designator), title))
        }
        return choices
    }

    /// "GND" for the 0 mV rail, else its voltage: "5 V", "3.3 V".
    nonisolated static func planeNetTitle(rail mv: UInt32) -> String {
        mv == 0 ? "GND" : "\(VoltageCheck.volts(mv)) V"
    }

    // MARK: Board texts

    /// Adds a "Text" on the silkscreen with its top-left at `point` (snapped)
    /// and selects it.
    @discardableResult
    func addText(at point: CGPoint) -> UUID {
        return recordEdit(.pcb, "Add Text") { () -> UUID in
            let text = BoardText(id: UUID(), text: "Text", anchor: BoardGrid.snap(point, spacing: self.board.grid), size: BoardText.defaultSize)
            texts.append(text)
            select(.text(text.id))
            return text.id
        }
    }

    /// Changes a text's words. An empty string is refused and the text kept.
    func setText(_ id: UUID, _ value: String) {
        recordEdit(.pcb, "Edit Text") {
            guard !value.isEmpty, let index = texts.firstIndex(where: { $0.id == id }) else { return }
            texts[index].text = value
        }
    }

    /// Sets a text's font size in mil, clamped to `BoardText.sizeRange`.
    func setTextSize(_ id: UUID, _ size: Int) {
        recordEdit(.pcb, "Change Font Size") {
            guard let index = texts.firstIndex(where: { $0.id == id }) else { return }
            texts[index].size = min(max(size, BoardText.sizeRange.lowerBound), BoardText.sizeRange.upperBound)
        }
    }

    func setTextLayer(_ id: UUID, _ layer: TextLayer) {
        recordEdit(.pcb, "Change Layer") {
            guard let index = texts.firstIndex(where: { $0.id == id }) else { return }
            texts[index].layer = layer
        }
    }

    /// Puts a text on the top or bottom of the board. Its box stays where it
    /// is. Does nothing on a 1-layer board, except to bring a text back to Top.
    func setTextSide(_ id: UUID, _ side: BoardSide) {
        recordEdit(.pcb, "Change Side") {
            guard side == .top || board.layers > 1,
                  let index = texts.firstIndex(where: { $0.id == id }), texts[index].side != side else { return }
            texts[index].side = side
        }
    }

    /// Moves a text's anchor to `point`, snapped to the grid.
    func moveText(_ id: UUID, to point: CGPoint) {
        recordEdit(.pcb, "Move") {
            guard let index = texts.firstIndex(where: { $0.id == id }), !texts[index].locked else { return }
            texts[index].anchor = BoardGrid.snap(point, spacing: self.board.grid)
        }
    }

    /// Sets a text's rotation: 0, 90, 180 or 270. Other values are ignored.
    func setTextRotation(_ id: UUID, _ rotation: Int) {
        recordEdit(.pcb, "Rotate") {
            guard BoardPlacement.rotations.contains(rotation),
                  let index = texts.firstIndex(where: { $0.id == id }), !texts[index].locked else { return }
            texts[index].rotation = rotation
        }
    }

    /// Turns a text 90° clockwise about its anchor.
    func rotateText(_ id: UUID, counterclockwise: Bool = false) {
        recordEdit(.pcb, "Rotate") {
            guard let index = texts.firstIndex(where: { $0.id == id }), !texts[index].locked else { return }
            texts[index].rotation = (texts[index].rotation + (counterclockwise ? 270 : 90)) % 360
        }
    }

    func deleteText(_ id: UUID) {
        recordEdit(.pcb, "Delete Text") {
            texts.removeAll { $0.id == id }
            pruneSelection()
        }
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

    /// The footprint whose drawn reference text is under a board point (mm),
    /// within `tolerance` of its box: Top before Bottom, the last one drawn
    /// first. `height` is the text's drawn height (mm); a reference that is
    /// hidden, or whose layer is off, can't be hit.
    func referenceHit(at point: CGPoint, tolerance: CGFloat, height: Double = BoardReference.height,
                      visible: BoardVisibility = BoardVisibility()) -> UUID? {
        for side in [BoardSide.top, .bottom] {
            if let hit = chips.last(where: { chip in
                guard let placement = chip.board, placement.side == side,
                      visible.showsMark(placement.reference, side: side),
                      let rect = chip.boardReferenceRect(height: height) else { return false }
                return rect.insetBy(dx: -tolerance, dy: -tolerance).contains(point)
            }) { return hit.id }
        }
        return nil
    }

    /// The pad under a board point (mm) and what to call it, for the PCB
    /// page's hover label: a part's pad ("U3.5 A0"; "U3.5" when the pin's
    /// name is just its number; "U3 centre pad" for an unnumbered one), or a
    /// card's connector pin ("Edge.13 A0", with the edge bus's wire name) or
    /// named header pad off the bus. Top before Bottom; a pad that
    /// isn't drawn can't be hit.
    func padLabel(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> (pad: PlacedPad, label: String)? {
        func hits(_ pad: PlacedPad) -> Bool {
            guard pad.rect.contains(point) else { return false }
            guard pad.shape == .round else { return true }
            let dx = (point.x - pad.rect.midX) / (pad.rect.width / 2)
            let dy = (point.y - pad.rect.midY) / (pad.rect.height / 2)
            return dx * dx + dy * dy <= 1
        }
        for side in [BoardSide.top, .bottom] {
            for (chip, pads) in zip(chips, chipPads).reversed() where chip.board?.side == side {
                guard let pad = pads.last(where: {
                    $0.plated && visible.showsPad(drill: $0.drill, side: side) && hits($0)
                }) else { continue }
                guard let designator = pad.designator else { return (pad, "\(chip.reference) centre pad") }
                let name = chip.part.pins.first { $0.designator == designator }?.name ?? ""
                let label = "\(chip.reference).\(designator)"
                return (pad, name.isEmpty || name == designator ? label : "\(label) \(name)")
            }
        }
        let named = (board.fingers + board.extraPads).filter { !$0.name.isEmpty }
        guard let finger = named.last(where: {
            visible.showsPad(drill: $0.drill, side: $0.side) && hits($0.pad)
        }) else { return nil }
        // A connector pin is named like a part's pad ("Edge.13"); one on the
        // bus also shows the edge bus's wire of the same position
        // (`BoardSettings.pinNames`), as "Edge.13 A0".
        guard board.fingers.contains(finger) else { return (finger.pad, finger.name) }
        let label = "\(board.connectorName).\(finger.name)"
        if let busID = edgeBusID, let bus = bus(busID),
           let index = board.pinNames.firstIndex(of: finger.name), index < bus.wires.count {
            let wire = bus.wires[index].name
            if !wire.isEmpty, wire != finger.name { return (finger.pad, "\(label) \(wire)") }
        }
        return (finger.pad, label)
    }


    // MARK: Traces (PCB)

    /// The pads on `layer`, with their parts: a surface-mount pad is on its
    /// part's side layer; a through-hole pad is on every layer.
    func pads(on layer: CopperLayer) -> [(chip: PlacedChip, pad: PlacedPad)] {
        zip(chips, chipPads).flatMap { chip, pads -> [(chip: PlacedChip, pad: PlacedPad)] in
            guard let side = chip.board?.side else { return [] }
            return pads
                .filter { $0.plated && ($0.drill > 0 || side.copperLayer == layer) }
                .map { (chip: chip, pad: $0) }
        }
    }

    /// Where a trace end pressed or released at `point` lands on `layer`:
    /// the centre of a pad there, else of a via, else the nearest of a
    /// plated hole's four contacts (`BoardHole.contacts`), else the inner end of a
    /// card's edge finger or header pad there (`EdgeFinger.contact`), else a vertex of a trace on that layer
    /// within `tolerance`, else the nearest point of a trace segment within
    /// `tolerance`, else the nearest trace-grid point (`BoardSettings.traceGrid`).
    func traceAnchor(at point: CGPoint, layer: CopperLayer, tolerance: CGFloat) -> CGPoint {
        if let hit = pads(on: layer).last(where: { $0.pad.rect.contains(point) }) {
            return CGPoint(x: hit.pad.rect.midX, y: hit.pad.rect.midY)
        }
        if let via = via(at: point) { return via.centre }
        if let hole = holes.last(where: { $0.plated && Self.distance(from: point, to: $0.centre) <= $0.diameter / 2 }),
           let contact = hole.contact(nearest: point) {
            return contact
        }
        if let finger = (board.fingers + board.extraPads).last(where: { $0.isOn(layer) && $0.rect.contains(point) }) {
            return finger.contact
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
        return best?.point ?? BoardGrid.snap(point, spacing: self.board.traceGrid)
    }

    /// Why no trace can go on `layer`, or nil when one can.
    private func traceLayerRefusal(_ layer: CopperLayer) -> String? {
        guard board.copperLayers.contains(layer) else { return "\(layer.title) is not on this board." }
        guard board.style(of: layer).type != .plane else { return "\(layer.title) is a plane." }
        return nil
    }

    /// Adds a trace from `a` to `b` on `layer`, routed at `angle`
    /// (`TraceRoute.route`; `diagonalFirst` puts the angled leg first). See
    /// `addTrace(points:layer:)`.
    @discardableResult
    func addTrace(from a: CGPoint, to b: CGPoint, layer: CopperLayer, angle: Int = TraceRoute.defaultAngle,
                  diagonalFirst: Bool) -> UUID? {
        addTrace(points: TraceRoute.route(from: a, to: b, angle: angle, legFirst: diagonalFirst), layer: layer)
    }

    /// Adds a trace along `points` (simplified) on `layer`, as wide as the
    /// class of the net its first point is on (`netRules(at:layer:)`), and
    /// joins it to every trace it touches there. Returns its id, or nil when
    /// nothing was added: no length, or a layer that is a plane or not on
    /// the board.
    @discardableResult
    func addTrace(points: [CGPoint], layer: CopperLayer) -> UUID? {
        guard let start = points.first else { return nil }
        return addTrace(points: points, layer: layer, width: netRules(at: start, layer: layer).width)
    }

    /// Adds a trace `width` mil wide (clamped to `BoardTrace.widthRange`);
    /// otherwise as `addTrace(points:layer:)`.
    @discardableResult
    func addTrace(points: [CGPoint], layer: CopperLayer, width: Int) -> UUID? {
        return recordEdit(.pcb, "Add Trace") { () -> UUID? in
            let points = TraceRoute.simplified(points)
            guard TraceRoute.isLegal(points), traceLayerRefusal(layer) == nil else { return nil }
            let width = min(max(width, BoardTrace.widthRange.lowerBound), BoardTrace.widthRange.upperBound)
            let trace = BoardTrace(id: UUID(), layer: layer, width: width, points: points)
            traces.append(trace)
            join(trace.id)
            return trace.id
        }
    }

    /// Makes every point where trace `id` touches or crosses another trace
    /// on its layer a vertex of both. Inserting a vertex on a straight
    /// segment keeps its angle, so both stay legal.
    func join(_ id: UUID) {
        recordEdit(.pcb, "Join Traces") {
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
        for finger in board.fingers where finger.isOn(layer) {
            let touched = members.contains { trace in
                trace.segments.contains { Self.segment($0.0, $0.1, touches: finger.rect) }
            }
            if touched { found.append((board.connectorName, finger.name)) }
        }
        for pad in board.extraPads where pad.isOn(layer) {
            let touched = members.contains { trace in
                trace.segments.contains { Self.segment($0.0, $0.1, touches: pad.rect) }
            }
            // "ICSP.1": its group and pin.
            let parts = pad.name.split(separator: ".", maxSplits: 1).map(String.init)
            if touched, parts.count == 2 { found.append((parts[0], parts[1])) }
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
        recordEdit(.pcb, "Change Trace Width") {
            guard let index = traces.firstIndex(where: { $0.id == id }) else { return }
            traces[index].width = min(max(mil, BoardTrace.widthRange.lowerBound), BoardTrace.widthRange.upperBound)
        }
    }

    /// Moves one trace to another layer and joins it there. Ignored for a
    /// plane or a layer not on the board. Its run on the old layer stays.
    func setTraceLayer(_ id: UUID, _ layer: CopperLayer) {
        recordEdit(.pcb, "Change Layer") {
            guard traceLayerRefusal(layer) == nil,
                  let index = traces.firstIndex(where: { $0.id == id }), traces[index].layer != layer else { return }
            traces[index].layer = layer
            join(id)
        }
    }

    // MARK: Routing primitives (plan-38-G)

    /// Removes traces and vias by id, locked or not, inside the caller's
    /// edit (`applyRouting`, `unroute`); the callers decide what may go.
    func removeCopper(traces ids: Set<UUID>, vias viaIDs: Set<UUID>) {
        if !ids.isEmpty { traces.removeAll { ids.contains($0.id) } }
        if !viaIDs.isEmpty { vias.removeAll { viaIDs.contains($0.id) } }
        pruneSelection()
    }

    /// Appends a trace as it is (no join), inside the caller's edit. False
    /// when the board refuses it: a plane or absent layer, or no length.
    func appendTrace(_ trace: BoardTrace) -> Bool {
        guard traceLayerRefusal(trace.layer) == nil, TraceRoute.isLegal(trace.points) else { return false }
        traces.append(trace)
        return true
    }

    /// Appends a via, inside the caller's edit. False on a 1-layer board.
    func appendVia(_ via: BoardVia) -> Bool {
        guard board.layers > 1 else { return false }
        vias.append(via)
        return true
    }

    func deleteTrace(_ id: UUID) {
        recordEdit(.pcb, "Delete Trace") {
            traces.removeAll { $0.id == id }
            pruneSelection()
        }
    }

    /// The PCB Delete tool's click: deletes the text under `point`, else the
    /// via, else the trace (as `text(at:)`, `via(at:)` and `trace(at:)` hit
    /// them). Footprints are never deleted. Returns whether something was.
    @discardableResult
    func deleteBoardItem(at point: CGPoint, tolerance: CGFloat, visible: BoardVisibility = BoardVisibility()) -> Bool {
        if let hit = text(at: point, visible: visible) {
            deleteText(hit.id)
            return true
        }
        if let hit = via(at: point, visible: visible) {
            deleteVia(hit.id)
            return true
        }
        if let hit = hole(at: point, visible: visible) {
            deleteHole(hit.id)
            return true
        }
        if let hit = trace(at: point, tolerance: tolerance, visible: visible) {
            deleteTrace(hit.id)
            return true
        }
        return false
    }

    /// The selected traces, in drawing order, for ⌘C.
    func selectedTraces() -> [BoardTrace] {
        let ids = Set(boardSelection.items.compactMap { item -> UUID? in
            if case let .trace(id) = item { return id }
            return nil
        })
        return traces.filter { ids.contains($0.id) }
    }

    /// ⌘V on the PCB: adds a copy of each of `copied` moved by `offset`
    /// (mm), with new ids, its width and layer kept, joined to whatever it
    /// touches, as one step. The copies become the whole selection. A trace
    /// whose layer isn't on this board, or is a plane, is skipped. Returns
    /// the new ids.
    @discardableResult
    func pasteTraces(_ copied: [BoardTrace], offset: CGSize) -> [UUID] {
        let pasted = copied.filter { traceLayerRefusal($0.layer) == nil }
        guard !pasted.isEmpty else { return [] }
        return recordEdit(.pcb, "Paste") { () -> [UUID] in
            var ids: [UUID] = []
            for trace in pasted {
                let copy = BoardTrace(id: UUID(), layer: trace.layer, width: trace.width,
                                      points: trace.points.map {
                                          CGPoint(x: $0.x + offset.width, y: $0.y + offset.height)
                                      })
                traces.append(copy)
                ids.append(copy.id)
            }
            for id in ids { join(id) }
            boardSelection.set(Set(ids.map(BoardItem.trace)))
            return ids
        }
    }

    /// ⌘⌫ on the PCB: deletes every selected text, trace, via and hole, as
    /// one step. Selected footprints stay. Returns whether anything was
    /// deleted.
    @discardableResult
    func deleteBoardSelection() -> Bool {
        var textIDs: Set<UUID> = [], traceIDs: Set<UUID> = [], viaIDs: Set<UUID> = [], holeIDs: Set<UUID> = []
        var pourIDs: Set<UUID> = [], keepoutIDs: Set<UUID> = []
        for item in boardSelection.items {
            switch item {
            case let .text(id): textIDs.insert(id)
            case let .trace(id): traceIDs.insert(id)
            case let .via(id): viaIDs.insert(id)
            case let .hole(id): holeIDs.insert(id)
            case let .pour(id): pourIDs.insert(id)
            case let .keepout(id): keepoutIDs.insert(id)
            case .footprint: break
            }
        }
        guard !textIDs.isEmpty || !traceIDs.isEmpty || !viaIDs.isEmpty || !holeIDs.isEmpty || !pourIDs.isEmpty
                || !keepoutIDs.isEmpty else { return false }
        recordEdit(.pcb, "Delete") {
            texts.removeAll { textIDs.contains($0.id) }
            traces.removeAll { traceIDs.contains($0.id) }
            vias.removeAll { viaIDs.contains($0.id) }
            holes.removeAll { holeIDs.contains($0.id) }
            pours.removeAll { pourIDs.contains($0.id) }
            keepouts.removeAll { keepoutIDs.contains($0.id) }
            pruneSelection()
        }
        return true
    }

    /// ⌘X on the PCB, after the selected traces were copied: deletes just
    /// those traces (what ⌘C takes), as one step named "Cut". Returns
    /// whether any were deleted.
    @discardableResult
    func cutBoardTraces() -> Bool {
        let traceIDs = Set(selectedTraces().map(\.id))
        guard !traceIDs.isEmpty else { return false }
        recordEdit(.pcb, "Cut") {
            traces.removeAll { traceIDs.contains($0.id) }
            pruneSelection()
        }
        return true
    }

    /// Moves every vertex of the traces `ids` by `offset` (mm), then joins
    /// each one where it now lands.
    func moveTraces(_ ids: Set<UUID>, by offset: CGSize) {
        recordEdit(.pcb, "Move") {
            for index in traces.indices where ids.contains(traces[index].id) && !traces[index].locked {
                traces[index].points = traces[index].points.map {
                    CGPoint(x: $0.x + offset.width, y: $0.y + offset.height)
                }
            }
            for id in ids { join(id) }
        }
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

    // MARK: Vias (PCB)

    /// The via whose metal is under `point` (mm): the last one drawn there.
    /// Vias are drawn while any copper layer is.
    func via(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> BoardVia? {
        guard visible.showsAnyCopper else { return nil }
        return vias.last { Self.distance(from: point, to: $0.centre) <= $0.diameter / 2 }
    }

    /// Where a via placed at `point` lands: a vertex of a trace on any layer
    /// within `tolerance`, else the nearest point of a trace segment within
    /// `tolerance`, else the nearest trace-grid point.
    func viaAnchor(at point: CGPoint, tolerance: CGFloat) -> CGPoint {
        var best: (point: CGPoint, distance: CGFloat)?
        for trace in traces {
            for vertex in trace.points {
                let d = Self.distance(from: point, to: vertex)
                if d <= tolerance, d < best?.distance ?? .infinity { best = (vertex, d) }
            }
        }
        if let best { return best.point }
        for trace in traces {
            for (a, b) in trace.segments {
                let near = Self.nearest(on: a, b, to: point)
                let d = Self.distance(from: point, to: near)
                if d <= tolerance, d < best?.distance ?? .infinity { best = (near, d) }
            }
        }
        return best?.point ?? BoardGrid.snap(point, spacing: self.board.traceGrid)
    }

    /// Adds a via at `point` (already anchored) with the sizes of the class
    /// of the net there (`netRules(at:layer:)`; the defaults on no net or
    /// the Default class) and selects it. Nil on a 1-layer board, which has
    /// nothing to join.
    @discardableResult
    func addVia(at point: CGPoint) -> UUID? {
        guard board.layers > 1 else { return nil }
        let rules = netRules(at: point, layer: nil)
        return recordEdit(.pcb, "Add Via") { () -> UUID? in
            let via = BoardVia(id: UUID(), centre: point, drill: rules.viaDrill, diameter: rules.viaDiameter)
            vias.append(via)
            boardSelection.replace(with: .via(via.id))
            return via.id
        }
    }

    /// Moves a via's centre to `point`, snapped to the trace grid.
    func moveVia(_ id: UUID, to point: CGPoint) {
        recordEdit(.pcb, "Move") {
            guard let index = vias.firstIndex(where: { $0.id == id }), !vias[index].locked else { return }
            let target = BoardGrid.snap(point, spacing: self.board.traceGrid)
            if vias[index].centre != target { vias[index].centre = target }
        }
    }

    /// Moves a via's centre to exactly `point` (the sidebar's X and Y).
    func setViaCentre(_ id: UUID, _ point: CGPoint) {
        recordEdit(.pcb, "Move") {
            guard let index = vias.firstIndex(where: { $0.id == id }), !vias[index].locked,
                  vias[index].centre != point else { return }
            vias[index].centre = point
        }
    }

    /// Sets a via's hole, clamped to `BoardVia.drillRange` and to leave the
    /// thinnest ring inside its metal.
    func setViaDrill(_ id: UUID, _ mm: Double) {
        recordEdit(.pcb, "Change Via Hole") {
            guard let index = vias.firstIndex(where: { $0.id == id }) else { return }
            let most = min(BoardVia.drillRange.upperBound, vias[index].diameter - 2 * BoardVia.minimumRing)
            let value = min(max(mm, BoardVia.drillRange.lowerBound), most)
            if vias[index].drill != value { vias[index].drill = value }
        }
    }

    /// Sets a via's metal diameter, clamped to `BoardVia.diameterRange` and
    /// to keep the thinnest ring round its hole.
    func setViaDiameter(_ id: UUID, _ mm: Double) {
        recordEdit(.pcb, "Change Via Size") {
            guard let index = vias.firstIndex(where: { $0.id == id }) else { return }
            let least = max(BoardVia.diameterRange.lowerBound, vias[index].drill + 2 * BoardVia.minimumRing)
            let value = max(min(mm, BoardVia.diameterRange.upperBound), least)
            if vias[index].diameter != value { vias[index].diameter = value }
        }
    }

    /// Sets whether the solder mask covers a via on `side`.
    func setViaTenting(_ id: UUID, _ side: BoardSide, _ tented: Bool) {
        recordEdit(.pcb, "Change Via Tenting") {
            guard let index = vias.firstIndex(where: { $0.id == id }) else { return }
            switch side {
            case .top: if vias[index].tentedTop != tented { vias[index].tentedTop = tented }
            case .bottom: if vias[index].tentedBottom != tented { vias[index].tentedBottom = tented }
            }
        }
    }

    // MARK: Design rules (PCB)

    /// Sets the board's design rules (each 0 or more) and the manufacturer
    /// they are named after. One undo step.
    func setDesignRules(_ rules: DesignRules, manufacturer: DRCManufacturer) {
        recordEdit(.pcb, "Change Design Rules") {
            var next = rules
            for rule in DesignRules.groups.flatMap(\.rules) {
                let value = next[keyPath: rule.field]
                next[keyPath: rule.field] = value.isFinite ? max(0, value) : 0
            }
            if board.rules != next { board.rules = next }
            if board.rulesManufacturer != manufacturer { board.rulesManufacturer = manufacturer }
        }
    }

    // MARK: Net classes (PCB)

    /// Adds a net class named `name` (trimmed, at most 40 characters; blank
    /// is "Class") with the Default class's values and no nets, last in
    /// priority.
    @discardableResult
    func addNetClass(named name: String) -> UUID {
        recordEdit(.pcb, "Add Net Class") {
            let start = NetRules.defaults(for: board.rules)
            let netClass = NetClass(id: UUID(), name: Self.netClassName(name) ?? "Class", width: start.width,
                                    viaDrill: start.viaDrill, viaDiameter: start.viaDiameter)
            board.netClasses.append(netClass)
            return netClass.id
        }
    }

    func removeNetClass(_ id: UUID) {
        recordEdit(.pcb, "Delete Net Class") {
            board.netClasses.removeAll { $0.id == id }
        }
    }

    /// Renames a class; a blank name is ignored, a long one cut to 40
    /// characters.
    func renameNetClass(_ id: UUID, to name: String) {
        recordEdit(.pcb, "Rename Net Class") {
            guard let index = netClassIndex(id), let value = Self.netClassName(name),
                  board.netClasses[index].name != value else { return }
            board.netClasses[index].name = value
        }
    }

    /// Sets a class's trace width, clamped to `BoardTrace.widthRange`.
    func setNetClassWidth(_ id: UUID, _ mil: Int) {
        recordEdit(.pcb, "Change Net Class Width") {
            guard let index = netClassIndex(id) else { return }
            let value = min(max(mil, BoardTrace.widthRange.lowerBound), BoardTrace.widthRange.upperBound)
            if board.netClasses[index].width != value { board.netClasses[index].width = value }
        }
    }

    /// Sets a class's clearance, clamped to `LayerStyle.clearanceRange`;
    /// nil is the DRC's minimum.
    func setNetClassClearance(_ id: UUID, _ mm: Double?) {
        recordEdit(.pcb, "Change Net Class Clearance") {
            guard let index = netClassIndex(id) else { return }
            let range = LayerStyle.clearanceRange
            let value = mm.map { min(max($0, range.lowerBound), range.upperBound) }
            if board.netClasses[index].clearance != value { board.netClasses[index].clearance = value }
        }
    }

    /// Sets a class's via, clamped as `setViaDrill` and `setViaDiameter`
    /// clamp a via: the hole to `BoardVia.drillRange`, then the metal to
    /// `BoardVia.diameterRange` and at least the thinnest ring round it.
    func setNetClassVia(_ id: UUID, drill: Double, diameter: Double) {
        recordEdit(.pcb, "Change Net Class Via") {
            guard let index = netClassIndex(id) else { return }
            let hole = min(max(drill, BoardVia.drillRange.lowerBound), BoardVia.drillRange.upperBound,
                           BoardVia.diameterRange.upperBound - 2 * BoardVia.minimumRing)
            let least = max(BoardVia.diameterRange.lowerBound, hole + 2 * BoardVia.minimumRing)
            let metal = max(min(diameter, BoardVia.diameterRange.upperBound), least)
            if board.netClasses[index].viaDrill != hole { board.netClasses[index].viaDrill = hole }
            if board.netClasses[index].viaDiameter != metal { board.netClasses[index].viaDiameter = metal }
        }
    }

    /// Puts `net` in class `classID`, out of every other class; nil puts it
    /// back in the Default class.
    func assignNet(_ net: PlaneNet, to classID: UUID?) {
        recordEdit(.pcb, "Change Net Class") {
            for index in board.netClasses.indices where board.netClasses[index].members.contains(net) {
                board.netClasses[index].members.removeAll { $0 == net }
            }
            if let classID, let index = netClassIndex(classID) { board.netClasses[index].members.append(net) }
        }
    }

    /// Moves a class to `position` in the list (clamped); order is
    /// priority.
    func moveNetClass(_ id: UUID, to position: Int) {
        recordEdit(.pcb, "Move Net Class") {
            guard let index = netClassIndex(id) else { return }
            let target = min(max(position, 0), board.netClasses.count - 1)
            guard target != index else { return }
            let netClass = board.netClasses.remove(at: index)
            board.netClasses.insert(netClass, at: target)
        }
    }

    /// The width and via new copper at `point` gets: those of the class of
    /// the net there — a pad's whose copper holds the point on `layer`
    /// (a through-hole pad on any layer; nil `layer` is any), else a via's
    /// whose metal holds it, else a trace's on `layer` whose copper holds
    /// it — or the Default class's when it is on no net or a net in no class.
    func netRules(at point: CGPoint, layer: CopperLayer?) -> NetRules {
        let copper = connectivity()
        guard let net = net(at: point, layer: layer, in: copper),
              let netClass = netClass(ofNet: net, in: copper) else { return NetRules.defaults(for: board.rules) }
        return NetRules(width: netClass.width, clearance: netClass.clearance, viaDrill: netClass.viaDrill,
                        viaDiameter: netClass.viaDiameter)
    }

    /// The net (of `copper`) whose copper holds `point`, by the order
    /// `netRules(at:layer:)` describes: a plated pad's on `layer`, else a
    /// via's, else a trace's on `layer`; nil when none does.
    func net(at point: CGPoint, layer: CopperLayer?, in copper: BoardConnectivity) -> Int? {
        func onLayer(_ pad: BoardConnectivity.Pad) -> Bool {
            layer == nil || pad.pad.drill > 0 || pad.side.copperLayer == layer
        }
        if let p = copper.pads.indices.first(where: {
            copper.pads[$0].pad.plated && onLayer(copper.pads[$0]) && copper.pads[$0].pad.rect.contains(point)
                && copper.netOfPad[$0] != nil
        }) {
            return copper.netOfPad[p]
        }
        let netOfRoot = copper.netOfRoot()
        if let v = copper.vias.indices.first(where: {
            Self.distance(from: point, to: copper.vias[$0].centre) <= copper.vias[$0].diameter / 2
                && netOfRoot[copper.root[copper.viaNode($0)]] != nil
        }) {
            return netOfRoot[copper.root[copper.viaNode(v)]]
        }
        if let t = copper.traces.indices.first(where: { t in
            let trace = copper.traces[t]
            return (layer == nil || trace.layer == layer) && netOfRoot[copper.root[copper.traceNode(t)]] != nil
                && trace.segments.contains { Self.distance(from: point, toSegment: $0.0, $0.1) <= trace.widthMM / 2 }
        }) {
            return netOfRoot[copper.root[copper.traceNode(t)]]
        }
        return nil
    }

    /// The class net `net` of `connectivity` is in: the first, in list
    /// order, any of whose members names it; nil is the Default class.
    func netClass(ofNet net: Int, in connectivity: BoardConnectivity) -> NetClass? {
        board.netClasses.first { $0.members.contains { connectivity.net(of: $0) == net } }
    }


    // MARK: Net layer rules and groups (PCB)

    /// Sets a class's layer rule; nil is any trace layer. A rule with no
    /// layers is ignored.
    func setNetClassLayerRule(_ id: UUID, _ rule: LayerRule?) {
        recordEdit(.pcb, "Change Layer Rule") {
            guard let index = netClassIndex(id), rule.map({ !$0.layers.isEmpty }) ?? true,
                  board.netClasses[index].layerRule != rule else { return }
            board.netClasses[index].layerRule = rule
        }
    }

    /// Sets one net's own layer rule, before its class's; nil removes it.
    /// A rule with no layers is ignored.
    func setNetLayerRule(_ net: PlaneNet, _ rule: LayerRule?) {
        recordEdit(.pcb, "Change Layer Rule") {
            guard rule.map({ !$0.layers.isEmpty }) ?? true else { return }
            if let rule {
                if let index = board.netOverrides.firstIndex(where: { $0.net == net }) {
                    if board.netOverrides[index].layerRule != rule { board.netOverrides[index].layerRule = rule }
                } else {
                    board.netOverrides.append(NetOverride(net: net, layerRule: rule))
                }
            } else {
                board.netOverrides.removeAll { $0.net == net }
            }
        }
    }

    /// Adds an empty group (trimmed name, at most 40 characters; blank is
    /// "Group"), last.
    @discardableResult
    func addNetGroup(named name: String, mode: NetGroupMode) -> UUID {
        recordEdit(.pcb, "Add Net Group") {
            let group = NetGroup(id: UUID(), name: Self.netClassName(name) ?? "Group", mode: mode)
            board.netGroups.append(group)
            return group.id
        }
    }

    func removeNetGroup(_ id: UUID) {
        recordEdit(.pcb, "Delete Net Group") {
            board.netGroups.removeAll { $0.id == id }
        }
    }

    func renameNetGroup(_ id: UUID, to name: String) {
        recordEdit(.pcb, "Rename Net Group") {
            guard let index = netGroupIndex(id), let value = Self.netClassName(name),
                  board.netGroups[index].name != value else { return }
            board.netGroups[index].name = value
        }
    }

    /// Changes a group's mode. A group with more than 2 nets can't be a
    /// pair: refused, with `editorMessage` saying why. Returns whether the
    /// group has `mode` now.
    @discardableResult
    func setNetGroupMode(_ id: UUID, _ mode: NetGroupMode) -> Bool {
        guard let index = netGroupIndex(id) else { return false }
        let group = board.netGroups[index]
        if mode == .diffPair, group.members.count > NetGroup.pairSize {
            editorMessage = "A differential pair has 2 nets; \(group.name) has \(group.members.count)."
            return false
        }
        recordEdit(.pcb, "Change Net Group") {
            if board.netGroups[index].mode != mode { board.netGroups[index].mode = mode }
        }
        return true
    }

    /// Sets a group's tolerance, clamped to `NetGroup.toleranceRange`.
    func setNetGroupTolerance(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.tolerance, mm, NetGroup.toleranceRange)
    }

    /// Sets a pair's gap, clamped to `NetGroup.gapRange`.
    func setNetGroupGap(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.gap, mm, NetGroup.gapRange)
    }

    /// Sets how much of a pair may run apart, clamped to
    /// `NetGroup.maxUncoupledRange`.
    func setNetGroupMaxUncoupled(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.maxUncoupled, mm, NetGroup.maxUncoupledRange)
    }

    /// Sets a group's largest tuning bump, clamped to
    /// `NetGroup.maxMeanderRange`.
    func setNetGroupMaxMeander(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.maxMeander, mm, NetGroup.maxMeanderRange)
    }

    private func setNetGroupValue(_ id: UUID, _ key: WritableKeyPath<NetGroup, Double>, _ mm: Double,
                                  _ range: ClosedRange<Double>) {
        recordEdit(.pcb, "Change Net Group") {
            guard let index = netGroupIndex(id) else { return }
            let value = min(max(mm, range.lowerBound), range.upperBound)
            if board.netGroups[index][keyPath: key] != value { board.netGroups[index][keyPath: key] = value }
        }
    }

    /// Puts `net` in group `groupID`, out of any other group. A pair that
    /// already has 2 other nets refuses it, with `editorMessage` saying
    /// why. Returns whether the net is in the group now.
    @discardableResult
    func assignNet(_ net: PlaneNet, toGroup groupID: UUID) -> Bool {
        guard let index = netGroupIndex(groupID) else { return false }
        let group = board.netGroups[index]
        if group.members.contains(net) { return true }
        if group.mode == .diffPair, group.members.count >= NetGroup.pairSize {
            editorMessage = "\(group.name) already has its 2 nets."
            return false
        }
        recordEdit(.pcb, "Change Net Group") {
            for other in board.netGroups.indices where board.netGroups[other].members.contains(net) {
                board.netGroups[other].members.removeAll { $0 == net }
            }
            board.netGroups[index].members.append(net)
        }
        return true
    }

    /// Takes `net` out of whichever group it is in.
    func unassignNet(_ net: PlaneNet) {
        recordEdit(.pcb, "Change Net Group") {
            for index in board.netGroups.indices where board.netGroups[index].members.contains(net) {
                board.netGroups[index].members.removeAll { $0 == net }
            }
        }
    }

    /// The layer rule net `net` of `connectivity` follows: its own
    /// override, else its class's, else none.
    func layerRule(ofNet net: Int, in connectivity: BoardConnectivity) -> LayerRule? {
        if let own = board.netOverrides.first(where: { $0.layerRule != nil && connectivity.net(of: $0.net) == net }) {
            return own.layerRule
        }
        return netClass(ofNet: net, in: connectivity)?.layerRule
    }

    /// The trace layers net `net` may use and those it prefers. Only: the
    /// rule's layers that are trace layers on the board (none means it
    /// can't be routed), with no preference. Prefer: every trace layer,
    /// preferring the rule's. No rule: every trace layer, no preference.
    func effectiveLayers(ofNet net: Int, in connectivity: BoardConnectivity) -> EffectiveLayers {
        let trace = board.traceLayers
        guard let rule = layerRule(ofNet: net, in: connectivity) else {
            return EffectiveLayers(allowed: trace, preferred: [], rule: nil)
        }
        let named = trace.filter(rule.layers.contains)
        switch rule.mode {
        case .only: return EffectiveLayers(allowed: named, preferred: [], rule: rule)
        case .prefer: return EffectiveLayers(allowed: trace, preferred: named, rule: rule)
        }
    }

    /// The group net `net` of `connectivity` is in: the first any of whose
    /// members names it.
    func group(ofNet net: Int, in connectivity: BoardConnectivity) -> NetGroup? {
        board.netGroups.first { $0.members.contains { connectivity.net(of: $0) == net } }
    }

    private func netGroupIndex(_ id: UUID) -> Int? {
        board.netGroups.firstIndex { $0.id == id }
    }
    private func netClassIndex(_ id: UUID) -> Int? {
        board.netClasses.firstIndex { $0.id == id }
    }

    /// A class name as entered: trimmed and cut to the longest allowed, or
    /// nil when blank.
    private static func netClassName(_ name: String) -> String? {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        return trimmed.isEmpty ? nil : String(trimmed.prefix(NetClass.nameLength.upperBound))
    }

    func deleteVia(_ id: UUID) {
        recordEdit(.pcb, "Delete Via") {
            vias.removeAll { $0.id == id }
            pruneSelection()
        }
    }

    /// The traces whose copper touches a via's metal, on any layer.
    func traces(touching via: BoardVia) -> [BoardTrace] {
        traces.filter { trace in
            trace.segments.contains {
                Self.distance(from: via.centre, toSegment: $0.0, $0.1) <= via.diameter / 2 + trace.widthMM / 2
            }
        }
    }

    // MARK: Pours and keep-outs (PCB)

    /// `rect` snapped to the trace grid, at least `BoardPour.minimumSide` wide and tall.
    private func snappedRect(_ rect: CGRect) -> CGRect {
        let grid = board.traceGrid
        let r = rect.standardized
        let minX = BoardGrid.snap(r.minX, spacing: grid), minY = BoardGrid.snap(r.minY, spacing: grid)
        let maxX = BoardGrid.snap(r.maxX, spacing: grid), maxY = BoardGrid.snap(r.maxY, spacing: grid)
        return BoardPour.normalized(CGRect(x: minX, y: minY, width: maxX - minX, height: maxY - minY))
    }

    /// Adds a pour over `rect` (snapped to the trace grid) on `layer`, and
    /// selects it. It takes the settings of the last pour added; its net is
    /// the last pour's, else ground when the schematic has it, else none.
    /// Nil when `layer` is not a trace layer of the board.
    @discardableResult
    func addPour(rect: CGRect, layer: CopperLayer) -> UUID? {
        guard board.traceLayers.contains(layer) else { return nil }
        return recordEdit(.pcb, "Add Pour") { () -> UUID in
            var pour = BoardPour(id: UUID(), rect: snappedRect(rect), layer: layer)
            if let like = pours.last {
                pour.net = like.net
                pour.clearance = like.clearance
                pour.connection = like.connection
                pour.thermalGap = like.thermalGap
                pour.spokeWidth = like.spokeWidth
                pour.minWidth = like.minWidth
            } else if planeNetChoices().contains(where: { $0.net == .ground }) {
                pour.net = .ground
            }
            pours.append(pour)
            boardSelection.replace(with: .pour(pour.id))
            return pour.id
        }
    }

    /// Moves a pour so its top-left corner is at `origin`, snapped to the trace grid.
    func movePour(_ id: UUID, to origin: CGPoint) {
        recordEdit(.pcb, "Move") {
            guard let index = pours.firstIndex(where: { $0.id == id }), !pours[index].locked else { return }
            let target = CGRect(origin: BoardGrid.snap(origin, spacing: board.traceGrid), size: pours[index].rect.size)
            if pours[index].rect != target { pours[index].rect = target }
        }
    }

    /// Sets a pour's rectangle (the sidebar's X, Y, width and height, a
    /// resize), snapped to the trace grid.
    func setPourRect(_ id: UUID, _ rect: CGRect, name: String = "Resize") {
        recordEdit(.pcb, name) {
            guard let index = pours.firstIndex(where: { $0.id == id }), !pours[index].locked else { return }
            let target = snappedRect(rect)
            if pours[index].rect != target { pours[index].rect = target }
        }
    }

    /// Puts a pour on `layer`, a trace layer of the board.
    func setPourLayer(_ id: UUID, _ layer: CopperLayer) {
        recordEdit(.pcb, "Change Pour Layer") {
            guard board.traceLayers.contains(layer), let index = pours.firstIndex(where: { $0.id == id }),
                  pours[index].layer != layer else { return }
            pours[index].layer = layer
        }
    }

    /// Sets a pour's net (`planeNetChoices`), or none.
    func setPourNet(_ id: UUID, _ net: PlaneNet?) {
        recordEdit(.pcb, "Change Pour Net") {
            guard let index = pours.firstIndex(where: { $0.id == id }), pours[index].net != net else { return }
            pours[index].net = net
        }
    }

    /// Sets a pour's clearance (mm), clamped to `LayerStyle.clearanceRange`;
    /// nil is the DRC's minimum.
    func setPourClearance(_ id: UUID, _ mm: Double?) {
        recordEdit(.pcb, "Change Pour") {
            guard let index = pours.firstIndex(where: { $0.id == id }) else { return }
            let value = mm.map { min(max($0, LayerStyle.clearanceRange.lowerBound), LayerStyle.clearanceRange.upperBound) }
            if pours[index].clearance != value { pours[index].clearance = value }
        }
    }

    func setPourConnection(_ id: UUID, _ connection: PourConnection) {
        recordEdit(.pcb, "Change Pour") {
            guard let index = pours.firstIndex(where: { $0.id == id }), pours[index].connection != connection else { return }
            pours[index].connection = connection
        }
    }

    func setPourThermalGap(_ id: UUID, _ mm: Double) {
        recordEdit(.pcb, "Change Pour") {
            guard let index = pours.firstIndex(where: { $0.id == id }) else { return }
            let value = min(max(mm, BoardPour.gapRange.lowerBound), BoardPour.gapRange.upperBound)
            if pours[index].thermalGap != value { pours[index].thermalGap = value }
        }
    }

    func setPourSpokeWidth(_ id: UUID, _ mm: Double) {
        recordEdit(.pcb, "Change Pour") {
            guard let index = pours.firstIndex(where: { $0.id == id }) else { return }
            let value = min(max(mm, BoardPour.spokeRange.lowerBound), BoardPour.spokeRange.upperBound)
            if pours[index].spokeWidth != value { pours[index].spokeWidth = value }
        }
    }

    func setPourMinWidth(_ id: UUID, _ mm: Double) {
        recordEdit(.pcb, "Change Pour") {
            guard let index = pours.firstIndex(where: { $0.id == id }) else { return }
            let value = min(max(mm, BoardPour.minWidthRange.lowerBound), BoardPour.minWidthRange.upperBound)
            if pours[index].minWidth != value { pours[index].minWidth = value }
        }
    }

    func deletePour(_ id: UUID) {
        recordEdit(.pcb, "Delete") {
            pours.removeAll { $0.id == id }
            pruneSelection()
        }
    }

    /// Adds a keep-out over `rect` (snapped to the trace grid) on `layers`
    /// (nil: every layer), and selects it.
    @discardableResult
    func addKeepout(rect: CGRect, layers: Set<CopperLayer>? = nil) -> UUID {
        recordEdit(.pcb, "Add Keep-out") { () -> UUID in
            let keepout = BoardKeepout(id: UUID(), rect: snappedRect(rect), layers: layers)
            keepouts.append(keepout)
            boardSelection.replace(with: .keepout(keepout.id))
            return keepout.id
        }
    }

    /// Moves a keep-out so its top-left corner is at `origin`, snapped to the trace grid.
    func moveKeepout(_ id: UUID, to origin: CGPoint) {
        recordEdit(.pcb, "Move") {
            guard let index = keepouts.firstIndex(where: { $0.id == id }), !keepouts[index].locked else { return }
            let target = CGRect(origin: BoardGrid.snap(origin, spacing: board.traceGrid), size: keepouts[index].rect.size)
            if keepouts[index].rect != target { keepouts[index].rect = target }
        }
    }

    func setKeepoutRect(_ id: UUID, _ rect: CGRect, name: String = "Resize") {
        recordEdit(.pcb, name) {
            guard let index = keepouts.firstIndex(where: { $0.id == id }), !keepouts[index].locked else { return }
            let target = snappedRect(rect)
            if keepouts[index].rect != target { keepouts[index].rect = target }
        }
    }

    /// Sets the layers a keep-out applies to; nil is every layer.
    func setKeepoutLayers(_ id: UUID, _ layers: Set<CopperLayer>?) {
        recordEdit(.pcb, "Change Keep-out") {
            guard let index = keepouts.firstIndex(where: { $0.id == id }), keepouts[index].layers != layers else { return }
            keepouts[index].layers = layers
        }
    }

    func deleteKeepout(_ id: UUID) {
        recordEdit(.pcb, "Delete") {
            keepouts.removeAll { $0.id == id }
            pruneSelection()
        }
    }

    // MARK: Holes (PCB)

    /// The hole under `point` (mm): the last one drawn there, within its
    /// land (plated) or its hole. Holes are drawn while any copper layer is.
    func hole(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> BoardHole? {
        guard visible.showsAnyCopper else { return nil }
        return holes.last { Self.distance(from: point, to: $0.centre) <= $0.outer / 2 }
    }

    /// Adds a hole at `point`, snapped to the trace grid, and selects it.
    /// It takes the size, plating and net of the last hole added, so a set
    /// of mounting holes is quick to place; the first is a non-plated M3.
    @discardableResult
    func addHole(at point: CGPoint) -> UUID {
        recordEdit(.pcb, "Add Hole") { () -> UUID in
            let like = holes.last ?? BoardHole(id: UUID(), centre: .zero)
            let hole = BoardHole(id: UUID(), centre: BoardGrid.snap(point, spacing: self.board.traceGrid),
                                 size: like.size, drill: like.drill, diameter: like.diameter, plated: like.plated,
                                 net: like.net)
            holes.append(hole)
            boardSelection.replace(with: .hole(hole.id))
            return hole.id
        }
    }

    /// Moves a hole's centre to `point`, snapped to the trace grid.
    func moveHole(_ id: UUID, to point: CGPoint) {
        recordEdit(.pcb, "Move") {
            guard let index = holes.firstIndex(where: { $0.id == id }), !holes[index].locked else { return }
            let target = BoardGrid.snap(point, spacing: self.board.traceGrid)
            if holes[index].centre != target { holes[index].centre = target }
        }
    }

    /// Moves a hole's centre to exactly `point` (the sidebar's X and Y).
    func setHoleCentre(_ id: UUID, _ point: CGPoint) {
        recordEdit(.pcb, "Move") {
            guard let index = holes.firstIndex(where: { $0.id == id }), !holes[index].locked,
                  holes[index].centre != point else { return }
            holes[index].centre = point
        }
    }

    /// Sets a hole's size: a screw size sets its hole and land.
    func setHoleSize(_ id: UUID, _ size: HoleSize) {
        recordEdit(.pcb, "Change Hole Size") {
            guard let index = holes.firstIndex(where: { $0.id == id }) else { return }
            var hole = holes[index]
            hole.apply(size)
            if holes[index] != hole { holes[index] = hole }
        }
    }

    /// Sets a hole's drill, clamped to `BoardHole.drillRange`. The land
    /// grows to keep the thinnest ring round it. The size becomes Custom.
    func setHoleDrill(_ id: UUID, _ mm: Double) {
        recordEdit(.pcb, "Change Hole Size") {
            guard let index = holes.firstIndex(where: { $0.id == id }) else { return }
            var hole = holes[index]
            hole.drill = min(max(mm, BoardHole.drillRange.lowerBound), BoardHole.drillRange.upperBound)
            hole.diameter = max(hole.diameter, hole.drill + 2 * BoardHole.minimumRing)
            if hole.drill != holes[index].drill { hole.size = .custom }
            if holes[index] != hole { holes[index] = hole }
        }
    }

    /// Sets a hole's land diameter, clamped to `BoardHole.diameterRange` and
    /// to keep the thinnest ring round its hole. The size becomes Custom.
    func setHoleDiameter(_ id: UUID, _ mm: Double) {
        recordEdit(.pcb, "Change Hole Size") {
            guard let index = holes.firstIndex(where: { $0.id == id }) else { return }
            let least = max(BoardHole.diameterRange.lowerBound, holes[index].drill + 2 * BoardHole.minimumRing)
            let value = max(min(mm, BoardHole.diameterRange.upperBound), least)
            guard holes[index].diameter != value else { return }
            holes[index].diameter = value
            holes[index].size = .custom
        }
    }

    /// Makes a hole plated (a copper land that can carry a net) or bare.
    func setHolePlated(_ id: UUID, _ plated: Bool) {
        recordEdit(.pcb, plated ? "Plate Hole" : "Unplate Hole") {
            guard let index = holes.firstIndex(where: { $0.id == id }), holes[index].plated != plated else { return }
            holes[index].plated = plated
        }
    }

    /// Sets a plated hole's net (`planeNetChoices`), or none.
    func setHoleNet(_ id: UUID, _ net: PlaneNet?) {
        recordEdit(.pcb, "Change Hole Net") {
            guard let index = holes.firstIndex(where: { $0.id == id }), holes[index].net != net else { return }
            holes[index].net = net
        }
    }

    func deleteHole(_ id: UUID) {
        recordEdit(.pcb, "Delete Hole") {
            holes.removeAll { $0.id == id }
            pruneSelection()
        }
    }

    /// The traces whose copper touches a plated hole's land, on any layer.
    func traces(touching hole: BoardHole) -> [BoardTrace] {
        guard hole.plated else { return [] }
        return traces.filter { trace in
            trace.segments.contains {
                Self.distance(from: hole.centre, toSegment: $0.0, $0.1) <= hole.diameter / 2 + trace.widthMM / 2
            }
        }
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
    nonisolated static func segment(_ a: CGPoint, _ b: CGPoint, touches rect: CGRect) -> Bool {
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

    // MARK: Lock Position (PCB)

    /// Whether a PCB item's position is locked. False for an item that
    /// isn't there.
    func isLocked(_ item: BoardItem) -> Bool {
        switch item {
        case let .footprint(id): chips.first { $0.id == id }?.board?.locked ?? false
        case let .text(id): texts.first { $0.id == id }?.locked ?? false
        case let .trace(id): traces.first { $0.id == id }?.locked ?? false
        case let .via(id): vias.first { $0.id == id }?.locked ?? false
        case let .hole(id): holes.first { $0.id == id }?.locked ?? false
        case let .pour(id): pours.first { $0.id == id }?.locked ?? false
        case let .keepout(id): keepouts.first { $0.id == id }?.locked ?? false
        }
    }

    /// Locks or unlocks a PCB item's position (the sidebar's Lock Position).
    func setLocked(_ item: BoardItem, _ locked: Bool) {
        guard isLocked(item) != locked else { return }
        recordEdit(.pcb, locked ? "Lock Position" : "Unlock Position") {
            switch item {
            case let .footprint(id):
                if let index = chips.firstIndex(where: { $0.id == id }), chips[index].board != nil {
                    chips[index].board?.locked = locked
                }
            case let .text(id):
                if let index = texts.firstIndex(where: { $0.id == id }) { texts[index].locked = locked }
            case let .trace(id):
                if let index = traces.firstIndex(where: { $0.id == id }) { traces[index].locked = locked }
            case let .via(id):
                if let index = vias.firstIndex(where: { $0.id == id }) { vias[index].locked = locked }
            case let .hole(id):
                if let index = holes.firstIndex(where: { $0.id == id }) { holes[index].locked = locked }
            case let .pour(id):
                if let index = pours.firstIndex(where: { $0.id == id }) { pours[index].locked = locked }
            case let .keepout(id):
                if let index = keepouts.firstIndex(where: { $0.id == id }) { keepouts[index].locked = locked }
            }
        }
    }

    // MARK: Group drags (PCB)

    /// Where every selected footprint (its centre) and text (its anchor) is
    /// now, for a group drag. Locked items are left out, so they stay put.
    func boardGroupBase(grab: CGPoint? = nil) -> BoardGroupBase {
        var base = BoardGroupBase()
        base.grab = grab
        for item in boardSelection.items where !isLocked(item) {
            switch item {
            case let .footprint(id):
                if let centre = chips.first(where: { $0.id == id })?.board?.centre { base.footprints[id] = centre }
            case let .text(id):
                if let anchor = texts.first(where: { $0.id == id })?.anchor { base.texts[id] = anchor }
            case let .trace(id):
                if let points = traces.first(where: { $0.id == id })?.points { base.traces[id] = points }
            case let .via(id):
                if let centre = vias.first(where: { $0.id == id })?.centre { base.vias[id] = centre }
            case let .hole(id):
                if let centre = holes.first(where: { $0.id == id })?.centre { base.holes[id] = centre }
            case let .pour(id):
                if let rect = pours.first(where: { $0.id == id })?.rect { base.pours[id] = rect }
            case let .keepout(id):
                if let rect = keepouts.first(where: { $0.id == id })?.rect { base.keepouts[id] = rect }
            }
        }
        return base
    }

    /// One frame of a group drag on the PCB. One item moves as it always
    /// has, snapped onto the grid; a lone trace by the end nearer the grab,
    /// onto the trace grid. Several all move by the same `delta`
    /// snapped to the board grid, so they keep their places relative to
    /// each other.
    func moveBoardGroup(_ base: BoardGroupBase, by delta: CGSize) {
        recordEdit(.pcb, "Move") {
            func moved(_ point: CGPoint, by offset: CGSize) -> CGPoint {
                CGPoint(x: point.x + offset.width, y: point.y + offset.height)
            }
            var offset = CGSize(width: BoardGrid.snap(delta.width, spacing: self.board.grid),
                                height: BoardGrid.snap(delta.height, spacing: self.board.grid))
            // A lone trace snaps by its end: the one nearer where it was
            // grabbed (its start without a grab) lands on the trace grid.
            // A click (no movement yet) leaves it where it is.
            if base.count == 1, delta != .zero, let points = base.traces.values.first, let first = points.first,
               let last = points.last {
                let end = base.grab.map {
                    Self.distance(from: $0, to: last) < Self.distance(from: $0, to: first) ? last : first
                } ?? first
                let target = BoardGrid.snap(moved(end, by: delta), spacing: self.board.traceGrid)
                offset = CGSize(width: target.x - end.x, height: target.y - end.y)
            }
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
                // A via placed on a trace stays there until the pointer moves.
                if let (id, centre) = base.vias.first, delta != .zero { moveVia(id, to: moved(centre, by: delta)) }
                if let (id, centre) = base.holes.first { moveHole(id, to: moved(centre, by: delta)) }
                if let (id, rect) = base.pours.first { movePour(id, to: moved(rect.origin, by: delta)) }
                if let (id, rect) = base.keepouts.first { moveKeepout(id, to: moved(rect.origin, by: delta)) }
                return
            }
            for index in pours.indices {
                guard let rect = base.pours[pours[index].id] else { continue }
                let target = CGRect(origin: moved(rect.origin, by: offset), size: rect.size)
                if pours[index].rect != target { pours[index].rect = target }
            }
            for index in keepouts.indices {
                guard let rect = base.keepouts[keepouts[index].id] else { continue }
                let target = CGRect(origin: moved(rect.origin, by: offset), size: rect.size)
                if keepouts[index].rect != target { keepouts[index].rect = target }
            }
            for index in holes.indices {
                guard let centre = base.holes[holes[index].id] else { continue }
                let target = moved(centre, by: offset)
                if holes[index].centre != target { holes[index].centre = target }
            }
            for index in vias.indices {
                guard let centre = base.vias[vias[index].id] else { continue }
                let target = moved(centre, by: offset)
                if vias[index].centre != target { vias[index].centre = target }
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
    }

    /// The end of a PCB group drag: each moved trace joins what it now
    /// touches. Traces that weren't selected stay where they are.
    func settleBoardGroup(_ base: BoardGroupBase) {
        recordEdit(.pcb, "Move") {
            for id in base.traces.keys { join(id) }
        }
    }

    /// What a Select press at `point` (board mm) picks: a text, else a via,
    /// else a trace (within `tolerance` of its edge), else a footprint; Top
    /// before Bottom within each. Only drawn items count.
    func boardItem(at point: CGPoint, tolerance: CGFloat,
                   visible: BoardVisibility = BoardVisibility()) -> BoardItem? {
        if let text = text(at: point, visible: visible) { return .text(text.id) }
        if let via = via(at: point, visible: visible) { return .via(via.id) }
        if let hole = hole(at: point, visible: visible) { return .hole(hole.id) }
        if let trace = trace(at: point, tolerance: tolerance, visible: visible) { return .trace(trace.id) }
        if let chip = footprint(at: point, visible: visible), chip.board != nil { return .footprint(chip.id) }
        return nil
    }

    /// Sets or clears an EEPROM's load file. Does nothing for other parts.
    /// The memory window calls it only while the simulation is stopped.
    func setMemoryFile(_ id: UUID, _ file: MemoryFile?) {
        recordEdit(.schematic, "Change Load File") {
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].part.kind == PART_EEPROM else { return }
            chips[index].memoryFile = file
        }
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
                                                 buses: buses, labels: netLabels,
                                                 known: currentVoltageIssues(library), library: library) {
                editorMessage = VoltageCheck.message(issue, chips: chips, edit: .turnOn)
                return nil
            }
        }
        if pressed { editorMessage = nil }
        chips[index].switchesOn[0] = pressed
        return pressed
    }

    /// Every switch's positions (DIP and tactile), by part, for
    /// `restoreSwitches` to put back.
    var switchPositions: [UUID: [Bool]] {
        Dictionary(uniqueKeysWithValues: chips.lazy.filter { !$0.switchesOn.isEmpty }.map { ($0.id, $0.switchesOn) })
    }

    /// Puts switches back to `saved` (from `switchPositions`), for example
    /// when play mode ends: flips made while the simulation ran are input to
    /// that run, not edits. Parts not in `saved`, or whose position count
    /// changed, are left alone.
    func restoreSwitches(_ saved: [UUID: [Bool]]) {
        for index in chips.indices {
            guard let positions = saved[chips[index].id],
                  positions.count == chips[index].switchesOn.count,
                  positions != chips[index].switchesOn else { continue }
            chips[index].switchesOn = positions
        }
    }

    /// Releases every pressed tactile switch (play mode ended).
    func releaseAllButtons() {
        for index in chips.indices where chips[index].isPressed {
            chips[index].switchesOn[0] = false
        }
    }

    func toggleSwitch(_ id: UUID, position: Int) -> Bool? {
        return recordEdit(.schematic, "Flip Switch") { () -> Bool? in
            guard let index = chips.firstIndex(where: { $0.id == id }),
                  chips[index].switchesOn.indices.contains(position - 1) else { return nil }
            if !chips[index].switchesOn[position - 1], let library {
                var candidate = chips
                candidate[index].switchesOn[position - 1] = true
                if let issue = VoltageCheck.firstNew(before: chips, wires, after: candidate, wires,
                                                     buses: buses, labels: netLabels,
                                                 known: currentVoltageIssues(library), library: library) {
                    editorMessage = VoltageCheck.message(issue, chips: chips, edit: .turnOn)
                    return nil
                }
            }
            editorMessage = nil
            chips[index].switchesOn[position - 1].toggle()
            return chips[index].switchesOn[position - 1]
        }
    }

    // MARK: Pins

    /// Where a wire leaves or enters a pin: the end of its pin stub.
    func pinTip(_ ref: PinRef) -> CGPoint? {
        guard let position = chipIndex[ref.chipID] else { return nil }
        let chip = chips[position]
        guard let slot = chip.layout.slots.first(where: { $0.pin.index == ref.index }) else { return nil }
        return chip.tip(of: slot)
    }

    /// Per chip, the pins (`ChipPin.index`) a wire ends on. The schematic
    /// leaves out those pins' numbers.
    var connectedPins: [UUID: Set<Int>] {
        var pins: [UUID: Set<Int>] = [:]
        for wire in wires {
            for end in [wire.start, wire.end] {
                if case .pin(let ref) = end { pins[ref.chipID, default: []].insert(ref.index) }
            }
        }
        return pins
    }

    /// Tip position of every pin on every placed chip and bus port.
    var pinTips: [CGPoint] {
        chips.flatMap { chip in
            chip.layout.slots.compactMap { slot in
                pinTip(PinRef(chipID: chip.id, index: slot.pin.index))
            }
        } + busPinRefs.compactMap { busPinTip($0) } + netLabels.map(\.tip)
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
        case let .label(id):
            return netLabelTip(id)
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
        case let .label(id):
            return netLabelTip(id)
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
        case .label:
            return .horizontal
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
        if let id = netLabelTip(at: point, radius: pinRadius) {
            return .label(id)
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
        guard let wire = wires.first(where: { $0.id == id }) else { return [] }
        return path(for: wire)
    }

    /// `path(forWire:)` for a wire already in hand: no search of `wires`, so
    /// a loop over every wire stays linear.
    func path(for wire: Wire) -> [CGPoint] {
        guard let start = endTip(wire.start),
              let end = endTip(wire.end) else {
            return []
        }
        if let kept = keptPaths[wire.id], kept.start == start, kept.end == end, kept.wire == wire { return kept.path }
        // Same rules as settleWires, applied on the fly, so the wire looks the
        // same while dragging as it does after the drop.
        let interior = OrthogonalRoute.settled(start: start, interior: wire.interior, end: end)
        let path = [start] + interior + [end]
        // Routes of wires that are gone would pile up.
        if keptPaths.count > 2 * wires.count + 16 { keptPaths.removeAll(keepingCapacity: true) }
        keptPaths[wire.id] = (wire, start, end, path)
        return path
    }

    var wirePaths: [[CGPoint]] {
        wires.map { path(for: $0) }
    }

    /// Connects two anchors with an orthogonal route. A grid point on an existing
    /// wire becomes a junction: that wire is split there and the new wire branches
    /// from it.
    ///
    /// Refused, with nothing changed, when the wire would put a voltage out of
    /// range on a part (a problem the drawing doesn't already have). Returns
    /// why, which is also `editorMessage`; nil otherwise.
    func connect(_ start: LineAnchor, _ end: LineAnchor, path: [CGPoint]? = nil) -> String? {
        return recordEdit(.schematic, "Add Wire") { () -> String? in
            guard start != end,
                  let a = anchorTip(start), let b = anchorTip(end), a != b else {
                return nil
            }

            // The check runs on stand-in ends before anything changes: resolving a
            // point on a wire for real splits that wire.
            if let library, let standInStart = standIn(for: start), let standInEnd = standIn(for: end) {
                let candidate = wires + [Wire(start: standInStart, end: standInEnd, interior: [])]
                if let issue = VoltageCheck.firstNew(before: chips, wires, after: chips, candidate,
                                                     buses: buses, labels: netLabels,
                                                 known: currentVoltageIssues(library), library: library) {
                    editorMessage = VoltageCheck.message(issue, chips: chips, edit: .connect)
                    return editorMessage
                }
            }
            editorMessage = nil

            // Resolve the start first, so a split for it is in place before the end is looked up.
            guard let from = endpoint(for: start), let to = endpoint(for: end),
                  let fromTip = endTip(from), let toTip = endTip(to) else {
                // A split made for the start is joined back up.
                joinThroughJunctions()
                return nil
            }
            // The ends are always the tips; a drawn path gives the bends between.
            let bends = path.map { Array($0.dropFirst().dropLast()) }
                ?? Array(OrthogonalRoute.leg(from: fromTip, to: toTip, first: anchorAxis(start),
                                             last: anchorAxis(end), preferred: .horizontal).dropLast())
            let route = OrthogonalRoute.simplified([fromTip] + bends + [toTip])
            wires.append(Wire(start: from, end: to, interior: Array(route.dropFirst().dropLast())))
            // A line drawn on from a loose end (a deleted pin's spot)
            // continues that wire: the spot is no longer loose, and where
            // only the two wires meet they join into one.
            for end in [from, to] {
                if case let .junction(id) = end, let index = junctions.firstIndex(where: { $0.id == id }) {
                    junctions[index].isLoose = false
                }
            }
            joinThroughJunctions()
            pruneSelection()
            return nil
        }
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
        case let .label(id):
            return .label(id)
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
        case let .label(id):
            return .label(id)
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

        // A wire with no stored bends only shows a midpoint (`settled`);
        // neither half keeps it, so each straight half stays one segment.
        func bends(_ piece: ArraySlice<CGPoint>) -> [CGPoint] {
            let points = Array(piece)
            let kept = hit.wire.interior.isEmpty ? OrthogonalRoute.simplified(points) : points
            return Array(kept.dropFirst().dropLast())
        }
        let original = wires[index]
        wires[index].end = .junction(created.id)
        wires[index].interior = bends(path[0...k])
        wires.insert(
            Wire(
                start: .junction(created.id),
                end: original.end,
                interior: bends(path[k...])
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
        joinThroughJunctions()
        pruneSelection()
    }

    /// Joins the two wires at every plain junction where only two wire ends
    /// meet, as removing a branch leaves them: one wire runs through the
    /// spot instead, with bends only where the route turns. Loose
    /// junctions (plan-30) stay, since a pin can still land on them.
    private func joinThroughJunctions() {
        var changed = true
        while changed {
            changed = false
            // The wires on each junction, found in one pass over the wires
            // (not one pass per junction).
            var wiresOn: [UUID: [Int]] = [:]
            for index in wires.indices {
                if case let .junction(id) = wires[index].start { wiresOn[id, default: []].append(index) }
                if case let .junction(id) = wires[index].end, wires[index].start != wires[index].end {
                    wiresOn[id, default: []].append(index)
                }
            }
            for junction in junctions where !junction.isLoose {
                let here = WireEnd.junction(junction.id)
                guard let on = wiresOn[junction.id], on.count == 2 else { continue }
                // The first wire runs into the junction, the second out of it.
                var first = wires[on[0]], second = wires[on[1]]
                if first.start == here { first = Self.reversed(first) }
                if second.end == here { second = Self.reversed(second) }
                // A wire on the junction at both ends, or two wires between
                // the same two ends, would join into a loop.
                guard first.end == here, second.start == here, first.start != here, second.end != here,
                      first.start != second.end,
                      let from = endTip(first.start), let to = endTip(second.end) else { continue }
                // Bends on a straight run go, the junction's spot among them,
                // so a straight line comes back as one segment.
                let route = OrthogonalRoute.simplified(
                    [from] + first.interior + [junction.point] + second.interior + [to])
                wires[on[0]] = Wire(id: wires[on[0]].id, start: first.start, end: second.end,
                                    interior: Array(route.dropFirst().dropLast()))
                wires.remove(at: on[1])
                junctions.removeAll { $0.id == junction.id }
                changed = true
                break
            }
        }
    }

    /// The same wire, its start and end swapped.
    private static func reversed(_ wire: Wire) -> Wire {
        Wire(id: wire.id, start: wire.end, end: wire.start, interior: wire.interior.reversed())
    }

    /// Turns every wire end that `matches` into a loose junction at that
    /// end's tip, so the wire keeps its route when the pin goes away (plan-30).
    /// Ends with the same tip share one junction. A junction already at that
    /// point is reused and marked loose. Call it while the tips still
    /// resolve, before the pins go away; an end whose tip doesn't resolve is
    /// left alone.
    private func loosen(where matches: (WireEnd) -> Bool) {
        var byPoint: [CGPoint: UUID] = [:]
        func looseJunction(at point: CGPoint) -> UUID {
            if let id = byPoint[point] { return id }
            let id: UUID
            if let index = junctions.firstIndex(where: { $0.point == point }) {
                junctions[index].isLoose = true
                id = junctions[index].id
            } else {
                let created = Junction(point: point, isLoose: true)
                junctions.append(created)
                id = created.id
            }
            byPoint[point] = id
            return id
        }
        for index in wires.indices {
            if matches(wires[index].start), let tip = endTip(wires[index].start) {
                wires[index].start = .junction(looseJunction(at: tip))
            }
            if matches(wires[index].end), let tip = endTip(wires[index].end) {
                wires[index].end = .junction(looseJunction(at: tip))
            }
        }
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
        recordEdit(.schematic, "Move") {
            settle { attaches($0, toPort: portID) }
        }
    }

    /// Re-fits the wires on every port of a bus, when a bus drag ends, and
    /// connects loose ends its ports' pins landed on.
    func settleWires(attachedToBus busID: UUID) {
        recordEdit(.schematic, "Move") {
            for port in bus(busID)?.ports ?? [] {
                settleWires(attachedToPort: port.id)
            }
            adoptLooseEnds(ports: Set(bus(busID)?.ports.map(\.id) ?? []))
        }
    }

    /// Whether either end of the wire is on a loose junction (plan-30).
    func hasLooseEnd(_ wire: Wire) -> Bool {
        [wire.start, wire.end].contains { end in
            guard case let .junction(id) = end else { return false }
            return junctions.contains { $0.id == id && $0.isLoose }
        }
    }

    /// Whether a junction gets a dot: always for a plain junction; for a
    /// loose one only where 3 or more wire ends meet, since a dot reads as a
    /// connection.
    func showsDot(_ junction: Junction) -> Bool {
        guard junction.isLoose else { return true }
        let end = WireEnd.junction(junction.id)
        let count = wires.reduce(0) { $0 + ($1.start == end ? 1 : 0) + ($1.end == end ? 1 : 0) }
        return count >= 3
    }

    /// Connects loose ends to pins after a drop (plan-30). A loose junction
    /// joins a pin when its point is that pin's tip and either the junction
    /// or the pin's part or port is in the dropped set; every wire end on the
    /// junction moves to the pin, and the junction goes away. Each candidate
    /// goes through the same voltage check as `connect`; one that fails stays
    /// loose and sets `editorMessage`. When two pins share a tip, the first
    /// part in `chips` order wins, then bus order. Removals never call this.
    func adoptLooseEnds(chips chipIDs: Set<UUID> = [], ports portIDs: Set<UUID> = [],
                        junctions junctionIDs: Set<UUID> = [], labels labelIDs: Set<UUID> = []) {
        recordEdit(.schematic, "Connect") {
            guard junctions.contains(where: \.isLoose) else { return }
            @MainActor func tips(of placed: [PlacedChip], ports includes: (BusPort) -> Bool,
                                 labels: [NetLabel]) -> [CGPoint: WireEnd] {
                var map: [CGPoint: WireEnd] = [:]
                for chip in placed {
                    for slot in chip.layout.slots {
                        let ref = PinRef(chipID: chip.id, index: slot.pin.index)
                        if let tip = pinTip(ref), map[tip] == nil { map[tip] = .pin(ref) }
                    }
                }
                for bus in buses {
                    for port in bus.ports where includes(port) {
                        for wire in bus.pinWires(of: port) {
                            let ref = BusPinRef(portID: port.id, wireID: wire.id)
                            if let tip = busPinTip(ref), map[tip] == nil { map[tip] = .busPin(ref) }
                        }
                    }
                }
                for label in labels where map[label.tip] == nil {
                    map[label.tip] = .label(label.id)
                }
                return map
            }
            let dropped = tips(of: chips.filter { chipIDs.contains($0.id) }, ports: { portIDs.contains($0.id) },
                               labels: netLabels.filter { labelIDs.contains($0.id) })
            // A dragged loose end may land on any pin.
            let every = junctionIDs.isEmpty ? [:] : tips(of: chips, ports: { _ in true }, labels: netLabels)
            var adopted = false
            for junction in junctions where junction.isLoose {
                let lookup = junctionIDs.contains(junction.id) ? every : dropped
                guard let pin = lookup[junction.point] else { continue }
                let end = WireEnd.junction(junction.id)
                // A wire whose other end is already this pin would end on itself.
                guard !wires.contains(where: { ($0.start == end && $0.end == pin) || ($0.end == end && $0.start == pin) })
                else { continue }
                let candidate = wires.map { wire in
                    var wire = wire
                    if wire.start == end { wire.start = pin }
                    if wire.end == end { wire.end = pin }
                    return wire
                }
                if let library,
                   let issue = VoltageCheck.firstNew(before: chips, wires, after: chips, candidate,
                                                     buses: buses, labels: netLabels,
                                                 known: currentVoltageIssues(library), library: library) {
                    editorMessage = VoltageCheck.message(issue, chips: chips, edit: .connect)
                    continue
                }
                wires = candidate
                junctions.removeAll { $0.id == junction.id }
                adopted = true
            }
            if adopted { pruneSelection() }
        }
    }

    /// Called when a block is released. Re-fits the wires attached to it and
    /// stores the result, so the change stays rather than being recomputed per frame.
    func settleWires(attachedTo chipID: UUID) {
        recordEdit(.schematic, "Move") {
            settle { attaches($0, to: chipID) }
        }
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
        recordEdit(.schematic, "Move Wire") {
            guard let index = wires.firstIndex(where: { $0.id == id }), path.count >= 2 else { return }
            wires[index].interior = Array(path.dropFirst().dropLast())
        }
    }

    /// A bend point on a wire, if `point` is on one.
    func nodeHit(at point: CGPoint, tolerance: CGFloat = 6) -> (wire: UUID, index: Int)? {
        for wire in wires.reversed() {
            let path = path(for: wire)
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
            let path = path(for: wire)
            for index in 0..<max(path.count - 1, 0)
                where Self.distance(from: point, toSegment: path[index], path[index + 1]) <= tolerance {
                return (wire.id, index)
            }
        }
        return nil
    }

    /// Double-click on a wire: adds a bend at that point.
    func insertNode(at point: CGPoint, tolerance: CGFloat = 6) {
        recordEdit(.schematic, "Add Bend") {
            guard let hit = segmentHit(at: point, tolerance: tolerance) else { return }
            let path = path(forWire: hit.wire)
            setWirePath(hit.wire, OrthogonalRoute.insertNode(path, segment: hit.index, at: point))
        }
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
        return recordEdit(.schematic, "Add Bus") { () -> UUID? in
            applyBusDraft(busDraft(from: start, to: end))
        }
    }

    /// The Bus tool along a drawn route: `addBus(from:to:)` for the first of
    /// `candidates` the bus rules accept.
    @discardableResult
    func addBus(candidates: [[CGPoint]]) -> UUID? {
        return recordEdit(.schematic, "Add Bus") { () -> UUID? in
            applyBusDraft(busDraft(candidates: candidates))
        }
    }

    private func applyBusDraft(_ result: Result<BusDraft, BusJoinRefusal>) -> UUID? {
        switch result {
        case let .failure(refusal):
            if let message = refusal.message { editorMessage = message }
            return nil
        case let .success(draft):
            if let index = draft.busIndex {
                let old = buses[index]
                if buses.contains(where: { old.isLinked(with: $0) }),
                   draft.bus.wires.map(\.name) != old.wires.map(\.name) || draft.bus.card != old.card {
                    editorMessage = "\(old.name) is linked with another bus of its name, so joining can't change its wires."
                    return nil
                }
                buses[index] = draft.bus
            } else {
                var added = draft.bus
                added.name = Bus.defaultBusName(taken: buses.map(\.name))
                buses.append(added)
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
        return recordEdit(.schematic, "Change Bus Width") { () -> String? in
            guard let index = buses.firstIndex(where: { $0.id == id }) else { return nil }
            let bus = buses[index]
            let group = linkGroup(index)
            if groupHasEdge(group) { return edgeBusLock(bus) }
            if !bus.card.isCustom { return Self.typeSetsWires }
            if let highest = highestUsedWire(group), count < highest + 1 {
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
            syncLinked(index)
            return nil
        }
    }

    /// Renames one wire of a bus. Another wire of the bus may have the same
    /// name (the wire list warns); each wire is still its own net. Returns
    /// why when the name is refused, and changes nothing then.
    func renameBusWire(bus id: UUID, wire wireID: UUID, to name: String) -> String? {
        return recordEdit(.schematic, "Rename Bus Wire") { () -> String? in
            guard let index = buses.firstIndex(where: { $0.id == id }),
                  let w = buses[index].wires.firstIndex(where: { $0.id == wireID }) else { return nil }
            if !buses[index].card.isCustom { return Self.typeSetsWires }
            let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
            if trimmed.isEmpty { return "A wire name can't be empty." }
            if trimmed.count > Bus.nameLengthRange.upperBound { return "A wire name is at most \(Bus.nameLengthRange.upperBound) characters." }
            if trimmed.contains(where: \.isWhitespace) { return "A wire name can't contain spaces." }
            buses[index].wires[w].name = trimmed
            syncLinked(index)
            return nil
        }
    }

    /// Sets a bus's wires to `names`, in order (the Bus panel's Import).
    /// Wires keep their places and ids (so ports and the wires on their pins
    /// stay) and are renamed; wires are added or dropped at the end. One
    /// undo step. Refused, with why, and nothing changes, when the bus type
    /// sets the wires, a name is invalid, the count is out of range, a port
    /// uses a wire that would go, or the card edge uses the bus and the
    /// count would change.
    func setBusWireNames(_ id: UUID, _ names: [String]) -> String? {
        return recordEdit(.schematic, "Import Bus Wires") { () -> String? in
            guard let index = buses.firstIndex(where: { $0.id == id }) else { return nil }
            let bus = buses[index]
            if !bus.card.isCustom { return Self.typeSetsWires }
            let names = names.map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
            guard Bus.wireCountRange.contains(names.count) else {
                return "A bus carries \(Bus.wireCountRange.lowerBound)–\(Bus.wireCountRange.upperBound) wires."
            }
            if names.contains(where: { $0.isEmpty }) { return "A wire name can't be empty." }
            if names.contains(where: { $0.count > Bus.nameLengthRange.upperBound }) { return "A wire name is at most \(Bus.nameLengthRange.upperBound) characters." }
            if names.contains(where: { $0.contains(where: \.isWhitespace) }) { return "A wire name can't contain spaces." }
            let group = linkGroup(index)
            if names.count != bus.wires.count {
                if groupHasEdge(group) { return edgeBusLock(bus) }
                if let highest = highestUsedWire(group), names.count < highest + 1 {
                    return "\(bus.wires[highest].name) is used by a port; this bus needs at least \(highest + 1) wires."
                }
            }
            var wires = Array(bus.wires.prefix(names.count))
            for (i, name) in names.enumerated() {
                if i < wires.count { wires[i].name = name } else { wires.append(BusWire(name: name)) }
            }
            buses[index].wires = wires
            syncLinked(index)
            return nil
        }
    }

    /// Moves one wire of a bus to place `to` (0-based, in the order after
    /// the move), keeping its id and name, so its net and the wires on its
    /// port pins go with it. Linked buses move the wire at the same place
    /// the same way, so wire i of each stays one net. Ports show their pins
    /// in bus order, so their wires are re-fitted. Refused, with why, while
    /// the bus type sets the wires.
    func moveBusWire(bus id: UUID, wire wireID: UUID, to place: Int) -> String? {
        return recordEdit(.schematic, "Reorder Bus Wires") { () -> String? in
            guard let index = buses.firstIndex(where: { $0.id == id }),
                  let from = buses[index].wires.firstIndex(where: { $0.id == wireID }) else { return nil }
            if !buses[index].card.isCustom { return Self.typeSetsWires }
            let to = min(max(place, 0), buses[index].wires.count - 1)
            guard to != from else { return nil }
            for member in linkGroup(index) where from < buses[member].wires.count && to < buses[member].wires.count {
                let wire = buses[member].wires.remove(at: from)
                buses[member].wires.insert(wire, at: to)
                for port in buses[member].ports {
                    settleWires(attachedToPort: port.id)
                }
            }
            return nil
        }
    }

    private static let typeSetsWires = "The bus type sets its wires; set Type to Custom to change them."

    /// The bus the board's card edge uses, while the board is a card and
    /// the bus has as many wires as the card has pins.
    var edgeBusID: UUID? {
        guard board.type.isCard, let id = board.edgeBus, let bus = bus(id),
              bus.wires.count == board.pinCount else { return nil }
        return id
    }

    /// The buses a card edge can use: those with as many wires as the card
    /// has pins. Only the count matters, not the names.
    var edgeBusChoices: [Bus] {
        guard board.type.isCard else { return [] }
        return buses.filter { $0.wires.count == board.pinCount }
    }

    /// Chooses the bus the card edge uses, or none. A bus with the wrong
    /// wire count is ignored.
    func setEdgeBus(_ id: UUID?) {
        recordEdit(.pcb, "Change Card Bus") {
            if let id, !edgeBusChoices.contains(where: { $0.id == id }) { return }
            if board.edgeBus != id { board.edgeBus = id }
        }
    }

    private func edgeBusUndeletable(_ bus: Bus) -> String {
        "\(bus.name) is in use as the card edge, so it can't be deleted. Set the PCB's Bus to None first."
    }

    private func edgeBusLock(_ bus: Bus) -> String {
        "\(bus.name) is in use as the card edge; its wire count is locked."
    }

    /// The bus that renaming bus `id` to `name` would link it with: another
    /// bus of that name (ignoring case) not linked with it already. Nil when
    /// the rename links nothing new.
    func busLinkTarget(_ id: UUID, named name: String) -> Bus? {
        guard let bus = bus(id) else { return nil }
        let lowered = name.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
        return buses.first { $0.id != id && !bus.isLinked(with: $0) && $0.name.lowercased() == lowered }
    }

    /// Renames a bus. Refused, with why, when the name is empty. A bus
    /// linked with others (same name) leaves them; a new case for the same
    /// name renames them all.
    ///
    /// Another bus's name links the two (`busLinkTarget`), and only with
    /// `link`: this bus takes that one's type and wires (its wires keep
    /// their places and ids; they are renamed, added or dropped at the end)
    /// and its exact name. Without `link` that is refused. Refused too when
    /// a port uses a wire the other bus doesn't have, or this bus is the
    /// card edge and the wire count would change.
    func setBusName(_ id: UUID, _ name: String, link: Bool = false) -> String? {
        return recordEdit(.schematic, "Rename Bus") { () -> String? in
            guard let index = buses.firstIndex(where: { $0.id == id }) else { return nil }
            let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
            if trimmed.isEmpty { return "A bus name can't be empty." }
            let bus = buses[index]
            if let target = busLinkTarget(id, named: trimmed) {
                guard link else { return "\(trimmed) is already used by another bus." }
                let count = target.wires.count
                if bus.id == edgeBusID, count != bus.wires.count {
                    return "\(bus.name) is in use as the card edge; it can only link with a bus of \(bus.wires.count) wires."
                }
                let used = bus.ports.reduce(into: Set<UUID>()) { $0.formUnion($1.wireIDs) }
                if let highest = bus.wires.lastIndex(where: { used.contains($0.id) }), highest >= count {
                    return "\(bus.wires[highest].name) is used by a port; \(target.name) has only \(count) wires."
                }
                buses[index] = Self.matching(bus, target)
                buses[index].name = target.name
                return nil
            }
            if trimmed.lowercased() == bus.name.lowercased() {
                for other in linkGroup(index) where buses[other].name != trimmed { buses[other].name = trimmed }
            } else if bus.name != trimmed {
                buses[index].name = trimmed
            }
            return nil
        }
    }

    /// `index` and the buses linked with it (same name, ignoring case), in
    /// order.
    private func linkGroup(_ index: Int) -> [Int] {
        buses.indices.filter { $0 == index || buses[index].isLinked(with: buses[$0]) }
    }

    /// Is one of these buses the card edge's?
    private func groupHasEdge(_ group: [Int]) -> Bool {
        guard let edge = edgeBusID else { return false }
        return group.contains { buses[$0].id == edge }
    }

    /// The highest wire place a port on any of these buses uses.
    private func highestUsedWire(_ group: [Int]) -> Int? {
        group.compactMap { i -> Int? in
            let used = buses[i].ports.reduce(into: Set<UUID>()) { $0.formUnion($1.wireIDs) }
            return buses[i].wires.lastIndex { used.contains($0.id) }
        }.max()
    }

    /// `bus` with `lead`'s type and wire names: its wires keep their places
    /// and ids, and are renamed, added or dropped at the end.
    private static func matching(_ bus: Bus, _ lead: Bus) -> Bus {
        var result = bus
        var wires = Array(bus.wires.prefix(lead.wires.count))
        for (i, wire) in lead.wires.enumerated() {
            if i < wires.count { wires[i].name = wire.name } else { wires.append(BusWire(name: wire.name)) }
        }
        result.wires = wires
        result.card = lead.card
        return result
    }

    /// Gives every bus linked with `index` its type and wires.
    private func syncLinked(_ index: Int) {
        for other in linkGroup(index) where other != index {
            let matched = Self.matching(buses[other], buses[index])
            if matched != buses[other] { buses[other] = matched }
        }
    }

    /// Sets what a bus carries. A card type makes the wires its pins, in
    /// order: existing wires keep their places (and ids, so ports and their
    /// wire ends stay) and are renamed, and wires are added or dropped at
    /// the end. Custom keeps the wires as they are, to edit freely.
    /// Refused, with why, when a port uses a wire the card has no pin for,
    /// or the card has fewer than 2 pins.
    func setBusCard(_ id: UUID, _ card: BusCard) -> String? {
        return recordEdit(.schematic, "Change Bus Type") { () -> String? in
            guard let index = buses.firstIndex(where: { $0.id == id }), buses[index].card != card else { return nil }
            guard let names = card.pinNames else {
                buses[index].card = card
                syncLinked(index)
                return nil
            }
            let bus = buses[index]
            guard Bus.wireCountRange.contains(names.count) else {
                return "A bus needs \(Bus.wireCountRange.lowerBound) to \(Bus.wireCountRange.upperBound) wires; this card has \(names.count) pins."
            }
            let group = linkGroup(index)
            if groupHasEdge(group), names.count != bus.wires.count { return edgeBusLock(bus) }
            if let highest = highestUsedWire(group), highest >= names.count {
                return "\(bus.wires[highest].name) is used by a port; this card has only \(names.count) pins."
            }
            var wires = Array(bus.wires.prefix(names.count))
            for (i, name) in names.enumerated() {
                if i < wires.count { wires[i].name = name } else { wires.append(BusWire(name: name)) }
            }
            buses[index].wires = wires
            buses[index].card = card
            syncLinked(index)
            return nil
        }
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
        return recordEdit(.schematic, "Add Port") { () -> String? in
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
    }

    /// Sets which bus wires are pins on a port (ticks, Select All, Deselect
    /// All). More pins than fit is refused with a message and changes
    /// nothing; a port that grows may move left (up) to fit. The wires on
    /// unticked pins stay, with loose ends.
    func setPortWires(_ portID: UUID, _ ids: Set<UUID>) -> String? {
        return recordEdit(.schematic, "Change Port Pins") { () -> String? in
            guard let at = portLocation(portID) else { return nil }
            let bus = buses[at.bus]
            let port = bus.ports[at.port]
            let wanted = ids.intersection(bus.wires.map(\.id))
            guard let start = bus.fit(port: port, slots: wanted.count, preferredStart: port.start) else {
                return "Not enough room on this bus line for \(wanted.count) pins."
            }
            let removing = port.wireIDs.subtracting(wanted)
            if !removing.isEmpty {
                loosen { endsOn($0, port: portID, wireIDs: removing) }
                pruneJunctions()
            }
            buses[at.bus].ports[at.port].start = start
            buses[at.bus].ports[at.port].wireIDs = wanted
            // Pins after an added one shift by a pitch; re-fit their wires.
            settleWires(attachedToPort: portID)
            return nil
        }
    }

    /// Sets the side a port's pins come out of, if it suits the line. Ports
    /// on opposite sides may share a place, so a port on the new side that is
    /// in the way refuses the switch with a message.
    @discardableResult
    func setPortSide(_ portID: UUID, _ side: ChipLayout.Side) -> String? {
        return recordEdit(.schematic, "Change Port Side") { () -> String? in
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
    }

    /// Moves a port along its line to `start`. Stores it only if the fit
    /// rule keeps it unchanged (on its line, clear of the other ports and the
    /// Ts). Returns whether it did.
    @discardableResult
    func slidePort(_ portID: UUID, to start: CGFloat) -> Bool {
        return recordEdit(.schematic, "Move Port") { () -> Bool in
            guard let at = portLocation(portID) else { return false }
            let bus = buses[at.bus]
            let port = bus.ports[at.port]
            guard start != port.start else { return true }
            guard bus.fit(port: port, slots: bus.slots(of: port), preferredStart: start) == start else { return false }
            buses[at.bus].ports[at.port].start = start
            return true
        }
    }

    /// One frame of a port slide: `base` (the start at the press) plus
    /// `along` (the drag along the line), snapped to the grid and clamped to
    /// where the port may go. It never leaves its line or passes a line
    /// end, another port or a T.
    func slidePort(_ portID: UUID, from base: CGFloat, by along: CGFloat) {
        recordEdit(.schematic, "Move Port") {
            guard let (bus, port) = port(portID), let range = bus.slideRange(of: port) else { return }
            let wanted = SchematicGrid.snap(base + along)
            slidePort(portID, to: min(max(wanted, range.lowerBound), range.upperBound))
        }
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
        return recordEdit(.schematic, "Move Bus") { () -> Bool in
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
    }

    /// One frame of a bus drag. `step(offset)` gives the lines for a drag of
    /// `offset` from the press (against the press-time bus). The drag walks
    /// from `done` (the last offset stored) toward `target` one grid step at
    /// a time, storing each step through `setBusLines`, and stops at the
    /// first refused one. So a drag never jumps over something it may not
    /// pass, such as a T end over a port. Returns the offset reached.
    @discardableResult
    func dragBus(_ id: UUID, from done: CGSize, to target: CGSize, step: (CGSize) -> [BusLine]?) -> CGSize {
        return recordEdit(.schematic, "Move Bus") { () -> CGSize in
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
        recordEdit(.schematic, "Delete") {
            guard let pending = pendingRemoval else { return }
            pendingRemoval = nil
            switch pending.action {
            case let .port(id): removePort(id)
            case let .busLine(bus, line): removeBusLine(bus: bus, line: line)
            case let .untick(port, keeping): _ = setPortWires(port, keeping)
            }
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
        return recordEdit(.schematic, "Change Port Pins") { () -> String? in
            guard let (_, port) = port(portID) else { return nil }
            let removing = port.wireIDs.subtracting(ids)
            let count = wiresOnPins(port: portID, removing: removing)
            if count > 0 {
                pendingRemoval = PendingRemoval(
                    message: "Remove \(Self.counted(removing.count, "pin"))? The \(Self.counted(count, "wire")) on them stay, with loose ends.",
                    action: .untick(port: portID, keeping: ids))
                return nil
            }
            return setPortWires(portID, ids)
        }
    }

    /// Removes a port. The wires on its pins stay, with loose ends.
    private func removePort(_ portID: UUID) {
        guard let at = portLocation(portID) else { return }
        loosen { endsOn($0, port: portID) }
        buses[at.bus].ports.remove(at: at.port)
        pruneJunctions()
        if selectedPortID == portID { selectedPortID = nil }
    }

    /// Removes one bus line, with its ports; the wires on their pins stay, with
    /// loose ends. A bus
    /// with no lines left is removed. If the rest falls into pieces, the piece
    /// holding the first remaining line keeps the bus; each other piece
    /// becomes a bus of its own with the same wire names and fresh wire ids,
    /// and its ports (and the wire ends on them) map to those ids by list
    /// position.
    private func removeBusLine(bus id: UUID, line lineID: UUID) {
        guard let index = buses.firstIndex(where: { $0.id == id }) else { return }
        var bus = buses[index]
        if bus.id == edgeBusID, bus.lines.count == 1 {
            editorMessage = edgeBusUndeletable(bus)
            return
        }
        // Loosen first, while the removed ports' pin tips still resolve.
        for port in bus.ports where port.line == lineID {
            loosen { endsOn($0, port: port.id) }
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
            split.append(Bus(name: Bus.defaultBusName(taken: (buses + split).map(\.name)), card: bus.card,
                             lines: piece, wires: newWires, ports: moved))
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

    /// Removes whatever is under `point`: a block first (its wires stay, with
    /// loose ends where its pins were), then a net label (the same), then
    /// a bus port, then a wire, then a bus line, then a text block. A port with
    /// wires on its pins, or a bus line with ports, asks first (`pendingRemoval`); while
    /// that is up, nothing is erased.
    func erase(at point: CGPoint, tolerance: CGFloat = 6) {
        recordEdit(.schematic, "Delete") {
            guard pendingRemoval == nil else { return }
            if let hit = chip(at: point) {
                loosen { attaches($0, to: hit.id) }
                chips.removeAll { $0.id == hit.id }
            } else if let hit = netLabel(at: point) {
                deleteNetLabel(hit.id)
            } else if let portID = portHit(at: point) {
                let count = wiresOnPins(port: portID)
                if count > 0 {
                    pendingRemoval = PendingRemoval(
                        message: "Delete this port? The \(Self.counted(count, "wire")) on its pins stay, with loose ends.",
                        action: .port(portID))
                } else {
                    removePort(portID)
                }
                return
            } else if let hit = segmentHit(at: point, tolerance: tolerance) {
                wires.removeAll { $0.id == hit.wire }
            } else if let hit = busHit(at: point, tolerance: tolerance), let bus = bus(hit.bus) {
                // The card edge's bus can't go: not its last line.
                if bus.id == edgeBusID, bus.lines.count == 1 {
                    editorMessage = edgeBusUndeletable(bus)
                    return
                }
                let ports = bus.ports.filter { $0.line == hit.line }
                if ports.isEmpty {
                    removeBusLine(bus: hit.bus, line: hit.line)
                } else {
                    let count = ports.map { wiresOnPins(port: $0.id) }.reduce(0, +)
                    pendingRemoval = PendingRemoval(
                        message: "Delete this bus line and its \(Self.counted(ports.count, "port"))? The \(Self.counted(count, "wire")) on their pins stay, with loose ends.",
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
    }

    /// ⌘⌫ on the Schematic: deletes the selected parts (their wires stay,
    /// with loose ends, as the Eraser leaves them), wires and text blocks, as
    /// one step; junctions go once no wire ends on them. A selected bus port
    /// is deleted as the Eraser deletes one, asking first when wires are on
    /// its pins. Selected buses go whole, with their ports; the wires on
    /// their pins stay, with loose ends. The card edge's bus stays
    /// (`editorMessage` says why). Returns whether anything was deleted or
    /// asked.
    /// ⌘X passes `name` "Cut" (with no port selected: a port isn't copied).
    @discardableResult
    func deleteSchematicSelection(named name: String = "Delete") -> Bool {
        guard pendingRemoval == nil else { return false }
        if let portID = selectedPortID {
            recordEdit(.schematic, "Delete") {
                let count = wiresOnPins(port: portID)
                if count > 0 {
                    pendingRemoval = PendingRemoval(
                        message: "Delete this port? The \(Self.counted(count, "wire")) on its pins stay, with loose ends.",
                        action: .port(portID))
                } else {
                    removePort(portID)
                }
            }
            return true
        }
        var chipIDs: Set<UUID> = [], wireIDs: Set<UUID> = [], noteIDs: Set<UUID> = []
        var labelIDs: Set<UUID> = [], busIDs: Set<UUID> = []
        for item in schematicSelection.items {
            switch item {
            case let .chip(id): chipIDs.insert(id)
            case let .wire(id): wireIDs.insert(id)
            case let .note(id): noteIDs.insert(id)
            case let .label(id): labelIDs.insert(id)
            case let .bus(id): busIDs.insert(id)
            case .junction: break
            }
        }
        var message: String?
        if let edge = edgeBusID, busIDs.contains(edge), let bus = bus(edge) {
            busIDs.remove(edge)
            message = edgeBusUndeletable(bus)
        }
        guard !chipIDs.isEmpty || !wireIDs.isEmpty || !noteIDs.isEmpty || !labelIDs.isEmpty || !busIDs.isEmpty else {
            if let message { editorMessage = message }
            return false
        }
        let portIDs = Set(buses.filter { busIDs.contains($0.id) }.flatMap { $0.ports.map(\.id) })
        recordEdit(.schematic, name) {
            wires.removeAll { wireIDs.contains($0.id) }
            loosen { end in
                switch end {
                case let .label(id): labelIDs.contains(id)
                case let .busPin(ref): portIDs.contains(ref.portID)
                default: chipIDs.contains { attaches(end, to: $0) }
                }
            }
            buses.removeAll { busIDs.contains($0.id) }
            if message != nil { editorMessage = message }
            chips.removeAll { chipIDs.contains($0.id) }
            netLabels.removeAll { labelIDs.contains($0.id) }
            notes.removeAll { noteIDs.contains($0.id) }
            if let editing = editingNoteID, noteIDs.contains(editing) { editingNoteID = nil }
            pruneJunctions()
        }
        return true
    }

    // MARK: Geometry

    nonisolated static func distance(from a: CGPoint, to b: CGPoint) -> CGFloat {
        hypot(a.x - b.x, a.y - b.y)
    }

    nonisolated static func distance(from point: CGPoint, toSegment a: CGPoint, _ b: CGPoint) -> CGFloat {
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
