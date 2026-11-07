import AppKit
import CoreGraphics
import Foundation
import SwiftUI

/// Shared helpers for harness mains: the app's real objects, file open/save,
/// and offscreen PNGs of the app's real views.
@MainActor
enum Harness {
    /// The loaded part catalog (from the H.app bundle's part files).
    static let catalog: PartCatalogStore = {
        let store = PartCatalogStore()
        store.load()
        return store
    }()

    static let simulation = SimulationController(library: catalog.library)

    static func newDocument() -> SchematicDocument {
        SchematicDocument(library: catalog.library)
    }

    /// Opens a schematic file into a new document, as File > Open does.
    static func open(_ path: String) throws -> SchematicDocument {
        let url = URL(fileURLWithPath: path)
        let document = newDocument()
        let file = try SchematicFile.decode(Data(contentsOf: url))
        try file.apply(to: document, catalog: catalog, fileURL: url)
        return document
    }

    /// Saves a document, as File > Save does.
    static func save(_ document: SchematicDocument, to path: String) throws {
        let url = URL(fileURLWithPath: path)
        try SchematicFile(document: document, fileURL: url).encoded().write(to: url)
    }

    /// Renders a view to a PNG at 2×, in the light appearance.
    static func render<V: View>(_ view: V, size: CGSize, to path: String) {
        let content = view
            .frame(width: size.width, height: size.height)
            .environment(\.colorScheme, .light)
        let renderer = ImageRenderer(content: content)
        renderer.scale = 2
        guard let image = renderer.cgImage,
              let data = NSBitmapImageRep(cgImage: image).representation(using: .png, properties: [:]) else {
            print("render failed: \(path)")
            return
        }
        do {
            try FileManager.default.createDirectory(
                at: URL(fileURLWithPath: path).deletingLastPathComponent(), withIntermediateDirectories: true)
            try data.write(to: URL(fileURLWithPath: path))
            print("wrote \(path)")
        } catch {
            print("write failed: \(path): \(error)")
        }
    }

    /// Renders the schematic canvas. `origin` is the world point at the
    /// top-left; `tool` decides tool-only overlays.
    static func renderCanvas(_ document: SchematicDocument, to path: String,
                             size: CGSize = CGSize(width: 600, height: 400),
                             origin: CGPoint = .zero, zoom: CGFloat = 1, tool: SchematicTool = .pointer) {
        let viewport = CanvasViewport()
        viewport.updateSize(size)
        viewport.pan(to: CGSize(width: -origin.x * zoom, height: -origin.y * zoom))
        if zoom != 1 {
            // setZoom keeps the centre fixed, so set it first and re-pan.
            viewport.setZoom(zoom)
            viewport.pan(to: CGSize(width: -origin.x * viewport.zoom, height: -origin.y * viewport.zoom))
        }
        let view = SchematicCanvasView(
            catalog: catalog, document: document, viewport: viewport, tool: tool, simulation: simulation)
        render(view, size: size, to: path)
    }

    /// Renders the Schematic inspector (what the sidebar shows for the selection).
    static func renderSidebar(_ document: SchematicDocument, to path: String,
                              size: CGSize = CGSize(width: 320, height: 600)) {
        let view = SchematicSidebarView(catalog: catalog, document: document, simulation: simulation)
        render(view, size: size, to: path)
    }
}
