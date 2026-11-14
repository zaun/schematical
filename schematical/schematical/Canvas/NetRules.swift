import Foundation

/// How a layer rule holds a net to its layers.
nonisolated enum LayerRuleMode: String, CaseIterable, Hashable, Sendable {
    /// The net's copper may be only on these layers.
    case only
    /// The net's copper is cheaper on these layers; any trace layer is
    /// allowed.
    case prefer

    var title: String {
        switch self {
        case .only: "Only on"
        case .prefer: "Prefer"
        }
    }
}

/// The copper layers a net is held to (`only`) or prefers (`prefer`). Its
/// layers are kept when they leave the board, as a layer style is, and
/// ignored while they are gone (`SchematicDocument.effectiveLayers`).
nonisolated struct LayerRule: Hashable, Sendable {
    var mode: LayerRuleMode
    /// Not empty.
    var layers: [CopperLayer]
}

/// A layer rule for one net, before its class's rule.
nonisolated struct NetOverride: Hashable, Sendable {
    var net: PlaneNet
    var layerRule: LayerRule?
}

/// What a net group asks of its nets.
nonisolated enum NetGroupMode: String, CaseIterable, Hashable, Sendable {
    /// Every net ends within the tolerance of the longest.
    case lengthMatch
    /// Two nets routed side by side at the gap.
    case diffPair

    var title: String {
        switch self {
        case .lengthMatch: "Length Match"
        case .diffPair: "Differential Pair"
        }
    }
}

/// A set of nets routed together: an equal-length group, or a coupled
/// differential pair. A net is in at most one group. A pair has at most 2
/// members (exactly 2 to be routed); a length group needs 2 or more to do
/// anything. Whether each member is a two-terminal net is decided when
/// routing, not stored. A member that names no net any more stays.
nonisolated struct NetGroup: Identifiable, Hashable, Sendable {
    let id: UUID
    var name: String
    var mode: NetGroupMode
    var members: [PlaneNet] = []
    /// mm: how far apart the members' lengths may end.
    var tolerance: Double = NetGroup.defaultTolerance
    /// mm, a pair's: the edge-to-edge gap between its two traces.
    var gap: Double = NetGroup.defaultGap
    /// mm, a pair's: how much of it may run apart near its pads.
    var maxUncoupled: Double = NetGroup.defaultMaxUncoupled
    /// mm: the largest sideways size one tuning bump may reach.
    var maxMeander: Double = NetGroup.defaultMaxMeander

    static let toleranceRange: ClosedRange<Double> = 0.01...50
    static let gapRange: ClosedRange<Double> = 0.05...10
    static let maxUncoupledRange: ClosedRange<Double> = 0...50
    static let maxMeanderRange: ClosedRange<Double> = 0.5...50
    static let defaultTolerance = 0.25
    static let defaultGap = 0.2
    static let defaultMaxUncoupled = 5.0
    static let defaultMaxMeander = 4.0
    /// The most members a pair has.
    static let pairSize = 2

    /// Why these values can't make a group, as a file error detail after
    /// "net group <id> ", or nil.
    static func problem(mode: NetGroupMode, members: Int, tolerance: Double, gap: Double, maxUncoupled: Double,
                        maxMeander: Double, name: String) -> String? {
        func n(_ value: Double) -> String { String(format: "%g", value) }
        if name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty { return "has no name" }
        if !NetClass.nameLength.contains(name.count) { return "name must be 1 to 40 characters" }
        if mode == .diffPair, members > pairSize { return "pair has \(members) members; it can have at most 2" }
        if !toleranceRange.contains(tolerance) { return "tolerance \(n(tolerance)) must be 0.01 to 50 mm" }
        if !gapRange.contains(gap) { return "gap \(n(gap)) must be 0.05 to 10 mm" }
        if !maxUncoupledRange.contains(maxUncoupled) {
            return "maxUncoupled \(n(maxUncoupled)) must be 0 to 50 mm"
        }
        if !maxMeanderRange.contains(maxMeander) { return "maxMeander \(n(maxMeander)) must be 0.5 to 50 mm" }
        return nil
    }

    var problem: String? {
        Self.problem(mode: mode, members: members.count, tolerance: tolerance, gap: gap, maxUncoupled: maxUncoupled,
                     maxMeander: maxMeander, name: name)
    }
}

/// The layers a net may use (`allowed`, in stack order) and those it
/// prefers (`preferred`, a subset; empty is no preference).
nonisolated struct EffectiveLayers: Hashable, Sendable {
    var allowed: [CopperLayer]
    var preferred: [CopperLayer]
    /// The rule these came from, if any.
    var rule: LayerRule? = nil
}
