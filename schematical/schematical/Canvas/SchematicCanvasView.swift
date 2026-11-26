import AppKit
import SwiftUI

/// The editing surface. Its gestures depend on the active tool:
/// - Pointer: click a part, wire or junction to select it; Shift-click adds it
///   to the selection or takes it out (plan-15). Drag a part or junction to
///   move it. Drag a wire's bend point to move it, or drag a segment to shift
///   it. Shift-drag any of them, or drag an item that is part of a larger
///   selection, to move the whole selection by one snapped step (selected
///   wires move their bends; their ends stay on their pins). Double-click a
///   wire to add a bend there. Click a bus to select it; drag a bus line,
///   end, corner or T (lines stay straight, joints stay T or L, ports ride
///   along); drag a port to slide it along its line. Buses and ports are
///   selected on their own. Drag empty space to pan, which also clears the
///   selection; Shift-drag empty space to draw a box that adds every part,
///   junction and wire it touches to the selection. Dropping a part so a pin
///   lands on a loose wire end connects it.
/// - Line: drag from one pin to another to connect them with a right-angled wire.
///   Press Escape during the drag to cancel it (no wire is placed).
///   A grid point on an existing wire can be a start or end too; it becomes a
///   junction, so wires branch there. Only connection points are highlighted.
/// - Bus: drag to draw a bus line or L; start or end on a bus to join it at a
///   T or L, or to extend it. A straight drag between two line ends in line
///   with each other joins them end to end into one line. Leaving an L
///   straight out of one of its lines makes it a T. A drag from one bus to
///   another joins them into the bus it starts on. Press Escape during the
///   drag to cancel it (no bus is placed).
/// - Bus Port: click a bus line (not its ends or a T) to add an empty port
///   there; tick wires in its panel to give it pins.
/// - Eraser: click a block, wire or bus line to remove it. A block's wires stay,
///   with loose ends. A drag erases nothing.
/// - Flip: click a part to flip it left and right.
/// - VCC and GND: click the canvas to place a supply there.
/// - Net Label: click to add a net label there (or on a label, to select
///   it); its name and type are edited in the sidebar. Then back to the
///   Pointer tool. Labels with the same name are one net. With the
///   Pointer, a label drags like a part, and ⌘F flips it.
/// - Text: click to add a text block there (or on a block, to select it);
///   its words are edited in the sidebar. Then back to the Pointer tool.
///   With the Pointer, a text block drags like a part, and a double-click
///   edits its words.
///
/// Clicking a DIP switch lever flips that position, with the Pointer tool or
/// in play mode; clicking a slide switch moves its knob to the next position. A press that drags moves the part (or pans) instead.
///
/// Parts dragged from the drawer are dropped here and centred on the drop point.
/// The schematic is drawn in world coordinates and scaled by the viewport's zoom.
struct SchematicCanvasView: View {
    let catalog: PartCatalogStore
    let document: SchematicDocument
    let viewport: CanvasViewport
    let tool: SchematicTool
    /// The supply the VCC tool places (a `SupplyChoice` name).
    var supplyVoltage = SupplyChoice.defaultName
    /// The simulation. While it runs (play mode), the canvas only pans.
    let simulation: SimulationController
    private var isRunning: Bool { simulation.isRunning }
    /// Called with a memory part's id when it is double-clicked, to show its contents.
    var onOpenMemory: (UUID) -> Void = { _ in }
    /// Called with a processor part's id when it is double-clicked, to show its state.
    var onOpenCpu: (UUID) -> Void = { _ in }
    var onOpenTerminal: (UUID) -> Void = { _ in }
    var onOpenRegisters: (UUID) -> Void = { _ in }
    /// Called when the title block is double-clicked, to show Document Info.
    var onOpenDocumentInfo: () -> Void = {}
    /// Where the pointer is over the canvas (canvas units), for ⌘V; set as
    /// it moves.
    var pointer: BoardPointer?
    /// Called after a part dragged from the drawer is placed on the canvas,
    /// and after the Text tool adds or picks a text block.
    var onPartPlaced: () -> Void = {}

    @State private var interaction: Interaction?
    /// Zoom when the current pinch began, nil between pinches.
    @State private var pinchStartZoom: CGFloat?
    /// The press in progress as the system reported it, and how far the
    /// canvas has scrolled under it since it began (plan: a scroll moves the
    /// world, not the pointer). Both are cleared when the press ends.
    @State private var rawPress: DragPress?
    @State private var pressShift: CGSize = .zero
    /// What the current press did to the selection; a click finishes it.
    @State private var selectionPress: SelectionPress<SchematicItem>?
    @State private var isDropTargeted = false
    @State private var hoveredChipID: UUID?
    /// The unit of `hoveredChipID` under the pointer, for a part drawn as its units.
    @State private var hoveredUnitKey: String?
    @State private var hoveredWireID: UUID?
    @State private var hoveredBusID: UUID?
    @State private var hoveredPortID: UUID?
    @State private var hoveredNoteID: UUID?
    @State private var isTitleBlockHovered = false
    @State private var hoveredLabelID: UUID?
    /// The connection point under the pointer in Line mode, when not drawing.
    @State private var hoverAnchor: LineAnchor?
    /// Watches for Space, Esc and `/` while a wire or bus is being drawn.
    /// Installed through the editor's monitors, which the document empties
    /// when it closes.
    @State private var drawingKeyMonitor: UUID?
    @Environment(\.editorEventMonitors) private var eventMonitors

    private enum Interaction {
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

    /// Actual points per world unit (not the zoom level shown).
    private var zoom: CGFloat { viewport.scale }

    /// Hit tolerances are in screen points, so they are divided by zoom.
    private var lineTolerance: CGFloat { 6 / zoom }
    private var pinRadius: CGFloat { 8 / zoom }
    /// A bus is 6 wide, so half its width is added to the wire tolerance.
    private var busTolerance: CGFloat { lineTolerance + 3 / zoom }

    var body: some View {
        // The world layer is an overlay so its size never feeds back into layout.
        // Its grid size depends on the viewport, which depends on this view.
        Color.clear
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .overlay(alignment: .topLeading) {
                worldLayer
            }
            // Render the scaled world at screen resolution. Without this each
            // symbol is drawn at full size and then shrunk, and below about
            // half size thin lines fall between pixels and vanish.
            .drawingGroup()
            .background(Color(nsColor: .textBackgroundColor))
            .contentShape(Rectangle())
            .clipped()
            .onGeometryChange(for: CGSize.self) { proxy in
                proxy.size
            } action: { newSize in
                viewport.updateSize(newSize)
            }
            .onChange(of: document.ercFocus) { _, focus in
                // A line selected in the Electrical Rules Check: bring its mark into view.
                if let focus { viewport.reveal(focus.point) }
            }
            .gesture(canvasGesture)
            .simultaneousGesture(pinchGesture)
            .scrollPans(viewport) { scrolledMidPress(by: $0) }
            .onContinuousHover { phase in
                switch phase {
                case let .active(location):
                    pointer?.point = viewport.world(location)
                    updateHover(at: viewport.world(location))
                case .ended:
                    pointer?.point = nil
                    clearHover()
                }
            }
            .onAppear { installDrawingKeyMonitor() }
            .onDisappear { removeDrawingKeyMonitor() }
            .onChange(of: isRunning) { _, running in
                // Drop any highlight left from before play mode started (⌘R).
                if running {
                    clearHover()
                } else {
                    // Leaving play mode lets go of every held button.
                    document.releaseAllButtons()
                    if case .holdButton? = interaction { interaction = nil }
                }
            }
            .simultaneousGesture(
                SpatialTapGesture(count: 2).onEnded { value in
                    guard tool == .pointer else { return }
                    let point = viewport.world(value.location)
                    // Double-clicking a memory part opens its memory view, in
                    // play mode too, which is when it has contents to show.
                    if let chip = document.chip(at: point), chip.part.isMemory {
                        onOpenMemory(chip.id)
                        return
                    }
                    // A processor, or a simulated microcontroller, opens its
                    // processor window the same way.
                    if let chip = document.chip(at: point), chip.part.hasProcessorWindow {
                        onOpenCpu(chip.id)
                        return
                    }
                    // A serial part with a channel opens its terminal.
                    if let chip = document.chip(at: point), chip.part.hasTerminal {
                        onOpenTerminal(chip.id)
                        return
                    }
                    // A converter, RTC or display driver opens its register window.
                    if let chip = document.chip(at: point), chip.part.hasRegisters {
                        onOpenRegisters(chip.id)
                        return
                    }
                    guard !isRunning else { return }
                    // A text block under nothing else edits its words.
                    if document.chip(at: point) == nil, document.netLabel(at: point) == nil,
                       document.segmentHit(at: point, tolerance: lineTolerance) == nil,
                       let note = document.note(at: point) {
                        document.select(.note(note.id))
                        document.editingNoteID = note.id
                        return
                    }
                    // The title block opens Document Info.
                    if document.chip(at: point) == nil, document.netLabel(at: point) == nil,
                       document.segmentHit(at: point, tolerance: lineTolerance) == nil,
                       document.titleBlock(at: point) {
                        document.select(.titleBlock)
                        onOpenDocumentInfo()
                        return
                    }
                    document.insertNode(at: point, tolerance: lineTolerance)
                }
            )
            .dropDestination(for: ChipPartDragItem.self) { items, location in
                guard !isRunning, let item = items.first,
                      let dropped = catalog.part(named: item.partNumber) else {
                    return false
                }
                // A header takes its Settings size (plan-23-C); others are as dropped.
                let part = NewPartOptions.headerPart(for: dropped, catalog: catalog)
                document.placeChip(
                    part: part,
                    symbol: symbol(for: part),
                    centeredAt: viewport.world(location),
                    color: NewPartOptions.color(for: part),
                    footprintID: NewPartOptions.footprintID(for: part),
                    gender: NewPartOptions.gender(for: part)
                )
                onPartPlaced()
                return true
            } isTargeted: { isDropTargeted = $0 }
            .overlay(alignment: .topLeading) {
                if case let .boxSelect(_, from, to)? = interaction {
                    SelectionBox(from: from, to: to)
                }
            }
            .overlay {
                if isDropTargeted {
                    Rectangle()
                        .strokeBorder(
                            Color.accentColor.opacity(0.6),
                            style: StrokeStyle(lineWidth: 2, dash: [8, 6])
                        )
                        .allowsHitTesting(false)
                }
            }
    }

    /// Everything in schematic coordinates, scaled and shifted by the viewport.
    private var worldLayer: some View {
        ZStack(alignment: .topLeading) {
            GridBackground(rect: gridRect, step: gridStep)
                .equatable()

            // The title block is under the text blocks, so a note can sit on it.
            TitleBlockLayer(layout: document.titleBlockLayout,
                            isSelected: document.schematicSelection.contains(.titleBlock),
                            isHovered: isTitleBlockHovered, area: gridRect)

            // Under everything else: wires and parts stay visible over a block.
            NoteLayer(
                notes: document.notes,
                selectedNoteIDs: selectedNoteIDs,
                hoveredNoteID: hoveredNoteID,
                area: gridRect
            )

            // Under the wires, so wires crossing a bus stay visible.
            BusLayer(
                buses: document.buses,
                selectedBusIDs: selectedBusIDs,
                hoveredBusID: hoveredBusID,
                preview: busPreview,
                area: gridRect
            )

            PortLayer(
                buses: document.buses,
                selectedPortID: document.selectedPortID,
                hoveredPortID: hoveredPortID,
                area: gridRect
            )

            // Their own views, which read the wires, junctions and parts:
            // an edit to a label, bus or note leaves their drawing alone.
            SchematicWiresLayer(
                document: document,
                simulation: simulation,
                selectedWireIDs: selectedWireIDs,
                hoveredWireID: hoveredWireID,
                preview: connectionPreview,
                area: gridRect
            )

            NetLabelLayer(
                labels: document.netLabels,
                selectedIDs: selectedLabelIDs,
                hoveredID: hoveredLabelID,
                area: gridRect
            )

            SchematicPartsLayer(document: document, simulation: simulation, hoveredChipID: hoveredChipID,
                                hoveredUnitKey: hoveredUnitKey)

            PinMarkerLayer(
                tips: tool == .line ? document.pinTips : [],
                highlight: highlightPoint,
                area: gridRect
            )

            // The Electrical Rules Check's marks: on screen only, never in an export.
            if !document.ercMarkers.isEmpty {
                ErcMarkerLayer(markers: document.ercMarkers, selected: document.ercSelectedKeys, area: gridRect)
            }
        }
        .scaleEffect(zoom, anchor: .topLeading)
        .offset(viewport.offset)
    }

    /// The wires in the Schematic selection.
    private var selectedWireIDs: Set<UUID> {
        Set(document.schematicSelection.items.compactMap { item in
            if case let .wire(id) = item { return id }
            return nil
        })
    }

    /// The net labels in the Schematic selection.
    private var selectedLabelIDs: Set<UUID> {
        Set(document.schematicSelection.items.compactMap { item in
            if case let .label(id) = item { return id }
            return nil
        })
    }

    /// The text blocks in the Schematic selection.
    private var selectedNoteIDs: Set<UUID> {
        Set(document.schematicSelection.items.compactMap { item in
            if case let .note(id) = item { return id }
            return nil
        })
    }

    /// The buses in the Schematic selection.
    private var selectedBusIDs: Set<UUID> {
        Set(document.schematicSelection.items.compactMap { item in
            if case let .bus(id) = item { return id }
            return nil
        })
    }

    /// Grid spacing in world units. Dots get sparse when zoomed out so they stay legible.
    private var gridStep: CGFloat {
        zoom < 0.5 ? SchematicGrid.spacing * 5 : SchematicGrid.spacing
    }

    /// The visible world area, widened to whole grid steps.
    private var gridRect: CGRect {
        let visible = viewport.visibleWorldRect
        let step = gridStep
        let minX = (visible.minX / step).rounded(.down) * step
        let minY = (visible.minY / step).rounded(.down) * step
        let maxX = (visible.maxX / step).rounded(.up) * step
        let maxY = (visible.maxY / step).rounded(.up) * step
        return CGRect(x: minX, y: minY, width: maxX - minX, height: maxY - minY)
    }

    /// The Line route being drawn: locked legs, then the live leg to the
    /// anchor under the cursor or the snapped cursor.
    private var connectionPreview: ConnectionPreview? {
        guard case let .connecting(from, draft, cursor)? = interaction else { return nil }
        let end = lineEnd(from: from, cursor: cursor)
        return ConnectionPreview(points: draft.path(to: end.point, endAxis: end.axis),
                                 target: end.target == nil ? nil : end.point)
    }

    /// Where a Line route ends for `cursor`: the tip of the anchor under it
    /// (other than the start) and the axis that pin forces, or else the
    /// snapped cursor. The preview, Space and the release all use this.
    private func lineEnd(from: LineAnchor, cursor: CGPoint) -> (point: CGPoint, axis: RouteAxis?, target: LineAnchor?) {
        if let anchor = lineAnchor(at: cursor), anchor != from, let tip = document.anchorTip(anchor) {
            return (tip, document.anchorAxis(anchor), anchor)
        }
        return (SchematicGrid.snap(cursor), nil, nil)
    }

    /// What the Bus tool would draw: the lines `addBus` would add, or the
    /// route as drawn when releasing would be refused.
    private var busPreview: BusPreview? {
        guard case let .drawingBus(draft, cursor)? = interaction else { return nil }
        let candidates = draft.busCandidates(to: SchematicGrid.snap(cursor))
        guard let drawn = candidates.first, drawn.count >= 2 else { return nil }
        switch document.busDraft(candidates: candidates) {
        case let .success(result):
            return BusPreview(points: result.path, refused: false)
        case .failure:
            return BusPreview(points: drawn, refused: true)
        }
    }

    /// The point to highlight in Line mode: where a line would land while it is
    /// being drawn, or else the connection point under the pointer. Nothing else is
    /// highlighted in this mode.
    private var highlightPoint: CGPoint? {
        guard tool == .line else { return nil }
        if let preview = connectionPreview {
            return preview.target
        }
        return hoverAnchor.flatMap { document.anchorTip($0) }
    }

    private func lineAnchor(at point: CGPoint) -> LineAnchor? {
        document.lineAnchor(at: point, pinRadius: pinRadius, gridRadius: lineTolerance)
    }

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
    private func handleDrawingKey(_ event: NSEvent) -> Bool {
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
            let end = lineEnd(from: from, cursor: cursor)
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
                if candidates.count > 1, case let .success(result) = document.busDraft(candidates: candidates),
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

    /// Trackpad pinch: zooms about the point the fingers started on.
    private var pinchGesture: some Gesture {
        MagnifyGesture()
            .onChanged { value in
                let start = pinchStartZoom ?? viewport.zoom
                pinchStartZoom = start
                viewport.setZoom(start * value.magnification, around: value.startLocation)
            }
            .onEnded { _ in pinchStartZoom = nil }
    }

    private var canvasGesture: some Gesture {
        DragGesture(minimumDistance: 0)
            .onChanged { value in
                let raw = DragPress(value)
                rawPress = raw
                handleChanged(raw.shifted(by: pressShift))
            }
            .onEnded { value in
                handleEnded(DragPress(value).shifted(by: pressShift))
                rawPress = nil
                pressShift = .zero
            }
    }

    /// The canvas scrolled by `shift` while a press is down: the press's start
    /// moves with the content (so a drag, a wire or a box stays anchored to the
    /// world it began on) and the drag's effect is worked out again for the
    /// pointer's unchanged place.
    private func scrolledMidPress(by shift: CGSize) {
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
        handleChanged(raw.shifted(by: pressShift))
    }

    private func handleChanged(_ value: DragPress) {
        // A press on the canvas ends any text field's editing (the parts
        // search, a sidebar field), as a click outside a field does in any
        // Mac app, so ⌘C / ⌘V / ⌘X / ⌘A then reach the Schematic. A new
        // window or tab starts with its search field focused.
        if interaction == nil, let window = NSApp.keyWindow, window.firstResponder is NSText {
            window.makeFirstResponder(nil)
        }
        let start = viewport.world(value.startLocation)
        let current = viewport.world(value.location)
        let delta = CGSize(
            width: value.translation.width / zoom,
            height: value.translation.height / zoom
        )

        // Play mode: a press on a tactile switch's button holds it down
        // until the mouse comes up; a drag anywhere else pans, and nothing
        // else can start.
        if isRunning {
            if interaction == nil, let chip = document.chip(at: start), chip.buttonHit(at: start) {
                interaction = .holdButton(chip: chip.id)
                pressButton(chip.id, pressed: true)
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

        switch tool {
        case .pointer:
            if interaction == nil {
                beginPointerInteraction(at: start, screen: value.startLocation)
            }
            switch interaction {
            case let .moveGroup(base, _)?:
                document.moveSchematicGroup(base, by: delta)

            case let .dragNode(wire, base, index)?:
                document.setWirePath(
                    wire,
                    OrthogonalRoute.moveNode(base, at: index, to: SchematicGrid.snap(current))
                )

            case let .dragSegment(wire, base, index)?:
                let offset = CGSize(
                    width: SchematicGrid.snap(delta.width),
                    height: SchematicGrid.snap(delta.height)
                )
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

        case .line:
            if interaction == nil {
                if let anchor = lineAnchor(at: start), let tip = document.anchorTip(anchor) {
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
                document.addPort(at: start, tolerance: busTolerance)
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
                onPartPlaced()
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
                onPartPlaced()
            }
            interaction = .idle

        case .vcc, .ground, .pullUp, .pullDown:
            // One supply per press, centred under where the press started.
            if interaction == nil,
               let name = tool.supplyName(voltage: supplyVoltage),
               let part = catalog.part(named: name) {
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

    /// Highlights the block under the pointer, or else the wire segment under it.
    /// In Line mode, highlights only the connection point under the pointer.
    /// In play mode, nothing is highlighted.
    private func clearHover() {
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

    private func updateHover(at point: CGPoint) {
        clearHover()
        if isRunning {
            return
        }
        if tool == .line {
            hoverAnchor = lineAnchor(at: point)
            return
        }
        if let chip = document.piece(at: point) {
            hoveredChipID = chip.id
            hoveredUnitKey = chip.unitKey
        } else if let label = document.netLabel(at: point) {
            hoveredLabelID = label.id
        } else if let port = document.portHit(at: point) {
            hoveredPortID = port
        } else if let wire = document.segmentHit(at: point, tolerance: lineTolerance)?.wire {
            hoveredWireID = wire
        } else if let bus = document.busHit(at: point, tolerance: busTolerance)?.bus {
            hoveredBusID = bus
        } else if let note = document.note(at: point) {
            hoveredNoteID = note.id
        } else {
            isTitleBlockHovered = document.titleBlock(at: point)
        }
    }

    /// Decides what a Pointer press does. Shift is read once, here, at the
    /// start of the press. `screen` is the press in screen points.
    private func beginPointerInteraction(at point: CGPoint, screen: CGPoint) {
        let shift = NSEvent.modifierFlags.contains(.shift)
        if let hit = document.piece(at: point) {
            // A unit of a part drawn as its units is picked; the drag then moves the picked units.
            selectionPress = document.pressPiece(hit, shift: shift)
            interaction = .moveGroup(base: document.schematicGroupBase(), lever: shift ? nil : hit.id)
        } else if let label = document.netLabel(at: point) {
            selectionPress = document.pressSelection(.label(label.id), shift: shift)
            interaction = .moveGroup(base: document.schematicGroupBase(), lever: nil)
        } else if let port = document.portHit(at: point), let found = document.port(port) {
            // A press selects the port; dragging slides it along its line.
            document.selectedPortID = port
            interaction = .slidePort(port: port, base: found.port.start)
        } else if let junction = document.junctionHit(at: point, tolerance: lineTolerance) {
            selectionPress = document.pressSelection(.junction(junction.id), shift: shift)
            interaction = .moveGroup(base: document.schematicGroupBase(), lever: nil)
        } else if let node = document.nodeHit(at: point, tolerance: lineTolerance) {
            let press = document.pressSelection(.wire(node.wire), shift: shift)
            selectionPress = press
            interaction = press.dragsGroup
                ? .moveGroup(base: document.schematicGroupBase(), lever: nil)
                : .dragNode(wire: node.wire, base: document.path(forWire: node.wire), index: node.index)
        } else if let segment = document.segmentHit(at: point, tolerance: lineTolerance) {
            let press = document.pressSelection(.wire(segment.wire), shift: shift)
            selectionPress = press
            interaction = press.dragsGroup
                ? .moveGroup(base: document.schematicGroupBase(), lever: nil)
                : .dragSegment(wire: segment.wire, base: document.path(forWire: segment.wire), index: segment.index)
        } else if let end = document.busEndHit(at: point, tolerance: busTolerance),
                  let base = document.bus(end.bus) {
            // As for a wire: alone, a drag reshapes the bus; in a larger
            // selection (or with Shift) it moves the group.
            let press = document.pressSelection(.bus(end.bus), shift: shift)
            selectionPress = press
            interaction = press.dragsGroup
                ? .moveGroup(base: document.schematicGroupBase(), lever: nil)
                : .dragBusEnd(bus: end.bus, base: base, line: end.line, end: end.end, done: .zero)
        } else if let line = document.busHit(at: point, tolerance: busTolerance),
                  let base = document.bus(line.bus) {
            let press = document.pressSelection(.bus(line.bus), shift: shift)
            selectionPress = press
            interaction = press.dragsGroup
                ? .moveGroup(base: document.schematicGroupBase(), lever: nil)
                : .dragBusLine(bus: line.bus, base: base, line: line.line, done: .zero)
        } else if let note = document.note(at: point) {
            selectionPress = document.pressSelection(.note(note.id), shift: shift)
            interaction = .moveGroup(base: document.schematicGroupBase(), lever: nil)
        } else if document.titleBlock(at: point) {
            selectionPress = document.pressSelection(.titleBlock, shift: shift)
            interaction = .moveGroup(base: document.schematicGroupBase(), lever: nil)
        } else if shift {
            interaction = .boxSelect(base: document.schematicSelection.items, from: screen, to: screen)
        } else {
            document.pressEmptySchematic(shift: false)
            interaction = .pan(startOffset: viewport.offset)
        }
    }

    private func handleEnded(_ value: DragPress) {
        switch interaction {
        case let .connecting(from, draft, _)?:
            let end = lineEnd(from: from, cursor: viewport.world(value.location))
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
            if lever != nil, isClick(value) {
                toggleSwitchLever(at: viewport.world(value.startLocation))
            }
            document.settleSchematicGroup(base)
        case .erasing?:
            // A drag erases nothing.
            if isClick(value) {
                document.erase(at: viewport.world(value.startLocation), tolerance: lineTolerance)
            }
        case let .holdButton(chip)?:
            pressButton(chip, pressed: false)
        case .pan?:
            // In play mode every press starts a pan; a click blows or restores
            // a fuse, else flips a lever.
            if isRunning, isClick(value) {
                let point = viewport.world(value.startLocation)
                if !toggleFuse(at: point) { toggleSwitchLever(at: point) }
            }
        default:
            break
        }
        if let press = selectionPress {
            document.releaseSelection(press, wasClick: isClick(value))
        }
        selectionPress = nil
        interaction = nil
        if document.isGestureOpen { document.endGesture() }
    }

    /// A press that ended about where it started, in screen points.
    private func isClick(_ value: DragPress) -> Bool {
        hypot(value.translation.width, value.translation.height) < 3
    }

    /// Presses or releases a tactile switch's button in the document and the
    /// running simulation (ON = contact closed: pressed on a Normal Open part,
    /// released on a Normal Close one). A press the document refuses changes
    /// nothing.
    private func pressButton(_ id: UUID, pressed: Bool) {
        guard let isPressed = document.setButton(id, pressed: pressed),
              let chip = document.chips.first(where: { $0.id == id }) else { return }
        simulation.setSwitch(id, position: 1, on: isPressed != chip.part.isNormallyClosed)
    }

    /// Blows the fuse under `point` in the running simulation, or restores
    /// it when it is blown (plan-47-M); any point of its body and leads
    /// counts. False when no fuse is there, so the click goes on to the
    /// switch levers.
    private func toggleFuse(at point: CGPoint) -> Bool {
        guard let chip = document.chip(at: point), chip.part.kind == PART_FUSE else { return false }
        simulation.toggleFuse(chip.id)
        return true
    }

    /// Flips the switch position whose lever is under `point`, in the
    /// document and, in play mode, in the running simulation. A position the
    /// document refuses to turn ON never reaches the simulation.
    private func toggleSwitchLever(at point: CGPoint) {
        // A slide switch moves its knob to the next position on a click
        // anywhere on its body (plan-58-G).
        if let chip = document.chip(at: point), let next = chip.nextSlidePosition {
            if let state = document.setSlideState(chip.id, to: next) {
                simulation.setSlide(chip.id, state: state)
            }
            return
        }
        guard let chip = document.chip(at: point),
              let position = chip.switchPosition(at: point),
              let isOn = document.toggleSwitch(chip.id, position: position) else { return }
        simulation.setSwitch(chip.id, position: position, on: isOn)
    }
}
