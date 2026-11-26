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

/// What the User Library window does outside itself, injectable so a test
/// answers it (plan-68-AC): Finder, alerts, the Add Part… file panel and the
/// Part Editor.
@MainActor
struct UserLibraryActions {
    var reveal: @MainActor ([URL]) -> Void = { NSWorkspace.shared.activateFileViewerSelecting($0) }
    var ask: @MainActor (NSAlert) async -> NSApplication.ModalResponse = { await UserLibraryPrompts.run($0) }
    var addPart: @MainActor (PartCatalogStore) async -> Void = { await PartFileLoading.addToLibrary(catalog: $0) }
    /// Opens a part in the Part Editor; nil = the window's own `openWindow`.
    var edit: (@MainActor (PartEditorTarget) -> Void)?
}

/// The User Library window's choices and changes (plan-68-AC: the logic
/// `UserLibraryWindowView` held).
@MainActor
enum UserLibraryWindowModel {
    /// The Part Editor target for a row.
    static func target(_ row: UserLibraryRow) -> PartEditorTarget {
        PartEditorTarget(source: .libraryPart(uuid: row.id))
    }

    /// The row Edit… opens: the only chosen row, when the Part Editor has a form for its kind.
    static func editable(_ chosen: [UserLibraryRow]) -> UserLibraryRow? {
        chosen.count == 1 && chosen[0].canEdit ? chosen[0] : nil
    }

    /// The rows of `ids`, in `rows`' order.
    static func picked(_ rows: [UserLibraryRow], _ ids: Set<String>) -> [UserLibraryRow] {
        rows.filter { ids.contains($0.id) }
    }

    /// Asks, then removes each part; failures are listed in one alert. The ids
    /// that were asked about (removed or not), so the selection drops them;
    /// empty when the user cancelled or there was nothing to remove.
    static func remove(_ rows: [UserLibraryRow], catalog: PartCatalogStore, actions: UserLibraryActions) async
        -> Set<String> {
        guard let first = rows.first else { return [] }
        let alert = rows.count == 1
            ? UserLibraryPrompts.removeAlert(name: first.name)
            : UserLibraryPrompts.removeAlert(count: rows.count)
        guard await actions.ask(alert) == .alertFirstButtonReturn else { return [] }
        var failures: [(name: String, error: Error)] = []
        for row in rows {
            do {
                try catalog.removeFromUserLibrary(uuid: row.id)
            } catch {
                failures.append((row.name, error))
            }
        }
        if !failures.isEmpty {
            _ = await actions.ask(UserLibraryPrompts.failuresAlert(
                title: "Couldn't remove from your User Library", failures: failures))
        }
        return Set(rows.map(\.id))
    }

    /// Removes a library file that could not be loaded; a failure is shown in an alert.
    static func removeProblem(_ problem: UserPartLibraryProblem, catalog: PartCatalogStore,
                              actions: UserLibraryActions) async {
        do {
            try catalog.removeUserLibraryProblem(problem)
        } catch {
            _ = await actions.ask(UserLibraryPrompts.failuresAlert(
                title: "Couldn't remove from your User Library", failures: [(problem.url.lastPathComponent, error)]))
        }
    }
}

/// File › User Library…: lists the parts the user keeps, removes them, opens
/// them in the Part Editor, adds a part file and shows a part's file in
/// Finder. A library file that could not be loaded is listed with the reason.
struct UserLibraryWindowView: View {
    let catalog: PartCatalogStore
    let actions: UserLibraryActions
    @State private var selection: Set<String>
    @Environment(\.openWindow) private var openWindow

    /// `actions` nil: the app's own (Finder, alerts, the file panel, `openWindow`).
    @MainActor
    init(catalog: PartCatalogStore, actions: UserLibraryActions? = nil) {
        self.catalog = catalog
        self.actions = actions ?? UserLibraryActions()
        _selection = State(initialValue: [])
    }

    var body: some View {
        let rows = UserLibraryRow.rows(from: catalog.userLibraryItems)
        let chosen = UserLibraryWindowModel.picked(rows, selection)
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
                    rowMenu(UserLibraryWindowModel.picked(rows, ids))
                } primaryAction: { ids in
                    _ = UserLibraryWindowModel.editable(UserLibraryWindowModel.picked(rows, ids)).map(edit)
                }
            }
            Divider()
            HStack {
                Button("Add Part…") { Task { await actions.addPart(catalog) } }
                Spacer()
                Button("Edit…") { _ = UserLibraryWindowModel.editable(chosen).map(edit) }
                    .disabled(UserLibraryWindowModel.editable(chosen) == nil)
                Button("Show in Finder") { actions.reveal(chosen.map(\.url)) }
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

    /// A row's context menu: Edit… for one editable part, then Show in Finder and Remove….
    @ViewBuilder
    func rowMenu(_ picked: [UserLibraryRow]) -> some View {
        if let row = UserLibraryWindowModel.editable(picked) {
            Button("Edit…") { edit(row) }
        }
        if !picked.isEmpty {
            Button("Show in Finder") { actions.reveal(picked.map(\.url)) }
            Button("Remove…") { Task { await remove(picked) } }
        }
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
                    Button("Show in Finder") { actions.reveal([problem.url]) }
                    Button("Remove") {
                        Task { await UserLibraryWindowModel.removeProblem(problem, catalog: catalog, actions: actions) }
                    }
                }
            }
        }
        .padding(10)
    }

    private func edit(_ row: UserLibraryRow) {
        let target = UserLibraryWindowModel.target(row)
        if let open = actions.edit { open(target) } else { openWindow(id: PartEditorScene.id, value: target) }
    }

    private func remove(_ rows: [UserLibraryRow]) async {
        selection.subtract(await UserLibraryWindowModel.remove(rows, catalog: catalog, actions: actions))
    }
}
