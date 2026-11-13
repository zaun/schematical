import Foundation

/// The rows of the Schematic sidebar's Details section: a part's overview,
/// then the facts of its kind. A row whose value would be empty is left out.
nonisolated enum PartFacts {
    /// The rows for a placed part. Package and Footprint follow the part's
    /// chosen footprint choice, and an LED's Color its own color.
    static func rows(for chip: PlacedChip) -> [(label: String, value: String)] {
        let part = chip.part
        var rows: [(label: String, value: String)] = []
        func add(_ label: String, _ value: String) {
            if !value.isEmpty { rows.append((label, value)) }
        }

        add("Kind", kindName(part))
        add("Function", part.function)
        add("Package", chip.footprintChoice?.package ?? part.package)
        if let footprint = chip.footprintChoice?.footprint ?? part.footprint {
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
        case let .tactileSwitch(actuator, ratedMV, normallyClosed, _, body):
            add("Contact", normallyClosed ? "Normal close, momentary" : "Normal open, momentary")
            add("Terminals", String(part.pins.filter { !$0.isHidden }.count))
            add("Actuator", actuator)
            if let body { add("Body", body.label) }
            if ratedMV > 0 { add("Rating", "\(VoltageCheck.volts(ratedMV)) V max") }
        case let .led(colors):
            add("Color", chip.color ?? colors.first ?? "")
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
            add("Panel", info.panel.facts)
            add("Backlight", info.backlightColors.isEmpty ? "None" : info.backlightColors.joined(separator: ", "))
        case let .serial(model, channels, grade, host):
            add("Model", model)
            add("Host bus", serialHost(host))
            add("Channels", String(channels))
            add("Speed grade", grade)
        case let .pio(model, grade, maxHz):
            add("Model", modelName(model))
            add("Speed grade", grade)
            add("Max clock", "\(maxHz / 1_000_000) MHz")
        case let .connector(_, ratedMV, ratedMA, _, usb?):
            add("Type", "USB-\(usb)")
            if ratedMV > 0 { add("Rated voltage", "\(VoltageCheck.volts(ratedMV)) V") }
            if ratedMA > 0 { add("Rated current", String(format: "%.1f A", Double(ratedMA) / 1000)) }
        case let .connector(centerPinUM, ratedMV, ratedMA, contacts, nil):
            add("Centre pin", String(format: "%.1f mm", Double(centerPinUM) / 1000))
            add("Rating", "\(VoltageCheck.volts(ratedMV)) V, \(String(format: "%.1f", Double(ratedMA) / 1000)) A")
            add("Sleeve contact", contacts > 0 ? "Normally closed when unplugged" : "None")
        case let .header(rows, perRow, genders):
            add("Rows", String(rows))
            add("Pins per row", String(perRow))
            add("Gender", chip.gender ?? genders.first ?? "")
        case let .converter(_, bits, channels):
            add("Bits", String(bits))
            add("Channels", String(channels))
        case .rtc:
            break
        case let .diode(type, diodes, vrMV, ioMA, vzMV, pdMW):
            add("Type", diodeType(type))
            if diodes > 1 { add("Diodes", String(diodes)) }
            if type == "zener" {
                add("Zener voltage", "\(milli(vzMV)) V")
                add("Power", "\(milli(pdMW)) W")
            } else {
                add("Reverse voltage", "\(milli(vrMV)) V")
                add("Forward current", "\(milli(ioMA)) A")
            }
        case let .capacitor(type, _, farads, ratedMV, dielectric, tolerance):
            add("Type", capacitorType(type))
            add("Capacitance", CapacitorValue.text(chip.farads ?? farads))
            if ratedMV > 0 { add("Rated voltage", "\(milli(ratedMV)) V") }
            add("Dielectric", dielectric ?? "")
            add("Tolerance", tolerance ?? "")
        }

        if part.supplyDomains.count == 2 {
            // A dual-supply part (plan-20-I): one range per side.
            for (side, domain) in zip(["A", "B"], part.supplyDomains) {
                let label = domain.power.isEmpty ? "Supply \(side)" : "Supply \(side) (\(domain.power))"
                add(label, "\(VoltageCheck.volts(domain.minMV))–\(VoltageCheck.volts(domain.maxMV)) V")
                add("Input max \(side)", "\(VoltageCheck.volts(domain.inputMaxMV)) V")
            }
        } else if let vcc = part.vcc {
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
        case PART_DECODER: return "Decoder"
        case PART_BUFFER: return "Buffer"
        case PART_MUX: return "Multiplexer"
        case PART_HEADER: return "Header"
        case PART_ARITH: return "Arithmetic"
        case PART_CONVERTER: return "Converter"
        case PART_RTC: return "RTC"
        case PART_DIODE: return "Diode"
        case PART_CAPACITOR: return "Capacitor"
        case PART_REGISTER: return "Register"
        default: return ""
        }
    }

    /// A part's kind as shown: a tactile switch as "Tactile switch", a
    /// converter by its type ("ADC", "DAC"), other
    /// kinds as `kindName(_: PartKind)`.
    static func kindName(_ part: CatalogPart) -> String {
        if part.isTactileSwitch { return "Tactile switch" }
        if case .connector(_, _, _, _, _?) = part.details { return "USB connector" }
        if case let .converter(type, _, _) = part.details {
            switch type {
            case "adc": return "ADC"
            case "dac": return "DAC"
            default: return type.uppercased()
            }
        }
        return kindName(part.kind)
    }

    /// A diode type as shown (plan-29): "Rectifier", "Fast recovery", ….
    /// A capacitor type as shown (plan-33-H).
    static func capacitorType(_ type: String) -> String {
        switch type {
        case "ceramic": return "Ceramic"
        case "aluminum": return "Aluminum electrolytic"
        case "aluminum-polymer": return "Aluminum polymer"
        case "tantalum": return "Tantalum"
        case "tantalum-polymer": return "Tantalum polymer"
        case "niobium-oxide": return "Niobium oxide"
        default: return type
        }
    }

    static func diodeType(_ type: String) -> String {
        switch type {
        case "rectifier": return "Rectifier"
        case "fast": return "Fast recovery"
        case "switching": return "Switching"
        case "schottky": return "Schottky"
        case "zener": return "Zener"
        case "bridge": return "Bridge"
        default: return type
        }
    }

    /// Thousandths as a trimmed decimal: 50000 → "50", 500 → "0.5",
    /// 75 → "0.075" (millivolts as volts, milliamps as amps, …).
    static func milli(_ value: UInt32) -> String {
        var text = String(format: "%.3f", Double(value) / 1000)
        while text.hasSuffix("0") { text.removeLast() }
        if text.hasSuffix(".") { text.removeLast() }
        return text
    }

    /// A parallel I/O model as shown: "6521 PIA", "6522 VIA"; others raw.
    static func modelName(_ model: String) -> String {
        switch model {
        case "pia6521": return "6521 PIA"
        case "via6522": return "6522 VIA"
        default: return model
        }
    }

    /// A serial part's host-side interface as shown: "Parallel", "SPI",
    /// "I²C", "I²C/SPI", "USB"; others raw.
    static func serialHost(_ host: String) -> String {
        switch host {
        case "parallel": return "Parallel"
        case "spi": return "SPI"
        case "i2c": return "I²C"
        case "i2c-spi": return "I²C/SPI"
        case "usb": return "USB"
        default: return host
        }
    }

    /// A processor core as shown: "6502 (NMOS)", "65C02 (WDC)", "Z80", "65C816 (WDC)".
    static func coreName(_ core: String) -> String {
        switch core {
        case "6502": return "6502 (NMOS)"
        case "65c02": return "65C02 (WDC)"
        case "z80": return "Z80"
        case "65816": return "65C816 (WDC)"
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
