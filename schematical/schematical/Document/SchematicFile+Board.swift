import CoreGraphics
import Foundation
import UniformTypeIdentifiers

extension SchematicFile {
    // MARK: PCB values

    /// The file form of a board's design rules.
    static func entry(_ rules: DesignRules, manufacturer: DRCManufacturer) -> Board.DesignRulesEntry {
        var entry = Board.DesignRulesEntry(manufacturer: manufacturer.rawValue)
        for field in Board.DesignRulesEntry.fields { entry[keyPath: field.entry] = rules[keyPath: field.rule] }
        return entry
    }

    /// The stored design rules and their manufacturer. A file without
    /// them takes the app's defaults (Settings ▸ DRC Defaults), and so does
    /// each rule the entry leaves out; the manufacturer is then Custom
    /// unless the entry names one.
    static func designRules(_ entry: Board.DesignRulesEntry?) throws -> (DesignRules, DRCManufacturer) {
        var rules = DesignRules.appDefaults()
        guard let entry else { return (rules, DRCManufacturer.appDefault()) }
        for field in Board.DesignRulesEntry.fields {
            guard let value = entry[keyPath: field.entry] else { continue }
            guard value.isFinite, value >= 0 else {
                throw SchematicFileError.badPCB("design rule \(field.name) \(number(value)) must be 0 or more")
            }
            rules[keyPath: field.rule] = value
        }
        return (rules, entry.manufacturer.flatMap(DRCManufacturer.init(rawValue:)) ?? .custom)
    }

    /// The board's settings, with defaults for missing keys.
    func boardSettings() throws -> BoardSettings {
        var settings = BoardSettings()
        (settings.rules, settings.rulesManufacturer) = try Self.designRules(board?.designRules)
        guard let board else { return settings }
        func size(_ value: Double?, _ name: String) throws -> Double? {
            guard let value else { return nil }
            guard BoardSettings.sizeRange.contains(value) else {
                throw SchematicFileError.badPCB("board \(name) \(Self.number(value)) must be 1 to 1000 mm")
            }
            return value
        }
        settings.width = try size(board.width, "width") ?? settings.width
        settings.height = try size(board.height, "height") ?? settings.height
        if let layers = board.layers {
            guard BoardSettings.layerCounts.contains(layers) else {
                throw SchematicFileError.badPCB("board layers \(layers) must be 1, 2, 4 or 6")
            }
            settings.layers = layers
        }
        settings.color = try Self.value(board.color, of: BoardColor.self, "board color", default: settings.color)
        settings.units = try Self.value(board.units, of: BoardUnits.self, "board units", default: settings.units)
        if let grid = board.grid {
            guard BoardSettings.gridRange.contains(grid) else {
                throw SchematicFileError.badPCB("board grid \(Self.number(grid)) must be 0.01 to 10 mm")
            }
            settings.grid = grid
        }
        settings.stackup = try Self.stackup(board.stackup, layers: settings.layers)
        settings.styles = try Self.layerStyles(board.layerStyles ?? [], layers: settings.layers)
        settings.netClasses = try Self.netClasses(board.netClasses ?? [])
        settings.netOverrides = try Self.netOverrides(board.netLayerRules ?? [])
        settings.netGroups = try Self.netGroups(board.netGroups ?? [])
        settings.type = try Self.value(board.type, of: BoardType.self, "board type", default: .custom)
        settings.edgeBus = board.edgeBus
        if let entry = board.card { settings.customCard = Self.customCard(entry) }
        if let entry = board.cardOptions { settings.cardOptions = try Self.cardOptions(entry, of: settings.type) }
        if settings.needsTwoLayers, settings.layers < 2 {
            throw SchematicFileError.badPCB("board type \(settings.type.rawValue) needs 2 or more layers")
        }
        if settings.type == .customCard {
            let card = settings.customCard
            settings.width = max(settings.width, card.indent + card.tabLength)
            settings.height = max(settings.height, card.depth + 1)
        }
        if settings.type != .customCard, let card = settings.card {
            // A card's height is its standard's; its length is at least the least.
            settings.height = card.height
            settings.width = min(max(settings.width, card.minimumLength), card.maximumLength)
        }
        if let entries = board.outline {
            guard settings.type == .custom else {
                throw SchematicFileError.badPCB("board outline needs the board type custom")
            }
            let points = entries.map { CGPoint(x: $0.x, y: $0.y) }
            if let problem = BoardSettings.problem(outline: points) {
                throw SchematicFileError.badPCB("board outline \(problem)")
            }
            // The outline wins over the width and height the file also has.
            let bounds = BoardSettings.bounds(of: points)
            settings.customOutline = BoardGeometry.clockwise(points)
            settings.width = bounds.width
            settings.height = bounds.height
        }
        return settings
    }

    /// The board's stackup: the entry's values over the standard stackup for
    /// `layers` layers.
    static func stackup(_ entry: Board.StackupEntry?, layers: Int) throws -> BoardStackup {
        var stackup = BoardStackup.standard(layers: layers)
        guard let entry else { return stackup }
        if let raw = entry.finish {
            guard let finish = SurfaceFinish(rawValue: raw) else {
                let names = SurfaceFinish.allCases.map(\.rawValue).joined(separator: ", ")
                throw SchematicFileError.badPCB("board stackup finish \"\(raw)\" must be one of \(names)")
            }
            stackup.finish = finish
        }
        if let rise = entry.temperatureRise { stackup.temperatureRise = rise }
        if let copper = entry.copper { stackup.copper = copper }
        if let dielectrics = entry.dielectrics {
            stackup.dielectrics = try dielectrics.map { item in
                guard let kind = DielectricKind(rawValue: item.type) else {
                    throw SchematicFileError.badPCB("board stackup dielectric type \"\(item.type)\" must be core or prepreg")
                }
                return Dielectric(kind: kind, thickness: item.thickness, er: item.er)
            }
        }
        if let problem = BoardStackup.problem(copper: stackup.copper, dielectrics: stackup.dielectrics,
                                              temperatureRise: stackup.temperatureRise, layers: layers) {
            throw SchematicFileError.badPCB("board stackup \(problem)")
        }
        return stackup
    }

    /// The stored layer styles of a board with `layers` copper layers, with
    /// defaults for missing colours and types.
    static func layerStyles(_ entries: [Board.LayerStyleEntry],
                                    layers: Int) throws -> [CopperLayer: LayerStyle] {
        let onBoard = CopperLayer.allCases(for: layers)
        var styles: [CopperLayer: LayerStyle] = [:]
        for entry in entries {
            guard let layer = CopperLayer(rawValue: entry.layer) else {
                throw SchematicFileError.badPCB("layer \"\(entry.layer)\" must be top, inner2 to inner5, or bottom")
            }
            let name = "layer \(layer.rawValue)"
            guard onBoard.contains(layer) else {
                throw SchematicFileError.badPCB("layer \"\(layer.rawValue)\" is not on a \(layers)-layer board")
            }
            guard styles[layer] == nil else {
                throw SchematicFileError.badPCB("\(name) is listed twice")
            }
            var style = LayerStyle.defaultStyle(of: layer)
            if let raw = entry.color {
                guard let color = LayerColor(hex: raw) else {
                    throw SchematicFileError.badPCB("\(name) color \"\(raw)\" must be #RRGGBB")
                }
                style.color = color
            }
            if let raw = entry.type {
                let allowed: [LayerType] = layer.isInner ? [.plane, .wire] : [.component]
                guard let type = LayerType(rawValue: raw), allowed.contains(type) else {
                    let list = allowed.map(\.rawValue).joined(separator: " or ")
                    throw SchematicFileError.badPCB("\(name) type \"\(raw)\" must be \(list)")
                }
                style.type = type
            }
            if let raw = entry.net {
                guard layer.isInner, let net = PlaneNet(fileValue: raw) else {
                    throw SchematicFileError.badPCB(layer.isInner
                        ? "\(name) net \"\(raw)\" must be ground, power:<mV> or pin:<part id>/<pin>"
                        : "\(name) is not a plane; it has no net")
                }
                style.net = net
            }
            if let value = entry.clearance {
                guard layer.isInner else { throw SchematicFileError.badPCB("\(name) is not a plane; it has no clearance") }
                guard LayerStyle.clearanceRange.contains(value) else {
                    throw SchematicFileError.badPCB("\(name) clearance \(Self.number(value)) must be 0.05 to 10 mm")
                }
                style.clearance = value
            }
            styles[layer] = style
        }
        return styles
    }

    /// The stored net classes, each checked (`NetClass.problem`), with no
    /// id twice and every member a net name a Plane could have.
    static func netClasses(_ entries: [Board.NetClassEntry]) throws -> [NetClass] {
        var seen = Set<UUID>()
        return try entries.map { entry in
            let name = "net class \(entry.id.uuidString)"
            guard seen.insert(entry.id).inserted else { throw SchematicFileError.badPCB("\(name) is listed twice") }
            if let problem = NetClass.problem(width: entry.width, clearance: entry.clearance, viaDrill: entry.viaDrill,
                                              viaDiameter: entry.viaDiameter, name: entry.name) {
                throw SchematicFileError.badPCB("\(name) \(problem)")
            }
            let members = try entry.members.map { try Self.member($0, name) }
            var rule: LayerRule?
            if entry.layerMode != nil || entry.layers != nil {
                guard let mode = entry.layerMode, let layers = entry.layers else {
                    throw SchematicFileError.badPCB("\(name) needs both layerMode and layers, or neither")
                }
                rule = try Self.layerRule(mode: mode, layers: layers, name)
            }
            return NetClass(id: entry.id, name: entry.name, width: entry.width, clearance: entry.clearance,
                            viaDrill: entry.viaDrill, viaDiameter: entry.viaDiameter, members: members,
                            layerRule: rule)
        }
    }

    /// A stored net name (`PlaneNet.fileValue`) of `owner`.
    static func member(_ raw: String, _ owner: String) throws -> PlaneNet {
        guard let net = PlaneNet(fileValue: raw) else {
            throw SchematicFileError.badPCB(
                "\(owner) member \"\(raw)\" must be ground, power:<mV> or pin:<part id>/<pin>")
        }
        return net
    }

    /// A stored layer rule of `owner`. A layer not on the board is kept
    /// (it is the user's, as a layer style is); a name that is no layer
    /// is an error.
    static func layerRule(mode raw: String, layers: [String], _ owner: String) throws -> LayerRule {
        guard let mode = LayerRuleMode(rawValue: raw) else {
            throw SchematicFileError.badPCB("\(owner) layer mode \"\(raw)\" must be only or prefer")
        }
        guard !layers.isEmpty else { throw SchematicFileError.badPCB("\(owner) layer rule has no layers") }
        var seen: [CopperLayer] = []
        for value in layers {
            guard let layer = CopperLayer(rawValue: value) else {
                throw SchematicFileError.badPCB("\(owner) layer \"\(value)\" must be top, inner2 to inner5, or bottom")
            }
            if !seen.contains(layer) { seen.append(layer) }
        }
        return LayerRule(mode: mode, layers: seen)
    }

    /// The stored per-net layer rules, one per net.
    static func netOverrides(_ entries: [Board.NetLayerRuleEntry]) throws -> [NetOverride] {
        var result: [NetOverride] = []
        for entry in entries {
            let name = "net layer rule \"\(entry.net)\""
            guard let net = PlaneNet(fileValue: entry.net) else {
                throw SchematicFileError.badPCB("\(name) net must be ground, power:<mV> or pin:<part id>/<pin>")
            }
            guard !result.contains(where: { $0.net == net }) else {
                throw SchematicFileError.badPCB("\(name) is listed twice")
            }
            result.append(NetOverride(net: net, layerRule: try Self.layerRule(mode: entry.mode, layers: entry.layers, name)))
        }
        return result
    }

    /// The stored net groups, each checked (`NetGroup.problem`), with no id
    /// twice and no net in two groups.
    static func netGroups(_ entries: [Board.NetGroupEntry]) throws -> [NetGroup] {
        var seen = Set<UUID>()
        var grouped = Set<PlaneNet>()
        return try entries.map { entry in
            let name = "net group \(entry.id.uuidString)"
            guard seen.insert(entry.id).inserted else { throw SchematicFileError.badPCB("\(name) is listed twice") }
            guard let mode = NetGroupMode(rawValue: entry.mode) else {
                throw SchematicFileError.badPCB("\(name) mode \"\(entry.mode)\" must be lengthMatch or diffPair")
            }
            var group = NetGroup(id: entry.id, name: entry.name, mode: mode)
            group.tolerance = entry.tolerance ?? group.tolerance
            group.gap = entry.gap ?? group.gap
            group.maxUncoupled = entry.maxUncoupled ?? group.maxUncoupled
            group.maxMeander = entry.maxMeander ?? group.maxMeander
            group.members = try entry.members.map { try Self.member($0, name) }
            if let problem = group.problem { throw SchematicFileError.badPCB("\(name) \(problem)") }
            for (raw, net) in zip(entry.members, group.members) {
                guard grouped.insert(net).inserted else {
                    throw SchematicFileError.badPCB("\(name) member \"\(raw)\" is in another group")
                }
            }
            return group
        }
    }

    /// A stored trace, checked against the board's layers.
    static func boardTrace(_ entry: Board.TraceEntry, settings: BoardSettings) throws -> BoardTrace {
        let name = "trace \(entry.id.uuidString)"
        guard let layer = CopperLayer(rawValue: entry.layer) else {
            throw SchematicFileError.badPCB("\(name) layer \"\(entry.layer)\" must be top, inner2 to inner5, or bottom")
        }
        guard settings.copperLayers.contains(layer) else {
            throw SchematicFileError.badPCB("\(name) layer \"\(entry.layer)\" is not on a \(settings.layers)-layer board")
        }
        guard settings.style(of: layer).type != .plane else {
            throw SchematicFileError.badPCB("\(name) layer \(layer.rawValue) is a plane")
        }
        let width = entry.width ?? BoardTrace.defaultWidth
        guard BoardTrace.widthRange.contains(width) else {
            throw SchematicFileError.badPCB("\(name) width \(width) must be 5 to 100 mil")
        }
        let points = entry.points.map { CGPoint(x: $0.x, y: $0.y) }
        if let problem = TraceRoute.problem(points) {
            throw SchematicFileError.badPCB("\(name) \(problem)")
        }
        return BoardTrace(id: entry.id, layer: layer, width: width, points: points, locked: entry.locked ?? false,
                          auto: entry.auto ?? false)
    }

    /// A stored pour, checked against the board's layers and ranges.
    static func boardPour(_ entry: Board.PourEntry, settings: BoardSettings) throws -> BoardPour {
        let name = "pour \(entry.id.uuidString)"
        guard let layer = CopperLayer(rawValue: entry.layer) else {
            throw SchematicFileError.badPCB("\(name) layer \"\(entry.layer)\" must be top, inner2 to inner5, or bottom")
        }
        guard settings.copperLayers.contains(layer) else {
            throw SchematicFileError.badPCB("\(name) layer \"\(entry.layer)\" is not on a \(settings.layers)-layer board")
        }
        guard settings.style(of: layer).type != .plane else {
            throw SchematicFileError.badPCB("\(name) layer \(layer.rawValue) is a plane")
        }
        var pour = BoardPour(id: entry.id, rect: CGRect(x: entry.x, y: entry.y, width: entry.width, height: entry.height),
                             layer: layer)
        if let raw = entry.net {
            guard let net = PlaneNet(fileValue: raw) else {
                throw SchematicFileError.badPCB("\(name) net \"\(raw)\" must be ground, power:<mV> or pin:<part id>/<pin>")
            }
            pour.net = net
        }
        pour.clearance = entry.clearance
        pour.connection = try Self.value(entry.connection, of: PourConnection.self, "\(name) connection", default: .thermal)
        pour.thermalGap = entry.thermalGap ?? pour.thermalGap
        pour.spokeWidth = entry.spokeWidth ?? pour.spokeWidth
        pour.minWidth = entry.minWidth ?? pour.minWidth
        pour.locked = entry.locked ?? false
        if let problem = BoardPour.problem(rect: pour.rect, clearance: pour.clearance, thermalGap: pour.thermalGap,
                                           spokeWidth: pour.spokeWidth, minWidth: pour.minWidth) {
            throw SchematicFileError.badPCB("\(name) \(problem)")
        }
        return pour
    }

    /// A stored keep-out. A layer the board doesn't have is kept and ignored.
    static func boardKeepout(_ entry: Board.KeepoutEntry) throws -> BoardKeepout {
        let name = "keepout \(entry.id.uuidString)"
        var keepout = BoardKeepout(id: entry.id, rect: CGRect(x: entry.x, y: entry.y, width: entry.width, height: entry.height))
        if let names = entry.layers {
            var layers = Set<CopperLayer>()
            for raw in names {
                guard let layer = CopperLayer(rawValue: raw) else {
                    throw SchematicFileError.badPCB("\(name) layer \"\(raw)\" must be top, inner2 to inner5, or bottom")
                }
                layers.insert(layer)
            }
            keepout.layers = layers
        }
        keepout.locked = entry.locked ?? false
        if let problem = BoardKeepout.problem(rect: keepout.rect) {
            throw SchematicFileError.badPCB("\(name) \(problem)")
        }
        return keepout
    }

    /// A stored cutout (plan-65). A polygon is kept clockwise.
    static func boardCutout(_ entry: Board.CutoutEntry) throws -> BoardCutout {
        let name = "cutout \(entry.id.uuidString)"
        let shape: BoardCutout.Shape
        switch entry.shape {
        case "rounded":
            guard let x = entry.x, let y = entry.y, let width = entry.width, let height = entry.height else {
                throw SchematicFileError.badPCB("\(name) needs x, y, width and height")
            }
            shape = .rounded(rect: CGRect(x: x, y: y, width: width, height: height), radius: entry.radius ?? 0)
        case "polygon":
            guard let points = entry.points else { throw SchematicFileError.badPCB("\(name) needs points") }
            shape = .polygon(points.map { CGPoint(x: $0.x, y: $0.y) })
        default:
            throw SchematicFileError.badPCB("\(name) shape \"\(entry.shape)\" must be rounded or polygon")
        }
        if let problem = BoardCutout.problem(shape) { throw SchematicFileError.badPCB("\(name) \(problem)") }
        var cutout = BoardCutout(id: entry.id, shape: shape)
        if case let .polygon(points) = shape { cutout.shape = .polygon(BoardGeometry.clockwise(points)) }
        cutout.locked = entry.locked ?? false
        return cutout
    }

    /// A stored footprint placement, with defaults for missing keys.
    func placement(_ pcb: Placement, reference: String) throws -> BoardPlacement {
        var placement = BoardPlacement(centre: CGPoint(x: pcb.x, y: pcb.y), rotation: pcb.rotation)
        // A placement saved without an origin is from before origins: those
        // were all top left. New placements default to pad 1's centre.
        placement.origin = try Self.value(pcb.origin, of: FootprintOrigin.self, "\(reference) origin",
                                          default: .topLeft)
        placement.outline = try Self.value(pcb.outline, of: MarkLayer.self, "\(reference) outline",
                                           default: placement.outline)
        placement.pin1 = try Self.value(pcb.pin1, of: MarkLayer.self, "\(reference) pin1", default: placement.pin1)
        placement.reference = try Self.value(pcb.reference, of: MarkLayer.self, "\(reference) reference",
                                             default: placement.reference)
        placement.side = try Self.value(pcb.side, of: BoardSide.self, "\(reference) side", default: placement.side)
        placement.locked = pcb.locked ?? false
        let limit = BoardSettings.sizeRange.upperBound
        for (key, value) in [("refX", pcb.refX), ("refY", pcb.refY)] {
            if let value, !(abs(value) <= limit) {
                throw SchematicFileError.badPCB("\(reference) \(key) \(value) must be within \(Int(limit)) mm")
            }
        }
        placement.referenceOffset = CGPoint(x: pcb.refX ?? 0, y: pcb.refY ?? 0)
        return placement
    }

    /// A stored board text, with defaults for missing keys. Its anchor is
    /// snapped to the component grid (`grid`, mm).
    func boardText(_ text: Board.Text, grid: Double) throws -> BoardText {
        let name = "text \(text.id.uuidString)"
        guard !text.text.isEmpty else {
            throw SchematicFileError.badPCB("\(name) is empty")
        }
        var result = BoardText(id: text.id, text: text.text, anchor: BoardGrid.snap(CGPoint(x: text.x, y: text.y), spacing: grid),
                               size: BoardText.defaultSize)
        if let size = text.size {
            guard BoardText.sizeRange.contains(size) else {
                throw SchematicFileError.badPCB("\(name) size \(size) must be 10 to 1000 mil")
            }
            result.size = size
        }
        result.layer = try Self.value(text.layer, of: TextLayer.self, "\(name) layer", default: result.layer)
        result.side = try Self.value(text.side, of: BoardSide.self, "\(name) side", default: result.side)
        result.locked = text.locked ?? false
        if let rotation = text.rotation {
            guard BoardPlacement.rotations.contains(rotation) else {
                throw SchematicFileError.badPCB("\(name) rotation \(rotation) must be 0, 90, 180 or 270")
            }
            result.rotation = rotation
        }
        return result
    }

    /// The case whose raw value is `raw`, `fallback` when there is none, or
    /// a `badPCB` error listing the allowed values.
}

extension SchematicFile {
    // MARK: Card settings (the board's, and a bus's)

    static func entry(_ card: CustomCard) -> Board.CardEntry {
        Board.CardEntry(pads: card.pads, pitch: card.pitch, padWidth: card.padWidth, padLength: card.padLength,
                        padOffset: card.padOffset, depth: card.depth, indent: card.indent, margin: card.margin,
                        chamfer: card.chamfer, bothSides: card.bothSides)
    }

    /// The type's settings as written: every option's value (only the
    /// `onBus` ones for a bus), defaults included.
    static func entry(_ options: CardOptions, of type: BoardType, bus: Bool) -> [String: BoardValue] {
        guard let definition = type.definition else { return [:] }
        let values = definition.resolved(options)
        return values.filter { id, _ in !bus || definition.options.contains { $0.id == id && $0.onBus } }
    }

    /// A stored custom card edge; missing keys are the defaults, and
    /// out-of-range settings are brought into range, as the sidebar does.
    static func customCard(_ entry: Board.CardEntry) -> CustomCard {
        var card = CustomCard()
        card.pads = entry.pads ?? card.pads
        card.pitch = entry.pitch ?? card.pitch
        card.padWidth = entry.padWidth ?? card.padWidth
        card.padLength = entry.padLength ?? card.padLength
        card.padOffset = entry.padOffset ?? card.padOffset
        card.depth = entry.depth ?? card.depth
        card.indent = entry.indent ?? card.indent
        card.margin = entry.margin ?? card.margin
        card.chamfer = entry.chamfer ?? card.chamfer
        card.bothSides = entry.bothSides ?? card.bothSides
        return card.clamped()
    }

    /// Stored settings of `type`; missing keys are the defaults, and keys
    /// that aren't its options are ignored. Throws `badPCB` for a value
    /// that isn't one of its option's choices.
    static func cardOptions(_ entry: [String: BoardValue], of type: BoardType) throws -> CardOptions {
        var options = CardOptions()
        for option in type.definition?.options ?? [] {
            guard let value = entry[option.id] else { continue }
            guard option.choice(for: value) != nil else {
                throw SchematicFileError.badPCB("card \(option.id) \(value.quoted) must be \(option.choiceList)")
            }
            options[option.id] = value
        }
        return options
    }

    static func value<Choice: RawRepresentable & CaseIterable>(
        _ raw: String?, of type: Choice.Type, _ name: String, default fallback: Choice
    ) throws -> Choice where Choice.RawValue == String {
        guard let raw else { return fallback }
        if let value = Choice(rawValue: raw) { return value }
        let names = Choice.allCases.map(\.rawValue)
        let list = names.count == 2
            ? "\(names[0]) or \(names[1])"
            : names.dropLast().joined(separator: ", ") + " or " + names.last!
        throw SchematicFileError.badPCB("\(name) \"\(raw)\" must be \(list)")
    }

    /// A number as written in a message: 0 or 1000.5, not 0.0.
    static func number(_ value: Double) -> String {
        value == value.rounded() && abs(value) < 1e15 ? String(Int(value)) : String(value)
    }
}

// MARK: Bus values

extension SchematicFile {
    /// A file bus as a model bus, checked: wire count and names, straight
    /// lines on the grid, no overlaps, only T and L joints, one piece.
    @MainActor
    static func modelBus(_ bus: Bus) throws -> SchematicModelBus {
        let id = bus.id.uuidString
        let lines = bus.lines.map {
            BusLine(id: $0.id, a: CGPoint(x: $0.a.x, y: $0.a.y), b: CGPoint(x: $0.b.x, y: $0.b.y))
        }
        var model = SchematicModelBus(
            id: bus.id, lines: lines, wires: bus.wires.map { BusWire(id: $0.id, name: $0.name) })
        model.name = bus.name?.trimmingCharacters(in: .whitespacesAndNewlines) ?? ""
        // Its type, kept only while the wires are still exactly the card's pins.
        do {
            var card = BusCard()
            card.type = try Self.value(bus.type, of: BoardType.self, "type", default: .custom)
            if let entry = bus.card { card.customCard = Self.customCard(entry) }
            if let entry = bus.cardOptions { card.cardOptions = try Self.cardOptions(entry, of: card.type) }
            if card.pinNames == nil || card.pinNames == model.wires.map(\.name) { model.card = card }
        } catch let SchematicFileError.badPCB(detail) {
            throw SchematicFileError.badBus("bus \(id) \(detail)")
        }

        switch model.problem {
        case nil:
            break
        case .noLines?:
            throw SchematicFileError.badBus("bus \(id) has no lines")
        case let .notStraight(line)?:
            throw SchematicFileError.badBus("line \(line.uuidString) is not a horizontal or vertical line on the grid")
        case let .overlap(first, second)?:
            throw SchematicFileError.badBus("lines \(first.uuidString) and \(second.uuidString) overlap")
        case let .badJoint(_, point)?:
            throw SchematicFileError.badBus("bus \(id) has a joint at \(pointText(point)) that is not a T or an L")
        case .notOnePiece?:
            throw SchematicFileError.badBus("bus \(id) is not one piece")
        }

        guard SchematicModelBus.wireCountRange.contains(bus.wires.count) else {
            throw SchematicFileError.badBus("bus \(id) has \(bus.wires.count) wires; it must have 2 to 999")
        }
        for wire in bus.wires {
            guard SchematicModelBus.nameLengthRange.contains(wire.name.count),
                  !wire.name.contains(where: \.isWhitespace) else {
                throw SchematicFileError.badBus("bus \(id) wire name \"\(wire.name)\" must be 1 to \(SchematicModelBus.nameLengthRange.upperBound) characters with no spaces")
            }
        }

        // Ports: on a line of this bus, a side that suits the line, wires of
        // this bus, and fitting where they are.
        var withPorts = model
        let wireIDs = Set(model.wires.map(\.id))
        for entry in bus.ports ?? [] {
            let portID = entry.id.uuidString
            guard let line = model.line(entry.line) else {
                throw SchematicFileError.badBus("port \(portID) line \(entry.line.uuidString) is not on bus \(id)")
            }
            guard let side = sideValue(entry.side), BusPort.sides(for: line).contains(side) else {
                let allowed = line.isHorizontal ? "top or bottom on a horizontal line" : "left or right on a vertical line"
                throw SchematicFileError.badBus("port \(portID) side \"\(entry.side)\" must be \(allowed)")
            }
            for wire in entry.wires where !wireIDs.contains(wire) {
                throw SchematicFileError.badBus("port \(portID) wire \(wire.uuidString) is not on bus \(id)")
            }
            withPorts.ports.append(BusPort(id: entry.id, line: entry.line, start: CGFloat(entry.start),
                                           side: side, wireIDs: Set(entry.wires)))
        }
        for port in withPorts.ports
            where withPorts.fit(port: port, slots: withPorts.slots(of: port), preferredStart: port.start) != port.start {
            throw SchematicFileError.badBus("port \(port.id.uuidString) does not fit on its line")
        }
        return withPorts
    }

    static func sideName(_ side: ChipLayout.Side) -> String {
        switch side {
        case .top: "top"
        case .bottom: "bottom"
        case .left: "left"
        case .right: "right"
        }
    }

    static func sideValue(_ name: String) -> ChipLayout.Side? {
        switch name {
        case "top": .top
        case "bottom": .bottom
        case "left": .left
        case "right": .right
        default: nil
        }
    }

    /// A point as written in a message: (100, 200).
    static func pointText(_ point: CGPoint) -> String {
        "(\(number(Double(point.x))), \(number(Double(point.y))))"
    }
}

/// The model's types, named apart from the file's nested types of the same name.
typealias SchematicModelWire = Wire
typealias SchematicModelJunction = Junction
typealias SchematicModelBus = Bus

extension SchematicFile.End {
    /// Saves a pin by its designator, looked up on the placed chip.
    init(_ end: WireEnd, chips: [PlacedChip]) {
        switch end {
        case let .pin(ref):
            let pins = chips.first { $0.id == ref.chipID }?.part.pins ?? []
            let designator = pins.indices.contains(ref.index) ? pins[ref.index].designator : "\(ref.index + 1)"
            self = .pin(part: ref.chipID, pin: .designator(designator))
        case let .junction(id):
            self = .junction(id)
        case let .busPin(ref):
            self = .port(port: ref.portID, wire: ref.wireID)
        case let .label(id):
            self = .label(id)
        }
    }
}
