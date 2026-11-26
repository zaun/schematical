import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Board

    /// Sets the board's size in mm, each side clamped to `BoardSettings.sizeRange`.
    func setBoardSize(width: Double, height: Double) {
        editBoard("Change Board Size") { $0.setSize(width: width, height: height) }
    }

    /// Runs `change` on the board's settings as one undo step named `name`.
    func editBoard(_ name: String, _ change: (inout BoardSettings) -> Void) {
        edit(.pcb, name) { scope in layout.updateSettings(scope, change) }
    }

    /// Sets the card settings (lanes, length, key, size, memory) and fits
    /// the board to them: the standard's height, and its length unless the
    /// card's length is free (PCI Express), when it stays within range.
    func setCardOptions(_ options: CardOptions) {
        editBoard("Change Card") { $0.setCardOptions(options) }
    }

    /// Sets the custom card's edge connector (clamped to fit), and grows
    /// the board to hold its tab.
    func setCustomCard(_ card: CustomCard) {
        editBoard("Change Card Edge") { $0.setCustomCard(card) }
    }

    /// Makes the board a custom rectangle, a custom card (keeping its size,
    /// grown to fit the tab) or a standard card. A card takes
    /// its standard's height and length (the length stays editable), and
    /// at least 2 layers when it has fingers, which are on both sides. Back to
    /// Custom keeps the size. Nothing on the board moves.
    func setBoardType(_ type: BoardType) {
        editBoard("Change Board Type") { $0.setType(type) }
    }

    /// Makes the board a free-form Custom board with this outline (plan-65):
    /// its bounds' top-left at (0, 0), no crossing, each side 1 to 1000 mm. It
    /// is stored clockwise and sets the board's width and height to its
    /// bounds; parts, cutouts and everything else stay where they are.
    /// False, with no undo step, when `BoardSettings.problem(outline:)` refuses it.
    @discardableResult
    func setBoardOutline(_ points: [CGPoint]) -> Bool {
        guard BoardSettings.problem(outline: points) == nil else { return false }
        editBoard("Change Board Outline") { $0.setOutline(points) }
        return true
    }

    /// File ▸ Import ▸ DXF… (plan-65-I), one undo step named `Import DXF`. *Outline and
    /// cutouts*: the board takes the outline (a free-form Custom board), every cutout it had
    /// is replaced by the DXF's, and nothing else moves. *Cutouts only*: the DXF's loops are added.
    /// The new cutouts are the selection. Does nothing, with no step, when the outline is
    /// refused (`BoardSettings.problem(outline:)`) or there is nothing to import.
    func applyDXFImport(_ placed: DXFPlacedImport, mode: DXFImportMode) {
        edit(.pcb, "Import DXF") { scope in
            if mode == .outlineAndCutouts {
                guard let outline = placed.outline, setBoardOutline(outline) else { return }
                layout.updateFeatures(scope) { $0.cutouts.removeAll() }
            }
            guard mode == .outlineAndCutouts || !placed.cutouts.isEmpty else { return }
            var added: [BoardItem] = []
            for loop in placed.cutouts {
                if let id = addCutout(.polygon(loop)) { added.append(.cutout(id)) }
            }
            boardSelection.set(Set(added))
            pruneSelection()
        }
    }

    /// Puts the rectangle back: the free-form outline is dropped and the
    /// board keeps its width and height (the old bounds).
    func resetBoardOutline() {
        editBoard("Reset Board Outline") { $0.resetOutline() }
    }

    /// Sets the component grid (mm), clamped to `BoardSettings.gridRange`.
    /// The trace grid follows it. Nothing on the board moves.
    func setBoardGrid(_ mm: Double) {
        editBoard("Change Grid") { $0.setGrid(mm) }
    }

    /// The units the PCB sidebar shows sizes in.
    func setBoardUnits(_ units: BoardUnits) {
        editBoard("Change Units") { $0.units = units }
    }

    /// The board's solder-mask colour.
    func setBoardColor(_ color: BoardColor) {
        editBoard("Change Board Color") { $0.color = color }
    }

    // MARK: Board layers

    /// Why the board can't have `count` copper layers, or nil when it can.
    /// A count is refused when it would drop a layer that holds something.
    func canSetLayerCount(_ count: Int) -> String? {
        board.layerCountProblem(count, bottomItems: chips.contains { $0.board?.side == .bottom }
                                    || texts.contains { $0.side == .bottom },
                                traceLayers: Set(traces.map(\.layer)), pourLayers: Set(pours.map(\.layer)),
                                hasVias: !vias.isEmpty)
    }

    /// Sets the copper layer count, unless `canSetLayerCount` refuses it.
    func setLayerCount(_ count: Int) {
        guard canSetLayerCount(count) == nil else { return }
        editBoard("Change Layers") { $0.layers = count }
    }

    // MARK: Stackup (PCB)

    func setStackupFinish(_ finish: SurfaceFinish) {
        editBoard("Change Stackup") { $0.stackup.finish = finish }
    }

    /// Sets the temperature rise the current estimate allows, clamped to
    /// `BoardStackup.temperatureRiseRange`.
    func setStackupTemperatureRise(_ degrees: Double) {
        editBoard("Change Stackup") { $0.setTemperatureRise(degrees) }
    }

    /// Sets the thickness (mm) of the copper layer at `index` (Top is 0),
    /// clamped to `BoardStackup.copperRange`.
    func setCopperThickness(_ mm: Double, at index: Int) {
        editBoard("Change Stackup") { $0.setCopperThickness(mm, at: index) }
    }

    /// Changes the dielectric at `index` (the one below copper layer
    /// `index`): any of its kind, thickness (mm) and Er; values are clamped
    /// to `BoardStackup.thicknessRange` and `erRange`.
    func setDielectric(at index: Int, kind: DielectricKind? = nil, thickness: Double? = nil, er: Double? = nil) {
        editBoard("Change Stackup") { $0.setDielectric(at: index, kind: kind, thickness: thickness, er: er) }
    }

    /// Puts the board's standard stackup for its layer count back, keeping
    /// nothing of the old copper and dielectrics, the finish or the rise.
    func resetStackup() {
        editBoard("Reset Stackup") { $0.resetStackup() }
    }

    func setLayerColor(_ layer: CopperLayer, _ color: LayerColor) {
        editBoard("Change Layer Color") { $0.setLayerColor(layer, color) }
    }

    /// Why `layer` can't become `type`, or nil when it can. Top and Bottom
    /// are always Component; an inner layer is Plane or Wire.
    func canSetLayerType(_ layer: CopperLayer, _ type: LayerType) -> String? {
        board.layerTypeProblem(layer, type, hasTraces: traces.contains { $0.layer == layer },
                               hasPours: pours.contains { $0.layer == layer })
    }

    func setLayerType(_ layer: CopperLayer, _ type: LayerType) {
        guard canSetLayerType(layer, type) == nil else { return }
        editBoard("Change Layer Type") { $0.setLayerType(layer, type) }
    }

    /// Sets a Plane layer's net; nil leaves it on no net.
    func setLayerNet(_ layer: CopperLayer, _ net: PlaneNet?) {
        editBoard("Change Plane Net") { $0.setLayerNet(layer, net) }
    }

    /// Sets a Plane layer's clearance (mm), clamped to
    /// `LayerStyle.clearanceRange`; nil goes back to the DRC's minimum.
    func setLayerClearance(_ layer: CopperLayer, _ mm: Double?) {
        editBoard("Change Plane Clearance") { $0.setLayerClearance(layer, mm) }
    }

    /// The nets a Plane can carry, as its Net picker lists them: ground,
    /// then each power rail from low to high (every supply symbol and pull
    /// resistor of one voltage is one rail), then each signal net the
    /// schematic wires, named by up to three of its pins ("U1.3, R2.1, …")
    /// and stored by its first.
    func planeNetChoices() -> [(net: PlaneNet, title: String)] {
        let netlist = NetlistBuilder.build(chips: chips, wires: wires, buses: buses, labels: netLabels)
        var rails = Set<UInt32>()
        var railNets = Set<Int>()
        for chip in chips {
            if case let .supply(_, mv, _) = chip.part.details { rails.insert(mv) }
        }
        for connection in netlist.connections {
            if case .supply = chips[connection.part].part.details { railNets.insert(connection.net) }
        }
        var choices: [(net: PlaneNet, title: String)] = rails.sorted().map { mv in
            (PlaneNet.rail(mv: mv), Self.planeNetTitle(rail: mv))
        }
        var pinsOfNet: [Int: [(chip: PlacedChip, designator: String)]] = [:]
        for connection in netlist.connections where !railNets.contains(connection.net) {
            let chip = chips[connection.part]
            pinsOfNet[connection.net, default: []].append((chip, connection.pin))
        }
        typealias NetPin = (chip: PlacedChip, designator: String)
        func before(_ a: NetPin, _ b: NetPin) -> Bool {
            let byPart = a.chip.reference.localizedStandardCompare(b.chip.reference)
            if byPart != .orderedSame { return byPart == .orderedAscending }
            return a.designator.localizedStandardCompare(b.designator) == .orderedAscending
        }
        let signals = pinsOfNet.values.map { $0.sorted(by: before) }
            .filter { $0.count > 1 }
            .sorted { before($0[0], $1[0]) }
        for pins in signals {
            var title = pins.prefix(3).map { "\($0.chip.reference).\($0.designator)" }.joined(separator: ", ")
            if pins.count > 3 { title += ", …" }
            choices.append((.pin(chip: pins[0].chip.id, designator: pins[0].designator), title))
        }
        return choices
    }

    /// "GND" for the 0 mV rail, else its voltage: "5 V", "3.3 V".
    nonisolated static func planeNetTitle(rail mv: UInt32) -> String {
        mv == 0 ? "GND" : "\(VoltageCheck.volts(mv)) V"
    }

    // MARK: Design rules (PCB)

    /// Sets the board's design rules (each 0 or more) and the manufacturer
    /// they are named after. One undo step.
    func setDesignRules(_ rules: DesignRules, manufacturer: DRCManufacturer) {
        editBoard("Change Design Rules") { $0.setDesignRules(rules, manufacturer: manufacturer) }
    }

    // MARK: Net classes (PCB)

    /// Adds a net class named `name` (trimmed, at most 40 characters; blank
    /// is "Class") with the Default class's values and no nets, last in
    /// priority.
    @discardableResult
    func addNetClass(named name: String) -> UUID {
        edit(.pcb, "Add Net Class") { scope in layout.updateSettings(scope) { $0.addNetClass(named: name) } }
    }

    func removeNetClass(_ id: UUID) {
        editBoard("Delete Net Class") { $0.removeNetClass(id) }
    }

    /// Renames a class; a blank name is ignored, a long one cut to 40
    /// characters.
    func renameNetClass(_ id: UUID, to name: String) {
        editBoard("Rename Net Class") { $0.renameNetClass(id, to: name) }
    }

    /// Sets a class's trace width, clamped to `BoardTrace.widthRange`.
    func setNetClassWidth(_ id: UUID, _ mil: Int) {
        editBoard("Change Net Class Width") { $0.setNetClassWidth(id, mil) }
    }

    /// Sets a class's clearance, clamped to `LayerStyle.clearanceRange`;
    /// nil is the DRC's minimum.
    func setNetClassClearance(_ id: UUID, _ mm: Double?) {
        editBoard("Change Net Class Clearance") { $0.setNetClassClearance(id, mm) }
    }

    /// Sets a class's via, clamped as `setViaDrill` and `setViaDiameter`
    /// clamp a via: the hole to `BoardVia.drillRange`, then the metal to
    /// `BoardVia.diameterRange` and at least the thinnest ring round it.
    func setNetClassVia(_ id: UUID, drill: Double, diameter: Double) {
        editBoard("Change Net Class Via") { $0.setNetClassVia(id, drill: drill, diameter: diameter) }
    }

    /// Puts `net` in class `classID`, out of every other class; nil puts it
    /// back in the Default class. The copper already on the net takes the
    /// class's rules: every trace its width, every via its hole and metal.
    /// One undo step.
    func assignNet(_ net: PlaneNet, to classID: UUID?) {
        edit(.pcb, "Change Net Class") { scope in
            layout.updateSettings(scope) { $0.assignNet(net, toClass: classID) }
            // Copper drawn before the net changed class matches new copper;
            // the class's clearance is read by the checks.
            let copper = connectivity()
            guard let netID = copper.net(of: net) else { return }
            let rules = board.netRules(ofNet: netID, in: copper)
            let items = copperItems(ofNet: netID, in: copper)
            layout.updateCopper(scope) { $0.apply(rules, traces: items.traces, vias: items.vias) }
        }
    }

    /// Moves a class to `position` in the list (clamped); order is
    /// priority.
    func moveNetClass(_ id: UUID, to position: Int) {
        editBoard("Move Net Class") { $0.moveNetClass(id, to: position) }
    }

    /// The width and via new copper at `point` gets: those of the class of
    /// the net there — a pad's whose copper holds the point on `layer`
    /// (a through-hole pad on any layer; nil `layer` is any), else a via's
    /// whose metal holds it, else a trace's on `layer` whose copper holds
    /// it — or the Default class's when it is on no net or a net in no class.
    func netRules(at point: CGPoint, layer: CopperLayer?) -> NetRules {
        let copper = connectivity()
        return board.netRules(ofNet: net(at: point, layer: layer, in: copper), in: copper)
    }

    /// The net (of `copper`) whose copper holds `point`, by the order
    /// `netRules(at:layer:)` describes: a plated pad's on `layer`, else a
    /// via's, else a trace's on `layer`; nil when none does.
    func net(at point: CGPoint, layer: CopperLayer?, in copper: BoardConnectivity) -> Int? {
        func onLayer(_ pad: BoardConnectivity.Pad) -> Bool {
            layer == nil || pad.pad.drill > 0 || pad.side.copperLayer == layer
        }
        if let p = copper.pads.indices.first(where: {
            copper.pads[$0].pad.plated && onLayer(copper.pads[$0]) && copper.pads[$0].pad.rect.contains(point)
                && copper.netOfPad[$0] != nil
        }) {
            return copper.netOfPad[p]
        }
        let netOfRoot = copper.netOfRoot()
        if let v = copper.vias.indices.first(where: {
            Self.distance(from: point, to: copper.vias[$0].centre) <= copper.vias[$0].diameter / 2
                && netOfRoot[copper.root[copper.viaNode($0)]] != nil
        }) {
            return netOfRoot[copper.root[copper.viaNode(v)]]
        }
        if let t = copper.traces.indices.first(where: { t in
            let trace = copper.traces[t]
            return (layer == nil || trace.layer == layer) && netOfRoot[copper.root[copper.traceNode(t)]] != nil
                && trace.segments.contains { Self.distance(from: point, toSegment: $0.0, $0.1) <= trace.widthMM / 2 }
        }) {
            return netOfRoot[copper.root[copper.traceNode(t)]]
        }
        return nil
    }

    /// The class net `net` of `connectivity` is in: the first, in list
    /// order, any of whose members names it; nil is the Default class.
    func netClass(ofNet net: Int, in connectivity: BoardConnectivity) -> NetClass? {
        board.netClass(ofNet: net, in: connectivity)
    }

    // MARK: Net layer rules and groups (PCB)

    /// Sets a class's layer rule; nil is any trace layer. A rule with no
    /// layers is ignored.
    func setNetClassLayerRule(_ id: UUID, _ rule: LayerRule?) {
        editBoard("Change Layer Rule") { $0.setNetClassLayerRule(id, rule) }
    }

    /// Sets one net's own layer rule, before its class's; nil removes it.
    /// A rule with no layers is ignored.
    func setNetLayerRule(_ net: PlaneNet, _ rule: LayerRule?) {
        editBoard("Change Layer Rule") { $0.setNetLayerRule(net, rule) }
    }

    /// Adds an empty group (trimmed name, at most 40 characters; blank is
    /// "Group"), last.
    @discardableResult
    func addNetGroup(named name: String, mode: NetGroupMode) -> UUID {
        edit(.pcb, "Add Net Group") { scope in layout.updateSettings(scope) { $0.addNetGroup(named: name, mode: mode) } }
    }

    func removeNetGroup(_ id: UUID) {
        editBoard("Delete Net Group") { $0.removeNetGroup(id) }
    }

    func renameNetGroup(_ id: UUID, to name: String) {
        editBoard("Rename Net Group") { $0.renameNetGroup(id, to: name) }
    }

    /// Changes a group's mode. A group with more than 2 nets can't be a
    /// pair: refused, with `editorMessage` saying why. Returns whether the
    /// group has `mode` now.
    @discardableResult
    func setNetGroupMode(_ id: UUID, _ mode: NetGroupMode) -> Bool {
        guard board.netGroupIndex(id) != nil else { return false }
        if let problem = board.netGroupModeProblem(id, mode) {
            editorMessage = problem
            return false
        }
        editBoard("Change Net Group") { $0.setNetGroupMode(id, mode) }
        return true
    }

    /// Sets a group's tolerance, clamped to `NetGroup.toleranceRange`.
    func setNetGroupTolerance(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.tolerance, mm, NetGroup.toleranceRange)
    }

    /// Sets a pair's gap, clamped to `NetGroup.gapRange`.
    func setNetGroupGap(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.gap, mm, NetGroup.gapRange)
    }

    /// Sets how much of a pair may run apart, clamped to
    /// `NetGroup.maxUncoupledRange`.
    func setNetGroupMaxUncoupled(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.maxUncoupled, mm, NetGroup.maxUncoupledRange)
    }

    /// Sets a group's largest tuning bump, clamped to
    /// `NetGroup.maxMeanderRange`.
    func setNetGroupMaxMeander(_ id: UUID, _ mm: Double) {
        setNetGroupValue(id, \.maxMeander, mm, NetGroup.maxMeanderRange)
    }

    func setNetGroupValue(_ id: UUID, _ key: WritableKeyPath<NetGroup, Double>, _ mm: Double,
                                  _ range: ClosedRange<Double>) {
        editBoard("Change Net Group") { $0.setNetGroupValue(id, key, mm, range) }
    }

    /// Puts `net` in group `groupID`, out of any other group. A pair that
    /// already has 2 other nets refuses it, with `editorMessage` saying
    /// why. Returns whether the net is in the group now.
    @discardableResult
    func assignNet(_ net: PlaneNet, toGroup groupID: UUID) -> Bool {
        guard board.netGroupIndex(groupID) != nil else { return false }
        if let problem = board.netGroupJoinProblem(net, groupID) {
            editorMessage = problem
            return false
        }
        editBoard("Change Net Group") { $0.assignNet(net, toGroup: groupID) }
        return true
    }

    /// Takes `net` out of whichever group it is in.
    func unassignNet(_ net: PlaneNet) {
        editBoard("Change Net Group") { $0.unassignNet(net) }
    }

    /// The layer rule net `net` of `connectivity` follows: its own
    /// override, else its class's, else none.
    func layerRule(ofNet net: Int, in connectivity: BoardConnectivity) -> LayerRule? {
        board.layerRule(ofNet: net, in: connectivity)
    }

    /// The trace layers net `net` may use and those it prefers. Only: the
    /// rule's layers that are trace layers on the board (none means it
    /// can't be routed), with no preference. Prefer: every trace layer,
    /// preferring the rule's. No rule: every trace layer, no preference.
    func effectiveLayers(ofNet net: Int, in connectivity: BoardConnectivity) -> EffectiveLayers {
        board.effectiveLayers(ofNet: net, in: connectivity)
    }

    /// The group net `net` of `connectivity` is in: the first any of whose
    /// members names it.
    func group(ofNet net: Int, in connectivity: BoardConnectivity) -> NetGroup? {
        board.group(ofNet: net, in: connectivity)
    }

    func deleteVia(_ id: UUID) {
        edit(.pcb, "Delete Via") { scope in
            layout.updateCopper(scope) { $0.remove(traces: [], vias: [id]) }
            pruneSelection()
        }
    }

    /// The traces whose copper touches a via's metal, on any layer.
    func traces(touching via: BoardVia) -> [BoardTrace] {
        layout.copper.traces(touching: via.centre, radius: via.diameter / 2)
    }
}
