import AppKit
import UniformTypeIdentifiers

/// What the Save As panel writes.
enum SaveFormat: Int, CaseIterable {
    /// The app's own file (`SchematicFile`).
    case schematical
    /// A KiCad project (`KiCadExport`): `.kicad_pro`, `.kicad_sch`, `.kicad_pcb`.
    case kicad

    var title: String {
        switch self {
        case .schematical: "Schematical"
        case .kicad: "KiCad Project"
        }
    }

    var fileExtension: String {
        switch self {
        case .schematical: SchematicFile.fileExtension
        case .kicad: "kicad_pro"
        }
    }

    var contentType: UTType {
        switch self {
        case .schematical: SchematicFile.contentType
        case .kicad: UTType(filenameExtension: "kicad_pro", conformingTo: .json) ?? .json
        }
    }

    /// Said under the menu when the format is chosen.
    var note: String {
        switch self {
        case .schematical: ""
        case .kicad: "Writes the .kicad_pro, .kicad_sch and .kicad_pcb files for KiCad 8 or later. This window's file is not changed."
        }
    }
}

/// The Format menu in a Save As panel's accessory area. It sets the panel's
/// file type and the extension of the name in it; `format` is the choice
/// when the panel closes.
@MainActor
final class SaveFormatPicker: NSObject {
    private let panel: NSSavePanel
    private let baseName: String
    private let popup = NSPopUpButton(frame: .zero, pullsDown: false)
    private let note = NSTextField(wrappingLabelWithString: "")

    private(set) var format: SaveFormat = .schematical

    /// Puts the menu in `panel`, set to Schematical with `baseName` as the
    /// file name.
    init(panel: NSSavePanel, baseName: String) {
        self.panel = panel
        self.baseName = baseName
        super.init()

        let label = NSTextField(labelWithString: "Format:")
        popup.addItems(withTitles: SaveFormat.allCases.map(\.title))
        popup.target = self
        popup.action = #selector(changed)
        note.font = .systemFont(ofSize: NSFont.smallSystemFontSize)
        note.textColor = .secondaryLabelColor
        note.preferredMaxLayoutWidth = 320

        let row = NSStackView(views: [label, popup])
        row.orientation = .horizontal
        row.spacing = 8
        let column = NSStackView(views: [row, note])
        column.orientation = .vertical
        column.alignment = .leading
        column.spacing = 6
        column.edgeInsets = NSEdgeInsets(top: 8, left: 16, bottom: 8, right: 16)
        panel.accessoryView = column
        apply(stem: baseName)
    }

    @objc private func changed() {
        let previous = format
        format = SaveFormat(rawValue: popup.indexOfSelectedItem) ?? .schematical
        // Only the old format's own extension comes off ("v1.2" stays "v1.2").
        var stem = panel.nameFieldStringValue
        if stem.hasSuffix("." + previous.fileExtension) { stem.removeLast(previous.fileExtension.count + 1) }
        apply(stem: stem)
    }

    /// The panel's type and name (`stem` and the extension) for the chosen format.
    private func apply(stem: String) {
        panel.allowedContentTypes = [format.contentType]
        panel.nameFieldStringValue = "\(stem).\(format.fileExtension)"
        note.stringValue = format.note
        note.isHidden = format.note.isEmpty
    }
}
