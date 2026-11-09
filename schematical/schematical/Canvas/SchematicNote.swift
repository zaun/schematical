import AppKit

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

    /// The font the canvas and exports draw the text in.
    var font: NSFont { NSFont.systemFont(ofSize: CGFloat(size)) }

    /// The text's lines, empty ones included.
    var lines: [String] {
        text.split(separator: "\n", omittingEmptySubsequences: false).map(String.init)
    }

    /// The distance from one line's top to the next one's.
    var lineHeight: CGFloat {
        ceil(NSLayoutManager().defaultLineHeight(for: font))
    }

    /// Where line `index` starts: its top-left.
    func lineOrigin(_ index: Int) -> CGPoint {
        CGPoint(x: origin.x + Self.padding, y: origin.y + Self.padding + CGFloat(index) * lineHeight)
    }

    /// The block: the text plus padding, grown to whole grid steps so the
    /// frame lies on the grid.
    var rect: CGRect {
        let attributes: [NSAttributedString.Key: Any] = [.font: font]
        let widest = lines.map { ($0 as NSString).size(withAttributes: attributes).width }.max() ?? 0
        let width = ceil(widest) + 2 * Self.padding
        let height = CGFloat(lines.count) * lineHeight + 2 * Self.padding
        let step = SchematicGrid.spacing
        return CGRect(x: origin.x, y: origin.y,
                      width: max((width / step).rounded(.up), 1) * step,
                      height: max((height / step).rounded(.up), 1) * step)
    }
}
