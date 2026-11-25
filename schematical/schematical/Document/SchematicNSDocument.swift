import AppKit
import SwiftUI

/// A `.schematical` file as a system document (plan-61-B). AppKit reads and
/// writes it, autosaves it in place, keeps its versions and recents, and asks
/// about it at close and quit; this class turns that into the app's
/// `SchematicFile` and owns what the editor works on.
@objc(SchematicNSDocument)
final class SchematicNSDocument: NSDocument {
    /// What the editor works on: the document, its simulation, the exporter.
    let model = SchematicEditorModel()

    /// Where the write in progress ends up (`writeSafely` hands
    /// `data(ofType:)` no URL, and writes to a temporary file first).
    private var destination: URL?

    nonisolated override class var autosavesInPlace: Bool { true }

    override init() {
        super.init()
        // One undo manager per document (plan-61-A): AppKit's Undo and Redo
        // items, and the Edited state, follow the document's own.
        undoManager = model.document.undoManager
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
        let offered = UserLibraryPrompts.offerable(embedded, catalog: catalog)
        if !offered.isEmpty {
            Task { [weak self] in
                guard let self, !self.windowControllers.isEmpty else { return }
                await self.model.file.offerToAdd(offered)
            }
        }
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
