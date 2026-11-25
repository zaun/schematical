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
    var tool: PCBTool = .select
    /// The layers to draw (the View popover). Hidden items can't be hit.
    var visible = BoardVisibility()
    /// The layer the Trace tool draws on.
    var traceLayer: CopperLayer = .top
    /// The Trace tool's angle (`TraceRoute.angles`), between the straight
    /// run and the angled leg.
    var traceAngle = TraceRoute.defaultAngle
    /// Whether the Trace tool's live leg walks around other nets' copper
    /// (`AssistRouter`, plan-38-L); an illegal one draws red and a release
    /// there adds nothing.
    var traceAssist = false
    /// Called with a hidden layer the Trace tool just drew on; the owner
    /// shows it.
    var onShowLayer: (CopperLayer) -> Void = { _ in }
    /// Called with the layer a V while drawing moved the trace to; the
    /// owner makes it the Trace tool's layer.
    var onLayerChange: (CopperLayer) -> Void = { _ in }
    /// Where the pointer is over the board, for ⌘V; set as it moves.
    var pointer: BoardPointer?
    /// The net Board Details' Nets tab highlights: its pads, traces,
    /// vias and unrouted lines drawn bright over everything else dimmed.
    /// Window state, never saved; Esc clears it.
    var highlightedNet: Binding<PlaneNet?> = .constant(nil)
    /// The copper an Autoroute run has routed so far, drawn at 60 % in each
    /// layer's colour while it runs; never saved.
    var routePreview: (traces: [RoutedTrace], vias: [RoutedVia])? = nil
    /// Called with what the Route tool clicked: a net, or one connection.
    var onRoute: (RoutingOptions.Scope) -> Void = { _ in }
    /// Called when the Text tool has placed a text; the owner returns to Select.
    var onPlaced: () -> Void = {}
    /// Drawing for File > Export > Image (`ImageExport`): no grid, hover,
    /// tool preview or selection.
    var exporting = false
    /// With `exporting`, drill holes are cut out to transparency instead of
    /// filled with the canvas colour.
    var clearsHoles = false

    @State private var interaction: Interaction?
    /// Zoom when the current pinch began, nil between pinches.
    @State private var pinchStartZoom: CGFloat?
    /// The press in progress as the system reported it, and how far the
    /// canvas has scrolled under it since it began. Cleared when it ends.
    @State private var rawPress: (start: CGPoint, translation: CGSize)?
    @State private var pressShift: CGSize = .zero
    /// Where the pointer is, for the hovered pad and the Trace tool's
    /// landing ring. Only `PCBHoverLayer` reads it, so the pointer moving
    /// redraws that layer and not the board.
    @State private var hover = BoardHover()
    /// Where the current press started, in screen points.
    @State private var pressLocation: CGPoint = .zero
    /// What the current press did to the selection; a click finishes it.
    @State private var selectionPress: SelectionPress<BoardItem>?
    /// Space, `/` and Esc while a trace is being drawn. Installed through
    /// the editor's monitors, which the document empties when it closes.
    @State private var drawingKeyMonitor: UUID?
    @Environment(\.editorEventMonitors) private var eventMonitors
    /// The Trace assist for the trace being drawn, built at its press.
    @State private var assist: AssistRouter?
    /// The layer the trace being drawn is on: `traceLayer` at the press,
    /// then the next layer after each V; nil when no trace is drawn.
    @State private var drawingLayer: CopperLayer?
    /// The Trace tool's layer and angle as they are now, for the key
    /// monitor (`TraceKeyContext`).
    @State private var live = TraceKeyContext()

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
    private var scale: CGFloat { viewport.zoom * BoardScale.pointsPerMM }

    var body: some View {
        Canvas { context, size in
            draw(in: &context, size: size)
        }
        .background(Self.background)
        .overlay(alignment: .topLeading) {
            if case let .boxSelect(_, to)? = interaction {
                SelectionBox(from: pressLocation, to: to)
            }
        }
        .overlay(alignment: .topLeading) {
            if !exporting {
                PCBHoverLayer(document: document, viewport: viewport, hover: hover, visible: shown,
                              showsPad: showsHoveredPad, ringLayer: hoverRingLayer)
            }
        }
        .contentShape(Rectangle())
        .clipped()
        .onGeometryChange(for: CGSize.self) { proxy in
            proxy.size
        } action: { newSize in
            viewport.updateSize(newSize)
        }
        .gesture(
            DragGesture(minimumDistance: 0)
                .onChanged { value in
                    rawPress = (value.startLocation, value.translation)
                    handle(start: value.startLocation, translation: shifted(value.translation))
                }
                .onEnded { value in
                    end(translation: shifted(value.translation))
                    rawPress = nil
                    pressShift = .zero
                }
        )
        .scrollPans(viewport) { scrolledMidPress(by: $0) }
        .simultaneousGesture(
            MagnifyGesture()
                .onChanged { value in
                    let start = pinchStartZoom ?? viewport.zoom
                    pinchStartZoom = start
                    viewport.setZoom(start * value.magnification, around: value.startLocation)
                }
                .onEnded { _ in pinchStartZoom = nil }
        )
        .onContinuousHover { phase in
            switch phase {
            case let .active(location):
                hover.point = board(location)
                hover.location = location
                pointer?.point = hover.point
            case .ended:
                hover.point = nil
                hover.location = nil
                pointer?.point = nil
            }
        }
        .onAppear { installDrawingKeyMonitor() }
        .onDisappear { removeDrawingKeyMonitor() }
        .onChange(of: traceLayer, initial: true) { live.layer = traceLayer }
        .onChange(of: traceAngle, initial: true) { live.angle = traceAngle }
    }

    /// The layer the Trace tool draws on now: the drawn trace's, else the
    /// menu's.
    private var activeLayer: CopperLayer { drawingLayer ?? traceLayer }

    private func installDrawingKeyMonitor() {
        guard drawingKeyMonitor == nil else { return }
        drawingKeyMonitor = eventMonitors.add(matching: .keyDown) { event in
            handleDrawingKey(event) ? nil : event
        }
    }

    private func removeDrawingKeyMonitor() {
        eventMonitors.remove(drawingKeyMonitor)
        drawingKeyMonitor = nil
    }

    /// Space, Esc and `/` while a trace is being drawn. Returns whether the
    /// key was used; any other key, or any key when no trace is being
    /// drawn, is left alone.
    private func handleDrawingKey(_ event: NSEvent) -> Bool {
        guard case let .drawTrace(draft, cursor)? = interaction else {
            // Esc, with no trace being drawn, clears a highlighted net.
            if event.keyCode == 53, highlightedNet.wrappedValue != nil {
                highlightedNet.wrappedValue = nil
                return true
            }
            return false
        }
        var next = draft
        // This closure was made once, at install: the view's own `traceLayer`
        // and `traceAngle` here are those of then; `live` has them now.
        let layer = drawingLayer ?? live.layer
        let plain = event.modifierFlags.intersection([.command, .control, .option]).isEmpty
        if event.keyCode == 53 {
            // Esc with nothing locked drops the whole trace; the rest of the
            // drag does nothing.
            if !next.unlock() { interaction = .idle; return true }
        } else if event.keyCode == 49 {
            if !event.isARepeat {
                let end = TraceDrawing.anchor(document, at: cursor, layer: layer, scale: scale)
                if let assist {
                    // The assist's leg, when it found a legal one.
                    if let leg = assist.route(from: next.legStart, to: end) { next.lock(leg: leg) }
                } else {
                    next.lock(to: end, angle: live.angle)
                }
            }
        } else if event.charactersIgnoringModifiers == "/", plain {
            next.flip()
        } else if event.charactersIgnoringModifiers?.lowercased() == "v", plain,
                  !event.modifierFlags.contains(.shift) {
            // V: a via where the trace has got to, and on from it on the
            // next layer (plan-38-L).
            if !event.isARepeat, let (draft, nextLayer) = TraceDrawing.dropVia(
                document, draft: next, to: cursor, layer: layer, scale: scale, angle: live.angle, assist: assist) {
                next = draft
                drawingLayer = nextLayer
                assist = assist == nil ? nil : AssistRouter(document: document, layer: nextLayer, start: draft.start)
                onLayerChange(nextLayer)
            }
        } else {
            return false
        }
        interaction = .drawTrace(draft: next, cursor: cursor)
        return true
    }

    /// Whether the pad under the pointer is outlined and named: while
    /// nothing is being dragged (a trace being drawn still shows it, at its
    /// end).
    private var showsHoveredPad: Bool {
        switch interaction {
        case nil, .drawTrace?: true
        default: false
        }
    }

    /// The layer whose landing ring follows the pointer: the Trace tool's,
    /// while no trace is being drawn (a drawn trace has its own ring).
    private var hoverRingLayer: CopperLayer? {
        guard tool == .trace else { return nil }
        if case .drawTrace? = interaction { return nil }
        return traceLayer
    }

    /// A screen point in board millimetres.
    private func board(_ screen: CGPoint) -> CGPoint {
        let world = viewport.world(screen)
        return CGPoint(x: world.x / BoardScale.pointsPerMM, y: world.y / BoardScale.pointsPerMM)
    }

    // MARK: Drawing

    private func draw(in context: inout GraphicsContext, size: CGSize) {
        // Everything below is in board mm.
        context.translateBy(x: viewport.offset.width, y: viewport.offset.height)
        context.scaleBy(x: scale, y: scale)
        let board = document.board
        let colour = board.color

        let visible = shown
        let chipPads = document.chipPads
        // The board, under the grid so the dots show on it: a rectangle,
        // or a card's outline with its edge tab. With the PCB layer hidden,
        // only its outline, so the Planes' shading shows on the canvas.
        let boardShape = Path { path in path.addLines(board.outline); path.closeSubpath() }
        if visible.shows(.board) { context.fill(boardShape, with: .color(colour.fill)) }
        context.stroke(boardShape, with: .color(colour.silkscreen), lineWidth: 0.2)
        if !exporting { drawGrid(in: context, size: size) }

        // The copper's nets, read only when a Plane is drawn.
        let planeCopper = board.copperLayers.contains { visible.showsPlane($0, style: board.style(of: $0)) }
            || !document.pours.isEmpty ? document.connectivity() : nil
        let pourDraws = planeCopper.map {
            Self.pourPaths(pours: document.pours, visible: visible, board: board, copper: $0)
        } ?? []
        let rules = board.rules
        // Copper from Bottom up to Top: each layer's plane tint, then the
        // surface-mount pads of the side it is on. Bottom's marks and texts
        // go straight after Bottom's copper, under everything else.
        for layer in board.copperLayers.reversed() {
            let style = board.style(of: layer)
            for (pour, fill) in pourDraws where pour.layer == layer {
                if !fill.isEmpty {
                    // Even-odd: each piece is its outer loop and its holes, whatever their winding.
                    context.fill(fill, with: .color(style.color.color.opacity(visible.shows(.board) ? 0.35 : 0.5)),
                                 style: FillStyle(eoFill: true))
                }
                if !exporting {
                    context.stroke(Path(pour.rect), with: .color(style.color.color),
                                   style: StrokeStyle(lineWidth: 1 / scale, dash: [4 / scale, 3 / scale]))
                }
            }
            if visible.showsPlane(layer, style: style), let planeCopper {
                drawPlane(on: layer, colour: style.color.color, shape: boardShape, copper: planeCopper,
                          clearance: style.clearance(rules: rules), edge: rules.minCopperToEdge,
                          opacity: visible.shows(.board) ? 0.2 : 0.45, in: &context)
            }
            if visible.showsTrace(on: layer) {
                drawTraces(on: layer, colour: style.color.color, in: &context)
            }
            let side: BoardSide? = layer == .top ? .top : layer == .bottom ? .bottom : nil
            if let side, visible.shows(.copper(layer)) {
                // A card's gold fingers on this side.
                for finger in board.fingers where !finger.isThroughHole && finger.side == side {
                    context.fill(Path(finger.rect), with: .color(style.color.color))
                }
                for (chip, pads) in zip(document.chips, chipPads) {
                    guard let placed = chip.board?.side else { continue }
                    for pad in pads where pad.drill == 0 && pad.layerSide(on: placed) == side {
                        context.fill(Self.outline(of: pad), with: .color(style.color.color))
                    }
                }
            }
            if side == .bottom {
                drawMarksAndTexts(on: .bottom, in: &context, colour: colour, visible: visible)
            }
        }
        // A 1-layer board has no Bottom layer, and nothing on its bottom.

        // Keep-outs: design marks, not copper, so not in an image export.
        if !exporting {
            for keepout in document.keepouts where document.keepoutIsShown(keepout, visible) {
                context.stroke(Self.keepoutHatch(keepout.rect, spacing: 1), with: .color(.red.opacity(0.15)),
                               lineWidth: 1 / scale)
                context.stroke(Path(keepout.rect), with: .color(.red),
                               style: StrokeStyle(lineWidth: 1 / scale, dash: [4 / scale, 3 / scale]))
            }
        }
        // The handles of the one selected pour or keep-out.
        if !exporting, let rect = document.resizableRect {
            let side = 8 / scale
            for handle in PourHandle.allCases {
                let at = handle.position(in: rect)
                let box = Path(CGRect(x: at.x - side / 2, y: at.y - side / 2, width: side, height: side))
                context.fill(box, with: .color(.white))
                context.stroke(box, with: .color(.accentColor), lineWidth: 1.5 / scale)
            }
        }
        if case let .drawRect(_, from, to) = interaction {
            let band = CGRect(x: min(from.x, to.x), y: min(from.y, to.y), width: abs(from.x - to.x), height: abs(from.y - to.y))
            context.stroke(Path(band), with: .color(.accentColor),
                           style: StrokeStyle(lineWidth: 1 / scale, dash: [4 / scale, 3 / scale]))
        }

        // Through-hole pads and vias go through every layer: copper, with
        // the canvas showing through the drill.
        if visible.showsAnyCopper {
            for via in document.vias {
                context.fill(Path(ellipseIn: via.rect), with: .color(Self.copper))
                let d = via.drill
                let hole = CGRect(x: via.centre.x - d / 2, y: via.centre.y - d / 2, width: d, height: d)
                fillHole(Path(ellipseIn: hole), in: &context)
            }
            // The board's own header pads and mounting holes, and the
            // drilled holes.
            let boardPads = (board.fingers.filter(\.isThroughHole) + board.extraPads).map(\.pad) + board.holes.map(\.pad)
                + document.holes.map(\.pad)
            for pad in boardPads + chipPads.flatMap({ $0.filter { $0.drill > 0 } }) {
                // A non-plated hole has no copper: just the hole.
                if pad.plated { context.fill(Self.outline(of: pad), with: .color(Self.copper)) }
                if let hole = pad.holeRect {
                    fillHole(Path(roundedRect: hole, cornerRadius: pad.drill / 2), in: &context)
                }
            }
        }

        drawMarksAndTexts(on: .top, in: &context, colour: colour, visible: visible)

        if visible.shows(.ratsnest) { drawRatsnest(in: &context) }
        // An export has no pointer, highlight or selection.
        if !exporting, let key = highlightedNet.wrappedValue {
            drawHighlight(of: key, visible: visible, in: &context)
        }
        if !exporting, let routePreview { drawRoutePreview(routePreview, in: &context) }
        drawTraceTool(in: &context)
        guard !exporting else { return }

        // The hovered pad's outline and the Trace tool's ring at the
        // pointer are `PCBHoverLayer`'s, over this canvas.

        // Selection boxes, one per selected item that is drawn, on top of
        // everything. A selected item on a hidden layer stays selected.
        let margin = 3 / scale
        for item in document.boardSelection.items {
            let selected: CGRect?
            switch item {
            case let .footprint(id):
                selected = document.chips.first { $0.id == id && visible.showsFootprint($0) }?.boardBounds
            case let .text(id):
                selected = document.texts.first { $0.id == id && visible.showsText($0) }?.boardRect
            case .trace:
                // A selected trace has its own underlay (`drawTraces`).
                selected = nil
            case let .via(id):
                // A ring just outside the via's metal.
                if visible.showsAnyCopper, let via = document.vias.first(where: { $0.id == id }) {
                    context.stroke(Path(ellipseIn: via.rect.insetBy(dx: -margin, dy: -margin)),
                                   with: .color(.accentColor), lineWidth: 2 / scale)
                }
                selected = nil
            case let .hole(id):
                // A ring just outside the hole's land (or bare hole).
                if visible.showsAnyCopper, let hole = document.holes.first(where: { $0.id == id }) {
                    context.stroke(Path(ellipseIn: hole.rect.insetBy(dx: -margin, dy: -margin)),
                                   with: .color(.accentColor), lineWidth: 2 / scale)
                }
                selected = nil
            case let .pour(id):
                selected = document.pours.first(where: { $0.id == id && visible.shows(.copper($0.layer)) })?.rect
            case let .keepout(id):
                selected = document.keepouts.first(where: { $0.id == id })?.rect
            case let .cutout(id):
                selected = document.cutouts.first(where: { $0.id == id })?.shape.bounds
            }
            if let selected {
                context.stroke(Path(selected.insetBy(dx: -margin, dy: -margin)),
                               with: .color(.accentColor), lineWidth: 2 / scale)
            }
        }
    }

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
                           layer: CopperLayer) -> Bool {
        guard moved >= 3 else { return false }
        let rect = CGRect(x: min(from.x, to.x), y: min(from.y, to.y), width: abs(from.x - to.x), height: abs(from.y - to.y))
        guard rect.width >= BoardPour.minimumSide, rect.height >= BoardPour.minimumSide else { return false }
        switch kind {
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

    /// A Plane layer: its tint (`opacity`) over the board, pulled back from the
    /// board's edge, with `planeCuts` cleared out of it: the copper not on
    /// its net by the Plane's `clearance` (`LayerStyle.clearance(rules:)`),
    /// non-plated holes and the edge by `edge` (the DRC's min copper to
    /// edge), and the drills of what joins it. Plated
    /// pads and vias on its net join the Plane solid round their drill.
    private func drawPlane(on layer: CopperLayer, colour: Color, shape: Path, copper: BoardConnectivity,
                           clearance: Double, edge: Double, opacity: Double, in context: inout GraphicsContext) {
        let cuts = Self.planeCuts(on: layer, copper: copper)
        context.drawLayer { plane in
            plane.fill(shape, with: .color(colour.opacity(opacity)))
            plane.blendMode = .clear
            // A clearance is the cut's outline grown by it all round: a
            // stroke twice as wide, with round joins, over the filled cut.
            func clear(_ path: Path, by distance: Double) {
                guard !path.isEmpty else { return }
                plane.fill(path, with: .color(.black))
                if distance > 0 {
                    plane.stroke(path, with: .color(.black), style: StrokeStyle(lineWidth: 2 * distance, lineJoin: .round))
                }
            }
            if edge > 0 {
                plane.stroke(shape, with: .color(.black), style: StrokeStyle(lineWidth: 2 * edge, lineJoin: .round))
            }
            clear(cuts.copper, by: clearance)
            clear(cuts.edges, by: edge)
            clear(cuts.drills, by: 0)
        }
    }

    /// The traces on one layer, after their selection underlays: an accent
    /// stroke 4 points wider under each selected trace, and a fainter one
    /// under the other traces joined to it.
    private func drawTraces(on layer: CopperLayer, colour: Color, in context: inout GraphicsContext) {
        let onLayer = document.traces.filter { $0.layer == layer }
        guard !onLayer.isEmpty else { return }
        let selected = exporting ? [] : Set(document.boardSelection.items.compactMap { item -> UUID? in
            if case let .trace(id) = item { return id }
            return nil
        })
        var joined = Set<UUID>()
        for id in selected where onLayer.contains(where: { $0.id == id }) {
            joined.formUnion(document.connectedRun(of: id))
        }
        joined.subtract(selected)
        for trace in onLayer where selected.contains(trace.id) || joined.contains(trace.id) {
            let opacity = selected.contains(trace.id) ? 1 : 0.35
            context.stroke(Self.path(trace.points), with: .color(.accentColor.opacity(opacity)),
                           style: Self.traceStyle(width: trace.widthMM + 4 / scale))
        }
        for trace in onLayer {
            context.stroke(Self.path(trace.points), with: .color(colour), style: Self.traceStyle(width: trace.widthMM))
        }
    }

    /// A thin red line for each connection the schematic makes that no
    /// copper does yet (`SchematicDocument.ratsnest`). While a trace is
    /// being drawn it counts, so its net's line runs from where it ends.
    private func drawRatsnest(in context: inout GraphicsContext) {
        var drawing: BoardTrace?
        if tool == .trace, case let .drawTrace(draft, cursor) = interaction {
            let (route, allowed) = TraceDrawing.assisted(document, draft: draft, to: cursor, layer: activeLayer,
                                                         scale: scale, angle: traceAngle, assist: assist)
            if allowed, TraceRoute.isLegal(route) {
                drawing = BoardTrace(id: UUID(), layer: activeLayer,
                                     width: TraceDrawing.width(document, draft: draft, layer: activeLayer), points: route)
            }
        }
        let lines = document.ratsnest(drawing: drawing)
        guard !lines.isEmpty else { return }
        let path = Path { path in
            for (a, b) in lines {
                path.move(to: a)
                path.addLine(to: b)
            }
        }
        context.stroke(path, with: .color(.red), lineWidth: 1 / scale)
    }

    /// A highlighted net: everything drawn so far dimmed to about a third,
    /// then the net's pads, traces and vias in white and its unrouted lines
    /// in red, over it. A net the schematic no longer has dims nothing.
    private func drawHighlight(of key: PlaneNet, visible: BoardVisibility, in context: inout GraphicsContext) {
        let copper = document.connectivity()
        guard let net = copper.net(of: key) else { return }
        let items = document.copperItems(ofNet: net, in: copper)
        let world = CGRect(x: -100_000, y: -100_000, width: 200_000, height: 200_000)
        context.fill(Path(world), with: .color(Self.background.opacity(0.65)))
        let bright = Color.white.opacity(0.95)
        for trace in copper.traces where items.traces.contains(trace.id) && visible.showsTrace(on: trace.layer) {
            context.stroke(Self.path(trace.points), with: .color(bright), style: Self.traceStyle(width: trace.widthMM))
        }
        if visible.showsAnyCopper {
            for pad in items.pads.sorted() {
                context.fill(Self.outline(of: copper.pads[pad].pad), with: .color(bright))
            }
            for via in copper.vias where items.vias.contains(via.id) {
                context.fill(Path(ellipseIn: via.rect), with: .color(bright))
            }
        }
        if visible.shows(.ratsnest) {
            let lines = document.ratsnest(in: copper, net: net)
            let path = Path { path in
                for (a, b) in lines {
                    path.move(to: a)
                    path.addLine(to: b)
                }
            }
            context.stroke(path, with: .color(.red), lineWidth: 2 / scale)
        }
    }

    /// An Autoroute run's copper so far, at 60 % in each layer's colour.
    private func drawRoutePreview(_ preview: (traces: [RoutedTrace], vias: [RoutedVia]),
                                  in context: inout GraphicsContext) {
        let board = document.board
        let visible = shown
        for trace in preview.traces where visible.showsTrace(on: trace.layer) {
            let colour = board.style(of: trace.layer).color.color.opacity(0.6)
            context.stroke(Self.path(trace.points), with: .color(colour), style: Self.traceStyle(width: trace.width))
        }
        guard visible.showsAnyCopper else { return }
        for via in preview.vias {
            let rect = CGRect(x: via.centre.x - via.diameter / 2, y: via.centre.y - via.diameter / 2,
                              width: via.diameter, height: via.diameter)
            context.fill(Path(ellipseIn: rect), with: .color(Self.copper.opacity(0.6)))
        }
    }

    /// The Trace tool's preview while dragging, and the green ring where a
    /// release would land. (Before a press, the ring follows the pointer:
    /// `PCBHoverLayer`.)
    private func drawTraceTool(in context: inout GraphicsContext) {
        guard tool == .trace else { return }
        let ring: CGPoint?
        var legal = true
        if case let .drawTrace(draft, cursor) = interaction {
            let (route, allowed) = TraceDrawing.assisted(document, draft: draft, to: cursor, layer: activeLayer,
                                                         scale: scale, angle: traceAngle, assist: assist)
            legal = allowed
            let mil = TraceDrawing.width(document, draft: draft, layer: activeLayer)
            let width = Double(mil) * 0.0254
            if allowed {
                let colour = document.board.style(of: activeLayer).color.color.opacity(0.6)
                context.stroke(Self.path(route), with: .color(colour), style: Self.traceStyle(width: width))
            } else {
                // Where the assist found no legal way (a release adds
                // nothing): red and dashed, so it reads on a red Top layer too.
                let dash = max(width * 3, 4 / scale)
                context.stroke(Self.path(route), with: .color(.red),
                               style: StrokeStyle(lineWidth: max(width, 2 / scale), lineCap: .butt,
                                                  dash: [dash, dash]))
            }
            ring = route.last
        } else {
            ring = nil
        }
        if let ring {
            // Green where a release would land; a red cross where it can't.
            let r = 6 / scale
            if legal {
                context.stroke(Path(ellipseIn: CGRect(x: ring.x - r, y: ring.y - r, width: 2 * r, height: 2 * r)),
                               with: .color(.green), lineWidth: 2 / scale)
            } else {
                let cross = Path { path in
                    path.move(to: CGPoint(x: ring.x - r, y: ring.y - r)); path.addLine(to: CGPoint(x: ring.x + r, y: ring.y + r))
                    path.move(to: CGPoint(x: ring.x - r, y: ring.y + r)); path.addLine(to: CGPoint(x: ring.x + r, y: ring.y - r))
                }
                context.stroke(cross, with: .color(.red), lineWidth: 2.5 / scale)
            }
        }
    }

    /// A drill hole: the canvas colour, or (`clearsHoles`) cut through.
    private func fillHole(_ path: Path, in context: inout GraphicsContext) {
        if clearsHoles {
            var cut = context
            cut.blendMode = .destinationOut
            cut.fill(path, with: .color(.black))
        } else {
            context.fill(path, with: .color(Self.background))
        }
    }

    /// The board as File > Export > Image draws it (`exporting`), in a
    /// picture `size` points big.
    func drawExport(in context: inout GraphicsContext, size: CGSize) {
        draw(in: &context, size: size)
    }

    static func path(_ points: [CGPoint]) -> Path {
        Path { path in path.addLines(points) }
    }

    static func traceStyle(width: CGFloat) -> StrokeStyle {
        StrokeStyle(lineWidth: width, lineCap: .round, lineJoin: .round)
    }

    /// The layers this canvas shows, for the board's current copper layers.
    private var shown: BoardVisibility {
        var visible = visible
        visible.copperLayers = document.board.copperLayers
        return visible
    }

    /// One side's footprint marks, then its texts.
    private func drawMarksAndTexts(on side: BoardSide, in context: inout GraphicsContext, colour: BoardColor,
                                   visible: BoardVisibility) {
        for chip in document.chips where chip.board?.side == side {
            drawMarks(chip, in: &context, colour: colour, visible: visible)
        }
        for text in document.texts where text.side == side && visible.showsText(text) {
            drawText(text, in: &context, colour: colour)
        }
    }

    /// A footprint's marks: body outline, pin-1 mark and reference, each
    /// where its layer is shown. On Bottom the geometry is already mirrored
    /// (see `BoardPlacement`), and the reference reads backwards.
    private func drawMarks(_ chip: PlacedChip, in context: inout GraphicsContext, colour: BoardColor,
                           visible: BoardVisibility) {
        guard let body = chip.boardBody, let placement = chip.board else { return }
        let side = placement.side
        if visible.showsMark(placement.outline, side: side), let ink = placement.outline.color(on: colour) {
            let outline = Path { path in
                if body.round {
                    path.addEllipse(in: body.rect)
                } else {
                    path.addRect(body.rect)
                }
                if let band = chip.cathodeBand {
                    path.move(to: band.a)
                    path.addLine(to: band.b)
                }
            }
            context.stroke(outline, with: .color(ink), lineWidth: 0.15)
        }

        if let mark = chip.pin1Mark, visible.showsMark(placement.pin1, side: side),
           let ink = placement.pin1.color(on: colour) {
            context.fill(Path(ellipseIn: CGRect(x: mark.x - 0.2, y: mark.y - 0.2, width: 0.4, height: 0.4)),
                         with: .color(ink))
        }

        // Reference: 1 mm tall, but never under 8 points on screen. Mirrored
        // on Bottom, as a real bottom silkscreen reads from above.
        if visible.showsMark(placement.reference, side: side), let ink = placement.reference.color(on: colour),
           let at = chip.boardReferenceCentre {
            let height = max(1, 8 / scale)
            let text = Text(chip.reference)
                .font(.system(size: height, weight: .medium))
                .foregroundStyle(ink)
            context.drawLayer { layer in
                layer.translateBy(x: at.x, y: at.y)
                if side == .bottom { layer.scaleBy(x: -1, y: 1) }
                layer.draw(text, at: .zero, anchor: .center)
            }
        }
    }

    /// A text: laid out at the reference font size, scaled to the board (see
    /// BoardTextGeometry), turned about its anchor. On Bottom it is mirrored
    /// inside its own box, so the box doesn't move.
    private func drawText(_ text: BoardText, in context: inout GraphicsContext, colour: BoardColor) {
        let width = text.side == .bottom ? text.localSize.width : 0
        context.drawLayer { layer in
            layer.translateBy(x: text.anchor.x, y: text.anchor.y)
            layer.rotate(by: .degrees(Double(text.rotation)))
            if text.side == .bottom {
                layer.translateBy(x: width, y: 0)
                layer.scaleBy(x: -1, y: 1)
            }
            layer.scaleBy(x: text.drawScale, y: text.drawScale)
            layer.draw(Text(text.text).font(Font(text.font)).foregroundStyle(text.layer.color(on: colour)),
                       at: .zero, anchor: .topLeading)
        }
    }

    /// Dots every 4 component-grid steps (2.54 mm on the default grid),
    /// hidden when they would be closer than 8 points on screen. The trace
    /// grid gets no dots.
    private func drawGrid(in context: GraphicsContext, size: CGSize) {
        let step = document.board.grid * 4
        guard step * scale >= 8 else { return }
        let topLeft = board(.zero)
        let bottomRight = board(CGPoint(x: size.width, y: size.height))
        let dot = 2.5 / scale
        var dots = Path()
        var x = (topLeft.x / step).rounded(.down) * step
        while x <= bottomRight.x {
            var y = (topLeft.y / step).rounded(.down) * step
            while y <= bottomRight.y {
                dots.addEllipse(in: CGRect(x: x - dot / 2, y: y - dot / 2, width: dot, height: dot))
                y += step
            }
            x += step
        }
        context.fill(dots, with: .color(Color(white: 0.42)))
    }

    // MARK: Gestures

    /// `translation` less the scroll since the press began: the drag as if the
    /// canvas had stayed put.
    private func shifted(_ translation: CGSize) -> CGSize {
        CGSize(width: translation.width - pressShift.width, height: translation.height - pressShift.height)
    }

    /// The canvas scrolled by `shift` while a press is down: the press's start
    /// moves with the content, and the drag's effect is worked out again for
    /// the pointer's unchanged place.
    private func scrolledMidPress(by shift: CGSize) {
        guard let raw = rawPress, interaction != nil else { return }
        if case let .pan(startOffset)? = interaction {
            // A pan is the offset itself: its start moves with the scroll.
            interaction = .pan(startOffset: CGSize(width: startOffset.width + shift.width,
                                                   height: startOffset.height + shift.height))
        } else {
            pressShift = CGSize(width: pressShift.width + shift.width, height: pressShift.height + shift.height)
            pressLocation = CGPoint(x: pressLocation.x + shift.width, y: pressLocation.y + shift.height)
        }
        handle(start: raw.start, translation: shifted(raw.translation))
    }

    /// One step of a drag that started at `start` (screen points) and has
    /// moved by `translation`. On the first step it decides what the drag
    /// does: the Text tool places a text there (and the drag moves it); the
    /// Trace tool starts a trace; the Select tool picks a text, else a
    /// trace, else a footprint, else clears the selection and pans. Returns
    /// what the drag is doing.
    @discardableResult
    func handle(start: CGPoint, translation: CGSize) -> Interaction {
        if interaction == nil {
            pressLocation = start
            // The whole press is one undo step (plan-31-C).
            document.beginGesture(.pcb)
        }
        let current = interaction ?? begin(at: start)
        interaction = current
        let offset = CGSize(width: translation.width / scale, height: translation.height / scale)
        switch current {
        case let .moveGroup(base):
            document.moveBoardGroup(base, by: offset)
        case let .editTrace(base):
            document.dragTraceSegment(base, by: offset)
        case let .moveReference(id, from):
            ReferenceDragging.drag(document, id: id, from: from, by: offset)
        case let .pan(startOffset):
            viewport.pan(to: CGSize(
                width: startOffset.width + translation.width,
                height: startOffset.height + translation.height
            ))
        case let .boxSelect(base, _):
            let to = CGPoint(x: pressLocation.x + translation.width, y: pressLocation.y + translation.height)
            interaction = .boxSelect(base: base, to: to)
            let a = board(pressLocation), b = board(to)
            document.boxSelectBoard(
                CGRect(x: min(a.x, b.x), y: min(a.y, b.y), width: abs(a.x - b.x), height: abs(a.y - b.y)),
                base: base, visible: shown
            )
            return .boxSelect(base: base, to: to)
        case let .drawTrace(draft, _):
            let cursor = board(CGPoint(x: pressLocation.x + translation.width, y: pressLocation.y + translation.height))
            interaction = .drawTrace(draft: draft, cursor: cursor)
            return .drawTrace(draft: draft, cursor: cursor)
        case let .resize(item, handle):
            document.resizePourItem(item, handle: handle,
                                    to: board(CGPoint(x: pressLocation.x + translation.width,
                                                      y: pressLocation.y + translation.height)))
        case let .drawRect(kind, from, _):
            let cursor = board(CGPoint(x: pressLocation.x + translation.width, y: pressLocation.y + translation.height))
            interaction = .drawRect(kind: kind, from: from, to: cursor)
            return .drawRect(kind: kind, from: from, to: cursor)
        case .idle:
            break
        }
        return current
    }

    /// Decides what a press does. Shift is read once, here, at the start.
    private func begin(at start: CGPoint) -> Interaction {
        let point = board(start)
        let shift = NSEvent.modifierFlags.contains(.shift)
        switch tool {
        case .trace:
            let anchored = TraceDrawing.anchor(document, at: point, layer: traceLayer, scale: scale)
            // The assist's grid is built once, here (plan-38-L).
            assist = traceAssist ? AssistRouter(document: document, layer: traceLayer, start: anchored) : nil
            drawingLayer = traceLayer
            return .drawTrace(draft: TraceDraft(start: anchored), cursor: point)
        case .via:
            // A press places a via (snapped onto a trace, or the trace grid)
            // and the drag moves it. Nothing on a 1-layer board.
            let anchored = document.viaAnchor(at: point, tolerance: TraceDrawing.tolerance / scale)
            guard document.addVia(at: anchored) != nil else { return .idle }
            return .moveGroup(base: document.boardGroupBase())
        case .hole:
            // A press places a hole on the trace grid, and the drag moves it.
            document.addHole(at: point)
            return .moveGroup(base: document.boardGroupBase())
        case .pour, .keepout:
            // The drag is the rectangle; a release without one makes nothing.
            return .drawRect(kind: tool, from: point, to: point)
        case .route:
            // A click routes the net of the pad, or the one connection of
            // the unrouted line, under it.
            if let scope = RouteTool.scope(document, at: point, tolerance: TraceDrawing.tolerance / scale, visible: shown) {
                onRoute(scope)
            }
            return .idle
        case .delete:
            // One deletion per press: the text, via, hole or trace where it
            // started.
            document.deleteBoardItem(at: point, tolerance: TraceDrawing.tolerance / scale, visible: shown)
            return .idle
        case .text:
            // The new text is the whole selection, and the drag moves it.
            document.addText(at: point)
            onPlaced()
            return .moveGroup(base: document.boardGroupBase())
        case .select:
            // Cmd-press on a reference's text: the drag moves only the reference.
            if NSEvent.modifierFlags.contains(.command),
               let press = ReferenceDragging.press(document, at: point, scale: scale, visible: shown) {
                return .moveReference(id: press.id, from: press.from)
            }
            if let hit = document.pourHandle(at: point, tolerance: 6 / scale) {
                return .resize(item: hit.item, handle: hit.handle)
            }
            if let item = document.boardItem(at: point, tolerance: TraceDrawing.tolerance / scale, visible: shown) {
                let press = document.pressSelection(item, shift: shift)
                selectionPress = press
                // A plain drag on a lone trace edits it, sliding the segment
                // under the pointer; Cmd-drag moves the trace as one piece.
                if case let .trace(id) = item, !press.dragsGroup, !NSEvent.modifierFlags.contains(.command),
                   let base = document.traceSegmentBase(id, at: point) {
                    return .editTrace(base: base)
                }
                return .moveGroup(base: document.boardGroupBase(grab: point))
            }
            if shift {
                return .boxSelect(base: document.boardSelection.items, to: start)
            }
            document.pressEmptyBoard(shift: false)
            return .pan(startOffset: viewport.offset)
        }
    }

    /// The end of a press: a click (under 3 points of movement) finishes
    /// what the press left pending in the selection.
    private func end(translation: CGSize) {
        if case let .drawTrace(draft, cursor) = interaction {
            TraceDrawing.finish(document, draft: draft, to: cursor, layer: activeLayer, scale: scale,
                                angle: traceAngle, visible: shown, assist: assist, onShowLayer: onShowLayer)
        }
        if case let .drawRect(kind, from, to) = interaction {
            Self.finishRect(document, kind: kind, from: from, to: to,
                            moved: hypot(translation.width, translation.height), layer: traceLayer)
        }
        assist = nil
        drawingLayer = nil
        if case let .moveGroup(base) = interaction {
            document.settleBoardGroup(base)
        }
        if case let .editTrace(base) = interaction {
            document.settleTraceSegment(base)
        }
        if let press = selectionPress {
            document.releaseSelection(press, wasClick: hypot(translation.width, translation.height) < 3)
        }
        selectionPress = nil
        interaction = nil
        if document.isGestureOpen { document.endGesture() }
    }
}

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
private struct PCBHoverLayer: View {
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
private struct PadLabel: View {
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
        case .delete: "Delete — click a trace, via, hole or text to delete it (footprints stay)"
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
        case .delete: "eraser"
        case .route: "point.topleft.down.curvedto.point.bottomright.up"
        }
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
