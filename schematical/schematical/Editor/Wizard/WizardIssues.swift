import Foundation

// MARK: - What a wizard step still needs (plan-46-E § 1)

extension WizardStep {
    /// What is still missing or wrong on this step, in plain words; empty
    /// when the step is complete. Pure: the step list shows a dot for a step
    /// with issues, and Review lists them all. Issues never stop the user
    /// from moving on.
    func issues(in draft: PartDraft) -> [String] {
        switch page {
        case .kind, .datasheet, .review:
            return []
        case .identity:
            return draft.name.trimmingCharacters(in: .whitespaces).isEmpty ? ["The part has no part number"] : []
        case .pinout:
            guard let pinout = draft.pinouts.first(where: { $0.name == draft.pinout }) else {
                return ["The part's pinout “\(draft.pinout)” does not exist"]
            }
            return pinout.pins.isEmpty ? ["The part has no pins"] : []
        case .footprint:
            return (draft.footprint ?? "").isEmpty ? ["No footprint is chosen"] : []
        case .variant, .supply, .kindFields:
            guard let schema = draft.schema else { return [] }
            let pins = Set(draft.pinNames)
            var issues: [String] = []
            for qualified in keys {
                guard let key = KindKey(qualified: qualified),
                      let field = schema.field(qualified, variant: draft.variant) else { continue }
                let object = key.side == .device ? draft.kindDevice : draft.kindPart
                issues += field.issues(value: object[field.key], pins: pins, path: field.label)
            }
            return issues
        }
    }
}

extension Field {
    /// The issues of one key's value: a required key left empty, and pin
    /// names that are not pins of the part, at any depth the schema reaches.
    func issues(value: JSONValue?, pins: Set<String>, path: String) -> [String] {
        guard let value, !type.isEmpty(value) else {
            return required ? ["\(path) is empty"] : []
        }
        return type.issues(value: value, pins: pins, path: path)
    }

    static func issues(in object: [String: JSONValue], fields: [Field], pins: Set<String>, path: String) -> [String] {
        fields.flatMap { $0.issues(value: object[$0.key], pins: pins, path: "\(path) › \($0.label)") }
    }
}

extension FieldType {
    /// Whether a value is what a new, untouched key of this type holds:
    /// nothing to show for it yet. A number or a yes/no is never empty (0 and
    /// "no" are answers), nor is a fixed choice (it always has one).
    func isEmpty(_ value: JSONValue) -> Bool {
        switch value {
        case .null: return true
        case let .string(text):
            if case .choice = self { return false }
            return text.isEmpty
        case let .array(items):
            if case .voltRange = self { return items == [.number(0), .number(0)] }
            return items.isEmpty
        case let .object(object): return object.isEmpty
        case .number, .bool: return false
        }
    }

    func issues(value: JSONValue, pins: Set<String>, path: String) -> [String] {
        switch (self, value) {
        case let (.pin, .string(name)):
            return pins.contains(name) ? [] : ["\(path): there is no pin named “\(name)”"]
        case let (.pins, .array(items)):
            return items.compactMap { item in
                guard case let .string(name) = item, !pins.contains(name) else { return nil }
                return "\(path): there is no pin named “\(name)”"
            }
        case let (.object(fields), .object(object)):
            return Field.issues(in: object, fields: fields, pins: pins, path: path)
        case let (.list(fields), .array(items)):
            return items.enumerated().flatMap { index, item -> [String] in
                guard case let .object(object) = item else { return [] }
                return Field.issues(in: object, fields: fields, pins: pins, path: "\(path) \(index + 1)")
            }
        case let (.map(_, fields), .object(object)):
            return object.keys.sorted().flatMap { key -> [String] in
                guard case let .object(entry)? = object[key] else { return [] }
                return Field.issues(in: entry, fields: fields, pins: pins, path: "\(path) \(key)")
            }
        case let (.valueMap(_, inner), .object(object)):
            return object.keys.sorted().flatMap { key in
                inner.issues(value: object[key] ?? .null, pins: pins, path: "\(path) \(key)")
            }
        case let (.listOf(inner), .array(items)):
            return items.enumerated().flatMap { index, item in
                inner.issues(value: item, pins: pins, path: "\(path) \(index + 1)")
            }
        case let (.oneOf(types), value):
            guard let inner = types.first(where: { $0.matches(value) }) else { return [] }
            return inner.issues(value: value, pins: pins, path: path)
        default:
            return []
        }
    }
}
