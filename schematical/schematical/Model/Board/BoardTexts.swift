import CoreGraphics
import Foundation

/// The board's free silkscreen texts and their edits (plan-68-E). The grid
/// and layer count come in as parameters.
struct BoardTexts {
    var texts: [BoardText]

    private func index(_ id: UUID) -> Int? {
        texts.firstIndex { $0.id == id }
    }

    /// Adds "Text" with its top-left at `point` snapped to `grid`.
    mutating func add(at point: CGPoint, grid: Double) -> BoardText {
        let text = BoardText(id: UUID(), text: "Text", anchor: BoardGrid.snap(point, spacing: grid), size: BoardText.defaultSize)
        texts.append(text)
        return text
    }

    /// Changes a text's words. An empty string is refused and the text kept.
    mutating func setText(_ id: UUID, _ value: String) {
        guard !value.isEmpty, let index = index(id) else { return }
        texts[index].text = value
    }

    /// A text's font size in mil, clamped to `BoardText.sizeRange`.
    mutating func setSize(_ id: UUID, _ size: Int) {
        guard let index = index(id) else { return }
        texts[index].size = min(max(size, BoardText.sizeRange.lowerBound), BoardText.sizeRange.upperBound)
    }

    mutating func setLayer(_ id: UUID, _ layer: TextLayer) {
        guard let index = index(id) else { return }
        texts[index].layer = layer
    }

    /// Puts a text on `side`; only Top on a board of `layers` 1.
    mutating func setSide(_ id: UUID, _ side: BoardSide, layers: Int) {
        guard side == .top || layers > 1, let index = index(id), texts[index].side != side else { return }
        texts[index].side = side
    }

    /// Moves an unlocked text's anchor to `point` snapped to `grid`.
    mutating func move(_ id: UUID, to point: CGPoint, grid: Double) {
        guard let index = index(id), !texts[index].locked else { return }
        texts[index].anchor = BoardGrid.snap(point, spacing: grid)
    }

    /// Sets an unlocked text's rotation: 0, 90, 180 or 270; others are ignored.
    mutating func setRotation(_ id: UUID, _ rotation: Int) {
        guard BoardPlacement.rotations.contains(rotation), let index = index(id), !texts[index].locked else { return }
        texts[index].rotation = rotation
    }

    /// Turns an unlocked text 90° about its anchor.
    mutating func rotate(_ id: UUID, counterclockwise: Bool) {
        guard let index = index(id), !texts[index].locked else { return }
        texts[index].rotation = (texts[index].rotation + (counterclockwise ? 270 : 90)) % 360
    }

    mutating func setLocked(_ id: UUID, _ locked: Bool) {
        if let index = index(id) { texts[index].locked = locked }
    }

    mutating func remove(_ ids: Set<UUID>) {
        texts.removeAll { ids.contains($0.id) }
    }

    /// Copies of `copied` moved by `offset` (mm), snapped to `grid`, with new
    /// ids, unlocked, on Top on a board of `layers` 1. Returns them.
    mutating func paste(_ copied: [BoardText], offset: CGSize, grid: Double, layers: Int) -> [BoardText] {
        let pasted = copied.map { text in
            let moved = CGPoint(x: text.anchor.x + offset.width, y: text.anchor.y + offset.height)
            return BoardText(id: UUID(), text: text.text, anchor: BoardGrid.snap(moved, spacing: grid),
                             size: text.size, layer: text.layer, rotation: text.rotation,
                             side: layers == 1 ? .top : text.side)
        }
        texts += pasted
        return pasted
    }

    /// One frame of a group drag: each text in `anchors` goes to `shift` of
    /// where it was at the press.
    mutating func move(_ anchors: [UUID: CGPoint], by shift: (CGPoint) -> CGPoint) {
        for index in texts.indices {
            guard let anchor = anchors[texts[index].id] else { continue }
            let target = shift(anchor)
            if texts[index].anchor != target { texts[index].anchor = target }
        }
    }

    /// The text under a board point (mm): the last one drawn there, a Top
    /// text before a Bottom one. A text that isn't drawn can't be hit.
    func text(at point: CGPoint, visible: BoardVisibility) -> BoardText? {
        for side in [BoardSide.top, .bottom] {
            if let hit = texts.last(where: { $0.side == side && visible.showsText($0) && $0.boardRect.contains(point) }) {
                return hit
            }
        }
        return nil
    }
}
