import Foundation

/// The groups the Part Wizard's "What are you making?" step lists the kinds
/// under (plan-46-D § 3.1), in the order shown.
enum KindGroup: String, CaseIterable, Sendable {
    case gates = "Logic gates and flip-flops"
    case logic = "Logic functions"
    case memory = "Memory"
    case processors = "Processors and interface chips"
    case displays = "Displays and lights"
    case discretes = "Passives and discretes"
    case connectors = "Connectors and switches"
    case power = "Power and clocks"
    case other = "Other"
}

/// What the Part Editor says about a kind to someone who has never seen the
/// part format (plan-46-D): its name in plain words, one line on what it
/// covers, and bundled parts of that kind to recognize it by.
struct KindInfo: Sendable {
    /// The kind name in part files (`"gate"`).
    let kind: String
    /// The name shown in the picker and in window titles ("Logic Gate").
    let name: String
    /// One line: what the kind covers, with familiar part numbers.
    let blurb: String
    /// Bundled part names of this kind (each exists in the catalog; a test checks).
    let examples: [String]
    let group: KindGroup
    /// false: the backend has no model for the kind (`"simulated": false`).
    var simulated = true
}

extension PartKinds {
    /// One entry per kind (`PartKinds.all`). A new kind needs an entry here:
    /// `KindInfoTests` fails without one.
    static let info: [String: KindInfo] = {
        let entries = [
            // Logic gates and flip-flops
            KindInfo(kind: "gate", name: "Logic Gate",
                     blurb: "AND, OR, NAND, NOR, XOR gates, inverters and plain buffers: 74HC00, 74HC04, CD4011.",
                     examples: ["SN74HC00N", "SN74HC20DR", "SN74ALS05AN"], group: .gates),
            KindInfo(kind: "buffer", name: "Buffer",
                     blurb: "Bus buffers with an enable pin, transceivers and level shifters: 74HC125, 74HC245, 74LVC1T45.",
                     examples: ["SN74LVTH126PW", "SN74ABT241APW", "SN74LVC1T45DRLR"], group: .gates),
            KindInfo(kind: "dff", name: "D Flip-Flop",
                     blurb: "Flip-flops that copy D to Q on a clock edge, with clear and preset: 74HC74, 74HC175.",
                     examples: ["SN74LV74ARGYR", "CD74HC175EE4"], group: .gates),
            KindInfo(kind: "jkff", name: "JK Flip-Flop",
                     blurb: "Flip-flops with J and K inputs that set, clear or toggle on a clock edge: 74LS109, 74LS112.",
                     examples: ["SN74LS109ANSR"], group: .gates),
            KindInfo(kind: "latch", name: "Latch",
                     blurb: "Latches that follow their inputs while enabled and hold them otherwise: 74HC373, 74HC573, 74HC4511.",
                     examples: ["SN74HC573ADBR", "SN74AHC373PWR", "CD74HC4511PWRG4"], group: .gates),

            // Logic functions
            KindInfo(kind: "mux", name: "Multiplexer",
                     blurb: "Data selectors that pass one of several inputs, and small register files: 74HC151, 74HC157, 74HC670.",
                     examples: ["SN74ALS153N", "SN74AHCT157DGVR"], group: .logic),
            KindInfo(kind: "decoder", name: "Decoder",
                     blurb: "Decoders, demultiplexers, priority encoders and 7-segment decoders: 74HC138, 74HC139, CD40147.",
                     examples: ["SN74HCT139DR", "CD40147BE"], group: .logic),
            KindInfo(kind: "arith", name: "Arithmetic",
                     blurb: "Adders, comparators, parity checkers and the 74181 ALU: 74HC283, 74HC85, 74HC280.",
                     examples: ["SN74ALS520DW", "CD74ACT280M96"], group: .logic),
            KindInfo(kind: "counter", name: "Counter",
                     blurb: "Binary and decade counters, ripple counters and dividers: 74HC161, 74HC193, CD4020.",
                     examples: ["SN74LS293N", "CD74HC4020E"], group: .logic),
            KindInfo(kind: "shift", name: "Shift Register",
                     blurb: "Shift registers, serial to parallel and back, and FIFOs: 74HC595, 74HC165, 74HC299.",
                     examples: ["SN74HC595DRG3", "CD74HC299M96E4"], group: .logic),
            KindInfo(kind: "register", name: "Register",
                     blurb: "Rows of D flip-flops on one clock, with clock enable or 3-state outputs: 74HC377, 74HC574, CD40174.",
                     examples: ["74HC377D,653", "SN74LS377DWR"], group: .logic),

            KindInfo(kind: "pld", name: "Programmable Logic",
                     blurb: "GALs and PLDs you program with your own logic equations: ATF16V8, ATF22V10, ATF750C. Symbol, footprint and facts only: they are not simulated yet.",
                     examples: ["ATF16V8B-15PU", "ATF22V10C-15PU", "ATF750CL-15PU"], group: .logic, simulated: false),

            // Memory
            KindInfo(kind: "sram", name: "SRAM",
                     blurb: "Parallel static RAM, read and written through address and data pins: 6264, 62256.",
                     examples: ["AS6C6264-55PCN", "AS7C256A-12TC"], group: .memory),
            KindInfo(kind: "eeprom", name: "EEPROM",
                     blurb: "Parallel EEPROMs that keep their data without power: AT28C64, AT28C256.",
                     examples: ["AT28C256F-15JU", "AT28C64B-15JU"], group: .memory),
            KindInfo(kind: "serialmem", name: "Serial memory",
                     blurb: "SPI memory chips: serial EEPROM, F-RAM and NOR flash, read and written a byte stream at a time: 25LC256, FM25L16B, AT25XE021A.",
                     examples: ["AT25040B-SSHL-T", "FM25L16B-GTR", "AT25XE021A-SSHN-T"], group: .memory),

            // Processors and interface chips
            KindInfo(kind: "cpu", name: "Processor",
                     blurb: "Microprocessors that run a program from memory on a bus: the 65C02 and 65C816.",
                     examples: ["W65C02S6TPG-14", "W65C816S6TPG-14"], group: .processors),
            KindInfo(kind: "mcu", name: "Microcontroller",
                     blurb: "Single-chip computers (AVR, ESP32, RP2040). Symbol, footprint and facts only: they are not simulated.",
                     examples: ["ATMEGA328P-PU", "ATTINY13A-PU", "SC0914(13)"], group: .processors, simulated: false),
            KindInfo(kind: "pio", name: "Parallel I/O",
                     blurb: "Port chips a processor reads and writes pins through: the 6521 PIA and 6522 VIA.",
                     examples: ["W65C21S6TPG-14", "W65C22S6TPG-14"], group: .processors),
            KindInfo(kind: "serial", name: "Serial",
                     blurb: "UARTs, ACIAs and the I2C, SPI and USB bridges to a serial line: 16550, 6551, MAX3100.",
                     examples: ["TL16C550DRHB", "MAX3100EEE+"], group: .processors),
            KindInfo(kind: "converter", name: "Converter",
                     blurb: "Analog-to-digital and digital-to-analog converters with an SPI, I2C or parallel bus: ADS7142, TLV5639.",
                     examples: ["ADS7142IRUGT", "TLV5639IPW"], group: .processors),
            KindInfo(kind: "rtc", name: "Real-Time Clock",
                     blurb: "Clock and calendar chips that keep the time on a battery: BQ32000, BQ4802.",
                     examples: ["BQ32000D", "BQ4802YDW"], group: .processors),

            // Displays and lights
            KindInfo(kind: "led", name: "LED",
                     blurb: "A single light-emitting diode with an anode and a cathode.",
                     examples: ["GENERIC-LED"], group: .displays),
            KindInfo(kind: "7seg", name: "7-Segment Display",
                     blurb: "LED digit displays, one to four digits, common anode or common cathode.",
                     examples: ["LTC-4727JR", "LTS-6960HR"], group: .displays),
            KindInfo(kind: "lcd", name: "LCD Glass",
                     blurb: "Segment LCD glass: cells between a segment pin and a backplane pin, static or multiplexed (1:2, 1:3, 1:4).",
                     examples: ["LCD-S401C39TR", "GENERIC-LCD-8D-1:4"], group: .displays),
            KindInfo(kind: "leddriver", name: "LED Display Driver",
                     blurb: "Chips that scan digits or LEDs for you after a host writes them over SPI, I2C or a parallel bus: ICM7218, MAX6951, AS1115.",
                     examples: ["ICM7218AIPI+", "MAX6951CEE+T"], group: .displays),
            KindInfo(kind: "lcddriver", name: "LCD Display Driver",
                     blurb: "Chips that drive segment LCD glass (backplanes and segments) from a display RAM a host writes: PCF8576, PCF85176, BU9796.",
                     examples: ["PCF85176T/1Y", "BU9796AFS-E2"], group: .displays),
            KindInfo(kind: "charlcd", name: "Character LCD",
                     blurb: "Text LCD modules with an HD44780-style controller: 16x2, 20x4.",
                     examples: ["NHD-0216BZ-FL-YBW"], group: .displays),
            KindInfo(kind: "nixie", name: "Nixie Tube",
                     blurb: "Neon numeric indicator tubes with one anode and a cathode per numeral: IN-14, IN-12A, IN-18.",
                     examples: ["IN-14", "IN-18"], group: .displays),
            KindInfo(kind: "dekatron", name: "Dekatron",
                     blurb: "Cold-cathode decade counting tubes: a glow that steps round ten cathodes as the guides are pulsed (GC10B, OG-4, Z504S).",
                     examples: ["GC10B", "Z504S"], group: .displays),

            // Passives and discretes
            KindInfo(kind: "resistor", name: "Resistor",
                     blurb: "Fixed resistors and resistor networks.",
                     examples: ["RC0805JR-07150RL", "GENERIC-RESISTOR"], group: .discretes),
            KindInfo(kind: "capacitor", name: "Capacitor",
                     blurb: "Ceramic, electrolytic and tantalum capacitors.",
                     examples: ["C0402C680K5RACTU", "TAP337K006CRW"], group: .discretes),
            KindInfo(kind: "diode", name: "Diode",
                     blurb: "Rectifier, switching, Schottky and Zener diodes, and bridge rectifiers: 1N4148, 1N5819, 1N5221.",
                     examples: ["1N5221B", "S1DHE3_A/H"], group: .discretes),
            KindInfo(kind: "transistor", name: "Transistor",
                     blurb: "Bipolar transistors, MOSFETs and transistor arrays: 2N2222, 2N7000.",
                     examples: ["PBSS4330PASX", "GENERIC-NMOS"], group: .discretes),

            // Connectors and switches
            KindInfo(kind: "connector", name: "Connector",
                     blurb: "Power jacks and USB sockets that bring power onto the board.",
                     examples: ["PJ-037A", "UJ2-AH-1-SMT-TR"], group: .connectors),
            KindInfo(kind: "socket", name: "IC Socket",
                     blurb: "A socket a DIP or PLCC chip plugs into: the socket's footprint replaces the chip's on the board.",
                     examples: ["GENERIC-SOCKET-DIP-14-300", "GENERIC-SOCKET-PLCC-44"], group: .connectors),
            KindInfo(kind: "header", name: "Pin Header",
                     blurb: "Rows of pins or sockets, one or two rows, for jumper wires and ribbon cables.",
                     examples: ["GENERIC-HEADER-1X40", "GENERIC-HEADER-2X13"], group: .connectors),
            KindInfo(kind: "switch", name: "Switch",
                     blurb: "DIP switches, slide switches and push buttons you operate while the simulation runs.",
                     examples: ["TE-GDS0204", "PTS645VK58-2 LFS"], group: .connectors),

            // Power and clocks
            KindInfo(kind: "supply", name: "Supply",
                     blurb: "Power and ground symbols, and pull-up and pull-down resistors to a rail.",
                     examples: ["5V", "GND", "PULLUP-3V3"], group: .power),
            KindInfo(kind: "oscillator", name: "Oscillator",
                     blurb: "Clock oscillators that put out a square wave at a fixed frequency.",
                     examples: ["ECS-2100AX-160"], group: .power),
            KindInfo(kind: "crystal", name: "Crystal",
                     blurb: "Quartz crystals and ceramic resonators: the clock reference for a chip's oscillator pins.",
                     examples: ["GENERIC-CRYSTAL-8M", "GENERIC-CRYSTAL-32768"], group: .power),
            KindInfo(kind: "regulator", name: "Voltage Regulator",
                     blurb: "Linear regulators and LDOs, fixed or adjustable, and switching regulators: a regulated output from a higher input.",
                     examples: ["GENERIC-REG-5V", "GENERIC-REG-ADJ"], group: .power),
            KindInfo(kind: "powerswitch", name: "Power Switch",
                     blurb: "Load switches, current-limited and USB power switches and integrated ideal diodes: an input passed to an output, gated by an enable pin.",
                     examples: ["LM66200DRLR", "TPS22922YFPR"], group: .power),

            // Other
            KindInfo(kind: "generic", name: "Other part (no simulation)",
                     blurb: "Anything else: just a symbol and a footprint. It has pins you can wire, but it is not simulated.",
                     examples: [], group: .other, simulated: false),
        ]
        return Dictionary(uniqueKeysWithValues: entries.map { ($0.kind, $0) })
    }()

    /// The kinds of a group, in the order `info` lists them.
    static func kinds(in group: KindGroup) -> [KindInfo] {
        infoOrder.compactMap { info[$0] }.filter { $0.group == group }
    }

    /// The order the kinds are listed in within their groups.
    private static let infoOrder = [
        "gate", "buffer", "dff", "jkff", "latch",
        "mux", "decoder", "arith", "counter", "shift", "register", "pld",
        "sram", "eeprom", "serialmem",
        "cpu", "mcu", "pio", "serial", "converter", "rtc",
        "led", "7seg", "lcd", "leddriver", "lcddriver", "charlcd", "nixie", "dekatron",
        "resistor", "capacitor", "diode", "transistor",
        "connector", "header", "socket", "switch",
        "supply", "oscillator", "crystal", "regulator", "powerswitch",
        "generic",
    ]
}
