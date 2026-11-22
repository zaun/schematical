import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Editing a User Library part in the Part Editor: Save updates it, Save As…
/// makes a new one (plan-53-E).
@MainActor
final class PartEditorLibraryTests: XCTestCase {
    private var directory: URL!
    /// Stores, models and controllers made by tests, kept for the life of the
    /// process (freeing one inside a test aborts the host).
    private static var keptStores: [PartCatalogStore] = []
    private static var keptModels: [PartEditorModel] = []
    private static var keptControllers: [SchematicFileController] = []

    override func setUp() async throws {
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("editlib-\(UUID().uuidString)")
    }

    override func tearDown() async throws {
        try? FileManager.default.removeItem(at: directory)
    }

    private func makeStore() -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        Self.keptStores.append(store)
        store.attachUserLibrary(UserPartLibrary(directory: directory))
        return store
    }

    /// A generic 5-pin library part, added to the store's User Library.
    private func addPart(_ store: PartCatalogStore, _ uuid: String, _ name: String) throws -> CatalogPart {
        var draft = PartDraft(newOf: "generic")
        draft.uuid = uuid
        draft.name = name
        draft.function = "Sensor"
        draft.description = "A five-pin part"
        draft.package = "SOT-23"
        draft.pinouts[0].pins = (1...5).map { PinDraft(designator: "\($0)", name: "P\($0)", role: "inout") }
        draft.footprint = try XCTUnwrap(PartValidator.shared.footprints.first { $0.pads.count == 5 }).id
        return try store.addToUserLibrary(fileText: draft.fileText(), source: "\(name).schpart")
    }

    private func model(_ store: PartCatalogStore, _ uuid: String) -> PartEditorModel {
        let model = PartEditorModel(target: PartEditorTarget(source: .libraryPart(uuid: uuid)), catalog: store)
        Self.keptModels.append(model)
        return model
    }

    private func libraryFile(_ uuid: String) throws -> String {
        try String(contentsOf: UserPartLibrary(directory: directory).url(forUUID: uuid), encoding: .utf8)
    }

    func testTheWindowEditsTheLibraryPartAsItIs() throws {
        let store = makeStore()
        let uuid = "F1000000-0000-4000-8000-000000000001"
        try addPart(store, uuid, "ED-A")
        let editor = model(store, uuid)
        XCTAssertNil(editor.openError)
        XCTAssertEqual(editor.libraryUUID, uuid)
        XCTAssertFalse(editor.isNewPart)
        XCTAssertEqual(editor.mode, .allFields)
        XCTAssertEqual(editor.draft.uuid, uuid)
        XCTAssertEqual(editor.draft.name, "ED-A")
        XCTAssertFalse(editor.isDirty)
        XCTAssertNil(editor.fileURL)
    }

    func testAnUnknownOrRemovedPartSetsOpenError() throws {
        let store = makeStore()
        XCTAssertEqual(model(store, "F1000000-0000-4000-8000-0000000000FF").openError,
                       "That part is not in your User Library.")
        let uuid = "F1000000-0000-4000-8000-000000000002"
        try addPart(store, uuid, "ED-A2")
        try store.removeFromUserLibrary(uuid: uuid)
        let editor = model(store, uuid)
        XCTAssertNotNil(editor.openError)
        XCTAssertNil(editor.libraryUUID)
    }

    func testSaveUpdatesThePartInPlaceAndOpenSchematicsFollow() throws {
        let store = makeStore()
        let uuid = "F2000000-0000-4000-8000-000000000001"
        let v1 = try addPart(store, uuid, "ED-B")
        let doc = TestSupport.newDocument()
        Self.keptControllers.append(SchematicFileController(document: doc, catalog: store, willReplaceDocument: {}))
        doc.placeChip(part: v1, symbol: symbol(for: v1), centeredAt: CGPoint(x: 200, y: 200))
        let chipID = try XCTUnwrap(doc.chips.last?.id)

        let editor = model(store, uuid)
        editor.draft.pinouts[0].pins[0].name = "AA"
        XCTAssertTrue(editor.isDirty)
        let part = try editor.saveToLibrary()
        XCTAssertEqual(part.uuid, uuid)
        XCTAssertEqual(part.pins.first?.name, "AA")
        XCTAssertEqual(store.part(uuid: uuid)?.pins.first?.name, "AA")
        XCTAssertEqual(try libraryFile(uuid), store.registeredText(uuid: uuid))
        XCTAssertTrue(try libraryFile(uuid).contains("\"AA\""))
        XCTAssertEqual(store.allParts.filter { $0.uuid == uuid }.count, 1)
        XCTAssertEqual(store.userLibraryItems.count, 1)
        XCTAssertEqual(doc.chips.first { $0.id == chipID }?.part.pins.first?.name, "AA")
        XCTAssertFalse(editor.isDirty)
        XCTAssertEqual(editor.status, "Updated ED-B in your User Library")
        XCTAssertEqual(editor.libraryUUID, uuid)
    }

    func testSaveAsAddsANewPartAndRebindsTheWindow() throws {
        let store = makeStore()
        let uuid = "F3000000-0000-4000-8000-000000000001"
        try addPart(store, uuid, "ED-C")
        let firstText = try XCTUnwrap(store.registeredText(uuid: uuid))
        let firstFile = try libraryFile(uuid)

        let editor = model(store, uuid)
        editor.draft.description = "A second version"
        let second = try editor.saveAsNewLibraryPart()
        let secondUUID = try XCTUnwrap(second.uuid)
        XCTAssertNotEqual(secondUUID, uuid)
        // The first part is exactly as it was.
        XCTAssertEqual(store.registeredText(uuid: uuid), firstText)
        XCTAssertEqual(try libraryFile(uuid), firstFile)
        XCTAssertEqual(store.part(uuid: uuid)?.description, "A five-pin part")
        // The second records the first as its origin, and is named as the library names it.
        let stored = try PartFileObject(text: try XCTUnwrap(store.registeredText(uuid: secondUUID)))
        XCTAssertEqual(stored.object["originId"] as? String, uuid)
        XCTAssertEqual(second.name, "ED-C.2")
        XCTAssertEqual(editor.draft.name, "ED-C.2")
        XCTAssertEqual(editor.libraryUUID, secondUUID)
        XCTAssertEqual(store.userLibraryItems.count, 2)
        XCTAssertEqual(editor.status, "Added ED-C.2 to your User Library")
        XCTAssertFalse(editor.isDirty)
        // A following Save changes the second part only.
        editor.draft.description = "A third version"
        let updated = try editor.saveToLibrary()
        XCTAssertEqual(updated.uuid, secondUUID)
        XCTAssertEqual(store.part(uuid: secondUUID)?.description, "A third version")
        XCTAssertEqual(store.part(uuid: uuid)?.description, "A five-pin part")
        XCTAssertEqual(try libraryFile(uuid), firstFile)
    }

    func testABadDraftChangesNothing() throws {
        let store = makeStore()
        let uuid = "F4000000-0000-4000-8000-000000000001"
        let part = try addPart(store, uuid, "ED-D")
        let before = try libraryFile(uuid)
        let editor = model(store, uuid)
        editor.draft.footprint = "NO-SUCH-FOOTPRINT"
        XCTAssertThrowsError(try editor.saveToLibrary()) {
            guard case .backend? = $0 as? PartRegistrationError else { return XCTFail("got \($0)") }
        }
        XCTAssertThrowsError(try editor.saveAsNewLibraryPart())
        XCTAssertEqual(store.part(uuid: uuid), part)
        XCTAssertEqual(try libraryFile(uuid), before)
        XCTAssertEqual(store.userLibraryItems.count, 1)
        XCTAssertEqual(editor.libraryUUID, uuid)
    }

    func testSaveAfterTheRemovalThrowsNotInUserLibrary() throws {
        let store = makeStore()
        let uuid = "F5000000-0000-4000-8000-000000000001"
        try addPart(store, uuid, "ED-E")
        let editor = model(store, uuid)
        try store.removeFromUserLibrary(uuid: uuid)
        editor.draft.description = "Too late"
        XCTAssertThrowsError(try editor.saveToLibrary()) {
            XCTAssertEqual($0 as? PartRegistrationError, .notInUserLibrary(name: "ED-E"))
        }
    }

    func testOpeningAFileLeavesTheLibrary() throws {
        let store = makeStore()
        let uuid = "F6000000-0000-4000-8000-000000000001"
        try addPart(store, uuid, "ED-F")
        let editor = model(store, uuid)
        XCTAssertEqual(editor.libraryUUID, uuid)
        try editor.load(text: try XCTUnwrap(store.registeredText(uuid: uuid)), from: nil)
        XCTAssertNil(editor.libraryUUID)
    }
}
