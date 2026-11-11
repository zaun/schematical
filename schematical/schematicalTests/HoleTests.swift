import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Drilled holes on the PCB: screw sizes and Custom, plated or bare, a
/// plated hole's net and the four points traces join it at.
@MainActor
final class HoleTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func hole(_ doc: SchematicDocument, _ id: UUID) throws -> BoardHole {
        try XCTUnwrap(doc.holes.first { $0.id == id })
    }

    func testPlacingTakesTheLastHolesSettings() throws {
        let doc = TestSupport.newDocument()
        let first = doc.addHole(at: p(10.01, 10))
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Hole")
        XCTAssertEqual(doc.selectedHoleID, first)
        var one = try hole(doc, first)
        XCTAssertEqual(one.size, .m3)
        XCTAssertEqual(one.drill, 3.2)
        XCTAssertFalse(one.plated)
        XCTAssertEqual(one.centre, BoardGrid.snap(p(10.01, 10), spacing: doc.board.traceGrid))

        doc.setHoleSize(first, .no4)
        doc.setHolePlated(first, true)
        doc.setHoleNet(first, .ground)
        one = try hole(doc, first)
        XCTAssertEqual(one.drill, 3.0)
        XCTAssertEqual(one.diameter, 6.0)
        let second = try hole(doc, doc.addHole(at: p(50, 10)))
        XCTAssertEqual(second.size, .no4)
        XCTAssertTrue(second.plated)
        XCTAssertEqual(second.net, .ground)
    }

    func testSizes() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addHole(at: p(10, 10))
        doc.setHoleSize(id, .m2_5)
        XCTAssertEqual(try hole(doc, id).drill, 2.7)
        XCTAssertEqual(try hole(doc, id).diameter, 5.0)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Hole Size")

        // Editing a size makes it Custom; the land keeps a ring round it.
        doc.setHoleDrill(id, 6)
        var found = try hole(doc, id)
        XCTAssertEqual(found.size, .custom)
        XCTAssertEqual(found.drill, 6)
        XCTAssertEqual(found.diameter, 6.2, accuracy: 1e-9)
        doc.setHoleDiameter(id, 1)
        XCTAssertEqual(try hole(doc, id).diameter, 6.2, accuracy: 1e-9, "at least the hole and its ring")
        doc.setHoleDrill(id, 100)
        XCTAssertEqual(try hole(doc, id).drill, BoardHole.drillRange.upperBound)

        // Choosing Custom keeps the sizes; a screw size sets them again.
        doc.setHoleSize(id, .custom)
        XCTAssertEqual(try hole(doc, id).drill, 10)
        doc.setHoleSize(id, .m6)
        found = try hole(doc, id)
        XCTAssertEqual(found.drill, 6.4)
        XCTAssertEqual(found.diameter, 12)
        XCTAssertTrue(HoleSize.allCases.compactMap(\.sizes).allSatisfy {
            BoardHole.problem(drill: $0.drill, diameter: $0.diameter) == nil
        })
    }

    /// A plated hole's land is joined at four points round the hole; a
    /// bare hole has none.
    func testTracesSnapToTheFourContacts() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addHole(at: p(20, 20))
        let tolerance = TraceDrawing.tolerance / BoardScale.pointsPerMM
        let bare = try hole(doc, id)
        XCTAssertTrue(bare.contacts.isEmpty)
        XCTAssertEqual(doc.traceAnchor(at: p(21.6, 20.1), layer: .top, tolerance: tolerance),
                       BoardGrid.snap(p(21.6, 20.1), spacing: doc.board.traceGrid))

        doc.setHolePlated(id, true)
        let plated = try hole(doc, id)
        let r = (3.2 + 6.0) / 4, c = plated.centre
        XCTAssertEqual(plated.contacts, [p(c.x + r, c.y), p(c.x, c.y + r), p(c.x - r, c.y), p(c.x, c.y - r)])
        for contact in plated.contacts {
            let near = p(contact.x + (contact.x - c.x) * 0.1, contact.y + (contact.y - c.y) * 0.1 + 0.05)
            XCTAssertEqual(doc.traceAnchor(at: near, layer: .bottom, tolerance: tolerance), contact)
        }

        let trace = try XCTUnwrap(doc.addTrace(from: p(40, 20), to: plated.contacts[0], layer: .top,
                                               diagonalFirst: false))
        XCTAssertEqual(doc.traces(touching: plated).map(\.id), [trace])
        let copper = doc.connectivity()
        let pad = try XCTUnwrap(copper.pads.firstIndex { $0.hole == id })
        XCTAssertEqual(copper.root[pad], copper.root[copper.traceNode(0)])
    }

    /// A plated hole on GND is a pad of the GND net: it gets a ratsnest
    /// line to the GND pads until copper joins it, and a GND plane joins it.
    /// A bare hole is always cut out of a plane.
    func testPlatedHoleOnANet() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let ground = try TestSupport.place(doc, "GND", at: p(100, 400))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
        XCTAssertTrue(doc.ratsnest().isEmpty)

        let id = doc.addHole(at: p(5, 5))
        XCTAssertTrue(doc.ratsnest().isEmpty, "a bare hole is on no net")
        doc.setHolePlated(id, true)
        XCTAssertTrue(doc.ratsnest().isEmpty, "a plated hole on no net")
        doc.setHoleNet(id, .ground)
        XCTAssertEqual(doc.ratsnest().count, 1, "the hole joins GND")
        XCTAssertEqual(doc.planeNetChoices().first?.net, .ground)

        doc.setLayerNet(.inner(2), .ground)
        let copper = doc.connectivity()
        let pad = try XCTUnwrap(copper.pads.firstIndex { $0.hole == id })
        XCTAssertEqual(copper.pads[pad].label, "H1")
        XCTAssertFalse(copper.planeClearances(on: .inner(2)).pads.contains(pad), "on the plane's net")
        if led.part.pins.isEmpty == false, doc.chips.first?.boardPads.allSatisfy({ $0.drill > 0 }) == true {
            XCTAssertTrue(doc.ratsnest().isEmpty, "the plane joins the hole and the cathode")
        }

        doc.setHolePlated(id, false)
        let bare = doc.connectivity()
        XCTAssertTrue(bare.planeClearances(on: .inner(2)).pads.contains(pad))
        XCTAssertNil(bare.netOfPad[pad], "a bare hole keeps its net but isn't on it")
        XCTAssertEqual(try hole(doc, id).net, .ground)
    }

    func testSelectMoveLockAndDelete() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addHole(at: p(20, 20))
        XCTAssertEqual(doc.boardItem(at: p(21, 20), tolerance: 0.1), .hole(id))
        XCTAssertEqual(doc.boardItems(in: CGRect(x: 18, y: 18, width: 1, height: 1)), [.hole(id)])

        doc.select(.hole(id))
        doc.moveBoardGroup(doc.boardGroupBase(), by: CGSize(width: 5, height: 0))
        let moved = try hole(doc, id).centre.x
        XCTAssertEqual(moved, BoardGrid.snap(25, spacing: doc.board.traceGrid), accuracy: 1e-9)
        doc.setLocked(.hole(id), true)
        XCTAssertTrue(doc.boardGroupBase().isEmpty, "a locked hole stays put")
        doc.setHoleCentre(id, p(0, 0))
        XCTAssertEqual(try hole(doc, id).centre.x, moved)

        XCTAssertTrue(doc.deleteBoardSelection())
        XCTAssertTrue(doc.holes.isEmpty)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.holes.map(\.id), [id])
        XCTAssertTrue(doc.deleteBoardItem(at: p(25, 20), tolerance: 0.1))
        XCTAssertTrue(doc.holes.isEmpty)
    }

    func testFile() throws {
        let doc = TestSupport.newDocument()
        let plain = doc.addHole(at: p(5, 5))
        let custom = doc.addHole(at: p(30, 5))
        doc.setHoleDrill(custom, 1.5)
        doc.setHoleDiameter(custom, 3)
        doc.setHolePlated(custom, true)
        doc.setHoleNet(custom, .rail(mv: 5000))
        doc.setLocked(.hole(custom), true)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.holes, doc.holes)
        XCTAssertEqual(opened.holes.first?.id, plain)

        // Missing sizes are the size's.
        let id = UUID()
        let entry = ##"{ "layers": 2, "holes": [ { "id": "\##(id.uuidString)", "x": 1, "y": 2, "size": "#6" } ] }"##
        let read = try XCTUnwrap(try BoardLayersTests.open(board: entry).holes.first)
        XCTAssertEqual(read.drill, 3.7)
        XCTAssertEqual(read.diameter, 7)
        XCTAssertFalse(read.plated)

        XCTAssertEqual(BoardLayersTests.pcbError {
            _ = try BoardLayersTests.open(board: #"{ "holes": [ { "id": "\#(id.uuidString)", "x": 1, "y": 2, "size": "m7" } ] }"#)
        }, ##"hole \##(id.uuidString) size "m7" must be custom, m2, m2.5, m3, m4, m5, m6, #2, #4, #6, #8 or #10"##)
        XCTAssertEqual(BoardLayersTests.pcbError {
            _ = try BoardLayersTests.open(board: #"{ "holes": [ { "id": "\#(id.uuidString)", "x": 1, "y": 2, "drill": 3, "diameter": 3.1 } ] }"#)
        }, "hole \(id.uuidString) diameter 3.1 must be at least 0.2 mm more than its drill 3")
    }
}
