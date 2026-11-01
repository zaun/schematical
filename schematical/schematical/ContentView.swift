//
//  ContentView.swift
//  schematical
//
//  Created by Justin Zaun on 10/31/26.
//

import SwiftUI

struct ContentView: View {
    @State private var catalog = ChipCatalogStore()
    @State private var document = SchematicDocument()
    @State private var viewport = CanvasViewport()
    @State private var tool: SchematicTool = .pointer
    @State private var isDrawerShown = true

    var body: some View {
        SchematicCanvasView(catalog: catalog, document: document, viewport: viewport, tool: tool)
            .inspector(isPresented: $isDrawerShown) {
                ChipDrawerView(catalog: catalog)
                    .inspectorColumnWidth(min: 220, ideal: 260, max: 360)
            }
            .toolbar {
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
