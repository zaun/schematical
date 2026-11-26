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

    /// The gestures, hover and drawing keys (plan-68-AJ); a test hands in
    /// its own to set what the canvas shows.
    @State private var controller: SchematicCanvasController
    @Environment(\.editorEventMonitors) private var eventMonitors

    init(catalog: PartCatalogStore, document: SchematicDocument, viewport: CanvasViewport, tool: SchematicTool,
         supplyVoltage: String = SupplyChoice.defaultName, simulation: SimulationController,
         onOpenMemory: @escaping (UUID) -> Void, onOpenCpu: @escaping (UUID) -> Void,
         onOpenTerminal: @escaping (UUID) -> Void, onOpenRegisters: @escaping (UUID) -> Void,
         onOpenDocumentInfo: @escaping () -> Void, pointer: BoardPointer? = nil,
         onPartPlaced: @escaping () -> Void, controller: SchematicCanvasController? = nil) {
        self.catalog = catalog
        self.document = document
        self.viewport = viewport
        self.tool = tool
        self.supplyVoltage = supplyVoltage
        self.simulation = simulation
        self.onOpenMemory = onOpenMemory
        self.onOpenCpu = onOpenCpu
        self.onOpenTerminal = onOpenTerminal
        self.onOpenRegisters = onOpenRegisters
        self.onOpenDocumentInfo = onOpenDocumentInfo
        self.pointer = pointer
        self.onPartPlaced = onPartPlaced
        _controller = State(initialValue: controller ?? SchematicCanvasController())
    }

    /// What the controller works on, as this view has it now.
    private var context: SchematicCanvasContext {
        SchematicCanvasContext(catalog: catalog, document: document, viewport: viewport, tool: tool,
                               supplyVoltage: supplyVoltage, simulation: simulation, onOpenMemory: onOpenMemory,
                               onOpenCpu: onOpenCpu, onOpenTerminal: onOpenTerminal, onOpenRegisters: onOpenRegisters,
                               onOpenDocumentInfo: onOpenDocumentInfo, pointer: pointer, onPartPlaced: onPartPlaced)
    }

    var body: some View {
        let context = context
        // The world layer is an overlay so its size never feeds back into layout.
        // Its grid size depends on the viewport, which depends on this view.
        Color.clear
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .overlay(alignment: .topLeading) {
                worldLayer(context)
            }
            // Render the scaled world at screen resolution. Without this each
            // symbol is drawn at full size and then shrunk, and below about
            // half size thin lines fall between pixels and vanish.
            .drawingGroup()
            .background(Color(nsColor: .textBackgroundColor))
            .contentShape(Rectangle())
            .clipped()
            .onGeometryChange(for: CGSize.self) { $0.size } action: { viewport.updateSize($0) }
            // A line selected in the Electrical Rules Check: bring its mark into view.
            .onChange(of: document.ercFocus) { _, focus in if let focus { viewport.reveal(focus.point) } }
            .gesture(canvasGesture(context))
            .simultaneousGesture(pinchGesture(context))
            .scrollPans(viewport) { controller.scrolledMidPress(by: $0, in: context) }
            .onContinuousHover { controller.hover(Self.location(of: $0), in: context) }
            .onAppear { controller.installDrawingKeyMonitor(eventMonitors, context: context) }
            .onDisappear { controller.removeDrawingKeyMonitor(eventMonitors) }
            .onChange(of: simulation.isRunning) { _, running in controller.runningChanged(running, in: context) }
            .simultaneousGesture(SpatialTapGesture(count: 2).onEnded { controller.doubleClick(at: $0.location, in: context) })
            .dropDestination(for: ChipPartDragItem.self) { controller.drop(partNumber: $0.first?.partNumber, at: $1, in: context) }
            isTargeted: { controller.isDropTargeted = $0 }
            .overlay(alignment: .topLeading) {
                if let box = controller.selectionBox {
                    SelectionBox(from: box.from, to: box.to)
                }
            }
            .overlay {
                if controller.isDropTargeted {
                    Rectangle()
                        .strokeBorder(
                            Color.accentColor.opacity(0.6),
                            style: StrokeStyle(lineWidth: 2, dash: [8, 6])
                        )
                        .allowsHitTesting(false)
                }
            }
    }

    /// Where a hover is, nil when it ended.
    static func location(of phase: HoverPhase) -> CGPoint? {
        if case let .active(location) = phase { return location }
        return nil
    }

    /// Everything in schematic coordinates, scaled and shifted by the viewport.
    private func worldLayer(_ context: SchematicCanvasContext) -> some View {
        let gridRect = context.gridRect
        return ZStack(alignment: .topLeading) {
            GridBackground(rect: gridRect, step: context.gridStep)
                .equatable()

            // The title block is under the text blocks, so a note can sit on it.
            TitleBlockLayer(layout: document.titleBlockLayout,
                            isSelected: document.schematicSelection.contains(.titleBlock),
                            isHovered: controller.isTitleBlockHovered, area: gridRect)

            // Under everything else: wires and parts stay visible over a block.
            NoteLayer(
                notes: document.notes,
                selectedNoteIDs: context.selected { if case let .note(id) = $0 { id } else { nil } },
                hoveredNoteID: controller.hoveredNoteID,
                area: gridRect
            )

            // Under the wires, so wires crossing a bus stay visible.
            BusLayer(
                buses: document.buses,
                selectedBusIDs: context.selected { if case let .bus(id) = $0 { id } else { nil } },
                hoveredBusID: controller.hoveredBusID,
                preview: controller.busPreview(in: context),
                area: gridRect
            )

            PortLayer(
                buses: document.buses,
                selectedPortID: document.selectedPortID,
                hoveredPortID: controller.hoveredPortID,
                area: gridRect
            )

            // Their own views, which read the wires, junctions and parts:
            // an edit to a label, bus or note leaves their drawing alone.
            SchematicWiresLayer(
                document: document,
                simulation: simulation,
                selectedWireIDs: context.selected { if case let .wire(id) = $0 { id } else { nil } },
                hoveredWireID: controller.hoveredWireID,
                preview: controller.connectionPreview(in: context),
                area: gridRect
            )

            NetLabelLayer(
                labels: document.netLabels,
                selectedIDs: context.selected { if case let .label(id) = $0 { id } else { nil } },
                hoveredID: controller.hoveredLabelID,
                area: gridRect
            )

            SchematicPartsLayer(document: document, simulation: simulation, hoveredChipID: controller.hoveredChipID,
                                hoveredUnitKey: controller.hoveredUnitKey)

            PinMarkerLayer(
                tips: tool == .line ? document.pinTips : [],
                highlight: controller.highlightPoint(in: context),
                area: gridRect
            )

            // The Electrical Rules Check's marks: on screen only, never in an export.
            if !document.ercMarkers.isEmpty {
                ErcMarkerLayer(markers: document.ercMarkers, selected: document.ercSelectedKeys, area: gridRect)
            }
        }
        .scaleEffect(context.zoom, anchor: .topLeading)
        .offset(viewport.offset)
    }

    /// Trackpad pinch: zooms about the point the fingers started on.
    private func pinchGesture(_ context: SchematicCanvasContext) -> some Gesture {
        MagnifyGesture()
            .onChanged { controller.pinchChanged(magnification: $0.magnification, startLocation: $0.startLocation, in: context) }
            .onEnded { _ in controller.pinchEnded() }
    }

    private func canvasGesture(_ context: SchematicCanvasContext) -> some Gesture {
        DragGesture(minimumDistance: 0)
            // Shift and the key window are read as the system reports the press.
            .onChanged { controller.pressChanged(DragPress(startLocation: $0.startLocation, location: $0.location, translation: $0.translation), shift: NSEvent.modifierFlags.contains(.shift), keyWindow: NSApp.keyWindow, in: context) }
            .onEnded { controller.pressEnded(DragPress(startLocation: $0.startLocation, location: $0.location, translation: $0.translation), in: context) }
    }
}
