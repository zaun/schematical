import SwiftUI

/// The editing surface. Its gestures depend on the active tool:
/// - Pointer: drag a block to move it. Drag a wire's bend point to move it, or
///   drag a segment to shift it. Double-click a wire to add a bend there.
///   Drag empty space to pan.
/// - Line: drag from one pin to another to connect them with a right-angled wire.
///   A grid point on an existing wire can be a start or end too; it becomes a
///   junction, so wires branch there. Only connection points are highlighted.
/// - Eraser: click or drag over a block or wire to remove it.
/// - Flip: click a part to flip it left and right.
/// - VCC and GND: click the canvas to place a supply there.
///
/// Clicking a DIP switch lever flips that position, with the Pointer tool or
/// in play mode. A press that drags moves the part (or pans) instead.
///
/// Parts dragged from the drawer are dropped here and centred on the drop point.
/// The schematic is drawn in world coordinates and scaled by the viewport's zoom.
struct SchematicCanvasView: View {
    let catalog: ChipCatalogStore
    let document: SchematicDocument
    let viewport: CanvasViewport
    let tool: SchematicTool
    /// The simulation. While it runs (play mode), the canvas only pans.
    let simulation: SimulationController
    private var isRunning: Bool { simulation.isRunning }
    /// Called with a memory part's id when it is double-clicked, to show its contents.
    var onOpenMemory: (UUID) -> Void = { _ in }
    /// Called after a part dragged from the drawer is placed on the canvas.
    var onPartPlaced: () -> Void = {}

    @State private var interaction: Interaction?
    @State private var isDropTargeted = false
    @State private var hoveredChipID: UUID?
    @State private var hoveredWireID: UUID?
    /// The connection point under the pointer in Line mode, when not drawing.
    @State private var hoverAnchor: LineAnchor?

    private enum Interaction {
        case moveChip(id: UUID, startOrigin: CGPoint)
        case dragNode(wire: UUID, base: [CGPoint], index: Int)
        case dragSegment(wire: UUID, base: [CGPoint], index: Int)
        case connecting(from: LineAnchor, cursor: CGPoint)
        case pan(startOffset: CGSize)
        case idle
    }

    private var zoom: CGFloat { viewport.zoom }

    /// Hit tolerances are in screen points, so they are divided by zoom.
    private var lineTolerance: CGFloat { 6 / zoom }
    private var pinRadius: CGFloat { 8 / zoom }

    var body: some View {
        // The world layer is an overlay so its size never feeds back into layout.
        // Its grid size depends on the viewport, which depends on this view.
        Color.clear
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .overlay(alignment: .topLeading) {
                worldLayer
            }
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
                    hoveredChipID = nil
                    hoveredWireID = nil
                    hoverAnchor = nil
                }
            }
            .onChange(of: isRunning) { _, running in
                // Drop any highlight left from before play mode started (⌘R).
                if running {
                    hoveredChipID = nil
                    hoveredWireID = nil
                    hoverAnchor = nil
                }
            }
            .simultaneousGesture(
                SpatialTapGesture(count: 2).onEnded { value in
                    guard tool == .pointer else { return }
                    let point = viewport.world(value.location)
                    // Double-clicking a memory part opens its memory view, in
                    // play mode too, which is when it has contents to show.
                    if let chip = document.chip(at: point), chip.chip.category == memoryCategory {
                        onOpenMemory(chip.id)
                        return
                    }
                    guard !isRunning else { return }
                    document.insertNode(at: point, tolerance: lineTolerance)
                }
            )
            .dropDestination(for: ChipPartDragItem.self) { items, location in
                guard !isRunning, let item = items.first,
                      let chip = catalog.chip(forPartNumber: item.partNumber) else {
                    return false
                }
                document.placeChip(
                    partNumber: item.partNumber,
                    chip: chip,
                    symbol: catalog.symbol(forPartNumber: item.partNumber),
                    centeredAt: viewport.world(location)
                )
                onPartPlaced()
                return true
            } isTargeted: { isDropTargeted = $0 }
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

            WireLayer(
                routes: document.wires.map { wire in
                    RoutedWire(
                        id: wire.id,
                        points: document.path(forWire: wire.id),
                        color: simulation.wireColor(wire.id),
                        dashed: simulation.wireIsFloating(wire.id)
                    )
                },
                junctions: document.junctions.map(\.point),
                hoveredWireID: hoveredWireID,
                preview: connectionPreview
            )

            ForEach(document.chips) { chip in
                Group {
                    switch chip.symbol {
                    case .block, .oscillator:
                        ChipBlockView(
                            chip: chip,
                            isSelected: chip.id == document.selectedChipID,
                            isHovered: chip.id == hoveredChipID
                        )
                    case .diode:
                        DiodeSymbolView(
                            chip: chip,
                            isSelected: chip.id == document.selectedChipID,
                            isHovered: chip.id == hoveredChipID,
                            isLit: simulation.litChips.contains(chip.id)
                        )
                    case .vcc, .ground:
                        SupplySymbolView(
                            chip: chip,
                            isSelected: chip.id == document.selectedChipID,
                            isHovered: chip.id == hoveredChipID
                        )
                    case .dipSwitch:
                        DipSwitchSymbolView(
                            chip: chip,
                            isSelected: chip.id == document.selectedChipID,
                            isHovered: chip.id == hoveredChipID
                        )
                    }
                }
                .offset(x: chip.viewOrigin.x, y: chip.viewOrigin.y)
            }

            PinMarkerLayer(
                tips: tool == .line ? document.pinTips : [],
                highlight: highlightPoint
            )
        }
        .scaleEffect(zoom, anchor: .topLeading)
        .offset(viewport.offset)
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

    private var connectionPreview: ConnectionPreview? {
        guard case let .connecting(from, cursor)? = interaction,
              let start = document.anchorTip(from) else {
            return nil
        }

        let anchor = lineAnchor(at: cursor).flatMap { $0 == from ? nil : $0 }
        let target = anchor.flatMap { document.anchorTip($0) }
        let end = target ?? SchematicGrid.snap(cursor)
        return ConnectionPreview(points: OrthogonalRoute.route(from: start, to: end), target: target)
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

        // Play mode: a drag anywhere pans, and nothing else can start.
        if isRunning {
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
                beginPointerInteraction(at: start)
            }
            switch interaction {
            case let .moveChip(id, startOrigin)?:
                document.moveChip(
                    id,
                    to: CGPoint(x: startOrigin.x + delta.width, y: startOrigin.y + delta.height)
                )

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

            case let .pan(startOffset)?:
                viewport.pan(to: CGSize(
                    width: startOffset.width + value.translation.width,
                    height: startOffset.height + value.translation.height
                ))

            default:
                break
            }

        case .line:
            if interaction == nil {
                if let anchor = lineAnchor(at: start) {
                    interaction = .connecting(from: anchor, cursor: current)
                } else {
                    interaction = .idle
                }
            }
            if case let .connecting(from, _)? = interaction {
                interaction = .connecting(from: from, cursor: current)
            }

        case .eraser:
            document.erase(at: current, tolerance: lineTolerance)
            interaction = .idle

        case .flip:
            // One flip per press: the part under where the press started.
            if interaction == nil, let hit = document.chip(at: start) {
                document.flipChip(hit.id)
            }
            interaction = .idle

        case .vcc, .ground:
            // One supply per press, centred under where the press started.
            if interaction == nil,
               let name = tool.supplyName,
               let chip = catalog.supplyChip(named: name) {
                document.placeChip(
                    partNumber: name,
                    chip: chip,
                    symbol: catalog.symbol(forPartNumber: name),
                    centeredAt: start
                )
            }
            interaction = .idle
        }
    }

    /// Highlights the block under the pointer, or else the wire segment under it.
    /// In Line mode, highlights only the connection point under the pointer.
    /// In play mode, nothing is highlighted.
    private func updateHover(at point: CGPoint) {
        if isRunning {
            hoveredChipID = nil
            hoveredWireID = nil
            hoverAnchor = nil
            return
        }
        if tool == .line {
            hoveredChipID = nil
            hoveredWireID = nil
            hoverAnchor = lineAnchor(at: point)
            return
        }
        hoverAnchor = nil
        if let chip = document.chip(at: point) {
            hoveredChipID = chip.id
            hoveredWireID = nil
        } else {
            hoveredChipID = nil
            hoveredWireID = document.segmentHit(at: point, tolerance: lineTolerance)?.wire
        }
    }

    private func beginPointerInteraction(at point: CGPoint) {
        if let hit = document.chip(at: point) {
            document.selectedChipID = hit.id
            interaction = .moveChip(id: hit.id, startOrigin: hit.origin)
        } else if let node = document.nodeHit(at: point, tolerance: lineTolerance) {
            document.selectedChipID = nil
            interaction = .dragNode(
                wire: node.wire,
                base: document.path(forWire: node.wire),
                index: node.index
            )
        } else if let segment = document.segmentHit(at: point, tolerance: lineTolerance) {
            document.selectedChipID = nil
            interaction = .dragSegment(
                wire: segment.wire,
                base: document.path(forWire: segment.wire),
                index: segment.index
            )
        } else {
            document.selectedChipID = nil
            interaction = .pan(startOffset: viewport.offset)
        }
    }

    private func handleEnded(_ value: DragGesture.Value) {
        switch interaction {
        case let .connecting(from, _)?:
            if let target = lineAnchor(at: viewport.world(value.location)), target != from {
                document.connect(from, target)
            }
        case let .moveChip(id, _)?:
            if isClick(value) {
                toggleSwitchLever(at: viewport.world(value.startLocation))
            }
            document.settleWires(attachedTo: id)
        case .pan?:
            // In play mode every press starts a pan; a click flips a lever.
            if isRunning, isClick(value) {
                toggleSwitchLever(at: viewport.world(value.startLocation))
            }
        default:
            break
        }
        interaction = nil
    }

    /// A press that ended about where it started, in screen points.
    private func isClick(_ value: DragGesture.Value) -> Bool {
        hypot(value.translation.width, value.translation.height) < 3
    }

    /// Flips the switch position whose lever is under `point`, in the
    /// document and, in play mode, in the running simulation.
    private func toggleSwitchLever(at point: CGPoint) {
        guard let chip = document.chip(at: point),
              let position = chip.switchPosition(at: point),
              let isOn = document.toggleSwitch(chip.id, position: position) else { return }
        simulation.setSwitch(chip.id, position: position, on: isOn)
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
    /// Net color in play mode. Normal color otherwise.
    var color: Color = .primary
    /// Floating net (no driver) in play mode.
    var dashed = false
}

/// A small circle over each pin, shown in Line mode. The connection point under
/// the pointer, or the one a line would land on, is filled green.
private struct PinMarkerLayer: View {
    let tips: [CGPoint]
    let highlight: CGPoint?

    var body: some View {
        Canvas { context, _ in
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
    }
}

/// Draws wires as orthogonal polylines, plus the route being drawn from a pin.
/// The hovered wire gets a soft accent underlay. Junctions get a solid dot.
private struct WireLayer: View {
    let routes: [RoutedWire]
    let junctions: [CGPoint]
    let hoveredWireID: UUID?
    let preview: ConnectionPreview?

    var body: some View {
        Canvas { context, _ in
            let solid = StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round)
            let underlay = StrokeStyle(lineWidth: 6, lineCap: .round, lineJoin: .round)
            for route in routes {
                if route.id == hoveredWireID {
                    context.stroke(polyline(route.points), with: .color(Color.accentColor.opacity(0.35)), style: underlay)
                }
                let dashedStyle = StrokeStyle(lineWidth: 1.5, lineCap: .butt, lineJoin: .round, dash: [4, 3])
                context.stroke(
                    polyline(route.points),
                    with: .color(route.color),
                    style: route.dashed ? dashedStyle : solid
                )
            }

            for point in junctions {
                let rect = CGRect(x: point.x - 2.5, y: point.y - 2.5, width: 5, height: 5)
                context.fill(Path(ellipseIn: rect), with: .color(.primary))
            }

            if let preview {
                let dashed = StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round, dash: [4, 3])
                context.stroke(polyline(preview.points), with: .color(.accentColor), style: dashed)
            }
        }
    }

    private func polyline(_ points: [CGPoint]) -> Path {
        var path = Path()
        guard let first = points.first else { return path }
        path.move(to: first)
        for point in points.dropFirst() {
            path.addLine(to: point)
        }
        return path
    }
}
