import SwiftUI

/// The drawer's rows in a scroll view, building only those near the viewport.
/// Every row has a fixed height, so the content height is exact and spacers
/// stand in for the rows left out. The scroll position is this view's own
/// state: a scroll redraws the list and not the search field and menus
/// above it, and each row skips its body when it is unchanged
/// (`DrawerRowView`).
struct DrawerScrollList<Row: View>: View {
    /// How far past the viewport rows are built, so a scroll does not show
    /// blank space before the next redraw.
    private static var overscan: CGFloat { 300 }

    let layout: DrawerLayout
    /// The drawer's filter: header menus show it, so a change redraws rows.
    let filter: DrawerFilter
    @ViewBuilder let row: (DrawerRowItem) -> Row
    @State private var window: ScrollWindow

    init(layout: DrawerLayout, filter: DrawerFilter, @ViewBuilder row: @escaping (DrawerRowItem) -> Row) {
        self.layout = layout
        self.filter = filter
        self.row = row
        _window = State(initialValue: ScrollWindow())
    }

    var body: some View {
        // `window` is only as fresh as the last scroll report. When the list
        // shrinks (a group closes, another opens) the scroll view pulls the
        // offset back to the new end, so build the rows around where the
        // window can be at most.
        let span = window.bottom - window.top
        let top = min(window.top, max(0, layout.tops[layout.items.count] - span))
        let rows = layout.rows(from: top - Self.overscan, to: top + span + Self.overscan)
        ScrollView {
            VStack(alignment: .leading, spacing: 0) {
                Color.clear.frame(height: rows.top)
                ForEach(layout.items[rows.range]) { item in
                    DrawerRowView(item: item, filter: filter) { row(item) }
                        .equatable()
                }
                Color.clear.frame(height: rows.bottom)
            }
            .padding(.horizontal, 12)
            .padding(.vertical, 4)
        }
        // The content's size is part of what is watched: opening or closing
        // a group changes it with no change of offset, and the window then
        // has to be read again. The offset is held inside the content.
        .onScrollGeometryChange(for: ScrollWindow.self) { geometry in
            ScrollWindow(offset: geometry.contentOffset.y, height: geometry.containerSize.height,
                         content: geometry.contentSize.height)
        } action: { _, new in
            window = new
        }
        // The first window is made for the real height: the scroll geometry
        // reports only changes, and a drawer taller than the guess in
        // `ScrollWindow()` would start with its lower rows left out.
        .onGeometryChange(for: CGFloat.self) { proxy in
            proxy.size.height
        } action: { height in
            window = ScrollWindow(offset: window.top, height: height)
        }
    }
}

/// One row, redrawn only when its item or the drawer's filter changes.
struct DrawerRowView<Content: View>: View, Equatable {
    let item: DrawerRowItem
    let filter: DrawerFilter
    @ViewBuilder let content: () -> Content

    static func == (lhs: DrawerRowView, rhs: DrawerRowView) -> Bool {
        lhs.item == rhs.item && lhs.filter == rhs.filter
    }

    var body: some View { content() }
}

/// One line of the drawer: a group's header or a part. Every kind has a
/// fixed height, so a row's place in the list is known without building it.
struct DrawerRowItem: Identifiable, Equatable {
    enum Kind {
        case category(String)
        case subcategory(PartCatalogStore.SubcategoryGroup)
        case function(PartCatalogStore.FunctionGroup)
        case bits(PartCatalogStore.BitGroup)
        case part(CatalogPart)
    }

    /// How far each level is indented, the chevron's column.
    static let indent: CGFloat = 16
    static let headerHeight: CGFloat = 34
    static let partHeight: CGFloat = 46

    /// A group's id is its open/closed key ("subcategory:Memory/SRAM",
    /// "function:Memory/SRAM/32 KB", "…/8-bit"; "category:<name>" for a
    /// category); a part's is "part:<name>".
    let id: String
    let kind: Kind
    /// How many groups the row sits inside.
    let depth: Int
    /// Whether a group is open; false for a part.
    let isOpen: Bool

    var height: CGFloat {
        if case .part = kind { return Self.partHeight }
        return Self.headerHeight
    }

    /// Same row: the id names the group or part, and `depth` and `isOpen`
    /// are all that change about it while the filter stays the same.
    static func == (lhs: DrawerRowItem, rhs: DrawerRowItem) -> Bool {
        lhs.id == rhs.id && lhs.depth == rhs.depth && lhs.isOpen == rhs.isOpen
    }
}

/// The drawer's rows, top to bottom, with where each begins.
struct DrawerLayout {
    var items: [DrawerRowItem] = []
    /// `tops[i]` is where row `i` begins; the last entry is the total height.
    var tops: [CGFloat] = [0]

    /// The rows overlapping `from…to` (points from the top of the list), and
    /// the heights left out above and below them.
    func rows(from: CGFloat, to: CGFloat) -> (range: Range<Int>, top: CGFloat, bottom: CGFloat) {
        guard !items.isEmpty else { return (0..<0, 0, 0) }
        // First row whose bottom is below `from`.
        var low = 0, high = items.count
        while low < high {
            let mid = (low + high) / 2
            if tops[mid + 1] > from { high = mid } else { low = mid + 1 }
        }
        let first = low
        // First row that begins at or past `to`.
        low = first
        high = items.count
        while low < high {
            let mid = (low + high) / 2
            if tops[mid] >= to { high = mid } else { low = mid + 1 }
        }
        let last = max(low, first)
        return (first..<last, tops[first], tops[items.count] - tops[last])
    }
}

/// Where the list is scrolled, in coarse steps: the scroll view reports every
/// point of a scroll, and a redraw per point would cost more than it saves.
struct ScrollWindow: Equatable {
    static let step: CGFloat = 100
    var top: CGFloat = 0
    var bottom: CGFloat = 1200

    init() {}

    /// `content`, when known, keeps the offset inside the content: a scroll
    /// view that has just shrunk can still report its old, too-large offset.
    init(offset: CGFloat, height: CGFloat, content: CGFloat? = nil) {
        var clamped = max(0, offset)
        if let content { clamped = min(clamped, max(0, content - height)) }
        top = (clamped / Self.step).rounded(.down) * Self.step
        bottom = ((clamped + height) / Self.step).rounded(.up) * Self.step
    }
}

/// The drawer's work that only changes with its filter or its open groups,
/// kept between redraws: building the groups scans the whole library, and
/// the rows are rebuilt on each toggle but not on each scroll or hover. A plain
/// class, not observed: `ChipDrawerView.body` reads and fills it.
@MainActor
final class DrawerMemo {
    /// As `SchematicDocument`'s: an isolated deinit aborts the runtime when
    /// a closing document frees this.
    nonisolated deinit {}

    private struct GroupsKey: Equatable {
        let filter: DrawerFilter
        let partCount: Int
        let revision: Int
        let loadState: PartCatalogStore.LoadState
    }

    private struct LayoutKey: Equatable {
        let generation: Int
        let openCategory: String?
        let toggled: Set<String>
    }

    private var groupsKey: GroupsKey?
    private var cachedGroups: [PartCatalogStore.CategoryGroup] = []
    private var generation = 0
    private var layoutKey: LayoutKey?
    private var cachedLayout = DrawerLayout()

    func groups(for catalog: PartCatalogStore, filter: DrawerFilter) -> [PartCatalogStore.CategoryGroup] {
        let key = GroupsKey(filter: filter, partCount: catalog.partCount,
                           revision: catalog.derivedRevision, loadState: catalog.loadState)
        if key != groupsKey {
            cachedGroups = catalog.groups(matching: filter)
            groupsKey = key
            generation += 1
        }
        return cachedGroups
    }

    func layout(groups: [PartCatalogStore.CategoryGroup], openCategory: String?,
                toggled: Set<String>) -> DrawerLayout {
        let key = LayoutKey(generation: generation, openCategory: openCategory, toggled: toggled)
        if key != layoutKey {
            cachedLayout = Self.flatten(groups, openCategory: openCategory, toggled: toggled)
            layoutKey = key
        }
        return cachedLayout
    }

    /// The rows of the open groups, in drawer order.
    static func flatten(_ groups: [PartCatalogStore.CategoryGroup], openCategory: String?,
                                toggled: Set<String>) -> DrawerLayout {
        var items: [DrawerRowItem] = []

        func addFunction(_ function: PartCatalogStore.FunctionGroup, path: String, depth: Int) {
            let key = "function:\(path)/\(function.function)"
            let isOpen = toggled.contains(key)
            items.append(DrawerRowItem(id: key, kind: .function(function), depth: depth, isOpen: isOpen))
            guard isOpen else { return }
            if function.bitGroups.isEmpty {
                for part in function.parts {
                    items.append(DrawerRowItem(id: "part:\(part.name)", kind: .part(part), depth: depth + 1, isOpen: false))
                }
            } else {
                // ADCs and DACs are too many for one list: one group per resolution.
                for group in function.bitGroups {
                    let bitsKey = "\(key)/\(group.bits)-bit"
                    let bitsOpen = toggled.contains(bitsKey)
                    items.append(DrawerRowItem(id: bitsKey, kind: .bits(group), depth: depth + 1, isOpen: bitsOpen))
                    guard bitsOpen else { continue }
                    for part in group.parts {
                        items.append(DrawerRowItem(id: "part:\(part.name)", kind: .part(part), depth: depth + 2, isOpen: false))
                    }
                }
            }
        }

        func addSubcategory(_ subcategory: PartCatalogStore.SubcategoryGroup, depth: Int) {
            let path = subcategory.path.joined(separator: "/")
            let key = "subcategory:\(path)"
            let isOpen = toggled.contains(key)
            items.append(DrawerRowItem(id: key, kind: .subcategory(subcategory), depth: depth, isOpen: isOpen))
            guard isOpen else { return }
            for subgroup in subcategory.subgroups {
                addSubcategory(subgroup, depth: depth + 1)
            }
            for function in subcategory.functions {
                addFunction(function, path: path, depth: depth + 1)
            }
            for part in subcategory.parts {
                items.append(DrawerRowItem(id: "part:\(part.name)", kind: .part(part), depth: depth + 1, isOpen: false))
            }
        }

        for category in groups {
            let isOpen = openCategory == category.category
            items.append(DrawerRowItem(id: "category:\(category.category)", kind: .category(category.category),
                                       depth: 0, isOpen: isOpen))
            guard isOpen else { continue }
            for subcategory in category.subcategories {
                addSubcategory(subcategory, depth: 1)
            }
            for function in category.functions {
                addFunction(function, path: category.category, depth: 1)
            }
        }

        var tops: [CGFloat] = [0]
        tops.reserveCapacity(items.count + 1)
        for item in items {
            tops.append(tops[tops.count - 1] + item.height)
        }
        return DrawerLayout(items: items, tops: tops)
    }
}

fileprivate extension Optional where Wrapped == String {
    /// Opens `category` as the drawer's open category, replacing the open
    /// one, or closes it, leaving none open; closing another changes nothing.
    mutating func setDrawerOpen(_ category: String, _ open: Bool) {
        if open {
            self = category
        } else if self == category {
            self = nil
        }
    }
}

extension Set where Element == String {
    /// Forgets every open drawer group below `category`: their keys are
    /// "subcategory:<category>/…" and "function:<category>/…", the bits
    /// groups' keys extending a function's.
    mutating func closeGroups(inCategory category: String) {
        let prefixes = ["subcategory:\(category)/", "function:\(category)/"]
        self = filter { key in !prefixes.contains { key.hasPrefix($0) } }
    }
}

fileprivate extension Set where Element == String {
    /// Opens or closes the drawer group `key`: the set holds the keys whose
    /// state differs from `defaultOpen`.
    mutating func setDrawerOpen(_ key: String, _ open: Bool, defaultOpen: Bool) {
        if open == defaultOpen {
            remove(key)
        } else {
            insert(key)
        }
    }
}

/// Which drawer groups are open (plan-68-AB: out of `ChipDrawerView`): at
/// most one category, and the groups below it by their `DrawerRowItem.id`
/// keys ("subcategory:…", "function:…", "…/8-bit"). All start closed.
struct DrawerOpenGroups: Equatable {
    /// The one open category, or nil when all are closed. Opening a category
    /// closes the one that was open.
    var category: String?
    /// The open groups below the categories.
    var toggled: Set<String> = []

    /// `openGroups`: group keys to start open, e.g. a subcategory, plus at
    /// most one "category:<name>".
    init(_ openGroups: Set<String> = []) {
        let categoryPrefix = "category:"
        category = openGroups.first { $0.hasPrefix(categoryPrefix) }.map { String($0.dropFirst(categoryPrefix.count)) }
        toggled = openGroups.filter { !$0.hasPrefix(categoryPrefix) }
    }

    /// Opens or closes a group. Opening a category closes the open one. A
    /// category that closes takes its open sub-groups with it, so it opens
    /// again showing only its own groups.
    mutating func toggle(_ item: DrawerRowItem) {
        switch item.kind {
        case let .category(name):
            let before = category
            category.setDrawerOpen(name, !item.isOpen)
            if let before, before != category { toggled.closeGroups(inCategory: before) }
        case .subcategory, .function, .bits:
            toggled.setDrawerOpen(item.id, !item.isOpen, defaultOpen: false)
        case .part:
            break
        }
    }
}
