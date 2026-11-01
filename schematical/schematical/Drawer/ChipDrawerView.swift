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
/// files), then by readable function: Memory → SRAM → 32 KB. Switches, LEDs
/// and 7-segment displays skip the function level. The top bar filters by
/// category and package. Each category header carries the menus for the kinds
/// of part in it: family and gate count for gates and flip-flops, a count of
/// positions for switches, a color for LEDs and displays; a group of
/// 7-segment displays has digit count and size menus.
struct ChipDrawerView: View {
    let catalog: PartCatalogStore

    @State private var filter = DrawerFilter()
    @AppStorage(PreferenceKey.showObsoleteParts) private var showObsolete = false
    @AppStorage(PreferenceKey.showMilitaryParts) private var showMilitary = false
    @AppStorage(PreferenceKey.showAutomotiveParts) private var showAutomotive = false
    /// Keys whose open state the user has changed from its default, as keys from
    /// `categoryKey`, `subcategoryKey` and `functionKey`. Categories default open;
    /// subcategories and functions default closed.
    @State private var toggled: Set<String> = []

    var body: some View {
        let groups = catalog.groups(matching: activeFilter)

        VStack(spacing: 0) {
            HStack(spacing: 8) {
                searchField
                voltageMenu
            }
            .padding(.horizontal, 12)
            .padding(.top, 8)
            filterBar

            List {
                ForEach(groups) { category in
                    DisclosureGroup(
                        isExpanded: expansion(for: categoryKey(category.category), defaultOpen: true)
                    ) {
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
                                // Menus in the label keep their own clicks; the rest toggles the group.
                                subcategoryHeader(subcategory.subcategory)
                                    .contentShape(Rectangle())
                                    .onTapGesture {
                                        toggle(key, defaultOpen: false)
                                    }
                            }
                        }
                        ForEach(category.functions) { function in
                            functionGroup(function, path: category.category)
                        }
                    } label: {
                        // Menus in the header keep their own clicks; the rest toggles the group.
                        categoryHeader(category.category)
                            .contentShape(Rectangle())
                            .onTapGesture {
                                toggle(categoryKey(category.category), defaultOpen: true)
                            }
                    }
                }
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

    /// Supply voltage next to the search field: lists only parts that run at
    /// that rail (see `DrawerFilter.voltageMV`).
    private var voltageMenu: some View {
        let choices = catalog.voltageChoices
        let selected = choices.first { $0.mv == filter.voltageMV }?.name
        return Menu {
            Button("All voltages") {
                filter.voltageMV = nil
            }
            Divider()
            ForEach(choices, id: \.name) { choice in
                Button(choice.name) {
                    filter.voltageMV = choice.mv
                }
            }
        } label: {
            Text(selected ?? "All")
                .font(.caption)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
        .help("Show only parts that run at this supply voltage")
    }

    private var filterBar: some View {
        HStack(spacing: 8) {
            Picker("Category", selection: $filter.category) {
                Text("All categories").tag(String?.none)
                ForEach(catalog.categories, id: \.self) { category in
                    Text(category).tag(String?.some(category))
                }
            }
            .labelsHidden()

            Picker("Package", selection: $filter.package) {
                Text("All packages").tag(String?.none)
                ForEach(catalog.packages, id: \.self) { package in
                    Text(package).tag(String?.some(package))
                }
            }
            .labelsHidden()
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
                || kinds.contains(PART_JKFF.rawValue) {
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
            if catalog.kinds(inSubcategory: subcategory).contains(PART_7SEG.rawValue) {
                countMenu(counts: catalog.displayDigitCounts, selection: $filter.displayDigits, unit: "digit")
                textMenu(catalog.displaySizes, selection: $filter.displaySize, all: "All sizes")
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
    private func countMenu(counts: [Int], selection: Binding<Int?>, unit: String) -> some View {
        func label(_ count: Int) -> String {
            count == 1 ? "1 \(unit)" : "\(count) \(unit)s"
        }
        return Menu {
            Button("All counts") {
                selection.wrappedValue = nil
            }
            Divider()
            ForEach(counts, id: \.self) { count in
                Button(label(count)) {
                    selection.wrappedValue = count
                }
            }
        } label: {
            Text(selection.wrappedValue.map(label) ?? "All counts")
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
            ForEach(function.parts) { entry in
                partRow(entry)
            }
        } label: {
            Text(function.function)
                .font(.body.weight(.medium))
                .frame(maxWidth: .infinity, alignment: .leading)
                .contentShape(Rectangle())
                .onTapGesture {
                    toggle(key, defaultOpen: false)
                }
        }
    }

    private func partRow(_ part: CatalogPart) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text(part.name)
                Spacer(minLength: 8)
                if let count = part.count, count > 0 {
                    Text("(\(count))")
                }
            }
            .font(.system(.body, design: .monospaced))
            Text(subtitle(for: part))
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .draggable(ChipPartDragItem(partNumber: part.name))
    }

    /// Package, then the kind's facts: family, speed grade, actuator or color;
    /// for a 7-segment display also its size and whether it is common anode
    /// (CA) or common cathode (CC); last, its voltage (`voltageLabel`).
    private func subtitle(for part: CatalogPart) -> String {
        ([part.package] + part.detailLabels + [part.voltageLabel].compactMap { $0 })
            .filter { !$0.isEmpty }
            .joined(separator: " · ")
    }

    // MARK: Collapsing

    private func categoryKey(_ category: String) -> String {
        "category:\(category)"
    }

    private func subcategoryKey(_ category: String, _ subcategory: String) -> String {
        "subcategory:\(category)/\(subcategory)"
    }

    /// `path` is the category, or "category/subcategory".
    private func functionKey(_ path: String, _ function: String) -> String {
        "function:\(path)/\(function)"
    }

    /// Opens or closes a group when its label is clicked, like its chevron.
    private func toggle(_ key: String, defaultOpen: Bool) {
        withAnimation(.easeInOut(duration: 0.15)) {
            expansion(for: key, defaultOpen: defaultOpen).wrappedValue.toggle()
        }
    }

    private func expansion(for key: String, defaultOpen: Bool) -> Binding<Bool> {
        Binding(
            get: { toggled.contains(key) ? !defaultOpen : defaultOpen },
            set: { isExpanded in
                if isExpanded == defaultOpen {
                    toggled.remove(key)
                } else {
                    toggled.insert(key)
                }
            }
        )
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
