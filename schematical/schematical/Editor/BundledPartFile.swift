import Foundation

/// The copy path (plan-32-C § 4.5): a bundled part as a single part file, so
/// the Part Editor can start from it. Bundled files are never edited; the copy
/// has a new uuid.
enum BundledPartFile {
    /// The bundled part files read so far, by kind.
    @MainActor private static var files: [String: JSONValue] = [:]

    /// The bundled part file of a kind, parsed once.
    @MainActor
    static func file(kind: String) -> JSONValue? {
        if let file = files[kind] { return file }
        guard let url = Bundle.main.url(forResource: kind, withExtension: "json"),
              let data = try? Data(contentsOf: url),
              let file = try? JSONDecoder().decode(JSONValue.self, from: data) else { return nil }
        files[kind] = file
        return file
    }

    /// Every part name of a kind's bundled file, in file order.
    @MainActor
    static func partNames(kind: String) -> [String] {
        guard case let .object(root)? = file(kind: kind), case let .array(devices)? = root["devices"] else { return [] }
        return devices.flatMap { device -> [String] in
            guard case let .object(keys) = device, case let .array(parts)? = keys["parts"] else { return [] }
            return parts.compactMap { part in
                guard case let .object(partKeys) = part, case let .string(name)? = partKeys["name"] else { return nil }
                return name
            }
        }
    }

    /// The single part file for the bundled part `name` of `kind`: its device
    /// with only that part, and a new uuid. Footprints stay bundled (not
    /// inlined). Nil when there is no such part.
    @MainActor
    static func copy(name: String, kind: String, uuid: String = UUID().uuidString) -> JSONValue? {
        guard case let .object(root)? = file(kind: kind), case let .array(devices)? = root["devices"] else { return nil }
        for device in devices {
            guard case var .object(keys) = device, case let .array(parts)? = keys["parts"] else { continue }
            for part in parts {
                guard case let .object(partKeys) = part, partKeys["name"] == .string(name) else { continue }
                keys["parts"] = .array([part])
                return .object([
                    "format": .string("schematical-part"),
                    "version": .number(1),
                    "uuid": .string(uuid),
                    "kind": .string(kind),
                    "devices": .array([.object(keys)]),
                ])
            }
        }
        return nil
    }

    /// The copy of a catalog part, by its kind (`kind_find` names).
    @MainActor
    static func copy(of part: CatalogPart) -> JSONValue? {
        guard let kind = PartKinds.name(of: part.kind) else { return nil }
        return copy(name: part.name, kind: kind)
    }
}

extension PartKinds {
    /// The part file kind name of a backend kind.
    static func name(of kind: PartKind) -> String? {
        switch kind {
        case PART_GATE: "gate"
        case PART_DFF: "dff"
        case PART_JKFF: "jkff"
        case PART_MUX: "mux"
        case PART_DECODER: "decoder"
        case PART_LATCH: "latch"
        case PART_BUFFER: "buffer"
        case PART_ARITH: "arith"
        case PART_COUNTER: "counter"
        case PART_SHIFT: "shift"
        case PART_SRAM: "sram"
        case PART_EEPROM: "eeprom"
        case PART_SWITCH: "switch"
        case PART_LED: "led"
        case PART_7SEG: "7seg"
        case PART_OSCILLATOR: "oscillator"
        case PART_SUPPLY: "supply"
        case PART_CPU: "cpu"
        case PART_PIO: "pio"
        case PART_SERIAL: "serial"
        case PART_CHARLCD: "charlcd"
        case PART_CONNECTOR: "connector"
        case PART_CONVERTER: "converter"
        case PART_RTC: "rtc"
        case PART_DIODE: "diode"
        case PART_HEADER: "header"
        case PART_CAPACITOR: "capacitor"
        case PART_REGISTER: "register"
        default: nil
        }
    }
}
