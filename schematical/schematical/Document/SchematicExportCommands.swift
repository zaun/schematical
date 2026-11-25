import SwiftUI

/// File › Import, before Export: a DXF drawing's loops as the board's edge and its cutouts
/// (plan-65-I, `SchematicExporter.importDXF`).
struct SchematicImportMenu: View {
    let file: SchematicExporter?

    var body: some View {
        Menu("Import") {
            Button("DXF…") {
                Task { await file?.importDXF() }
            }
            .disabled(file == nil)
        }
    }
}

/// File › Export, after the document's Save items: an image of the focused
/// window's schematic, PCB page or board render (the options sheet,
/// `ImageExportDialog`), a PDF of the schematic and each PCB layer at 1:1
/// (`PrintExport`), its board as Gerber files, or the whole design as a
/// KiCad project (three files, into a folder) or an EasyEDA Pro project.
/// `SchematicFileCommands` puts it in the File menu.
struct SchematicExportMenu: View {
    let file: SchematicExporter?

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
            Button("KiCad Project…") {
                Task { await file?.exportKiCadProject() }
            }
            .disabled(file == nil)
            Button("EasyEDA Pro Project…") {
                Task { await file?.exportEasyEDAProProject() }
            }
            .disabled(file == nil)
        }
    }
}
