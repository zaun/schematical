import CoreGraphics
import Foundation

// MARK: - Selection items

/// One thing that can be selected on the Schematic page.
nonisolated enum SchematicItem: Hashable, Sendable {
    case chip(UUID)
    case wire(UUID)
    case junction(UUID)
}

/// One thing that can be selected on the PCB page: a part's footprint (by
/// the part's id), a board text or a trace.
nonisolated enum BoardItem: Hashable, Sendable {
    case footprint(UUID)
    case text(UUID)
    case trace(UUID)
}

// MARK: - Selection set

/// What a Select/Pointer press on an item did to the selection, and what a
/// release without movement (a click) still has to do.
nonisolated struct SelectionPress<Item: Hashable>: Equatable {
    enum ClickAction: Equatable {
        case none
        /// Shift-click on an item that was already selected: drop it.
        case remove(Item)
        /// Plain click on one item of a larger selection: keep only it.
        case reduce(Item)
    }

    /// Whether the drag moves every selected item as one group. False when
    /// the press made the item the whole selection, so the drag does what a
    /// drag on that item alone does (for a wire, edit its route).
    let dragsGroup: Bool
    let onClick: ClickAction
}

/// The selected items of one page. Editor state only: never saved.
nonisolated struct ItemSelection<Item: Hashable>: Equatable {
    private(set) var items: Set<Item> = []

    var isEmpty: Bool { items.isEmpty }
    var count: Int { items.count }

    /// The item when exactly one is selected.
    var sole: Item? { items.count == 1 ? items.first : nil }

    func contains(_ item: Item) -> Bool { items.contains(item) }

    /// A plain click: only `item` is selected.
    mutating func replace(with item: Item) {
        items = [item]
    }

    /// A Shift-click: `item` goes in if it was out, out if it was in.
    mutating func toggle(_ item: Item) {
        if items.remove(item) == nil {
            items.insert(item)
        }
    }

    mutating func insert(_ item: Item) {
        items.insert(item)
    }

    mutating func remove(_ item: Item) {
        items.remove(item)
    }

    mutating func clear() {
        items.removeAll()
    }

    /// Keeps only the items `keep` accepts (for example, ones still in the
    /// document after an erase).
    mutating func filter(_ keep: (Item) -> Bool) {
        let kept = items.filter(keep)
        if kept.count != items.count {
            items = kept
        }
    }

    /// A Select/Pointer press on `item`.
    ///
    /// - Shift: the item joins the selection and the drag moves the group.
    ///   A click on an item that was already in it removes it again.
    /// - No Shift, on an item of a larger selection: the drag moves the
    ///   group; a click keeps only that item.
    /// - No Shift otherwise: the item becomes the whole selection.
    mutating func press(_ item: Item, shift: Bool) -> SelectionPress<Item> {
        if shift {
            if items.contains(item) {
                return SelectionPress(dragsGroup: true, onClick: .remove(item))
            }
            items.insert(item)
            return SelectionPress(dragsGroup: true, onClick: .none)
        }
        if items.contains(item), items.count > 1 {
            return SelectionPress(dragsGroup: true, onClick: .reduce(item))
        }
        replace(with: item)
        return SelectionPress(dragsGroup: false, onClick: .none)
    }

    /// The end of a press: a click finishes what the press left pending.
    mutating func release(_ press: SelectionPress<Item>, wasClick: Bool) {
        guard wasClick else { return }
        switch press.onClick {
        case .none: break
        case let .remove(item): remove(item)
        case let .reduce(item): replace(with: item)
        }
    }
}

// MARK: - Group drags

/// Where every selected Schematic item was when a group drag started.
/// The drag applies one snapped delta to all of it.
nonisolated struct SchematicGroupBase: Equatable {
    var chips: [UUID: CGPoint] = [:]
    var junctions: [UUID: CGPoint] = [:]
    /// Selected wires' stored bends. Their ends stay attached.
    var wires: [UUID: [CGPoint]] = [:]
    /// Every wire the release settles: the selected ones and every wire
    /// with an end on a moved part or junction.
    var affectedWires: Set<UUID> = []
    /// Affected wires whose route was all right angles at the press; the
    /// release keeps them that way.
    var orthogonalWires: Set<UUID> = []

    var isEmpty: Bool { chips.isEmpty && junctions.isEmpty && wires.isEmpty }
}

/// Where every selected PCB item was when a group drag started (board mm).
nonisolated struct BoardGroupBase: Equatable {
    var footprints: [UUID: CGPoint] = [:]
    var texts: [UUID: CGPoint] = [:]

    var isEmpty: Bool { footprints.isEmpty && texts.isEmpty }
}

extension OrthogonalRoute {
    /// Whether every segment of a path is horizontal or vertical.
    nonisolated static func isOrthogonal(_ path: [CGPoint]) -> Bool {
        zip(path, path.dropFirst()).allSatisfy { $0.x == $1.x || $0.y == $1.y }
    }
}
