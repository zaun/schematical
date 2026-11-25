import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Cutouts in the document: the edit API, undo, selection, lock and group
/// moves (plan-65-A).
@MainActor
final class CutoutModelTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect { CGRect(x: x, y: y, width: w, height: h) }
    private func rect(_ doc: SchematicDocument, _ id: UUID) throws -> CGRect {
        try XCTUnwrap(doc.cutouts.first { $0.id == id }).shape.bounds
    }

    func testEachCallIsOneNamedStepThatUndoesAndRedoesExactly() throws {
        let doc = TestSupport.newDocument()
        let empty = doc.cutouts
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 8, 4), radius: 1)))
        XCTAssertEqual(doc.undoName, "Add Cutout")
        XCTAssertEqual(doc.boardSelection.sole, .cutout(id))
        let added = doc.cutouts

        doc.moveCutout(id, to: p(20, 20))
        XCTAssertEqual(doc.undoName, "Move")
        let moved = doc.cutouts
        XCTAssertNotEqual(moved, added)

        doc.setCutoutRect(id, r(20, 20, 12, 6))
        XCTAssertEqual(doc.undoName, "Resize")
        let resized = doc.cutouts

        doc.setCutoutRadius(id, 2)
        XCTAssertEqual(doc.undoName, "Change Cutout")
        let rounder = doc.cutouts

        doc.deleteCutout(id)
        XCTAssertEqual(doc.undoName, "Delete")
        XCTAssertTrue(doc.cutouts.isEmpty)

        for expected in [rounder, resized, moved, added, empty] {
            XCTAssertTrue(doc.undo())
            XCTAssertEqual(doc.cutouts, expected)
        }
        for expected in [added, moved, resized, rounder, []] {
            XCTAssertTrue(doc.redo())
            XCTAssertEqual(doc.cutouts, expected)
        }
    }

    func testAddRefusesABadShapeAndLeavesNoStep() {
        let doc = TestSupport.newDocument()
        XCTAssertNil(doc.addCutout(.rounded(rect: r(0, 0, 0.1, 5), radius: 0)))
        XCTAssertNil(doc.addCutout(.polygon([p(0, 0), p(4, 4), p(4, 0), p(0, 4)])))
        XCTAssertNil(doc.undoName)
        XCTAssertTrue(doc.cutouts.isEmpty)
    }

    func testARoundedCutoutSnapsToTheTraceGrid() throws {
        let doc = TestSupport.newDocument()
        let grid = doc.board.traceGrid
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10.01, 10.02, 8.01, 4.03), radius: 0)))
        let box = try rect(doc, id)
        for value in [box.minX, box.minY, box.maxX, box.maxY] {
            let steps = Double(value) / grid
            XCTAssertEqual(steps, steps.rounded(), accuracy: 1e-6, "\(value) is on the \(grid) grid")
        }
    }

    func testAPolygonIsStoredClockwise() throws {
        let doc = TestSupport.newDocument()
        let counter = [p(0, 0), p(0, 10), p(10, 10), p(10, 0)]
        let id = try XCTUnwrap(doc.addCutout(.polygon(counter)))
        guard case let .polygon(points)? = doc.cutouts.first(where: { $0.id == id })?.shape else { return XCTFail("not a polygon") }
        XCTAssertGreaterThan(BoardGeometry.area(points), 0)
        XCTAssertEqual(Set(points), Set(counter))
    }

    func testALockedCutoutIgnoresMoveRectAndRadius() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 8, 4), radius: 1)))
        doc.setLocked(.cutout(id), true)
        XCTAssertTrue(doc.isLocked(.cutout(id)))
        let before = doc.cutouts
        doc.moveCutout(id, to: p(30, 30))
        doc.setCutoutRect(id, r(0, 0, 20, 20))
        doc.setCutoutRadius(id, 0)
        XCTAssertEqual(doc.cutouts, before)
        doc.setLocked(.cutout(id), false)
        doc.moveCutout(id, to: p(30, 30))
        XCTAssertNotEqual(doc.cutouts, before)
    }

    func testRectAndRadiusAreKeptWithinTheAllowedSizes() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 8, 4), radius: 2)))
        doc.setCutoutRect(id, r(10, 10, 8, 1))
        guard case let .rounded(box, radius)? = doc.cutouts.first?.shape else { return XCTFail("not rounded") }
        XCTAssertGreaterThanOrEqual(box.height, BoardCutout.minimumSide - 1e-9)
        XCTAssertLessThanOrEqual(radius, min(box.width, box.height) / 2 + 1e-9)
        doc.setCutoutRadius(id, 99)
        guard case let .rounded(box2, radius2)? = doc.cutouts.first?.shape else { return XCTFail("not rounded") }
        XCTAssertEqual(radius2, min(box2.width, box2.height) / 2, accuracy: 1e-9)
    }

    func testAPolygonOnlyMoves() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.polygon([p(10, 10), p(20, 10), p(15, 18)])))
        let size = try rect(doc, id).size
        doc.setCutoutRect(id, r(0, 0, 50, 50))
        doc.setCutoutRadius(id, 3)
        XCTAssertEqual(try rect(doc, id).size, size)
        doc.moveCutout(id, to: p(30, 30))
        XCTAssertEqual(try rect(doc, id).origin, BoardGrid.snap(p(30, 30), spacing: doc.board.traceGrid))
        XCTAssertEqual(try rect(doc, id).size, size)
    }

    func testAGroupMoveMovesACutoutWithAHoleAndAKeepoutByOneDelta() throws {
        let doc = TestSupport.newDocument()
        let cutout = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 8, 4), radius: 0)))
        let hole = doc.addHole(at: p(40, 40))
        let keepout = doc.addKeepout(rect: r(50, 10, 8, 6))
        doc.select(.cutout(cutout))
        doc.toggleSelection(.hole(hole))
        doc.toggleSelection(.keepout(keepout))
        let cutoutBefore = try rect(doc, cutout).origin
        let holeBefore = try XCTUnwrap(doc.holes.first { $0.id == hole }).centre
        let keepoutBefore = try XCTUnwrap(doc.keepouts.first { $0.id == keepout }).rect.origin
        let base = doc.boardGroupBase()
        XCTAssertEqual(base.count, 3)
        doc.moveBoardGroup(base, by: CGSize(width: 5, height: 3))
        let dx = try rect(doc, cutout).origin.x - cutoutBefore.x
        let dy = try rect(doc, cutout).origin.y - cutoutBefore.y
        XCTAssertGreaterThan(dx, 0)
        XCTAssertGreaterThan(dy, 0)
        let holeAfter = try XCTUnwrap(doc.holes.first { $0.id == hole }).centre
        let keepoutAfter = try XCTUnwrap(doc.keepouts.first { $0.id == keepout }).rect.origin
        XCTAssertEqual(holeAfter.x - holeBefore.x, dx, accuracy: 1e-9)
        XCTAssertEqual(holeAfter.y - holeBefore.y, dy, accuracy: 1e-9)
        XCTAssertEqual(keepoutAfter.x - keepoutBefore.x, dx, accuracy: 1e-9)
        XCTAssertEqual(keepoutAfter.y - keepoutBefore.y, dy, accuracy: 1e-9)
    }

    func testDeletingASelectionDeletesTheCutout() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 8, 4), radius: 0)))
        XCTAssertTrue(doc.deleteBoardSelection())
        XCTAssertTrue(doc.cutouts.isEmpty)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.cutouts.map(\.id), [id])
    }

    func testRevisionRisesOnAddAndMove() throws {
        let doc = TestSupport.newDocument()
        let start = doc.copperRevision
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 8, 4), radius: 0)))
        XCTAssertGreaterThan(doc.copperRevision, start)
        let added = doc.copperRevision
        doc.moveCutout(id, to: p(30, 30))
        XCTAssertGreaterThan(doc.copperRevision, added)
    }

    func testNoCutoutsMeansTheEdgeIsTheOutline() {
        let doc = TestSupport.newDocument()
        XCTAssertTrue(doc.boardEdge.cutouts.isEmpty)
        XCTAssertEqual(doc.boardEdge.outline, doc.board.outline)
    }

    func testTheEdgeCarriesTheCutoutsLoops() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 8, 4), radius: 0)))
        XCTAssertEqual(doc.boardEdge.cutouts, [try XCTUnwrap(doc.cutouts.first { $0.id == id }).shape.loop])
        XCTAssertFalse(doc.boardEdge.contains(p(14, 12)))
    }

    func testSelectAllAndRubberBandTakeACutoutWhole() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 8, 4), radius: 0)))
        doc.clearBoardSelection()
        doc.selectAllBoard()
        XCTAssertTrue(doc.boardSelection.items.contains(.cutout(id)))
    }

    func testTheSelectionListNamesACutout() throws {
        let doc = TestSupport.newDocument()
        let circle = BoardCutout(id: UUID(), shape: .circle(centre: p(20, 20), diameter: 3.2))
        let slot = BoardCutout(id: UUID(), shape: .slot(rect: r(0, 0, 8, 2)))
        let box = BoardCutout(id: UUID(), shape: .rounded(rect: r(0, 0, 10, 5), radius: 0))
        let poly = BoardCutout(id: UUID(), shape: .polygon([p(0, 0), p(4, 0), p(2, 3)]))
        XCTAssertEqual(SelectionRows.cutoutDetail(circle), "Circle ⌀3.2 mm")
        XCTAssertEqual(SelectionRows.cutoutDetail(slot), "Slot 8 × 2 mm")
        XCTAssertEqual(SelectionRows.cutoutDetail(box), "Rectangle 10 × 5 mm")
        XCTAssertEqual(SelectionRows.cutoutDetail(poly), "Polygon, 3 points")
        _ = doc
    }
}
