import CoreGraphics
import Foundation

/// Net classes, layer rules and net groups (plan-68-E): the board's rules
/// for the copper of each net. What a net's copper is comes in as a
/// `BoardConnectivity`; nothing here reads a document.
extension BoardSettings {
    /// A class or group name as entered: trimmed and cut to the longest
    /// allowed, or nil when blank.
    static func netClassName(_ name: String) -> String? {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        return trimmed.isEmpty ? nil : String(trimmed.prefix(NetClass.nameLength.upperBound))
    }

    func netClassIndex(_ id: UUID) -> Int? {
        netClasses.firstIndex { $0.id == id }
    }

    func netGroupIndex(_ id: UUID) -> Int? {
        netGroups.firstIndex { $0.id == id }
    }

    // MARK: Net classes

    /// Adds a class named `name` (blank is "Class") with the Default class's
    /// values and no nets, last in priority.
    mutating func addNetClass(named name: String) -> UUID {
        let start = NetRules.defaults(for: rules)
        let netClass = NetClass(id: UUID(), name: Self.netClassName(name) ?? "Class", width: start.width,
                                viaDrill: start.viaDrill, viaDiameter: start.viaDiameter)
        netClasses.append(netClass)
        return netClass.id
    }

    mutating func removeNetClass(_ id: UUID) {
        netClasses.removeAll { $0.id == id }
    }

    /// Renames a class; a blank name is ignored, a long one cut.
    mutating func renameNetClass(_ id: UUID, to name: String) {
        guard let index = netClassIndex(id), let value = Self.netClassName(name),
              netClasses[index].name != value else { return }
        netClasses[index].name = value
    }

    /// A class's trace width, clamped to `BoardTrace.widthRange`.
    mutating func setNetClassWidth(_ id: UUID, _ mil: Int) {
        guard let index = netClassIndex(id) else { return }
        let value = min(max(mil, BoardTrace.widthRange.lowerBound), BoardTrace.widthRange.upperBound)
        if netClasses[index].width != value { netClasses[index].width = value }
    }

    /// A class's clearance, clamped to `LayerStyle.clearanceRange`; nil is
    /// the DRC's minimum.
    mutating func setNetClassClearance(_ id: UUID, _ mm: Double?) {
        guard let index = netClassIndex(id) else { return }
        let range = LayerStyle.clearanceRange
        let value = mm.map { min(max($0, range.lowerBound), range.upperBound) }
        if netClasses[index].clearance != value { netClasses[index].clearance = value }
    }

    /// A class's via: the hole clamped to `BoardVia.drillRange`, then the
    /// metal to `BoardVia.diameterRange` and the thinnest ring round it.
    mutating func setNetClassVia(_ id: UUID, drill: Double, diameter: Double) {
        guard let index = netClassIndex(id) else { return }
        let hole = min(max(drill, BoardVia.drillRange.lowerBound), BoardVia.drillRange.upperBound,
                       BoardVia.diameterRange.upperBound - 2 * BoardVia.minimumRing)
        let least = max(BoardVia.diameterRange.lowerBound, hole + 2 * BoardVia.minimumRing)
        let metal = max(min(diameter, BoardVia.diameterRange.upperBound), least)
        if netClasses[index].viaDrill != hole { netClasses[index].viaDrill = hole }
        if netClasses[index].viaDiameter != metal { netClasses[index].viaDiameter = metal }
    }

    /// Puts `net` in class `classID`, out of every other; nil is the Default class.
    mutating func assignNet(_ net: PlaneNet, toClass classID: UUID?) {
        for index in netClasses.indices where netClasses[index].members.contains(net) {
            netClasses[index].members.removeAll { $0 == net }
        }
        if let classID, let index = netClassIndex(classID) { netClasses[index].members.append(net) }
    }

    /// Moves a class to `position` in the list (clamped); order is priority.
    mutating func moveNetClass(_ id: UUID, to position: Int) {
        guard let index = netClassIndex(id) else { return }
        let target = min(max(position, 0), netClasses.count - 1)
        guard target != index else { return }
        let netClass = netClasses.remove(at: index)
        netClasses.insert(netClass, at: target)
    }

    /// The class net `net` of `connectivity` is in: the first, in list
    /// order, any of whose members names it; nil is the Default class.
    func netClass(ofNet net: Int, in connectivity: BoardConnectivity) -> NetClass? {
        netClasses.first { $0.members.contains { connectivity.net(of: $0) == net } }
    }

    /// The width and via new copper on net `net` gets: its class's, or the
    /// Default class's.
    func netRules(ofNet net: Int?, in connectivity: BoardConnectivity) -> NetRules {
        guard let net, let netClass = netClass(ofNet: net, in: connectivity) else { return NetRules.defaults(for: rules) }
        return NetRules(width: netClass.width, clearance: netClass.clearance, viaDrill: netClass.viaDrill,
                        viaDiameter: netClass.viaDiameter)
    }

    // MARK: Layer rules

    /// A class's layer rule; nil is any trace layer. A rule with no layers
    /// is ignored.
    mutating func setNetClassLayerRule(_ id: UUID, _ rule: LayerRule?) {
        guard let index = netClassIndex(id), rule.map({ !$0.layers.isEmpty }) ?? true,
              netClasses[index].layerRule != rule else { return }
        netClasses[index].layerRule = rule
    }

    /// One net's own layer rule, before its class's; nil removes it.
    mutating func setNetLayerRule(_ net: PlaneNet, _ rule: LayerRule?) {
        guard rule.map({ !$0.layers.isEmpty }) ?? true else { return }
        guard let rule else {
            netOverrides.removeAll { $0.net == net }
            return
        }
        if let index = netOverrides.firstIndex(where: { $0.net == net }) {
            if netOverrides[index].layerRule != rule { netOverrides[index].layerRule = rule }
        } else {
            netOverrides.append(NetOverride(net: net, layerRule: rule))
        }
    }

    /// The layer rule net `net` of `connectivity` follows: its own override,
    /// else its class's, else none.
    func layerRule(ofNet net: Int, in connectivity: BoardConnectivity) -> LayerRule? {
        if let own = netOverrides.first(where: { $0.layerRule != nil && connectivity.net(of: $0.net) == net }) {
            return own.layerRule
        }
        return netClass(ofNet: net, in: connectivity)?.layerRule
    }

    /// The trace layers net `net` may use and those it prefers (see
    /// `SchematicDocument.effectiveLayers`).
    func effectiveLayers(ofNet net: Int, in connectivity: BoardConnectivity) -> EffectiveLayers {
        let trace = traceLayers
        guard let rule = layerRule(ofNet: net, in: connectivity) else {
            return EffectiveLayers(allowed: trace, preferred: [], rule: nil)
        }
        let named = trace.filter(rule.layers.contains)
        switch rule.mode {
        case .only: return EffectiveLayers(allowed: named, preferred: [], rule: rule)
        case .prefer: return EffectiveLayers(allowed: trace, preferred: named, rule: rule)
        }
    }

    // MARK: Net groups

    /// Adds an empty group (blank is "Group"), last.
    mutating func addNetGroup(named name: String, mode: NetGroupMode) -> UUID {
        let group = NetGroup(id: UUID(), name: Self.netClassName(name) ?? "Group", mode: mode)
        netGroups.append(group)
        return group.id
    }

    mutating func removeNetGroup(_ id: UUID) {
        netGroups.removeAll { $0.id == id }
    }

    mutating func renameNetGroup(_ id: UUID, to name: String) {
        guard let index = netGroupIndex(id), let value = Self.netClassName(name),
              netGroups[index].name != value else { return }
        netGroups[index].name = value
    }

    /// Why group `id` can't take `mode` (a pair has 2 nets), or nil.
    func netGroupModeProblem(_ id: UUID, _ mode: NetGroupMode) -> String? {
        guard let index = netGroupIndex(id) else { return nil }
        let group = netGroups[index]
        guard mode == .diffPair, group.members.count > NetGroup.pairSize else { return nil }
        return "A differential pair has 2 nets; \(group.name) has \(group.members.count)."
    }

    mutating func setNetGroupMode(_ id: UUID, _ mode: NetGroupMode) {
        guard let index = netGroupIndex(id), netGroups[index].mode != mode else { return }
        netGroups[index].mode = mode
    }

    /// One of a group's lengths, clamped to `range`.
    mutating func setNetGroupValue(_ id: UUID, _ key: WritableKeyPath<NetGroup, Double>, _ mm: Double,
                                   _ range: ClosedRange<Double>) {
        guard let index = netGroupIndex(id) else { return }
        let value = min(max(mm, range.lowerBound), range.upperBound)
        if netGroups[index][keyPath: key] != value { netGroups[index][keyPath: key] = value }
    }

    /// Why `net` can't join group `groupID` (a full pair), or nil.
    func netGroupJoinProblem(_ net: PlaneNet, _ groupID: UUID) -> String? {
        guard let index = netGroupIndex(groupID) else { return nil }
        let group = netGroups[index]
        guard !group.members.contains(net), group.mode == .diffPair, group.members.count >= NetGroup.pairSize else {
            return nil
        }
        return "\(group.name) already has its 2 nets."
    }

    /// Puts `net` in group `groupID`, out of any other.
    mutating func assignNet(_ net: PlaneNet, toGroup groupID: UUID) {
        guard let index = netGroupIndex(groupID), !netGroups[index].members.contains(net) else { return }
        unassignNet(net)
        netGroups[index].members.append(net)
    }

    /// Takes `net` out of whichever group it is in.
    mutating func unassignNet(_ net: PlaneNet) {
        for index in netGroups.indices where netGroups[index].members.contains(net) {
            netGroups[index].members.removeAll { $0 == net }
        }
    }

    /// The group net `net` of `connectivity` is in: the first any of whose
    /// members names it.
    func group(ofNet net: Int, in connectivity: BoardConnectivity) -> NetGroup? {
        netGroups.first { $0.members.contains { connectivity.net(of: $0) == net } }
    }
}
