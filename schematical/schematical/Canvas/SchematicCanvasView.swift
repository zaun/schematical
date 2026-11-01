import SwiftUI

/// The editing surface. Its gestures depend on the active tool:
/// - Pointer: drag a block to move it. Drag a wire's bend point to move it, or
///   drag a segment to shift it. Double-click a wire to add a bend there.
///   Drag empty space to pan.
/// - Line: drag from one pin to another to connect them with a right-angled wire.
/// - Eraser: click or drag over a block or wire to remove it.
///
/// Parts dragged from the drawer are dropped here and centred on the drop point.
/// The schematic is drawn in world coordinates and scaled by the viewport's zoom.
struct SchematicCanvasView: View {
    let catalog: ChipCatalogStore
    let document: SchematicDocument
    let viewport: CanvasViewport
    let tool: SchematicTool

    @State private var interaction: Interaction?
    @State private var isDropTargeted = false
    @State private var hoveredChipID: UUID?
    @State private var hoveredWireID: UUID?

    private enum Interaction {
        case moveChip(id: UUID, startOrigin: CGPoint)
        case dragNode(wire: UUID, base: [CGPoint], index: Int)
        case dragSegment(wire: UUID, base: [CGPoint], index: Int)
        case connecting(from: PinRef, cursor: CGPoint)
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
                }
            }
            .simultaneousGesture(
                SpatialTapGesture(count: 2).onEnded { value in
                    guard tool == .pointer else { return }
                    document.insertNode(at: viewport.world(value.location), tolerance: lineTolerance)
                }
            )
            .dropDestination(for: ChipPartDragItem.self) { items, location in
                guard let item = items.first,
                      let chip = catalog.chip(forPartNumber: item.partNumber) else {
                    return false
                }
                document.placeChip(
                    partNumber: item.partNumber,
                    chip: chip,
                    centeredAt: viewport.world(location)
                )
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
                routes: document.wires.map { RoutedWire(id: $0.id, points: document.path(forWire: $0.id)) },
                hoveredWireID: hoveredWireID,
                preview: connectionPreview
            )

            ForEach(document.chips) { chip in
                ChipBlockView(
                    chip: chip,
                    isSelected: chip.id == document.selectedChipID,
                    isHovered: chip.id == hoveredChipID
                )
                .offset(x: chip.origin.x - ChipLayout.labelMargin, y: chip.origin.y)
            }

            PinMarkerLayer(
                tips: tool == .line ? document.pinTips : [],
                target: connectionPreview?.target
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
              let start = document.pinTip(from) else {
            return nil
        }

        let hoveredPin = document.pin(at: cursor, radius: pinRadius).flatMap { $0 == from ? nil : $0 }
        let target = hoveredPin.flatMap { document.pinTip($0) }
        let end = target ?? SchematicGrid.snap(cursor)
        return ConnectionPreview(points: OrthogonalRoute.route(from: start, to: end), target: target)
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
                if let pin = document.pin(at: start, radius: pinRadius) {
                    interaction = .connecting(from: pin, cursor: current)
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
        }
    }

    /// Highlights the block under the pointer, or else the wire segment under it.
    private func updateHover(at point: CGPoint) {
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
            if let target = document.pin(at: viewport.world(value.location), radius: pinRadius),
               target != from {
                document.addWire(from: from, to: target)
            }
        case let .moveChip(id, _)?:
            document.settleWires(attachedTo: id)
        default:
            break
        }
        interaction = nil
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
}

/// A small circle over each pin, shown in Line mode. The pin the wire would
/// connect to is filled green.
private struct PinMarkerLayer: View {
    let tips: [CGPoint]
    let target: CGPoint?

    var body: some View {
        Canvas { context, _ in
            for tip in tips {
                if tip == target {
                    let rect = CGRect(x: tip.x - 6, y: tip.y - 6, width: 12, height: 12)
                    context.fill(Path(ellipseIn: rect), with: .color(.green))
                    context.stroke(Path(ellipseIn: rect), with: .color(.green.opacity(0.6)), lineWidth: 1)
                } else {
                    let rect = CGRect(x: tip.x - 4, y: tip.y - 4, width: 8, height: 8)
                    context.stroke(Path(ellipseIn: rect), with: .color(.accentColor), lineWidth: 1.25)
                }
            }
        }
    }
}

/// Draws wires as orthogonal polylines, plus the route being drawn from a pin.
/// The hovered wire gets a soft accent underlay.
private struct WireLayer: View {
    let routes: [RoutedWire]
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
                context.stroke(polyline(route.points), with: .color(.primary), style: solid)
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
