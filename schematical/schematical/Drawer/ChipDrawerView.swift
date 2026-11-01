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
/// The top bar filters by category and package. The family menu sits in the
/// Logic category header because families only apply to logic chips.
struct ChipDrawerView: View {
    let catalog: ChipCatalogStore

    @State private var filter = DrawerFilter()
    /// Collapsed categories and functions, as keys from `categoryKey` and `functionKey`.
    @State private var collapsed: Set<String> = []

    var body: some View {
        let groups = catalog.groups(matching: filter)

        VStack(spacing: 0) {
            filterBar

            List {
                ForEach(groups) { category in
                    DisclosureGroup(isExpanded: expansion(for: categoryKey(category.category))) {
                        ForEach(category.functions) { function in
                            DisclosureGroup(
                                isExpanded: expansion(for: functionKey(category.category, function.function))
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
        .searchable(text: $filter.query, prompt: "Search parts")
        .safeAreaInset(edge: .bottom) {
            statusFooter
        }
    }

    // MARK: Filters

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

    // MARK: Rows

    private func partRow(_ entry: ChipCatalogStore.PartEntry) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            Text("\(entry.part.name) | \(entry.chip.gateCount)")
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

    private func expansion(for key: String) -> Binding<Bool> {
        Binding(
            get: { !collapsed.contains(key) },
            set: { isExpanded in
                if isExpanded {
                    collapsed.remove(key)
                } else {
                    collapsed.insert(key)
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
