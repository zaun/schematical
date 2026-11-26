import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-68-AC: what the Schematic sidebar's Options section reads and changes
/// (`PartOptionsModel`), and the number-and-unit fields' logic
/// (`UnitValueEntry`).
@MainActor
final class PartOptionsModelTests: XCTestCase {
    private var catalog: PartCatalogStore { TestSupport.catalog }

    private func model(_ name: String, in given: SchematicDocument? = nil) throws
        -> (PartOptionsModel, SchematicDocument) {
        let doc = given ?? TestSupport.newDocument()
        let chip = try TestSupport.place(doc, name, at: CGPoint(x: 100, y: 100))
        return (PartOptionsModel(catalog: catalog, document: doc, chip: chip), doc)
    }

    func testTheViewChoiceSplitsAPartWithUnits() throws {
        let (gate, _) = try model("SN74HC00N")
        XCTAssertEqual(gate.unitViewName, "Gate")
        XCTAssertFalse(gate.isSplit)
        gate.setSplit(true)
        XCTAssertTrue(gate.isSplit)
        gate.setSplit(false)
        XCTAssertFalse(gate.isSplit)
        XCTAssertNil(try model("GENERIC-LED").0.unitViewName)
    }

    func testAHeaderResizesAndKeepsItsGender() throws {
        let (header, doc) = try model("GENERIC-HEADER-1X08")
        XCTAssertEqual(header.headerSize?.rows, 1)
        XCTAssertEqual(header.headerSize?.perRow, 8)
        XCTAssertEqual(header.genders, ["Male", "Female"])
        XCTAssertEqual(header.gender, "Male")
        header.setGender("Female")
        XCTAssertEqual(header.gender, "Female")
        header.setRows(2)
        XCTAssertEqual(header.current.part.name, "GENERIC-HEADER-2X08")
        header.setPerRow(4)
        XCTAssertEqual(header.current.part.name, "GENERIC-HEADER-2X04")
        // A size that is not a part changes nothing.
        header.setPerRow(HeaderFamily.maxPerRow + 1)
        XCTAssertEqual(header.current.part.name, "GENERIC-HEADER-2X04")
        XCTAssertEqual(doc.chips.count, 1)

        let items = header.perRowItems
        XCTAssertEqual(items.map(\.n), Array(1...HeaderFamily.maxPerRow))
        XCTAssertEqual(items.first?.label, "1")
    }

    func testThePinsPerRowMenuSaysWhichSizesRemoveWires() throws {
        let doc = TestSupport.newDocument()
        let (header, _) = try model("GENERIC-HEADER-1X08", in: doc)
        let other = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: CGPoint(x: 400, y: 100))
        let chip = header.current
        for (mine, theirs) in [(7, 0), (5, 1), (7, 2)] {
            XCTAssertNil(doc.connect(.pin(PinRef(chipID: chip.id, index: mine)), .pin(PinRef(chipID: other.id, index: theirs))))
        }
        XCTAssertEqual(PartOptionsSection.wiredPins(of: chip.id, in: doc).sorted(), [6, 8, 8])
        let labels = Dictionary(uniqueKeysWithValues: header.perRowItems.map { ($0.n, $0.label) })
        XCTAssertEqual(labels[8], "8")
        XCTAssertEqual(labels[7], "7 — removes 2 wires")
        XCTAssertEqual(labels[5], "5 — removes 3 wires")
        XCTAssertEqual(PartOptionsSection.pinsPerRowLabel(6, rows: 1, wiredPins: [7]), "6 — removes 1 wire")
    }

    func testAPartThatIsNotAHeaderHasNoSizeOrGender() throws {
        let (led, _) = try model("GENERIC-LED")
        XCTAssertNil(led.headerSize)
        XCTAssertEqual(led.genders, [])
        XCTAssertEqual(led.gender, "")
        XCTAssertEqual(led.perRowItems.count, 0)
        led.setRows(2)
        led.setPerRow(2)
        XCTAssertEqual(led.current.part.name, "GENERIC-LED")
    }

    func testAnLEDsColorAndFootprint() throws {
        let (led, _) = try model("GENERIC-LED")
        XCTAssertGreaterThan(led.colors.count, 1)
        XCTAssertEqual(led.color, led.colors[0])
        led.setColor(led.colors[1])
        XCTAssertEqual(led.color, led.colors[1])
        XCTAssertEqual(led.current.color, led.colors[1])

        let choices = led.footprintChoices
        XCTAssertGreaterThan(choices.count, 1)
        XCTAssertEqual(led.footprintID, led.current.part.footprint?.id)
        led.setFootprint(choices[1].footprint.id)
        XCTAssertEqual(led.footprintID, choices[1].footprint.id)
        XCTAssertNil(led.ohms)
        XCTAssertNil(led.farads)
        XCTAssertNil(led.testPoint)

        let (gate, _) = try model("SN74HC00N")
        XCTAssertEqual(gate.colors, [])
        XCTAssertEqual(gate.color, "")
        XCTAssertEqual(gate.footprintChoices.count, 0)
    }

    func testAResistorsAndACapacitorsValue() throws {
        let (resistor, _) = try model("GENERIC-RESISTOR")
        XCTAssertEqual(resistor.ohms, resistor.current.ohms ?? ResistorValue.defaultOhms)
        resistor.setOhms(4700)
        XCTAssertEqual(resistor.ohms, 4700)
        XCTAssertNil(resistor.farads)

        let (pull, _) = try model("PULLUP-5V")
        XCTAssertNotNil(pull.ohms)

        let (capacitor, _) = try model("GENERIC-CAPACITOR")
        XCTAssertEqual(capacitor.farads, capacitor.current.farads ?? CapacitorValue.defaultFarads)
        capacitor.setFarads(2.2e-6)
        XCTAssertEqual(try XCTUnwrap(capacitor.farads), 2.2e-6, accuracy: 1e-15)
        XCTAssertNil(capacitor.ohms)
    }

    func testATestPointsSizeAndMounting() throws {
        let (point, _) = try model("GENERIC-TESTPOINT")
        let start = try XCTUnwrap(point.testPoint)
        XCTAssertEqual(point.footprintChoices.count, 0, "a test point shows Size, not the Footprint menu")
        point.setTestPointSize(2.0)
        XCTAssertEqual(point.testPoint?.size, 2.0)
        XCTAssertEqual(point.testPoint?.throughHole, start.throughHole)
        point.setThroughHole(!start.throughHole)
        XCTAssertEqual(point.testPoint?.throughHole, !start.throughHole)
        XCTAssertEqual(point.current.footprintID, TestPointFamily.footprintID(size: 2.0, throughHole: !start.throughHole))

        // Any other part: no test point, and the setters change nothing.
        let (led, _) = try model("GENERIC-LED")
        let footprint = led.footprintID
        led.setTestPointSize(2.0)
        led.setThroughHole(true)
        XCTAssertEqual(led.footprintID, footprint)
    }

    func testTheMountingAndTheSocketPickersChoice() throws {
        let (gate, doc) = try model("SN74HC00N")
        XCTAssertTrue(gate.isSocketable)
        XCTAssertEqual(gate.mounting, .ic)
        let maker = try TestSupport.part("A 14-LC-TR")
        // With no socket, the picker's choice changes nothing.
        gate.useSocket(maker)
        XCTAssertNil(gate.current.socket)

        gate.setMounting(.withSocket)
        XCTAssertEqual(gate.mounting, .withSocket)
        XCTAssertEqual(gate.current.socket?.part.name, "GENERIC-SOCKET-DIP-14-300")
        gate.useSocket(maker)
        XCTAssertEqual(gate.current.socket, ChipSocket(part: maker, mode: .withSocket))
        gate.setMounting(.socketOnly)
        XCTAssertEqual(gate.current.socket, ChipSocket(part: maker, mode: .socketOnly))

        // A chip gone from the document: the model reads the chip it was made with and changes nothing.
        doc.select(.chip(gate.chip.id))
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertEqual(gate.current.id, gate.chip.id)
        XCTAssertNil(gate.current.socket)
        gate.useSocket(maker)
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertFalse(try model("GENERIC-LED").0.isSocketable)
    }

    // MARK: UnitValueEntry

    func testAnEntryShowsTheValueInItsLargestUnit() {
        let ohms = UnitValueEntry<ResistorValue>(4700)
        XCTAssertEqual(ohms.text, "4.7")
        XCTAssertEqual(ohms.shown, "4.7")
        XCTAssertEqual(ohms.unit, .kilo)
        let farads = UnitValueEntry<CapacitorValue>(1e-7)
        XCTAssertEqual(farads.text, "100")
        XCTAssertEqual(farads.unit, .nano)
        XCTAssertEqual(ResistorValue.symbol(.mega), "MΩ")
        XCTAssertEqual(CapacitorValue.symbol(.micro), "µF")
        XCTAssertEqual(CapacitorValue.value(number: 2, unit: .micro), 2e-6)
    }

    func testApplyCommitsANewNumberAndPutsBadInputBack() {
        var entry = UnitValueEntry<ResistorValue>(1000)
        XCTAssertNil(entry.apply(1000), "nothing typed")
        entry.text = "2.2"
        XCTAssertEqual(entry.apply(1000), 2200)
        XCTAssertEqual(entry.shown, "2.2")
        entry.text = "lots"
        XCTAssertNil(entry.apply(2200))
        XCTAssertEqual(entry.text, "2.2", "bad input puts the value back")
        entry.text = "-5"
        XCTAssertNil(entry.apply(2200))
        XCTAssertEqual(entry.text, "2.2", "an invalid value puts the value back")
    }

    func testPickingAUnitCommitsTheShownNumberInIt() {
        var entry = UnitValueEntry<ResistorValue>(4700)
        XCTAssertNil(entry.pick(.kilo, 4700), "the same unit")
        XCTAssertEqual(entry.pick(.mega, 4700), 4_700_000)
        XCTAssertEqual(entry.unit, .mega)
        XCTAssertEqual(entry.text, "4.7")
        // Text that doesn't parse: the stored number in the new unit.
        entry.text = "x"
        XCTAssertEqual(entry.pick(.ohm, 4_700_000), 4.7)
        // A result that is not valid puts the stored value back.
        entry.text = "5000"
        XCTAssertNil(entry.pick(.mega, 4.7))
        XCTAssertEqual(entry.text, "4.7")
        XCTAssertEqual(entry.unit, .ohm)
    }

    func testRefreshFollowsTheStoredValueUnlessTheUserIsTyping() {
        var entry = UnitValueEntry<CapacitorValue>(1e-9)
        entry.refresh(4.7e-6, isFocused: false)
        XCTAssertEqual(entry.text, "4.7")
        XCTAssertEqual(entry.unit, .micro)
        entry.refresh(2e-9, isFocused: true)
        XCTAssertEqual(entry.text, "2", "focused but not typing: shown")
        entry.text = "33"
        entry.refresh(1e-11, isFocused: true)
        XCTAssertEqual(entry.text, "33", "typing: the text stays")
        XCTAssertEqual(entry.unit, .pico, "only the unit follows")
    }
}
