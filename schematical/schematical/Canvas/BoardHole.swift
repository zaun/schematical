import CoreGraphics
import Foundation

/// The size of a board hole: a common screw's, which sets the hole and its
/// copper land, or Custom, whose sizes are set by hand. Raw values are the
/// file values.
///
/// The holes are close-fit clearance holes: ISO 273 fine for metric
/// screws, the usual number drill for UNC ones (#42, #32, #27, #18, #9). The
/// land is as wide as the screw's pan head (ISO 7045, ASME B18.6.3),
/// rounded up to 0.5 mm, so the head sits on copper.
nonisolated enum HoleSize: String, CaseIterable, Hashable, Sendable {
    case custom
    case m2, m2_5 = "m2.5", m3, m4, m5, m6
    case no2 = "#2", no4 = "#4", no6 = "#6", no8 = "#8", no10 = "#10"

    var title: String {
        switch self {
        case .custom: "Custom"
        case .m2: "M2"
        case .m2_5: "M2.5"
        case .m3: "M3"
        case .m4: "M4"
        case .m5: "M5"
        case .m6: "M6"
        case .no2: "#2"
        case .no4: "#4"
        case .no6: "#6"
        case .no8: "#8"
        case .no10: "#10"
        }
    }

    /// The hole and land diameters (mm); nil for Custom.
    var sizes: (drill: Double, diameter: Double)? {
        switch self {
        case .custom: nil
        case .m2: (2.2, 4.0)
        case .m2_5: (2.7, 5.0)
        case .m3: (3.2, 6.0)
        case .m4: (4.3, 8.0)
        case .m5: (5.3, 10.0)
        case .m6: (6.4, 12.0)
        case .no2: (2.4, 4.5)
        case .no4: (3.0, 6.0)
        case .no6: (3.7, 7.0)
        case .no8: (4.3, 8.5)
        case .no10: (5.0, 9.5)
        }
    }
}

/// A hole drilled through the board: a mounting hole for a screw, or any
/// other hole. Non-plated, it is a bare hole with no copper. Plated, it has
/// a copper land on every layer that can carry a net (ground, a power rail
/// or a signal net, as a Plane does), and traces join it at four points on
/// its land: right, below, left and above its hole. Sizes in mm.
nonisolated struct BoardHole: Identifiable, Hashable, Sendable {
    let id: UUID
    /// mm. Placed and moved on the trace grid.
    var centre: CGPoint
    /// A screw size, or Custom. A screw size sets `drill` and `diameter`;
    /// changing either makes it Custom.
    var size: HoleSize = .m3
    /// The hole's diameter, within `drillRange`.
    var drill: Double = 3.2
    /// The land's outer diameter when plated, within `diameterRange` and at
    /// least `minimumRing` × 2 more than the hole. Kept while not plated,
    /// so it comes back.
    var diameter: Double = 6.0
    var plated = false
    /// A plated hole's net; nil is none. Kept while not plated.
    var net: PlaneNet? = nil
    /// Locked: dragging and the sidebar's X and Y leave it where it is.
    var locked = false

    static let drillRange: ClosedRange<Double> = 0.3...10
    static let diameterRange: ClosedRange<Double> = 0.5...20
    /// The thinnest ring of copper round a plated hole.
    static let minimumRing = 0.1

    /// The copper round the hole, each side (plated).
    var ring: Double { (diameter - drill) / 2 }

    /// What it covers: its land when plated, else its hole.
    var outer: Double { plated ? diameter : drill }

    /// The square bounds of what it covers.
    var rect: CGRect {
        CGRect(x: centre.x - outer / 2, y: centre.y - outer / 2, width: outer, height: outer)
    }

    /// Where traces join a plated hole: the middle of its ring to the
    /// right, below, left and above the hole. None when not plated.
    var contacts: [CGPoint] {
        guard plated else { return [] }
        let r = (drill + diameter) / 4
        return [CGPoint(x: centre.x + r, y: centre.y), CGPoint(x: centre.x, y: centre.y + r),
                CGPoint(x: centre.x - r, y: centre.y), CGPoint(x: centre.x, y: centre.y - r)]
    }

    /// The contact nearest `point`; nil when not plated.
    func contact(nearest point: CGPoint) -> CGPoint? {
        contacts.min { hypot($0.x - point.x, $0.y - point.y) < hypot($1.x - point.x, $1.y - point.y) }
    }

    /// As a part's pad, for connectivity, design rules and drawing: a round
    /// land through every layer, or a bare hole.
    var pad: PlacedPad {
        PlacedPad(designator: nil, rect: rect, drill: drill, shape: .round, radius: 0, chamfer: 0, corners: [],
                  plated: plated)
    }

    /// Sets the size: a screw size sets the hole and land; Custom keeps
    /// them.
    mutating func apply(_ size: HoleSize) {
        self.size = size
        if let sizes = size.sizes { (drill, diameter) = sizes }
    }

    /// Why these sizes can't go together, as a file error detail after
    /// "hole <id> ", or nil.
    static func problem(drill: Double, diameter: Double) -> String? {
        func n(_ value: Double) -> String { String(format: "%g", value) }
        if !drillRange.contains(drill) { return "drill \(n(drill)) must be 0.3 to 10 mm" }
        if !diameterRange.contains(diameter) { return "diameter \(n(diameter)) must be 0.5 to 20 mm" }
        if diameter < drill + 2 * minimumRing - 1e-9 {
            return "diameter \(n(diameter)) must be at least 0.2 mm more than its drill \(n(drill))"
        }
        return nil
    }
}
