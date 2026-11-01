//
//  ContentView.swift
//  schematical
//
//  Created by Justin Zaun on 10/31/26.
//

import SwiftUI

struct ContentView: View {
    @State private var catalog: ChipCatalogStore
    @State private var document: SchematicDocument
    @State private var viewport = CanvasViewport()
    @State private var tool: SchematicTool = .pointer
    @State private var isDrawerShown = true
    @State private var simulation: SimulationController
    @State private var file: SchematicFileController
    /// Identifies this window to the memory windows it opens.
    @State private var sessionID = UUID()
    @Environment(\.openWindow) private var openWindow
    @Environment(\.dismissWindow) private var dismissWindow
    /// Memory windows opened from this window, closed along with it.
    @State private var memoryWindows: Set<MemoryWindowTarget> = []

    init() {
        let catalog = ChipCatalogStore()
        let document = SchematicDocument()
        let simulation = SimulationController()
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

    var body: some View {
        SchematicCanvasView(
            catalog: catalog,
            document: document,
            viewport: viewport,
            tool: tool,
            simulation: simulation,
            onOpenMemory: { chipID in
                let target = MemoryWindowTarget(sessionID: sessionID, chipID: chipID)
                memoryWindows.insert(target)
                openWindow(id: "memory", value: target)
            }
        ) {
            // Placing a part from the drawer returns to the Pointer tool.
            tool = .pointer
        }
            .inspector(isPresented: $isDrawerShown) {
                ChipDrawerView(catalog: catalog)
                    .disabled(simulation.isRunning)
                    // macOS opens an inspector at its minimum width and ignores
                    // `ideal`, so the minimum is the width it should open at.
                    .inspectorColumnWidth(min: 270, ideal: 270, max: 500)
            }
            .toolbar {
                ToolbarItemGroup(placement: .navigation) {
                    fileControls
                }

                ToolbarItem(placement: .navigation) {
                    Button {
                        if !simulation.isRunning {
                            // Play mode starts with the Pointer tool and nothing selected.
                            tool = .pointer
                            document.selectedChipID = nil
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

                ToolbarItem(placement: .principal) {
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
                }

                ToolbarItemGroup(placement: .primaryAction) {
                    zoomControls
                }

                ToolbarItem(placement: .primaryAction) {
                    Toggle(isOn: $isDrawerShown) {
                        Label("Parts Drawer", systemImage: "sidebar.trailing")
                    }
                    .toggleStyle(.button)
                    .help("Show or hide the parts drawer")
                }
            }
            .overlay(alignment: .bottom) {
                if let message = simulation.errorMessage {
                    Label("Simulation stopped: \(message)", systemImage: "exclamationmark.triangle.fill")
                        .font(.callout)
                        .padding(.horizontal, 14)
                        .padding(.vertical, 8)
                        .background(.red.opacity(0.85), in: Capsule())
                        .foregroundStyle(.white)
                        .padding(.bottom, 16)
                }
            }
            .task { catalog.load() }
            .onAppear {
                SchematicSessions.shared.register(sessionID, document: document, simulation: simulation)
            }
            .onDisappear {
                for target in memoryWindows {
                    dismissWindow(id: "memory", value: target)
                }
                memoryWindows = []
            }
            .focusedSceneValue(\.schematicFile, file)
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
            viewport.zoomOut()
        } label: {
            Label("Zoom Out", systemImage: "minus.magnifyingglass")
        }
        .keyboardShortcut("-", modifiers: .command)
        .help("Zoom out (⌘-)")

        Menu {
            ForEach(CanvasViewport.zoomLevels, id: \.self) { level in
                Button("\(Int((level * 100).rounded()))%") {
                    viewport.setZoom(level)
                }
            }
            Divider()
            Button("Actual Size") {
                viewport.setZoom(1)
            }
            .keyboardShortcut("0", modifiers: .command)
        } label: {
            Text("\(Int((viewport.zoom * 100).rounded()))%")
                .monospacedDigit()
        }
        .help("Zoom level")

        Button {
            viewport.zoomIn()
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
