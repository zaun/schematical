import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Vias: the Via tool's placing, moving, the sidebar's sizes, joins across
/// layers, delete, undo and the file.
@MainActor
final class ViaTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    private let tolerance = TraceDrawing.tolerance / BoardScale.pointsPerMM

    private func via(_ doc: SchematicDocument, _ id: UUID) throws -> BoardVia {
        try XCTUnwrap(doc.vias.first { $0.id == id })
    }

    func testAddViaHasDefaultsAndIsSelected() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addVia(at: p(5.08, 5.08)))
        let added = try via(doc, id)
        XCTAssertEqual(added.centre, p(5.08, 5.08))
        XCTAssertEqual(added.drill, 0.4)
        XCTAssertEqual(added.diameter, 0.8)
        XCTAssertEqual(added.ring, 0.2, accuracy: 1e-9)
        XCTAssertEqual(doc.selectedViaID, id)
        XCTAssertEqual(PCBSidebarView.content(for: doc), .via(id))
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Via")

        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.vias.isEmpty)
        XCTAssertTrue(doc.redo(on: .pcb))
        XCTAssertEqual(doc.vias.map(\.id), [id])
    }

    func testOneLayerBoardHasNoVias() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        XCTAssertNil(doc.addVia(at: p(0, 0)))

        let two = TestSupport.newDocument()
        XCTAssertNotNil(two.addVia(at: p(0, 0)))
        XCTAssertEqual(two.canSetLayerCount(1), "Delete the vias first.")
        XCTAssertNil(two.canSetLayerCount(4))
    }

    /// A via lands on a trace there, else on the trace grid.
    func testViaAnchor() throws {
        let doc = TestSupport.newDocument()
        XCTAssertNotNil(doc.addTrace(from: p(0, 0), to: p(10, 0), layer: .bottom, diagonalFirst: false))
        XCTAssertEqual(doc.viaAnchor(at: p(10.05, 0.05), tolerance: tolerance), p(10, 0), "the end")
        XCTAssertEqual(doc.viaAnchor(at: p(4.1, 0.1), tolerance: tolerance), p(4.1, 0), "on the segment")
        let free = doc.viaAnchor(at: p(3.3, 5.0), tolerance: tolerance)
        XCTAssertEqual(free.x, 3.175, accuracy: 1e-9)
        XCTAssertEqual(free.y, 5.08, accuracy: 1e-9)
    }

    /// Trace ends snap to a via's centre on any layer.
    func testTraceEndsSnapToVias() throws {
        let doc = TestSupport.newDocument()
        try XCTUnwrap(doc.addVia(at: p(5.08, 5.08)))
        for layer in [CopperLayer.top, .bottom] {
            XCTAssertEqual(doc.traceAnchor(at: p(5.3, 5.0), layer: layer, tolerance: tolerance), p(5.08, 5.08))
        }
    }

    func testMoveSnapsToTheTraceGridAndGroupsMoveTogether() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addVia(at: p(0, 0)))
        doc.moveBoardGroup(doc.boardGroupBase(), by: CGSize(width: 1.0, height: 0.4))
        let moved = try via(doc, id).centre
        XCTAssertEqual(moved.x, 0.9525, accuracy: 1e-9)
        XCTAssertEqual(moved.y, 0.3175, accuracy: 1e-9)

        // With a trace in the selection, both move by one snapped step.
        let trace = try XCTUnwrap(doc.addTrace(from: p(10, 10), to: p(20, 10), layer: .top, diagonalFirst: false))
        doc.select(.via(id))
        doc.toggleSelection(.trace(trace))
        let base = doc.boardGroupBase()
        doc.moveBoardGroup(base, by: CGSize(width: 1.0, height: 0))
        XCTAssertEqual(try via(doc, id).centre.x, moved.x + 1.27, accuracy: 1e-9)
        XCTAssertEqual(doc.traces.first?.points.first?.x ?? 0, 11.27, accuracy: 1e-9)

        // A zero step leaves a via placed on a trace where it is.
        let onTrace = try XCTUnwrap(doc.addVia(at: p(14.1, 10)))
        doc.moveBoardGroup(doc.boardGroupBase(), by: .zero)
        XCTAssertEqual(try via(doc, onTrace).centre, p(14.1, 10))
    }

    func testSizesClampAndKeepARing() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addVia(at: p(0, 0)))
        doc.setViaDrill(id, 0.7)
        XCTAssertEqual(try via(doc, id).drill, 0.7, accuracy: 1e-9, "0.8 metal leaves 0.05 a side")
        doc.setViaDrill(id, 0.75)
        XCTAssertEqual(try via(doc, id).drill, 0.7, accuracy: 1e-9)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Via Hole")
        doc.setViaDiameter(id, 0.5)
        XCTAssertEqual(try via(doc, id).diameter, 0.8, accuracy: 1e-9, "can't shrink past the hole")
        doc.setViaDiameter(id, 20)
        XCTAssertEqual(try via(doc, id).diameter, 10)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Via Size")
        doc.setViaDrill(id, 0.01)
        XCTAssertEqual(try via(doc, id).drill, 0.1)
        doc.setViaCentre(id, p(1.234, 5.678))
        XCTAssertEqual(try via(doc, id).centre, p(1.234, 5.678), "X and Y are kept as typed")
    }

    func testSelectPicksAViaOverATraceAndDeleteRemovesIt() throws {
        let doc = TestSupport.newDocument()
        XCTAssertNotNil(doc.addTrace(from: p(0, 0), to: p(10, 0), layer: .top, diagonalFirst: false))
        let id = try XCTUnwrap(doc.addVia(at: p(5, 0)))
        XCTAssertEqual(doc.boardItem(at: p(5.1, 0), tolerance: tolerance), .via(id))
        XCTAssertEqual(doc.boardItems(in: CGRect(x: 4.5, y: -1, width: 1, height: 2)).contains(.via(id)), true)

        XCTAssertTrue(doc.deleteBoardItem(at: p(5.1, 0), tolerance: tolerance))
        XCTAssertTrue(doc.vias.isEmpty)
        XCTAssertEqual(doc.traces.count, 1, "the trace under it stays")

        let again = try XCTUnwrap(doc.addVia(at: p(5, 0)))
        XCTAssertTrue(doc.deleteBoardSelection())
        XCTAssertFalse(doc.vias.contains { $0.id == again })
        XCTAssertTrue(doc.boardSelection.isEmpty)
    }

    /// Two SMD parts wired together, routed Top → via → Bottom → via → Top.
    func testViasJoinTracesOnDifferentLayers() throws {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 400, y: 100))
        let y = try TestSupport.pin(one, "1Y"), a = try TestSupport.pin(two, "1A")
        XCTAssertNil(doc.connect(.pin(y), .pin(a)))
        func centre(_ chip: PlacedChip, _ ref: PinRef) throws -> CGPoint {
            let placed = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
            let pad = try XCTUnwrap(placed.boardPads.first { $0.designator == placed.part.pins[ref.index].designator })
            return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
        }
        let from = try centre(one, y), to = try centre(two, a)
        let down = p(from.x, from.y + 20), across = p(to.x, from.y + 20)
        XCTAssertNotNil(doc.addTrace(from: from, to: down, layer: .top, diagonalFirst: false))
        XCTAssertNotNil(doc.addTrace(from: down, to: across, layer: .bottom, diagonalFirst: false))
        XCTAssertNotNil(doc.addTrace(from: across, to: to, layer: .top, diagonalFirst: false))
        XCTAssertEqual(doc.ratsnest().count, 1, "no vias yet: the layers don't meet")

        XCTAssertNotNil(doc.addVia(at: down))
        XCTAssertEqual(doc.ratsnest().count, 1, "one via isn't enough")
        let second = try XCTUnwrap(doc.addVia(at: across))
        XCTAssertTrue(doc.ratsnest().isEmpty)
        XCTAssertEqual(doc.traces(touching: try via(doc, second)).count, 2)
    }

    func testFileRoundTripAndDefaults() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addVia(at: p(2.54, 3.81)))
        doc.setViaDrill(id, 0.3)
        doc.setViaDiameter(id, 0.65)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.vias, doc.vias)

        // No vias: no key.
        let empty = TestSupport.newDocument()
        XCTAssertNil(try SchematicFile.decode(SchematicFile(document: empty, fileURL: nil).encoded()).board?.vias)

        // Missing sizes are the defaults.
        let viaID = UUID()
        let plain = try BoardLayersTests.open(board: #"{ "vias": [ { "id": "\#(viaID)", "x": 1, "y": 2 } ] }"#)
        XCTAssertEqual(plain.vias, [BoardVia(id: viaID, centre: p(1, 2))])
    }

    func testFileErrors() {
        let id = UUID()
        let name = "via \(id.uuidString)"
        func board(_ extra: String, layers: Int = 2) -> String {
            #"{ "layers": \#(layers), "vias": [ { "id": "\#(id)", "x": 0, "y": 0\#(extra) } ] }"#
        }
        let cases: [(String, String)] = [
            (board(#", "drill": 0.05"#), "\(name) drill 0.05 must be 0.1 to 5 mm"),
            (board(#", "drill": 0.4, "diameter": 11"#), "\(name) diameter 11 must be 0.2 to 10 mm"),
            (board(#", "drill": 0.5, "diameter": 0.55"#),
             "\(name) diameter 0.55 must be at least 0.1 mm more than its drill 0.5"),
            (board("", layers: 1), "\(name) is on a 1-layer board"),
        ]
        for (json, message) in cases {
            XCTAssertEqual(BoardLayersTests.pcbError { _ = try BoardLayersTests.open(board: json) }, message, json)
        }
    }

    // MARK: Dragging a trace

    /// A lone trace snaps by the end nearer the grab, onto the trace grid.
    func testDraggedTraceSnapsByTheEndNearTheGrab() throws {
        let doc = TestSupport.newDocument()
        // Off the grid at both ends, by different amounts.
        let trace = BoardTrace(id: UUID(), layer: .top, width: 10, points: [p(0.1, 0.1), p(10.2, 0.1)])
        doc.replaceContents(chips: [], wires: [], junctions: [], buses: [], nextReference: 1,
                            board: BoardSettings(), texts: [], traces: [trace])
        doc.select(.trace(trace.id))

        // Grabbed near the start: the start lands on the grid.
        doc.moveBoardGroup(doc.boardGroupBase(grab: p(0.5, 0.1)), by: CGSize(width: 2.0, height: 1.0))
        var points = try XCTUnwrap(doc.traces.first).points
        XCTAssertEqual(points[0].x, 2.2225, accuracy: 1e-9)
        XCTAssertEqual(points[0].y, 0.9525, accuracy: 1e-9)
        XCTAssertEqual(points[1].x - points[0].x, 10.1, accuracy: 1e-9, "its shape is kept")

        // Grabbed near the end: the end lands on the grid.
        doc.moveBoardGroup(doc.boardGroupBase(grab: p(10, 1)), by: CGSize(width: 2.0, height: 1.0))
        points = try XCTUnwrap(doc.traces.first).points
        for value in [points[1].x, points[1].y] {
            let steps = value / 0.3175
            XCTAssertEqual(steps, steps.rounded(), accuracy: 1e-9, "\(points)")
        }

        // A click without moving leaves it alone.
        let before = try XCTUnwrap(doc.traces.first).points
        doc.moveBoardGroup(doc.boardGroupBase(grab: p(5, 1)), by: .zero)
        XCTAssertEqual(doc.traces.first?.points, before)
    }
}
