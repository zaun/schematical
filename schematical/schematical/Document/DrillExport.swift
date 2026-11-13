import CoreGraphics
import Foundation

/// The board's holes as Excellon drill files, for the Gerber zip: one for
/// plated holes (through-hole pads, vias, plated holes and header pads)
/// and one for non-plated ones (bare holes, mounting holes). Metric,
/// absolute, decimal coordinates, in the Gerber files' frame (y up from
/// the board's bottom edge). A slot is a routed `G85` slot. See
/// `docs/export.md` § Drill files.
nonisolated enum DrillExport {
    /// One hole: a round hole (`a` = `b`) or a slot from `a` to `b`.
    struct Hole: Equatable {
        let a: CGPoint
        let b: CGPoint
        /// mm.
        let diameter: Double

        var isSlot: Bool { !TraceRoute.same(a, b) }
    }

    /// The plated and non-plated holes: every pad with a drill, and every
    /// via (plated).
    static func holes(_ copper: BoardConnectivity) -> (plated: [Hole], bare: [Hole]) {
        var plated: [Hole] = [], bare: [Hole] = []
        for entry in copper.pads {
            guard let hole = entry.pad.holeSegment else { continue }
            let drill = Hole(a: hole.a, b: hole.b, diameter: 2 * hole.radius)
            if entry.pad.plated { plated.append(drill) } else { bare.append(drill) }
        }
        for via in copper.vias {
            plated.append(Hole(a: via.centre, b: via.centre, diameter: via.drill))
        }
        return (plated, bare)
    }

    /// One Excellon file: a tool per diameter (smallest first, to 1 µm),
    /// its round holes, then its slots. `top` is the board's top edge
    /// (mm), as for `GerberWriter`; `layers` the copper layer count.
    static func file(_ holes: [Hole], plated: Bool, layers: Int, top: Double, software: String) -> Data {
        let kind = plated ? "Plated,1,\(layers),PTH" : "NonPlated,1,\(layers),NPTH"
        var lines = ["M48",
                     "; DRILL file {\(software)}",
                     "; FORMAT={-:-/ absolute / metric / decimal}",
                     "; #@! TF.GenerationSoftware,jgz,\(software)",
                     "; #@! TF.FileFunction,\(kind)",
                     "FMAT,2",
                     "METRIC"]
        let sizes = Array(Set(holes.map { micrometres($0.diameter) })).sorted()
        for (index, size) in sizes.enumerated() {
            lines.append("T\(index + 1)C\(number(Double(size) / 1000))")
        }
        lines += ["%", "G90", "G05"]
        for (index, size) in sizes.enumerated() {
            let tool = holes.filter { micrometres($0.diameter) == size }
            lines.append("T\(index + 1)")
            for hole in tool where !hole.isSlot { lines.append(xy(hole.a, top: top)) }
            for hole in tool where hole.isSlot { lines.append("\(xy(hole.a, top: top))G85\(xy(hole.b, top: top))") }
        }
        lines += ["T0", "M30"]
        return Data((lines.joined(separator: "\n") + "\n").utf8)
    }

    private static func micrometres(_ mm: Double) -> Int { Int((mm * 1000).rounded()) }

    private static func xy(_ point: CGPoint, top: Double) -> String {
        "X\(number(Double(point.x)))Y\(number(top - Double(point.y)))"
    }

    /// mm to 0.1 µm, 3 or 4 decimals: "1.200", "0.0005".
    static func number(_ mm: Double) -> String {
        var text = String(format: "%.4f", mm)
        while text.hasSuffix("0"), let dot = text.firstIndex(of: "."), text.distance(from: dot, to: text.endIndex) > 4 {
            text.removeLast()
        }
        return text == "-0.000" ? "0.000" : text
    }
}
