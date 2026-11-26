import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Traces (PCB)

    /// Runs `change` on the board's traces and vias as one undo step named `name`.
    func editCopper<T>(_ name: String, _ change: (inout BoardCopper) -> T) -> T {
        edit(.pcb, name) { scope in layout.updateCopper(scope, change) }
    }

    /// The pads on `layer`, with their parts: a surface-mount pad is on its
    /// layer's side (its part's, or the other for an opposite pad); a
    /// through-hole pad is on every layer.
    func pads(on layer: CopperLayer) -> [(chip: PlacedChip, pad: PlacedPad)] {
        zip(chips, chipPads).flatMap { chip, pads -> [(chip: PlacedChip, pad: PlacedPad)] in
            guard let side = chip.board?.side else { return [] }
            return pads
                .filter { $0.plated && ($0.drill > 0 || $0.layerSide(on: side).copperLayer == layer) }
                .map { (chip: chip, pad: $0) }
        }
    }

    /// Where a trace end pressed or released at `point` lands on `layer`:
    /// the centre of a pad there, else of a via, else the nearest of a
    /// plated hole's four contacts (`BoardHole.contacts`), else the inner end of a
    /// card's edge finger or header pad there (`EdgeFinger.contact`), else a vertex of a trace on that layer
    /// within `tolerance`, else the nearest point of a trace segment within
    /// `tolerance`, else the nearest trace-grid point (`BoardSettings.traceGrid`).
    func traceAnchor(at point: CGPoint, layer: CopperLayer, tolerance: CGFloat) -> CGPoint {
        if let hit = pads(on: layer).last(where: { $0.pad.rect.contains(point) }) {
            return CGPoint(x: hit.pad.rect.midX, y: hit.pad.rect.midY)
        }
        if let via = via(at: point) { return via.centre }
        if let hole = holes.last(where: { $0.plated && Self.distance(from: point, to: $0.centre) <= $0.diameter / 2 }),
           let contact = hole.contact(nearest: point) {
            return contact
        }
        if let finger = (board.fingers + board.extraPads).last(where: { $0.isOn(layer) && $0.rect.contains(point) }) {
            return finger.contact
        }
        return layout.copper.nearestTracePoint(to: point, layer: layer, tolerance: tolerance)
            ?? BoardGrid.snap(point, spacing: board.traceGrid)
    }

    /// Adds a trace from `a` to `b` on `layer`, routed at `angle`
    /// (`TraceRoute.route`; `diagonalFirst` puts the angled leg first). See
    /// `addTrace(points:layer:)`.
    @discardableResult
    func addTrace(from a: CGPoint, to b: CGPoint, layer: CopperLayer, angle: Int = TraceRoute.defaultAngle,
                  diagonalFirst: Bool) -> UUID? {
        addTrace(points: TraceRoute.route(from: a, to: b, angle: angle, legFirst: diagonalFirst), layer: layer)
    }

    /// Adds a trace along `points` (simplified) on `layer`, as wide as the
    /// class of the net its first point is on (`netRules(at:layer:)`), and
    /// joins it to every trace it touches there. Returns its id, or nil when
    /// nothing was added: no length, or a layer that is a plane or not on
    /// the board.
    @discardableResult
    func addTrace(points: [CGPoint], layer: CopperLayer) -> UUID? {
        guard let start = points.first else { return nil }
        return addTrace(points: points, layer: layer, width: netRules(at: start, layer: layer).width)
    }

    /// Adds a trace `width` mil wide (clamped to `BoardTrace.widthRange`);
    /// otherwise as `addTrace(points:layer:)`.
    @discardableResult
    func addTrace(points: [CGPoint], layer: CopperLayer, width: Int) -> UUID? {
        let settings = board
        return editCopper("Add Trace") { $0.addTrace(points: points, layer: layer, width: width, settings: settings) }
    }

    /// Makes every point where trace `id` touches or crosses another trace
    /// on its layer a vertex of both. Inserting a vertex on a straight
    /// segment keeps its angle, so both stay legal.
    func join(_ id: UUID) {
        editCopper("Join Traces") { $0.join(id) }
    }

    /// Replaces the vertices of an unlocked trace with `points` (legal: at
    /// least two, none of zero length), keeping its id, layer, width and
    /// flags; the router's push moves existing copper this way (plan-66-C).
    /// Not joined to anything: the caller keeps the trace's contacts.
    @discardableResult
    func setTracePoints(_ id: UUID, _ points: [CGPoint]) -> Bool {
        guard layout.copper.canSetPoints(id, points) else { return false }
        editCopper("Move Trace") { $0.setPoints(id, points) }
        return true
    }

    /// The traces on the same layer reachable from `id` through shared
    /// vertices, `id` first.
    func connectedRun(of id: UUID) -> [UUID] {
        layout.copper.connectedRun(of: id)
    }

    /// The pads the traces of `run` touch on their layer, as
    /// "<reference>.<designator>" ("U1.3", or "U1.EP" for an unnumbered
    /// centre pad), sorted by reference, then designator.
    func pads(touching run: [UUID]) -> [String] {
        let members = traces.filter { run.contains($0.id) }
        guard let layer = members.first?.layer else { return [] }
        func touched(_ rect: CGRect) -> Bool {
            members.contains { trace in trace.segments.contains { Self.segment($0.0, $0.1, touches: rect) } }
        }
        var found: [(reference: String, designator: String)] = []
        for (chip, pad) in pads(on: layer) {
            let hit = members.contains { trace in trace.segments.contains { Self.segment($0.0, $0.1, touches: pad) } }
            if hit { found.append((chip.reference, pad.designator ?? "EP")) }
        }
        for finger in board.fingers where finger.isOn(layer) && touched(finger.rect) {
            found.append((board.connectorName, finger.name))
        }
        for pad in board.extraPads where pad.isOn(layer) && touched(pad.rect) {
            // "ICSP.1": its group and pin.
            let parts = pad.name.split(separator: ".", maxSplits: 1).map(String.init)
            if parts.count == 2 { found.append((parts[0], parts[1])) }
        }
        return found
            .sorted {
                $0.reference != $1.reference
                    ? $0.reference.localizedStandardCompare($1.reference) == .orderedAscending
                    : $0.designator.localizedStandardCompare($1.designator) == .orderedAscending
            }
            .map { "\($0.reference).\($0.designator)" }
    }

    /// Sets a trace's width in mil, clamped to `BoardTrace.widthRange`.
    func setTraceWidth(_ id: UUID, _ mil: Int) {
        editCopper("Change Trace Width") { $0.setWidth(id, mil) }
    }

    /// Moves one trace to another layer and joins it there. Ignored for a
    /// plane or a layer not on the board. Its run on the old layer stays.
    func setTraceLayer(_ id: UUID, _ layer: CopperLayer) {
        let settings = board
        editCopper("Change Layer") { $0.setLayer(id, layer, settings: settings) }
    }

    // MARK: Routing primitives (plan-38-G)

    /// Removes traces and vias by id, locked or not, inside the caller's
    /// edit (`applyRouting`, `unroute`); the callers decide what may go.
    func removeCopper(traces ids: Set<UUID>, vias viaIDs: Set<UUID>) {
        editCopper("Delete") { $0.remove(traces: ids, vias: viaIDs) }
        pruneSelection()
    }

    /// Appends a trace as it is (no join), inside the caller's edit. False
    /// when the board refuses it: a plane or absent layer, or no length.
    func appendTrace(_ trace: BoardTrace) -> Bool {
        let settings = board
        return editCopper("Add Trace") { $0.append(trace, settings: settings) }
    }

    /// Appends a via, inside the caller's edit. False on a 1-layer board.
    func appendVia(_ via: BoardVia) -> Bool {
        let layers = board.layers
        return editCopper("Add Via") { $0.append(via, layers: layers) }
    }

    func deleteTrace(_ id: UUID) {
        edit(.pcb, "Delete Trace") { scope in
            layout.updateCopper(scope) { $0.remove(traces: [id], vias: []) }
            pruneSelection()
        }
    }

    /// The PCB Delete tool's click: deletes the text under `point`, else the
    /// via, else the trace (as `text(at:)`, `via(at:)` and `trace(at:)` hit
    /// them). Footprints are never deleted. Returns whether something was.
    @discardableResult
    func deleteBoardItem(at point: CGPoint, tolerance: CGFloat, visible: BoardVisibility = BoardVisibility()) -> Bool {
        if let hit = text(at: point, visible: visible) {
            deleteText(hit.id)
            return true
        }
        if let hit = via(at: point, visible: visible) {
            deleteVia(hit.id)
            return true
        }
        if let hit = hole(at: point, visible: visible) {
            deleteHole(hit.id)
            return true
        }
        if let hit = trace(at: point, tolerance: tolerance, visible: visible) {
            deleteTrace(hit.id)
            return true
        }
        switch pourItem(at: point, tolerance: tolerance, visible: visible) {
        case let .pour(id)?:
            deletePour(id)
            return true
        case let .keepout(id)?:
            deleteKeepout(id)
            return true
        case let .cutout(id)?:
            deleteCutout(id)
            return true
        default:
            return false
        }
    }

    /// The ids of the selected PCB items `pick` takes.
    func selected(_ pick: (BoardItem) -> UUID?) -> Set<UUID> {
        Set(boardSelection.items.compactMap(pick))
    }

    /// The selected traces, in drawing order, for ⌘C.
    func selectedTraces() -> [BoardTrace] {
        let ids = selected { if case let .trace(id) = $0 { id } else { nil } }
        return traces.filter { ids.contains($0.id) }
    }

    /// The selected board texts, in creation order, for ⌘C.
    func selectedTexts() -> [BoardText] {
        let ids = selected { if case let .text(id) = $0 { id } else { nil } }
        return texts.filter { ids.contains($0.id) }
    }

    /// The selected vias, in creation order, for ⌘C.
    func selectedVias() -> [BoardVia] {
        let ids = selected { if case let .via(id) = $0 { id } else { nil } }
        return vias.filter { ids.contains($0.id) }
    }

    /// ⌘V on the PCB: adds a copy of each of `copied` moved by `offset`
    /// (mm), with new ids, its width and layer kept, joined to whatever it
    /// touches, as one step. The copies become the whole selection. A trace
    /// whose layer isn't on this board, or is a plane, is skipped. Returns
    /// the new ids.
    @discardableResult
    func pasteTraces(_ copied: [BoardTrace], offset: CGSize) -> [UUID] {
        pasteBoard(traces: copied, texts: [], offset: offset)
    }

    /// ⌘V on the PCB: adds a copy of each of `traces` (as `pasteTraces`), of
    /// each of `texts` and of each of `vias`, moved by `offset` (mm), with
    /// new ids, as one step. A text keeps its words, size, layer, rotation
    /// and side (Top on a 1-layer board), lands on the grid and is not
    /// locked. A via keeps its drill, diameter and tenting, lands on the
    /// trace grid, is not locked and is not the autorouter's; none are
    /// pasted on a 1-layer board. The copies become the whole selection.
    /// Returns the new ids.
    @discardableResult
    func pasteBoard(traces copied: [BoardTrace], texts copiedTexts: [BoardText], vias copiedVias: [BoardVia] = [],
                    offset: CGSize) -> [UUID] {
        let settings = board
        guard copied.contains(where: { settings.traceLayerRefusal($0.layer) == nil }) || !copiedTexts.isEmpty
                || (settings.layers > 1 && !copiedVias.isEmpty) else { return [] }
        return edit(.pcb, "Paste") { scope -> [UUID] in
            let copper = layout.updateCopper(scope) {
                $0.paste(traces: copied, vias: copiedVias, offset: offset, settings: settings)
            }
            let texts = layout.updateTexts(scope) {
                $0.paste(copiedTexts, offset: offset, grid: settings.grid, layers: settings.layers)
            }
            let items = copper.traces.map(BoardItem.trace) + texts.map { .text($0.id) } + copper.vias.map(BoardItem.via)
            boardSelection.set(Set(items))
            return copper.traces + texts.map(\.id) + copper.vias
        }
    }

    /// ⌘⌫ on the PCB: deletes every selected text, trace, via and hole, as
    /// one step. Selected footprints stay. Returns whether anything was
    /// deleted.
    @discardableResult
    func deleteBoardSelection() -> Bool {
        var textIDs: Set<UUID> = [], traceIDs: Set<UUID> = [], viaIDs: Set<UUID> = [], holeIDs: Set<UUID> = []
        var pourIDs: Set<UUID> = [], keepoutIDs: Set<UUID> = [], cutoutIDs: Set<UUID> = []
        for item in boardSelection.items {
            switch item {
            case let .text(id): textIDs.insert(id)
            case let .trace(id): traceIDs.insert(id)
            case let .via(id): viaIDs.insert(id)
            case let .hole(id): holeIDs.insert(id)
            case let .pour(id): pourIDs.insert(id)
            case let .keepout(id): keepoutIDs.insert(id)
            case let .cutout(id): cutoutIDs.insert(id)
            case .footprint: break
            }
        }
        guard !textIDs.isEmpty || !traceIDs.isEmpty || !viaIDs.isEmpty || !holeIDs.isEmpty || !pourIDs.isEmpty
                || !keepoutIDs.isEmpty || !cutoutIDs.isEmpty else { return false }
        edit(.pcb, "Delete") { scope in
            layout.updateTexts(scope) { $0.remove(textIDs) }
            layout.updateCopper(scope) { $0.remove(traces: traceIDs, vias: viaIDs) }
            layout.updateFeatures(scope) {
                $0.remove(holes: holeIDs, pours: pourIDs, keepouts: keepoutIDs, cutouts: cutoutIDs)
            }
            pruneSelection()
        }
        return true
    }

    /// ⌘X on the PCB, after the selection was copied: deletes just the
    /// selected traces, texts and vias (what ⌘C takes), as one step named "Cut".
    /// Returns whether any were deleted.
    @discardableResult
    func cutBoardSelection() -> Bool {
        let traceIDs = Set(selectedTraces().map(\.id))
        let textIDs = Set(selectedTexts().map(\.id))
        let viaIDs = Set(selectedVias().map(\.id))
        guard !traceIDs.isEmpty || !textIDs.isEmpty || !viaIDs.isEmpty else { return false }
        edit(.pcb, "Cut") { scope in
            layout.updateCopper(scope) { $0.remove(traces: traceIDs, vias: viaIDs) }
            layout.updateTexts(scope) { $0.remove(textIDs) }
            pruneSelection()
        }
        return true
    }

    /// Moves every vertex of the traces `ids` by `offset` (mm), then joins
    /// each one where it now lands.
    func moveTraces(_ ids: Set<UUID>, by offset: CGSize) {
        editCopper("Move") { $0.moveTraces(ids, by: offset) }
    }

    /// The trace under a board point (mm): the last one drawn there (Top's
    /// layer first, down the stack) whose layer is shown, within half its
    /// width plus `tolerance`.
    func trace(at point: CGPoint, tolerance: CGFloat, visible: BoardVisibility = BoardVisibility()) -> BoardTrace? {
        layout.copper.trace(at: point, tolerance: tolerance, layers: board.copperLayers.filter(visible.showsTrace(on:)))
    }

    // MARK: Vias (PCB)

    /// The via whose metal is under `point` (mm): the last one drawn there.
    /// Vias are drawn while any copper layer is.
    func via(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> BoardVia? {
        guard visible.showsAnyCopper else { return nil }
        return layout.copper.via(at: point)
    }

    /// Where a via placed at `point` lands: a vertex of a trace on any layer
    /// within `tolerance`, else the nearest point of a trace segment within
    /// `tolerance`, else the nearest trace-grid point.
    func viaAnchor(at point: CGPoint, tolerance: CGFloat) -> CGPoint {
        layout.copper.nearestTracePoint(to: point, layer: nil, tolerance: tolerance)
            ?? BoardGrid.snap(point, spacing: board.traceGrid)
    }

    /// Adds a via at `point` (already anchored) with the sizes of the class
    /// of the net there (`netRules(at:layer:)`; the defaults on no net or
    /// the Default class) and selects it. Nil on a 1-layer board, which has
    /// nothing to join.
    @discardableResult
    func addVia(at point: CGPoint) -> UUID? {
        guard board.layers > 1 else { return nil }
        let rules = netRules(at: point, layer: nil)
        return edit(.pcb, "Add Via") { scope -> UUID? in
            let id = layout.updateCopper(scope) { $0.addVia(at: point, rules: rules) }
            boardSelection.replace(with: .via(id))
            return id
        }
    }

    /// Moves a via's centre to `point`, snapped to the trace grid.
    func moveVia(_ id: UUID, to point: CGPoint) {
        let target = BoardGrid.snap(point, spacing: board.traceGrid)
        editCopper("Move") { $0.setViaCentre(id, target) }
    }

    /// Moves a via's centre to exactly `point` (the sidebar's X and Y).
    func setViaCentre(_ id: UUID, _ point: CGPoint) {
        editCopper("Move") { $0.setViaCentre(id, point) }
    }

    /// Sets a via's hole, clamped to `BoardVia.drillRange` and to leave the
    /// thinnest ring inside its metal.
    func setViaDrill(_ id: UUID, _ mm: Double) {
        editCopper("Change Via Hole") { $0.setViaDrill(id, mm) }
    }

    /// Sets a via's metal diameter, clamped to `BoardVia.diameterRange` and
    /// to keep the thinnest ring round its hole.
    func setViaDiameter(_ id: UUID, _ mm: Double) {
        editCopper("Change Via Size") { $0.setViaDiameter(id, mm) }
    }

    /// Sets whether the solder mask covers a via on `side`.
    func setViaTenting(_ id: UUID, _ side: BoardSide, _ tented: Bool) {
        editCopper("Change Via Tenting") { $0.setViaTenting(id, side, tented) }
    }

    // MARK: Trace geometry

    /// Whether segment a–b meets `rect` (inside it, or crossing or touching
    /// its edge).
    nonisolated static func segment(_ a: CGPoint, _ b: CGPoint, touches rect: CGRect) -> Bool {
        BoardCopper.segment(a, b, touches: rect)
    }

    // MARK: Segment drags (PCB)

    /// What a plain Select drag on trace `id` at `point` (board mm) takes
    /// hold of: the segment nearest the press, the trace's points now and
    /// which of them must stay where they are. Nil for a locked trace or
    /// none.
    func traceSegmentBase(_ id: UUID, at point: CGPoint) -> TraceSegmentBase? {
        guard let layer = traces.first(where: { $0.id == id })?.layer else { return nil }
        let pads = pads(on: layer).map(\.1.rect) + board.fingers.filter { $0.isOn(layer) }.map(\.rect)
            + board.extraPads.filter { $0.isOn(layer) }.map(\.rect)
        return layout.copper.segmentBase(id, at: point, pads: pads)
    }

    /// One frame of a plain drag on a trace segment: the segment slides
    /// sideways by the part of `delta` (mm) at right angles to it, snapped
    /// to the trace grid, and the rest of the trace follows (`TraceRoute.slide`).
    func dragTraceSegment(_ base: TraceSegmentBase, by delta: CGSize) {
        let grid = board.traceGrid
        editCopper("Move Segment") { $0.dragSegment(base, by: delta, grid: grid) }
    }

    /// The end of a segment drag: the trace joins what it now touches.
    func settleTraceSegment(_ base: TraceSegmentBase) {
        editCopper("Move Segment") { $0.join(base.id) }
    }

    /// What a Select press at `point` (board mm) picks: a text, else a via,
    /// else a trace (within `tolerance` of its edge), else a footprint; Top
    /// before Bottom within each. Only drawn items count.
    func boardItem(at point: CGPoint, tolerance: CGFloat,
                   visible: BoardVisibility = BoardVisibility()) -> BoardItem? {
        if let text = text(at: point, visible: visible) { return .text(text.id) }
        if let via = via(at: point, visible: visible) { return .via(via.id) }
        if let hole = hole(at: point, visible: visible) { return .hole(hole.id) }
        if let trace = trace(at: point, tolerance: tolerance, visible: visible) { return .trace(trace.id) }
        if let chip = footprint(at: point, visible: visible), chip.board != nil { return .footprint(chip.id) }
        return pourItem(at: point, tolerance: tolerance, visible: visible)
    }
}
