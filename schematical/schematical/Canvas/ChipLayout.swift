import CoreGraphics
import Foundation

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
    /// A crystal or ceramic resonator (plan-44): two plates with a rectangle
    /// between them, `X1` on the left and `X2` on the right, a three-pin
    /// resonator's `GND` pin at the bottom centre (`CrystalSymbolView`).
    case crystal
    /// A two-terminal resistor (plan-34): a horizontal zig-zag between pin 1
    /// (left) and pin 2 (right), its value above (`ResistorSymbolView`).
    /// Resistor arrays are blocks.
    case resistor
    /// A fixed inductor (plan-47-M): four humps between pin 1 (left) and
    /// pin 2 (right), its inductance above (`InductorSymbolView`).
    case inductor
    /// A fuse or PTC resettable fuse (plan-47-M): a rectangle with the lead
    /// through it between pin 1 (left) and pin 2 (right), its rated current
    /// above; a PTC adds the thermistor stroke (`FuseSymbolView`). A blown
    /// fuse (play mode) draws with a gap in the lead.
    case fuse
    /// A relay (plan-47-N): the coil (a solid-state relay's LED) on the left,
    /// each pole's `NO`, `COM` and `NC` terminals on the right with an arm
    /// that swings to `NO` while the relay is operated (`RelaySymbolView`).
    case relay
    /// A buzzer (plan-47-N): a half disc between `+` (left) and `-` (right);
    /// sound waves while it is on (`BuzzerSymbolView`).
    case buzzer
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
    /// A slide switch (plan-58): the common terminal on the left, the throws
    /// on the right, a blade for each pole and a knob on a slot under them
    /// (`SlideSwitchSymbolView`).
    case slideSwitch
    case sevenSegment
    /// A nixie tube: cathodes on the left, the anode on the right, the tube
    /// in the middle (`NixieSymbolView`).
    case nixie
    /// A dekatron: cathodes on the left, the anode and guides on the right,
    /// the ring of ten cathode positions in the middle (`DekatronSymbolView`).
    case dekatron
    case characterLCD
    case barrelJack
    /// A generic pin header (plan-23): pins on the left, and with two rows
    /// the even pins on the right.
    case header
    /// A test point (plan-57): one pin on the left of a 40 x 20 body, drawn as
    /// a small circle on the end of its lead (`TestPointSymbolView`).
    case testPoint
    /// One gate of a part drawn as its units (plan-67): the standard logic
    /// symbol, `GateSymbolView`. Only a piece (`PlacedChip.piece`) has it; no
    /// whole part does.
    case gate

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

    /// Rotate turns every part drawn as its own symbol in quarter turns
    /// (`PlacedChip.rotation`): LEDs, diodes, resistors, capacitors,
    /// crystals, inductors, fuses, buzzers, transistors, test points and
    /// barrel jacks, and a gate of a part drawn as its units. Parts drawn as a
    /// box (logic chips, switches, displays, relays, headers) and the supplies
    /// don't rotate.
    var canRotate: Bool {
        switch self {
        case .diode, .plainDiode, .capacitor, .resistor, .crystal, .inductor, .fuse, .buzzer, .transistor,
             .testPoint, .barrelJack, .gate:
            true
        default:
            false
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
        case .block, .oscillator, .gate:
            // A whole part is never a gate; a gate piece's layout is `init(gate:pins:)`.
            self = ChipLayout(pins: pins)
        case .barrelJack:
            self = Self.barrelJack(pins)
        case .header:
            self = Self.header(pins, rows: part.headerSize?.rows ?? 1)
        case .testPoint:
            self = Self.testPoint(pins)
        case .diode, .plainDiode:
            self = Self.diode(pins)
        case .capacitor, .resistor:
            self = Self.capacitor(pins)
        case .crystal:
            self = Self.crystal(pins)
        case .inductor, .fuse:
            self = Self.twoTerminals(pins)
        case .relay:
            self = Self.relay(part)
        case .buzzer:
            self = Self.buzzer(pins)
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
        case .slideSwitch:
            self = Self.slideSwitch(pins, circuit: part.slideStates?.circuit ?? "SPDT")
        case .sevenSegment:
            self = Self.sevenSegment(pins, display: part.display)
        case .nixie:
            self = Self.nixie(pins, nixie: part.nixie)
        case .dekatron:
            self = Self.dekatron(pins, dekatron: part.dekatron)
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

    /// Width of one digit's cell: a row of dots (the IV-25's P1-P7) is wider
    /// than a numeral.
    static func sevenSegmentCellWidth(_ display: DisplayInfo) -> CGFloat {
        display.leds.contains { $0.segment.hasPrefix("P") } ? 42 : sevenSegmentDigitSize.width
    }

    /// The gap between digits: narrower for a display of more than eight (the IV-27's 14).
    static func sevenSegmentGap(_ display: DisplayInfo) -> CGFloat {
        display.digits > 8 ? 4 : sevenSegmentDigitGap
    }

    /// Width of the face that shows the digits, colon included.
    static func sevenSegmentFaceWidth(_ display: DisplayInfo) -> CGFloat {
        let digits = CGFloat(max(display.digits, 1))
        let colon = display.colonAfter > 0 ? sevenSegmentColonWidth : 0
        return digits * sevenSegmentCellWidth(display) + (digits + 1) * sevenSegmentGap(display) + colon
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

    /// Dekatron: the ring of cathode positions in the middle of the body.
    static let dekatronFaceSize = CGSize(width: 64, height: 64)

    /// Cathode pins on the left; the anode and the guides on the right, then
    /// (after a gap) any other pin.
    private static func dekatron(_ pins: [ChipPin], dekatron: DekatronInfo?) -> ChipLayout {
        guard let dekatron else { return ChipLayout(pins: pins) }
        let byIndex = Dictionary(pins.map { ($0.index, $0) }, uniquingKeysWith: { first, _ in first })
        let left = dekatron.cathodePins.compactMap { byIndex[$0] }
        let used = Set(dekatron.cathodePins + dekatron.guidePins + [dekatron.anodePin])
        let unused = pins.filter { !used.contains($0.index) }.sorted { $0.index < $1.index }
        let right = stack([[dekatron.anodePin].compactMap { byIndex[$0] }, dekatron.guidePins.compactMap { byIndex[$0] }, unused])
        let faceRows = Int(((dekatronFaceSize.height + 16) / pitch).rounded(.up))
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
        let width = SchematicGrid.snap(dekatronFaceSize.width + 2 * sevenSegmentNameColumn + 5)
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

    /// Slide switch body width, and the height of the strip under the poles
    /// that holds the knob's slot (`SlideSwitchSymbolView`).
    static let slideBodyWidth: CGFloat = 120
    static let slideKnobStrip: CGFloat = 20

    /// A slide switch (plan-58): per pole, the common terminal (`C`, `C1`, …) on
    /// the left, midway between its two throws (`A` above `B`) on the right;
    /// SPST and DPST poles have `A` on the left and `B` on the right, one row
    /// each. Pins are placed by name, so the pinout's order does not matter.
    private static func slideSwitch(_ pins: [ChipPin], circuit: String) -> ChipLayout {
        let poles = circuit.hasPrefix("D") ? 2 : 1
        let hasCommon = circuit.hasSuffix("DT")
        func pin(_ base: String, _ pole: Int) -> ChipPin? {
            let names = poles == 1 ? [base] : ["\(base)\(pole + 1)"]
            return pins.first { names.contains($0.name) }
        }
        var slots: [PinSlot] = []
        let rows = hasCommon ? 2 * poles : poles
        for pole in 0..<poles {
            if hasCommon {
                let top = centerY(forRow: 2 * pole), bottom = centerY(forRow: 2 * pole + 1)
                if let common = pin("C", pole) { slots.append(PinSlot(pin: common, side: .left, centerY: (top + bottom) / 2)) }
                if let a = pin("A", pole) { slots.append(PinSlot(pin: a, side: .right, centerY: top)) }
                if let b = pin("B", pole) { slots.append(PinSlot(pin: b, side: .right, centerY: bottom)) }
            } else {
                if let a = pin("A", pole) { slots.append(PinSlot(pin: a, side: .left, centerY: centerY(forRow: pole))) }
                if let b = pin("B", pole) { slots.append(PinSlot(pin: b, side: .right, centerY: centerY(forRow: pole))) }
            }
        }
        // Any other pin the part has keeps a place on the right below the poles.
        let placed = Set(slots.map(\.pin.index))
        for (offset, pin) in pins.filter({ !placed.contains($0.index) }).enumerated() {
            slots.append(PinSlot(pin: pin, side: .right, centerY: centerY(forRow: rows + offset)))
        }
        let total = rows + slots.filter { $0.centerY > centerY(forRow: rows - 1) }.count
        return ChipLayout(
            size: CGSize(width: slideBodyWidth, height: headerHeight + pitch * CGFloat(total) + slideKnobStrip),
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

    /// A test point: its one pin on the left, centred in a header-wide, one-row-tall body.
    static func testPoint(_ pins: [ChipPin]) -> ChipLayout {
        let slots = pins.sorted { $0.index < $1.index }.prefix(1).map {
            PinSlot(pin: $0, side: .left, centerY: pitch / 2)
        }
        return ChipLayout(size: CGSize(width: headerBodyWidth, height: pitch), slots: slots)
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

    /// A supply symbol: its size, and its one pin (the part's first) on `side`.
    private static func supply(_ pins: [ChipPin], side: Side, size: CGSize = supplySize) -> ChipLayout {
        ChipLayout(
            size: size,
            slots: pins.prefix(1).map { PinSlot(pin: $0, side: side, centerY: 0, centerX: supplyPinX) }
        )
    }

    init(size: CGSize, slots: [PinSlot]) {
        self.size = size
        self.slots = slots
    }

    /// The same body mirrored left to right: every left or right pin moves to
    /// the opposite side, its row unchanged, and a top or bottom pin (a
    /// resonator's `GND`) stays on its edge, mirrored along it.
    func flipped() -> ChipLayout {
        ChipLayout(
            size: size,
            slots: slots.map { slot in
                let sideways = slot.side == .left || slot.side == .right
                return PinSlot(
                    pin: slot.pin,
                    side: sideways ? slot.side.opposite : slot.side,
                    centerY: slot.centerY,
                    centerX: sideways ? slot.centerX : size.width - slot.centerX
                )
            }
        )
    }

    /// The box a body turns in: its size, each side rounded up to the grid
    /// (two `pitch` halves), so a pin on the grid stays on it when the part
    /// turns. Only the switched barrel jack (95 tall) is not its own box.
    static func turnBox(_ size: CGSize) -> CGSize {
        let grid = pitch / 2
        return CGSize(width: (size.width / grid).rounded(.up) * grid, height: (size.height / grid).rounded(.up) * grid)
    }

    /// The same body turned clockwise by `degrees` (0, 90, 180 or 270): the
    /// size (its `turnBox`) swaps for a quarter turn and every pin moves to
    /// the side and position its point on the body edge turns to.
    func rotated(by degrees: Int) -> ChipLayout {
        let turns = ((degrees / 90) % 4 + 4) % 4
        guard turns != 0 else { return self }
        let box = Self.turnBox(size)
        let width = box.width, height = box.height
        let newSize = turns == 2 ? box : CGSize(width: height, height: width)
        return ChipLayout(
            size: newSize,
            slots: slots.map { slot in
                // The pin's point on the body edge, then where it turns to.
                let edge: CGPoint
                switch slot.side {
                case .left: edge = CGPoint(x: 0, y: slot.centerY)
                case .right: edge = CGPoint(x: size.width, y: slot.centerY)
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

    /// A crystal or resonator (plan-44-G): `X1` on the left, `X2` on the
    /// right, the diode's box; a pin named `GND` (a three-pin resonator's
    /// common pin) at the bottom centre. Every other pin (the `NC` pads of a
    /// four-pad part) is not drawn. Falls back to the block layout if a
    /// terminal is missing.
    private static func crystal(_ pins: [ChipPin]) -> ChipLayout {
        guard let first = pins.first(where: { $0.name == "X1" }),
              let second = pins.first(where: { $0.name == "X2" }) else {
            return ChipLayout(pins: pins)
        }
        var slots = [
            PinSlot(pin: first, side: .left, centerY: diodeCentreY),
            PinSlot(pin: second, side: .right, centerY: diodeCentreY),
        ]
        if let ground = pins.first(where: { $0.name == "GND" }) {
            slots.append(PinSlot(pin: ground, side: .bottom, centerY: 0, centerX: diodeSize.width / 2))
        }
        return ChipLayout(size: diodeSize, slots: slots)
    }

    /// An inductor or a fuse (plan-47-M): pin `1` on the left, pin `2` on the
    /// right, the diode's box. Every other pin (an `NC` pad, a further pad
    /// of the same name, joined to its twin inside the part) is not drawn.
    /// Falls back to the block layout if a terminal is missing.
    private static func twoTerminals(_ pins: [ChipPin]) -> ChipLayout {
        guard let first = pins.first(where: { $0.name == "1" }),
              let second = pins.first(where: { $0.name == "2" }) else {
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

    /// A relay (plan-47-N): the coil pins on the left and each pole's `NO`,
    /// `COM` and `NC` pins on the right (`RelayShape` says which row each is
    /// on), under a header with the part number. The body is `relayBodyWidth`
    /// wide and a half row taller than its lowest pin row, which leaves room
    /// for the coil voltage. A relay whose pins do not read as one
    /// (`RelayShape` is nil) falls back to the block layout.
    static let relayBodyWidth: CGFloat = 140

    private static func relay(_ part: CatalogPart) -> ChipLayout {
        guard let shape = RelayShape(part: part) else {
            return ChipLayout(pins: part.pins.filter { !$0.isHidden })
        }
        var slots: [PinSlot] = []
        for item in shape.leftPins {
            slots.append(PinSlot(pin: item.pin, side: .left, centerY: centerY(forRow: item.row)))
        }
        for item in shape.rightPins {
            slots.append(PinSlot(pin: item.pin, side: .right, centerY: centerY(forRow: item.row)))
        }
        return ChipLayout(
            size: CGSize(width: relayBodyWidth, height: headerHeight + pitch * CGFloat(shape.rowCount) + pitch / 2),
            slots: slots
        )
    }

    /// A buzzer (plan-47-N): `+` on the left, `-` on the right, on the
    /// diode's centre line; any other pin (a feedback electrode, a mounting
    /// pad) is not drawn. Falls back to the block layout if a terminal is
    /// missing.
    private static func buzzer(_ pins: [ChipPin]) -> ChipLayout {
        guard let plus = pins.first(where: { $0.name == "+" }),
              let minus = pins.first(where: { $0.name == "-" }) else {
            return ChipLayout(pins: pins)
        }
        return ChipLayout(
            size: diodeSize,
            slots: [
                PinSlot(pin: plus, side: .left, centerY: diodeCentreY),
                PinSlot(pin: minus, side: .right, centerY: diodeCentreY),
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

    /// A gate symbol's body is 60 wide and 20 per input tall, at least 40.
    static let gateWidth: CGFloat = 60

    static func gateHeight(inputs: Int) -> CGFloat {
        max(40, pitch * CGFloat(inputs))
    }

    /// Where a gate's output `index` of `count` leaves, down the body: one in
    /// the middle; two a grid step above (the true one) and below it.
    static func gateOutputY(_ index: Int, of count: Int, height: CGFloat) -> CGFloat {
        count == 1 ? height / 2 : height / 2 - 10 + 20 * CGFloat(index)
    }

    /// Where a gate's input `index` of `count` enters: 20 apart, centred.
    static func gateInputY(_ index: Int, of count: Int, height: CGFloat) -> CGFloat {
        height / 2 - 10 * CGFloat(count - 1) + 20 * CGFloat(index)
    }

    /// One gate (plan-67): its inputs down the left side, its one or two
    /// outputs on the right. `pins` are the part's; a pin of the shape that is
    /// not among them is left out.
    init(gate shape: GateShape, pins: [ChipPin]) {
        let inputs = shape.inputs.compactMap { index in pins.first { $0.index == index } }
        let outputs = shape.outputs.compactMap { output in pins.first { $0.index == output.pin } }
        let height = Self.gateHeight(inputs: inputs.count)
        var slots: [PinSlot] = []
        for (index, pin) in inputs.enumerated() {
            slots.append(PinSlot(pin: pin, side: .left,
                                 centerY: Self.gateInputY(index, of: inputs.count, height: height)))
        }
        for (index, pin) in outputs.enumerated() {
            slots.append(PinSlot(pin: pin, side: .right,
                                 centerY: Self.gateOutputY(index, of: outputs.count, height: height)))
        }
        self.slots = slots
        self.size = CGSize(width: Self.gateWidth, height: height)
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
