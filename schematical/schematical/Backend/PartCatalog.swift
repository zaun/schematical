import Foundation
import Observation

// MARK: - The C library

/// Owns the backend's part library (`parts.h`) for the app's life. Circuits
/// borrow it, so it outlives every simulation.
nonisolated final class PartLibraryHandle: @unchecked Sendable {
    let pointer: OpaquePointer

    init() {
        pointer = part_library_create()
    }

    deinit {
        part_library_destroy(pointer)
    }
}

// MARK: - Parts as Swift values

/// How a pin is used (`PinRole` in parts.h).
nonisolated enum ChipPinRole: Hashable, Sendable {
    case input
    case output
    case inOut
    case power
    case ground
    case noConnect
    /// A numbered position with no lead. Never drawn; nothing can be wired to it.
    case absent
}

nonisolated struct ChipPin: Hashable, Sendable, Identifiable {
    /// The group a pin is in unless the part file names another.
    static let defaultGroup = "default"

    /// Position in the part's pinout, from 0; the backend's pin index.
    let index: Int
    /// The package's label for the pin: "14" on a DIP, "A1" on a BGA.
    let designator: String
    let name: String
    let role: ChipPinRole
    /// Pins in one group are drawn together on the block, such as an SRAM's
    /// address pins. Most pins are in the default group.
    let group: String

    var id: Int { index }

    var isInDefaultGroup: Bool { group == Self.defaultGroup }

    /// Positions with no lead are left out of every symbol.
    var isHidden: Bool { role == .absent }
}

/// How a 7-segment display is drawn and which LED is which.
nonisolated struct DisplayInfo: Hashable, Sendable {
    /// One LED: a segment of a digit (1 = leftmost), or a colon or dot LED
    /// (digit 0; segment "L1", "L2" or "L3").
    struct LED: Hashable, Sendable {
        let digit: Int
        let segment: String
    }

    let digits: Int
    let commonAnode: Bool
    /// Digit the colon (L1, L2) and the raised dot (L3) sit after; 0 = none.
    let colonAfter: Int
    let dotAfter: Int
    let color: String
    /// Digit height, such as "0.56 in".
    let size: String
    /// The LED chip, such as "AlInGaP Super Red".
    let chip: String
    /// In the order the simulation reports their lights.
    let leds: [LED]
    /// Segment forward voltage, millivolts (shown, not checked).
    var vfMinMV: UInt32 = 0
    var vfMaxMV: UInt32 = 0
    /// Pin indices (into `CatalogPart.pins`) for the left (segments) and right
    /// (commons) sides, top to bottom.
    let segmentPins: [Int]
    let commonPins: [Int]
}

/// A character LCD module's panel (`PartCharLcd.panel`).
nonisolated enum LCDPanel: Hashable, Sendable {
    /// STN positive: dark dots on yellow-green.
    case yellowGreen
    /// STN negative: light dots on blue.
    case blueNegative

    var title: String {
        switch self {
        case .yellowGreen: return "Yellow-green"
        case .blueNegative: return "Blue negative"
        }
    }
}

/// How a character LCD module is drawn and what it is.
nonisolated struct CharacterLCDInfo: Hashable, Sendable {
    let columns: Int
    let rows: Int
    let controller: String
    /// The character ROM ("A00").
    let rom: String
    let panel: LCDPanel
    /// Nil when the module has no backlight.
    let backlightColor: String?
    /// Pin indices (into `CatalogPart.pins`): RS, R/W, E, DB0-DB7.
    let busPins: [Int]
}

/// A part's land pattern, copied from the backend's `PartFootprint`. Sizes and
/// positions are millimetres; the origin is the body centre, y grows down.
nonisolated struct Footprint: Hashable, Sendable {
    enum PadShape: Hashable, Sendable {
        case rect
        /// An ellipse; a circle when width = height.
        case round
    }

    /// Corners of a rect pad, as seen on the board (y down). The raw values
    /// are the backend's `PAD_CORNER_*` bits.
    struct Corners: OptionSet, Hashable, Sendable {
        let rawValue: Int
        static let topLeft = Corners(rawValue: 1)
        static let topRight = Corners(rawValue: 2)
        static let bottomRight = Corners(rawValue: 4)
        static let bottomLeft = Corners(rawValue: 8)

        /// The same corners after a clockwise turn of `degrees` (a multiple
        /// of 90): each 90° moves top-left to top-right, and so on round.
        func rotated(by degrees: Int) -> Corners {
            let steps = ((degrees / 90) % 4 + 4) % 4
            var bits = rawValue & 0xF
            for _ in 0..<steps { bits = ((bits << 1) | (bits >> 3)) & 0xF }
            return Corners(rawValue: bits)
        }

        /// The same corners mirrored left to right when `on` (a bottom-side
        /// footprint): top-left and top-right swap, and so do the bottom two.
        func mirrored(_ on: Bool) -> Corners {
            guard on else { return self }
            let bits = rawValue & 0xF
            return Corners(rawValue: ((bits & 0b0101) << 1) | ((bits & 0b1010) >> 1))
        }
    }

    struct Pad: Hashable, Sendable {
        /// The pin this pad belongs to (`ChipPin.designator`); nil for an
        /// unnumbered centre pad.
        let designator: String?
        let centre: CGPoint
        let size: CGSize
        /// Hole diameter; 0 = surface mount.
        let drill: Double
        let shape: PadShape
        /// Rect pads only: every corner rounded by this radius; 0 = square.
        let radius: Double
        /// Rect pads only: `corners` cut at 45°, this far along each edge.
        let chamfer: Double
        let corners: Corners
    }

    let id: String
    let description: String
    let body: CGSize
    let bodyRound: Bool
    let pads: [Pad]

    // Ids are unique across the library, so a footprint is its id.
    static func == (a: Footprint, b: Footprint) -> Bool { a.id == b.id }
    func hash(into hasher: inout Hasher) { hasher.combine(id) }
}

/// One orderable part, copied from the backend's `Part` once at load.
nonisolated struct CatalogPart: Identifiable, Hashable, Sendable {
    /// One of a part's `footprintChoices`: the package name shown for it and
    /// its footprint.
    struct FootprintChoice: Hashable, Sendable {
        let package: String
        let footprint: Footprint
    }

    struct Flags: Hashable, Sendable {
        var obsolete = false
        var military = false
        var automotive = false
        var enhanced = false
        var ceramic = false
    }

    /// The facts that depend on the part's kind.
    enum Details: Hashable, Sendable {
        case logic(family: String, units: Int)
        case sram(words: Int, bits: Int, accessNs: Int, grade: String)
        case eeprom(words: Int, bits: Int, accessNs: Int, grade: String, pageBytes: Int, writeCycleUs: Int)
        /// `ratedMV`: the contacts' switching voltage rating.
        case dipSwitch(positions: Int, actuator: String, ratedMV: UInt32)
        /// An LED: the colors a placed one may have; the first is the default.
        case led(colors: [String])
        case display(DisplayInfo)
        case oscillator(family: String, minHz: UInt64, maxHz: UInt64, defaultHz: UInt64, enablePin: String?)
        /// `mv`: the supply's voltage in millivolts (0 for ground); `pull`:
        /// a pull-up or pull-down part (drives weakly).
        case supply(level: Int, mv: UInt32, pull: Bool)
        /// A processor: its core ("6502", "65c02", "z80"), speed grade and
        /// highest clock.
        case cpu(core: String, grade: String, maxHz: UInt64)
        case characterLCD(CharacterLCDInfo)
        /// A serial controller (plan-12): its model, its serial channels
        /// (0 for the SPI-to-I2C bridge) and its grade.
        case serial(model: String, channels: Int, grade: String)
        case connector(centerPinUM: UInt32, ratedMV: UInt32, ratedMA: UInt32, contacts: Int)
        /// A parallel I/O peripheral on the 65xx bus (plan-14): its model
        /// ("pia6521", "via6522"), speed grade and highest clock.
        case pio(model: String, grade: String, maxHz: UInt64)
        /// A generic pin header (plan-23): rows (1 or 2), pins per row, and
        /// the genders a placed one may have (the first is the default).
        case header(rows: Int, perRow: Int, genders: [String])
        /// An ADC or DAC (plan-24): its type ("adc", "dac"), resolution in
        /// bits and channels.
        case converter(type: String, bits: Int, channels: Int)
        /// A real-time clock (plan-24): no kind-specific facts.
        case rtc
    }

    let name: String
    let kind: PartKind
    let package: String
    /// The parts-drawer leaf: "NAND", "32 KB", "LED", a frequency band.
    let function: String
    let description: String
    /// The parts-drawer path above the function, such as ["Memory", "SRAM"].
    /// Empty for parts that are never listed (VCC, GND).
    let drawer: [String]
    let flags: Flags
    let pins: [ChipPin]
    let details: Details
    /// Nil only for parts with no package (the supplies).
    let footprint: Footprint?
    /// The second footprint the PCB Style option switches to (Connector
    /// Only); nil for most parts.
    let connectorFootprint: Footprint?
    /// The footprints a placed part may use instead of `footprint`, in
    /// picker order; one of them is `footprint` (the default). Empty for
    /// most parts.
    let footprintChoices: [FootprintChoice]
    /// The recommended supply range and the highest input voltage,
    /// millivolts. Nil for supplies, LEDs, 7-segment displays and switches.
    let vcc: (minMV: UInt32, maxMV: UInt32, inputMaxMV: UInt32)?
    /// A dual-supply part's two supply domains (plan-20-F), A side first: the
    /// supply pin's name and the side's range; empty for single-supply parts.
    let supplyDomains: [(power: String, minMV: UInt32, maxMV: UInt32, inputMaxMV: UInt32)]

    var id: String { name }

    // Names are unique across the library, so a part is its name.
    static func == (a: CatalogPart, b: CatalogPart) -> Bool { a.name == b.name }
    func hash(into hasher: inout Hasher) { hasher.combine(name) }

    /// The voltage shown in the drawer: the supply range ("2–6 V") for logic,
    /// memory and oscillators, the forward voltage ("VF 2–2.6 V") for a
    /// 7-segment display, the rating ("24 V max") for a switch. Nil for LEDs
    /// and supplies.
    var voltageLabel: String? {
        if let vcc {
            return "\(VoltageCheck.volts(vcc.minMV))–\(VoltageCheck.volts(vcc.maxMV)) V"
        }
        switch details {
        case let .display(info) where info.vfMaxMV > 0:
            return "VF \(VoltageCheck.volts(info.vfMinMV))–\(VoltageCheck.volts(info.vfMaxMV)) V"
        case let .dipSwitch(_, _, ratedMV) where ratedMV > 0:
            return "\(VoltageCheck.volts(ratedMV)) V max"
        case let .connector(_, ratedMV, _, _) where ratedMV > 0:
            return "\(VoltageCheck.volts(ratedMV)) V max"
        default:
            return nil
        }
    }

    /// Gate, flip-flop, latch, counter, shift register, decoder, buffer,
    /// multiplexer or arithmetic chip: a logic chip with a family and a unit
    /// count.
    var isLogic: Bool {
        kind == PART_GATE || kind == PART_DFF || kind == PART_JKFF ||
            kind == PART_LATCH || kind == PART_COUNTER || kind == PART_SHIFT ||
            kind == PART_DECODER || kind == PART_BUFFER || kind == PART_MUX || kind == PART_ARITH
    }

    /// SRAM or EEPROM: a part with a memory window.
    var isMemory: Bool { kind == PART_SRAM || kind == PART_EEPROM }

    /// A processor: a part with a processor window.
    var isCpu: Bool { kind == PART_CPU }

    /// A converter or RTC: a part with a register window (plan-24-I).
    var hasRegisters: Bool { kind == PART_CONVERTER || kind == PART_RTC }

    /// A pull-up or pull-down: a resistor on the board, with a resistance
    /// (`PlacedChip.ohms`) and footprint choices.
    var isPullResistor: Bool {
        if case .supply(_, _, pull: true) = details { return true }
        return false
    }

    /// A serial part with at least one channel: a part with a terminal window.
    var hasTerminal: Bool {
        if case let .serial(_, channels, _) = details { return channels > 0 }
        return false
    }

    /// A memory's word count and width, from the part's facts.
    var memorySize: MemorySize? {
        switch details {
        case let .sram(words, bits, _, _): return MemorySize(words: words, bits: bits)
        case let .eeprom(words, bits, _, _, _, _): return MemorySize(words: words, bits: bits)
        default: return nil
        }
    }

    var logicFamily: String? {
        if case let .logic(family, _) = details { return family }
        return nil
    }

    var display: DisplayInfo? {
        if case let .display(info) = details { return info }
        return nil
    }

    var characterLCD: CharacterLCDInfo? {
        if case let .characterLCD(info) = details { return info }
        return nil
    }

    /// A header's rows and pins per row; nil for other parts.
    var headerSize: (rows: Int, perRow: Int)? {
        if case let .header(rows, perRow, _) = details { return (rows, perRow) }
        return nil
    }

    /// A header's genders (the first is the default); empty for other parts.
    var genders: [String] {
        if case let .header(_, _, genders) = details { return genders }
        return []
    }

    /// The colors of an LED (the first is the default), or a 7-segment
    /// display's one color. Empty for other parts.
    var colors: [String] {
        switch details {
        case let .led(colors): return colors
        case let .display(info): return [info.color]
        default: return []
        }
    }

    /// Every package the part can be placed in: its own, then each footprint
    /// choice's.
    var allPackages: [String] {
        [package] + footprintChoices.map(\.package)
    }

    /// The count shown next to the name in the drawer: gates or flip-flops per
    /// package, switch positions, display digits. Nil for other parts.
    var count: Int? {
        switch details {
        case let .logic(_, units): return units
        case let .dipSwitch(positions, _, _): return positions
        case let .display(info): return info.digits
        default: return nil
        }
    }

    /// The label at the right of the part's drawer row: the count in
    /// parentheses ("(4)"), or an oscillator's frequency ("32.768 kHz").
    var trailingLabel: String? {
        if case let .oscillator(_, _, _, defaultHz, _) = details {
            return PartFacts.hertz(defaultHz)
        }
        if case let .characterLCD(info) = details { return "\(info.columns)×\(info.rows)" }
        if let count, count > 0 { return "(\(count))" }
        return nil
    }

    /// The drawer's second line after the package: family, speed grade,
    /// actuator, color or size.
    var detailLabels: [String] {
        switch details {
        case let .logic(family, _): return [family]
        case let .sram(_, _, accessNs, _): return ["\(accessNs) ns"]
        case let .eeprom(_, _, accessNs, _, _, _): return ["\(accessNs) ns"]
        case let .dipSwitch(_, actuator, _): return [actuator]
        case let .led(colors): return colors.count == 1 ? colors : ["\(colors.count) colors"]
        case let .display(info): return [info.color, info.size, info.commonAnode ? "CA" : "CC"]
        case let .oscillator(family, _, _, _, _): return [family]
        case .supply: return []
        case let .cpu(_, _, maxHz): return ["\(maxHz / 1_000_000) MHz"]
        case let .characterLCD(info):
            return [info.panel.title] + (info.backlightColor.map { ["\($0) backlight"] } ?? [])
        case let .serial(_, channels, _): return channels > 1 ? ["\(channels) channels"] : []
        case let .pio(_, _, maxHz): return ["\(maxHz / 1_000_000) MHz"]
        case let .connector(centerPinUM, _, ratedMA, contacts):
            return [String(format: "%.1f mm", Double(centerPinUM) / 1000),
                    String(format: "%.1f A", Double(ratedMA) / 1000)] + (contacts > 0 ? ["switched"] : [])
        case let .header(rows, perRow, _): return ["\(rows)×\(perRow)"]
        case let .converter(_, bits, channels):
            return ["\(bits)-bit"] + (channels > 1 ? ["\(channels) channels"] : [])
        case .rtc: return []
        }
    }
}

// MARK: - Reading the C structs

nonisolated private func string(_ pointer: UnsafePointer<CChar>?) -> String {
    pointer.map { String(cString: $0) } ?? ""
}

nonisolated private func pinRole(_ role: PinRole) -> ChipPinRole {
    switch role {
    case PIN_INPUT: return .input
    case PIN_OUTPUT: return .output
    case PIN_INOUT: return .inOut
    case PIN_POWER: return .power
    case PIN_GROUND: return .ground
    case PIN_ABSENT: return .absent
    default: return .noConnect
    }
}

nonisolated private func indices(_ pointer: UnsafePointer<Int>?, _ count: Int) -> [Int] {
    guard let pointer else { return [] }
    return (0..<count).map { pointer[$0] }
}

extension Footprint {
    /// Copies one footprint out of the library.
    nonisolated init(_ footprint: PartFootprint) {
        id = string(footprint.id)
        description = string(footprint.description)
        body = CGSize(width: footprint.body_width, height: footprint.body_height)
        bodyRound = footprint.body_round != 0
        pads = (0..<footprint.pad_count).compactMap { index in
            guard let pad = footprint.pads?[index] else { return nil }
            return Pad(designator: pad.designator.map { String(cString: $0) },
                       centre: CGPoint(x: pad.x, y: pad.y),
                       size: CGSize(width: pad.width, height: pad.height),
                       drill: pad.drill,
                       shape: pad.shape == PAD_ROUND ? .round : .rect,
                       radius: pad.radius,
                       chamfer: pad.chamfer,
                       corners: Corners(rawValue: Int(pad.corners)))
        }
    }
}

extension CatalogPart {
    /// Copies one part out of the library. `footprint` and
    /// `connectorFootprint` are the Swift copies of `part.footprint` and
    /// `part.connector_footprint`, shared by every part that uses them.
    nonisolated init(_ part: Part, footprint: Footprint?, connectorFootprint: Footprint? = nil,
                     footprintChoices: [FootprintChoice] = []) {
        self.footprint = footprint
        self.connectorFootprint = connectorFootprint
        self.footprintChoices = footprintChoices
        vcc = part.vcc.map { ($0.pointee.min_mv, $0.pointee.max_mv, $0.pointee.input_max_mv) }
        supplyDomains = (0..<part.domain_count).compactMap { k in
            guard let domain = part.domains?[k], let range = domain.vcc?.pointee else { return nil }
            let power = domain.power_pin_count > 0
                ? string(part.pins?[domain.power_pins[0]].name) : ""
            return (power, range.min_mv, range.max_mv, range.input_max_mv)
        }
        name = string(part.name)
        kind = part.kind
        package = string(part.package)
        function = string(part.function)
        description = string(part.description)
        drawer = (0..<part.drawer_count).map { string(part.drawer?[$0]) }
        flags = Flags(
            obsolete: part.flags & UInt32(PART_OBSOLETE) != 0,
            military: part.flags & UInt32(PART_MILITARY) != 0,
            automotive: part.flags & UInt32(PART_AUTOMOTIVE) != 0,
            enhanced: part.flags & UInt32(PART_ENHANCED) != 0,
            ceramic: part.flags & UInt32(PART_CERAMIC) != 0
        )
        pins = (0..<part.pin_count).compactMap { index in
            guard let pin = part.pins?[index] else { return nil }
            return ChipPin(index: index, designator: string(pin.designator), name: string(pin.name),
                           role: pinRole(pin.role), group: string(pin.group))
        }

        if let logic = part.logic?.pointee {
            details = .logic(family: string(logic.family), units: Int(logic.units))
        } else if let sram = part.sram?.pointee {
            details = .sram(words: Int(sram.words), bits: Int(sram.bits),
                            accessNs: Int(sram.access_ps / 1000), grade: string(sram.grade))
        } else if let eeprom = part.eeprom?.pointee {
            details = .eeprom(words: Int(eeprom.words), bits: Int(eeprom.bits),
                              accessNs: Int(eeprom.access_ps / 1000), grade: string(eeprom.grade),
                              pageBytes: Int(eeprom.page_bytes),
                              writeCycleUs: Int(eeprom.write_cycle_ps / 1_000_000))
        } else if let dip = part.dip_switch?.pointee {
            details = .dipSwitch(positions: Int(dip.positions), actuator: string(dip.actuator),
                                 ratedMV: dip.rated_mv)
        } else if let led = part.led?.pointee {
            details = .led(colors: (0..<led.color_count).map { string(led.colors?[$0]) })
        } else if let display = part.display?.pointee {
            let leds: [DisplayInfo.LED] = (0..<display.led_count).compactMap { index in
                guard let led = display.leds?[index] else { return nil }
                return DisplayInfo.LED(digit: Int(led.digit), segment: string(led.segment))
            }
            details = .display(DisplayInfo(
                digits: Int(display.digits),
                commonAnode: display.common_anode != 0,
                colonAfter: Int(display.colon_after),
                dotAfter: Int(display.dot_after),
                color: string(display.color),
                size: string(display.size),
                chip: string(display.chip),
                leds: leds,
                vfMinMV: display.vf_min_mv,
                vfMaxMV: display.vf_max_mv,
                segmentPins: indices(display.segment_pins, display.segment_pin_count),
                commonPins: indices(display.common_pins, display.common_pin_count)
            ))
        } else if let osc = part.oscillator?.pointee {
            details = .oscillator(family: string(osc.family), minHz: osc.min_hz, maxHz: osc.max_hz,
                                  defaultHz: osc.default_hz,
                                  enablePin: osc.enable_pin.map { String(cString: $0) })
        } else if let supply = part.supply?.pointee {
            details = .supply(level: Int(supply.level), mv: supply.mv, pull: supply.pull != 0)
        } else if let cpu = part.cpu?.pointee {
            details = .cpu(core: string(cpu.core), grade: string(cpu.grade), maxHz: cpu.max_hz)
        } else if let lcd = part.charlcd?.pointee {
            details = .characterLCD(CharacterLCDInfo(
                columns: Int(lcd.columns),
                rows: Int(lcd.rows),
                controller: string(lcd.controller),
                rom: string(lcd.rom),
                panel: string(lcd.panel) == "blue-negative" ? .blueNegative : .yellowGreen,
                backlightColor: lcd.backlight_color.map { String(cString: $0) },
                busPins: indices(lcd.bus_pins, lcd.bus_pin_count)
            ))
        } else if let serial = part.serial?.pointee {
            details = .serial(model: string(serial.model), channels: Int(serial.channels),
                              grade: string(serial.grade))
        } else if let pio = part.pio?.pointee {
            details = .pio(model: string(pio.model), grade: string(pio.grade), maxHz: pio.max_hz)
        } else if let header = part.header?.pointee {
            let rows = max(Int(header.rows), 1)
            details = .header(rows: rows, perRow: Int(part.pin_count) / rows,
                              genders: (0..<header.gender_count).map { string(header.genders?[$0]) })
        } else if let connector = part.connector?.pointee {
            details = .connector(centerPinUM: connector.center_diameter_um, ratedMV: connector.rated_mv,
                                 ratedMA: connector.rated_ma, contacts: Int(connector.contacts))
        } else if let converter = part.converter?.pointee {
            details = .converter(type: string(converter.type), bits: Int(converter.bits),
                                 channels: Int(converter.channels))
        } else if part.kind == PART_RTC {
            details = .rtc
        } else {
            details = .supply(level: 0, mv: 0, pull: false)
        }
    }
}

/// Loads every part file bundled with the app (`Parts/*.json`, copied flat
/// into the bundle's resources) into a library.
nonisolated enum BackendCatalog {
    enum LoadError: LocalizedError {
        case unavailable
        case malformed(String)

        var errorDescription: String? {
            switch self {
            case .unavailable:
                return "The app has no part files"
            case .malformed(let detail):
                return "A part file could not be read (\(detail))"
            }
        }
    }

    /// The footprint file; every other bundled JSON resource is a part file.
    static let footprintsFile = "footprints.json"

    /// Loads `footprints.json` first (part files name its footprints), then
    /// every other bundled JSON resource as a part file, in file-name order.
    static func load(into library: PartLibraryHandle) throws -> [CatalogPart] {
        let urls = (Bundle.main.urls(forResourcesWithExtension: "json", subdirectory: nil) ?? [])
            .sorted { $0.lastPathComponent < $1.lastPathComponent }
        guard let footprintsURL = urls.first(where: { $0.lastPathComponent == footprintsFile }) else {
            throw LoadError.unavailable
        }
        try loadFile(footprintsURL, with: load_footprints, into: library)
        for url in urls where url != footprintsURL {
            try loadFile(url, with: load_parts, into: library)
        }

        var footprints: [String: Footprint] = [:]
        for index in 0..<part_library_footprint_count(library.pointer) {
            guard let footprint = part_library_footprint_at(library.pointer, index) else { continue }
            let copy = Footprint(footprint.pointee)
            footprints[copy.id] = copy
        }
        let count = part_library_count(library.pointer)
        return (0..<count).compactMap { index in
            part_library_at(library.pointer, index).map { part in
                CatalogPart(part.pointee,
                            footprint: part.pointee.footprint.flatMap { footprints[string($0.pointee.id)] },
                            connectorFootprint: part.pointee.connector_footprint
                                .flatMap { footprints[string($0.pointee.id)] },
                            footprintChoices: (0..<part.pointee.footprint_choice_count).compactMap { k in
                                guard let choice = part.pointee.footprint_choices?[k],
                                      let footprint = choice.footprint.flatMap({ footprints[string($0.pointee.id)] })
                                else { return nil }
                                return CatalogPart.FootprintChoice(package: string(choice.package),
                                                                   footprint: footprint)
                            })
            }
        }
    }

    /// Reads one bundled file and hands it to a backend loader.
    private static func loadFile(
        _ url: URL,
        with loader: (OpaquePointer?, UnsafePointer<CChar>?, UnsafePointer<CChar>?,
                      UnsafeMutablePointer<UnsafeMutablePointer<CChar>?>?) -> Int32,
        into library: PartLibraryHandle
    ) throws {
        let text: String
        do {
            text = try String(contentsOf: url, encoding: .utf8)
        } catch {
            throw LoadError.malformed("\(url.lastPathComponent): \(error.localizedDescription)")
        }
        var error: UnsafeMutablePointer<CChar>?
        if loader(library.pointer, text, url.lastPathComponent, &error) != 0 {
            let message = error.map { String(cString: $0) } ?? url.lastPathComponent
            free(error)
            throw LoadError.malformed(message)
        }
    }
}

// MARK: - Filtering

struct DrawerFilter: Equatable {
    var query = ""
    /// The drawer's top level, such as Logic or Memory.
    var category: String?
    var package: String?
    /// Logic family. Only narrows gates and flip-flops.
    var family: String?
    /// Gates or flip-flops per package. Only narrows gates and flip-flops.
    var gateCount: Int?
    /// Positions per switch. Only narrows switches.
    var switchPositions: Int?
    /// LED color ("Red", "Green", ...). Only narrows LEDs and 7-segment displays.
    var color: String?
    /// Digits and digit height. Only narrow 7-segment displays.
    var displayDigits: Int?
    var displaySize: String?
    /// Supply rail in millivolts. Keeps parts whose supply range covers it and
    /// switches rated for at least it; LEDs and displays have no supply and stay.
    var voltageMV: UInt32?
    /// Which flagged parts to list. Set from the user's preferences.
    var visibility = PartVisibility()
}

/// Which kinds of flagged parts the drawer lists. A part with several flags is
/// listed only when every one of its flags is allowed.
struct PartVisibility: Equatable {
    var showObsolete = false
    var showMilitary = false
    var showAutomotive = false

    func allows(_ part: CatalogPart) -> Bool {
        (showObsolete || !part.flags.obsolete)
            && (showMilitary || !part.flags.military)
            && (showAutomotive || !part.flags.automotive)
    }
}

/// How a part is drawn on the canvas.
func symbol(for part: CatalogPart) -> PartSymbol {
    switch part.kind {
    case PART_SWITCH: return .dipSwitch
    case PART_LED: return .diode
    case PART_7SEG: return .sevenSegment
    case PART_CHARLCD: return .characterLCD
    case PART_OSCILLATOR: return .oscillator
    case PART_SUPPLY:
        guard case let .supply(level, _, pull) = part.details else { return .vcc }
        if pull { return level == 0 ? .pullDown : .pullUp }
        return level == 0 ? .ground : .vcc
    case PART_CONNECTOR: return .barrelJack
    case PART_HEADER: return .header
    default: return .block
    }
}

// MARK: - Store

/// Holds the part library as the drawer sees it. The part files are bundled
/// with the app, so the library is read once at launch.
@MainActor
@Observable
final class PartCatalogStore {
    enum LoadState: Equatable {
        case notLoaded
        case loaded
        case failed(String)
    }

    struct FunctionGroup: Identifiable, Hashable {
        let function: String
        let parts: [CatalogPart]

        var id: String { function }
    }

    struct SubcategoryGroup: Identifiable, Hashable {
        let subcategory: String
        let functions: [FunctionGroup]
        /// Parts listed straight under the subcategory, with no function level:
        /// switches (the function is only the vendor's series), LEDs,
        /// 7-segment displays and character LCDs.
        var parts: [CatalogPart] = []

        var id: String { subcategory }
    }

    struct CategoryGroup: Identifiable, Hashable {
        let category: String
        /// Parts with a second drawer level, grouped under it (Memory → SRAM → ...).
        let subcategories: [SubcategoryGroup]
        /// Parts without one, listed directly under the category.
        let functions: [FunctionGroup]

        var id: String { category }
    }

    /// The C library; circuits are built from it.
    let library = PartLibraryHandle()
    private(set) var loadState: LoadState = .notLoaded

    /// Listed parts (a non-empty drawer path).
    private var entries: [CatalogPart] = []
    private var partsByName: [String: CatalogPart] = [:]

    var partCount: Int { entries.count }

    var categories: [String] {
        sortedNames(entries.compactMap(\.drawer.first))
    }

    var packages: [String] {
        sortedNames(entries.flatMap(\.allPackages))
    }

    /// The kinds of part listed under a top-level drawer category.
    func kinds(inCategory category: String) -> Set<UInt32> {
        Set(entries.filter { $0.drawer.first == category }.map(\.kind.rawValue))
    }

    /// The kinds of part listed under a second-level drawer group.
    func kinds(inSubcategory subcategory: String) -> Set<UInt32> {
        Set(entries.filter { $0.drawer.count > 1 && $0.drawer[1] == subcategory }.map(\.kind.rawValue))
    }

    /// Logic families of the gates and flip-flops in a category.
    func families(in category: String) -> [String] {
        sortedNames(entries.filter { $0.drawer.first == category }.compactMap(\.logicFamily))
    }

    /// Gates (or flip-flops) per package in a category, ascending.
    func logicCounts(in category: String) -> [Int] {
        Array(Set(entries.filter { $0.drawer.first == category && $0.isLogic }.compactMap(\.count))).sorted()
    }

    /// Positions per switch in a category, ascending.
    func switchPositionCounts(in category: String) -> [Int] {
        Array(Set(entries.filter { $0.drawer.first == category && $0.kind == PART_SWITCH }
            .compactMap(\.count))).sorted()
    }

    /// LED and display colors in a category.
    func colors(in category: String) -> [String] {
        sortedNames(entries.filter { $0.drawer.first == category }.flatMap(\.colors))
    }

    /// Digit counts and digit heights of the 7-segment displays.
    var displayDigitCounts: [Int] {
        Array(Set(entries.compactMap(\.display).map(\.digits))).sorted()
    }

    var displaySizes: [String] {
        sortedNames(entries.compactMap(\.display).map(\.size))
    }

    /// The VCC tool's rails (`SupplyChoice`) that at least one listed part
    /// runs at, in menu order, with their voltage in millivolts.
    var voltageChoices: [(name: String, mv: UInt32)] {
        SupplyChoice.names.compactMap { name in
            guard case let .supply(_, mv, _)? = partsByName[name]?.details, mv > 0,
                  entries.contains(where: { $0.vcc.map { $0.minMV...$0.maxMV ~= mv } ?? false })
            else { return nil }
            return (name, mv)
        }
    }

    /// Any part by name, listed or not (VCC and GND are not listed).
    func part(named name: String) -> CatalogPart? {
        partsByName[name]
    }

    /// Parts grouped by drawer path, then by function, after applying the filter.
    func groups(matching filter: DrawerFilter) -> [CategoryGroup] {
        let needle = filter.query.trimmingCharacters(in: .whitespaces)

        let matches = entries.filter { part in
            if !filter.visibility.allows(part) {
                return false
            }
            if let category = filter.category, part.drawer.first != category {
                return false
            }
            if let package = filter.package, !part.allPackages.contains(package) {
                return false
            }
            if part.isLogic {
                if let family = filter.family, part.logicFamily != family {
                    return false
                }
                if let count = filter.gateCount, part.count != count {
                    return false
                }
            }
            if part.kind == PART_SWITCH, let count = filter.switchPositions, part.count != count {
                return false
            }
            if let mv = filter.voltageMV {
                if let vcc = part.vcc, !(vcc.minMV...vcc.maxMV ~= mv) {
                    return false
                }
                if case let .dipSwitch(_, _, ratedMV) = part.details, ratedMV > 0, ratedMV < mv {
                    return false
                }
            }
            if let color = filter.color, !part.colors.isEmpty, !part.colors.contains(color) {
                return false
            }
            if let display = part.display {
                if let count = filter.displayDigits, display.digits != count {
                    return false
                }
                if let size = filter.displaySize, display.size != size {
                    return false
                }
            }
            let fields = [part.name, part.function, part.description] + part.drawer
            return needle.isEmpty || fields.contains { $0.localizedStandardContains(needle) }
        }

        let byCategory = Dictionary(grouping: matches, by: { $0.drawer[0] })
        return byCategory
            .sorted { $0.key.localizedStandardCompare($1.key) == .orderedAscending }
            .map { category, parts in
                let bySubcategory = Dictionary(
                    grouping: parts.filter { $0.drawer.count > 1 },
                    by: { $0.drawer[1] }
                )
                let subcategories = bySubcategory
                    .sorted { $0.key.localizedStandardCompare($1.key) == .orderedAscending }
                    .map { subcategory, parts in
                        Self.listsPartsDirectly(parts)
                            ? SubcategoryGroup(subcategory: subcategory, functions: [], parts: Self.byCountThenName(parts))
                            : SubcategoryGroup(subcategory: subcategory, functions: Self.functionGroups(parts))
                    }
                return CategoryGroup(
                    category: category,
                    subcategories: subcategories,
                    functions: Self.functionGroups(parts.filter { $0.drawer.count == 1 })
                )
            }
    }

    /// Switches, LEDs, 7-segment displays, character LCDs and the generic
    /// header are listed with no function level.
    private static func listsPartsDirectly(_ parts: [CatalogPart]) -> Bool {
        parts.allSatisfy {
            $0.kind == PART_SWITCH || $0.kind == PART_LED || $0.kind == PART_7SEG || $0.kind == PART_CHARLCD
                || $0.kind == PART_HEADER
        }
    }

    /// Parts by count (positions, digits), then by name.
    private static func byCountThenName(_ parts: [CatalogPart]) -> [CatalogPart] {
        parts.sorted {
            ($0.count ?? 0) != ($1.count ?? 0)
                ? ($0.count ?? 0) < ($1.count ?? 0)
                : $0.name.localizedStandardCompare($1.name) == .orderedAscending
        }
    }

    /// Parts grouped by function, functions and parts in natural order
    /// ("8 KB" before "32 KB").
    private static func functionGroups(_ parts: [CatalogPart]) -> [FunctionGroup] {
        Dictionary(grouping: parts, by: \.function)
            .sorted { $0.key.localizedStandardCompare($1.key) == .orderedAscending }
            .map { function, parts in
                FunctionGroup(
                    function: function,
                    parts: parts.sorted { $0.name.localizedStandardCompare($1.name) == .orderedAscending }
                )
            }
    }

    private func sortedNames(_ names: [String]) -> [String] {
        Array(Set(names)).sorted { $0.localizedStandardCompare($1) == .orderedAscending }
    }

    func load() {
        do {
            let parts = try BackendCatalog.load(into: library)
            partsByName = Dictionary(parts.map { ($0.name, $0) }, uniquingKeysWith: { first, _ in first })
            // The 80 header sizes are one generic header in the drawer
            // (`HeaderFamily`); `part(named:)` still finds every size.
            entries = parts.filter {
                !$0.drawer.isEmpty && ($0.kind != PART_HEADER || $0.name == HeaderFamily.defaultPart)
            }
            loadState = .loaded
        } catch {
            loadState = .failed(error.localizedDescription)
        }
    }
}
