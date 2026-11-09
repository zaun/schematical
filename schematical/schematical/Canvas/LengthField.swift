import SwiftUI

/// A text field for a length, shown in the board's units (mm or inches) and
/// stored in mm. It commits on Return or when it loses focus; text that
/// doesn't parse puts the field back. `commit` returns the length the document
/// kept (it may clamp or snap), which the field then shows.
struct LengthField: View {
    let title: String
    let millimetres: Double
    let units: BoardUnits
    /// Shows 2 more decimals (4 in mm, 5 in inches), for grid sizes.
    var precise = false
    let commit: (Double) -> Double

    @State private var text = ""
    /// What the field last showed of the stored value; when `text` differs,
    /// the user is typing and outside changes don't overwrite it.
    @State private var shown = ""
    @FocusState private var isFocused: Bool

    var body: some View {
        TextField(title, text: $text)
            .focused($isFocused)
            .multilineTextAlignment(.trailing)
            .onSubmit(apply)
            .onChange(of: isFocused) { _, focused in
                if !focused { apply() }
            }
            .onChange(of: millimetres) { refresh() }
            .onChange(of: units) { refresh() }
            .onAppear { refresh() }
    }

    /// Shows the stored value, unless the user is typing a new one.
    private func refresh() {
        if !isFocused || text == shown {
            show(millimetres)
        }
    }

    private func show(_ mm: Double) {
        shown = Self.format(mm, units: units, precise: precise)
        text = shown
    }

    private func apply() {
        guard text != shown else { return }
        guard let value = Self.parse(text, units: units) else {
            show(millimetres)
            return
        }
        show(commit(value))
    }

    /// mm to 2 decimals or inches to 3 (2 more each when `precise`),
    /// trailing zeros trimmed: 101.6 mm in inches is "4", 0.635 mm is
    /// "0.64" mm or "0.025" in, and "0.635" mm when precise.
    static func format(_ mm: Double, units: BoardUnits, precise: Bool = false) -> String {
        let extra = precise ? 2 : 0
        let (value, places) = units == .mm ? (mm, 2 + extra) : (mm / 25.4, 3 + extra)
        let scale = pow(10, Double(places))
        // Round half away from zero on a value nudged past float error, so
        // 0.635 (stored as 0.63499…) shows as 0.64.
        let rounded = (value * scale * (1 + 1e-12)).rounded(.toNearestOrAwayFromZero) / scale
        var string = String(format: "%.\(places)f", rounded)
        while string.contains("."), string.hasSuffix("0") {
            string.removeLast()
        }
        if string.hasSuffix(".") {
            string.removeLast()
        }
        return string == "-0" ? "0" : string
    }

    /// The length in mm typed in `units`; a trailing "mm", "in" or `"` is
    /// ignored. Nil when it isn't a number.
    static func parse(_ text: String, units: BoardUnits) -> Double? {
        var trimmed = text.trimmingCharacters(in: .whitespaces)
        for suffix in ["mm", "in", "\""] where trimmed.hasSuffix(suffix) {
            trimmed = String(trimmed.dropLast(suffix.count)).trimmingCharacters(in: .whitespaces)
            break
        }
        guard let value = Double(trimmed), value.isFinite else { return nil }
        return units == .mm ? value : value * 25.4
    }
}
