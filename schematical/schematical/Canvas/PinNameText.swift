import SwiftUI

extension ChipPin {
    /// The pin's name as a block symbol draws it (plan-39-C): its
    /// alternatives joined by "/", each active-low one with an overline and,
    /// when it is longer than one character, its one trailing "#" removed
    /// (`CE#` flagged draws as an overlined `CE`). A pin with no flag draws
    /// its name exactly as written.
    func paintedName(size: CGFloat, weight: Font.Weight = .regular, monospaced: Bool = false,
                     ink: Ink = .primary) -> PaintedText {
        guard isActiveLow else {
            return PaintedText(name, size: size, weight: weight, monospaced: monospaced, ink: ink)
        }
        var string = ""
        var overlines: [Range<Int>] = []
        for (index, alternative) in alternatives.enumerated() {
            if index > 0 { string += "/" }
            var text = alternative.text
            if alternative.isActiveLow, text.count > 1, text.hasSuffix("#") { text.removeLast() }
            let start = string.count
            string += text
            if alternative.isActiveLow, !text.isEmpty { overlines.append(start..<string.count) }
        }
        return PaintedText(string, size: size, weight: weight, monospaced: monospaced, ink: ink,
                           overlines: overlines)
    }
}
