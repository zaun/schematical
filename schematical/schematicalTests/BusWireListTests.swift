import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Export and Import of a bus's wire names.
@MainActor
final class BusWireListTests: XCTestCase {
    func testRoundTrip() throws {
        let names = ["D0", "D1", "A,B", "Q\"", "CLK"]
        let text = BusWireList.text(names)
        XCTAssertTrue(text.hasPrefix("wire,name\n0,D0\n"))
        XCTAssertEqual(try BusWireList.names(in: text).get(), names)
    }

    func testPlainListAndBlankLines() throws {
        XCTAssertEqual(try BusWireList.names(in: "A\r\n\nB\n  C  \n").get(), ["A", "B", "C"])
    }

    func testRefusals() {
        XCTAssertEqual((try? BusWireList.names(in: "A\n").get()), nil)
        XCTAssertEqual((try? BusWireList.names(in: "A\nTOOLONG\n").get()), nil)
        XCTAssertEqual((try? BusWireList.names(in: "A\nB C\n").get()), nil)
        XCTAssertEqual((try? BusWireList.names(in: "A\n0,\n").get()), nil)
    }

    func testImportRenamesAddsAndDrops() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: CGPoint(x: 100, y: 100), to: CGPoint(x: 300, y: 100)))
        let before = try XCTUnwrap(doc.bus(id)).wires
        XCTAssertNil(doc.setBusWireNames(id, ["X", "Y", "Z"]))
        var wires = try XCTUnwrap(doc.bus(id)).wires
        XCTAssertEqual(wires.map(\.name), ["X", "Y", "Z"])
        XCTAssertEqual(wires.prefix(3).map(\.id), before.prefix(3).map(\.id))
        XCTAssertEqual(doc.undoName(on: .schematic), "Import Bus Wires")
        XCTAssertNil(doc.setBusWireNames(id, ["P", "Q"]))
        wires = try XCTUnwrap(doc.bus(id)).wires
        XCTAssertEqual(wires.map(\.name), ["P", "Q"])
        XCTAssertNotNil(doc.setBusWireNames(id, ["A", "B C"]))
        XCTAssertNotNil(doc.setBusWireNames(id, ["A"]))
        XCTAssertEqual(try XCTUnwrap(doc.bus(id)).wires.map(\.name), ["P", "Q"])
    }
}
