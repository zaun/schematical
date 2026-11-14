import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-41-A: a footprint's reference text has a position stored relative to
/// the part (`BoardPlacement.referenceOffset`).
@MainActor
final class ReferencePositionTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func chip(_ doc: SchematicDocument, _ id: UUID) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.id == id })
    }

    private func assertEqual(_ a: CGPoint, _ b: CGPoint, accuracy: Double = 1e-9,
                             file: StaticString = #filePath, line: UInt = #line) {
        XCTAssertEqual(a.x, b.x, accuracy: accuracy, file: file, line: line)
        XCTAssertEqual(a.y, b.y, accuracy: accuracy, file: file, line: line)
    }

    /// A DIP-14: a long, narrow body, so a wrong turn or mirror shows.
    private func withPart() throws -> (doc: SchematicDocument, id: UUID) {
        let doc = TestSupport.newDocument()
        return (doc, try TestSupport.place(doc, "SN74HC00N", at: p(200, 200)).id)
    }

    // MARK: The inverse

    func testLocalInvertsBoardForEveryRotationAndSide() throws {
        for side in [BoardSide.top, .bottom] {
            for rotation in BoardPlacement.rotations {
                var placement = BoardPlacement(centre: p(37.5, -12.25), rotation: rotation)
                placement.side = side
                for local in [p(0, 0), p(3, -7), p(-5.5, 2), p(1.27, 8.9)] {
                    assertEqual(placement.local(placement.board(local)), local)
                    assertEqual(placement.board(placement.local(local)), local)
                }
            }
        }
    }

    // MARK: Where the reference is

    func testNoOffsetIsTheBodyCentre() throws {
        let c = try withPart()
        let chip = try chip(c.doc, c.id)
        let body = try XCTUnwrap(chip.boardBody)
        assertEqual(try XCTUnwrap(chip.boardReferenceCentre), p(body.rect.midX, body.rect.midY))
        let rect = try XCTUnwrap(chip.boardReferenceRect())
        assertEqual(p(rect.midX, rect.midY), p(body.rect.midX, body.rect.midY))
        XCTAssertEqual(rect.height, 1.0, accuracy: 0.5)
    }

    func testMoveReferenceSnapsToTheTraceGridAndStoresARelativeOffset() throws {
        let c = try withPart()
        let before = try XCTUnwrap(try chip(c.doc, c.id).boardReferenceCentre)
        c.doc.moveReference(c.id, toCentre: p(before.x + 3.1, before.y - 4.9))
        let after = try chip(c.doc, c.id)
        let target = try XCTUnwrap(after.boardReferenceCentre)
        let grid = c.doc.board.traceGrid
        XCTAssertEqual((target.x / grid).rounded() * grid, target.x, accuracy: 1e-3)
        XCTAssertEqual((target.y / grid).rounded() * grid, target.y, accuracy: 1e-3)
        XCTAssertEqual(target.x, before.x + 3.1, accuracy: grid / 2 + 1e-3)
        XCTAssertEqual(target.y, before.y - 4.9, accuracy: grid / 2 + 1e-3)
        XCTAssertNotEqual(after.board?.referenceOffset, .zero)
    }

    func testReferenceFollowsMovesTurnsAndSide() throws {
        let c = try withPart()
        c.doc.moveReference(c.id, toCentre: p(210, 195))
        var chip = try chip(c.doc, c.id)
        let offset = try XCTUnwrap(chip.board?.referenceOffset)
        XCTAssertNotEqual(offset, .zero)

        // Moved: the reference keeps its place on the part.
        let beforeMove = try XCTUnwrap(chip.boardReferenceCentre)
        let centre = try XCTUnwrap(chip.board?.centre)
        chip.board?.centre = p(centre.x + 10, centre.y + 5)
        let afterMove = try XCTUnwrap(chip.boardReferenceCentre)
        assertEqual(afterMove, p(beforeMove.x + 10, beforeMove.y + 5))

        // Turned and mirrored: always the footprint-frame offset, placed.
        for side in [BoardSide.top, .bottom] {
            for rotation in BoardPlacement.rotations {
                chip.board?.rotation = rotation
                chip.board?.side = side
                let placement = try XCTUnwrap(chip.board)
                assertEqual(try XCTUnwrap(chip.boardReferenceCentre), placement.board(offset))
            }
        }
        // On Bottom at 0° the offset's x mirrors about the body centre.
        chip.board?.rotation = 0
        chip.board?.side = .bottom
        let placement = try XCTUnwrap(chip.board)
        assertEqual(try XCTUnwrap(chip.boardReferenceCentre),
                    p(placement.centre.x - offset.x, placement.centre.y + offset.y))
    }

    func testRotateFootprintCarriesTheReference() throws {
        let c = try withPart()
        c.doc.moveReference(c.id, toCentre: p(210, 195))
        c.doc.rotateFootprint(c.id)
        let chip = try chip(c.doc, c.id)
        let placement = try XCTUnwrap(chip.board)
        XCTAssertEqual(placement.rotation, 90)
        assertEqual(try XCTUnwrap(chip.boardReferenceCentre), placement.board(placement.referenceOffset))
    }

    func testResetReferenceRestoresZero() throws {
        let c = try withPart()
        c.doc.moveReference(c.id, toCentre: p(210, 195))
        c.doc.resetReference(c.id)
        XCTAssertEqual(try chip(c.doc, c.id).board?.referenceOffset, .zero)
    }

    func testLockedFootprintsReferenceStillMoves() throws {
        let c = try withPart()
        c.doc.setLocked(.footprint(c.id), true)
        c.doc.moveReference(c.id, toCentre: p(210, 195))
        XCTAssertNotEqual(try chip(c.doc, c.id).board?.referenceOffset, .zero)
        XCTAssertEqual(try chip(c.doc, c.id).board?.locked, true)
    }

    // MARK: Undo

    func testMoveAndResetAreOnePcbStepEach() throws {
        let c = try withPart()
        c.doc.moveReference(c.id, toCentre: p(210, 195))
        let moved = try chip(c.doc, c.id).board?.referenceOffset
        XCTAssertEqual(c.doc.undoName(on: .pcb), "Move Reference")
        c.doc.resetReference(c.id)
        XCTAssertEqual(c.doc.undoName(on: .pcb), "Reset Reference")
        XCTAssertTrue(c.doc.undo(on: .pcb))
        XCTAssertEqual(try chip(c.doc, c.id).board?.referenceOffset, moved)
        XCTAssertTrue(c.doc.undo(on: .pcb))
        XCTAssertEqual(try chip(c.doc, c.id).board?.referenceOffset, .zero)
        XCTAssertTrue(c.doc.redo(on: .pcb))
        XCTAssertEqual(try chip(c.doc, c.id).board?.referenceOffset, moved)
    }

    // MARK: A swapped footprint clears it

    func testFootprintSwapsResetTheReference() throws {
        let doc = TestSupport.newDocument()
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: p(100, 100))
        doc.moveReference(pull.id, toCentre: p(103, 98))
        XCTAssertNotEqual(try chip(doc, pull.id).board?.referenceOffset, .zero)
        doc.setPartFootprint(pull.id, "RES-0805")
        XCTAssertEqual(try chip(doc, pull.id).board?.referenceOffset, .zero, "footprint choice")

        let lcdPart = try XCTUnwrap(TestSupport.allParts.first { $0.connectorFootprint != nil })
        let lcd = try TestSupport.place(doc, lcdPart.name, at: p(300, 100))
        doc.moveReference(lcd.id, toCentre: p(305, 98))
        XCTAssertNotEqual(try chip(doc, lcd.id).board?.referenceOffset, .zero)
        doc.setFootprintStyle(lcd.id, .connector)
        XCTAssertEqual(try chip(doc, lcd.id).board?.referenceOffset, .zero, "style")

        let header = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: p(500, 100))
        doc.moveReference(header.id, toCentre: p(505, 96))
        XCTAssertNotEqual(try chip(doc, header.id).board?.referenceOffset, .zero)
        doc.setHeaderSize(header.id, to: try TestSupport.part("GENERIC-HEADER-1X04"))
        XCTAssertEqual(try chip(doc, header.id).board?.referenceOffset, .zero, "header size")
    }
}
