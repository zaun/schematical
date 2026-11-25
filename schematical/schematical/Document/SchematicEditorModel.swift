import AppKit
import Foundation
import Observation

/// What one schematic window edits (plan-61-B): the document, its simulation
/// and the exporter, with the file's URL and the page shown. The window's
/// `SchematicNSDocument` owns one; `ContentView` is built on it.
@MainActor
@Observable
final class SchematicEditorModel {
    /// Identifies this document to the child windows it opens and to the
    /// agent-control tools.
    @ObservationIgnored let id = UUID()
    @ObservationIgnored let document: SchematicDocument
    @ObservationIgnored let simulation: SimulationController
    /// Exports and print for this document.
    @ObservationIgnored let file: SchematicFileController
    /// The catalog the document's parts come from.
    @ObservationIgnored let catalog: PartCatalogStore
    /// The system document this is the model of.
    @ObservationIgnored weak var platformDocument: SchematicNSDocument?

    /// The page the editor shows.
    var page = EditorPage.schematic

    /// A new, blank schematic with the standard net classes (Power: 5V, 3V3,
    /// 12V, 24V; and Logic); an opened file replaces them with its own. Or,
    /// for a test, the model of an existing `document`.
    init(catalog: PartCatalogStore = .shared, document existing: SchematicDocument? = nil) {
        let document: SchematicDocument
        if let existing {
            document = existing
        } else {
            document = SchematicDocument(library: catalog.library)
            document.addStandardNetClasses()
        }
        self.catalog = catalog
        self.document = document
        self.simulation = SimulationController(library: catalog.library)
        self.file = SchematicFileController(document: document, catalog: catalog)
        // A User Library part edited in place reaches every open document
        // (plan-53-D).
        catalog.partUpdated = { [weak catalog] uuid, part in
            guard let catalog else { return }
            SchematicNSDocument.updateOpenDocuments(uuid: uuid, to: part, catalog: catalog)
        }
    }

    /// The document's file, nil for one never saved. Set by the window's
    /// `SchematicNSDocument`; the exporter names its files after it.
    var fileURL: URL? {
        get { file.fileURL }
        set { file.fileURL = newValue }
    }
}
