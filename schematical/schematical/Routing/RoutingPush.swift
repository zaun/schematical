import CoreGraphics
import Foundation

/// Push and shove (plan-66-D § 3). A connection with no legal way, but a way
/// through other nets' copper, takes that copper up and lays itself; each piece
/// of the copper is then laid again at once, inside a corridor around where it
/// was, keeping its ends, its layers and no more vias than it had. The whole
/// is one transaction: it all happens, or the state is as it was.
nonisolated extension Router.Run {
    /// A piece of copper a push has taken up and must lay again.
    struct PushVictim {
        /// What it was.
        var entry: LedgerEntry
        var routing: RoutingNet
        /// Where it may start and where it must end, each with its tie.
        var starts: [RoutingTerminal]
        var ends: [RoutingTerminal]
    }

    /// What `pushRoute` lays: the connection that asked, or a victim.
    enum PushPiece {
        case connection(main: [Int], target: [Int])
        case victim(PushVictim)
    }

    /// Lays a connection of `net` from `sources` to `targets` by moving the copper in
    /// its way. True: it is laid, and every piece it displaced is laid again within
    /// `pushReach` of where it was. False: nothing changed (the state is put back
    /// exactly), because no way needs only pushes, a piece has no room, or the search stopped.
    mutating func tryPush(_ net: RoutingNet, sources: [Int], targets: [Int], mainComponents: [Int],
                          targetComponents: [Int], plan: RoutingPlanCosts? = nil, isCancelled: () -> Bool) -> Bool {
        let saved = state
        pushTried += 1
        var added = Set<Int>()
        let ok = pushRoute(net, sources: sources, targets: targets,
                           piece: .connection(main: mainComponents, target: targetComponents), corridor: nil, plan: plan,
                           depth: 0, added: &added, isCancelled: isCancelled)
        corridor.clear()
        avoid.clear()
        guard ok, !stopped else {
            state = saved
            return false
        }
        pushKept += 1
        return true
    }

    private mutating func pushRoute(_ net: RoutingNet, sources: [Int], targets: [Int], piece: PushPiece, corridor field: [Float]?,
                                    plan: RoutingPlanCosts?, depth: Int, added: inout Set<Int>,
                                    isCancelled: () -> Bool) -> Bool {
        var hard = rules(net)
        hard.present = 0
        hard.maxExpansions = strategy.pushMaxExpansions
        // A legal way in the room there is.
        if let found = find(hard, sources: sources, targets: targets, plan: plan, corridor: field, isCancelled: isCancelled),
           let id = lay(found, net: net, piece: piece, allowed: hard.allowed) {
            added.insert(id)
            return true
        }
        guard !stopped, depth < strategy.pushDepth else { return false }
        // Else a way through copper that can move, and the copper it crosses.
        var relaxed = hard
        relaxed.present = strategy.present
        relaxed.pushPrice = 2 * strategy.present
        guard let detour = find(relaxed, sources: sources, targets: targets, plan: plan, corridor: field, isCancelled: isCancelled)
        else { return false }
        let ids = state.victims(of: detour, net: net.id)
        guard !ids.isEmpty, ids.count <= strategy.pushMaxVictims else { return false }
        var victims: [PushVictim] = []
        for id in ids {
            guard !added.contains(id), let victim = pushVictim(id) else { return false }
            victims.append(victim)
        }
        state.remove(ids)
        // The connection first, as far from where the copper was as the room allows, so it has room beside.
        var under = field
        if field == nil {
            avoid.makeAvoiding(victims.map { (traces: $0.entry.traces, vias: $0.entry.vias) }, grid: grid,
                               radius: strategy.pushReach, penalty: strategy.pushDeviation)
            under = avoid.values
        }
        guard let second = find(hard, sources: sources, targets: targets, plan: plan, corridor: under, isCancelled: isCancelled),
              let laid = lay(second, net: net, piece: piece, allowed: hard.allowed) else { return false }
        added.insert(laid)
        avoid.clear()
        // Then each piece, laid again beside it.
        for victim in victims.sorted(by: { $0.entry.id < $1.entry.id }) {
            corridor.make(for: (victim.entry.traces, victim.entry.vias), grid: grid, reach: strategy.pushReach,
                          deviation: strategy.pushDeviation)
            // Not through what this push has laid.
            let own = victim.routing.width / 2
            corridor.forbid(added.compactMap { id in
                state.entries[id].map { laid in
                    let gap = max(state.netsByID[laid.net]?.clearance ?? 0, victim.routing.clearance, problem.rules.minClearance)
                    return (laid.traces, laid.vias, gap)
                }
            }, grid: grid, reach: own)
            let relaid = pushRoute(victim.routing, sources: victim.starts.map(\.node), targets: victim.ends.map(\.node),
                                   piece: .victim(victim), corridor: corridor.values, plan: nil, depth: depth + 1,
                                   added: &added, isCancelled: isCancelled)
            corridor.clear()
            guard relaid else { return false }
        }
        return true
    }

    /// Puts `found` on the ledger as `piece`; nil when it may not be (a victim's
    /// accept rule). Returns the new entry's id.
    private mutating func lay(_ found: [Int], net: RoutingNet, piece: PushPiece, allowed: [Bool]) -> Int? {
        switch piece {
        case let .connection(main, target):
            return addConnection(found, net: net, mainComponents: main, targetComponents: target, allowed: allowed)
        case let .victim(victim):
            let startTie = victim.starts.first { $0.node == found[0] }?.tie
            let endTie = victim.ends.first { $0.node == found[found.count - 1] }?.tie
            let copper = Router.copper(found, startTie: startTie, endTie: endTie, net: net, grid: grid)
            // The same layers in the same order, and no more vias than it had.
            guard copper.traces.map(\.layer) == victim.entry.traces.map(\.layer),
                  copper.vias.count <= victim.entry.vias.count else { return nil }
            let chain = victim.entry.chain
            return state.add(net: net.id, path: found, traces: copper.traces, vias: copper.vias, allowed: allowed,
                             chain: chain, touched: chain != nil)
        }
    }

    /// The entry `id` as a piece a push may move, or nil. A chain of existing copper may always be
    /// (when its ends have terminals); a connection the run laid only when it joins two terminals of
    /// its net and no other connection of the net is attached to it, and the net is one the improvement
    /// may take up (not a coupled pair, not fanned out to a Plane).
    private func pushVictim(_ id: Int) -> PushVictim? {
        guard let entry = state.entries[id], let routing = state.netsByID[entry.net] else { return nil }
        if let chain = entry.chain {
            guard let terminals = state.chainTerminals[chain], !terminals.a.isEmpty, !terminals.b.isEmpty else { return nil }
            return PushVictim(entry: entry, routing: routing, starts: terminals.a, ends: terminals.b)
        }
        guard polishable(entry.net), let first = entry.path.first, let last = entry.path.last, first != last,
              let onNode = state.componentOfNode[entry.net], onNode[first] != nil, onNode[last] != nil else { return nil }
        let others = state.ledger(of: entry.net).filter { $0.id != id }
        let attached = others.contains { other in
            [other.path.first, other.path.last].compactMap { $0 }.contains { entry.nodes.contains($0) }
                || [first, last].contains { other.nodes.contains($0) && onNode[$0] == nil }
        }
        guard !attached else { return nil }
        let components = Array(0..<(state.terminals[entry.net]?.count ?? 0))
        return PushVictim(entry: entry, routing: routing,
                          starts: [RoutingTerminal(node: first, tie: state.tie(first, net: entry.net, components: components))],
                          ends: [RoutingTerminal(node: last, tie: state.tie(last, net: entry.net, components: components))])
    }
}
