import CoreGraphics
import SwiftUI

/// Where everything in the title block goes (plan-64-C). Pure: the canvas and
/// every export draw from one layout.
///
/// A fixed 360 wide, in rows of cells separated by lines:
///
///     Title (360)
///     Author (240) | Revision (120)
///     Created (180) | Updated (180)
///     Description (360)
///
/// A cell is its label (8 pt) over its value, wrapped to the cell. A row is as
/// tall as its tallest cell, grown to whole grid steps, so the frame and
/// every line lie on the grid. Text is measured with CoreText under
/// `SchematicNote`'s lock: the layout runs on the main thread and on
/// SwiftUI's render thread.
nonisolated struct TitleBlockLayout: Equatable {
    /// One cell: its box, its label and the wrapped lines of its value.
    struct Cell: Equatable {
        var rect: CGRect
        var label: String
        var lines: [String]
        /// The value's font size in points.
        var size: Int
        var weight: Font.Weight
        /// The label's top-left.
        var labelOrigin: CGPoint
        /// The value's first line's top-left.
        var textOrigin: CGPoint
        var lineHeight: CGFloat
    }

    static let width: CGFloat = 360
    static let padding = SchematicNote.padding
    static let labelSize = 8
    static let valueSize = 12
    static let titleSize = 14

    let origin: CGPoint
    /// The frame.
    let rect: CGRect
    let cells: [Cell]
    /// The lines between rows and between the cells of a row.
    let separators: [(CGPoint, CGPoint)]

    static func == (a: TitleBlockLayout, b: TitleBlockLayout) -> Bool {
        a.origin == b.origin && a.rect == b.rect && a.cells == b.cells
            && a.separators.count == b.separators.count
            && zip(a.separators, b.separators).allSatisfy { $0.0 == $1.0 && $0.1 == $1.1 }
    }

    init(_ info: DocumentInfo, origin: CGPoint) {
        let rows: [[(label: String, text: String, width: CGFloat, size: Int, weight: Font.Weight)]] = [
            [("Title", info.title, 360, Self.titleSize, .semibold)],
            [("Author", info.author, 240, Self.valueSize, .regular),
             ("Revision", info.revision, 120, Self.valueSize, .regular)],
            [("Created", DocumentInfo.dateText(info.created), 180, Self.valueSize, .regular),
             ("Updated", DocumentInfo.dateText(info.updated), 180, Self.valueSize, .regular)],
            [("Description", info.description, 360, Self.valueSize, .regular)],
        ]
        let step = SchematicGrid.spacing
        let labelHeight = SchematicNote.metrics(Self.labelSize).lineHeight
        var cells: [Cell] = []
        var separators: [(CGPoint, CGPoint)] = []
        var y = origin.y
        for (rowIndex, row) in rows.enumerated() {
            let wrapped = row.map { Self.wrap($0.text, width: $0.width - 2 * Self.padding, size: $0.size) }
            var height: CGFloat = 0
            for (cell, lines) in zip(row, wrapped) {
                let lineHeight = SchematicNote.metrics(cell.size).lineHeight
                height = max(height, 2 * Self.padding + labelHeight + CGFloat(lines.count) * lineHeight)
            }
            height = (height / step).rounded(.up) * step
            var x = origin.x
            for (column, (cell, lines)) in zip(row, wrapped).enumerated() {
                let box = CGRect(x: x, y: y, width: cell.width, height: height)
                cells.append(Cell(
                    rect: box, label: cell.label, lines: lines, size: cell.size, weight: cell.weight,
                    labelOrigin: CGPoint(x: x + Self.padding, y: y + Self.padding),
                    textOrigin: CGPoint(x: x + Self.padding, y: y + Self.padding + labelHeight),
                    lineHeight: SchematicNote.metrics(cell.size).lineHeight))
                if column > 0 { separators.append((CGPoint(x: x, y: y), CGPoint(x: x, y: y + height))) }
                x += cell.width
            }
            y += height
            if rowIndex < rows.count - 1 {
                separators.append((CGPoint(x: origin.x, y: y), CGPoint(x: origin.x + Self.width, y: y)))
            }
        }
        self.origin = origin
        self.cells = cells
        self.separators = separators
        rect = CGRect(x: origin.x, y: origin.y, width: Self.width, height: y - origin.y)
    }

    /// `text` broken into lines no wider than `width` at `size` points,
    /// monospaced: at line breaks, then greedily at spaces; a word wider than
    /// a line is cut by characters. An empty text is one empty line.
    static func wrap(_ text: String, width: CGFloat, size: Int) -> [String] {
        let font = SchematicNote.metrics(size).font
        func fits(_ line: String) -> Bool { SchematicNote.width(of: line, in: font) <= width }
        var lines: [String] = []
        for paragraph in text.split(separator: "\n", omittingEmptySubsequences: false) {
            var current = ""
            for word in paragraph.split(separator: " ", omittingEmptySubsequences: false).map(String.init) {
                let joined = current.isEmpty ? word : current + " " + word
                if fits(joined) {
                    current = joined
                    continue
                }
                if !current.isEmpty { lines.append(current) }
                current = ""
                // A word that is wider than a line on its own: cut it.
                var rest = Substring(word)
                while !rest.isEmpty, !fits(String(rest)) {
                    var count = rest.count - 1
                    while count > 1, !fits(String(rest.prefix(count))) { count -= 1 }
                    lines.append(String(rest.prefix(count)))
                    rest = rest.dropFirst(count)
                }
                current = String(rest)
            }
            lines.append(current)
        }
        return lines.isEmpty ? [""] : lines
    }
}
