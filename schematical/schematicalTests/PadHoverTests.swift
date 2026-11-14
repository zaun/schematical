import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The PCB page's hover label for a pad (`SchematicDocument.padLabel`).
@MainActor
final class PadHoverTests: XCTestCase {
    private func padCentre(_ chip: PlacedChip, _ designator: String) throws -> CGPoint {
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    func testHoveringAPadNamesItsPartPinAndName() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let chip = try XCTUnwrap(doc.chips.first)
        let k = try XCTUnwrap(chip.part.pins.first { $0.name == "K" })
        let hit = try XCTUnwrap(doc.padLabel(at: try padCentre(chip, k.designator)))
        XCTAssertEqual(hit.pad.designator, k.designator)
        XCTAssertEqual(hit.label, "\(chip.reference).\(k.designator) K")
    }

    func testAThroughHoleChipPadIsNamed() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        let chip = try XCTUnwrap(doc.chips.first)
        let pin = try XCTUnwrap(chip.part.pins.first { $0.designator == "7" })
        let hit = try XCTUnwrap(doc.padLabel(at: try padCentre(chip, "7")))
        XCTAssertEqual(hit.label, "\(chip.reference).7 \(pin.name)")
    }

    func testOffPadAndHiddenPadsHaveNoLabel() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let chip = try XCTUnwrap(doc.chips.first)
        let bounds = try XCTUnwrap(chip.boardBounds)
        XCTAssertNil(doc.padLabel(at: CGPoint(x: bounds.maxX + 10, y: bounds.maxY + 10)))

        let centre = try padCentre(chip, try XCTUnwrap(chip.boardPads.first?.designator))
        let noCopper = BoardVisibility(hidden: Set(doc.board.copperLayers.map { .copper($0) }),
                                       copperLayers: doc.board.copperLayers)
        XCTAssertNil(doc.padLabel(at: centre, visible: noCopper))
    }

    func testACardFingerShowsItsEdgeBusWireName() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.coco)
        let board = doc.board
        let id = try XCTUnwrap(doc.addBus(from: CGPoint(x: 100, y: 100), to: CGPoint(x: 300, y: 100)))
        XCTAssertNil(doc.setBusWireCount(id, board.pinCount))
        let finger = try XCTUnwrap(board.fingers.first { $0.name == board.pinNames[3] })
        let centre = CGPoint(x: finger.rect.midX, y: finger.rect.midY)
        let pin = "\(board.connectorName).\(finger.name)"

        // No edge bus yet: just the pin.
        XCTAssertEqual(doc.padLabel(at: centre)?.label, pin)

        let wire = try XCTUnwrap(doc.bus(id)?.wires[3])
        XCTAssertNil(doc.renameBusWire(bus: id, wire: wire.id, to: "CLK"))
        doc.setEdgeBus(id)
        XCTAssertEqual(doc.edgeBusID, id)
        XCTAssertEqual(doc.padLabel(at: centre)?.label, "\(pin) CLK")
    }
}
