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
    var drawer: [String]
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
