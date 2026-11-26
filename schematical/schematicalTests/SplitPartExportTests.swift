import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Exports of a part drawn as its units (plan-67-F): the picture exports draw
/// each piece, KiCad gets a multi-unit symbol with one reference, EasyEDA Pro
/// one component whose symbol holds every unit. Documents without such a part
/// export as before.
@MainActor
final class SplitPartExportTests: XCTestCase {
    private func gate(_ doc: SchematicDocument) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.partNumber == "SN74HC00N" })
    }

    private func count(_ needle: String, in text: String) -> Int {
        text.components(separatedBy: needle).count - 1
    }

    // MARK: Pictures

    func testSVGNamesEveryPiece() throws {
        let doc = try ExportSamples.split()
        let svg = String(decoding: try XCTUnwrap(SchematicExport.svg(SchematicScene(doc))), as: UTF8.self)
        for key in ["A", "B", "C", "D", "P"] {
            XCTAssertEqual(count(">U1-\(key)<", in: svg), 1, key)
        }
        XCTAssertEqual(count(">U1<", in: svg), 0)
        // The other parts keep their plain labels.
        XCTAssertEqual(count(">D1<", in: svg), 1)
        // The block of leftover pins carries the part number; the gates do not.
        XCTAssertEqual(count(">SN74HC00N<", in: svg), 1)
    }

    func testBoundsCoverEveryPiece() throws {
        let doc = try ExportSamples.split()
        let bounds = try XCTUnwrap(SchematicExport.bounds(of: SchematicScene(doc)))
        for piece in try gate(doc).pieces {
            XCTAssertTrue(bounds.contains(piece.bodyRect), piece.drawnReference)
        }
    }

    func testUnsplitSceneIsUnchanged() throws {
        // The same document, split and joined again, draws as one never split.
        func build(splitFirst: Bool) throws -> Data {
            let doc = TestSupport.newDocument()
            let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 300, y: 300))
            let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 700, y: 200))
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(gate, "1Y")), .pin(try TestSupport.pin(led, "A"))))
            if splitFirst {
                doc.setPartSplit(gate.id, true)
                doc.setPartSplit(gate.id, false)
            }
            return try XCTUnwrap(SchematicExport.svg(SchematicScene(doc)))
        }
        XCTAssertEqual(try build(splitFirst: true), try build(splitFirst: false))
    }

    // MARK: KiCad

    private func kicadSchematic(_ doc: SchematicDocument, base: String) throws -> String {
        let file = try XCTUnwrap(KiCadExport.files(for: doc, base: base).first { $0.name.hasSuffix(".kicad_sch") })
        return String(decoding: file.data, as: UTF8.self)
    }

    func testKiCadUnits() throws {
        let doc = try ExportSamples.split()
        let chip = try gate(doc)
        let text = try kicadSchematic(doc, base: "split")
        let name = "SN74HC00N_units_An0_Bf90_Cn0_Dn0_Pn0"
        XCTAssertEqual(count("(symbol \"Schematical:\(name)\"\n", in: text), 1, "one library symbol")
        for unit in 1...5 {
            XCTAssertEqual(count("(symbol \"\(name)_\(unit)_1\"", in: text), 1, "unit \(unit)")
        }
        XCTAssertEqual(count("(symbol \"\(name)_0_1\"", in: text), 0)
        XCTAssertEqual(count("(symbol \"\(name)_6_1\"", in: text), 0)
        // Five instances, units 1 to 5, all U1.
        let instances = text.components(separatedBy: "\t(symbol\n").dropFirst()
            .filter { $0.contains("(lib_id \"Schematical:\(name)\")") }
        XCTAssertEqual(instances.count, 5)
        for (index, instance) in instances.enumerated() {
            XCTAssertTrue(instance.contains("\t\t(unit \(index + 1))\n"), "instance \(index + 1)")
            XCTAssertTrue(instance.contains("(reference \"U1\") (unit \(index + 1))"), "instance \(index + 1)")
            XCTAssertTrue(instance.contains("(property \"Reference\" \"U1\""), "instance \(index + 1)")
        }
        // The first carries the part's uuid (the board's footprint points at it); the rest have their own.
        XCTAssertTrue(instances[0].contains("(uuid \"\(chip.id.uuidString.lowercased())\")"))
        XCTAssertEqual(count("(uuid \"\(chip.id.uuidString.lowercased())\")", in: text), 1)
        // Each instance lists its own unit's pins: 3 for a gate, 2 for the block.
        XCTAssertEqual(instances.map { count("\t\t(pin \"", in: $0) }, [3, 3, 3, 3, 2])
        // Each sits at its piece's corner.
        let k = KiCadExport.mmPerUnit
        let pieces = chip.pieces
        let first = try XCTUnwrap(instances.first)
        let at = try XCTUnwrap(first.components(separatedBy: "(at ").dropFirst().first?.components(separatedBy: " "))
        let x0 = try XCTUnwrap(Double(at[0])), y0 = try XCTUnwrap(Double(at[1]))
        for (index, instance) in instances.enumerated() {
            let parts = try XCTUnwrap(instance.components(separatedBy: "(at ").dropFirst().first?.components(separatedBy: " "))
            let x = try XCTUnwrap(Double(parts[0])), y = try XCTUnwrap(Double(parts[1]))
            XCTAssertEqual(x - x0, (pieces[index].origin.x - pieces[0].origin.x) * k, accuracy: 0.001, "unit \(index + 1) x")
            XCTAssertEqual(y - y0, (pieces[index].origin.y - pieces[0].origin.y) * k, accuracy: 0.001, "unit \(index + 1) y")
        }
    }

    func testKiCadPinsOnce() throws {
        let text = try kicadSchematic(try ExportSamples.split(), base: "split")
        let name = "SN74HC00N_units_An0_Bf90_Cn0_Dn0_Pn0"
        let start = try XCTUnwrap(text.range(of: "(symbol \"Schematical:\(name)\"\n"))
        let rest = text[start.upperBound...]
        let end = try XCTUnwrap(rest.range(of: "\n\t\t(symbol \"Schematical:") ?? rest.range(of: "\n\t)\n"))
        let definition = String(rest[..<end.lowerBound])
        for number in 1...14 {
            XCTAssertEqual(count("(number \"\(number)\"", in: definition), 1, "pin \(number)")
        }
        XCTAssertEqual(count("(rectangle", in: definition), 5)
    }

    func testKiCadUnsplitUnchanged() throws {
        for fixture in Fixture.names {
            let text = try kicadSchematic(try Fixture.open(fixture), base: fixture)
            XCTAssertEqual(count("_units", in: text), 0, fixture)
            // Every part instance is unit 1.
            XCTAssertEqual(count("\t\t(unit 1)\n", in: text), count("\t(symbol\n", in: text), fixture)
        }
        // A part split and joined again exports as one never split.
        func build(splitFirst: Bool) throws -> String {
            let doc = TestSupport.newDocument()
            let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 300, y: 300))
            if splitFirst {
                doc.setPartSplit(gate.id, true)
                doc.setPartSplit(gate.id, false)
            }
            // The part's id is random per document: compare with it blanked.
            return try kicadSchematic(doc, base: "same").replacingOccurrences(of: gate.id.uuidString.lowercased(), with: "ID")
                .replacingOccurrences(of: gate.id.uuidString, with: "ID")
        }
        XCTAssertEqual(count("_units", in: try build(splitFirst: true)), 0)
    }

    // MARK: EasyEDA Pro

    private func lines(_ text: String) throws -> [[Any]] {
        try text.split(separator: "\n").map { line in
            try XCTUnwrap(try JSONSerialization.jsonObject(with: Data(line.utf8)) as? [Any])
        }
    }

    private func easyEDA(_ doc: SchematicDocument) throws -> (sheet: String, symbols: [String]) {
        let entries = EasyEDAProExport.entries(for: doc, base: "split")
        let sheet = try XCTUnwrap(entries.first { $0.name.hasSuffix("/1.esch") })
        let symbols = entries.filter { $0.name.hasSuffix(".esym") }.map { String(decoding: $0.data, as: UTF8.self) }
        return (String(decoding: sheet.data, as: UTF8.self), symbols)
    }

    func testEasyEDAOneComponent() throws {
        let doc = try ExportSamples.split()
        let (sheet, _) = try easyEDA(doc)
        let designators = try lines(sheet).filter { line in
            line.first as? String == "ATTR" && line.contains { $0 as? String == "Designator" } && line.contains { $0 as? String == "U1" }
        }
        XCTAssertEqual(designators.count, 1)
        XCTAssertFalse(sheet.contains("U1-A"))
        // One component per part: the gate, the LED, the resistor and three rails.
        XCTAssertEqual(try lines(sheet).filter { $0.first as? String == "COMPONENT" }.count, doc.chips.count)
    }

    func testEasyEDASymbolHoldsEveryPiece() throws {
        let doc = try ExportSamples.split()
        let chip = try gate(doc)
        let (_, symbols) = try easyEDA(doc)
        let own = try symbols.filter { text in
            try lines(text).filter { $0.first as? String == "PIN" }.count == 14
        }
        XCTAssertEqual(own.count, 1, "one symbol holds all 14 pins")
        let records = try lines(try XCTUnwrap(own.first))
        XCTAssertEqual(records.filter { $0.first as? String == "RECT" }.count, 5)
        // Each pin, moved by the part's corner, is where the app has it.
        let pins = records.filter { $0.first as? String == "PIN" }.map { line -> String in
            let x = (line[4] as? NSNumber)?.doubleValue ?? .nan, y = (line[5] as? NSNumber)?.doubleValue ?? .nan
            return "\(Int((x + chip.origin.x).rounded())),\(Int((y + chip.origin.y).rounded()))"
        }
        let tips = chip.layout.slots.map { slot -> String in
            let tip = chip.tip(of: slot)
            return "\(Int(tip.x.rounded())),\(Int(tip.y.rounded()))"
        }
        XCTAssertEqual(Set(pins), Set(tips))
        XCTAssertEqual(Set(pins).count, 14)
    }

    func testEasyEDAUnsplitUnchanged() throws {
        // A fixture has no part drawn as its units: every part symbol has one rectangle.
        for fixture in Fixture.names {
            let doc = try Fixture.open(fixture)
            XCTAssertFalse(doc.chips.contains { $0.isSplit }, fixture)
        }
        // A part split and joined again makes the same symbol as one never split.
        func symbolTexts(splitFirst: Bool) throws -> [Int] {
            let doc = TestSupport.newDocument()
            let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 300, y: 300))
            if splitFirst {
                doc.setPartSplit(gate.id, true)
                doc.setPartSplit(gate.id, false)
            }
            return try easyEDA(doc).symbols.map { text in try lines(text).filter { $0.first as? String == "RECT" }.count }
        }
        XCTAssertEqual(try symbolTexts(splitFirst: true), try symbolTexts(splitFirst: false))
        XCTAssertEqual(try symbolTexts(splitFirst: true), [1])
    }
}
