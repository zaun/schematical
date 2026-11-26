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
    /// will not show. (`editor` is a test's, nil in the app: not compared.)
    static func == (lhs: ChipDrawerView, rhs: ChipDrawerView) -> Bool {
        lhs.catalog === rhs.catalog
    }

    @Environment(\.openWindow) private var openWindow
    private let editor: ((PartEditorTarget) -> Void)?
    @State private var filter = DrawerFilter()
    @AppStorage(PreferenceKey.showObsoleteParts) private var showObsolete = false
    @AppStorage(PreferenceKey.showMilitaryParts) private var showMilitary = false
    @AppStorage(PreferenceKey.showAutomotiveParts) private var showAutomotive = false
    /// The open category and groups below it.
    @State private var open: DrawerOpenGroups
    /// The groups and rows built for the current filter and open groups, kept
    /// between redraws (see `DrawerMemo`).
    @State private var memo = DrawerMemo()

    /// `openGroups`: group keys to start open, e.g. a subcategory, plus at
    /// most one "category:<name>". Empty in the app.
    /// `editor` opens a part in the Part Editor; nil = the window's own `openWindow`.
    init(catalog: PartCatalogStore, openGroups: Set<String> = [], editor: ((PartEditorTarget) -> Void)? = nil) {
        self.catalog = catalog
        self.editor = editor
        _open = State(initialValue: DrawerOpenGroups(openGroups))
    }

    var body: some View {
        let groups = memo.groups(for: catalog, filter: activeFilter)
        let layout = memo.layout(groups: groups, openCategory: open.category, toggled: open.toggled)

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
                Button { filter.query = "" } label: {
                    Image(systemName: "xmark.circle.fill")
                        .foregroundStyle(.secondary)
                }
                .buttonStyle(.plain)
                .help("Clear search")
                .accessibilityLabel("Clear search")
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
        HStack {
            Text(category)
                .font(.headline)
            Spacer()
            switch DrawerHeaderMenu.forCategory(kinds: catalog.kinds(inCategory: category)) {
            case .logic:
                textMenu(catalog.families(in: category), selection: $filter.family, all: "All families")
                countMenu(counts: catalog.logicCounts(in: category), selection: $filter.gateCount, unit: "gate")
            case .switches:
                countMenu(counts: catalog.switchPositionCounts(in: category),
                          selection: $filter.switchPositions, unit: "position")
            case .colors:
                textMenu(catalog.colors(in: category), selection: $filter.color, all: "All colors")
            default:
                EmptyView()
            }
        }
    }

    @ViewBuilder
    private func subcategoryHeader(_ subcategory: PartCatalogStore.SubcategoryGroup) -> some View {
        HStack {
            Text(subcategory.subcategory)
                .font(.body.weight(.semibold))
            Spacer()
            switch DrawerHeaderMenu.forGroup(subcategory, catalog: catalog) {
            case .displays:
                countMenu(counts: catalog.displayDigitCounts, selection: $filter.displayDigits, unit: "digit")
                textMenu(catalog.displaySizes, selection: $filter.displaySize, all: "All sizes")
            case .tactile:
                textMenu(catalog.tactileActuators, selection: $filter.tactileActuator, all: "All actuators")
            default:
                EmptyView()
            }
        }
    }

    /// Menu of text values, such as colors or sizes, with an "all" choice first.
    private func textMenu(_ values: [String], selection: Binding<String?>, all: String) -> some View {
        Menu {
            Button(all) { selection.wrappedValue = nil }
            Divider()
            ForEach(values, id: \.self) { value in Button(value) { selection.wrappedValue = value } }
        } label: {
            Text(selection.wrappedValue ?? all)
                .font(.caption)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
    }

    /// Menu of counts. `unit` names one of what is counted, such as "gate"
    /// ("4 gates") or "position" ("8 positions").
    private func countMenu(counts: [Int], selection: Binding<Int?>, unit: String,
                           all: String = "All counts") -> some View {
        Menu {
            Button(all) { selection.wrappedValue = nil }
            Divider()
            ForEach(counts, id: \.self) { count in Button(DrawerHeaderMenu.countLabel(count, unit: unit)) { selection.wrappedValue = count } }
        } label: {
            Text(selection.wrappedValue.map { DrawerHeaderMenu.countLabel($0, unit: unit) } ?? all)
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
        .onTapGesture { open.toggle(item) }
        .frame(height: DrawerRowItem.headerHeight)
        .overlay(alignment: .bottom) { Divider() }
        .padding(.leading, CGFloat(item.depth) * DrawerRowItem.indent)
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
            Text(DrawerPartMenu.subtitle(for: part))
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
            ForEach(DrawerPartMenu.items(for: part, listing: catalog.listing(of: part))) { item in
                Button(item.title) { Task { await DrawerPartMenu.perform(item, part: part, catalog: catalog, open: openEditor) } }
            }
        }
        .padding(.leading, CGFloat(depth) * DrawerRowItem.indent)
    }

    /// Opens the part editor on `target`.
    private func openEditor(_ target: PartEditorTarget) {
        if let editor { editor(target) } else { openWindow(id: PartEditorScene.id, value: target) }
    }

    // MARK: Status

    @ViewBuilder
    private var emptyState: some View {
        if !filter.query.isEmpty {
            ContentUnavailableView.search(text: filter.query)
        } else if catalog.partCount == 0 {
            ContentUnavailableView("No parts yet", systemImage: "cpu",
                                   description: Text(DrawerStatus.emptyDescription(catalog.loadState)))
        }
    }

    private var statusFooter: some View {
        let status = DrawerStatus(catalog.loadState, partCount: catalog.partCount)
        return HStack(spacing: 6) {
            if let icon = status.icon {
                Image(systemName: icon)
                    .foregroundStyle(status.isFailure ? .orange : .green)
            } else {
                ProgressView()
                    .controlSize(.small)
            }
            Text(status.text)
                .lineLimit(2)
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
