import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The sidebar panels of a pour and a keep-out (plan-52-F).
@MainActor
final class PourPanelTests: XCTestCase {
    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect { CGRect(x: x, y: y, width: w, height: h) }

    /// Two LEDs on a 4-layer board, cathodes on ground, anodes together.
    private func board() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        var leds: [PlacedChip] = []
        for n in 0..<2 {
            let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100 + 300 * CGFloat(n), y: 100))
            let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 100 + 300 * CGFloat(n), y: 400))
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
            doc.moveFootprint(led.id, to: CGPoint(x: 30 + 30 * CGFloat(n), y: 40))
            leds.append(led)
        }
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(leds[0], "A")), .pin(try TestSupport.pin(leds[1], "A"))))
        return doc
    }

    func testTheSidebarShowsAPourOrAKeepoutWhenItIsTheSoleSelection() throws {
        let doc = TestSupport.newDocument()
        let pour = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 20), layer: .top))
        XCTAssertEqual(PCBSidebarView.content(for: doc), .pour(pour))
        let keepout = doc.addKeepout(rect: r(40, 10, 5, 5))
        XCTAssertEqual(PCBSidebarView.content(for: doc), .keepout(keepout))
        doc.selectBoardItems([.pour(pour), .keepout(keepout)])
        XCTAssertEqual(PCBSidebarView.content(for: doc), .selection)
        doc.clearBoardSelection()
        XCTAssertEqual(PCBSidebarView.content(for: doc), .board)
    }

    func testLayerChoicesAreTheTraceLayers() {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        XCTAssertEqual(PourSection.layerChoices(doc.board), [.top, .bottom], "inner layers start as Planes")
        doc.setLayerType(.inner(2), .wire)
        XCTAssertEqual(PourSection.layerChoices(doc.board), [.top, .inner(2), .bottom])
    }

    func testFillSummary() throws {
        let doc = try board()
        guard doc.chips.first?.boardPads.allSatisfy({ $0.drill > 0 }) == true else {
            throw XCTSkip("the LED footprint is not through-hole")
        }
        let id = try XCTUnwrap(doc.addPour(rect: r(0, 0, doc.board.width, doc.board.height), layer: .top))
        var pour = try XCTUnwrap(doc.pours.first)
        XCTAssertEqual(PourSection.fillSummary(pour, copper: doc.connectivity()), "Fill: 1 piece, 2 pins joined")
        doc.setPourNet(id, nil)
        pour = try XCTUnwrap(doc.pours.first)
        XCTAssertEqual(PourSection.fillSummary(pour, copper: doc.connectivity()), "No fill: choose a net")
        doc.setPourNet(id, .rail(mv: 9000))
        pour = try XCTUnwrap(doc.pours.first)
        XCTAssertEqual(PourSection.fillSummary(pour, copper: doc.connectivity()), "No fill: its net is not in the schematic")
    }

    func testFitToBoardIsOneUndoStep() throws {
        let doc = TestSupport.newDocument()
        let pour = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 20), layer: .top))
        let keepout = doc.addKeepout(rect: r(40, 10, 5, 5))
        let bounds = try XCTUnwrap(doc.boardOutlineBounds)
        doc.fitPourToBoard(pour)
        XCTAssertEqual(doc.undoName, "Fit to Board")
        XCTAssertEqual(doc.pours[0].rect.width, bounds.width, accuracy: doc.board.traceGrid)
        XCTAssertEqual(doc.pours[0].rect.height, bounds.height, accuracy: doc.board.traceGrid)
        doc.fitKeepoutToBoard(keepout)
        XCTAssertEqual(doc.keepouts[0].rect.minX, bounds.minX, accuracy: doc.board.traceGrid)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.keepouts[0].rect.width, 5, accuracy: doc.board.traceGrid)
    }

    func testKeepoutLayerSwitches() {
        let doc = TestSupport.newDocument()
        let board = doc.board
        XCTAssertEqual(board.copperLayers, [.top, .bottom])
        XCTAssertEqual(PourSection.setting(.top, on: false, in: nil, board: board), [.bottom])
        XCTAssertNil(PourSection.setting(.top, on: true, in: [.bottom], board: board), "all layers on is nil again")
        XCTAssertEqual(PourSection.setting(.bottom, on: false, in: [.bottom], board: board), [])
    }
}
