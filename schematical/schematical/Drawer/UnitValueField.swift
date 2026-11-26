import SwiftUI

/// A value typed as a number and a unit (ohms, farads): what `UnitValueField`
/// needs of `ResistorValue` and `CapacitorValue`.
nonisolated protocol UnitValueScale {
    associatedtype Unit: CaseIterable, Hashable, Sendable where Unit.AllCases: RandomAccessCollection
    /// The value in the largest unit that keeps the number at least 1.
    static func split(_ value: Double) -> (number: Double, unit: Unit)
    static func numberText(_ number: Double) -> String
    static func parse(_ text: String) -> Double?
    /// `number` in `unit`; nil when the result isn't a valid value.
    static func value(number: Double, unit: Unit) -> Double?
    static func symbol(_ unit: Unit) -> String
}

nonisolated extension ResistorValue: UnitValueScale {
    static func value(number: Double, unit: Unit) -> Double? { ohms(number: number, unit: unit) }
    static func symbol(_ unit: Unit) -> String { unit.symbol }
}

nonisolated extension CapacitorValue: UnitValueScale {
    static func value(number: Double, unit: Unit) -> Double? { farads(number: number, unit: unit) }
    static func symbol(_ unit: Unit) -> String { unit.symbol }
}

/// What a number-and-unit field shows and commits (plan-68-AC: the logic
/// `OhmsField` and `CapacitanceField` held in their views). The number
/// commits on Return or when the field loses focus; picking a unit commits
/// the shown number in that unit. Input that doesn't parse, or gives a value
/// that isn't valid, puts the row back.
nonisolated struct UnitValueEntry<Scale: UnitValueScale>: Equatable {
    var text = ""
    /// What the field last showed of the stored value; when `text` differs,
    /// the user is typing and outside changes don't overwrite it.
    var shown = ""
    var unit: Scale.Unit

    init(_ value: Double) {
        unit = Scale.split(value).unit
        show(value)
    }

    /// Shows `value` in its own unit.
    mutating func show(_ value: Double) {
        let split = Scale.split(value)
        shown = Scale.numberText(split.number)
        text = shown
        unit = split.unit
    }

    /// The stored value changed: show it, unless the user is typing a new one
    /// (then only the unit follows).
    mutating func refresh(_ stored: Double, isFocused: Bool) {
        if !isFocused || text == shown {
            show(stored)
        } else {
            unit = Scale.split(stored).unit
        }
    }

    /// Return or focus lost: the value to commit, nil when there is nothing
    /// new (bad input puts `stored` back).
    mutating func apply(_ stored: Double) -> Double? {
        guard text != shown else { return nil }
        guard let number = Scale.parse(text), let value = Scale.value(number: number, unit: unit) else {
            show(stored)
            return nil
        }
        show(value)
        return value
    }

    /// A unit picked: the shown number in that unit, to commit; nil when the
    /// unit is the same or the result is not valid (then `stored` is put back).
    mutating func pick(_ newUnit: Scale.Unit, _ stored: Double) -> Double? {
        guard newUnit != unit else { return nil }
        let number = Scale.parse(text) ?? Scale.split(stored).number
        guard let value = Scale.value(number: number, unit: newUnit) else {
            show(stored)
            return nil
        }
        show(value)
        return value
    }
}

/// A number field and a unit menu, labeled `title`, for a value kept outside
/// (`UnitValueEntry` decides what it shows and commits).
struct UnitValueField<Scale: UnitValueScale>: View {
    let title: String
    let value: Double
    let commit: (Double) -> Void

    @State private var entry: UnitValueEntry<Scale>
    @FocusState private var isFocused: Bool

    init(title: String, value: Double, commit: @escaping (Double) -> Void) {
        self.title = title
        self.value = value
        self.commit = commit
        _entry = State(initialValue: UnitValueEntry(value))
    }

    var body: some View {
        LabeledContent(title) {
            HStack(spacing: 6) {
                TextField(title, text: $entry.text)
                    .labelsHidden()
                    .focused($isFocused)
                    .multilineTextAlignment(.trailing)
                    .frame(minWidth: 60)
                    .onSubmit { _ = entry.apply(value).map(commit) }
                    .onChange(of: isFocused) { _, focused in if !focused { _ = entry.apply(value).map(commit) } }
                Picker("Unit", selection: Binding(get: { entry.unit }, set: { _ = entry.pick($0, value).map(commit) })) {
                    ForEach(Array(Scale.Unit.allCases), id: \.self) { Text(Scale.symbol($0)).tag($0) }
                }
                .pickerStyle(.menu)
                .labelsHidden()
                .fixedSize()
            }
        }
        .onChange(of: value) { entry.refresh(value, isFocused: isFocused) }
        .onAppear { entry.show(value) }
    }
}

/// A pull resistor's resistance: a number field and a unit menu (Ω, kΩ,
/// MΩ), showing `ohms` in the largest unit with a number of at least 1. Also
/// used by Settings ▸ New Pull Resistor.
struct OhmsField: View {
    let ohms: Double
    let commit: (Double) -> Void

    var body: some View {
        UnitValueField<ResistorValue>(title: "Ohms", value: ohms, commit: commit)
    }
}

/// The generic capacitor's capacitance (plan-33-H): a number field and a
/// unit menu (pF, nF, µF), committing like `OhmsField`.
struct CapacitanceField: View {
    let farads: Double
    let commit: (Double) -> Void

    var body: some View {
        UnitValueField<CapacitorValue>(title: "Capacitance", value: farads, commit: commit)
    }
}
