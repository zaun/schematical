import AppKit
import SwiftUI

/// What a press on the PCB canvas is doing, and the rest of the canvas's
/// press, pinch, hover and key state (plan-68-AH: moved out of
/// `PCBCanvasView` so each step can be tested on its own). Each step takes
/// the canvas as it is now (`PCBCanvasView`, a value holding the document,
/// viewport, tool and callbacks), as the view's own closures read it.
@MainActor @Observable
final class PCBCanvasController {
    typealias Interaction = PCBCanvasView.Interaction

    /// What the current press does; nil between presses.
    var interaction: Interaction?
    /// Where the current press started, in screen points.
    var pressLocation: CGPoint = .zero
    /// The Trace assist for the trace being drawn, built at its press.
    var assist: AssistRouter?
    /// The layer the trace being drawn is on: `traceLayer` at the press,
    /// then the next layer after each V; nil when no trace is drawn.
    var drawingLayer: CopperLayer?

    /// Zoom when the current pinch began, nil between pinches.
    @ObservationIgnored var pinchStartZoom: CGFloat?
    /// The press in progress as the system reported it, and how far the
    /// canvas has scrolled under it since it began. Cleared when it ends.
    @ObservationIgnored var rawPress: (start: CGPoint, translation: CGSize)?
    @ObservationIgnored var pressShift: CGSize = .zero
    /// What the current press did to the selection; a click finishes it.
    @ObservationIgnored var selectionPress: SelectionPress<BoardItem>?
    /// Space, `/` and Esc while a trace is being drawn. Installed through
    /// the editor's monitors, which the document empties when it closes.
    @ObservationIgnored var drawingKeyMonitor: UUID?
    /// The Trace tool's layer and angle as they are now, for the key
    /// monitor (`TraceKeyContext`).
    @ObservationIgnored let live = TraceKeyContext()
    /// Where the pointer is, for the hovered pad and the Trace tool's
    /// landing ring. Only `PCBHoverLayer` reads it, so the pointer moving
    /// redraws that layer and not the board.
    @ObservationIgnored let hover = BoardHover()

    nonisolated deinit {}

    /// What the drawing reads of the press, read in the view's body so a
    /// change redraws the board.
    var drawState: PCBCanvasView.DrawState {
        PCBCanvasView.DrawState(interaction: interaction, assist: assist, drawingLayer: drawingLayer)
    }

    // MARK: Pinch, hover, scroll

    func pinchChanged(_ magnification: CGFloat, at location: CGPoint, viewport: CanvasViewport) {
        let start = pinchStartZoom ?? viewport.zoom
        pinchStartZoom = start
        viewport.setZoom(start * magnification, around: location)
    }

    func pinchEnded() { pinchStartZoom = nil }

    /// The pointer over the canvas at `location` (screen points), or off it (nil).
    func hovered(at location: CGPoint?, in canvas: PCBCanvasView) {
        hover.point = location.map(canvas.board)
        hover.location = location
        canvas.pointer?.point = hover.point
    }

    /// The canvas scrolled by `shift` while a press is down: the press's start
    /// moves with the content, and the drag's effect is worked out again for
    /// the pointer's unchanged place.
    func scrolledMidPress(by shift: CGSize, in canvas: PCBCanvasView) {
        guard let raw = rawPress, interaction != nil else { return }
        if case let .pan(startOffset)? = interaction {
            // A pan is the offset itself: its start moves with the scroll.
            interaction = .pan(startOffset: CGSize(width: startOffset.width + shift.width,
                                                   height: startOffset.height + shift.height))
        } else {
            pressShift = CGSize(width: pressShift.width + shift.width, height: pressShift.height + shift.height)
            pressLocation = CGPoint(x: pressLocation.x + shift.width, y: pressLocation.y + shift.height)
        }
        handle(start: raw.start, translation: shifted(raw.translation), in: canvas)
    }

    /// `translation` less the scroll since the press began: the drag as if the
    /// canvas had stayed put.
    func shifted(_ translation: CGSize) -> CGSize {
        CGSize(width: translation.width - pressShift.width, height: translation.height - pressShift.height)
    }

    // MARK: The press

    /// The drag gesture moved: one step of the press.
    func dragChanged(start: CGPoint, translation: CGSize, in canvas: PCBCanvasView,
                     modifiers: NSEvent.ModifierFlags = NSEvent.modifierFlags) {
        rawPress = (start, translation)
        handle(start: start, translation: shifted(translation), in: canvas, modifiers: modifiers)
    }

    /// The drag gesture ended.
    func dragEnded(translation: CGSize, in canvas: PCBCanvasView) {
        end(translation: shifted(translation), in: canvas)
        rawPress = nil
        pressShift = .zero
    }

    /// The layer the Trace tool draws on now: the drawn trace's, else the
    /// menu's.
    func activeLayer(_ canvas: PCBCanvasView) -> CopperLayer { drawingLayer ?? canvas.traceLayer }

    /// Whether the pad under the pointer is outlined and named: while
    /// nothing is being dragged (a trace being drawn still shows it, at its
    /// end).
    var showsHoveredPad: Bool {
        switch interaction {
        case nil, .drawTrace?: true
        default: false
        }
    }

    /// The layer whose landing ring follows the pointer: the Trace tool's,
    /// while no trace is being drawn (a drawn trace has its own ring).
    func hoverRingLayer(_ canvas: PCBCanvasView) -> CopperLayer? {
        guard canvas.tool == .trace else { return nil }
        if case .drawTrace? = interaction { return nil }
        return canvas.traceLayer
    }

    /// One step of a drag that started at `start` (screen points) and has
    /// moved by `translation`. On the first step it decides what the drag
    /// does: the Text tool places a text there (and the drag moves it); the
    /// Trace tool starts a trace; the Select tool picks a text, else a
    /// trace, else a footprint, else clears the selection and pans. Returns
    /// what the drag is doing. `modifiers` are read once, at the first step.
    @discardableResult
    func handle(start: CGPoint, translation: CGSize, in canvas: PCBCanvasView,
                modifiers: NSEvent.ModifierFlags = NSEvent.modifierFlags) -> Interaction {
        let document = canvas.document
        if interaction == nil {
            pressLocation = start
            // The whole press is one undo step (plan-31-C).
            document.beginGesture(.pcb)
        }
        let current = interaction ?? begin(at: start, in: canvas, modifiers: modifiers)
        interaction = current
        let scale = canvas.scale
        let offset = CGSize(width: translation.width / scale, height: translation.height / scale)
        let at = CGPoint(x: pressLocation.x + translation.width, y: pressLocation.y + translation.height)
        switch current {
        case let .moveGroup(base):
            document.moveBoardGroup(base, by: offset)
        case let .editTrace(base):
            document.dragTraceSegment(base, by: offset)
        case let .moveReference(id, from):
            ReferenceDragging.drag(document, id: id, from: from, by: offset)
        case let .pan(startOffset):
            canvas.viewport.pan(to: CGSize(width: startOffset.width + translation.width,
                                           height: startOffset.height + translation.height))
        case let .boxSelect(base, _):
            interaction = .boxSelect(base: base, to: at)
            let a = canvas.board(pressLocation), b = canvas.board(at)
            document.boxSelectBoard(CGRect(x: min(a.x, b.x), y: min(a.y, b.y), width: abs(a.x - b.x), height: abs(a.y - b.y)),
                                    base: base, visible: canvas.shown)
        case let .drawTrace(draft, _):
            interaction = .drawTrace(draft: draft, cursor: canvas.board(at))
        case let .resize(item, handle):
            document.resizePourItem(item, handle: handle, to: canvas.board(at))
        case let .drawRect(kind, from, _):
            interaction = .drawRect(kind: kind, from: from, to: canvas.board(at))
        case .idle:
            break
        }
        return interaction ?? current
    }

    /// Decides what a press does. Shift is read once, here, at the start.
    private func begin(at start: CGPoint, in canvas: PCBCanvasView, modifiers: NSEvent.ModifierFlags) -> Interaction {
        let document = canvas.document
        let point = canvas.board(start)
        let shift = modifiers.contains(.shift)
        let scale = canvas.scale
        let tolerance = TraceDrawing.tolerance / scale
        switch canvas.tool {
        case .trace:
            let anchored = TraceDrawing.anchor(document, at: point, layer: canvas.traceLayer, scale: scale)
            // The assist's grid is built once, here (plan-38-L).
            assist = canvas.traceAssist ? AssistRouter(document: document, layer: canvas.traceLayer, start: anchored) : nil
            drawingLayer = canvas.traceLayer
            return .drawTrace(draft: TraceDraft(start: anchored), cursor: point)
        case .via:
            // A press places a via (snapped onto a trace, or the trace grid)
            // and the drag moves it. Nothing on a 1-layer board.
            guard document.addVia(at: document.viaAnchor(at: point, tolerance: tolerance)) != nil else { return .idle }
            return .moveGroup(base: document.boardGroupBase())
        case .hole:
            // A press places a hole on the trace grid, and the drag moves it.
            document.addHole(at: point)
            return .moveGroup(base: document.boardGroupBase())
        case .pour, .keepout, .cutout:
            // The drag is the rectangle; a release without one makes nothing.
            return .drawRect(kind: canvas.tool, from: point, to: point)
        case .route:
            // A click routes the net of the pad, or the one connection of
            // the unrouted line, under it.
            if let scope = RouteTool.scope(document, at: point, tolerance: tolerance, visible: canvas.shown) {
                canvas.onRoute(scope)
            }
            return .idle
        case .delete:
            // One deletion per press: the text, via, hole or trace where it
            // started.
            document.deleteBoardItem(at: point, tolerance: tolerance, visible: canvas.shown)
            return .idle
        case .text:
            // The new text is the whole selection, and the drag moves it.
            document.addText(at: point)
            canvas.onPlaced()
            return .moveGroup(base: document.boardGroupBase())
        case .select:
            return beginSelect(at: start, point: point, in: canvas, modifiers: modifiers, shift: shift)
        }
    }

    /// A Select tool press: a reference (⌘), a pour's handle, an item, else
    /// a box (Shift) or a pan.
    private func beginSelect(at start: CGPoint, point: CGPoint, in canvas: PCBCanvasView,
                             modifiers: NSEvent.ModifierFlags, shift: Bool) -> Interaction {
        let document = canvas.document
        let scale = canvas.scale
        let command = modifiers.contains(.command)
        // Cmd-press on a reference's text: the drag moves only the reference.
        if command, let press = ReferenceDragging.press(document, at: point, scale: scale, visible: canvas.shown) {
            return .moveReference(id: press.id, from: press.from)
        }
        if let hit = document.pourHandle(at: point, tolerance: 6 / scale) {
            return .resize(item: hit.item, handle: hit.handle)
        }
        if let item = document.boardItem(at: point, tolerance: TraceDrawing.tolerance / scale, visible: canvas.shown) {
            let press = document.pressSelection(item, shift: shift)
            selectionPress = press
            // A plain drag on a lone trace edits it, sliding the segment
            // under the pointer; Cmd-drag moves the trace as one piece.
            if case let .trace(id) = item, !press.dragsGroup, !command,
               let base = document.traceSegmentBase(id, at: point) {
                return .editTrace(base: base)
            }
            return .moveGroup(base: document.boardGroupBase(grab: point))
        }
        if shift {
            return .boxSelect(base: document.boardSelection.items, to: start)
        }
        document.pressEmptyBoard(shift: false)
        return .pan(startOffset: canvas.viewport.offset)
    }

    /// The end of a press: a click (under 3 points of movement) finishes
    /// what the press left pending in the selection.
    func end(translation: CGSize, in canvas: PCBCanvasView) {
        let document = canvas.document
        let moved = hypot(translation.width, translation.height)
        if case let .drawTrace(draft, cursor) = interaction {
            TraceDrawing.finish(document, draft: draft, to: cursor, layer: activeLayer(canvas), scale: canvas.scale,
                                angle: canvas.traceAngle, visible: canvas.shown, assist: assist,
                                onShowLayer: canvas.onShowLayer)
        }
        if case let .drawRect(kind, from, to) = interaction {
            PCBCanvasView.finishRect(document, kind: kind, from: from, to: to, moved: moved, layer: canvas.traceLayer,
                                     cutoutShape: canvas.cutoutShape)
        }
        assist = nil
        drawingLayer = nil
        if case let .moveGroup(base) = interaction { document.settleBoardGroup(base) }
        if case let .editTrace(base) = interaction { document.settleTraceSegment(base) }
        if let press = selectionPress { document.releaseSelection(press, wasClick: moved < 3) }
        selectionPress = nil
        interaction = nil
        if document.isGestureOpen { document.endGesture() }
    }

    // MARK: Keys while drawing

    func installDrawingKeyMonitor(_ monitors: EditorEventMonitors, in canvas: PCBCanvasView) {
        guard drawingKeyMonitor == nil else { return }
        drawingKeyMonitor = monitors.add(matching: .keyDown) { [weak self] in self?.monitored($0, in: canvas) ?? $0 }
    }

    /// What the key monitor returns for `event`: nil when a drawing key
    /// used it, else the event, left for the rest of the app.
    func monitored(_ event: NSEvent, in canvas: PCBCanvasView) -> NSEvent? {
        handleDrawingKey(event, in: canvas) ? nil : event
    }

    func removeDrawingKeyMonitor(_ monitors: EditorEventMonitors) {
        monitors.remove(drawingKeyMonitor)
        drawingKeyMonitor = nil
    }

    /// Space, Esc, `/` and V while a trace is being drawn. Returns whether the
    /// key was used; any other key, or any key when no trace is being
    /// drawn, is left alone. `canvas` is the view as it was when the monitor
    /// was installed: its `traceLayer` and `traceAngle` are those of then;
    /// `live` has them now.
    func handleDrawingKey(_ event: NSEvent, in canvas: PCBCanvasView) -> Bool {
        guard case let .drawTrace(draft, cursor)? = interaction else {
            // Esc, with no trace being drawn, clears a highlighted net.
            guard event.keyCode == 53, canvas.highlightedNet.wrappedValue != nil else { return false }
            canvas.highlightedNet.wrappedValue = nil
            return true
        }
        let document = canvas.document
        let scale = canvas.scale
        var next = draft
        let layer = drawingLayer ?? live.layer
        let plain = event.modifierFlags.intersection([.command, .control, .option]).isEmpty
        let key = event.charactersIgnoringModifiers ?? ""
        if event.keyCode == 53 {
            // Esc with nothing locked drops the whole trace; the rest of the
            // drag does nothing.
            if !next.unlock() { interaction = .idle; return true }
        } else if event.keyCode == 49 {
            if !event.isARepeat { lockCorner(&next, cursor: cursor, layer: layer, canvas: canvas) }
        } else if key == "/", plain {
            next.flip()
        } else if key.lowercased() == "v", plain, !event.modifierFlags.contains(.shift) {
            // V: a via where the trace has got to, and on from it on the
            // next layer (plan-38-L).
            if !event.isARepeat, let (draft, nextLayer) = TraceDrawing.dropVia(
                document, draft: next, to: cursor, layer: layer, scale: scale, angle: live.angle, assist: assist) {
                next = draft
                drawingLayer = nextLayer
                assist = assist == nil ? nil : AssistRouter(document: document, layer: nextLayer, start: draft.start)
                canvas.onLayerChange(nextLayer)
            }
        } else {
            return false
        }
        interaction = .drawTrace(draft: next, cursor: cursor)
        return true
    }

    /// Space: locks a corner where the pointer's end lands, the assist's leg
    /// when it found a legal one.
    private func lockCorner(_ draft: inout TraceDraft, cursor: CGPoint, layer: CopperLayer, canvas: PCBCanvasView) {
        let end = TraceDrawing.anchor(canvas.document, at: cursor, layer: layer, scale: canvas.scale)
        if let assist {
            if let leg = assist.route(from: draft.legStart, to: end) { draft.lock(leg: leg) }
        } else {
            draft.lock(to: end, angle: live.angle)
        }
    }
}
