import CoreGraphics
import Foundation

/// What a run leaves on the board: the copper it laid, and the chains of
/// existing copper it moved.
nonisolated struct FinishedCopper: Sendable {
    var traces: [RoutedTrace]
    var vias: [RoutedVia]
    var moved: [MovedChain] = []
    /// Every chain's copper as it stands after the run, moved or not: what
    /// later checks (tuning) must treat as copper of other nets.
    var chainTraces: [RoutedTrace] = []
    var chainVias: [RoutedVia] = []
}

nonisolated extension Router.Run {
    /// The ledger's copper as a cleanup works on it.
    func cleanupInput() -> RoutingCleanup {
        var traces: [RoutedTrace] = [], vias: [RoutedVia] = []
        var traceEntry: [Int] = [], viaEntry: [Int] = []
        for entry in state.ledger {
            traces += entry.traces
            traceEntry += Array(repeating: entry.id, count: entry.traces.count)
            vias += entry.vias
            viaEntry += Array(repeating: entry.id, count: entry.vias.count)
        }
        var cleanup = RoutingCleanup(problem: problem, traces: traces, vias: vias, traceEntry: traceEntry, viaEntry: viaEntry)
        // A coupled pair's copper stays as it was routed: cleaning one trace
        // of it would break the coupling.
        cleanup.frozen = Set(traces.indices.filter { traces[$0].net.map(pairNets.contains) ?? false })
        cleanup.frozenVias = Set(vias.indices.filter { vias[$0].net.map(pairNets.contains) ?? false })
        // Existing copper no push has touched stays exactly as it is.
        for t in traces.indices where state.entries[traceEntry[t]].map({ $0.chain != nil && !$0.touched }) ?? false {
            cleanup.frozen.insert(t)
        }
        for v in vias.indices where state.entries[viaEntry[v]].map({ $0.chain != nil && !$0.touched }) ?? false {
            cleanup.frozenVias.insert(v)
        }
        return cleanup
    }

    /// The routed copper, cleaned and checked (plan-38-H § 3 steps 7–8): the
    /// cleanup's copper is verified; a connection with a violation goes back
    /// to its raw copper; one whose raw copper breaks a rule too is ripped
    /// up and routed again with a margin of half a cell more each time (at
    /// most 3 times), else left unrouted with the reason.
    mutating func finishedCopper() -> FinishedCopper {
        var attempt = 0
        var done: FinishedCopper?
        while done == nil {
            var cleanup = cleanupInput()
            if cleaning { cleanup.run() }
            var copper = cleanup.result
            var raw = Set<Int>()
            var illegal: [Int] = []
            // A touched chain cleanup changed the shape of (a via taken out, a run split) goes back to what
            // the push left: the document's chain must keep its runs and vias.
            let restructured = Set(state.ledger.filter { $0.chain != nil && $0.touched }.map(\.id).filter { id in
                !Self.sameShape(copper, entry: id, ledger: state.entries)
            })
            if !restructured.isEmpty {
                raw.formUnion(restructured)
                copper = Self.reverting(copper, entries: restructured, ledger: state.entries)
            }
            while true {
                let violations = RoutingVerifier.verify(problem, traces: copper.traces, vias: copper.vias)
                guard !violations.isEmpty else { break }
                // A violation is the new or touched copper's: existing copper no push has touched
                // may already break a rule, and the router does not repair the user's board.
                let bad = Set(violations.flatMap { v in v.traces.map { copper.traceEntry[$0] } + v.vias.map { copper.viaEntry[$0] } }
                    .filter { state.entries[$0].map { $0.chain == nil || $0.touched } ?? true })
                let fresh = bad.subtracting(raw)
                if fresh.isEmpty {
                    illegal = bad.sorted()
                    break
                }
                raw.formUnion(fresh)
                copper = Self.reverting(copper, entries: fresh, ledger: state.entries)
            }
            guard !illegal.isEmpty else {
                done = finished(copper)
                continue
            }
            // A touched chain that is illegal even as the push left it goes back to where it was; the
            // violation is then the new copper's, which the repair below handles.
            let back = illegal.filter { state.entries[$0].map { $0.chain != nil && $0.touched } ?? false }
            if !back.isEmpty {
                for id in back { restoreChain(id) }
                illegal.removeAll { back.contains($0) }
                if illegal.isEmpty { continue }
            }
            attempt += 1
            let nets = Set(illegal.compactMap { state.entries[$0]?.net })
            if attempt > Self.repairAttempts || stopped {
                state.rip(illegal)
                for net in nets { illegalNets.insert(net) }
                continue
            }
            reroute(illegal, margin: 0.5 * grid.pitch * Double(attempt))
        }
        return done!
    }

    static let repairAttempts = 3

    /// Puts a chain's copper back as the problem had it, untouched.
    mutating func restoreChain(_ id: Int) {
        guard let entry = state.entries[id], let chain = entry.chain, problem.movable.indices.contains(chain) else { return }
        let original = problem.movable[chain]
        let traces = original.runs.map { RoutedTrace(layer: $0.layer, width: original.width, points: $0.points, net: original.net) }
        let vias = original.vias.map {
            RoutedVia(centre: $0.centre, drill: original.viaDrill, diameter: original.viaDiameter, net: original.net)
        }
        state.replace(id, traces: traces, vias: vias, touched: false)
    }

    /// Whether the copper of chain entry `id` in `copper` still has the entry's runs (same ends, either
    /// way round) and as many vias.
    static func sameShape(_ copper: (traces: [RoutedTrace], vias: [RoutedVia], traceEntry: [Int], viaEntry: [Int]),
                          entry id: Int, ledger: [Int: LedgerEntry]) -> Bool {
        guard let entry = ledger[id] else { return true }
        let traces = copper.traces.indices.filter { copper.traceEntry[$0] == id }.map { copper.traces[$0] }
        let vias = copper.vias.indices.filter { copper.viaEntry[$0] == id }
        guard traces.count == entry.traces.count, vias.count == entry.vias.count else { return false }
        return zip(traces, entry.traces).allSatisfy { cleaned, raw in
            guard let a = cleaned.points.first, let b = cleaned.points.last, let c = raw.points.first, let d = raw.points.last
            else { return false }
            return (TraceRoute.same(a, c) && TraceRoute.same(b, d)) || (TraceRoute.same(a, d) && TraceRoute.same(b, c))
        }
    }

    /// The copper `copper` holds, split into what the run laid and the chains of existing copper (moved or not).
    private func finished(_ copper: (traces: [RoutedTrace], vias: [RoutedVia], traceEntry: [Int], viaEntry: [Int])) -> FinishedCopper {
        func chain(_ entry: Int) -> Int? { state.entries[entry]?.chain }
        var result = FinishedCopper(traces: [], vias: [])
        for (t, trace) in copper.traces.enumerated() {
            if chain(copper.traceEntry[t]) == nil { result.traces.append(trace) } else { result.chainTraces.append(trace) }
        }
        for (v, via) in copper.vias.enumerated() {
            if chain(copper.viaEntry[v]) == nil { result.vias.append(via) } else { result.chainVias.append(via) }
        }
        for entry in state.ledger where entry.chain != nil && entry.touched {
            guard let id = entry.chain, problem.movable.indices.contains(id) else { continue }
            let original = problem.movable[id]
            let traces = copper.traces.indices.filter { copper.traceEntry[$0] == entry.id }.map { copper.traces[$0] }
            let vias = copper.vias.indices.filter { copper.viaEntry[$0] == entry.id }.map { copper.vias[$0] }
            guard traces.count == original.runs.count, vias.count == original.vias.count else { continue }
            // Each run in the chain's own direction.
            var runs: [[CGPoint]] = []
            for (k, trace) in traces.enumerated() {
                let wanted = original.runs[k].points
                let flipped = trace.points.first.map { first in wanted.last.map { TraceRoute.same(first, $0) } ?? false } ?? false
                runs.append(flipped ? trace.points.reversed() : trace.points)
            }
            let centres = vias.map(\.centre)
            let changed = zip(runs, original.runs).contains { $0 != $1.points } || zip(centres, original.vias).contains { $0 != $1.centre }
            if changed { result.moved.append(MovedChain(chain: id, runs: runs, vias: centres)) }
        }
        result.moved.sort { $0.chain < $1.chain }
        return result
    }

    /// `copper` with the pieces of `entries` swapped for their raw copper.
    static func reverting(_ copper: (traces: [RoutedTrace], vias: [RoutedVia], traceEntry: [Int], viaEntry: [Int]),
                          entries: Set<Int>, ledger: [Int: LedgerEntry])
        -> (traces: [RoutedTrace], vias: [RoutedVia], traceEntry: [Int], viaEntry: [Int]) {
        var result = copper
        let keepT = result.traces.indices.filter { !entries.contains(result.traceEntry[$0]) }
        let keepV = result.vias.indices.filter { !entries.contains(result.viaEntry[$0]) }
        result.traces = keepT.map { copper.traces[$0] }
        result.traceEntry = keepT.map { copper.traceEntry[$0] }
        result.vias = keepV.map { copper.vias[$0] }
        result.viaEntry = keepV.map { copper.viaEntry[$0] }
        for id in entries.sorted() {
            guard let entry = ledger[id] else { continue }
            result.traces += entry.traces
            result.traceEntry += Array(repeating: id, count: entry.traces.count)
            result.vias += entry.vias
            result.viaEntry += Array(repeating: id, count: entry.vias.count)
        }
        return result
    }

    /// Rips `entries` up and routes their nets' open connections again on
    /// a grid where those nets' copper is `margin` wider each side; the new
    /// copper is stored at its true width, so it keeps `margin` more than
    /// the rules need.
    mutating func reroute(_ entries: [Int], margin: Double) {
        let nets = Set(entries.compactMap { state.entries[$0]?.net })
        let groupsBefore = Dictionary(uniqueKeysWithValues: nets.map { ($0, state.groups(of: $0).components.count) })
        defer {
            // A net the margin route couldn't join again as it was: its
            // copper could not be made legal.
            for net in nets where state.groups(of: net).components.count > groupsBefore[net] ?? 0 {
                illegalNets.insert(net)
            }
        }
        rips += entries.count
        state.rip(entries)
        var wide = problem
        // The chains are already in the ledger copied below: seeding them again would double them.
        wide.movable = []
        wide.options.pushAndShove = false
        for i in wide.nets.indices where nets.contains(wide.nets[i].id) {
            wide.nets[i].width += 2 * margin
            wide.nets[i].viaDiameter += 2 * margin
        }
        var temp = Router.Run(problem: wide, strategy: strategy, seed: seed)
        for entry in state.ledger {
            let allowed = grid.layers.map { _ in true }
            temp.state.add(net: entry.net, path: entry.path, traces: entry.traces, vias: entry.vias, allowed: allowed,
                           chain: entry.chain, touched: entry.touched)
        }
        let before = Set(temp.state.entries.keys)
        for net in nets.sorted() {
            guard let routing = temp.state.netsByID[net] else { continue }
            _ = temp.join(routing, present: 0, round: rounds, progress: { _ in }, isCancelled: { false })
        }
        for entry in temp.state.ledger where !before.contains(entry.id) {
            guard let real = state.netsByID[entry.net] else { continue }
            let traces = entry.traces.map { trace -> RoutedTrace in
                var trace = trace
                trace.width = real.width
                return trace
            }
            let vias = entry.vias.map { via -> RoutedVia in
                var via = via
                via.diameter = real.viaDiameter
                via.drill = real.viaDrill
                return via
            }
            let allowed = grid.layers.map { real.allowed.contains($0) }
            state.add(net: entry.net, path: entry.path, traces: traces, vias: vias, allowed: allowed)
        }
    }
}
