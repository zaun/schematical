import SwiftUI

// MARK: - Bindings into JSON values

extension Binding where Value == [String: JSONValue] {
    /// One key of the object; setting nil removes it.
    func key(_ key: String) -> Binding<JSONValue?> {
        Binding<JSONValue?>(
            get: { wrappedValue[key] },
            set: { wrappedValue[key] = $0 }
        )
    }
}

extension Binding where Value == JSONValue? {
    /// The value as an object (an empty one when missing); setting an empty
    /// object on an optional key removes it.
    func object(removeWhenEmpty: Bool) -> Binding<[String: JSONValue]> {
        Binding<[String: JSONValue]>(
            get: {
                if case let .object(object)? = wrappedValue { return object }
                return [:]
            },
            set: { wrappedValue = $0.isEmpty && removeWhenEmpty ? nil : .object($0) }
        )
    }

    func array() -> Binding<[JSONValue]> {
        Binding<[JSONValue]>(
            get: {
                if case let .array(items)? = wrappedValue { return items }
                return []
            },
            set: { wrappedValue = .array($0) }
        )
    }

    /// A string; setting "" removes an optional key.
    func string(removeWhenEmpty: Bool) -> Binding<String> {
        Binding<String>(
            get: {
                if case let .string(text)? = wrappedValue { return text }
                return ""
            },
            set: { wrappedValue = $0.isEmpty && removeWhenEmpty ? nil : .string($0) }
        )
    }

    /// A number; nil removes the key.
    func number() -> Binding<Double?> {
        Binding<Double?>(
            get: {
                if case let .number(value)? = wrappedValue { return value }
                return nil
            },
            set: { wrappedValue = $0.map { .number($0) } }
        )
    }
}

extension Binding where Value == JSONValue? {
    /// A yes/no as a picker's tag: 1 yes, 0 no, nil absent.
    func boolTag() -> Binding<Int?> {
        Binding<Int?>(
            get: {
                if case let .bool(flag)? = wrappedValue { return flag ? 1 : 0 }
                return nil
            },
            set: { wrappedValue = $0.map { .bool($0 == 1) } }
        )
    }

    /// One end of a `[min, max]` volt range; setting either end makes the
    /// pair (the other end 0 when there was none).
    func voltBound(_ index: Int) -> Binding<Double?> {
        Binding<Double?>(
            get: {
                guard case let .array(items)? = wrappedValue, items.count == 2,
                      case let .number(volts) = items[index] else { return nil }
                return volts
            },
            set: { newValue in
                var items: [JSONValue] = [.number(0), .number(0)]
                if case let .array(current)? = wrappedValue, current.count == 2 { items = current }
                items[index] = .number(newValue ?? 0)
                wrappedValue = .array(items)
            }
        )
    }

    /// Which of `types` the value has (the first when none or no value);
    /// choosing another starts that shape empty.
    func shapeIndex(of types: [FieldType]) -> Binding<Int> {
        Binding<Int>(
            get: { wrappedValue.flatMap { current in types.firstIndex { $0.matches(current) } } ?? 0 },
            set: { wrappedValue = types[$0].emptyValue }
        )
    }
}

extension Binding where Value == [JSONValue] {
    /// One entry, nil past the end; setting nil stores `null`, and a set past
    /// the end (the entry was just removed) does nothing.
    func element(_ index: Int) -> Binding<JSONValue?> {
        Binding<JSONValue?>(
            get: { wrappedValue.indices.contains(index) ? wrappedValue[index] : nil },
            set: { if wrappedValue.indices.contains(index) { wrappedValue[index] = $0 ?? .null } }
        )
    }

    /// One entry as a string ("" when it is not one).
    func string(_ index: Int) -> Binding<String> {
        Binding<String>(
            get: {
                guard wrappedValue.indices.contains(index), case let .string(value) = wrappedValue[index] else { return "" }
                return value
            },
            set: { if wrappedValue.indices.contains(index) { wrappedValue[index] = .string($0) } }
        )
    }

    /// One entry as a number (nil when it is not one); setting nil stores 0.
    func number(_ index: Int) -> Binding<Double?> {
        Binding<Double?>(
            get: {
                guard wrappedValue.indices.contains(index), case let .number(value) = wrappedValue[index] else { return nil }
                return value
            },
            set: { if wrappedValue.indices.contains(index) { wrappedValue[index] = .number($0 ?? 0) } }
        )
    }

    /// One entry as an object (empty when it is not one).
    func object(_ index: Int) -> Binding<[String: JSONValue]> {
        Binding<[String: JSONValue]>(
            get: {
                guard wrappedValue.indices.contains(index), case let .object(object) = wrappedValue[index] else { return [:] }
                return object
            },
            set: { if wrappedValue.indices.contains(index) { wrappedValue[index] = .object($0) } }
        )
    }
}

/// The editors' choices and edits that are not bindings.
enum FieldEditing {
    /// The options, plus the current string when it is not one of them (so a
    /// file's value shows rather than vanishing).
    static func options(_ options: [String], current: JSONValue?) -> [String] {
        if case let .string(text)? = current, !text.isEmpty, !options.contains(text) { return options + [text] }
        return options
    }

    /// The object with a new entry under `key` (trimmed); nil when the key is
    /// blank or already there.
    static func adding(_ key: String, entry: JSONValue, to object: [String: JSONValue]) -> [String: JSONValue]? {
        let key = key.trimmingCharacters(in: .whitespaces)
        guard !key.isEmpty, object[key] == nil else { return nil }
        var object = object
        object[key] = entry
        return object
    }

    /// What JSON text gives a key: nil removes it (blank text, optional key),
    /// a value when it parses; `.failure` when it does not.
    static func parse(_ text: String, required: Bool) -> Result<JSONValue?, JSONTextError> {
        let trimmed = text.trimmingCharacters(in: .whitespacesAndNewlines)
        if trimmed.isEmpty, !required { return .success(nil) }
        guard let parsed = try? JSONValue(text: trimmed) else { return .failure(JSONTextError()) }
        return .success(parsed)
    }

    struct JSONTextError: Error, Equatable {
        let message = "Not valid JSON"
    }
}

// MARK: - One field

/// The control for one key of a kind's object, by its `FieldType`.
struct FieldEditor: View {
    let field: Field
    @Binding var value: JSONValue?
    /// The part's pin names, for pin pickers.
    let pinNames: [String]
    @Environment(\.guidedFields) private var guided

    var body: some View {
        // In a wizard step, the kind's guide (or the shared one) names and
        // explains the field (plan-46-L); All fields keeps the schema's label.
        let guide = guided.flatMap { Guidance.guide(kind: $0.kind, path: "\($0.path).\(field.key)") }
        VStack(alignment: .leading, spacing: 4) {
            FieldValueEditor(label: guide?.label ?? field.label, type: field.type, required: field.required,
                             value: $value, pinNames: pinNames)
                .environment(\.guidedFields, guided?.descending(field.key, label: field.label))
                .environment(\.fieldExample, guide?.example)
            if let guide {
                FieldGuideView(guide: guide, examplePlaced: field.type.takesPlaceholder)
            } else if let help = field.help {
                Text(help).font(.caption).foregroundStyle(.secondary)
            }
            if guided != nil {
                // The key as the part file and the docs write it.
                Text(field.key).font(.caption2.monospaced()).foregroundStyle(.tertiary)
                    .accessibilityLabel("Key \(field.key)")
            }
        }
    }
}

/// A value's control, recursive for nested objects and lists.
struct FieldValueEditor: View {
    let label: String
    let type: FieldType
    let required: Bool
    @Binding var value: JSONValue?
    let pinNames: [String]
    /// A typical value, as the placeholder of a single text field (plan-46-L).
    @Environment(\.fieldExample) private var example

    var body: some View {
        switch type {
        case .text:
            TextField(label, text: $value.string(removeWhenEmpty: !required), prompt: example.map { Text($0) })
        case .note:
            TextField(label, text: $value.string(removeWhenEmpty: !required), prompt: example.map { Text($0) },
                      axis: .vertical)
                .lineLimit(1...4)
        case let .number(unit):
            NumberField(label: unit.isEmpty ? label : "\(label) (\(unit))", value: $value.number(), example: example)
        case .integer:
            NumberField(label: label, value: $value.number(), integer: true, example: example)
        case .bool:
            Picker(label, selection: $value.boolTag()) {
                if !required { Text("—").tag(Int?.none) }
                Text("Yes").tag(Int?.some(1))
                Text("No").tag(Int?.some(0))
            }
        case let .choice(options):
            Picker(label, selection: $value.string(removeWhenEmpty: !required)) {
                if !required { Text("—").tag("") }
                ForEach(FieldEditing.options(options, current: value), id: \.self) { Text($0).tag($0) }
            }
        case let .suggested(options):
            HStack {
                TextField(label, text: $value.string(removeWhenEmpty: !required), prompt: example.map { Text($0) })
                Menu {
                    // One line: a menu builds its items only as it opens on screen.
                    ForEach(options, id: \.self) { option in Button(option) { value = .string(option) } }
                } label: {
                    Image(systemName: "chevron.down")
                }
                .menuStyle(.borderlessButton)
                .fixedSize()
            }
        case .pin:
            PinPicker(label: label, selection: $value.string(removeWhenEmpty: !required),
                      pinNames: pinNames, optional: !required)
        case .pins:
            StringListEditor(label: label, items: $value.array(), pinNames: pinNames)
        case .names:
            StringListEditor(label: label, items: $value.array(), pinNames: nil)
        case let .numbers(unit):
            NumberListEditor(label: unit.isEmpty ? label : "\(label) (\(unit))", items: $value.array())
        case .voltRange:
            VoltRangeEditor(label: label, value: $value)
        case let .object(fields):
            ObjectEditor(label: label, fields: fields, required: required, value: $value, pinNames: pinNames)
        case let .list(fields):
            ListEditor(label: label, fields: fields, items: $value.array(), pinNames: pinNames)
        case let .map(keyLabel, fields):
            MapEditor(label: label, keyLabel: keyLabel, entryType: .object(fields),
                      object: $value.object(removeWhenEmpty: !required), pinNames: pinNames)
        case let .valueMap(keyLabel, entryType):
            MapEditor(label: label, keyLabel: keyLabel, entryType: entryType,
                      object: $value.object(removeWhenEmpty: !required), pinNames: pinNames)
        case let .listOf(entryType):
            ValueListEditor(label: label, entryType: entryType, items: $value.array(), pinNames: pinNames)
        case .json:
            JSONTextEditor(label: label, value: $value, required: required)
        case let .oneOf(types):
            OneOfEditor(label: label, types: types, required: required, value: $value, pinNames: pinNames)
        }
    }
}

/// A number text field; empty is nil.
struct NumberField: View {
    let label: String
    @Binding var value: Double?
    var integer = false
    /// A typical value, shown while the field is empty.
    var example: String? = nil

    var body: some View {
        if integer {
            TextField(label, value: $value, format: .number.precision(.fractionLength(0)).grouping(.never),
                      prompt: example.map { Text($0) })
        } else {
            TextField(label, value: $value, format: .number.grouping(.never), prompt: example.map { Text($0) })
        }
    }
}

struct PinPicker: View {
    let label: String
    @Binding var selection: String
    let pinNames: [String]
    var optional = false

    var body: some View {
        Picker(label, selection: $selection) {
            if optional || selection.isEmpty { Text("—").tag("") }
            ForEach(FieldEditing.options(pinNames, current: .string(selection)), id: \.self) { Text($0).tag($0) }
        }
    }
}

/// A list of strings: pin pickers when `pinNames` is given, text otherwise.
struct StringListEditor: View {
    let label: String
    @Binding var items: [JSONValue]
    let pinNames: [String]?

    var body: some View {
        LabeledContent(label) {
            VStack(alignment: .leading, spacing: 4) {
                ForEach(items.indices, id: \.self) { index in
                    HStack {
                        if let pinNames {
                            PinPicker(label: "", selection: $items.string(index), pinNames: pinNames).labelsHidden()
                        } else {
                            TextField("", text: $items.string(index)).labelsHidden()
                        }
                        Button { items.remove(at: index) } label: { Image(systemName: "minus.circle") }
                            .buttonStyle(.borderless)
                    }
                }
                Button { items.append(.string(pinNames?.first ?? "")) } label: {
                    Label("Add", systemImage: "plus.circle")
                }
                .buttonStyle(.borderless)
            }
        }
    }
}

struct NumberListEditor: View {
    let label: String
    @Binding var items: [JSONValue]

    var body: some View {
        LabeledContent(label) {
            VStack(alignment: .leading, spacing: 4) {
                ForEach(items.indices, id: \.self) { index in
                    HStack {
                        NumberField(label: "", value: $items.number(index)).labelsHidden()
                        Button { items.remove(at: index) } label: { Image(systemName: "minus.circle") }
                            .buttonStyle(.borderless)
                    }
                }
                Button { items.append(.number(0)) } label: { Label("Add", systemImage: "plus.circle") }
                    .buttonStyle(.borderless)
            }
        }
    }
}

struct VoltRangeEditor: View {
    let label: String
    @Binding var value: JSONValue?

    var body: some View {
        LabeledContent("\(label) (V)") {
            HStack {
                NumberField(label: "min", value: $value.voltBound(0))
                Text("to")
                NumberField(label: "max", value: $value.voltBound(1))
                if value != nil {
                    Button { value = nil } label: { Image(systemName: "xmark.circle") }
                        .buttonStyle(.borderless)
                        .help("Remove")
                }
            }
        }
    }
}

/// A nested object: its fields, or an Add button while an optional one is absent.
struct ObjectEditor: View {
    let label: String
    let fields: [Field]
    let required: Bool
    @Binding var value: JSONValue?
    let pinNames: [String]

    var body: some View {
        if value == nil && !required {
            LabeledContent(label) {
                Button { value = FieldType.object(fields).emptyValue } label: { Label("Add", systemImage: "plus.circle") }
                    .buttonStyle(.borderless)
            }
        } else {
            GroupBox {
                VStack(alignment: .leading, spacing: 6) {
                    HStack {
                        Text(label).font(.headline)
                        Spacer()
                        if !required {
                            Button { value = nil } label: { Image(systemName: "minus.circle") }
                                .buttonStyle(.borderless)
                                .help("Remove \(label)")
                        }
                    }
                    FieldsEditor(fields: fields, object: $value.object(removeWhenEmpty: false), pinNames: pinNames)
                }
                .padding(4)
            }
        }
    }
}

/// The fields of one object.
struct FieldsEditor: View {
    let fields: [Field]
    @Binding var object: [String: JSONValue]
    let pinNames: [String]
    /// Set inside a wizard step (plan-46-I); nil in All fields, which shows
    /// every field.
    @Environment(\.guidedFields) private var guided

    var body: some View {
        if let guided, let conditions = PartKinds.conditions[guided.kind] {
            let layout = GuidedFieldLayout.make(fields: fields, path: guided.bare, conditions: conditions,
                                                object: object)
            editors(layout.leading + layout.shown)
            if !layout.more.isEmpty {
                DisclosureGroup("More \(guided.label.lowercased()) settings") {
                    editors(layout.more)
                }
            }
        } else {
            ForEach(fields) { field in
                AnyView(FieldEditor(field: field, value: $object.key(field.key), pinNames: pinNames))
            }
        }
    }

    private func editors(_ keys: [String]) -> some View {
        ForEach(keys.compactMap { key in fields.first { $0.key == key } }) { field in
            AnyView(FieldEditor(field: field, value: $object.key(field.key), pinNames: pinNames))
        }
    }
}

/// An array of objects: one box per entry, add and remove.
struct ListEditor: View {
    let label: String
    let fields: [Field]
    @Binding var items: [JSONValue]
    let pinNames: [String]

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack {
                Text(label).font(.headline)
                Spacer()
                Button { items.append(FieldType.object(fields).emptyValue) } label: {
                    Label("Add", systemImage: "plus.circle")
                }
                .buttonStyle(.borderless)
            }
            ForEach(items.indices, id: \.self) { index in
                GroupBox {
                    VStack(alignment: .leading, spacing: 6) {
                        HStack {
                            Text("\(label) \(index + 1)").foregroundStyle(.secondary)
                            Spacer()
                            Button { items.remove(at: index) } label: { Image(systemName: "minus.circle") }
                                .buttonStyle(.borderless)
                        }
                        AnyView(FieldsEditor(fields: fields, object: $items.object(index), pinNames: pinNames))
                    }
                    .padding(4)
                }
            }
        }
    }
}

/// A list of values of one type.
struct ValueListEditor: View {
    let label: String
    let entryType: FieldType
    @Binding var items: [JSONValue]
    let pinNames: [String]

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack {
                Text(label).font(.headline)
                Spacer()
                Button { items.append(entryType.emptyValue) } label: { Label("Add", systemImage: "plus.circle") }
                    .buttonStyle(.borderless)
            }
            ForEach(items.indices, id: \.self) { index in
                HStack(alignment: .top) {
                    AnyView(FieldValueEditor(label: "\(index + 1)", type: entryType, required: true,
                                             value: $items.element(index), pinNames: pinNames))
                    Button { items.remove(at: index) } label: { Image(systemName: "minus.circle") }
                        .buttonStyle(.borderless)
                }
            }
        }
    }
}

/// An object whose keys the user names (a timing table by family).
struct MapEditor: View {
    let label: String
    let keyLabel: String
    let entryType: FieldType
    @Binding var object: [String: JSONValue]
    let pinNames: [String]
    @State private var newKey: String

    init(label: String, keyLabel: String, entryType: FieldType, object: Binding<[String: JSONValue]>,
         pinNames: [String]) {
        self.label = label
        self.keyLabel = keyLabel
        self.entryType = entryType
        _object = object
        self.pinNames = pinNames
        _newKey = State(initialValue: "")
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text(label).font(.headline)
            ForEach(object.keys.sorted(), id: \.self) { key in
                GroupBox {
                    VStack(alignment: .leading, spacing: 6) {
                        HStack {
                            Text("\(keyLabel): \(key)").foregroundStyle(.secondary)
                            Spacer()
                            Button { object.removeValue(forKey: key) } label: { Image(systemName: "minus.circle") }
                                .buttonStyle(.borderless)
                        }
                        AnyView(FieldValueEditor(label: key, type: entryType, required: true,
                                                 value: $object.key(key), pinNames: pinNames))
                    }
                    .padding(4)
                }
            }
            HStack {
                TextField("New \(keyLabel.lowercased())", text: $newKey)
                Button("Add \(keyLabel)") {
                    guard let added = FieldEditing.adding(newKey, entry: entryType.emptyValue, to: object) else { return }
                    object = added
                    newKey = ""
                }
                .disabled(newKey.trimmingCharacters(in: .whitespaces).isEmpty)
            }
        }
    }
}

/// A key the backend accepts in several shapes: a shape picker and the
/// chosen shape's control. Changing the shape starts that shape empty.
struct OneOfEditor: View {
    let label: String
    let types: [FieldType]
    let required: Bool
    @Binding var value: JSONValue?
    let pinNames: [String]

    var body: some View {
        let index = $value.shapeIndex(of: types)
        VStack(alignment: .leading, spacing: 4) {
            Picker("\(label) as", selection: index) {
                ForEach(types.indices, id: \.self) { Text(types[$0].shape).tag($0) }
            }
            .pickerStyle(.segmented)
            AnyView(FieldValueEditor(label: label, type: types[index.wrappedValue], required: required, value: $value,
                                     pinNames: pinNames))
        }
    }
}

/// A key with more than one shape: JSON text, applied when it parses.
struct JSONTextEditor: View {
    let label: String
    @Binding var value: JSONValue?
    let required: Bool
    @State private var text: String
    @State private var error: String?

    init(label: String, value: Binding<JSONValue?>, required: Bool) {
        self.label = label
        _value = value
        self.required = required
        _text = State(initialValue: "")
        _error = State(initialValue: nil)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(label)
            TextEditor(text: $text)
                .font(.system(.body, design: .monospaced))
                .frame(minHeight: 60)
                .onChange(of: text) { _, newText in apply(newText) }
            if let error { Text(error).font(.caption).foregroundStyle(.red) }
        }
        .onAppear { text = (try? value?.text()) ?? "" }
    }

    private func apply(_ newText: String) {
        switch FieldEditing.parse(newText, required: required) {
        case let .success(parsed):
            if parsed != value { value = parsed }
            error = nil
        case let .failure(failure):
            error = failure.message
        }
    }
}
