import Foundation
import Observation

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
    /// Actuator ("Standard", "Round Button", ...). Only narrows tactile switches.
    var tactileActuator: String?
    /// Channel count, by converter type ("adc", "dac"), so the ADC and DAC
    /// groups filter separately. Resolution is not a filter: the groups are
    /// split by bits instead (`FunctionGroup.bitGroups`).
    var converterChannels: [String: Int] = [:]
    /// Supply rail in millivolts. Keeps parts whose supply range covers it and
    /// switches rated for at least it; LEDs and displays have no supply and stay.
    var voltageMV: UInt32?
    /// Which flagged parts to list. Set from the user's preferences.
    var visibility = PartVisibility()

    /// Whether any menu narrows the list (everything but the query and
    /// `visibility`).
    var hasMenuFilter: Bool {
        category != nil || package != nil || family != nil || gateCount != nil || switchPositions != nil
            || color != nil || displayDigits != nil || displaySize != nil || tactileActuator != nil
            || !converterChannels.isEmpty || voltageMV != nil
    }
}

/// Which kinds of flagged parts the drawer lists. A part with several flags is
/// listed only when every one of its flags is allowed.
struct PartVisibility: Equatable {
    var showObsolete = false
    var showMilitary = false
    var showAutomotive = false
}

/// How a part is drawn on the canvas.
func symbol(for part: CatalogPart) -> PartSymbol {
    switch part.kind {
    case PART_SWITCH: return part.isTactileSwitch ? .tactileSwitch : part.isSlideSwitch ? .slideSwitch : .dipSwitch
    case PART_LED: return .diode
    case PART_7SEG, PART_LCD: return .sevenSegment
    case PART_NIXIE: return .nixie
    case PART_DEKATRON: return .dekatron
    case PART_CHARLCD: return .characterLCD
    case PART_OSCILLATOR: return .oscillator
    case PART_SUPPLY:
        guard case let .supply(level, _, pull) = part.details else { return .vcc }
        if pull { return level == 0 ? .pullDown : .pullUp }
        return level == 0 ? .ground : .vcc
    case PART_CONNECTOR:
        // A test point is a circle on a stub; a USB connector and a card slot are plain blocks of their pins.
        if part.isTestPoint { return .testPoint }
        if case .connector(_, _, _, _, _?) = part.details { return .block }
        if case .cardSlot = part.details { return .block }
        if case .wireConnector = part.details { return .block }
        return .barrelJack
    case PART_HEADER: return .header
    case PART_DIODE:
        // A single diode with one anode and cathode draws as a diode; arrays
        // and bridges as a block.
        guard case let .diode(_, diodes, _, _, _, _) = part.details, diodes == 1,
              part.pins.contains(where: { $0.name == "A" }),
              part.pins.contains(where: { $0.name == "K" }) else { return .block }
        return .plainDiode
    case PART_CAPACITOR: return .capacitor
    case PART_RESISTOR:
        // A single resistor with pins 1 and 2 draws as a resistor; arrays
        // as a block (plan-34-G).
        guard case let .resistor(_, _, _, resistors) = part.details, resistors == 1,
              part.pins.contains(where: { $0.name == "1" }),
              part.pins.contains(where: { $0.name == "2" }) else { return .block }
        return .resistor
    case PART_TRANSISTOR:
        // A single transistor with one pin of each B, C, E (or G, D, S)
        // draws as a transistor; arrays, and parts whose pins are named
        // otherwise, as a block (plan-40-H).
        guard case let .transistor(info) = part.details, info.elements == 1 else { return .block }
        let names = Set(part.pins.filter { !$0.isHidden }.map(\.name))
        return names == ["B", "C", "E"] || names == ["G", "D", "S"] ? .transistor : .block
    case PART_REGULATOR:
        // A regulator draws as a block, its pins named (plan-42-I).
        return .block
    case PART_POWERSWITCH:
        // A power switch draws as a block, its pins named (plan-55-J).
        return .block
    case PART_CRYSTAL:
        // A crystal or resonator with its two terminals draws as the
        // plate-rectangle-plate symbol (plan-44-G); anything else as a block.
        return part.pins.contains(where: { $0.name == "X1" }) && part.pins.contains(where: { $0.name == "X2" })
            ? .crystal : .block
    case PART_INDUCTOR, PART_FUSE:
        // A fixed inductor or a fuse with its two terminals (pins named 1 and
        // 2) draws as the coil or the rectangle (plan-47-M); else as a block.
        guard part.pins.contains(where: { $0.name == "1" }), part.pins.contains(where: { $0.name == "2" })
        else { return .block }
        return part.kind == PART_INDUCTOR ? .inductor : .fuse
    case PART_RELAY:
        // A relay whose pins read as a coil and poles (`RelayShape`) draws
        // as a relay (plan-47-N); anything else as a block.
        return RelayShape(part: part) == nil ? .block : .relay
    case PART_BUZZER:
        // A buzzer with its `+` and `-` pins draws as the half disc.
        return part.pins.contains(where: { $0.name == "+" }) && part.pins.contains(where: { $0.name == "-" })
            ? .buzzer : .block
    default: return .block
    }
}
