import AppKit
import SwiftUI

/// A `.schematical` file as a system document (plan-61-B). AppKit reads and
/// writes it, autosaves it in place, keeps its versions and recents, and asks
/// about it at close and quit; this class turns that into the app's
/// `SchematicFile` and owns what the editor works on.
@objc(SchematicNSDocument)
final class SchematicNSDocument: NSDocument {
    /// What the editor works on: the document, its simulation, the exporter.
    let model: SchematicEditorModel

    /// Every open document whose schematic places the custom part `uuid` (of
    /// `catalog`) switches to `newPart`: its simulation is stopped first, then
    /// `SchematicDocument.updatePart` swaps the chips. Returns how many
    /// documents changed.
    @discardableResult
    static func updateOpenDocuments(uuid: String, to newPart: CatalogPart, catalog: PartCatalogStore) -> Int {
        var changed = 0
        for case let open as SchematicNSDocument in NSDocumentController.shared.documents
        where open.model.catalog === catalog {
            let document = open.model.document
            guard document.chips.contains(where: { $0.part.uuid == uuid }) else { continue }
            open.model.simulation.stop()
            if document.updatePart(uuid: uuid, to: newPart) { changed += 1 }
        }
        return changed
    }

    /// Where the write in progress ends up (`writeSafely` hands
    /// `data(ofType:)` no URL, and writes to a temporary file first).
    private var destination: URL?

    nonisolated override class var autosavesInPlace: Bool { true }

    override init() {
        model = SchematicEditorModel()
        super.init()
        connect()
    }

    /// A document around an existing model (tests).
    init(model: SchematicEditorModel) {
        self.model = model
        super.init()
        connect()
    }

    private func connect() {
        // One undo manager per document (plan-61-A): AppKit's Undo and Redo
        // items, and the Edited state, follow the document's own.
        undoManager = model.document.undoManager
        model.platformDocument = self
    }

    /// A file in AppKit's autosave folder belongs to a document with no place
    /// of its own yet: its load-file paths are absolute, as for any unsaved
    /// schematic.
    nonisolated static func isAutosaveLocation(_ url: URL) -> Bool {
        url.path.contains("/Autosave Information/")
    }

    /// The URL load-file paths are relative to, for a file at `url`.
    nonisolated static func relativeBase(_ url: URL?) -> URL? {
        guard let url, !isAutosaveLocation(url) else { return nil }
        return url
    }

    nonisolated override var fileURL: URL? {
        get { super.fileURL }
        set {
            super.fileURL = newValue
            // AppKit sets this on the main thread (the class reads and
            // writes on it); the model's copy is what the views observe.
            if Thread.isMainThread {
                MainActor.assumeIsolated { model.fileURL = newValue }
            } else {
                DispatchQueue.main.async { [weak self] in self?.model.fileURL = newValue }
            }
        }
    }

    // MARK: Reading and writing

    // AppKit calls these on the main thread: the class does not opt in to
    // concurrent reading or asynchronous writing.

    nonisolated override func read(from url: URL, ofType typeName: String) throws {
        let data = try Data(contentsOf: url)
        try MainActor.assumeIsolated {
            try load(SchematicFile.decode(data), base: Self.relativeBase(url))
        }
    }

    /// Replaces the document's contents with `file` (an open, or Revert).
    /// The simulation was built from the old schematic, so it stops first;
    /// applying the file ends the undo history.
    private func load(_ file: SchematicFile, base: URL?) throws {
        model.simulation.stop()
        let catalog = PartCatalogStore.shared
        let embedded = try file.apply(to: model.document, catalog: catalog, fileURL: base)
        // Custom parts that came with the file and are not in the User
        // Library: offer to add them (plan-53-C), once the file is in a
        // window. The open is already done.
        pendingOffer = UserLibraryPrompts.offerable(embedded, catalog: catalog)
        if !windowControllers.isEmpty { offerPendingParts() }
    }

    /// The custom parts of the file just read that the User Library lacks.
    private var pendingOffer: [CatalogPart] = []

    private func offerPendingParts() {
        let offered = pendingOffer
        pendingOffer = []
        guard !offered.isEmpty else { return }
        Task { [weak self] in await self?.model.file.offerToAdd(offered) }
    }

    nonisolated override func writeSafely(to url: URL, ofType typeName: String,
                                          for saveOperation: NSDocument.SaveOperationType) throws {
        MainActor.assumeIsolated { destination = url }
        defer { MainActor.assumeIsolated { destination = nil } }
        try super.writeSafely(to: url, ofType: typeName, for: saveOperation)
    }

    nonisolated override func data(ofType typeName: String) throws -> Data {
        try MainActor.assumeIsolated { try snapshot().encoded() }
    }

    /// The file a write stores: load-file paths relative to where the write
    /// ends up. A save also ends a typing step, so undo never splits one.
    private func snapshot() -> SchematicFile {
        model.document.willSave()
        return SchematicFile(document: model.document, fileURL: Self.relativeBase(destination ?? fileURL))
    }
}

// MARK: - The window

extension SchematicNSDocument {
    /// The document's window hosts the SwiftUI editor. `.toolbar` content
    /// reaches the window through the hosting controller's scene bridging.
    override func makeWindowControllers() {
        let host = NSHostingController(rootView: AnyView(ContentView(model: model)))
        host.sceneBridgingOptions = [.toolbars]
        let window = NSWindow(contentViewController: host)
        window.setContentSize(NSSize(width: 1100, height: 700))
        window.toolbarStyle = .unifiedCompact
        window.titleVisibility = .hidden
        window.isRestorable = true
        window.setFrameAutosaveName("SchematicDocument")
        let controller = NSWindowController(window: window)
        controller.shouldCascadeWindows = true
        addWindowController(controller)
        model.file.window = window
        offerPendingParts()
    }

    /// Closing a document stops its simulation and closes the windows it
    /// opened (Memory, Bill of Materials, …).
    override func close() {
        model.simulation.stop()
        ChildWindowCenter.shared.closeAll(model.id)
        super.close()
    }
}

// MARK: - The File menu's document items

/// New, Open…, Open Recent, Close, Close All, Save, Duplicate, Save As…,
/// Rename…, Move To… and Revert To. SwiftUI builds the main menu and adds
/// these only for a `DocumentGroup`, so the app declares them. Each sends the
/// standard action down the responder chain, where `NSDocument` and
/// `NSDocumentController` answer it.
struct DocumentMenuCommands: Commands {
    @FocusedValue(\.schematicFile) private var file
    @FocusedValue(\.partEditor) private var editor

    private func send(_ selector: String) {
        NSApp.sendAction(Selector(selector), to: nil, from: nil)
    }

    /// ⌘O and ⌘S are the Part Editor's own in its window (`SchematicFileCommands`).
    private func key(_ character: KeyEquivalent) -> KeyboardShortcut? {
        editor == nil ? KeyboardShortcut(character, modifiers: .command) : nil
    }

    var body: some Commands {
        CommandGroup(replacing: .newItem) {
            Button("New") { NSDocumentController.shared.newDocument(nil) }
                .keyboardShortcut("n", modifiers: .command)
            Button("Open…") { NSDocumentController.shared.openDocument(nil) }
                .keyboardShortcut(key("o"))
            Menu("Open Recent") {
                ForEach(NSDocumentController.shared.recentDocumentURLs, id: \.self) { url in
                    Button(url.lastPathComponent) {
                        NSDocumentController.shared.openDocument(withContentsOf: url, display: true) { _, _, _ in }
                    }
                }
                Divider()
                Button("Clear Menu") { NSDocumentController.shared.clearRecentDocuments(nil) }
            }
        }
        CommandGroup(replacing: .saveItem) {
            Button("Close") { send("performClose:") }
                .keyboardShortcut("w", modifiers: .command)
            Button("Close All") {
                NSDocumentController.shared.closeAllDocuments(withDelegate: nil, didCloseAllSelector: nil, contextInfo: nil)
            }
            .keyboardShortcut("w", modifiers: [.command, .option])
            Button("Save") { send("saveDocument:") }
                .keyboardShortcut(key("s"))
                .disabled(file == nil)
            Button(DocumentMenuCommands.duplicateTitle) { send("duplicateDocument:") }
                .keyboardShortcut("s", modifiers: [.command, .shift])
                .disabled(file == nil)
            // Shown in place of Duplicate while Option is held (`OptionOnlyMenuItems`).
            Button(DocumentMenuCommands.saveAsTitle) { send("saveDocumentAs:") }
                .keyboardShortcut("s", modifiers: [.command, .shift, .option])
                .disabled(file == nil)
            Button("Rename…") { send("renameDocument:") }
                .disabled(file == nil)
            Button("Move To…") { send("moveDocument:") }
                .disabled(file == nil)
            Menu("Revert To") {
                Button("Last Saved") { send("revertDocumentToSaved:") }
                Button("Browse All Versions…") { send("browseDocumentVersions:") }
            }
            .disabled(file == nil)
        }
    }

    static let duplicateTitle = "Duplicate"
    static let saveAsTitle = "Save As…"
}
