//
//  SchematicNSDocument.swift
//  schematical — plan-59 spike, candidate H
//
//  The document is an NSDocument subclass. Its window hosts the SwiftUI
//  editor in an NSHostingController; AppKit does the file coordination,
//  autosave, versions, recents, close and quit prompts, tabs and restoration.
//

import AppKit
import PDFKit
import QuartzCore
import SwiftUI

@objc(SchematicNSDocument)
final class SchematicNSDocument: NSDocument {
    /// What the editor works on: the document, its simulation, the undo
    /// bridge and the exporter (the same pieces candidate G's model holds;
    /// its ReferenceFileDocument half is unused here).
    let model = SchematicDocumentModel()
    /// Where the write in progress ends up (`writeSafely` hands `data(ofType:)`
    /// a temporary file).
    private var destination: URL?

    nonisolated override class var autosavesInPlace: Bool { true }

    override init() {
        super.init()
        model.connectUndo(undoManager)
        model.openChild = { [weak self] kind, chip in self?.openChild(kind, chip) }
        SpikeLog.log("H document init main=\(Thread.isMainThread) sameUndoManager=\(model.undoManager === undoManager)")
    }

    /// A file in AppKit's autosave folder belongs to a document with no place
    /// of its own yet: its load-file paths are absolute, as for any unsaved
    /// schematic.
    nonisolated private static func isAutosaveLocation(_ url: URL) -> Bool {
        url.path.contains("/Autosave Information/")
    }

    nonisolated override var fileURL: URL? {
        get { super.fileURL }
        set {
            super.fileURL = newValue
            let model = model
            if Thread.isMainThread {
                MainActor.assumeIsolated { model.attach(fileURL: newValue) }
            } else {
                DispatchQueue.main.async { model.attach(fileURL: newValue) }
            }
        }
    }

    // MARK: Reading and writing

    // AppKit calls these on the main thread: the class does not opt in to
    // concurrent reading or asynchronous writing.
    nonisolated override func read(from url: URL, ofType typeName: String) throws {
        let base = Self.isAutosaveLocation(url) ? nil : url
        SpikeLog.log("H read main=\(Thread.isMainThread) url=\(url.path) base=\(base?.path ?? "nil")")
        let data = try Data(contentsOf: url)
        try MainActor.assumeIsolated {
            try model.load(data, fileURL: base)
            undoManager?.removeAllActions()
        }
    }

    nonisolated override func writeSafely(to url: URL, ofType typeName: String, for saveOperation: NSDocument.SaveOperationType) throws {
        MainActor.assumeIsolated { destination = url }
        defer { MainActor.assumeIsolated { destination = nil } }
        try super.writeSafely(to: url, ofType: typeName, for: saveOperation)
    }

    nonisolated override func data(ofType typeName: String) throws -> Data {
        try MainActor.assumeIsolated { try snapshotData() }
    }

    private func snapshotData() throws -> Data {
        let target = destination ?? fileURL
        let base = target.flatMap { Self.isAutosaveLocation($0) ? nil : $0 }
        model.undoBridge?.endCoalescing()
        let start = CACurrentMediaTime()
        let file = SchematicFile(document: model.document, fileURL: base)
        SpikeLog.log(String(format: "H snapshot main=%@ ms=%.3f destination=%@ memoryPaths=%@",
                            Thread.isMainThread ? "true" : "false", (CACurrentMediaTime() - start) * 1000,
                            base?.path ?? "nil", file.parts.compactMap { $0.memoryFile?.path }.description))
        return try file.encoded()
    }

    // MARK: Windows

    private func makeHost() -> NSHostingController<AnyView> {
        let view = ContentView(model: model, fileURL: fileURL)
            .modifier(DocumentWiring(model: model, fileURL: fileURL))
        let host = NSHostingController(rootView: AnyView(view))
        host.sceneBridgingOptions = [.toolbars]
        return host
    }

    override func makeWindowControllers() {
        let window = NSWindow(contentViewController: makeHost())
        window.setContentSize(NSSize(width: 1100, height: 700))
        window.toolbarStyle = .unifiedCompact
        window.titleVisibility = .hidden
        window.isRestorable = true
        let controller = NSWindowController(window: window)
        controller.shouldCascadeWindows = true
        addWindowController(controller)
        // Opening a file replaces a blank, untouched Untitled document (probe A8):
        // AppKit leaves that to the app.
        if fileURL != nil {
            for other in NSDocumentController.shared.documents where other !== self {
                guard let blank = other as? SchematicNSDocument, blank.fileURL == nil, !blank.isDocumentEdited,
                      blank.autosavedContentsFileURL == nil, blank.model.document.chips.isEmpty else { continue }
                SpikeLog.log("H closing the blank Untitled document: a file was opened")
                blank.close()
            }
        }
    }

    override func close() {
        model.simulation.stop()
        ChildWindowCenter.shared.closeAll(model.id)
        SpikeLog.log("H document close: simulation stopped, child windows closed")
        let windows = windowControllers.compactMap(\.window)
        // Empty the hosted tree first, while the window is still up: that
        // withdraws the focused value, which otherwise keeps the model (the
        // trace showed SwiftUI's focusedValues holding it after the close).
        for window in windows {
            (window.contentViewController as? NSHostingController<AnyView>)?.rootView = AnyView(EmptyView())
        }
        super.close()
        // Take the SwiftUI tree out of the closed window: left in, its state
        // (and so the document's objects) stayed alive (probe A6).
        windows.forEach { $0.contentViewController = nil }
    }

    // MARK: Restoration (the page a window shows)

    override func encodeRestorableState(with coder: NSCoder) {
        super.encodeRestorableState(with: coder)
        coder.encode(model.page.rawValue, forKey: "editorPage")
    }

    override func restoreState(with coder: NSCoder) {
        super.restoreState(with: coder)
        guard let raw = coder.decodeObject(of: NSString.self, forKey: "editorPage") as String?,
              let page = EditorPage(rawValue: raw), page != model.page else { return }
        model.page = page
        // The editor's page is view state: a fresh hosting controller starts on the restored page.
        windowControllers.first?.window?.contentViewController = makeHost()
        SpikeLog.log("H restored page \(page.rawValue)")
    }

    // MARK: Print (File > Print…, printDocument:)

    override func printOperation(withSettings printSettings: [NSPrintInfo.AttributeKey: Any]) throws -> NSPrintOperation {
        let options = PrintOptions.saved()
        guard let data = PrintExport.pdf(for: model.document, name: displayName, options),
              let pdf = PDFDocument(data: data),
              let operation = PrintExport.printOperation(for: pdf, paper: options.paper) else {
            throw SchematicFileController.ExportError.nothingToPrint
        }
        operation.jobTitle = displayName
        return operation
    }

    // MARK: Child windows

    private func openChild(_ kind: String, _ chip: UUID?) {
        let open = ChildWindowOpener(model: model)
        let id = model.id
        switch (kind, chip) {
        case let ("memory", chip?): open(id: "memory", value: MemoryWindowTarget(documentID: id, chipID: chip))
        case let ("cpu", chip?): open(id: "cpu", value: CpuWindowTarget(documentID: id, chipID: chip))
        case let ("registers", chip?): open(id: "registers", value: RegisterWindowTarget(documentID: id, chipID: chip))
        case let ("terminal", chip?): open(id: "terminal", value: TerminalWindowTarget(documentID: id, chipID: chip))
        case ("bom", _): open(id: "bom", value: BillOfMaterialsTarget(documentID: id))
        case ("drc", _): open(id: "drc", value: DesignRuleCheckTarget(documentID: id))
        case ("preview", _): open(id: "board-preview", value: BoardPreviewTarget(documentID: id))
        default: SpikeLog.log("openChild: nothing to open for \(kind)")
        }
    }
}

// MARK: - Child windows without scenes

/// The seven child windows as AppKit windows hosting their SwiftUI views
/// (a hosted editor has no scene to `openWindow` from). Keyed by the
/// document's id and the window's target; closed with the document.
@MainActor
final class ChildWindowCenter {
    static let shared = ChildWindowCenter()
    private var windows: [UUID: [AnyHashable: NSWindow]] = [:]

    func open<Target: Hashable, Content: View>(_ target: Target, document id: UUID, title: String, size: NSSize,
                                                @ViewBuilder content: () -> Content) {
        if let window = windows[id]?[AnyHashable(target)] {
            window.makeKeyAndOrderFront(nil)
            return
        }
        let host = NSHostingController(rootView: content())
        host.sceneBridgingOptions = [.toolbars]
        let window = NSWindow(contentViewController: host)
        window.title = title
        window.setContentSize(size)
        window.isReleasedWhenClosed = false
        window.isRestorable = false
        windows[id, default: [:]][AnyHashable(target)] = window
        window.makeKeyAndOrderFront(nil)
    }

    func close<Target: Hashable>(_ target: Target, document id: UUID) {
        windows[id]?.removeValue(forKey: AnyHashable(target))?.close()
    }

    func closeAll(_ id: UUID) {
        let closing = windows.removeValue(forKey: id) ?? [:]
        closing.values.forEach { $0.close() }
        SpikeLog.log("H child windows closed: \(closing.count) of document \(id.uuidString.prefix(8))")
    }
}

/// Stands where `@Environment(\.openWindow)` stood in `ContentView`: the same
/// call syntax, opening through `ChildWindowCenter`.
@MainActor
struct ChildWindowOpener {
    let model: SchematicDocumentModel

    func callAsFunction<Value: Hashable>(id: String, value: Value) {
        let center = ChildWindowCenter.shared
        let document = model.id
        switch value {
        case let target as MemoryWindowTarget:
            center.open(target, document: document, title: "Memory", size: NSSize(width: 760, height: 560)) { MemoryWindowView(target: target) }
        case let target as CpuWindowTarget:
            center.open(target, document: document, title: "Processor", size: NSSize(width: 420, height: 360)) { CpuWindowView(target: target) }
        case let target as RegisterWindowTarget:
            center.open(target, document: document, title: "Registers", size: NSSize(width: 440, height: 420)) { RegisterWindowView(target: target) }
        case let target as TerminalWindowTarget:
            center.open(target, document: document, title: "Terminal", size: NSSize(width: 640, height: 420)) { TerminalWindowView(target: target) }
        case let target as BillOfMaterialsTarget:
            center.open(target, document: document, title: "Bill of Materials", size: NSSize(width: 760, height: 420)) { BillOfMaterialsWindowView(target: target) }
        case let target as DesignRuleCheckTarget:
            center.open(target, document: document, title: "Design Rule Check", size: NSSize(width: 720, height: 420)) { DesignRuleCheckWindowView(target: target) }
        case let target as BoardPreviewTarget:
            center.open(target, document: document, title: "Board Preview", size: NSSize(width: 1000, height: 720)) { BoardPreviewWindowView(target: target) }
        default:
            SpikeLog.log("no AppKit child window for scene \(id)")
        }
    }
}

/// Stands where `@Environment(\.dismissWindow)` stood in `ContentView`.
@MainActor
struct ChildWindowCloser {
    let model: SchematicDocumentModel

    func callAsFunction<Value: Hashable>(id: String, value: Value) {
        ChildWindowCenter.shared.close(value, document: model.id)
    }
}

// MARK: - The File menu's document items

/// New, Open…, Open Recent, Save, Duplicate, Rename…, Move To… and Revert:
/// SwiftUI builds the main menu and adds these only for a DocumentGroup, so
/// H declares them. Each sends the standard action down the responder chain,
/// where NSDocument and NSDocumentController answer it.
struct DocumentMenuCommands: Commands {
    @FocusedValue(\.schematicModel) private var model

    private func send(_ selector: String) {
        NSApp.sendAction(Selector(selector), to: nil, from: nil)
    }

    var body: some Commands {
        CommandGroup(replacing: .newItem) {
            Button("New") { NSDocumentController.shared.newDocument(nil) }
                .keyboardShortcut("n", modifiers: .command)
            Button("Open…") { NSDocumentController.shared.openDocument(nil) }
                .keyboardShortcut("o", modifiers: .command)
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
            Button("Close All") { NSDocumentController.shared.closeAllDocuments(withDelegate: nil, didCloseAllSelector: nil, contextInfo: nil) }
                .keyboardShortcut("w", modifiers: [.command, .option])
            Button("Save") { send("saveDocument:") }
                .keyboardShortcut("s", modifiers: .command)
                .disabled(model == nil)
            Button("Duplicate") { send("duplicateDocument:") }
                .keyboardShortcut("s", modifiers: [.command, .shift])
                .disabled(model == nil)
            Button("Save As…") { send("saveDocumentAs:") }
                .keyboardShortcut("s", modifiers: [.command, .shift, .option])
                .disabled(model == nil)
            Button("Rename…") { send("renameDocument:") }
                .disabled(model == nil)
            Button("Move To…") { send("moveDocument:") }
                .disabled(model == nil)
            Menu("Revert To") {
                Button("Last Saved") { send("revertDocumentToSaved:") }
                Button("Browse All Versions…") { send("browseDocumentVersions:") }
            }
            .disabled(model == nil)
            let _ = SpikeLog.log("commands evaluated: focused document \(model.map { String($0.id.uuidString.prefix(8)) } ?? "none")")
        }
    }
}
