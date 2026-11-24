import SwiftUI

/// When a key is worth showing in the Part Wizard (plan-46-I): only while a
/// sibling key holds one of these values. Display only: a hidden key keeps
/// its value and is saved; All fields shows every key.
struct FieldCondition: Sendable, Equatable {
    /// The key beside this one that decides (`style`).
    let sibling: String
    /// The sibling's values this key belongs to.
    let values: Set<String>
}

extension PartKinds {
    /// The `bus` keys each bus style uses (docs/parts-format.md § `converter`
    /// › `bus`); a key in several styles is shown for each.
    private static let busStyles: [String: [String]] = [
        "spi": ["cs", "sclk", "mosi", "miso", "mode", "csActive"],
        "i2c": ["scl", "sda", "address", "table"],
        "parallel": ["data", "cs", "rd", "wr", "rw", "address", "byteSelect"],
        "clocked": ["clock", "data", "oe", "clockOut"],
    ]

    private static let busConditions: [String: FieldCondition] = {
        var styles: [String: Set<String>] = [:]
        for (style, keys) in busStyles {
            for key in keys { styles["bus.\(key)", default: []].insert(style) }
        }
        return styles.mapValues { FieldCondition(sibling: "style", values: $0) }
    }()

    /// Kind → dotted path of a key (no side: `"bus.cs"`) → its condition.
    /// `converter`, `rtc` and the display drivers share the bus loader, so
    /// they share the table.
    static let conditions: [String: [String: FieldCondition]] = [
        "converter": busConditions,
        "rtc": busConditions,
        "leddriver": busConditions,
        "lcddriver": busConditions,
    ]

    /// One plain sentence per model of each kind with variants, for the
    /// model picker (plan-46-M fills it in; a missing blurb shows the value
    /// alone).
    static func variantBlurb(kind: String, value: String) -> String? {
        variantBlurbs[kind]?[value]
    }
}

/// How the fields of one object are laid out in Guided mode.
struct GuidedFieldLayout: Equatable {
    /// The deciding keys (`style`), always first.
    var leading: [String] = []
    /// Conditioned keys whose condition holds.
    var shown: [String] = []
    /// Conditioned keys hidden because another value is chosen.
    var hidden: [String] = []
    /// Keys with no condition, under "More … settings". Empty for an object
    /// with no conditions at all (its fields are then all `shown`).
    var more: [String] = []

    /// The layout of an object's fields. `path` is the object's dotted path
    /// without its side (`"bus"`); `object` holds its values.
    static func make(fields: [Field], path: String, conditions: [String: FieldCondition],
                     object: [String: JSONValue]) -> GuidedFieldLayout {
        let own = fields.map { field in (field.key, conditions[path.isEmpty ? field.key : "\(path).\(field.key)"]) }
        guard own.contains(where: { $0.1 != nil }) else {
            return GuidedFieldLayout(shown: fields.map(\.key))
        }
        let siblings = Set(own.compactMap { $0.1?.sibling })
        var layout = GuidedFieldLayout()
        for (key, condition) in own {
            if siblings.contains(key) {
                layout.leading.append(key)
            } else if let condition {
                var value = ""
                if case let .string(text)? = object[condition.sibling] { value = text }
                if condition.values.contains(value) { layout.shown.append(key) } else { layout.hidden.append(key) }
            } else {
                layout.more.append(key)
            }
        }
        return layout
    }
}

/// Where a `FieldsEditor` is in a kind's keys while the Part Wizard shows it:
/// nil in All fields mode, where nothing is hidden and nothing is explained.
struct GuidedFields: Equatable {
    let kind: String
    /// Side-qualified dotted path of the object being edited: `"device"`,
    /// `"device.bus"`.
    var path: String
    /// The object's label, for "More bus settings".
    var label: String = ""

    /// The path without its side: `""` at the top, `"bus"`, `"bus.spi"`.
    var bare: String {
        guard let dot = path.firstIndex(of: ".") else { return "" }
        return String(path[path.index(after: dot)...])
    }

    func descending(_ key: String, label: String) -> GuidedFields {
        GuidedFields(kind: kind, path: "\(path).\(key)", label: label)
    }
}

extension EnvironmentValues {
    /// Set by a wizard step around the kind's fields.
    @Entry var guidedFields: GuidedFields? = nil
}
