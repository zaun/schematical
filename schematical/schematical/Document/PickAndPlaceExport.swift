import CoreGraphics
import Foundation

/// The pick-and-place (CPL) file of the Gerber export, in the layout JLCPCB and PCBWay assembly take:
/// one row per surface-mount part placed on the board. See `docs/export.md` § Pick and place.
///
/// Columns: `Designator,Mid X,Mid Y,Layer,Rotation` (JLCPCB's names; PCBWay reads the same content). The position is the footprint's centre
/// (`BoardPlacement.centre`, the body centre) in millimetres, in the Gerber files' frame (x as on the
/// board, y up from the board's bottom edge), so it lines up with the copper. `Layer` is `Top` or
/// `Bottom`. `Rotation` is degrees counter-clockwise as seen from the top, the footprint's angle in the
/// KiCad export (`(360 − clockwise turn) % 360`); a fab compares it with its own library's zero
/// direction, which it shows in its placement preview.
@MainActor
enum PickAndPlaceExport {
    // `file(for:)` reads the document, so the enum is main-actor isolated; `rows` and `csv` are pure.
    /// One part of the file.
    struct Row: Equatable {
        let designator: String
        let x: Double
        let y: Double
        let bottom: Bool
        let rotation: Int
    }

    /// The rows for `chips`, in natural reference order (C before R before U, U2 before U10). `top` is the
    /// board's top edge (mm), as the Gerber writer's: y is written as `top − board y`.
    nonisolated static func rows(for chips: [PlacedChip], top: Double) -> [Row] {
        chips.filter(\.isSurfaceMount).compactMap { chip in
            guard let placement = chip.board else { return nil }
            return Row(designator: chip.fabricationReference, x: Double(placement.centre.x),
                       y: top - Double(placement.centre.y), bottom: placement.side == .bottom,
                       rotation: (360 - placement.rotation) % 360)
        }
        .sorted { $0.designator.localizedStandardCompare($1.designator) == .orderedAscending }
    }

    /// The rows as CSV (RFC 4180, CRLF line ends, a header row), positions in mm to 4 decimals.
    nonisolated static func csv(_ rows: [Row]) -> String {
        let header = ["Designator", "Mid X", "Mid Y", "Layer", "Rotation"]
        let body = rows.map { row in
            [CSV.field(row.designator), number(row.x), number(row.y),
             row.bottom ? "Bottom" : "Top", String(row.rotation)]
        }
        return ([header] + body).map { $0.joined(separator: ",") }.joined(separator: "\r\n") + "\r\n"
    }

    /// The file for `document`, or nil when it has no surface-mount parts on the board.
    static func file(for document: SchematicDocument, base: String, top: Double) -> GerberExport.File? {
        let rows = rows(for: document.chips, top: top)
        guard !rows.isEmpty else { return nil }
        return GerberExport.File(name: "\(base)-CPL.csv", data: Data(csv(rows).utf8))
    }

    private nonisolated static func number(_ value: Double) -> String { KiCad.n(value) }
}

/// CSV text (RFC 4180).
nonisolated enum CSV {
    /// `text`, quoted when it holds a comma, a quote or a line break.
    static func field(_ text: String) -> String {
        guard text.contains(where: { $0 == "," || $0 == "\"" || $0.isNewline }) else { return text }
        return "\"" + text.replacingOccurrences(of: "\"", with: "\"\"") + "\""
    }
}

extension PlacedChip {
    /// The reference a fab sees (the placement file's designator, the Bill of
    /// Materials' socket line): the chip's, or `X` plus it for a chip in a socket
    /// mode, where the part on the board is the socket (`U1` becomes `XU1`;
    /// plan-51-H).
    nonisolated var fabricationReference: String {
        socket == nil ? reference : "X" + reference
    }

    /// Whether the part is placed on the board as a surface-mount part: it has a footprint with at least
    /// one pad and none of them drilled. A fab's machines place only these; through-hole parts are
    /// soldered by hand or not assembled.
    nonisolated var isSurfaceMount: Bool {
        guard board != nil else { return false }
        let pads = boardPads
        return !pads.isEmpty && pads.allSatisfy { $0.drill == 0 }
    }
}
