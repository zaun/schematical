import AppKit
import XCTest
@testable import schematical

/// A drawer part's right-click menu (`DrawerPartMenu`, plan-68-AB): which
/// items it has and what each does, with the alerts answered by the test
/// and a User Library folder the test made.
@MainActor
final class DrawerPartMenuTests: XCTestCase {
    private var directory: URL!
    private var savedPresenter: ExportPresenter!
    /// Alerts shown, by message text.
    private var alerts: [String] = []
    /// The button each alert is answered with.
    private var answer = NSApplication.ModalResponse.alertFirstButtonReturn
    /// Stores made by tests, kept for the life of the process (see `UserLibraryCatalogTests`).
    private static var kept: [PartCatalogStore] = []

    override func setUp() {
        super.setUp()
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("drawer-menu-\(UUID().uuidString)")
        savedPresenter = DrawerPartMenu.presenter
        alerts = []
        var presenter = ExportPresenter()
        presenter.keyWindow = { nil }
        presenter.alert = { [unowned self] alert, window in
            XCTAssertNil(window)
            alerts.append(alert.messageText)
            return answer
        }
        DrawerPartMenu.presenter = presenter
    }

    override func tearDown() {
        DrawerPartMenu.presenter = savedPresenter
        try? FileManager.default.removeItem(at: directory)
        super.tearDown()
    }

    private func store(attached: Bool = true) -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        if attached { store.attachUserLibrary(UserPartLibrary(directory: directory)) }
        return store
    }

    private func register(_ store: PartCatalogStore, _ uuid: String, _ name: String) throws -> CatalogPart {
        try store.register(fileText: PartRegistrationTests.fileText(uuid: uuid, name: name), source: "test")
    }

    func testItemsFollowTheListing() throws {
        let store = store()
        let file = try register(store, "B1000000-0000-4000-8000-000000000001", "DM-A")
        XCTAssertEqual(DrawerPartMenu.items(for: file, listing: .file), [.addToLibrary, .copyToEditor])
        let uuid = try XCTUnwrap(file.uuid)
        XCTAssertEqual(DrawerPartMenu.items(for: file, listing: .user), [.edit(uuid: uuid), .remove, .copyToEditor])
        XCTAssertEqual(DrawerPartMenu.items(for: file, listing: .removed), [.copyToEditor])
        let bundled = try TestSupport.part("SN74HC00N")
        XCTAssertNil(bundled.uuid)
        XCTAssertEqual(DrawerPartMenu.items(for: bundled, listing: nil), [.copyToEditor])
        if let fixed = TestSupport.catalog.allParts.first(where: { part in
            PartKinds.name(of: part.kind).map { !PartDraft.enabledKinds.contains($0) } ?? true
        }) {
            XCTAssertEqual(DrawerPartMenu.items(for: fixed, listing: nil), [], fixed.name)
        }
        XCTAssertEqual([DrawerPartMenu.Item.addToLibrary, .edit(uuid: "u"), .remove, .copyToEditor].map(\.id),
                       ["Add to User Library", "Edit…", "Remove from User Library…", "Copy to Editor…"])
    }

    func testEditAndCopyOpenTheEditor() async throws {
        let store = store()
        let part = try register(store, "B2000000-0000-4000-8000-000000000001", "DM-B")
        var opened: [PartEditorTarget] = []
        await DrawerPartMenu.perform(.edit(uuid: "B2000000-0000-4000-8000-000000000001"), part: part,
                                     catalog: store) { opened.append($0) }
        await DrawerPartMenu.perform(.copyToEditor, part: part, catalog: store) { opened.append($0) }
        XCTAssertEqual(opened.map(\.source), [.libraryPart(uuid: "B2000000-0000-4000-8000-000000000001"),
                                            .copy(name: "DM-B")])
        XCTAssertEqual(alerts, [])
    }

    func testAddThenRemove() async throws {
        let store = store()
        let part = try register(store, "B3000000-0000-4000-8000-000000000001", "DM-C")
        await DrawerPartMenu.perform(.addToLibrary, part: part, catalog: store) { _ in XCTFail("no editor") }
        XCTAssertEqual(store.listing(of: part), .user)
        XCTAssertEqual(alerts, [])

        answer = .alertSecondButtonReturn
        await DrawerPartMenu.perform(.remove, part: part, catalog: store) { _ in }
        XCTAssertEqual(alerts, ["Remove “DM-C” from your User Library?"])
        XCTAssertEqual(store.listing(of: part), .user, "cancelled")

        answer = .alertFirstButtonReturn
        await DrawerPartMenu.perform(.remove, part: part, catalog: store) { _ in }
        XCTAssertEqual(store.listing(of: part), .removed)
    }

    func testFailuresShowAnAlert() async throws {
        let store = store(attached: false)
        let part = try register(store, "B4000000-0000-4000-8000-000000000001", "DM-D")
        await DrawerPartMenu.perform(.addToLibrary, part: part, catalog: store) { _ in }
        await DrawerPartMenu.perform(.remove, part: part, catalog: store) { _ in }
        XCTAssertEqual(alerts, ["Couldn't add to your User Library", "Remove “DM-D” from your User Library?",
                                "Couldn't remove from your User Library"])
        // A bundled part (no uuid) asks nothing.
        alerts = []
        await DrawerPartMenu.perform(.remove, part: try TestSupport.part("SN74HC00N"), catalog: store) { _ in }
        XCTAssertEqual(alerts, [])
    }
}
