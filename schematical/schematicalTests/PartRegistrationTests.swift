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
        XCTAssertTrue(store.categories.contains("Custom A"))
        XCTAssertEqual(part.footprint?.id, "CUSTOM-A0000000")
        XCTAssertEqual(part.footprint?.pads.count, 5)
        let listed = store.groups(matching: DrawerFilter()).first { $0.category == "Custom A" }
        XCTAssertNotNil(listed)
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

    /// (c) A different file with a registered uuid is refused.
    func testSameUUIDOtherContentIsRefused() throws {
        let uuid = "C0000000-0000-4000-8000-000000000001"
        try store.register(fileText: Self.fileText(uuid: uuid, name: "REG-C"), source: "c.schpart")
        XCTAssertThrowsError(try store.register(
            fileText: Self.fileText(uuid: uuid, name: "REG-C", description: "Changed"), source: "c2.schpart")) {
            XCTAssertEqual($0 as? PartRegistrationError, .uuidInUse(uuid: uuid, name: "REG-C"))
        }
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
