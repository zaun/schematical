//
//  ContentView.swift
//  schematical
//
//  Created by Justin Zaun on 10/31/26.
//

import SwiftUI

/// The window's two pages: the schematic editor and the PCB.
enum EditorPage: String, CaseIterable, Identifiable {
    case schematic
    case pcb

    var id: String { rawValue }

    var title: String {
        switch self {
        case .schematic: "Schematic"
        case .pcb: "PCB"
        }
    }
}

struct ContentView: View {
    @State private var catalog: PartCatalogStore
    @State private var document: SchematicDocument
    @State private var viewport = CanvasViewport()
    /// The PCB page's own pan and zoom. It opens at 25 %, so a whole board
    /// fits.
    @State private var boardViewport = CanvasViewport(zoom: 0.25)
    @State private var page = EditorPage.schematic
    @State private var tool: SchematicTool = .pointer
    @State private var pcbTool = PCBTool.select
    /// Layers the PCB page doesn't draw (the View popover). The hidden set,
    /// so layers added later start shown. Per window; not saved.
    @State private var hiddenLayers: Set<BoardViewLayer> = []
    @State private var isLayerViewShown = false
    /// The layer the PCB Trace tool draws on (the trace layer menu).
    @State private var traceLayer: CopperLayer = .top
    @State private var isDrawerShown = true
    /// The PCB page's inspector (board, footprint or text settings).
    @State private var isPCBSidebarShown = true
    @State private var simulation: SimulationController
    @State private var file: SchematicFileController
    /// Identifies this window to the memory windows it opens.
    @State private var sessionID = UUID()
    @Environment(\.openWindow) private var openWindow
    @Environment(\.dismissWindow) private var dismissWindow
    /// Memory windows opened from this window, closed along with it.
    @State private var memoryWindows: Set<MemoryWindowTarget> = []
    @State private var cpuWindows: Set<CpuWindowTarget> = []
    @State private var terminalWindows: Set<TerminalWindowTarget> = []
    @State private var isBillOfMaterialsOpen = false
    /// Simulation speed (circuit time per wall time), an app preference.
    @AppStorage("simulationSpeed") private var simulationSpeed = 1.0
    /// The supply the VCC tool places (a `SupplyChoice` name), an app preference.
    @AppStorage("supplyVoltage") private var supplyVoltage = SupplyChoice.defaultName

    /// The speed menu's choices: real time down to 1/1,000,000.
    private static let speeds: [(label: String, value: Double)] = [
        ("1×", 1), ("1/10", 1e-1), ("1/100", 1e-2), ("1/1,000", 1e-3),
        ("1/10,000", 1e-4), ("1/100,000", 1e-5), ("1/1,000,000", 1e-6),
    ]

    private var speedLabel: String {
        Self.speeds.first { abs($0.value - simulationSpeed) < $0.value * 1e-6 }?.label ?? "1×"
    }

    init() {
        let catalog = PartCatalogStore()
        let document = SchematicDocument(library: catalog.library)
        let simulation = SimulationController(library: catalog.library)
        _catalog = State(initialValue: catalog)
        _document = State(initialValue: document)
        _simulation = State(initialValue: simulation)
        // Opening a file stops any running simulation first, since the
        // simulation was built from the old schematic.
        _file = State(initialValue: SchematicFileController(
            document: document,
            catalog: catalog,
            willReplaceDocument: { simulation.stop() }
        ))
    }

    /// The zoom controls act on the page being shown.
    private var activeViewport: CanvasViewport {
        page == .schematic ? viewport : boardViewport
    }

    /// One inspector column: the parts drawer on the Schematic page, the PCB
    /// sidebar on the PCB page. Each page remembers whether it is shown.
    private var inspectorBinding: Binding<Bool> {
        Binding(
            get: { page == .schematic ? isDrawerShown : isPCBSidebarShown },
            set: { shown in
                if page == .schematic {
                    isDrawerShown = shown
                } else {
                    isPCBSidebarShown = shown
                }
            }
        )
    }

    /// The layers the Trace tool can draw on: the board's, less its planes.
    private var traceLayers: [CopperLayer] { document.board.traceLayers }

    /// Exactly one footprint or board text selected, which Rotate turns
    /// (plan-15: never a group). A trace doesn't turn. Rotate is only on
    /// the PCB page, where the simulation never runs.
    private var canRotate: Bool {
        if document.selectedTextID != nil { return true }
        guard let id = document.selectedFootprintID else { return false }
        return document.chips.first { $0.id == id }?.board != nil
    }

    @ViewBuilder
    private var pageContent: some View {
        switch page {
        case .schematic:
            SchematicCanvasView(
                catalog: catalog,
                document: document,
                viewport: viewport,
                tool: tool,
                supplyVoltage: supplyVoltage,
                simulation: simulation,
                onOpenMemory: { chipID in
                    let target = MemoryWindowTarget(sessionID: sessionID, chipID: chipID)
                    memoryWindows.insert(target)
                    openWindow(id: "memory", value: target)
                },
                onOpenCpu: { chipID in
                    let target = CpuWindowTarget(sessionID: sessionID, chipID: chipID)
                    cpuWindows.insert(target)
                    openWindow(id: "cpu", value: target)
                },
                onOpenTerminal: { chipID in
                    let target = TerminalWindowTarget(sessionID: sessionID, chipID: chipID)
                    terminalWindows.insert(target)
                    openWindow(id: "terminal", value: target)
                }
            ) {
                // Placing a part from the drawer returns to the Pointer tool.
                tool = .pointer
            }
        case .pcb:
            PCBCanvasView(document: document, viewport: boardViewport, tool: pcbTool,
                          visible: BoardVisibility(hidden: hiddenLayers), traceLayer: traceLayer,
                          onShowLayer: { hiddenLayers.remove(.copper($0)) }) {
                // Placing a text returns to the Select tool.
                pcbTool = .select
            }
        }
    }

    var body: some View {
        pageContent
            .inspector(isPresented: inspectorBinding) {
                ZStack {
                    // Kept in the tree on the PCB page too, so the parts
                    // library keeps its search, filters and open groups.
                    SchematicSidebarView(catalog: catalog, document: document, simulation: simulation,
                                         isActive: page == .schematic)
                    if page == .pcb {
                        PCBSidebarView(document: document)
                    }
                }
                // macOS opens an inspector at its minimum width and ignores
                // `ideal`, so the minimum is the width it should open at.
                .inspectorColumnWidth(min: 350, ideal: 350, max: 500)
            }
            .toolbar {
                ToolbarItemGroup(placement: .navigation) {
                    fileControls
                }

                // The simulation belongs to the Schematic page: the PCB page has
                // no Run/Stop (so no ⌘R), speed menu or reached speed.
                if page == .schematic {
                    ToolbarItem(placement: .navigation) {
                        Button {
                            if !simulation.isRunning {
                                // Play mode starts with the Pointer (and PCB
                                // Select) tool and nothing selected.
                                tool = .pointer
                                pcbTool = .select
                                document.clearAllSelections()
                            }
                            simulation.toggle(document: document)
                        } label: {
                            Label(
                                simulation.isRunning ? "Stop" : "Run",
                                systemImage: simulation.isRunning ? "stop.fill" : "play.fill"
                            )
                        }
                        .keyboardShortcut("r", modifiers: .command)
                        .help(simulation.isRunning ? "Stop the simulation (⌘R)" : "Run the simulation (⌘R)")
                    }

                    ToolbarItem(placement: .navigation) {
                        Menu(speedLabel) {
                            ForEach(Self.speeds, id: \.value) { choice in
                                Button(choice.label) {
                                    simulationSpeed = choice.value
                                    simulation.setSpeed(choice.value)
                                }
                            }
                        }
                        .help("Simulation speed: circuit time per real time")
                    }

                    ToolbarItem(placement: .navigation) {
                        ReachedSpeedLabel(simulation: simulation)
                    }
                }

                ToolbarItemGroup(placement: .principal) {
                    Picker("Page", selection: $page) {
                        ForEach(EditorPage.allCases) { page in
                            Text(page.title).tag(page)
                        }
                    }
                    .pickerStyle(.segmented)
                    .labelsHidden()
                    .help("Show the schematic or the PCB")

                    if page == .schematic {
                        Picker("Tool", selection: $tool) {
                            ForEach(SchematicTool.allCases) { tool in
                                Label(tool.title, systemImage: tool.symbol)
                                    .help(tool.title)
                                    .tag(tool)
                            }
                        }
                        .pickerStyle(.segmented)
                        .labelStyle(.iconOnly)
                        .labelsHidden()
                        .disabled(simulation.isRunning)

                        // The VCC and Pull-up tools' rail. Picking one also picks
                        // VCC, unless Pull-up is the tool.
                        Menu(supplyVoltage) {
                            ForEach(SupplyChoice.names, id: \.self) { name in
                                Button(name) {
                                    supplyVoltage = name
                                    tool = tool == .pullUp ? .pullUp : .vcc
                                }
                            }
                        }
                        .help("Supply voltage for the VCC and Pull-up tools")
                        .disabled(simulation.isRunning)
                    } else {
                        Picker("Tool", selection: $pcbTool) {
                            ForEach(PCBTool.allCases) { tool in
                                Label(tool.title, systemImage: tool.symbol)
                                    .help(tool.title)
                                    .tag(tool)
                            }
                        }
                        .pickerStyle(.segmented)
                        .labelStyle(.iconOnly)
                        .labelsHidden()

                        // The Trace tool's layer: every board layer that
                        // isn't a plane. Picking one also picks Trace and
                        // shows that layer.
                        Menu(traceLayer.title) {
                            ForEach(traceLayers, id: \.self) { layer in
                                Button(layer.title) {
                                    traceLayer = layer
                                    pcbTool = .trace
                                    hiddenLayers.remove(.copper(layer))
                                }
                            }
                        }
                        .help("Layer for the Trace tool")
                    }
                }

                if page == .pcb {
                    ToolbarItem(placement: .primaryAction) {
                        Button {
                            if let id = document.selectedTextID {
                                document.rotateText(id)
                            } else if let id = document.selectedFootprintID {
                                document.rotateFootprint(id)
                            }
                        } label: {
                            Label("Rotate", systemImage: "rotate.right")
                        }
                        .keyboardShortcut("r", modifiers: [.command, .option])
                        .help("Rotate the selection 90° (⌥⌘R)")
                        .disabled(!canRotate)
                    }
                }

                // Which layers the PCB page draws, just left of the zoom.
                if page == .pcb {
                    ToolbarItem(placement: .primaryAction) {
                        Button {
                            isLayerViewShown.toggle()
                        } label: {
                            Label("View", systemImage: "square.3.layers.3d")
                        }
                        .help("Choose which layers to show")
                        .popover(isPresented: $isLayerViewShown, arrowEdge: .bottom) {
                            LayerViewPopover(board: document.board, hidden: $hiddenLayers)
                        }
                    }
                }

                ToolbarItemGroup(placement: .primaryAction) {
                    zoomControls
                }

                ToolbarItem(placement: .primaryAction) {
                    if page == .schematic {
                        Toggle(isOn: $isDrawerShown) {
                            Label("Parts Drawer", systemImage: "sidebar.trailing")
                        }
                        .toggleStyle(.button)
                        .help("Show or hide the parts drawer")
                    } else {
                        Toggle(isOn: $isPCBSidebarShown) {
                            Label("Inspector", systemImage: "sidebar.trailing")
                        }
                        .toggleStyle(.button)
                        .help("Show or hide the inspector")
                    }
                }
            }
            .overlay(alignment: .bottom) {
                if page == .schematic, let message = simulation.errorMessage {
                    Label("Simulation stopped: \(message)", systemImage: "exclamationmark.triangle.fill")
                        .font(.callout)
                        .padding(.horizontal, 14)
                        .padding(.vertical, 8)
                        .background(.red.opacity(0.85), in: Capsule())
                        .foregroundStyle(.white)
                        .padding(.bottom, 16)
                } else if page == .schematic, let message = document.editorMessage {
                    // A refused wire or switch: says why, then clears.
                    Label(message, systemImage: "bolt.trianglebadge.exclamationmark")
                        .font(.callout)
                        .padding(.horizontal, 14)
                        .padding(.vertical, 8)
                        .background(.orange.opacity(0.9), in: Capsule())
                        .foregroundStyle(.white)
                        .padding(.bottom, 16)
                }
            }
            // A trace layer that left the board or became a plane goes back
            // to Top.
            .onChange(of: traceLayers) { _, layers in
                if !layers.contains(traceLayer) { traceLayer = .top }
            }
            .task(id: document.editorMessage) {
                guard document.editorMessage != nil else { return }
                try? await Task.sleep(for: .seconds(4))
                if !Task.isCancelled {
                    document.editorMessage = nil
                }
            }
            // Erasing a port with wires, a bus line with ports, or unticking
            // a wired pin asks first.
            .alert(
                document.pendingRemoval?.message ?? "",
                isPresented: Binding(
                    get: { document.pendingRemoval != nil },
                    set: { if !$0 { document.cancelPendingRemoval() } }
                )
            ) {
                Button("Delete", role: .destructive) { document.confirmPendingRemoval() }
                Button("Cancel", role: .cancel) { document.cancelPendingRemoval() }
            }
            .onChange(of: page) { _, newPage in
                // The simulation never runs on the PCB page. Coming back to
                // the Schematic does not restart it.
                if newPage == .pcb {
                    simulation.stop()
                }
            }
            .task { catalog.load() }
            .onAppear {
                SchematicSessions.shared.register(sessionID, document: document, simulation: simulation)
                simulation.setSpeed(simulationSpeed)
            }
            .onDisappear {
                for target in memoryWindows {
                    dismissWindow(id: "memory", value: target)
                }
                memoryWindows = []
                for target in cpuWindows {
                    dismissWindow(id: "cpu", value: target)
                }
                cpuWindows = []
                for target in terminalWindows {
                    dismissWindow(id: "terminal", value: target)
                }
                terminalWindows = []
                if isBillOfMaterialsOpen {
                    dismissWindow(id: "bom", value: BillOfMaterialsTarget(sessionID: sessionID))
                    isBillOfMaterialsOpen = false
                }
            }
            .focusedSceneValue(\.schematicFile, file)
            .focusedSceneValue(\.openBillOfMaterials) {
                isBillOfMaterialsOpen = true
                openWindow(id: "bom", value: BillOfMaterialsTarget(sessionID: sessionID))
            }
            .background(WindowReader { file.window = $0 })
            .navigationTitle(file.displayName)
    }

    /// Open and Save. Their keyboard shortcuts live on the File menu items.
    @ViewBuilder
    private var fileControls: some View {
        Button {
            Task { await file.open() }
        } label: {
            Label("Open", systemImage: "folder")
        }
        .help("Open a schematic (⌘O)")

        Button {
            Task { await file.save() }
        } label: {
            Label("Save", systemImage: "square.and.arrow.down")
        }
        .help("Save the schematic (⌘S)")
    }

    @ViewBuilder
    private var zoomControls: some View {
        Button {
            activeViewport.zoomOut()
        } label: {
            Label("Zoom Out", systemImage: "minus.magnifyingglass")
        }
        .keyboardShortcut("-", modifiers: .command)
        .help("Zoom out (⌘-)")

        Menu {
            ForEach(CanvasViewport.zoomLevels, id: \.self) { level in
                Button("\(Int((level * 100).rounded()))%") {
                    activeViewport.setZoom(level)
                }
            }
            Divider()
            Button("Actual Size") {
                activeViewport.setZoom(1)
            }
            .keyboardShortcut("0", modifiers: .command)
        } label: {
            Text("\(Int((activeViewport.zoom * 100).rounded()))%")
                .monospacedDigit()
        }
        .help("Zoom level")

        Button {
            activeViewport.zoomIn()
        } label: {
            Label("Zoom In", systemImage: "plus.magnifyingglass")
        }
        .keyboardShortcut("=", modifiers: .command)
        .help("Zoom in (⌘=)")
    }
}

#Preview {
    ContentView()
}

/// "running at 0.078×" while the simulation runs below 90 % of the chosen speed.
struct ReachedSpeedLabel: View {
    let simulation: SimulationController

    /// The text shown, or nil when the simulation keeps up (or isn't running).
    static func text(for simulation: SimulationController) -> String? {
        guard simulation.isRunning, let reached = simulation.reachedSpeed,
              reached < 0.9 * simulation.speed else { return nil }
        return "running at \(formatSpeed(reached))×"
    }

    /// Two significant digits, e.g. "0.078".
    static func formatSpeed(_ value: Double) -> String {
        let formatter = NumberFormatter()
        formatter.usesSignificantDigits = true
        formatter.maximumSignificantDigits = 2
        formatter.minimumSignificantDigits = 1
        formatter.numberStyle = .decimal
        return formatter.string(from: NSNumber(value: value)) ?? String(value)
    }

    var body: some View {
        if let text = Self.text(for: simulation) {
            Text(text)
                .foregroundStyle(.secondary)
                .help("The circuit is too busy to run at the chosen speed.")
        }
    }
}

/// The PCB page's View popover: a checkbox per copper layer (with its
/// colour), then Silkscreen and Document, and Show All. It stays open while
/// the user ticks several boxes; a menu would close after each one.
private struct LayerViewPopover: View {
    let board: BoardSettings
    @Binding var hidden: Set<BoardViewLayer>

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            ForEach(board.copperLayers, id: \.self) { layer in
                toggle(.copper(layer)) {
                    HStack(spacing: 6) {
                        RoundedRectangle(cornerRadius: 2)
                            .fill(board.style(of: layer).color.color)
                            .frame(width: 12, height: 12)
                        Text(layer.title)
                    }
                }
            }
            Divider()
            toggle(.silkscreen) { Text("Silkscreen") }
            toggle(.document) { Text("Document") }
            Divider()
            Button("Show All") { hidden.removeAll() }
                .disabled(hidden.isEmpty)
        }
        .toggleStyle(.checkbox)
        .padding(12)
        .frame(minWidth: 160, alignment: .leading)
    }

    private func toggle(_ layer: BoardViewLayer, @ViewBuilder label: () -> some View) -> some View {
        Toggle(isOn: Binding(
            get: { !hidden.contains(layer) },
            set: { shown in
                if shown { hidden.remove(layer) } else { hidden.insert(layer) }
            }
        ), label: label)
        .accessibilityLabel(layer.title)
    }
}
