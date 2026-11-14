import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Net classes (plan-38-A): their edits and clamps, the file, net
/// resolution and names, and the `auto` flag on traces and vias.
@MainActor
final class NetClassTests: XCTestCase {
    private func netClass(_ doc: SchematicDocument, _ id: UUID) throws -> NetClass {
        try XCTUnwrap(doc.board.netClasses.first { $0.id == id })
    }

    // MARK: Phase 1 — edits

    func testAddRenameRemoveAreEachOneUndoStep() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNetClass(named: "  Power  ")
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Net Class")
        let added = try netClass(doc, id)
        XCTAssertEqual(added.name, "Power")
        XCTAssertEqual(added.width, BoardTrace.defaultWidth)
        XCTAssertNil(added.clearance)
        XCTAssertEqual(added.viaDrill, 0.4)
        XCTAssertEqual(added.viaDiameter, 0.8)
        XCTAssertTrue(added.members.isEmpty)

        doc.renameNetClass(id, to: "Supply")
        XCTAssertEqual(try netClass(doc, id).name, "Supply")
        XCTAssertEqual(doc.undoName(on: .pcb), "Rename Net Class")
        doc.renameNetClass(id, to: "   ")
        XCTAssertEqual(try netClass(doc, id).name, "Supply", "a blank name is ignored")
        doc.renameNetClass(id, to: String(repeating: "x", count: 50))
        XCTAssertEqual(try netClass(doc, id).name.count, 40)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, id).name, "Power")

        doc.removeNetClass(id)
        XCTAssertTrue(doc.board.netClasses.isEmpty)
        XCTAssertEqual(doc.undoName(on: .pcb), "Delete Net Class")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.board.netClasses.map(\.id), [id])
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.board.netClasses.isEmpty)
    }

    func testWidthClearanceAndViaClamp() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNetClass(named: "A")
        doc.setNetClassWidth(id, 4)
        XCTAssertEqual(try netClass(doc, id).width, 5)
        doc.setNetClassWidth(id, 101)
        XCTAssertEqual(try netClass(doc, id).width, 100)
        doc.setNetClassWidth(id, 20)
        XCTAssertEqual(try netClass(doc, id).width, 20)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Net Class Width")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, id).width, 100)

        doc.setNetClassClearance(id, 0.01)
        XCTAssertEqual(try netClass(doc, id).clearance, 0.05)
        doc.setNetClassClearance(id, 20)
        XCTAssertEqual(try netClass(doc, id).clearance, 10)
        doc.setNetClassClearance(id, nil)
        XCTAssertNil(try netClass(doc, id).clearance)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, id).clearance, 10)

        doc.setNetClassVia(id, drill: 0.6, diameter: 0.6)
        var via = try netClass(doc, id)
        XCTAssertEqual(via.viaDrill, 0.6)
        XCTAssertEqual(via.viaDiameter, 0.7, accuracy: 1e-9, "the metal keeps the thinnest ring")
        doc.setNetClassVia(id, drill: 0.01, diameter: 50)
        via = try netClass(doc, id)
        XCTAssertEqual(via.viaDrill, 0.1)
        XCTAssertEqual(via.viaDiameter, 10)
        XCTAssertNil(via.problem)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Net Class Via")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, id).viaDrill, 0.6)
    }

    func testAssignNetMovesBetweenClassesAndBackToDefault() throws {
        let doc = TestSupport.newDocument()
        let a = doc.addNetClass(named: "A")
        let b = doc.addNetClass(named: "B")
        let pin = PlaneNet.pin(chip: UUID(), designator: "3")
        doc.assignNet(.ground, to: a)
        doc.assignNet(pin, to: a)
        XCTAssertEqual(try netClass(doc, a).members, [.ground, pin])
        doc.assignNet(.ground, to: b)
        XCTAssertEqual(try netClass(doc, a).members, [pin])
        XCTAssertEqual(try netClass(doc, b).members, [.ground])
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Net Class")
        doc.assignNet(.ground, to: nil)
        XCTAssertTrue(try netClass(doc, b).members.isEmpty)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, b).members, [.ground])
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, a).members, [.ground, pin])
        XCTAssertTrue(try netClass(doc, b).members.isEmpty)
    }

    func testMoveNetClassChangesPriority() throws {
        let doc = TestSupport.newDocument()
        let a = doc.addNetClass(named: "A")
        let b = doc.addNetClass(named: "B")
        let c = doc.addNetClass(named: "C")
        doc.moveNetClass(c, to: 0)
        XCTAssertEqual(doc.board.netClasses.map(\.id), [c, a, b])
        XCTAssertEqual(doc.undoName(on: .pcb), "Move Net Class")
        doc.moveNetClass(c, to: 99)
        XCTAssertEqual(doc.board.netClasses.map(\.id), [a, b, c])
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.board.netClasses.map(\.id), [c, a, b])
    }

    func testProblem() {
        func problem(width: Int = 10, clearance: Double? = nil, drill: Double = 0.4, diameter: Double = 0.8,
                     name: String = "A") -> String? {
            NetClass.problem(width: width, clearance: clearance, viaDrill: drill, viaDiameter: diameter, name: name)
        }
        XCTAssertNil(problem())
        XCTAssertNil(problem(width: 5, clearance: 0.05))
        XCTAssertNotNil(problem(width: 4))
        XCTAssertNotNil(problem(width: 101))
        XCTAssertNotNil(problem(clearance: 0.04))
        XCTAssertNotNil(problem(drill: 0.4, diameter: 0.49))
        XCTAssertNotNil(problem(name: "   "))
        XCTAssertNotNil(problem(name: ""))
        XCTAssertNotNil(problem(name: String(repeating: "x", count: 41)))
    }
}
