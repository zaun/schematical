import AppKit
import SwiftUI

/// Where the pointer is over a PCB canvas: in board mm, and on screen.
/// Observed, and read only by `PCBHoverLayer`.
@MainActor @Observable
final class BoardHover {
    var point: CGPoint?
    var location: CGPoint?

    /// As `TraceKeyContext`'s: an isolated deinit aborts the runtime.
    nonisolated deinit {}
}

/// What follows the pointer over the PCB canvas, drawn over it: the hovered
/// pad's outline and its name beside the pointer, and the Trace tool's
/// green ring where a press would land. A layer of its own, so the pointer
/// moving redraws only this and never the board under it.
struct PCBHoverLayer: View {
    let document: SchematicDocument
    let viewport: CanvasViewport
    let hover: BoardHover
    /// The layers the canvas shows; a pad that isn't drawn isn't hovered.
    let visible: BoardVisibility
    /// False during a move, pan or box.
    let showsPad: Bool
    /// The Trace tool's layer while it waits for a press, else nil.
    let ringLayer: CopperLayer?

    var body: some View {
        let scale = viewport.zoom * BoardScale.pointsPerMM
        let offset = viewport.offset
        let point = hover.point
        let hovered = showsPad ? point.flatMap { document.padLabel(at: $0, visible: visible) } : nil
        let ring = ringLayer.flatMap { layer in
            point.map { TraceDrawing.anchor(document, at: $0, layer: layer, scale: scale) }
        }
        Canvas { context, _ in
            guard hovered != nil || ring != nil else { return }
            context.translateBy(x: offset.width, y: offset.height)
            context.scaleBy(x: scale, y: scale)
            if let ring {
                let r = 6 / scale
                context.stroke(Path(ellipseIn: CGRect(x: ring.x - r, y: ring.y - r, width: 2 * r, height: 2 * r)),
                               with: .color(.green), lineWidth: 2 / scale)
            }
            if let hovered {
                context.stroke(PCBCanvasView.outline(of: hovered.pad), with: .color(.white.opacity(0.9)),
                               lineWidth: 1.5 / scale)
            }
        }
        .overlay(alignment: .topLeading) {
            if let hovered, let location = hover.location {
                PadLabel(text: hovered.label + "\n" + document.netName(ofPad: hovered.pad, in: document.connectivity()))
                    .offset(x: location.x + 14, y: location.y + 14)
            }
        }
        .allowsHitTesting(false)
    }
}

/// The hovered pad's name and, on a second line, its net, in a small tag
/// beside the pointer.
struct PadLabel: View {
    let text: String

    var body: some View {
        Text(text)
            .font(.system(size: 11, weight: .medium))
            .foregroundStyle(.white)
            .multilineTextAlignment(.leading)
            .padding(.horizontal, 5)
            .padding(.vertical, 2)
            .background(Color.black.opacity(0.75), in: RoundedRectangle(cornerRadius: 4))
            .fixedSize()
    }
}

/// The steps of a Cmd-drag on a reference, kept apart from the view so tests
/// can run them in the order the canvas does: hit the press, move while
/// dragging.
@MainActor
enum ReferenceDragging {
    /// The reference under a press at `point` (board mm), if any; its footprint
    /// becomes the whole selection unless it is already selected. `scale` is
    /// screen points per mm. Returns the footprint and where its reference's
    /// centre is now.
    static func press(_ document: SchematicDocument, at point: CGPoint, scale: CGFloat,
                      visible: BoardVisibility) -> (id: UUID, from: CGPoint)? {
        // The text is drawn at least 8 points tall (`PCBCanvasView.drawMarks`).
        let height = max(BoardReference.height, 8 / Double(scale))
        guard let id = document.referenceHit(at: point, tolerance: TraceDrawing.tolerance / scale,
                                             height: height, visible: visible),
              let from = document.chips.first(where: { $0.id == id })?.boardReferenceCentre else { return nil }
        if !document.boardSelection.items.contains(.footprint(id)) { document.select(.footprint(id)) }
        return (id, from)
    }

    /// One frame of the drag: the reference's centre goes to where it was at
    /// the press plus the pointer's movement `offset` (mm), snapped. A press
    /// with no movement yet leaves the reference where it is.
    static func drag(_ document: SchematicDocument, id: UUID, from: CGPoint, by offset: CGSize) {
        guard offset != .zero else { return }
        document.moveReference(id, toCentre: CGPoint(x: from.x + offset.width, y: from.y + offset.height))
    }
}

/// Where the pointer is over a PCB canvas (board mm), or nil when it is off
/// it. A plain class, so the pointer moving doesn't redraw its owner.
@MainActor
final class BoardPointer {
    var point: CGPoint?

    /// As `TraceKeyContext`'s: an isolated deinit aborts the runtime.
    nonisolated deinit {}
}

/// The Trace tool's layer and angle as they are now. The canvas's key
/// monitor is a closure made once, when the canvas appears, so the view it
/// holds has the layer and angle of then; the canvas keeps these current
/// (`onChange`) and the monitor reads them here.
@MainActor
final class TraceKeyContext {
    var layer: CopperLayer = .top
    var angle = TraceRoute.defaultAngle

    /// As `AssistRouter`'s: an isolated deinit aborts the runtime.
    nonisolated deinit {}
}

/// The steps of a Trace tool drag, kept apart from the view so tests can
/// run them in the order the canvas does: anchor the press, preview while
/// dragging, finish on release.
@MainActor
enum TraceDrawing {
    /// Screen points within which an end snaps to a trace, as the schematic
    /// Line tool's tolerance.
    static let tolerance: CGFloat = 6

    /// Where an end at `point` lands (see `SchematicDocument.traceAnchor`).
    /// `scale` is screen points per mm.
    static func anchor(_ document: SchematicDocument, at point: CGPoint, layer: CopperLayer,
                       scale: CGFloat) -> CGPoint {
        document.traceAnchor(at: point, layer: layer, tolerance: tolerance / scale)
    }

    /// The route a release at `cursor` would add: the draft's locked legs,
    /// then the live leg at `angle` to where the cursor lands, or (with
    /// `assist`) the assist's leg there.
    static func preview(_ document: SchematicDocument, draft: TraceDraft, to cursor: CGPoint,
                        layer: CopperLayer, scale: CGFloat, angle: Int, assist: AssistRouter? = nil) -> [CGPoint] {
        assisted(document, draft: draft, to: cursor, layer: layer, scale: scale, angle: angle, assist: assist).points
    }

    /// `preview`, and whether a release would add it: with `assist`, only
    /// when the assist found a legal leg to where the cursor lands (else
    /// the points are the plain leg, drawn red). Without, always.
    static func assisted(_ document: SchematicDocument, draft: TraceDraft, to cursor: CGPoint, layer: CopperLayer,
                         scale: CGFloat, angle: Int, assist: AssistRouter?) -> (points: [CGPoint], isLegal: Bool) {
        let end = anchor(document, at: cursor, layer: layer, scale: scale)
        guard let assist, !TraceRoute.same(draft.legStart, end) else { return (draft.path(to: end, angle: angle), true) }
        guard let leg = assist.route(from: draft.legStart, to: end) else { return (draft.path(to: end, angle: angle), false) }
        return (draft.path(through: leg), true)
    }

    /// The message an illegal assisted release leaves.
    static let noRouteMessage = "No legal route there."

    /// V while drawing (plan-38-L): the trace so far is added (as a
    /// release would add it), a via of its net's class size stands where it
    /// ends, and drawing goes on from the via on the next trace layer
    /// (`board.traceLayers` after `layer`, wrapping; never a Plane). Runs
    /// inside the drag's gesture, so the whole drag stays one undo step.
    /// Returns the new draft and layer; nil, changing nothing, on a board
    /// with one trace layer, where the assisted leg is illegal, or where
    /// the via wouldn't keep clear of other nets' copper or holes (both
    /// say why in `editorMessage`).
    static func dropVia(_ document: SchematicDocument, draft: TraceDraft, to cursor: CGPoint, layer: CopperLayer,
                        scale: CGFloat, angle: Int, assist: AssistRouter?,
                        rules: DesignRules? = nil) -> (draft: TraceDraft, layer: CopperLayer)? {
        let rules = rules ?? document.board.rules
        let layers = document.board.traceLayers
        guard layers.count >= 2, let index = layers.firstIndex(of: layer) else { return nil }
        let route = assisted(document, draft: draft, to: cursor, layer: layer, scale: scale, angle: angle, assist: assist)
        guard route.isLegal else {
            document.editorMessage = noRouteMessage
            return nil
        }
        guard let centre = route.points.last else { return nil }
        if let why = AssistRouter.viaProblem(document, at: centre, start: draft.start, layer: layer, rules: rules) {
            document.editorMessage = why
            return nil
        }
        // The trace first, so the via finds its net (and class size) there.
        document.addTrace(points: route.points, layer: layer, width: width(document, draft: draft, layer: layer))
        guard document.addVia(at: centre) != nil else { return nil }
        return (TraceDraft(start: centre), layers[(index + 1) % layers.count])
    }

    /// The width, in mil, of the trace a draft draws: that of the class of
    /// the net it starts on (`SchematicDocument.netRules`). The live preview
    /// and the trace the release adds both read it.
    static func width(_ document: SchematicDocument, draft: TraceDraft, layer: CopperLayer) -> Int {
        document.netRules(at: draft.start, layer: layer).width
    }

    /// The release: adds the trace (nothing for a zero-length one), makes it
    /// the whole selection, and asks for its layer to be shown if hidden.
    /// With `assist` and no legal leg to the cursor, adds nothing and says
    /// so (`editorMessage`).
    @discardableResult
    static func finish(_ document: SchematicDocument, draft: TraceDraft, to cursor: CGPoint, layer: CopperLayer,
                       scale: CGFloat, angle: Int, visible: BoardVisibility, assist: AssistRouter? = nil,
                       onShowLayer: (CopperLayer) -> Void) -> UUID? {
        let route = assisted(document, draft: draft, to: cursor, layer: layer, scale: scale, angle: angle, assist: assist)
        guard route.isLegal else {
            document.editorMessage = noRouteMessage
            return nil
        }
        let points = route.points
        guard let id = document.addTrace(points: points, layer: layer,
                                         width: width(document, draft: draft, layer: layer)) else {
            return nil
        }
        document.select(.trace(id))
        if !visible.showsTrace(on: layer) { onShowLayer(layer) }
        return id
    }
}

/// The PCB page's tools: Select (move footprints, texts and traces, pan),
/// Text (click to place a text), Trace (drag to draw a trace), Via (click
/// to place a via), Hole (click to place a drilled hole) and Delete (click
/// a text, via, hole or trace to delete it).
enum PCBTool: String, CaseIterable, Identifiable {
    case select
    case text
    case trace
    case via
    case hole
    /// Drag a rectangle to flood it with copper (plan-52).
    case pour
    /// Drag a rectangle the pours stay out of (plan-52).
    case keepout
    /// Drag a rectangle, circle or slot to cut out of the board (plan-65).
    case cutout
    case delete
    /// Click a pad to route its net, a ratsnest line to route that one
    /// connection, with the last Autoroute options (plan-38-K).
    case route

    var id: String { rawValue }

    var title: String {
        switch self {
        case .select: "Select"
        case .text: "Text"
        case .trace: "Trace"
        case .via: "Via"
        case .hole: "Hole"
        case .pour: "Pour"
        case .keepout: "Keep-out"
        case .cutout: "Cutout"
        case .delete: "Delete"
        case .route: "Route"
        }
    }

    /// The toolbar tooltip: the title, plus the drawing keys for Trace.
    var tooltip: String {
        switch self {
        case .trace: "Trace — Space: lock a corner · /: flip the bend · A: change the angle · Esc: undo a corner"
        case .via: "Via — click to place a via, drag to move it (2 or more layers)"
        case .hole: "Hole — click to place a mounting or drilled hole, drag to move it"
        case .pour: "Pour — drag a rectangle to flood it with copper on the Trace layer; it keeps its clearance from other nets"
        case .keepout: "Keep-out — drag a rectangle the pours stay out of"
        case .cutout: "Cutout — drag a rectangle, circle or slot to cut out of the board"
        case .delete: "Delete — click a trace, via, hole, text, pour, keep-out or cutout edge to delete it (footprints stay)"
        case .route: "Route — click a pad to route its net, a ratsnest line to route that one connection"
        default: title
        }
    }

    var symbol: String {
        switch self {
        case .select: "cursorarrow"
        case .text: "textformat"
        case .trace: "line.diagonal"
        case .via: "circle.circle"
        case .hole: "smallcircle.circle"
        case .pour: "rectangle.fill"
        case .keepout: "rectangle.badge.xmark"
        case .cutout: "rectangle.dashed"
        case .delete: "eraser"
        case .route: "point.topleft.down.curvedto.point.bottomright.up"
        }
    }
}

/// What the Cutout tool draws (plan-65-E): a rectangle is the dragged box, a
/// slot the box rounded to half its short side, a circle a square as long as
/// the longer drag side, in the drag's direction.
enum CutoutShape: String, CaseIterable, Identifiable {
    case rectangle, circle, slot

    var id: String { rawValue }

    var title: String {
        switch self {
        case .rectangle: "Rectangle"
        case .circle: "Circle"
        case .slot: "Slot"
        }
    }

    /// The shape a drag from `from` to `to` (board mm) makes.
    func shape(from: CGPoint, to: CGPoint) -> BoardCutout.Shape {
        switch self {
        case .rectangle:
            return .rounded(rect: Self.box(from, to), radius: 0)
        case .slot:
            return .slot(rect: Self.box(from, to))
        case .circle:
            let side = max(abs(to.x - from.x), abs(to.y - from.y))
            let end = CGPoint(x: from.x + (to.x >= from.x ? side : -side), y: from.y + (to.y >= from.y ? side : -side))
            let box = Self.box(from, end)
            return .rounded(rect: box, radius: box.width / 2)
        }
    }

    private static func box(_ a: CGPoint, _ b: CGPoint) -> CGRect {
        CGRect(x: min(a.x, b.x), y: min(a.y, b.y), width: abs(a.x - b.x), height: abs(a.y - b.y))
    }
}

/// What a Route tool click names (plan-38-K § 3).
@MainActor
enum RouteTool {
    /// The net of a pad under `point` (one on a shown layer), else the one
    /// connection of an unrouted line within `tolerance` mm, else nil.
    static func scope(_ document: SchematicDocument, at point: CGPoint, tolerance: CGFloat,
                      visible: BoardVisibility = BoardVisibility()) -> RoutingOptions.Scope? {
        let copper = document.connectivity()
        for (p, pad) in copper.pads.enumerated() where pad.pad.plated && pad.pad.rect.contains(point) {
            let shown = pad.pad.drill > 0 ? visible.showsAnyCopper : visible.shows(.copper(pad.side.copperLayer))
            guard shown, let net = copper.netOfPad[p], let key = copper.planeNet(ofNet: net) else { continue }
            return .nets([key])
        }
        guard visible.shows(.ratsnest) else { return nil }
        let line = document.ratsnest().min {
            SchematicDocument.distance(from: point, toSegment: $0.0, $0.1)
                < SchematicDocument.distance(from: point, toSegment: $1.0, $1.1)
        }
        if let line, SchematicDocument.distance(from: point, toSegment: line.0, line.1) <= tolerance {
            return .connection(from: line.0, to: line.1)
        }
        return nil
    }
}
