import CoreGraphics
import Foundation

/// Improving what is routed. The first routing takes the first legal way
/// for each connection in turn, so early connections take room later ones
/// needed and later ones hop layers to get by. Once everything that can be
/// routed is, every connection is taken up again, one at a time, and routed
/// afresh with vias and bends priced high: on its own, with its whole net,
/// or through other nets' copper, which is then routed again too. A move
/// is kept only when it leaves no more connections unrouted and the copper
/// it touched scores lower (wire + vias + bends); otherwise the state goes
/// back exactly as it was. Passes repeat until one keeps nothing.
extension Router.Run {
    /// A net the improvement may take up: not fanned out to a Plane, not a
    /// coupled pair's (a pair is one piece, and stays as routed).
    func polishable(_ net: Int) -> Bool {
        guard let routing = state.netsByID[net], routing.impossible == nil else { return false }
        return !pinned.contains(net) && !pairNets.contains(net) && !fansOut(routing)
    }

    /// The score of `nets`' copper in `state`: connections left unrouted,
    /// and wire in cells plus the improvement costs of every via and bend.
    func polishScore(_ nets: Set<Int>, in state: RoutingState) -> (unrouted: Int, cost: Double) {
        var unrouted = 0
        for net in nets.sorted() { unrouted += max(0, state.groups(of: net).components.count - 1) }
        var cost = 0.0
        for entry in nets.sorted().flatMap({ state.ledger(of: $0) }) {
            cost += Double(entry.vias.count) * strategy.polishVia
            for trace in entry.traces {
                let p = trace.points
                for i in 1..<max(p.count, 1) {
                    let dx = p[i].x - p[i - 1].x, dy = p[i].y - p[i - 1].y
                    cost += hypot(dx, dy) / grid.pitch
                    guard i + 1 < p.count else { continue }
                    let ex = p[i + 1].x - p[i].x, ey = p[i + 1].y - p[i].y
                    if abs(dx * ey - dy * ex) > 1e-9 { cost += strategy.polishBend }
                }
            }
        }
        return (unrouted, cost)
    }

    /// One move: rips `ids` up, joins their nets again (through other nets'
    /// copper too, with `shove`), joins whatever nets that took copper from,
    /// and keeps the outcome only if it is better. Returns the box of the
    /// copper it changed when it was kept, else nil.
    mutating func attempt(rip ids: [Int], shove: Bool, round: Int, progress: (RoutingProgress) -> Void,
                          isCancelled: () -> Bool) -> CGRect? {
        let nets = Set(ids.compactMap { state.entries[$0]?.net })
        guard !nets.isEmpty, nets.allSatisfy(polishable) else { return nil }
        let saved = state
        polishTried += 1
        state.rip(ids)
        var touched = nets
        var lost: [Int] = []
        for net in nets.sorted() {
            guard let routing = state.netsByID[net] else { continue }
            lost += join(routing, present: shove ? strategy.polishPresent : 0, round: round, shove: shove,
                         progress: { _ in }, isCancelled: isCancelled)
        }
        // The nets that gave way: joined again, legally only.
        for net in Set(lost).sorted() where !nets.contains(net) {
            touched.insert(net)
            guard let routing = state.netsByID[net] else { continue }
            _ = join(routing, present: 0, round: round, progress: { _ in }, isCancelled: isCancelled)
        }
        let before = polishScore(touched, in: saved), after = polishScore(touched, in: state)
        let better = after.unrouted != before.unrouted ? after.unrouted < before.unrouted : after.cost < before.cost - 1e-6
        guard better, !stopped else {
            state = saved
            return nil
        }
        polishKept += 1
        report(progress, round: round, net: "Improving")
        var box = CGRect.null
        for net in touched.sorted() {
            for entry in saved.ledger(of: net) + state.ledger(of: net) { box = box.union(entry.box) }
        }
        return box
    }

    /// How far from a connection a change may be and still open a better way for it, mm.
    static let polishReach = 4.0

    /// The improvement passes (see above). Does nothing for a strategy
    /// without them. A stop leaves the last kept state, which is legal.
    ///
    /// A connection is tried through other nets' copper only when it has a
    /// via: without one the legal search has already found it the best way.
    /// A connection (or net) whose move was not kept is not tried again
    /// until a kept move has changed copper within `polishReach` of it:
    /// nothing nearer has changed, so the same search would give the same
    /// answer.
    mutating func polish(progress: (RoutingProgress) -> Void, isCancelled: () -> Bool) {
        guard strategy.polishPasses > 0, !stopped else { return }
        polishing = true
        defer { polishing = false }
        // Kept moves' boxes, in order; and for each entry (by id) or net (by -1 - id) how many there were when it last failed.
        var changes: [CGRect] = []
        var failedAt: [Int: Int] = [:]
        func stale(_ key: Int, _ box: CGRect) -> Bool {
            guard let since = failedAt[key] else { return false }
            let near = box.insetBy(dx: -Self.polishReach, dy: -Self.polishReach)
            return !changes[since...].contains { $0.intersects(near) }
        }
        for pass in 1...strategy.polishPasses {
            var kept = 0
            // Each connection alone: the legal way first, then through others.
            for id in state.ledger.map(\.id) {
                if stopped || isCancelled() { stopped = true; return }
                guard let entry = state.entries[id], !stale(id, entry.box) else { continue }
                if let box = attempt(rip: [id], shove: false, round: pass, progress: progress, isCancelled: isCancelled) {
                    changes.append(box)
                    kept += 1
                } else if strategy.polishPresent > 0, entry.vias.count > 0,
                          let box = attempt(rip: [id], shove: true, round: pass, progress: progress, isCancelled: isCancelled) {
                    changes.append(box)
                    kept += 1
                } else {
                    failedAt[id] = changes.count
                }
            }
            // Each net whole: its connections joined afresh, in a new tree.
            for net in routable.sorted(by: { $0.id < $1.id }) where polishable(net.id) {
                if stopped || isCancelled() { stopped = true; return }
                let entries = state.ledger(of: net.id)
                guard entries.count > 1 || unrouted(net) > 0 else { continue }
                let pads = net.components.flatMap { $0.pads.map(\.bounds) }
                let box = (entries.map(\.box) + pads).reduce(CGRect.null) { $0.union($1) }
                guard !stale(-1 - net.id, box) else { continue }
                if entries.isEmpty {
                    // Nothing routed of it yet: there may be room now.
                    let saved = state
                    let lost = join(net, present: 0, round: pass, progress: { _ in }, isCancelled: isCancelled)
                    if lost.isEmpty, !stopped, state.groups(of: net.id).components.count < saved.groups(of: net.id).components.count {
                        changes.append(state.ledger(of: net.id).reduce(CGRect.null) { $0.union($1.box) })
                        kept += 1
                    } else {
                        state = saved
                        failedAt[-1 - net.id] = changes.count
                    }
                } else if let changed = attempt(rip: entries.map(\.id), shove: false, round: pass, progress: progress,
                                                isCancelled: isCancelled) {
                    changes.append(changed)
                    kept += 1
                } else {
                    failedAt[-1 - net.id] = changes.count
                }
            }
            if stopped || kept == 0 { break }
        }
    }
}
