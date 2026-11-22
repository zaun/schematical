import Foundation
import XCTest
@testable import schematical

/// Custom parts registered at run time from single part files (plan-32-A).
@MainActor
final class PartRegistrationTests: XCTestCase {
    /// A catalog of its own, so the parts these tests add stay out of the
    /// shared one other tests read.
    private static let store: PartCatalogStore = {
        let store = PartCatalogStore()
        store.load()
        return store
    }()

    /// A single part file: a copy of the bundled 1G00 NAND device with one
    /// part, on the bundled SOT23-5 footprint or on an inline one.
    static func fileText(uuid: String, name: String, drawer: [String] = ["Custom", "Gates"],
                         description: String = "Single 2-input NAND gate",
                         inlineFootprint: Bool = false, gateInput: String = "B") -> String {
        let footprintID = "CUSTOM-" + uuid.prefix(8)
        let names = ["A", "B", "GND", "Y", "VCC"]
        let roles = ["input", "input", "ground", "output", "power"]
        var pins: [[String: Any]] = []
        for index in 0..<5 {
            let pin: [String: Any] = ["number": index + 1, "name": names[index], "role": roles[index]]
            pins.append(pin)
        }
        let gate: [String: Any] = ["op": "nand", "inputs": ["A", gateInput], "output": "Y"]
        let timing: [String: Any] = ["HC": ["rise": 9, "fall": 9, "vcc": [2, 6]] as [String: Any]]
        let part: [String: Any] = ["name": name, "manufacturer": "Generic", "package": "SOT-23", "pinout": "p5",
                                   "footprint": inlineFootprint ? footprintID : "SOT23-5", "family": "HC"]
        let device: [String: Any] = ["id": "1G00", "function": "NAND", "description": description,
                                     "drawer": drawer, "pinouts": ["p5": pins], "gates": [gate],
                                     "timing": timing, "parts": [part]]
        var file: [String: Any] = ["format": "schematical-part", "version": 1, "uuid": uuid, "kind": "gate",
                                   "devices": [device]]
        if inlineFootprint {
            let centres: [(Double, Double)] = [(-1, -1), (-1, 0), (-1, 1), (1, 1), (1, -1)]
            var pads: [[String: Any]] = []
            for (index, centre) in centres.enumerated() {
                let pad: [String: Any] = ["pin": index + 1, "x": centre.0, "y": centre.1,
                                          "width": 0.6, "height": 0.6, "shape": "rect"]
                pads.append(pad)
            }
            let footprint: [String: Any] = ["id": footprintID, "description": "Test pads", "style": "explicit",
                                            "pins": 5, "body": ["width": 3.0, "height": 3.0], "pads": pads]
            file["footprints"] = [footprint]
        }
        let data = try! JSONSerialization.data(withJSONObject: file, options: [.sortedKeys])
        return String(decoding: data, as: UTF8.self)
    }

    private var store: PartCatalogStore { Self.store }

    /// (a) A valid file registers; `part(named:)` returns it and its drawer
    /// path is listed.
    func testValidFileRegisters() throws {
        let uuid = "A0000000-0000-4000-8000-000000000001"
        let part = try store.register(fileText: Self.fileText(uuid: uuid, name: "REG-A", drawer: ["Custom A", "Gates"],
                                                              inlineFootprint: true),
                                      source: "a.schpart")
        XCTAssertEqual(part.name, "REG-A")
        XCTAssertEqual(part.uuid, uuid)
        XCTAssertEqual(store.part(named: "REG-A"), part)
        XCTAssertEqual(store.part(uuid: uuid), part)
        // plan-53-B: every registered custom part lists under File Library,
        // below it the file's own sub-group.
        XCTAssertFalse(store.categories.contains("Custom A"))
        XCTAssertTrue(store.categories.contains("File Library"))
        XCTAssertEqual(part.drawer, ["File Library", "Gates"])
        XCTAssertEqual(store.listing(of: part), .file)
        XCTAssertEqual(part.footprint?.id, "CUSTOM-A0000000")
        XCTAssertEqual(part.footprint?.pads.count, 5)
        let listed = try XCTUnwrap(store.groups(matching: DrawerFilter()).first { $0.category == "File Library" })
        XCTAssertTrue(listed.allParts.contains(part))
        XCTAssertEqual(listed.subcategories.first { $0.subcategory == "Gates" }?.allParts.contains(part), true)
    }

    /// (a2) A one-level drawer lists directly under File Library; a bundled
    /// part has no listing.
    func testOneLevelDrawerListsDirectlyUnderFileLibrary() throws {
        let uuid = "A0000000-0000-4000-8000-000000000002"
        let part = try store.register(fileText: Self.fileText(uuid: uuid, name: "REG-A2", drawer: ["Custom"]),
                                      source: "a2.schpart")
        XCTAssertEqual(part.drawer, ["File Library"])
        let category = try XCTUnwrap(store.groups(matching: DrawerFilter()).first { $0.category == "File Library" })
        XCTAssertTrue(category.functions.flatMap(\.parts).contains(part))
        XCTAssertNil(store.listing(of: try XCTUnwrap(store.part(named: "SN74HC00N"))))
    }

    /// (a3) A removed part is out of the drawer but still resolves; the same
    /// content registered again lists it as a File Library part.
    func testRemovedPartIsHiddenButResolvesAndRelists() throws {
        let uuid = "A0000000-0000-4000-8000-000000000003"
        let text = Self.fileText(uuid: uuid, name: "REG-A3")
        let part = try store.register(fileText: text, source: "a3.schpart")
        store.setListing(.removed, for: uuid)
        XCTAssertFalse(store.allParts.contains(part))
        XCTAssertFalse(store.groups(matching: DrawerFilter()).flatMap(\.allParts).contains(part))
        XCTAssertEqual(store.part(named: "REG-A3"), part)
        XCTAssertEqual(store.part(uuid: uuid), part)
        XCTAssertEqual(store.listing(of: part), .removed)
        let again = try store.register(fileText: text, source: "a3.schpart")
        XCTAssertEqual(again, part)
        XCTAssertEqual(store.listing(of: again), .file)
        XCTAssertTrue(store.allParts.contains(again))
        XCTAssertEqual(again.drawer, ["File Library", "Gates"])
    }

    /// (a4) `.user` listings: registering as `.user` lists under User Library;
    /// registering a `file` part as `.user` moves it.
    func testUserListingAndMoveFromFileLibrary() throws {
        let uuid = "A0000000-0000-4000-8000-000000000004"
        let text = Self.fileText(uuid: uuid, name: "REG-A4")
        let file = try store.register(fileText: text, source: "a4.schpart")
        XCTAssertEqual(file.drawer, ["File Library", "Gates"])
        let user = try store.register(fileText: text, source: "a4.schpart", listing: .user)
        XCTAssertEqual(user, file)
        XCTAssertEqual(user.drawer, ["User Library", "Gates"])
        XCTAssertEqual(store.listing(of: user), .user)
        XCTAssertEqual(store.allParts.filter { $0.uuid == uuid }.count, 1)
        // A user part registered as a file part again stays in the User Library.
        XCTAssertEqual(try store.register(fileText: text, source: "a4.schpart").drawer, ["User Library", "Gates"])
        XCTAssertTrue(store.kinds(inCategory: "User Library").contains(PART_GATE.rawValue))
    }

    /// (b) The same file twice returns the registered part, once.
    func testSameFileTwiceReturnsTheRegisteredPart() throws {
        let text = Self.fileText(uuid: "B0000000-0000-4000-8000-000000000001", name: "REG-B")
        let first = try store.register(fileText: text, source: "b.schpart")
        let count = store.partCount
        let second = try store.register(fileText: text, source: "b.schpart")
        XCTAssertEqual(first, second)
        XCTAssertEqual(store.partCount, count)
    }

    /// A registered file's text with the device description replaced.
    private func described(_ text: String, _ description: String) throws -> String {
        var file = try PartFileObject(text: text)
        var devices = try XCTUnwrap(file.object["devices"] as? [[String: Any]])
        devices[0]["description"] = description
        file.object["devices"] = devices
        return file.canonicalText
    }

    private func footprintIDs(_ text: String) throws -> [String] {
        let file = try PartFileObject(text: text)
        return (file.object["footprints"] as? [[String: Any]] ?? []).compactMap { $0["id"] as? String }
    }

    /// (c) A different file with a registered uuid is registered under a new
    /// uuid (plan-53-A): the first part is unchanged, the second keeps the
    /// file's own name, records the first's uuid as `originId`, and has its
    /// inline footprint renamed to the new uuid's.
    func testSameUUIDOtherContentGetsANewUUID() throws {
        let uuid = "C0000000-0000-4000-8000-000000000001"
        let first = try store.register(
            fileText: Self.fileText(uuid: uuid, name: "REG-C", inlineFootprint: true), source: "c.schpart")
        let second = try store.register(
            fileText: Self.fileText(uuid: uuid, name: "REG-C2", description: "Changed", inlineFootprint: true),
            source: "c2.schpart")
        XCTAssertEqual(first.uuid, uuid)
        XCTAssertEqual(store.part(uuid: uuid), first)
        XCTAssertEqual(first.description, "Single 2-input NAND gate")
        let fresh = try XCTUnwrap(second.uuid)
        XCTAssertNotEqual(fresh, uuid)
        XCTAssertEqual(second.name, "REG-C2")
        XCTAssertEqual(second.description, "Changed")
        let text = try XCTUnwrap(store.registeredText(uuid: fresh))
        XCTAssertEqual(try PartFileObject(text: text).object["originId"] as? String, uuid)
        XCTAssertEqual(try footprintIDs(text), ["CUSTOM-" + fresh.prefix(8)])
        XCTAssertEqual(second.footprint?.id, "CUSTOM-" + fresh.prefix(8))
        XCTAssertEqual(try footprintIDs(try XCTUnwrap(store.registeredText(uuid: uuid))), ["CUSTOM-C0000000"])
        // The first file again is still the first part.
        XCTAssertEqual(try store.register(
            fileText: Self.fileText(uuid: uuid, name: "REG-C", inlineFootprint: true), source: "c.schpart"), first)
    }

    /// (c2) A chain of versions keeps the first version's uuid as `originId`.
    func testOriginIdIsTheFirstVersion() throws {
        let uuid = "C1000000-0000-4000-8000-000000000001"
        try store.register(fileText: Self.fileText(uuid: uuid, name: "REG-C3"), source: "v1")
        let v2 = try store.register(
            fileText: Self.fileText(uuid: uuid, name: "REG-C3", description: "Second"), source: "v2")
        let v2uuid = try XCTUnwrap(v2.uuid)
        let v3text = try described(try XCTUnwrap(store.registeredText(uuid: v2uuid)), "Third")
        let v3 = try store.register(fileText: v3text, source: "v3")
        let v3uuid = try XCTUnwrap(v3.uuid)
        XCTAssertNotEqual(v3uuid, v2uuid)
        XCTAssertNotEqual(v3uuid, uuid)
        let stored = try PartFileObject(text: try XCTUnwrap(store.registeredText(uuid: v3uuid)))
        XCTAssertEqual(stored.object["originId"] as? String, uuid)
        XCTAssertEqual(v3.description, "Third")
    }

    /// (c3) `uuidConflicts` is false for an unknown uuid and for the same
    /// content, true for other content under a registered uuid.
    func testUUIDConflicts() throws {
        let uuid = "C2000000-0000-4000-8000-000000000001"
        let text = Self.fileText(uuid: uuid, name: "REG-C4")
        XCTAssertFalse(store.uuidConflicts(text))
        try store.register(fileText: text, source: "c4")
        XCTAssertFalse(store.uuidConflicts(text))
        XCTAssertTrue(store.uuidConflicts(Self.fileText(uuid: uuid, name: "REG-C4", description: "Other")))
    }

    /// (d) A new uuid with a bundled name gets `.2`, the next `.3`; and the
    /// renamed file registered again (as a document saves it) is the same part.
    func testNameClashGetsTheLowestFreeSuffix() throws {
        XCTAssertNotNil(store.part(named: "SN74HC00N"))
        let second = try store.register(
            fileText: Self.fileText(uuid: "D0000000-0000-4000-8000-000000000002", name: "SN74HC00N"), source: "d2")
        let third = try store.register(
            fileText: Self.fileText(uuid: "D0000000-0000-4000-8000-000000000003", name: "SN74HC00N"), source: "d3")
        XCTAssertEqual(second.name, "SN74HC00N.2")
        XCTAssertEqual(third.name, "SN74HC00N.3")
        XCTAssertEqual(store.part(named: "SN74HC00N")?.uuid, nil)
        let text = try XCTUnwrap(store.registeredText(uuid: "D0000000-0000-4000-8000-000000000002"))
        XCTAssertTrue(text.contains("\"SN74HC00N.2\""))
        XCTAssertEqual(try store.register(fileText: text, source: "again"), second)
        // The original, unsuffixed file is the same part too.
        XCTAssertEqual(try store.register(
            fileText: Self.fileText(uuid: "D0000000-0000-4000-8000-000000000002", name: "SN74HC00N"), source: "d2"),
                       second)
    }

    /// (e) A file the backend rejects throws its message and adds nothing.
    func testBackendRejectionChangesNothing() throws {
        let count = store.partCount
        let footprints = part_library_footprint_count(store.library.pointer)
        let names = part_library_count(store.library.pointer)
        XCTAssertThrowsError(try store.register(
            fileText: Self.fileText(uuid: "E0000000-0000-4000-8000-000000000001", name: "REG-E",
                                    inlineFootprint: true, gateInput: "NOPE"),
            source: "e.schpart")) { error in
            guard case let .backend(message)? = error as? PartRegistrationError else {
                return XCTFail("expected .backend, got \(error)")
            }
            XCTAssertTrue(message.hasPrefix("e.schpart: devices[0] (1G00)"), message)
            XCTAssertTrue(message.contains("NOPE"), message)
        }
        XCTAssertEqual(store.partCount, count)
        XCTAssertEqual(part_library_footprint_count(store.library.pointer), footprints)
        XCTAssertEqual(part_library_count(store.library.pointer), names)
        XCTAssertNil(store.part(named: "REG-E"))
        XCTAssertNil(store.part(uuid: "E0000000-0000-4000-8000-000000000001"))
    }

    /// (f) A registered part's kind and pins are the backend's.
    func testRegisteredPartMatchesTheBackend() throws {
        let uuid = "F0000000-0000-4000-8000-000000000001"
        let part = try store.register(fileText: Self.fileText(uuid: uuid, name: "REG-F"), source: "f.schpart")
        let backend = try XCTUnwrap(part_library_find_uuid(store.library.pointer, uuid)).pointee
        XCTAssertEqual(part.kind, backend.kind)
        XCTAssertEqual(part.kind, PART_GATE)
        XCTAssertEqual(part.pins.count, backend.pin_count)
        XCTAssertEqual(part.pins.map(\.name), ["A", "B", "GND", "Y", "VCC"])
        XCTAssertEqual(part.footprint?.id, "SOT23-5")
    }

    /// (g) The Load Part… reader returns a file's text and refuses a file
    /// that is not UTF-8.
    func testPartFileReader() throws {
        let dir = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: dir) }
        let good = dir.appendingPathComponent("good.schpart")
        let text = Self.fileText(uuid: "A1000000-0000-4000-8000-000000000001", name: "REG-G")
        try text.write(to: good, atomically: true, encoding: .utf8)
        XCTAssertEqual(try PartFileReader.read(url: good), text)
        let bad = dir.appendingPathComponent("bad.schpart")
        try Data([0xFF, 0xFE, 0x00, 0xC3]).write(to: bad)
        XCTAssertThrowsError(try PartFileReader.read(url: bad)) {
            guard case .notAPartFile? = $0 as? PartRegistrationError else {
                return XCTFail("expected .notAPartFile, got \($0)")
            }
        }
    }
}
