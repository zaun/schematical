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
    /// The board's copper layers in stack order.
    var layers: [CopperLayer]
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

    /// Connections to make, over every net.
    var connections: Int { nets.reduce(0) { $0 + $1.connections } }
}
