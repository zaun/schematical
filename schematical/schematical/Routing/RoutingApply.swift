import CoreGraphics
import Foundation

/// What applying a routing result did.
nonisolated struct RoutingSummary: Hashable, Sendable {
    var addedTraces = 0
    var addedVias = 0
    var removedTraces = 0
    var removedVias = 0
    /// Each net left with unrouted connections: its name and how many.
    var unrouted: [UnroutedNet] = []
    /// Traces the board refused (a plane or absent layer), and chains of
    /// existing copper not moved because the board changed since the problem
    /// was made: only a stale problem can have them.
    var skipped = 0
    /// Existing traces and vias push and shove moved (plan-66).
    var movedTraces = 0
    var movedVias = 0
    var stats = RoutingStats()

    struct UnroutedNet: Hashable, Sendable {
        var name: String
        var count: Int
        var connections: [String]
    }

    var unroutedConnections: Int { unrouted.reduce(0) { $0 + $1.count } }
}

extension SchematicDocument {
    /// Puts a routing result on the board as one undo step, "Autoroute":
    /// deletes the copper the problem cleared, adds the result's vias and
    /// traces marked `auto` (each trace joined to whatever it touches, as
    /// the Trace tool's are), and selects what was added. A result that adds
    /// and removes nothing records nothing.
    @discardableResult
    func applyRouting(_ result: RoutingResult, problem: RoutingProblem) -> RoutingSummary {
        var summary = RoutingSummary()
        summary.stats = result.stats
        summary.unrouted = result.nets.filter { $0.unroutedConnections > 0 }.map {
            RoutingSummary.UnroutedNet(name: $0.name, count: $0.unroutedConnections, connections: $0.unrouted)
        }
        guard !isPlaying else { return summary }
        recordEdit(.pcb, "Autoroute") {
            // Locked copper stays, even if a stale problem names it.
            let traceIDs = Set(traces.filter { !$0.locked && problem.removedTraces.contains($0.id) }.map(\.id))
            let viaIDs = Set(vias.filter { !$0.locked && problem.removedVias.contains($0.id) }.map(\.id))
            removeCopper(traces: traceIDs, vias: viaIDs)
            summary.removedTraces = traceIDs.count
            summary.removedVias = viaIDs.count
            moveChains(result.moved, problem: problem, summary: &summary)
            var added: Set<BoardItem> = []
            for routed in result.vias {
                let via = BoardVia(id: UUID(), centre: routed.centre, drill: routed.drill, diameter: routed.diameter,
                                   auto: true)
                if appendVia(via) {
                    added.insert(.via(via.id))
                    summary.addedVias += 1
                }
            }
            for routed in result.traces {
                let mil = min(max(Int((routed.width / 0.0254).rounded()), BoardTrace.widthRange.lowerBound),
                              BoardTrace.widthRange.upperBound)
                let trace = BoardTrace(id: UUID(), layer: routed.layer, width: mil, points: routed.points, auto: true)
                if appendTrace(trace) {
                    join(trace.id)
                    added.insert(.trace(trace.id))
                    summary.addedTraces += 1
                } else {
                    summary.skipped += 1
                }
            }
            if !added.isEmpty { selectBoardItems(added) }
        }
        return summary
    }

    /// Moves the chains of existing copper `moved` names (plan-66-C): each
    /// run's vertices `first...last` of its trace are replaced by the new
    /// points, each via's centre set. A chain is skipped, whole, and counted
    /// in `summary.skipped`, when its copper is not as the problem recorded it
    /// (a trace gone, locked or edited), or when an end that is not a trace's
    /// own end would move (a vertex another chain of the trace shares).
    /// Nothing is joined or selected: a chain keeps its anchors, so its contacts.
    private func moveChains(_ moved: [MovedChain], problem: RoutingProblem, summary: inout RoutingSummary) {
        struct Splice {
            var trace: UUID
            var first: Int
            var last: Int
            var points: [CGPoint]
        }
        var splices: [Splice] = []
        var centres: [(via: UUID, centre: CGPoint)] = []
        for move in moved.sorted(by: { $0.chain < $1.chain }) {
            guard problem.movable.indices.contains(move.chain) else { summary.skipped += 1; continue }
            let chain = problem.movable[move.chain]
            var mine: [Splice] = [], mineVias: [(via: UUID, centre: CGPoint)] = []
            var valid = move.runs.count == chain.runs.count && move.vias.count == chain.vias.count
            for (k, run) in chain.runs.enumerated() where valid {
                guard let trace = traces.first(where: { $0.id == run.trace }), !trace.locked,
                      trace.points.indices.contains(run.last), run.first < run.last else { valid = false; break }
                let recorded = Array(trace.points[run.first...run.last])
                guard recorded == (run.reversed ? run.points.reversed() : run.points) else { valid = false; break }
                var points = move.runs[k]
                if run.reversed { points.reverse() }
                guard points.count >= 2 else { valid = false; break }
                // An end inside the trace is shared with the next chain along it: it stays where it is.
                if run.first > 0, points.first != recorded.first { valid = false }
                if run.last < trace.points.count - 1, points.last != recorded.last { valid = false }
                mine.append(Splice(trace: run.trace, first: run.first, last: run.last, points: points))
            }
            for (k, via) in chain.vias.enumerated() where valid {
                guard let real = vias.first(where: { $0.id == via.id }), !real.locked, real.centre == via.centre else {
                    valid = false
                    break
                }
                mineVias.append((via.id, move.vias[k]))
            }
            if valid { splices += mine; centres += mineVias } else { summary.skipped += 1 }
        }
        // Along each trace from its far end back, so the indices of the rest still hold.
        for id in Set(splices.map(\.trace)).sorted(by: { $0.uuidString < $1.uuidString }) {
            guard var points = traces.first(where: { $0.id == id })?.points else { continue }
            for splice in splices.filter({ $0.trace == id }).sorted(by: { $0.first > $1.first }) {
                points.replaceSubrange(splice.first...splice.last, with: splice.points)
            }
            if setTracePoints(id, TraceRoute.clean(points)) {
                summary.movedTraces += splices.filter { $0.trace == id }.count
            } else {
                summary.skipped += 1
            }
        }
        for (via, centre) in centres {
            setViaCentre(via, centre)
            summary.movedVias += 1
        }
    }

    /// Deletes the unlocked traces and vias of the nets `nets` (as
    /// `BoardConnectivity` numbers them), only the autorouted ones when
    /// `onlyAuto`, as one undo step, "Unroute". Returns how many pieces went;
    /// 0 while the simulation runs.
    @discardableResult
    func unroute(nets: Set<Int>, onlyAuto: Bool) -> Int {
        unroute(onlyAuto: onlyAuto) { net in net.map(nets.contains) ?? false }
    }

    /// Deletes the unlocked traces and vias of every net (and, unless
    /// `onlyAuto`, the copper on no net), as one undo step.
    @discardableResult
    func unrouteAll(onlyAuto: Bool) -> Int {
        unroute(onlyAuto: onlyAuto) { net in net != nil || !onlyAuto }
    }

    private func unroute(onlyAuto: Bool, picks: (Int?) -> Bool) -> Int {
        guard !isPlaying else { return 0 }
        let copper = connectivity()
        let netOfRoot = copper.netOfRoot()
        var traceIDs = Set<UUID>(), viaIDs = Set<UUID>()
        for (t, trace) in copper.traces.enumerated() where !trace.locked && (!onlyAuto || trace.auto)
            && picks(netOfRoot[copper.root[copper.traceNode(t)]]) {
            traceIDs.insert(trace.id)
        }
        for (v, via) in copper.vias.enumerated() where !via.locked && (!onlyAuto || via.auto)
            && picks(netOfRoot[copper.root[copper.viaNode(v)]]) {
            viaIDs.insert(via.id)
        }
        guard !traceIDs.isEmpty || !viaIDs.isEmpty else { return 0 }
        recordEdit(.pcb, "Unroute") { removeCopper(traces: traceIDs, vias: viaIDs) }
        return traceIDs.count + viaIDs.count
    }
}

/// The routing scope a selection names.
enum RoutingScope {
    /// Footprints selected: their nets (and those of any selected trace or
    /// via). Only traces or vias: their nets. Nothing: every net.
    @MainActor
    static func fromSelection(_ document: SchematicDocument) -> RoutingOptions.Scope {
        let items = document.boardSelection.items
        guard !items.isEmpty else { return .all }
        let copper = document.connectivity()
        var footprints: [UUID] = []
        var keys = Set<PlaneNet>()
        for item in items.sorted(by: { "\($0)" < "\($1)" }) {
            switch item {
            case let .footprint(id): footprints.append(id)
            case let .trace(id):
                if let net = document.net(ofTrace: id, in: copper), let key = copper.planeNet(ofNet: net) { keys.insert(key) }
            case let .via(id):
                if let net = document.net(ofVia: id, in: copper), let key = copper.planeNet(ofNet: net) { keys.insert(key) }
            default: break
            }
        }
        if !footprints.isEmpty {
            guard !keys.isEmpty else { return .footprints(footprints) }
            // The footprints' nets and the copper's, as nets.
            let indices = Set(document.chips.indices.filter { footprints.contains(document.chips[$0].id) })
            for p in copper.pads.indices where indices.contains(copper.pads[p].chip) {
                if let net = copper.netOfPad[p], let key = copper.planeNet(ofNet: net) { keys.insert(key) }
            }
        }
        if keys.isEmpty { return footprints.isEmpty ? .all : .footprints(footprints) }
        return .nets(keys.sorted { $0.fileValue < $1.fileValue })
    }
}
