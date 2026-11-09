import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-16-B: parts and texts on the bottom side, mirrored.
@MainActor
final class BoardSideTests: XCTestCase {
    private func doc() -> SchematicDocument { TestSupport.newDocument() }

    private func chip(_ doc: SchematicDocument, _ id: UUID) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.id == id })
    }

    private func pad(_ chip: PlacedChip, _ designator: String) throws -> PlacedPad {
        try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
    }

    private func assertEqual(_ a: CGPoint, _ b: CGPoint, file: StaticString = #filePath, line: UInt = #line) {
        XCTAssertEqual(a.x, b.x, accuracy: 1e-9, file: file, line: line)
        XCTAssertEqual(a.y, b.y, accuracy: 1e-9, file: file, line: line)
    }

    private func centre(_ rect: CGRect) -> CGPoint { CGPoint(x: rect.midX, y: rect.midY) }

    /// The SOIC-14's pad 1 at rotation 0 on Bottom sits at the mirrored x of
    /// its Top position, about the body centre.
    func testBottomMirrorsPad1() throws {
        var chip = try TestSupport.place(doc(), "SN74HC00D", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(chip.footprint?.id, "SOIC-14")
        var placement = try XCTUnwrap(chip.board)
        placement.rotation = 0
        chip.board = placement
        let top = try pad(chip, "1")
        placement.side = .bottom
        chip.board = placement
        let bottom = try pad(chip, "1")
        let c = placement.centre
        assertEqual(centre(bottom.rect), CGPoint(x: 2 * c.x - top.rect.midX, y: top.rect.midY))
        XCTAssertEqual(bottom.rect.size, top.rect.size)
    }

    /// At 90° a Bottom pad 1 is `rotate(mirror(p), 90) + centre`.
    func testBottomAtNinetyMirrorsBeforeTurning() throws {
        var chip = try TestSupport.place(doc(), "SN74HC00D", at: CGPoint(x: 100, y: 100))
        let local = try XCTUnwrap(chip.footprint?.pads.first { $0.designator == "1" }).centre
        var placement = try XCTUnwrap(chip.board)
        placement.rotation = 90
        placement.side = .bottom
        chip.board = placement
        let expected = BoardPlacement.rotate(CGPoint(x: -local.x, y: local.y), by: 90)
        assertEqual(centre(try pad(chip, "1").rect),
                    CGPoint(x: placement.centre.x + expected.x, y: placement.centre.y + expected.y))
    }

    func testCornersMirror() throws {
        typealias C = Footprint.Corners
        XCTAssertEqual(C.topLeft.mirrored(true), .topRight)
        XCTAssertEqual(C.bottomRight.mirrored(true), .bottomLeft)
        XCTAssertEqual(([.topLeft, .bottomLeft] as C).mirrored(true), [.topRight, .bottomRight])
        XCTAssertEqual(C.topLeft.mirrored(false), .topLeft)

        // A chamfered pad of a real footprint swaps its corners on Bottom.
        var chip = try TestSupport.place(doc(), "SN74AUP1G00DPWR", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(chip.footprint?.id, "X2SON-5-DPW")
        var placement = try XCTUnwrap(chip.board)
        placement.rotation = 0
        chip.board = placement
        let topPads = chip.boardPads.filter { !$0.corners.isEmpty }
        XCTAssertFalse(topPads.isEmpty)
        placement.side = .bottom
        chip.board = placement
        var swapped = 0
        for top in topPads {
            let bottom = try pad(chip, try XCTUnwrap(top.designator))
            XCTAssertEqual(bottom.corners, top.corners.mirrored(true))
            if bottom.corners != top.corners { swapped += 1 }
        }
        XCTAssertGreaterThan(swapped, 0, "some pad has a one-sided chamfer")
    }

    func testSideChangeKeepsOrigin() throws {
        let doc = doc()
        doc.setLayerCount(2)
        let placed = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        for origin in FootprintOrigin.allCases {
            for rotation in [0, 90] {
                doc.setFootprintOrigin(placed.id, origin)
                while try chip(doc, placed.id).board?.rotation != rotation { doc.rotateFootprint(placed.id) }
                doc.moveFootprintOrigin(placed.id, to: CGPoint(x: 20, y: 20))
                let before = try XCTUnwrap(try chip(doc, placed.id).boardOrigin)
                doc.setFootprintSide(placed.id, .bottom)
                XCTAssertEqual(try chip(doc, placed.id).board?.side, .bottom)
                assertEqual(try XCTUnwrap(try chip(doc, placed.id).boardOrigin), before)
                doc.setFootprintSide(placed.id, .top)
                assertEqual(try XCTUnwrap(try chip(doc, placed.id).boardOrigin), before)
            }
        }
    }

    /// Changing side mirrors every pad left to right about the body centre,
    /// at every rotation: at 90° the SOIC's pad rows stay on the same edges.
    /// (Pads are measured from the body centre, which the origin snap may
    /// shift.)
    func testSideChangeMirrorsLeftToRightAtEveryRotation() throws {
        let doc = doc()
        doc.setLayerCount(2)
        let placed = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        for rotation in BoardPlacement.rotations {
            while try chip(doc, placed.id).board?.rotation != rotation { doc.rotateFootprint(placed.id) }
            let top = try chip(doc, placed.id)
            let tc = try XCTUnwrap(top.board).centre
            doc.setFootprintSide(placed.id, .bottom)
            let bottom = try chip(doc, placed.id)
            XCTAssertEqual(bottom.board?.side, .bottom)
            let bc = try XCTUnwrap(bottom.board).centre
            for designator in ["1", "7", "8", "14"] {
                let t = centre(try pad(top, designator).rect), b = centre(try pad(bottom, designator).rect)
                assertEqual(CGPoint(x: b.x - bc.x, y: b.y - bc.y), CGPoint(x: tc.x - t.x, y: t.y - tc.y))
            }
            doc.setFootprintSide(placed.id, .top)
            XCTAssertEqual(try chip(doc, placed.id).board?.rotation, rotation)
        }
    }

    func testOneLayerBoardKeepsTop() throws {
        let doc = doc()
        doc.setLayerCount(1)
        let placed = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        doc.setFootprintSide(placed.id, .bottom)
        XCTAssertEqual(try chip(doc, placed.id).board?.side, .top)
        let text = doc.addText(at: .zero)
        doc.setTextSide(text, .bottom)
        XCTAssertEqual(doc.texts.first?.side, .top)
    }

    func testTextBoxDoesNotMoveWithSide() throws {
        let doc = doc()
        let id = doc.addText(at: CGPoint(x: 5, y: 5))
        doc.setTextRotation(id, 90)
        let before = try XCTUnwrap(doc.texts.first).boardRect
        doc.setTextSide(id, .bottom)
        XCTAssertEqual(doc.texts.first?.side, .bottom)
        XCTAssertEqual(doc.texts.first?.boardRect, before)
    }

    func testSideRoundTrip() throws {
        let doc = doc()
        let placed = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        doc.setFootprintSide(placed.id, .bottom)
        let text = doc.addText(at: .zero)
        doc.setTextSide(text, .bottom)
        let placement = try XCTUnwrap(try chip(doc, placed.id).board)

        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.chips.first { $0.id == placed.id }?.board, placement)
        XCTAssertEqual(opened.texts.first?.side, .bottom)
    }

    func testErrors() throws {
        let partID = UUID(), textID = UUID()
        func part(_ pcb: String) -> String {
            """
            [ { "id": "\(partID)", "reference": "U2", "partNumber": "SN74HC00D", "x": 0, "y": 0,
                "flipped": false, "pcb": \(pcb) } ]
            """
        }
        func text(_ extra: String) -> String {
            #"{ "layers": LAYERS, "texts": [ { "id": "\#(textID)", "text": "A", "x": 0, "y": 0\#(extra) } ] }"#
        }
        XCTAssertEqual(BoardLayersTests.pcbError {
            _ = try BoardLayersTests.open(board: #"{ "layers": 2 }"#,
                                          parts: part(#"{ "x": 0, "y": 0, "rotation": 0, "side": "left" }"#))
        }, #"U2 side "left" must be top or bottom"#)
        XCTAssertEqual(BoardLayersTests.pcbError {
            _ = try BoardLayersTests.open(board: #"{ "layers": 1 }"#,
                                          parts: part(#"{ "x": 0, "y": 0, "rotation": 0, "side": "bottom" }"#))
        }, "U2 is on the bottom of a 1-layer board")
        XCTAssertEqual(BoardLayersTests.pcbError {
            _ = try BoardLayersTests.open(board: text(#", "side": "left""#).replacingOccurrences(of: "LAYERS", with: "2"))
        }, "text \(textID.uuidString) side \"left\" must be top or bottom")
        XCTAssertEqual(BoardLayersTests.pcbError {
            _ = try BoardLayersTests.open(board: text(#", "side": "bottom""#).replacingOccurrences(of: "LAYERS", with: "1"))
        }, "text \(textID.uuidString) is on the bottom of a 1-layer board")
        // A missing side is Top.
        let opened = try BoardLayersTests.open(board: text("").replacingOccurrences(of: "LAYERS", with: "1"))
        XCTAssertEqual(opened.texts.first?.side, .top)
    }

    func testOneLayerRefusedWhileSomethingIsOnBottom() throws {
        let doc = doc()
        let placed = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        XCTAssertNil(doc.canSetLayerCount(1))
        doc.setFootprintSide(placed.id, .bottom)
        XCTAssertEqual(doc.canSetLayerCount(1), "Move every part and text to Top first.")
        XCTAssertNil(doc.canSetLayerCount(4))
        doc.setLayerCount(1)
        XCTAssertEqual(doc.board.layers, 2)
        doc.setFootprintSide(placed.id, .top)
        let text = doc.addText(at: .zero)
        doc.setTextSide(text, .bottom)
        XCTAssertNotNil(doc.canSetLayerCount(1))
        doc.setTextSide(text, .top)
        doc.setLayerCount(1)
        XCTAssertEqual(doc.board.layers, 1)
    }
}
