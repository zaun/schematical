import AppKit
import SwiftUI

/// The User Library's window (plan-53-F).
enum UserLibraryScene {
    static let id = "user-library"
}

/// One part of the User Library as the window's table shows it.
struct UserLibraryRow: Identifiable, Equatable {
    /// The part's uuid.
    let id: String
    let name: String
    let manufacturer: String
    let kind: String
    let package: String
    let added: Date
    /// The part's file in the library folder.
    let url: URL
    /// Whether the Part Editor has a form for the part's kind (Edit…).
    let canEdit: Bool

    /// The rows for `items`, in natural name order.
    static func rows(from items: [PartCatalogStore.UserLibraryItem]) -> [UserLibraryRow] {
        items.map { item in
            let kindName = PartKinds.name(of: item.part.kind)
            return UserLibraryRow(
                id: item.id,
                name: item.part.name,
                manufacturer: item.part.manufacturer,
                kind: kindName.map(PartDraft.displayName(of:)) ?? "",
                package: item.part.package,
                added: item.added,
                url: item.url,
                canEdit: kindName.map { PartDraft.enabledKinds.contains($0) } ?? false)
        }
        .sorted { $0.name.localizedStandardCompare($1.name) == .orderedAscending }
    }
}

/// File › User Library…: lists the parts the user keeps, removes them, opens
/// them in the Part Editor, adds a part file and shows a part's file in
/// Finder. A library file that could not be loaded is listed with the reason.
struct UserLibraryWindowView: View {
    let catalog: PartCatalogStore
    @State private var selection: Set<String> = []
    @Environment(\.openWindow) private var openWindow

    var body: some View {
        let rows = UserLibraryRow.rows(from: catalog.userLibraryItems)
        let chosen = rows.filter { selection.contains($0.id) }
        VStack(spacing: 0) {
            if !catalog.userLibraryProblems.isEmpty {
                problems
                Divider()
            }
            if rows.isEmpty {
                ContentUnavailableView(
                    "No Parts in Your Library",
                    systemImage: "books.vertical",
                    description: Text("Parts you add with Load Part… or Add to Library are kept here.")
                )
            } else {
                Table(rows, selection: $selection) {
                    TableColumn("Name") { Text($0.name).monospaced() }
                        .width(min: 100, ideal: 170)
                    TableColumn("Manufacturer") { Text($0.manufacturer) }
                        .width(min: 80, ideal: 130)
                    TableColumn("Kind") { Text($0.kind) }
                        .width(min: 60, ideal: 100)
                    TableColumn("Package") { Text($0.package) }
                        .width(min: 60, ideal: 90)
                    TableColumn("Added") { Text($0.added, format: .dateTime.year().month().day()) }
                        .width(min: 80, ideal: 100)
                }
                .contextMenu(forSelectionType: String.self) { ids in
                    let picked = rows.filter { ids.contains($0.id) }
                    if picked.count == 1, picked[0].canEdit {
                        Button("Edit…") { edit(picked[0]) }
                    }
                    if !picked.isEmpty {
                        Button("Show in Finder") { reveal(picked) }
                        Button("Remove…") { Task { await remove(picked) } }
                    }
                } primaryAction: { ids in
                    if let row = rows.first(where: { ids.contains($0.id) }), ids.count == 1, row.canEdit {
                        edit(row)
                    }
                }
            }
            Divider()
            HStack {
                Button("Add Part…") { Task { await PartFileLoading.addToLibrary(catalog: catalog) } }
                Spacer()
                Button("Edit…") { if let row = chosen.first { edit(row) } }
                    .disabled(chosen.count != 1 || chosen.first?.canEdit != true)
                Button("Show in Finder") { reveal(chosen) }
                    .disabled(chosen.isEmpty)
                Button("Remove…") { Task { await remove(chosen) } }
                    .disabled(chosen.isEmpty)
            }
            .padding(10)
        }
        .frame(minWidth: 560, minHeight: 280)
        .navigationTitle("User Library")
        .navigationSubtitle("\(rows.count) \(rows.count == 1 ? "part" : "parts")")
    }

    /// Library files that could not be loaded, with the reason and a way to
    /// get rid of each.
    private var problems: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Couldn't load").font(.headline)
            ForEach(catalog.userLibraryProblems) { problem in
                HStack(alignment: .firstTextBaseline) {
                    Image(systemName: "exclamationmark.triangle").foregroundStyle(.orange)
                    VStack(alignment: .leading) {
                        Text(problem.url.lastPathComponent).monospaced()
                        Text(problem.reason).font(.caption).foregroundStyle(.secondary)
                    }
                    Spacer()
                    Button("Show in Finder") { NSWorkspace.shared.activateFileViewerSelecting([problem.url]) }
                    Button("Remove") {
                        do {
                            try catalog.removeUserLibraryProblem(problem)
                        } catch {
                            Task {
                                _ = await UserLibraryPrompts.run(UserLibraryPrompts.failuresAlert(
                                    title: "Couldn't remove from your User Library",
                                    failures: [(problem.url.lastPathComponent, error)]))
                            }
                        }
                    }
                }
            }
        }
        .padding(10)
    }

    private func edit(_ row: UserLibraryRow) {
        openWindow(id: PartEditorScene.id, value: PartEditorTarget(source: .libraryPart(uuid: row.id)))
    }

    private func reveal(_ rows: [UserLibraryRow]) {
        NSWorkspace.shared.activateFileViewerSelecting(rows.map(\.url))
    }

    /// Asks, then removes each part; failures are listed in one alert.
    private func remove(_ rows: [UserLibraryRow]) async {
        guard let first = rows.first else { return }
        let alert = rows.count == 1
            ? UserLibraryPrompts.removeAlert(name: first.name)
            : UserLibraryPrompts.removeAlert(count: rows.count)
        guard await UserLibraryPrompts.run(alert) == .alertFirstButtonReturn else { return }
        var failures: [(name: String, error: Error)] = []
        for row in rows {
            do {
                try catalog.removeFromUserLibrary(uuid: row.id)
            } catch {
                failures.append((row.name, error))
            }
        }
        selection.subtract(rows.map(\.id))
        if !failures.isEmpty {
            _ = await UserLibraryPrompts.run(UserLibraryPrompts.failuresAlert(
                title: "Couldn't remove from your User Library", failures: failures))
        }
    }
}
