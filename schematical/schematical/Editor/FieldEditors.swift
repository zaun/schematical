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

extension Binding where Value == JSONValue {
    var optional: Binding<JSONValue?> {
        Binding<JSONValue?>(get: { wrappedValue }, set: { wrappedValue = $0 ?? .null })
    }
}

// MARK: - One field

/// The control for one key of a kind's object, by its `FieldType`.
struct FieldEditor: View {
    let field: Field
    @Binding var value: JSONValue?
    /// The part's pin names, for pin pickers.
    let pinNames: [String]

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            FieldValueEditor(label: field.label, type: field.type, required: field.required,
                             value: $value, pinNames: pinNames)
            if let help = field.help {
                Text(help).font(.caption).foregroundStyle(.secondary)
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

    var body: some View {
        switch type {
        case .text:
            TextField(label, text: $value.string(removeWhenEmpty: !required))
        case .note:
            TextField(label, text: $value.string(removeWhenEmpty: !required), axis: .vertical)
                .lineLimit(1...4)
        case let .number(unit):
            NumberField(label: unit.isEmpty ? label : "\(label) (\(unit))", value: $value.number())
        case .integer:
            NumberField(label: label, value: $value.number(), integer: true)
        case .bool:
            Picker(label, selection: boolSelection) {
                if !required { Text("—").tag(Int?.none) }
                Text("Yes").tag(Int?.some(1))
                Text("No").tag(Int?.some(0))
            }
        case let .choice(options):
            Picker(label, selection: $value.string(removeWhenEmpty: !required)) {
                if !required { Text("—").tag("") }
                ForEach(withCurrent(options), id: \.self) { Text($0).tag($0) }
            }
        case let .suggested(options):
            HStack {
                TextField(label, text: $value.string(removeWhenEmpty: !required))
                Menu {
                    ForEach(options, id: \.self) { option in
                        Button(option) { value = .string(option) }
                    }
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
        }
    }

    private var boolSelection: Binding<Int?> {
        Binding<Int?>(
            get: {
                if case let .bool(flag)? = value { return flag ? 1 : 0 }
                return nil
            },
            set: { value = $0.map { .bool($0 == 1) } }
        )
    }

    /// The options, plus the current value when it is not one of them (so a
    /// file's value shows rather than vanishing).
    private func withCurrent(_ options: [String]) -> [String] {
        if case let .string(current)? = value, !current.isEmpty, !options.contains(current) {
            return options + [current]
        }
        return options
    }
}

/// A number text field; empty is nil.
struct NumberField: View {
    let label: String
    @Binding var value: Double?
    var integer = false

    var body: some View {
        if integer {
            TextField(label, value: $value, format: .number.precision(.fractionLength(0)).grouping(.never))
        } else {
            TextField(label, value: $value, format: .number.grouping(.never))
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
            ForEach(options, id: \.self) { Text($0).tag($0) }
        }
    }

    private var options: [String] {
        !selection.isEmpty && !pinNames.contains(selection) ? pinNames + [selection] : pinNames
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
                        let text = Binding<String>(
                            get: {
                                guard items.indices.contains(index), case let .string(value) = items[index] else { return "" }
                                return value
                            },
                            set: { if items.indices.contains(index) { items[index] = .string($0) } }
                        )
                        if let pinNames {
                            PinPicker(label: "", selection: text, pinNames: pinNames).labelsHidden()
                        } else {
                            TextField("", text: text).labelsHidden()
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
                        NumberField(label: "", value: Binding<Double?>(
                            get: {
                                guard items.indices.contains(index), case let .number(value) = items[index] else { return nil }
                                return value
                            },
                            set: { if items.indices.contains(index) { items[index] = .number($0 ?? 0) } }
                        )).labelsHidden()
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
                NumberField(label: "min", value: bound(0))
                Text("to")
                NumberField(label: "max", value: bound(1))
                if value != nil {
                    Button { value = nil } label: { Image(systemName: "xmark.circle") }
                        .buttonStyle(.borderless)
                        .help("Remove")
                }
            }
        }
    }

    private func bound(_ index: Int) -> Binding<Double?> {
        Binding<Double?>(
            get: {
                guard case let .array(items)? = value, items.count == 2, case let .number(volts) = items[index] else {
                    return nil
                }
                return volts
            },
            set: { newValue in
                var items: [JSONValue] = [.number(0), .number(0)]
                if case let .array(current)? = value, current.count == 2 { items = current }
                items[index] = .number(newValue ?? 0)
                value = .array(items)
            }
        )
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

    var body: some View {
        ForEach(fields) { field in
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
                        AnyView(FieldsEditor(fields: fields, object: entry(index), pinNames: pinNames))
                    }
                    .padding(4)
                }
            }
        }
    }

    private func entry(_ index: Int) -> Binding<[String: JSONValue]> {
        Binding<[String: JSONValue]>(
            get: {
                guard items.indices.contains(index), case let .object(object) = items[index] else { return [:] }
                return object
            },
            set: { if items.indices.contains(index) { items[index] = .object($0) } }
        )
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
                                             value: element(index), pinNames: pinNames))
                    Button { items.remove(at: index) } label: { Image(systemName: "minus.circle") }
                        .buttonStyle(.borderless)
                }
            }
        }
    }

    private func element(_ index: Int) -> Binding<JSONValue?> {
        Binding<JSONValue?>(
            get: { items.indices.contains(index) ? items[index] : nil },
            set: { if items.indices.contains(index) { items[index] = $0 ?? .null } }
        )
    }
}

/// An object whose keys the user names (a timing table by family).
struct MapEditor: View {
    let label: String
    let keyLabel: String
    let entryType: FieldType
    @Binding var object: [String: JSONValue]
    let pinNames: [String]
    @State private var newKey = ""

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
                    let key = newKey.trimmingCharacters(in: .whitespaces)
                    guard !key.isEmpty, object[key] == nil else { return }
                    object[key] = entryType.emptyValue
                    newKey = ""
                }
                .disabled(newKey.trimmingCharacters(in: .whitespaces).isEmpty)
            }
        }
    }
}

/// A key with more than one shape: JSON text, applied when it parses.
struct JSONTextEditor: View {
    let label: String
    @Binding var value: JSONValue?
    let required: Bool
    @State private var text = ""
    @State private var error: String?

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
        let trimmed = newText.trimmingCharacters(in: .whitespacesAndNewlines)
        if trimmed.isEmpty, !required {
            value = nil
            error = nil
        } else if let parsed = try? JSONValue(text: trimmed) {
            if parsed != value { value = parsed }
            error = nil
        } else {
            error = "Not valid JSON"
        }
    }
}
