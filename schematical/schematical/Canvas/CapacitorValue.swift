import Foundation

/// A capacitance, in farads (plan-33-H). A real capacitor's value is its
/// part's (`CatalogPart.Details.capacitor`); only `GENERIC-CAPACITOR` keeps
/// one per placed part (`PlacedChip.farads`). Neither affects the simulation.
nonisolated enum CapacitorValue {
    /// A new generic capacitor's capacitance: 0.1 µF (plan-33-A § Decisions).
    static let defaultFarads: Double = 1e-7
    /// The smallest capacitance allowed: 0.1 pF.
    static let minFarads: Double = 1e-13
    /// The largest capacitance allowed: 1 F.
    static let maxFarads: Double = 1

    /// At least `minFarads`, at most `maxFarads`, and finite.
    static func isValid(_ farads: Double) -> Bool {
        farads.isFinite && farads >= minFarads * (1 - 1e-9) && farads <= maxFarads
    }

    /// The units the Capacitance field offers.
    enum Unit: CaseIterable, Hashable, Sendable {
        case pico, nano, micro

        var scale: Double {
            switch self {
            case .pico: 1e-12
            case .nano: 1e-9
            case .micro: 1e-6
            }
        }

        var symbol: String {
            switch self {
            case .pico: "pF"
            case .nano: "nF"
            case .micro: "µF"
            }
        }
    }

    /// `farads` as a number in the largest unit that keeps it at least 1
    /// (below 1 pF: picofarads). 4.7e-9 → (4.7, nF); 4.7e-3 → (4700, µF).
    static func split(_ farads: Double) -> (number: Double, unit: Unit) {
        let unit = Unit.allCases.last { farads / $0.scale >= 1 - 1e-9 } ?? .pico
        return (farads / unit.scale, unit)
    }

    /// At most three significant digits (more when the integer part needs
    /// them: 4700), trailing zeros trimmed: 4.7 → "4.7", 100 → "100", 0.1 → "0.1".
    static func numberText(_ number: Double) -> String {
        guard number.isFinite, number > 0 else { return "0" }
        let digits = max(0, 2 - Int(floor(log10(number) + 1e-9)))
        var string = String(format: "%.\(digits)f", number)
        if string.contains(".") {
            while string.hasSuffix("0") { string.removeLast() }
            if string.hasSuffix(".") { string.removeLast() }
        }
        return string
    }

    /// The value as the drawer, the symbol and the Bill of Materials show
    /// it: 1e-11 → "10 pF", 4.7e-9 → "4.7 nF", 1e-7 → "100 nF",
    /// 2.2e-5 → "22 µF", 4.7e-3 → "4700 µF", 1e-13 → "0.1 pF".
    static func text(_ farads: Double) -> String {
        let (number, unit) = split(farads)
        return numberText(number) + " " + unit.symbol
    }

    /// `number` in `unit`, in farads (three significant digits); nil when
    /// the result isn't `isValid`.
    static func farads(number: Double, unit: Unit) -> Double? {
        guard number.isFinite, number > 0 else { return nil }
        let rounded = Double(numberText(number)) ?? number
        let value = rounded * unit.scale
        return isValid(value) ? value : nil
    }

    /// The number in `text` (spaces trimmed); nil when it isn't a finite number.
    static func parse(_ text: String) -> Double? {
        guard let value = Double(text.trimmingCharacters(in: .whitespaces)), value.isFinite else {
            return nil
        }
        return value
    }
}
