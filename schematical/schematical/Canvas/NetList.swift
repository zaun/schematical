import CoreGraphics
import Foundation

/// One net on the board, as the PCB sidebar's Nets section lists it.
nonisolated struct NetRow: Identifiable, Hashable, Sendable {
    /// The net's index in this connectivity (renumbers on any edit).
    let net: Int
    /// The net's stable name (`BoardConnectivity.planeNet(ofNet:)`): what
    /// its class, layer rule and group are keyed by.
    let key: PlaneNet
    let name: String
    /// Its pads on the board.
    let pads: Int
    /// The connections still to route: its copper groups less one.
    let unrouted: Int
    /// Its class; nil is Default.
    let classID: UUID?
    /// Its own layer rule, if set (not the class's).
    let layerRule: LayerRule?
    let groupID: UUID?
    /// Its pads' labels ("U1.3"), for the filter.
    let padLabels: [String]

    var id: PlaneNet { key }
    /// A supply rail (ground included).
    var isRail: Bool {
        if case .rail = key { return true }
        return false
    }

    /// Whether the filter text `filter` picks this row: blank picks every
    /// row; otherwise its name or one of its pads' labels contains it,
    /// ignoring case.
    func matches(_ filter: String) -> Bool {
        let text = filter.trimmingCharacters(in: .whitespaces)
        guard !text.isEmpty else { return true }
        return name.localizedCaseInsensitiveContains(text)
            || padLabels.contains { $0.localizedCaseInsensitiveContains(text) }
    }
}

/// What the net list is built from; the Nets section rebuilds it when this
/// changes.
struct NetListInput: Hashable {
    let chips: [PlacedChip]
    let wires: [Wire]
    let buses: [Bus]
    let labels: [NetLabel]
    let board: BoardSettings
    let traces: [BoardTrace]
    let vias: [BoardVia]
    let holes: [BoardHole]

    @MainActor init(_ document: SchematicDocument) {
        chips = document.chips
        wires = document.wires
        buses = document.buses
        labels = document.netLabels
        board = document.board
        traces = document.traces
        vias = document.vias
        holes = document.holes
    }
}

/// The copper of one net: what the canvas highlights.
nonisolated struct NetCopper: Hashable, Sendable {
    /// Indices into `BoardConnectivity.pads`.
    var pads: Set<Int> = []
    var traces: Set<UUID> = []
    var vias: Set<UUID> = []
}

extension SchematicDocument {
    /// Every net with a pad on the board: rails first, then by name in
    /// natural order.
    func netRows() -> [NetRow] {
        netRows(in: connectivity())
    }

    func netRows(in copper: BoardConnectivity) -> [NetRow] {
        var padsOfNet: [Int: [Int]] = [:]
        for (pad, net) in copper.netOfPad { padsOfNet[net, default: []].append(pad) }
        var unrouted: [Int: Int] = [:]
        for (net, groups) in copper.netGroups() { unrouted[net] = groups.count - 1 }
        let rows: [NetRow] = padsOfNet.compactMap { net, pads in
            guard let key = copper.planeNet(ofNet: net) else { return nil }
            return NetRow(net: net, key: key, name: copper.netNames[net] ?? "Net \(net + 1)", pads: pads.count,
                          unrouted: unrouted[net] ?? 0, classID: netClass(ofNet: net, in: copper)?.id,
                          layerRule: board.netOverrides.first { $0.net == key }?.layerRule,
                          groupID: group(ofNet: net, in: copper)?.id,
                          padLabels: pads.sorted().map { copper.pads[$0].label })
        }
        return rows.sorted { a, b in
            if a.isRail != b.isRail { return a.isRail }
            let order = a.name.compare(b.name, options: [.numeric, .caseInsensitive])
            return order == .orderedSame ? a.net < b.net : order == .orderedAscending
        }
    }

    /// The pads, traces and vias of net `net` of `copper`: everything
    /// joined to one of its pads.
    func copperItems(ofNet net: Int, in copper: BoardConnectivity) -> NetCopper {
        var roots = Set<Int>()
        var result = NetCopper()
        for (pad, padNet) in copper.netOfPad where padNet == net {
            result.pads.insert(pad)
            roots.insert(copper.root[pad])
        }
        for (t, trace) in copper.traces.enumerated() where roots.contains(copper.root[copper.traceNode(t)]) {
            result.traces.insert(trace.id)
        }
        for (v, via) in copper.vias.enumerated() where roots.contains(copper.root[copper.viaNode(v)]) {
            result.vias.insert(via.id)
        }
        return result
    }

    /// The copper of the net `key` names, or nil when it names none.
    func copperItems(ofNet key: PlaneNet) -> NetCopper? {
        let copper = connectivity()
        return copper.net(of: key).map { copperItems(ofNet: $0, in: copper) }
    }
}
