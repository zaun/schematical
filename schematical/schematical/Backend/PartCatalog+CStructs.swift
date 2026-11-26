import Foundation
import Observation

// MARK: - Reading the C structs

nonisolated func string(_ pointer: UnsafePointer<CChar>?) -> String {
    pointer.map { String(cString: $0) } ?? ""
}

nonisolated func pinRole(_ role: PinRole) -> ChipPinRole {
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

nonisolated func pinSide(_ side: PinSide) -> ChipPinSide {
    switch side {
    case PIN_SIDE_IN: return .input
    case PIN_SIDE_OUT: return .output
    default: return .auto
    }
}

nonisolated func indices(_ pointer: UnsafePointer<Int>?, _ count: Int) -> [Int] {
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
