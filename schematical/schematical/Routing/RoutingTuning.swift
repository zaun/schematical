import CoreGraphics
import Foundation

/// How a length-match group came out of a run.
nonisolated struct GroupOutcome: Hashable, Sendable {
    enum Status: Hashable, Sendable {
        /// Every eligible member within the tolerance of the longest.
        case matched
        /// Some members still too short: the spread left, and why.
        case residual(spread: Double, reason: String?)
        /// A member that can't be matched, and why (the first one found).
        case ineligible(net: Int, reason: String)
        /// A differential pair routed side by side; how much of it runs apart, mm.
        case coupled(uncoupled: Double)
        /// A differential pair routed as two ordinary nets, and why.
        case uncoupled(reason: String)
    }

    let group: UUID
    var name: String
    /// The longest eligible member's length, mm: what the others are tuned to.
    var target: Double
    /// Each measured member's length, mm, after tuning.
    var lengths: [Int: Double]
    /// Longest less shortest, mm.
    var spread: Double
    var status: Status
}

/// Length matching (plan-38-I § 3): each too-short member of a group gets
/// accordion bumps on its own new straight segments until its length is
/// within the tolerance of the longest member's, which is never touched.
/// Every bump is checked exactly against every other net's copper first.
nonisolated enum RoutingTuning {
    static let noRoutedCopper = "no routed copper to tune"
    static let multiDrop = "more than two pins"
    static let notRouted = "not fully routed"

    /// A net's length in `traces` plus its kept copper in `problem`.
    static func length(of net: Int, traces: [RoutedTrace], problem: RoutingProblem) -> Double {
        var segments: [(CGPoint, CGPoint)] = []
        for trace in traces where trace.net == net { segments += zip(trace.points, trace.points.dropFirst()) }
        for obstacle in problem.obstacles where obstacle.kind == .trace && obstacle.net == net {
            if case let .segment(a, b) = obstacle.shape.core { segments.append((a, b)) }
        }
        return RoutingLength.of(segments: segments)
    }

    /// Tunes every length-match group of `problem` in `traces`; returns each
    /// group's outcome.
    static func tune(_ problem: RoutingProblem, traces: inout [RoutedTrace], vias: [RoutedVia],
                     outcomes: [NetOutcome], pairs: [UUID: GroupOutcome.Status] = [:], tuneLengths: Bool = true,
                     tuneSkew: Bool = true) -> [GroupOutcome] {
        var cleanup = RoutingCleanup(problem: problem, traces: traces, vias: vias,
                                     traceEntry: Array(traces.indices), viaEntry: Array(vias.indices))
        let nets = Dictionary(uniqueKeysWithValues: problem.nets.map { ($0.id, $0) })
        let outcomeOf = Dictionary(uniqueKeysWithValues: outcomes.map { ($0.net, $0) })
        var results: [GroupOutcome] = []
        for group in problem.groups.sorted(by: { $0.id.uuidString < $1.id.uuidString }) where group.mode == .lengthMatch {
            var ineligible: (Int, String)?
            var members: [Int] = []
            for net in group.nets {
                if let routing = nets[net], routing.pins > 2 {
                    if ineligible == nil { ineligible = (net, multiDrop) }
                    continue
                }
                if let outcome = outcomeOf[net], outcome.unroutedConnections > 0 {
                    if ineligible == nil { ineligible = (net, notRouted) }
                    continue
                }
                members.append(net)
            }
            func measured() -> [Int: Double] {
                Dictionary(uniqueKeysWithValues: members.map { ($0, length(of: $0, traces: cleanup.traces, problem: problem)) })
            }
            var lengths = measured()
            let target = lengths.values.max() ?? 0
            var reason: String?
            for net in members.sorted() where tuneLengths && target - (lengths[net] ?? 0) > group.tolerance {
                let deficit = target - (lengths[net] ?? 0) - group.tolerance / 2
                let added = tune(net, by: deficit, group: group, cleanup: &cleanup, problem: problem)
                if added <= 0, !cleanup.traces.contains(where: { $0.net == net }) { reason = noRoutedCopper }
            }
            lengths = measured()
            let values = lengths.values
            let spread = (values.max() ?? 0) - (values.min() ?? 0)
            let status: GroupOutcome.Status
            if let (net, why) = ineligible {
                status = .ineligible(net: net, reason: why)
            } else if spread <= group.tolerance + 1e-6 {
                status = .matched
            } else {
                status = .residual(spread: spread, reason: reason)
            }
            results.append(GroupOutcome(group: group.id, name: group.name, target: target, lengths: lengths,
                                        spread: spread, status: status))
        }
        // Differential pairs: a coupled pair's shorter net is tuned to the
        // longer, its bumps only on the side away from its partner, so the
        // pair is never squeezed (plan-38-J § 3 "Skew").
        for group in problem.groups.sorted(by: { $0.id.uuidString < $1.id.uuidString }) where group.mode == .diffPair {
            let status = pairs[group.id] ?? .uncoupled(reason: "routing pairs is off")
            var lengths = Dictionary(uniqueKeysWithValues: group.nets.map {
                ($0, length(of: $0, traces: cleanup.traces, problem: problem))
            })
            if case .coupled = status, group.nets.count == 2, tuneSkew {
                let (a, b) = (group.nets[0], group.nets[1])
                let short = (lengths[a] ?? 0) < (lengths[b] ?? 0) ? a : b
                let partner = short == a ? b : a
                let skew = abs((lengths[a] ?? 0) - (lengths[b] ?? 0))
                if skew > group.tolerance {
                    let partnerTraces = cleanup.traces.filter { $0.net == partner }
                    let outer: (CGPoint, CGPoint) -> Double = { u, v in
                        let l = hypot(v.x - u.x, v.y - u.y)
                        let n = CGPoint(x: -(v.y - u.y) / l, y: (v.x - u.x) / l)
                        let mid = CGPoint(x: (u.x + v.x) / 2, y: (u.y + v.y) / 2)
                        func distance(_ sign: Double) -> Double {
                            let p = CGPoint(x: mid.x + n.x * sign, y: mid.y + n.y * sign)
                            var nearest = Double.infinity
                            for t in partnerTraces {
                                for (c, d) in zip(t.points, t.points.dropFirst()) {
                                    nearest = min(nearest, SchematicDocument.distance(from: p, toSegment: c, d))
                                }
                            }
                            return nearest
                        }
                        return distance(1) >= distance(-1) ? 1 : -1
                    }
                    _ = tune(short, by: skew - group.tolerance / 2, group: group, cleanup: &cleanup, problem: problem,
                             side: outer)
                    lengths = Dictionary(uniqueKeysWithValues: group.nets.map {
                        ($0, length(of: $0, traces: cleanup.traces, problem: problem))
                    })
                }
            }
            let values = lengths.values
            results.append(GroupOutcome(group: group.id, name: group.name, target: values.max() ?? 0, lengths: lengths,
                                        spread: (values.max() ?? 0) - (values.min() ?? 0), status: status))
        }
        traces = cleanup.traces
        return results
    }

    /// Adds up to `deficit` mm to net `net` with bumps on its new traces'
    /// straight segments, longest first; returns what it added.
    static func tune(_ net: Int, by deficit: Double, group: RoutingGroup, cleanup: inout RoutingCleanup,
                     problem: RoutingProblem, side forced: ((CGPoint, CGPoint) -> Double)? = nil) -> Double {
        guard let routing = problem.nets.first(where: { $0.id == net }) else { return 0 }
        let width = routing.width
        let gap = max(routing.clearance, problem.rules.minClearance, 2 * width)
        let spacing = width + gap
        let cut = problem.options.angle == .octilinear ? spacing / 4 : 0
        let footprint = Meander.footprint(spacing: spacing, cut: cut)
        let room = 2 * spacing
        var left = deficit
        // Hosts: every segment of the net's new traces, longest first.
        var hosts: [(trace: Int, a: CGPoint, b: CGPoint, length: Double)] = []
        for t in cleanup.traces.indices where cleanup.traces[t].net == net {
            _ = cleanup.anchors(of: t)
            let points = cleanup.traces[t].points
            for (a, b) in zip(points, points.dropFirst()) {
                hosts.append((t, a, b, hypot(b.x - a.x, b.y - a.y)))
            }
        }
        hosts.sort { $0.length != $1.length ? $0.length > $1.length : $0.trace < $1.trace }
        for host in hosts where left > 1e-6 && host.length >= 2 * room + footprint {
            let layer = cleanup.traces[host.trace].layer
            var bumps: [CGPoint] = []
            var offset = room + cut
            var side: Double = forced?(host.a, host.b) ?? 1
            while left > 1e-6, offset + spacing + cut <= host.length - room {
                let full = Meander.added(amplitude: group.maxMeander,
                                         cut: Meander.fitted(cut: cut, amplitude: group.maxMeander, spacing: spacing))
                let want = full <= left ? group.maxMeander
                    : Meander.amplitude(forAdded: left, cut: cut, spacing: spacing, in: 0...group.maxMeander)
                var placed = false
                let tries = forced == nil ? [(want, side), (want / 2, side), (want, -side), (want / 2, -side)] : [(want, side), (want / 2, side)]
                for (amplitude, s) in tries where amplitude > 1e-4 {
                    let points = Meander.bumpPoints(from: host.a, to: host.b, at: offset, side: s, amplitude: amplitude,
                                                    spacing: spacing, cut: cut)
                    guard cleanup.isLegal(points, layer: layer, width: width, net: net) else { continue }
                    bumps += points
                    left -= Meander.added(amplitude: amplitude,
                                          cut: Meander.fitted(cut: cut, amplitude: amplitude, spacing: spacing))
                    side = forced == nil ? -s : s
                    placed = true
                    break
                }
                offset += 2 * spacing
                if !placed { continue }
            }
            guard !bumps.isEmpty else { continue }
            var points = cleanup.traces[host.trace].points
            if let k = points.indices.dropLast().first(where: { TraceRoute.same(points[$0], host.a) && TraceRoute.same(points[$0 + 1], host.b) }) {
                points.insert(contentsOf: bumps, at: k + 1)
                cleanup.replace(host.trace, points: TraceRoute.clean(points))
            }
        }
        return deficit - left
    }
}
