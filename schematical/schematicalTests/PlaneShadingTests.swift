import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// What a Plane's shading leaves out (`PCBCanvasView.planeCuts`): the
/// copper not on its net, every non-plated hole, and the drills of what
/// joins it; and the Ratsnest view layer.
@MainActor
final class PlaneShadingTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    func testCutsByWhatEachHoleIs() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let ground = try TestSupport.place(doc, "GND", at: p(100, 400))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
        let placed = try XCTUnwrap(doc.chips.first { $0.id == led.id })
        guard placed.boardPads.allSatisfy({ $0.drill > 0 }) else {
            throw XCTSkip("the LED footprint is not through-hole")
        }
        let cathode = placed.part.pins[try TestSupport.pin(led, "K").index].designator
        let anode = placed.part.pins[try TestSupport.pin(led, "A").index].designator
        let k = try XCTUnwrap(placed.boardPads.first { $0.designator == cathode })
        let a = try XCTUnwrap(placed.boardPads.first { $0.designator == anode })
        func centre(_ rect: CGRect) -> CGPoint { p(rect.midX, rect.midY) }

        doc.setLayerNet(.inner(2), .ground)
        let bare = doc.addHole(at: p(10, 10))
        let plated = doc.addHole(at: p(30, 10))
        doc.setHolePlated(plated, true)
        doc.setHoleNet(plated, .ground)
        let via = try XCTUnwrap(doc.addVia(at: p(50, 10)))
        let holes = doc.holes, vias = doc.vias
        let bareHole = try XCTUnwrap(holes.first { $0.id == bare })
        let groundHole = try XCTUnwrap(holes.first { $0.id == plated })
        let lone = try XCTUnwrap(vias.first { $0.id == via })

        let cuts = PCBCanvasView.planeCuts(on: .inner(2), copper: doc.connectivity())
        // The anode isn't on GND: its whole pad is cleared.
        let anodeEdge = p(a.rect.maxX - 0.05, a.rect.midY)
        XCTAssertTrue(cuts.copper.contains(anodeEdge))
        // The cathode joins: only its drill, nothing round it.
        XCTAssertTrue(cuts.drills.contains(centre(k.rect)))
        XCTAssertFalse(cuts.copper.contains(centre(k.rect)))
        XCTAssertFalse(cuts.drills.contains(p(k.rect.maxX - 0.05, k.rect.midY)), "the land round it is plane")
        // A bare hole is cut by its drill, as a board edge.
        XCTAssertTrue(cuts.edges.contains(bareHole.centre))
        XCTAssertFalse(cuts.edges.contains(p(bareHole.centre.x + bareHole.drill / 2 + 0.1, bareHole.centre.y)))
        // A plated hole on GND joins: its drill only.
        XCTAssertTrue(cuts.drills.contains(groundHole.centre))
        XCTAssertFalse(cuts.drills.contains(p(groundHole.centre.x + groundHole.drill / 2 + 0.2, groundHole.centre.y)))
        // A via on no net is cleared, metal and all.
        XCTAssertTrue(cuts.copper.contains(p(lone.centre.x + lone.diameter / 2 - 0.02, lone.centre.y)))

        // On a Plane with no net, everything plated is cleared.
        let none = PCBCanvasView.planeCuts(on: .inner(3), copper: doc.connectivity())
        XCTAssertTrue(none.copper.contains(centre(k.rect)))
        XCTAssertTrue(none.copper.contains(p(groundHole.centre.x + groundHole.drill / 2 + 0.2, groundHole.centre.y)))
        XCTAssertTrue(none.drills.isEmpty)
    }

    func testRatsnestCanBeHidden() {
        var visible = BoardVisibility()
        XCTAssertTrue(visible.shows(.ratsnest))
        visible.hidden.insert(.ratsnest)
        XCTAssertFalse(visible.shows(.ratsnest))
        XCTAssertTrue(visible.showsAnyCopper, "only the lines go")
        XCTAssertEqual(BoardViewLayer.ratsnest.title, "Ratsnest")
    }

    func testBoardFillCanBeHidden() {
        var visible = BoardVisibility()
        XCTAssertTrue(visible.shows(.board))
        visible.hidden.insert(.board)
        XCTAssertFalse(visible.shows(.board))
        XCTAssertTrue(visible.showsAnyCopper)
        XCTAssertEqual(BoardViewLayer.board.title, "PCB")
    }

    /// A Plane's clearance is its own once set, the DRC's minimum till then.
    func testPlaneClearance() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        var rules = DesignRules.customDefault
        rules.minClearance = 0.15
        XCTAssertNil(doc.board.style(of: .inner(2)).clearance)
        XCTAssertEqual(doc.board.style(of: .inner(2)).clearance(rules: rules), 0.15)

        doc.setLayerClearance(.inner(2), 0.5)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Plane Clearance")
        XCTAssertEqual(doc.board.style(of: .inner(2)).clearance(rules: rules), 0.5)
        XCTAssertEqual(doc.board.style(of: .inner(3)).clearance(rules: rules), 0.15, "each Plane its own")
        doc.setLayerClearance(.inner(2), 50)
        XCTAssertEqual(doc.board.style(of: .inner(2)).clearance, LayerStyle.clearanceRange.upperBound)
        doc.setLayerClearance(.top, 1)
        XCTAssertNil(doc.board.style(of: .top).clearance, "only an inner layer")

        // Kept while a Wire layer, so it comes back with the Plane.
        doc.setLayerClearance(.inner(2), 0.8)
        doc.setLayerType(.inner(2), .wire)
        XCTAssertEqual(PlanesSection.planes(of: doc.board), [.inner(3)])
        doc.setLayerType(.inner(2), .plane)
        XCTAssertEqual(doc.board.style(of: .inner(2)).clearance, 0.8)
        XCTAssertEqual(PlanesSection.planes(of: doc.board), [.inner(2), .inner(3)])

        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.style(of: .inner(2)).clearance, 0.8)
        XCTAssertNil(opened.board.style(of: .inner(3)).clearance)

        doc.setLayerClearance(.inner(2), nil)
        XCTAssertNil(doc.board.style(of: .inner(2)).clearance)

        let layers = #"{ "layers": 4, "layerStyles": [ { "layer": "inner2", "clearance": 20 } ] }"#
        XCTAssertEqual(BoardLayersTests.pcbError { _ = try BoardLayersTests.open(board: layers) },
                       "layer inner2 clearance 20 must be 0.05 to 10 mm")
    }
}
