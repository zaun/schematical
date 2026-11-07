import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-16-E: the Trace tool, trace selection and the trace sidebar. The
/// drags run `TraceDrawing`'s steps in the order `PCBCanvasView` runs them,
/// as plan-15's gesture tests do for the Select tool.
@MainActor
final class TraceToolTests: XCTestCase {
    /// Screen points per mm at 100 %.
    private let scale = BoardScale.pointsPerMM

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// A Trace drag from `from` to `to` (board mm), as the canvas runs it.
    @discardableResult
    private func drag(_ doc: SchematicDocument, from: CGPoint, to: CGPoint, layer: CopperLayer = .top,
                      visible: BoardVisibility = BoardVisibility(),
                      onShowLayer: (CopperLayer) -> Void = { _ in }) -> UUID? {
        let start = TraceDrawing.anchor(doc, at: from, layer: layer, scale: scale)
        _ = TraceDrawing.preview(doc, from: start, to: to, layer: layer, scale: scale, diagonalFirst: false)
        return TraceDrawing.finish(doc, from: start, to: to, layer: layer, scale: scale, diagonalFirst: false,
                                   visible: visible, onShowLayer: onShowLayer)
    }

    func testDragAddsRoutedTopTraceAndSelectsIt() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(drag(doc, from: p(2.54, 2.54), to: p(10.16, 5.08)))
        XCTAssertEqual(doc.traces.count, 1)
        let trace = try XCTUnwrap(doc.traces.first)
        XCTAssertEqual(trace.layer, .top)
        XCTAssertEqual(trace.width, 10)
        XCTAssertEqual(trace.points.count, 3)
        for (got, want) in zip(trace.points, [p(2.54, 2.54), p(7.62, 2.54), p(10.16, 5.08)]) {
            XCTAssertTrue(TraceRoute.same(got, want), "\(trace.points)")
        }
        XCTAssertEqual(doc.boardSelection.items, [.trace(id)])
        // The tool stays Trace: the next drag adds another.
        XCTAssertNotNil(drag(doc, from: p(2.54, 10.16), to: p(5.08, 10.16)))
        XCTAssertEqual(doc.traces.count, 2)
    }

    func testZeroLengthPressAddsNothing() {
        let doc = TestSupport.newDocument()
        XCTAssertNil(drag(doc, from: p(2.54, 2.54), to: p(2.6, 2.5)))
        XCTAssertTrue(doc.traces.isEmpty)
    }

    func testDrawingOnHiddenLayerShowsIt() {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        doc.setLayerType(.inner(3), .wire)
        var shown: [CopperLayer] = []
        let hidden = BoardVisibility(hidden: [.copper(.inner(3))], copperLayers: doc.board.copperLayers)
        XCTAssertNotNil(drag(doc, from: p(0, 0), to: p(5.08, 0), layer: .inner(3), visible: hidden,
                             onShowLayer: { shown.append($0) }))
        XCTAssertEqual(shown, [.inner(3)])
        XCTAssertEqual(doc.traces.first?.layer, .inner(3))
        // A visible layer isn't asked for.
        XCTAssertNotNil(drag(doc, from: p(0, 5.08), to: p(5.08, 5.08), layer: .inner(3),
                             onShowLayer: { shown.append($0) }))
        XCTAssertEqual(shown, [.inner(3)])
    }

    // MARK: Phase 2: selection and sidebar

    func testClickOnTraceSelectsItOverFootprintUnderIt() throws {
        let doc = TestSupport.newDocument()
        let part = try TestSupport.place(doc, "SN74HC00D", at: p(100, 100))
        let bounds = try XCTUnwrap(doc.chips.first { $0.id == part.id }?.boardBounds)
        // A trace straight across the part's box, between pad rows.
        let y = BoardGrid.snap(bounds.midY)
        let id = try XCTUnwrap(doc.addTrace(from: p(BoardGrid.snap(bounds.minX - 2), y),
                                            to: p(BoardGrid.snap(bounds.maxX + 2), y), layer: .top,
                                            diagonalFirst: false))
        let tolerance = TraceDrawing.tolerance / scale
        XCTAssertEqual(doc.boardItem(at: p(bounds.midX, y), tolerance: tolerance), .trace(id))
        XCTAssertEqual(doc.boardItem(at: p(bounds.midX, bounds.minY + 0.3), tolerance: tolerance),
                       .footprint(part.id), "off the trace: the footprint")
        let press = doc.pressSelection(.trace(id), shift: false)
        doc.releaseSelection(press, wasClick: true)
        XCTAssertEqual(doc.selectedTraceID, id)
        XCTAssertEqual(PCBSidebarView.content(for: doc), .trace(id))
        XCTAssertNil(doc.selectedFootprintID)
        XCTAssertNil(doc.selectedTextID, "so Rotate stays disabled")
    }

    func testShiftClickAddsTraceToFootprintSelection() throws {
        let doc = TestSupport.newDocument()
        let part = try TestSupport.place(doc, "SN74HC00D", at: p(100, 100))
        let id = try XCTUnwrap(doc.addTrace(from: p(50, 50), to: p(60, 50), layer: .top, diagonalFirst: false))
        doc.select(.footprint(part.id))
        let press = doc.pressSelection(.trace(id), shift: true)
        doc.releaseSelection(press, wasClick: true)
        XCTAssertEqual(doc.boardSelection.items, [.footprint(part.id), .trace(id)])
        XCTAssertEqual(PCBSidebarView.content(for: doc), .selection)
        let rows = SelectionRows.board(doc)
        XCTAssertEqual(rows.map(\.accessibilityLabel).last, "Trace, Top, 10 mil")
        // Shift-click again takes it out.
        let again = doc.pressSelection(.trace(id), shift: true)
        doc.releaseSelection(again, wasClick: true)
        XCTAssertEqual(doc.boardSelection.items, [.footprint(part.id)])
    }

    func testGroupDragMovesTraceBySnappedDeltaAndRejoins() throws {
        let doc = TestSupport.newDocument()
        let part = try TestSupport.place(doc, "SN74HC00D", at: p(100, 100))
        let other = try XCTUnwrap(doc.addTrace(from: p(20.32, 0), to: p(20.32, 20.32), layer: .top,
                                               diagonalFirst: false))
        let moving = try XCTUnwrap(doc.addTrace(from: p(0, 5.08), to: p(10.16, 5.08), layer: .top,
                                                diagonalFirst: false))
        let start = try XCTUnwrap(doc.traces.first { $0.id == moving }).points
        let centre = try XCTUnwrap(doc.chips.first { $0.id == part.id }?.board?.centre)
        doc.select(.footprint(part.id))
        _ = doc.pressSelection(.trace(moving), shift: true)
        let base = doc.boardGroupBase()
        XCTAssertEqual(base.traces[moving], start)
        // Frames of a drag, ending 15.3 mm right (snaps to 15.24).
        for step in 1...4 {
            doc.moveBoardGroup(base, by: CGSize(width: 15.3 * CGFloat(step) / 4, height: 0))
        }
        let dx = BoardGrid.snap(15.3)
        let moved = try XCTUnwrap(doc.traces.first { $0.id == moving }).points
        XCTAssertEqual(moved.count, 2, "no join until the release")
        for (got, want) in zip(moved, start) {
            XCTAssertTrue(TraceRoute.same(got, p(want.x + dx, want.y)))
        }
        XCTAssertEqual(doc.chips.first { $0.id == part.id }?.board?.centre, p(centre.x + dx, centre.y))
        doc.settleBoardGroup(base)
        // Now from 15.24 to 25.4: across the other trace at x = 20.32.
        let crossing = p(20.32, 5.08)
        XCTAssertTrue(try XCTUnwrap(doc.traces.first { $0.id == moving }).points.contains { TraceRoute.same($0, crossing) })
        XCTAssertTrue(try XCTUnwrap(doc.traces.first { $0.id == other }).points.contains { TraceRoute.same($0, crossing) })
        XCTAssertEqual(Set(doc.connectedRun(of: moving)), [moving, other])
    }

    func testWidthClampsAndLayerPickerHasNoPlane() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(6)
        doc.setLayerType(.inner(4), .wire)
        let id = try XCTUnwrap(doc.addTrace(from: p(0, 0), to: p(5, 0), layer: .top, diagonalFirst: false))
        doc.setTraceWidth(id, 3)
        XCTAssertEqual(doc.traces.first?.width, 5)
        doc.setTraceWidth(id, 250)
        XCTAssertEqual(doc.traces.first?.width, 100)
        XCTAssertEqual(TracePanel.layers(for: doc.board), [.top, .inner(4), .bottom])
        // Delete drops it from the selection.
        doc.select(.trace(id))
        doc.deleteTrace(id)
        XCTAssertTrue(doc.traces.isEmpty)
        XCTAssertTrue(doc.boardSelection.isEmpty)
    }
}
