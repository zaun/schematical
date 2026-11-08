import Foundation

/// The generic pin header (plan-23): 80 backend parts, one per size
/// (`GENERIC-HEADER-1X01` … `-2X40`), shown as one drawer row. A placed
/// header changes size by swapping its part (`SchematicDocument.setHeaderSize`).
nonisolated enum HeaderFamily {
    /// The drawer row's title.
    static let title = "GENERIC-HEADER"
    /// The part a drop places: 1×2.
    static let defaultPart = "GENERIC-HEADER-1X02"
    static let maxPerRow = 40
    /// The drawer row's second line.
    static let subtitle = "1 or 2 rows · 1–40 pins per row · 2 footprints"

    /// The part number of a size: `name(rows: 2, perRow: 8)` is
    /// "GENERIC-HEADER-2X08". The catalog has it only for 1–2 rows and
    /// 1–40 per row.
    static func name(rows: Int, perRow: Int) -> String {
        "GENERIC-HEADER-\(rows)X" + (perRow < 10 && perRow >= 0 ? "0\(perRow)" : "\(perRow)")
    }
}
