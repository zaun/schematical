import Foundation

// MARK: - The part editor's field schema (plan-32-C)

/// What one key of a kind's device or part object holds, and so which control
/// the Part Editor shows for it. The keys and their rules are the backend's
/// (`backend/src/models/<kind>.c`); a schema only says how to edit them.
indirect enum FieldType: Sendable {
    /// A string.
    case text
    /// A longer string (notes).
    case note
    /// A number, with its unit for the label ("ns", "V", "Hz").
    case number(unit: String)
    /// A whole number.
    case integer
    case bool
    /// One of a fixed list of strings.
    case choice([String])
    /// A string that is usually one of these, but may be any.
    case suggested([String])
    /// The name of one of the part's pins.
    case pin
    /// A list of pin names.
    case pins
    /// A list of strings.
    case names
    /// A list of numbers.
    case numbers(unit: String)
    /// `[min, max]` volts.
    case voltRange
    /// An object with these keys.
    case object([Field])
    /// An array of objects with these keys.
    case list([Field])
    /// An object whose keys the user names (`keyLabel`), each an object with these keys.
    case map(keyLabel: String, [Field])
    /// An object whose keys the user names, each a value of this type.
    case valueMap(keyLabel: String, FieldType)
    /// A list of values of this type.
    case listOf(FieldType)
    /// A key the backend accepts in more than one shape: edited as JSON text.
    case json
    /// A key the backend accepts in more than one shape, each with its own
    /// control: a shape picker, then that shape's control.
    case oneOf([FieldType])

    /// The JSON shape this type edits (for `oneOf`).
    var shape: String {
        switch self {
        case .text, .note, .choice, .suggested, .pin: "text"
        case .number, .integer: "number"
        case .bool: "yes/no"
        case .pins, .names, .numbers, .voltRange, .list, .listOf: "list"
        case .object, .map, .valueMap: "group"
        case .json, .oneOf: "JSON"
        }
    }

    /// Whether a value has this type's JSON shape.
    func matches(_ value: JSONValue) -> Bool {
        switch (shape, value) {
        case ("text", .string), ("number", .number), ("yes/no", .bool), ("list", .array), ("group", .object): true
        case ("JSON", _): true
        default: false
        }
    }

    /// The keys of nested objects, for `formKeys`.
    var nestedKeys: Set<String> {
        switch self {
        case let .object(fields), let .list(fields), let .map(_, fields):
            return Field.keys(fields)
        case let .valueMap(_, type), let .listOf(type):
            return type.nestedKeys
        case let .oneOf(types):
            return types.reduce(into: Set<String>()) { $0.formUnion($1.nestedKeys) }
        default:
            return []
        }
    }

    /// A new value for an added entry.
    var emptyValue: JSONValue {
        switch self {
        case .text, .note, .pin, .suggested: return .string("")
        case let .choice(options): return .string(options.first ?? "")
        case .number, .integer: return .number(0)
        case .bool: return .bool(false)
        case .pins, .names, .numbers, .list, .listOf: return .array([])
        case .voltRange: return .array([.number(0), .number(0)])
        case let .object(fields):
            return .object(Dictionary(uniqueKeysWithValues: fields.filter(\.required).map { ($0.key, $0.type.emptyValue) }))
        case .map, .valueMap: return .object([:])
        case .json: return .null
        case let .oneOf(types): return types.first?.emptyValue ?? .null
        }
    }
}

/// One key of an object, with its control.
struct Field: Sendable, Identifiable {
    let key: String
    let label: String
    let type: FieldType
    /// Required keys are always written; optional ones are left out when empty.
    var required = false
    /// One line under the control.
    var help: String? = nil
    /// For a device of a kind with variants: the values of `variantKey`
    /// (`model`, `type`) this key belongs to; nil = every variant.
    var variants: Set<String>? = nil

    var id: String { key }

    static func keys(_ fields: [Field]) -> Set<String> {
        fields.reduce(into: Set<String>()) { keys, field in
            keys.insert(field.key)
            keys.formUnion(field.type.nestedKeys)
        }
    }
}

/// A kind's keys beyond the generic device and part keys: what the kind's
/// section of the editor edits.
struct KindSchema: Sendable {
    /// The kind name in part files (`"gate"`).
    let kind: String
    /// The device key that picks a variant (`"model"`), if the kind has one.
    var variantKey: String? = nil
    let device: [Field]
    let part: [Field]

    /// Every key the schema has a control for, nested ones included.
    var keys: Set<String> {
        Field.keys(device).union(Field.keys(part))
    }

    /// The device fields shown for a variant (every field when the kind has
    /// no variants or no variant is chosen).
    func deviceFields(variant: String?) -> [Field] {
        guard variantKey != nil, let variant else { return device }
        return device.filter { $0.variants == nil || $0.variants!.contains(variant) }
    }
}

// MARK: - Keys a value has that its field has no control for

extension FieldType {
    /// Dotted paths of object keys inside `value` this type has no control
    /// for. They are kept (the value is stored whole) but not editable, so
    /// the round-trip tests count them as extras.
    func unknownKeys(in value: JSONValue, path: String) -> [String] {
        // A value of another shape has no control either.
        guard matches(value) else { return ["\(path) (shape)"] }
        switch (self, value) {
        case let (.object(fields), .object(object)):
            return Field.unknownKeys(in: object, fields: fields, path: path)
        case let (.list(fields), .array(items)):
            return items.enumerated().flatMap { index, item -> [String] in
                guard case let .object(object) = item else { return ["\(path)[\(index)] (shape)"] }
                return Field.unknownKeys(in: object, fields: fields, path: "\(path)[\(index)]")
            }
        case let (.map(_, fields), .object(object)):
            return object.flatMap { key, item -> [String] in
                guard case let .object(entry) = item else { return ["\(path).\(key) (shape)"] }
                return Field.unknownKeys(in: entry, fields: fields, path: "\(path).\(key)")
            }
        case let (.valueMap(_, type), .object(object)):
            return object.flatMap { key, item in type.unknownKeys(in: item, path: "\(path).\(key)") }
        case let (.listOf(type), .array(items)):
            return items.enumerated().flatMap { index, item in type.unknownKeys(in: item, path: "\(path)[\(index)]") }
        case let (.oneOf(types), value):
            guard let type = types.first(where: { $0.matches(value) }) else { return ["\(path) (shape)"] }
            return type.unknownKeys(in: value, path: path)
        default:
            return []
        }
    }
}

extension Field {
    static func unknownKeys(in object: [String: JSONValue], fields: [Field], path: String) -> [String] {
        let byKey = Dictionary(fields.map { ($0.key, $0) }, uniquingKeysWith: { first, _ in first })
        return object.flatMap { key, value -> [String] in
            guard let field = byKey[key] else { return ["\(path).\(key)"] }
            return field.type.unknownKeys(in: value, path: "\(path).\(key)")
        }
    }
}

// MARK: - The kinds

enum PartKinds {
    /// The kinds the backend loads (`kinds.c`), as their part files name them.
    static let all = [
        "7seg", "arith", "buffer", "charlcd", "connector", "converter", "counter", "cpu", "decoder",
        "dff", "diode", "eeprom", "gate", "header", "jkff", "latch", "led", "mux", "oscillator",
        "pio", "rtc", "serial", "shift", "sram", "supply", "switch",
    ]

    /// The names shown in the New Part picker and window titles.
    static let displayNames: [String: String] = [
        "7seg": "7-Segment Display", "arith": "Arithmetic", "buffer": "Buffer", "charlcd": "Character LCD",
        "connector": "Connector", "converter": "Converter", "counter": "Counter", "cpu": "Processor",
        "decoder": "Decoder", "dff": "D Flip-Flop", "diode": "Diode", "eeprom": "EEPROM", "gate": "Gate",
        "header": "Pin Header", "jkff": "JK Flip-Flop", "latch": "Latch", "led": "LED", "mux": "Multiplexer",
        "oscillator": "Oscillator", "pio": "Parallel I/O", "rtc": "Real-Time Clock", "serial": "Serial",
        "shift": "Shift Register", "sram": "SRAM", "supply": "Supply", "switch": "Switch",
    ]

    /// Each kind's editor schema; a kind is enabled in the editor once it has one.
    static let schemas: [String: KindSchema] = {
        var schemas: [String: KindSchema] = [:]
        for schema in [
            GateDraft.schema,
            DffDraft.schema, JkffDraft.schema, MuxDraft.schema, DecoderDraft.schema,
            LatchDraft.schema, BufferDraft.schema, ArithDraft.schema,
            CounterDraft.schema, ShiftDraft.schema,
        ] {
            schemas[schema.kind] = schema
        }
        return schemas
    }()
}
