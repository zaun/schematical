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
    let documentID: UUID
}

struct BoardPreviewWindowView: View {
    let target: BoardPreviewTarget

    var body: some View {
        if let session = SchematicEditorModel.session(target.documentID) {
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

/// What the preview shows (side, view, size) and Save Image….
@MainActor
@Observable
final class BoardPreviewModel {
    nonisolated deinit {}

    let document: SchematicDocument
    var face = BoardRenderFace.front
    var angle = BoardRenderAngle.angled
    /// The canvas's size as laid out, in points.
    var size = CGSize(width: 900, height: 640)
    /// Asks where to save an image named so: the save panel in the app.
    @ObservationIgnored var chooseURL: (String) -> URL? = { BoardPreviewModel.savePanel(name: $0) }
    /// Shows a failed write: an alert in the app.
    @ObservationIgnored var showError: (Error) -> Void = { NSAlert(error: $0).runModal() }

    init(document: SchematicDocument) {
        self.document = document
    }

    /// The suggested file name.
    var imageName: String { "Board \(face.title).png" }

    /// The view as shown, at twice its size, as a PNG; nil when it cannot be drawn.
    func pngData() -> Data? {
        let content = BoardPreviewView.canvas(document: document, face: face, angle: angle)
            .frame(width: size.width, height: size.height)
        let renderer = ImageRenderer(content: content)
        renderer.scale = 2
        guard let image = renderer.cgImage else { return nil }
        return NSBitmapImageRep(cgImage: image).representation(using: .png, properties: [:])
    }

    /// Save Image…: asks where, then writes the PNG there.
    func saveImage() {
        guard let data = pngData(), let url = chooseURL(imageName) else { return }
        do {
            try data.write(to: url)
        } catch {
            showError(error)
        }
    }

    /// The save panel, for PNG, suggesting `name`; nil when cancelled.
    static func savePanel(name: String) -> URL? {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.png]
        panel.nameFieldStringValue = name
        return panel.runModal() == .OK ? panel.url : nil
    }
}

struct BoardPreviewView: View {
    @State private var model: BoardPreviewModel

    static let backdrop = Gradient(colors: [Color(white: 0.24), Color(white: 0.11)])

    init(document: SchematicDocument) {
        self.init(model: BoardPreviewModel(document: document))
    }

    init(model: BoardPreviewModel) {
        _model = State(initialValue: model)
    }

    var body: some View {
        Self.canvas(document: model.document, face: model.face, angle: model.angle)
            .onGeometryChange(for: CGSize.self) { $0.size } action: { model.size = $0 }
            .frame(minWidth: 480, minHeight: 360)
            .toolbar {
                ToolbarItem(placement: .principal) {
                    Picker("Side", selection: $model.face) {
                        ForEach(BoardRenderFace.allCases) { Text($0.title).tag($0) }
                    }
                    .pickerStyle(.segmented)
                    .help("Show the front (top) or back (bottom) of the board")
                }
                ToolbarItem(placement: .principal) {
                    Picker("View", selection: $model.angle) {
                        ForEach(BoardRenderAngle.allCases) { angle in
                            Label(angle.title, systemImage: angle.symbol).tag(angle)
                        }
                    }
                    .pickerStyle(.segmented)
                    .help("Look straight down, or at an angle")
                }
                ToolbarItem(placement: .primaryAction) {
                    Button {
                        model.saveImage()
                    } label: {
                        Label("Save Image…", systemImage: "square.and.arrow.down")
                    }
                    .help("Save this view as a PNG image")
                }
            }
    }

    static func canvas(document: SchematicDocument, face: BoardRenderFace, angle: BoardRenderAngle) -> some View {
        Canvas { context, size in
            BoardRenderer(document: document, face: face, angle: angle).draw(in: &context, size: size)
        }
        .background(RadialGradient(gradient: backdrop, center: .center, startRadius: 0, endRadius: 900))
    }
}
