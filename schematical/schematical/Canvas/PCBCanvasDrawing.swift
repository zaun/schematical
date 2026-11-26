import AppKit
import SwiftUI

/// How `PCBCanvasView` draws the board (plan-68-AH: kept apart from its
/// body and gestures). `press` is what the press in progress shows.
extension PCBCanvasView {
    /// Draws the board as the page shows it, with `press` in progress.
    func draw(in context: inout GraphicsContext, size: CGSize, press: DrawState) {
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
        // The outline and every cutout, filled even-odd so a cutout is a hole (plan-65-E).
        let boardShape = Self.boardPath(document.boardEdge)
        if visible.shows(.board) { context.fill(boardShape, with: .color(colour.fill), style: FillStyle(eoFill: true)) }
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
        if case let .drawRect(kind, from, to) = press.interaction {
            let band = CGRect(x: min(from.x, to.x), y: min(from.y, to.y), width: abs(from.x - to.x), height: abs(from.y - to.y))
            // A cutout's preview is the shape it will make.
            let preview = kind == .cutout && band.width > 0 && band.height > 0
                ? Path { path in path.addLines(cutoutShape.shape(from: from, to: to).loop); path.closeSubpath() }
                : Path(band)
            context.stroke(preview, with: .color(.accentColor),
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

        if visible.shows(.ratsnest) { drawRatsnest(press, in: &context) }
        // An export has no pointer, highlight or selection.
        if !exporting, let key = highlightedNet.wrappedValue {
            drawHighlight(of: key, visible: visible, in: &context)
        }
        if !exporting, let routePreview { drawRoutePreview(routePreview, in: &context) }
        drawTraceTool(press, in: &context)
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
            plane.fill(shape, with: .color(colour.opacity(opacity)), style: FillStyle(eoFill: true))
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
    private func drawRatsnest(_ press: DrawState, in context: inout GraphicsContext) {
        var drawing: BoardTrace?
        if tool == .trace, case let .drawTrace(draft, cursor) = press.interaction {
            let (route, allowed) = TraceDrawing.assisted(document, draft: draft, to: cursor, layer: press.drawingLayer ?? traceLayer,
                                                         scale: scale, angle: traceAngle, assist: press.assist)
            if allowed, TraceRoute.isLegal(route) {
                drawing = BoardTrace(id: UUID(), layer: press.drawingLayer ?? traceLayer,
                                     width: TraceDrawing.width(document, draft: draft, layer: press.drawingLayer ?? traceLayer), points: route)
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
    private func drawTraceTool(_ press: DrawState, in context: inout GraphicsContext) {
        guard tool == .trace else { return }
        let ring: CGPoint?
        var legal = true
        if case let .drawTrace(draft, cursor) = press.interaction {
            let (route, allowed) = TraceDrawing.assisted(document, draft: draft, to: cursor, layer: press.drawingLayer ?? traceLayer,
                                                         scale: scale, angle: traceAngle, assist: press.assist)
            legal = allowed
            let mil = TraceDrawing.width(document, draft: draft, layer: press.drawingLayer ?? traceLayer)
            let width = Double(mil) * 0.0254
            if allowed {
                let colour = document.board.style(of: press.drawingLayer ?? traceLayer).color.color.opacity(0.6)
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
        draw(in: &context, size: size, press: DrawState())
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

}
