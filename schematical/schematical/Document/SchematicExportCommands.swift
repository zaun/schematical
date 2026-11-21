import SwiftUI

/// File > Export, after Save As…: an image of the focused window's
/// schematic, PCB page or board render (the options sheet,
/// `ImageExportDialog`), a PDF of the schematic and each PCB layer at 1:1
/// (`PrintExport`), or its board as Gerber files.
/// `SchematicFileCommands` puts it in the File menu.
struct SchematicExportMenu: View {
    let file: SchematicFileController?

    var body: some View {
        Menu("Export") {
            Button("Image…") {
                Task { await file?.exportImage() }
            }
            .keyboardShortcut("e", modifiers: [.command, .shift])
            .disabled(file == nil)
            Button("PDF…") {
                Task { await file?.exportPDF() }
            }
            .disabled(file == nil)
            Button("Gerber PCB…") {
                Task { await file?.exportGerber() }
            }
            .disabled(file == nil)
        }
    }
}
