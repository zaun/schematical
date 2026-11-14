import Foundation

/// A named set of routing rules shared by some nets: trace width,
/// clearance and via size. A net is in the first class, in list order,
/// any of whose members names it; a net in no class is in the implicit
/// Default class (`defaultWidth`, the DRC's minimum clearance, and the
/// default via). Members are `PlaneNet`s, the stable names a Plane uses, so
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

    static let defaultWidth = BoardTrace.defaultWidth
    static let defaultViaDrill = BoardVia.defaultDrill
    static let defaultViaDiameter = BoardVia.defaultDiameter
    static let nameLength = 1...40

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
