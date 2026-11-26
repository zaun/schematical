import CoreGraphics
import Foundation
import Observation

/// What File ▸ Import ▸ DXF… will make, in board mm (y down): the new outline
/// (outline mode only) and the cutouts' loops (plan-65-I).
nonisolated struct DXFPlacedImport: Hashable, Sendable {
    var outline: [CGPoint]?
    var cutouts: [[CGPoint]]
}

nonisolated enum DXFPlacement {
    /// The map from the DXF's frame (mm, y up) to the board's (mm, y down).
    /// *Outline and cutouts*: y is flipped and the outline's top-left moved to
    /// (0, 0), so the DXF's own position does not matter. *Cutouts only*: the
    /// DXF's origin lands at (`offset.x`, `offset.y`) on the board, y flipped.
    static func map(mode: DXFImportMode, outline: [CGPoint]?, offset: CGPoint) -> (CGPoint) -> CGPoint {
        switch mode {
        case .cutoutsOnly:
            return { CGPoint(x: offset.x + $0.x, y: offset.y - $0.y) }
        case .outlineAndCutouts:
            let flipped = (outline ?? []).map { CGPoint(x: $0.x, y: -$0.y) }
            let minX = flipped.map(\.x).min() ?? 0, minY = flipped.map(\.y).min() ?? 0
            return { CGPoint(x: $0.x - minX, y: -$0.y - minY) }
        }
    }

    /// The plan placed on the board.
    static func place(plan: DXFImportPlan, mode: DXFImportMode, offset: CGPoint) -> DXFPlacedImport {
        let move = map(mode: mode, outline: plan.outline, offset: offset)
        return DXFPlacedImport(outline: plan.outline.map { $0.map(move) }, cutouts: plan.cutouts.map { $0.map(move) })
    }
}

/// The choices of the DXF import sheet, and what they make (plan-65-I). The
/// sheet is a view of it, and the tests drive it without one.
@MainActor
@Observable
final class DXFImportState {
    /// As `SchematicExporter`'s: an isolated deinit of a main-actor class aborts the runtime (the test host).
    nonisolated deinit {}

    let fileName: String
    let drawing: DXFDrawing
    var layers: Set<String>
    var units: DXFUnits
    var mode: DXFImportMode = .outlineAndCutouts
    /// Cutouts only: where the drawing's origin lands on the board, mm (y down).
    var offsetX = 0.0
    var offsetY: Double
    /// The board's outline and size, for the preview and the default offset.
    let boardOutline: [CGPoint]

    /// The drawing's units when it says them, else the board's.
    init(fileName: String, drawing: DXFDrawing, board: BoardSettings) {
        self.fileName = fileName
        self.drawing = drawing
        layers = DXFLayerPick.defaultSelection(layers: drawing.layers)
        units = drawing.units ?? (board.units == .inch ? .inch : .millimetre)
        offsetY = board.height
        boardOutline = board.outline
    }

    /// One drawing unit in mm.
    var scale: Double { units.millimetres }

    var loopSet: DXFLoopSet { DXFLoops.build(drawing, layers: layers, scale: scale) }

    var plan: DXFImportPlan { DXFImportPlan.make(loops: loopSet.loops, mode: mode) }

    var placed: DXFPlacedImport { DXFPlacement.place(plan: plan, mode: mode, offset: CGPoint(x: offsetX, y: offsetY)) }

    /// Why Import is unavailable, or nil.
    var problem: String? {
        let plan = plan
        let placed = DXFPlacement.place(plan: plan, mode: mode, offset: CGPoint(x: offsetX, y: offsetY))
        switch mode {
        case .outlineAndCutouts:
            guard let outline = placed.outline else { return "The drawing has no closed outline on the chosen layers." }
            if let detail = BoardSettings.problem(outline: outline) { return "The outline " + detail + "." }
        case .cutoutsOnly:
            if placed.cutouts.isEmpty { return "The drawing has no closed loops on the chosen layers." }
        }
        for loop in placed.cutouts {
            if let detail = BoardCutout.problem(.polygon(loop)) { return "A cutout " + detail + "." }
        }
        return nil
    }

    /// "1 outline, 2 cutouts", "1 cutout".
    var summary: String {
        let plan = plan
        func cutouts(_ n: Int) -> String { "\(n) cutout\(n == 1 ? "" : "s")" }
        switch mode {
        case .outlineAndCutouts: return (plan.outline == nil ? [] : ["1 outline"]).appending(cutouts(plan.cutouts.count)).joined(separator: ", ")
        case .cutoutsOnly: return cutouts(plan.cutouts.count)
        }
    }

    /// What was left out and why: ignored loops, open chains, self-crossing loops, skipped entity types.
    var notes: [String] {
        let set = loopSet
        var out = DXFImportPlan.make(loops: set.loops, mode: mode).ignored.map { "\($0.count) \($0.reason.title)" }
        if set.openChains > 0 { out.append("\(set.openChains) open chain\(set.openChains == 1 ? "" : "s") not closed") }
        if set.crossingSelf > 0 { out.append("\(set.crossingSelf) loop\(set.crossingSelf == 1 ? "" : "s") cross themselves") }
        for (name, count) in drawing.skipped.sorted(by: { $0.key < $1.key }) { out.append("\(name) ×\(count) skipped") }
        return out
    }

    /// Every loop of the chosen layers in board mm, with its role, for the preview.
    var previewLoops: [(points: [CGPoint], role: Role)] {
        let plan = plan
        let move = DXFPlacement.map(mode: mode, outline: plan.outline, offset: CGPoint(x: offsetX, y: offsetY))
        return loopSet.loops.map { loop in
            let role: Role
            if let outline = plan.outline, loop.points == outline { role = .outline }
            else if plan.cutouts.contains(loop.points) { role = .cutout } else { role = .ignored }
            return (loop.points.map(move), role)
        }
    }

    enum Role { case outline, cutout, ignored }
}

private extension Array {
    func appending(_ element: Element) -> [Element] { self + [element] }
}
