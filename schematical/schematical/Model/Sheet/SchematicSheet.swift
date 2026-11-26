import Foundation
import Observation

/// What is drawn on one schematic sheet (plan-68-D): its wires and
/// junctions, buses, net labels and text blocks. Each array is observed on
/// its own, as when the document stored them, and only changes through an
/// `update…` call, which needs an `EditScope`: only a recorded edit (or the
/// document's undo and file load) can make one.
@MainActor
@Observable
final class SchematicSheet {
    nonisolated deinit {}

    private(set) var wires: [Wire] = [] { didSet { copperRevision &+= 1 } }
    private(set) var junctions: [Junction] = []
    /// Buses, in drawing order (plan-13). They carry no current themselves.
    private(set) var buses: [Bus] = [] { didSet { copperRevision &+= 1 } }
    /// Net labels, in drawing order. Labels with the same name are one net.
    private(set) var netLabels: [NetLabel] = [] { didSet { copperRevision &+= 1 } }
    /// Text blocks, in drawing order.
    private(set) var notes: [SchematicNote] = []
    /// Counts every change to what connectivity reads here (wires, buses,
    /// net labels); the document adds it to its own.
    private(set) var copperRevision = 0
    /// The route cache every `Wiring` from here shares.
    @ObservationIgnored let routes = WireRoutes()

    var wiring: Wiring { Wiring(wires: wires, junctions: junctions, routes: routes) }
    var busNetwork: BusNetwork { BusNetwork(buses: buses) }
    var labels: NetLabels { NetLabels(labels: netLabels) }
    var sheetNotes: SheetNotes { SheetNotes(notes: notes) }

    /// Runs `body` on the wires and junctions and writes back the arrays it
    /// changed.
    func updateWiring<T>(_ scope: EditScope, _ body: (inout Wiring) throws -> T) rethrows -> T {
        var value = wiring
        defer {
            if value.wires != wires { wires = value.wires }
            if value.junctions != junctions { junctions = value.junctions }
        }
        return try body(&value)
    }

    func updateBuses<T>(_ scope: EditScope, _ body: (inout BusNetwork) throws -> T) rethrows -> T {
        var value = busNetwork
        defer { if value.buses != buses { buses = value.buses } }
        return try body(&value)
    }

    func updateLabels<T>(_ scope: EditScope, _ body: (inout NetLabels) throws -> T) rethrows -> T {
        var value = labels
        defer { if value.labels != netLabels { netLabels = value.labels } }
        return try body(&value)
    }

    func updateNotes<T>(_ scope: EditScope, _ body: (inout SheetNotes) throws -> T) rethrows -> T {
        var value = sheetNotes
        defer { if value.notes != notes { notes = value.notes } }
        return try body(&value)
    }

    /// Writes saved content back (undo, redo, a file load): only the arrays
    /// that differ.
    func restore(wires: [Wire], junctions: [Junction], buses: [Bus], netLabels: [NetLabel],
                 notes: [SchematicNote], _ scope: EditScope) {
        if self.wires != wires { self.wires = wires }
        if self.junctions != junctions { self.junctions = junctions }
        if self.buses != buses { self.buses = buses }
        if self.netLabels != netLabels { self.netLabels = netLabels }
        if self.notes != notes { self.notes = notes }
    }
}
