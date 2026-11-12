import Foundation

/// One pin row of a pinout (docs/parts-format.md § Pinouts and pins).
struct PinDraft: Hashable, Identifiable, Sendable {
    var id = UUID()
    /// The pin number (numbered pinouts) or ball name ("A1", ball pinouts).
    var designator: String
    var name: String
    var role: String
    var group: String = ""
    /// "" = auto, "in" or "out".
    var side: String = ""

    static let roles = ["input", "output", "inout", "power", "ground", "nc", "absent"]

    // The id is the row's identity in the form, not part of the pin.
    static func == (a: PinDraft, b: PinDraft) -> Bool {
        a.designator == b.designator && a.name == b.name && a.role == b.role && a.group == b.group && a.side == b.side
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(designator)
        hasher.combine(name)
        hasher.combine(role)
        hasher.combine(group)
        hasher.combine(side)
    }
}

/// One named pinout of the device.
struct PinoutDraft: Hashable, Identifiable, Sendable {
    enum Style: String, Hashable, Sendable { case numbered, ball }

    var id = UUID()
    var name: String
    var style: Style
    var pins: [PinDraft]

    static func == (a: PinoutDraft, b: PinoutDraft) -> Bool {
        a.name == b.name && a.style == b.style && a.pins == b.pins
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(name)
        hasher.combine(style)
        hasher.combine(pins)
    }
}

enum PartDraftError: LocalizedError, Equatable {
    case notAPartFile(String)

    var errorDescription: String? {
        switch self {
        case let .notAPartFile(detail): "This is not a part file (\(detail))."
        }
    }
}

/// The Part Editor's model of one single part file (plan-32-A § 4.1): typed
/// fields for the keys every kind has, the kind's own keys (edited through its
/// `KindSchema`), and every other key kept unchanged in `extras`, so opening
/// and saving a file never drops data. `init(file:)` and `encoded()` are
/// inverses for every key a file has.
struct PartDraft: Equatable, Sendable {
    // File
    var uuid: String
    var kind: String
    /// Inline footprints (plan-32-D), kept as they are.
    var footprints: [JSONValue]? = nil

    // Device
    var deviceID: String
    var function: String
    var description: String
    var drawer: [String]
    var note: String? = nil
    var pinouts: [PinoutDraft]

    // Part
    var name: String
    var package: String
    var pinout: String
    var flags: [String] = []
    var footprint: String? = nil
    var manufacturer: String? = nil
    var datasheet: String? = nil

    /// The kind's own device and part keys (its `KindSchema`).
    var kindDevice: [String: JSONValue] = [:]
    var kindPart: [String: JSONValue] = [:]

    /// Keys with no control: kept and written back as they were.
    var deviceExtras: [String: JSONValue] = [:]
    var partExtras: [String: JSONValue] = [:]
    var fileExtras: [String: JSONValue] = [:]

    // MARK: Kinds

    /// Every kind name, sorted (the backend's kinds, one part file each).
    static let kindNames = PartKinds.all.sorted()

    /// The kinds the editor has a form for.
    static var enabledKinds: [String] {
        kindNames.filter { PartKinds.schemas[$0] != nil }
    }

    static func displayName(of kind: String) -> String {
        PartKinds.displayNames[kind] ?? kind
    }

    /// The generic keys every kind's form edits.
    static let genericDeviceKeys: Set<String> = ["id", "function", "description", "drawer", "note", "pinouts", "parts"]
    static let genericPartKeys: Set<String> = ["name", "package", "pinout", "flags", "footprint", "manufacturer", "datasheet"]
    static let pinKeys: Set<String> = ["number", "ball", "name", "role", "group", "side"]

    /// Every key the form has a control for, for a kind: the generic ones and
    /// the kind schema's (nested ones included). Empty for a kind with no form.
    static func formKeys(_ kind: String) -> Set<String> {
        guard let schema = PartKinds.schemas[kind] else { return [] }
        return genericDeviceKeys.union(genericPartKeys).union(pinKeys).union(schema.keys)
    }

    static let flagNames = ["obsolete", "military", "automotive", "enhanced", "ceramic"]

    var schema: KindSchema? { PartKinds.schemas[kind] }

    /// The current variant (`model`/`type`) for kinds that have one.
    var variant: String? {
        guard let key = schema?.variantKey, case let .string(value)? = kindDevice[key] else { return nil }
        return value
    }

    // MARK: A new part

    /// A blank part of a kind: one empty numbered pinout, a new uuid.
    init(newOf kind: String) {
        uuid = UUID().uuidString
        self.kind = kind
        deviceID = "custom"
        function = ""
        description = ""
        drawer = ["Custom"]
        pinouts = [PinoutDraft(name: "default", style: .numbered, pins: [])]
        name = ""
        package = ""
        pinout = "default"
        manufacturer = "Generic"
        if let schema = PartKinds.schemas[kind] {
            for field in schema.device where field.required {
                kindDevice[field.key] = field.type.emptyValue
            }
            for field in schema.part where field.required {
                kindPart[field.key] = field.type.emptyValue
            }
        }
    }

    // MARK: Reading a file

    init(file: JSONValue) throws {
        guard case let .object(root) = file else { throw PartDraftError.notAPartFile("not a JSON object") }
        guard case let .string(uuid)? = root["uuid"] else { throw PartDraftError.notAPartFile("no uuid") }
        guard case let .string(kind)? = root["kind"] else { throw PartDraftError.notAPartFile("no kind") }
        guard case let .array(devices)? = root["devices"], devices.count == 1,
              case var .object(device)? = devices.first,
              case let .array(parts)? = device["parts"], parts.count == 1,
              case var .object(part)? = parts.first else {
            throw PartDraftError.notAPartFile("not exactly one device with one part")
        }
        self.uuid = uuid
        self.kind = kind
        if case let .array(footprints)? = root["footprints"] { self.footprints = footprints }
        fileExtras = root.filter { !["format", "version", "uuid", "kind", "devices", "footprints"].contains($0.key) }
        if root["footprints"] != nil, footprints == nil { fileExtras["footprints"] = root["footprints"] }

        func take(_ object: inout [String: JSONValue], _ key: String) -> JSONValue? {
            object.removeValue(forKey: key)
        }
        func string(_ value: JSONValue?) -> String? {
            if case let .string(text)? = value { return text }
            return nil
        }
        func strings(_ value: JSONValue?) -> [String]? {
            guard case let .array(items)? = value else { return nil }
            let texts = items.compactMap { item -> String? in
                if case let .string(text) = item { return text }
                return nil
            }
            return texts.count == items.count ? texts : nil
        }
        /// Takes a key only when it has the expected shape; otherwise it stays an extra.
        func takeString(_ object: inout [String: JSONValue], _ key: String) -> String? {
            guard let text = string(object[key]) else { return nil }
            object.removeValue(forKey: key)
            return text
        }

        _ = take(&device, "parts")
        deviceID = takeString(&device, "id") ?? ""
        function = takeString(&device, "function") ?? ""
        description = takeString(&device, "description") ?? ""
        if let drawer = strings(device["drawer"]) {
            self.drawer = drawer
            device.removeValue(forKey: "drawer")
        } else {
            drawer = []
        }
        note = takeString(&device, "note")
        if let pinouts = Self.pinouts(device["pinouts"]) {
            self.pinouts = pinouts
            device.removeValue(forKey: "pinouts")
        } else {
            pinouts = []
        }

        name = takeString(&part, "name") ?? ""
        package = takeString(&part, "package") ?? ""
        pinout = takeString(&part, "pinout") ?? ""
        if let flags = strings(part["flags"]) {
            self.flags = flags
            part.removeValue(forKey: "flags")
        }
        footprint = takeString(&part, "footprint")
        manufacturer = takeString(&part, "manufacturer")
        datasheet = takeString(&part, "datasheet")

        if let schema = PartKinds.schemas[kind] {
            for field in schema.device {
                if let value = device.removeValue(forKey: field.key) { kindDevice[field.key] = value }
            }
            for field in schema.part {
                if let value = part.removeValue(forKey: field.key) { kindPart[field.key] = value }
            }
        }
        deviceExtras = device
        partExtras = part
    }

    /// The pinouts object, or nil when it is not in the expected shape (it
    /// then stays an extra).
    private static func pinouts(_ value: JSONValue?) -> [PinoutDraft]? {
        guard case let .object(map)? = value else { return nil }
        var result: [PinoutDraft] = []
        for name in map.keys.sorted() {
            guard case let .array(pins)? = map[name] else { return nil }
            var drafts: [PinDraft] = []
            var style: PinoutDraft.Style = .numbered
            for pin in pins {
                guard case let .object(keys) = pin, Set(keys.keys).isSubset(of: pinKeys) else { return nil }
                guard case let .string(pinName)? = keys["name"], case let .string(role)? = keys["role"] else { return nil }
                var designator: String
                if case let .number(number)? = keys["number"], keys["ball"] == nil,
                   number == number.rounded(), number >= 0 {
                    designator = String(Int(number))
                    style = .numbered
                } else if case let .string(ball)? = keys["ball"], keys["number"] == nil {
                    designator = ball
                    style = .ball
                } else {
                    return nil
                }
                var draft = PinDraft(designator: designator, name: pinName, role: role)
                if let group = keys["group"] {
                    guard case let .string(text) = group, !text.isEmpty else { return nil }
                    draft.group = text
                }
                if let side = keys["side"] {
                    guard case let .string(text) = side, !text.isEmpty else { return nil }
                    draft.side = text
                }
                drafts.append(draft)
            }
            result.append(PinoutDraft(name: name, style: style, pins: drafts))
        }
        return result
    }

    // MARK: Writing a file

    func encoded() -> JSONValue {
        var device = deviceExtras
        device["id"] = .string(deviceID)
        device["function"] = .string(function)
        device["description"] = .string(description)
        device["drawer"] = .array(drawer.map { .string($0) })
        if let note { device["note"] = .string(note) }
        device["pinouts"] = .object(Dictionary(pinouts.map { pinout in
            (pinout.name, JSONValue.array(pinout.pins.map { pin in
                var keys: [String: JSONValue] = ["name": .string(pin.name), "role": .string(pin.role)]
                if pinout.style == .ball {
                    keys["ball"] = .string(pin.designator)
                } else {
                    keys["number"] = .number(Double(Int(pin.designator) ?? 0))
                }
                if !pin.group.isEmpty { keys["group"] = .string(pin.group) }
                if !pin.side.isEmpty { keys["side"] = .string(pin.side) }
                return .object(keys)
            }))
        }, uniquingKeysWith: { first, _ in first }))
        device.merge(kindDevice) { _, new in new }

        var part = partExtras
        part["name"] = .string(name)
        part["package"] = .string(package)
        part["pinout"] = .string(pinout)
        if !flags.isEmpty { part["flags"] = .array(flags.map { .string($0) }) }
        if let footprint { part["footprint"] = .string(footprint) }
        if let manufacturer { part["manufacturer"] = .string(manufacturer) }
        if let datasheet { part["datasheet"] = .string(datasheet) }
        part.merge(kindPart) { _, new in new }
        device["parts"] = .array([.object(part)])

        var root = fileExtras
        root["format"] = .string("schematical-part")
        root["version"] = .number(1)
        root["uuid"] = .string(uuid)
        root["kind"] = .string(kind)
        if let footprints { root["footprints"] = .array(footprints) }
        root["devices"] = .array([.object(device)])
        return .object(root)
    }

    /// The file's text: sorted keys, two-space indent.
    func fileText() -> String {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys, .withoutEscapingSlashes]
        guard let data = try? encoder.encode(encoded()) else { return "" }
        return String(decoding: data, as: UTF8.self)
    }

    init(fileText: String) throws {
        let value: JSONValue
        do {
            value = try JSONValue(text: fileText)
        } catch {
            throw PartDraftError.notAPartFile("not JSON")
        }
        try self.init(file: value)
    }

    /// All keys left without a control (`extras`), as dotted paths, for the
    /// round-trip tests (plan-32-F … J require none for a kind with a form).
    var extraKeys: [String] {
        var keys = fileExtras.keys.map { $0 } + deviceExtras.keys.map { "device.\($0)" }
            + partExtras.keys.map { "part.\($0)" }
        if let schema {
            keys += Field.unknownKeys(in: kindDevice, fields: schema.device, path: "device")
            keys += Field.unknownKeys(in: kindPart, fields: schema.part, path: "part")
        }
        return keys.sorted()
    }

    /// Pin names in every pinout, for pin pickers.
    var pinNames: [String] {
        var seen = Set<String>()
        return pinouts.flatMap(\.pins).map(\.name).filter { seen.insert($0).inserted }
    }

    /// The pin count of the part's pinout.
    var pinCount: Int {
        pinouts.first { $0.name == pinout }?.pins.count ?? 0
    }
}
