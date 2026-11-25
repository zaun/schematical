import Foundation
import UniformTypeIdentifiers

/// A file format the app writes: its own, and the two project exports
/// (File › Export). Gives each its extension and content type.
enum SaveFormat: Int, CaseIterable {
    /// The app's own file (`SchematicFile`).
    case schematical
    /// A KiCad project (`KiCadExport`): `.kicad_pro`, `.kicad_sch`, `.kicad_pcb`.
    case kicad
    /// An EasyEDA Pro project (`EasyEDAProExport`): one `.epro` file.
    case easyEDAPro

    var fileExtension: String {
        switch self {
        case .schematical: SchematicFile.fileExtension
        case .kicad: "kicad_pro"
        case .easyEDAPro: EasyEDAProExport.fileExtension
        }
    }

    var contentType: UTType {
        switch self {
        case .schematical: SchematicFile.contentType
        case .kicad: UTType(filenameExtension: "kicad_pro", conformingTo: .json) ?? .json
        case .easyEDAPro: UTType(filenameExtension: EasyEDAProExport.fileExtension, conformingTo: .zip) ?? .zip
        }
    }
}
