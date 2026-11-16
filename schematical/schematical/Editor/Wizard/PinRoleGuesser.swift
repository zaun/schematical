import Foundation

/// Guesses a pin's role and its active-low alternatives from what a
/// datasheet's pin table says (plan-46-H § 1). They are guesses: the wizard
/// shows them for review before and after they are applied.
enum PinRoleGuesser {
    /// The type column's tokens, upper case, by role.
    static let typeTokens: [String: String] = {
        var tokens: [String: String] = [:]
        for token in ["I", "IN", "INPUT"] { tokens[token] = "input" }
        for token in ["O", "OUT", "OUTPUT"] { tokens[token] = "output" }
        for token in ["I/O", "IO", "B", "BIDIR"] { tokens[token] = "inout" }
        for token in ["P", "PWR", "POWER", "S", "SUPPLY"] { tokens[token] = "power" }
        for token in ["G", "GND", "GROUND"] { tokens[token] = "ground" }
        for token in ["NC", "—", "-"] { tokens[token] = "nc" }
        return tokens
    }()

    static let powerNames = ["VCC", "VDD", "VDDA", "AVDD", "DVDD", "VIN", "VBAT", "V+", "VS"]
    static let groundNames = ["GND", "VSS", "AGND", "DGND", "PGND", "V-", "EP", "PAD", "THERMAL PAD"]
    static let noConnectNames = ["NC", "N.C.", "DNC"]

    /// The role a type cell names; nil when it is not a type token.
    static func role(forType type: String) -> String? {
        typeTokens[type.trimmingCharacters(in: .whitespaces).uppercased()]
    }

    /// Whether a cell is one of the type tokens (for guessing the type column).
    static func isTypeToken(_ cell: String) -> Bool { role(forType: cell) != nil }

    /// A supply name, or one with a digit or `_x` suffix (`VCC2`, `VDD_IO`).
    private static func matches(_ name: String, _ bases: [String]) -> Bool {
        bases.contains { base in
            guard name.hasPrefix(base) else { return false }
            let rest = name.dropFirst(base.count)
            if rest.isEmpty { return true }
            if rest.allSatisfy(\.isNumber) { return true }
            return rest.first == "_" && rest.count > 1
        }
    }

    /// The role of a pin: a supply or no-connect name decides it (a TI table
    /// prints `—` in the type column for VCC and GND), then the type cell;
    /// with neither, `input`, for the user to review.
    static func role(name: String, type: String?) -> String {
        let upper = name.trimmingCharacters(in: .whitespaces).uppercased()
        if matches(upper, powerNames) { return "power" }
        if matches(upper, groundNames) { return "ground" }
        if noConnectNames.contains(upper) { return "nc" }
        if let type, let role = role(forType: type) { return role }
        return "input"
    }

    private static let overbars: Set<Unicode.Scalar> = ["\u{0305}", "\u{0304}"]

    /// A pasted name without its combining overlines and macrons: the
    /// active-low flag carries the overbar, and the block symbol draws it.
    static func cleaned(_ name: String) -> String {
        String(String.UnicodeScalarView(name.unicodeScalars.filter { !overbars.contains($0) }))
    }

    /// Whether one alternative of a name, as pasted, is marked active low:
    /// it ends with `#`, `_N`, `_B` or `*`, starts with `~`, starts with a
    /// lower-case `n` followed by an upper-case letter (`nCS`), or carries an
    /// overline or a macron. A leading `/` is not a marker: `/` separates
    /// alternatives.
    static func isActiveLow(alternative: String) -> Bool {
        if alternative.unicodeScalars.contains(where: { overbars.contains($0) }) { return true }
        let text = cleaned(alternative)
        if text.hasSuffix("#") || text.hasSuffix("*") || text.hasSuffix("_N") || text.hasSuffix("_B") { return true }
        if text.hasPrefix("~"), text.count > 1 { return true }
        let scalars = Array(text)
        if scalars.count >= 2, scalars[0] == "n", scalars[1].isUppercase { return true }
        return false
    }

    /// A pin from a table row: the name as pasted (less its overbars), the
    /// guessed role, and the active-low alternatives. Power, ground and
    /// no-connect pins are never flagged.
    static func pin(designator: String, name: String, type: String?) -> PinDraft {
        let clean = cleaned(name)
        var pin = PinDraft(designator: designator, name: clean, role: role(name: clean, type: type))
        if PinDraft.signalRoles.contains(pin.role) {
            for alternative in name.split(separator: "/", omittingEmptySubsequences: false)
            where isActiveLow(alternative: String(alternative)) {
                pin.activeLow.insert(cleaned(String(alternative)))
            }
        }
        return pin
    }
}
