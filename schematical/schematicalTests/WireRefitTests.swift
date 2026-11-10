import CoreGraphics
import XCTest
@testable import schematical

/// Wires on a moved part keep the way they leave its pins
/// (`OrthogonalRoute.refit`), and removing a branch joins the two wires it
/// split back into one.
@MainActor
final class WireRefitTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    func testStraightVerticalWireStaysVerticalAtBothEnds() {
        // Diode-OR: one cathode straight down to another; the lower diode moves right.
        let route = OrthogonalRoute.refit([p(430, 200), p(430, 230), p(430, 260)], start: p(430, 200), end: p(450, 260))
        XCTAssertEqual(route, [p(430, 200), p(430, 230), p(450, 230), p(450, 260)])
    }

    func testTwoPointWireBecomesAZAlongItsAxis() {
        let route = OrthogonalRoute.refit([p(0, 0), p(0, 100)], start: p(0, 0), end: p(40, 100))
        XCTAssertEqual(route, [p(0, 0), p(0, 50), p(40, 50), p(40, 100)])
    }

    func testLKeepsItsCorner() {
        // Down from the pin, then across; the pin moves right.
        let route = OrthogonalRoute.refit([p(0, 0), p(0, 60), p(100, 60)], start: p(20, 0), end: p(100, 60))
        XCTAssertEqual(route, [p(20, 0), p(20, 60), p(100, 60)])
    }

    func testHorizontalWireStillBendsAtTheMiddle() {
        let route = OrthogonalRoute.refit([p(0, 0), p(50, 0), p(100, 0)], start: p(0, 0), end: p(100, 40))
        XCTAssertEqual(route, [p(0, 0), p(50, 0), p(50, 40), p(100, 40)])
    }

    /// A wire drawn straight down off a diode's cathode still leaves it
    /// downward after the diode moves.
    func testWireDownFromADiodeFollowsItDown() throws {
        let doc = TestSupport.newDocument()
        let diode = try TestSupport.place(doc, "1N4148", at: p(400, 200))
        let chip = try TestSupport.place(doc, "SN74HC00N", at: p(300, 500))
        let k = try TestSupport.pin(diode, "K"), a = try TestSupport.pin(chip, "1A")
        let kTip = try XCTUnwrap(doc.pinTip(k)), aTip = try XCTUnwrap(doc.pinTip(a))
        XCTAssertNil(doc.connect(.pin(k), .pin(a), path: [kTip, p(kTip.x, aTip.y), aTip]))
        let id = try XCTUnwrap(doc.wires.last?.id)

        doc.select(.chip(diode.id))
        let base = doc.schematicGroupBase()
        doc.moveSchematicGroup(base, by: CGSize(width: 40, height: -20))
        doc.settleSchematicGroup(base)

        let moved = try XCTUnwrap(doc.pinTip(k))
        let path = doc.path(forWire: id)
        XCTAssertEqual(path.first, moved)
        XCTAssertEqual(path[1].x, moved.x, "the wire still leaves the cathode downward: \(path)")
        XCTAssertGreaterThan(path[1].y, moved.y)
        XCTAssertEqual(path.last, aTip)
        XCTAssertTrue(OrthogonalRoute.isOrthogonal(path))
    }

    /// Branch off a wire, then erase the branch: one wire again, no junction.
    func testErasingABranchJoinsTheWireBackUp() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let right = try TestSupport.place(doc, "SN74HC00N", at: p(600, 200))
        let below = try TestSupport.place(doc, "SN74HC00N", at: p(400, 500))
        let y = try TestSupport.pin(left, "1Y"), a = try TestSupport.pin(right, "1A")
        XCTAssertNil(doc.connect(.pin(y), .pin(a)))
        let original = OrthogonalRoute.simplified(doc.path(forWire: try XCTUnwrap(doc.wires.first?.id)))
        // Two grid steps along the wire's first run, off the pin.
        let branchAt = p(original[0].x + 20, original[0].y)
        XCTAssertNil(doc.connect(.point(branchAt), .pin(try TestSupport.pin(below, "1A"))))
        XCTAssertEqual(doc.wires.count, 3)
        XCTAssertEqual(doc.junctions.count, 1)

        let branch = try XCTUnwrap(doc.wires.first { $0.end == .pin(try TestSupport.pin(below, "1A")) })
        let branchPath = doc.path(forWire: branch.id)
        doc.erase(at: CGPoint(x: (branchPath[0].x + branchPath[1].x) / 2, y: (branchPath[0].y + branchPath[1].y) / 2))

        XCTAssertEqual(doc.wires.count, 1)
        XCTAssertTrue(doc.junctions.isEmpty)
        let wire = try XCTUnwrap(doc.wires.first)
        XCTAssertEqual(Set([wire.start, wire.end]), Set([WireEnd.pin(y), WireEnd.pin(a)]))
        var bends = wire.interior
        if wire.start != .pin(y) { bends.reverse() }
        XCTAssertEqual(bends, Array(original.dropFirst().dropLast()), "no bend left where the branch was")
    }

    /// Branch off a straight wire, then erase the branch: one straight wire
    /// with no stored bends, like a freshly drawn one.
    func testErasingABranchOffAStraightWireLeavesOneSegment() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let right = try TestSupport.place(doc, "SN74HC00N", at: p(600, 200))
        let below = try TestSupport.place(doc, "SN74HC00N", at: p(400, 500))
        let y = try TestSupport.pin(left, "1Y")
        let yTip = try XCTUnwrap(doc.pinTip(y))
        // Right's input on 1Y's row: right moves so its 1A tip lines up.
        let a = try TestSupport.pin(right, "1A")
        let aTip = try XCTUnwrap(doc.pinTip(a))
        doc.select(.chip(right.id))
        doc.moveSchematicGroup(doc.schematicGroupBase(), by: CGSize(width: 0, height: yTip.y - aTip.y))
        XCTAssertNil(doc.connect(.pin(y), .pin(a)))
        XCTAssertEqual(doc.wires.first?.interior, [])
        let lined = try XCTUnwrap(doc.pinTip(a))
        XCTAssertEqual(lined.y, yTip.y)

        // Branch between the pin and the shown midpoint, so the far half
        // would hold that midpoint if it were kept.
        XCTAssertNil(doc.connect(.point(p(yTip.x + 20, yTip.y)), .pin(try TestSupport.pin(below, "1A"))))
        XCTAssertTrue(doc.wires.filter { $0.end != .pin(try! TestSupport.pin(below, "1A")) }.allSatisfy { $0.interior.isEmpty },
                      "each straight half is one segment")
        let branch = try XCTUnwrap(doc.wires.first { $0.end == .pin(try TestSupport.pin(below, "1A")) })
        let branchPath = doc.path(forWire: branch.id)
        doc.erase(at: CGPoint(x: (branchPath[0].x + branchPath[1].x) / 2, y: (branchPath[0].y + branchPath[1].y) / 2))

        XCTAssertEqual(doc.wires.count, 1)
        XCTAssertTrue(doc.junctions.isEmpty)
        XCTAssertEqual(doc.wires.first?.interior, [])
    }

    /// Draw a wire, delete the part, extend the line from the loose end to
    /// another pin: one wire, pin to pin.
    func testExtendingFromALooseEndIsOneWire() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let gone = try TestSupport.place(doc, "SN74HC00N", at: p(500, 200))
        let other = try TestSupport.place(doc, "SN74HC00N", at: p(800, 300))
        let y = try TestSupport.pin(left, "1Y")
        XCTAssertNil(doc.connect(.pin(y), .pin(try TestSupport.pin(gone, "1A"))))
        doc.erase(at: CGPoint(x: gone.bodyRect.midX, y: gone.bodyRect.midY))
        let loose = try XCTUnwrap(doc.junctions.first { $0.isLoose })

        let a = try TestSupport.pin(other, "1A")
        XCTAssertNil(doc.connect(.point(loose.point), .pin(a)))

        XCTAssertEqual(doc.wires.count, 1)
        XCTAssertTrue(doc.junctions.isEmpty)
        let wire = try XCTUnwrap(doc.wires.first)
        XCTAssertEqual(Set([wire.start, wire.end]), Set([WireEnd.pin(y), WireEnd.pin(a)]))
        XCTAssertTrue(OrthogonalRoute.isOrthogonal(doc.path(forWire: wire.id)))
        let route = [try XCTUnwrap(doc.endTip(wire.start))] + wire.interior + [try XCTUnwrap(doc.endTip(wire.end))]
        XCTAssertEqual(OrthogonalRoute.simplified(route), route, "no bend on a straight run")
    }

    /// The same, drawn from the new pin to the loose end.
    func testExtendingToALooseEndIsOneWire() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let gone = try TestSupport.place(doc, "SN74HC00N", at: p(500, 200))
        let other = try TestSupport.place(doc, "SN74HC00N", at: p(800, 300))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(left, "1Y")), .pin(try TestSupport.pin(gone, "1A"))))
        doc.erase(at: CGPoint(x: gone.bodyRect.midX, y: gone.bodyRect.midY))
        let loose = try XCTUnwrap(doc.junctions.first { $0.isLoose })
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(other, "1A")), .point(loose.point)))
        XCTAssertEqual(doc.wires.count, 1)
        XCTAssertTrue(doc.junctions.isEmpty)
    }

    /// A loose end (a deleted pin's spot) with two wires on it stays.
    func testLooseJunctionIsNotJoined() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let mid = try TestSupport.place(doc, "1N4148", at: p(400, 200))
        let right = try TestSupport.place(doc, "SN74HC00N", at: p(600, 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(left, "1Y")), .pin(try TestSupport.pin(mid, "A"))))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(mid, "A")), .pin(try TestSupport.pin(right, "1A"))))
        doc.erase(at: CGPoint(x: mid.bodyRect.midX, y: mid.bodyRect.minY + ChipLayout.diodeCentreY))
        XCTAssertEqual(doc.wires.count, 2)
        XCTAssertEqual(doc.junctions.filter(\.isLoose).count, 1)
    }
}
