import CoreGraphics
import Foundation

extension SchematicDocument {
    /// The routing job `options` describe on this document, as a value the
    /// engine can work on off the main actor (plan-38-D § 3):
    ///
    /// - **Scope** picks nets: all, some by key, those on some parts' pads,
    ///   or one unrouted line (its two ends name two components of a net).
    /// - **Include**: a net is routed when it has 2 or more components;
    ///   `.untouchedNets` also needs it to have no trace or via.
    /// - **Existing**: locked copper always stays; `.clearAuto` removes the
    ///   scope's unlocked auto traces and vias, `.clearUnlocked` all of its
    ///   unlocked ones. Copper on no net goes only with `.clearUnlocked` and
    ///   scope `.all`. Removed copper is named in `removedTraces` /
    ///   `removedVias` and is absent from the rest.
    /// - Every other piece of copper is an obstacle tagged with its net, so
    ///   a net may run over its own copper and never another's.
    func routingProblem(options: RoutingOptions, rules: DesignRules = DesignRules.current()) -> RoutingProblem {
        let full = connectivity()
        let fullNetOfRoot = full.netOfRoot()
        let scoped = scopeNets(options.scope, in: full)

        // What the options clear.
        var removedTraces = Set<UUID>(), removedVias = Set<UUID>()
        if options.existing != .keepAll {
            func clears(locked: Bool, auto: Bool, net: Int?) -> Bool {
                guard !locked, options.existing == .clearUnlocked || auto else { return false }
                guard let net else {
                    if case .all = options.scope { return options.existing == .clearUnlocked }
                    return false
                }
                return scoped.contains(net)
            }
            for (t, trace) in full.traces.enumerated()
            where clears(locked: trace.locked, auto: trace.auto, net: fullNetOfRoot[full.root[full.traceNode(t)]]) {
                removedTraces.insert(trace.id)
            }
            for (v, via) in full.vias.enumerated()
            where clears(locked: via.locked, auto: via.auto, net: fullNetOfRoot[full.root[full.viaNode(v)]]) {
                removedVias.insert(via.id)
            }
        }
        let copper = removedTraces.isEmpty && removedVias.isEmpty
            ? full : connectivity(excludingTraces: removedTraces, excludingVias: removedVias)
        let netOfRoot = copper.netOfRoot()

        // Each net's components, by root, in node order.
        var componentRoots: [Int: [Int]] = [:]
        for (pad, net) in copper.netOfPad.sorted(by: { $0.key < $1.key }) {
            let root = copper.root[pad]
            if componentRoots[net]?.contains(root) != true { componentRoots[net, default: []].append(root) }
        }
        var hasCopper = Set<Int>()
        for t in copper.traces.indices { if let net = netOfRoot[copper.root[copper.traceNode(t)]] { hasCopper.insert(net) } }
        for v in copper.vias.indices { if let net = netOfRoot[copper.root[copper.viaNode(v)]] { hasCopper.insert(net) } }

        let routable = options.enabled(board.traceLayers)
        let planes: [CopperLayer: Int] = copper.planeNets

        func padLayers(_ pad: BoardConnectivity.Pad) -> Set<CopperLayer>? {
            pad.pad.drill > 0 ? nil : [pad.side.copperLayer]
        }
        func component(_ root: Int) -> RoutingComponent {
            var result = RoutingComponent()
            for (p, pad) in copper.pads.enumerated() where copper.root[p] == root && pad.pad.plated {
                result.pads.append(CopperGeometry.shape(of: pad.pad, layers: padLayers(pad), node: p))
                result.padLabels.append(pad.label)
            }
            for (t, trace) in copper.traces.enumerated() where copper.root[copper.traceNode(t)] == root {
                result.traces.append(RoutedTrace(layer: trace.layer, width: trace.widthMM, points: trace.points))
            }
            for (v, via) in copper.vias.enumerated() where copper.root[copper.viaNode(v)] == root {
                result.vias.append(RoutedVia(centre: via.centre, drill: via.drill, diameter: via.diameter))
            }
            return result
        }

        // The nets to route.
        var nets: [RoutingNet] = []
        for net in scoped.sorted() {
            guard var roots = componentRoots[net], let key = copper.planeNet(ofNet: net) else { continue }
            if case let .connection(from, to) = options.scope {
                guard let a = componentRoot(at: from, net: net, roots: roots, in: copper),
                      let b = componentRoot(at: to, net: net, roots: roots, in: copper), a != b else { continue }
                roots = [a, b]
            }
            guard roots.count >= 2 else { continue }
            if options.include == .untouchedNets, hasCopper.contains(net) { continue }
            let netClass = netClass(ofNet: net, in: copper)
            let mil = options.widthOverride ?? netClass?.width ?? NetClass.defaultWidth
            let width = max(Double(mil) * 0.0254, rules.minTraceWidth)
            let clearance = max(options.clearanceOverride ?? netClass?.clearance ?? rules.minClearance, rules.minClearance)
            let effective = effectiveLayers(ofNet: net, in: copper)
            let allowed = Set(effective.allowed).intersection(routable)
            let preferred = Set(effective.preferred).intersection(allowed)
            let name = copper.netNames[net] ?? "Net \(net + 1)"
            var impossible: String?
            if allowed.isEmpty {
                impossible = "\(name) has no layer it may be routed on"
            } else if let pad = copper.pads.indices.first(where: {
                copper.netOfPad[$0] == net && copper.pads[$0].pad.plated && copper.pads[$0].pad.drill == 0
                    && !allowed.contains(copper.pads[$0].side.copperLayer)
            }) {
                let limit = routable.filter(allowed.contains).map(\.title).joined(separator: " and ")
                impossible = "Pad \(copper.pads[pad].label) is on \(copper.pads[pad].side.copperLayer.title); "
                    + "\(name) is limited to \(limit)"
            }
            nets.append(RoutingNet(
                id: net, name: name, key: key, width: width, clearance: clearance,
                viaDrill: netClass?.viaDrill ?? NetClass.defaultViaDrill,
                viaDiameter: netClass?.viaDiameter ?? NetClass.defaultViaDiameter,
                allowed: allowed, preferred: preferred, components: roots.map(component),
                planeLayer: board.copperLayers.first { planes[$0] == net },
                group: group(ofNet: net, in: copper)?.id, impossible: impossible))
        }

        // Every piece of copper and every keep-out, tagged with its net.
        var obstacles: [RoutingObstacle] = []
        for (p, pad) in copper.pads.enumerated() {
            let isBoardPad = pad.chip < 0 && pad.hole == nil && !pad.label.hasPrefix("Board hole ")
            if pad.pad.plated {
                obstacles.append(RoutingObstacle(shape: CopperGeometry.shape(of: pad.pad, layers: padLayers(pad), node: p),
                                                 net: copper.netOfPad[p], exemptFromEdge: isBoardPad))
            } else if let hole = pad.pad.holeSegment {
                obstacles.append(RoutingObstacle(shape: CopperShape(core: .segment(hole.a, hole.b), radius: hole.radius,
                                                                    layers: nil, node: p), net: nil))
            }
        }
        for (t, trace) in copper.traces.enumerated() {
            let node = copper.traceNode(t)
            for (a, b) in trace.segments {
                obstacles.append(RoutingObstacle(shape: CopperShape(core: .segment(a, b), radius: trace.widthMM / 2,
                                                                    layers: [trace.layer], node: node),
                                                 net: netOfRoot[copper.root[node]]))
            }
        }
        for (v, via) in copper.vias.enumerated() {
            let node = copper.viaNode(v)
            obstacles.append(RoutingObstacle(shape: CopperShape(core: .segment(via.centre, via.centre),
                                                                radius: via.diameter / 2, layers: nil, node: node),
                                             net: netOfRoot[copper.root[node]]))
        }
        let outline = board.outline
        for (a, b) in zip(outline, outline.dropFirst() + outline.prefix(1)) {
            obstacles.append(RoutingObstacle(shape: CopperShape(core: .segment(a, b), radius: 0, layers: nil, node: -1),
                                             net: nil))
        }

        let groups = board.netGroups.map { group in
            RoutingGroup(id: group.id, name: group.name, mode: group.mode,
                         nets: group.members.compactMap { copper.net(of: $0) }, tolerance: group.tolerance,
                         gap: group.gap, maxUncoupled: group.maxUncoupled, maxMeander: group.maxMeander)
        }
        return RoutingProblem(outline: outline, layers: board.copperLayers, routable: routable, planes: planes,
                              rules: rules, options: options, nets: nets, obstacles: obstacles, groups: groups,
                              removedTraces: removedTraces, removedVias: removedVias)
    }

    /// The nets `scope` names in `copper`.
    private func scopeNets(_ scope: RoutingOptions.Scope, in copper: BoardConnectivity) -> Set<Int> {
        let all = Set(copper.netOfPad.values)
        switch scope {
        case .all:
            return all
        case let .nets(keys):
            return Set(keys.compactMap { copper.net(of: $0) }).intersection(all)
        case let .footprints(ids):
            let indices = Set(chips.indices.filter { ids.contains(chips[$0].id) })
            return Set(copper.pads.indices.filter { indices.contains(copper.pads[$0].chip) }.compactMap {
                copper.netOfPad[$0]
            })
        case let .connection(from, to):
            let netOfRoot = copper.netOfRoot()
            let roots = [from, to].compactMap { point in
                componentRoot(at: point, net: nil, roots: nil, in: copper)
            }
            let nets = Set(roots.compactMap { netOfRoot[$0] })
            return roots.count == 2 && nets.count == 1 ? nets : []
        }
    }

    /// The root of the copper group with a pad centre or trace vertex at
    /// `point` (within 1 µm), of net `net` if given, among `roots` if given.
    private func componentRoot(at point: CGPoint, net: Int?, roots: [Int]?, in copper: BoardConnectivity) -> Int? {
        let netOfRoot = copper.netOfRoot()
        func ok(_ root: Int) -> Bool {
            (net == nil || netOfRoot[root] == net) && (roots?.contains(root) ?? true) && netOfRoot[root] != nil
        }
        func near(_ a: CGPoint) -> Bool { hypot(a.x - point.x, a.y - point.y) < 1e-3 }
        for (p, pad) in copper.pads.enumerated() where near(pad.centre) && ok(copper.root[p]) { return copper.root[p] }
        for (t, trace) in copper.traces.enumerated() {
            let root = copper.root[copper.traceNode(t)]
            if ok(root), trace.points.contains(where: near) { return root }
        }
        return nil
    }
}
