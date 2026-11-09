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
///   junction and wire it touches to the selection.
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
/// - Eraser: click a block, wire or bus line to remove it. A drag erases nothing.
/// - Flip: click a part to flip it left and right.
/// - VCC and GND: click the canvas to place a supply there.
/// - Text: click to add a text block there (or on a block, to select it);
///   its words are edited in the sidebar. Then back to the Pointer tool.
///   With the Pointer, a text block drags like a part, and a double-click
///   edits its words.
///
/// Clicking a DIP switch lever flips that position, with the Pointer tool or
/// in play mode. A press that drags moves the part (or pans) instead.
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
    /// Called after a part dragged from the drawer is placed on the canvas,
    /// and after the Text tool adds or picks a text block.
    var onPartPlaced: () -> Void = {}

    @State private var interaction: Interaction?
    /// What the current press did to the selection; a click finishes it.
    @State private var selectionPress: SelectionPress<SchematicItem>?
    @State private var isDropTargeted = false
    @State private var hoveredChipID: UUID?
    @State private var hoveredWireID: UUID?
    @State private var hoveredBusID: UUID?
    @State private var hoveredPortID: UUID?
    @State private var hoveredNoteID: UUID?
    /// The connection point under the pointer in Line mode, when not drawing.
    @State private var hoverAnchor: LineAnchor?
    /// Watches for Space, Esc and `/` while a wire or bus is being drawn.
    @State private var drawingKeyMonitor: Any?

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
            .gesture(canvasGesture)
            .onContinuousHover { phase in
                switch phase {
                case let .active(location):
                    updateHover(at: viewport.world(location))
                case .ended:
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
                    // A processor opens its processor window the same way.
                    if let chip = document.chip(at: point), chip.part.isCpu {
                        onOpenCpu(chip.id)
                        return
                    }
                    // A serial part with a channel opens its terminal.
                    if let chip = document.chip(at: point), chip.part.hasTerminal {
                        onOpenTerminal(chip.id)
                        return
                    }
                    // A converter or RTC opens its register window.
                    if let chip = document.chip(at: point), chip.part.hasRegisters {
                        onOpenRegisters(chip.id)
                        return
                    }
                    guard !isRunning else { return }
                    // A text block under nothing else edits its words.
                    if document.chip(at: point) == nil,
                       document.segmentHit(at: point, tolerance: lineTolerance) == nil,
                       let note = document.note(at: point) {
                        document.select(.note(note.id))
                        document.editingNoteID = note.id
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
                selectedBusID: document.selectedBusID,
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

            WireLayer(
                routes: document.wires.map { wire in
                    RoutedWire(
                        id: wire.id,
                        points: document.path(forWire: wire.id),
                        style: simulation.wireStyle(wire.id)
                    )
                },
                junctions: document.junctions.map { junction in
                    JunctionDot(point: junction.point, isSelected: document.isSelected(.junction(junction.id)))
                },
                selectedWireIDs: selectedWireIDs,
                hoveredWireID: hoveredWireID,
                preview: connectionPreview,
                area: gridRect
            )

            ForEach(document.chips) { chip in
                Group {
                    switch chip.symbol {
                    case .block, .oscillator:
                        ChipBlockView(
                            chip: chip,
                            isSelected: document.isSelected(.chip(chip.id)),
                            isHovered: chip.id == hoveredChipID
                        )
                    case .barrelJack:
                        BarrelJackSymbolView(
                            chip: chip,
                            isSelected: document.isSelected(.chip(chip.id)),
                            isHovered: chip.id == hoveredChipID
                        )
                    case .header:
                        HeaderSymbolView(
                            chip: chip,
                            isSelected: document.isSelected(.chip(chip.id)),
                            isHovered: chip.id == hoveredChipID
                        )
                    case .diode:
                        DiodeSymbolView(
                            chip: chip,
                            isSelected: document.isSelected(.chip(chip.id)),
                            isHovered: chip.id == hoveredChipID,
                            level: simulation.lights[chip.id]?.first ?? 0
                        )
                    case .vcc, .ground, .pullUp, .pullDown:
                        SupplySymbolView(
                            chip: chip,
                            isSelected: document.isSelected(.chip(chip.id)),
                            isHovered: chip.id == hoveredChipID
                        )
                    case .dipSwitch:
                        DipSwitchSymbolView(
                            chip: chip,
                            isSelected: document.isSelected(.chip(chip.id)),
                            isHovered: chip.id == hoveredChipID
                        )
                    case .tactileSwitch:
                        TactileSwitchSymbolView(
                            chip: chip,
                            isSelected: document.isSelected(.chip(chip.id)),
                            isHovered: chip.id == hoveredChipID
                        )
                    case .sevenSegment:
                        SevenSegmentSymbolView(
                            chip: chip,
                            isSelected: document.isSelected(.chip(chip.id)),
                            isHovered: chip.id == hoveredChipID,
                            levels: simulation.lights[chip.id] ?? []
                        )
                    case .characterLCD:
                        CharacterLCDSymbolView(
                            chip: chip,
                            isSelected: document.isSelected(.chip(chip.id)),
                            isHovered: chip.id == hoveredChipID,
                            frame: simulation.lcdFrames[chip.id],
                            backlight: simulation.lights[chip.id]?.first ?? 0
                        )
                    }
                }
                .offset(x: chip.viewOrigin.x, y: chip.viewOrigin.y)
            }

            PinMarkerLayer(
                tips: tool == .line ? document.pinTips : [],
                highlight: highlightPoint,
                area: gridRect
            )
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

    /// The text blocks in the Schematic selection.
    private var selectedNoteIDs: Set<UUID> {
        Set(document.schematicSelection.items.compactMap { item in
            if case let .note(id) = item { return id }
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
        drawingKeyMonitor = NSEvent.addLocalMonitorForEvents(matching: .keyDown) { event in
            handleDrawingKey(event) ? nil : event
        }
    }

    private func removeDrawingKeyMonitor() {
        if let drawingKeyMonitor {
            NSEvent.removeMonitor(drawingKeyMonitor)
        }
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
                if draft.canFlip(endAxis: end.axis) { draft.flip() }
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
                if draft.canFlip(endAxis: nil) { draft.flip() }
            }
            interaction = .drawingBus(draft: draft, cursor: cursor)
            return true

        default:
            return false
        }
    }

    private var canvasGesture: some Gesture {
        DragGesture(minimumDistance: 0)
            .onChanged { value in handleChanged(value) }
            .onEnded { value in handleEnded(value) }
    }

    private func handleChanged(_ value: DragGesture.Value) {
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

        case .flip:
            // One flip per press: the part under where the press started.
            if interaction == nil, let hit = document.chip(at: start) {
                document.flipChip(hit.id)
            }
            interaction = .idle

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
        hoveredWireID = nil
        hoveredBusID = nil
        hoveredPortID = nil
        hoveredNoteID = nil
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
        if let chip = document.chip(at: point) {
            hoveredChipID = chip.id
        } else if let port = document.portHit(at: point) {
            hoveredPortID = port
        } else if let wire = document.segmentHit(at: point, tolerance: lineTolerance)?.wire {
            hoveredWireID = wire
        } else if let bus = document.busHit(at: point, tolerance: busTolerance)?.bus {
            hoveredBusID = bus
        } else {
            hoveredNoteID = document.note(at: point)?.id
        }
    }

    /// Decides what a Pointer press does. Shift is read once, here, at the
    /// start of the press. `screen` is the press in screen points.
    private func beginPointerInteraction(at point: CGPoint, screen: CGPoint) {
        let shift = NSEvent.modifierFlags.contains(.shift)
        if let hit = document.chip(at: point) {
            selectionPress = document.pressSelection(.chip(hit.id), shift: shift)
            interaction = .moveGroup(base: document.schematicGroupBase(), lever: shift ? nil : hit.id)
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
            document.selectedBusID = end.bus
            interaction = .dragBusEnd(bus: end.bus, base: base, line: end.line, end: end.end, done: .zero)
        } else if let line = document.busHit(at: point, tolerance: busTolerance),
                  let base = document.bus(line.bus) {
            document.selectedBusID = line.bus
            interaction = .dragBusLine(bus: line.bus, base: base, line: line.line, done: .zero)
        } else if let note = document.note(at: point) {
            selectionPress = document.pressSelection(.note(note.id), shift: shift)
            interaction = .moveGroup(base: document.schematicGroupBase(), lever: nil)
        } else if shift {
            interaction = .boxSelect(base: document.schematicSelection.items, from: screen, to: screen)
        } else {
            document.pressEmptySchematic(shift: false)
            interaction = .pan(startOffset: viewport.offset)
        }
    }

    private func handleEnded(_ value: DragGesture.Value) {
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
            // In play mode every press starts a pan; a click flips a lever.
            if isRunning, isClick(value) {
                toggleSwitchLever(at: viewport.world(value.startLocation))
            }
        default:
            break
        }
        if let press = selectionPress {
            document.releaseSelection(press, wasClick: isClick(value))
        }
        selectionPress = nil
        interaction = nil
    }

    /// A press that ended about where it started, in screen points.
    private func isClick(_ value: DragGesture.Value) -> Bool {
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

    /// Flips the switch position whose lever is under `point`, in the
    /// document and, in play mode, in the running simulation. A position the
    /// document refuses to turn ON never reaches the simulation.
    private func toggleSwitchLever(at point: CGPoint) {
        guard let chip = document.chip(at: point),
              let position = chip.switchPosition(at: point),
              let isOn = document.toggleSwitch(chip.id, position: position) else { return }
        simulation.setSwitch(chip.id, position: position, on: isOn)
    }
}

/// The box a Shift-drag on empty canvas draws, between two screen points.
struct SelectionBox: View {
    let from: CGPoint
    let to: CGPoint

    var body: some View {
        let rect = CGRect(x: min(from.x, to.x), y: min(from.y, to.y),
                          width: abs(from.x - to.x), height: abs(from.y - to.y))
        Rectangle()
            .fill(Color.accentColor.opacity(0.12))
            .overlay(Rectangle().strokeBorder(Color.accentColor.opacity(0.8), lineWidth: 1))
            .frame(width: rect.width, height: rect.height)
            .offset(x: rect.minX, y: rect.minY)
            .allowsHitTesting(false)
    }
}

/// Dot grid in world coordinates. Equatable so it only redraws when the visible area changes.
private struct GridBackground: View, Equatable {
    let rect: CGRect
    let step: CGFloat

    var body: some View {
        Canvas { context, _ in
            var dots = Path()

            var x = rect.minX
            while x <= rect.maxX {
                var y = rect.minY
                while y <= rect.maxY {
                    dots.addEllipse(in: CGRect(
                        x: x - rect.minX - 0.75,
                        y: y - rect.minY - 0.75,
                        width: 1.5,
                        height: 1.5
                    ))
                    y += step
                }
                x += step
            }

            context.fill(dots, with: .color(Color.secondary.opacity(0.5)))
        }
        .frame(width: rect.width, height: rect.height)
        .offset(x: rect.minX, y: rect.minY)
    }
}

/// The route being drawn from a pin, and the pin it would land on, if any.
struct ConnectionPreview {
    let points: [CGPoint]
    let target: CGPoint?
}

struct RoutedWire: Identifiable {
    let id: UUID
    let points: [CGPoint]
    /// Play mode: the net's value, toggling, floating or shorted.
    var style: WireStyle = .normal
}

private extension View {
    /// Sizes a world-coordinate `Canvas` to `rect` (the visible world area)
    /// and places it there. A Canvas only shows what falls inside its own frame,
    /// so without this anything at a negative coordinate, or past the view's
    /// size, is cut off. The Canvas must shift its drawing by `-rect.origin`.
    func worldCanvas(_ rect: CGRect) -> some View {
        frame(width: rect.width, height: rect.height)
            .offset(x: rect.minX, y: rect.minY)
    }
}

/// Moves a Canvas's drawing so world point `rect.origin` is at its top-left;
/// pairs with `worldCanvas(_:)`.
private extension GraphicsContext {
    mutating func shiftToWorld(_ rect: CGRect) {
        translateBy(x: -rect.minX, y: -rect.minY)
    }
}

/// A small circle over each pin, shown in Line mode. The connection point under
/// the pointer, or the one a line would land on, is filled green.
private struct PinMarkerLayer: View {
    let tips: [CGPoint]
    let highlight: CGPoint?
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            context.shiftToWorld(area)
            for tip in tips where tip != highlight {
                let rect = CGRect(x: tip.x - 4, y: tip.y - 4, width: 8, height: 8)
                context.stroke(Path(ellipseIn: rect), with: .color(.accentColor), lineWidth: 1.25)
            }

            if let highlight {
                let rect = CGRect(x: highlight.x - 6, y: highlight.y - 6, width: 12, height: 12)
                context.fill(Path(ellipseIn: rect), with: .color(.green))
                context.stroke(Path(ellipseIn: rect), with: .color(.green.opacity(0.6)), lineWidth: 1)
            }
        }
        .worldCanvas(area)
    }
}

/// Draws text blocks: each line of the words from the top-left, inside an
/// optional frame. A selected block's frame (or, unframed, its outline) is
/// accent; the hovered one gets a soft accent underlay. Not private, so the
/// export can draw it too.
struct NoteLayer: View {
    let notes: [SchematicNote]
    var selectedNoteIDs: Set<UUID> = []
    var hoveredNoteID: UUID?
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            var shifted = context
            shifted.shiftToWorld(area)
            var painter = ScreenPainter(shifted)
            Self.paint(&painter, notes: notes, selectedNoteIDs: selectedNoteIDs, hoveredNoteID: hoveredNoteID)
        }
        .worldCanvas(area)
    }

    /// Draws the blocks in world coordinates.
    static func paint<P: SchematicPainter>(_ painter: inout P, notes: [SchematicNote],
                                           selectedNoteIDs: Set<UUID>, hoveredNoteID: UUID?) {
        for note in notes {
            let rect = note.rect
            let frame = Path(rect)
            let isSelected = selectedNoteIDs.contains(note.id)
            if note.id == hoveredNoteID, !isSelected {
                painter.stroke(frame, with: Ink.accent.opacity(0.35), lineWidth: 5)
            }
            if note.framed {
                painter.fill(frame, with: .paper)
                painter.stroke(frame, with: isSelected ? .accent : .secondary, lineWidth: isSelected ? 2 : 1)
            } else if isSelected {
                painter.stroke(frame, with: .accent,
                               style: StrokeStyle(lineWidth: 1.5, dash: [4, 3]))
            }
            for (index, line) in note.lines.enumerated() where !line.isEmpty {
                painter.draw(PaintedText(line, size: CGFloat(note.size)), at: note.lineOrigin(index),
                             anchor: .topLeading)
            }
        }
    }
}

/// A junction's dot, accent when it is selected.
struct JunctionDot {
    let point: CGPoint
    var isSelected = false
}

/// Draws wires as orthogonal polylines, plus the route being drawn from a pin.
/// The hovered wire gets a soft accent underlay; selected wires are drawn in
/// the accent colour. Junctions get a solid dot, larger and accent when
/// selected. Not private, so the harness can render it on its own.
struct WireLayer: View {
    let routes: [RoutedWire]
    let junctions: [JunctionDot]
    var selectedWireIDs: Set<UUID> = []
    let hoveredWireID: UUID?
    let preview: ConnectionPreview?
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            var shifted = context
            shifted.shiftToWorld(area)
            var painter = ScreenPainter(shifted)
            Self.paint(&painter, routes: routes, junctions: junctions, selectedWireIDs: selectedWireIDs,
                       hoveredWireID: hoveredWireID, preview: preview?.points)
        }
        .worldCanvas(area)
    }

    /// Draws the wires in world coordinates. `preview` is the route being drawn.
    static func paint<P: SchematicPainter>(_ painter: inout P, routes: [RoutedWire], junctions: [JunctionDot],
                                           selectedWireIDs: Set<UUID>, hoveredWireID: UUID?, preview: [CGPoint]?) {
        let solid = StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round)
        let underlay = StrokeStyle(lineWidth: 6, lineCap: .round, lineJoin: .round)
        for route in routes {
            if route.id == hoveredWireID {
                painter.stroke(polyline(route.points), with: Ink.accent.opacity(0.35), style: underlay)
            }
            let dashedStyle = StrokeStyle(lineWidth: 1.5, lineCap: .butt, lineJoin: .round, dash: [4, 3])
            let path = polyline(route.points)
            if selectedWireIDs.contains(route.id) {
                painter.stroke(path, with: .accent, style: StrokeStyle(lineWidth: 2.5, lineCap: .round, lineJoin: .round))
                continue
            }
            switch route.style {
            case .normal:
                painter.stroke(path, with: .primary, style: solid)
            case .value(let color):
                painter.stroke(path, with: Ink(role: .color(color)), style: solid)
            case .floating:
                painter.stroke(path, with: .secondary, style: dashedStyle)
            case .shorted:
                painter.stroke(path, with: Ink(role: .color(.red)), style: solid)
            case .toggling:
                painter.stroke(path, with: .secondary, style: solid)
                painter.stroke(path, with: Ink(role: .color(.green)), style: dashedStyle)
            }
        }

        for junction in junctions {
            let r: CGFloat = junction.isSelected ? 4 : 2.5
            let point = junction.point
            let rect = CGRect(x: point.x - r, y: point.y - r, width: 2 * r, height: 2 * r)
            painter.fill(Path(ellipseIn: rect), with: junction.isSelected ? .accent : .primary)
        }

        if let preview {
            let dashed = StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round, dash: [4, 3])
            painter.stroke(polyline(preview), with: .accent, style: dashed)
        }
    }

    private static func polyline(_ points: [CGPoint]) -> Path {
        var path = Path()
        guard let first = points.first else { return path }
        path.move(to: first)
        for point in points.dropFirst() {
            path.addLine(to: point)
        }
        return path
    }
}

/// The route the Bus tool is drawing. `refused` when releasing would be
/// refused (a joint that is not a T or an L, or too many wires).
struct BusPreview {
    let points: [CGPoint]
    let refused: Bool
}

/// Draws buses: each line 6 wide with square ends, a filled square closing
/// each L corner, and a dot on each T. The hovered bus gets a soft accent
/// underlay and the selected one is drawn in the accent colour. Also draws
/// the route the Bus tool is drawing, dashed. Not private, so the harness can
/// render it on its own.
struct BusLayer: View {
    let buses: [Bus]
    let selectedBusID: UUID?
    let hoveredBusID: UUID?
    let preview: BusPreview?
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            var shifted = context
            shifted.shiftToWorld(area)
            var painter = ScreenPainter(shifted)
            Self.paint(&painter, buses: buses, selectedBusID: selectedBusID, hoveredBusID: hoveredBusID,
                       preview: preview)
        }
        .worldCanvas(area)
    }

    /// Draws the buses in world coordinates.
    static func paint<P: SchematicPainter>(_ painter: inout P, buses: [Bus], selectedBusID: UUID?,
                                           hoveredBusID: UUID?, preview: BusPreview?) {
        let width = Bus.lineWidth
        for bus in buses {
            if bus.id == hoveredBusID {
                let underlay = StrokeStyle(lineWidth: width * 2, lineCap: .round)
                for line in bus.lines {
                    painter.stroke(segment(line.a, line.b), with: Ink.accent.opacity(0.35), style: underlay)
                }
            }
            let ink: Ink = bus.id == selectedBusID ? .accent : .primary
            for line in bus.lines {
                painter.stroke(segment(line.a, line.b), with: ink, style: StrokeStyle(lineWidth: width, lineCap: .butt))
            }
            for joint in bus.joints {
                let p = joint.point
                switch joint {
                case .corner:
                    let square = CGRect(x: p.x - width / 2, y: p.y - width / 2, width: width, height: width)
                    painter.fill(Path(square), with: ink)
                case .tee:
                    let dot = CGRect(x: p.x - width, y: p.y - width, width: width * 2, height: width * 2)
                    painter.fill(Path(ellipseIn: dot), with: ink)
                }
            }
        }

        if let preview, let first = preview.points.first {
            var path = Path()
            path.move(to: first)
            for point in preview.points.dropFirst() { path.addLine(to: point) }
            let dashed = StrokeStyle(lineWidth: width, lineCap: .butt, lineJoin: .miter, dash: [8, 6])
            let ink: Ink = preview.refused ? Ink(role: .color(Color.red.opacity(0.6))) : .accent
            painter.stroke(path, with: ink, style: dashed)
        }
    }

    private static func segment(_ a: CGPoint, _ b: CGPoint) -> Path {
        var path = Path()
        path.move(to: a)
        path.addLine(to: b)
        return path
    }
}

/// Draws bus ports: each body (outlined; accent when selected or hovered),
/// a stub per pin out to its tip, and each pin's wire name inside the body
/// next to its pin, turned 90° on a horizontal line where pins are close.
struct PortLayer: View {
    let buses: [Bus]
    let selectedPortID: UUID?
    let hoveredPortID: UUID?
    /// The visible world area; drawing outside it is not shown.
    let area: CGRect

    var body: some View {
        Canvas { context, _ in
            var shifted = context
            shifted.shiftToWorld(area)
            var painter = ScreenPainter(shifted)
            Self.paint(&painter, buses: buses, selectedPortID: selectedPortID, hoveredPortID: hoveredPortID)
        }
        .worldCanvas(area)
    }

    /// Draws the ports in world coordinates.
    static func paint<P: SchematicPainter>(_ painter: inout P, buses: [Bus], selectedPortID: UUID?,
                                           hoveredPortID: UUID?) {
        for bus in buses {
            for port in bus.ports {
                guard let body = bus.body(of: port), let line = bus.line(port.line) else { continue }
                let highlighted = port.id == selectedPortID || port.id == hoveredPortID
                let outline: Ink = highlighted ? .accent : .primary
                painter.fill(Path(body), with: .paper)
                painter.stroke(Path(body), with: outline, lineWidth: highlighted ? 2 : 1.5)

                for (index, wire) in bus.pinWires(of: port).enumerated() {
                    let along = bus.pinAlong(port, index: index)
                    guard let edge = bus.point(on: port, along: along, out: BusPort.depth),
                          let tip = bus.point(on: port, along: along, out: BusPort.depth + BusPort.stub),
                          let label = bus.point(on: port, along: along, out: BusPort.depth / 2) else { continue }
                    var stub = Path()
                    stub.move(to: edge)
                    stub.addLine(to: tip)
                    painter.stroke(stub, with: outline, lineWidth: 1.5)

                    let text = PaintedText(wire.name, size: 10, weight: .medium, monospaced: true)
                    if line.isHorizontal {
                        var turned = painter
                        turned.translateBy(x: label.x, y: label.y)
                        turned.rotate(by: .degrees(-90))
                        turned.draw(text, at: .zero, anchor: .center)
                    } else {
                        painter.draw(text, at: label, anchor: .center)
                    }
                }
            }
        }
    }
}
