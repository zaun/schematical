import SwiftUI
import UniformTypeIdentifiers

/// The three File > Export items (plan-28-D).
enum SchematicExportFormat: CaseIterable, Identifiable {
    case svg, png, jpg

    var id: Self { self }

    var menuTitle: String {
        switch self {
        case .svg: "SVG Schematic"
        case .png: "PNG Schematic"
        case .jpg: "JPG Schematic"
        }
    }

    var fileExtension: String {
        switch self {
        case .svg: "svg"
        case .png: "png"
        case .jpg: "jpg"
        }
    }

    var contentType: UTType {
        switch self {
        case .svg: .svg
        case .png: .png
        case .jpg: .jpeg
        }
    }

    /// The file's bytes; nil for an empty scene.
    func data(for scene: SchematicScene) -> Data? {
        switch self {
        case .svg: SchematicExport.svg(scene)
        case .png: SchematicExport.png(scene)
        case .jpg: SchematicExport.jpg(scene)
        }
    }
}

/// File > Export, after Save As…: writes the focused window's schematic.
/// `SchematicFileCommands` puts it in the File menu. The items stay enabled
/// on an empty schematic; the action says why it can't export.
struct SchematicExportMenu: View {
    let file: SchematicFileController?

    var body: some View {
        Menu("Export") {
            ForEach(SchematicExportFormat.allCases) { format in
                Button(format.menuTitle) {
                    Task { await file?.export(format) }
                }
                .disabled(file == nil)
            }
        }
    }
}
