import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-26: pull-ups and pull-downs are resistors with a resistance, a
/// footprint choice and an `R<n>` reference, saved in the file.
@MainActor
final class PullResistorTests: XCTestCase {
    /// A placed pull-up is R1, 1 kΩ, Radial, on the PCB with two pads.
    func testPlacingAPullUp() throws {
        let doc = TestSupport.newDocument()
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 100, y: 100))
        XCTAssertTrue(pull.part.isPullResistor)
        XCTAssertEqual(pull.reference, "R1")
        XCTAssertEqual(pull.ohms, 1000)
        XCTAssertEqual(pull.footprintID, "RES-RADIAL-2.54")
        XCTAssertNotNil(pull.board)
        XCTAssertEqual(pull.boardPads.count, 2)
    }

    /// Pulls count R1, R2; a chip placed after them is still U1. VCC and GND
    /// are not resistors and have no footprint.
    func testReferences() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 100, y: 100))
        let second = try TestSupport.place(doc, "PULLDOWN", at: CGPoint(x: 300, y: 100))
        let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 500, y: 100))
        XCTAssertEqual(second.reference, "R2")
        XCTAssertEqual(chip.reference, "U1")
        XCTAssertNil(chip.ohms)
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 700, y: 100))
        XCTAssertFalse(gnd.part.isPullResistor)
        XCTAssertNil(gnd.ohms)
        XCTAssertNil(gnd.board)
    }

    /// `setPartOhms` keeps a valid value and ignores invalid ones and non-pull parts.
    func testSetPartOhms() throws {
        let doc = TestSupport.newDocument()
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 100, y: 100))
        let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 400, y: 100))
        doc.setPartOhms(pull.id, 4700)
        XCTAssertEqual(doc.chips[0].ohms, 4700)
        for bad in [0, -1, Double.infinity, 2e9] {
            doc.setPartOhms(pull.id, bad)
            XCTAssertEqual(doc.chips[0].ohms, 4700, "\(bad)")
        }
        doc.setPartOhms(chip.id, 4700)
        XCTAssertNil(doc.chips[1].ohms)
    }

    func testSetPartFootprint() throws {
        let doc = TestSupport.newDocument()
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 100, y: 100))
        doc.setPartFootprint(pull.id, "RES-0805")
        XCTAssertEqual(doc.chips[0].footprintID, "RES-0805")
        XCTAssertEqual(doc.chips[0].footprint?.id, "RES-0805")
        XCTAssertEqual(doc.chips[0].footprintChoice?.package, "0805")
    }

    /// Save and open keeps reference, ohms, footprint and PCB centre.
    func testRoundTrip() throws {
        let doc = TestSupport.newDocument()
        let pull = try TestSupport.place(doc, "PULLDOWN", at: CGPoint(x: 100, y: 100))
        doc.setPartOhms(pull.id, 2200)
        doc.setPartFootprint(pull.id, "RES-1206")
        let saved = try XCTUnwrap(doc.chips[0].board)
        let file = try SchematicFile.decode(SchematicFile(document: doc, fileURL: nil).encoded())
        XCTAssertEqual(file.parts[0].ohms, 2200)
        XCTAssertEqual(file.parts[0].footprint, "RES-1206")
        XCTAssertNotNil(file.parts[0].pcb)
        let opened = try BoardLayersTests.roundTrip(doc)
        let chip = try XCTUnwrap(opened.chips.first)
        XCTAssertEqual(chip.reference, "R1")
        XCTAssertEqual(chip.ohms, 2200)
        XCTAssertEqual(chip.footprintID, "RES-1206")
        let board = try XCTUnwrap(chip.board)
        XCTAssertEqual(board.centre.x, saved.centre.x, accuracy: 1e-9)
        XCTAssertEqual(board.centre.y, saved.centre.y, accuracy: 1e-9)
    }

    /// A file part with these extra keys (JSON, leading comma) as a version 4 file.
    private static func open(pull extra: String) throws -> SchematicDocument {
        let json = """
        { "format": "\(SchematicFile.formatName)", "version": 4, "nextReference": 2,
          "parts": [{ "id": "\(UUID().uuidString)", "reference": "U1", "partNumber": "PULLDOWN",
                      "x": 100, "y": 100, "flipped": false\(extra) }],
          "junctions": [], "wires": [] }
        """
        let opened = TestSupport.newDocument()
        try SchematicFile.decode(Data(json.utf8)).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        return opened
    }

    /// An older file's pull (no ohms, footprint or pcb) opens as 1 kΩ,
    /// Radial, placed on the PCB, keeping its reference.
    func testOldPullOpensWithDefaults() throws {
        let opened = try Self.open(pull: "")
        let chip = try XCTUnwrap(opened.chips.first)
        XCTAssertEqual(chip.reference, "U1")
        XCTAssertEqual(chip.ohms, 1000)
        XCTAssertEqual(chip.footprintID, "RES-RADIAL-2.54")
        XCTAssertNotNil(chip.board)
    }

    func testBadOhmsFails() {
        XCTAssertThrowsError(try Self.open(pull: ", \"ohms\": 0")) { error in
            guard case let SchematicFileError.badOhms(reference, ohms) = error else {
                return XCTFail("unexpected \(error)")
            }
            XCTAssertEqual(reference, "U1")
            XCTAssertEqual(ohms, 0)
            XCTAssertEqual(error.localizedDescription,
                           "U1 has resistance 0 Ω; it must be more than 0 and at most 1 GΩ.")
        }
        XCTAssertThrowsError(try Self.open(pull: ", \"ohms\": 2e9"))
        XCTAssertNoThrow(try Self.open(pull: ", \"ohms\": 1e9"))
    }

    // MARK: plan-26-B: value text

    func testResistorValueText() {
        XCTAssertEqual(ResistorValue.text(1000), "1kΩ")
        XCTAssertEqual(ResistorValue.text(4700), "4.7kΩ")
        XCTAssertEqual(ResistorValue.text(470), "470Ω")
        XCTAssertEqual(ResistorValue.text(1e6), "1MΩ")
        XCTAssertEqual(ResistorValue.text(0.5), "0.5Ω")
        XCTAssertEqual(ResistorValue.text(1e9), "1000MΩ")
        XCTAssertEqual(ResistorValue.text(1234), "1.234kΩ")
        let split = ResistorValue.split(1500)
        XCTAssertEqual(split.number, 1.5, accuracy: 1e-12)
        XCTAssertEqual(split.unit, .kilo)
        XCTAssertEqual(ResistorValue.split(0.5).unit, .ohm)
    }

    func testResistorValueOhmsAndParse() throws {
        XCTAssertNil(ResistorValue.ohms(number: 2000, unit: .mega))
        XCTAssertNil(ResistorValue.ohms(number: 0, unit: .kilo))
        XCTAssertNil(ResistorValue.ohms(number: -1, unit: .ohm))
        XCTAssertEqual(try XCTUnwrap(ResistorValue.ohms(number: 1.2345, unit: .kilo)), 1235, accuracy: 1e-9)
        XCTAssertEqual(try XCTUnwrap(ResistorValue.ohms(number: 4.7, unit: .kilo)), 4700, accuracy: 1e-9)
        XCTAssertEqual(ResistorValue.ohms(number: 1000, unit: .mega), 1e9)
        XCTAssertEqual(ResistorValue.parse(" 4.7 "), 4.7)
        XCTAssertNil(ResistorValue.parse("abc"))
        XCTAssertNil(ResistorValue.parse("inf"))
        XCTAssertNil(ResistorValue.parse(""))
    }

    // MARK: plan-26-B: Settings ▸ New Pull Resistor

    func testNewPartOptions() throws {
        let suite = "PullResistorTests.\(UUID().uuidString)"
        let defaults = try XCTUnwrap(UserDefaults(suiteName: suite))
        defer { defaults.removePersistentDomain(forName: suite) }
        let pull = try TestSupport.part("PULLUP-5V")
        let chip = try TestSupport.part("SN74HC00D")

        XCTAssertNil(NewPartOptions.ohms(for: pull, defaults: defaults))
        XCTAssertNil(NewPartOptions.footprintID(for: pull, defaults: defaults))

        defaults.set(2200.0, forKey: PreferenceKey.pullResistorOhms)
        defaults.set("RES-1206", forKey: PreferenceKey.pullResistorFootprint)
        XCTAssertEqual(NewPartOptions.ohms(for: pull, defaults: defaults), 2200)
        XCTAssertEqual(NewPartOptions.footprintID(for: pull, defaults: defaults), "RES-1206")
        XCTAssertNil(NewPartOptions.ohms(for: chip, defaults: defaults))
        XCTAssertNil(NewPartOptions.footprintID(for: chip, defaults: defaults))
        // The LED keys don't leak onto a pull resistor, nor the pull keys onto the LED.
        let led = try TestSupport.part("GENERIC-LED")
        XCTAssertNil(NewPartOptions.footprintID(for: led, defaults: defaults))

        defaults.set(-5.0, forKey: PreferenceKey.pullResistorOhms)
        XCTAssertNil(NewPartOptions.ohms(for: pull, defaults: defaults))
    }
}
