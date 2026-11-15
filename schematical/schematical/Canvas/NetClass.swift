import Foundation

/// The rules new copper on a net gets from its class, or the Default
/// class's (`defaults(for:)`).
nonisolated struct NetRules: Hashable, Sendable {
    /// mil.
    var width: Int
    /// mm; nil is the DRC's minimum.
    var clearance: Double? = nil
    var viaDrill: Double
    var viaDiameter: Double

    /// The Default class under `rules`: the smallest copper the design
    /// rules allow, so a net in no class is always legal for the chosen
    /// manufacturer and leaves the most room. The width is the narrowest
    /// whole mil at or over the min trace width (never under 5 mil, a
    /// trace's least); the via is the min via drill with the widest of the
    /// min via diameter and the drill plus the min annular ring each side;
    /// the clearance is the DRC's minimum.
    static func defaults(for rules: DesignRules) -> NetRules {
        let mil = Int((rules.minTraceWidth / 0.0254 - 1e-6).rounded(.up))
        let width = min(max(mil, BoardTrace.widthRange.lowerBound), BoardTrace.widthRange.upperBound)
        let drill = min(max(rules.minViaDrill, BoardVia.drillRange.lowerBound), BoardVia.drillRange.upperBound)
        let ring = max(rules.minAnnularRing, BoardVia.minimumRing)
        let least = max(rules.minViaDiameter, drill + 2 * ring, BoardVia.diameterRange.lowerBound)
        // To the micron, upwards, so the sum's rounding never leaves it under a minimum.
        let diameter = min((least * 1000 - 1e-6).rounded(.up) / 1000, BoardVia.diameterRange.upperBound)
        return NetRules(width: width, clearance: nil, viaDrill: drill, viaDiameter: diameter)
    }
}

/// A named set of routing rules shared by some nets: trace width,
/// clearance and via size. A net is in the first class, in list order,
/// any of whose members names it; a net in no class is in the implicit
/// Default class (`NetRules.defaults(for:)`: the design rules' minimum
/// width, clearance and via). Members are `PlaneNet`s, the stable names a Plane uses, so
/// a class keeps its nets across rewiring of their other pins. A class
/// whose members no longer name any net stays: it is the user's data.
nonisolated struct NetClass: Identifiable, Hashable, Sendable {
    let id: UUID
    var name: String
    /// mil, within `BoardTrace.widthRange`.
    var width: Int = NetClass.defaultWidth
    /// mm, within `LayerStyle.clearanceRange`; nil is the DRC's minimum.
    var clearance: Double? = nil
    /// mm; together a legal via (`BoardVia.problem`).
    var viaDrill: Double = NetClass.defaultViaDrill
    var viaDiameter: Double = NetClass.defaultViaDiameter
    /// The nets in the class, in the order they were added.
    var members: [PlaneNet] = []
    /// The layers the class's nets are held to or prefer; nil is any
    /// trace layer. A net's own override (`BoardSettings.netOverrides`)
    /// comes first.
    var layerRule: LayerRule? = nil

    /// What a class made from values alone starts with (the file's defaults
    /// for a trace and a via); a class the user adds starts as the Default
    /// class (`SchematicDocument.addNetClass`).
    static let defaultWidth = BoardTrace.defaultWidth
    static let defaultViaDrill = BoardVia.defaultDrill
    static let defaultViaDiameter = BoardVia.defaultDiameter
    static let nameLength = 1...40
    /// The class "Add Power Class" makes (`SchematicDocument.addPowerClass`).
    static let powerName = "Power"
    static let powerWidth = 20

    /// The clearance the class keeps: its own, or the DRC's minimum.
    func clearance(rules: DesignRules) -> Double { clearance ?? rules.minClearance }

    /// Why these values can't make a class, as a file error detail after
    /// "net class <id> ", or nil.
    static func problem(width: Int, clearance: Double?, viaDrill: Double, viaDiameter: Double,
                        name: String) -> String? {
        func n(_ value: Double) -> String { String(format: "%g", value) }
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        if trimmed.isEmpty { return "has no name" }
        if !nameLength.contains(name.count) { return "name must be 1 to 40 characters" }
        if !BoardTrace.widthRange.contains(width) { return "width \(width) must be 5 to 100 mil" }
        if let clearance, !LayerStyle.clearanceRange.contains(clearance) {
            return "clearance \(n(clearance)) must be 0.05 to 10 mm"
        }
        if let via = BoardVia.problem(drill: viaDrill, diameter: viaDiameter) { return "via " + via }
        return nil
    }

    /// Why this class isn't legal, or nil.
    var problem: String? {
        Self.problem(width: width, clearance: clearance, viaDrill: viaDrill, viaDiameter: viaDiameter, name: name)
    }
}
