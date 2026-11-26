import AppKit
import SwiftUI

/// What the Schematic canvas works on, as `SchematicCanvasView` has it at
/// the moment of each call: its inputs, rebuilt on every call so a tool or
/// supply change is seen at once.
@MainActor
struct SchematicCanvasContext {
    let catalog: PartCatalogStore
    let document: SchematicDocument
    let viewport: CanvasViewport
    let tool: SchematicTool
    /// The supply the VCC tool places (a `SupplyChoice` name).
    var supplyVoltage = SupplyChoice.defaultName
    /// The simulation. While it runs (play mode), the canvas only pans.
    let simulation: SimulationController
    var onOpenMemory: (UUID) -> Void = { _ in }
    var onOpenCpu: (UUID) -> Void = { _ in }
    var onOpenTerminal: (UUID) -> Void = { _ in }
    var onOpenRegisters: (UUID) -> Void = { _ in }
    var onOpenDocumentInfo: () -> Void = {}
    /// Where the pointer is over the canvas (canvas units), for ⌘V.
    var pointer: BoardPointer?
    var onPartPlaced: () -> Void = {}

    var isRunning: Bool { simulation.isRunning }

    /// Actual points per world unit (not the zoom level shown).
    var zoom: CGFloat { viewport.scale }

    /// Hit tolerances are in screen points, so they are divided by zoom.
    var lineTolerance: CGFloat { 6 / zoom }
    var pinRadius: CGFloat { 8 / zoom }
    /// A bus is 6 wide, so half its width is added to the wire tolerance.
    var busTolerance: CGFloat { lineTolerance + 3 / zoom }

    /// Grid spacing in world units. Dots get sparse when zoomed out so they stay legible.
    var gridStep: CGFloat {
        zoom < 0.5 ? SchematicGrid.spacing * 5 : SchematicGrid.spacing
    }

    /// The visible world area, widened to whole grid steps.
    var gridRect: CGRect {
        let visible = viewport.visibleWorldRect
        let step = gridStep
        let minX = (visible.minX / step).rounded(.down) * step
        let minY = (visible.minY / step).rounded(.down) * step
        let maxX = (visible.maxX / step).rounded(.up) * step
        let maxY = (visible.maxY / step).rounded(.up) * step
        return CGRect(x: minX, y: minY, width: maxX - minX, height: maxY - minY)
    }

    func lineAnchor(at point: CGPoint) -> LineAnchor? {
        document.lineAnchor(at: point, pinRadius: pinRadius, gridRadius: lineTolerance)
    }

    /// The ids of the Schematic selection's items that `pick` names: its
    /// wires, net labels, text blocks or buses.
    func selected(_ pick: (SchematicItem) -> UUID?) -> Set<UUID> {
        Set(document.schematicSelection.items.compactMap(pick))
    }
}

/// The Schematic canvas's gestures, hover and drawing keys (plan-68-AJ: the
/// logic `SchematicCanvasView` held in `@State`, moved here so it is tested
/// on its own). The view keeps one, hands it each press, hover and key with
/// its `SchematicCanvasContext`, and draws from what it holds. See
/// `SchematicCanvasView` for what each tool does.
@MainActor
@Observable
final class SchematicCanvasController {
    enum Interaction {
        /// Moves every selected part, junction and wire bend from `base`.
        /// `lever` is the part pressed without Shift: a click flips its lever.
        case moveGroup(base: SchematicGroupBase, lever: UUID?)
        case dragNode(wire: UUID, base: [CGPoint], index: Int)
        case dragSegment(wire: UUID, base: [CGPoint], index: Int)
        /// Line tool: the route from the anchor pressed on, with its locked
        /// legs, to the cursor.
        case connecting(from: LineAnchor, draft: RouteDraft, cursor: CGPoint)
        /// Bus tool: the route from the snapped press point, with its locked
        /// legs, to the cursor.
        case drawingBus(draft: RouteDraft, cursor: CGPoint)
        /// Pointer drags on a bus, against the bus as it was at the press.
        /// `done` is the drag offset stored so far (see `dragBus`).
        case dragBusEnd(bus: UUID, base: Bus, line: UUID, end: BusEnd, done: CGSize)
        case dragBusLine(bus: UUID, base: Bus, line: UUID, done: CGSize)
        /// Pointer drag on a bus port: slides it along its line from `base`,
        /// its start at the press.
        case slidePort(port: UUID, base: CGFloat)
        case pan(startOffset: CGSize)
        /// Play mode: the mouse is held on a tactile switch's button
        /// (plan-27-J); releasing it releases the button.
        case holdButton(chip: UUID)
        /// Shift-drag on empty space: a selection box between two screen
        /// points, adding to `base`, the selection at the press.
        case boxSelect(base: Set<SchematicItem>, from: CGPoint, to: CGPoint)
        /// Eraser press; erases on release if it was a click.
        case erasing
        case idle
    }

    private(set) var interaction: Interaction?
    /// Zoom when the current pinch began, nil between pinches.
    private(set) var pinchStartZoom: CGFloat?
    /// The press in progress as the system reported it, and how far the
    /// canvas has scrolled under it since it began (a scroll moves the
    /// world, not the pointer). Both are cleared when the press ends.
    private(set) var rawPress: DragPress?
    private(set) var pressShift: CGSize = .zero
    /// What the current press did to the selection; a click finishes it.
    private(set) var selectionPress: SelectionPress<SchematicItem>?
    var isDropTargeted = false
    private(set) var hoveredChipID: UUID?
    /// The unit of `hoveredChipID` under the pointer, for a part drawn as its units.
    private(set) var hoveredUnitKey: String?
    private(set) var hoveredWireID: UUID?
    private(set) var hoveredBusID: UUID?
    private(set) var hoveredPortID: UUID?
    private(set) var hoveredNoteID: UUID?
    private(set) var isTitleBlockHovered = false
    private(set) var hoveredLabelID: UUID?
    /// The connection point under the pointer in Line mode, when not drawing.
    private(set) var hoverAnchor: LineAnchor?
    /// Watches for Space, Esc and `/` while a wire or bus is being drawn.
    /// Installed through the editor's monitors, which the document empties
    /// when it closes.
    private(set) var drawingKeyMonitor: UUID?

    init() {}

    /// As `SchematicDocument`'s: an isolated deinit aborts the runtime.
    nonisolated deinit {}

    // MARK: Previews

    /// The Line route being drawn: locked legs, then the live leg to the
    /// anchor under the cursor or the snapped cursor.
    func connectionPreview(in context: SchematicCanvasContext) -> ConnectionPreview? {
        guard case let .connecting(from, draft, cursor)? = interaction else { return nil }
        let end = lineEnd(from: from, cursor: cursor, in: context)
        return ConnectionPreview(points: draft.path(to: end.point, endAxis: end.axis),
                                 target: end.target == nil ? nil : end.point)
    }

    /// Where a Line route ends for `cursor`: the tip of the anchor under it
    /// (other than the start) and the axis that pin forces, or else the
    /// snapped cursor. The preview, Space and the release all use this.
    func lineEnd(from: LineAnchor, cursor: CGPoint, in context: SchematicCanvasContext)
        -> (point: CGPoint, axis: RouteAxis?, target: LineAnchor?) {
        if let anchor = context.lineAnchor(at: cursor), anchor != from, let tip = context.document.anchorTip(anchor) {
            return (tip, context.document.anchorAxis(anchor), anchor)
        }
        return (SchematicGrid.snap(cursor), nil, nil)
    }

    /// What the Bus tool would draw: the lines `addBus` would add, or the
    /// route as drawn when releasing would be refused.
    func busPreview(in context: SchematicCanvasContext) -> BusPreview? {
        guard case let .drawingBus(draft, cursor)? = interaction else { return nil }
        let candidates = draft.busCandidates(to: SchematicGrid.snap(cursor))
        guard let drawn = candidates.first, drawn.count >= 2 else { return nil }
        switch context.document.busDraft(candidates: candidates) {
        case let .success(result):
            return BusPreview(points: result.path, refused: false)
        case .failure:
            return BusPreview(points: drawn, refused: true)
        }
    }

    /// The point to highlight in Line mode: where a line would land while it is
    /// being drawn, or else the connection point under the pointer. Nothing else is
    /// highlighted in this mode.
    func highlightPoint(in context: SchematicCanvasContext) -> CGPoint? {
        guard context.tool == .line else { return nil }
        if let preview = connectionPreview(in: context) {
            return preview.target
        }
        return hoverAnchor.flatMap { context.document.anchorTip($0) }
    }

    /// The selection box being drawn, in screen points.
    var selectionBox: (from: CGPoint, to: CGPoint)? {
        guard case let .boxSelect(_, from, to)? = interaction else { return nil }
        return (from, to)
    }

    // MARK: Drawing keys

    func installDrawingKeyMonitor(_ monitors: EditorEventMonitors, context: SchematicCanvasContext) {
        guard drawingKeyMonitor == nil else { return }
        drawingKeyMonitor = monitors.add(matching: .keyDown) { [weak self] in self?.monitoredKey($0, in: context) ?? $0 }
    }

    func removeDrawingKeyMonitor(_ monitors: EditorEventMonitors) {
        monitors.remove(drawingKeyMonitor)
        drawingKeyMonitor = nil
    }

    /// What the key monitor does with a key: nil when the drawing used it.
    func monitoredKey(_ event: NSEvent, in context: SchematicCanvasContext) -> NSEvent? {
        handleDrawingKey(event, in: context) ? nil : event
    }

    private enum DrawingKey {
        /// Space: lock the live leg.
        case lock
        /// Esc: drop the last lock, or the whole drawing when nothing is locked.
        case back
        /// `/`: flip the live leg's L.
        case flip
    }

    /// Space, Esc and `/` while a Line or Bus is being drawn. Returns whether
    /// the key was used; any other key, or any key when nothing is being
    /// drawn, is left alone.
    func handleDrawingKey(_ event: NSEvent, in context: SchematicCanvasContext) -> Bool {
        let key: DrawingKey
        if event.keyCode == 53 {
            key = .back
        } else if event.keyCode == 49 {
            key = .lock
        } else if event.charactersIgnoringModifiers == "/",
                  event.modifierFlags.intersection([.command, .control, .option]).isEmpty {
            key = .flip
        } else {
            return false
        }

        switch interaction {
        case let .connecting(from, draft, cursor)?:
            var draft = draft
            let end = lineEnd(from: from, cursor: cursor, in: context)
            switch key {
            case .lock:
                if !event.isARepeat { draft.lock(to: end.point, endAxis: end.axis) }
            case .back:
                // Dropping the whole drawing leaves the rest of the drag doing nothing.
                if !draft.unlock() { interaction = .idle; return true }
            case .flip:
                draft.flip()
            }
            interaction = .connecting(from: from, draft: draft, cursor: cursor)
            return true

        case let .drawingBus(draft, cursor)?:
            var draft = draft
            let end = SchematicGrid.snap(cursor)
            switch key {
            case .lock:
                guard !event.isARepeat else { return true }
                // Lock what the preview shows: when only the flipped live leg
                // would be accepted, that is the one drawn.
                let candidates = draft.busCandidates(to: end)
                if candidates.count > 1, case let .success(result) = context.document.busDraft(candidates: candidates),
                   result.path == candidates[1] {
                    draft.flip()
                }
                draft.lock(to: end, endAxis: nil)
            case .back:
                if !draft.unlock() { interaction = .idle; return true }
            case .flip:
                draft.flip()
            }
            interaction = .drawingBus(draft: draft, cursor: cursor)
            return true

        default:
            return false
        }
    }

    // MARK: Pinch and scroll

    /// Trackpad pinch: zooms about the point the fingers started on.
    func pinchChanged(magnification: CGFloat, startLocation: CGPoint, in context: SchematicCanvasContext) {
        let start = pinchStartZoom ?? context.viewport.zoom
        pinchStartZoom = start
        context.viewport.setZoom(start * magnification, around: startLocation)
    }

    func pinchEnded() {
        pinchStartZoom = nil
    }

    /// The canvas scrolled by `shift` while a press is down: the press's start
    /// moves with the content (so a drag, a wire or a box stays anchored to the
    /// world it began on) and the drag's effect is worked out again for the
    /// pointer's unchanged place.
    func scrolledMidPress(by shift: CGSize, in context: SchematicCanvasContext) {
        guard let raw = rawPress, interaction != nil else { return }
        switch interaction {
        case let .pan(startOffset)?:
            // A pan is the offset itself: its start moves with the scroll.
            interaction = .pan(startOffset: CGSize(width: startOffset.width + shift.width,
                                                   height: startOffset.height + shift.height))
        case let .boxSelect(base, from, to)?:
            interaction = .boxSelect(base: base, from: CGPoint(x: from.x + shift.width, y: from.y + shift.height), to: to)
            pressShift = CGSize(width: pressShift.width + shift.width, height: pressShift.height + shift.height)
        default:
            pressShift = CGSize(width: pressShift.width + shift.width, height: pressShift.height + shift.height)
        }
        handleChanged(raw.shifted(by: pressShift), shift: false, in: context)
    }

    // MARK: Press

    /// The canvas's drag gesture changed: `raw` as the system reported it.
    /// `shift` is whether Shift is down now; only a press's start reads it.
    /// A press's start ends any text field's editing in `keyWindow` (the
    /// parts search, a sidebar field), as a click outside a field does in
    /// any Mac app, so ⌘C / ⌘V / ⌘X / ⌘A then reach the Schematic. A new
    /// window or tab starts with its search field focused.
    func pressChanged(_ raw: DragPress, shift: Bool, keyWindow: NSWindow? = nil, in context: SchematicCanvasContext) {
        if interaction == nil, let keyWindow, keyWindow.firstResponder is NSText {
            keyWindow.makeFirstResponder(nil)
        }
        rawPress = raw
        handleChanged(raw.shifted(by: pressShift), shift: shift, in: context)
    }

    /// The canvas's drag gesture ended.
    func pressEnded(_ raw: DragPress, in context: SchematicCanvasContext) {
        handleEnded(raw.shifted(by: pressShift), in: context)
        rawPress = nil
        pressShift = .zero
    }

    private func handleChanged(_ value: DragPress, shift: Bool, in context: SchematicCanvasContext) {
        let document = context.document, viewport = context.viewport
        let start = viewport.world(value.startLocation)
        let current = viewport.world(value.location)
        let delta = CGSize(
            width: value.translation.width / context.zoom,
            height: value.translation.height / context.zoom
        )

        // Play mode: a press on a tactile switch's button holds it down
        // until the mouse comes up; a drag anywhere else pans, and nothing
        // else can start.
        if context.isRunning {
            if interaction == nil, let chip = document.chip(at: start), chip.buttonHit(at: start) {
                interaction = .holdButton(chip: chip.id)
                pressButton(chip.id, pressed: true, in: context)
                return
            }
            if case .holdButton? = interaction { return }
            if interaction == nil {
                interaction = .pan(startOffset: viewport.offset)
            }
            if case let .pan(startOffset)? = interaction {
                viewport.pan(to: CGSize(
                    width: startOffset.width + value.translation.width,
                    height: startOffset.height + value.translation.height
                ))
            }
            return
        }

        // The whole press, whatever it does, is one undo step (plan-31-B).
        if interaction == nil { document.beginGesture(.schematic) }

        switch context.tool {
        case .pointer:
            if interaction == nil {
                beginPointerInteraction(at: start, screen: value.startLocation, shift: shift, in: context)
            }
            dragPointer(value, current: current, delta: delta, in: context)

        case .line:
            if interaction == nil {
                if let anchor = context.lineAnchor(at: start), let tip = document.anchorTip(anchor) {
                    let draft = RouteDraft(start: tip, startAxis: document.anchorAxis(anchor))
                    interaction = .connecting(from: anchor, draft: draft, cursor: current)
                } else {
                    interaction = .idle
                }
            }
            if case let .connecting(from, draft, _)? = interaction {
                interaction = .connecting(from: from, draft: draft, cursor: current)
            }

        case .busPort:
            // One port per press, where the press started. A refusal sets
            // document.editorMessage.
            if interaction == nil {
                document.addPort(at: start, tolerance: context.busTolerance)
            }
            interaction = .idle

        case .bus:
            if interaction == nil {
                interaction = .drawingBus(draft: RouteDraft(start: SchematicGrid.snap(start), startAxis: nil), cursor: current)
            }
            if case let .drawingBus(draft, _)? = interaction {
                interaction = .drawingBus(draft: draft, cursor: current)
            }

        case .eraser:
            // Erases on release, and only for a click (see handleEnded).
            if interaction == nil {
                interaction = .erasing
            }

        case .text:
            // One block per press, its top-left where the press started. A
            // press on a block selects that one instead.
            if interaction == nil {
                if let hit = document.note(at: start) {
                    document.select(.note(hit.id))
                    document.editingNoteID = hit.id
                } else {
                    document.addNote(at: start)
                }
                context.onPartPlaced()
            }
            interaction = .idle

        case .netLabel:
            // One label per press, its point where the press started. A
            // press on a label selects that one instead.
            if interaction == nil {
                if let hit = document.netLabel(at: start) {
                    document.select(.label(hit.id))
                } else {
                    document.addNetLabel(at: start)
                }
                context.onPartPlaced()
            }
            interaction = .idle

        case .vcc, .ground, .pullUp, .pullDown:
            // One supply per press, centred under where the press started.
            if interaction == nil,
               let name = context.tool.supplyName(voltage: context.supplyVoltage),
               let part = context.catalog.part(named: name) {
                document.placeChip(
                    part: part,
                    symbol: symbol(for: part),
                    centeredAt: start,
                    footprintID: NewPartOptions.footprintID(for: part),
                    ohms: NewPartOptions.ohms(for: part)
                )
            }
            interaction = .idle
        }
    }

    /// One frame of a Pointer drag, for the interaction its press began.
    private func dragPointer(_ value: DragPress, current: CGPoint, delta: CGSize, in context: SchematicCanvasContext) {
        let document = context.document, viewport = context.viewport
        switch interaction {
        case let .moveGroup(base, _)?:
            document.moveSchematicGroup(base, by: delta)

        case let .dragNode(wire, base, index)?:
            document.setWirePath(wire, OrthogonalRoute.moveNode(base, at: index, to: SchematicGrid.snap(current)))

        case let .dragSegment(wire, base, index)?:
            let offset = CGSize(width: SchematicGrid.snap(delta.width), height: SchematicGrid.snap(delta.height))
            document.setWirePath(wire, OrthogonalRoute.moveSegment(base, at: index, by: offset))

        case let .dragBusEnd(bus, base, line, end, done)?:
            // Walks toward the cursor in grid steps; a refused step leaves
            // the bus at the last good one.
            guard let point = base.line(line)?.point(end) else { break }
            let target = CGSize(width: current.x - point.x, height: current.y - point.y)
            let reached = document.dragBus(bus, from: done, to: target) { offset in
                BusEdit.moveEnd(base, line: line, end: end,
                                to: CGPoint(x: point.x + offset.width, y: point.y + offset.height))
            }
            interaction = .dragBusEnd(bus: bus, base: base, line: line, end: end, done: reached)

        case let .dragBusLine(bus, base, line, done)?:
            let reached = document.dragBus(bus, from: done, to: delta) { offset in
                BusEdit.moveLine(base, line: line, by: offset)
            }
            interaction = .dragBusLine(bus: bus, base: base, line: line, done: reached)

        case let .slidePort(port, base)?:
            guard let found = document.port(port), let line = found.bus.line(found.port.line) else { break }
            document.slidePort(port, from: base, by: line.isHorizontal ? delta.width : delta.height)

        case let .pan(startOffset)?:
            viewport.pan(to: CGSize(
                width: startOffset.width + value.translation.width,
                height: startOffset.height + value.translation.height
            ))

        case let .boxSelect(base, from, _)?:
            interaction = .boxSelect(base: base, from: from, to: value.location)
            let a = viewport.world(from), b = viewport.world(value.location)
            document.boxSelectSchematic(
                CGRect(x: min(a.x, b.x), y: min(a.y, b.y), width: abs(a.x - b.x), height: abs(a.y - b.y)),
                base: base
            )

        default:
            break
        }
    }

    /// Decides what a Pointer press does. Shift is read once, here, at the
    /// start of the press. `screen` is the press in screen points.
    private func beginPointerInteraction(at point: CGPoint, screen: CGPoint, shift: Bool,
                                         in context: SchematicCanvasContext) {
        let document = context.document
        let tolerance = context.lineTolerance, busTolerance = context.busTolerance
        /// A press on a wire or bus: alone, a drag reshapes it; in a larger
        /// selection (or with Shift) it moves the group.
        func press(_ item: SchematicItem, reshape: Interaction) {
            let press = document.pressSelection(item, shift: shift)
            selectionPress = press
            interaction = press.dragsGroup ? .moveGroup(base: document.schematicGroupBase(), lever: nil) : reshape
        }
        /// A press on an item that always moves with the group.
        func pressGroup(_ item: SchematicItem) {
            selectionPress = document.pressSelection(item, shift: shift)
            interaction = .moveGroup(base: document.schematicGroupBase(), lever: nil)
        }
        if let hit = document.piece(at: point) {
            // A unit of a part drawn as its units is picked; the drag then moves the picked units.
            selectionPress = document.pressPiece(hit, shift: shift)
            interaction = .moveGroup(base: document.schematicGroupBase(), lever: shift ? nil : hit.id)
        } else if let label = document.netLabel(at: point) {
            pressGroup(.label(label.id))
        } else if let port = document.portHit(at: point), let found = document.port(port) {
            // A press selects the port; dragging slides it along its line.
            document.selectedPortID = port
            interaction = .slidePort(port: port, base: found.port.start)
        } else if let junction = document.junctionHit(at: point, tolerance: tolerance) {
            pressGroup(.junction(junction.id))
        } else if let node = document.nodeHit(at: point, tolerance: tolerance) {
            press(.wire(node.wire), reshape: .dragNode(wire: node.wire, base: document.path(forWire: node.wire),
                                                       index: node.index))
        } else if let segment = document.segmentHit(at: point, tolerance: tolerance) {
            press(.wire(segment.wire), reshape: .dragSegment(wire: segment.wire, base: document.path(forWire: segment.wire),
                                                             index: segment.index))
        } else if let end = document.busEndHit(at: point, tolerance: busTolerance), let base = document.bus(end.bus) {
            press(.bus(end.bus), reshape: .dragBusEnd(bus: end.bus, base: base, line: end.line, end: end.end, done: .zero))
        } else if let line = document.busHit(at: point, tolerance: busTolerance), let base = document.bus(line.bus) {
            press(.bus(line.bus), reshape: .dragBusLine(bus: line.bus, base: base, line: line.line, done: .zero))
        } else if let note = document.note(at: point) {
            pressGroup(.note(note.id))
        } else if document.titleBlock(at: point) {
            pressGroup(.titleBlock)
        } else if shift {
            interaction = .boxSelect(base: document.schematicSelection.items, from: screen, to: screen)
        } else {
            document.pressEmptySchematic(shift: false)
            interaction = .pan(startOffset: context.viewport.offset)
        }
    }

    private func handleEnded(_ value: DragPress, in context: SchematicCanvasContext) {
        let document = context.document, viewport = context.viewport
        switch interaction {
        case let .connecting(from, draft, _)?:
            let end = lineEnd(from: from, cursor: viewport.world(value.location), in: context)
            if let target = end.target {
                // A refused wire changes nothing; document.editorMessage says why.
                _ = document.connect(from, target, path: draft.path(to: end.point, endAxis: end.axis))
            }
        case let .drawingBus(draft, _)?:
            // A refused drag changes nothing; document.editorMessage says why.
            document.addBus(candidates: draft.busCandidates(to: SchematicGrid.snap(viewport.world(value.location))))
        case let .dragBusEnd(bus, _, _, _, _)?, let .dragBusLine(bus, _, _, _)?:
            // Ports rode along with their lines; keep the wires on their pins.
            document.settleWires(attachedToBus: bus)
        case let .slidePort(port, _)?:
            document.settleWires(attachedToPort: port)
            document.adoptLooseEnds(ports: [port])
        case let .moveGroup(base, lever)?:
            if lever != nil, value.isClick {
                toggleSwitchLever(at: viewport.world(value.startLocation), in: context)
            }
            document.settleSchematicGroup(base)
        case .erasing?:
            // A drag erases nothing.
            if value.isClick {
                document.erase(at: viewport.world(value.startLocation), tolerance: context.lineTolerance)
            }
        case let .holdButton(chip)?:
            pressButton(chip, pressed: false, in: context)
        case .pan?:
            // In play mode every press starts a pan; a click blows or restores
            // a fuse, else flips a lever.
            if context.isRunning, value.isClick {
                let point = viewport.world(value.startLocation)
                if !toggleFuse(at: point, in: context) { toggleSwitchLever(at: point, in: context) }
            }
        default:
            break
        }
        if let press = selectionPress {
            document.releaseSelection(press, wasClick: value.isClick)
        }
        selectionPress = nil
        interaction = nil
        if document.isGestureOpen { document.endGesture() }
    }

    /// Presses or releases a tactile switch's button in the document and the
    /// running simulation (ON = contact closed: pressed on a Normal Open part,
    /// released on a Normal Close one). A press the document refuses changes
    /// nothing.
    private func pressButton(_ id: UUID, pressed: Bool, in context: SchematicCanvasContext) {
        guard let isPressed = context.document.setButton(id, pressed: pressed),
              let chip = context.document.chips.first(where: { $0.id == id }) else { return }
        context.simulation.setSwitch(id, position: 1, on: isPressed != chip.part.isNormallyClosed)
    }

    /// Blows the fuse under `point` in the running simulation, or restores
    /// it when it is blown (plan-47-M); any point of its body and leads
    /// counts. False when no fuse is there, so the click goes on to the
    /// switch levers.
    private func toggleFuse(at point: CGPoint, in context: SchematicCanvasContext) -> Bool {
        guard let chip = context.document.chip(at: point), chip.part.kind == PART_FUSE else { return false }
        context.simulation.toggleFuse(chip.id)
        return true
    }

    /// Flips the switch position whose lever is under `point`, in the
    /// document and, in play mode, in the running simulation. A position the
    /// document refuses to turn ON never reaches the simulation.
    private func toggleSwitchLever(at point: CGPoint, in context: SchematicCanvasContext) {
        let document = context.document
        // A slide switch moves its knob to the next position on a click
        // anywhere on its body (plan-58-G).
        if let chip = document.chip(at: point), let next = chip.nextSlidePosition {
            if let state = document.setSlideState(chip.id, to: next) {
                context.simulation.setSlide(chip.id, state: state)
            }
            return
        }
        guard let chip = document.chip(at: point),
              let position = chip.switchPosition(at: point),
              let isOn = document.toggleSwitch(chip.id, position: position) else { return }
        context.simulation.setSwitch(chip.id, position: position, on: isOn)
    }

    // MARK: Hover

    /// The pointer moved over the canvas to `location` (screen points), or
    /// left it (nil).
    func hover(_ location: CGPoint?, in context: SchematicCanvasContext) {
        if let location {
            let point = context.viewport.world(location)
            context.pointer?.point = point
            updateHover(at: point, in: context)
        } else {
            context.pointer?.point = nil
            clearHover()
        }
    }

    func clearHover() {
        hoveredChipID = nil
        hoveredUnitKey = nil
        hoveredWireID = nil
        hoveredBusID = nil
        hoveredPortID = nil
        hoveredNoteID = nil
        isTitleBlockHovered = false
        hoveredLabelID = nil
        hoverAnchor = nil
    }

    /// Highlights the block under the pointer, or else the wire segment under it.
    /// In Line mode, highlights only the connection point under the pointer.
    /// In play mode, nothing is highlighted.
    private func updateHover(at point: CGPoint, in context: SchematicCanvasContext) {
        let document = context.document
        clearHover()
        if context.isRunning {
            return
        }
        if context.tool == .line {
            hoverAnchor = context.lineAnchor(at: point)
            return
        }
        if let chip = document.piece(at: point) {
            hoveredChipID = chip.id
            hoveredUnitKey = chip.unitKey
        } else if let label = document.netLabel(at: point) {
            hoveredLabelID = label.id
        } else if let port = document.portHit(at: point) {
            hoveredPortID = port
        } else if let wire = document.segmentHit(at: point, tolerance: context.lineTolerance)?.wire {
            hoveredWireID = wire
        } else if let bus = document.busHit(at: point, tolerance: context.busTolerance)?.bus {
            hoveredBusID = bus
        } else if let note = document.note(at: point) {
            hoveredNoteID = note.id
        } else {
            isTitleBlockHovered = document.titleBlock(at: point)
        }
    }

    // MARK: Play mode, double-click, drop

    /// Play mode started or stopped.
    func runningChanged(_ running: Bool, in context: SchematicCanvasContext) {
        // Drop any highlight left from before play mode started (⌘R).
        if running {
            clearHover()
        } else {
            // Leaving play mode lets go of every held button.
            context.document.releaseAllButtons()
            if case .holdButton? = interaction { interaction = nil }
        }
    }

    /// A double-click at `location` (screen points).
    func doubleClick(at location: CGPoint, in context: SchematicCanvasContext) {
        guard context.tool == .pointer else { return }
        let document = context.document
        let point = context.viewport.world(location)
        if let chip = document.chip(at: point) {
            // Double-clicking a memory part opens its memory view, in play
            // mode too, which is when it has contents to show. A processor,
            // or a simulated microcontroller, opens its processor window the
            // same way; a serial part with a channel its terminal; a
            // converter, RTC or display driver its register window.
            if chip.part.isMemory { return context.onOpenMemory(chip.id) }
            if chip.part.hasProcessorWindow { return context.onOpenCpu(chip.id) }
            if chip.part.hasTerminal { return context.onOpenTerminal(chip.id) }
            if chip.part.hasRegisters { return context.onOpenRegisters(chip.id) }
        }
        guard !context.isRunning else { return }
        let onNothingElse = document.chip(at: point) == nil && document.netLabel(at: point) == nil
            && document.segmentHit(at: point, tolerance: context.lineTolerance) == nil
        // A text block under nothing else edits its words.
        if onNothingElse, let note = document.note(at: point) {
            document.select(.note(note.id))
            document.editingNoteID = note.id
            return
        }
        // The title block opens Document Info.
        if onNothingElse, document.titleBlock(at: point) {
            document.select(.titleBlock)
            context.onOpenDocumentInfo()
            return
        }
        document.insertNode(at: point, tolerance: context.lineTolerance)
    }

    /// A part dragged from the drawer dropped at `location` (screen points):
    /// placed centred there. False when it is refused (play mode, a part the
    /// catalog doesn't have).
    func drop(partNumber: String?, at location: CGPoint, in context: SchematicCanvasContext) -> Bool {
        guard !context.isRunning, let partNumber, let dropped = context.catalog.part(named: partNumber) else {
            return false
        }
        // A header takes its Settings size (plan-23-C); others are as dropped.
        let part = NewPartOptions.headerPart(for: dropped, catalog: context.catalog)
        context.document.placeChip(
            part: part,
            symbol: symbol(for: part),
            centeredAt: context.viewport.world(location),
            color: NewPartOptions.color(for: part),
            footprintID: NewPartOptions.footprintID(for: part),
            gender: NewPartOptions.gender(for: part)
        )
        context.onPartPlaced()
        return true
    }
}
