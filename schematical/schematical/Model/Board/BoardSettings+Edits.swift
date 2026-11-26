import CoreGraphics
import Foundation

/// The board's own edits (plan-68-E): size and type, outline, grid, units
/// and colour, layers, stackup and design rules. The document records each
/// as one step; what a refusal needs from the rest of the board comes in as
/// parameters.
extension BoardSettings {
    // MARK: Board

    /// Sets the size in mm, each side clamped to `sizeRange`; a card keeps
    /// its standard's height, a custom card at least its tab. A free-form
    /// outline fixes the size.
    mutating func setSize(width: Double, height: Double) {
        if isFreeForm { return }
        let range = Self.sizeRange
        if type == .customCard {
            let card = customCard.clamped()
            self.width = min(max(width, card.indent + card.tabLength), range.upperBound)
            self.height = min(max(height, card.depth + 1), range.upperBound)
            return
        }
        if let card {
            self.width = min(max(width, card.minimumLength), card.maximumLength)
            self.height = card.height
            return
        }
        self.width = min(max(width, range.lowerBound), range.upperBound)
        self.height = min(max(height, range.lowerBound), range.upperBound)
    }

    /// Sets the card settings and fits the board to them: the standard's
    /// height, and its length unless the card's length is free.
    mutating func setCardOptions(_ options: CardOptions) {
        guard cardOptions != options else { return }
        cardOptions = options
        guard type.hasOptions, let card else { return }
        height = card.height
        width = card.isFixedSize ? card.length : min(max(width, card.minimumLength), card.maximumLength)
    }

    /// Sets the custom card's edge connector (clamped to fit), and grows the
    /// board to hold its tab.
    mutating func setCustomCard(_ card: CustomCard) {
        let value = card.clamped()
        if customCard != value { customCard = value }
        guard type == .customCard else { return }
        width = max(width, value.indent + value.tabLength)
        height = max(height, value.depth + 1)
    }

    /// Makes the board a custom rectangle, a custom card or a standard card
    /// (see `SchematicDocument.setBoardType`).
    mutating func setType(_ type: BoardType) {
        guard self.type != type else { return }
        self.type = type
        // A free-form outline belongs to the Custom type alone.
        customOutline = nil
        if type != .customCard, let card {
            width = card.length
            height = card.height
        }
        if type == .customCard {
            let card = customCard.clamped()
            width = max(width, card.indent + card.tabLength)
            height = max(height, card.depth + 1)
        }
        if needsTwoLayers, layers < 2 { layers = 2 }
    }

    /// Makes the board free-form Custom with `points` (already checked by
    /// `problem(outline:)`), stored clockwise, its size the outline's bounds.
    mutating func setOutline(_ points: [CGPoint]) {
        let outline = BoardGeometry.clockwise(points)
        let bounds = Self.bounds(of: outline)
        type = .custom
        customOutline = outline
        width = bounds.width
        height = bounds.height
    }

    mutating func resetOutline() {
        if customOutline != nil { customOutline = nil }
    }

    /// Sets the component grid (mm), clamped to `gridRange`.
    mutating func setGrid(_ mm: Double) {
        let value = min(max(mm, Self.gridRange.lowerBound), Self.gridRange.upperBound)
        if grid != value { grid = value }
    }

    // MARK: Layers

    /// Why no trace can go on `layer`, or nil when one can.
    func traceLayerRefusal(_ layer: CopperLayer) -> String? {
        guard copperLayers.contains(layer) else { return "\(layer.title) is not on this board." }
        guard style(of: layer).type != .plane else { return "\(layer.title) is a plane." }
        return nil
    }

    /// Why the board can't have `count` copper layers, or nil when it can:
    /// a count that would drop a layer holding something. `bottomItems`:
    /// a part or text is on Bottom; `traceLayers`/`pourLayers`: the layers
    /// traces and pours are on.
    func layerCountProblem(_ count: Int, bottomItems: Bool, traceLayers: Set<CopperLayer>,
                           pourLayers: Set<CopperLayer>, hasVias: Bool) -> String? {
        guard Self.layerCounts.contains(count) else { return "Boards have 1, 2, 4 or 6 layers." }
        let dropped = Set(copperLayers).subtracting(CopperLayer.allCases(for: count))
        if dropped.contains(.bottom), bottomItems { return "Move every part and text to Top first." }
        if let layer = copperLayers.first(where: { dropped.contains($0) && traceLayers.contains($0) }) {
            return "Delete or move the traces on \(layer.title) first."
        }
        if let layer = copperLayers.first(where: { dropped.contains($0) && pourLayers.contains($0) }) {
            return "Delete or move the pours on \(layer.title) first."
        }
        if count == 1, hasVias { return "Delete the vias first." }
        if count == 1, needsTwoLayers { return "A card has fingers on both sides." }
        return nil
    }

    /// Why `layer` can't become `type`, or nil when it can. `holdsCopper`:
    /// the layer has traces or pours (which, refused, says which).
    func layerTypeProblem(_ layer: CopperLayer, _ type: LayerType, hasTraces: Bool, hasPours: Bool) -> String? {
        guard copperLayers.contains(layer) else { return "\(layer.title) is not on this board." }
        guard layer.isInner else { return "\(layer.title) is always a component layer." }
        guard type != .component else { return "Inner layers are planes or wires." }
        if type == .plane, hasTraces { return "Move or delete this layer's traces first." }
        if type == .plane, hasPours { return "Move or delete this layer's pours first." }
        return nil
    }

    /// Changes one field of a layer's style; an inner layer only when `inner`.
    private mutating func changeStyle(_ layer: CopperLayer, inner: Bool, _ change: (inout LayerStyle) -> Void) {
        guard copperLayers.contains(layer), !inner || layer.isInner else { return }
        var style = style(of: layer)
        change(&style)
        if style != self.style(of: layer) { styles[layer] = style }
    }

    mutating func setLayerColor(_ layer: CopperLayer, _ color: LayerColor) {
        changeStyle(layer, inner: false) { $0.color = color }
    }

    mutating func setLayerType(_ layer: CopperLayer, _ type: LayerType) {
        changeStyle(layer, inner: false) { $0.type = type }
    }

    mutating func setLayerNet(_ layer: CopperLayer, _ net: PlaneNet?) {
        changeStyle(layer, inner: true) { $0.net = net }
    }

    /// A Plane layer's clearance (mm), clamped to `LayerStyle.clearanceRange`.
    mutating func setLayerClearance(_ layer: CopperLayer, _ mm: Double?) {
        let value = mm.map { min(max($0, LayerStyle.clearanceRange.lowerBound), LayerStyle.clearanceRange.upperBound) }
        changeStyle(layer, inner: true) { $0.clearance = value }
    }

    // MARK: Stackup

    mutating func setTemperatureRise(_ degrees: Double) {
        let value = min(max(degrees, BoardStackup.temperatureRiseRange.lowerBound),
                        BoardStackup.temperatureRiseRange.upperBound)
        if stackup.temperatureRise != value { stackup.temperatureRise = value }
    }

    /// The copper layer at `index` (Top is 0), clamped to `BoardStackup.copperRange`.
    mutating func setCopperThickness(_ mm: Double, at index: Int) {
        guard stackup.copper.indices.contains(index) else { return }
        let value = min(max(mm, BoardStackup.copperRange.lowerBound), BoardStackup.copperRange.upperBound)
        if stackup.copper[index] != value { stackup.copper[index] = value }
    }

    /// Any of a dielectric's kind, thickness (mm) and Er, each clamped.
    mutating func setDielectric(at index: Int, kind: DielectricKind?, thickness: Double?, er: Double?) {
        guard stackup.dielectrics.indices.contains(index) else { return }
        var dielectric = stackup.dielectrics[index]
        if let kind { dielectric.kind = kind }
        if let thickness {
            dielectric.thickness = min(max(thickness, BoardStackup.thicknessRange.lowerBound),
                                       BoardStackup.thicknessRange.upperBound)
        }
        if let er { dielectric.er = min(max(er, BoardStackup.erRange.lowerBound), BoardStackup.erRange.upperBound) }
        if stackup.dielectrics[index] != dielectric { stackup.dielectrics[index] = dielectric }
    }

    /// The standard stackup for the layer count, nothing of the old one kept.
    mutating func resetStackup() {
        let standard = BoardStackup.standard(layers: layers)
        if stackup != standard { stackup = standard }
    }

    // MARK: Design rules

    /// Sets the rules (each 0 or more; a value that is not finite is 0) and
    /// the manufacturer they are named after.
    mutating func setDesignRules(_ rules: DesignRules, manufacturer: DRCManufacturer) {
        var next = rules
        for rule in DesignRules.groups.flatMap(\.rules) {
            let value = next[keyPath: rule.field]
            next[keyPath: rule.field] = value.isFinite ? max(0, value) : 0
        }
        if self.rules != next { self.rules = next }
        if rulesManufacturer != manufacturer { rulesManufacturer = manufacturer }
    }
}
