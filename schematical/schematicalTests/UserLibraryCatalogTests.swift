import Foundation
import XCTest
@testable import schematical

/// The catalog and the User Library's folder together (plan-53-B).
@MainActor
final class UserLibraryCatalogTests: XCTestCase {
    private var directory: URL!
    /// Stores made by tests, kept for the life of the process: freeing a
    /// `PartCatalogStore` inside a test aborts the host (see
    /// `TestSupport.newDocument()`).
    private static var kept: [PartCatalogStore] = []

    override func setUp() async throws {
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("userlib-\(UUID().uuidString)")
    }

    override func tearDown() async throws {
        try? FileManager.default.removeItem(at: directory)
    }

    /// A fresh catalog with the bundled parts; `attached` also attaches the
    /// test's folder (a "launch").
    private func makeStore(attached: Bool = true) -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        if attached { store.attachUserLibrary(UserPartLibrary(directory: directory)) }
        return store
    }

    private func fileText(_ uuid: String, _ name: String, drawer: [String] = ["Custom", "Gates"]) -> String {
        PartRegistrationTests.fileText(uuid: uuid, name: name, drawer: drawer)
    }

    private func files() -> [URL] {
        ((try? FileManager.default.contentsOfDirectory(at: directory, includingPropertiesForKeys: nil)) ?? [])
            .sorted { $0.lastPathComponent < $1.lastPathComponent }
    }

    /// (a) Adding lists the part under User Library with its sub-group and
    /// writes `<uuid>.schpart` holding the registered text.
    func testAddListsUnderUserLibraryAndWritesTheFile() throws {
        let store = makeStore()
        let uuid = "A1000000-0000-4000-8000-000000000001"
        let part = try store.addToUserLibrary(fileText: fileText(uuid, "UL-A"), source: "a")
        XCTAssertEqual(part.drawer, ["User Library", "Gates"])
        XCTAssertEqual(store.listing(of: part), .user)
        let category = try XCTUnwrap(store.groups(matching: DrawerFilter()).first { $0.category == "User Library" })
        XCTAssertEqual(category.subcategories.first { $0.subcategory == "Gates" }?.allParts.contains(part), true)
        XCTAssertEqual(files().map(\.lastPathComponent), ["\(uuid).schpart"])
        let written = try String(contentsOf: files()[0], encoding: .utf8)
        XCTAssertEqual(written, store.registeredText(uuid: uuid))
        XCTAssertEqual(store.userLibraryItems.map(\.part), [part])
        XCTAssertEqual(store.userLibraryItems.first?.url, files()[0])
    }

    /// (b) A second catalog attached to the same folder (a relaunch) lists the
    /// same part.
    func testRelaunchListsTheSamePart() throws {
        let uuid = "A2000000-0000-4000-8000-000000000001"
        let first = try makeStore().addToUserLibrary(fileText: fileText(uuid, "UL-B"), source: "b")
        let second = makeStore()
        let part = try XCTUnwrap(second.part(uuid: uuid))
        XCTAssertEqual(part, first)
        XCTAssertEqual(part.name, "UL-B")
        XCTAssertEqual(second.listing(of: part), .user)
        XCTAssertEqual(part.drawer, ["User Library", "Gates"])
        XCTAssertEqual(second.userLibraryProblems, [])
        XCTAssertEqual(second.userLibraryItems.map(\.part), [part])
    }

    /// (c) Removing deletes the file and hides the part; a later launch lacks it.
    func testRemoveDeletesTheFileAndHidesThePart() throws {
        let store = makeStore()
        let uuid = "A3000000-0000-4000-8000-000000000001"
        let part = try store.addToUserLibrary(fileText: fileText(uuid, "UL-C"), source: "c")
        try store.removeFromUserLibrary(uuid: uuid)
        XCTAssertEqual(files(), [])
        XCTAssertEqual(store.listing(of: part), .removed)
        XCTAssertFalse(store.groups(matching: DrawerFilter()).flatMap(\.allParts).contains(part))
        XCTAssertEqual(store.part(uuid: uuid), part)
        XCTAssertTrue(store.userLibraryItems.isEmpty)
        XCTAssertNil(makeStore().part(uuid: uuid))
    }

    /// (d) A File Library part given to `addToUserLibrary(_:)` becomes a user part.
    func testAFilePartMovesToTheUserLibrary() throws {
        let store = makeStore()
        let uuid = "A4000000-0000-4000-8000-000000000001"
        let part = try store.register(fileText: fileText(uuid, "UL-D"), source: "d")
        XCTAssertEqual(store.listing(of: part), .file)
        XCTAssertEqual(files(), [])
        try store.addToUserLibrary(part)
        XCTAssertEqual(store.listing(of: part), .user)
        XCTAssertEqual(store.part(uuid: uuid)?.drawer, ["User Library", "Gates"])
        XCTAssertEqual(files().count, 1)
    }

    /// (d2) The drawer rebuilds on `derivedRevision`, not on the part count: a part
    /// moving between categories leaves the count as it was (found by the real-app
    /// run: the part stayed under File Library after Add to User Library).
    func testMovingAPartChangesTheDrawerRevisionNotTheCount() throws {
        let store = makeStore()
        let part = try store.register(fileText: fileText("A4000000-0000-4000-8000-000000000009", "UL-D2"), source: "d2")
        let count = store.partCount
        let revision = store.derivedRevision
        try store.addToUserLibrary(part)
        XCTAssertEqual(store.partCount, count)
        XCTAssertNotEqual(store.derivedRevision, revision)
    }

    /// (e) A name a bundled part has is suffixed once, and the file holds the
    /// suffixed name, so a relaunch lists the same name.
    func testASuffixedNameIsWhatTheFileHolds() throws {
        let store = makeStore()
        let uuid = "A5000000-0000-4000-8000-000000000001"
        let part = try store.addToUserLibrary(fileText: fileText(uuid, "SN74HC00N"), source: "e")
        XCTAssertEqual(part.name, "SN74HC00N.2")
        XCTAssertTrue(try String(contentsOf: files()[0], encoding: .utf8).contains("\"SN74HC00N.2\""))
        XCTAssertEqual(makeStore().part(uuid: uuid)?.name, "SN74HC00N.2")
    }

    /// (f) A corrupt file and a second file with a taken uuid are problems and
    /// do not stop the others.
    func testBadFilesAreProblemsAndTheRestLoad() throws {
        let good = "A6000000-0000-4000-8000-000000000001"
        let library = UserPartLibrary(directory: directory)
        try library.write(uuid: good, text: fileText(good, "UL-F"))
        try Data("not json".utf8).write(to: directory.appendingPathComponent("b-corrupt.schpart"))
        try Data(fileText(good, "UL-F-DUP").utf8).write(to: directory.appendingPathComponent("z-dup.schpart"))
        // A file the loader refuses.
        let bad = "A6000000-0000-4000-8000-000000000002"
        try Data(fileText(bad, "UL-F-BAD").replacingOccurrences(of: "\"nand\"", with: "\"nope\"").utf8)
            .write(to: directory.appendingPathComponent("\(bad).schpart"))
        let store = makeStore()
        XCTAssertEqual(store.userLibraryItems.map(\.part.name), ["UL-F"])
        XCTAssertEqual(Set(store.userLibraryProblems.map(\.url.lastPathComponent)),
                       ["b-corrupt.schpart", "z-dup.schpart", "\(bad).schpart"])
        XCTAssertFalse(store.userLibraryProblems.contains { $0.reason.isEmpty })
    }

    /// (g) With no library attached, adding throws.
    func testNoLibraryThrows() throws {
        let store = makeStore(attached: false)
        XCTAssertThrowsError(try store.addToUserLibrary(
            fileText: fileText("A7000000-0000-4000-8000-000000000001", "UL-G"), source: "g")) {
            XCTAssertEqual($0 as? PartRegistrationError, .noUserLibrary)
        }
    }

    /// (h) A failed write throws `libraryWrite` and the part stays a File
    /// Library part.
    func testAFailedWriteKeepsThePartInTheFileLibrary() throws {
        try Data("in the way".utf8).write(to: directory)
        let store = makeStore()
        let uuid = "A8000000-0000-4000-8000-000000000001"
        XCTAssertThrowsError(try store.addToUserLibrary(fileText: fileText(uuid, "UL-H"), source: "h")) {
            guard case .libraryWrite? = $0 as? PartRegistrationError else {
                return XCTFail("expected libraryWrite, got \($0)")
            }
        }
        let part = try XCTUnwrap(store.part(uuid: uuid))
        XCTAssertEqual(store.listing(of: part), .file)
        XCTAssertEqual(part.drawer, ["File Library", "Gates"])
        XCTAssertTrue(store.userLibraryItems.isEmpty)
    }

    /// (i) The User Library category has its own kinds, so the drawer's
    /// filter menus follow it.
    func testUserLibraryCategoryHasItsKinds() throws {
        let store = makeStore()
        XCTAssertFalse(store.categories.contains("User Library"))
        try store.addToUserLibrary(fileText: fileText("A9000000-0000-4000-8000-000000000001", "UL-I"), source: "i")
        XCTAssertTrue(store.categories.contains("User Library"))
        XCTAssertTrue(store.kinds(inCategory: "User Library").contains(PART_GATE.rawValue))
    }
}
