//
//  BoardPreviewWindow.swift
//  schematical
//
//  Board Preview: a 2D picture of one schematic window's PCB as it might
//  look made, front or back, straight down or at an angle. Opened from the
//  PCB page's Preview button; it follows the board as it is edited.
//

import AppKit
import SwiftUI
import UniformTypeIdentifiers

/// Which schematic window's board to preview.
nonisolated struct BoardPreviewTarget: Codable, Hashable, Sendable {
    let sessionID: UUID
}

struct BoardPreviewWindowView: View {
    let target: BoardPreviewTarget

    var body: some View {
        if let session = SchematicSessions.shared.session(target.sessionID) {
            BoardPreviewView(document: session.document)
        } else {
            ContentUnavailableView(
                "Schematic Closed",
                systemImage: "photo",
                description: Text("The board this preview belongs to is no longer open.")
            )
            .frame(minWidth: 520, minHeight: 240)
        }
    }
}

struct BoardPreviewView: View {
    let document: SchematicDocument
    @State private var face = BoardRenderFace.front
    @State private var angle = BoardRenderAngle.angled
    @State private var size = CGSize(width: 900, height: 640)

    static let backdrop = Gradient(colors: [Color(white: 0.24), Color(white: 0.11)])

    var body: some View {
        canvas
            .onGeometryChange(for: CGSize.self) { $0.size } action: { size = $0 }
            .frame(minWidth: 480, minHeight: 360)
            .toolbar {
                ToolbarItem(placement: .principal) {
                    Picker("Side", selection: $face) {
                        ForEach(BoardRenderFace.allCases) { Text($0.title).tag($0) }
                    }
                    .pickerStyle(.segmented)
                    .help("Show the front (top) or back (bottom) of the board")
                }
                ToolbarItem(placement: .principal) {
                    Picker("View", selection: $angle) {
                        ForEach(BoardRenderAngle.allCases) { angle in
                            Label(angle.title, systemImage: angle.symbol).tag(angle)
                        }
                    }
                    .pickerStyle(.segmented)
                    .help("Look straight down, or at an angle")
                }
                ToolbarItem(placement: .primaryAction) {
                    Button {
                        saveImage()
                    } label: {
                        Label("Save Image…", systemImage: "square.and.arrow.down")
                    }
                    .help("Save this view as a PNG image")
                }
            }
    }

    private var canvas: some View {
        Self.canvas(document: document, face: face, angle: angle)
    }

    static func canvas(document: SchematicDocument, face: BoardRenderFace, angle: BoardRenderAngle) -> some View {
        Canvas { context, size in
            BoardRenderer(document: document, face: face, angle: angle).draw(in: &context, size: size)
        }
        .background(RadialGradient(gradient: backdrop, center: .center, startRadius: 0, endRadius: 900))
    }

    /// The view as shown, at twice its size, as a PNG.
    private func saveImage() {
        let content = Self.canvas(document: document, face: face, angle: angle)
            .frame(width: size.width, height: size.height)
        let renderer = ImageRenderer(content: content)
        renderer.scale = 2
        guard let image = renderer.cgImage,
              let data = NSBitmapImageRep(cgImage: image).representation(using: .png, properties: [:]) else { return }
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.png]
        panel.nameFieldStringValue = "Board \(face.title).png"
        guard panel.runModal() == .OK, let url = panel.url else { return }
        do {
            try data.write(to: url)
        } catch {
            NSAlert(error: error).runModal()
        }
    }
}
