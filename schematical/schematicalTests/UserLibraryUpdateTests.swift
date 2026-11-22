import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Replacing a User Library part in place, at once (plan-53-D).
@MainActor
final class UserLibraryUpdateTests: XCTestCase {
    private var directory: URL!
    /// Stores made by tests, kept for the life of the process (freeing one
    /// inside a test aborts the host; see `TestSupport.newDocument()`).
    private static var kept: [PartCatalogStore] = []

    override func setUp() async throws {
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("update-\(UUID().uuidString)")
    }

    override func tearDown() async throws {
        try? FileManager.default.removeItem(at: directory)
    }

    private func makeStore(attached: Bool = true) -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        if attached { store.attachUserLibrary(UserPartLibrary(directory: directory)) }
        return store
    }

    private func text(_ uuid: String, _ name: String, description: String = "Single 2-input NAND gate") -> String {
        PartRegistrationTests.fileText(uuid: uuid, name: name, description: description, inlineFootprint: true)
    }

    private func fileContents(_ uuid: String) throws -> String {
        try String(contentsOf: UserPartLibrary(directory: directory).url(forUUID: uuid), encoding: .utf8)
    }

    /// A version of `part`'s file with the device description replaced.
    private func described(_ store: PartCatalogStore, _ uuid: String, _ description: String) throws -> String {
        try mutated(store, uuid) { object in
            var devices = object["devices"] as! [[String: Any]]
            devices[0]["description"] = description
            object["devices"] = devices
        }
    }

    private func renamed(_ store: PartCatalogStore, _ uuid: String, _ name: String) throws -> String {
        try mutated(store, uuid) { object in
            var devices = object["devices"] as! [[String: Any]]
            var parts = devices[0]["parts"] as! [[String: Any]]
            parts[0]["name"] = name
            devices[0]["parts"] = parts
            object["devices"] = devices
        }
    }

    private func mutated(_ store: PartCatalogStore, _ uuid: String,
                         _ change: (inout [String: Any]) -> Void) throws -> String {
        var file = try PartFileObject(text: try XCTUnwrap(store.registeredText(uuid: uuid)))
        change(&file.object)
        return file.canonicalText
    }

    func testAnUpdateReplacesThePartEverywhere() throws {
        let store = makeStore()
        let uuid = "D1000000-0000-4000-8000-000000000001"
        let old = try store.addToUserLibrary(fileText: text(uuid, "UP-A"), source: "a")
        let newText = try described(store, uuid, "Brand new description")
        let part = try store.updateUserLibraryPart(uuid: uuid, fileText: newText)
        XCTAssertEqual(part.uuid, uuid)
        XCTAssertEqual(part.description, "Brand new description")
        XCTAssertEqual(store.part(uuid: uuid)?.description, "Brand new description")
        XCTAssertEqual(store.part(named: "UP-A")?.description, "Brand new description")
        XCTAssertEqual(store.registeredText(uuid: uuid), newText)
        XCTAssertEqual(try fileContents(uuid), newText)
        XCTAssertEqual(part.drawer, ["User Library", "Gates"])
        XCTAssertEqual(store.listing(of: part), .user)
        // Listed once, found by its new description.
        XCTAssertEqual(store.allParts.filter { $0.uuid == uuid }.count, 1)
        var filter = DrawerFilter()
        filter.query = "Brand new description"
        XCTAssertTrue(store.groups(matching: filter).flatMap(\.allParts).contains(part))
        // The old value is still readable.
        XCTAssertEqual(old.description, "Single 2-input NAND gate")
        XCTAssertEqual(old.pins.count, 5)
        // The backend finds the new one by name and uuid.
        let byName = try XCTUnwrap(part_library_find(store.library.pointer, "UP-A"))
        XCTAssertEqual(String(cString: byName.pointee.description), "Brand new description")
    }

    func testARenameFreesTheOldNameAndSuffixesATakenOne() throws {
        let store = makeStore()
        let uuid = "D2000000-0000-4000-8000-000000000001"
        try store.addToUserLibrary(fileText: text(uuid, "UP-B"), source: "b")
        let renamedText = try renamed(store, uuid, "UP-B-RENAMED")
        let part = try store.updateUserLibraryPart(uuid: uuid, fileText: renamedText)
        XCTAssertEqual(part.name, "UP-B-RENAMED")
        XCTAssertNil(store.part(named: "UP-B"))
        XCTAssertEqual(store.part(named: "UP-B-RENAMED"), part)
        // A name another part has gets the suffix.
        let taken = try renamed(store, uuid, "SN74HC00N")
        let suffixed = try store.updateUserLibraryPart(uuid: uuid, fileText: taken)
        XCTAssertEqual(suffixed.name, "SN74HC00N.2")
        XCTAssertEqual(store.part(named: "SN74HC00N")?.uuid, nil)
    }

    func testAnUpdateThatChangesTheKindOrUUIDIsRefused() throws {
        let store = makeStore()
        let uuid = "D3000000-0000-4000-8000-000000000001"
        let old = try store.addToUserLibrary(fileText: text(uuid, "UP-C"), source: "c")
        let before = try fileContents(uuid)
        let otherKind = try mutated(store, uuid) { $0["kind"] = "buffer" }
        XCTAssertThrowsError(try store.updateUserLibraryPart(uuid: uuid, fileText: otherKind)) {
            guard case .identityChanged? = $0 as? PartRegistrationError else { return XCTFail("got \($0)") }
        }
        let otherUUID = try mutated(store, uuid) { $0["uuid"] = "D3000000-0000-4000-8000-0000000000FF" }
        XCTAssertThrowsError(try store.updateUserLibraryPart(uuid: uuid, fileText: otherUUID)) {
            guard case .identityChanged? = $0 as? PartRegistrationError else { return XCTFail("got \($0)") }
        }
        XCTAssertEqual(store.part(uuid: uuid), old)
        XCTAssertEqual(store.registeredText(uuid: uuid), before)
        XCTAssertEqual(try fileContents(uuid), before)
    }

    func testAFilePartIsNotUpdatable() throws {
        let store = makeStore()
        let uuid = "D4000000-0000-4000-8000-000000000001"
        try store.register(fileText: text(uuid, "UP-D"), source: "d")
        let changed = try described(store, uuid, "Changed")
        XCTAssertThrowsError(try store.updateUserLibraryPart(uuid: uuid, fileText: changed)) {
            XCTAssertEqual($0 as? PartRegistrationError, .notInUserLibrary(name: "UP-D"))
        }
    }

    func testARejectedTextKeepsTheOldVersion() throws {
        let store = makeStore()
        let uuid = "D5000000-0000-4000-8000-000000000001"
        let old = try store.addToUserLibrary(fileText: text(uuid, "UP-E"), source: "e")
        let before = try fileContents(uuid)
        let bad = try described(store, uuid, "Bad").replacingOccurrences(of: "\"nand\"", with: "\"nope\"")
        XCTAssertThrowsError(try store.updateUserLibraryPart(uuid: uuid, fileText: bad)) {
            guard case .backend? = $0 as? PartRegistrationError else { return XCTFail("got \($0)") }
        }
        XCTAssertEqual(store.part(uuid: uuid), old)
        XCTAssertEqual(store.part(named: "UP-E")?.description, "Single 2-input NAND gate")
        XCTAssertEqual(try fileContents(uuid), before)
        // The backend still has the old version, found by name and by uuid.
        XCTAssertNotNil(part_library_find_uuid(store.library.pointer, uuid))
        let byName = try XCTUnwrap(part_library_find(store.library.pointer, "UP-E"))
        XCTAssertEqual(String(cString: byName.pointee.description), "Single 2-input NAND gate")
        // And a good update still works afterwards.
        let good = try store.updateUserLibraryPart(uuid: uuid, fileText: try described(store, uuid, "Good"))
        XCTAssertEqual(good.description, "Good")
    }

    func testAnUnwritableFolderChangesNothing() throws {
        let store = makeStore()
        let uuid = "D6000000-0000-4000-8000-000000000001"
        let old = try store.addToUserLibrary(fileText: text(uuid, "UP-F"), source: "f")
        let changed = try described(store, uuid, "Changed")
        try FileManager.default.removeItem(at: directory)
        try Data("in the way".utf8).write(to: directory)
        XCTAssertThrowsError(try store.updateUserLibraryPart(uuid: uuid, fileText: changed)) {
            guard case .libraryWrite? = $0 as? PartRegistrationError else { return XCTFail("got \($0)") }
        }
        XCTAssertEqual(store.part(uuid: uuid), old)
        XCTAssertEqual(store.part(uuid: uuid)?.description, "Single 2-input NAND gate")
    }

    func testARelaunchListsTheNewVersion() throws {
        let store = makeStore()
        let uuid = "D7000000-0000-4000-8000-000000000001"
        try store.addToUserLibrary(fileText: text(uuid, "UP-G"), source: "g")
        try store.updateUserLibraryPart(uuid: uuid, fileText: try described(store, uuid, "Version two"))
        let relaunched = makeStore()
        XCTAssertEqual(relaunched.part(uuid: uuid)?.description, "Version two")
        XCTAssertEqual(relaunched.userLibraryProblems, [])
    }

    func testTheSameTextIsANoOpAndTheCallbackFiresOnceForARealUpdate() throws {
        let store = makeStore()
        let uuid = "D8000000-0000-4000-8000-000000000001"
        let old = try store.addToUserLibrary(fileText: text(uuid, "UP-H"), source: "h")
        var calls: [(String, String)] = []
        store.partUpdated = { calls.append(($0, $1.description)) }
        XCTAssertEqual(try store.updateUserLibraryPart(uuid: uuid, fileText: try XCTUnwrap(store.registeredText(uuid: uuid))), old)
        XCTAssertEqual(calls.count, 0)
        try store.updateUserLibraryPart(uuid: uuid, fileText: try described(store, uuid, "Changed"))
        XCTAssertEqual(calls.count, 1)
        XCTAssertEqual(calls.first?.0, uuid)
        XCTAssertEqual(calls.first?.1, "Changed")
    }
}
