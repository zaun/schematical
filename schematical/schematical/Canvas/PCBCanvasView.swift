import AppKit
import SwiftUI

/// The PCB page: every placed part that has a footprint, drawn as its pads,
/// drill holes, body outline, pin-1 mark and reference, on a 0.635 mm grid.
/// The view always looks from the top: bottom-side parts and texts are drawn
/// mirrored, under the top-side ones, and a press picks a Top item first.
///
/// Only the layers `visible` shows are drawn (see `BoardVisibility`), back
/// to front: the board and grid; each copper layer from Bottom up to Top,
/// with its Plane tint and, on Top and Bottom, that side's surface-mount
/// pads in the layer's colour (Bottom's marks and texts right after
/// Bottom's copper); through-hole pads in copper; Top's marks and texts;
/// the selection. A press hits only what is drawn.
///
/// With the Select tool, press on a text, trace or footprint (in that order)
/// to select it and drag to move it (snapped to the grid). A plain drag on a
/// lone trace slides the segment under the pointer sideways and the trace
/// follows, its ends and joins staying put; ⌘-drag moves the whole trace as
/// one piece. Shift-click adds an item to the
/// selection or takes it out (plan-15); Shift-drag, or a drag on an item of a
/// larger selection, moves every selected item by one snapped step. Press on
/// empty space to pan, which also clears the selection; Shift-drag empty
/// space to draw a box that adds every drawn item it touches. With
/// the Text tool, a press places a text and goes back to Select. With the
/// Trace tool, a drag draws a trace on `traceLayer` at `traceAngle`
/// (`TraceDrawing`, `TraceDraft`): a live preview in the layer's colour
/// while dragging, a green ring where the end lands, and on release the
/// trace is added, joined and selected. While dragging, as with the
/// schematic's Line tool, Space locks a corner, `/` puts the angled leg
/// first and Esc drops the last lock (or, with none, the trace). The tool
/// stays Trace. With the Via tool, a press places a via (on a trace there,
/// else on the trace grid) and the drag moves it; the tool stays Via. With
/// the Delete tool, a press deletes the text, via or trace under it;
/// footprints are never deleted. Nothing runs while this page
/// shows (`ContentView` stops a run when switching to it).
///
/// Traces draw with their layer's copper, in its colour, at their width. A
/// selected trace gets an accent underlay; the rest of its joined run gets
/// a fainter one.
///
/// Connections the schematic makes that no copper does yet show as thin red
/// lines between their pads (`SchematicDocument.ratsnest`), over the marks.
/// While a trace is drawn, its net's line runs from the trace's end.
///
/// Hovering a drawn pad outlines it and shows its name and net beside the pointer
/// (`SchematicDocument.padLabel`), except during a move, pan or box.
///
/// Board positions are millimetres; the view draws `BoardScale.pointsPerMM`
/// world units per mm and the viewport maps world units to the screen.
struct PCBCanvasView: View {
    let document: SchematicDocument
    let viewport: CanvasViewport
    var tool: PCBTool
    /// The layers to draw (the View popover). Hidden items can't be hit.
    var visible: BoardVisibility
    /// The layer the Trace tool draws on.
    var traceLayer: CopperLayer
    /// What the Cutout tool draws (plan-65).
    var cutoutShape: CutoutShape
    /// The Trace tool's angle (`TraceRoute.angles`), between the straight
    /// run and the angled leg.
    var traceAngle: Int
    /// Whether the Trace tool's live leg walks around other nets' copper
    /// (`AssistRouter`, plan-38-L); an illegal one draws red and a release
    /// there adds nothing.
    var traceAssist: Bool
    /// Called with a hidden layer the Trace tool just drew on; the owner
    /// shows it.
    var onShowLayer: (CopperLayer) -> Void
    /// Called with the layer a V while drawing moved the trace to; the
    /// owner makes it the Trace tool's layer.
    var onLayerChange: (CopperLayer) -> Void
    /// Where the pointer is over the board, for ⌘V; set as it moves.
    var pointer: BoardPointer?
    /// The net Board Details' Nets tab highlights: its pads, traces,
    /// vias and unrouted lines drawn bright over everything else dimmed.
    /// Window state, never saved; Esc clears it.
    var highlightedNet: Binding<PlaneNet?>
    /// The copper an Autoroute run has routed so far, drawn at 60 % in each
    /// layer's colour while it runs; never saved.
    var routePreview: (traces: [RoutedTrace], vias: [RoutedVia])?
    /// Called with what the Route tool clicked: a net, or one connection.
    var onRoute: (RoutingOptions.Scope) -> Void
    /// Called when the Text tool has placed a text; the owner returns to Select.
    var onPlaced: () -> Void
    /// Drawing for File > Export > Image (`ImageExport`): no grid, hover,
    /// tool preview or selection.
    var exporting: Bool
    /// With `exporting`, drill holes are cut out to transparency instead of
    /// filled with the canvas colour.
    var clearsHoles: Bool

    /// The press, pinch, hover and key state; the view's own unless given
    /// (a test drives one).
    var controller: PCBCanvasController?

    @State private var ownController: PCBCanvasController
    @Environment(\.editorEventMonitors) private var eventMonitors

    init(document: SchematicDocument, viewport: CanvasViewport, tool: PCBTool = .select,
         visible: BoardVisibility = BoardVisibility(), traceLayer: CopperLayer = .top,
         cutoutShape: CutoutShape = .rectangle, traceAngle: Int = TraceRoute.defaultAngle, traceAssist: Bool = false,
         onShowLayer: @escaping (CopperLayer) -> Void = { _ in }, onLayerChange: @escaping (CopperLayer) -> Void = { _ in },
         pointer: BoardPointer? = nil, highlightedNet: Binding<PlaneNet?> = .constant(nil),
         routePreview: (traces: [RoutedTrace], vias: [RoutedVia])? = nil,
         onRoute: @escaping (RoutingOptions.Scope) -> Void = { _ in }, onPlaced: @escaping () -> Void = {},
         exporting: Bool = false, clearsHoles: Bool = false, controller: PCBCanvasController? = nil) {
        self.document = document
        self.viewport = viewport
        self.tool = tool
        self.visible = visible
        self.traceLayer = traceLayer
        self.cutoutShape = cutoutShape
        self.traceAngle = traceAngle
        self.traceAssist = traceAssist
        self.onShowLayer = onShowLayer
        self.onLayerChange = onLayerChange
        self.pointer = pointer
        self.highlightedNet = highlightedNet
        self.routePreview = routePreview
        self.onRoute = onRoute
        self.onPlaced = onPlaced
        self.exporting = exporting
        self.clearsHoles = clearsHoles
        self.controller = controller
        _ownController = State(initialValue: PCBCanvasController())
    }

    /// The controller in use.
    var state: PCBCanvasController { controller ?? ownController }

    /// What the drawing reads of the press in progress (`PCBCanvasController`).
    struct DrawState {
        var interaction: Interaction?
        var assist: AssistRouter?
        var drawingLayer: CopperLayer?
    }

    enum Interaction: Equatable {
        /// Moves every selected footprint, text and trace from where they
        /// were at the press.
        case moveGroup(base: BoardGroupBase)
        /// Plain drag on a lone trace: slides the segment under the pointer
        /// (`SchematicDocument.dragTraceSegment`).
        case editTrace(base: TraceSegmentBase)
        case pan(startOffset: CGSize)
        /// Cmd-drag on a reference's text: moves only that reference (of
        /// footprint `id`), from where its centre was at the press (board mm).
        case moveReference(id: UUID, from: CGPoint)
        /// Shift-drag on empty board: a selection box from the press to
        /// `to` (screen points), adding to `base`, the selection at the press.
        case boxSelect(base: Set<BoardItem>, to: CGPoint)
        /// A trace being drawn: its draft (start already anchored, plus any
        /// locked legs) and the pointer at `cursor` (board mm).
        case drawTrace(draft: TraceDraft, cursor: CGPoint)
        /// A handle of the selected pour or keep-out being dragged (plan-52-F).
        case resize(item: BoardItem, handle: PourHandle)
        /// A Pour or Keep-out being dragged out: from where it was pressed to
        /// the pointer (board mm).
        case drawRect(kind: PCBTool, from: CGPoint, to: CGPoint)
        /// The rest of a press that does nothing more: a Delete click, or a
        /// trace dropped with Esc.
        case idle
    }

    static let background = Color(white: 0.13)
    static let copper = Color(red: 0.80, green: 0.62, blue: 0.30)

    /// A pad's copper: a circle, an oval (a stadium: the short side's
    /// semicircle at each end of the long side, as KiCad's and EasyEDA's
    /// ovals are, and as `CopperGeometry.shape(of:)` checks it), a
    /// rectangle, a rectangle with rounded corners, or one with some corners
    /// cut at 45°.
    static func outline(of pad: PlacedPad) -> Path {
        let r = pad.rect
        if pad.shape == .round {
            guard abs(r.width - r.height) > 1e-9 else { return Path(ellipseIn: r) }
            return Path(roundedRect: r, cornerRadius: min(r.width, r.height) / 2)
        }
        if pad.radius > 0 { return Path(roundedRect: r, cornerRadius: pad.radius) }
        guard pad.chamfer > 0, !pad.corners.isEmpty else { return Path(r) }
        let c = pad.chamfer
        // Clockwise from the top-left; a cut corner becomes two points.
        let corners: [(Footprint.Corners, CGPoint, CGPoint, CGPoint)] = [
            (.topLeft, CGPoint(x: r.minX, y: r.minY), CGPoint(x: r.minX, y: r.minY + c), CGPoint(x: r.minX + c, y: r.minY)),
            (.topRight, CGPoint(x: r.maxX, y: r.minY), CGPoint(x: r.maxX - c, y: r.minY), CGPoint(x: r.maxX, y: r.minY + c)),
            (.bottomRight, CGPoint(x: r.maxX, y: r.maxY), CGPoint(x: r.maxX, y: r.maxY - c), CGPoint(x: r.maxX - c, y: r.maxY)),
            (.bottomLeft, CGPoint(x: r.minX, y: r.maxY), CGPoint(x: r.minX + c, y: r.maxY), CGPoint(x: r.minX, y: r.maxY - c)),
        ]
        let points = corners.flatMap { corner, square, before, after in
            pad.corners.contains(corner) ? [before, after] : [square]
        }
        return Path { path in path.addLines(points); path.closeSubpath() }
    }

    /// Screen points per board millimetre.
    var scale: CGFloat { viewport.zoom * BoardScale.pointsPerMM }

    var body: some View {
        let state = state
        let press = state.drawState
        let pressLocation = state.pressLocation
        Canvas { context, size in
            draw(in: &context, size: size, press: press)
        }
        .background(Self.background)
        .overlay(alignment: .topLeading) {
            if case let .boxSelect(_, to)? = press.interaction {
                SelectionBox(from: pressLocation, to: to)
            }
        }
        .overlay(alignment: .topLeading) {
            if !exporting {
                PCBHoverLayer(document: document, viewport: viewport, hover: state.hover, visible: shown,
                              showsPad: state.showsHoveredPad, ringLayer: state.hoverRingLayer(self))
            }
        }
        .contentShape(Rectangle())
        .clipped()
        .onGeometryChange(for: CGSize.self) { $0.size } action: { viewport.updateSize($0) }
        .gesture(
            DragGesture(minimumDistance: 0)
                .onChanged { state.dragChanged(start: $0.startLocation, translation: $0.translation, in: self) }
                .onEnded { state.dragEnded(translation: $0.translation, in: self) }
        )
        .scrollPans(viewport) { state.scrolledMidPress(by: $0, in: self) }
        .simultaneousGesture(
            MagnifyGesture()
                .onChanged { state.pinchChanged($0.magnification, at: $0.startLocation, viewport: viewport) }
                .onEnded { _ in state.pinchEnded() }
        )
        .onContinuousHover { state.hovered(at: Self.location(of: $0), in: self) }
        .onAppear { state.installDrawingKeyMonitor(eventMonitors, in: self) }
        .onDisappear { state.removeDrawingKeyMonitor(eventMonitors) }
        .onChange(of: traceLayer, initial: true) { state.live.layer = traceLayer }
        .onChange(of: traceAngle, initial: true) { state.live.angle = traceAngle }
    }

    /// Where a hover phase has the pointer; nil once it has left.
    static func location(of phase: HoverPhase) -> CGPoint? {
        if case let .active(location) = phase { return location }
        return nil
    }

    /// A screen point in board millimetres.
    func board(_ screen: CGPoint) -> CGPoint {
        let world = viewport.world(screen)
        return CGPoint(x: world.x / BoardScale.pointsPerMM, y: world.y / BoardScale.pointsPerMM)
    }

    /// The board's shape: its outline and every cutout's loop, to be filled
    /// even-odd so the cutouts are holes (plan-65-E).
    static func boardPath(_ edge: BoardEdge) -> Path { Path(edge.path) }

    /// Each pour on a shown layer of the board with the path of its fill
    /// (empty with no net or no copper), for drawing.
    static func pourPaths(pours: [BoardPour], visible: BoardVisibility, board: BoardSettings,
                          copper: BoardConnectivity) -> [(pour: BoardPour, fill: Path)] {
        pours.compactMap { pour in
            guard board.copperLayers.contains(pour.layer), visible.shows(.copper(pour.layer)) else { return nil }
            return (pour, copper.pourFills[pour.id].map { Path($0.path) } ?? Path())
        }
    }

    /// The release of a Pour or Keep-out drag from `from` to `to` (board mm), the
    /// pointer having moved `moved` screen points: makes the item when it was
    /// a drag (3 points or more) and each side is at least `BoardPour.minimumSide`.
    @discardableResult
    static func finishRect(_ document: SchematicDocument, kind: PCBTool, from: CGPoint, to: CGPoint, moved: CGFloat,
                           layer: CopperLayer, cutoutShape: CutoutShape = .rectangle) -> Bool {
        guard moved >= 3 else { return false }
        let rect = CGRect(x: min(from.x, to.x), y: min(from.y, to.y), width: abs(from.x - to.x), height: abs(from.y - to.y))
        guard rect.width >= BoardPour.minimumSide, rect.height >= BoardPour.minimumSide else { return false }
        switch kind {
        case .cutout:
            return document.addCutout(cutoutShape.shape(from: from, to: to)) != nil
        case .pour: return document.addPour(rect: rect, layer: layer) != nil
        case .keepout:
            document.addKeepout(rect: rect)
            return true
        default: return false
        }
    }

    /// The 45° lines of a keep-out's hatch, `spacing` mm apart, inside `rect`.
    static func keepoutHatch(_ rect: CGRect, spacing: CGFloat) -> Path {
        var path = Path()
        var offset = -rect.height
        while offset < rect.width {
            // A line from (offset, height) up to (offset + height, 0), clipped to the rect.
            let a = CGPoint(x: rect.minX + offset, y: rect.maxY), b = CGPoint(x: rect.minX + offset + rect.height, y: rect.minY)
            let t0 = max(0, (rect.minX - a.x) / (b.x - a.x)), t1 = min(1, (rect.maxX - a.x) / (b.x - a.x))
            if t1 > t0 {
                path.move(to: CGPoint(x: a.x + (b.x - a.x) * t0, y: a.y + (b.y - a.y) * t0))
                path.addLine(to: CGPoint(x: a.x + (b.x - a.x) * t1, y: a.y + (b.y - a.y) * t1))
            }
            offset += spacing
        }
        return path
    }

    /// What a Plane on `layer` leaves out, by the rule each part keeps:
    /// `copper`, the copper of every plated through-hole pad and via not on
    /// its net, kept the copper clearance away; `edges`, every non-plated
    /// hole's drill (a round hole or a slot), kept the copper-to-edge
    /// distance away, as the board's own edge is; and `drills`, the bare
    /// drill of each pad and via that joins the Plane, which has no
    /// clearance.
    static func planeCuts(on layer: CopperLayer, copper: BoardConnectivity) -> (copper: Path, edges: Path, drills: Path) {
        let cleared = copper.planeClearances(on: layer)
        let cut = Set(cleared.pads), cutVias = Set(cleared.vias)
        var metal = Path(), edges = Path(), drills = Path()
        func drill(_ pad: PlacedPad) -> Path? {
            pad.holeRect.map { Path(roundedRect: $0, cornerRadius: pad.drill / 2) }
        }
        for (p, entry) in copper.pads.enumerated() where entry.pad.drill > 0 {
            guard let hole = drill(entry.pad) else { continue }
            if !entry.pad.plated {
                edges.addPath(hole)
            } else if cut.contains(p) {
                metal.addPath(outline(of: entry.pad))
            } else {
                drills.addPath(hole)
            }
        }
        for (v, via) in copper.vias.enumerated() {
            if cutVias.contains(v) {
                metal.addPath(Path(ellipseIn: via.rect))
            } else {
                let d = via.drill
                drills.addPath(Path(ellipseIn: CGRect(x: via.centre.x - d / 2, y: via.centre.y - d / 2,
                                                      width: d, height: d)))
            }
        }
        return (metal, edges, drills)
    }

    static func path(_ points: [CGPoint]) -> Path {
        Path { path in path.addLines(points) }
    }

    static func traceStyle(width: CGFloat) -> StrokeStyle {
        StrokeStyle(lineWidth: width, lineCap: .round, lineJoin: .round)
    }

    /// The layers this canvas shows, for the board's current copper layers.
    var shown: BoardVisibility {
        var visible = visible
        visible.copperLayers = document.board.copperLayers
        return visible
    }
}
