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
    /// The alternatives of the name (split at "/", as written) that are
    /// active low (plan-39-A). Saved as `true`, a list or nothing; see
    /// `activeLowValue`.
    var activeLow: Set<String> = []

    static let roles = ["input", "output", "inout", "power", "ground", "nc", "absent"]

    // The id is the row's identity in the form, not part of the pin.
    static func == (a: PinDraft, b: PinDraft) -> Bool {
        a.designator == b.designator && a.name == b.name && a.role == b.role && a.group == b.group &&
            a.side == b.side && a.activeLow == b.activeLow
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(designator)
        hasher.combine(name)
        hasher.combine(role)
        hasher.combine(group)
        hasher.combine(side)
        hasher.combine(activeLow)
    }

    static let signalRoles: Set<String> = ["input", "output", "inout"]

    /// The name's alternatives, left to right (a name with no "/" has one).
    var alternatives: [String] {
        name.split(separator: "/", omittingEmptySubsequences: false).map(String.init)
    }

    /// Unique alternatives in name order, for the editor's menu.
    var menuAlternatives: [String] {
        var seen: Set<String> = []
        return alternatives.filter { seen.insert($0).inserted }
    }

    /// The canonical "activeLow" for the file (plan-39-A § 3): `true` when
    /// every alternative is flagged, the flagged ones in name order when some
    /// are, nil when none are or the role takes no flag. Entries that are no
    /// longer an alternative of the name are dropped.
    var activeLowValue: JSONValue? {
        guard Self.signalRoles.contains(role) else { return nil }
        let flagged = menuAlternatives.filter { activeLow.contains($0) }
        if flagged.isEmpty { return nil }
        if flagged.count == menuAlternatives.count { return .bool(true) }
        return .array(flagged.map { .string($0) })
    }
}

/// One of a part's `footprintChoices`: a package name and a bundled footprint id.
struct FootprintChoiceDraft: Hashable, Identifiable, Sendable {
    var id = UUID()
    var package: String
    var footprint: String

    init(package: String, footprint: String) {
        self.package = package
        self.footprint = footprint
    }

    init?(json: JSONValue) {
        guard case let .object(keys) = json, keys.count == 2, case let .string(package)? = keys["package"],
              case let .string(footprint)? = keys["footprint"] else { return nil }
        self.init(package: package, footprint: footprint)
    }

    func encoded() -> JSONValue {
        .object(["package": .string(package), "footprint": .string(footprint)])
    }

    static func == (a: FootprintChoiceDraft, b: FootprintChoiceDraft) -> Bool {
        a.package == b.package && a.footprint == b.footprint
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(package)
        hasher.combine(footprint)
    }
}

/// One inline datasheet of a single part file (plan-46-A): a
/// `datasheets.json` entry without "md".
struct DatasheetDraft: Equatable, Sendable {
    var id: String
    var url: String

    init(id: String, url: String) {
        self.id = id
        self.url = url
    }

    /// nil when the entry is not exactly `{"id", "url"}` strings (the key
    /// then stays an extra, and the loader reports what is wrong with it).
    init?(json: JSONValue) {
        guard case let .object(keys) = json, keys.count == 2, case let .string(id)? = keys["id"],
              case let .string(url)? = keys["url"] else { return nil }
        self.init(id: id, url: url)
    }

    func encoded() -> JSONValue {
        .object(["id": .string(id), "url": .string(url)])
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
    /// Inline datasheets (plan-46-A): `datasheets.json` entries without "md",
    /// their ids new to the library. The part's `datasheet` names one.
    var datasheets: [DatasheetDraft]? = nil

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
    /// The PCB Style option's second footprint (bundled character LCDs).
    var connectorFootprint: String? = nil
    /// The footprints a placed part may use, each with its package name
    /// (bundled parts: the generic LED, headers, pull resistors).
    var footprintChoices: [FootprintChoiceDraft]? = nil

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
        PartKinds.info[kind]?.name ?? kind
    }

    /// The generic keys every kind's form edits.
    static let genericDeviceKeys: Set<String> = ["id", "function", "description", "drawer", "note", "pinouts", "parts"]
    static let genericPartKeys: Set<String> = ["name", "package", "pinout", "flags", "footprint", "manufacturer",
                                               "datasheet", "connectorFootprint", "footprintChoices"]
    static let pinKeys: Set<String> = ["number", "ball", "name", "role", "group", "side", "activeLow"]

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
        if case let .array(entries)? = root["datasheets"] {
            let drafts = entries.compactMap(DatasheetDraft.init(json:))
            if drafts.count == entries.count { datasheets = drafts }
        }
        fileExtras = root.filter {
            !["format", "version", "uuid", "kind", "devices", "footprints", "datasheets"].contains($0.key)
        }
        if root["footprints"] != nil, footprints == nil { fileExtras["footprints"] = root["footprints"] }
        if root["datasheets"] != nil, datasheets == nil { fileExtras["datasheets"] = root["datasheets"] }

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
        connectorFootprint = takeString(&part, "connectorFootprint")
        if case let .array(items)? = part["footprintChoices"] {
            let choices = items.compactMap(FootprintChoiceDraft.init(json:))
            if choices.count == items.count {
                footprintChoices = choices
                part.removeValue(forKey: "footprintChoices")
            }
        }

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
                switch keys["activeLow"] {
                case nil:
                    break
                case .bool(true)?:
                    draft.activeLow = Set(draft.alternatives)
                case let .array(items)? where !items.isEmpty:
                    for item in items {
                        guard case let .string(text) = item else { return nil }
                        draft.activeLow.insert(text)
                    }
                default:
                    return nil
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
                if let activeLow = pin.activeLowValue { keys["activeLow"] = activeLow }
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
        if let connectorFootprint { part["connectorFootprint"] = .string(connectorFootprint) }
        if let footprintChoices { part["footprintChoices"] = .array(footprintChoices.map { $0.encoded() }) }
        part.merge(kindPart) { _, new in new }
        device["parts"] = .array([.object(part)])

        var root = fileExtras
        root["format"] = .string("schematical-part")
        root["version"] = .number(1)
        root["uuid"] = .string(uuid)
        root["kind"] = .string(kind)
        if let footprints { root["footprints"] = .array(footprints) }
        if let datasheets { root["datasheets"] = .array(datasheets.map { $0.encoded() }) }
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

    /// The part's custom footprint (plan-32-D): the file's inline `explicit`
    /// footprint the part's `footprint` names. Setting one makes it the file's
    /// only inline footprint and the part's footprint; setting nil removes the
    /// `footprints` key and leaves the part with no footprint, for the bundled
    /// picker.
    var customFootprint: CustomFootprintDraft? {
        get {
            guard let footprint, let entries = footprints else { return nil }
            for entry in entries {
                if case let .object(keys) = entry, keys["id"] == .string(footprint) {
                    return CustomFootprintDraft(json: entry)
                }
            }
            return nil
        }
        set {
            if let newValue {
                footprints = [newValue.encoded()]
                footprint = newValue.id
            } else {
                if let current = customFootprint, footprint == current.id { footprint = nil }
                footprints = nil
            }
        }
    }

    // MARK: Renaming a pin

    /// Carries a pin's new name into the kind's keys (plan-46-D § 3.3). Kind
    /// fields hold pin names, so a pin renamed in the pin table would leave
    /// them naming a pin that is gone (the loader then says `unknown pin`).
    ///
    /// Call it with the pinout as it was before the rename. When `old` names
    /// exactly one pin of the part's pinout and `new` names none, every
    /// `.pin` value equal to `old` and every element of a `.pins` value equal
    /// to `old` becomes `new`, at any depth the schema reaches. Whole names
    /// only: renaming "A" never touches "A/CS". A `.json` value is left as it
    /// is (there is no schema to read it by). The pin table itself is not
    /// changed here. Returns how many values were rewritten.
    @discardableResult
    mutating func renamePin(from old: String, to new: String) -> Int {
        guard old != new, !old.isEmpty, !new.isEmpty, let schema else { return 0 }
        let pins = pinouts.first { $0.name == pinout }?.pins ?? []
        guard pins.filter({ $0.name == old }).count == 1, !pins.contains(where: { $0.name == new }) else { return 0 }
        var count = 0
        func rewrite(_ object: [String: JSONValue], _ fields: [Field]) -> [String: JSONValue] {
            var result = object
            for field in fields {
                if let value = object[field.key] { result[field.key] = rewrite(value, field.type) }
            }
            return result
        }
        func rewrite(_ value: JSONValue, _ type: FieldType) -> JSONValue {
            switch (type, value) {
            case (.pin, .string(old)):
                count += 1
                return .string(new)
            case let (.pins, .array(items)):
                return .array(items.map { item in
                    guard item == .string(old) else { return item }
                    count += 1
                    return .string(new)
                })
            case let (.object(fields), .object(object)):
                return .object(rewrite(object, fields))
            case let (.list(fields), .array(items)):
                return .array(items.map { item in
                    guard case let .object(object) = item else { return item }
                    return .object(rewrite(object, fields))
                })
            case let (.map(_, fields), .object(object)):
                return .object(object.mapValues { item in
                    guard case let .object(entry) = item else { return item }
                    return .object(rewrite(entry, fields))
                })
            case let (.valueMap(_, inner), .object(object)):
                return .object(object.mapValues { rewrite($0, inner) })
            case let (.listOf(inner), .array(items)):
                return .array(items.map { rewrite($0, inner) })
            case let (.oneOf(types), value):
                guard let inner = types.first(where: { $0.matches(value) }) else { return value }
                return rewrite(value, inner)
            default:
                return value
            }
        }
        kindDevice = rewrite(kindDevice, schema.device)
        kindPart = rewrite(kindPart, schema.part)
        return count
    }

    // MARK: The inline datasheet

    /// The id the part's own (inline) datasheet has: "CUSTOM-" and the first
    /// eight characters of the uuid, as a custom footprint's id.
    var inlineDatasheetID: String { "CUSTOM-" + uuid.prefix(8) }

    /// The URL of the inline datasheet the part names; nil when the part has
    /// no datasheet or names a bundled one.
    var datasheetURL: String? {
        guard let datasheet else { return nil }
        return datasheets?.first { $0.id == datasheet }?.url
    }

    /// Gives the part its own datasheet: one inline entry holding `url`, named
    /// by the part. nil or an empty text removes the inline entry, and the
    /// part's `datasheet` when it named it.
    mutating func setDatasheetURL(_ url: String?) {
        let text = (url ?? "").trimmingCharacters(in: .whitespacesAndNewlines)
        if text.isEmpty {
            if let datasheet, datasheets?.contains(where: { $0.id == datasheet }) == true {
                self.datasheet = nil
            }
            datasheets = nil
        } else {
            let entry = DatasheetDraft(id: inlineDatasheetID, url: text)
            datasheets = [entry]
            datasheet = entry.id
        }
    }

    /// The pin numbers of the part's pinout, for the pad table.
    var pinNumbers: [Int] {
        (pinouts.first { $0.name == pinout }?.pins ?? []).compactMap { Int($0.designator) }
    }

    /// The pin count of the part's pinout.
    var pinCount: Int {
        pinouts.first { $0.name == pinout }?.pins.count ?? 0
    }
}
