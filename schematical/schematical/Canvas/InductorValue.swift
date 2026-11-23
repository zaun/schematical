import Foundation

/// An inductance, in picohenries (plan-47-M). A real inductor's value is its
/// part's (`CatalogPart.Details.inductor`); it never affects the simulation:
/// an inductor conducts like a wire.
nonisolated enum InductorValue {
    /// The units the text uses.
    enum Unit: CaseIterable, Hashable, Sendable {
        case pico, nano, micro, milli, henry

        /// Picohenries in one unit.
        var scale: Double {
            switch self {
            case .pico: 1
            case .nano: 1e3
            case .micro: 1e6
            case .milli: 1e9
            case .henry: 1e12
            }
        }

        var symbol: String {
            switch self {
            case .pico: "pH"
            case .nano: "nH"
            case .micro: "µH"
            case .milli: "mH"
            case .henry: "H"
            }
        }
    }

    /// `pH` as a number in the largest unit that keeps it at least 1
    /// (below 1 nH: picohenries). 4_700_000 → (4.7, µH).
    static func split(_ pH: Double) -> (number: Double, unit: Unit) {
        let unit = Unit.allCases.last { pH / $0.scale >= 1 - 1e-9 } ?? .pico
        return (pH / unit.scale, unit)
    }

    /// The value as the drawer, the symbol and the Facts sidebar show it, at
    /// most three significant digits: 4_700_000 → "4.7 µH", 10_000 → "10 nH",
    /// 1_000_000_000 → "1 mH", 100 → "100 pH".
    static func text(pH: UInt64) -> String {
        let (number, unit) = split(Double(pH))
        return CapacitorValue.numberText(number) + " " + unit.symbol
    }
}
