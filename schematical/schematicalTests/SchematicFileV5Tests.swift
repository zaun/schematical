import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Version 5 schematics carry their custom parts (plan-32-B).
@MainActor
final class SchematicFileV5Tests: XCTestCase {
    /// A catalog of its own: these tests register custom parts.
    private static let store: PartCatalogStore = {
        let store = PartCatalogStore()
        store.load()
        return store
    }()

    private var store: PartCatalogStore { Self.store }

    /// A plain schematic (SN74HC00N, 5V, GENERIC-LED) as the build before
    /// plan-32 saved it after one save → open → save, captured by running that
    /// build's encoder (commit 656b6d1).
    private static let headGolden = #"""
{
  "board" : {
    "color" : "green",
    "grid" : 0.635,
    "height" : 80,
    "layerStyles" : [
      {
        "color" : "#C83737",
        "layer" : "top",
        "type" : "component"
      },
      {
        "color" : "#3A6FD8",
        "layer" : "bottom",
        "type" : "component"
      }
    ],
    "layers" : 2,
    "texts" : [

    ],
    "traces" : [

    ],
    "units" : "mm",
    "width" : 100
  },
  "buses" : [

  ],
  "format" : "schematical",
  "junctions" : [

  ],
  "nextReference" : 4,
  "notes" : [

  ],
  "parts" : [
    {
      "flipped" : false,
      "id" : "CBB811AD-6E14-4245-8683-E61351B9693F",
      "partNumber" : "SN74HC00N",
      "pcb" : {
        "origin" : "pad1Center",
        "outline" : "document",
        "pin1" : "silkscreen",
        "reference" : "silkscreen",
        "rotation" : 0,
        "side" : "top",
        "x" : 0,
        "y" : 0
      },
      "reference" : "U1",
      "x" : 140,
      "y" : -30
    },
    {
      "flipped" : false,
      "id" : "C074A3ED-26D2-4A1C-AFDF-8D281937C1FB",
      "partNumber" : "5V",
      "reference" : "U2",
      "x" : 380,
      "y" : 110
    },
    {
      "color" : "Red",
      "flipped" : false,
      "footprint" : "LED-RADIAL-5MM",
      "id" : "289FEFEA-9C9E-4AAB-8575-165E435B1A71",
      "partNumber" : "GENERIC-LED",
      "pcb" : {
        "origin" : "pad1Center",
        "outline" : "document",
        "pin1" : "silkscreen",
        "reference" : "silkscreen",
        "rotation" : 0,
        "side" : "top",
        "x" : 10.16,
        "y" : -6.985
      },
      "reference" : "U3",
      "x" : 580,
      "y" : 100
    }
  ],
  "version" : 4,
  "wires" : [

  ]
}
"""#

    private func open(_ data: Data, into doc: SchematicDocument? = nil) throws -> SchematicDocument {
        let doc = doc ?? TestSupport.newDocument()
        try SchematicFile.decode(data).apply(to: doc, catalog: store, fileURL: nil)
        return doc
    }

    private func save(_ doc: SchematicDocument) throws -> Data {
        try SchematicFile(document: doc, fileURL: nil, catalog: store).encoded()
    }

    private func json(_ data: Data) throws -> [String: Any] {
        try XCTUnwrap(JSONSerialization.jsonObject(with: data) as? [String: Any])
    }

    /// A version 5 file placing one custom part twice; the part's file is
    /// PartRegistrationTests' fixture.
    private func v5File(uuid: String, name: String, customParts: Bool = true,
                        description: String = "Single 2-input NAND gate") throws -> Data {
        let part = try JSONSerialization.jsonObject(
            with: Data(PartRegistrationTests.fileText(uuid: uuid, name: name, description: description).utf8))
        var file: [String: Any] = [
            "format": "schematical", "version": 5, "nextReference": 3, "junctions": [], "wires": [],
            "parts": [
                ["id": "11111111-0000-4000-8000-000000000001", "reference": "U1", "partNumber": name,
                 "partUUID": uuid, "x": 200, "y": 120, "flipped": false] as [String: Any],
            ],
        ]
        if customParts {
            file["customParts"] = [part]
        }
        return try JSONSerialization.data(withJSONObject: file, options: [.sortedKeys])
    }

    // MARK: Phase 1 — shape

    /// (a) A plain schematic saves as version 4 with no `customParts`, and
    /// byte for byte as the build before plan-32 saved it.
    func testPlainSchematicIsVersion4AsBefore() throws {
        let golden = Data(Self.headGolden.utf8)
        let text = String(decoding: try save(try open(golden)), as: UTF8.self)
        XCTAssertTrue(text.contains("\"version\" : 4"))
        XCTAssertFalse(text.contains("customParts"))
        XCTAssertFalse(text.contains("partUUID"))
        XCTAssertEqual(text, Self.headGolden)
    }

    /// (b) A version 5 file's `customParts` decodes and re-encodes to the same
    /// JSON value.
    func testCustomPartsRoundTripAsJSON() throws {
        let data = try v5File(uuid: "B5000000-0000-4000-8000-000000000001", name: "V5-B")
        let file = try SchematicFile.decode(data)
        XCTAssertEqual(file.customParts?.count, 1)
        XCTAssertEqual(file.parts.first?.partUUID, "B5000000-0000-4000-8000-000000000001")
        let again = try JSONSerialization.jsonObject(with: file.encoded()) as? NSDictionary
        let original = try JSONSerialization.jsonObject(with: data) as? NSDictionary
        XCTAssertEqual(again?["customParts"] as? NSArray, original?["customParts"] as? NSArray)
    }

    /// (c) A version 6 file is refused.
    func testVersion6IsRefused() {
        let data = Data(#"{"format":"schematical","version":6,"nextReference":1,"parts":[],"junctions":[],"wires":[]}"#.utf8)
        XCTAssertThrowsError(try SchematicFile.decode(data)) {
            guard case .newerVersion(6)? = $0 as? SchematicFileError else {
                return XCTFail("expected newerVersion(6), got \($0)")
            }
        }
    }

    // MARK: Phase 2 — open

    /// (d) A file whose custom part is not loaded opens; the placed part is
    /// the embedded one.
    func testOpenRegistersTheEmbeddedPart() throws {
        let uuid = "D5000000-0000-4000-8000-000000000001"
        XCTAssertNil(store.part(uuid: uuid))
        let doc = try open(try v5File(uuid: uuid, name: "V5-D"))
        XCTAssertEqual(doc.chips.count, 1)
        XCTAssertEqual(doc.chips.first?.part.uuid, uuid)
        XCTAssertEqual(doc.chips.first?.part.name, "V5-D")
        XCTAssertEqual(doc.chips.first?.part.pins.map(\.name), ["A", "B", "GND", "Y", "VCC"])
    }

    /// (e) Opening the same file twice adds the part once.
    func testOpeningTwiceAddsThePartOnce() throws {
        let data = try v5File(uuid: "E5000000-0000-4000-8000-000000000001", name: "V5-E")
        _ = try open(data)
        let count = store.partCount
        _ = try open(data)
        XCTAssertEqual(store.partCount, count)
    }

    /// (f) A uuid already loaded with other content refuses the file and
    /// leaves the document alone.
    func testConflictingUUIDLeavesTheDocument() throws {
        let uuid = "F5000000-0000-4000-8000-000000000001"
        try store.register(fileText: PartRegistrationTests.fileText(uuid: uuid, name: "V5-F"), source: "f")
        let doc = TestSupport.newDocument()
        let bundled = try XCTUnwrap(store.part(named: "SN74HC00N"))
        doc.placeChip(part: bundled, symbol: symbol(for: bundled), centeredAt: CGPoint(x: 100, y: 100))
        let before = doc.chips.map(\.id)
        XCTAssertThrowsError(try open(try v5File(uuid: uuid, name: "V5-F", description: "Other"), into: doc)) {
            guard case let .customPartConflict(conflictUUID, name)? = $0 as? SchematicFileError else {
                return XCTFail("expected customPartConflict, got \($0)")
            }
            XCTAssertEqual(conflictUUID, uuid)
            XCTAssertEqual(name, "V5-F")
        }
        XCTAssertEqual(doc.chips.map(\.id), before)
    }

    /// (g) A placed `partUUID` with no `customParts` entry is an unknown part.
    func testMissingCustomPartIsUnknown() throws {
        XCTAssertThrowsError(try open(try v5File(uuid: "A5000000-0000-4000-8000-00000000000F", name: "V5-G",
                                                  customParts: false))) {
            guard case let .unknownParts(names)? = $0 as? SchematicFileError else {
                return XCTFail("expected unknownParts, got \($0)")
            }
            XCTAssertEqual(names, ["V5-G"])
        }
    }

    // MARK: Phase 3 — save

    private func customPart(_ uuid: String, _ name: String) throws -> CatalogPart {
        try store.register(fileText: PartRegistrationTests.fileText(uuid: uuid, name: name), source: name)
    }

    /// (h) Two placements of one custom part embed it once.
    func testTwoPlacementsEmbedThePartOnce() throws {
        let part = try customPart("A5000000-0000-4000-8000-000000000008", "V5-H")
        let doc = TestSupport.newDocument()
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100))
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 300, y: 100))
        let file = try json(try save(doc))
        XCTAssertEqual(file["version"] as? Int, 5)
        let custom = try XCTUnwrap(file["customParts"] as? [[String: Any]])
        XCTAssertEqual(custom.count, 1)
        XCTAssertEqual(custom.first?["uuid"] as? String, "A5000000-0000-4000-8000-000000000008")
    }

    /// (i) Only the custom placement carries `partUUID`; its `partNumber` is
    /// the registered (suffixed) name.
    func testOnlyCustomPlacementsCarryAUUID() throws {
        let part = try customPart("A5000000-0000-4000-8000-000000000009", "SN74HC00N")
        XCTAssertEqual(part.name, "SN74HC00N.2")
        let bundled = try XCTUnwrap(store.part(named: "SN74HC00N"))
        let doc = TestSupport.newDocument()
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100))
        doc.placeChip(part: bundled, symbol: symbol(for: bundled), centeredAt: CGPoint(x: 300, y: 100))
        let parts = try XCTUnwrap(try json(try save(doc))["parts"] as? [[String: Any]])
        let custom = try XCTUnwrap(parts.first { $0["partUUID"] != nil })
        XCTAssertEqual(custom["partNumber"] as? String, "SN74HC00N.2")
        XCTAssertEqual(custom["partUUID"] as? String, "A5000000-0000-4000-8000-000000000009")
        XCTAssertEqual(parts.filter { $0["partUUID"] == nil }.map { $0["partNumber"] as? String }, ["SN74HC00N"])
    }

    /// (j) save → open → save gives the same bytes.
    func testSaveOpenSaveIsStable() throws {
        let part = try customPart("A500000A-0000-4000-8000-000000000001", "V5-J")
        let doc = TestSupport.newDocument()
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100))
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 300, y: 100))
        let first = try save(doc)
        let second = try save(try open(first))
        XCTAssertEqual(String(decoding: second, as: UTF8.self), String(decoding: first, as: UTF8.self))
    }
}
