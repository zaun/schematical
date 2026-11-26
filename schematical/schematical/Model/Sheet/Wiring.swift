import CoreGraphics
import Foundation

/// A sheet's wires and junctions, and the rules for editing them (plan-68-D):
/// how a wire is routed between its ends, how a point on a wire becomes a
/// junction, how loose ends are made and adopted. Pins, bus port pins and
/// label points come in through `PinTips`; nothing here reads a document.
struct Wiring {
    var wires: [Wire]
    var junctions: [Junction]
    /// The sheet's route cache, shared: not part of the value's content.
    var routes = WireRoutes()

    // MARK: Tips and routes

    /// Where a wire leaves or enters one of its ends.
    func endTip(_ end: WireEnd, _ tips: PinTips) -> CGPoint? {
        switch end {
        case let .pin(ref): tips.pin(ref)
        case let .junction(id): junctions.first { $0.id == id }?.point
        case let .busPin(ref): tips.busPin(ref)
        case let .label(id): tips.label(id)
        }
    }

    /// The point a line would start or end at.
    func anchorTip(_ anchor: LineAnchor, _ tips: PinTips) -> CGPoint? {
        switch anchor {
        case let .pin(ref): tips.pin(ref)
        case let .point(point): point
        case let .busPin(ref): tips.busPin(ref)
        case let .label(id): tips.label(id)
        }
    }

    /// Full route of a wire, from one end's tip to the other; empty when an
    /// end does not resolve.
    func path(for wire: Wire, _ tips: PinTips) -> [CGPoint] {
        guard let start = endTip(wire.start, tips), let end = endTip(wire.end, tips) else { return [] }
        return routes.path(for: wire, start: start, end: end, wireCount: wires.count)
    }

    func path(forWire id: UUID, _ tips: PinTips) -> [CGPoint] {
        guard let wire = wires.first(where: { $0.id == id }) else { return [] }
        return path(for: wire, tips)
    }

    /// Per part, the pins (`ChipPin.index`) a wire ends on.
    var connectedPins: [UUID: Set<Int>] {
        var pins: [UUID: Set<Int>] = [:]
        for wire in wires {
            for end in [wire.start, wire.end] {
                if case .pin(let ref) = end { pins[ref.chipID, default: []].insert(ref.index) }
            }
        }
        return pins
    }

    /// Whether either end of the wire is on a loose junction (plan-30).
    func hasLooseEnd(_ wire: Wire) -> Bool {
        [wire.start, wire.end].contains { end in
            guard case let .junction(id) = end else { return false }
            return junctions.contains { $0.id == id && $0.isLoose }
        }
    }

    /// Whether a junction gets a dot: always for a plain junction; for a
    /// loose one only where 3 or more wire ends meet, since a dot reads as a
    /// connection.
    func showsDot(_ junction: Junction) -> Bool {
        guard junction.isLoose else { return true }
        let end = WireEnd.junction(junction.id)
        return wires.reduce(0) { $0 + ($1.start == end ? 1 : 0) + ($1.end == end ? 1 : 0) } >= 3
    }

    // MARK: Hits

    /// A bend point on a wire, if `point` is on one.
    func nodeHit(at point: CGPoint, tolerance: CGFloat, _ tips: PinTips) -> (wire: UUID, index: Int)? {
        for wire in wires.reversed() {
            let path = path(for: wire, tips)
            let bends = path.indices.dropFirst().dropLast()
            if let index = bends.first(where: { SchematicDocument.distance(from: point, to: path[$0]) <= tolerance }) {
                return (wire.id, index)
            }
        }
        return nil
    }

    /// A segment of a wire, if `point` is on one. `index` is the segment's
    /// start point within the wire's path.
    func segmentHit(at point: CGPoint, tolerance: CGFloat, _ tips: PinTips) -> (wire: UUID, index: Int)? {
        for wire in wires.reversed() {
            let path = path(for: wire, tips)
            for index in 0..<max(path.count - 1, 0)
                where SchematicDocument.distance(from: point, toSegment: path[index], path[index + 1]) <= tolerance {
                return (wire.id, index)
            }
        }
        return nil
    }

    /// The wire passing through `point`, and the segment it's on.
    func wireHit(at point: CGPoint, _ tips: PinTips) -> (wire: Wire, index: Int)? {
        guard let hit = segmentHit(at: point, tolerance: 0.5, tips),
              let wire = wires.first(where: { $0.id == hit.wire }) else { return nil }
        return (wire, hit.index)
    }

    // MARK: Connecting

    /// A wire end in the same net as the end `anchor` would get, without
    /// changing anything: a point on a wire stands in as that wire's start.
    func standIn(for anchor: LineAnchor, _ tips: PinTips) -> WireEnd? {
        switch anchor {
        case let .pin(ref): return .pin(ref)
        case let .point(point):
            if let existing = junctions.first(where: { $0.point == point }) { return .junction(existing.id) }
            if let hit = wireHit(at: point, tips) { return hit.wire.start }
            return .junction(UUID())
        case let .busPin(ref): return .busPin(ref)
        case let .label(id): return .label(id)
        }
    }

    /// The wire end for an anchor. Creates a junction when the anchor is a
    /// point on a wire.
    mutating func endpoint(for anchor: LineAnchor, _ tips: PinTips) -> WireEnd? {
        switch anchor {
        case let .pin(ref): .pin(ref)
        case let .point(point): junction(at: point, tips).map { WireEnd.junction($0) }
        case let .busPin(ref): .busPin(ref)
        case let .label(id): .label(id)
        }
    }

    /// The junction at `point`, creating one if an existing wire passes
    /// through it: the wire is split into two wires that meet there.
    mutating func junction(at point: CGPoint, _ tips: PinTips) -> UUID? {
        if let existing = junctions.first(where: { $0.point == point }) { return existing.id }
        guard let hit = wireHit(at: point, tips),
              let index = wires.firstIndex(where: { $0.id == hit.wire.id }) else { return nil }
        var path = path(forWire: hit.wire.id, tips)
        if !path.contains(point) { path.insert(point, at: hit.index + 1) }
        guard let k = path.firstIndex(of: point), k > 0, k < path.count - 1 else { return nil }

        let created = Junction(point: point)
        junctions.append(created)
        // A wire with no stored bends only shows a midpoint (`settled`);
        // neither half keeps it, so each straight half stays one segment.
        func bends(_ piece: ArraySlice<CGPoint>) -> [CGPoint] {
            let points = Array(piece)
            let kept = hit.wire.interior.isEmpty ? OrthogonalRoute.simplified(points) : points
            return Array(kept.dropFirst().dropLast())
        }
        let original = wires[index]
        wires[index].end = .junction(created.id)
        wires[index].interior = bends(path[0...k])
        wires.insert(Wire(start: .junction(created.id), end: original.end, interior: bends(path[k...])), at: index + 1)
        return created.id
    }

    /// Adds a wire from `start` to `end` (a point on a wire becomes a
    /// junction there), along `path`'s bends or an L leaving and entering
    /// along `first` and `last`. False, with any split for the start joined
    /// back up, when an end does not resolve.
    @discardableResult
    mutating func connect(_ start: LineAnchor, _ end: LineAnchor, path: [CGPoint]?, first: RouteAxis?,
                          last: RouteAxis?, _ tips: PinTips) -> Bool {
        // Resolve the start first, so a split for it is in place before the end is looked up.
        guard let from = endpoint(for: start, tips), let to = endpoint(for: end, tips),
              let fromTip = endTip(from, tips), let toTip = endTip(to, tips) else {
            joinThroughJunctions(tips)
            return false
        }
        // The ends are always the tips; a drawn path gives the bends between.
        let bends = path.map { Array($0.dropFirst().dropLast()) }
            ?? Array(OrthogonalRoute.leg(from: fromTip, to: toTip, first: first, last: last,
                                         preferred: .horizontal).dropLast())
        let route = OrthogonalRoute.simplified([fromTip] + bends + [toTip])
        wires.append(Wire(start: from, end: to, interior: Array(route.dropFirst().dropLast())))
        // A line drawn on from a loose end (a deleted pin's spot) continues
        // that wire: the spot is no longer loose, and where only the two
        // wires meet they join into one.
        for end in [from, to] {
            if case let .junction(id) = end, let index = junctions.firstIndex(where: { $0.id == id }) {
                junctions[index].isLoose = false
            }
        }
        joinThroughJunctions(tips)
        return true
    }

    // MARK: Junctions and loose ends

    /// Drops junctions that no wire ends at any more, then joins wires
    /// through the plain junctions left with two ends.
    mutating func pruneJunctions(_ tips: PinTips) {
        let used = Set(wires.flatMap { [$0.start, $0.end] })
        junctions.removeAll { !used.contains(.junction($0.id)) }
        joinThroughJunctions(tips)
    }

    /// Joins the two wires at every plain junction where only two wire ends
    /// meet, as removing a branch leaves them: one wire runs through the
    /// spot instead, with bends only where the route turns. Loose
    /// junctions (plan-30) stay, since a pin can still land on them.
    mutating func joinThroughJunctions(_ tips: PinTips) {
        while joinOnce(tips) {}
    }

    /// One join of `joinThroughJunctions`; false when none is left.
    private mutating func joinOnce(_ tips: PinTips) -> Bool {
        // The wires on each junction, found in one pass over the wires.
        var wiresOn: [UUID: [Int]] = [:]
        for index in wires.indices {
            if case let .junction(id) = wires[index].start { wiresOn[id, default: []].append(index) }
            if case let .junction(id) = wires[index].end, wires[index].start != wires[index].end {
                wiresOn[id, default: []].append(index)
            }
        }
        for junction in junctions where !junction.isLoose {
            let here = WireEnd.junction(junction.id)
            guard let on = wiresOn[junction.id], on.count == 2 else { continue }
            // The first wire runs into the junction, the second out of it.
            var first = wires[on[0]], second = wires[on[1]]
            if first.start == here { first = Self.reversed(first) }
            if second.end == here { second = Self.reversed(second) }
            // A wire on the junction at both ends, or two wires between the
            // same two ends, would join into a loop.
            guard first.end == here, second.start == here, first.start != here, second.end != here,
                  first.start != second.end,
                  let from = endTip(first.start, tips), let to = endTip(second.end, tips) else { continue }
            // Bends on a straight run go, the junction's spot among them, so a
            // straight line comes back as one segment.
            let route = OrthogonalRoute.simplified([from] + first.interior + [junction.point] + second.interior + [to])
            wires[on[0]] = Wire(id: wires[on[0]].id, start: first.start, end: second.end,
                                interior: Array(route.dropFirst().dropLast()))
            wires.remove(at: on[1])
            junctions.removeAll { $0.id == junction.id }
            return true
        }
        return false
    }

    /// The same wire, its start and end swapped.
    static func reversed(_ wire: Wire) -> Wire {
        Wire(id: wire.id, start: wire.end, end: wire.start, interior: wire.interior.reversed())
    }

    /// Turns every wire end that `matches` into a loose junction at that
    /// end's tip, so the wire keeps its route when the pin goes away
    /// (plan-30). Ends with the same tip share one junction; a junction
    /// already at that point is reused and marked loose. Run it while the
    /// tips still resolve; an end whose tip doesn't is left alone.
    mutating func loosen(where matches: (WireEnd) -> Bool, _ tips: PinTips) {
        var byPoint: [CGPoint: UUID] = [:]
        func looseJunction(at point: CGPoint) -> UUID {
            if let id = byPoint[point] { return id }
            let id: UUID
            if let index = junctions.firstIndex(where: { $0.point == point }) {
                junctions[index].isLoose = true
                id = junctions[index].id
            } else {
                let created = Junction(point: point, isLoose: true)
                junctions.append(created)
                id = created.id
            }
            byPoint[point] = id
            return id
        }
        for index in wires.indices {
            if matches(wires[index].start), let tip = endTip(wires[index].start, tips) {
                wires[index].start = .junction(looseJunction(at: tip))
            }
            if matches(wires[index].end), let tip = endTip(wires[index].end, tips) {
                wires[index].end = .junction(looseJunction(at: tip))
            }
        }
    }

    /// Connects loose ends to pins after a drop (plan-30): a loose junction
    /// whose point is a tip in `dropped` (or in `every`, for a junction in
    /// `draggedJunctions`) moves every wire end on it to that pin, and goes.
    /// `accepts(before, after)` checks each candidate (the voltage check);
    /// one it refuses stays loose. Returns whether any end was adopted.
    mutating func adoptLooseEnds(dropped: [CGPoint: WireEnd], every: [CGPoint: WireEnd],
                                 draggedJunctions: Set<UUID>, accepts: ([Wire], [Wire]) -> Bool) -> Bool {
        var adopted = false
        for junction in junctions where junction.isLoose {
            let lookup = draggedJunctions.contains(junction.id) ? every : dropped
            guard let pin = lookup[junction.point] else { continue }
            let end = WireEnd.junction(junction.id)
            // A wire whose other end is already this pin would end on itself.
            guard !wires.contains(where: { ($0.start == end && $0.end == pin) || ($0.end == end && $0.start == pin) })
            else { continue }
            let candidate = wires.map { wire in
                var wire = wire
                if wire.start == end { wire.start = pin }
                if wire.end == end { wire.end = pin }
                return wire
            }
            guard accepts(wires, candidate) else { continue }
            wires = candidate
            junctions.removeAll { $0.id == junction.id }
            adopted = true
        }
        return adopted
    }

    // MARK: Re-fitting

    /// Re-fits every wire with an end that `touches` matches to its ends'
    /// tips (`OrthogonalRoute.settled`), storing the result.
    mutating func settle(where touches: (WireEnd) -> Bool, _ tips: PinTips) {
        for index in wires.indices where touches(wires[index].start) || touches(wires[index].end) {
            guard let start = endTip(wires[index].start, tips), let end = endTip(wires[index].end, tips) else { continue }
            wires[index].interior = OrthogonalRoute.settled(start: start, interior: wires[index].interior, end: end)
        }
    }

    /// Stores a new route for a wire: the points between its ends.
    mutating func setPath(_ id: UUID, _ path: [CGPoint]) {
        guard let index = wires.firstIndex(where: { $0.id == id }), path.count >= 2 else { return }
        wires[index].interior = Array(path.dropFirst().dropLast())
    }

    /// Adds a bend where `point` is on a wire.
    mutating func insertNode(at point: CGPoint, tolerance: CGFloat, _ tips: PinTips) {
        guard let hit = segmentHit(at: point, tolerance: tolerance, tips) else { return }
        setPath(hit.wire, OrthogonalRoute.insertNode(path(forWire: hit.wire, tips), segment: hit.index, at: point))
    }

    /// Every wire end through `remap` (bus pins moving to other wire ids).
    mutating func remapEnds(_ remap: (WireEnd) -> WireEnd) {
        for index in wires.indices {
            wires[index].start = remap(wires[index].start)
            wires[index].end = remap(wires[index].end)
        }
    }
}
