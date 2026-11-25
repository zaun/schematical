import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Rotate on the Schematic: LEDs, diodes, resistors and capacitors turn in
/// quarter turns; nothing else does. The symbol's drawing follows the pins.
@MainActor
final class SchematicRotateTests: XCTestCase {
    private static let rotatable = ["GENERIC-LED", "1N4001-E3/54", "GENERIC-RESISTOR", "GENERIC-CAPACITOR"]

    /// Turned a quarter, an LED stands up: size swaps, the anode's pin is on
    /// top and the cathode's below, on grid points. A half turn swaps the
    /// sides; four turns are no turn.
    func testLayoutTurns() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let flat = led.layout
        let left = try XCTUnwrap(flat.slots.first { $0.side == .left })
        let right = try XCTUnwrap(flat.slots.first { $0.side == .right })

        let turns: [(Int, ChipLayout.Side, ChipLayout.Side)] = [(90, .top, .bottom), (180, .right, .left),
                                                                 (270, .bottom, .top), (0, .left, .right)]
        for (degrees, leftSide, rightSide) in turns {
            var turned = led
            turned.rotation = degrees
            let layout = turned.layout
            XCTAssertEqual(layout.slots.first { $0.pin.index == left.pin.index }?.side, leftSide, "\(degrees)")
            XCTAssertEqual(layout.slots.first { $0.pin.index == right.pin.index }?.side, rightSide, "\(degrees)")
            let quarter = degrees == 90 || degrees == 270
            XCTAssertEqual(layout.size, quarter ? CGSize(width: flat.size.height, height: flat.size.width) : flat.size)
            XCTAssertEqual(turned.hasVerticalPins, quarter)
            for slot in layout.slots {
                let tip = turned.tip(of: slot)
                XCTAssertEqual(tip.x.truncatingRemainder(dividingBy: 10), 0, "\(degrees) x")
                XCTAssertEqual(tip.y.truncatingRemainder(dividingBy: 10), 0, "\(degrees) y")
            }
        }
    }

    /// The drawing's lead ends land on the pins' tips for every flip and
    /// turn, so the symbol never drifts from its pins.
    func testSymbolFollowsThePins() throws {
        let doc = TestSupport.newDocument()
        for name in Self.rotatable {
            let chip = try TestSupport.place(doc, name, at: CGPoint(x: 200, y: 200))
            XCTAssertTrue(chip.symbol.canRotate, name)
            let size = chip.unturnedSize
            let stub = ChipLayout.stubLength, mid = ChipLayout.diodeCentreY
            let basePins = chip.layout.slots
            let leftPin = try XCTUnwrap(basePins.first { $0.side == .left }).pin.index
            let rightPin = try XCTUnwrap(basePins.first { $0.side == .right }).pin.index

            for flipped in [false, true] {
                for degrees in [0, 90, 180, 270] {
                    var turned = chip
                    turned.isFlipped = flipped
                    turned.rotation = degrees
                    let origin = turned.viewOrigin
                    func absolute(_ point: CGPoint) -> CGPoint { CGPoint(x: origin.x + point.x, y: origin.y + point.y) }
                    let label = "\(name) flipped=\(flipped) \(degrees)"
                    let first = absolute(turned.symbolPoint(-stub, mid))
                    let second = absolute(turned.symbolPoint(size.width + stub, mid))
                    let tipOfLeft = turned.tip(of: try XCTUnwrap(turned.layout.slots.first { $0.pin.index == leftPin }))
                    let tipOfRight = turned.tip(of: try XCTUnwrap(turned.layout.slots.first { $0.pin.index == rightPin }))
                    XCTAssertEqual(first.x, tipOfLeft.x, accuracy: 1e-9, label)
                    XCTAssertEqual(first.y, tipOfLeft.y, accuracy: 1e-9, label)
                    XCTAssertEqual(second.x, tipOfRight.x, accuracy: 1e-9, label)
                    XCTAssertEqual(second.y, tipOfRight.y, accuracy: 1e-9, label)

                    // The frame holds every lead end.
                    let frame = turned.symbolFrameSize
                    for point in [turned.symbolPoint(-stub, mid), turned.symbolPoint(size.width + stub, mid)] {
                        XCTAssertTrue(CGRect(origin: .zero, size: frame).insetBy(dx: -1e-9, dy: -1e-9).contains(point),
                                      label)
                    }
                }
            }
        }
    }

    /// Rotate turns clockwise, ⇧ counterclockwise, one undo each, and the
    /// wires stay attached to the pins they were on.
    func testRotateChipUndoAndWires() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 200, y: 200))
        let resistor = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 400, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(try TestSupport.pin(resistor, "1"))))
        func turn(_ id: UUID) -> Int { doc.chips.first { $0.id == id }?.rotation ?? -1 }

        doc.rotateChip(led.id)
        XCTAssertEqual(turn(led.id), 90)
        XCTAssertEqual(doc.undoName, "Rotate")
        doc.rotateChip(led.id, counterclockwise: true)
        doc.rotateChip(led.id, counterclockwise: true)
        XCTAssertEqual(turn(led.id), 270)
        XCTAssertEqual(doc.wires.count, 1, "the wire stays")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(turn(led.id), 0)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(turn(led.id), 90)

        // A wire end follows its pin.
        let route = try XCTUnwrap(doc.wires.first.map { doc.path(for: $0) })
        let cathode = try XCTUnwrap(doc.chips.first { $0.id == led.id }).layout.slots
            .first { $0.pin.name == "K" }
        let tip = try XCTUnwrap(doc.chips.first { $0.id == led.id }).tip(of: try XCTUnwrap(cathode))
        XCTAssertTrue(route.contains(tip), "the wire ends on the turned pin")
    }

    /// Other parts don't rotate; the part symbols that can are the four.
    func testOnlyTwoTerminalPartsRotate() throws {
        let doc = TestSupport.newDocument()
        let gate = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        let supply = try TestSupport.place(doc, "5V", at: CGPoint(x: 300, y: 100))
        doc.rotateChip(gate.id)
        doc.rotateChip(supply.id)
        XCTAssertEqual(doc.chips.map(\.rotation), [0, 0])
        XCTAssertNotEqual(doc.undoName, "Rotate")
        for symbol in [PartSymbol.block, .vcc, .ground, .pullUp, .pullDown, .transistor, .dipSwitch, .header] {
            XCTAssertFalse(symbol.canRotate, "\(symbol)")
        }
        for symbol in [PartSymbol.diode, .plainDiode, .capacitor, .resistor] {
            XCTAssertTrue(symbol.canRotate, "\(symbol)")
        }
    }

    /// The turn is saved, opened, and left out of the file when 0.
    func testSaveAndOpen() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let cap = try TestSupport.place(doc, "GENERIC-CAPACITOR", at: CGPoint(x: 300, y: 100))
        doc.rotateChip(led.id, counterclockwise: true)
        doc.flipChip(led.id)
        let file = try SchematicFile.decode(SchematicFile(document: doc, fileURL: nil).encoded())
        XCTAssertEqual(file.parts[0].rotation, 270)
        XCTAssertNil(file.parts[1].rotation)

        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.chips.map(\.rotation), [270, 0])
        XCTAssertEqual(opened.chips[0].isFlipped, true)
        XCTAssertEqual(opened.chips.first { $0.id == cap.id }?.rotation, 0)
        XCTAssertEqual(opened.chips[0].layout, doc.chips[0].layout)
    }

    /// A turn on a part that can't turn, or that isn't a quarter, is ignored.
    func testBadRotationInAFileIsIgnored() throws {
        func rotation(of part: String, _ value: Int) throws -> Int? {
            let json = """
            { "format": "\(SchematicFile.formatName)", "version": 5, "nextReference": 1,
              "parts": [{ "id": "\(UUID().uuidString)", "reference": "X1", "partNumber": "\(part)",
                          "x": 100, "y": 100, "flipped": false, "rotation": \(value) }],
              "junctions": [], "wires": [] }
            """
            let opened = TestSupport.newDocument()
            try SchematicFile.decode(Data(json.utf8)).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
            return opened.chips.first?.rotation
        }
        XCTAssertEqual(try rotation(of: "GENERIC-LED", 90), 90)
        XCTAssertEqual(try rotation(of: "GENERIC-LED", -90), 270)
        XCTAssertEqual(try rotation(of: "GENERIC-LED", 450), 90)
        XCTAssertEqual(try rotation(of: "SN74HC00D", 90), 0)
    }
}
