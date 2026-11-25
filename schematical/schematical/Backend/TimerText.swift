import Foundation

/// What one section of a 555-type timer is doing in play mode (plan-48-H):
/// the backend's `TimerState`, read once right after the circuit starts
/// (`circuit_timer_state`). Times are picoseconds.
nonisolated struct TimerSection: Hashable, Sendable {
    let mode: TimerMode
    let reason: TimerReason
    let firstPs: UInt64
    let highPs: UInt64
    let lowPs: UInt64

    /// A section the circuit does not time (`TIMER_STOPPED`).
    var isStopped: Bool { mode == TIMER_STOPPED }
}

/// The words the sidebar and the block's warning mark use for a timer section.
/// Pure functions of a `TimerSection`.
nonisolated enum TimerText {
    /// `Astable`, `One-shot`, `Latch` or `Not timing`.
    static func mode(_ section: TimerSection) -> String {
        switch section.mode {
        case TIMER_ASTABLE: return "Astable"
        case TIMER_ONE_SHOT: return "One-shot"
        case TIMER_LATCH: return "Latch"
        default: return "Not timing"
        }
    }

    /// An astable section's output frequency, `687 Hz`; "" when it has no period.
    static func frequency(_ section: TimerSection) -> String {
        let period = section.highPs + section.lowPs
        guard section.mode == TIMER_ASTABLE, period > 0 else { return "" }
        return PartFacts.hertz(UInt64((1e12 / Double(period)).rounded()))
    }

    /// An astable section's duty cycle, `52.4 % high`; "" when it has no period.
    static func duty(_ section: TimerSection) -> String {
        let period = section.highPs + section.lowPs
        guard section.mode == TIMER_ASTABLE, period > 0 else { return "" }
        return PartFacts.trimmed(Double(section.highPs) / Double(period) * 100, decimals: 1) + " % high"
    }

    /// A time as the sidebar shows it: `ns` under a microsecond, `µs` under a
    /// millisecond, `ms` under a second, else `s`; three significant digits,
    /// trailing zeros dropped (`762 µs`, `1.1 ms`).
    static func duration(_ ps: UInt64) -> String {
        let value = Double(ps)
        let steps: [(Double, String)] = [(1, "ps"), (1e3, "ns"), (1e6, "µs"), (1e9, "ms"), (1e12, "s")]
        var index = 0
        for (i, step) in steps.enumerated() where value >= step.0 {
            index = i
        }
        // Rounded to three digits, a 999.5 of one unit is 1 of the next.
        while index + 1 < steps.count, rounded(value / steps[index].0) >= 1000 {
            index += 1
        }
        return trimmed(rounded(value / steps[index].0)) + " " + steps[index].1
    }

    /// The backend's sentence for a reason, first letter in upper case.
    static func reason(_ reason: TimerReason) -> String {
        guard let text = timer_reason_text(reason) else { return "" }
        let sentence = String(cString: text)
        return sentence.prefix(1).uppercased() + sentence.dropFirst()
    }

    /// The reason a section is stopped, for a block's warning mark; nil when it is timing.
    static func warning(_ sections: [TimerSection]) -> String? {
        sections.first { $0.isStopped }.map { reason($0.reason) }
    }

    private static func rounded(_ value: Double) -> Double {
        Double(String(format: "%.3g", value)) ?? value
    }

    private static func trimmed(_ value: Double) -> String {
        var text = String(format: "%.3f", value)
        while text.hasSuffix("0") { text.removeLast() }
        if text.hasSuffix(".") { text.removeLast() }
        return text
    }
}
