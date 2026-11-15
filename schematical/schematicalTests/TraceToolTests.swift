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
        let draft = TraceDraft(start: TraceDrawing.anchor(doc, at: from, layer: layer, scale: scale))
        _ = TraceDrawing.preview(doc, draft: draft, to: to, layer: layer, scale: scale, angle: 45)
        return TraceDrawing.finish(doc, draft: draft, to: to, layer: layer, scale: scale, angle: 45,
                                   visible: visible, onShowLayer: onShowLayer)
    }

    func testDragAddsRoutedTopTraceAndSelectsIt() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(drag(doc, from: p(2.54, 2.54), to: p(10.16, 5.08)))
        XCTAssertEqual(doc.traces.count, 1)
        let trace = try XCTUnwrap(doc.traces.first)
        XCTAssertEqual(trace.layer, .top)
        XCTAssertEqual(trace.width, NetRules.defaults(for: .current()).width, "the Default class's width")
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
        XCTAssertEqual(rows.map(\.accessibilityLabel).last, "Trace, Top, \(NetRules.defaults(for: .current()).width) mil")
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

    // MARK: Net classes (plan-38-C)

    /// Two LEDs wired K to A, the net in a 20 mil Power class with a
    /// 0.5 / 1.0 mm via; the centre of the first LED's K pad.
    private func powerPair() throws -> (doc: SchematicDocument, start: CGPoint) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: p(400, 200))
        let k = try TestSupport.pin(one, "K")
        XCTAssertNil(doc.connect(.pin(k), .pin(try TestSupport.pin(two, "A"))))
        let power = doc.addNetClass(named: "Power")
        doc.setNetClassWidth(power, 20)
        doc.setNetClassVia(power, drill: 0.5, diameter: 1)
        let designator = one.part.pins[k.index].designator
        doc.assignNet(.pin(chip: one.id, designator: designator), to: power)
        let chip = try XCTUnwrap(doc.chips.first { $0.id == one.id })
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return (doc, p(pad.rect.midX, pad.rect.midY))
    }

    func testTraceFromAPowerClassPadIsTheClassWidth() throws {
        let (doc, start) = try powerPair()
        let id = try XCTUnwrap(drag(doc, from: start, to: p(start.x + 7.62, start.y)))
        XCTAssertEqual(doc.traces.first { $0.id == id }?.width, 20)
    }

    func testTraceFromEmptyBoardIsTheDefaultWidth() throws {
        let (doc, start) = try powerPair()
        let id = try XCTUnwrap(drag(doc, from: p(start.x, start.y + 20), to: p(start.x + 7.62, start.y + 20)))
        XCTAssertEqual(doc.traces.first { $0.id == id }?.width, NetRules.defaults(for: .current()).width)
    }

    func testLiveTraceAndCommittedTraceAreOneWidth() throws {
        let (doc, start) = try powerPair()
        let end = p(start.x + 7.62, start.y + 2.54)
        let draft = TraceDraft(start: TraceDrawing.anchor(doc, at: start, layer: .top, scale: scale))
        let live = TraceDrawing.width(doc, draft: draft, layer: .top)
        let id = try XCTUnwrap(TraceDrawing.finish(doc, draft: draft, to: end, layer: .top, scale: scale, angle: 45,
                                                   visible: BoardVisibility(), onShowLayer: { _ in }))
        XCTAssertEqual(live, 20)
        XCTAssertEqual(doc.traces.first { $0.id == id }?.width, live)
    }

    func testViaOnAPowerClassPadHasTheClassSize() throws {
        let (doc, start) = try powerPair()
        let id = try XCTUnwrap(doc.addVia(at: start))
        let via = try XCTUnwrap(doc.vias.first { $0.id == id })
        XCTAssertEqual(via.drill, 0.5)
        XCTAssertEqual(via.diameter, 1)
        let plain = try XCTUnwrap(doc.addVia(at: p(start.x, start.y + 20)))
        let standard = NetRules.defaults(for: .current())
        XCTAssertEqual(doc.vias.first { $0.id == plain }?.drill, standard.viaDrill)
        XCTAssertEqual(doc.vias.first { $0.id == plain }?.diameter, standard.viaDiameter)
    }

    // MARK: Trace assist (plan-38-L)

    /// Two 8-pin headers in a row 30 mm apart, pin 1 to pin 1: the straight
    /// way from one pin 1 to the other crosses the first header's other pads.
    private func headers() throws -> (doc: SchematicDocument, from: CGPoint, to: CGPoint) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: p(100, 100))
        let two = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: p(400, 100))
        doc.moveFootprint(one.id, to: p(15, 30))
        doc.moveFootprint(two.id, to: p(45, 30))
        let a = try TestSupport.pin(one, "1"), b = try TestSupport.pin(two, "1")
        XCTAssertNil(doc.connect(.pin(a), .pin(b)))
        func centre(_ chip: PlacedChip, _ ref: PinRef) throws -> CGPoint {
            let placed = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
            let designator = placed.part.pins[ref.index].designator
            let pad = try XCTUnwrap(placed.boardPads.first { $0.designator == designator })
            return p(pad.rect.midX, pad.rect.midY)
        }
        return (doc, try centre(one, a), try centre(two, b))
    }

    /// An assisted drag, as the canvas runs it: the assist built at the
    /// press, the preview, the release, all in one gesture.
    private func assistedDrag(_ doc: SchematicDocument, from: CGPoint, to: CGPoint)
        -> (preview: [CGPoint], legal: Bool, id: UUID?) {
        doc.beginGesture(.pcb)
        defer { doc.endGesture() }
        let start = TraceDrawing.anchor(doc, at: from, layer: .top, scale: scale)
        let assist = AssistRouter(document: doc, layer: .top, start: start, rules: .customDefault)
        let draft = TraceDraft(start: start)
        let preview = TraceDrawing.assisted(doc, draft: draft, to: to, layer: .top, scale: scale, angle: 45, assist: assist)
        let id = TraceDrawing.finish(doc, draft: draft, to: to, layer: .top, scale: scale, angle: 45,
                                     visible: BoardVisibility(), assist: assist, onShowLayer: { _ in })
        return (preview.points, preview.isLegal, id)
    }

    func testAnAssistedTraceIsItsPreviewAndClearOfForeignPads() throws {
        let (doc, from, to) = try headers()
        let drag = assistedDrag(doc, from: from, to: to)
        XCTAssertTrue(drag.legal)
        let id = try XCTUnwrap(drag.id)
        let trace = try XCTUnwrap(doc.traces.first { $0.id == id })
        XCTAssertEqual(trace.points, drag.preview)
        XCTAssertGreaterThan(trace.points.count, 2, "it walks around pins 2–8")
        XCTAssertEqual(doc.designRuleCheck(rules: .customDefault).filter { $0.kind == .clearance }.map(\.issue), [])
        XCTAssertTrue(doc.ratsnest().isEmpty, "pin 1 to pin 1 is joined")
    }

    func testAnUnreachableAssistedReleaseAddsNothing() throws {
        let (doc, from, to) = try headers()
        // A trace on no net from the board's top edge to its bottom edge,
        // between the headers, as its own step.
        let ys = doc.board.outline.map(\.y)
        let x = (from.x + to.x) / 2 + 3
        doc.beginGesture(.pcb)
        let wall = try XCTUnwrap(doc.addTrace(points: [p(x, ys.min()!), p(x, ys.max()!)], layer: .top, width: 10))
        doc.endGesture()
        doc.editorMessage = nil
        let drag = assistedDrag(doc, from: from, to: to)
        XCTAssertFalse(drag.legal)
        XCTAssertNil(drag.id)
        XCTAssertEqual(doc.traces.map(\.id), [wall])
        XCTAssertEqual(doc.editorMessage, TraceDrawing.noRouteMessage)
        // The drag recorded no step: one undo takes the wall away.
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.traces.isEmpty)
    }

    // MARK: The V key (plan-38-L)

    /// A drag from `from` with V pressed at `via`, released at `to`, in one
    /// gesture, as the canvas runs it. Returns the layer V moved to (nil when
    /// V did nothing) and the trace the release added.
    @discardableResult
    private func vDrag(_ doc: SchematicDocument, from: CGPoint, via: CGPoint, to: CGPoint, layer: CopperLayer = .top)
        -> (layer: CopperLayer?, last: UUID?) {
        doc.beginGesture(.pcb)
        defer { doc.endGesture() }
        var draft = TraceDraft(start: TraceDrawing.anchor(doc, at: from, layer: layer, scale: scale))
        var current = layer
        let dropped = TraceDrawing.dropVia(doc, draft: draft, to: via, layer: layer, scale: scale, angle: 45,
                                           assist: nil, rules: .customDefault)
        if let dropped {
            draft = dropped.draft
            current = dropped.layer
        }
        let last = TraceDrawing.finish(doc, draft: draft, to: to, layer: current, scale: scale, angle: 45,
                                       visible: BoardVisibility(), onShowLayer: { _ in })
        return (dropped?.layer, last)
    }

    func testVDropsAViaAndGoesOnOnTheOtherLayer() throws {
        let (doc, start) = try powerPair()
        let corner = p(start.x + 7.62, start.y + 5.08), end = p(start.x + 15.24, start.y + 5.08)
        let (layer, last) = vDrag(doc, from: start, via: corner, to: end)
        XCTAssertEqual(layer, .bottom)
        XCTAssertEqual(doc.traces.count, 2)
        let top = try XCTUnwrap(doc.traces.first { $0.layer == .top })
        XCTAssertTrue(TraceRoute.same(top.points.first!, start))
        XCTAssertTrue(TraceRoute.same(top.points.last!, corner))
        let via = try XCTUnwrap(doc.vias.first)
        XCTAssertEqual(doc.vias.count, 1)
        XCTAssertTrue(TraceRoute.same(via.centre, corner))
        XCTAssertEqual(via.drill, 0.5, "the start net's class via")
        XCTAssertEqual(via.diameter, 1)
        let bottom = try XCTUnwrap(doc.traces.first { $0.id == last })
        XCTAssertEqual(bottom.layer, .bottom)
        XCTAssertTrue(TraceRoute.same(bottom.points.first!, corner))
        XCTAssertEqual(bottom.width, 20, "still the class width")
        // The whole drag is one step.
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.traces.isEmpty)
        XCTAssertTrue(doc.vias.isEmpty)
    }

    func testVOnAOneLayerBoardDoesNothing() throws {
        let (doc, start) = try powerPair()
        doc.setLayerCount(1)
        let corner = p(start.x + 7.62, start.y + 5.08)
        let (layer, _) = vDrag(doc, from: start, via: corner, to: p(start.x + 15.24, start.y + 5.08))
        XCTAssertNil(layer)
        XCTAssertTrue(doc.vias.isEmpty)
        XCTAssertEqual(doc.traces.count, 1, "only the release's trace")
        XCTAssertEqual(doc.traces.first?.layer, .top)
    }

    func testVWhereAViaWouldBreakClearanceAddsNothing() throws {
        let (doc, start) = try powerPair()
        // The same LED's A pad: another net (none), 2.54 mm from its K.
        let one = try XCTUnwrap(doc.chips.first)
        let a = try TestSupport.pin(one, "A")
        let designator = one.part.pins[a.index].designator
        let pad = try XCTUnwrap(one.boardPads.first { $0.designator == designator })
        let near = p(pad.rect.midX, pad.rect.midY)
        doc.editorMessage = nil
        doc.beginGesture(.pcb)
        let draft = TraceDraft(start: TraceDrawing.anchor(doc, at: start, layer: .top, scale: scale))
        XCTAssertNil(TraceDrawing.dropVia(doc, draft: draft, to: near, layer: .top, scale: scale, angle: 45,
                                          assist: nil, rules: .customDefault))
        doc.endGesture()
        XCTAssertTrue(doc.traces.isEmpty)
        XCTAssertTrue(doc.vias.isEmpty)
        XCTAssertEqual(doc.editorMessage, AssistRouter.viaMessage)
    }

    func testVSkipsAPlane() throws {
        let (doc, start) = try powerPair()
        doc.setLayerCount(4)
        doc.setLayerType(.inner(2), .plane)
        XCTAssertFalse(doc.board.traceLayers.contains(.inner(2)))
        let corner = p(start.x + 7.62, start.y + 5.08)
        let (layer, _) = vDrag(doc, from: start, via: corner, to: p(start.x + 15.24, start.y + 5.08))
        let layers = doc.board.traceLayers
        XCTAssertEqual(layer, layers[1], "the next trace layer after Top: \(layers)")
        XCTAssertNotEqual(layer, .inner(2))
        // From the last trace layer, V wraps to Top.
        let from = try XCTUnwrap(layers.last)
        let draft = TraceDraft(start: corner)
        let back = TraceDrawing.dropVia(doc, draft: draft, to: p(corner.x, corner.y + 10.16), layer: from, scale: scale,
                                        angle: 45, assist: nil, rules: .customDefault)
        XCTAssertEqual(back?.layer, .top)
    }

    func testWithoutAssistTheDragIsThePlainRoute() throws {
        let (doc, from, to) = try headers()
        let draft = TraceDraft(start: TraceDrawing.anchor(doc, at: from, layer: .top, scale: scale))
        let plain = TraceDrawing.assisted(doc, draft: draft, to: to, layer: .top, scale: scale, angle: 45, assist: nil)
        XCTAssertTrue(plain.isLegal)
        XCTAssertEqual(plain.points, TraceDrawing.preview(doc, draft: draft, to: to, layer: .top, scale: scale, angle: 45))
        XCTAssertEqual(plain.points.count, 2, "straight across the pads, as before")
    }
}
