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
}
