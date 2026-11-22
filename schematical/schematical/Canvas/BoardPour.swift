import CoreGraphics
import Foundation

/// How a pour joins the pads of its own net: a ring cut round each pad with
/// four spokes left across it (`thermal`), or copper right up to the pad
/// (`solid`). Raw values are the file values.
nonisolated enum PourConnection: String, CaseIterable, Hashable, Sendable {
    case thermal, solid

    var title: String {
        switch self {
        case .thermal: "Thermal Relief"
        case .solid: "Solid"
        }
    }
}

/// A copper pour (plan-52): a rectangle on one trace layer that is flood
/// filled with copper on one net. The fill keeps its clearance from the pads,
/// vias and traces of other nets and from keep-outs (`PourFiller`), and joins
/// the pads of its own net. Sizes in mm.
nonisolated struct BoardPour: Identifiable, Hashable, Sendable {
    let id: UUID
    /// mm; positive size, each side at least `minimumSide`.
    var rect: CGRect
    /// A trace layer of the board (never a Plane).
    var layer: CopperLayer
    /// The net the pour carries; nil is none, so it fills nothing.
    var net: PlaneNet? = nil
    /// The clearance to other nets, within `LayerStyle.clearanceRange`; nil is
    /// the DRC's min clearance.
    var clearance: Double? = nil
    var connection: PourConnection = .thermal
    /// The ring cut round a pad in thermal mode.
    var thermalGap: Double = 0.5
    /// The width of the four spokes across the ring.
    var spokeWidth: Double = 0.5
    /// A piece of fill whose bounding box is narrower than this is dropped.
    var minWidth: Double = 0.2
    /// Locked: dragging and the sidebar's X, Y, width and height leave it be.
    var locked = false

    static let minimumSide = 0.5
    static let gapRange: ClosedRange<Double> = 0.1...5
    static let spokeRange: ClosedRange<Double> = 0.1...5
    static let minWidthRange: ClosedRange<Double> = 0.05...2

    /// The clearance the pour keeps: its own, or the DRC's minimum.
    func clearance(rules: DesignRules) -> Double { clearance ?? rules.minClearance }

    /// `rect` with a positive size, each side at least `minimumSide`
    /// (grown to the right and down).
    static func normalized(_ rect: CGRect) -> CGRect {
        let r = rect.standardized
        return CGRect(x: r.minX, y: r.minY, width: max(r.width, minimumSide), height: max(r.height, minimumSide))
    }

    /// Why these values can't go together, as a file error detail after
    /// "pour <id> ", or nil.
    static func problem(rect: CGRect, clearance: Double?, thermalGap: Double, spokeWidth: Double,
                        minWidth: Double) -> String? {
        func n(_ value: Double) -> String { String(format: "%g", value) }
        if rect.width < minimumSide - 1e-9 { return "width \(n(rect.width)) must be at least 0.5 mm" }
        if rect.height < minimumSide - 1e-9 { return "height \(n(rect.height)) must be at least 0.5 mm" }
        if let clearance, !LayerStyle.clearanceRange.contains(clearance) {
            return "clearance \(n(clearance)) must be 0.05 to 10 mm"
        }
        if !gapRange.contains(thermalGap) { return "thermalGap \(n(thermalGap)) must be 0.1 to 5 mm" }
        if !spokeRange.contains(spokeWidth) { return "spokeWidth \(n(spokeWidth)) must be 0.1 to 5 mm" }
        if !minWidthRange.contains(minWidth) { return "minWidth \(n(minWidth)) must be 0.05 to 2 mm" }
        return nil
    }
}

/// A keep-out (plan-52): a rectangle that pours stay out of, on chosen
/// layers. It blocks copper pours only; traces, vias and parts may still go
/// there.
nonisolated struct BoardKeepout: Identifiable, Hashable, Sendable {
    let id: UUID
    /// mm; positive size, each side at least `BoardPour.minimumSide`.
    var rect: CGRect
    /// The layers it applies to; nil is every copper layer of the board.
    var layers: Set<CopperLayer>? = nil
    var locked = false

    func applies(to layer: CopperLayer) -> Bool { layers?.contains(layer) ?? true }

    /// Why this size is wrong, as a file error detail after "keepout <id> ",
    /// or nil.
    static func problem(rect: CGRect) -> String? {
        func n(_ value: Double) -> String { String(format: "%g", value) }
        if rect.width < BoardPour.minimumSide - 1e-9 { return "width \(n(rect.width)) must be at least 0.5 mm" }
        if rect.height < BoardPour.minimumSide - 1e-9 { return "height \(n(rect.height)) must be at least 0.5 mm" }
        return nil
    }
}
