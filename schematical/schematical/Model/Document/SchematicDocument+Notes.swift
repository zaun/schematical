import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Text blocks (Schematic)

    /// Adds a text block with its top-left at `point` (snapped), selects it
    /// and asks the sidebar to edit its words.
    @discardableResult
    func addNote(at point: CGPoint) -> UUID {
        return edit(.schematic, "Add Text") { scope -> UUID in
            let note = sheet.updateNotes(scope) { $0.add(at: point) }
            select(.note(note.id))
            editingNoteID = note.id
            return note.id
        }
    }

    /// Changes a block's words. An empty string is refused and the words kept.
    func setNoteText(_ id: UUID, _ value: String) {
        edit(.schematic, "Edit Text", coalescing: "note-text:\(id)") { scope in
            sheet.updateNotes(scope) { $0.setText(id, value) }
        }
    }

    /// Sets a block's font size in points, clamped to `SchematicNote.sizeRange`.
    func setNoteSize(_ id: UUID, _ size: Int) {
        edit(.schematic, "Change Font Size") { scope in
            sheet.updateNotes(scope) { $0.setSize(id, size) }
        }
    }

    func setNoteFramed(_ id: UUID, _ framed: Bool) {
        edit(.schematic, "Change Frame") { scope in
            sheet.updateNotes(scope) { $0.setFramed(id, framed) }
        }
    }

    func deleteNote(_ id: UUID) {
        edit(.schematic, "Delete Text") { scope in
            sheet.updateNotes(scope) { $0.remove([id]) }
            if editingNoteID == id { editingNoteID = nil }
            pruneSelection()
        }
    }

    /// The text block under a point: the last one drawn there.
    func note(at point: CGPoint) -> SchematicNote? {
        sheet.sheetNotes.note(at: point)
    }
}
