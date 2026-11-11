import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// A Plane layer's net: choosing it, saving it, and what it does to the
/// board's copper (the ratsnest, and the holes it is cut away around).
@MainActor
final class PlaneNetTests: XCTestCase {
    /// Two LEDs on a 4-layer board, each cathode wired to a GND symbol of
    /// its own, and anode A of one wired to anode A of the other.
    private func board() throws -> (doc: SchematicDocument, one: PlacedChip, two: PlacedChip) {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        var leds: [PlacedChip] = []
        for n in 0..<2 {
            let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100 + 300 * n, y: 100))
            let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 100 + 300 * n, y: 400))
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
            leds.append(led)
        }
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(leds[0], "A")), .pin(try TestSupport.pin(leds[1], "A"))))
        return (doc, leds[0], leds[1])
    }

    func testChoicesListGroundThenSignalNets() throws {
        let (doc, one, _) = try board()
        let choices = doc.planeNetChoices()
        XCTAssertEqual(choices.first?.net, .ground)
        XCTAssertEqual(choices.first?.title, "GND")
        let signal = try XCTUnwrap(choices.last)
        let anode = one.part.pins[try TestSupport.pin(one, "A").index].designator
        XCTAssertEqual(signal.net, .pin(chip: one.id, designator: anode))
        XCTAssertEqual(choices.count, 2, "\(choices.map(\.title))")
        XCTAssertEqual(SchematicDocument.planeNetTitle(rail: 3300), "3.3 V")
    }

    func testGroundPlaneJoinsGroundPadsOnly() throws {
        let (doc, _, _) = try board()
        guard doc.chips.first?.boardPads.allSatisfy({ $0.drill > 0 }) == true else {
            throw XCTSkip("the LED footprint is not through-hole")
        }
        XCTAssertEqual(doc.ratsnest().count, 2, "GND and the anodes are unrouted")
        doc.setLayerNet(.inner(2), .ground)
        XCTAssertEqual(doc.ratsnest().count, 1, "the plane routes GND; the anodes stay")

        let copper = doc.connectivity()
        let cleared = copper.planeClearances(on: .inner(2))
        let ground = copper.planeNets[.inner(2)]
        XCTAssertNotNil(ground)
        XCTAssertEqual(cleared.pads.count, 2, "the two anode pads are cut away")
        XCTAssertTrue(cleared.pads.allSatisfy { copper.netOfPad[$0] != ground })
        // Layer 3 is a Plane on no net: every hole is cut.
        XCTAssertEqual(copper.planeClearances(on: .inner(3)).pads.count, 4)
    }

    func testWireLayerKeepsItsNetButDoesNotJoin() throws {
        let (doc, _, _) = try board()
        doc.setLayerNet(.inner(2), .ground)
        doc.setLayerType(.inner(2), .wire)
        XCTAssertEqual(doc.board.style(of: .inner(2)).net, .ground)
        XCTAssertEqual(doc.ratsnest().count, 2)
        XCTAssertTrue(doc.connectivity().planeNets.isEmpty)
    }

    func testViaOnTheNetJoinsThePlane() throws {
        let (doc, _, _) = try board()
        doc.setLayerNet(.inner(2), .ground)
        let lone = try XCTUnwrap(doc.addVia(at: CGPoint(x: 50, y: 50)))
        let copper = doc.connectivity()
        let index = try XCTUnwrap(copper.vias.firstIndex { $0.id == lone })
        XCTAssertEqual(copper.planeClearances(on: .inner(2)).vias, [index], "a via on no net is cut away")
    }

    func testNetIsOneUndoStepOnThePCB() throws {
        let (doc, _, _) = try board()
        doc.setLayerNet(.inner(2), .rail(mv: 5000))
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Plane Net")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertNil(doc.board.style(of: .inner(2)).net)
    }

    func testNetRoundTrips() throws {
        let (doc, one, _) = try board()
        doc.setLayerNet(.inner(2), .ground)
        doc.setLayerNet(.inner(3), .pin(chip: one.id, designator: "1"))
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.style(of: .inner(2)).net, .ground)
        XCTAssertEqual(opened.board.style(of: .inner(3)).net, .pin(chip: one.id, designator: "1"))
        for net in [PlaneNet.ground, .rail(mv: 3300), .pin(chip: one.id, designator: "A1")] {
            XCTAssertEqual(PlaneNet(fileValue: net.fileValue), net)
        }
        XCTAssertNil(PlaneNet(fileValue: "power:0"))
        XCTAssertNil(PlaneNet(fileValue: "pin:nope/1"))
    }

    func testBadNetsAreRefused() {
        let cases = [
            (#"{ "layers": 4, "layerStyles": [ { "layer": "inner2", "net": "vcc" } ] }"#,
             #"layer inner2 net "vcc" must be ground, power:<mV> or pin:<part id>/<pin>"#),
            (#"{ "layers": 4, "layerStyles": [ { "layer": "top", "net": "ground" } ] }"#,
             "layer top is not a plane; it has no net"),
        ]
        for (board, message) in cases {
            XCTAssertEqual(BoardLayersTests.pcbError { _ = try BoardLayersTests.open(board: board) }, message)
        }
    }
}
