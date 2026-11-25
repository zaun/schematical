import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Copper pours and keep-outs (plan-52-A): the value types, the document's
/// state and undo, and the edit API.
@MainActor
final class PourModelTests: XCTestCase {
    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect {
        CGRect(x: x, y: y, width: w, height: h)
    }

    // MARK: Value types

    func testPourDefaults() {
        let pour = BoardPour(id: UUID(), rect: r(0, 0, 10, 10), layer: .top)
        XCTAssertNil(pour.net)
        XCTAssertNil(pour.clearance)
        XCTAssertEqual(pour.connection, .thermal)
        XCTAssertEqual(pour.thermalGap, 0.5)
        XCTAssertEqual(pour.spokeWidth, 0.5)
        XCTAssertEqual(pour.minWidth, 0.2)
        XCTAssertFalse(pour.locked)
        var rules = DesignRules.customDefault
        XCTAssertEqual(pour.clearance(rules: rules), rules.minClearance)
        var own = pour
        own.clearance = 0.7
        rules.minClearance = 0.1
        XCTAssertEqual(own.clearance(rules: rules), 0.7)
    }

    func testRanges() {
        XCTAssertEqual(BoardPour.minimumSide, 0.5)
        XCTAssertEqual(BoardPour.gapRange, 0.1...5)
        XCTAssertEqual(BoardPour.spokeRange, 0.1...5)
        XCTAssertEqual(BoardPour.minWidthRange, 0.05...2)
        XCTAssertEqual(LayerStyle.clearanceRange, 0.05...10)
        XCTAssertNil(BoardPour.problem(rect: r(0, 0, 1, 1), clearance: nil, thermalGap: 0.5, spokeWidth: 0.5, minWidth: 0.2))
        XCTAssertEqual(BoardPour.problem(rect: r(0, 0, 0.1, 5), clearance: nil, thermalGap: 0.5, spokeWidth: 0.5, minWidth: 0.2),
                       "width 0.1 must be at least 0.5 mm")
        XCTAssertEqual(BoardPour.problem(rect: r(0, 0, 5, 0.1), clearance: nil, thermalGap: 0.5, spokeWidth: 0.5, minWidth: 0.2),
                       "height 0.1 must be at least 0.5 mm")
        XCTAssertEqual(BoardPour.problem(rect: r(0, 0, 5, 5), clearance: 20, thermalGap: 0.5, spokeWidth: 0.5, minWidth: 0.2),
                       "clearance 20 must be 0.05 to 10 mm")
        XCTAssertEqual(BoardPour.problem(rect: r(0, 0, 5, 5), clearance: nil, thermalGap: 9, spokeWidth: 0.5, minWidth: 0.2),
                       "thermalGap 9 must be 0.1 to 5 mm")
        XCTAssertEqual(BoardPour.problem(rect: r(0, 0, 5, 5), clearance: nil, thermalGap: 0.5, spokeWidth: 9, minWidth: 0.2),
                       "spokeWidth 9 must be 0.1 to 5 mm")
        XCTAssertEqual(BoardPour.problem(rect: r(0, 0, 5, 5), clearance: nil, thermalGap: 0.5, spokeWidth: 0.5, minWidth: 3),
                       "minWidth 3 must be 0.05 to 2 mm")
        XCTAssertNil(BoardKeepout.problem(rect: r(0, 0, 1, 1)))
        XCTAssertEqual(BoardKeepout.problem(rect: r(0, 0, 0.1, 1)), "width 0.1 must be at least 0.5 mm")
    }

    func testNormalizedOfAReversedAndATooSmallRect() {
        XCTAssertEqual(BoardPour.normalized(r(10, 10, -4, -3)), r(6, 7, 4, 3))
        XCTAssertEqual(BoardPour.normalized(r(1, 2, 0.1, 0.2)), r(1, 2, 0.5, 0.5))
        XCTAssertEqual(BoardPour.normalized(r(0, 0, 5, 5)), r(0, 0, 5, 5))
    }

    func testKeepoutAppliesToItsLayers() {
        var keepout = BoardKeepout(id: UUID(), rect: r(0, 0, 5, 5))
        XCTAssertTrue(keepout.applies(to: .top))
        XCTAssertTrue(keepout.applies(to: .inner(2)))
        keepout.layers = [.bottom]
        XCTAssertFalse(keepout.applies(to: .top))
        XCTAssertTrue(keepout.applies(to: .bottom))
        keepout.layers = []
        XCTAssertFalse(keepout.applies(to: .bottom), "an empty set blocks nothing")
    }

    // MARK: Document state and undo

    func testAddedPourUndoesAndRedoes() throws {
        let doc = TestSupport.newDocument()
        XCTAssertTrue(doc.pours.isEmpty)
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 15), layer: .top))
        XCTAssertEqual(doc.undoName, "Add Pour")
        XCTAssertEqual(doc.pours.map(\.id), [id])
        XCTAssertEqual(doc.boardSelection.sole, .pour(id))
        let added = doc.pours
        XCTAssertTrue(doc.undo())
        XCTAssertTrue(doc.pours.isEmpty)
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.pours, added)
    }

    func testAddedKeepoutUndoesAndRedoes() {
        let doc = TestSupport.newDocument()
        let id = doc.addKeepout(rect: r(5, 5, 10, 10), layers: [.top])
        XCTAssertEqual(doc.undoName, "Add Keep-out")
        XCTAssertEqual(doc.keepouts.map(\.id), [id])
        let added = doc.keepouts
        XCTAssertTrue(doc.undo())
        XCTAssertTrue(doc.keepouts.isEmpty)
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.keepouts, added)
    }

    func testAStepThatChangesAPourAndAHoleUndoesBoth() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 15), layer: .top))
        doc.recordEdit(.pcb, "Both") {
            doc.setPourConnection(id, .solid)
            doc.addHole(at: CGPoint(x: 50, y: 50))
        }
        XCTAssertEqual(doc.undoName, "Both")
        XCTAssertEqual(doc.holes.count, 1)
        XCTAssertTrue(doc.undo())
        XCTAssertTrue(doc.holes.isEmpty)
        XCTAssertEqual(doc.pours.first?.connection, .thermal)
    }

    func testChangingAPourBumpsTheCopperRevision() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 15), layer: .top))
        let revision = doc.copperRevision
        doc.setPourClearance(id, 0.4)
        XCTAssertGreaterThan(doc.copperRevision, revision)
        let kept = doc.copperRevision
        doc.addKeepout(rect: r(0, 0, 5, 5))
        XCTAssertGreaterThan(doc.copperRevision, kept)
    }

    // MARK: Edit API

    /// The first pour drawn starts with KiCad's zone defaults; the type's
    /// own defaults (what a file that leaves them out reads as) are unchanged.
    func testFirstPourStartsWithKiCadsDefaults() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 15), layer: .top))
        let pour = try XCTUnwrap(doc.pours.first { $0.id == id })
        XCTAssertEqual(pour.clearance, 0.5)
        XCTAssertEqual(pour.minWidth, 0.25)
        XCTAssertEqual(pour.thermalGap, 0.5)
        XCTAssertEqual(pour.spokeWidth, 0.5)
        let later = try XCTUnwrap(doc.addPour(rect: r(40, 10, 10, 10), layer: .bottom))
        XCTAssertEqual(doc.pours.first { $0.id == later }?.clearance, 0.5)
    }

    func testNewPourTakesTheLastPoursSettingsAndNet() throws {
        let doc = TestSupport.newDocument()
        _ = try TestSupport.place(doc, "GND", at: CGPoint(x: 100, y: 100))
        let first = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 15), layer: .top))
        XCTAssertEqual(doc.pours.first?.net, .ground, "the first pour takes ground when the schematic has it")
        doc.setPourNet(first, .rail(mv: 5000))
        doc.setPourConnection(first, .solid)
        doc.setPourThermalGap(first, 0.8)
        doc.setPourClearance(first, 0.4)
        let second = try XCTUnwrap(doc.addPour(rect: r(40, 10, 10, 10), layer: .bottom))
        let pour = try XCTUnwrap(doc.pours.first { $0.id == second })
        XCTAssertEqual(pour.net, .rail(mv: 5000))
        XCTAssertEqual(pour.connection, .solid)
        XCTAssertEqual(pour.thermalGap, 0.8)
        XCTAssertEqual(pour.clearance, 0.4)
        let none = TestSupport.newDocument()
        let bare = try XCTUnwrap(none.addPour(rect: r(0, 0, 5, 5), layer: .top))
        XCTAssertNil(none.pours.first { $0.id == bare }?.net, "no ground in the schematic: no net")
    }

    func testAddPourSnapsAndRefusesAPlaneLayer() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let id = try XCTUnwrap(doc.addPour(rect: r(10.01, 10.02, 20, 15), layer: .top))
        let grid = doc.board.traceGrid
        let rect = try XCTUnwrap(doc.pours.first { $0.id == id }?.rect)
        XCTAssertEqual(rect.minX, BoardGrid.snap(10.01, spacing: grid))
        XCTAssertEqual(rect.minY, BoardGrid.snap(10.02, spacing: grid))
        XCTAssertNil(doc.addPour(rect: r(0, 0, 5, 5), layer: .inner(2)), "inner layers start as Planes")
        doc.setLayerType(.inner(2), .wire)
        XCTAssertNotNil(doc.addPour(rect: r(0, 0, 5, 5), layer: .inner(2)))
        XCTAssertNil(doc.addPour(rect: r(0, 0, 5, 5), layer: .inner(3)))
    }

    func testPourSettersClampAndSkipUnchangedValues() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 15), layer: .top))
        func pour() throws -> BoardPour { try XCTUnwrap(doc.pours.first { $0.id == id }) }
        doc.setPourClearance(id, 99)
        XCTAssertEqual(try pour().clearance, 10)
        doc.setPourClearance(id, 0.001)
        XCTAssertEqual(try pour().clearance, 0.05)
        doc.setPourClearance(id, nil)
        XCTAssertNil(try pour().clearance)
        doc.setPourThermalGap(id, 99)
        XCTAssertEqual(try pour().thermalGap, 5)
        doc.setPourSpokeWidth(id, 0)
        XCTAssertEqual(try pour().spokeWidth, 0.1)
        doc.setPourMinWidth(id, 99)
        XCTAssertEqual(try pour().minWidth, 2)
        XCTAssertEqual(doc.undoName, "Change Pour")
        doc.setPourNet(id, .ground)
        XCTAssertEqual(doc.undoName, "Change Pour Net")
        let steps = doc.undoName
        doc.setPourNet(id, .ground)
        XCTAssertEqual(doc.undoName, steps)
        doc.setPourConnection(id, .solid)
        doc.setPourConnection(id, .solid)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(try pour().connection, .thermal, "the repeated set was not a second step")
    }

    func testPourRectMoveAndResize() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 15), layer: .top))
        doc.movePour(id, to: CGPoint(x: 40, y: 30))
        XCTAssertEqual(doc.undoName, "Move")
        let size = try XCTUnwrap(doc.pours.first?.rect.size)
        XCTAssertEqual(size.width, 20, accuracy: doc.board.traceGrid, "edges snap to the trace grid")
        XCTAssertEqual(size.height, 15, accuracy: doc.board.traceGrid)
        XCTAssertEqual(doc.pours.first?.rect.origin, BoardGrid.snap(CGPoint(x: 40, y: 30), spacing: doc.board.traceGrid))
        doc.setPourRect(id, r(5, 5, 0.1, 40))
        XCTAssertEqual(doc.undoName, "Resize")
        XCTAssertEqual(doc.pours.first?.rect.width, CGFloat(BoardPour.minimumSide))
        doc.setPourLayer(id, .inner(3))
        XCTAssertEqual(doc.pours.first?.layer, .top, "not a layer of this 2-layer board")
        doc.setPourLayer(id, .bottom)
        XCTAssertEqual(doc.undoName, "Change Pour Layer")
        XCTAssertEqual(doc.pours.first?.layer, .bottom)
    }

    func testLockedPoursAndKeepoutsStayPut() throws {
        let doc = TestSupport.newDocument()
        let pour = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 15), layer: .top))
        let keepout = doc.addKeepout(rect: r(50, 10, 5, 5))
        doc.setLocked(.pour(pour), true)
        doc.setLocked(.keepout(keepout), true)
        XCTAssertTrue(doc.isLocked(.pour(pour)))
        XCTAssertTrue(doc.isLocked(.keepout(keepout)))
        let pours = doc.pours, keepouts = doc.keepouts
        doc.movePour(pour, to: CGPoint(x: 0, y: 0))
        doc.setPourRect(pour, r(0, 0, 3, 3))
        doc.moveKeepout(keepout, to: CGPoint(x: 0, y: 0))
        doc.setKeepoutRect(keepout, r(0, 0, 3, 3))
        XCTAssertEqual(doc.pours, pours)
        XCTAssertEqual(doc.keepouts, keepouts)
        doc.setLocked(.pour(pour), false)
        XCTAssertFalse(doc.isLocked(.pour(pour)))
    }

    func testKeepoutLayersAndDelete() {
        let doc = TestSupport.newDocument()
        let id = doc.addKeepout(rect: r(5, 5, 10, 10))
        XCTAssertNil(doc.keepouts.first?.layers)
        doc.setKeepoutLayers(id, [.bottom])
        XCTAssertEqual(doc.keepouts.first?.layers, [.bottom])
        XCTAssertEqual(doc.undoName, "Change Keep-out")
        doc.deleteKeepout(id)
        XCTAssertTrue(doc.keepouts.isEmpty)
        XCTAssertNil(doc.boardSelection.sole)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.keepouts.count, 1)
    }

    func testDeleteSelectionTakesPoursAndKeepouts() throws {
        let doc = TestSupport.newDocument()
        let pour = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 15), layer: .top))
        let keepout = doc.addKeepout(rect: r(50, 10, 5, 5))
        doc.selectBoardItems([.pour(pour), .keepout(keepout)])
        XCTAssertTrue(doc.deleteBoardSelection())
        XCTAssertTrue(doc.pours.isEmpty)
        XCTAssertTrue(doc.keepouts.isEmpty)
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.pours.count, 1)
        XCTAssertEqual(doc.keepouts.count, 1)
    }

    func testGroupDragMovesRectsByTheSnappedDelta() throws {
        let doc = TestSupport.newDocument()
        let pour = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 15), layer: .top))
        let keepout = doc.addKeepout(rect: r(50, 10, 5, 5))
        doc.selectBoardItems([.pour(pour), .keepout(keepout)])
        let base = doc.boardGroupBase()
        XCTAssertEqual(base.count, 2)
        let pourFrom = try XCTUnwrap(doc.pours.first?.rect.origin), keepoutFrom = try XCTUnwrap(doc.keepouts.first?.rect.origin)
        doc.moveBoardGroup(base, by: CGSize(width: 5, height: 3))
        let dx = BoardGrid.snap(5, spacing: doc.board.grid), dy = BoardGrid.snap(3, spacing: doc.board.grid)
        XCTAssertEqual(doc.pours.first?.rect.origin, CGPoint(x: pourFrom.x + dx, y: pourFrom.y + dy))
        XCTAssertEqual(doc.keepouts.first?.rect.origin, CGPoint(x: keepoutFrom.x + dx, y: keepoutFrom.y + dy))
    }

    func testSelectionListNamesThem() throws {
        let doc = TestSupport.newDocument()
        let pour = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 15), layer: .top))
        let keepout = doc.addKeepout(rect: r(50, 10, 5, 5))
        doc.selectBoardItems([.pour(pour), .keepout(keepout)])
        let rows = SelectionRows.board(doc)
        XCTAssertEqual(rows.map(\.title), ["Pour", "Keep-out"])
    }
}
