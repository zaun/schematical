import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Board types with a header instead of an edge tab: the Raspberry Pi HAT
/// and the Arduino Uno shield. Their header pads are the board's pins,
/// carried by a bus the same way a card's fingers are.
@MainActor
final class HeaderBoardTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func pin(_ board: BoardSettings, _ name: String) throws -> EdgeFinger {
        try XCTUnwrap(board.fingers.first { $0.name == name }, name)
    }

    private func centre(_ pin: EdgeFinger) -> CGPoint { CGPoint(x: pin.rect.midX, y: pin.rect.midY) }

    func testPiHat() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        doc.setBoardType(.piHat)
        let board = doc.board
        XCTAssertEqual(board.width, 65)
        XCTAssertEqual(board.height, 56.5)
        XCTAssertEqual(board.card?.isFixedSize, true)
        XCTAssertEqual(board.layers, 1, "a header needs no Bottom layer")
        XCTAssertNil(doc.canSetLayerCount(1))
        XCTAssertEqual(board.pinNames, (1...40).map { "\($0)" })
        XCTAssertEqual(board.connectorName, "Header")

        // Centred 29 mm right of the left holes, on their line: pin 1 at
        // the left on the inner row, pin 2 on the edge row.
        let one = try pin(board, "1"), two = try pin(board, "2"), forty = try pin(board, "40")
        XCTAssertEqual(centre(one).x, 32.5 - 9.5 * 2.54, accuracy: 1e-9)
        XCTAssertEqual(centre(one).y, 3.5 + 1.27, accuracy: 1e-9)
        XCTAssertEqual(centre(two).x, centre(one).x, accuracy: 1e-9)
        XCTAssertEqual(centre(two).y, 3.5 - 1.27, accuracy: 1e-9)
        XCTAssertEqual(centre(forty).x, 32.5 + 9.5 * 2.54, accuracy: 1e-9)
        XCTAssertFalse(one.round, "pin 1 is square")
        XCTAssertTrue(two.round)
        XCTAssertTrue(board.fingers.allSatisfy { $0.drill == 1.1 })

        XCTAssertEqual(board.holes.map(\.centre), [p(3.5, 3.5), p(61.5, 3.5), p(3.5, 52.5), p(61.5, 52.5)])
        XCTAssertTrue(board.holes.allSatisfy { $0.drill == 2.75 })
        // Rounded 3 mm corners: nothing reaches the very corner.
        XCTAssertFalse(board.outline.contains(p(0, 0)))
        let xs = board.outline.map(\.x), ys = board.outline.map(\.y)
        XCTAssertEqual(xs.min()!, 0, accuracy: 1e-9)
        XCTAssertEqual(xs.max()!, 65, accuracy: 1e-9)
        XCTAssertEqual(ys.max()!, 56.5, accuracy: 1e-9)

        doc.setBoardSize(width: 200, height: 200)
        XCTAssertEqual(doc.board.width, 65, "fixed")
    }

    func testArduinoShield() throws {
        var board = BoardSettings()
        board.type = .arduinoShield
        XCTAssertEqual(board.pinCount, 32)
        XCTAssertEqual(board.pinNames.prefix(4), ["D0", "D1", "D2", "D3"])
        XCTAssertEqual(board.pinNames[14], "A0")
        XCTAssertEqual(board.pinNames.last, "NC")
        XCTAssertEqual(Set(board.pinNames), Set(board.fingers.map(\.name)), "every pin in the order")
        XCTAssertTrue(board.pinNames.allSatisfy { Bus.nameLengthRange.contains($0.count) },
                      "the names fit a bus wire")
        let inch = 25.4, height = 2.1 * inch
        // Mils from the bottom-left, as SparkFun's ARDUINO_R3.
        func at(_ x: Double, _ y: Double) -> CGPoint { p(x * inch / 1000, height - y * inch / 1000) }
        for (name, x, y) in [("D0", 2500.0, 2000.0), ("D7", 1800, 2000), ("D8", 1640, 2000), ("SCL", 740, 2000),
                             ("NC", 1100, 100), ("VIN", 1800, 100), ("A0", 2000, 100), ("A5", 2500, 100)] {
            let found = centre(try pin(board, name)), want = at(x, y)
            XCTAssertEqual(found.x, want.x, accuracy: 1e-9, name)
            XCTAssertEqual(found.y, want.y, accuracy: 1e-9, name)
        }
        XCTAssertEqual(board.holes.count, 4)
        let shape = CGMutablePath()
        shape.addLines(between: board.outline)
        shape.closeSubpath()
        for pad in board.fingers {
            XCTAssertTrue(shape.contains(centre(pad)), "\(pad.name) on the board")
        }
        for hole in board.holes { XCTAssertTrue(shape.contains(hole.centre)) }
    }

    /// A header pad is on every layer: a trace on Top or Bottom ends at its
    /// centre and joins it.
    func testTracesConnectToHeaderPads() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.piHat)
        let seven = try pin(doc.board, "7")
        let tolerance = TraceDrawing.tolerance / BoardScale.pointsPerMM
        let inside = p(seven.rect.midX + 0.3, seven.rect.midY)
        XCTAssertEqual(doc.traceAnchor(at: inside, layer: .top, tolerance: tolerance), centre(seven))
        XCTAssertEqual(doc.traceAnchor(at: inside, layer: .bottom, tolerance: tolerance), centre(seven))
        let id = try XCTUnwrap(doc.addTrace(from: p(seven.rect.midX, 30), to: centre(seven), layer: .bottom,
                                            diagonalFirst: false))
        XCTAssertEqual(doc.pads(touching: [id]), ["Header.7"])
    }

    /// The header is a hidden port on its bus, as a card edge is: a pin
    /// whose wire reaches a part gets a ratsnest line to it.
    func testHeaderPinsGetRatsnestLines() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(600, 600))
        let k = try TestSupport.pin(led, "K")
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        var card = BusCard()
        card.type = .arduinoShield
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertEqual(doc.bus(id)?.wires.map(\.name), BoardSettings(type: .arduinoShield).pinNames)
        XCTAssertNil(doc.addPort(at: p(200, 100)))
        let port = try XCTUnwrap(doc.bus(id)?.ports.first)
        let d13 = try XCTUnwrap(doc.bus(id)?.wires.first { $0.name == "D13" })
        XCTAssertNil(doc.setPortWires(port.id, [d13.id]))
        XCTAssertNil(doc.connect(.busPin(BusPinRef(portID: port.id, wireID: d13.id)), .pin(k)))

        doc.setBoardType(.arduinoShield)
        doc.setEdgeBus(id)
        XCTAssertEqual(doc.edgeBusID, id)
        let header = try pin(doc.board, "D13")
        let lines = doc.ratsnest()
        XCTAssertEqual(lines.count, 1)
        let line = try XCTUnwrap(lines.first)
        XCTAssertTrue([line.0, line.1].contains { abs($0.x - header.rect.midX) < 1e-6 && abs($0.y - header.rect.midY) < 1e-6 },
                      "\(lines)")
    }

    func testFile() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        doc.setBoardType(.arduinoShield)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.type, .arduinoShield)
        XCTAssertEqual(opened.board.layers, 1)
        XCTAssertEqual(opened.board.width, 68.58, accuracy: 1e-9)
        XCTAssertEqual(opened.board.height, 53.34, accuracy: 1e-9)
        let hat = try BoardLayersTests.open(board: #"{ "type": "piHat", "layers": 1, "width": 10, "height": 10 }"#)
        XCTAssertEqual(hat.board.width, 65)
        XCTAssertEqual(hat.board.height, 56.5)
    }

    /// A mounting hole has no copper: a Plane is cut round it.
    func testMountingHolesCutPlanes() {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.piHat)
        let copper = doc.connectivity()
        let holes = copper.pads.indices.filter { copper.pads[$0].label.hasPrefix("Hole") }
        XCTAssertEqual(holes.count, 4)
        XCTAssertTrue(holes.allSatisfy { !copper.pads[$0].pad.plated })
        XCTAssertTrue(Set(holes).isSubset(of: copper.planeClearances(on: .bottom).pads))
    }
}
