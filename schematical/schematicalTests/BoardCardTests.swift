import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Board types: Custom, and the ISA 8-bit, ISA 16-bit and PCI cards with
/// their edge tab, notches and gold fingers.
@MainActor
final class BoardCardTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func finger(_ board: BoardSettings, _ name: String, _ side: BoardSide) throws -> EdgeFinger {
        try XCTUnwrap(board.fingers.first { $0.name == name && $0.side == side }, "\(name) on \(side)")
    }

    func testCustomIsTheDefaultRectangle() {
        let board = BoardSettings()
        XCTAssertEqual(board.type, .custom)
        XCTAssertEqual(board.outline, [p(0, 0), p(100, 0), p(100, 80), p(0, 80)])
        XCTAssertTrue(board.fingers.isEmpty)
    }

    func testChoosingACardSetsItsSizeAndBackToCustomKeepsIt() {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        doc.setBoardType(.pci)
        XCTAssertEqual(doc.board.width, 174.63)
        XCTAssertEqual(doc.board.height, 106.68)
        XCTAssertEqual(doc.board.layers, 2, "fingers are on both sides")
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Board Type")
        XCTAssertEqual(doc.canSetLayerCount(1), "A card has fingers on both sides.")

        // Length is editable within the standard; height is not.
        doc.setBoardSize(width: 200, height: 50)
        XCTAssertEqual(doc.board.width, 200)
        XCTAssertEqual(doc.board.height, 106.68)
        doc.setBoardSize(width: 50, height: 50)
        XCTAssertEqual(doc.board.width, 122, "clears the connector")
        doc.setBoardSize(width: 500, height: 50)
        XCTAssertEqual(doc.board.width, 312, "full length")

        doc.setBoardType(.isa16)
        XCTAssertEqual(doc.board.height, 121.92, accuracy: 1e-9)
        XCTAssertEqual(doc.board.width, 174.625, accuracy: 1e-9)

        doc.setBoardType(.custom)
        XCTAssertEqual(doc.board.height, 121.92, accuracy: 1e-9)
        doc.setBoardSize(width: 50, height: 40)
        XCTAssertEqual(doc.board.width, 50)
        XCTAssertEqual(doc.board.height, 40)
        XCTAssertNil(doc.canSetLayerCount(1))

        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.board.type, .isa16)
    }

    func testISAFingers() throws {
        var board = BoardSettings()
        board.type = .isa8
        XCTAssertEqual(board.fingers.count, 62)
        let a1 = try finger(board, "A1", .top), b31 = try finger(board, "B31", .bottom)
        XCTAssertEqual(a1.rect.midX, 20.32, accuracy: 1e-9)
        XCTAssertEqual(a1.rect.width, 1.524, accuracy: 1e-9)
        XCTAssertEqual(a1.rect.maxY, 106.68, accuracy: 1e-9, "down to the card edge")
        XCTAssertEqual(a1.rect.height, 7.62, accuracy: 1e-9)
        XCTAssertEqual(b31.rect.midX, 96.52, accuracy: 1e-9)
        XCTAssertNil(board.fingers.first { $0.name.hasPrefix("C") })

        board.type = .isa16
        XCTAssertEqual(board.fingers.count, 98)
        XCTAssertEqual(try finger(board, "C1", .top).rect.midX, 106.68, accuracy: 1e-9)
        XCTAssertEqual(try finger(board, "D18", .bottom).rect.midX, 149.86, accuracy: 1e-9)
    }

    func testPCIFingers() throws {
        var board = BoardSettings()
        board.type = .pci
        XCTAssertEqual(board.fingers.count, 116, "58 a side: no 12, 13, 50, 51")
        let b1 = try finger(board, "B1", .top)
        XCTAssertEqual(b1.rect.midX, 41.605, accuracy: 1e-9)
        XCTAssertEqual(b1.rect.width, 1.016, accuracy: 1e-9)
        XCTAssertEqual(b1.rect.minY, 106.68 - 6.89, accuracy: 1e-9)
        XCTAssertEqual(b1.rect.maxY, 106.68 - 2.413, accuracy: 1e-9)
        XCTAssertEqual(try finger(board, "A62", .bottom).rect.midX, 119.075, accuracy: 1e-9)
        for n in [12, 13, 50, 51] {
            XCTAssertNil(board.fingers.first { $0.name == "B\(n)" || $0.name == "A\(n)" }, "\(n) is a key")
        }
        XCTAssertNotNil(board.fingers.first { $0.name == "B11" })
        XCTAssertNotNil(board.fingers.first { $0.name == "A52" })
    }

    /// The outline stays inside the board's box, reaches the bottom only
    /// at the tabs, and every finger sits on the card.
    func testOutlines() {
        for type in [BoardType.isa8, .isa16, .pci] {
            var board = BoardSettings()
            board.type = type
            let card = type.card!
            board.width = card.length
            board.height = card.height
            let outline = board.outline
            XCTAssertEqual(outline.first, p(0, 0), "\(type)")
            for point in outline {
                XCTAssertTrue(point.x >= 0 && point.x <= board.width && point.y >= 0 && point.y <= board.height,
                              "\(type) \(point)")
            }
            let shape = CGMutablePath()
            shape.addLines(between: outline)
            shape.closeSubpath()
            for finger in board.fingers {
                XCTAssertTrue(shape.contains(CGPoint(x: finger.rect.midX, y: finger.rect.midY)),
                              "\(type) \(finger.name) on the card")
            }
            // Below the main edge, past the tab: no card.
            XCTAssertFalse(shape.contains(CGPoint(x: board.width - 1, y: board.height - 1)), "\(type)")
        }
        // ISA 16-bit: the key notch between the sections is open to the edge.
        var isa = BoardSettings()
        isa.type = .isa16
        let shape = CGMutablePath()
        shape.addLines(between: isa.outline)
        shape.closeSubpath()
        XCTAssertFalse(shape.contains(CGPoint(x: 101.6, y: 120)), "the notch")
        XCTAssertTrue(shape.contains(CGPoint(x: 96, y: 120)), "the 8-bit tab")
        // PCI: the 3.3 V key notch is cut into the tab.
        var pci = BoardSettings()
        pci.type = .pci
        let pciShape = CGMutablePath()
        pciShape.addLines(between: pci.outline)
        pciShape.closeSubpath()
        XCTAssertFalse(pciShape.contains(CGPoint(x: 56.21, y: 105)), "3.3 V key")
        XCTAssertFalse(pciShape.contains(CGPoint(x: 104.47, y: 105)), "5 V key")
        XCTAssertTrue(pciShape.contains(CGPoint(x: 60, y: 105)), "the tab")
        XCTAssertTrue(pciShape.contains(CGPoint(x: 7.5, y: 105)), "the bracket foot")
        XCTAssertFalse(pciShape.contains(CGPoint(x: 25, y: 105)), "between the foot and the tab")
    }

    /// Trace ends snap to a finger's inner end on its own side, and a trace
    /// there lists the finger in the sidebar.
    func testTracesConnectToFingers() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.isa8)
        let a1 = try finger(doc.board, "A1", .top)
        let tolerance = TraceDrawing.tolerance / BoardScale.pointsPerMM
        let inside = CGPoint(x: a1.rect.midX + 0.2, y: a1.rect.midY)
        XCTAssertEqual(doc.traceAnchor(at: inside, layer: .top, tolerance: tolerance), a1.contact)
        // On Bottom it is B1, which sits right under A1.
        XCTAssertEqual(doc.traceAnchor(at: inside, layer: .bottom, tolerance: tolerance),
                       try finger(doc.board, "B1", .bottom).contact)
        // Off the fingers, the trace grid as usual.
        XCTAssertEqual(doc.traceAnchor(at: p(a1.rect.midX, 50.1), layer: .top, tolerance: tolerance).y,
                       BoardGrid.snap(50.1, spacing: 0.3175), accuracy: 1e-9)

        let id = try XCTUnwrap(doc.addTrace(from: p(a1.contact.x, 50), to: a1.contact, layer: .top,
                                            diagonalFirst: false))
        XCTAssertEqual(doc.pads(touching: [id]), ["Edge.A1"])
    }

    func testFile() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.isa16)
        doc.setBoardSize(width: 250, height: 0)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.type, .isa16)
        XCTAssertEqual(opened.board.width, 250)
        XCTAssertEqual(opened.board.height, 121.92, accuracy: 1e-9)

        // A custom board writes no type; missing means custom.
        let plain = try SchematicFile.decode(SchematicFile(document: TestSupport.newDocument(), fileURL: nil).encoded())
        XCTAssertNil(plain.board?.type)
        XCTAssertEqual(try BoardLayersTests.open(board: #"{ "layers": 2 }"#).board.type, .custom)

        // A card's height comes from its standard, whatever the file says.
        let card = try BoardLayersTests.open(board: #"{ "type": "pci", "width": 20, "height": 30 }"#)
        XCTAssertEqual(card.board.height, 106.68)
        XCTAssertEqual(card.board.width, 122)

        XCTAssertEqual(BoardLayersTests.pcbError { _ = try BoardLayersTests.open(board: #"{ "type": "eisa" }"#) },
                       #"board type "eisa" must be custom, customCard, isa8, isa16, pci, nes, atari2600, atari5200, atari7800, atari8bit, atariST, jaguar, lynx, coco, pcie, m2, miniPCIe, sodimm, dimm, piHat, arduinoShield, featherWing, pmod, mikroBus or beagleCape"#)
        XCTAssertEqual(BoardLayersTests.pcbError {
            _ = try BoardLayersTests.open(board: #"{ "type": "isa8", "layers": 1 }"#)
        }, "board type isa8 needs 2 or more layers")
    }

    // MARK: Custom card

    func testCustomCardFingersAndTab() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 60, height: 40)
        doc.setBoardType(.customCard)
        var card = CustomCard()
        card.pads = 10
        card.pitch = 2.54
        card.padWidth = 1.5
        card.padLength = 5
        card.padOffset = 1
        card.depth = 8
        card.indent = 12
        card.margin = 3
        doc.setCustomCard(card)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Card Edge")
        // Choosing the type grew it to the default 20-finger tab; a smaller
        // tab doesn't shrink it.
        XCTAssertEqual(doc.board.width, 10 + 2 * 2.54 + 19 * 2.54, accuracy: 1e-9)
        XCTAssertEqual(doc.board.height, 40)
        XCTAssertEqual(doc.board.fingers.count, 20)
        let a1 = try finger(doc.board, "A1", .top), b10 = try finger(doc.board, "B10", .bottom)
        XCTAssertEqual(a1.rect.midX, 15, accuracy: 1e-9)
        XCTAssertEqual(a1.rect.width, 1.5, accuracy: 1e-9)
        XCTAssertEqual(a1.rect.maxY, 39, accuracy: 1e-9, "1 above the bottom")
        XCTAssertEqual(a1.rect.height, 5, accuracy: 1e-9)
        XCTAssertEqual(b10.rect.midX, 15 + 9 * 2.54, accuracy: 1e-9)

        // Its outline: the body down to 32, the tab from 12 to 12 + tab length.
        let shape = CGMutablePath()
        shape.addLines(between: doc.board.outline)
        shape.closeSubpath()
        XCTAssertTrue(shape.contains(CGPoint(x: 20, y: 38)), "the tab")
        XCTAssertFalse(shape.contains(CGPoint(x: 5, y: 38)), "left of the tab")
        XCTAssertFalse(shape.contains(CGPoint(x: 45, y: 38)), "right of the tab")
        XCTAssertTrue(shape.contains(CGPoint(x: 5, y: 31)), "the body")

        // One side only.
        card.bothSides = false
        doc.setCustomCard(card)
        XCTAssertEqual(doc.board.fingers.count, 10)
        XCTAssertTrue(doc.board.fingers.allSatisfy { $0.side == .top })

        // More fingers than fit: the board grows to hold the tab.
        card.pads = 40
        doc.setCustomCard(card)
        XCTAssertEqual(doc.board.width, 12 + 2 * 3 + 39 * 2.54, accuracy: 1e-9)
        // And can't be shrunk below it, though the height is free.
        doc.setBoardSize(width: 10, height: 20)
        XCTAssertEqual(doc.board.width, 12 + 2 * 3 + 39 * 2.54, accuracy: 1e-9)
        XCTAssertEqual(doc.board.height, 20)
        doc.setBoardSize(width: 10, height: 2)
        XCTAssertEqual(doc.board.height, 9, "depth + 1")
    }

    func testCustomCardClamps() {
        var card = CustomCard()
        card.pads = 0
        card.pitch = 1
        card.padWidth = 3
        card.depth = 4
        card.padOffset = 3
        card.padLength = 10
        card.margin = 0
        card.chamfer = 9
        let fixed = card.clamped()
        XCTAssertEqual(fixed.pads, 1)
        XCTAssertEqual(fixed.padWidth, 0.95, accuracy: 1e-9, "narrower than the pitch")
        XCTAssertEqual(fixed.padLength, 1, accuracy: 1e-9, "on the tab")
        XCTAssertEqual(fixed.margin, 0.475, accuracy: 1e-9, "half a finger")
        XCTAssertEqual(fixed.chamfer, 0, accuracy: 1e-9, "no room")
    }

    func testCustomCardFile() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.customCard)
        var card = CustomCard()
        card.pads = 7
        card.pitch = 3.96
        card.bothSides = false
        doc.setCustomCard(card)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.type, .customCard)
        XCTAssertEqual(opened.board.customCard, doc.board.customCard)
        XCTAssertEqual(opened.board.width, doc.board.width)
        XCTAssertEqual(opened.board.height, doc.board.height)

        // Other types write no card; missing keys are the defaults.
        XCTAssertNil(try SchematicFile.decode(SchematicFile(document: TestSupport.newDocument(), fileURL: nil)
            .encoded()).board?.card)
        let sparse = try BoardLayersTests.open(board: #"{ "type": "customCard", "card": { "pads": 4 } }"#)
        var expected = CustomCard()
        expected.pads = 4
        XCTAssertEqual(sparse.board.customCard, expected.clamped())
    }

    // MARK: Cartridges

    func testCartridgesHaveAFixedSize() {
        let doc = TestSupport.newDocument()
        let sizes: [(BoardType, Double, Double)] = [
            (.nes, 100, 110), (.atari2600, 41.3, 47), (.coco, 53.34, 44.45), (.atari5200, 50.8, 50.8),
            (.atari7800, 46.99, 82.55), (.atari8bit, 53.8, 64.8), (.atariST, 55.5, 25.5), (.jaguar, 94, 49.2),
            (.lynx, 55, 22),
        ]
        for (type, width, height) in sizes {
            doc.setBoardType(type)
            XCTAssertEqual(doc.board.width, width, "\(type)")
            XCTAssertEqual(doc.board.height, height, "\(type)")
            doc.setBoardSize(width: 200, height: 200)
            XCTAssertEqual(doc.board.width, width, "\(type) stays")
            XCTAssertEqual(doc.board.height, height, "\(type) stays")
            XCTAssertEqual(doc.board.card?.isFixedSize, true)
            for point in doc.board.outline {
                XCTAssertTrue(point.x >= 0 && point.x <= width + 1e-9 && point.y >= 0 && point.y <= height + 1e-9,
                              "\(type) \(point)")
            }
            let shape = CGMutablePath()
            shape.addLines(between: doc.board.outline)
            shape.closeSubpath()
            for finger in doc.board.fingers {
                XCTAssertTrue(shape.contains(CGPoint(x: finger.rect.midX, y: finger.rect.midY)),
                              "\(type) pin \(finger.name) on the board")
            }
        }
    }

    func testNESPins() throws {
        var board = BoardSettings()
        board.type = .nes
        XCTAssertEqual(board.fingers.count, 72)
        let pin1 = try finger(board, "1", .top), pin36 = try finger(board, "36", .top)
        XCTAssertEqual(pin1.rect.midX, 94.25, accuracy: 1e-9, "pin 1 at the right")
        XCTAssertEqual(pin1.rect.width, 3.0, accuracy: 1e-9)
        XCTAssertEqual(pin36.rect.midX, 5.75, accuracy: 1e-9)
        let pin2 = try finger(board, "2", .top), pin3 = try finger(board, "3", .top)
        XCTAssertEqual(pin2.rect.midX - pin3.rect.midX, 2.5, accuracy: 1e-9, "2.50 mm pitch")
        XCTAssertEqual(pin2.rect.width, 2.0, accuracy: 1e-9)
        XCTAssertEqual(try finger(board, "37", .bottom).rect, pin1.rect, "37 behind 1")
        XCTAssertEqual(pin1.rect.maxY, 109, accuracy: 1e-9)
        XCTAssertEqual(pin1.rect.height, 12, accuracy: 1e-9)
    }

    func testAtariPins() throws {
        var board = BoardSettings()
        board.type = .atari2600
        XCTAssertEqual(board.fingers.count, 24)
        let pin13 = try finger(board, "13", .top), pin24 = try finger(board, "24", .top)
        XCTAssertLessThan(pin13.rect.midX, pin24.rect.midX, "13 at the left of the component side")
        XCTAssertEqual(pin24.rect.midX - pin13.rect.midX, 11 * 2.54, accuracy: 1e-9)
        XCTAssertEqual(try finger(board, "1", .bottom).rect, pin24.rect, "1 behind 24")
        XCTAssertEqual(try finger(board, "12", .bottom).rect, pin13.rect, "12 behind 13")
        XCTAssertEqual((pin13.rect.midX + pin24.rect.midX) / 2, 41.3 / 2, accuracy: 1e-9, "centred")
    }

    func testCoCoPins() throws {
        var board = BoardSettings()
        board.type = .coco
        XCTAssertEqual(board.fingers.count, 40)
        let pin1 = try finger(board, "1", .top), pin39 = try finger(board, "39", .top)
        XCTAssertEqual(pin1.rect.midX, 2.54, accuracy: 1e-9)
        XCTAssertEqual(pin39.rect.midX, 50.8, accuracy: 1e-9)
        XCTAssertEqual(try finger(board, "2", .bottom).rect, pin1.rect, "2 behind 1")
        XCTAssertNil(board.fingers.first { $0.name == "2" && $0.side == .top }, "even pins on the back")
    }

    func testCartridgeFile() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.atari2600)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.type, .atari2600)
        XCTAssertEqual(opened.board.width, 41.3)
        // A stored size is replaced by the cartridge's.
        let odd = try BoardLayersTests.open(board: #"{ "type": "nes", "width": 20, "height": 30 }"#)
        XCTAssertEqual(odd.board.width, 100)
        XCTAssertEqual(odd.board.height, 110)
    }

    func testMoreAtariPins() throws {
        var board = BoardSettings()

        board.type = .atari8bit
        XCTAssertEqual(board.fingers.count, 30)
        XCTAssertEqual(try finger(board, "A", .bottom).rect, try finger(board, "1", .top).rect, "A behind 1")
        XCTAssertEqual(try finger(board, "S", .bottom).rect, try finger(board, "15", .top).rect, "S behind 15")
        XCTAssertNil(board.fingers.first { $0.name == "G" })

        board.type = .atari5200
        XCTAssertEqual(board.fingers.count, 36)
        XCTAssertEqual(try finger(board, "36", .bottom).rect, try finger(board, "1", .top).rect)
        XCTAssertLessThan(try finger(board, "18", .top).rect.height, try finger(board, "17", .top).rect.height,
                          "the interlock pin is shorter")

        board.type = .atari7800
        XCTAssertEqual(board.fingers.count, 32)
        XCTAssertEqual(try finger(board, "32", .bottom).rect, try finger(board, "1", .top).rect)
        let two = try finger(board, "2", .top).rect.midX, three = try finger(board, "3", .top).rect.midX
        XCTAssertEqual(three - two, 5.08, accuracy: 1e-9, "a skipped place")
        // 7800 pins 3–14 sit where the 2600's 13–24 do, measured from the centre.
        var a2600 = BoardSettings()
        a2600.type = .atari2600
        XCTAssertEqual(three - 46.99 / 2, try finger(a2600, "13", .top).rect.midX - 41.3 / 2, accuracy: 1e-9)

        board.type = .atariST
        XCTAssertEqual(board.fingers.count, 40)
        let one = try finger(board, "1", .top).rect.midX, odd3 = try finger(board, "3", .top).rect.midX
        XCTAssertEqual(one - odd3, 2.0, accuracy: 1e-9, "2.0 mm pitch, pin 1 at the right")
        XCTAssertEqual(try finger(board, "2", .bottom).rect.midX, one, accuracy: 1e-9)

        board.type = .jaguar
        XCTAssertEqual(board.fingers.count, 104)
        XCTAssertNil(board.fingers.first { $0.name == "A13" || $0.name == "B14" }, "the key")
        XCTAssertEqual(try finger(board, "A2", .top).rect.midX - (try finger(board, "A1", .top).rect.midX), 1.5,
                       accuracy: 1e-9)

        board.type = .lynx
        XCTAssertEqual(board.fingers.count, 34)
        XCTAssertTrue(board.fingers.allSatisfy { $0.side == .bottom }, "one face only")
        XCTAssertEqual(try finger(board, "1", .bottom).rect.midX, 1.27, accuracy: 1e-9)
    }

    // MARK: Modern expansion cards

    func testPCIExpress() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.pcie)
        XCTAssertEqual(doc.board.height, 111.15)
        XCTAssertEqual(doc.board.width, 167.65)
        for (lanes, pins) in [(1, 18), (4, 32), (8, 49), (16, 82)] {
            var options = doc.board.cardOptions
            options.pcieLanes = lanes
            doc.setCardOptions(options)
            XCTAssertEqual(doc.board.fingers.count, 2 * pins, "x\(lanes)")
            XCTAssertNotNil(doc.board.fingers.first { $0.name == "B\(pins)" })
            XCTAssertNil(doc.board.fingers.first { $0.name == "B\(pins + 1)" })
        }
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Card")
        let b1 = try finger(doc.board, "B1", .top), b11 = try finger(doc.board, "B11", .top)
        let b12 = try finger(doc.board, "B12", .top)
        XCTAssertEqual(b1.rect.midX, 45.65, accuracy: 1e-9)
        XCTAssertEqual(b12.rect.midX - b11.rect.midX, 3.0, accuracy: 1e-9, "the key between 11 and 12")
        XCTAssertEqual(try finger(doc.board, "A1", .bottom).rect.midX, b1.rect.midX, accuracy: 1e-9)
        XCTAssertLessThan(try finger(doc.board, "A1", .bottom).rect.height, b1.rect.height, "PRSNT1# is short")

        var options = doc.board.cardOptions
        options.lowProfile = true
        doc.setCardOptions(options)
        XCTAssertEqual(doc.board.height, 68.90)
        doc.setBoardSize(width: 300, height: 0)
        XCTAssertEqual(doc.board.width, 167.65, "a low-profile card's longest")

        let shape = CGMutablePath()
        shape.addLines(between: doc.board.outline)
        shape.closeSubpath()
        XCTAssertFalse(shape.contains(CGPoint(x: 57.15, y: 68.9 - 1)), "the key")
        XCTAssertTrue(shape.contains(CGPoint(x: 7.5, y: 68.9 - 6)), "the foot")
        XCTAssertFalse(shape.contains(CGPoint(x: 7.5, y: 68.9 - 2)), "the foot stops short of the edge")
        XCTAssertFalse(shape.contains(CGPoint(x: 25, y: 68.9 - 6)), "between the foot and the PCI block")
    }

    func testM2() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.m2)
        XCTAssertEqual(doc.board.width, 22)
        XCTAssertEqual(doc.board.height, 80, "2280 by default")
        var options = doc.board.cardOptions
        options.m2Length = 42
        options.m2Key = .bm
        doc.setCardOptions(options)
        XCTAssertEqual(doc.board.height, 42)
        XCTAssertEqual(doc.board.fingers.count, 75 - 16, "B and M drop 8 pins each")
        XCTAssertNil(doc.board.fingers.first { $0.name == "12" || $0.name == "66" })
        let pin1 = try finger(doc.board, "1", .top), pin75 = try finger(doc.board, "75", .top)
        XCTAssertEqual(pin1.rect.midX, 11 + 9.25, accuracy: 1e-9, "pin 1 at the right")
        XCTAssertEqual(pin75.rect.midX, 11 - 9.25, accuracy: 1e-9)
        let pin2 = try finger(doc.board, "2", .bottom)
        XCTAssertEqual(pin1.rect.midX - pin2.rect.midX, 0.25, accuracy: 1e-9)
        XCTAssertGreaterThan(pin2.rect.height, pin1.rect.height, "bottom pads reach further")

        let shape = CGMutablePath()
        shape.addLines(between: doc.board.outline)
        shape.closeSubpath()
        XCTAssertFalse(shape.contains(CGPoint(x: 11 + 5.625, y: 42 - 1)), "the B key")
        XCTAssertFalse(shape.contains(CGPoint(x: 11 - 6.125, y: 42 - 1)), "the M key")
        XCTAssertFalse(shape.contains(CGPoint(x: 11, y: 0.5)), "the screw notch")
        XCTAssertFalse(shape.contains(CGPoint(x: 0.3, y: 42 - 1)), "the strip is narrower")
        XCTAssertTrue(shape.contains(CGPoint(x: 0.3, y: 42 - 5)), "full width above it")
    }

    func testMiniPCIe() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.miniPCIe)
        XCTAssertEqual(doc.board.width, 30)
        XCTAssertEqual(doc.board.height, 50.95)
        XCTAssertEqual(doc.board.fingers.count, 52)
        XCTAssertEqual(try finger(doc.board, "1", .top).rect.midX, 3.2, accuracy: 1e-9)
        XCTAssertEqual(try finger(doc.board, "52", .bottom).rect.midX, 26.8, accuracy: 1e-9)
        XCTAssertEqual(try finger(doc.board, "17", .top).rect.midX - (try finger(doc.board, "15", .top).rect.midX),
                       4.0, accuracy: 1e-9, "the key between 15 and 17")
        var options = doc.board.cardOptions
        options.halfSize = true
        doc.setCardOptions(options)
        XCTAssertEqual(doc.board.height, 26.8)
    }

    func testCardOptionsFile() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.m2)
        var options = doc.board.cardOptions
        options.m2Length = 30
        options.m2Key = .ae
        doc.setCardOptions(options)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.type, .m2)
        XCTAssertEqual(opened.board.cardOptions, doc.board.cardOptions)
        XCTAssertEqual(opened.board.height, 30)
        XCTAssertNil(try SchematicFile.decode(SchematicFile(document: TestSupport.newDocument(), fileURL: nil)
            .encoded()).board?.cardOptions)
        XCTAssertEqual(BoardLayersTests.pcbError {
            _ = try BoardLayersTests.open(board: #"{ "type": "pcie", "cardOptions": { "lanes": 2 } }"#)
        }, "card lanes 2 must be 1, 4, 8 or 16")
        XCTAssertEqual(BoardLayersTests.pcbError {
            _ = try BoardLayersTests.open(board: #"{ "type": "m2", "cardOptions": { "length": 50 } }"#)
        }, "card length 50 must be 30, 42, 60, 80 or 110")
    }

    func testMemoryModules() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.sodimm)
        let expected: [(BoardType, MemoryGeneration, Double, Double, Int)] = [
            (.sodimm, .ddr3, 67.6, 30, 204), (.sodimm, .ddr4, 69.6, 30, 260), (.sodimm, .ddr5, 69.6, 30, 262),
            (.dimm, .ddr3, 133.35, 30, 240), (.dimm, .ddr4, 133.35, 31.25, 288), (.dimm, .ddr5, 133.35, 31.25, 288),
        ]
        for (type, memory, width, height, pins) in expected {
            doc.setBoardType(type)
            var options = doc.board.cardOptions
            options.memory = memory
            doc.setCardOptions(options)
            XCTAssertEqual(doc.board.width, width, "\(type) \(memory)")
            XCTAssertEqual(doc.board.height, height, "\(type) \(memory)")
            XCTAssertEqual(doc.board.fingers.count, pins, "\(type) \(memory)")
            let shape = CGMutablePath()
            shape.addLines(between: doc.board.outline)
            shape.closeSubpath()
            for finger in doc.board.fingers {
                XCTAssertTrue(shape.contains(CGPoint(x: finger.rect.midX, y: finger.rect.midY)),
                              "\(type) \(memory) pin \(finger.name)")
            }
        }

        // SO-DIMM rows: each even pin behind the odd one, half a pitch right.
        doc.setBoardType(.sodimm)
        var options = doc.board.cardOptions
        options.memory = .ddr4
        doc.setCardOptions(options)
        let one = try finger(doc.board, "1", .top), two = try finger(doc.board, "2", .bottom)
        XCTAssertEqual(one.rect.midX, 1.425, accuracy: 1e-9)
        XCTAssertEqual(two.rect.midX - one.rect.midX, 0.25, accuracy: 1e-9)
        XCTAssertEqual(try finger(doc.board, "145", .top).rect.midX - (try finger(doc.board, "143", .top).rect.midX),
                       2.5, accuracy: 1e-9, "the key between 143 and 145")

        // DDR4 DIMM: 145 behind 1; centre fingers longer than end ones.
        doc.setBoardType(.dimm)
        XCTAssertEqual(try finger(doc.board, "145", .bottom).rect, try finger(doc.board, "1", .top).rect)
        XCTAssertGreaterThan(try finger(doc.board, "70", .top).rect.height,
                             try finger(doc.board, "1", .top).rect.height)

        // DDR5: the registered key sits further right.
        options = doc.board.cardOptions
        options.memory = .ddr5
        doc.setCardOptions(options)
        let shape = CGMutablePath()
        shape.addLines(between: doc.board.outline)
        shape.closeSubpath()
        XCTAssertFalse(shape.contains(CGPoint(x: 68.10, y: 31.25 - 1)), "UDIMM key")
        options.registered = true
        doc.setCardOptions(options)
        let rdimm = CGMutablePath()
        rdimm.addLines(between: doc.board.outline)
        rdimm.closeSubpath()
        XCTAssertTrue(rdimm.contains(CGPoint(x: 68.10, y: 31.25 - 1)))
        XCTAssertFalse(rdimm.contains(CGPoint(x: 70.55, y: 31.25 - 1)), "RDIMM key")
        XCTAssertEqual(try BoardLayersTests.roundTrip(doc).board.cardOptions, doc.board.cardOptions)
    }
}
