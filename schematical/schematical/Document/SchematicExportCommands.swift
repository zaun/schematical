import SwiftUI

/// File > Export, after Save As…: an image of the focused window's
/// schematic, PCB page or board render (the options sheet,
/// `ImageExportDialog`), or its board as Gerber files.
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
            Button("Gerber PCB…") {
                Task { await file?.exportGerber() }
            }
            .disabled(file == nil)
        }
    }
}
