import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Pours and keep-outs (PCB)

    /// Runs `change` on the board's holes, pours, keep-outs and cutouts as
    /// one undo step named `name`.
    func editFeatures<T>(_ name: String, _ change: (inout BoardFeatures) -> T) -> T {
        edit(.pcb, name) { scope in layout.updateFeatures(scope, change) }
    }

    /// Adds a pour over `rect` (snapped to the trace grid) on `layer`, and
    /// selects it. It takes the settings of the last pour added; its net is
    /// the last pour's, else ground when the schematic has it, else none.
    /// Nil when `layer` is not a trace layer of the board.
    @discardableResult
    func addPour(rect: CGRect, layer: CopperLayer) -> UUID? {
        guard board.traceLayers.contains(layer) else { return nil }
        let grid = board.traceGrid
        return edit(.pcb, "Add Pour") { scope -> UUID in
            let ground = pours.isEmpty && planeNetChoices().contains(where: { $0.net == .ground })
            let id = layout.updateFeatures(scope) { $0.addPour(rect: rect, layer: layer, grid: grid, ground: ground) }
            boardSelection.replace(with: .pour(id))
            return id
        }
    }

    /// Moves a pour so its top-left corner is at `origin`, snapped to the trace grid.
    func movePour(_ id: UUID, to origin: CGPoint) {
        let grid = board.traceGrid
        editFeatures("Move") { $0.movePour(id, to: origin, grid: grid) }
    }

    /// Sets a pour's rectangle (the sidebar's X, Y, width and height, a
    /// resize), snapped to the trace grid.
    func setPourRect(_ id: UUID, _ rect: CGRect, name: String = "Resize") {
        let grid = board.traceGrid
        editFeatures(name) { $0.setPourRect(id, rect, grid: grid) }
    }

    /// Puts a pour on `layer`, a trace layer of the board.
    func setPourLayer(_ id: UUID, _ layer: CopperLayer) {
        guard board.traceLayers.contains(layer) else { return }
        editFeatures("Change Pour Layer") { $0.setPourLayer(id, layer) }
    }

    /// Sets a pour's net (`planeNetChoices`), or none.
    func setPourNet(_ id: UUID, _ net: PlaneNet?) {
        editFeatures("Change Pour Net") { $0.setPourNet(id, net) }
    }

    /// Sets a pour's clearance (mm), clamped to `LayerStyle.clearanceRange`;
    /// nil is the DRC's minimum.
    func setPourClearance(_ id: UUID, _ mm: Double?) {
        editFeatures("Change Pour") { $0.setPourClearance(id, mm) }
    }

    func setPourConnection(_ id: UUID, _ connection: PourConnection) {
        editFeatures("Change Pour") { $0.setPourConnection(id, connection) }
    }

    func setPourThermalGap(_ id: UUID, _ mm: Double) {
        editFeatures("Change Pour") { $0.setPourThermalGap(id, mm) }
    }

    func setPourSpokeWidth(_ id: UUID, _ mm: Double) {
        editFeatures("Change Pour") { $0.setPourSpokeWidth(id, mm) }
    }

    func setPourMinWidth(_ id: UUID, _ mm: Double) {
        editFeatures("Change Pour") { $0.setPourMinWidth(id, mm) }
    }

    /// Removes holes, pours, keep-outs or cutouts as one undo step named `name`.
    func deleteFeatures(_ name: String, holes: Set<UUID> = [], pours: Set<UUID> = [],
                                keepouts: Set<UUID> = [], cutouts: Set<UUID> = []) {
        edit(.pcb, name) { scope in
            layout.updateFeatures(scope) { $0.remove(holes: holes, pours: pours, keepouts: keepouts, cutouts: cutouts) }
            pruneSelection()
        }
    }

    func deletePour(_ id: UUID) {
        deleteFeatures("Delete", pours: [id])
    }

    /// Adds a keep-out over `rect` (snapped to the trace grid) on `layers`
    /// (nil: every layer), and selects it.
    @discardableResult
    func addKeepout(rect: CGRect, layers: Set<CopperLayer>? = nil) -> UUID {
        let grid = board.traceGrid
        return edit(.pcb, "Add Keep-out") { scope -> UUID in
            let id = layout.updateFeatures(scope) { $0.addKeepout(rect: rect, layers: layers, grid: grid) }
            boardSelection.replace(with: .keepout(id))
            return id
        }
    }

    /// Moves a keep-out so its top-left corner is at `origin`, snapped to the trace grid.
    func moveKeepout(_ id: UUID, to origin: CGPoint) {
        let grid = board.traceGrid
        editFeatures("Move") { $0.moveKeepout(id, to: origin, grid: grid) }
    }

    func setKeepoutRect(_ id: UUID, _ rect: CGRect, name: String = "Resize") {
        let grid = board.traceGrid
        editFeatures(name) { $0.setKeepoutRect(id, rect, grid: grid) }
    }

    /// Sets the layers a keep-out applies to; nil is every layer.
    func setKeepoutLayers(_ id: UUID, _ layers: Set<CopperLayer>?) {
        editFeatures("Change Keep-out") { $0.setKeepoutLayers(id, layers) }
    }

    func deleteKeepout(_ id: UUID) {
        deleteFeatures("Delete", keepouts: [id])
    }

    // MARK: Cutouts (PCB)

    /// Adds a cutout of `shape` and selects it. A rounded shape's box is
    /// snapped to the trace grid (and kept within the allowed sizes); a polygon
    /// is stored clockwise. Nil, with no undo step, when `BoardCutout.problem`
    /// refuses the shape.
    @discardableResult
    func addCutout(_ shape: BoardCutout.Shape) -> UUID? {
        guard BoardCutout.problem(shape) == nil else { return nil }
        let grid = board.traceGrid
        return edit(.pcb, "Add Cutout") { scope -> UUID in
            let id = layout.updateFeatures(scope) { $0.addCutout(shape, grid: grid) }
            boardSelection.replace(with: .cutout(id))
            return id
        }
    }

    /// Moves a cutout so its bounds' top-left corner is at `origin`, snapped to the trace grid.
    func moveCutout(_ id: UUID, to origin: CGPoint) {
        let grid = board.traceGrid
        editFeatures("Move") { $0.moveCutout(id, to: origin, grid: grid) }
    }

    /// Sets a rounded cutout's box (snapped to the trace grid, at least 0.5 mm a side); its radius
    /// stays within the new box. Does nothing to a polygon or a locked cutout.
    func setCutoutRect(_ id: UUID, _ rect: CGRect, name: String = "Resize") {
        let grid = board.traceGrid
        editFeatures(name) { $0.setCutoutRect(id, rect, grid: grid) }
    }

    /// Sets a rounded cutout's corner radius, within 0 to half its short side.
    func setCutoutRadius(_ id: UUID, _ radius: Double) {
        editFeatures("Change Cutout") { $0.setCutoutRadius(id, radius) }
    }

    func deleteCutout(_ id: UUID) {
        deleteFeatures("Delete", cutouts: [id])
    }

    /// Whether a keep-out is drawn with `visible`: any layer it applies to shows.
    func keepoutIsShown(_ keepout: BoardKeepout, _ visible: BoardVisibility) -> Bool {
        BoardFeatures.keepoutIsShown(keepout, visible, layers: board.copperLayers)
    }

    /// The pour or keep-out whose edge is within `tolerance` of `point` (mm):
    /// the last pour, else the last keep-out. Pressing inside one does not
    /// hit it (it is as big as a board), only its edge does.
    func pourItem(at point: CGPoint, tolerance: CGFloat, visible: BoardVisibility = BoardVisibility()) -> BoardItem? {
        layout.features.item(at: point, tolerance: tolerance, visible: visible, layers: board.copperLayers)
    }

    // MARK: Holes (PCB)

    /// The hole under `point` (mm): the last one drawn there, within its
    /// land (plated) or its hole. Holes are drawn while any copper layer is.
    func hole(at point: CGPoint, visible: BoardVisibility = BoardVisibility()) -> BoardHole? {
        guard visible.showsAnyCopper else { return nil }
        return layout.features.hole(at: point)
    }

    /// Adds a hole at `point`, snapped to the trace grid, and selects it.
    /// It takes the size, plating and net of the last hole added, so a set
    /// of mounting holes is quick to place; the first is a non-plated M3.
    @discardableResult
    func addHole(at point: CGPoint) -> UUID {
        let grid = board.traceGrid
        return edit(.pcb, "Add Hole") { scope -> UUID in
            let id = layout.updateFeatures(scope) { $0.addHole(at: point, grid: grid) }
            boardSelection.replace(with: .hole(id))
            return id
        }
    }

    /// Moves a hole's centre to `point`, snapped to the trace grid.
    func moveHole(_ id: UUID, to point: CGPoint) {
        let target = BoardGrid.snap(point, spacing: board.traceGrid)
        editFeatures("Move") { $0.setHoleCentre(id, target) }
    }

    /// Moves a hole's centre to exactly `point` (the sidebar's X and Y).
    func setHoleCentre(_ id: UUID, _ point: CGPoint) {
        editFeatures("Move") { $0.setHoleCentre(id, point) }
    }

    /// Sets a hole's size: a screw size sets its hole and land.
    func setHoleSize(_ id: UUID, _ size: HoleSize) {
        editFeatures("Change Hole Size") { $0.setHoleSize(id, size) }
    }

    /// Sets a hole's drill, clamped to `BoardHole.drillRange`. The land
    /// grows to keep the thinnest ring round it. The size becomes Custom.
    func setHoleDrill(_ id: UUID, _ mm: Double) {
        editFeatures("Change Hole Size") { $0.setHoleDrill(id, mm) }
    }

    /// Sets a hole's land diameter, clamped to `BoardHole.diameterRange` and
    /// to keep the thinnest ring round its hole. The size becomes Custom.
    func setHoleDiameter(_ id: UUID, _ mm: Double) {
        editFeatures("Change Hole Size") { $0.setHoleDiameter(id, mm) }
    }

    /// Makes a hole plated (a copper land that can carry a net) or bare.
    func setHolePlated(_ id: UUID, _ plated: Bool) {
        editFeatures(plated ? "Plate Hole" : "Unplate Hole") { $0.setHolePlated(id, plated) }
    }

    /// Sets a plated hole's net (`planeNetChoices`), or none.
    func setHoleNet(_ id: UUID, _ net: PlaneNet?) {
        editFeatures("Change Hole Net") { $0.setHoleNet(id, net) }
    }

    func deleteHole(_ id: UUID) {
        deleteFeatures("Delete Hole", holes: [id])
    }

    /// The traces whose copper touches a plated hole's land, on any layer.
    func traces(touching hole: BoardHole) -> [BoardTrace] {
        guard hole.plated else { return [] }
        return layout.copper.traces(touching: hole.centre, radius: hole.diameter / 2)
    }
}
