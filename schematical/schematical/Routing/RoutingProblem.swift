import CoreGraphics
import Foundation

/// A trace as the router sees it: its layer, its width in mm and its points
/// in mm.
nonisolated struct RoutedTrace: Hashable, Sendable {
    var layer: CopperLayer
    /// mm.
    var width: Double
    var points: [CGPoint]
    /// The net it is on, in a routing result; nil in a problem's kept copper.
    var net: Int? = nil
}

/// A via as the router sees it, in mm.
nonisolated struct RoutedVia: Hashable, Sendable {
    var centre: CGPoint
    var drill: Double
    var diameter: Double
    /// The net it is on, in a routing result.
    var net: Int? = nil
}

/// One piece of a net already joined by copper: its pads' copper and the
/// kept traces and vias between them. The router joins a net's components.
nonisolated struct RoutingComponent: Hashable, Sendable {
    var pads: [CopperShape] = []
    var traces: [RoutedTrace] = []
    var vias: [RoutedVia] = []
    /// The pads' labels ("U1.3"), for messages.
    var padLabels: [String] = []
}

/// A net to route, with every rule that holds for it.
nonisolated struct RoutingNet: Identifiable, Hashable, Sendable {
    /// Its net in the document's connectivity at the snapshot.
    let id: Int
    var name: String
    var key: PlaneNet
    /// mm.
    var width: Double
    /// mm, never below the DRC's min clearance.
    var clearance: Double
    var viaDrill: Double
    var viaDiameter: Double
    /// The layers it may use (its layer rule ∩ the run's enabled layers).
    var allowed: Set<CopperLayer>
    /// The layers it prefers; empty is no preference.
    var preferred: Set<CopperLayer>
    var components: [RoutingComponent]
    /// The Plane layer carrying the net, if one: its through-hole pads and
    /// vias are joined by the Plane.
    var planeLayer: CopperLayer?
    var group: UUID?
    /// Why it can't be routed at all (a pad off its only layers), or nil.
    var impossible: String?
    /// Its pins on the board (the pads of one pin are one pin); a group
    /// tunes only two-pin nets (plan-38-I).
    var pins: Int = 2

    /// Connections to make: components less one.
    var connections: Int { max(0, components.count - 1) }
}

/// Copper or a keep-out the router must keep clear of. `net` nil is no
/// net: a non-plated hole, a board hole, the outline. A shape with a net
/// may be crossed by that net's own copper.
nonisolated struct RoutingObstacle: Hashable, Sendable {
    enum Kind: Hashable, Sendable {
        case pad, trace, via, hole, outline
    }

    var shape: CopperShape
    var net: Int?
    var kind: Kind = .pad
    /// A plated through-hole pad's or a via's hole (hole-to-hole checks).
    var drill: CopperShape? = nil
    /// An edge finger or header pad: at the board's edge by design, so the
    /// router doesn't keep it the copper-to-edge distance from the outline
    /// (as `checkEdge` doesn't).
    var exemptFromEdge = false
}

/// A net group as the router reads it, with its nets resolved.
nonisolated struct RoutingGroup: Identifiable, Hashable, Sendable {
    let id: UUID
    var name: String
    var mode: NetGroupMode
    /// Net ids, in member order; members that name no net are left out.
    var nets: [Int]
    var tolerance: Double
    var gap: Double
    var maxUncoupled: Double
    var maxMeander: Double
}

/// A value-type picture of one routing job: the board, the nets to route
/// with their rules, and every other piece of copper as an obstacle. Built
/// on the main actor (`SchematicDocument.routingProblem`); the engine runs
/// on it off the main actor and never touches the document.
nonisolated struct RoutingProblem: Hashable, Sendable {
    /// The board's outline, clockwise, mm.
    var outline: [CGPoint]
    /// The board's cutouts, each a loop, mm (plan-65-D): edges the copper stays out of and away from.
    var cutouts: [[CGPoint]] = []
    /// The board's copper layers in stack order.
    var layers: [CopperLayer]
    /// The outline and the cutouts as one `BoardEdge`: what the grid, the verifier and the cleanup read.
    var edge: BoardEdge { BoardEdge(outline: outline, cutouts: cutouts) }
    /// The layers traces may go on this run (no Planes, only enabled ones).
    var routable: [CopperLayer]
    /// Each Plane layer's net, for those that have one.
    var planes: [CopperLayer: Int]
    var rules: DesignRules
    var options: RoutingOptions
    var nets: [RoutingNet]
    var obstacles: [RoutingObstacle]
    var groups: [RoutingGroup]
    /// The unlocked copper the options clear before routing (plan-38-G
    /// deletes it); absent from `nets` and `obstacles`.
    var removedTraces: Set<UUID>
    var removedVias: Set<UUID>
    /// Existing copper of nets this run does not route that push and shove
    /// may move (plan-66-B): stretches of trace and via between anchors.
    /// Empty unless `options.pushAndShove`. Their traces and vias are not in
    /// `obstacles`.
    var movable: [MovableChain] = []
    /// One `RoutingNet` of rules (no components) for each net with a chain.
    var bystanders: [RoutingNet] = []

    /// Connections to make, over every net.
    var connections: Int { nets.reduce(0) { $0 + $1.connections } }
}

/// A stretch of one net's existing copper from one anchor to the next (a
/// pad, a junction, a free end, a via with other than two ends), through
/// bends and two-ended vias: the unit push and shove moves (plan-66).
nonisolated struct MovableChain: Hashable, Sendable, Identifiable {
    /// Index into `RoutingProblem.movable`.
    let id: Int
    /// The net, as `BoardConnectivity` numbers it.
    var net: Int
    /// mm: every run's width.
    var width: Double
    /// mm; both 0 when the chain has no via.
    var viaDrill: Double
    var viaDiameter: Double
    /// End A to end B.
    var runs: [Run]
    /// `runs.count - 1` of them: via `i` joins runs `i` and `i + 1`.
    var vias: [Via]
    /// Exactly two: A (the start of `runs[0]`) and B (the end of `runs.last`).
    var ends: [End]

    struct Run: Hashable, Sendable {
        /// The `BoardTrace` the run lies in.
        var trace: UUID
        var layer: CopperLayer
        /// Vertex indices into that trace's points, `first < last`.
        var first: Int
        var last: Int
        /// The chain runs from `last` down to `first`.
        var reversed: Bool
        /// In chain order, mm.
        var points: [CGPoint]
    }

    struct Via: Hashable, Sendable {
        var id: UUID
        var centre: CGPoint
    }

    struct End: Hashable, Sendable {
        /// The anchor's exact point.
        var point: CGPoint
        /// The layer of the run that ends here.
        var layer: CopperLayer
        /// What a search starts from.
        var anchor: RoutingComponent
    }
}
