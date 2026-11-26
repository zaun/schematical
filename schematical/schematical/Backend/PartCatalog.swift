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

    /// Takes one single-part-file part out of the library (plan-53-D): its
    /// name, uuid and the inline footprint and datasheet ids named. The
    /// memory stays, so parts already read stay valid. False when no part
    /// has the uuid.
    func retire(uuid: String, footprintIDs: [String], datasheetIDs: [String]) -> Bool {
        func withCStrings<R>(_ strings: [String], _ body: ([UnsafePointer<CChar>?]) -> R) -> R {
            var pointers: [UnsafePointer<CChar>?] = []
            var duplicates: [UnsafeMutablePointer<CChar>] = []
            for string in strings {
                if let copy = strdup(string) {
                    duplicates.append(copy)
                    pointers.append(UnsafePointer(copy))
                }
            }
            defer { duplicates.forEach { free($0) } }
            return body(pointers)
        }
        return withCStrings(footprintIDs) { footprints in
            withCStrings(datasheetIDs) { datasheets in
                part_library_retire_part(pointer, uuid, footprints, footprints.count,
                                         datasheets, datasheets.count) == 0
            }
        }
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

/// Which side of the block a pin is drawn on. `.auto`: inputs on the left,
/// outputs and inouts on the right. `.input`/`.output`: left/right whatever the
/// role, such as a transceiver's A port (left) and B port (right).
nonisolated enum ChipPinSide: Hashable, Sendable {
    case auto, input, output
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
    /// The block side the part file puts the pin on; `.auto` follows its role.
    var side: ChipPinSide = .auto
    /// The part file's "activeLow" (plan-39-A): bit i set means alternative i
    /// of `name` (split at "/") is active low. 0 = none.
    var activeLowMask: UInt32 = 0

    var id: Int { index }

    /// One slash-separated part of a pin's name, as written ("A0", "CS",
    /// "CE#"), and whether the part file marks it active low.
    struct Alternative: Hashable, Sendable {
        let text: String
        let isActiveLow: Bool
    }

    /// The name split at "/", left to right, each with its polarity. A name
    /// of more than 32 alternatives can only be flagged as a whole, so bits
    /// past 31 follow an all-ones mask.
    var alternatives: [Alternative] {
        name.split(separator: "/", omittingEmptySubsequences: false).enumerated().map { index, part in
            let low = index < 32 ? activeLowMask & (UInt32(1) << UInt32(index)) != 0
                                 : activeLowMask == UInt32.max
            return Alternative(text: String(part), isActiveLow: low)
        }
    }

    /// Some alternative of the name is active low.
    var isActiveLow: Bool { activeLowMask != 0 }

    var isInDefaultGroup: Bool { group == Self.defaultGroup }

    /// Positions with no lead are left out of every symbol.
    var isHidden: Bool { role == .absent }
}

/// How a 7-segment display is drawn and which LED is which.
/// A switch's nominal body size from the part file's "bodySize" (length,
/// the larger side, then width), in micrometres.
nonisolated struct BodySize: Hashable, Sendable {
    let lengthUM: UInt32
    let widthUM: UInt32

    /// "6 × 6 mm", "6.2 × 3.5 mm".
    var label: String {
        func mm(_ um: UInt32) -> String {
            um % 1000 == 0 ? String(um / 1000) : String(format: "%g", Double(um) / 1000)
        }
        return "\(mm(lengthUM)) × \(mm(widthUM)) mm"
    }
}

/// A transistor part's facts (plan-40-H), from the backend's
/// `PartTransistor`. Every rating is 0 when not given; an array's facts hold
/// for every element.
nonisolated struct TransistorInfo: Hashable, Sendable {
    /// The first element's type: "npn", "pnp", "nmos" or "pmos".
    let type: String
    /// The elements in the package (more than 1: an array).
    let elements: Int
    /// The elements' types differ (a complementary pair).
    let mixed: Bool
    let darlington: Bool
    /// An "nmos" or "pmos" part.
    let mosfet: Bool
    /// Rds(on) is specified at a gate drive of 4.5 V or less.
    let logicLevel: Bool
    /// Vceo or Vds, in millivolts.
    let vmaxMV: UInt32
    /// Ic or Id, in milliamps.
    let imaxMA: UInt32
    /// Pd, in milliwatts.
    let powerMW: UInt32
    /// The minimum DC current gain.
    let hfe: UInt32
    /// The maximum gate threshold voltage, in millivolts.
    let vgsthMV: UInt32
    /// The maximum gate-source voltage, in millivolts.
    let vgsMV: UInt32
    /// A pre-biased BJT's base and base-emitter resistors, in milliohms.
    let r1Milliohm: UInt64
    let r2Milliohm: UInt64
    /// The maximum Rds(on), in milliohms.
    let rdsonMilliohm: UInt64

    /// A pre-biased BJT: one with a base resistor.
    var preBiased: Bool { !mosfet && (r1Milliohm > 0 || r2Milliohm > 0) }
    /// A P-type element: "pnp" or "pmos".
    var pType: Bool { type == "pnp" || type == "pmos" }
}

/// A voltage regulator part's facts (plan-42-I), from the backend's
/// `PartRegulator`. Every value is 0 when not given.
nonisolated struct RegulatorInfo: Hashable, Sendable {
    /// The first element's type: "linear", "buck", "boost", "buck-boost",
    /// "charge-pump" or "sepic".
    let type: String
    /// The regulators in the package (more than 1: a dual, triple or quad).
    let regulators: Int
    /// The elements' types differ.
    let mixed: Bool
    /// Some element has an adjust pin.
    let adjustable: Bool
    /// One output voltage per element, in millivolts (0 = not fixed).
    let voutMV: [UInt32]
    /// One reference voltage per element, in millivolts (0 = none).
    let vrefMV: [UInt32]
    /// The highest output an adjustable part can be set to, in millivolts.
    let voutMaxMV: UInt32
    /// The input range's ends, in millivolts.
    let vinMinMV: UInt32
    let vinMaxMV: UInt32
    /// A linear element's dropout, in millivolts.
    let dropoutMV: UInt32
    /// The output current per element, in milliamps.
    let ioutMA: UInt32
    /// The quiescent current, in microamps.
    let iqUA: UInt32
    /// A switching element's frequency, in hertz.
    let fswHz: UInt64
}

/// A power switch part's facts (plan-55-J), from the backend's
/// `PartPowerSwitch`. Every value is 0 when not given.
nonisolated struct PowerSwitchInfo: Hashable, Sendable {
    /// "load", "limited" (a current-limited or USB switch) or "ideal-diode".
    let type: String
    /// The switches in the package (more than 1: a dual, triple or quad).
    let switches: Int
    /// The input range's ends, in millivolts.
    let vinMinMV: UInt32
    let vinMaxMV: UInt32
    /// The continuous current per switch and a current-limited switch's
    /// limit, in milliamps.
    let ioutMA: UInt32
    let ilimitMA: UInt32
    /// The on-resistance, in milliohms.
    let ronMOhm: UInt32
    /// The quiescent current, in microamps.
    let iqUA: UInt32
}

/// A crystal or ceramic resonator part's facts (plan-44-G), from the
/// backend's `PartCrystal`. A value is 0 when not given.
nonisolated struct CrystalInfo: Hashable, Sendable {
    /// "crystal" or "resonator" (a ceramic resonator).
    let type: String
    /// The frequency in hertz.
    let hz: UInt64
    /// The load capacitance in femtofarads (18 pF = 18 000); 0 = series
    /// resonant, or a resonator with built-in capacitors.
    let loadFF: UInt64
    /// The frequency tolerance and stability in parts per million.
    let tolerancePPM: UInt32
    let stabilityPPM: UInt32
    /// The ESR (a resonator's impedance) in milliohms.
    let esrMOhm: UInt64
    /// A resonator with built-in load capacitors (a third, common pin).
    let capacitors: Bool

    /// "±20 ppm"; empty when not given.
    static func ppmText(_ ppm: UInt32) -> String { ppm > 0 ? "±\(ppm) ppm" : "" }
}

/// An inductor part's facts (plan-47-M), from the backend's `PartInductor`.
/// A value is 0 (or nil) when not given.
nonisolated struct InductorInfo: Hashable, Sendable {
    /// The inductance in picohenries.
    let pH: UInt64
    /// As the datasheet prints it ("±20%"); nil when not given.
    let tolerance: String?
    /// The rated (heating) and the saturation current, in milliamps.
    let ratedMA: UInt32
    let isatMA: UInt32
    /// The maximum DC resistance, in micro-ohms.
    let dcrMicroOhm: UInt64
    /// "shielded", "semi-shielded" or "unshielded"; nil when not given.
    let shielding: String?
}

/// A fuse or PTC resettable fuse part's facts (plan-47-M), from the backend's
/// `PartFuse`. A value is 0 (or nil) when not given.
nonisolated struct FuseInfo: Hashable, Sendable {
    /// "fuse" or "ptc".
    let type: String
    /// A fuse's rated current; a PTC's hold current, in milliamps.
    let ratedMA: UInt32
    /// A PTC's trip current, in milliamps (0 for a fuse).
    let tripMA: UInt32
    /// The voltage ratings, in millivolts (a PTC's maximum voltage is `dcMV`).
    let acMV: UInt32
    let dcMV: UInt32
    /// A fuse's "fast", "medium" or "slow"; nil for a PTC.
    let speed: String?
    /// A fuse's breaking capacity, a PTC's maximum fault current, in amps.
    let breakingA: UInt32

    var isPTC: Bool { type == "ptc" }
}

/// A relay part's facts (plan-47-M), from the backend's `PartRelay`. A value
/// is 0 when not given.
nonisolated struct RelayInfo: Hashable, Sendable {
    /// "electromechanical", "reed" or "solid-state".
    let type: String
    /// Contacts (poles) and coils in the package; `channels`: armatures.
    let poles: Int
    let coils: Int
    let latching: Bool
    let channels: Int
    /// The nominal coil voltage in millivolts (0 for a solid-state relay) and
    /// the coil resistance in milliohms.
    let coilMV: UInt32
    let coilMOhm: UInt64
    /// The contact (load) current in milliamps and the maximum switching
    /// voltage in millivolts.
    let contactMA: UInt32
    let contactMV: UInt32
    /// The datasheet's operate and release times in microseconds.
    let operateUS: UInt32
    let releaseUS: UInt32
    /// A solid-state relay's maximum on-resistance, in milliohms.
    let onMOhm: UInt64
}

/// A buzzer part's facts (plan-47-M), from the backend's `PartBuzzer`. A
/// value is 0 (or empty) when not given.
nonisolated struct BuzzerInfo: Hashable, Sendable {
    /// "indicator" (built-in driver) or "transducer" (external drive).
    let driver: String
    /// "piezo" or "magnetic".
    let technology: String
    /// The rated (resonant) frequency in hertz.
    let hz: UInt32
    /// The rated voltage and the operating range, in millivolts.
    let ratedMV: UInt32
    let minMV: UInt32
    let maxMV: UInt32
    /// The supply current in microamperes.
    let uA: UInt32
    /// The sound pressure level in hundredths of a dB.
    let splCDB: UInt32
}

/// A 555-type timer part's facts (plan-48-H), from the backend's `PartTimer`.
nonisolated struct TimerInfo: Hashable, Sendable {
    /// Timer sections: 1, or 2 for a 556.
    let timers: Int
    /// "bipolar" or "cmos".
    let process: String
    /// The highest astable frequency, in hertz.
    let maxHz: UInt64
}

/// An op-amp part's facts (plan-48-H), from the backend's `PartOpamp`. A
/// value is 0 when the datasheet and DigiKey give none.
nonisolated struct OpampInfo: Hashable, Sendable {
    /// "general", "jfet" or "cmos".
    let type: String
    /// Amplifiers in the package: 1, 2 or 4.
    let amplifiers: Int
    /// Rail-to-rail input and output.
    let railIn: Bool
    let railOut: Bool
    /// The gain bandwidth product in hertz.
    let gbwHz: UInt64
    /// The slew rate in millivolts per microsecond.
    let slewMvUs: UInt32
    /// The maximum input offset voltage in microvolts.
    let vosUv: UInt32
}

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
    /// Segment forward voltage, millivolts (shown, not checked). A
    /// numitron's segment voltage; a VFD's anode voltage.
    var vfMinMV: UInt32 = 0
    var vfMaxMV: UInt32 = 0
    /// What the segments are: "led", "numitron" (incandescent filaments) or
    /// "vfd" (vacuum fluorescent; the commons are grids).
    var type: String = "led"
    /// Pin indices (into `CatalogPart.pins`) for the left (segments) and right
    /// (commons) sides, top to bottom.
    let segmentPins: [Int]
    let commonPins: [Int]
}

/// A dekatron's facts (`PartDekatron`): one anode, ten cathode positions in a
/// ring and the guides that step the glow round it.
nonisolated struct DekatronInfo: Hashable, Sendable {
    /// Pin indices (into `CatalogPart.pins`): `positionPins[p]` is the pin of
    /// the cathode at position p (0-9, also the order the simulation reports
    /// the lights); several positions may share a pin.
    let positionPins: [Int]
    /// The distinct cathode pins, in the file's order.
    let cathodePins: [Int]
    let anodePin: Int
    /// The guides, in the file's order (a single-pulse tube's first guide is
    /// the driven one).
    let guidePins: [Int]
    /// A two-guide tube counts both ways with two overlapping pulses; a
    /// single-pulse tube counts forward with one.
    let twoGuide: Bool
    /// The glow color name ("Orange"), the bulb size, "side" or "end" view.
    let color: String
    let size: String
    let view: String
    /// Least anode supply, running voltage, millivolts.
    let strikeMV: UInt32
    let sustainMV: UInt32
    /// Anode current range, microamps; the highest counting rate, hertz.
    let currentMinUA: UInt32
    let currentMaxUA: UInt32
    let rateHz: UInt32
}

/// A nixie tube's facts (`PartNixie`): one anode and one cathode per glyph.
nonisolated struct NixieInfo: Hashable, Sendable {
    /// What each cathode shows, in the order the simulation reports their
    /// lights: a numeral, a symbol, "DP" (right decimal point) or "LDP".
    let glyphs: [String]
    /// Pin indices (into `CatalogPart.pins`): `cathodePins[i]` is glyph i's.
    let cathodePins: [Int]
    let anodePins: [Int]
    /// The glow color name ("Orange").
    let color: String
    /// Character height, such as "18 mm".
    let size: String
    /// "side" (read through the envelope's side) or "end" (through its top).
    let view: String
    /// Firing voltage, the least the anode supply may be; maintaining
    /// voltage; millivolts.
    let strikeMV: UInt32
    let sustainMV: UInt32
    /// Cathode current range, microamps.
    let currentMinUA: UInt32
    let currentMaxUA: UInt32

    /// The numerals and symbols, without the decimal points.
    var characters: [String] { glyphs.filter { $0 != "DP" && $0 != "LDP" } }
}

/// A serial (SPI) memory chip's facts (`PartSerialmem`). Zero (nil for the
/// JEDEC ID) means the datasheet value is not given.
nonisolated struct SerialMemoryInfo: Hashable, Sendable {
    /// "eeprom", "fram", "flash", "dataflash" or "pageeeprom".
    let type: String
    /// Capacity in bytes.
    let size: UInt32
    /// Address bytes after a READ or WRITE opcode.
    let addressBytes: Int
    /// The EEPROM write page or the flash program page, bytes.
    let pageBytes: UInt32
    /// The EEPROM write cycle or the flash page-program time, picoseconds.
    let writePS: UInt64
    let sectorErasePS: UInt64
    let chipErasePS: UInt64
    /// The RDID bytes as hex; nil when the chip has none.
    let jedecID: String?
    let maxClockKHz: UInt32
    /// Write cycles.
    let endurance: UInt64
    /// Years.
    let retentionYears: UInt32
}

/// A character LCD module's panel (`PartCharLcd.panel`).
nonisolated enum LCDPanel: Hashable, Sendable {
    /// STN positive: dark dots on yellow-green.
    case yellowGreen
    /// STN negative: light dots on blue.
    case blueNegative
    /// STN positive: dark dots on light gray (plan-35).
    case gray
    /// FSTN positive: black dots on white (plan-35).
    case white

    /// The part file's `"panel"` string; anything unknown is yellow-green.
    init(_ name: String) {
        switch name {
        case "blue-negative": self = .blueNegative
        case "gray": self = .gray
        case "white": self = .white
        default: self = .yellowGreen
        }
    }

    var title: String {
        switch self {
        case .yellowGreen: return "Yellow-green"
        case .blueNegative: return "Blue negative"
        case .gray: return "Gray"
        case .white: return "White"
        }
    }

    /// The Facts sidebar's "Panel" row.
    var facts: String {
        switch self {
        case .yellowGreen: return "STN yellow-green, positive"
        case .blueNegative: return "STN blue, negative"
        case .gray: return "STN gray, positive"
        case .white: return "FSTN white, positive"
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
    /// The backlight LEDs' colors, in the order the simulation reports their
    /// lights; empty when the module has no backlight (plan-35: 1 or 3).
    let backlightColors: [String]
    /// Pin indices (into `CatalogPart.pins`): RS, R/W, E (or E1, E2),
    /// DB0-DB7.
    let busPins: [Int]
    /// 1, or 2 for a module with two controllers (a 40 x 4: E1 and E2).
    let enables: Int

    /// The first LED's color; nil when the module has no backlight.
    var backlightColor: String? { backlightColors.first }

    /// The Facts sidebar's "Backlight" row and the drawer's second line.
    var backlightTitle: String? {
        switch backlightColors.count {
        case 0: return nil
        case 1: return backlightColors[0]
        default:
            return backlightColors == ["Red", "Green", "Blue"] ? "RGB" : backlightColors.joined(separator: ", ")
        }
    }
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
        /// A slotted hole's length along the pad's longer side (its width is
        /// `drill`); 0 = a round hole.
        var slotLength: Double = 0
        /// False for a non-plated hole: no copper, the pad is the hole itself.
        var plated: Bool = true
        /// A surface-mount pad on the other face of the board from the
        /// footprint's own side (plan-54-E); it swaps layers with the part's
        /// side. See `PlacedPad.layerSide(on:)`.
        var opposite: Bool = false
    }

    let id: String
    let description: String
    let body: CGSize
    let bodyRound: Bool
    let pads: [Pad]
    /// The housing straddles the board edge (plan-54-E): the placement check
    /// asks only that its pads are on the board.
    var edgeMount: Bool = false

    // Ids are unique across the library, so a footprint is its id.
    static func == (a: Footprint, b: Footprint) -> Bool { a.id == b.id }
    func hash(into hasher: inout Hasher) { hasher.combine(id) }
}

/// One orderable part, copied from the backend's `Part` once at load.
/// How one gate is drawn (plan-67): the body, which pins come in on the left
/// and which leave on the right, and its marks. `PartUnit` in `parts.h`.
nonisolated struct GateShape: Hashable, Sendable {
    /// The outline. NAND is `.and` with an inverted output, an inverter is
    /// `.buffer` with one.
    enum Body: Hashable, Sendable { case and, or, xor, buffer }

    struct Output: Hashable, Sendable {
        /// `ChipPin.index`.
        let pin: Int
        /// Drawn with the inversion bubble.
        let inverted: Bool
    }

    let body: Body
    /// `ChipPin.index` of each input, top to bottom.
    let inputs: [Int]
    /// One or two; the true output first (CD4068B has both).
    let outputs: [Output]
    /// Schmitt-trigger inputs: the hysteresis mark.
    let schmitt: Bool
    /// Open-drain outputs: the diamond mark.
    let openDrain: Bool
}

/// What a unit of a part is drawn as.
nonisolated enum UnitShape: Hashable, Sendable {
    case gate(GateShape)
    /// A plain block of its pins, like a whole part's.
    case block
}

/// One piece of a part the Schematic can draw as its own symbol (plan-67):
/// a gate of a gate IC, or the block of the pins no gate owns.
nonisolated struct PartUnit: Hashable, Sendable, Identifiable {
    /// "A", "B", …; "P" for the block of leftover pins.
    let key: String
    /// `ChipPin.index` of every pin the unit draws. Never a hidden pin.
    let pins: [Int]
    let shape: UnitShape
    var id: String { key }

    /// The key of the block that holds the pins no gate owns.
    static let blockKey = "P"

    /// The key of gate unit `index`: A, B, … skipping P.
    static func key(forGate index: Int) -> String {
        let letters = "ABCDEFGHIJKLMNOQRSTUVWXYZ"
        guard index < letters.count else { return "U\(index + 1)" }
        return String(letters[letters.index(letters.startIndex, offsetBy: index)])
    }
}

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
        /// A tactile (momentary) switch, plan-27: one contact, closed while
        /// pressed (normally open) or while released (normally closed);
        /// `joined`: pin pairs connected inside the part (2 on a
        /// four-terminal switch, 0 on a two-terminal one); `body`: its
        /// nominal size, the drawer group it is listed under.
        case tactileSwitch(actuator: String, ratedMV: UInt32, normallyClosed: Bool, joined: Int, body: BodySize?)
        /// A slide switch (plan-58): `circuit` the first word of its function
        /// ("SPST", "SPDT", "DPST", "DPDT"); `positions` the knob positions
        /// (2 or 3); `rest` the one a new chip starts in (1-based); `masks`
        /// the contacts closed in each position, bit c - 1 for contact c.
        case slideSwitch(circuit: String, positions: Int, rest: Int, masks: [UInt32], actuator: String, ratedMV: UInt32)
        /// An LED: the colors a placed one may have; the first is the default.
        case led(colors: [String])
        case display(DisplayInfo)
        case nixie(NixieInfo)
        case dekatron(DekatronInfo)
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
        /// `host`: the host-side interface ("parallel", "spi", "i2c", "i2c-spi").
        case serial(model: String, channels: Int, grade: String, host: String)
        /// A barrel jack (`usb` nil) or a USB connector (`usb` "A", "B" or
        /// "C"; no centre pin or contacts, ratings 0 when not given).
        case connector(centerPinUM: UInt32, ratedMV: UInt32, ratedMA: UInt32, contacts: Int, usb: String?)
        /// A PCB terminal block or a JST board header (plan-45): `kind` is
        /// "terminal" or "jst"; `variant` the terminal block's clamp ("screw",
        /// "spring", "push-in", "pluggable") or the JST series ("XH", "PH",
        /// "EH", "ZH", "SH", "GH", "VH"); `pitchUM` the pole pitch in
        /// micrometres; the ratings are 0 when the maker gives none. A passive
        /// row of pins.
        case wireConnector(kind: String, variant: String, pitchUM: UInt32, ratedMV: UInt32, ratedMA: UInt32)
        /// A card slot (plan-54): a socket for a memory or SIM card, a memory
        /// module or a card edge; a passive block of pins. `card`: what the
        /// slot takes ("microSD", "DDR4 SODIMM", "PCI Express"); `pitchUM`:
        /// the contact pitch in micrometres, 0 when the part gives none.
        case cardSlot(card: String, pitchUM: UInt32)
        /// A test point (plan-57): one passive pin, a bare pad or plated hole
        /// to probe; a `connector` device with `"testPoint": true`.
        case testPoint
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
        /// An LED or LCD display driver (plan-56): no kind-specific facts.
        case displayDriver
        /// A serial (SPI) memory chip: EEPROM, F-RAM or flash (`SerialMemoryInfo`).
        case serialmem(SerialMemoryInfo)
        /// A diode part (plan-29): its type ("rectifier", "fast",
        /// "switching", "schottky", "zener", "bridge"), the diodes in the
        /// package, reverse voltage and forward current per diode (0 for a
        /// Zener), and a Zener's voltage and power (else 0).
        case diode(type: String, diodes: Int, vrMV: UInt32, ioMA: UInt32, vzMV: UInt32, pdMW: UInt32)
        /// A capacitor (plan-33): its type ("ceramic", "aluminum",
        /// "aluminum-polymer", "tantalum", "tantalum-polymer",
        /// "niobium-oxide"), whether it is polarized, its capacitance in
        /// farads, its rated voltage (0 = none: the generic part), and a
        /// ceramic's dielectric and the tolerance when given.
        case capacitor(type: String, polarized: Bool, farads: Double, ratedMV: UInt32,
                       dielectric: String?, tolerance: String?)
        /// A microcontroller (plan-36): its core ("avr", "avrxt", "riscv", …),
        /// highest clock, and flash, RAM, EEPROM and PSRAM in bytes (0 = none).
        case mcu(core: String, maxHz: UInt64, flash: UInt32, ram: UInt32, eeprom: UInt32, psram: UInt32)
        /// A programmable logic device (GAL, ATF16V8, ATF22V10, ATF750): its
        /// output macrocells and its speed grade, the maximum pin-to-pin
        /// delay, in picoseconds. Not simulated yet.
        case pld(macrocells: Int, tpdPS: UInt64)
        /// A resistor (plan-34): its resistance in ohms (0 = a jumper), its
        /// tolerance in percent and rated power per element in watts (nil when
        /// not given: the generic part, a jumper's tolerance), and the
        /// resistor elements in the package (more than 1: an array).
        case resistor(ohms: Double, tolerancePercent: Double?, powerW: Double?, resistors: Int)
        /// A transistor (plan-40): one or more BJT or MOSFET elements
        /// (more than 1: an array), its ratings (`TransistorInfo`).
        case transistor(TransistorInfo)
        /// A voltage regulator (plan-42): one to four linear or switching
        /// regulators (more than 1: a dual, triple or quad), its facts
        /// (`RegulatorInfo`).
        case regulator(RegulatorInfo)
        /// A power switch (plan-55): a load switch, a current-limited or
        /// USB switch or an integrated ideal diode, one to four switches
        /// (more than 1: a dual, triple or quad), its facts
        /// (`PowerSwitchInfo`).
        case powerswitch(PowerSwitchInfo)
        /// A crystal or ceramic resonator (plan-44): a two-terminal
        /// frequency reference, its facts (`CrystalInfo`).
        case crystal(CrystalInfo)
        /// An IC socket (plan-51): a passive block of pins a DIP or PLCC chip
        /// plugs into. `family`: "DIP" or "PLCC"; `rowUM`: a DIP socket's row
        /// spacing in micrometres (0 for PLCC); `mounting`: "through-hole" or
        /// "surface-mount"; `finish`: the contact finish ("gold", "tin",
        /// "tin-lead"), nil when not given. The chip footprints it holds are
        /// `CatalogPart.fits`.
        case socket(family: String, rowUM: UInt32, mounting: String, finish: String?)
        /// A fixed inductor (plan-47): a two-terminal part, its facts
        /// (`InductorInfo`).
        case inductor(InductorInfo)
        /// A fuse or PTC resettable fuse (plan-47), its facts (`FuseInfo`).
        case fuse(FuseInfo)
        /// A relay (plan-47), its facts (`RelayInfo`).
        case relay(RelayInfo)
        /// A buzzer (plan-47), its facts (`BuzzerInfo`).
        case buzzer(BuzzerInfo)
        /// A 555-type timer (plan-48), its facts (`TimerInfo`).
        case timer(TimerInfo)
        /// An operational amplifier (plan-48): facts only, it drives nothing
        /// in play mode (`OpampInfo`).
        case opamp(OpampInfo)
    }

    let name: String
    /// A custom part's identity: the `uuid` of the single part file it was
    /// registered from (plan-32-A). Nil for every bundled part.
    let uuid: String?
    let kind: PartKind
    let package: String
    /// The parts-drawer leaf: "NAND", "32 KB", "LED", a frequency band.
    let function: String
    let description: String
    /// The parts-drawer path above the function, 1–3 levels such as
    /// ["Memory", "SRAM"] or ["Discrete", "Resistors", "Chip"]. Empty for
    /// parts that are never listed (VCC, GND).
    private(set) var drawer: [String]
    let flags: Flags
    /// The company that makes the part; "Generic" for parts no one company
    /// makes (supplies, headers, the generic LED). Empty if not given.
    let manufacturer: String
    /// The manufacturer's PDF of the part's datasheet; nil for Generic parts.
    let datasheetURL: URL?
    /// The app's own Markdown datasheet for the part: its path under the
    /// bundled Datasheets folder ("gate-and/sn74hc08.md"); nil when the
    /// datasheet hasn't been converted.
    let datasheetMarkdown: String?
    let pins: [ChipPin]
    /// The pieces the Schematic can draw the part as (plan-67): a gate IC's
    /// gates, then the block `P` of the pins no gate owns. Empty for a part
    /// with no unit view.
    let units: [PartUnit]
    let details: Details
    /// False for a part the backend has no model for (plan-36: the
    /// microcontrollers). Run is off while one is on the schematic.
    let isSimulated: Bool
    /// Nil only for parts with no package (the supplies).
    let footprint: Footprint?
    /// The second footprint the PCB Style option switches to (Connector
    /// Only); nil for most parts.
    let connectorFootprint: Footprint?
    /// The footprints a placed part may use instead of `footprint`, in
    /// picker order; one of them is `footprint` (the default). Empty for
    /// most parts.
    let footprintChoices: [FootprintChoice]
    /// A socket's `fits` (plan-51): the chip footprints it holds, in file
    /// order. Empty for every other kind.
    let fits: [Footprint]
    /// The recommended supply range and the highest input voltage,
    /// millivolts. Nil for supplies, LEDs, 7-segment displays and switches.
    let vcc: (minMV: UInt32, maxMV: UInt32, inputMaxMV: UInt32)?
    /// A dual-supply part's two supply domains (plan-20-F), A side first: the
    /// supply pin's name and the side's range; empty for single-supply parts.
    let supplyDomains: [(power: String, minMV: UInt32, maxMV: UInt32, inputMaxMV: UInt32)]

    var id: String { name }

    /// A copy listed under another top-level drawer category, its deeper
    /// levels kept (plan-53-B: `["Custom", "Gates"]` lists as
    /// `["User Library", "Gates"]`). A part with no drawer path is never
    /// listed and is returned as it is.
    func listed(in category: String) -> CatalogPart {
        guard !drawer.isEmpty else { return self }
        var copy = self
        copy.drawer = [category] + drawer.dropFirst()
        return copy
    }

    // Names are unique across the library, so a part is its name (and, for a
    // custom part, its uuid).
    static func == (a: CatalogPart, b: CatalogPart) -> Bool { a.name == b.name && a.uuid == b.uuid }
    func hash(into hasher: inout Hasher) {
        hasher.combine(name)
        hasher.combine(uuid)
    }

    /// The voltage shown in the drawer: the supply range ("2–6 V") for logic,
    /// memory and oscillators, the forward voltage ("VF 2–2.6 V") for a
    /// 7-segment display, the rating ("24 V max") for a switch, the firing
    /// voltage ("170 V min") for a nixie tube. Nil for LEDs and supplies.
    var voltageLabel: String? {
        if let vcc {
            return "\(VoltageCheck.volts(vcc.minMV))–\(VoltageCheck.volts(vcc.maxMV)) V"
        }
        switch details {
        case let .display(info) where info.vfMaxMV > 0:
            return "VF \(VoltageCheck.volts(info.vfMinMV))–\(VoltageCheck.volts(info.vfMaxMV)) V"
        case let .nixie(info):
            return "\(VoltageCheck.volts(info.strikeMV)) V min"
        case let .dekatron(info):
            return "\(VoltageCheck.volts(info.strikeMV)) V min"
        case let .dipSwitch(_, _, ratedMV) where ratedMV > 0,
             let .tactileSwitch(_, ratedMV, _, _, _) where ratedMV > 0,
             let .slideSwitch(_, _, _, _, _, ratedMV) where ratedMV > 0:
            return "\(VoltageCheck.volts(ratedMV)) V max"
        case let .connector(_, ratedMV, _, _, _) where ratedMV > 0,
             let .wireConnector(_, _, _, ratedMV, _) where ratedMV > 0:
            return "\(VoltageCheck.volts(ratedMV)) V max"
        default:
            return nil
        }
    }

    /// A tactile (momentary push-button) switch, plan-27.
    var isTactileSwitch: Bool {
        if case .tactileSwitch = details { return true }
        return false
    }

    /// A slide switch (plan-58): a latching switch whose knob sits in one of
    /// two or three positions.
    var isSlideSwitch: Bool {
        if case .slideSwitch = details { return true }
        return false
    }

    /// A slide switch's circuit ("SPDT"), knob positions, starting position
    /// (both 1-based counts) and the contacts closed in each position (bit
    /// c - 1 for contact c); nil for other parts.
    var slideStates: (circuit: String, positions: Int, rest: Int, masks: [UInt32])? {
        if case let .slideSwitch(circuit, positions, rest, masks, _, _) = details {
            return (circuit, positions, rest, masks)
        }
        return nil
    }

    /// How many contacts a slide switch has (`PlacedChip.switchesOn` has one
    /// entry for each): the highest contact any of its positions closes; 0 for
    /// other parts.
    var slideContacts: Int {
        guard let slide = slideStates else { return 0 }
        return UInt32.bitWidth - slide.masks.reduce(0, |).leadingZeroBitCount
    }

    /// A terminal block or a JST board header (plan-45).
    var isWireConnector: Bool {
        if case .wireConnector = details { return true }
        return false
    }

    /// A card slot: a socket for a memory or SIM card, a module or a card edge (plan-54).
    var isCardSlot: Bool {
        if case .cardSlot = details { return true }
        return false
    }

    /// A test point: one passive pin on a bare pad or plated hole (plan-57).
    var isTestPoint: Bool {
        if case .testPoint = details { return true }
        return false
    }

    /// A tactile switch's contact is closed at rest (Normal Close).
    var isNormallyClosed: Bool {
        if case let .tactileSwitch(_, _, normallyClosed, _, _) = details { return normallyClosed }
        return false
    }

    /// A tactile switch's actuator ("Standard", "Round Button"); nil for
    /// other parts.
    var tactileActuator: String? {
        if case let .tactileSwitch(actuator, _, _, _, _) = details { return actuator }
        return nil
    }

    /// The drawer group a part is listed under, below its drawer path: a
    /// tactile switch's body size ("6 × 6 mm"), else its function.
    var drawerLeaf: String {
        if case let .tactileSwitch(_, _, _, _, body?) = details { return body.label }
        return function
    }

    /// Gate, flip-flop, latch, counter, shift register, decoder, buffer,
    /// multiplexer, arithmetic chip or register: a logic chip with a family and a unit
    /// count.
    var isLogic: Bool {
        kind == PART_GATE || kind == PART_DFF || kind == PART_JKFF ||
            kind == PART_LATCH || kind == PART_COUNTER || kind == PART_SHIFT ||
            kind == PART_DECODER || kind == PART_BUFFER || kind == PART_MUX || kind == PART_ARITH ||
            kind == PART_REGISTER
    }

    /// SRAM or EEPROM: a part with a memory window.
    var isMemory: Bool { kind == PART_SRAM || kind == PART_EEPROM }

    /// A processor (the cpu kind: a CPU on a bus).
    var isCpu: Bool { kind == PART_CPU }

    /// A microcontroller the backend runs (plan-62): it takes firmware and
    /// has a processor window. One with no simulation model is facts only.
    var isSimulatedMcu: Bool { kind == PART_MCU && isSimulated }

    /// A part with a processor window: a processor, or a simulated
    /// microcontroller.
    var hasProcessorWindow: Bool { isCpu || isSimulatedMcu }

    /// A part that takes a "load file on start": an EEPROM, or a simulated
    /// microcontroller (its firmware).
    var hasLoadFile: Bool { kind == PART_EEPROM || isSimulatedMcu }

    /// A converter, RTC or display driver: a part with a register window
    /// (plan-24-I, plan-56-G).
    var hasRegisters: Bool {
        kind == PART_CONVERTER || kind == PART_RTC || kind == PART_LEDDRIVER || kind == PART_LCDDRIVER
    }

    /// A pull-up or pull-down: a resistor on the board, with a resistance
    /// (`PlacedChip.ohms`) and footprint choices.
    var isPullResistor: Bool {
        if case .supply(_, _, pull: true) = details { return true }
        return false
    }

    /// Whether the part has an orientation its pin-1 mark on the silkscreen
    /// shows. A single resistor (and a pull resistor) and a capacitor that
    /// isn't polarized have none: either way round is the same part, so a
    /// new placement starts with that mark hidden (`BoardLayout.nextSpot`).
    var hasPinOneMark: Bool {
        switch details {
        case let .resistor(_, _, _, resistors) where resistors == 1: return false
        case .supply(_, _, pull: true): return false
        case .capacitor(_, polarized: false, _, _, _, _): return false
        default: return true
        }
    }

    /// `GENERIC-CAPACITOR` (plan-33): a capacitor of no one maker, with a
    /// capacitance per placed part (`PlacedChip.farads`) and footprint choices.
    var isGenericCapacitor: Bool { kind == PART_CAPACITOR && manufacturer == "Generic" }

    /// `GENERIC-RESISTOR` (plan-34): a resistor of no one maker, with a
    /// resistance per placed part (`PlacedChip.ohms`) and footprint choices.
    var isGenericResistor: Bool { kind == PART_RESISTOR && manufacturer == "Generic" }

    /// A part whose placed copies carry their own resistance
    /// (`PlacedChip.ohms`, the Ohms field): the pulls and `GENERIC-RESISTOR`.
    var hasOhmsField: Bool { isPullResistor || isGenericResistor }

    /// The resistance of a resistor part, in ohms; nil for every other kind.
    var resistorOhms: Double? {
        if case let .resistor(ohms, _, _, _) = details { return ohms }
        return nil
    }

    /// A transistor part's facts (plan-40-H); nil for every other kind.
    var transistor: TransistorInfo? {
        if case let .transistor(info) = details { return info }
        return nil
    }

    /// A voltage regulator part's facts (plan-42-I); nil for every other kind.
    var regulator: RegulatorInfo? {
        if case let .regulator(info) = details { return info }
        return nil
    }

    /// A power switch part's facts (plan-55-J); nil for every other kind.
    var powerswitch: PowerSwitchInfo? {
        if case let .powerswitch(info) = details { return info }
        return nil
    }

    /// A crystal or ceramic resonator part's facts (plan-44-G); nil for every other kind.
    var crystal: CrystalInfo? {
        if case let .crystal(info) = details { return info }
        return nil
    }

    /// The frequency of a crystal part, in hertz; nil for every other kind.
    var crystalHz: UInt64? { crystal?.hz }

    /// An inductor part's facts (plan-47-M); nil for every other kind.
    var inductor: InductorInfo? {
        if case let .inductor(info) = details { return info }
        return nil
    }

    /// A fuse or PTC part's facts (plan-47-M); nil for every other kind.
    var fuse: FuseInfo? {
        if case let .fuse(info) = details { return info }
        return nil
    }

    /// A relay part's facts (plan-47-M); nil for every other kind.
    var relay: RelayInfo? {
        if case let .relay(info) = details { return info }
        return nil
    }

    /// A buzzer part's facts (plan-47-M); nil for every other kind.
    var buzzer: BuzzerInfo? {
        if case let .buzzer(info) = details { return info }
        return nil
    }

    /// A timer part's facts (plan-48-H); nil for every other kind.
    var timer: TimerInfo? {
        if case let .timer(info) = details { return info }
        return nil
    }

    /// An op-amp part's facts (plan-48-H); nil for every other kind.
    var opamp: OpampInfo? {
        if case let .opamp(info) = details { return info }
        return nil
    }

    /// The number the drawer sorts the parts of a leaf by (plan-47-M): a
    /// crystal's frequency, an inductor's inductance, a fuse's rated (a PTC's
    /// hold) current, a buzzer's frequency, a relay's coil voltage; nil for
    /// every other kind.
    var drawerValue: Double? {
        switch details {
        case let .crystal(info): return Double(info.hz)
        case let .inductor(info): return Double(info.pH)
        case let .fuse(info): return Double(info.ratedMA)
        case let .buzzer(info): return Double(info.hz)
        case let .relay(info): return Double(info.coilMV)
        default: return nil
        }
    }

    /// The capacitance of a capacitor part; nil for every other kind.
    var capacitorFarads: Double? {
        if case let .capacitor(_, _, farads, _, _, _) = details { return farads }
        return nil
    }

    /// A serial part with at least one channel: a part with a terminal window.
    var hasTerminal: Bool {
        if case let .serial(_, channels, _, _) = details { return channels > 0 }
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

    /// The memory a load file fills: an EEPROM's words, or a simulated
    /// microcontroller's flash in bytes. Nil for a part with no load file.
    var loadFileSize: MemorySize? {
        if kind == PART_EEPROM { return memorySize }
        if isSimulatedMcu, case let .mcu(_, _, flash, _, _, _) = details {
            return MemorySize(words: Int(flash), bits: 8)
        }
        return nil
    }

    var logicFamily: String? {
        if case let .logic(family, _) = details { return family }
        return nil
    }

    var display: DisplayInfo? {
        if case let .display(info) = details { return info }
        return nil
    }

    var nixie: NixieInfo? {
        if case let .nixie(info) = details { return info }
        return nil
    }

    var dekatron: DekatronInfo? {
        if case let .dekatron(info) = details { return info }
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
    /// display's or nixie tube's one color. Empty for other parts.
    var colors: [String] {
        switch details {
        case let .led(colors): return colors
        case let .display(info): return [info.color]
        case let .nixie(info): return [info.color]
        case let .dekatron(info): return [info.color]
        default: return []
        }
    }

    /// Every package the part can be placed in: its own, then each footprint
    /// choice's.
    var allPackages: [String] {
        [package] + footprintChoices.map(\.package)
    }

    /// The count shown next to the name in the drawer: gates or flip-flops per
    /// package, switch positions, display digits, a card slot's or a terminal
    /// block's pins. Nil for other parts.
    var count: Int? {
        switch details {
        case let .logic(_, units): return units
        case let .dipSwitch(positions, _, _): return positions
        case let .display(info): return info.digits
        case .cardSlot, .wireConnector: return pins.count
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
        if let hz = crystalHz { return PartFacts.hertz(hz) }
        switch details {
        case let .inductor(info): return InductorValue.text(pH: info.pH)
        case let .fuse(info): return info.ratedMA > 0 ? PartFacts.amps(mA: info.ratedMA) : nil
        case let .relay(info):
            // The coil voltage; a solid-state relay has no coil, its load current instead.
            if info.coilMV > 0 { return PartFacts.volts(mV: info.coilMV) }
            return info.type == "solid-state" && info.contactMA > 0 ? PartFacts.amps(mA: info.contactMA) : nil
        case let .buzzer(info): return info.hz > 0 ? PartFacts.hertz(UInt64(info.hz)) : nil
        default: break
        }
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
        case let .slideSwitch(_, _, _, _, actuator, _): return [actuator]
        case let .tactileSwitch(actuator, _, normallyClosed, _, _):
            return [actuator] + (normallyClosed ? ["Normal close"] : [])
        case let .led(colors): return colors.count == 1 ? colors : ["\(colors.count) colors"]
        case let .display(info): return [info.color, info.size, info.commonAnode ? "CA" : "CC"]
        case let .nixie(info): return [info.size, info.view == "end" ? "End view" : "Side view"]
        case let .dekatron(info): return [info.size, info.twoGuide ? "2 guides" : "Single pulse"]
        case let .oscillator(family, _, _, _, _): return [family]
        case .supply: return []
        case let .cpu(_, _, maxHz): return ["\(maxHz / 1_000_000) MHz"]
        case let .characterLCD(info):
            return [info.panel.title] + (info.backlightTitle.map { ["\($0) backlight"] } ?? [])
        case let .serial(_, channels, _, host):
            return [PartFacts.serialHost(host)] + (channels > 1 ? ["\(channels) channels"] : [])
        case let .pio(_, _, maxHz): return ["\(maxHz / 1_000_000) MHz"]
        case let .wireConnector(kind, variant, pitchUM, _, ratedMA):
            let pitch = String(format: "%.2f mm", Double(pitchUM) / 1000)
            let current = ratedMA > 0 ? [String(format: "%g A", Double(ratedMA) / 1000)] : []
            return kind == "jst" ? [variant, pitch] : [pitch, PartFacts.clampName(variant)] + current
        case let .connector(_, _, ratedMA, _, usb?):
            return ["USB-\(usb)"] + (ratedMA > 0 ? [String(format: "%.1f A", Double(ratedMA) / 1000)] : [])
        case let .connector(centerPinUM, _, ratedMA, contacts, nil):
            return [String(format: "%.1f mm", Double(centerPinUM) / 1000),
                    String(format: "%.1f A", Double(ratedMA) / 1000)] + (contacts > 0 ? ["switched"] : [])
        case let .cardSlot(card, pitchUM):
            return [card] + (pitchUM > 0 ? [String(format: "%.2f mm", Double(pitchUM) / 1000)] : [])
        case .testPoint: return []
        case let .header(rows, perRow, _): return ["\(rows)×\(perRow)"]
        case let .converter(_, bits, channels):
            return ["\(bits)-bit"] + (channels > 1 ? ["\(channels) channels"] : [])
        case .rtc, .displayDriver: return []
        case let .serialmem(info):
            // The function level of the drawer already says the density.
            return info.maxClockKHz > 0 ? [PartFacts.hertz(UInt64(info.maxClockKHz) * 1000)] : []
        case let .diode(type, _, vrMV, ioMA, vzMV, pdMW):
            if type == "zener" {
                return ["\(PartFacts.milli(vzMV)) V", "\(PartFacts.milli(pdMW)) W"]
            }
            return ["\(PartFacts.milli(vrMV)) V", "\(PartFacts.milli(ioMA)) A"]
        case let .capacitor(_, _, farads, ratedMV, dielectric, _):
            return [CapacitorValue.text(farads)] + (ratedMV > 0 ? ["\(PartFacts.milli(ratedMV)) V"] : [])
                + (dielectric.map { [$0] } ?? [])
        case let .mcu(_, maxHz, _, _, _, _): return [PartFacts.hertz(maxHz)]
        case let .pld(_, tpdPS): return [PartFacts.duration(tpdPS)]
        case let .resistor(ohms, tolerancePercent, powerW, _):
            return [ResistorValue.text(ohms)]
                + (tolerancePercent.map { [PartFacts.percentText($0)] } ?? [])
                + (powerW.map { [PartFacts.wattsText($0)] } ?? [])
        case let .transistor(info):
            // The maximum rating pair, as the diode's: Vceo / Vds, Ic / Id.
            return (info.vmaxMV > 0 ? ["\(PartFacts.milli(info.vmaxMV)) V"] : [])
                + (info.imaxMA > 0 ? ["\(PartFacts.milli(info.imaxMA)) A"] : [])
        case let .regulator(info):
            // The output and the output current: "3.3 V", "1.8/3.3 V" for a
            // dual, "ADJ" for an adjustable part (plan-42-I).
            let outputs = info.voutMV.filter { $0 > 0 }
            let output = outputs.isEmpty ? (info.adjustable ? ["ADJ"] : [])
                : ["\(outputs.map { PartFacts.milli($0) }.joined(separator: "/")) V"]
            return output + (info.ioutMA > 0 ? ["\(PartFacts.milli(info.ioutMA)) A"] : [])
        case let .powerswitch(info):
            // The input maximum and the current: "≤ 5.5 V", "1.5 A" (plan-55-J).
            return (info.vinMaxMV > 0 ? ["≤ \(PartFacts.milli(info.vinMaxMV)) V"] : [])
                + (info.ioutMA > 0 ? ["\(PartFacts.milli(info.ioutMA)) A"] : [])
        case let .crystal(info):
            // The load capacitance and the tolerance, when the part gives them.
            return (info.loadFF > 0 ? [CapacitorValue.text(Double(info.loadFF) / 1e15)] : [])
                + (info.tolerancePPM > 0 ? [CrystalInfo.ppmText(info.tolerancePPM)] : [])
        case let .socket(_, _, mounting, finish):
            // The mounting and the contact finish: "through hole", "gold".
            return [mounting == "surface-mount" ? "surface mount" : "through hole"] + (finish.map { [$0] } ?? [])
        case let .inductor(info):
            // The rated current and the shielding (plan-47-M).
            return (info.ratedMA > 0 ? [PartFacts.amps(mA: info.ratedMA)] : [])
                + (info.shielding.map { [PartFacts.capitalized($0)] } ?? [])
        case let .fuse(info):
            if info.isPTC {
                // Trip current and maximum voltage.
                return (info.tripMA > 0 ? ["\(PartFacts.amps(mA: info.tripMA)) trip"] : [])
                    + (info.dcMV > 0 ? [PartFacts.volts(mV: info.dcMV)] : [])
            }
            // Speed and voltage (DC rating, else AC).
            let volts = info.dcMV > 0 ? info.dcMV : info.acMV
            return (info.speed.map { [PartFacts.capitalized($0)] } ?? [])
                + (volts > 0 ? [PartFacts.volts(mV: volts)] : [])
        case let .relay(info):
            // The contact current, and "latching" when it is.
            return (info.contactMA > 0 ? [PartFacts.amps(mA: info.contactMA)] : [])
                + (info.latching ? ["latching"] : [])
        case let .buzzer(info):
            // The rated voltage and the technology.
            return (info.ratedMV > 0 ? [PartFacts.volts(mV: info.ratedMV)] : [])
                + (info.technology.isEmpty ? [] : [PartFacts.capitalized(info.technology)])
        case let .timer(info):
            return [info.process == "cmos" ? "CMOS" : "Bipolar"]
        case let .opamp(info):
            return info.gbwHz > 0 ? [PartFacts.hertz(info.gbwHz)] : []
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

nonisolated private func pinSide(_ side: PinSide) -> ChipPinSide {
    switch side {
    case PIN_SIDE_IN: return .input
    case PIN_SIDE_OUT: return .output
    default: return .auto
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
                       corners: Corners(rawValue: Int(pad.corners)),
                       slotLength: pad.slot_length,
                       plated: pad.npth == 0,
                       opposite: pad.opposite != 0)
        }
        edgeMount = footprint.edge_mount != 0
    }
}

extension CatalogPart {
    /// What the sidebar's View option calls the unit view: "Gate" for a part
    /// whose units are gates; nil for a part with no unit view.
    var unitViewName: String? {
        units.contains { if case .gate = $0.shape { true } else { false } } ? "Gate" : nil
    }

    /// The part's units: the backend's (`part_unit_count`), keyed A, B, …,
    /// then one block keyed P for every drawn pin none of them owns.
    private nonisolated static func units(of part: Part, pins: [ChipPin]) -> [PartUnit] {
        var units: [PartUnit] = []
        withUnsafePointer(to: part) { pointer in
            for index in 0..<part_unit_count(pointer) {
                guard let unit = part_unit_at(pointer, index)?.pointee else { continue }
                let body: GateShape.Body = switch unit.shape {
                case UNIT_AND: .and
                case UNIT_OR: .or
                case UNIT_XOR: .xor
                default: .buffer
                }
                let hidden = Set(pins.filter(\.isHidden).map(\.index))
                let inputs = (0..<unit.input_count).compactMap { unit.inputs?[$0] }.map(Int.init)
                    .filter { !hidden.contains($0) }
                let both = [unit.outputs.0, unit.outputs.1]
                let outputs = both.prefix(Int(unit.output_count))
                    .map { GateShape.Output(pin: Int($0.pin), inverted: $0.inverted != 0) }
                    .filter { !hidden.contains($0.pin) }
                units.append(PartUnit(
                    key: PartUnit.key(forGate: units.count), pins: inputs + outputs.map(\.pin),
                    shape: .gate(GateShape(body: body, inputs: inputs, outputs: outputs,
                                           schmitt: unit.schmitt != 0, openDrain: unit.open_drain != 0))))
            }
        }
        guard !units.isEmpty else { return [] }
        let owned = Set(units.flatMap(\.pins))
        let rest = pins.filter { !$0.isHidden && !owned.contains($0.index) }.map(\.index)
        if !rest.isEmpty { units.append(PartUnit(key: PartUnit.blockKey, pins: rest, shape: .block)) }
        return units
    }

    /// Copies one part out of the library. `footprint` and
    /// `connectorFootprint` are the Swift copies of `part.footprint` and
    /// `part.connector_footprint`, shared by every part that uses them.
    nonisolated init(_ part: Part, footprint: Footprint?, connectorFootprint: Footprint? = nil,
                     footprintChoices: [FootprintChoice] = [], fits: [Footprint] = []) {
        self.footprint = footprint
        self.connectorFootprint = connectorFootprint
        self.footprintChoices = footprintChoices
        self.fits = fits
        vcc = part.vcc.map { ($0.pointee.min_mv, $0.pointee.max_mv, $0.pointee.input_max_mv) }
        supplyDomains = (0..<part.domain_count).compactMap { k in
            guard let domain = part.domains?[k], let range = domain.vcc?.pointee else { return nil }
            let power = domain.power_pin_count > 0
                ? string(part.pins?[domain.power_pins[0]].name) : ""
            return (power, range.min_mv, range.max_mv, range.input_max_mv)
        }
        name = string(part.name)
        uuid = part.uuid.map { String(cString: $0) }
        kind = part.kind
        isSimulated = part.simulated != 0
        package = string(part.package)
        function = string(part.function)
        description = string(part.description)
        manufacturer = string(part.manufacturer)
        datasheetURL = part.datasheet.flatMap { URL(string: string($0.pointee.url)) }
        datasheetMarkdown = part.datasheet.flatMap { $0.pointee.md }.map { String(cString: $0) }
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
                           role: pinRole(pin.role), group: string(pin.group),
                           side: pinSide(pin.side), activeLowMask: pin.active_low)
        }
        units = Self.units(of: part, pins: pins)

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
        } else if let dip = part.dip_switch?.pointee, dip.states > 0 {
            // The contacts each knob position closes, from the backend (plan-58-B).
            let masks: [UInt32] = (1...dip.states).map { state in
                var mask: UInt32 = 0
                let found = withUnsafePointer(to: part) { part_switch_state_contacts($0, state, &mask) }
                return found != 0 ? mask : 0
            }
            details = .slideSwitch(circuit: String(string(part.function).prefix { $0 != " " }),
                                   positions: Int(dip.states), rest: Int(dip.rest), masks: masks,
                                   actuator: string(dip.actuator), ratedMV: dip.rated_mv)
        } else if let dip = part.dip_switch?.pointee, dip.momentary != 0 {
            details = .tactileSwitch(actuator: string(dip.actuator), ratedMV: dip.rated_mv,
                                     normallyClosed: dip.normally_closed != 0, joined: Int(dip.joined),
                                     body: dip.body_um.0 > 0 ? BodySize(lengthUM: dip.body_um.0, widthUM: dip.body_um.1) : nil)
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
                type: display.type.map { String(cString: $0) } ?? "led",
                segmentPins: indices(display.segment_pins, display.segment_pin_count),
                commonPins: indices(display.common_pins, display.common_pin_count)
            ))
        } else if let nixie = part.nixie?.pointee {
            details = .nixie(NixieInfo(
                glyphs: (0..<nixie.glyph_count).map { string(nixie.glyphs?[$0]) },
                cathodePins: indices(nixie.cathode_pins, nixie.glyph_count),
                anodePins: indices(nixie.anode_pins, nixie.anode_pin_count),
                color: string(nixie.color),
                size: string(nixie.size),
                view: string(nixie.view),
                strikeMV: nixie.strike_mv,
                sustainMV: nixie.sustain_mv,
                currentMinUA: nixie.current_min_ua,
                currentMaxUA: nixie.current_max_ua
            ))
        } else if let dekatron = part.dekatron?.pointee {
            let positionPins = withUnsafeBytes(of: dekatron.position_pin) { Array($0.bindMemory(to: Int.self)) }
            details = .dekatron(DekatronInfo(
                positionPins: positionPins,
                cathodePins: indices(dekatron.cathode_pins, dekatron.cathode_pin_count),
                anodePin: dekatron.anode_pin,
                guidePins: indices(dekatron.guide_pins, dekatron.guide_count),
                twoGuide: dekatron.two_guide != 0,
                color: string(dekatron.color),
                size: string(dekatron.size),
                view: string(dekatron.view),
                strikeMV: dekatron.strike_mv,
                sustainMV: dekatron.sustain_mv,
                currentMinUA: dekatron.current_min_ua,
                currentMaxUA: dekatron.current_max_ua,
                rateHz: dekatron.rate_hz
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
                panel: LCDPanel(string(lcd.panel)),
                backlightColors: (0..<Int(lcd.backlight_count)).compactMap { lcd.backlight_colors?[$0].map { String(cString: $0) } },
                busPins: indices(lcd.bus_pins, lcd.bus_pin_count),
                enables: Int(lcd.enables)
            ))
        } else if let serial = part.serial?.pointee {
            details = .serial(model: string(serial.model), channels: Int(serial.channels),
                              grade: string(serial.grade), host: string(serial.host))
        } else if let pio = part.pio?.pointee {
            details = .pio(model: string(pio.model), grade: string(pio.grade), maxHz: pio.max_hz)
        } else if let header = part.header?.pointee {
            let rows = max(Int(header.rows), 1)
            details = .header(rows: rows, perRow: Int(part.pin_count) / rows,
                              genders: (0..<header.gender_count).map { string(header.genders?[$0]) })
        } else if let connector = part.connector?.pointee, connector.test_point != 0 {
            details = .testPoint
        } else if let connector = part.connector?.pointee, let card = connector.card {
            details = .cardSlot(card: string(card), pitchUM: connector.card_pitch_um)
        } else if let connector = part.connector?.pointee, connector.terminal != 0 || connector.jst != 0 {
            details = .wireConnector(kind: connector.terminal != 0 ? "terminal" : "jst",
                                     variant: connector.terminal != 0
                                         ? PartFacts.clampWord(connector.terminal) : PartFacts.jstSeries(connector.jst),
                                     pitchUM: connector.pitch_um, ratedMV: connector.rated_mv,
                                     ratedMA: connector.rated_ma)
        } else if let connector = part.connector?.pointee {
            details = .connector(centerPinUM: connector.center_diameter_um, ratedMV: connector.rated_mv,
                                 ratedMA: connector.rated_ma, contacts: Int(connector.contacts),
                                 usb: connector.usb == 0 ? nil
                                     : String(UnicodeScalar(UInt8(bitPattern: connector.usb))))
        } else if let converter = part.converter?.pointee {
            details = .converter(type: string(converter.type), bits: Int(converter.bits),
                                 channels: Int(converter.channels))
        } else if part.kind == PART_RTC {
            details = .rtc
        } else if part.kind == PART_LEDDRIVER || part.kind == PART_LCDDRIVER {
            details = .displayDriver
        } else if let memory = part.serialmem?.pointee {
            details = .serialmem(SerialMemoryInfo(
                type: string(memory.type), size: memory.size, addressBytes: Int(memory.address_bytes),
                pageBytes: memory.page_bytes, writePS: memory.write_ps,
                sectorErasePS: memory.sector_erase_ps, chipErasePS: memory.chip_erase_ps,
                jedecID: memory.jedec_id.map { String(cString: $0) },
                maxClockKHz: memory.max_clock_khz, endurance: memory.endurance,
                retentionYears: memory.retention_years))
        } else if let diode = part.diode?.pointee {
            details = .diode(type: string(diode.type), diodes: Int(diode.diode_count),
                             vrMV: diode.vr_mv, ioMA: diode.io_ma, vzMV: diode.vz_mv, pdMW: diode.pd_mw)
        } else if let capacitor = part.capacitor?.pointee {
            details = .capacitor(type: string(capacitor.type), polarized: capacitor.polarized != 0,
                                 farads: Double(capacitor.capacitance_ff) / 1e15,
                                 ratedMV: capacitor.rated_mv,
                                 dielectric: capacitor.dielectric.map { String(cString: $0) },
                                 tolerance: capacitor.tolerance.map { String(cString: $0) })
        } else if let mcu = part.mcu?.pointee {
            details = .mcu(core: string(mcu.core), maxHz: mcu.max_hz, flash: mcu.flash, ram: mcu.ram,
                           eeprom: mcu.eeprom, psram: mcu.psram)
        } else if let pld = part.pld?.pointee {
            details = .pld(macrocells: Int(pld.macrocells), tpdPS: pld.tpd_ps)
        } else if let resistor = part.resistor?.pointee {
            details = .resistor(ohms: Double(resistor.milliohms) / 1000,
                                tolerancePercent: resistor.tolerance_ppm == 0 ? nil
                                    : Double(resistor.tolerance_ppm) / 10000,
                                powerW: resistor.power_mw == 0 ? nil : Double(resistor.power_mw) / 1000,
                                resistors: Int(resistor.resistor_count))
        } else if let transistor = part.transistor?.pointee {
            details = .transistor(TransistorInfo(
                type: string(transistor.type), elements: Int(transistor.element_count),
                mixed: transistor.mixed != 0, darlington: transistor.darlington != 0,
                mosfet: transistor.mosfet != 0, logicLevel: transistor.logic_level != 0,
                vmaxMV: transistor.vmax_mv, imaxMA: transistor.imax_ma, powerMW: transistor.power_mw,
                hfe: transistor.hfe, vgsthMV: transistor.vgsth_mv, vgsMV: transistor.vgs_mv,
                r1Milliohm: transistor.r1_mohm, r2Milliohm: transistor.r2_mohm,
                rdsonMilliohm: transistor.rdson_mohm))
        } else if let regulator = part.regulator?.pointee {
            let count = Int(regulator.regulator_count)
            func values(_ pointer: UnsafePointer<UInt32>?) -> [UInt32] {
                pointer.map { Array(UnsafeBufferPointer(start: $0, count: count)) } ?? []
            }
            details = .regulator(RegulatorInfo(
                type: string(regulator.type), regulators: count, mixed: regulator.mixed != 0,
                adjustable: regulator.adjustable != 0, voutMV: values(regulator.vout_mv),
                vrefMV: values(regulator.vref_mv), voutMaxMV: regulator.vout_max_mv,
                vinMinMV: regulator.vin_min_mv, vinMaxMV: regulator.vin_max_mv,
                dropoutMV: regulator.dropout_mv, ioutMA: regulator.iout_ma,
                iqUA: regulator.iq_ua, fswHz: regulator.fsw_hz))
        } else if let powerswitch = part.powerswitch?.pointee {
            details = .powerswitch(PowerSwitchInfo(
                type: string(powerswitch.type), switches: Int(powerswitch.switch_count),
                vinMinMV: powerswitch.vin_min_mv, vinMaxMV: powerswitch.vin_max_mv,
                ioutMA: powerswitch.iout_ma, ilimitMA: powerswitch.ilimit_ma,
                ronMOhm: powerswitch.ron_mohm, iqUA: powerswitch.iq_ua))
        } else if let crystal = part.crystal?.pointee {
            details = .crystal(CrystalInfo(
                type: string(crystal.type), hz: crystal.hz, loadFF: crystal.load_ff,
                tolerancePPM: crystal.tolerance_ppm, stabilityPPM: crystal.stability_ppm,
                esrMOhm: crystal.esr_mohm, capacitors: crystal.capacitors != 0))
        } else if let socket = part.socket?.pointee {
            details = .socket(family: string(socket.family), rowUM: socket.row_um,
                              mounting: string(socket.mounting), finish: socket.finish.map { String(cString: $0) })
        } else if let inductor = part.inductor?.pointee {
            details = .inductor(InductorInfo(
                pH: inductor.ph, tolerance: inductor.tolerance.map { String(cString: $0) },
                ratedMA: inductor.rated_ma, isatMA: inductor.isat_ma, dcrMicroOhm: inductor.dcr_uohm,
                shielding: inductor.shielding.map { String(cString: $0) }))
        } else if let fuse = part.fuse?.pointee {
            details = .fuse(FuseInfo(
                type: string(fuse.type), ratedMA: fuse.rated_ma, tripMA: fuse.trip_ma,
                acMV: fuse.ac_mv, dcMV: fuse.dc_mv, speed: fuse.speed.map { String(cString: $0) },
                breakingA: fuse.breaking_a))
        } else if let relay = part.relay?.pointee {
            details = .relay(RelayInfo(
                type: string(relay.type), poles: Int(relay.poles), coils: Int(relay.coils),
                latching: relay.latching != 0, channels: Int(relay.channels), coilMV: relay.coil_mv,
                coilMOhm: relay.coil_mohm, contactMA: relay.contact_ma, contactMV: relay.contact_mv,
                operateUS: relay.operate_us, releaseUS: relay.release_us, onMOhm: relay.on_mohm))
        } else if let buzzer = part.buzzer?.pointee {
            details = .buzzer(BuzzerInfo(
                driver: string(buzzer.driver), technology: string(buzzer.technology), hz: buzzer.hz,
                ratedMV: buzzer.rated_mv, minMV: buzzer.min_mv, maxMV: buzzer.max_mv, uA: buzzer.ua,
                splCDB: buzzer.spl_cdb))
        } else if let timer = part.timer?.pointee {
            details = .timer(TimerInfo(timers: Int(timer.timer_count), process: string(timer.process),
                                       maxHz: timer.max_hz))
        } else if let opamp = part.opamp?.pointee {
            details = .opamp(OpampInfo(
                type: string(opamp.type), amplifiers: Int(opamp.amplifier_count), railIn: opamp.rail_in != 0,
                railOut: opamp.rail_out != 0, gbwHz: opamp.gbw_hz, slewMvUs: opamp.slew_mv_us, vosUv: opamp.vos_uv))
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

    /// The footprint file.
    static let footprintsFile = "footprints.json"
    /// The datasheet file; every other bundled JSON resource except
    /// `boards.json` is a part file.
    static let datasheetsFile = "datasheets.json"

    /// Loads `datasheets.json` and `footprints.json` first (part files name
    /// their entries), then every other bundled JSON resource as a part
    /// file, in file-name order.
    static func load(into library: PartLibraryHandle) throws -> [CatalogPart] {
        let urls = (Bundle.main.urls(forResourcesWithExtension: "json", subdirectory: nil) ?? [])
            .sorted { $0.lastPathComponent < $1.lastPathComponent }
        guard let footprintsURL = urls.first(where: { $0.lastPathComponent == footprintsFile }) else {
            throw LoadError.unavailable
        }
        guard let datasheetsURL = urls.first(where: { $0.lastPathComponent == datasheetsFile }) else {
            throw LoadError.unavailable
        }
        try loadFile(datasheetsURL, with: load_datasheets, into: library)
        try loadFile(footprintsURL, with: load_footprints, into: library)
        // The board standards (`boards.json`) are read by `BoardCatalog`.
        for url in urls where url != footprintsURL && url != datasheetsURL
            && url.lastPathComponent != BoardCatalog.fileName {
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
                catalogPart(part.pointee) { footprints[string($0.pointee.id)] }
            }
        }
    }

    /// Loads only `datasheets.json` and `footprints.json`: what a single part
    /// file's references resolve against (the Part Editor's validator).
    static func loadSupportFiles(into library: PartLibraryHandle) throws {
        guard let datasheets = Bundle.main.url(forResource: "datasheets", withExtension: "json"),
              let footprints = Bundle.main.url(forResource: "footprints", withExtension: "json") else {
            throw LoadError.unavailable
        }
        try loadFile(datasheets, with: load_datasheets, into: library)
        try loadFile(footprints, with: load_footprints, into: library)
    }

    /// Copies one backend part, its footprints looked up by `footprint`.
    static func catalogPart(_ part: Part,
                            footprint: (UnsafePointer<PartFootprint>) -> Footprint?) -> CatalogPart {
        CatalogPart(part,
                    footprint: part.footprint.flatMap(footprint),
                    connectorFootprint: part.connector_footprint.flatMap(footprint),
                    footprintChoices: (0..<part.footprint_choice_count).compactMap { k in
                        guard let choice = part.footprint_choices?[k],
                              let copy = choice.footprint.flatMap(footprint)
                        else { return nil }
                        return CatalogPart.FootprintChoice(package: string(choice.package), footprint: copy)
                    },
                    fits: (0..<part.fit_count).compactMap { k in
                        part.fits?[k].flatMap(footprint)
                    })
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

// MARK: - Custom parts (plan-32-A)

/// Why a single part file (`.schpart`) could not be added to the catalog.
nonisolated enum PartRegistrationError: LocalizedError, Equatable {
    /// The backend's loader refused the file; its message names the place.
    case backend(String)
    /// The file is not a part file at all (not UTF-8 text, or not a JSON object).
    case notAPartFile(String)
    /// The part is not in the User Library (it was removed, or never added).
    case notInUserLibrary(name: String)
    /// An in-place update that would change the part's uuid or kind.
    case identityChanged(detail: String)
    /// The User Library's file could not be written (plan-53-B).
    case libraryWrite(String)
    /// There is no User Library to add to (a catalog made without one).
    case noUserLibrary

    var errorDescription: String? {
        switch self {
        case .backend(let message):
            return message
        case .notAPartFile(let detail):
            return "This is not a part file (\(detail))."
        case .notInUserLibrary(let name):
            return "“\(name)” is no longer in your User Library. Use Save As… to keep it."
        case .identityChanged(let detail):
            return detail
        case .libraryWrite(let reason):
            return "The part could not be saved in your User Library: \(reason)"
        case .noUserLibrary:
            return "There is no User Library to add the part to."
        }
    }
}

/// Where a registered custom part is listed in the drawer (plan-53-B):
/// `user` in the User Library (kept on disk), `file` in the File Library
/// (this session only), `removed` not at all (it was taken out of the User
/// Library; documents that hold it still resolve it).
nonisolated enum PartListing: Equatable, Sendable {
    case user, file, removed

    /// The drawer's top-level category for a listed part.
    var category: String? {
        switch self {
        case .user: PartCategory.userLibrary
        case .file: PartCategory.fileLibrary
        case .removed: nil
        }
    }
}

nonisolated enum PartCategory {
    static let userLibrary = "User Library"
    static let fileLibrary = "File Library"

    /// The drawer's category order: the built-in categories by name, then the
    /// User Library, then the File Library, so a person's own parts sit at the
    /// bottom of the drawer.
    static func precedes(_ lhs: String, _ rhs: String) -> Bool {
        func tier(_ name: String) -> Int {
            name == userLibrary ? 1 : name == fileLibrary ? 2 : 0
        }
        let (a, b) = (tier(lhs), tier(rhs))
        if a != b { return a < b }
        return lhs.localizedStandardCompare(rhs) == .orderedAscending
    }
}

/// Reads a `.schpart` file's text for `PartCatalogStore.register`.
nonisolated enum PartFileReader {
    static func read(url: URL) throws -> String {
        let data: Data
        do {
            data = try Data(contentsOf: url)
        } catch {
            throw PartRegistrationError.notAPartFile("\(url.lastPathComponent): \(error.localizedDescription)")
        }
        guard let text = String(data: data, encoding: .utf8) else {
            throw PartRegistrationError.notAPartFile("\(url.lastPathComponent) is not UTF-8 text")
        }
        return text
    }
}

/// A single part file as a JSON object, for the store's clash rules.
nonisolated struct PartFileObject {
    var object: [String: Any]

    init(text: String) throws {
        guard let data = text.data(using: .utf8),
              let object = (try? JSONSerialization.jsonObject(with: data)) as? [String: Any] else {
            throw PartRegistrationError.notAPartFile("not a JSON object")
        }
        self.object = object
    }

    var uuid: String? { object["uuid"] as? String }

    /// The one part's name (`devices[0].parts[0].name`).
    var name: String? {
        get {
            let device = (object["devices"] as? [Any])?.first as? [String: Any]
            let part = (device?["parts"] as? [Any])?.first as? [String: Any]
            return part?["name"] as? String
        }
        set {
            guard var devices = object["devices"] as? [Any], var device = devices.first as? [String: Any],
                  var parts = device["parts"] as? [Any], var part = parts.first as? [String: Any]
            else { return }
            part["name"] = newValue
            parts[0] = part
            device["parts"] = parts
            devices[0] = device
            object["devices"] = devices
        }
    }

    /// Gives the part a new identity (plan-53-A § 3): a new `uuid`; `originId`
    /// is the file's own `originId` if it has one, else the uuid it had (the
    /// first version's uuid is kept down a chain of versions); and every
    /// string value equal to the old `CUSTOM-<first 8 of uuid>` (the inline
    /// footprint and datasheet ids and what names them) becomes the new
    /// uuid's. Keys are never renamed.
    mutating func freshenIdentity(newUUID: String = UUID().uuidString) {
        guard let old = uuid else { return }
        if object["originId"] == nil { object["originId"] = old }
        object["uuid"] = newUUID
        let oldID = "CUSTOM-" + old.prefix(8).uppercased()
        let newID = "CUSTOM-" + newUUID.prefix(8).uppercased()
        func renamed(_ value: Any) -> Any {
            if let text = value as? String { return text == oldID ? newID : text }
            if let dictionary = value as? [String: Any] { return dictionary.mapValues(renamed) }
            if let array = value as? [Any] { return array.map(renamed) }
            return value
        }
        object = object.mapValues(renamed)
    }

    /// Sorted keys, so equal content is equal text.
    var canonicalText: String {
        guard let data = try? JSONSerialization.data(
            withJSONObject: object, options: [.sortedKeys, .withoutEscapingSlashes]) else { return "" }
        return String(decoding: data, as: UTF8.self)
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

    func allows(_ part: CatalogPart) -> Bool {
        (showObsolete || !part.flags.obsolete)
            && (showMilitary || !part.flags.military)
            && (showAutomotive || !part.flags.automotive)
    }
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
        /// The group's name: the parts' function, or a tactile switch size
        /// ("6 × 6 mm"; `CatalogPart.drawerLeaf`).
        let function: String
        let parts: [CatalogPart]
        /// For ADCs and DACs, `parts` split by resolution, fewest bits first;
        /// empty for other parts.
        var bitGroups: [BitGroup] = []

        var id: String { function }
    }

    /// A converter function group's parts with one resolution.
    struct BitGroup: Identifiable, Hashable {
        let bits: Int
        let parts: [CatalogPart]

        var id: Int { bits }
        var title: String { "\(bits)-bit" }
    }

    /// A drawer group below a category: its second level (Memory → SRAM)
    /// or third (Discrete → Resistors → Chip).
    struct SubcategoryGroup: Identifiable, Hashable {
        let subcategory: String
        /// The drawer path down to and including this group, e.g.
        /// ["Discrete", "Resistors"].
        let path: [String]
        /// Parts with a drawer level below this one, grouped under it
        /// (Discrete → Resistors → Chip). Empty at the third level.
        var subgroups: [SubcategoryGroup] = []
        let functions: [FunctionGroup]
        /// Parts listed straight under the subcategory, with no function level:
        /// switches (the function is only the vendor's series), LEDs,
        /// 7-segment displays and character LCDs.
        var parts: [CatalogPart] = []

        var id: String { path.joined(separator: "/") }

        /// Every part in the group and the groups below it.
        var allParts: [CatalogPart] {
            parts + functions.flatMap(\.parts) + subgroups.flatMap(\.allParts)
        }
    }

    struct CategoryGroup: Identifiable, Hashable {
        let category: String
        /// Parts with a second drawer level, grouped under it (Memory → SRAM → ...).
        let subcategories: [SubcategoryGroup]
        /// Parts without one, listed directly under the category.
        let functions: [FunctionGroup]

        var id: String { category }

        /// Every part in the category.
        var allParts: [CatalogPart] {
            functions.flatMap(\.parts) + subcategories.flatMap(\.allParts)
        }
    }

    /// The app's one catalog, shared by every window. The part files are
    /// read the first time it's used, at launch.
    static let shared: PartCatalogStore = {
        let store = PartCatalogStore()
        store.load()
        // The user's parts (plan-53-B); only this catalog reads the folder.
        store.attachUserLibrary(UserPartLibrary(directory: UserPartLibrary.defaultDirectory))
        return store
    }()

    /// The C library; circuits are built from it.
    let library = PartLibraryHandle()
    private(set) var loadState: LoadState = .notLoaded

    /// Listed parts (a non-empty drawer path).
    private var entries: [CatalogPart] = []
    private var partsByName: [String: CatalogPart] = [:]
    /// The sockets by the id of each chip footprint they hold (plan-51-H),
    /// built the first time `sockets(fitting:)` asks and dropped whenever
    /// `partsByName` changes.
    @ObservationIgnored private var socketIndex: [String: [CatalogPart]]?
    /// Registered custom parts (plan-32-A), by uuid, and the text each was
    /// registered with.
    private var partsByUUID: [String: CatalogPart] = [:]
    private var registeredTexts: [String: String] = [:]
    /// Where each registered custom part is listed (plan-53-B), by uuid.
    private var listings: [String: PartListing] = [:]
    /// A part file's text that clashed with another part's uuid, and the new
    /// uuid it was registered under (plan-53-A): the same text again is that
    /// part.
    private var freshenedTexts: [String: String] = [:]
    /// The User Library's folder, once attached (`attachUserLibrary`); nil for
    /// a catalog made without one (tests).
    private(set) var userLibrary: UserPartLibrary?
    /// Library files that could not be loaded, for the library window.
    private(set) var userLibraryProblems: [UserPartLibraryProblem] = []
    /// When each User Library part was added (its file's creation date).
    private var userLibraryAdded: [String: Date] = [:]

    /// What the drawer's headers and menus show, built from `entries` in one
    /// pass the first time it is asked for (plan-43 follow-up: each header
    /// used to scan every part on every redraw). `entries` changes only in
    /// `load` and `register`, which drop it.
    @ObservationIgnored private var derivedCache: Derived? {
        didSet { if derivedCache == nil { derivedRevision += 1 } }
    }

    /// Counts the times `entries` changed in a way the drawer must rebuild for,
    /// including a part moving between categories or being replaced, which
    /// leave `partCount` as it was (plan-53: the drawer kept listing an added
    /// part under File Library).
    private(set) var derivedRevision = 0

    private var derived: Derived {
        if let derivedCache { return derivedCache }
        let built = Derived(entries)
        derivedCache = built
        return built
    }

    /// Facts about the listed parts, by drawer group, for the drawer's
    /// header menus, plus what the search and the sort need.
    ///
    /// `nonisolated`: it is plain data, and a main-actor class has an isolated
    /// deinit, which aborts the test host when `register` drops the cache
    /// inside a test ("pointer being freed was not allocated" in
    /// `swift_task_deinitOnExecutor`, macOS 15.8; `TestSupport.newDocument()`
    /// notes the same for documents).
    nonisolated private final class Derived {
        var kindsByGroup: [String: Set<UInt32>] = [:]
        var tactileGroups: Set<String> = []
        var familiesByCategory: [String: Set<String>] = [:]
        var logicCountsByCategory: [String: Set<Int>] = [:]
        var switchPositionsByCategory: [String: Set<Int>] = [:]
        var colorsByCategory: [String: Set<String>] = [:]
        var converterChannelsByType: [String: Set<Int>] = [:]
        var displayDigits: Set<Int> = []
        var displaySizes: Set<String> = []
        var tactileActuators: Set<String> = []
        var packages: Set<String> = []
        /// Every part's searchable text (name, function, group, description
        /// and drawer path), folded for case and accents, as UTF-8, each
        /// ending in a zero byte; `searchStarts[i]` is where entry `i` begins.
        let searchBytes: [UInt8]
        let searchStarts: [Int]
        /// Each part name's place in natural order ("8 KB" before "32 KB").
        let nameRank: [String: Int]
        /// Which entries' text holds `needle` (already folded, not empty).
        func matches(_ needle: String) -> [Bool] {
            var hit = [Bool](repeating: false, count: searchStarts.count)
            let wanted = Array(needle.utf8)
            searchBytes.withUnsafeBufferPointer { text in
                wanted.withUnsafeBufferPointer { pattern in
                    guard let base = text.baseAddress, let find = pattern.baseAddress else { return }
                    var offset = 0
                    while offset < text.count,
                          let found = memmem(base + offset, text.count - offset, find, pattern.count) {
                        let position = base.distance(to: found.assumingMemoryBound(to: UInt8.self))
                        // The entry holding this byte: the last start at or before it.
                        var low = 0, high = searchStarts.count - 1
                        while low < high {
                            let mid = (low + high + 1) / 2
                            if searchStarts[mid] <= position { low = mid } else { high = mid - 1 }
                        }
                        hit[low] = true
                        // The next entry's text may hold it too.
                        offset = low + 1 < searchStarts.count ? searchStarts[low + 1] : text.count
                    }
                }
            }
            return hit
        }

        /// Indices into `entries`, in natural name order: filtering in this
        /// order leaves every group sorted with no sort of its own.
        let nameOrder: [Int]
        /// Each part's flags in `entries` order: 1 obsolete, 2 military, 4 automotive.
        let flagBits: [UInt8]

        init(_ entries: [CatalogPart]) {
            struct Path { var drawer: [String]; var kinds: Set<UInt32> = []; var tactile = false }
            var paths: [String: Path] = [:]
            for part in entries {
                let category = part.drawer.first ?? ""
                let key = part.drawer.joined(separator: "/")
                paths[key, default: Path(drawer: part.drawer)].kinds.insert(part.kind.rawValue)
                if part.isTactileSwitch { paths[key]?.tactile = true }
                if part.isLogic {
                    if let family = part.logicFamily { familiesByCategory[category, default: []].insert(family) }
                    if let count = part.count { logicCountsByCategory[category, default: []].insert(count) }
                }
                if part.kind == PART_SWITCH, !part.isTactileSwitch, !part.isSlideSwitch, let count = part.count {
                    switchPositionsByCategory[category, default: []].insert(count)
                }
                if !part.colors.isEmpty { colorsByCategory[category, default: []].formUnion(part.colors) }
                if case let .converter(type, _, channels) = part.details {
                    converterChannelsByType[type, default: []].insert(channels)
                }
                if let display = part.display {
                    displayDigits.insert(display.digits)
                    displaySizes.insert(display.size)
                }
                if let actuator = part.tactileActuator { tactileActuators.insert(actuator) }
                packages.formUnion(part.allPackages)
            }
            // A group lists the kinds of every path at or below it.
            for path in paths.values {
                for level in 1...max(path.drawer.count, 1) where !path.drawer.isEmpty {
                    let key = path.drawer.prefix(level).joined(separator: "/")
                    kindsByGroup[key, default: []].formUnion(path.kinds)
                    if path.tactile { tactileGroups.insert(key) }
                }
            }
            let options: String.CompareOptions = [.caseInsensitive, .diacriticInsensitive, .widthInsensitive]
            var bytes: [UInt8] = []
            var starts: [Int] = []
            starts.reserveCapacity(entries.count)
            for part in entries {
                starts.append(bytes.count)
                bytes.append(contentsOf: ([part.name, part.function, part.drawerLeaf, part.description] + part.drawer)
                    .joined(separator: "\n")
                    .folding(options: options, locale: nil).utf8)
                bytes.append(0)
            }
            searchBytes = bytes
            searchStarts = starts
            let order = entries.indices.sorted {
                let c = entries[$0].name.localizedStandardCompare(entries[$1].name)
                return c == .orderedSame ? $0 < $1 : c == .orderedAscending
            }
            nameOrder = order
            flagBits = entries.map { part in
                (part.flags.obsolete ? 1 : 0) | (part.flags.military ? 2 : 0) | (part.flags.automotive ? 4 : 0)
            }
            var rank: [String: Int] = [:]
            rank.reserveCapacity(order.count)
            for (place, index) in order.enumerated() where rank[entries[index].name] == nil {
                rank[entries[index].name] = place
            }
            nameRank = rank
        }
    }

    var partCount: Int { entries.count }

    /// Every listed part (automation: search).
    var allParts: [CatalogPart] { entries }

    var categories: [String] {
        Array(Set(derived.kindsByGroup.keys.filter { !$0.contains("/") })).sorted(by: PartCategory.precedes)
    }

    var packages: [String] {
        sortedNames(Array(derived.packages))
    }

    /// The kinds of part listed under a top-level drawer category.
    func kinds(inCategory category: String) -> Set<UInt32> {
        derived.kindsByGroup[category] ?? []
    }

    /// The kinds of part listed under a drawer group: `path` is its drawer
    /// path, e.g. ["Indicators", "LED 7-Seg"].
    func kinds(inGroup path: [String]) -> Set<UInt32> {
        derived.kindsByGroup[path.joined(separator: "/")] ?? []
    }

    /// Logic families of the gates and flip-flops in a category.
    func families(in category: String) -> [String] {
        sortedNames(Array(derived.familiesByCategory[category] ?? []))
    }

    /// Gates (or flip-flops) per package in a category, ascending.
    func logicCounts(in category: String) -> [Int] {
        (derived.logicCountsByCategory[category] ?? []).sorted()
    }

    /// Positions per switch in a category, ascending.
    func switchPositionCounts(in category: String) -> [Int] {
        (derived.switchPositionsByCategory[category] ?? []).sorted()
    }

    /// The converter type ("adc", "dac") of a function group's parts, or nil
    /// when they are not converters.
    func converterType(of parts: [CatalogPart]) -> String? {
        guard case let .converter(type, _, _)? = parts.first?.details else { return nil }
        return type
    }

    /// Channel counts of one converter type, ascending.
    func converterChannels(type: String) -> [Int] {
        (derived.converterChannelsByType[type] ?? []).sorted()
    }

    /// LED and display colors in a category.
    func colors(in category: String) -> [String] {
        sortedNames(Array(derived.colorsByCategory[category] ?? []))
    }

    /// Digit counts and digit heights of the 7-segment displays.
    var displayDigitCounts: [Int] {
        derived.displayDigits.sorted()
    }

    var displaySizes: [String] {
        sortedNames(Array(derived.displaySizes))
    }

    /// Whether a drawer group (`path`, as for `kinds(inGroup:)`) lists
    /// tactile switches.
    func isTactile(group path: [String]) -> Bool {
        derived.tactileGroups.contains(path.joined(separator: "/"))
    }

    /// The tactile switches' actuators ("Round Button", "Standard", ...).
    var tactileActuators: [String] {
        sortedNames(Array(derived.tactileActuators))
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

    /// Any part by name, listed or not (VCC and GND are not listed). A rail
    /// of any voltage ("170V", "3V6") and its pull-up ("PULLUP-170V") exist
    /// as soon as they are asked for: see `customSupply`.
    /// The sockets that hold a chip footprint (plan-51-H): the `PART_SOCKET`
    /// parts whose `fits` name it, the generic one first, then by
    /// manufacturer, then by part number in natural order. Bundled sockets
    /// only (see the index).
    func sockets(fitting footprint: Footprint) -> [CatalogPart] {
        if socketIndex == nil {
            var index: [String: [CatalogPart]] = [:]
            // Bundled sockets only: a chip's file names its socket, and a file carries no
            // custom part but a placed chip's own (`customParts`).
            for part in partsByName.values where part.kind == PART_SOCKET && part.uuid == nil {
                for fit in part.fits { index[fit.id, default: []].append(part) }
            }
            socketIndex = index.mapValues { list in
                list.sorted { a, b in
                    let (ga, gb) = (a.manufacturer == "Generic", b.manufacturer == "Generic")
                    if ga != gb { return ga }
                    if a.manufacturer != b.manufacturer {
                        return a.manufacturer.localizedStandardCompare(b.manufacturer) == .orderedAscending
                    }
                    return a.name.localizedStandardCompare(b.name) == .orderedAscending
                }
            }
        }
        return socketIndex?[footprint.id] ?? []
    }

    /// The generic socket for a chip footprint: the first `Generic` entry of
    /// `sockets(fitting:)`; nil when there is none.
    func genericSocket(fitting footprint: Footprint) -> CatalogPart? {
        sockets(fitting: footprint).first { $0.manufacturer == "Generic" }
    }

    func part(named name: String) -> CatalogPart? {
        partsByName[name] ?? customSupplies[name] ?? customSupply(named: name)
    }

    /// The rails made on demand (the VCC tool's Custom… voltage, and a file
    /// that uses one), by name. Not observed: a lookup may add to it while a
    /// view draws, and no view lists these parts.
    @ObservationIgnored private var customSupplies: [String: CatalogPart] = [:]

    /// Loads the supply and the pull-up of the voltage `name` says, when it
    /// is a rail name the library does not have, and returns the one asked
    /// for. Both go into the C library, so circuits find them too.
    private func customSupply(named name: String) -> CatalogPart? {
        guard loadState == .loaded else { return nil }
        let pullPrefix = "PULLUP-"
        let rail = name.hasPrefix(pullPrefix) ? String(name.dropFirst(pullPrefix.count)) : name
        guard let mv = SupplyChoice.millivolts(name: rail) else { return nil }
        // A bundled rail's pull-up is bundled too; never load a name twice.
        let wanted = [rail, pullPrefix + rail].filter { partsByName[$0] == nil && customSupplies[$0] == nil }
        guard wanted.contains(name) else { return nil }
        let volts = String(format: "%g", Double(mv) / 1000)
        let devices = wanted.map { part -> String in
            part == rail
                ? """
                {"id":"\(part)","function":"\(part)","description":"Supply, \(volts) V, drives logic 1","drawer":[],
                 "pinouts":{"pin":[{"number":1,"name":"VCC","role":"power"}]},"volts":\(volts),
                 "parts":[{"name":"\(part)","manufacturer":"Generic","package":"","pinout":"pin"}]}
                """
                : """
                {"id":"\(part)","function":"Pull-up","description":"Pull-up to \(volts) V, drives logic 1 weakly","drawer":[],
                 "pinouts":{"pin":[{"number":1,"name":"PU","role":"output"}]},"volts":\(volts),"pull":true,
                 "parts":[{"name":"\(part)","manufacturer":"Generic","package":"Radial","pinout":"pin",
                  "footprint":"RES-RADIAL-2.54","footprintChoices":[
                   {"package":"0402","footprint":"RES-0402"},{"package":"0603","footprint":"RES-0603"},
                   {"package":"0805","footprint":"RES-0805"},{"package":"1206","footprint":"RES-1206"},
                   {"package":"Axial","footprint":"RES-AXIAL-10.16"},{"package":"Radial","footprint":"RES-RADIAL-2.54"}]}]}
                """
        }
        let text = """
            {"format":"schematical-parts","version":1,"kind":"supply","devices":[\(devices.joined(separator: ","))]}
            """
        var error: UnsafeMutablePointer<CChar>?
        if load_parts(library.pointer, text, "custom-supply-\(rail).json", &error) != 0 {
            free(error)
            return nil
        }
        for part in wanted {
            guard let loaded = part_library_find(library.pointer, part) else { continue }
            customSupplies[part] = BackendCatalog.catalogPart(loaded.pointee) { Footprint($0.pointee) }
        }
        return customSupplies[name]
    }

    /// A registered custom part by its uuid.
    func part(uuid: String) -> CatalogPart? {
        partsByUUID[uuid]
    }

    /// The text a custom part was registered with (sorted keys, its name as
    /// registered, suffix included); nil for an unknown uuid.
    func registeredText(uuid: String) -> String? {
        registeredTexts[uuid]
    }

    /// Whether the file's uuid is registered with other content (the
    /// comparison `register` makes, with the name as registered).
    func uuidConflicts(_ fileText: String) -> Bool {
        guard var file = try? PartFileObject(text: fileText), let uuid = file.uuid,
              let registered = partsByUUID[uuid] else { return false }
        file.name = registered.name
        return file.canonicalText != registeredTexts[uuid]
    }

    /// Adds a single part file's part to the catalog for the rest of the
    /// session (plan-32-A § 4.3). The same uuid with the same content returns
    /// the registered part; with other content the file is registered under a
    /// new uuid (plan-53-A: `PartFileObject.freshenIdentity`), so no two
    /// different parts share one. A name that is taken gets the lowest free
    /// suffix `.2`, `.3`, …. The backend checks the file; a failure changes
    /// nothing. The part is listed as `listing` says (the File Library by
    /// default, plan-53-B); a part that is already registered and listed
    /// `removed` is listed again (as `listing`, never `removed`), and one
    /// listed `file` is moved to the User Library by `listing: .user`.
    @discardableResult
    func register(fileText: String, source: String, listing: PartListing = .file) throws -> CatalogPart {
        var file = try PartFileObject(text: fileText)
        var clashingText: String?
        if let uuid = file.uuid, let registered = partsByUUID[uuid] {
            // The registered text carries the name as registered (maybe
            // suffixed); compare with the new file under that name.
            var compared = file
            compared.name = registered.name
            if compared.canonicalText == registeredTexts[uuid] {
                return relisted(uuid, as: listing) ?? registered
            }
            // The same clashing text again (a schematic opened twice) is the
            // part it became the first time, not another copy.
            if let fresh = freshenedTexts[file.canonicalText], partsByUUID[fresh] != nil {
                return relisted(fresh, as: listing) ?? registered
            }
            clashingText = file.canonicalText
            file.freshenIdentity()
        }
        if let name = file.name, partsByName[name] != nil {
            var n = 2
            while partsByName["\(name).\(n)"] != nil {
                n += 1
            }
            file.name = "\(name).\(n)"
        }
        let text = file.canonicalText
        var error: UnsafeMutablePointer<CChar>?
        if load_part_file(library.pointer, text, source, &error) != 0 {
            let message = error.map { String(cString: $0) } ?? source
            free(error)
            throw PartRegistrationError.backend(message)
        }
        guard let uuid = file.uuid, let loaded = part_library_find_uuid(library.pointer, uuid) else {
            throw PartRegistrationError.backend("\(source): the part did not load")
        }
        let backendPart = BackendCatalog.catalogPart(loaded.pointee) { Footprint($0.pointee) }
        let part = backendPart.listed(in: listing.category ?? PartCategory.fileLibrary)
        partsByName[part.name] = part
        socketIndex = nil
        partsByUUID[uuid] = part
        registeredTexts[uuid] = text
        listings[uuid] = listing
        if let clashingText { freshenedTexts[clashingText] = uuid }
        if !part.drawer.isEmpty {
            entries.append(part)
            derivedCache = nil
        }
        return part
    }

    /// A part that is already registered, found again by `register`: listed
    /// again if it had been removed, and moved to the User Library when that
    /// is what is asked for. Returns the registered part.
    private func relisted(_ uuid: String, as listing: PartListing) -> CatalogPart? {
        let current = listings[uuid]
        let wanted: PartListing = listing == .removed ? .file : listing
        if current == .removed || (wanted == .user && current == .file) {
            setListing(wanted, for: uuid)
        }
        return partsByUUID[uuid]
    }

    /// A part of the User Library with its file, for the library window.
    struct UserLibraryItem: Identifiable {
        let part: CatalogPart
        let added: Date
        let url: URL

        var id: String { part.uuid ?? part.name }
    }

    /// The User Library's parts, in natural name order.
    var userLibraryItems: [UserLibraryItem] {
        guard let userLibrary else { return [] }
        return listings.compactMap { uuid, listing -> UserLibraryItem? in
            guard listing == .user, let part = partsByUUID[uuid] else { return nil }
            return UserLibraryItem(part: part, added: userLibraryAdded[uuid] ?? Date(),
                                   url: userLibrary.url(forUUID: uuid))
        }
        .sorted { $0.part.name.localizedStandardCompare($1.part.name) == .orderedAscending }
    }

    /// Attaches the User Library's folder and registers every part in it,
    /// listed `user` (plan-53-B). A file the loader refuses, or whose uuid an
    /// earlier library file or an already registered part holds with other
    /// content, is recorded in `userLibraryProblems` and skipped.
    func attachUserLibrary(_ library: UserPartLibrary) {
        userLibrary = library
        let found = library.files()
        var problems = found.problems
        var claimed: [String: String] = [:]
        for file in found.files {
            if let earlier = claimed[file.uuid] {
                problems.append(UserPartLibraryProblem(
                    url: file.url, reason: "It has the same identity as \(earlier), which is already loaded."))
                continue
            }
            claimed[file.uuid] = file.url.lastPathComponent
            if partsByUUID[file.uuid] != nil, uuidConflicts(file.text) {
                problems.append(UserPartLibraryProblem(
                    url: file.url, reason: "Another part with this identity is already loaded."))
                continue
            }
            do {
                try register(fileText: file.text, source: file.url.lastPathComponent, listing: .user)
                userLibraryAdded[file.uuid] = file.added
            } catch {
                problems.append(UserPartLibraryProblem(url: file.url, reason: error.localizedDescription))
            }
        }
        userLibraryProblems = problems
    }

    /// Registers a part file and keeps it in the User Library: listed under
    /// User Library and written to the library folder (plan-53-B). A
    /// failed write leaves the part listed in the File Library and throws
    /// `libraryWrite`.
    @discardableResult
    func addToUserLibrary(fileText: String, source: String) throws -> CatalogPart {
        guard userLibrary != nil else { throw PartRegistrationError.noUserLibrary }
        let before = (try? PartFileObject(text: fileText))?.uuid.flatMap { listings[$0] }
        let part = try register(fileText: fileText, source: source, listing: .user)
        try persistToUserLibrary(part, previous: before ?? .file)
        return part
    }

    /// Keeps a part that is already registered (a File Library part) in the
    /// User Library.
    func addToUserLibrary(_ part: CatalogPart) throws {
        guard userLibrary != nil else { throw PartRegistrationError.noUserLibrary }
        guard let uuid = part.uuid, partsByUUID[uuid] != nil else {
            throw PartRegistrationError.notAPartFile("\(part.name) is not a custom part")
        }
        let previous = listings[uuid] ?? .file
        setListing(.user, for: uuid)
        try persistToUserLibrary(partsByUUID[uuid] ?? part, previous: previous)
    }

    private func persistToUserLibrary(_ part: CatalogPart, previous: PartListing) throws {
        guard let userLibrary, let uuid = part.uuid, let text = registeredTexts[uuid] else {
            throw PartRegistrationError.noUserLibrary
        }
        do {
            try userLibrary.write(uuid: uuid, text: text)
            userLibraryAdded[uuid] = userLibraryAdded[uuid] ?? Date()
        } catch {
            if previous != .user { setListing(previous == .removed ? .file : previous, for: uuid) }
            throw PartRegistrationError.libraryWrite(error.localizedDescription)
        }
    }

    /// Called after a User Library part is replaced in place (plan-53-D), with
    /// its uuid and the new part, so open schematics can follow. Set by
    /// `SchematicEditorModel` (to `SchematicNSDocument.updateOpenDocuments`).
    @ObservationIgnored var partUpdated: ((String, CatalogPart) -> Void)?

    /// Replaces a User Library part's content under the same uuid, now
    /// (plan-53-D): the library file, the backend's part (retired and loaded
    /// again) and the catalog's records. The kind and the uuid cannot change;
    /// a new name that another part has gets the usual suffix. On any failure
    /// the old version is still the part, and the old text is the file's.
    @discardableResult
    func updateUserLibraryPart(uuid: String, fileText: String) throws -> CatalogPart {
        guard let userLibrary else { throw PartRegistrationError.noUserLibrary }
        guard let old = partsByUUID[uuid], listings[uuid] == .user, let oldText = registeredTexts[uuid] else {
            throw PartRegistrationError.notInUserLibrary(name: partsByUUID[uuid]?.name ?? uuid)
        }
        var file = try PartFileObject(text: fileText)
        guard file.uuid == uuid else {
            throw PartRegistrationError.identityChanged(detail: "An update cannot change the part's identity.")
        }
        let oldFile = try PartFileObject(text: oldText)
        guard file.object["kind"] as? String == oldFile.object["kind"] as? String else {
            throw PartRegistrationError.identityChanged(detail: "An update cannot change the part's kind.")
        }
        // A name another part has is suffixed; the part's own old name is free.
        if let name = file.name, let other = partsByName[name], other.uuid != uuid {
            var n = 2
            while let taken = partsByName["\(name).\(n)"], taken.uuid != uuid { n += 1 }
            file.name = "\(name).\(n)"
        }
        let text = file.canonicalText
        if text == oldText { return old }

        func ids(_ key: String, of file: PartFileObject) -> [String] {
            (file.object[key] as? [[String: Any]] ?? []).compactMap { $0["id"] as? String }
        }
        do {
            try userLibrary.write(uuid: uuid, text: text)
        } catch {
            throw PartRegistrationError.libraryWrite(error.localizedDescription)
        }
        let source = "\(file.name ?? uuid).schpart"
        guard library.retire(uuid: uuid, footprintIDs: ids("footprints", of: oldFile),
                             datasheetIDs: ids("datasheets", of: oldFile)) else {
            try? userLibrary.write(uuid: uuid, text: oldText)
            throw PartRegistrationError.backend("\(source): the part could not be replaced")
        }
        var error: UnsafeMutablePointer<CChar>?
        if load_part_file(library.pointer, text, source, &error) != 0 {
            let message = error.map { String(cString: $0) } ?? source
            free(error)
            // Put the old version back.
            var restoreError: UnsafeMutablePointer<CChar>?
            _ = load_part_file(library.pointer, oldText, "\(old.name).schpart", &restoreError)
            free(restoreError)
            try? userLibrary.write(uuid: uuid, text: oldText)
            throw PartRegistrationError.backend(message)
        }
        guard let loaded = part_library_find_uuid(library.pointer, uuid) else {
            throw PartRegistrationError.backend("\(source): the part did not load")
        }
        let part = BackendCatalog.catalogPart(loaded.pointee) { Footprint($0.pointee) }
            .listed(in: PartCategory.userLibrary)
        if old.name != part.name { partsByName[old.name] = nil }
        partsByName[part.name] = part
        socketIndex = nil
        partsByUUID[uuid] = part
        registeredTexts[uuid] = text
        if let index = entries.firstIndex(where: { $0.uuid == uuid }) {
            entries[index] = part
        } else if !part.drawer.isEmpty {
            entries.append(part)
        }
        derivedCache = nil
        partUpdated?(uuid, part)
        return part
    }

    /// Removes a part from the User Library: deletes its file and takes it out
    /// of the drawer. It stays registered for the rest of the session, so
    /// documents that hold it still resolve it; the backend cannot unload it.
    func removeFromUserLibrary(uuid: String) throws {
        guard let userLibrary else { throw PartRegistrationError.noUserLibrary }
        do {
            try userLibrary.remove(uuid: uuid)
        } catch {
            throw PartRegistrationError.libraryWrite(error.localizedDescription)
        }
        userLibraryAdded[uuid] = nil
        if listings[uuid] != nil { setListing(.removed, for: uuid) }
    }

    /// Deletes a library file that could not be loaded and drops it from
    /// `userLibraryProblems` (plan-53-F).
    func removeUserLibraryProblem(_ problem: UserPartLibraryProblem) throws {
        guard let userLibrary else { throw PartRegistrationError.noUserLibrary }
        do {
            try userLibrary.removeFile(at: problem.url)
        } catch {
            throw PartRegistrationError.libraryWrite(error.localizedDescription)
        }
        userLibraryProblems.removeAll { $0 == problem }
    }

    /// Where a registered custom part is listed; nil for a bundled part.
    func listing(of part: CatalogPart) -> PartListing? {
        part.uuid.flatMap { listings[$0] }
    }

    /// Changes where a registered custom part is listed, in `entries` and in
    /// both lookups together (plan-53-B). `removed` takes it out of the drawer
    /// only: it stays registered, so documents that hold it still resolve it.
    func setListing(_ listing: PartListing, for uuid: String) {
        guard let current = partsByUUID[uuid], listings[uuid] != nil else { return }
        listings[uuid] = listing
        if let category = listing.category {
            let updated = current.listed(in: category)
            partsByName[updated.name] = updated
            socketIndex = nil
            partsByUUID[uuid] = updated
            if let index = entries.firstIndex(where: { $0.uuid == uuid }) {
                entries[index] = updated
            } else if !updated.drawer.isEmpty {
                entries.append(updated)
            }
        } else {
            entries.removeAll { $0.uuid == uuid }
        }
        derivedCache = nil
    }

    /// Whether `part` passes the menu filters of `filter` (not the query or
    /// the flags, which `groups` checks first).
    private func passes(_ part: CatalogPart, filter: DrawerFilter) -> Bool {
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
        if part.kind == PART_SWITCH, !part.isTactileSwitch, !part.isSlideSwitch, let count = filter.switchPositions, part.count != count {
            return false
        }
        if case let .converter(type, _, channels) = part.details,
           let wanted = filter.converterChannels[type], channels != wanted {
            return false
        }
        if let mv = filter.voltageMV {
            if let vcc = part.vcc, !(vcc.minMV...vcc.maxMV ~= mv) {
                return false
            }
            if case let .dipSwitch(_, _, ratedMV) = part.details, ratedMV > 0, ratedMV < mv {
                return false
            }
            if case let .tactileSwitch(_, ratedMV, _, _, _) = part.details, ratedMV > 0, ratedMV < mv {
                return false
            }
            if case let .slideSwitch(_, _, _, _, _, ratedMV) = part.details, ratedMV > 0, ratedMV < mv {
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
        if let wanted = filter.tactileActuator, let actuator = part.tactileActuator, actuator != wanted {
            return false
        }
        return true
    }

    /// Parts grouped by drawer path, then by function, after applying the filter.
    func groups(matching filter: DrawerFilter) -> [CategoryGroup] {
        let needle = filter.query.trimmingCharacters(in: .whitespaces)
            .folding(options: [.caseInsensitive, .diacriticInsensitive, .widthInsensitive], locale: nil)
        let derived = self.derived
        let rank = derived.nameRank

        // The flags and the query are checked on flat arrays; the part itself
        // is read only when a menu filter is set.
        let hidden: UInt8 = (filter.visibility.showObsolete ? 0 : 1) | (filter.visibility.showMilitary ? 0 : 2)
            | (filter.visibility.showAutomotive ? 0 : 4)
        let menuFiltered = filter.hasMenuFilter
        let queryHits = needle.isEmpty ? [] : derived.matches(needle)
        var matches: [CatalogPart] = []
        for index in derived.nameOrder where derived.flagBits[index] & hidden == 0 {
            if !needle.isEmpty, !queryHits[index] { continue }
            if menuFiltered, !passes(entries[index], filter: filter) { continue }
            matches.append(entries[index])
        }

        let byCategory = Dictionary(grouping: matches, by: { $0.drawer[0] })
        return byCategory
            .sorted { PartCategory.precedes($0.key, $1.key) }
            .map { category, parts in
                CategoryGroup(
                    category: category,
                    subcategories: Self.subgroups(parts, below: [category], rank: rank),
                    functions: Self.functionGroups(parts.filter { $0.drawer.count == 1 }, rank: rank)
                )
            }
    }

    /// The groups one drawer level below `path`, from the parts listed
    /// under it: each group holds its parts' deeper levels as subgroups, and
    /// the parts whose path ends there by function (or directly, as
    /// `listsPartsDirectly`).
    private static func subgroups(_ parts: [CatalogPart], below path: [String],
                                  rank: [String: Int]) -> [SubcategoryGroup] {
        let level = path.count
        let byName = Dictionary(grouping: parts.filter { $0.drawer.count > level }, by: { $0.drawer[level] })
        return byName
            .sorted { $0.key.localizedStandardCompare($1.key) == .orderedAscending }
            .map { name, parts in
                let groupPath = path + [name]
                let own = parts.filter { $0.drawer.count == groupPath.count }
                let subgroups = Self.subgroups(parts, below: groupPath, rank: rank)
                return listsPartsDirectly(own) || repeatsDrawerName(own, name)
                    ? SubcategoryGroup(subcategory: name, path: groupPath, subgroups: subgroups,
                                       functions: [], parts: byCountThenName(own, rank: rank))
                    : SubcategoryGroup(subcategory: name, path: groupPath, subgroups: subgroups,
                                       functions: functionGroups(own, rank: rank))
            }
    }

    /// DIP switches, LEDs, 7-segment displays, character LCDs, nixie tubes and
    /// the generic header are listed with no function level. Tactile switches are grouped
    /// by body size instead (`drawerLeaf`), slide switches by circuit and action
    /// (`SPDT On-On`, their function).
    private static func listsPartsDirectly(_ parts: [CatalogPart]) -> Bool {
        parts.allSatisfy {
            ($0.kind == PART_SWITCH && !$0.isTactileSwitch && !$0.isSlideSwitch) || $0.kind == PART_LED || $0.kind == PART_7SEG || $0.kind == PART_CHARLCD
                || $0.kind == PART_NIXIE || $0.kind == PART_DEKATRON || $0.kind == PART_LCD
                || $0.kind == PART_HEADER
        }
    }

    /// Card slots whose function is the drawer's own name (Card slot → SD →
    /// SD): a function level would repeat the group's title, so they are
    /// listed directly. A drawer that also holds other functions (SIM →
    /// Micro SIM, Nano SIM, SIM) keeps its function level.
    private static func repeatsDrawerName(_ parts: [CatalogPart], _ name: String) -> Bool {
        !parts.isEmpty && parts.allSatisfy { $0.isCardSlot && $0.function.caseInsensitiveCompare(name) == .orderedSame }
    }

    /// Parts by count (positions, digits), then by name.
    private static func byCountThenName(_ parts: [CatalogPart], rank: [String: Int]) -> [CatalogPart] {
        parts.sorted {
            ($0.count ?? 0) != ($1.count ?? 0)
                ? ($0.count ?? 0) < ($1.count ?? 0)
                : rank[$0.name, default: 0] < rank[$1.name, default: 0]
        }
    }

    /// Parts grouped by `drawerLeaf` (the function, or a tactile switch's
    /// size), groups in natural order, parts as given (`groups` passes them in
    /// natural name order) ("8 KB" before "32 KB").
    /// Capacitor leaves ("10 pF" … "4700 µF") go by capacitance instead
    /// (plan-33-H): the smallest `farads` of each group, ties by name; and
    /// resistor leaves ("< 1 kΩ" … "≥ 1 MΩ") by resistance (plan-34-G).
    private static func functionGroups(_ parts: [CatalogPart], rank: [String: Int] = [:]) -> [FunctionGroup] {
        let groups = Dictionary(grouping: parts, by: \.drawerLeaf)
        // The kind the leaf's parts share, if every part has a `drawerValue`
        // (crystal, inductor, fuse, buzzer, relay).
        let sharesDrawerValue = !parts.isEmpty && parts.allSatisfy { $0.drawerValue != nil }
        let relays = !parts.isEmpty && parts.allSatisfy { $0.relay != nil }
        let value: (CatalogPart) -> Double? =
            !parts.isEmpty && parts.allSatisfy({ $0.capacitorFarads != nil }) ? { $0.capacitorFarads }
            : !parts.isEmpty && parts.allSatisfy({ $0.resistorOhms != nil }) ? { $0.resistorOhms }
            // Relay leaves keep the name order: only their parts go by value.
            : sharesDrawerValue && !relays ? { $0.drawerValue }
            : { _ in nil }
        // Crystal leaves ("Under 4 MHz" … "32 MHz and over", "32 kHz" …) go
        // by the frequency of their lowest part (plan-44-G), the parts in a
        // leaf by frequency then name; inductor, fuse and buzzer leaves the
        // same by inductance, rated current and frequency, and a relay's
        // parts by coil voltage then name (plan-47-M).
        let byValue = !parts.isEmpty && parts.allSatisfy { value($0) != nil }
        func smallest(_ parts: [CatalogPart]) -> Double {
            parts.compactMap(value).min() ?? 0
        }
        return groups
            .sorted {
                if byValue {
                    let a = smallest($0.value), b = smallest($1.value)
                    if a != b { return a < b }
                }
                return $0.key.localizedStandardCompare($1.key) == .orderedAscending
            }
            .map { function, parts in
                let ordered = sharesDrawerValue
                    ? parts.sorted {
                        ($0.drawerValue ?? 0) != ($1.drawerValue ?? 0)
                            ? ($0.drawerValue ?? 0) < ($1.drawerValue ?? 0)
                            : rank[$0.name, default: 0] < rank[$1.name, default: 0]
                    }
                    // The card slots of a leaf go by pin count then name (plan-54-O),
                    // and so do a leaf's terminal blocks and JST headers (plan-45-H).
                    : parts.allSatisfy({ $0.isCardSlot || $0.isWireConnector }) ? byCountThenName(parts, rank: rank)
                    : parts
                return FunctionGroup(function: function, parts: ordered, bitGroups: bitGroups(parts))
            }
    }

    /// Converters split by resolution, fewest bits first; empty when the
    /// parts are not converters.
    private static func bitGroups(_ parts: [CatalogPart]) -> [BitGroup] {
        let byBits = Dictionary(grouping: parts) { part -> Int? in
            guard case let .converter(_, bits, _) = part.details else { return nil }
            return bits
        }
        guard byBits[nil] == nil else { return [] }
        return byBits.compactMap { bits, parts in bits.map { BitGroup(bits: $0, parts: parts) } }
            .sorted { $0.bits < $1.bits }
    }

    private func sortedNames(_ names: [String]) -> [String] {
        Array(Set(names)).sorted { $0.localizedStandardCompare($1) == .orderedAscending }
    }

    func load() {
        do {
            let parts = try BackendCatalog.load(into: library)
            partsByName = Dictionary(parts.map { ($0.name, $0) }, uniquingKeysWith: { first, _ in first })
            socketIndex = nil
            // The 80 header sizes are one generic header in the drawer
            // (`HeaderFamily`); `part(named:)` still finds every size.
            entries = parts.filter {
                !$0.drawer.isEmpty && ($0.kind != PART_HEADER || $0.name == HeaderFamily.defaultPart)
            }
            derivedCache = nil
            loadState = .loaded
        } catch {
            loadState = .failed(error.localizedDescription)
        }
    }
}
