import CoreGraphics
import Foundation

/// A sheet's text blocks and their edits (plan-68-D). Drawn under
/// everything else.
struct SheetNotes {
    var notes: [SchematicNote]

    /// Adds a block with its top-left at `point` (snapped) and the placeholder words.
    mutating func add(at point: CGPoint) -> SchematicNote {
        let note = SchematicNote(id: UUID(), text: SchematicNote.placeholder, origin: SchematicGrid.snap(point))
        notes.append(note)
        return note
    }

    /// Changes a block's words. An empty string is refused and the words kept.
    mutating func setText(_ id: UUID, _ value: String) {
        guard !value.isEmpty, let index = notes.firstIndex(where: { $0.id == id }), notes[index].text != value else { return }
        notes[index].text = value
    }

    /// Sets a block's font size in points, clamped to `SchematicNote.sizeRange`.
    mutating func setSize(_ id: UUID, _ size: Int) {
        guard let index = notes.firstIndex(where: { $0.id == id }) else { return }
        notes[index].size = min(max(size, SchematicNote.sizeRange.lowerBound), SchematicNote.sizeRange.upperBound)
    }

    mutating func setFramed(_ id: UUID, _ framed: Bool) {
        guard let index = notes.firstIndex(where: { $0.id == id }) else { return }
        notes[index].framed = framed
    }

    /// One frame of a group drag: each block in `origins` goes to `shift` of
    /// where it was at the press.
    mutating func move(_ origins: [UUID: CGPoint], by shift: (CGPoint) -> CGPoint) {
        for index in notes.indices {
            guard let origin = origins[notes[index].id] else { continue }
            let moved = shift(origin)
            if notes[index].origin != moved { notes[index].origin = moved }
        }
    }

    mutating func remove(_ ids: Set<UUID>) {
        notes.removeAll { ids.contains($0.id) }
    }

    /// The block under a point: the last one drawn there.
    func note(at point: CGPoint) -> SchematicNote? {
        notes.last { $0.rect.contains(point) }
    }
}
