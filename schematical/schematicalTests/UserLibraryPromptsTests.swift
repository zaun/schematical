import Foundation
import XCTest
@testable import schematical

/// The decisions behind the User Library's offers (plan-53-C).
@MainActor
final class UserLibraryPromptsTests: XCTestCase {
    private var directory: URL!
    /// Stores made by tests, kept for the life of the process (freeing one
    /// inside a test aborts the host; see `TestSupport.newDocument()`).
    private static var kept: [PartCatalogStore] = []

    override func setUp() async throws {
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("prompts-\(UUID().uuidString)")
    }

    override func tearDown() async throws {
        try? FileManager.default.removeItem(at: directory)
    }

    private func makeStore(folder: URL? = nil) -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        store.attachUserLibrary(UserPartLibrary(directory: folder ?? directory))
        return store
    }

    private func fileText(_ uuid: String, _ name: String, description: String = "Single 2-input NAND gate") -> String {
        PartRegistrationTests.fileText(uuid: uuid, name: name, description: description)
    }

    private func libraryFiles() -> [String] {
        ((try? FileManager.default.contentsOfDirectory(atPath: directory.path)) ?? []).sorted()
    }

    /// An `ask` that answers `choice` and records the names it was asked about.
    private final class Asker {
        var asked: [String] = []
        let choice: LibraryChoice
        init(_ choice: LibraryChoice) { self.choice = choice }
        func ask(_ name: String) -> LibraryChoice {
            asked.append(name)
            return choice
        }
    }

    func testAddListsInTheUserLibraryAndWritesTheFile() async throws {
        let store = makeStore()
        let uuid = "C1000000-0000-4000-8000-000000000001"
        let asker = Asker(.add)
        let outcome = try await PartFileLoading.load(text: fileText(uuid, "PR-A"), source: "a", catalog: store,
                                                     ask: asker.ask)
        guard case let .added(part) = outcome else { return XCTFail("expected added, got \(outcome)") }
        XCTAssertEqual(asker.asked, ["PR-A"])
        XCTAssertEqual(store.listing(of: part), .user)
        XCTAssertEqual(libraryFiles(), ["\(uuid).schpart"])
    }

    func testUseOnceListsInTheFileLibraryAndWritesNothing() async throws {
        let store = makeStore()
        let uuid = "C2000000-0000-4000-8000-000000000001"
        let outcome = try await PartFileLoading.load(text: fileText(uuid, "PR-B"), source: "b", catalog: store,
                                                     ask: Asker(.useOnce).ask)
        guard case let .usedOnce(part) = outcome else { return XCTFail("expected usedOnce, got \(outcome)") }
        XCTAssertEqual(store.listing(of: part), .file)
        XCTAssertEqual(libraryFiles(), [])
    }

    func testCancelRegistersNothing() async throws {
        let store = makeStore()
        let uuid = "C3000000-0000-4000-8000-000000000001"
        let outcome = try await PartFileLoading.load(text: fileText(uuid, "PR-C"), source: "c", catalog: store,
                                                     ask: Asker(.cancel).ask)
        guard case .cancelled = outcome else { return XCTFail("expected cancelled, got \(outcome)") }
        XCTAssertNil(store.part(uuid: uuid))
        XCTAssertEqual(libraryFiles(), [])
    }

    func testAPartAlreadyInTheLibraryIsReportedWithoutAsking() async throws {
        let store = makeStore()
        let uuid = "C4000000-0000-4000-8000-000000000001"
        let text = fileText(uuid, "PR-D")
        let added = try store.addToUserLibrary(fileText: text, source: "d")
        let asker = Asker(.cancel)
        let outcome = try await PartFileLoading.load(text: text, source: "d", catalog: store, ask: asker.ask)
        guard case let .alreadyInLibrary(part) = outcome else { return XCTFail("expected alreadyInLibrary, got \(outcome)") }
        XCTAssertEqual(part, added)
        XCTAssertEqual(asker.asked, [])
    }

    func testSameUUIDOtherContentAsksAndGetsAFreshUUID() async throws {
        let store = makeStore()
        let uuid = "C5000000-0000-4000-8000-000000000001"
        let first = try store.addToUserLibrary(fileText: fileText(uuid, "PR-E"), source: "e")
        let asker = Asker(.add)
        let outcome = try await PartFileLoading.load(
            text: fileText(uuid, "PR-E2", description: "Changed"), source: "e2", catalog: store, ask: asker.ask)
        guard case let .added(part) = outcome else { return XCTFail("expected added, got \(outcome)") }
        XCTAssertEqual(asker.asked, ["PR-E2"])
        XCTAssertNotEqual(part.uuid, uuid)
        XCTAssertEqual(store.part(uuid: uuid), first)
        XCTAssertEqual(libraryFiles().count, 2)
    }

    func testABadFileThrowsTheBackendMessage() async throws {
        let store = makeStore()
        let bad = fileText("C6000000-0000-4000-8000-000000000001", "PR-F")
            .replacingOccurrences(of: "\"nand\"", with: "\"nope\"")
        do {
            _ = try await PartFileLoading.load(text: bad, source: "f.schpart", catalog: store, ask: Asker(.add).ask)
            XCTFail("expected an error")
        } catch let error as PartRegistrationError {
            guard case .backend(let message) = error else { return XCTFail("expected backend, got \(error)") }
            XCTAssertTrue(message.contains("f.schpart"), message)
        }
        do {
            _ = try await PartFileLoading.load(text: "not json", source: "g", catalog: store, ask: Asker(.add).ask)
            XCTFail("expected an error")
        } catch let error as PartRegistrationError {
            XCTAssertEqual(error, .notAPartFile("not a JSON object"))
        }
        XCTAssertEqual(libraryFiles(), [])
    }

    /// `apply` returns the embedded parts once each; `offerable` drops the one
    /// already in the User Library.
    func testApplyReturnsTheEmbeddedPartsAndOfferableDropsLibraryOnes() throws {
        let store = makeStore()
        let a = "C7000000-0000-4000-8000-000000000001"
        let b = "C7000000-0000-4000-8000-000000000002"
        func placed(_ id: String, _ uuid: String, _ name: String, _ reference: String) -> [String: Any] {
            ["id": id, "reference": reference, "partNumber": name, "partUUID": uuid, "x": 200, "y": 120,
             "flipped": false]
        }
        let object: [String: Any] = [
            "format": "schematical", "version": 5, "nextReference": 4, "junctions": [], "wires": [],
            "customParts": [
                try JSONSerialization.jsonObject(with: Data(fileText(a, "PR-G1").utf8)),
                try JSONSerialization.jsonObject(with: Data(fileText(b, "PR-G2").utf8)),
            ],
            "parts": [
                placed("11111111-0000-4000-8000-0000000000A1", a, "PR-G1", "U1"),
                placed("11111111-0000-4000-8000-0000000000A2", a, "PR-G1", "U2"),
                placed("11111111-0000-4000-8000-0000000000A3", b, "PR-G2", "U3"),
            ],
        ]
        let data = try JSONSerialization.data(withJSONObject: object, options: [.sortedKeys])
        let document = TestSupport.newDocument()
        let parts = try SchematicFile.decode(data).apply(to: document, catalog: store, fileURL: nil)
        XCTAssertEqual(parts.map(\.name), ["PR-G1", "PR-G2"])
        XCTAssertEqual(UserLibraryPrompts.offerable(parts, catalog: store).map(\.name), ["PR-G1", "PR-G2"])
        try store.addToUserLibrary(parts[0])
        XCTAssertEqual(UserLibraryPrompts.offerable(parts, catalog: store).map(\.name), ["PR-G2"])
    }

    /// `add` returns a failure for a part it cannot add and still adds the
    /// others; with an unwritable folder every part fails.
    func testAddCollectsFailuresAndAddsTheRest() throws {
        let store = makeStore()
        let uuid = "C8000000-0000-4000-8000-000000000001"
        let custom = try store.register(fileText: fileText(uuid, "PR-H"), source: "h")
        let bundled = try XCTUnwrap(store.part(named: "SN74HC00N"))
        let failures = UserLibraryPrompts.add([bundled, custom], catalog: store)
        XCTAssertEqual(failures.map(\.name), ["SN74HC00N"])
        XCTAssertEqual(store.listing(of: custom), .user)

        let blocked = FileManager.default.temporaryDirectory.appendingPathComponent("blocked-\(UUID().uuidString)")
        try Data("in the way".utf8).write(to: blocked)
        defer { try? FileManager.default.removeItem(at: blocked) }
        let other = makeStore(folder: blocked)
        let one = try other.register(fileText: fileText("C8000000-0000-4000-8000-000000000002", "PR-H2"), source: "h2")
        let two = try other.register(fileText: fileText("C8000000-0000-4000-8000-000000000003", "PR-H3"), source: "h3")
        let all = UserLibraryPrompts.add([one, two], catalog: other)
        XCTAssertEqual(all.map(\.name), ["PR-H2", "PR-H3"])
        XCTAssertEqual(other.listing(of: one), .file)
    }
}
