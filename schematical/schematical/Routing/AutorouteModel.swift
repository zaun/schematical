import Foundation
import Observation

/// What the Autoroute sheet shows and does, apart from its layout
/// (plan-38-K § 3): the options (remembered between runs, all but the scope),
/// the scope chosen, a summary of what a run would do, the layer rows, and
/// the run itself (`RoutingSession`) and its results.
@MainActor
@Observable
final class AutorouteModel {
    /// Which nets a run routes.
    enum ScopeChoice: Hashable {
        case all
        /// The PCB selection's nets (`RoutingScope.fromSelection`).
        case selection
        case nets([PlaneNet])
        case footprints([UUID])
        /// Exactly this scope (the Route tool, a Nets row).
        case scope(RoutingOptions.Scope)
    }

    /// What a run with the current options would do.
    struct Summary: Equatable {
        var nets: Int
        var connections: Int
        var removedTraces: Int
        var removedVias: Int
        /// Each net no run can route, with why.
        var impossible: [String]
    }

    /// One copper layer's row.
    struct LayerRow: Identifiable, Equatable {
        var layer: CopperLayer
        /// A Plane layer: never routed on, only fanned out to.
        var isPlane: Bool
        var enabled: Bool
        var direction: RoutingOptions.Direction
        var id: CopperLayer { layer }
    }

    /// A net a finished run left unrouted.
    struct ResultRow: Identifiable, Equatable {
        var key: PlaneNet?
        var name: String
        var count: Int
        var reason: String
        var id: String { name }
    }

    static let notFoundReason = "no legal way found"

    let document: SchematicDocument
    let session = RoutingSession()
    @ObservationIgnored private let defaults: UserDefaults
    var options: RoutingOptions {
        didSet { save() }
    }
    var scopeChoice: ScopeChoice = .all
    private(set) var summary: Summary?

    /// Not isolated: an isolated deinit here aborts in the concurrency
    /// runtime (a task-local scope freed twice), as `RoutingSession`'s did.
    nonisolated deinit {}

    init(document: SchematicDocument, defaults: UserDefaults = .standard) {
        self.document = document
        self.defaults = defaults
        options = Self.load(defaults)
    }

    // MARK: Options

    static func load(_ defaults: UserDefaults) -> RoutingOptions {
        guard let data = defaults.data(forKey: PreferenceKey.autorouteOptions),
              var options = try? JSONDecoder().decode(RoutingOptions.self, from: data) else { return .standard }
        options.scope = .all
        return options
    }

    private func save() {
        var stored = options
        stored.scope = .all
        if let data = try? JSONEncoder().encode(stored) { defaults.set(data, forKey: PreferenceKey.autorouteOptions) }
    }

    /// Whether the board can have vias: one layer can't.
    var viasAvailable: Bool { document.board.layers > 1 }

    /// The scope the choice names now.
    var scope: RoutingOptions.Scope {
        switch scopeChoice {
        case .all: .all
        case .selection: RoutingScope.fromSelection(document)
        case let .nets(keys): .nets(keys)
        case let .footprints(ids): .footprints(ids)
        case let .scope(scope): scope
        }
    }

    /// The options a run uses: the stored ones, the scope, and no vias on
    /// a 1-layer board.
    var runOptions: RoutingOptions {
        var run = options
        run.scope = scope
        if !viasAvailable { run.vias = .none }
        return run
    }

    // MARK: Summary

    /// Works out what a run would do (the sheet calls it, debounced, when
    /// the options or the board change).
    func refreshSummary(rules: DesignRules = DesignRules.current()) {
        let problem = document.routingProblem(options: runOptions, rules: rules)
        summary = Summary(nets: problem.nets.count, connections: problem.connections,
                          removedTraces: problem.removedTraces.count, removedVias: problem.removedVias.count,
                          impossible: problem.nets.compactMap { net in net.impossible })
    }

    // MARK: Layers

    var layerRows: [LayerRow] {
        document.board.copperLayers.map { layer in
            let option = options.option(for: layer)
            return LayerRow(layer: layer, isPlane: document.board.style(of: layer).type == .plane,
                            enabled: option.enabled, direction: option.direction)
        }
    }

    func setLayer(_ layer: CopperLayer, enabled: Bool? = nil, direction: RoutingOptions.Direction? = nil) {
        var option = options.option(for: layer)
        if let enabled { option.enabled = enabled }
        if let direction { option.direction = direction }
        options.layers[layer] = option
    }

    /// Top horizontal, Bottom vertical, and the routable inner layers
    /// alternating between them in stack order.
    func useAlternatingDirections() {
        let routable = document.board.traceLayers
        for (n, layer) in routable.enumerated() {
            let direction: RoutingOptions.Direction
            switch layer {
            case .top: direction = .horizontal
            case .bottom: direction = routable.count == 1 ? .horizontal : .vertical
            case .inner: direction = n.isMultiple(of: 2) ? .horizontal : .vertical
            }
            setLayer(layer, direction: direction)
        }
    }

    // MARK: Running

    var canRoute: Bool { !document.isPlaying && !session.isRunning }

    /// Starts a run; refused (with `editorMessage`) while the simulation
    /// plays or a run is under way.
    @discardableResult
    func route(rules: DesignRules = DesignRules.current()) -> Bool {
        session.start(document: document, options: runOptions, rules: rules)
    }

    func stop() { session.stop() }
    func cancel() { session.cancel() }

    /// The nets the last run left unrouted, by name, each once, with how
    /// many connections and why.
    var results: [ResultRow] {
        guard let result = session.result else { return [] }
        let keys = Dictionary(uniqueKeysWithValues: (session.problem?.nets ?? []).map { ($0.id, $0.key) })
        return result.nets.filter { $0.unroutedConnections > 0 }.map { outcome in
            var reason = outcome.reason ?? Self.notFoundReason
            if case let .impossible(why) = outcome.status { reason = why }
            return ResultRow(key: keys[outcome.net], name: outcome.name, count: outcome.unroutedConnections, reason: reason)
        }.sorted { $0.name.compare($1.name, options: .numeric) == .orderedAscending }
    }

    /// Each group's outcome, as a line of text.
    var groupLines: [String] {
        (session.result?.groups ?? []).map { group in
            let status: String
            switch group.status {
            case .matched: status = "matched"
            case let .residual(spread, reason): status = String(format: "%.2f mm apart", spread) + (reason.map { " (\($0))" } ?? "")
            case let .ineligible(_, reason): status = "not matched: \(reason)"
            case let .coupled(uncoupled): status = String(format: "coupled, %.2f mm apart", uncoupled)
            case let .uncoupled(reason): status = "routed uncoupled: \(reason)"
            }
            return "\(group.name): \(status)"
        }
    }
}
