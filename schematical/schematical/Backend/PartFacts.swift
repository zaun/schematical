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
        if let footprint = chip.socket?.part.footprint ?? chip.footprintChoice?.footprint ?? part.footprint {
            add("Footprint", "\(footprint.id) — \(footprint.description)")
        }
        if let socket = chip.socket {
            add("Socket", "\(socket.part.name) (\(socket.mode == .withSocket ? "IC + Socket" : "Socket only"))")
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
            let volts = info.vfMinMV == info.vfMaxMV
                ? "\(VoltageCheck.volts(info.vfMaxMV)) V"
                : "\(VoltageCheck.volts(info.vfMinMV))–\(VoltageCheck.volts(info.vfMaxMV)) V"
            switch info.type {
            case "numitron":
                add("Type", "Numitron (incandescent)")
                add("Segments", info.chip)
                if info.vfMaxMV > 0 { add("Segment voltage", volts) }
            case "vfd":
                add("Type", "Vacuum fluorescent")
                add("Phosphor", info.chip)
                if info.vfMaxMV > 0 { add("Anode and grid voltage", volts) }
            case "vfd-static":
                add("Type", "Vacuum fluorescent, no grid")
                add("Phosphor", info.chip)
                if info.vfMaxMV > 0 { add("Anode voltage", volts) }
            default:
                add("Type", info.commonAnode ? "Common anode" : "Common cathode")
                add("LED chip", info.chip)
                if info.vfMaxMV > 0 { add("Forward voltage", volts) }
            }
        case let .nixie(info):
            add("Characters", info.characters.joined(separator: " "))
            if info.glyphs.contains("LDP") && info.glyphs.contains("DP") {
                add("Decimal points", "Left and right")
            } else if info.glyphs.contains("LDP") || info.glyphs.contains("DP") {
                add("Decimal point", info.glyphs.contains("DP") ? "Right" : "Left")
            }
            add("Character size", info.size)
            add("Viewing", info.view == "end" ? "End view (through the top)" : "Side view")
            add("Glow", info.color)
            add("Firing voltage", "\(VoltageCheck.volts(info.strikeMV)) V (anode supply, minimum)")
            add("Maintaining voltage", "\(VoltageCheck.volts(info.sustainMV)) V")
            add("Cathode current", info.currentMinUA == info.currentMaxUA
                ? "\(milli(info.currentMaxUA)) mA" : "\(milli(info.currentMinUA))–\(milli(info.currentMaxUA)) mA")
        case let .dekatron(info):
            add("Type", info.twoGuide ? "Decade counting tube, two guides (counts both ways)"
                                      : "Decade counting tube, single pulse (counts forward)")
            add("Cathodes", "\(info.cathodePins.count) pins for ten positions")
            add("Bulb", info.size)
            add("Viewing", info.view == "end" ? "End view (through the top)" : "Side view")
            add("Glow", info.color)
            add("Minimum anode supply", "\(VoltageCheck.volts(info.strikeMV)) V")
            add("Running voltage", "\(VoltageCheck.volts(info.sustainMV)) V")
            add("Anode current", info.currentMinUA == info.currentMaxUA
                ? "\(milli(info.currentMaxUA)) mA" : "\(milli(info.currentMinUA))–\(milli(info.currentMaxUA)) mA")
            add("Counting rate", "up to \(hertz(UInt64(info.rateHz)))")
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
        case let .wireConnector(kind, variant, pitchUM, ratedMV, ratedMA):
            add("Type", kind == "jst" ? "JST header" : "Terminal block")
            add("Pitch", String(format: "%.2f mm", Double(pitchUM) / 1000))
            add(kind == "jst" ? "Series" : "Clamp", kind == "jst" ? variant : clampName(variant))
            add("Positions", String(part.pins.filter { !$0.isHidden }.count))
            if ratedMV > 0 { add("Rated voltage", "\(VoltageCheck.volts(ratedMV)) V") }
            if ratedMA > 0 { add("Rated current", String(format: "%g A", Double(ratedMA) / 1000)) }
        case let .cardSlot(card, pitchUM):
            add("Card", card)
            if pitchUM > 0 { add("Pitch", String(format: "%.2f mm", Double(pitchUM) / 1000)) }
        case .testPoint:
            break
        case let .header(rows, perRow, genders):
            add("Rows", String(rows))
            add("Pins per row", String(perRow))
            add("Gender", chip.gender ?? genders.first ?? "")
        case let .converter(_, bits, channels):
            add("Bits", String(bits))
            add("Channels", String(channels))
        case .rtc, .displayDriver:
            break
        case let .serialmem(info):
            add("Type", serialMemoryType(info.type))
            add("Size", "\(density(info.size)) (\(bytes(info.size)))")
            if info.addressBytes > 0 { add("Address bytes", String(info.addressBytes)) }
            add("Page", bytes(info.pageBytes))
            switch info.type {
            case "flash", "dataflash":
                add("Page program", duration(info.writePS))
            default:
                add("Write cycle", duration(info.writePS))
            }
            add("Sector erase", duration(info.sectorErasePS))
            add("Chip erase", duration(info.chipErasePS))
            add("JEDEC ID", info.jedecID ?? "")
            if info.maxClockKHz > 0 { add("Max clock", hertz(UInt64(info.maxClockKHz) * 1000)) }
            if info.endurance > 0 { add("Endurance", enduranceText(info.endurance)) }
            if info.retentionYears > 0 { add("Retention", "\(info.retentionYears) years") }
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
        case let .mcu(core, maxHz, flash, ram, eeprom, psram):
            add("Core", mcuCoreName(core))
            add("Clock", hertz(maxHz))
            add("Flash", bytes(flash))
            add("RAM", bytes(ram))
            add("EEPROM", bytes(eeprom))
            add("PSRAM", bytes(psram))
        case let .pld(macrocells, tpdPS):
            add("Macrocells", String(macrocells))
            add("Delay (tPD)", duration(tpdPS))
        case let .resistor(ohms, tolerancePercent, powerW, resistors):
            add("Resistance", ResistorValue.text(part.hasOhmsField ? chip.ohms ?? ohms : ohms))
            add("Tolerance", tolerancePercent.map { "±" + percentText($0) } ?? "")
            add("Power", powerW.map(wattsText) ?? "")
            if resistors > 1 { add("Resistors", String(resistors)) }
        case let .transistor(info):
            add("Type", transistorType(info))
            if info.elements > 1 { add("Elements", String(info.elements)) }
            func rating(_ label: String, _ value: UInt32, _ unit: String, prefix: String = "") {
                if value > 0 { add(label, "\(prefix)\(milli(value)) \(unit)") }
            }
            if info.mosfet {
                rating("Vds", info.vmaxMV, "V")
                rating("Id", info.imaxMA, "A")
                if info.rdsonMilliohm > 0 { add("Rds(on)", milliohmsText(info.rdsonMilliohm)) }
                rating("Vgs(th)", info.vgsthMV, "V", prefix: "≤ ")
                rating("Vgs", info.vgsMV, "V", prefix: "±")
                rating("Power", info.powerMW, "W")
            } else {
                rating("Vceo", info.vmaxMV, "V")
                rating("Ic", info.imaxMA, "A")
                rating("Power", info.powerMW, "W")
                if info.hfe > 0 { add("hFE", String(info.hfe)) }
                if info.r1Milliohm > 0 { add("R1", milliohmsText(info.r1Milliohm)) }
                if info.r2Milliohm > 0 { add("R2", milliohmsText(info.r2Milliohm)) }
            }
        case let .crystal(info):
            add("Type", info.type == "resonator" ? "Ceramic resonator" : "Crystal")
            add("Frequency", hertz(info.hz))
            if info.loadFF > 0 { add("Load capacitance", CapacitorValue.text(Double(info.loadFF) / 1e15)) }
            add("Tolerance", CrystalInfo.ppmText(info.tolerancePPM))
            add("Stability", CrystalInfo.ppmText(info.stabilityPPM))
            if info.esrMOhm > 0 { add("ESR", milliohmsText(info.esrMOhm)) }
            if info.capacitors { add("Built-in capacitors", "Yes") }
        case let .powerswitch(info):
            add("Type", powerSwitchType(info))
            if info.switches > 1 { add(info.type == "ideal-diode" ? "Inputs" : "Channels", String(info.switches)) }
            if info.vinMaxMV > 0 {
                add("Input", info.vinMinMV > 0 ? "\(milli(info.vinMinMV))–\(milli(info.vinMaxMV)) V"
                                               : "up to \(milli(info.vinMaxMV)) V")
            } else if info.vinMinMV > 0 {
                add("Input", "from \(milli(info.vinMinMV)) V")
            }
            if info.ioutMA > 0 { add("Current", "\(milli(info.ioutMA)) A") }
            if info.type == "limited", info.ilimitMA > 0 { add("Current limit", "\(milli(info.ilimitMA)) A") }
            if info.ronMOhm > 0 { add("On-resistance", milliohmsText(UInt64(info.ronMOhm))) }
            if info.iqUA > 0 {
                add("Quiescent", info.iqUA >= 1000 ? "\(milli(info.iqUA)) mA" : "\(info.iqUA) µA")
            }
        case let .inductor(info):
            add("Inductance", InductorValue.text(pH: info.pH))
            add("Tolerance", info.tolerance ?? "")
            if info.ratedMA > 0 { add("Rated current", amps(mA: info.ratedMA)) }
            if info.isatMA > 0 { add("Saturation current", amps(mA: info.isatMA)) }
            if info.dcrMicroOhm > 0 { add("DC resistance", microohmsText(info.dcrMicroOhm)) }
            add("Shielding", info.shielding.map(capitalized) ?? "")
        case let .fuse(info):
            if info.isPTC {
                add("Type", "Resettable fuse (PTC)")
                if info.ratedMA > 0 { add("Hold current", amps(mA: info.ratedMA)) }
                if info.tripMA > 0 { add("Trip current", amps(mA: info.tripMA)) }
                if info.dcMV > 0 { add("Maximum voltage", volts(mV: info.dcMV)) }
                if info.breakingA > 0 { add("Maximum current", "\(info.breakingA) A") }
            } else {
                add("Type", "Fuse")
                if info.ratedMA > 0 { add("Rated current", amps(mA: info.ratedMA)) }
                add("Speed", info.speed.map(capitalized) ?? "")
                if info.acMV > 0 { add("Voltage (AC)", volts(mV: info.acMV)) }
                if info.dcMV > 0 { add("Voltage (DC)", volts(mV: info.dcMV)) }
                if info.breakingA > 0 { add("Breaking capacity", "\(info.breakingA) A") }
            }
        case let .relay(info):
            add("Type", relayType(info.type))
            add("Contacts", part.function)
            if info.coilMV > 0 { add("Coil voltage", volts(mV: info.coilMV)) }
            if info.coilMOhm > 0 { add("Coil resistance", milliohmsText(info.coilMOhm)) }
            if info.contactMA > 0 { add("Contact current", amps(mA: info.contactMA)) }
            if info.contactMV > 0 { add("Switching voltage", volts(mV: info.contactMV)) }
            if info.operateUS > 0 { add("Operate time", duration(UInt64(info.operateUS) * 1_000_000)) }
            if info.releaseUS > 0 { add("Release time", duration(UInt64(info.releaseUS) * 1_000_000)) }
            if info.onMOhm > 0 { add("On-resistance", milliohmsText(info.onMOhm)) }
            if info.latching { add("Latching", "Yes") }
        case let .buzzer(info):
            add("Type", info.driver == "indicator" ? "Indicator (built-in driver)"
                                                    : "Transducer (external drive)")
            add("Technology", capitalized(info.technology))
            if info.hz > 0 { add("Frequency", hertz(UInt64(info.hz))) }
            if info.ratedMV > 0 { add("Rated voltage", volts(mV: info.ratedMV)) }
            if info.minMV > 0, info.maxMV > 0 {
                add("Voltage range", "\(milli(info.minMV))–\(milli(info.maxMV)) V")
            }
            if info.uA > 0 { add("Current", info.uA >= 1000 ? amps(mA: info.uA / 1000) : "\(info.uA) µA") }
            if info.splCDB > 0 { add("Sound level", "\(trimmed(Double(info.splCDB) / 100, decimals: 1)) dB") }
        case let .timer(info):
            if info.timers > 1 { add("Timers", String(info.timers)) }
            add("Process", info.process == "cmos" ? "CMOS" : "Bipolar")
            if info.maxHz > 0 { add("Max frequency", hertz(info.maxHz)) }
        case let .opamp(info):
            if info.amplifiers > 1 { add("Amplifiers", String(info.amplifiers)) }
            add("Input", opampType(info.type))
            if info.gbwHz > 0 { add("Gain bandwidth", hertz(info.gbwHz)) }
            if info.slewMvUs > 0 { add("Slew rate", "\(trimmed(Double(info.slewMvUs) / 1000, decimals: 3)) V/µs") }
            if info.vosUv > 0 {
                add("Offset (max)", info.vosUv >= 1000 ? "\(trimmed(Double(info.vosUv) / 1000, decimals: 3)) mV"
                                                       : "\(info.vosUv) µV")
            }
            if info.railIn || info.railOut {
                add("Rail-to-rail", info.railIn && info.railOut ? "Input and output" : info.railOut ? "Output" : "Input")
            }
            add("Play mode", "Not simulated: drives nothing")
        case let .regulator(info):
            add("Type", regulatorType(info))
            if info.regulators > 1 { add("Regulators", String(info.regulators)) }
            let outputs = info.voutMV.filter { $0 > 0 }
            if !outputs.isEmpty {
                add("Output", outputs.map { "\(milli($0)) V" }.joined(separator: " / "))
            } else if info.adjustable {
                let refs = info.vrefMV.filter { $0 > 0 }
                var text = "Adjustable"
                if let ref = refs.first { text += ", Vref \(milli(ref)) V" }
                if info.voutMaxMV > 0 { text += " (to \(milli(info.voutMaxMV)) V)" }
                add("Output", text)
            }
            if info.vinMaxMV > 0 {
                add("Input", info.vinMinMV > 0 ? "\(milli(info.vinMinMV))–\(milli(info.vinMaxMV)) V"
                                               : "up to \(milli(info.vinMaxMV)) V")
            } else if info.vinMinMV > 0 {
                add("Input", "from \(milli(info.vinMinMV)) V")
            }
            if info.type == "linear", info.dropoutMV > 0 { add("Dropout", "\(milli(info.dropoutMV)) V") }
            if info.ioutMA > 0 { add("Current", "\(milli(info.ioutMA)) A") }
            if info.iqUA > 0 {
                add("Quiescent", info.iqUA >= 1000 ? "\(milli(info.iqUA)) mA" : "\(info.iqUA) µA")
            }
            if info.fswHz > 0 { add("Switching", hertz(info.fswHz)) }
        case let .socket(family, rowUM, mounting, finish):
            // Pins is the overview row above; the row spacing is a DIP's, in inches (plan-51-F).
            add("Family", family)
            if rowUM > 0 { add("Row spacing", String(format: "%.1f in", Double(rowUM) / 25400)) }
            add("Mounting", mounting)
            add("Finish", finish ?? "")
        }
        if !part.isSimulated { add("Simulation", "Not simulated") }

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
        case PART_LEDDRIVER: return "LED driver"
        case PART_LCDDRIVER: return "LCD driver"
        case PART_SERIALMEM: return "Serial memory"
        case PART_DIODE: return "Diode"
        case PART_CAPACITOR: return "Capacitor"
        case PART_REGISTER: return "Register"
        case PART_MCU: return "Microcontroller"
        case PART_PLD: return "Programmable logic"
        case PART_RESISTOR: return "Resistor"
        case PART_TRANSISTOR: return "Transistor"
        case PART_GENERIC: return "Part"
        case PART_NIXIE: return "Nixie tube"
        case PART_DEKATRON: return "Dekatron"
        case PART_LCD: return "LCD glass"
        case PART_REGULATOR: return "Voltage regulator"
        case PART_POWERSWITCH: return "Power switch"
        case PART_CRYSTAL: return "Crystal"
        case PART_SOCKET: return "IC socket"
        case PART_INDUCTOR: return "Inductor"
        case PART_FUSE: return "Fuse"
        case PART_RELAY: return "Relay"
        case PART_BUZZER: return "Buzzer"
        case PART_TIMER: return "Timer"
        case PART_OPAMP: return "Op-amp"
        default: return ""
        }
    }

    /// A part's kind as shown: a tactile switch as "Tactile switch", a
    /// converter by its type ("ADC", "DAC"), other
    /// kinds as `kindName(_: PartKind)`.
    /// A terminal block's clamp as the backend names it (`'s'`, `'c'`, `'p'`,
    /// `'h'`): "screw", "spring", "push-in" or "pluggable" (plan-45).
    static func clampWord(_ code: CChar) -> String {
        switch UnicodeScalar(UInt8(bitPattern: code)) {
        case "s": "screw"
        case "c": "spring"
        case "p": "push-in"
        default: "pluggable"
        }
    }

    /// The clamp word as the sidebar and the chip show it.
    static func clampName(_ word: String) -> String {
        switch word {
        case "screw": "Screw"
        case "spring": "Spring-cage"
        case "push-in": "Push-in"
        default: "Plug-in header"
        }
    }

    /// A JST header's series from the backend's first letter (`'X'` …).
    static func jstSeries(_ code: CChar) -> String {
        switch UnicodeScalar(UInt8(bitPattern: code)) {
        case "P": "PH"
        case "E": "EH"
        case "Z": "ZH"
        case "S": "SH"
        case "G": "GH"
        case "V": "VH"
        default: "XH"
        }
    }

    /// An op-amp's input type as the sidebar shows it.
    static func opampType(_ type: String) -> String {
        switch type {
        case "jfet": "JFET"
        case "cmos": "CMOS"
        default: "General purpose"
        }
    }

    static func kindName(_ part: CatalogPart) -> String {
        if part.isTactileSwitch { return "Tactile switch" }
        if case let .resistor(_, _, _, resistors) = part.details, resistors > 1 { return "Resistor array" }
        if let info = part.transistor, info.elements > 1 { return "Transistor array" }
        if let info = part.regulator {
            switch info.regulators {
            case 2: return "Dual voltage regulator"
            case 3: return "Triple voltage regulator"
            case 4: return "Quad voltage regulator"
            default: break
            }
        }
        if let info = part.powerswitch { return powerSwitchName(info) }
        if let info = part.timer, info.timers == 2 { return "Dual timer" }
        if let info = part.opamp {
            switch info.amplifiers {
            case 2: return "Dual op-amp"
            case 4: return "Quad op-amp"
            default: break
            }
        }
        if let info = part.crystal, info.type == "resonator" { return "Ceramic resonator" }
        if let info = part.fuse, info.isPTC { return "Resettable fuse" }
        if let info = part.relay {
            switch info.type {
            case "solid-state": return "Solid-state relay"
            case "reed": return "Reed relay"
            default: break
            }
        }
        if case .connector(_, _, _, _, _?) = part.details { return "USB connector" }
        if case let .wireConnector(kind, _, _, _, _) = part.details {
            return kind == "jst" ? "JST connector" : "Terminal block"
        }
        if part.isCardSlot { return "Card slot" }
        if part.isTestPoint { return "Test point" }
        if case let .converter(type, _, _) = part.details {
            switch type {
            case "adc": return "ADC"
            case "dac": return "DAC"
            default: return type.uppercased()
            }
        }
        return kindName(part.kind)
    }

    /// A serial memory's type as shown: "Serial EEPROM", "F-RAM", "NOR flash",
    /// "DataFlash", "Page EEPROM"; others raw.
    static func serialMemoryType(_ type: String) -> String {
        switch type {
        case "eeprom": return "Serial EEPROM"
        case "fram": return "F-RAM"
        case "flash": return "NOR flash"
        case "dataflash": return "DataFlash"
        case "pageeeprom": return "Page EEPROM"
        default: return type
        }
    }

    /// A serial memory's capacity as its datasheet states it: 512 bytes →
    /// "4 Kbit", 262144 bytes → "2 Mbit"; a size that is not a whole
    /// Kbit is in bytes.
    static func density(_ size: UInt32) -> String {
        let bits = UInt64(size) * 8
        if bits >= 1 << 20, bits % (1 << 20) == 0 { return "\(bits >> 20) Mbit" }
        if bits >= 1 << 10, bits % (1 << 10) == 0 { return "\(bits >> 10) Kbit" }
        return "\(size) bytes"
    }

    /// A time in picoseconds, in the unit that reads best: 5 000 000 000 →
    /// "5 ms", 1 400 000 → "1.4 µs", 250 000 → "250 ns"; 0 → "" (no row).
    static func duration(_ ps: UInt64) -> String {
        if ps == 0 { return "" }
        let value = Double(ps)
        if value >= 1e12 { return "\(trimmed(value / 1e12, decimals: 3)) s" }
        if value >= 1e9 { return "\(trimmed(value / 1e9, decimals: 3)) ms" }
        if value >= 1e6 { return "\(trimmed(value / 1e6, decimals: 3)) µs" }
        return "\(trimmed(value / 1e3, decimals: 3)) ns"
    }

    /// Write cycles: 1 000 000 → "1 million cycles", 100 000 000 000 000 →
    /// "100 trillion cycles", 100 000 → "100,000 cycles".
    static func enduranceText(_ cycles: UInt64) -> String {
        let scales: [(UInt64, String)] = [(1_000_000_000_000, "trillion"), (1_000_000_000, "billion"),
                                          (1_000_000, "million")]
        for (scale, word) in scales where cycles >= scale && cycles % (scale / 10) == 0 {
            return "\(trimmed(Double(cycles) / Double(scale), decimals: 2)) \(word) cycles"
        }
        let formatter = NumberFormatter()
        formatter.numberStyle = .decimal
        formatter.locale = Locale(identifier: "en_US")
        return "\(formatter.string(from: NSNumber(value: cycles)) ?? String(cycles)) cycles"
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

    /// A transistor's type as shown (plan-40-H): "NPN", "PNP Darlington",
    /// "NPN pre-biased", "N-channel logic-level", "P-channel"; a
    /// complementary pair "NPN + PNP" ("N-channel + P-channel").
    static func transistorType(_ info: TransistorInfo) -> String {
        if info.mosfet {
            if info.mixed { return "N-channel + P-channel" }
            let channel = info.pType ? "P-channel" : "N-channel"
            return info.logicLevel ? channel + " logic-level" : channel
        }
        if info.mixed { return "NPN + PNP" }
        let polarity = info.pType ? "PNP" : "NPN"
        if info.darlington { return polarity + " Darlington" }
        if info.preBiased { return polarity + " pre-biased" }
        return polarity
    }

    /// A power switch's type as shown (plan-55-J): "Load switch",
    /// "Current-limited switch", "Ideal diode".
    static func powerSwitchType(_ info: PowerSwitchInfo) -> String {
        switch info.type {
        case "load": return "Load switch"
        case "limited": return "Current-limited switch"
        case "ideal-diode": return "Ideal diode"
        default: return info.type.prefix(1).uppercased() + info.type.dropFirst()
        }
    }

    /// A power switch's kind as shown (plan-55-J): the type's name, with
    /// "Dual", "Triple" or "Quad" in front for 2 to 4 switches ("Dual load
    /// switch", "Quad current-limited switch", "Dual ideal diode").
    static func powerSwitchName(_ info: PowerSwitchInfo) -> String {
        let type = powerSwitchType(info)
        let count: String
        switch info.switches {
        case 2: count = "Dual"
        case 3: count = "Triple"
        case 4: count = "Quad"
        default: return type
        }
        return count + " " + type.prefix(1).lowercased() + type.dropFirst()
    }

    /// A regulator's type as shown (plan-42-I): "Linear", "Buck", "Boost",
    /// "Buck-boost", "Charge pump", "SEPIC"; a part whose elements differ
    /// "<first> + other".
    static func regulatorType(_ info: RegulatorInfo) -> String {
        let name: String
        switch info.type {
        case "linear": name = "Linear"
        case "buck": name = "Buck"
        case "boost": name = "Boost"
        case "buck-boost": name = "Buck-boost"
        case "charge-pump": name = "Charge pump"
        case "sepic": name = "SEPIC"
        default: name = info.type.prefix(1).uppercased() + info.type.dropFirst()
        }
        return info.mixed ? name + " + other" : name
    }

    /// A resistance in milliohms as shown (plan-40-H): 8 → "8 mΩ",
    /// 5000 → "5 Ω", 10000000 → "10 kΩ".
    static func milliohmsText(_ milliohms: UInt64) -> String {
        if milliohms < 1000 { return "\(milliohms) mΩ" }
        let (number, unit) = ResistorValue.split(Double(milliohms) / 1000)
        return ResistorValue.numberText(number) + " " + unit.symbol
    }

    /// A current in milliamps as shown (plan-47-M): 500 → "500 mA", 1600 → "1.6 A".
    static func amps(mA: UInt32) -> String {
        mA < 1000 ? "\(mA) mA" : "\(trimmed(Double(mA) / 1000, decimals: 3)) A"
    }

    /// A voltage in millivolts as shown (plan-47-M): 250000 → "250 V", 500 → "0.5 V".
    static func volts(mV: UInt32) -> String {
        "\(milli(mV)) V"
    }

    /// A resistance in micro-ohms as shown (plan-47-M): 700 → "700 µΩ",
    /// 29000 → "29 mΩ", 1500000 → "1.5 Ω", 2200000000 → "2.2 kΩ".
    static func microohmsText(_ microohms: UInt64) -> String {
        if microohms < 1000 { return "\(microohms) µΩ" }
        if microohms < 1_000_000 { return "\(trimmed(Double(microohms) / 1000, decimals: 3)) mΩ" }
        let (number, unit) = ResistorValue.split(Double(microohms) / 1e6)
        return ResistorValue.numberText(number) + " " + unit.symbol
    }

    /// "shielded" → "Shielded", "semi-shielded" → "Semi-shielded".
    static func capitalized(_ word: String) -> String {
        word.prefix(1).uppercased() + word.dropFirst()
    }

    /// A relay's type as shown (plan-47-M).
    static func relayType(_ type: String) -> String {
        switch type {
        case "electromechanical": return "Electromechanical"
        case "reed": return "Reed"
        case "solid-state": return "Solid-state"
        default: return capitalized(type)
        }
    }

    /// A resistor's tolerance as shown (plan-34-G): 1 → "1%", 0.1 → "0.1%".
    static func percentText(_ percent: Double) -> String {
        String(format: "%g%%", percent)
    }

    /// A resistor's power as shown (plan-34-G): 0.1 → "0.1 W", 0.0625 → "0.0625 W".
    static func wattsText(_ watts: Double) -> String {
        String(format: "%g W", watts)
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

    /// A microcontroller core as shown (plan-36): "AVR", "AVR XMEGA",
    /// "Dual Cortex-M0+", "RISC-V", …; others raw.
    static func mcuCoreName(_ core: String) -> String {
        switch core {
        case "avr": return "AVR"
        case "avrrc": return "AVR (reduced core)"
        case "avrxt": return "AVRxt"
        case "avrxm": return "AVR XMEGA"
        case "cortex-m0plus": return "Dual Cortex-M0+"
        case "xtensa-lx6": return "Xtensa LX6"
        case "xtensa-lx7": return "Xtensa LX7"
        case "riscv": return "RISC-V"
        default: return core
        }
    }

    /// A memory size in binary units: 512 → "512 bytes", 32768 → "32 KB",
    /// 8388608 → "8 MB"; 0 → "" (the row is left out).
    static func bytes(_ count: UInt32) -> String {
        let value = Double(count)
        if count == 0 { return "" }
        if value >= 1_048_576 { return "\(trimmed(value / 1_048_576, decimals: 2)) MB" }
        if value >= 1024 { return "\(trimmed(value / 1024, decimals: 2)) KB" }
        return "\(count) bytes"
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
