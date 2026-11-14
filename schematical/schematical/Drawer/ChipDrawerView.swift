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
/// files), then by readable function: Memory → SRAM → 32 KB. Tactile switches
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
    /// Open groups below the categories, as keys from `subcategoryKey`,
    /// `functionKey` and `bitsKey`. All start closed.
    @State private var toggled: Set<String> = []

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
        let groups = catalog.groups(matching: activeFilter)

        VStack(spacing: 0) {
            searchField
                .padding(.horizontal, 12)
                .padding(.top, 8)
            filterBar

            // A plain stack, not a List: SwiftUI's List is an NSOutlineView,
            // and opening groups of rows with different heights makes AppKit
            // log "reentrant operation in its NSTableView delegate". Not a
            // lazy stack either: it guessed the height of rows it had not
            // built, so after scrolling to the end, clicks on the lower rows
            // fell outside the scroll view's content and did nothing.
            ScrollView {
                VStack(alignment: .leading, spacing: 0) {
                ForEach(groups) { category in
                    DisclosureGroup(isExpanded: $openCategory[drawerOpenCategory: category.category]) {
                        ForEach(category.subcategories) { subcategory in
                            let key = subcategoryKey(category.category, subcategory.subcategory)
                            DisclosureGroup(isExpanded: expansion(for: key, defaultOpen: false)) {
                                ForEach(subcategory.functions) { function in
                                    functionGroup(function, path: "\(category.category)/\(subcategory.subcategory)")
                                }
                                ForEach(subcategory.parts) { entry in
                                    partRow(entry)
                                }
                            } label: {
                                subcategoryHeader(subcategory.subcategory)
                            }
                            // Set on each level: a nested group does not pick
                            // up the style from its parent's content.
                            .disclosureGroupStyle(DrawerDisclosureStyle())
                        }
                        ForEach(category.functions) { function in
                            functionGroup(function, path: category.category)
                        }
                    } label: {
                        categoryHeader(category.category)
                    }
                }
                }
                .disclosureGroupStyle(DrawerDisclosureStyle())
                .padding(.horizontal, 12)
                .padding(.vertical, 4)
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
    private func subcategoryHeader(_ subcategory: String) -> some View {
        HStack {
            Text(subcategory)
                .font(.body.weight(.semibold))
            Spacer()
            let kinds = catalog.kinds(inSubcategory: subcategory)
            if kinds.contains(PART_7SEG.rawValue) {
                countMenu(counts: catalog.displayDigitCounts, selection: $filter.displayDigits, unit: "digit")
                textMenu(catalog.displaySizes, selection: $filter.displaySize, all: "All sizes")
            } else if catalog.isTactile(subcategory: subcategory) {
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

    /// A function's parts, closed by default. `path` is the category, plus the
    /// subcategory when there is one, so equal function names in different
    /// groups open and close separately.
    private func functionGroup(_ function: PartCatalogStore.FunctionGroup, path: String) -> some View {
        let key = functionKey(path, function.function)
        return DisclosureGroup(isExpanded: expansion(for: key, defaultOpen: false)) {
            if function.bitGroups.isEmpty {
                ForEach(function.parts) { entry in
                    partRow(entry)
                }
            } else {
                // ADCs and DACs are too many for one list: one group per resolution.
                ForEach(function.bitGroups) { group in
                    DisclosureGroup(isExpanded: expansion(for: bitsKey(key, group.bits), defaultOpen: false)) {
                        ForEach(group.parts) { entry in
                            partRow(entry)
                        }
                    } label: {
                        Text(group.title)
                            .font(.body.weight(.medium))
                    }
                    .disclosureGroupStyle(DrawerDisclosureStyle())
                }
            }
        } label: {
            functionHeader(function)
        }
        .disclosureGroupStyle(DrawerDisclosureStyle())
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

    private func partRow(_ part: CatalogPart) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text(part.kind == PART_HEADER ? HeaderFamily.title : part.name)
                Spacer(minLength: 8)
                if let label = part.trailingLabel {
                    Text(label)
                }
            }
            .font(.system(.body, design: .monospaced))
            Text(subtitle(for: part))
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .padding(.vertical, 4)
        .padding(.leading, DrawerDisclosureStyle.indent)
        .frame(maxWidth: .infinity, alignment: .leading)
        .overlay(alignment: .bottom) { Divider() }
        .contentShape(Rectangle())
        .draggable(ChipPartDragItem(partNumber: part.name))
        .contextMenu {
            // plan-32-C: a copy with a new uuid; the library part is never edited.
            if let kind = PartKinds.name(of: part.kind), PartDraft.enabledKinds.contains(kind) {
                Button("Copy to Editor…") {
                    openWindow(id: PartEditorScene.id, value: PartEditorTarget(source: .copy(name: part.name)))
                }
            }
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

    // MARK: Collapsing

    private func subcategoryKey(_ category: String, _ subcategory: String) -> String {
        "subcategory:\(category)/\(subcategory)"
    }

    /// `path` is the category, or "category/subcategory".
    private func functionKey(_ path: String, _ function: String) -> String {
        "function:\(path)/\(function)"
    }

    /// One resolution's group inside an ADC or DAC group (`functionKey`).
    private func bitsKey(_ functionKey: String, _ bits: Int) -> String {
        "\(functionKey)/\(bits)-bit"
    }

    /// Derived from `$toggled` rather than built from get/set closures:
    /// SwiftUI cannot see a change behind closures, so the List kept its rows
    /// and groups never opened or closed.
    private func expansion(for key: String, defaultOpen: Bool) -> Binding<Bool> {
        $toggled[drawerOpen: key, defaultOpen: defaultOpen]
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

/// The drawer's groups, laid out like the outline list they replaced: a
/// chevron and the label on one row with a rule under it, and the open
/// content indented below.
private struct DrawerDisclosureStyle: DisclosureGroupStyle {
    /// How far each level is indented, the chevron's column.
    static let indent: CGFloat = 16

    func makeBody(configuration: Configuration) -> some View {
        VStack(alignment: .leading, spacing: 0) {
            HStack(spacing: 4) {
                Image(systemName: "chevron.right")
                    .font(.caption.weight(.bold))
                    .foregroundStyle(.secondary)
                    .rotationEffect(.degrees(configuration.isExpanded ? 90 : 0))
                    .frame(width: Self.indent - 4)
                configuration.label
                    .frame(maxWidth: .infinity, alignment: .leading)
            }
            // A click anywhere on the row, chevron or label, opens or closes
            // the group; menus in the label keep their own clicks. It goes
            // through the group's binding: setting the drawer's state
            // directly left groups built while scrolling unchanged.
            .contentShape(Rectangle())
            .onTapGesture {
                withAnimation(.easeInOut(duration: 0.15)) {
                    configuration.isExpanded.toggle()
                }
            }
            .frame(minHeight: 22)
            .padding(.vertical, 4)
            .overlay(alignment: .bottom) { Divider() }

            if configuration.isExpanded {
                VStack(alignment: .leading, spacing: 0) {
                    configuration.content
                }
                .padding(.leading, Self.indent)
            }
        }
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
