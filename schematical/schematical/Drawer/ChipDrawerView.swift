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

/// Right-hand drawer. Parts are grouped by category, then by readable function.
/// The top bar filters by category and package. The family and gate-count menus
/// sit in the Logic category header because they only apply to logic chips.
struct ChipDrawerView: View {
    let catalog: ChipCatalogStore

    @State private var filter = DrawerFilter()
    @AppStorage(PreferenceKey.showObsoleteParts) private var showObsolete = false
    @AppStorage(PreferenceKey.showMilitaryParts) private var showMilitary = false
    @AppStorage(PreferenceKey.showAutomotiveParts) private var showAutomotive = false
    /// Keys whose open state the user has changed from its default, as keys from
    /// `categoryKey` and `functionKey`. Categories default open; functions default closed.
    @State private var toggled: Set<String> = []

    var body: some View {
        let groups = catalog.groups(matching: activeFilter)

        VStack(spacing: 0) {
            searchField
            filterBar

            List {
                ForEach(groups) { category in
                    DisclosureGroup(
                        isExpanded: expansion(for: categoryKey(category.category), defaultOpen: true)
                    ) {
                        ForEach(category.functions) { function in
                            DisclosureGroup(
                                isExpanded: expansion(
                                    for: functionKey(category.category, function.function),
                                    defaultOpen: false
                                )
                            ) {
                                ForEach(function.parts) { entry in
                                    partRow(entry)
                                }
                            } label: {
                                Text(function.function)
                                    .font(.body.weight(.medium))
                            }
                        }
                    } label: {
                        categoryHeader(category.category)
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
        .padding(.horizontal, 12)
        .padding(.top, 8)
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

    @ViewBuilder
    private func categoryHeader(_ category: String) -> some View {
        HStack {
            Text(category)
                .font(.headline)
            Spacer()
            if category == familyCategory {
                familyMenu(for: category)
                countMenu(for: category)
            }
        }
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

    private func countMenu(for category: String) -> some View {
        Menu {
            Button("All counts") {
                filter.gateCount = nil
            }
            Divider()
            ForEach(catalog.gateCounts(in: category), id: \.self) { count in
                Button(gateCountLabel(count)) {
                    filter.gateCount = count
                }
            }
        } label: {
            Text(filter.gateCount.map(gateCountLabel) ?? "All counts")
                .font(.caption)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
    }

    private func gateCountLabel(_ count: Int) -> String {
        count == 1 ? "1 gate" : "\(count) gates"
    }

    // MARK: Rows

    private func partRow(_ entry: ChipCatalogStore.PartEntry) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text(entry.part.name)
                Spacer(minLength: 8)
                if entry.chip.gateCount > 0 {
                    Text("(\(entry.chip.gateCount))")
                }
            }
            .font(.system(.body, design: .monospaced))
            Text(subtitle(for: entry))
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .draggable(ChipPartDragItem(partNumber: entry.part.name))
    }

    private func subtitle(for entry: ChipCatalogStore.PartEntry) -> String {
        entry.part.family.isEmpty
            ? entry.part.package
            : "\(entry.part.package) · \(entry.part.family)"
    }

    // MARK: Collapsing

    private func categoryKey(_ category: String) -> String {
        "category:\(category)"
    }

    private func functionKey(_ category: String, _ function: String) -> String {
        "function:\(category)/\(function)"
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
            return "Loading the backend catalog…"
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
