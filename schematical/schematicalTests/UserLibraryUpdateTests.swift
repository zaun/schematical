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

    // MARK: Phase 3 — open schematics follow

    /// A generic part file with `pins` pins on a bundled footprint with as many
    /// pads (two versions of one part differ in pin count).
    private func genericText(_ uuid: String, _ name: String, pins: Int) throws -> String {
        var draft = PartDraft(newOf: "generic")
        draft.uuid = uuid
        draft.name = name
        draft.function = "Sensor"
        draft.description = "A \(pins)-pin part"
        draft.package = "SOT-23"
        draft.pinouts[0].pins = (1...pins).map { PinDraft(designator: "\($0)", name: "P\($0)", role: "inout") }
        draft.footprint = try XCTUnwrap(PartValidator.shared.footprints.first { $0.pads.count == pins }).id
        return try PartFileObject(text: draft.fileText()).canonicalText
    }

    private final class StopCounter { var count = 0 }

    private static var keptControllers: [SchematicFileController] = []

    private func controller(_ doc: SchematicDocument, _ store: PartCatalogStore,
                            stops: StopCounter) -> SchematicFileController {
        let controller = SchematicFileController(document: doc, catalog: store, willReplaceDocument: { stops.count += 1 })
        Self.keptControllers.append(controller)
        return controller
    }

    private func place(_ doc: SchematicDocument, _ part: CatalogPart, at point: CGPoint) throws -> PlacedChip {
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return try XCTUnwrap(doc.chips.last)
    }

    func testOpenDocumentsFollowAnUpdate() throws {
        let store = makeStore()
        let uuid = "E1000000-0000-4000-8000-000000000001"
        let v1 = try store.addToUserLibrary(fileText: try genericText(uuid, "UP-GEN", pins: 5), source: "g")
        XCTAssertEqual(v1.pins.count, 5)

        // Two documents place the part; one has wires on its pins 1 and 5.
        let stopsA = StopCounter(), stopsB = StopCounter(), stopsC = StopCounter()
        let docA = TestSupport.newDocument(), docB = TestSupport.newDocument(), docC = TestSupport.newDocument()
        _ = controller(docA, store, stops: stopsA)
        _ = controller(docB, store, stops: stopsB)
        _ = controller(docC, store, stops: stopsC)
        let chipA = try place(docA, v1, at: CGPoint(x: 200, y: 200))
        let other = try TestSupport.place(docA, "SN74HC00N", at: CGPoint(x: 500, y: 200))
        let pin1 = PinRef(chipID: chipA.id, index: 0), pin5 = PinRef(chipID: chipA.id, index: 4)
        XCTAssertNil(docA.connect(.pin(pin1), .pin(try TestSupport.pin(other, "1A"))))
        let wire1 = try XCTUnwrap(docA.wires.last?.id)
        XCTAssertNil(docA.connect(.pin(pin5), .pin(try TestSupport.pin(other, "1B"))))
        let wire5 = try XCTUnwrap(docA.wires.last?.id)
        let tip5 = try XCTUnwrap(docA.pinTip(pin5))
        let chipB = try place(docB, v1, at: CGPoint(x: 300, y: 300))
        _ = try TestSupport.place(docC, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        let chipsC = docC.chips.map(\.id)
        XCTAssertNotNil(docA.undoName)

        let v2 = try store.updateUserLibraryPart(uuid: uuid, fileText: try genericText(uuid, "UP-GEN", pins: 3))
        XCTAssertEqual(v2.pins.count, 3)

        // Both documents that place the part now hold the new version.
        let newA = try XCTUnwrap(docA.chips.first { $0.id == chipA.id })
        XCTAssertEqual(newA.part.pins.count, 3)
        XCTAssertEqual(newA.part.description, "A 3-pin part")
        XCTAssertEqual(docB.chips.first { $0.id == chipB.id }?.part.pins.count, 3)
        // The chip keeps its identity and place.
        XCTAssertEqual(newA.reference, chipA.reference)
        XCTAssertEqual(newA.origin, chipA.origin)
        // The wire on pin 1 is still on the chip; the one on pin 5 ends loose at its old tip.
        let w1 = try XCTUnwrap(docA.wires.first { $0.id == wire1 })
        XCTAssertTrue([w1.start, w1.end].contains(.pin(pin1)))
        let w5 = try XCTUnwrap(docA.wires.first { $0.id == wire5 })
        let loose = [w5.start, w5.end].compactMap { end -> Junction? in
            if case let .junction(id) = end { return docA.junctions.first { $0.id == id } }
            return nil
        }
        XCTAssertEqual(loose.count, 1)
        XCTAssertEqual(loose.first?.isLoose, true)
        XCTAssertEqual(loose.first?.point, tip5)
        // The undo history ends at the update; each affected window's simulation was stopped once.
        XCTAssertNil(docA.undoName)
        XCTAssertNil(docB.undoName)
        XCTAssertEqual(stopsA.count, 1)
        XCTAssertEqual(stopsB.count, 1)
        // A document without the part is untouched.
        XCTAssertEqual(stopsC.count, 0)
        XCTAssertEqual(docC.chips.map(\.id), chipsC)
        XCTAssertNotNil(docC.undoName)
    }

    func testTheUpdatedChipKeepsItsBoardPad1() throws {
        let store = makeStore()
        let uuid = "E2000000-0000-4000-8000-000000000001"
        let v1 = try store.addToUserLibrary(fileText: try genericText(uuid, "UP-GEN2", pins: 5), source: "g")
        let doc = TestSupport.newDocument()
        _ = controller(doc, store, stops: StopCounter())
        let chip = try place(doc, v1, at: CGPoint(x: 200, y: 200))
        let board = try XCTUnwrap(chip.board, "a placed part has a board placement")
        let pad1 = try XCTUnwrap(chip.footprint?.pads.first { $0.designator == chip.part.firstPinDesignator })
        let before = board.board(pad1.centre)

        try store.updateUserLibraryPart(uuid: uuid, fileText: try genericText(uuid, "UP-GEN2", pins: 3))
        let after = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
        XCTAssertNotEqual(after.footprint?.id, chip.footprint?.id)
        let newPad1 = try XCTUnwrap(after.footprint?.pads.first { $0.designator == after.part.firstPinDesignator })
        let landed = try XCTUnwrap(after.board).board(newPad1.centre)
        XCTAssertEqual(landed.x, before.x, accuracy: 0.001)
        XCTAssertEqual(landed.y, before.y, accuracy: 0.001)
    }
}
