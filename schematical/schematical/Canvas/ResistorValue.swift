import Foundation

/// A pull resistor's resistance, in ohms (plan-26). The value is saved per
/// placed pull (`PlacedChip.ohms`) and does not affect the simulation.
nonisolated enum ResistorValue {
    /// A new pull resistor's resistance: 1 kΩ.
    static let defaultOhms: Double = 1000
    /// The largest resistance allowed: 1 GΩ.
    static let maxOhms: Double = 1_000_000_000

    /// More than 0, at most `maxOhms`, and finite.
    static func isValid(_ ohms: Double) -> Bool {
        ohms.isFinite && ohms > 0 && ohms <= maxOhms
    }
}

// MARK: - Units and text (plan-26-B)

extension ResistorValue {
    /// The units the Ohms field offers.
    enum Unit: CaseIterable, Hashable, Sendable {
        case ohm, kilo, mega

        var scale: Double {
            switch self {
            case .ohm: 1
            case .kilo: 1e3
            case .mega: 1e6
            }
        }

        var symbol: String {
            switch self {
            case .ohm: "Ω"
            case .kilo: "kΩ"
            case .mega: "MΩ"
            }
        }
    }

    /// `ohms` as a number in the largest unit that keeps it at least 1
    /// (below 1 Ω: ohms). 4700 → (4.7, kΩ).
    static func split(_ ohms: Double) -> (number: Double, unit: Unit) {
        let unit = Unit.allCases.last { ohms / $0.scale >= 1 } ?? .ohm
        return (ohms / unit.scale, unit)
    }

    /// Up to 3 decimals, trailing zeros trimmed: 4.7 → "4.7", 1.2345 → "1.235".
    static func numberText(_ number: Double) -> String {
        let rounded = round3(number)
        var string = String(format: "%.3f", rounded)
        while string.contains("."), string.hasSuffix("0") {
            string.removeLast()
        }
        if string.hasSuffix(".") {
            string.removeLast()
        }
        return string == "-0" ? "0" : string
    }

    /// The value as shown on the symbol and in the Bill of Materials:
    /// 1000 → "1kΩ", 4700 → "4.7kΩ", 470 → "470Ω", 1e9 → "1000MΩ".
    static func text(_ ohms: Double) -> String {
        let (number, unit) = split(ohms)
        return numberText(number) + unit.symbol
    }

    /// `number` (rounded to 3 decimals) in `unit`, in ohms; nil when the
    /// result isn't `isValid`.
    static func ohms(number: Double, unit: Unit) -> Double? {
        let value = round3(number) * unit.scale
        return isValid(value) ? value : nil
    }

    /// The number in `text` (spaces trimmed); nil when it isn't a finite number.
    static func parse(_ text: String) -> Double? {
        guard let value = Double(text.trimmingCharacters(in: .whitespaces)), value.isFinite else {
            return nil
        }
        return value
    }

    /// Rounds half away from zero to 3 decimals, nudged past float error
    /// (the `LengthField.format` rounding), so 1.2345 gives 1.235.
    private static func round3(_ value: Double) -> Double {
        (value * 1000 * (1 + 1e-12)).rounded(.toNearestOrAwayFromZero) / 1000
    }
}
