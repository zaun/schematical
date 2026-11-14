import CoreGraphics
import Foundation

/// What one Autoroute run does (plan-38-D § 4). Everything but `scope` is
/// kept between runs (plan-38-K); `scope` comes from the selection.
nonisolated struct RoutingOptions: Codable, Hashable, Sendable {
    /// Which nets to route.
    enum Scope: Codable, Hashable, Sendable {
        case all
        case nets([PlaneNet])
        /// Every net on a pad of these parts.
        case footprints([UUID])
        /// One unrouted line: its two end points name two copper groups of
        /// one net.
        case connection(from: CGPoint, to: CGPoint)
    }

    /// Which of the scope's nets are routed.
    enum Include: String, Codable, Hashable, Sendable, CaseIterable {
        /// Every net with 2 or more copper groups.
        case incompleteNets
        /// Only such nets with no trace or via at all.
        case untouchedNets
    }

    /// What happens to the copper already there. Locked copper always stays.
    enum Existing: String, Codable, Hashable, Sendable, CaseIterable {
        case keepAll
        /// Remove the scope's unlocked autorouted traces and vias.
        case clearAuto
        /// Remove all of the scope's unlocked traces and vias.
        case clearUnlocked
    }

    enum Direction: String, Codable, Hashable, Sendable, CaseIterable {
        case auto, horizontal, vertical
    }

    struct LayerOption: Codable, Hashable, Sendable {
        var enabled = true
        var direction = Direction.auto
    }

    enum Vias: String, Codable, Hashable, Sendable, CaseIterable {
        case allowed, none
    }

    enum Angle: String, Codable, Hashable, Sendable, CaseIterable {
        /// 0°, 45° and 90° segments.
        case octilinear
        /// 0° and 90° only.
        case orthogonal
    }

    enum Strategy: String, Codable, Hashable, Sendable, CaseIterable {
        case fast, balanced, thorough
    }

    /// The routing grids, mm.
    static let grids: [Double] = [0.05, 0.1, 0.127, 0.254]

    var scope: Scope = .all
    var include: Include = .incompleteNets
    var existing: Existing = .keepAll
    /// By layer; a routable layer without an entry is enabled, `.auto`.
    var layers: [CopperLayer: LayerOption] = [:]
    var vias: Vias = .allowed
    var angle: Angle = .octilinear
    var strategy: Strategy = .balanced
    /// mm, one of `grids`.
    var grid: Double = 0.1
    /// mil; overrides every net's class width.
    var widthOverride: Int? = nil
    /// mm; overrides every net's class clearance.
    var clearanceOverride: Double? = nil
    var fanoutPlanes = true
    var tuneGroups = true
    var routePairs = true
    var seed: UInt64 = 1
    /// Seconds; nil is the strategy's own.
    var timeLimit: Double? = nil

    static let standard = RoutingOptions()

    /// The option for `layer`.
    func option(for layer: CopperLayer) -> LayerOption { layers[layer] ?? LayerOption() }

    /// The layers of `routable` this run may use.
    func enabled(_ routable: [CopperLayer]) -> [CopperLayer] { routable.filter { option(for: $0).enabled } }

    private enum CodingKeys: String, CodingKey {
        case scope, include, existing, layers, vias, angle, strategy, grid, widthOverride, clearanceOverride,
             fanoutPlanes, tuneGroups, routePairs, seed, timeLimit
    }

    init() {}

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        scope = try c.decodeIfPresent(Scope.self, forKey: .scope) ?? .all
        include = try c.decodeIfPresent(Include.self, forKey: .include) ?? .incompleteNets
        existing = try c.decodeIfPresent(Existing.self, forKey: .existing) ?? .keepAll
        let rawLayers = try c.decodeIfPresent([String: LayerOption].self, forKey: .layers) ?? [:]
        layers = [:]
        for (raw, option) in rawLayers {
            guard let layer = CopperLayer(rawValue: raw) else {
                throw DecodingError.dataCorruptedError(forKey: .layers, in: c, debugDescription: "layer \(raw)")
            }
            layers[layer] = option
        }
        vias = try c.decodeIfPresent(Vias.self, forKey: .vias) ?? .allowed
        angle = try c.decodeIfPresent(Angle.self, forKey: .angle) ?? .octilinear
        strategy = try c.decodeIfPresent(Strategy.self, forKey: .strategy) ?? .balanced
        grid = try c.decodeIfPresent(Double.self, forKey: .grid) ?? 0.1
        guard Self.grids.contains(grid) else {
            throw DecodingError.dataCorruptedError(forKey: .grid, in: c, debugDescription: "grid \(grid)")
        }
        widthOverride = try c.decodeIfPresent(Int.self, forKey: .widthOverride)
        clearanceOverride = try c.decodeIfPresent(Double.self, forKey: .clearanceOverride)
        fanoutPlanes = try c.decodeIfPresent(Bool.self, forKey: .fanoutPlanes) ?? true
        tuneGroups = try c.decodeIfPresent(Bool.self, forKey: .tuneGroups) ?? true
        routePairs = try c.decodeIfPresent(Bool.self, forKey: .routePairs) ?? true
        seed = try c.decodeIfPresent(UInt64.self, forKey: .seed) ?? 1
        timeLimit = try c.decodeIfPresent(Double.self, forKey: .timeLimit)
    }

    func encode(to encoder: Encoder) throws {
        var c = encoder.container(keyedBy: CodingKeys.self)
        try c.encode(scope, forKey: .scope)
        try c.encode(include, forKey: .include)
        try c.encode(existing, forKey: .existing)
        try c.encode(Dictionary(uniqueKeysWithValues: layers.map { ($0.key.rawValue, $0.value) }), forKey: .layers)
        try c.encode(vias, forKey: .vias)
        try c.encode(angle, forKey: .angle)
        try c.encode(strategy, forKey: .strategy)
        try c.encode(grid, forKey: .grid)
        try c.encodeIfPresent(widthOverride, forKey: .widthOverride)
        try c.encodeIfPresent(clearanceOverride, forKey: .clearanceOverride)
        try c.encode(fanoutPlanes, forKey: .fanoutPlanes)
        try c.encode(tuneGroups, forKey: .tuneGroups)
        try c.encode(routePairs, forKey: .routePairs)
        try c.encode(seed, forKey: .seed)
        try c.encodeIfPresent(timeLimit, forKey: .timeLimit)
    }
}

/// `PlaneNet` as its file text, so options holding nets are `Codable`.
extension PlaneNet: Codable {
    nonisolated init(from decoder: Decoder) throws {
        let c = try decoder.singleValueContainer()
        let raw = try c.decode(String.self)
        guard let net = PlaneNet(fileValue: raw) else {
            throw DecodingError.dataCorruptedError(in: c, debugDescription: "net \(raw)")
        }
        self = net
    }

    nonisolated func encode(to encoder: Encoder) throws {
        var c = encoder.singleValueContainer()
        try c.encode(fileValue)
    }
}
