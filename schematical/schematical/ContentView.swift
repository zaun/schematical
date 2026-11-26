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
    /// What the window holds and does (`ContentViewController`); this view
    /// draws it.
    @State private var controller: ContentViewController
    /// Simulation speed (circuit time per wall time), an app preference.
    @AppStorage("simulationSpeed") private var simulationSpeed = 1.0
    /// The Trace tool's router assist (plan-38-L), off until turned on.
    @AppStorage("traceAssist") private var traceAssist = false
    /// The supply the VCC tool places (a `SupplyChoice` name), an app
    /// preference; the controller sets it.
    @AppStorage(ContentViewController.supplyVoltageKey) private var supplyVoltage = SupplyChoice.defaultName

    init(model: SchematicEditorModel) {
        self.init(controller: ContentViewController(model: model))
    }

    /// The editor of `controller`'s window (tests keep the controller, to
    /// reach what the view shows).
    init(controller: ContentViewController) {
        _controller = State(initialValue: controller)
    }

    private var model: SchematicEditorModel { controller.model }
    private var document: SchematicDocument { controller.document }
    private var simulation: SimulationController { controller.simulation }

    enum EditKey { case rotate, rotateBack, flip, angle, delete, cut, copy, paste, selectAll }

    /// The editing key a press is: a bare A (no ⌘, ⌃, ⌥ or ⇧), ⇧⌘R, or
    /// ⌘⌫, ⌘R, ⌘F, ⌘X, ⌘C, ⌘V or ⌘A (⌘ alone). Never a held-key repeat.
    /// Nil for anything else.
    nonisolated static func editKey(keyCode: UInt16, _ characters: String?, _ modifiers: NSEvent.ModifierFlags,
                                    isRepeat: Bool) -> EditKey? {
        guard !isRepeat else { return nil }
        let held = modifiers.intersection([.command, .control, .option, .shift])
        // 51 is the Delete (backspace) key.
        if keyCode == 51 { return held == .command ? .delete : nil }
        if held == [.command, .shift] {
            return characters?.lowercased() == "r" ? .rotateBack : nil
        }
        if held == .command {
            switch characters {
            case "r": return .rotate
            case "f": return .flip
            case "x": return .cut
            case "c": return .copy
            case "v": return .paste
            case "a": return .selectAll
            default: return nil
            }
        }
        guard held.isEmpty else { return nil }
        return characters == "a" ? .angle : nil
    }

    @ViewBuilder
    private var pageContent: some View {
        switch controller.page {
        case .schematic:
            SchematicCanvasView(catalog: controller.catalog, document: document, viewport: controller.viewport,
                                tool: controller.tool, supplyVoltage: supplyVoltage, simulation: simulation,
                                onOpenMemory: controller.openMemory, onOpenCpu: controller.openCpu,
                                onOpenTerminal: controller.openTerminal, onOpenRegisters: controller.openRegisters,
                                onOpenDocumentInfo: controller.openDocumentInfo, pointer: controller.schematicPointer,
                                onPartPlaced: controller.partPlaced)
        case .pcb:
            PCBCanvasView(document: document, viewport: controller.boardViewport, tool: controller.pcbTool,
                          visible: controller.visibility, traceLayer: controller.traceLayer,
                          cutoutShape: controller.cutoutShape,
                          traceAngle: controller.traceAngle, traceAssist: traceAssist,
                          onShowLayer: controller.showLayer, onLayerChange: controller.changeTraceLayer,
                          pointer: controller.boardPointer, highlightedNet: $controller.highlightedNet,
                          routePreview: controller.routePreview, onRoute: controller.route,
                          onPlaced: controller.textPlaced)
            .overlay {
                if let angleFlash = controller.angleFlash {
                    Text("\(angleFlash)°")
                        .font(.system(size: 48, weight: .semibold, design: .rounded))
                        .monospacedDigit()
                        .padding(.horizontal, 24)
                        .padding(.vertical, 12)
                        .background(.regularMaterial, in: RoundedRectangle(cornerRadius: 16))
                        .opacity(controller.angleFlashOpacity)
                        .allowsHitTesting(false)
                        .accessibilityHidden(true)
                }
            }
        }
    }

    var body: some View {
        editor
            // The speed is an app preference: every window runs at it.
            .onChange(of: simulationSpeed) { _, speed in simulation.setSpeed(speed) }
            .alert("Reset References?", isPresented: $controller.isConfirmingResetReferences) {
                Button("Reset", action: controller.resetReferences)
                Button("Cancel", role: .cancel) {}
            } message: {
                Text("Every part gets its default prefix back (U, or R and C for resistors and capacitors) and is numbered again from 1, top to bottom and left to right. You can undo this.")
            }
            .alert("Custom Supply Voltage", isPresented: $controller.isAskingCustomVoltage) {
                TextField("Volts", text: $controller.customVoltageText)
                Button("Use", action: controller.useCustomVoltage)
                Button("Cancel", role: .cancel) {}
            } message: {
                Text(controller.customVoltageMessage)
            }
            .sheet(isPresented: $controller.isAutorouteOpen) {
                AutorouteSheet(model: controller.autorouteModel, onClose: controller.closeAutoroute,
                               onHighlight: controller.highlight, onOpenDesignRuleCheck: controller.openDesignRuleCheck)
                .interactiveDismissDisabled(controller.isRouting)
            }
    }

    private var editor: some View {
        pageContent
            .inspector(isPresented: $controller.isSidebarShown) {
                ZStack {
                    // Kept in the tree on the PCB page too, so the parts
                    // library keeps its search, filters and open groups.
                    SchematicSidebarView(catalog: controller.catalog, document: document, simulation: simulation,
                                         isActive: controller.page == .schematic)
                    if controller.page == .pcb {
                        PCBSidebarView(document: document, highlightedNet: $controller.highlightedNet,
                                       onRoute: controller.route)
                    }
                }
                // macOS opens an inspector at its minimum width and ignores
                // `ideal`, so the minimum is the width it should open at.
                .inspectorColumnWidth(min: 350, ideal: 350, max: 500)
            }
            .toolbar { toolbar }
            .overlay(alignment: .bottom) { messages }
            .onChange(of: controller.traceLayers) { _, layers in controller.traceLayersChanged(layers) }
            .task(id: document.editorMessage) { await controller.expireEditorMessage() }
            // Erasing a port with wires, a bus line with ports, or unticking
            // a wired pin asks first.
            .alert(document.pendingRemoval?.message ?? "", isPresented: $controller.isConfirmingRemoval) {
                Button("Delete", role: .destructive, action: document.confirmPendingRemoval)
                Button("Cancel", role: .cancel, action: document.cancelPendingRemoval)
            }
            .onChange(of: controller.page) { _, newPage in controller.pageChanged(to: newPage) }
            .onAppear { controller.appear(speed: simulationSpeed) }
            .onDisappear { controller.disappear() }
            // The canvases install their key monitors through the model's.
            .environment(\.editorEventMonitors, model.eventMonitors)
            .focusedSceneValue(\.schematicExporter, model.file)
            // The menus read the page, the selection and the rest through
            // the model (`EditorMenus`), not through values published here.
            .focusedSceneValue(\.schematicEditor, model)
    }

    /// The simulation's error, else a refused edit's reason (on either
    /// page), along the bottom.
    @ViewBuilder
    private var messages: some View {
        if controller.page == .schematic, let message = simulation.errorMessage {
            Label("Simulation stopped: \(message)", systemImage: "exclamationmark.triangle.fill")
                .font(.callout)
                .padding(.horizontal, 14)
                .padding(.vertical, 8)
                .background(.red.opacity(0.85), in: Capsule())
                .foregroundStyle(.white)
                .padding(.bottom, 16)
        } else if let message = document.editorMessage {
            Label(message, systemImage: "bolt.trianglebadge.exclamationmark")
                .font(.callout)
                .padding(.horizontal, 14)
                .padding(.vertical, 8)
                .background(.orange.opacity(0.9), in: Capsule())
                .foregroundStyle(.white)
                .padding(.bottom, 16)
        }
    }

    @ToolbarContentBuilder
    private var toolbar: some ToolbarContent {
        // The page picker on the left; the tools stay centred.
        ToolbarItem(placement: .navigation) {
            Picker("Page", selection: $controller.page) {
                ForEach(EditorPage.allCases) { page in
                    Text(page.title).tag(page)
                }
            }
            .pickerStyle(.segmented)
            .labelsHidden()
            .help("Show the schematic or the PCB")
        }

        // Run/Stop and the speed are in the Simulation menu
        // (`SimulationCommands`); only the slow-down notice is here.
        if controller.page == .schematic {
            ToolbarItem(placement: .navigation) {
                ReachedSpeedLabel(simulation: simulation)
            }
        }

        ToolbarItemGroup(placement: .principal) {
            if controller.page == .schematic {
                schematicTools
            } else {
                pcbTools
            }
        }

        if controller.page == .pcb {
            pcbActions
        }

        ToolbarItemGroup(placement: .primaryAction) {
            zoomControls
        }

        ToolbarItem(placement: .primaryAction) {
            Toggle(isOn: $controller.isSidebarShown) {
                Label(controller.page == .schematic ? "Parts Drawer" : "Inspector", systemImage: "sidebar.trailing")
            }
            .toggleStyle(.button)
            .help(controller.page == .schematic ? "Show or hide the parts drawer" : "Show or hide the inspector")
        }
    }

    /// The Schematic page's tools. Two pickers on one selection, so the
    /// supply tools stand apart from the drawing tools; then the VCC and
    /// Pull-up tools' rail. Picking one also picks VCC, unless Pull-up is
    /// the tool.
    @ViewBuilder
    private var schematicTools: some View {
        schematicToolPicker(SchematicTool.drawingTools)
        schematicToolPicker(SchematicTool.powerTools)
        Menu(supplyVoltage) {
            ForEach(SupplyChoice.names, id: \.self) { name in
                Button(name) { controller.pickSupply(name) }
            }
            Divider()
            // Any other rail, such as a nixie tube's 170 V.
            Button("Custom…", action: controller.askCustomVoltage)
        }
        .help("Supply voltage for the VCC and Pull-up tools")
        .disabled(simulation.isRunning)
    }

    /// The PCB page's tools: the tool picker, the Trace tool's layer (every
    /// board layer that isn't a plane; picking one also picks Trace and
    /// shows that layer), the Cutout tool's shape (picking one also picks
    /// the tool) and the Trace tool's router assist (plan-38-L), remembered.
    @ViewBuilder
    private var pcbTools: some View {
        Picker("Tool", selection: $controller.pcbTool) {
            ForEach(PCBTool.allCases) { tool in
                Label(tool.title, systemImage: tool.symbol)
                    .help(tool.tooltip)
                    .tag(tool)
            }
        }
        .pickerStyle(.segmented)
        .labelStyle(.iconOnly)
        .labelsHidden()

        Menu(controller.traceLayer.title) {
            ForEach(controller.traceLayers, id: \.self) { layer in
                Button(layer.title) { controller.pickTraceLayer(layer) }
            }
        }
        .help("Layer for the Trace tool")

        Menu(controller.cutoutShape.title) {
            ForEach(CutoutShape.allCases) { shape in
                Button(shape.title) { controller.pickCutoutShape(shape) }
            }
        }
        .help("Shape for the Cutout tool")

        Toggle(isOn: $traceAssist) {
            Label("Assist", systemImage: "wand.and.rays")
        }
        .toggleStyle(.button)
        .labelStyle(.iconOnly)
        .help("Trace assist — walks around obstacles and stops at illegal routes")
    }

    /// The PCB page's Rotate, Autoroute (Route ▸ Autoroute…'s sheet), Preview
    /// (a picture of the board as made, not a tool) and View (which layers
    /// it draws, just left of the zoom).
    @ToolbarContentBuilder
    private var pcbActions: some ToolbarContent {
        ToolbarItem(placement: .primaryAction) {
            Button(action: controller.rotate) {
                Label("Rotate", systemImage: "rotate.right")
            }
            .help("Rotate the selection 90° clockwise (⌘R; ⇧⌘R turns it the other way)")
            .disabled(!controller.canRotate)
        }
        ToolbarItem(placement: .primaryAction) {
            Button(action: controller.routeActions.autoroute) {
                Label("Autoroute", systemImage: "point.topleft.down.curvedto.point.bottomright.up")
            }
            .help("Route the board's unrouted connections (⌥⌘R)")
            .disabled(document.isPlaying || controller.isRouting)
        }
        ToolbarItem(placement: .primaryAction) {
            Button(action: controller.openBoardPreview) {
                Label("Preview", systemImage: "photo")
            }
            .help("Show a picture of the finished board, front or back")
        }
        ToolbarItem(placement: .primaryAction) {
            Button(action: controller.toggleLayerView) {
                Label("View", systemImage: "square.3.layers.3d")
            }
            .help("Choose which layers to show")
            .popover(isPresented: $controller.isLayerViewShown, arrowEdge: .bottom) {
                LayerViewPopover(board: document.board, hidden: $controller.hiddenLayers)
            }
        }
    }

    /// A segmented picker over some of the schematic tools.
    private func schematicToolPicker(_ tools: [SchematicTool]) -> some View {
        Picker("Tool", selection: $controller.tool) {
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
        Button(action: controller.zoomOut) {
            Label("Zoom Out", systemImage: "minus.magnifyingglass")
        }
        .keyboardShortcut("-", modifiers: .command)
        .help("Zoom out (⌘-)")

        Menu {
            ForEach(CanvasViewport.zoomLevels, id: \.self) { level in
                Button(ContentViewController.percent(level)) { controller.setZoom(level) }
            }
            Divider()
            Button("Actual Size") { controller.setZoom(1) }
                .keyboardShortcut("0", modifiers: .command)
        } label: {
            Text(ContentViewController.percent(controller.activeViewport.zoom))
                .monospacedDigit()
        }
        .help("Zoom level")

        Button(action: controller.zoomIn) {
            Label("Zoom In", systemImage: "plus.magnifyingglass")
        }
        .keyboardShortcut("=", modifiers: .command)
        .help("Zoom in (⌘=)")
    }
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
/// colour), then PCB (the board's fill), Silkscreen and Document, then Ratsnest (the unrouted
/// lines), and Show All. It stays open while
/// the user ticks several boxes; a menu would close after each one.
struct LayerViewPopover: View {
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
            toggle(.board) {
                HStack(spacing: 6) {
                    RoundedRectangle(cornerRadius: 2)
                        .fill(board.color.fill)
                        .frame(width: 12, height: 12)
                    Text("PCB")
                }
            }
            .help("The board's colour; hidden, only its outline shows, so the Planes' shading is easier to see")
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
