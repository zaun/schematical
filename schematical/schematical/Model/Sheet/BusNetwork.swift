import CoreGraphics
import Foundation

/// A sheet's buses and the rules for editing them (plan-68-D): joining a
/// drawn line, wire counts and names, linked buses (same name, ignoring
/// case), bus types, and bus line drags. Ports are in `BusNetwork+Ports`.
/// The card edge's bus comes in as `edge`; nothing here reads a document.
struct BusNetwork {
    var buses: [Bus]

    func bus(_ id: UUID) -> Bus? {
        buses.first { $0.id == id }
    }

    static let typeSetsWires = "The bus type sets its wires; set Type to Custom to change them."

    static func edgeBusUndeletable(_ bus: Bus) -> String {
        "\(bus.name) is in use as the card edge, so it can't be deleted. Set the PCB's Bus to None first."
    }

    static func edgeBusLock(_ bus: Bus) -> String {
        "\(bus.name) is in use as the card edge; its wire count is locked."
    }

    // MARK: Drawing

    /// Stores what a Bus tool drag made: `draft.bus` in place of the bus it
    /// joined, or added with the next free name. The bus it absorbed (if
    /// any) is still there; `removeAbsorbed` takes it out once its wires are
    /// moved. Refused, with why, when the joined bus is linked and the join
    /// would change its wires or type.
    mutating func place(_ draft: BusDraft) -> String? {
        if let index = draft.busIndex {
            let old = buses[index]
            if buses.contains(where: { old.isLinked(with: $0) }),
               draft.bus.wires.map(\.name) != old.wires.map(\.name) || draft.bus.card != old.card {
                return "\(old.name) is linked with another bus of its name, so joining can't change its wires."
            }
            buses[index] = draft.bus
        } else {
            var added = draft.bus
            added.name = Bus.defaultBusName(taken: buses.map(\.name))
            buses.append(added)
        }
        return nil
    }

    // MARK: Wires

    /// `index` and the buses linked with it (same name, ignoring case), in order.
    func linkGroup(_ index: Int) -> [Int] {
        buses.indices.filter { $0 == index || buses[index].isLinked(with: buses[$0]) }
    }

    /// Is one of these buses the card edge's?
    func groupHasEdge(_ group: [Int], edge: UUID?) -> Bool {
        guard let edge else { return false }
        return group.contains { buses[$0].id == edge }
    }

    /// The highest wire place a port on any of these buses uses.
    func highestUsedWire(_ group: [Int]) -> Int? {
        group.compactMap { i -> Int? in
            let used = buses[i].ports.reduce(into: Set<UUID>()) { $0.formUnion($1.wireIDs) }
            return buses[i].wires.lastIndex { used.contains($0.id) }
        }.max()
    }

    /// `bus` with `lead`'s type and wire names: its wires keep their places
    /// and ids, and are renamed, added or dropped at the end.
    static func matching(_ bus: Bus, _ lead: Bus) -> Bus {
        var result = bus
        result.wires = renamed(bus.wires, lead.wires.map(\.name))
        result.card = lead.card
        return result
    }

    /// `wires` renamed to `names` in order: kept places and ids, added or
    /// dropped at the end.
    static func renamed(_ wires: [BusWire], _ names: [String]) -> [BusWire] {
        var kept = Array(wires.prefix(names.count))
        for (i, name) in names.enumerated() {
            if i < kept.count { kept[i].name = name } else { kept.append(BusWire(name: name)) }
        }
        return kept
    }

    /// Gives every bus linked with `index` its type and wires.
    mutating func syncLinked(_ index: Int) {
        for other in linkGroup(index) where other != index {
            let matched = Self.matching(buses[other], buses[index])
            if matched != buses[other] { buses[other] = matched }
        }
    }

    /// Why a wire name is refused, or nil.
    static func wireNameProblem(_ name: String) -> String? {
        if name.isEmpty { return "A wire name can't be empty." }
        if name.count > Bus.nameLengthRange.upperBound {
            return "A wire name is at most \(Bus.nameLengthRange.upperBound) characters."
        }
        if name.contains(where: \.isWhitespace) { return "A wire name can't contain spaces." }
        return nil
    }

    /// Sets how many wires a bus carries, clamped to 2–999 (see
    /// `SchematicDocument.setBusWireCount`). Returns why when refused.
    mutating func setWireCount(_ id: UUID, _ count: Int, edge: UUID?) -> String? {
        guard let index = buses.firstIndex(where: { $0.id == id }) else { return nil }
        let bus = buses[index]
        let group = linkGroup(index)
        if groupHasEdge(group, edge: edge) { return Self.edgeBusLock(bus) }
        if !bus.card.isCustom { return Self.typeSetsWires }
        if let highest = highestUsedWire(group), count < highest + 1 {
            return "\(bus.wires[highest].name) is used by a port; this bus needs at least \(highest + 1) wires."
        }
        let wanted = min(max(count, Bus.wireCountRange.lowerBound), Bus.wireCountRange.upperBound)
        var wires = bus.wires
        if wanted < wires.count { wires.removeLast(wires.count - wanted) }
        while wires.count < wanted {
            wires.append(BusWire(name: Bus.defaultName(for: wires.count, taken: Set(wires.map(\.name)))))
        }
        buses[index].wires = wires
        syncLinked(index)
        return nil
    }

    /// Renames one wire of a bus. Returns why when the name is refused.
    mutating func renameWire(bus id: UUID, wire wireID: UUID, to name: String) -> String? {
        guard let index = buses.firstIndex(where: { $0.id == id }),
              let w = buses[index].wires.firstIndex(where: { $0.id == wireID }) else { return nil }
        if !buses[index].card.isCustom { return Self.typeSetsWires }
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        if let problem = Self.wireNameProblem(trimmed) { return problem }
        buses[index].wires[w].name = trimmed
        syncLinked(index)
        return nil
    }

    /// Sets a bus's wires to `names`, in order (the Bus panel's Import; see
    /// `SchematicDocument.setBusWireNames`). Returns why when refused.
    mutating func setWireNames(_ id: UUID, _ names: [String], edge: UUID?) -> String? {
        guard let index = buses.firstIndex(where: { $0.id == id }) else { return nil }
        let bus = buses[index]
        if !bus.card.isCustom { return Self.typeSetsWires }
        let names = names.map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
        guard Bus.wireCountRange.contains(names.count) else {
            return "A bus carries \(Bus.wireCountRange.lowerBound)–\(Bus.wireCountRange.upperBound) wires."
        }
        // Each rule over every name, in the order `wireNameProblem` checks them.
        for rule in [{ $0.isEmpty }, { $0.count > Bus.nameLengthRange.upperBound },
                     { $0.contains(where: \.isWhitespace) }] as [(String) -> Bool] {
            if let name = names.first(where: rule), let problem = Self.wireNameProblem(name) { return problem }
        }
        let group = linkGroup(index)
        if names.count != bus.wires.count {
            if groupHasEdge(group, edge: edge) { return Self.edgeBusLock(bus) }
            if let highest = highestUsedWire(group), names.count < highest + 1 {
                return "\(bus.wires[highest].name) is used by a port; this bus needs at least \(highest + 1) wires."
            }
        }
        buses[index].wires = Self.renamed(bus.wires, names)
        syncLinked(index)
        return nil
    }

    /// Moves one wire of a bus to place `to`, and the wire at the same place
    /// of every linked bus. Returns why when refused, and the ports whose
    /// pins moved (their wires need re-fitting).
    mutating func moveWire(bus id: UUID, wire wireID: UUID, to place: Int) -> (message: String?, ports: [UUID]) {
        guard let index = buses.firstIndex(where: { $0.id == id }),
              let from = buses[index].wires.firstIndex(where: { $0.id == wireID }) else { return (nil, []) }
        if !buses[index].card.isCustom { return (Self.typeSetsWires, []) }
        let to = min(max(place, 0), buses[index].wires.count - 1)
        guard to != from else { return (nil, []) }
        var ports: [UUID] = []
        for member in linkGroup(index) where from < buses[member].wires.count && to < buses[member].wires.count {
            let wire = buses[member].wires.remove(at: from)
            buses[member].wires.insert(wire, at: to)
            ports += buses[member].ports.map(\.id)
        }
        return (nil, ports)
    }

    /// The bus that renaming bus `id` to `name` would link it with: another
    /// bus of that name (ignoring case) not linked with it already.
    func linkTarget(_ id: UUID, named name: String) -> Bus? {
        guard let bus = bus(id) else { return nil }
        let lowered = name.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
        return buses.first { $0.id != id && !bus.isLinked(with: $0) && $0.name.lowercased() == lowered }
    }

    /// Renames a bus (see `SchematicDocument.setBusName`). Returns why when refused.
    mutating func setName(_ id: UUID, _ name: String, link: Bool, edge: UUID?) -> String? {
        guard let index = buses.firstIndex(where: { $0.id == id }) else { return nil }
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        if trimmed.isEmpty { return "A bus name can't be empty." }
        let bus = buses[index]
        if let target = linkTarget(id, named: trimmed) {
            guard link else { return "\(trimmed) is already used by another bus." }
            let count = target.wires.count
            if bus.id == edge, count != bus.wires.count {
                return "\(bus.name) is in use as the card edge; it can only link with a bus of \(bus.wires.count) wires."
            }
            let used = bus.ports.reduce(into: Set<UUID>()) { $0.formUnion($1.wireIDs) }
            if let highest = bus.wires.lastIndex(where: { used.contains($0.id) }), highest >= count {
                return "\(bus.wires[highest].name) is used by a port; \(target.name) has only \(count) wires."
            }
            buses[index] = Self.matching(bus, target)
            buses[index].name = target.name
            return nil
        }
        if trimmed.lowercased() == bus.name.lowercased() {
            for other in linkGroup(index) where buses[other].name != trimmed { buses[other].name = trimmed }
        } else if bus.name != trimmed {
            buses[index].name = trimmed
        }
        return nil
    }

    /// Sets what a bus carries (see `SchematicDocument.setBusCard`). Returns
    /// why when refused.
    mutating func setCard(_ id: UUID, _ card: BusCard, edge: UUID?) -> String? {
        guard let index = buses.firstIndex(where: { $0.id == id }), buses[index].card != card else { return nil }
        guard let names = card.pinNames else {
            buses[index].card = card
            syncLinked(index)
            return nil
        }
        let bus = buses[index]
        guard Bus.wireCountRange.contains(names.count) else {
            return "A bus needs \(Bus.wireCountRange.lowerBound) to \(Bus.wireCountRange.upperBound) wires; this card has \(names.count) pins."
        }
        let group = linkGroup(index)
        if groupHasEdge(group, edge: edge), names.count != bus.wires.count { return Self.edgeBusLock(bus) }
        if let highest = highestUsedWire(group), highest >= names.count {
            return "\(bus.wires[highest].name) is used by a port; this card has only \(names.count) pins."
        }
        buses[index].wires = Self.renamed(bus.wires, names)
        buses[index].card = card
        syncLinked(index)
        return nil
    }

    // MARK: Lines

    /// Whether no line end of one bus touches a line of another.
    static func areApart(_ buses: [Bus]) -> Bool {
        Bus.firstTouch(buses) == nil
    }

    /// The one way a drag changes a bus (see `SchematicDocument.setBusLines`).
    /// Returns whether it stored the lines.
    mutating func setLines(_ id: UUID, _ lines: [BusLine]) -> Bool {
        guard let index = buses.firstIndex(where: { $0.id == id }) else { return false }
        let old = buses[index]
        guard lines.map(\.id) == old.lines.map(\.id) else { return false }
        guard !zip(old.lines, lines).contains(where: { $0.isHorizontal != $1.isHorizontal || $0.direction != $1.direction }) else { return false }
        var edited = old
        edited.lines = lines
        guard edited.isValid, edited.jointShapes == old.jointShapes, edited.portsFit else { return false }
        var all = buses
        all[index] = edited
        guard Self.areApart(all) else { return false }
        buses[index] = edited
        return true
    }

    /// The bus line under `point`, if any (topmost bus first).
    func busHit(at point: CGPoint, tolerance: CGFloat) -> (bus: UUID, line: UUID)? {
        for bus in buses.reversed() {
            for line in bus.lines where SchematicDocument.distance(from: point, toSegment: line.a, line.b) <= tolerance {
                return (bus.id, line.id)
            }
        }
        return nil
    }

    /// The bus line end under `point`, if any.
    func busEndHit(at point: CGPoint, tolerance: CGFloat) -> (bus: UUID, line: UUID, end: BusEnd)? {
        for bus in buses.reversed() {
            for line in bus.lines {
                for end in BusEnd.allCases where SchematicDocument.distance(from: point, to: line.point(end)) <= tolerance {
                    return (bus.id, line.id, end)
                }
            }
        }
        return nil
    }
}
