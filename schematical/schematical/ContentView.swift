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

    init() {
        let catalog = ChipCatalogStore()
        let document = SchematicDocument()
        let simulation = SimulationController()
        _catalog = State(initialValue: catalog)
        _document = State(initialValue: document)
        _simulation = State(initialValue: simulation)
    }

    var body: some View {
        SchematicCanvasView(
            catalog: catalog,
            document: document,
            viewport: viewport,
            tool: tool,
            simulation: simulation
        ) {
            // Placing a part from the drawer returns to the Pointer tool.
            tool = .pointer
        }
            .inspector(isPresented: $isDrawerShown) {
                ChipDrawerView(catalog: catalog)
                    .disabled(simulation.isRunning)
                    .inspectorColumnWidth(min: 220, ideal: 260, max: 360)
            }
            .toolbar {
                ToolbarItem(placement: .navigation) {
                    Button {
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
            .navigationTitle("Schematical")
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
