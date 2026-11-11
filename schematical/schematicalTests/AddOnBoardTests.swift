import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Header boards for small hosts: the Adafruit FeatherWing, the Digilent
/// Pmod, the mikroBUS add-on board and the BeagleBone cape. Like a HAT,
/// their header pads are the board's pins, carried by a bus.
@MainActor
final class AddOnBoardTests: XCTestCase {
    private let inch = 25.4

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func centre(_ board: BoardSettings, _ name: String) throws -> CGPoint {
        let pin = try XCTUnwrap(board.fingers.first { $0.name == name }, name)
        return CGPoint(x: pin.rect.midX, y: pin.rect.midY)
    }

    private func assertNear(_ a: CGPoint, _ b: CGPoint, _ message: String = "",
                            file: StaticString = #filePath, line: UInt = #line) {
        XCTAssertEqual(a.x, b.x, accuracy: 1e-6, message, file: file, line: line)
        XCTAssertEqual(a.y, b.y, accuracy: 1e-6, message, file: file, line: line)
    }

    /// Every pin and hole inside the outline.
    private func assertInside(_ board: BoardSettings, file: StaticString = #filePath, line: UInt = #line) {
        let shape = CGMutablePath()
        shape.addLines(between: board.outline)
        shape.closeSubpath()
        for pin in board.fingers {
            XCTAssertTrue(shape.contains(CGPoint(x: pin.rect.midX, y: pin.rect.midY)), pin.name, file: file, line: line)
        }
        for hole in board.holes { XCTAssertTrue(shape.contains(hole.centre), file: file, line: line) }
        let xs = board.outline.map(\.x), ys = board.outline.map(\.y)
        XCTAssertEqual(xs.min()!, 0, accuracy: 1e-9, file: file, line: line)
        XCTAssertEqual(ys.min()!, 0, accuracy: 1e-9, file: file, line: line)
        XCTAssertEqual(xs.max()!, board.width, accuracy: 1e-9, file: file, line: line)
        XCTAssertEqual(ys.max()!, board.height, accuracy: 1e-9, file: file, line: line)
    }

    func testFeatherWing() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        doc.setBoardType(.featherWing)
        let board = doc.board
        XCTAssertEqual(board.width, 50.8, accuracy: 1e-9)
        XCTAssertEqual(board.height, 22.86, accuracy: 1e-9)
        XCTAssertEqual(board.card?.isFixedSize, true)
        XCTAssertEqual(board.layers, 1)
        XCTAssertEqual(board.pinCount, 28)
        XCTAssertEqual(Array(board.pinNames.prefix(4)), ["RST", "3V", "AREF", "GND"])
        XCTAssertEqual(Array(board.pinNames.suffix(2)), ["SCL", "SDA"])
        // The 16-pin strip centred 1.0 in from the left, the 12-pin 1.2 in,
        // 0.8 in apart, both ending at 1.75 in.
        assertNear(try centre(board, "RST"), p(0.25 * inch, 0.85 * inch))
        assertNear(try centre(board, "NC"), p(1.75 * inch, 0.85 * inch))
        assertNear(try centre(board, "VBAT"), p(0.65 * inch, 0.05 * inch))
        assertNear(try centre(board, "SDA"), p(1.75 * inch, 0.05 * inch))
        XCTAssertTrue(board.fingers.allSatisfy { $0.drill == 1.0 && $0.round })
        XCTAssertEqual(board.holes.count, 4)
        assertNear(board.holes[0].centre, p(2.54, 2.54))
        assertNear(board.holes[3].centre, p(48.26, 20.32))
        assertInside(board)
        doc.setBoardSize(width: 200, height: 200)
        XCTAssertEqual(doc.board.width, 50.8, accuracy: 1e-9, "fixed")
    }

    func testPmod() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.pmod)
        var board = doc.board
        XCTAssertEqual(board.height, 20.32, accuracy: 1e-9)
        XCTAssertEqual(board.width, 25.4, accuracy: 1e-9, "1 in for a new module")
        XCTAssertEqual(board.card?.isFixedSize, false)
        XCTAssertEqual(board.pinNames, ["IO1", "IO2", "IO3", "IO4", "GND", "VCC"])
        assertNear(try centre(board, "IO1"), p(3.81, 3.81))
        assertNear(try centre(board, "VCC"), p(3.81, 16.51), "centred on the 0.8 in side")
        XCTAssertFalse(board.fingers[0].round, "pin 1 is square")
        XCTAssertTrue(board.holes.isEmpty)
        assertInside(board)

        // The length is free, within range.
        doc.setBoardSize(width: 50, height: 99)
        XCTAssertEqual(doc.board.width, 50)
        XCTAssertEqual(doc.board.height, 20.32, accuracy: 1e-9)
        doc.setBoardSize(width: 1, height: 1)
        XCTAssertEqual(doc.board.width, 0.3 * inch, accuracy: 1e-9)

        // 12 pins: pins 7–12 at the edge, 1–6 a column further in.
        var options = doc.board.cardOptions
        options.pmodPins = 12
        doc.setCardOptions(options)
        board = doc.board
        XCTAssertEqual(board.pinCount, 12)
        XCTAssertEqual(board.width, 0.4 * inch, accuracy: 1e-9, "grows to clear the second column")
        assertNear(try centre(board, "IO1"), p(6.35, 3.81))
        assertNear(try centre(board, "IO5"), p(3.81, 3.81))
        assertNear(try centre(board, "VCC2"), p(3.81, 16.51))
        assertInside(board)

        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.type, .pmod)
        XCTAssertEqual(opened.board.cardOptions.pmodPins, 12)
        XCTAssertEqual(opened.board.width, 0.4 * inch, accuracy: 1e-9)

        // A bus of the Pmod type carries its pin count.
        var card = BusCard()
        card.type = .pmod
        card.cardOptions.pmodPins = 12
        XCTAssertEqual(card.pinNames?.count, 12)
        card.cardOptions.pmodPins = 6
        XCTAssertEqual(card.pinNames?.count, 6)

        XCTAssertEqual(BoardLayersTests.pcbError {
            _ = try BoardLayersTests.open(board: #"{ "type": "pmod", "cardOptions": { "pins": 8 } }"#)
        }, "card pins 8 must be 6 or 12")
    }

    func testMikroBus() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.mikroBus)
        var board = doc.board
        XCTAssertEqual(board.width, 25.4, accuracy: 1e-9)
        XCTAssertEqual(board.height, 28.6, accuracy: 1e-9)
        XCTAssertEqual(board.pinCount, 16)
        XCTAssertEqual(Array(board.pinNames.prefix(8)), ["AN", "RST", "CS", "SCK", "SDO", "SDI", "3V3", "GND1"])
        // Headers 22.86 mm apart, centred, bottom pins 3.81 mm up.
        assertNear(try centre(board, "GND1"), p(1.27, 28.6 - 3.81))
        assertNear(try centre(board, "AN"), p(1.27, 28.6 - 21.59))
        assertNear(try centre(board, "PWM"), p(24.13, 28.6 - 21.59))
        XCTAssertFalse(board.fingers.first { $0.name == "AN" }!.round, "AN is square")
        XCTAssertTrue(board.fingers.first { $0.name == "PWM" }!.round)
        // The bottom-right corner cut.
        XCTAssertTrue(board.outline.contains(p(25.4, 28.6 - 2.54)))
        XCTAssertFalse(board.outline.contains(p(25.4, 28.6)))
        assertInside(board)

        var options = doc.board.cardOptions
        options.mikroBusSize = .l
        doc.setCardOptions(options)
        board = doc.board
        XCTAssertEqual(board.height, 57.15, accuracy: 1e-9)
        assertNear(try centre(board, "GND2"), p(24.13, 57.15 - 3.81), "at the cut end on every size")
        assertInside(board)

        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.cardOptions.mikroBusSize, .l)
        XCTAssertEqual(opened.board.height, 57.15, accuracy: 1e-9)

        // The size moves no pin: a bus of the type saves none.
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        var card = BusCard()
        card.type = .mikroBus
        card.cardOptions.mikroBusSize = .m
        XCTAssertNil(doc.setBusCard(id, card))
        let file = try SchematicFile.decode(SchematicFile(document: doc, fileURL: nil).encoded())
        XCTAssertNil(file.buses?.first?.cardOptions)
        XCTAssertEqual(file.board?.cardOptions?.size, "l")
    }

    func testBeagleCape() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.beagleCape)
        let board = doc.board
        XCTAssertEqual(board.width, 86.36, accuracy: 1e-9)
        XCTAssertEqual(board.height, 54.61, accuracy: 1e-9)
        XCTAssertEqual(board.pinCount, 92)
        XCTAssertEqual(board.pinNames.first, "8.1")
        XCTAssertEqual(board.pinNames[46], "9.1")
        XCTAssertEqual(board.pinNames.last, "9.46")
        // Pin 1 at the Ethernet end, pin 2 above it: P8's odd pins on the
        // inner row, P9's on the edge row.
        assertNear(try centre(board, "8.1"), p(0.775 * inch, (2.15 - 1.975) * inch))
        assertNear(try centre(board, "8.2"), p(0.775 * inch, (2.15 - 2.075) * inch))
        assertNear(try centre(board, "8.46"), p(2.975 * inch, (2.15 - 2.075) * inch))
        assertNear(try centre(board, "9.1"), p(0.775 * inch, (2.15 - 0.075) * inch))
        assertNear(try centre(board, "9.2"), p(0.775 * inch, (2.15 - 0.175) * inch))
        XCTAssertFalse(board.fingers.first { $0.name == "9.1" }!.round, "pin 1 is square")
        XCTAssertEqual(board.holes.count, 4)
        // The Ethernet notch is off the board.
        let shape = CGMutablePath()
        shape.addLines(between: board.outline)
        shape.closeSubpath()
        XCTAssertFalse(shape.contains(p(0.4 * inch, (2.15 - 1.2) * inch)))
        XCTAssertFalse(shape.contains(p(0.1 * inch, (2.15 - 2.0) * inch)), "the strip above the notch")
        XCTAssertTrue(shape.contains(p(0.1 * inch, (2.15 - 0.5) * inch)))
        assertInside(board)

        // A bus of the cape's type carries 8.1 … 9.46.
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        XCTAssertNil(doc.setBusWireCount(id, 92))
        var card = BusCard()
        card.type = .beagleCape
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertEqual(doc.bus(id)?.wires.map(\.name), board.pinNames)
        doc.setEdgeBus(id)
        XCTAssertEqual(doc.edgeBusID, id)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.type, .beagleCape)
        XCTAssertEqual(opened.edgeBusID, id)
    }
}
