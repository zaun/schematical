import CoreGraphics
import Foundation
import Observation

/// What is on the PCB page (plan-68-E): the board's settings, texts,
/// traces and vias, holes, pours, keep-outs and cutouts. Each is observed
/// on its own, as when the document stored them, and only changes through
/// an `update…` call, which needs an `EditScope`.
@MainActor
@Observable
final class BoardContent {
    nonisolated deinit {}

    /// The PCB's size, layers, colour, units and rules.
    private(set) var settings = BoardSettings() { didSet { copperRevision &+= 1 } }
    /// Free texts on the PCB, in drawing order.
    private(set) var texts: [BoardText] = []
    /// Copper traces, in drawing order (plan-16).
    private(set) var traces: [BoardTrace] = [] { didSet { copperRevision &+= 1 } }
    private(set) var vias: [BoardVia] = [] { didSet { copperRevision &+= 1 } }
    /// Drilled holes (mounting holes and others), in drawing order.
    private(set) var holes: [BoardHole] = [] { didSet { copperRevision &+= 1 } }
    /// Copper pours (plan-52), in drawing order: a pour fills before the
    /// ones after it, and gives way to the ones before it.
    private(set) var pours: [BoardPour] = [] { didSet { copperRevision &+= 1 } }
    /// Rectangles the pours stay out of (plan-52).
    private(set) var keepouts: [BoardKeepout] = [] { didSet { copperRevision &+= 1 } }
    /// Regions of the board with no material (plan-65), in drawing order.
    private(set) var cutouts: [BoardCutout] = [] { didSet { copperRevision &+= 1 } }
    /// Counts every change to what connectivity reads here (all but the
    /// texts); the document adds it to its own.
    private(set) var copperRevision = 0

    /// The last connectivity, ratsnest and pour fills, and what they were
    /// built from (see Ratsnest.swift, PourConnectivity.swift).
    @ObservationIgnored var keptConnectivity: (revision: Int, copper: BoardConnectivity)?
    @ObservationIgnored var keptPourFills: (revision: Int, pours: [BoardPour], keepouts: [BoardKeepout],
                                            edge: BoardEdge, fills: [UUID: PourFill])?
    @ObservationIgnored var keptRatsnest: (revision: Int, lines: [(CGPoint, CGPoint)])?

    var boardTexts: BoardTexts { BoardTexts(texts: texts) }
    var copper: BoardCopper { BoardCopper(traces: traces, vias: vias) }
    var features: BoardFeatures { BoardFeatures(holes: holes, pours: pours, keepouts: keepouts, cutouts: cutouts) }

    func updateSettings<T>(_ scope: EditScope, _ body: (inout BoardSettings) throws -> T) rethrows -> T {
        var value = settings
        defer { if value != settings { settings = value } }
        return try body(&value)
    }

    func updateTexts<T>(_ scope: EditScope, _ body: (inout BoardTexts) throws -> T) rethrows -> T {
        var value = boardTexts
        defer { if value.texts != texts { texts = value.texts } }
        return try body(&value)
    }

    func updateCopper<T>(_ scope: EditScope, _ body: (inout BoardCopper) throws -> T) rethrows -> T {
        var value = copper
        defer {
            if value.traces != traces { traces = value.traces }
            if value.vias != vias { vias = value.vias }
        }
        return try body(&value)
    }

    func updateFeatures<T>(_ scope: EditScope, _ body: (inout BoardFeatures) throws -> T) rethrows -> T {
        var value = features
        defer {
            if value.holes != holes { holes = value.holes }
            if value.pours != pours { pours = value.pours }
            if value.keepouts != keepouts { keepouts = value.keepouts }
            if value.cutouts != cutouts { cutouts = value.cutouts }
        }
        return try body(&value)
    }

    /// Writes saved content back (undo, redo, a file load): only what differs.
    func restore(settings: BoardSettings, texts: [BoardText], traces: [BoardTrace], vias: [BoardVia],
                 holes: [BoardHole], pours: [BoardPour], keepouts: [BoardKeepout], cutouts: [BoardCutout],
                 _ scope: EditScope) {
        if self.settings != settings { self.settings = settings }
        if self.texts != texts { self.texts = texts }
        if self.traces != traces { self.traces = traces }
        if self.vias != vias { self.vias = vias }
        if self.holes != holes { self.holes = holes }
        if self.pours != pours { self.pours = pours }
        if self.keepouts != keepouts { self.keepouts = keepouts }
        if self.cutouts != cutouts { self.cutouts = cutouts }
    }
}
