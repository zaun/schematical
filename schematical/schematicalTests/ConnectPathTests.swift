import CoreGraphics
import XCTest
@testable import schematical

/// plan-25-A phase 3: Line wires placed along a drawn path, and the default
/// route leaving and entering pins along the side they face.
@MainActor
final class ConnectPathTests: XCTestCase {
    private func tip(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        try XCTUnwrap(doc.pinTip(ref))
    }

    /// The only pin of a supply part.
    private func onlyPin(_ chip: PlacedChip) throws -> PinRef {
        let slot = try XCTUnwrap(chip.layout.slots.first)
        return PinRef(chipID: chip.id, index: slot.pin.index)
    }

    private func wire(_ doc: SchematicDocument, from ref: PinRef) throws -> Wire {
        try XCTUnwrap(doc.wires.first { $0.start == .pin(ref) })
    }

    func testPinToPinThroughAUKeepsItsBends() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 300))
        let aK = try TestSupport.pin(a, "K"), bK = try TestSupport.pin(b, "K")
        let from = try tip(doc, aK), to = try tip(doc, bK)
        XCTAssertEqual(from.x, to.x, "both cathodes face right at the same x")
        let bends = [CGPoint(x: from.x + 50, y: from.y), CGPoint(x: from.x + 50, y: to.y)]
        XCTAssertNil(doc.connect(.pin(aK), .pin(bK), path: [from] + bends + [to]))
        let placed = try wire(doc, from: aK)
        XCTAssertEqual(placed.interior, bends)
        XCTAssertEqual(doc.path(forWire: placed.id), [from] + bends + [to])
    }

    func testPinToAPointOnAWireIsAnLLeavingSideways() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let right = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 100))
        let below = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 250, y: 300))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(left, "K")), .pin(try TestSupport.pin(right, "A"))))
        let first = doc.path(forWire: try XCTUnwrap(doc.wires.first).id)
        let middle = CGPoint(x: SchematicGrid.snap((first.first!.x + first.last!.x) / 2), y: first.first!.y)
        let belowA = try TestSupport.pin(below, "A")
        let start = try tip(doc, belowA)
        XCTAssertNotEqual(start.x, middle.x)
        XCTAssertNotEqual(start.y, middle.y)
        XCTAssertNil(doc.connect(.pin(belowA), .point(middle)))
        let placed = try wire(doc, from: belowA)
        // Out sideways along the pin, then one turn up to the wire: an L, not a Z.
        XCTAssertEqual(doc.path(forWire: placed.id), [start, CGPoint(x: middle.x, y: start.y), middle])
    }

    func testSupplyPinsLeaveVertically() throws {
        let doc = TestSupport.newDocument()
        let supply = try TestSupport.place(doc, "5V", at: CGPoint(x: 100, y: 100))
        let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 500, y: 400))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 250))
        let vcc = try onlyPin(supply), gnd = try onlyPin(ground)
        XCTAssertEqual(doc.anchorAxis(.pin(vcc)), .vertical)
        XCTAssertEqual(doc.anchorAxis(.pin(gnd)), .vertical)
        let ledA = try TestSupport.pin(led, "A"), ledK = try TestSupport.pin(led, "K")
        XCTAssertEqual(doc.anchorAxis(.pin(ledA)), .horizontal)

        // 5V's pin points down: the wire leaves downwards, then turns into the
        // anode sideways.
        XCTAssertNil(doc.connect(.pin(vcc), .pin(ledA)))
        let v = try tip(doc, vcc), a = try tip(doc, ledA)
        XCTAssertEqual(doc.path(forWire: try wire(doc, from: vcc).id), [v, CGPoint(x: v.x, y: a.y), a])

        // GND's pin points up: the wire from the cathode arrives from above.
        XCTAssertNil(doc.connect(.pin(ledK), .pin(gnd)))
        let k = try tip(doc, ledK), g = try tip(doc, gnd)
        XCTAssertEqual(doc.path(forWire: try wire(doc, from: ledK).id), [k, CGPoint(x: g.x, y: k.y), g])
    }

    func testPinToPinOnDifferentRowsIsStillTheMidpointZ() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        let aK = try TestSupport.pin(a, "K"), bA = try TestSupport.pin(b, "A")
        XCTAssertNil(doc.connect(.pin(aK), .pin(bA)))
        let from = try tip(doc, aK), to = try tip(doc, bA)
        let midX = SchematicGrid.snap((from.x + to.x) / 2)
        XCTAssertEqual(doc.path(forWire: try wire(doc, from: aK).id),
                       [from, CGPoint(x: midX, y: from.y), CGPoint(x: midX, y: to.y), to])
    }
}
