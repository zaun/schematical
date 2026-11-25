import AppKit
import Synchronization

/// A block of free text on the schematic: a comment or label. It may hold
/// several lines. It carries no current and is never simulated.
nonisolated struct SchematicNote: Identifiable, Hashable, Sendable {
    let id: UUID
    /// May hold line breaks; never empty.
    var text: String
    /// The top-left of the block. On the grid.
    var origin: CGPoint
    /// Font size in points, within `sizeRange`.
    var size: Int = Self.defaultSize
    /// Whether the block has a frame around it.
    var framed = true

    static let sizeRange = 6...72
    static let defaultSize = 12
    /// What the Text tool places.
    static let placeholder = "Text"

    /// Space between the frame and the text, on every side.
    static let padding: CGFloat = 6

    /// The monospaced font the canvas and exports draw the text in.
    var font: NSFont { Self.metrics(size).font }

    /// The text's lines, empty ones included.
    var lines: [String] {
        text.split(separator: "\n", omittingEmptySubsequences: false).map(String.init)
    }

    /// The distance from one line's top to the next one's.
    var lineHeight: CGFloat { Self.metrics(size).lineHeight }

    /// A size's font and line height, made once and shared.
    ///
    /// `rect` runs on the main thread for hit tests and on SwiftUI's render
    /// thread when the `drawingGroup` canvas draws, often at the same time.
    /// AppKit's text measuring (`NSLayoutManager`, `size(withAttributes:)`)
    /// is not safe on two threads at once and crashed inside CoreText, so the
    /// AppKit work is done once per size under this lock, and line widths are
    /// measured with CoreText, which is thread-safe.
    private static let metricsCache = Mutex<[Int: (font: NSFont, lineHeight: CGFloat)]>([:])

    static func metrics(_ size: Int) -> (font: NSFont, lineHeight: CGFloat) {
        metricsCache.withLock { cache in
            if let found = cache[size] { return found }
            let font = NSFont.monospacedSystemFont(ofSize: CGFloat(size), weight: .regular)
            let made = (font: font, lineHeight: ceil(NSLayoutManager().defaultLineHeight(for: font)))
            cache[size] = made
            return made
        }
    }

    /// How wide `line` is in `font`, the same as AppKit's measure.
    static func width(of line: String, in font: NSFont) -> CGFloat {
        let typeset = CTLineCreateWithAttributedString(
            CFAttributedStringCreate(nil, line as CFString, [kCTFontAttributeName: font] as CFDictionary))
        return CGFloat(CTLineGetTypographicBounds(typeset, nil, nil, nil))
    }

    /// Where line `index` starts: its top-left.
    func lineOrigin(_ index: Int) -> CGPoint {
        CGPoint(x: origin.x + Self.padding, y: origin.y + Self.padding + CGFloat(index) * lineHeight)
    }

    /// The block: the text plus padding, grown to whole grid steps so the
    /// frame lies on the grid.
    var rect: CGRect {
        let font = font
        let widest = lines.map { Self.width(of: $0, in: font) }.max() ?? 0
        let width = ceil(widest) + 2 * Self.padding
        let height = CGFloat(lines.count) * lineHeight + 2 * Self.padding
        let step = SchematicGrid.spacing
        return CGRect(x: origin.x, y: origin.y,
                      width: max((width / step).rounded(.up), 1) * step,
                      height: max((height / step).rounded(.up), 1) * step)
    }
}
