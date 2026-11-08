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
