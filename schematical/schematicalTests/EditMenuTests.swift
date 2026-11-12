import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The Edit menu's Select All, Cut and Reset References.
@MainActor
final class EditMenuTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    func testSelectAllSchematicTakesEverything() throws {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: p(300, 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "K")), .pin(try TestSupport.pin(two, "A"))))
        let label = doc.addNetLabel(at: p(100, 300))
        let note = doc.addNote(at: p(0, 400))

        doc.selectAllSchematic()
        let selected = doc.schematicSelection.items
        XCTAssertTrue(selected.contains(.chip(one.id)))
        XCTAssertTrue(selected.contains(.chip(two.id)))
        XCTAssertTrue(selected.contains(.label(label)))
        XCTAssertTrue(selected.contains(.note(note)))
        XCTAssertTrue(doc.wires.allSatisfy { selected.contains(.wire($0.id)) })
        XCTAssertEqual(doc.wires.count, 1)
        // The parts' footprints are selected on the PCB too.
        XCTAssertTrue(doc.isSelected(.footprint(one.id)))
    }

    func testSelectAllBoardTakesWhatIsShown() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let top = try XCTUnwrap(doc.addTrace(points: [p(5, 5), p(15, 5)], layer: .top))
        let bottom = try XCTUnwrap(doc.addTrace(points: [p(5, 10), p(15, 10)], layer: .bottom))

        doc.selectAllBoard()
        XCTAssertTrue(doc.isSelected(.footprint(led.id)))
        XCTAssertTrue(doc.isSelected(.trace(top)))
        XCTAssertTrue(doc.isSelected(.trace(bottom)))

        doc.clearBoardSelection()
        doc.selectAllBoard(visible: BoardVisibility(hidden: [.copper(.bottom)]))
        XCTAssertTrue(doc.isSelected(.trace(top)))
        XCTAssertFalse(doc.isSelected(.trace(bottom)))
    }

    func testCutBoardTracesDeletesOnlyTraces() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let trace = try XCTUnwrap(doc.addTrace(points: [p(5, 5), p(15, 5)], layer: .top))
        doc.endCoalescing()
        XCTAssertFalse(doc.cutBoardTraces(), "nothing selected")

        doc.selectAllBoard()
        XCTAssertTrue(doc.cutBoardTraces())
        XCTAssertFalse(doc.traces.contains { $0.id == trace })
        XCTAssertTrue(doc.chips.contains { $0.id == led.id })
        XCTAssertEqual(doc.undoName(on: .pcb), "Cut")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.traces.contains { $0.id == trace })
    }

    func testCutSchematicIsNamedCut() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        doc.endCoalescing()
        doc.select(.chip(led.id))
        XCTAssertNotNil(doc.schematicCopy())
        XCTAssertTrue(doc.deleteSchematicSelection(named: "Cut"))
        XCTAssertTrue(doc.chips.isEmpty)
        XCTAssertEqual(doc.undoName(on: .schematic), "Cut")
    }

    func testResetReferencesRenumbersInReadingOrder() throws {
        let doc = TestSupport.newDocument()
        // Placed out of reading order, and renamed.
        let lower = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 400))
        let upperRight = try TestSupport.place(doc, "GENERIC-LED", at: p(500, 100))
        let upperLeft = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: p(700, 400))
        let cap = try TestSupport.place(doc, "GENERIC-CAPACITOR", at: p(700, 100))
        XCTAssertNil(doc.renameChip(lower.id, prefix: "D", number: 7, page: .schematic))
        XCTAssertNil(doc.renameChip(pull.id, prefix: "R", number: 9, page: .schematic))
        doc.endCoalescing()

        XCTAssertTrue(doc.resetReferences(page: .schematic))
        func reference(_ chip: PlacedChip) -> String? { doc.chips.first { $0.id == chip.id }?.reference }
        XCTAssertEqual(reference(upperLeft), "U1")
        XCTAssertEqual(reference(upperRight), "U2")
        XCTAssertEqual(reference(lower), "U3")
        XCTAssertEqual(reference(pull), "R1")
        XCTAssertEqual(reference(cap), "C1")
        XCTAssertEqual(doc.nextReference, 4)
        XCTAssertEqual(doc.undoName(on: .schematic), "Reset References")

        // Again changes nothing, and records nothing.
        XCTAssertFalse(doc.resetReferences(page: .schematic))

        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(reference(lower), "D7")
        XCTAssertEqual(reference(pull), "R9")
    }
}
