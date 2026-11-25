import AppKit
import Foundation
import Observation
import PDFKit
import SwiftUI
import UniformTypeIdentifiers

/// Exports and print for one schematic document (plan-61-B: the document
/// class, `SchematicNSDocument`, does Save, Open, the close and quit prompts
/// and tabs). Built by the document's `SchematicEditorModel`.
@MainActor
@Observable
final class SchematicExporter {
    /// As `SchematicDocument`'s: an isolated deinit aborts the runtime when
    /// a closing document frees this.
    nonisolated deinit {}

    /// The document's file, nil for one never saved. Set by the document;
    /// exports are named after it and start in its folder.
    var fileURL: URL?

    /// The window showing this schematic: sheets attach to it.
    @ObservationIgnored
    weak var window: NSWindow?

    /// The system document this exports and prints, set by it: Print… goes
    /// through it.
    @ObservationIgnored
    weak var systemDocument: NSDocument?

    private let document: SchematicDocument
    private let catalog: PartCatalogStore

    init(document: SchematicDocument, catalog: PartCatalogStore) {
        self.document = document
        self.catalog = catalog
    }

    var displayName: String {
        fileURL?.deletingPathExtension().lastPathComponent ?? "Untitled"
    }

    // MARK: Actions

    /// File › Export › KiCad Project…: asks for a folder, then writes the
    /// project's three files into it, named after the schematic
    /// (`KiCadExport`). A folder, not a file name: the sandbox lets the app
    /// write only the one file a save panel names, and the project is
    /// three. Like any export, not a save.
    func exportKiCadProject() async {
        let panel = kiCadFolderPanel()
        guard await run(panel) == .OK, let folder = panel.url else { return }
        let existing = kiCadFiles(in: folder).filter { FileManager.default.fileExists(atPath: $0.path) }
        if !existing.isEmpty {
            let alert = NSAlert()
            alert.alertStyle = .warning
            alert.messageText = "Replace the KiCad project “\(kiCadBaseName)” in “\(folder.lastPathComponent)”?"
            alert.informativeText = "The folder already has "
                + existing.map { "“\($0.lastPathComponent)”" }.formatted(.list(type: .and))
                + ". Exporting replaces \(existing.count == 1 ? "it" : "them")."
            alert.addButton(withTitle: "Replace")
            alert.addButton(withTitle: "Cancel")
            guard await run(alert) == .alertFirstButtonReturn else { return }
        }
        do {
            try writeKiCadProject(into: folder)
        } catch {
            present(error, title: "Couldn't export “\(displayName)”")
        }
    }

    /// The folder chooser of `exportKiCadProject()`.
    func kiCadFolderPanel() -> NSOpenPanel {
        let panel = NSOpenPanel()
        panel.canChooseDirectories = true
        panel.canChooseFiles = false
        panel.allowsMultipleSelection = false
        panel.canCreateDirectories = true
        panel.prompt = "Export"
        panel.message = "Choose the folder for "
            + KiCadExport.extensions.map { "\(kiCadBaseName).\($0)" }.formatted(.list(type: .and)) + "."
        if let fileURL {
            panel.directoryURL = fileURL.deletingLastPathComponent()
        }
        return panel
    }

    /// The schematic's name as the KiCad files are named.
    var kiCadBaseName: String { KiCadExport.baseName(displayName) }

    /// The three files `writeKiCadProject(into:)` writes.
    func kiCadFiles(in folder: URL) -> [URL] {
        KiCadExport.extensions.map { folder.appendingPathComponent("\(kiCadBaseName).\($0)") }
    }

    /// Writes `<name>.kicad_pro`, `.kicad_sch` and `.kicad_pcb` into
    /// `folder`, `<name>` being the schematic's. The folder is one the user
    /// chose: its security scope is held for the write. Throws for a
    /// failed write.
    func writeKiCadProject(into folder: URL) throws {
        let scoped = folder.startAccessingSecurityScopedResource()
        defer { if scoped { folder.stopAccessingSecurityScopedResource() } }
        try writeKiCad(to: folder.appendingPathComponent("\(kiCadBaseName).kicad_pro"))
    }

    /// File › Export › EasyEDA Pro Project…: where to save, then the one
    /// `.epro` file (`EasyEDAProExport`). Like any export, not a save.
    func exportEasyEDAProProject() async {
        let panel = exportPanel(.easyEDAPro)
        guard await run(panel) == .OK, let url = panel.url else { return }
        do {
            try writeEasyEDAPro(to: url)
        } catch {
            present(error, title: "Couldn't export “\(displayName)”")
        }
    }

    /// A save panel for one exported file of `format`, named after the
    /// schematic and starting in its folder. It has no Format menu: each
    /// format is its own item under File › Export.
    func exportPanel(_ format: SaveFormat) -> NSSavePanel {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [format.contentType]
        panel.canCreateDirectories = true
        panel.nameFieldStringValue = "\(displayName).\(format.fileExtension)"
        if let fileURL {
            panel.directoryURL = fileURL.deletingLastPathComponent()
        }
        return panel
    }

    /// Writes the project as `url`, one `.epro` file named for its folder and
    /// name. Throws for a failed write.
    func writeEasyEDAPro(to url: URL) throws {
        let base = KiCadExport.baseName(url.deletingPathExtension().lastPathComponent)
        try EasyEDAProExport.data(for: document, base: base).write(to: url, options: .atomic)
    }

    /// Writes `<name>.kicad_pro`, `.kicad_sch` and `.kicad_pcb` into the
    /// folder of `url`, `<name>` being its file name without extension.
    /// Throws for a failed write.
    func writeKiCad(to url: URL) throws {
        let base = KiCadExport.baseName(url.deletingPathExtension().lastPathComponent)
        try KiCadExport.write(document, base: base, to: url.deletingLastPathComponent())
    }

    /// The offer to add a just-opened schematic's custom parts to the User
    /// Library (plan-53-C). Asked at every open; "Not Now" changes nothing.
    func offerToAdd(_ parts: [CatalogPart]) async {
        let alert = UserLibraryPrompts.fileOpenAlert(names: parts.map(\.name))
        guard await run(alert) == .alertFirstButtonReturn else { return }
        await UserLibraryPrompts.addShowingFailures(parts, catalog: catalog, on: window)
    }

    // MARK: Import

    /// What File ▸ Import ▸ DXF… hands back: the placed loops and the mode.
    struct DXFImportChoice {
        var placed: DXFPlacedImport
        var mode: DXFImportMode
    }

    /// Reads a DXF drawing; throws `DXFError` for a binary or non-DXF file.
    func loadDXF(from url: URL) throws -> DXFDrawing {
        try DXFReader.parse(data: Data(contentsOf: url))
    }

    /// File > Import > DXF…: asks for a drawing, shows what it holds in `DXFImportSheet`, and
    /// applies the choice as one undo step (`SchematicDocument.applyDXFImport`).
    func importDXF() async {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [UTType(filenameExtension: "dxf") ?? .data]
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false
        if let fileURL { panel.directoryURL = fileURL.deletingLastPathComponent() }
        guard await run(panel) == .OK, let url = panel.url else { return }
        do {
            let drawing = try loadDXF(from: url)
            let state = DXFImportState(fileName: url.lastPathComponent, drawing: drawing, board: document.board)
            let choice: DXFImportChoice? = await runSheet { finish in
                DXFImportSheet(state: state, onCancel: { finish(nil) },
                               onImport: { finish(DXFImportChoice(placed: state.placed, mode: state.mode)) })
            }
            if let choice { document.applyDXFImport(choice.placed, mode: choice.mode) }
        } catch {
            present(error, title: "Couldn't import “\(url.lastPathComponent)”")
        }
    }

    /// File > Export > Image…: the options sheet (`ImageExportDialog`),
    /// then where to save, then the file. An export is not a save: the
    /// window's file and saved state are unchanged.
    func exportImage() async {
        guard let options = await runImageOptions(ImageExportOptions.saved()) else { return }
        options.save()
        let panel = NSSavePanel()
        panel.allowedContentTypes = [options.format.contentType]
        panel.canCreateDirectories = true
        panel.nameFieldStringValue = "\(displayName) \(options.view.fileSuffix).\(options.format.fileExtension)"
        if let fileURL {
            panel.directoryURL = fileURL.deletingLastPathComponent()
        }
        guard await run(panel) == .OK, let url = panel.url else { return }
        do {
            try writeImage(options, to: url)
        } catch {
            present(error, title: "Couldn't export “\(displayName)”")
        }
    }

    /// Writes the picture `options` describe to `url`. Throws for an empty
    /// schematic or a failed write.
    func writeImage(_ options: ImageExportOptions, to url: URL) throws {
        guard let data = ImageExport.data(for: document, options) else {
            throw ExportError.nothingToExport
        }
        try data.write(to: url, options: .atomic)
    }

    /// The image options sheet on this window; nil when cancelled.
    private func runImageOptions(_ options: ImageExportOptions) async -> ImageExportOptions? {
        await runSheet { finish in
            ImageExportDialog(document: document, options: options, onCancel: { finish(nil) }, onExport: { finish($0) })
        }
    }

    /// `content` as a sheet on this window (a modal window when it has none), until it calls `finish`.
    private func runSheet<Content: View, Result>(
        _ content: (@escaping (Result?) -> Void) -> Content
    ) async -> Result? {
        await withCheckedContinuation { continuation in
            let sheet = NSWindow(contentRect: .zero, styleMask: [.titled], backing: .buffered, defer: true)
            let parent = window ?? NSApp.keyWindow
            var finished = false
            let finish: (Result?) -> Void = { result in
                guard !finished else { return }
                finished = true
                if let parent = sheet.sheetParent {
                    parent.endSheet(sheet)
                } else {
                    NSApp.stopModal()
                    sheet.orderOut(nil)
                }
                continuation.resume(returning: result)
            }
            let host = NSHostingController(rootView: content(finish))
            host.sizingOptions = .preferredContentSize
            sheet.contentViewController = host
            if let parent {
                parent.beginSheet(sheet)
            } else {
                NSApp.runModal(for: sheet)
            }
        }
    }

    /// File > Print…: the pages sheet (`PrintDialog`), then the system print panel for the PDF of those pages.
    /// Pages are laid out for the sheet's paper and printed at their own size. The sheet saves its choices
    /// and the document prints with them, the way the system's own print action does
    /// (`SchematicNSDocument.printOperation(withSettings:)`).
    func printPages() async {
        guard let options = await runPrintOptions(PrintOptions.saved(), action: .print) else { return }
        options.save()
        if let systemDocument {
            // The pages chosen in the sheet are not among what is saved (a
            // board's layers differ): they go to this one print.
            optionsForNextPrint = options
            systemDocument.print(withSettings: [:], showPrintPanel: true, delegate: nil, didPrint: nil, contextInfo: nil)
            return
        }
        // With no system document (a window of its own): the operation, run here.
        do {
            let operation = try printOperation(options, jobTitle: displayName)
            if let window {
                operation.runModal(for: window, delegate: nil, didRun: nil, contextInfo: nil)
            } else {
                operation.run()
            }
        } catch {
            present(error, title: "Couldn't print “\(displayName)”")
        }
    }

    /// What the pages sheet chose, for the print it leads to; taken once.
    @ObservationIgnored
    var optionsForNextPrint: PrintOptions?

    /// The options the document prints with: the pages sheet's, for the
    /// print it started, else the saved ones (the system's print action,
    /// with no sheet).
    func takePrintOptions() -> PrintOptions {
        defer { optionsForNextPrint = nil }
        return optionsForNextPrint ?? PrintOptions.saved()
    }

    /// The print operation for the pages `options` choose: `PrintExport`'s PDF, each page at its own size.
    /// Throws when no page is chosen or none has anything to draw.
    func printOperation(_ options: PrintOptions, jobTitle: String) throws -> NSPrintOperation {
        guard let data = PrintExport.pdf(for: document, name: displayName, options),
              let pdf = PDFDocument(data: data),
              let operation = PrintExport.printOperation(for: pdf, paper: options.paper) else {
            throw ExportError.nothingToPrint
        }
        operation.jobTitle = jobTitle
        return operation
    }

    /// File > Export > PDF…: the pages sheet, then where to save, then the file.
    func exportPDF() async {
        guard let options = await runPrintOptions(PrintOptions.saved(), action: .savePDF) else { return }
        options.save()
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.pdf]
        panel.canCreateDirectories = true
        panel.nameFieldStringValue = "\(displayName).pdf"
        if let fileURL {
            panel.directoryURL = fileURL.deletingLastPathComponent()
        }
        guard await run(panel) == .OK, let url = panel.url else { return }
        do {
            try writePDF(options, to: url)
        } catch {
            present(error, title: "Couldn't export “\(displayName)”")
        }
    }

    /// Writes the PDF `options` describe to `url`. Throws when no page is chosen or the write fails.
    func writePDF(_ options: PrintOptions, to url: URL) throws {
        guard let data = PrintExport.pdf(for: document, name: displayName, options) else {
            throw ExportError.nothingToPrint
        }
        try data.write(to: url, options: .atomic)
    }

    private func runPrintOptions(_ options: PrintOptions, action: PrintAction) async -> PrintOptions? {
        await runSheet { finish in
            PrintDialog(document: document, name: displayName, action: action, options: options,
                        onCancel: { finish(nil) }, onDone: { finish($0) })
        }
    }

    /// Asks where to save the board's Gerber files, then writes them there
    /// as one zip (`GerberExport`). Like any export, not a save.
    func exportGerber() async {
        guard await confirmExportDespiteProblems() else { return }
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.zip]
        panel.canCreateDirectories = true
        panel.nameFieldStringValue = "\(displayName) Gerbers.zip"
        if let fileURL {
            panel.directoryURL = fileURL.deletingLastPathComponent()
        }
        guard await run(panel) == .OK, let url = panel.url else { return }
        do {
            try writeGerber(to: url)
        } catch {
            present(error, title: "Couldn't export “\(displayName)”")
        }
    }

    /// Runs the design rule check, under the board's own rules, before a
    /// Gerber export. A board with problems is made as drawn, so a short or
    /// an unrouted net would ship: the user is told what the check found and
    /// chooses. True when the board is clean or they export anyway.
    private func confirmExportDespiteProblems() async -> Bool {
        let issues = document.designRuleCheck(rules: document.board.rules)
        guard !issues.isEmpty else { return true }
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.messageText = "The board has \(issues.count) design rule \(issues.count == 1 ? "problem" : "problems")"
        alert.informativeText = Self.problemSummary(issues)
            + " A fab makes the board as drawn. Fix them first (Window ▸ Design Rule Check lists each), or export anyway."
        alert.addButton(withTitle: "Cancel")
        alert.addButton(withTitle: "Export Anyway")
        return await run(alert) == .alertSecondButtonReturn
    }

    /// "1 short circuit, 2 unrouted connections and 3 others." The serious
    /// kinds first; every problem is counted.
    static func problemSummary(_ issues: [DesignRuleIssue]) -> String {
        func count(_ kind: DesignRuleIssue.Kind) -> Int { issues.filter { $0.kind == kind }.count }
        let shorts = count(.short), unrouted = count(.unrouted)
        var parts: [String] = []
        if shorts > 0 { parts.append("\(shorts) short \(shorts == 1 ? "circuit" : "circuits")") }
        if unrouted > 0 { parts.append("\(unrouted) unrouted \(unrouted == 1 ? "connection" : "connections")") }
        let others = issues.count - shorts - unrouted
        if others > 0 { parts.append("\(others) other \(others == 1 ? "problem" : "problems")") }
        let text = parts.count > 1 ? parts.dropLast().joined(separator: ", ") + " and " + parts.last! : parts.joined()
        return "The check found " + text + "."
    }

    /// Writes the board's Gerber zip to `url`; its files are named after the
    /// schematic. Throws for a failed write.
    func writeGerber(to url: URL) throws {
        try GerberExport.zip(for: document, base: gerberBaseName).write(to: url, options: .atomic)
    }

    /// The schematic's name with anything but letters, digits, `-` and `_`
    /// as `_`, so every fab's upload takes the file names.
    var gerberBaseName: String {
        let allowed = CharacterSet.alphanumerics.union(CharacterSet(charactersIn: "-_"))
        let name = String(displayName.unicodeScalars.map { $0.isASCII && allowed.contains($0) ? Character($0) : "_" })
        return name.isEmpty ? "board" : name
    }

    enum ExportError: LocalizedError {
        case nothingToExport
        case nothingToPrint

        var errorDescription: String? {
            switch self {
            case .nothingToExport: "The schematic has no parts, wires or buses."
            case .nothingToPrint: "No pages are chosen, or none has anything to draw."
            }
        }
    }

    // MARK: Helpers

    private func present(_ error: Error, title: String) {
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.messageText = title
        alert.informativeText = error.localizedDescription
        Task { _ = await run(alert) }
    }

    /// Shows a panel as a sheet on this schematic's window, or on its own if
    /// the window isn't known.
    private func run(_ panel: NSSavePanel) async -> NSApplication.ModalResponse {
        if let window = window ?? NSApp.keyWindow {
            return await panel.beginSheetModal(for: window)
        }
        return panel.runModal()
    }

    private func run(_ alert: NSAlert) async -> NSApplication.ModalResponse {
        if let window = window ?? NSApp.keyWindow {
            return await alert.beginSheetModal(for: window)
        }
        return alert.runModal()
    }
}

// MARK: - Single part files (plan-32-A)

/// The single part file type (`.schpart`), declared in the app's Info.plist.
enum PartFileType {
    nonisolated static let fileExtension = "schpart"
    static let contentType = UTType(exportedAs: "jgz.schematical.part", conformingTo: .json)

    /// The declared type, plus whatever type the system gives a `.schpart`
    /// file before the declaration is registered (as `SchematicFile.openableTypes`).
    static var openableTypes: [UTType] {
        var types = [contentType]
        if let byExtension = UTType(filenameExtension: fileExtension), byExtension != contentType {
            types.append(byExtension)
        }
        return types
    }
}

/// File › Load Part…: adds a `.schpart` file's part to the catalog for the
/// rest of the session. A failure shows the reason; nothing changes.
@MainActor
enum PartFileLoading {
    /// The open panel for a part file, as a sheet on the key window; nil when
    /// cancelled.
    private static func choosePartFile(message: String) async -> URL? {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = PartFileType.openableTypes
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        panel.message = message
        let response: NSApplication.ModalResponse
        if let window = NSApp.keyWindow {
            response = await panel.beginSheetModal(for: window)
        } else {
            response = panel.runModal()
        }
        guard response == .OK else { return nil }
        return panel.url
    }

    private static func showLoadFailure(_ url: URL, _ error: Error, verb: String) async {
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.messageText = "Couldn't \(verb) “\(url.lastPathComponent)”"
        alert.informativeText = error.localizedDescription
        _ = await UserLibraryPrompts.run(alert)
    }

    /// The User Library window's Add Part…: chooses a part file and adds it
    /// to the User Library with no question (plan-53-F).
    static func addToLibrary(catalog: PartCatalogStore? = nil) async {
        let catalog = catalog ?? PartCatalogStore.shared
        guard let url = await choosePartFile(message: "Choose a part file to add to your User Library") else { return }
        do {
            let text = try PartFileReader.read(url: url)
            try catalog.addToUserLibrary(fileText: text, source: url.lastPathComponent)
        } catch {
            await showLoadFailure(url, error, verb: "add")
        }
    }

    static func loadPart(catalog: PartCatalogStore? = nil) async {
        let catalog = catalog ?? PartCatalogStore.shared
        guard let url = await choosePartFile(message: "Choose a part file to load") else { return }
        do {
            let text = try PartFileReader.read(url: url)
            let outcome = try await load(text: text, source: url.lastPathComponent, catalog: catalog) { name in
                switch await UserLibraryPrompts.run(UserLibraryPrompts.loadPartAlert(name: name)) {
                case .alertFirstButtonReturn: .add
                case .alertSecondButtonReturn: .useOnce
                default: .cancel
                }
            }
            if case let .alreadyInLibrary(part) = outcome {
                _ = await UserLibraryPrompts.run(UserLibraryPrompts.alreadyInLibraryAlert(name: part.name))
            }
        } catch {
            await showLoadFailure(url, error, verb: "load")
        }
    }
}

/// The File menu's part file items, after Open….
struct PartFileMenuItems: View {
    @Environment(\.openWindow) private var openWindow

    var body: some View {
        Button("Load Part…") {
            Task { await PartFileLoading.loadPart() }
        }
        Button("User Library…") {
            openWindow(id: UserLibraryScene.id)
        }
    }
}

// MARK: - Menu commands

extension FocusedValues {
    /// The focused window's exporter, for the File menu.
    @Entry var schematicExporter: SchematicExporter?
}

/// The File menu's items that are the app's own, around the document items
/// (`DocumentMenuCommands`): the part items after Open Recent, then Export
/// and Print… after the document's Save group. In a Part Editor window
/// (plan-32-C) ⌘O and ⌘S are that window's Open Part… and Save Part….
struct SchematicFileCommands: Commands {
    @FocusedValue(\.schematicExporter) private var file
    @FocusedValue(\.partEditor) private var editor

    var body: some Commands {
        CommandGroup(after: .newItem) {
            PartEditorCommands(editor: editor)

            if let editor {
                Button("Open Part…") {
                    Task { await editor.open() }
                }
                .keyboardShortcut("o", modifiers: .command)
            }

            PartFileMenuItems()
        }

        CommandGroup(after: .saveItem) {
            if let editor {
                Button("Save Part…") {
                    Task { await editor.save() }
                }
                .keyboardShortcut("s", modifiers: .command)
                .disabled(!editor.isValid)
            }

            Divider()

            SchematicImportMenu(file: file)

            SchematicExportMenu(file: file)

            Divider()

            // The app's own Print: the system's is removed (`SimulationCommands`).
            Button("Print…") {
                Task { await file?.printPages() }
            }
            .keyboardShortcut("p", modifiers: .command)
            .disabled(file == nil)
        }
    }
}
