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

    /// The page the editor shows.
    var page = EditorPage.schematic

    /// A new, blank schematic with the standard net classes (Power: 5V, 3V3,
    /// 12V, 24V; and Logic). An opened file replaces them with its own.
    init(catalog: PartCatalogStore = .shared) {
        let document = SchematicDocument(library: catalog.library)
        document.addStandardNetClasses()
        let simulation = SimulationController(library: catalog.library)
        self.document = document
        self.simulation = simulation
        // Replacing the canvas stops any running simulation first, since
        // the simulation was built from the old schematic.
        self.file = SchematicFileController(document: document, catalog: catalog,
                                            willReplaceDocument: { [weak simulation] in simulation?.stop() })
    }

    /// The document's file, nil for one never saved. Set by the window's
    /// `SchematicNSDocument`; the exporter names its files after it.
    var fileURL: URL? {
        get { file.fileURL }
        set { file.fileURL = newValue }
    }
}
