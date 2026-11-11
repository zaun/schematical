//
//  ContentView.swift
//  schematical
//
//  Created by Justin Zaun on 10/31/26.
//

import AppKit
import SwiftUI

/// The window's two pages: the schematic editor and the PCB.
nonisolated enum EditorPage: String, CaseIterable, Identifiable, Codable, Sendable {
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
    /// The app-wide catalog, loaded once at launch.
    private let catalog = PartCatalogStore.shared
    @State private var document: SchematicDocument
    /// The schematic's pan and zoom. Its 100 % draws at 0.75 points per
    /// schematic unit (what used to be 75 %).
    @State private var viewport = CanvasViewport(baseScale: 0.75)
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
    /// Whether the sidebar is shown: the parts drawer on the Schematic page,
    /// the inspector on the PCB page. One flag for both pages.
    @State private var isSidebarShown = true
    @State private var simulation: SimulationController
    @State private var file: SchematicFileController
    /// Identifies this window to the memory windows it opens.
    @State private var sessionID = UUID()
    @Environment(\.openWindow) private var openWindow
    @Environment(\.dismissWindow) private var dismissWindow
    /// Memory windows opened from this window, closed along with it.
    @State private var memoryWindows: Set<MemoryWindowTarget> = []
    @State private var cpuWindows: Set<CpuWindowTarget> = []
    @State private var registerWindows: Set<RegisterWindowTarget> = []
    @State private var terminalWindows: Set<TerminalWindowTarget> = []
    @State private var isBillOfMaterialsOpen = false
    @State private var isDesignRuleCheckOpen = false
    /// Watches for the editing keys: R rotates, A changes the trace angle
    /// and ⌘C / ⌘V copy and paste traces on the PCB, F flips on the
    /// Schematic, ⌘C / ⌘V copy and paste its selection, ⌘⌫ deletes on both.
    @State private var editKeyMonitor: Any?
    /// The Trace tool's angle, kept until changed; not saved in the file.
    @State private var traceAngle = TraceRoute.defaultAngle
    /// The angle shown in the middle of the PCB after A, and how opaque it
    /// is; `angleFlashID` lets only the latest press fade it.
    @State private var angleFlash: Int?
    @State private var angleFlashOpacity = 0.0
    @State private var angleFlashID = UUID()
    /// Where the pointer is over the PCB, for ⌘V.
    @State private var boardPointer = BoardPointer()
    /// Where the pointer is over the Schematic, for ⌘V.
    @State private var schematicPointer = BoardPointer()
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
        let catalog = PartCatalogStore.shared
        let document = SchematicDocument(library: catalog.library)
        let simulation = SimulationController(library: catalog.library)
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
    /// sidebar on the PCB page. Both pages share one shown flag.
    private var inspectorBinding: Binding<Bool> { $isSidebarShown }

    /// The layers the Trace tool can draw on: the board's, less its planes.
    private var traceLayers: [CopperLayer] { document.board.traceLayers }

    /// Exactly one footprint or board text selected, which Rotate turns
    /// (plan-15: never a group). A trace doesn't turn. Rotate is only on
    /// the PCB page, where the simulation never runs. Not when it is locked.
    private var canRotate: Bool {
        if let id = document.selectedTextID { return !document.isLocked(.text(id)) }
        guard let id = document.selectedFootprintID,
              let board = document.chips.first(where: { $0.id == id })?.board else { return false }
        return !board.locked
    }

    /// The Rotate button: turns the selected board text, else the selected
    /// footprint.
    private func rotateSelection() {
        if let id = document.selectedTextID {
            document.rotateText(id)
        } else if let id = document.selectedFootprintID {
            document.rotateFootprint(id)
        }
    }

    /// The part Flip would turn: the sole selected part on the Schematic,
    /// when it can flip and the simulation isn't running.
    private var flippableChipID: UUID? {
        guard page == .schematic, !simulation.isRunning, let id = document.selectedChipID,
              document.chips.first(where: { $0.id == id })?.symbol.canFlip == true else { return nil }
        return id
    }

    /// The editing keys, in this window only and never while a text field
    /// is being typed in: R presses Rotate on the PCB (when it is enabled);
    /// A steps the Trace tool's angle while Trace is the PCB tool; F flips
    /// the selected part or net label on the Schematic; ⌘⌫
    /// deletes the selection (on the PCB only its texts and traces); ⌘C
    /// copies the selected traces on the PCB and ⌘V pastes them centred on
    /// the pointer (`TraceClipboard`); on the Schematic they copy the
    /// selection and paste it the same way (`SchematicClipboard`; not while
    /// the simulation runs). Every other key, and a key with nothing to act
    /// on, is left alone.
    private func installEditKeyMonitor() {
        guard editKeyMonitor == nil else { return }
        editKeyMonitor = NSEvent.addLocalMonitorForEvents(matching: .keyDown) { event in
            guard let window = file.window, event.window === window, !(window.firstResponder is NSText),
                  let key = Self.editKey(keyCode: event.keyCode, event.charactersIgnoringModifiers,
                                         event.modifierFlags, isRepeat: event.isARepeat) else { return event }
            switch key {
            case .rotate:
                guard page == .pcb, canRotate else { return event }
                rotateSelection()
            case .flip:
                if page == .schematic, !simulation.isRunning, let id = document.selectedNetLabelID {
                    document.flipNetLabel(id)
                    return nil
                }
                guard let id = flippableChipID else { return event }
                document.flipChip(id)
            case .angle:
                guard page == .pcb, pcbTool == .trace else { return event }
                traceAngle = TraceRoute.nextAngle(after: traceAngle)
                flashAngle(traceAngle)
            case .delete:
                let deleted = page == .pcb
                    ? document.deleteBoardSelection()
                    : !simulation.isRunning && document.deleteSchematicSelection()
                guard deleted else { return event }
            case .copy:
                if page == .schematic {
                    guard let copied = document.schematicCopy() else { return event }
                    SchematicClipboard.copy(copied)
                    return nil
                }
                let copied = document.selectedTraces()
                guard !copied.isEmpty else { return event }
                TraceClipboard.copy(copied)
            case .paste:
                if page == .schematic {
                    guard !simulation.isRunning, let copied = SchematicClipboard.contents else { return event }
                    let offset = schematicPointer.point.map { SchematicClipboard.offset(centring: copied, on: $0) }
                        ?? SchematicClipboard.nextOffset()
                    document.pasteSchematic(copied, offset: offset)
                    return nil
                }
                guard !TraceClipboard.traces.isEmpty else { return event }
                let copied = TraceClipboard.traces
                let offset = boardPointer.point.map { TraceClipboard.offset(centring: copied, on: $0, grid: document.board.traceGrid) }
                    ?? TraceClipboard.nextOffset()
                document.pasteTraces(copied, offset: offset)
            }
            return nil
        }
    }

    private func removeEditKeyMonitor() {
        if let editKeyMonitor { NSEvent.removeMonitor(editKeyMonitor) }
        editKeyMonitor = nil
    }

    /// Shows `angle` in the middle of the PCB for 500 ms, then fades it out
    /// over 250 ms. A newer press restarts it.
    private func flashAngle(_ angle: Int) {
        let id = UUID()
        angleFlashID = id
        angleFlash = angle
        angleFlashOpacity = 1
        Task { @MainActor in
            try? await Task.sleep(for: .milliseconds(500))
            guard angleFlashID == id else { return }
            withAnimation(.easeOut(duration: 0.25)) { angleFlashOpacity = 0 }
        }
    }

    enum EditKey { case rotate, flip, angle, delete, copy, paste }

    /// The editing key a press is: a bare R, F or A (no ⌘, ⌃, ⌥ or ⇧), or
    /// ⌘⌫, ⌘C or ⌘V (⌘ alone). Never a held-key repeat. Nil for anything
    /// else.
    nonisolated static func editKey(keyCode: UInt16, _ characters: String?, _ modifiers: NSEvent.ModifierFlags,
                                    isRepeat: Bool) -> EditKey? {
        guard !isRepeat else { return nil }
        let held = modifiers.intersection([.command, .control, .option, .shift])
        // 51 is the Delete (backspace) key.
        if keyCode == 51 { return held == .command ? .delete : nil }
        if held == .command {
            switch characters {
            case "c": return .copy
            case "v": return .paste
            default: return nil
            }
        }
        guard held.isEmpty else { return nil }
        switch characters {
        case "r": return .rotate
        case "f": return .flip
        case "a": return .angle
        default: return nil
        }
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
                },
                onOpenRegisters: { chipID in
                    let target = RegisterWindowTarget(sessionID: sessionID, chipID: chipID)
                    registerWindows.insert(target)
                    openWindow(id: "registers", value: target)
                },
                pointer: schematicPointer
            ) {
                // Placing a part from the drawer returns to the Pointer tool.
                tool = .pointer
            }
        case .pcb:
            PCBCanvasView(document: document, viewport: boardViewport, tool: pcbTool,
                          visible: BoardVisibility(hidden: hiddenLayers), traceLayer: traceLayer,
                          traceAngle: traceAngle, onShowLayer: { hiddenLayers.remove(.copper($0)) },
                          pointer: boardPointer) {
                // Placing a text returns to the Select tool.
                pcbTool = .select
            }
            .overlay {
                if let angleFlash {
                    Text("\(angleFlash)°")
                        .font(.system(size: 48, weight: .semibold, design: .rounded))
                        .monospacedDigit()
                        .padding(.horizontal, 24)
                        .padding(.vertical, 12)
                        .background(.regularMaterial, in: RoundedRectangle(cornerRadius: 16))
                        .opacity(angleFlashOpacity)
                        .allowsHitTesting(false)
                        .accessibilityHidden(true)
                }
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
                        // Two pickers on one selection, so the supply tools
                        // stand apart from the drawing tools.
                        schematicToolPicker(SchematicTool.drawingTools)
                        schematicToolPicker(SchematicTool.powerTools)

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
                                    .help(tool.tooltip)
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
                            rotateSelection()
                        } label: {
                            Label("Rotate", systemImage: "rotate.right")
                        }
                        .keyboardShortcut("r", modifiers: [.command, .option])
                        .help("Rotate the selection 90° (R or ⌥⌘R)")
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
                        Toggle(isOn: $isSidebarShown) {
                            Label("Parts Drawer", systemImage: "sidebar.trailing")
                        }
                        .toggleStyle(.button)
                        .help("Show or hide the parts drawer")
                    } else {
                        Toggle(isOn: $isSidebarShown) {
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
            .task {
                // A file opened from another window lands here, in a new tab.
                file.loadClaimedOpen()
            }
            .onAppear {
                file.openNewWindow = { openWindow(id: "schematic") }
                SchematicSessions.shared.register(sessionID, document: document, simulation: simulation)
                simulation.setSpeed(simulationSpeed)
                installEditKeyMonitor()
            }
            .onDisappear {
                removeEditKeyMonitor()
                for target in memoryWindows {
                    dismissWindow(id: "memory", value: target)
                }
                memoryWindows = []
                for target in cpuWindows {
                    dismissWindow(id: "cpu", value: target)
                }
                cpuWindows = []
                for target in registerWindows {
                    dismissWindow(id: "registers", value: target)
                }
                registerWindows = []
                for target in terminalWindows {
                    dismissWindow(id: "terminal", value: target)
                }
                terminalWindows = []
                if isBillOfMaterialsOpen {
                    dismissWindow(id: "bom", value: BillOfMaterialsTarget(sessionID: sessionID))
                    isBillOfMaterialsOpen = false
                }
                if isDesignRuleCheckOpen {
                    dismissWindow(id: "drc", value: DesignRuleCheckTarget(sessionID: sessionID))
                    isDesignRuleCheckOpen = false
                }
            }
            .focusedSceneValue(\.schematicFile, file)
            .focusedSceneValue(\.editorPage, page)
            // Edit › Undo / Redo act on the page shown (plan-31-D).
            .focusedSceneValue(\.schematicUndo, UndoTarget(document: document, page: page))
            .focusedSceneValue(\.openBillOfMaterials) {
                isBillOfMaterialsOpen = true
                openWindow(id: "bom", value: BillOfMaterialsTarget(sessionID: sessionID))
            }
            // The check is of the PCB, so it opens from the PCB page only.
            .focusedSceneValue(\.openDesignRuleCheck, page == .pcb ? {
                isDesignRuleCheckOpen = true
                openWindow(id: "drc", value: DesignRuleCheckTarget(sessionID: sessionID))
            } : nil)
            .background(WindowReader { file.window = $0 })
            .navigationTitle(file.displayName)
    }

    /// A segmented picker over some of the schematic tools.
    private func schematicToolPicker(_ tools: [SchematicTool]) -> some View {
        Picker("Tool", selection: $tool) {
            ForEach(tools) { tool in
                Label(tool.title, systemImage: tool.symbol)
                    .help(tool.tooltip)
                    .tag(tool)
            }
        }
        .pickerStyle(.segmented)
        .labelStyle(.iconOnly)
        .labelsHidden()
        .disabled(simulation.isRunning)
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
/// colour), then Silkscreen and Document, then Ratsnest (the unrouted
/// lines), and Show All. It stays open while
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
            toggle(.ratsnest) {
                HStack(spacing: 6) {
                    Rectangle()
                        .fill(Color.red)
                        .frame(width: 12, height: 1)
                    Text("Ratsnest")
                }
            }
            .help("The unrouted connections: a red line for each the schematic makes that no copper does yet")
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
