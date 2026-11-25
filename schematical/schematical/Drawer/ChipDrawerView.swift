import CoreTransferable
import SwiftUI
import UniformTypeIdentifiers

/// Payload carried by a drag from the drawer to the canvas.
nonisolated struct ChipPartDragItem: Codable, Hashable, Sendable, Transferable {
    let partNumber: String

    static var transferRepresentation: some TransferRepresentation {
        CodableRepresentation(contentType: .json)
    }
}

/// Right-hand drawer. Parts are grouped by their drawer path (from the part
/// files, up to three levels), then by readable function: Memory → SRAM →
/// 32 KB, Discrete → Resistors → Chip → 1 kΩ – 5 kΩ. Tactile switches
/// are grouped by body size instead (Switch → Tactile → 6 × 6 mm); DIP
/// switches, LEDs and 7-segment displays skip the function level. The top bar filters by
/// package and supply voltage. Each category header carries the menus for the kinds
/// of part in it: family and gate count for gates and flip-flops, a count of
/// positions for switches, a color for LEDs and displays; a group of
/// 7-segment displays has digit count and size menus, the tactile switch
/// group an actuator menu, and the ADC and DAC
/// groups have a channels menu and are split into groups by bits.
/// Categories start closed, and only one is open at a time.
struct ChipDrawerView: View, Equatable {
    let catalog: PartCatalogStore

    /// The sidebar redraws whenever the selected part or bus moves, and
    /// rebuilding this list costs far more than a drag step (plan-17). With
    /// `.equatable()` at the call site, a parent redraw that keeps the same
    /// catalog skips this view; its own state and the catalog it observes
    /// still redraw it. Compare any new stored input here, or changes to it
    /// will not show.
    static func == (lhs: ChipDrawerView, rhs: ChipDrawerView) -> Bool {
        lhs.catalog === rhs.catalog
    }

    @Environment(\.openWindow) private var openWindow
    @State private var filter = DrawerFilter()
    @AppStorage(PreferenceKey.showObsoleteParts) private var showObsolete = false
    @AppStorage(PreferenceKey.showMilitaryParts) private var showMilitary = false
    @AppStorage(PreferenceKey.showAutomotiveParts) private var showAutomotive = false
    /// The one open category, or nil when all are closed. Opening a category
    /// closes the one that was open.
    @State private var openCategory: String?
    /// Open groups below the categories, by their `DrawerRowItem.id` keys
    /// ("subcategory:…", "function:…", "…/8-bit"). All start closed.
    @State private var toggled: Set<String> = []
    /// The groups and rows built for the current filter and open groups, kept
    /// between redraws (see `DrawerMemo`).
    @State private var memo = DrawerMemo()

    /// `openGroups`: group keys to start open, e.g. a subcategory, plus at
    /// most one "category:<name>". Empty in the app.
    init(catalog: PartCatalogStore, openGroups: Set<String> = []) {
        self.catalog = catalog
        let categoryPrefix = "category:"
        _openCategory = State(initialValue: openGroups.first { $0.hasPrefix(categoryPrefix) }
            .map { String($0.dropFirst(categoryPrefix.count)) })
        _toggled = State(initialValue: openGroups.filter { !$0.hasPrefix(categoryPrefix) })
    }

    var body: some View {
        let groups = memo.groups(for: catalog, filter: activeFilter)
        let layout = memo.layout(groups: groups, openCategory: openCategory, toggled: toggled)

        VStack(spacing: 0) {
            searchField
                .padding(.horizontal, 12)
                .padding(.top, 8)
            filterBar

            // Only the rows near the viewport are built (`DrawerScrollList`;
            // the library lists tens of thousands of parts, and one open group
            // can hold thousands). Not a List: SwiftUI's
            // List is an NSOutlineView, and opening groups of rows with
            // different heights makes AppKit log "reentrant operation in its
            // NSTableView delegate". Not a LazyVStack either: it guessed the
            // height of rows it had not built, so after scrolling to the end,
            // clicks on the lower rows fell outside the scroll view's content
            // and did nothing.
            DrawerScrollList(layout: layout, filter: activeFilter) { item in
                rowView(item)
            }
            .overlay {
                if groups.isEmpty {
                    emptyState
                }
            }
        }
        .safeAreaInset(edge: .bottom) {
            statusFooter
        }
    }

    // MARK: Filters

    /// The drawer's own filter plus the part kinds allowed in Settings.
    private var activeFilter: DrawerFilter {
        var active = filter
        active.visibility = PartVisibility(
            showObsolete: showObsolete,
            showMilitary: showMilitary,
            showAutomotive: showAutomotive
        )
        return active
    }

    /// Always-visible search field at the top of the drawer.
    private var searchField: some View {
        HStack(spacing: 6) {
            Image(systemName: "magnifyingglass")
                .foregroundStyle(.secondary)
            TextField("Search parts", text: $filter.query)
                .textFieldStyle(.plain)
            if !filter.query.isEmpty {
                Button {
                    filter.query = ""
                } label: {
                    Image(systemName: "xmark.circle.fill")
                        .foregroundStyle(.secondary)
                }
                .buttonStyle(.plain)
                .help("Clear search")
            }
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 5)
        .background(.quaternary.opacity(0.6), in: RoundedRectangle(cornerRadius: 6))
    }

    private var filterBar: some View {
        HStack(spacing: 8) {
            Picker("Package", selection: $filter.package) {
                Text("All packages").tag(String?.none)
                ForEach(catalog.packages, id: \.self) { package in
                    Text(package).tag(String?.some(package))
                }
            }
            .labelsHidden()

            // Supply voltage: lists only parts that run at that rail
            // (see `DrawerFilter.voltageMV`).
            Picker("Voltage", selection: $filter.voltageMV) {
                Text("All voltages").tag(UInt32?.none)
                ForEach(catalog.voltageChoices, id: \.name) { choice in
                    Text(choice.name).tag(UInt32?.some(choice.mv))
                }
            }
            .labelsHidden()
            .help("Show only parts that run at this supply voltage")
        }
        .pickerStyle(.menu)
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
        .frame(maxWidth: .infinity, alignment: .leading)
    }

    /// The filter menus follow the kinds of part in the category: family and
    /// count for gates and flip-flops, positions for switches, color for LEDs
    /// and displays.
    @ViewBuilder
    private func categoryHeader(_ category: String) -> some View {
        let kinds = catalog.kinds(inCategory: category)
        HStack {
            Text(category)
                .font(.headline)
            Spacer()
            if kinds.contains(PART_GATE.rawValue) || kinds.contains(PART_DFF.rawValue)
                || kinds.contains(PART_JKFF.rawValue) || kinds.contains(PART_LATCH.rawValue)
                || kinds.contains(PART_COUNTER.rawValue) || kinds.contains(PART_SHIFT.rawValue)
                || kinds.contains(PART_DECODER.rawValue)
                || kinds.contains(PART_BUFFER.rawValue) || kinds.contains(PART_MUX.rawValue)
                || kinds.contains(PART_ARITH.rawValue) || kinds.contains(PART_REGISTER.rawValue) {
                familyMenu(for: category)
                countMenu(counts: catalog.logicCounts(in: category), selection: $filter.gateCount, unit: "gate")
            } else if kinds.contains(PART_SWITCH.rawValue) {
                countMenu(counts: catalog.switchPositionCounts(in: category),
                          selection: $filter.switchPositions, unit: "position")
            } else if kinds.contains(PART_LED.rawValue) || kinds.contains(PART_7SEG.rawValue) {
                textMenu(catalog.colors(in: category), selection: $filter.color, all: "All colors")
            }
        }
    }

    @ViewBuilder
    private func subcategoryHeader(_ subcategory: PartCatalogStore.SubcategoryGroup) -> some View {
        HStack {
            Text(subcategory.subcategory)
                .font(.body.weight(.semibold))
            Spacer()
            // A group's kinds include its sub-groups', so a group with
            // sub-groups leaves the menus to them (Vintage → Display tubes).
            let kinds = subcategory.subgroups.isEmpty ? catalog.kinds(inGroup: subcategory.path) : []
            if kinds.contains(PART_7SEG.rawValue) {
                countMenu(counts: catalog.displayDigitCounts, selection: $filter.displayDigits, unit: "digit")
                textMenu(catalog.displaySizes, selection: $filter.displaySize, all: "All sizes")
            } else if subcategory.subgroups.isEmpty, catalog.isTactile(group: subcategory.path) {
                textMenu(catalog.tactileActuators, selection: $filter.tactileActuator, all: "All actuators")
            }
        }
    }

    /// Menu of text values, such as colors or sizes, with an "all" choice first.
    private func textMenu(_ values: [String], selection: Binding<String?>, all: String) -> some View {
        Menu {
            Button(all) {
                selection.wrappedValue = nil
            }
            Divider()
            ForEach(values, id: \.self) { value in
                Button(value) {
                    selection.wrappedValue = value
                }
            }
        } label: {
            Text(selection.wrappedValue ?? all)
                .font(.caption)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
    }

    private func familyMenu(for category: String) -> some View {
        Menu {
            Button("All families") {
                filter.family = nil
            }
            Divider()
            ForEach(catalog.families(in: category), id: \.self) { family in
                Button(family) {
                    filter.family = family
                }
            }
        } label: {
            Text(filter.family ?? "All families")
                .font(.caption)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
    }

    /// Menu of counts. `unit` names one of what is counted, such as "gate"
    /// ("4 gates") or "position" ("8 positions").
    private func countMenu(counts: [Int], selection: Binding<Int?>, unit: String,
                           all: String = "All counts") -> some View {
        func label(_ count: Int) -> String {
            count == 1 ? "1 \(unit)" : "\(count) \(unit)s"
        }
        return Menu {
            Button(all) {
                selection.wrappedValue = nil
            }
            Divider()
            ForEach(counts, id: \.self) { count in
                Button(label(count)) {
                    selection.wrappedValue = count
                }
            }
        } label: {
            Text(selection.wrappedValue.map(label) ?? all)
                .font(.caption)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
    }

    // MARK: Rows

    @ViewBuilder
    private func rowView(_ item: DrawerRowItem) -> some View {
        switch item.kind {
        case let .category(name):
            groupHeader(item) { categoryHeader(name) }
        case let .subcategory(subcategory):
            groupHeader(item) { subcategoryHeader(subcategory) }
        case let .function(function):
            groupHeader(item) { functionHeader(function) }
        case let .bits(group):
            groupHeader(item) {
                Text(group.title)
                    .font(.body.weight(.medium))
            }
        case let .part(part):
            partRow(part, depth: item.depth)
        }
    }

    /// A group's row: a chevron and the label on one line with a rule under
    /// it. A click anywhere on the row, chevron or label, opens or closes the
    /// group; menus in the label keep their own clicks.
    private func groupHeader<Label: View>(_ item: DrawerRowItem,
                                          @ViewBuilder label: () -> Label) -> some View {
        HStack(spacing: 4) {
            Image(systemName: "chevron.right")
                .font(.caption.weight(.bold))
                .foregroundStyle(.secondary)
                .rotationEffect(.degrees(item.isOpen ? 90 : 0))
                .animation(.easeInOut(duration: 0.15), value: item.isOpen)
                .frame(width: DrawerRowItem.indent - 4)
            label()
                .frame(maxWidth: .infinity, alignment: .leading)
        }
        .contentShape(Rectangle())
        .onTapGesture { toggle(item) }
        .frame(height: DrawerRowItem.headerHeight)
        .overlay(alignment: .bottom) { Divider() }
        .padding(.leading, CGFloat(item.depth) * DrawerRowItem.indent)
    }

    /// Opens or closes a group. Opening a category closes the open one. A
    /// category that closes takes its open sub-groups with it, so it opens
    /// again showing only its own groups.
    private func toggle(_ item: DrawerRowItem) {
        switch item.kind {
        case let .category(name):
            let before = openCategory
            openCategory[drawerOpenCategory: name] = !item.isOpen
            if let before, before != openCategory { toggled.closeGroups(inCategory: before) }
        case .subcategory, .function, .bits:
            toggled[drawerOpen: item.id, defaultOpen: false] = !item.isOpen
        case .part:
            break
        }
    }

    /// An ADC or DAC group carries a Channels menu.
    @ViewBuilder
    private func functionHeader(_ function: PartCatalogStore.FunctionGroup) -> some View {
        HStack {
            Text(function.function)
                .font(.body.weight(.medium))
            Spacer()
            if let type = catalog.converterType(of: function.parts) {
                countMenu(counts: catalog.converterChannels(type: type),
                          selection: $filter.converterChannels[type], unit: "channel", all: "All channels")
            }
        }
    }

    private func partRow(_ part: CatalogPart, depth: Int) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text(part.kind == PART_HEADER ? HeaderFamily.title : part.name)
                    .lineLimit(1)
                Spacer(minLength: 8)
                if let label = part.trailingLabel {
                    Text(label)
                        .lineLimit(1)
                }
            }
            .font(.system(.body, design: .monospaced))
            Text(subtitle(for: part))
                .font(.caption)
                .foregroundStyle(.secondary)
                .lineLimit(1)
        }
        .padding(.leading, DrawerRowItem.indent)
        .frame(maxWidth: .infinity, alignment: .leading)
        .frame(height: DrawerRowItem.partHeight)
        .overlay(alignment: .bottom) { Divider() }
        .contentShape(Rectangle())
        .draggable(ChipPartDragItem(partNumber: part.name))
        .contextMenu {
            // plan-53-C: a part that came with a schematic can be kept; a
            // kept one can be taken out again.
            if part.uuid != nil {
                switch catalog.listing(of: part) {
                case .file:
                    Button("Add to User Library") {
                        Task { await UserLibraryPrompts.addShowingFailures([part], catalog: catalog) }
                    }
                case .user:
                    // plan-53-E: edit the library part itself; Save updates it.
                    if let uuid = part.uuid, let kind = PartKinds.name(of: part.kind),
                       PartDraft.enabledKinds.contains(kind) {
                        Button("Edit…") {
                            openWindow(id: PartEditorScene.id,
                                       value: PartEditorTarget(source: .libraryPart(uuid: uuid)))
                        }
                    }
                    Button("Remove from User Library…") {
                        Task { await removeFromUserLibrary(part) }
                    }
                default:
                    EmptyView()
                }
            }
            // plan-32-C: a copy with a new uuid; the library part is never edited.
            if let kind = PartKinds.name(of: part.kind), PartDraft.enabledKinds.contains(kind) {
                Button("Copy to Editor…") {
                    openWindow(id: PartEditorScene.id, value: PartEditorTarget(source: .copy(name: part.name)))
                }
            }
        }
        .padding(.leading, CGFloat(depth) * DrawerRowItem.indent)
    }

    /// Right-click › Remove from User Library…: after the confirmation, deletes
    /// the part's library file and takes it out of the drawer.
    private func removeFromUserLibrary(_ part: CatalogPart) async {
        guard let uuid = part.uuid,
              await UserLibraryPrompts.run(UserLibraryPrompts.removeAlert(name: part.name)) == .alertFirstButtonReturn
        else { return }
        do {
            try catalog.removeFromUserLibrary(uuid: uuid)
        } catch {
            _ = await UserLibraryPrompts.run(UserLibraryPrompts.failuresAlert(
                title: "Couldn't remove from your User Library", failures: [(part.name, error)]))
        }
    }

    /// Package, then the kind's facts: family, speed grade, actuator or color;
    /// for a 7-segment display also its size and whether it is common anode
    /// (CA) or common cathode (CC); last, its voltage (`voltageLabel`). A part
    /// with footprint choices shows their count in the package slot.
    private func subtitle(for part: CatalogPart) -> String {
        if part.kind == PART_HEADER { return HeaderFamily.subtitle }
        let package = part.footprintChoices.isEmpty ? part.package : "\(part.footprintChoices.count) footprints"
        return ([package] + part.detailLabels + [part.voltageLabel].compactMap { $0 })
            .filter { !$0.isEmpty }
            .joined(separator: " · ")
    }

    // MARK: Status

    @ViewBuilder
    private var emptyState: some View {
        if !filter.query.isEmpty {
            ContentUnavailableView.search(text: filter.query)
        } else if catalog.partCount == 0 {
            ContentUnavailableView(
                "No parts yet",
                systemImage: "cpu",
                description: Text(emptyDescription)
            )
        }
    }

    private var emptyDescription: String {
        switch catalog.loadState {
        case .notLoaded:
            return "Loading the part library…"
        case .loaded:
            return "The backend reports no supported parts."
        case .failed(let message):
            return message
        }
    }

    private var statusFooter: some View {
        HStack(spacing: 6) {
            switch catalog.loadState {
            case .notLoaded:
                ProgressView()
                    .controlSize(.small)
                Text("Loading backend")
            case .loaded:
                Image(systemName: "checkmark.circle.fill")
                    .foregroundStyle(.green)
                Text("\(catalog.partCount) parts from backend")
            case .failed(let message):
                Image(systemName: "exclamationmark.triangle.fill")
                    .foregroundStyle(.orange)
                Text(message)
                    .lineLimit(2)
            }
            Spacer(minLength: 0)
        }
        .font(.caption)
        .foregroundStyle(.secondary)
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(.bar)
    }
}

/// The drawer's rows in a scroll view, building only those near the viewport.
/// Every row has a fixed height, so the content height is exact and spacers
/// stand in for the rows left out. The scroll position is this view's own
/// state: a scroll redraws the list and not the search field and menus
/// above it, and each row skips its body when it is unchanged
/// (`DrawerRowView`).
private struct DrawerScrollList<Row: View>: View {
    /// How far past the viewport rows are built, so a scroll does not show
    /// blank space before the next redraw.
    private static var overscan: CGFloat { 300 }

    let layout: DrawerLayout
    /// The drawer's filter: header menus show it, so a change redraws rows.
    let filter: DrawerFilter
    @ViewBuilder let row: (DrawerRowItem) -> Row
    @State private var window = ScrollWindow()

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
private struct DrawerRowView<Content: View>: View, Equatable {
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
private struct DrawerRowItem: Identifiable, Equatable {
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
private struct DrawerLayout {
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
private struct ScrollWindow: Equatable {
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
private final class DrawerMemo {
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
    private static func flatten(_ groups: [PartCatalogStore.CategoryGroup], openCategory: String?,
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

private extension Optional where Wrapped == String {
    /// Whether `category` is the drawer's open category. Opening it replaces
    /// the open one; closing it leaves none open.
    subscript(drawerOpenCategory category: String) -> Bool {
        get { self == category }
        set {
            if newValue {
                self = category
            } else if self == category {
                self = nil
            }
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

private extension Set where Element == String {
    /// Whether the drawer group `key` is open: `toggled` holds the keys whose
    /// state differs from `defaultOpen`.
    subscript(drawerOpen key: String, defaultOpen defaultOpen: Bool) -> Bool {
        get { contains(key) != defaultOpen }
        set {
            if newValue == defaultOpen {
                remove(key)
            } else {
                insert(key)
            }
        }
    }
}
