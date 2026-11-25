import CoreGraphics
import Foundation

/// How one net came out of a run.
nonisolated enum NetStatus: Hashable, Sendable {
    /// Every connection routed.
    case complete
    /// Some routed, `unrouted` not.
    case partial(unrouted: Int)
    /// None routed.
    case failed
    /// Not searched: its rules leave it no way (`RoutingNet.impossible`).
    case impossible(String)
}

/// One net's outcome.
nonisolated struct NetOutcome: Hashable, Sendable {
    let net: Int
    var name: String
    var status: NetStatus
    /// Connections still unrouted.
    var unroutedConnections: Int
    /// Each unrouted connection, by a pad of each side ("U1.3 – U2.4").
    var unrouted: [String] = []
    /// Why some of it is unrouted, when the reason isn't simply that no way
    /// was found (`Router.illegalReason`).
    var reason: String? = nil
}

/// Counts for a run.
nonisolated struct RoutingStats: Hashable, Sendable {
    var connections = 0
    var routed = 0
    var vias = 0
    /// mm of new trace.
    var wireLength = 0.0
    var seconds = 0.0
    /// Corners in the new traces.
    var bends = 0
    var rounds = 0
    var rips = 0
    var expansions = 0
    /// Planning passes made, and the nodes two nets' plans still shared after the last.
    var planPasses = 0
    var planOveruse = 0
    /// Improvement moves tried and kept (`RoutingPolish`).
    var polishTried = 0
    var polishKept = 0
    /// Push moves tried and kept, and the traces and vias they moved
    /// (plan-66-D/E fill these; zero until then and whenever the option is off).
    var pushTried = 0
    var pushKept = 0
    var movedTraces = 0
    var movedVias = 0
    /// Whether the run stopped early (cancelled or out of time).
    var stopped = false
}

/// What a run produced: new copper (every piece tagged with its net) and
/// how each net came out.
nonisolated struct RoutingResult: Hashable, Sendable {
    var traces: [RoutedTrace] = []
    var vias: [RoutedVia] = []
    var nets: [NetOutcome] = []
    var stats = RoutingStats()
    /// Each length-match group's outcome (plan-38-I).
    var groups: [GroupOutcome] = []
}

/// How far a run has got, reported after each connection.
nonisolated struct RoutingProgress: Hashable, Sendable {
    /// The most connections routed at once so far this run: it never goes
    /// back, though rip-ups make `current` dip while negotiating.
    var routed: Int
    var total: Int
    /// The negotiation round, from 1; 0 during the first pass.
    var round: Int
    /// The net being worked on.
    var net: String
    /// The connections routed right now.
    var current: Int = 0
    /// The copper routed so far, for a live preview; set at most every
    /// half second (`Router.Run.snapshotInterval`), nil otherwise.
    var traces: [RoutedTrace]? = nil
    var vias: [RoutedVia]? = nil
}
