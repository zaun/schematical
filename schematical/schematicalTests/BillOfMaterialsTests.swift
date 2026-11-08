import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The Bill of Materials window's lines and CSV.
@MainActor
final class BillOfMaterialsTests: XCTestCase {
    /// Same part number groups into one line, references in natural order;
    /// supplies are left out; lines follow their first reference.
    func testGroupsByPartAndSkipsSupplies() throws {
        let doc = TestSupport.newDocument()
        for x in stride(from: 0, to: 1200, by: 100) {
            try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: CGFloat(x), y: 100))
        }
        try TestSupport.place(doc, "GND", at: CGPoint(x: 100, y: 400))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 400))
        XCTAssertNil(doc.renameChip(led.id, prefix: "D", number: 1))

        let lines = BillOfMaterials.lines(for: doc.chips)
        XCTAssertEqual(lines.count, 2)
        let gates = try XCTUnwrap(lines.first { $0.partNumber == "SN74HC00D" })
        XCTAssertEqual(gates.quantity, 12)
        XCTAssertEqual(gates.references.prefix(3), ["U1", "U2", "U3"])
        XCTAssertEqual(gates.references.last, "U12")
        XCTAssertEqual(lines.first?.partNumber, led.partNumber, "D1 sorts before U1")
        XCTAssertFalse(lines.contains { $0.partNumber == "GND" })
    }

    /// LEDs of different colors are separate lines.
    func testLEDColorsSplitLines() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 100))
        let colors = a.part.colors
        try XCTSkipIf(colors.count < 2, "LED has one color")
        doc.setPartColor(b.id, colors[1])
        let lines = BillOfMaterials.lines(for: doc.chips)
        XCTAssertEqual(lines.map(\.color), [colors[0], colors[1]])
    }

    func testCSVQuotesFieldsWithCommas() {
        let line = BillOfMaterialsLine(partNumber: "X", package: "DIP-14", value: "", color: "",
                                       description: "Quad \"NAND\", 2-input", references: ["U1", "U2"])
        XCTAssertEqual(BillOfMaterials.csv([line]),
                       "Quantity,References,Part Number,Package,Value,Color,Description\r\n"
                       + "2,\"U1, U2\",X,DIP-14,,,\"Quad \"\"NAND\"\", 2-input\"\r\n")
    }

    /// plan-26-B: pull-ups at any rail and pull-downs with one value and
    /// package share a RESISTOR line; another value is another line.
    func testPullResistorsGroupByValueAndPackage() throws {
        let doc = TestSupport.newDocument()
        for (index, name) in ["PULLUP-5V", "PULLUP-3V3", "PULLDOWN"].enumerated() {
            let pull = try TestSupport.place(doc, name, at: CGPoint(x: CGFloat(100 + 200 * index), y: 100))
            XCTAssertTrue(pull.part.isPullResistor)
            doc.setPartOhms(pull.id, 4700)
            doc.setPartFootprint(pull.id, "RES-0805")
        }
        let fourth = try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 800, y: 100))
        doc.setPartFootprint(fourth.id, "RES-0805")

        let lines = BillOfMaterials.lines(for: doc.chips)
        XCTAssertEqual(lines.count, 2)
        let first = try XCTUnwrap(lines.first)
        XCTAssertEqual(first.partNumber, "RESISTOR")
        XCTAssertEqual(first.package, "0805")
        XCTAssertEqual(first.value, "4.7kΩ")
        XCTAssertEqual(first.description, "Resistor")
        XCTAssertEqual(first.quantity, 3)
        XCTAssertEqual(first.references, ["R1", "R2", "R3"])
        XCTAssertEqual(lines[1].value, "1kΩ")
        XCTAssertEqual(lines[1].references, ["R4"])
        XCTAssertEqual(BillOfMaterials.csv([first]).split(separator: "\r\n").last,
                       "3,\"R1, R2, R3\",RESISTOR,0805,4.7kΩ,,Resistor")
    }
}
