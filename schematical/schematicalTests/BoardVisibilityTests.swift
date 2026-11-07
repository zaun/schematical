import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-16-C: which layers the PCB canvas draws, and what a press can hit.
@MainActor
final class BoardVisibilityTests: XCTestCase {
    private func doc() -> SchematicDocument { TestSupport.newDocument() }

    private func hiding(_ layers: BoardViewLayer..., board: Int = 4) -> BoardVisibility {
        BoardVisibility(hidden: Set(layers), copperLayers: CopperLayer.allCases(for: board))
    }

    /// The §3 table, row by row, for a part on each side.
    func testTableForEachSide() {
        let all = hiding()
        for side in BoardSide.allCases {
            let own = BoardViewLayer.copper(side.copperLayer)
            let other = BoardViewLayer.copper(side == .top ? .bottom : .top)

            // Surface-mount pad: its side's layer.
            XCTAssertTrue(all.showsPad(drill: 0, side: side))
            XCTAssertFalse(hiding(own).showsPad(drill: 0, side: side))
            XCTAssertTrue(hiding(other, .copper(.inner(2))).showsPad(drill: 0, side: side))

            // Through-hole pad: any copper layer.
            XCTAssertTrue(hiding(own).showsPad(drill: 0.8, side: side))
            XCTAssertTrue(hiding(.copper(.top), .copper(.bottom), .copper(.inner(3))).showsPad(drill: 0.8, side: side))
            XCTAssertFalse(hiding(.copper(.top), .copper(.bottom), .copper(.inner(2)), .copper(.inner(3)))
                .showsPad(drill: 0.8, side: side))

            // Marks: their layer and their side's copper.
            for (mark, view) in [(MarkLayer.silkscreen, BoardViewLayer.silkscreen), (.document, .document)] {
                XCTAssertTrue(all.showsMark(mark, side: side))
                XCTAssertFalse(hiding(view).showsMark(mark, side: side))
                XCTAssertFalse(hiding(own).showsMark(mark, side: side))
                XCTAssertTrue(hiding(other).showsMark(mark, side: side))
            }
            XCTAssertFalse(all.showsMark(.hidden, side: side))

            // Texts: their layer and their side's copper.
            for (layer, view) in [(TextLayer.silkscreen, BoardViewLayer.silkscreen), (.document, .document)] {
                let text = BoardText(id: UUID(), text: "A", anchor: .zero, size: 50, layer: layer, side: side)
                XCTAssertTrue(all.showsText(text))
                XCTAssertFalse(hiding(view).showsText(text))
                XCTAssertFalse(hiding(own).showsText(text))
                XCTAssertTrue(hiding(other).showsText(text))
            }
        }

        // Plane fill: its layer, and only while it is a Plane.
        let plane = LayerStyle(color: LayerColor(red: 1, green: 2, blue: 3), type: .plane)
        let wire = LayerStyle(color: LayerColor(red: 1, green: 2, blue: 3), type: .wire)
        XCTAssertTrue(all.showsPlane(.inner(2), style: plane))
        XCTAssertFalse(all.showsPlane(.inner(2), style: wire))
        XCTAssertFalse(hiding(.copper(.inner(2))).showsPlane(.inner(2), style: plane))

        // Trace: its layer.
        XCTAssertTrue(all.showsTrace(on: .inner(3)))
        XCTAssertFalse(hiding(.copper(.inner(3))).showsTrace(on: .inner(3)))

        // showsAnyCopper uses the board's own layers.
        XCTAssertFalse(hiding(.copper(.top), .copper(.bottom), board: 2).showsAnyCopper)
        XCTAssertTrue(hiding(.copper(.top), .copper(.bottom), board: 4).showsAnyCopper)
    }

    func testHitTestsSkipHiddenItems() throws {
        let doc = doc()
        let smd = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        let bounds = try XCTUnwrap(try XCTUnwrap(doc.chips.first { $0.id == smd.id }).boardBounds)
        let inside = CGPoint(x: bounds.midX, y: bounds.midY)
        XCTAssertEqual(doc.footprint(at: inside, visible: hiding())?.id, smd.id)
        // Its side and the silkscreen hidden: only the document outline is
        // left, and that needs the side too, so nothing of it is drawn.
        XCTAssertNil(doc.footprint(at: inside, visible: hiding(.copper(.top), .silkscreen)))
        // Its pads hidden, but its silkscreen marks need Top as well.
        XCTAssertNil(doc.footprint(at: inside, visible: hiding(.copper(.top))))
        // Silkscreen and document hidden: the pads still show.
        XCTAssertEqual(doc.footprint(at: inside, visible: hiding(.silkscreen, .document))?.id, smd.id)

        let text = doc.addText(at: CGPoint(x: 200, y: 200))
        let rect = try XCTUnwrap(doc.texts.first).boardRect
        let onText = CGPoint(x: rect.midX, y: rect.midY)
        XCTAssertEqual(doc.text(at: onText, visible: hiding())?.id, text)
        XCTAssertNil(doc.text(at: onText, visible: hiding(.silkscreen)))
        XCTAssertNil(doc.text(at: onText, visible: hiding(.copper(.top))))
    }

    /// A through-hole footprint with no marks stays hittable while any
    /// copper layer shows.
    func testThroughHoleOnlyFootprintIsHittableWhileAnyCopperShows() throws {
        let doc = doc()
        doc.setLayerCount(4)
        let dip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        doc.setFootprintMarks(dip.id, outline: .hidden, pin1: .hidden, reference: .hidden)
        let chip = try XCTUnwrap(doc.chips.first { $0.id == dip.id })
        XCTAssertTrue(chip.boardPads.allSatisfy { $0.drill > 0 })
        let pad = try XCTUnwrap(chip.boardPads.first)
        let onPad = CGPoint(x: pad.rect.midX, y: pad.rect.midY)
        XCTAssertEqual(doc.footprint(at: onPad, visible: hiding(.copper(.top), .copper(.bottom), .copper(.inner(2))))?.id,
                       dip.id)
        XCTAssertNil(doc.footprint(at: onPad,
                                   visible: hiding(.copper(.top), .copper(.bottom), .copper(.inner(2)), .copper(.inner(3)))))
    }
}
