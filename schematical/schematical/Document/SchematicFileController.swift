import AppKit
import Foundation
import Observation
import SwiftUI

/// Save and Open for one window. Tracks which file the window's schematic came
/// from and whether it has changed since it was last saved or opened.
@MainActor
@Observable
final class SchematicFileController {
    /// The file the schematic was last saved to or opened from.
    private(set) var fileURL: URL?

    /// The window showing this schematic. Sheets attach to it, and closing it
    /// asks about unsaved changes first.
    @ObservationIgnored
    weak var window: NSWindow? {
        didSet {
            closeGuard.attach(to: window)
            if oldValue == nil { claimPendingOpen() }
        }
    }

    /// Opens a new, blank schematic window. Set by the window's view, which
    /// has SwiftUI's `openWindow`.
    @ObservationIgnored
    var openNewWindow: (() -> Void)?

    /// A file read by one window to open in a new tab: the next new window
    /// takes it.
    private struct PendingOpen {
        let url: URL
        let file: SchematicFile
        /// The window whose tab group the new window joins.
        weak var tabbedWith: NSWindow?
    }

    @ObservationIgnored
    private static var pendingOpens: [PendingOpen] = []

    /// The file this new window claimed, loaded once the catalog is.
    @ObservationIgnored
    private var claimedOpen: PendingOpen?

    /// Every open window's controller, so quitting can check them all.
    @ObservationIgnored
    private static let all = NSHashTable<SchematicFileController>.weakObjects()

    @ObservationIgnored
    private lazy var closeGuard = WindowCloseGuard { [weak self] window in
        guard let self else { return true }
        guard self.hasUnsavedChanges else { return true }
        Task {
            if await self.confirmDiscardingChanges() {
                window.close()
            }
        }
        return false
    }

    private let document: SchematicDocument
    private let catalog: PartCatalogStore
    /// Called before an opened file replaces the canvas, for example to stop
    /// a running simulation.
    private let willReplaceDocument: () -> Void

    /// The document as it was last saved or opened. A new window starts empty,
    /// which also counts as saved.
    private var savedSnapshot: SchematicFile

    init(document: SchematicDocument, catalog: PartCatalogStore, willReplaceDocument: @escaping () -> Void) {
        self.document = document
        self.catalog = catalog
        self.willReplaceDocument = willReplaceDocument
        self.savedSnapshot = SchematicFile(document: document, fileURL: nil)
        Self.all.add(self)
    }

    /// Asks about each open window with unsaved changes, one at a time.
    /// Returns false as soon as the user cancels one. Closed windows are
    /// skipped: closing the last window quits the app, and that window was
    /// already asked about.
    static func confirmQuit() async -> Bool {
        for controller in all.allObjects where controller.hasUnsavedChanges {
            guard let window = controller.window, window.isVisible else { continue }
            window.makeKeyAndOrderFront(nil)
            guard await controller.confirmDiscardingChanges() else { return false }
        }
        return true
    }

    var displayName: String {
        fileURL?.deletingPathExtension().lastPathComponent ?? "Untitled"
    }

    var hasUnsavedChanges: Bool {
        SchematicFile(document: document, fileURL: fileURL) != savedSnapshot
    }

    // MARK: Actions

    /// Saves to the current file, or asks where to save if there isn't one.
    func save() async {
        if let fileURL {
            write(to: fileURL)
        } else {
            await saveAs()
        }
    }

    /// Asks where to save, then saves there.
    func saveAs() async {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [SchematicFile.contentType]
        panel.canCreateDirectories = true
        panel.nameFieldStringValue = "\(displayName).\(SchematicFile.fileExtension)"
        if let fileURL {
            panel.directoryURL = fileURL.deletingLastPathComponent()
        }
        guard await run(panel) == .OK, let url = panel.url else { return }
        write(to: url)
    }

    /// True for a new window's untitled schematic that is still empty.
    var isBlank: Bool {
        fileURL == nil && !hasUnsavedChanges
    }

    /// Asks for a schematic and opens it. A blank untitled window loads it in
    /// place; any other window opens it in a new tab. A file that is already
    /// open brings its window to the front instead.
    func open() async {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = SchematicFile.openableTypes
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        guard await run(panel) == .OK, let url = panel.url else { return }

        if let existing = Self.all.allObjects.first(where: { $0.fileURL?.standardizedFileURL == url.standardizedFileURL }),
           let window = existing.window, window.isVisible {
            window.makeKeyAndOrderFront(nil)
            return
        }

        let file: SchematicFile
        do {
            file = try SchematicFile.decode(try Data(contentsOf: url))
        } catch {
            present(error, title: "Couldn't open “\(url.lastPathComponent)”")
            return
        }

        if isBlank || openNewWindow == nil {
            guard await confirmDiscardingChanges() else { return }
            load(file, from: url)
        } else {
            Self.pendingOpens.append(PendingOpen(url: url, file: file, tabbedWith: window))
            openNewWindow?()
        }
    }

    /// A new window takes the next file another window read for it, and
    /// joins that window's tabs right away, before it is first shown, so it
    /// never appears as a window of its own.
    private func claimPendingOpen() {
        guard let window, isBlank, !Self.pendingOpens.isEmpty else { return }
        let pending = Self.pendingOpens.removeFirst()
        claimedOpen = pending
        if let group = pending.tabbedWith, group !== window {
            window.tabbingMode = .preferred
            group.addTabbedWindow(window, ordered: .above)
            window.makeKeyAndOrderFront(nil)
        }
    }

    /// Loads the file this window claimed, if any. Call once the catalog is
    /// loaded.
    func loadClaimedOpen() {
        guard let pending = claimedOpen else { return }
        claimedOpen = nil
        load(pending.file, from: pending.url)
    }

    private func load(_ file: SchematicFile, from url: URL) {
        do {
            willReplaceDocument()
            try file.apply(to: document, catalog: catalog, fileURL: url)
            fileURL = url
            // Taken after apply, so a refreshed bookmark is not a change.
            savedSnapshot = SchematicFile(document: document, fileURL: url)
        } catch {
            present(error, title: "Couldn't open “\(url.lastPathComponent)”")
        }
    }

    /// Asks where to export the schematic as `format`, then writes it there.
    /// An export is not a save: the window's file and saved state are unchanged.
    func export(_ format: SchematicExportFormat) async {
        guard !SchematicScene(document).isEmpty else {
            let alert = NSAlert()
            alert.messageText = "Nothing to export"
            alert.informativeText = "Add a part, wire or bus first."
            _ = await run(alert)
            return
        }
        let panel = NSSavePanel()
        panel.allowedContentTypes = [format.contentType]
        panel.canCreateDirectories = true
        panel.nameFieldStringValue = "\(displayName).\(format.fileExtension)"
        if let fileURL {
            panel.directoryURL = fileURL.deletingLastPathComponent()
        }
        guard await run(panel) == .OK, let url = panel.url else { return }
        do {
            try writeExport(format, to: url)
        } catch {
            present(error, title: "Couldn't export “\(displayName)”")
        }
    }

    /// Writes the schematic as `format` to `url`. Throws for an empty
    /// schematic or a failed write.
    func writeExport(_ format: SchematicExportFormat, to url: URL) throws {
        guard let data = format.data(for: SchematicScene(document)) else {
            throw ExportError.nothingToExport
        }
        try data.write(to: url, options: .atomic)
    }

    enum ExportError: LocalizedError {
        case nothingToExport

        var errorDescription: String? {
            "The schematic has no parts, wires or buses."
        }
    }

    // MARK: Helpers

    private func write(to url: URL) {
        let snapshot = SchematicFile(document: document, fileURL: url)
        do {
            try snapshot.encoded().write(to: url, options: .atomic)
            fileURL = url
            savedSnapshot = snapshot
        } catch {
            present(error, title: "Couldn't save “\(url.lastPathComponent)”")
        }
    }

    /// True if it's fine to throw away the current canvas: it has no unsaved
    /// changes, the user saved it, or the user chose not to.
    func confirmDiscardingChanges() async -> Bool {
        guard hasUnsavedChanges else { return true }

        let alert = NSAlert()
        alert.messageText = "Do you want to save the changes to “\(displayName)”?"
        alert.informativeText = "Your changes will be lost if you don't save them."
        alert.addButton(withTitle: "Save")
        alert.addButton(withTitle: "Cancel")
        alert.addButton(withTitle: "Don't Save")

        switch await run(alert) {
        case .alertFirstButtonReturn:
            await save()
            return !hasUnsavedChanges
        case .alertThirdButtonReturn:
            return true
        default:
            return false
        }
    }

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

// MARK: - Menu commands

extension FocusedValues {
    /// The focused window's Save and Open handler, for the File menu.
    @Entry var schematicFile: SchematicFileController?
}

/// File menu items for the focused window's schematic.
struct SchematicFileCommands: Commands {
    @FocusedValue(\.schematicFile) private var file

    var body: some Commands {
        CommandGroup(after: .newItem) {
            Button("Open…") {
                Task { await file?.open() }
            }
            .keyboardShortcut("o", modifiers: .command)
            .disabled(file == nil)

            // Here rather than in a `.saveItem` group: the File menu of a
            // WindowGroup app never showed that group (only New, Open… and
            // Close appeared, and ⌘S did nothing).
            Divider()

            Button("Save") {
                Task { await file?.save() }
            }
            .keyboardShortcut("s", modifiers: .command)
            .disabled(file == nil)

            Button("Save As…") {
                Task { await file?.saveAs() }
            }
            .keyboardShortcut("s", modifiers: [.command, .shift])
            .disabled(file == nil)

            Divider()

            SchematicExportMenu(file: file)
        }
    }
}

// MARK: - Window close

/// Sits in front of SwiftUI's own window delegate so it can answer
/// `windowShouldClose`. Every other delegate message is passed straight through
/// to SwiftUI's delegate.
final class WindowCloseGuard: NSObject, NSWindowDelegate {
    private let shouldClose: (NSWindow) -> Bool
    private weak var original: NSWindowDelegate?
    private weak var window: NSWindow?

    init(shouldClose: @escaping (NSWindow) -> Bool) {
        self.shouldClose = shouldClose
    }

    func attach(to newWindow: NSWindow?) {
        guard newWindow !== window else { return }
        if let window, window.delegate === self {
            window.delegate = original
        }
        window = newWindow
        original = newWindow?.delegate
        newWindow?.delegate = self
    }

    func windowShouldClose(_ sender: NSWindow) -> Bool {
        if original?.windowShouldClose?(sender) == false {
            return false
        }
        return shouldClose(sender)
    }

    override func responds(to selector: Selector!) -> Bool {
        super.responds(to: selector) || (original?.responds(to: selector) ?? false)
    }

    override func forwardingTarget(for selector: Selector!) -> Any? {
        original?.responds(to: selector) == true ? original : super.forwardingTarget(for: selector)
    }
}

/// Reports the window a view is in, once it's added to one.
struct WindowReader: NSViewRepresentable {
    let onWindow: (NSWindow) -> Void

    func makeNSView(context: Context) -> NSView {
        ReaderView(onWindow: onWindow)
    }

    func updateNSView(_ nsView: NSView, context: Context) {}

    private final class ReaderView: NSView {
        let onWindow: (NSWindow) -> Void

        init(onWindow: @escaping (NSWindow) -> Void) {
            self.onWindow = onWindow
            super.init(frame: .zero)
        }

        required init?(coder: NSCoder) { nil }

        override func viewDidMoveToWindow() {
            super.viewDidMoveToWindow()
            if let window { onWindow(window) }
        }
    }
}
