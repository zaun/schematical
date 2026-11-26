import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AC: the User Library window as it is shown, its buttons and row
/// menu, and its remove logic (`UserLibraryWindowModel`), with Finder, the
/// alerts, the file panel and the Part Editor answered by the test.
@MainActor
final class UserLibraryWindowViewTests: XCTestCase {
    private var directory: URL!
    private var probes: [ViewProbe] = []
    private static var kept: [PartCatalogStore] = []

    /// What the window asked of the outside, in order.
    private var revealed: [[URL]] = []
    private var asked: [String] = []
    private var answer: NSApplication.ModalResponse = .alertFirstButtonReturn
    private var added = 0
    private var edited: [PartEditorTarget] = []

    override func setUp() async throws {
        // Closing a test window must not quit the test host.
        AppDelegate.quitsAfterLastWindowCloses = false
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("library-view-\(UUID().uuidString)")
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
        try? FileManager.default.removeItem(at: directory)
    }

    private var actions: UserLibraryActions {
        UserLibraryActions(
            reveal: { self.revealed.append($0) },
            ask: { alert in
                self.asked.append(alert.messageText)
                return self.answer
            },
            addPart: { _ in self.added += 1 },
            edit: { self.edited.append($0) })
    }

    private func makeStore(attached: Bool = true) -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        if attached { store.attachUserLibrary(UserPartLibrary(directory: directory)) }
        return store
    }

    private static let uuids = ["F2000000-0000-4000-8000-000000000001", "F2000000-0000-4000-8000-000000000002"]

    /// A store holding two parts, LIB-A and LIB-B, and, when asked, a file that does not load.
    private func filledStore(withProblem: Bool = false) throws -> PartCatalogStore {
        if withProblem {
            try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
            try Data("not json".utf8).write(to: directory.appendingPathComponent("corrupt.schpart"))
        }
        let store = makeStore()
        for (uuid, name) in zip(Self.uuids, ["LIB-A", "LIB-B"]) {
            try store.addToUserLibrary(fileText: PartRegistrationTests.fileText(uuid: uuid, name: name), source: name)
        }
        return store
    }

    private func show(_ store: PartCatalogStore, actions: UserLibraryActions? = nil) -> ViewProbe {
        let probe = ViewProbe(UserLibraryWindowView(catalog: store, actions: actions ?? self.actions),
                              size: NSSize(width: 720, height: 420))
        probes.append(probe)
        return probe
    }

    private func select(_ probe: ViewProbe, rows: IndexSet) throws {
        let table = try XCTUnwrap(probe.views(NSTableView.self).first, "no table")
        table.selectRowIndexes(rows, byExtendingSelection: false)
        probe.settle()
    }

    /// The Name column's text of each row of the window's table.
    private func tableTexts(_ probe: ViewProbe) throws -> [String] {
        let table = try XCTUnwrap(probe.views(NSTableView.self).first, "no table")
        return (0..<table.numberOfRows).map { row in
            guard let cell = table.view(atColumn: 0, row: row, makeIfNecessary: true) else { return "" }
            return ViewProbe.texts(in: cell).joined(separator: " ")
        }
    }

    private func isEnabled(_ probe: ViewProbe, _ title: String) throws -> Bool {
        ViewProbe.isEnabled(try probe.element(title, role: "AXButton"))
    }

    func testTheWindowAsTheAppMakesIt() {
        // The app's own actions; nothing is pressed.
        let probe = ViewProbe(UserLibraryWindowView(catalog: makeStore()), size: NSSize(width: 720, height: 420))
        probes.append(probe)
        XCTAssertTrue(probe.has("Add Part…", role: "AXButton"), "\(probe.labels())")
        XCTAssertNil(UserLibraryActions().edit, "the window's own openWindow opens the Part Editor")
    }

    func testAnEmptyLibrarySaysSo() {
        let probe = show(makeStore())
        XCTAssertTrue(probe.has("No Parts in Your Library"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Couldn't load"))
        XCTAssertFalse(try isEnabled(probe, "Show in Finder"))
    }

    func testTheButtonsActOnTheChosenRows() throws {
        let store = try filledStore()
        let probe = show(store)
        XCTAssertEqual(try tableTexts(probe), ["LIB-A", "LIB-B"])
        for title in ["Edit…", "Show in Finder", "Remove…"] { XCTAssertFalse(try isEnabled(probe, title), title) }

        try select(probe, rows: [0])
        XCTAssertTrue(try isEnabled(probe, "Edit…"))
        try probe.press("Edit…")
        XCTAssertEqual(edited.map(\.source), [.libraryPart(uuid: Self.uuids[0])])
        try probe.press("Show in Finder")
        XCTAssertEqual(revealed, [[UserPartLibrary(directory: directory).url(forUUID: Self.uuids[0])]])

        try select(probe, rows: [0, 1])
        XCTAssertEqual(try XCTUnwrap(probe.views(NSTableView.self).first).selectedRowIndexes, [0, 1])
        XCTAssertFalse(try isEnabled(probe, "Edit…"), "Edit… opens one part")
        XCTAssertTrue(try isEnabled(probe, "Remove…"), "\(probe.rolesAndLabels())")
        try probe.press("Remove…")
        XCTAssertTrue(probe.wait { store.userLibraryItems.isEmpty })
        XCTAssertEqual(asked, ["Remove 2 parts from your User Library?"])
        XCTAssertTrue(probe.has("No Parts in Your Library"), "\(probe.labels())")

        try probe.press("Add Part…")
        XCTAssertTrue(probe.wait { self.added == 1 })
    }

    func testAProblemFileIsListedAndRemoved() throws {
        let store = try filledStore(withProblem: true)
        let probe = show(store)
        XCTAssertTrue(probe.has("Couldn't load"), "\(probe.labels())")
        XCTAssertTrue(probe.has("corrupt.schpart"), "\(probe.labels())")
        let problem = try XCTUnwrap(store.userLibraryProblems.first)
        XCTAssertTrue(probe.has(problem.reason), "\(probe.labels())")
        try probe.press("Show in Finder")
        XCTAssertEqual(revealed, [[problem.url]], "the problem's own Show in Finder comes first")
        try probe.press("Remove")
        XCTAssertTrue(probe.wait { store.userLibraryProblems.isEmpty })
        XCTAssertFalse(FileManager.default.fileExists(atPath: problem.url.path))
        XCTAssertFalse(probe.has("Couldn't load"), "\(probe.labels())")
        XCTAssertTrue(asked.isEmpty)
    }

    func testTheRowMenu() throws {
        let store = try filledStore()
        let rows = UserLibraryRow.rows(from: store.userLibraryItems)
        let view = UserLibraryWindowView(catalog: store, actions: actions)
        let one = ViewProbe(VStack { view.rowMenu([rows[0]]) })
        probes.append(one)
        try one.press("Edit…")
        XCTAssertEqual(edited.map(\.source), [.libraryPart(uuid: rows[0].id)])
        try one.press("Show in Finder")
        XCTAssertEqual(revealed, [[rows[0].url]])
        answer = .alertSecondButtonReturn
        try one.press("Remove…")
        XCTAssertTrue(one.wait { self.asked.count == 1 })
        XCTAssertEqual(asked, ["Remove “LIB-A” from your User Library?"])
        XCTAssertEqual(store.userLibraryItems.count, 2, "Cancel removes nothing")

        let two = ViewProbe(VStack { view.rowMenu(rows) })
        probes.append(two)
        XCTAssertFalse(two.has("Edit…"), "\(two.labels())")
        XCTAssertTrue(two.has("Remove…"))
        let none = ViewProbe(VStack { view.rowMenu([]) })
        probes.append(none)
        XCTAssertFalse(none.has("Show in Finder"))
    }

    func testADoubleClickEditsAndTheRowMenuListsTheActions() throws {
        let store = try filledStore()
        let probe = show(store)
        let table = try XCTUnwrap(probe.views(NSTableView.self).first)
        try probe.doubleClick(table, row: 1)
        XCTAssertEqual(edited.map(\.source), [.libraryPart(uuid: Self.uuids[1])])

        let menu = try probe.contextMenu(table, row: 1)
        XCTAssertEqual(menu.items.map(\.title), ["Edit…", "Show in Finder", "Remove…"])
        let reveal = try XCTUnwrap(menu.items.firstIndex { $0.title == "Show in Finder" })
        menu.performActionForItem(at: reveal)
        probe.settle()
        XCTAssertEqual(revealed, [[UserPartLibrary(directory: directory).url(forUUID: Self.uuids[1])]])
    }

    // MARK: UserLibraryWindowModel

    func testEditableNeedsOneEditableRow() throws {
        let rows = UserLibraryRow.rows(from: try filledStore().userLibraryItems)
        XCTAssertEqual(UserLibraryWindowModel.editable([rows[0]]), rows[0])
        XCTAssertNil(UserLibraryWindowModel.editable(rows))
        XCTAssertNil(UserLibraryWindowModel.editable([]))
        let fixed = rows[0]
        let locked = UserLibraryRow(id: fixed.id, name: fixed.name, manufacturer: fixed.manufacturer, kind: fixed.kind,
                                    package: fixed.package, added: fixed.added, url: fixed.url, canEdit: false)
        XCTAssertNil(UserLibraryWindowModel.editable([locked]))
        XCTAssertEqual(UserLibraryWindowModel.picked(rows, [rows[1].id]), [rows[1]])
        XCTAssertEqual(UserLibraryWindowModel.target(fixed).source, .libraryPart(uuid: fixed.id))
    }

    func testRemoveAsksAndListsFailures() async throws {
        let store = try filledStore()
        let rows = UserLibraryRow.rows(from: store.userLibraryItems)
        let removedNothing = await UserLibraryWindowModel.remove([], catalog: store, actions: actions)
        XCTAssertEqual(removedNothing, [])
        XCTAssertTrue(asked.isEmpty, "nothing to remove: no question")

        answer = .alertSecondButtonReturn
        let cancelled = await UserLibraryWindowModel.remove([rows[0]], catalog: store, actions: actions)
        XCTAssertEqual(cancelled, [])
        XCTAssertEqual(store.userLibraryItems.count, 2)

        answer = .alertFirstButtonReturn
        let removed = await UserLibraryWindowModel.remove([rows[0]], catalog: store, actions: actions)
        XCTAssertEqual(removed, [rows[0].id])
        XCTAssertEqual(store.userLibraryItems.map(\.part.name), ["LIB-B"])

        // A store with no library folder cannot remove: one alert lists the failure.
        asked = []
        let detached = makeStore(attached: false)
        let failed = await UserLibraryWindowModel.remove([rows[1]], catalog: detached, actions: actions)
        XCTAssertEqual(failed, [rows[1].id])
        XCTAssertEqual(asked, ["Remove “LIB-B” from your User Library?", "Couldn't remove from your User Library"])
    }

    func testRemovingAProblemThatCannotBeRemovedSaysSo() async {
        let problem = UserPartLibraryProblem(url: directory.appendingPathComponent("gone.schpart"), reason: "bad")
        await UserLibraryWindowModel.removeProblem(problem, catalog: makeStore(attached: false), actions: actions)
        XCTAssertEqual(asked, ["Couldn't remove from your User Library"])
    }
}
