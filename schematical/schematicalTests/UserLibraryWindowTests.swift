import Foundation
import XCTest
@testable import schematical

/// The User Library window's data and actions (plan-53-F).
@MainActor
final class UserLibraryWindowTests: XCTestCase {
    private var directory: URL!
    private static var kept: [PartCatalogStore] = []

    override func setUp() async throws {
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("window-\(UUID().uuidString)")
    }

    override func tearDown() async throws {
        try? FileManager.default.removeItem(at: directory)
    }

    private func makeStore() -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        store.attachUserLibrary(UserPartLibrary(directory: directory))
        return store
    }

    func testRowsAreInNameOrderWithThePartsFacts() throws {
        let store = makeStore()
        let before = Date().addingTimeInterval(-5)
        for (uuid, name) in [("F1000000-0000-4000-8000-000000000003", "ROW-10"),
                             ("F1000000-0000-4000-8000-000000000001", "ROW-2"),
                             ("F1000000-0000-4000-8000-000000000002", "ROW-1")] {
            try store.addToUserLibrary(fileText: PartRegistrationTests.fileText(uuid: uuid, name: name), source: name)
        }
        // A File Library part is not in the window.
        try store.register(fileText: PartRegistrationTests.fileText(
            uuid: "F1000000-0000-4000-8000-000000000009", name: "ROW-FILE"), source: "file")
        let rows = UserLibraryRow.rows(from: store.userLibraryItems)
        XCTAssertEqual(rows.map(\.name), ["ROW-1", "ROW-2", "ROW-10"])
        let first = try XCTUnwrap(rows.first)
        XCTAssertEqual(first.id, "F1000000-0000-4000-8000-000000000002")
        XCTAssertEqual(first.manufacturer, "Generic")
        XCTAssertEqual(first.kind, PartDraft.displayName(of: "gate"))
        XCTAssertEqual(first.package, "SOT-23")
        XCTAssertGreaterThan(first.added, before)
        XCTAssertTrue(first.canEdit)
        XCTAssertEqual(first.url, UserPartLibrary(directory: directory).url(forUUID: first.id))
        XCTAssertTrue(FileManager.default.fileExists(atPath: first.url.path))

        try store.removeFromUserLibrary(uuid: first.id)
        XCTAssertEqual(UserLibraryRow.rows(from: store.userLibraryItems).map(\.name), ["ROW-2", "ROW-10"])
    }

    func testRemovingAProblemDeletesItsFile() throws {
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        let corrupt = directory.appendingPathComponent("corrupt.schpart")
        try Data("not json".utf8).write(to: corrupt)
        let store = makeStore()
        let problem = try XCTUnwrap(store.userLibraryProblems.first)
        XCTAssertEqual(problem.url.lastPathComponent, "corrupt.schpart")
        try store.removeUserLibraryProblem(problem)
        XCTAssertTrue(store.userLibraryProblems.isEmpty)
        XCTAssertFalse(FileManager.default.fileExists(atPath: corrupt.path))
        // A relaunch has no problem left.
        XCTAssertTrue(makeStore().userLibraryProblems.isEmpty)
    }

    func testTheRemoveAlertForSeveralParts() {
        let alert = UserLibraryPrompts.removeAlert(count: 3)
        XCTAssertEqual(alert.messageText, "Remove 3 parts from your User Library?")
        XCTAssertEqual(alert.informativeText, "Schematics that use them keep their own copy.")
        XCTAssertEqual(alert.buttons.map(\.title), ["Remove", "Cancel"])
    }
}
