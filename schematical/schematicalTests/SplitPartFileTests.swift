import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// A part drawn as its units in a file (plan-67-D): the optional `units`
/// list of a part. A file with no such part is written exactly as before.
@MainActor
final class SplitPartFileTests: XCTestCase {
    /// A split 7400 with B moved, flipped and turned, `2Y` wired to an LED.
    private func sample() throws -> (doc: SchematicDocument, gate: PlacedChip) {
        let doc = TestSupport.newDocument()
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 300, y: 300))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 700, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(gate, "2Y")), .pin(try TestSupport.pin(led, "A"))))
        doc.setPartSplit(gate.id, true)
        let b = UnitRef(chipID: gate.id, key: "B")
        let corner = try XCTUnwrap(doc.chips[0].unitPlacements.first { $0.key == "B" }).origin
        doc.moveUnit(b, to: CGPoint(x: corner.x + 200, y: corner.y - 40))
        doc.flipUnit(b)
        doc.rotateUnit(b)
        doc.endCoalescing()
        return (doc, try XCTUnwrap(doc.chips.first))
    }

    /// How many parts of a saved file carry a `units` key, read from the JSON
    /// itself (the board's settings have a `units` of their own).
    private static func partUnitKeys(in data: Data) -> Int {
        let json = (try? JSONSerialization.jsonObject(with: data)) as? [String: Any]
        let parts = json?["parts"] as? [[String: Any]] ?? []
        return parts.filter { $0["units"] != nil }.count
    }

    private func open(_ data: Data) throws -> SchematicDocument {
        let opened = TestSupport.newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        return opened
    }

    func testSaveAndOpen() throws {
        let (doc, gate) = try sample()
        let opened = try open(SchematicFile(document: doc, fileURL: nil).encoded())
        let again = try XCTUnwrap(opened.chips.first { $0.id == gate.id })
        XCTAssertTrue(again.isSplit)
        XCTAssertEqual(again.unitPlacements, gate.unitPlacements)
        XCTAssertEqual(again.origin, gate.origin)
        XCTAssertEqual(opened.wires.count, 1)
        let wire = try XCTUnwrap(doc.wires.first), same = try XCTUnwrap(opened.wires.first)
        XCTAssertEqual(same.start, wire.start)
        XCTAssertEqual(opened.endTip(same.start), doc.endTip(wire.start))
        XCTAssertEqual(opened.path(for: same), doc.path(for: wire))
    }

    func testKeyOmittedForABlock() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 300, y: 300))
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        XCTAssertEqual(Self.partUnitKeys(in: data), 0)
        XCTAssertNil(try SchematicFile.decode(data).parts[0].units)
        // Split: the key is there. Joined again: gone, and the bytes are the first save's.
        doc.setPartSplit(doc.chips[0].id, true)
        XCTAssertEqual(Self.partUnitKeys(in: try SchematicFile(document: doc, fileURL: nil).encoded()), 1)
        doc.setPartSplit(doc.chips[0].id, false)
        XCTAssertEqual(try SchematicFile(document: doc, fileURL: nil).encoded(), data)
    }

    func testWrittenShape() throws {
        let (doc, gate) = try sample()
        let part = try XCTUnwrap(try SchematicFile.decode(SchematicFile(document: doc, fileURL: nil).encoded()).parts.first)
        let units = try XCTUnwrap(part.units)
        XCTAssertEqual(units.map(\.unit), ["A", "B", "C", "D", "P"])
        XCTAssertEqual(units.map(\.flipped), [nil, true, nil, nil, nil])
        XCTAssertEqual(units.map(\.rotation), [nil, 90, nil, nil, nil])
        XCTAssertEqual(units.map { CGPoint(x: $0.x, y: $0.y) }, gate.unitPlacements.map(\.origin))
        // The part's own corner is the box around its units; no flip, no turn.
        XCTAssertEqual(CGPoint(x: part.x, y: part.y), gate.origin)
        XCTAssertFalse(part.flipped)
        XCTAssertNil(part.rotation)
    }

    /// One 7400 with `2Y` wired to nothing but a junction, and the given `units`.
    private func file(units: String) -> Data {
        let json = """
        { "format": "\(SchematicFile.formatName)", "version": 5, "nextReference": 2,
          "parts": [{ "id": "11111111-2222-3333-4444-555555555555", "reference": "U1", "partNumber": "SN74HC00N",
                      "x": 100, "y": 200, "flipped": false, "units": \(units) }],
          "junctions": [], "wires": [] }
        """
        return Data(json.utf8)
    }

    func testUnmatchedListIsIgnored() throws {
        let all = ["A", "B", "C", "D", "P"]
        func list(_ keys: [String]) -> String {
            "[" + keys.map { "{\"unit\": \"\($0)\", \"x\": 400, \"y\": 400}" }.joined(separator: ", ") + "]"
        }
        for keys in [Array(all.dropLast()), all + ["Z"], ["A", "A", "C", "D", "P"], []] {
            let opened = try open(file(units: list(keys)))
            let chip = try XCTUnwrap(opened.chips.first)
            XCTAssertFalse(chip.isSplit, "\(keys)")
            XCTAssertEqual(chip.origin, CGPoint(x: 100, y: 200), "\(keys)")
            XCTAssertEqual(chip.layout.slots.count, 14)
        }
        // The matching list, in any order, is taken.
        let opened = try open(file(units: list(all.reversed())))
        XCTAssertEqual(opened.chips.first?.unitPlacements.map(\.key), all)
        // A part with no units ignores the key.
        let resistor = """
        { "format": "\(SchematicFile.formatName)", "version": 5, "nextReference": 1,
          "parts": [{ "id": "\(UUID().uuidString)", "reference": "R1", "partNumber": "GENERIC-RESISTOR",
                      "x": 100, "y": 100, "flipped": false, "units": \(list(["A"])) }],
          "junctions": [], "wires": [] }
        """
        XCTAssertEqual(try open(Data(resistor.utf8)).chips.first?.isSplit, false)
    }

    func testBadUnitRotationIsDropped() throws {
        let units = """
        [{"unit": "A", "x": 100, "y": 100, "rotation": 45}, {"unit": "B", "x": 100, "y": 160, "rotation": -90},
         {"unit": "C", "x": 103, "y": 221, "rotation": 180, "flipped": true}, {"unit": "D", "x": 100, "y": 280},
         {"unit": "P", "x": 100, "y": 340, "rotation": 90}]
        """
        let chip = try XCTUnwrap(try open(file(units: units)).chips.first)
        XCTAssertEqual(chip.unitPlacements.map(\.rotation), [0, 270, 180, 0, 0])
        XCTAssertEqual(chip.unitPlacements.map(\.isFlipped), [false, false, true, false, false])
        // Positions are snapped to the grid, as a part's are.
        XCTAssertEqual(chip.unitPlacements[2].origin, CGPoint(x: 100, y: 220))
    }

    func testSaveOpenSaveIsStable() throws {
        let (doc, _) = try sample()
        let first = try SchematicFile(document: doc, fileURL: nil).encoded()
        let second = try SchematicFile(document: try open(first), fileURL: nil).encoded()
        XCTAssertEqual(first, second)
    }

    /// Every example opens and saves the same bytes twice over, and none gains the key.
    func testExamplesAreUnchanged() throws {
        var root = URL(fileURLWithPath: #filePath)
        for _ in 0..<3 { root.deleteLastPathComponent() }
        let folder = root.appendingPathComponent("examples")
        let files = try XCTUnwrap(FileManager.default.enumerator(at: folder, includingPropertiesForKeys: nil))
            .compactMap { $0 as? URL }.filter { $0.pathExtension == "schematical" }
        XCTAssertGreaterThanOrEqual(files.count, 9)
        for url in files {
            let opened = TestSupport.newDocument()
            try SchematicFile.decode(try Data(contentsOf: url)).apply(to: opened, catalog: TestSupport.catalog, fileURL: url)
            let first = try SchematicFile(document: opened, fileURL: url).encoded()
            XCTAssertEqual(Self.partUnitKeys(in: first), 0, url.lastPathComponent)
            let again = TestSupport.newDocument()
            try SchematicFile.decode(first).apply(to: again, catalog: TestSupport.catalog, fileURL: url)
            XCTAssertEqual(try SchematicFile(document: again, fileURL: url).encoded(), first, url.lastPathComponent)
            XCTAssertTrue(opened.chips.allSatisfy { !$0.isSplit }, url.lastPathComponent)
        }
    }
}
