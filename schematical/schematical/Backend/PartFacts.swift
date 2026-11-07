import Foundation

/// The rows of the Schematic sidebar's Details section: a part's overview,
/// then the facts of its kind. A row whose value would be empty is left out.
nonisolated enum PartFacts {
    static func rows(for part: CatalogPart) -> [(label: String, value: String)] {
        var rows: [(label: String, value: String)] = []
        func add(_ label: String, _ value: String) {
            if !value.isEmpty { rows.append((label, value)) }
        }

        add("Kind", kindName(part.kind))
        add("Function", part.function)
        add("Package", part.package)
        if let footprint = part.footprint {
            add("Footprint", "\(footprint.id) — \(footprint.description)")
        }
        if let connector = part.connectorFootprint {
            add("Connector", "\(connector.id) — \(connector.description)")
        }
        add("Pins", String(part.pins.filter { !$0.isHidden }.count))
        add("Flags", flagNames(part.flags).joined(separator: ", "))

        switch part.details {
        case let .logic(family, units):
            add("Family", family)
            add("Units", String(units))
        case let .sram(words, bits, accessNs, grade):
            add("Size", size(words: words, bits: bits))
            add("Access", "\(accessNs) ns")
            add("Grade", grade)
        case let .eeprom(words, bits, accessNs, grade, pageBytes, writeCycleUs):
            add("Size", size(words: words, bits: bits))
            add("Access", "\(accessNs) ns")
            add("Grade", grade)
            add("Page", "\(pageBytes) bytes")
            add("Write cycle", writeCycle(writeCycleUs))
        case let .dipSwitch(positions, actuator, ratedMV):
            add("Positions", String(positions))
            add("Actuator", actuator)
            if ratedMV > 0 { add("Rating", "\(VoltageCheck.volts(ratedMV)) V max") }
        case let .led(color):
            add("Color", color)
        case let .display(info):
            add("Digits", String(info.digits))
            add("Color", info.color)
            add("Digit size", info.size)
            add("Type", info.commonAnode ? "Common anode" : "Common cathode")
            add("LED chip", info.chip)
            if info.vfMaxMV > 0 {
                add("Forward voltage", "\(VoltageCheck.volts(info.vfMinMV))–\(VoltageCheck.volts(info.vfMaxMV)) V")
            }
        case let .oscillator(family, minHz, maxHz, defaultHz, enablePin):
            add("Family", family)
            if let body = part.footprint?.body {
                add("Size", bodySize(body))
            }
            add("Range", "\(hertz(minHz)) – \(hertz(maxHz))")
            add("Default", hertz(defaultHz))
            add("Enable pin", enablePin ?? "None")
        case let .supply(level, mv, pull):
            if pull {
                add("Level", level != 0 ? "High, weak (pull-up)" : "Low, weak (pull-down)")
            } else {
                add("Level", level != 0 ? "High (VCC)" : "Low (GND)")
            }
            if level != 0 { add("Voltage", "\(VoltageCheck.volts(mv)) V") }
        case let .cpu(core, grade, maxHz):
            add("Core", coreName(core))
            add("Speed grade", grade)
            add("Max clock", "\(maxHz / 1_000_000) MHz")
        case let .characterLCD(info):
            add("Characters", "\(info.columns) × \(info.rows)")
            add("Controller", info.controller)
            add("Character ROM", info.rom == "A00" ? "A00 (Japanese)" : info.rom)
            add("Panel", info.panel == .blueNegative ? "STN blue, negative" : "STN yellow-green, positive")
            add("Backlight", info.backlightColor ?? "None")
        case let .serial(model, channels, grade):
            add("Model", model)
            add("Channels", String(channels))
            add("Speed grade", grade)
        case let .pio(model, grade, maxHz):
            add("Model", modelName(model))
            add("Speed grade", grade)
            add("Max clock", "\(maxHz / 1_000_000) MHz")
        case let .connector(centerPinUM, ratedMV, ratedMA, contacts):
            add("Centre pin", String(format: "%.1f mm", Double(centerPinUM) / 1000))
            add("Rating", "\(VoltageCheck.volts(ratedMV)) V, \(String(format: "%.1f", Double(ratedMA) / 1000)) A")
            add("Sleeve contact", contacts > 0 ? "Normally closed when unplugged" : "None")
        }

        if let vcc = part.vcc {
            add("Supply range", "\(VoltageCheck.volts(vcc.minMV))–\(VoltageCheck.volts(vcc.maxMV)) V")
            if vcc.inputMaxMV > 0 { add("Input max", "\(VoltageCheck.volts(vcc.inputMaxMV)) V") }
        }
        return rows
    }

    static func kindName(_ kind: PartKind) -> String {
        switch kind {
        case PART_GATE: return "Logic gate"
        case PART_DFF: return "D flip-flop"
        case PART_JKFF: return "JK flip-flop"
        case PART_SRAM: return "SRAM"
        case PART_EEPROM: return "EEPROM"
        case PART_SWITCH: return "DIP switch"
        case PART_LED: return "LED"
        case PART_7SEG: return "7-segment display"
        case PART_OSCILLATOR: return "Oscillator"
        case PART_SUPPLY: return "Supply"
        case PART_LATCH: return "Latch"
        case PART_COUNTER: return "Counter"
        case PART_SHIFT: return "Shift register"
        case PART_CPU: return "Processor"
        case PART_CHARLCD: return "Character LCD"
        case PART_SERIAL: return "Serial controller"
        case PART_CONNECTOR: return "Barrel connector"
        case PART_PIO: return "Peripheral I/O"
        default: return ""
        }
    }

    /// A parallel I/O model as shown: "6521 PIA", "6522 VIA"; others raw.
    static func modelName(_ model: String) -> String {
        switch model {
        case "pia6521": return "6521 PIA"
        case "via6522": return "6522 VIA"
        default: return model
        }
    }

    /// A processor core as shown: "6502 (NMOS)", "65C02 (WDC)", "Z80".
    static func coreName(_ core: String) -> String {
        switch core {
        case "6502": return "6502 (NMOS)"
        case "65c02": return "65C02 (WDC)"
        case "z80": return "Z80"
        default: return core
        }
    }

    static func flagNames(_ flags: CatalogPart.Flags) -> [String] {
        var names: [String] = []
        if flags.obsolete { names.append("Obsolete") }
        if flags.military { names.append("Military") }
        if flags.automotive { names.append("Automotive") }
        if flags.enhanced { names.append("Enhanced") }
        if flags.ceramic { names.append("Ceramic") }
        return names
    }

    /// "32768 × 8 (32 KB)": words × bits, then the byte total.
    static func size(words: Int, bits: Int) -> String {
        let bytes = Double(words * bits) / 8
        let total: String
        if bytes < 1024 {
            total = "\(trimmed(bytes, decimals: 0)) bytes"
        } else if bytes < 1_048_576 {
            total = "\(trimmed(bytes / 1024, decimals: 2)) KB"
        } else {
            total = "\(trimmed(bytes / 1_048_576, decimals: 2)) MB"
        }
        return "\(words) × \(bits) (\(total))"
    }

    /// A body as length × width, longer side first: "3.2 × 2.5 mm".
    static func bodySize(_ body: CGSize) -> String {
        let long = max(body.width, body.height), short = min(body.width, body.height)
        return "\(trimmed(long, decimals: 2)) × \(trimmed(short, decimals: 2)) mm"
    }

    /// 32768 → "32.768 kHz", 110 000 000 → "110 MHz".
    static func hertz(_ hz: UInt64) -> String {
        let value = Double(hz)
        if value >= 1e6 { return "\(trimmed(value / 1e6, decimals: 3)) MHz" }
        if value >= 1e3 { return "\(trimmed(value / 1e3, decimals: 3)) kHz" }
        return "\(trimmed(value, decimals: 3)) Hz"
    }

    /// Under 1000 µs as µs, else ms to up to 3 decimals: 5000 → "5 ms".
    static func writeCycle(_ us: Int) -> String {
        us < 1000 ? "\(us) µs" : "\(trimmed(Double(us) / 1000, decimals: 3)) ms"
    }

    /// `value` to at most `decimals` places, trailing zeros removed.
    static func trimmed(_ value: Double, decimals: Int) -> String {
        var text = String(format: "%.\(decimals)f", value)
        if text.contains(".") {
            while text.hasSuffix("0") { text.removeLast() }
            if text.hasSuffix(".") { text.removeLast() }
        }
        return text
    }
}
