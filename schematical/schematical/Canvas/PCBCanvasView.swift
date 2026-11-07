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
/// With the Select tool, press on a text or footprint to select it and drag
/// to move it (snapped to the grid). Shift-click adds an item to the
/// selection or takes it out (plan-15); Shift-drag, or a drag on an item of a
/// larger selection, moves every selected item by one snapped step. Press on
/// empty space to pan; without Shift that also clears the selection. With
/// the Text tool, a press places a text and goes back to Select. With the
/// Trace tool, a drag draws a 45° trace on `traceLayer` (`TraceDrawing`):
/// a live preview in the layer's colour while dragging, a green ring where
/// the end lands, and on release the trace is added, joined and selected.
/// The tool stays Trace. Option puts the diagonal first. Nothing runs while
/// this page shows (`ContentView` stops a run when switching to it).
///
/// Traces draw with their layer's copper, in its colour, at their width. A
/// selected trace gets an accent underlay; the rest of its joined run gets
/// a fainter one.
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
    /// Called with a hidden layer the Trace tool just drew on; the owner
    /// shows it.
    var onShowLayer: (CopperLayer) -> Void = { _ in }
    /// Called when the Text tool has placed a text; the owner returns to Select.
    var onPlaced: () -> Void = {}

    @State private var interaction: Interaction?
    /// Where the pointer is (board mm), for the Trace tool's landing ring.
    @State private var hoverPoint: CGPoint?
    /// Where the current press started, in screen points.
    @State private var pressLocation: CGPoint = .zero
    /// What the current press did to the selection; a click finishes it.
    @State private var selectionPress: SelectionPress<BoardItem>?

    enum Interaction: Equatable {
        /// Moves every selected footprint and text from where they were at
        /// the press.
        case moveGroup(base: BoardGroupBase)
        case pan(startOffset: CGSize)
        /// A trace being drawn from `start` (already anchored) towards the
        /// pointer at `cursor` (board mm).
        case drawTrace(start: CGPoint, cursor: CGPoint)
    }

    static let background = Color(white: 0.13)
    static let copper = Color(red: 0.80, green: 0.62, blue: 0.30)

    /// A pad's copper: an ellipse, a rectangle, a rectangle with rounded
    /// corners, or one with some corners cut at 45°.
    static func outline(of pad: PlacedPad) -> Path {
        let r = pad.rect
        if pad.shape == .round { return Path(ellipseIn: r) }
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
        .contentShape(Rectangle())
        .clipped()
        .onGeometryChange(for: CGSize.self) { proxy in
            proxy.size
        } action: { newSize in
            viewport.updateSize(newSize)
        }
        .gesture(
            DragGesture(minimumDistance: 0)
                .onChanged { value in handle(start: value.startLocation, translation: value.translation) }
                .onEnded { value in end(translation: value.translation) }
        )
        .onContinuousHover { phase in
            switch phase {
            case let .active(location): hoverPoint = board(location)
            case .ended: hoverPoint = nil
            }
        }
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

        // The board, under the grid so the dots show on it.
        let boardRect = CGRect(x: 0, y: 0, width: board.width, height: board.height)
        context.fill(Path(boardRect), with: .color(colour.fill))
        context.stroke(Path(boardRect), with: .color(colour.silkscreen), lineWidth: 0.2)
        drawGrid(in: context, size: size)

        let visible = shown
        // Copper from Bottom up to Top: each layer's plane tint, then the
        // surface-mount pads of the side it is on. Bottom's marks and texts
        // go straight after Bottom's copper, under everything else.
        for layer in board.copperLayers.reversed() {
            let style = board.style(of: layer)
            if visible.showsPlane(layer, style: style) {
                context.fill(Path(boardRect), with: .color(style.color.color.opacity(0.2)))
            }
            if visible.showsTrace(on: layer) {
                drawTraces(on: layer, colour: style.color.color, in: &context)
            }
            let side: BoardSide? = layer == .top ? .top : layer == .bottom ? .bottom : nil
            if let side, visible.shows(.copper(layer)) {
                for chip in document.chips where chip.board?.side == side {
                    for pad in chip.boardPads where pad.drill == 0 {
                        context.fill(Self.outline(of: pad), with: .color(style.color.color))
                    }
                }
            }
            if side == .bottom {
                drawMarksAndTexts(on: .bottom, in: &context, colour: colour, visible: visible)
            }
        }
        // A 1-layer board has no Bottom layer, and nothing on its bottom.

        // Through-hole pads go through every layer: copper, with the canvas
        // showing through the drill.
        if visible.showsAnyCopper {
            for chip in document.chips {
                for pad in chip.boardPads where pad.drill > 0 {
                    context.fill(Self.outline(of: pad), with: .color(Self.copper))
                    let d = pad.drill
                    let hole = CGRect(x: pad.rect.midX - d / 2, y: pad.rect.midY - d / 2, width: d, height: d)
                    context.fill(Path(ellipseIn: hole), with: .color(Self.background))
                }
            }
        }

        drawMarksAndTexts(on: .top, in: &context, colour: colour, visible: visible)

        drawTraceTool(in: &context)

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
            }
            if let selected {
                context.stroke(Path(selected.insetBy(dx: -margin, dy: -margin)),
                               with: .color(.accentColor), lineWidth: 2 / scale)
            }
        }
    }

    /// The traces on one layer, after their selection underlays: an accent
    /// stroke 4 points wider under each selected trace, and a fainter one
    /// under the other traces joined to it.
    private func drawTraces(on layer: CopperLayer, colour: Color, in context: inout GraphicsContext) {
        let onLayer = document.traces.filter { $0.layer == layer }
        guard !onLayer.isEmpty else { return }
        let selected = Set(document.boardSelection.items.compactMap { item -> UUID? in
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

    /// The Trace tool's preview while dragging, and the green ring where a
    /// press or release would land.
    private func drawTraceTool(in context: inout GraphicsContext) {
        guard tool == .trace else { return }
        let ring: CGPoint?
        if case let .drawTrace(start, cursor) = interaction {
            let route = TraceDrawing.preview(document, from: start, to: cursor, layer: traceLayer, scale: scale,
                                             diagonalFirst: NSEvent.modifierFlags.contains(.option))
            let colour = document.board.style(of: traceLayer).color.color.opacity(0.6)
            context.stroke(Self.path(route), with: .color(colour),
                           style: Self.traceStyle(width: Double(BoardTrace.defaultWidth) * 0.0254))
            ring = route.last
        } else {
            ring = hoverPoint.map { TraceDrawing.anchor(document, at: $0, layer: traceLayer, scale: scale) }
        }
        if let ring {
            let r = 6 / scale
            context.stroke(Path(ellipseIn: CGRect(x: ring.x - r, y: ring.y - r, width: 2 * r, height: 2 * r)),
                           with: .color(.green), lineWidth: 2 / scale)
        }
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
        if visible.showsMark(placement.reference, side: side), let ink = placement.reference.color(on: colour) {
            let height = max(1, 8 / scale)
            let text = Text(chip.reference)
                .font(.system(size: height, weight: .medium))
                .foregroundStyle(ink)
            context.drawLayer { layer in
                layer.translateBy(x: body.rect.midX, y: body.rect.midY)
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

    /// Dots every 2.54 mm (4 grid steps), hidden when they would be closer
    /// than 8 points on screen.
    private func drawGrid(in context: GraphicsContext, size: CGSize) {
        let step = BoardGrid.spacing * 4
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

    /// One step of a drag that started at `start` (screen points) and has
    /// moved by `translation`. On the first step it decides what the drag
    /// does: the Text tool places a text there (and the drag moves it); the
    /// Select tool picks a text, else a footprint (texts draw on top), else
    /// clears the selection and pans. Returns what the drag is doing.
    @discardableResult
    func handle(start: CGPoint, translation: CGSize) -> Interaction {
        if interaction == nil { pressLocation = start }
        let current = interaction ?? begin(at: start)
        interaction = current
        let offset = CGSize(width: translation.width / scale, height: translation.height / scale)
        switch current {
        case let .moveGroup(base):
            document.moveBoardGroup(base, by: offset)
        case let .pan(startOffset):
            viewport.pan(to: CGSize(
                width: startOffset.width + translation.width,
                height: startOffset.height + translation.height
            ))
        case let .drawTrace(start, _):
            let cursor = board(CGPoint(x: pressLocation.x + translation.width, y: pressLocation.y + translation.height))
            interaction = .drawTrace(start: start, cursor: cursor)
            return .drawTrace(start: start, cursor: cursor)
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
            return .drawTrace(start: anchored, cursor: point)
        case .text:
            // The new text is the whole selection, and the drag moves it.
            document.addText(at: point)
            onPlaced()
            return .moveGroup(base: document.boardGroupBase())
        case .select:
            if let text = document.text(at: point, visible: shown) {
                selectionPress = document.pressSelection(.text(text.id), shift: shift)
                return .moveGroup(base: document.boardGroupBase())
            }
            if let hit = document.footprint(at: point, visible: shown), hit.board != nil {
                selectionPress = document.pressSelection(.footprint(hit.id), shift: shift)
                return .moveGroup(base: document.boardGroupBase())
            }
            document.pressEmptyBoard(shift: shift)
            return .pan(startOffset: viewport.offset)
        }
    }

    /// The end of a press: a click (under 3 points of movement) finishes
    /// what the press left pending in the selection.
    private func end(translation: CGSize) {
        if case let .drawTrace(start, cursor) = interaction {
            TraceDrawing.finish(document, from: start, to: cursor, layer: traceLayer, scale: scale,
                                diagonalFirst: NSEvent.modifierFlags.contains(.option),
                                visible: shown, onShowLayer: onShowLayer)
        }
        if let press = selectionPress {
            document.releaseSelection(press, wasClick: hypot(translation.width, translation.height) < 3)
        }
        selectionPress = nil
        interaction = nil
    }
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

    /// The route a release at `cursor` would add.
    static func preview(_ document: SchematicDocument, from start: CGPoint, to cursor: CGPoint,
                        layer: CopperLayer, scale: CGFloat, diagonalFirst: Bool) -> [CGPoint] {
        TraceRoute.route(from: start, to: anchor(document, at: cursor, layer: layer, scale: scale),
                         diagonalFirst: diagonalFirst)
    }

    /// The release: adds the trace (nothing for a zero-length one), makes it
    /// the whole selection, and asks for its layer to be shown if hidden.
    @discardableResult
    static func finish(_ document: SchematicDocument, from start: CGPoint, to cursor: CGPoint, layer: CopperLayer,
                       scale: CGFloat, diagonalFirst: Bool, visible: BoardVisibility,
                       onShowLayer: (CopperLayer) -> Void) -> UUID? {
        let end = anchor(document, at: cursor, layer: layer, scale: scale)
        guard let id = document.addTrace(from: start, to: end, layer: layer, diagonalFirst: diagonalFirst) else {
            return nil
        }
        document.select(.trace(id))
        if !visible.showsTrace(on: layer) { onShowLayer(layer) }
        return id
    }
}

/// The PCB page's tools: Select (move footprints, texts and traces, pan),
/// Text (click to place a text) and Trace (drag to draw a trace).
enum PCBTool: String, CaseIterable, Identifiable {
    case select
    case text
    case trace

    var id: String { rawValue }

    var title: String {
        switch self {
        case .select: "Select"
        case .text: "Text"
        case .trace: "Trace"
        }
    }

    var symbol: String {
        switch self {
        case .select: "cursorarrow"
        case .text: "textformat"
        case .trace: "line.diagonal"
        }
    }
}
