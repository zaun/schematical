import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// The PCB canvas's presses, pinches, hover and drawing keys
/// (`PCBCanvasController`), run on a canvas value as the view's closures
/// run them. The viewport is at 100 % with no offset, so a board point `b`
/// (mm) is the screen point `b × BoardScale.pointsPerMM`.
@MainActor
final class PCBCanvasControllerTests: XCTestCase {
    private let ppm = BoardScale.pointsPerMM

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    private func screen(_ mm: CGPoint) -> CGPoint { CGPoint(x: mm.x * ppm, y: mm.y * ppm) }
    /// A screen movement of `x`, `y` mm.
    private func moved(_ x: CGFloat, _ y: CGFloat) -> CGSize { CGSize(width: x * ppm, height: y * ppm) }

    private func canvas(_ doc: SchematicDocument, _ tool: PCBTool = .select, viewport: CanvasViewport? = nil,
                        traceAssist: Bool = false, highlighted: Binding<PlaneNet?> = .constant(nil),
                        pointer: BoardPointer? = nil, onRoute: @escaping (RoutingOptions.Scope) -> Void = { _ in },
                        onPlaced: @escaping () -> Void = {},
                        onLayerChange: @escaping (CopperLayer) -> Void = { _ in }) -> PCBCanvasView {
        PCBCanvasView(document: doc, viewport: viewport ?? CanvasViewport(), tool: tool, traceAssist: traceAssist,
                      onLayerChange: onLayerChange, pointer: pointer, highlightedNet: highlighted, onRoute: onRoute,
                      onPlaced: onPlaced)
    }

    /// A whole press: down at `from` (mm), one step by `by` (mm), released.
    private func press(_ controller: PCBCanvasController, _ view: PCBCanvasView, at from: CGPoint,
                       by: CGSize = .zero, modifiers: NSEvent.ModifierFlags = []) {
        controller.dragChanged(start: screen(from), translation: .zero, in: view, modifiers: modifiers)
        let step = moved(by.width, by.height)
        controller.dragChanged(start: screen(from), translation: step, in: view, modifiers: modifiers)
        controller.dragEnded(translation: step, in: view)
    }

    /// Two LEDs wired K to A, apart on the board; their pads' centres.
    private func pair() throws -> (doc: SchematicDocument, one: PlacedChip, k: CGPoint, a: CGPoint) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: p(400, 200))
        doc.moveFootprint(one.id, to: p(30, 40))
        doc.moveFootprint(two.id, to: p(60, 40))
        let kRef = try TestSupport.pin(one, "K"), aRef = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(kRef), .pin(aRef)))
        func centre(_ ref: PinRef) throws -> CGPoint {
            let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
            let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == chip.part.pins[ref.index].designator })
            return p(pad.rect.midX, pad.rect.midY)
        }
        return (doc, try XCTUnwrap(doc.chips.first { $0.id == one.id }), try centre(kRef), try centre(aRef))
    }

    // MARK: Tools

    /// The Trace tool: a press anchors a draft, a drag moves its cursor, the
    /// release adds the trace on the tool's layer, selects it, and the press
    /// was one undo step.
    func testTraceToolDrawsATrace() throws {
        let doc = TestSupport.newDocument()
        let controller = PCBCanvasController()
        let view = canvas(doc, .trace)
        controller.dragChanged(start: screen(p(2.54, 2.54)), translation: .zero, in: view, modifiers: [])
        XCTAssertTrue(doc.isGestureOpen)
        XCTAssertEqual(controller.drawingLayer, .top)
        XCTAssertNil(controller.assist)
        XCTAssertEqual(controller.activeLayer(view), .top)
        XCTAssertNil(controller.hoverRingLayer(view), "a drawn trace has its own ring")
        XCTAssertTrue(controller.showsHoveredPad)
        let step = moved(7.62, 2.54)
        let got = controller.handle(start: screen(p(2.54, 2.54)), translation: step, in: view)
        guard case let .drawTrace(_, cursor) = got else { return XCTFail("\(got)") }
        XCTAssertEqual(cursor.x, 10.16, accuracy: 1e-9)
        controller.dragEnded(translation: step, in: view)
        let trace = try XCTUnwrap(doc.traces.first)
        XCTAssertEqual(doc.boardSelection.items, [.trace(trace.id)])
        XCTAssertNil(controller.interaction)
        XCTAssertNil(controller.drawingLayer)
        XCTAssertFalse(doc.isGestureOpen)
        XCTAssertEqual(controller.hoverRingLayer(view), .top, "waiting for a press: the ring follows the pointer")
    }

    /// With the assist on, the press builds the assist router.
    func testTraceAssistIsBuiltAtThePress() {
        let doc = TestSupport.newDocument()
        let controller = PCBCanvasController()
        controller.dragChanged(start: screen(p(2.54, 2.54)), translation: .zero,
                               in: canvas(doc, .trace, traceAssist: true), modifiers: [])
        XCTAssertNotNil(controller.assist)
        XCTAssertNotNil(controller.drawState.assist)
    }

    /// Via and Hole: a press adds one and the drag moves it; no via on a
    /// 1-layer board.
    func testViaAndHolePlaceAndMove() throws {
        let doc = TestSupport.newDocument()
        let controller = PCBCanvasController()
        press(controller, canvas(doc, .via), at: p(10.16, 10.16), by: CGSize(width: 2.54, height: 0))
        let via = try XCTUnwrap(doc.vias.first)
        XCTAssertEqual(via.centre.x, 12.7, accuracy: 1e-6)
        press(controller, canvas(doc, .hole), at: p(20.32, 10.16), by: CGSize(width: 0, height: 2.54))
        let hole = try XCTUnwrap(doc.holes.first)
        XCTAssertEqual(hole.centre.y, 12.7, accuracy: 1e-6)

        let flat = TestSupport.newDocument()
        flat.setLayerCount(1)
        XCTAssertEqual(flat.board.layers, 1)
        controller.dragChanged(start: screen(p(30.48, 10.16)), translation: .zero, in: canvas(flat, .via), modifiers: [])
        XCTAssertEqual(controller.interaction, .idle)
        XCTAssertFalse(controller.showsHoveredPad)
        controller.dragEnded(translation: .zero, in: canvas(flat, .via))
        XCTAssertTrue(flat.vias.isEmpty)
    }

    /// Pour, Keep-out and Cutout: the drag is the rectangle, the release
    /// makes the item on the Trace layer.
    func testRectangleToolsDragOutTheirItem() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 100, height: 80)
        let controller = PCBCanvasController()
        let view = canvas(doc, .pour)
        controller.dragChanged(start: screen(p(10, 10)), translation: .zero, in: view, modifiers: [])
        controller.dragChanged(start: screen(p(10, 10)), translation: moved(20, 15), in: view, modifiers: [])
        guard case let .drawRect(kind, from, to)? = controller.interaction else {
            return XCTFail("\(String(describing: controller.interaction))")
        }
        XCTAssertEqual(kind, .pour)
        XCTAssertEqual(from, p(10, 10))
        XCTAssertEqual(to.x, 30, accuracy: 1e-9)
        controller.dragEnded(translation: moved(20, 15), in: view)
        XCTAssertEqual(doc.pours.count, 1)
        press(controller, canvas(doc, .keepout), at: p(50, 10), by: CGSize(width: 10, height: 10))
        XCTAssertEqual(doc.keepouts.count, 1)
        press(controller, canvas(doc, .cutout), at: p(50, 50), by: CGSize(width: 10, height: 10))
        XCTAssertEqual(doc.cutouts.count, 1)
    }

    /// Route: a press on a pad routes its net; on empty board, nothing.
    func testRouteToolNamesTheNetUnderThePress() throws {
        let (doc, _, k, _) = try pair()
        let controller = PCBCanvasController()
        var routed: [RoutingOptions.Scope] = []
        press(controller, canvas(doc, .route, onRoute: { routed.append($0) }), at: k)
        XCTAssertEqual(routed.count, 1)
        guard case .nets? = routed.first else { return XCTFail("\(routed)") }
        press(controller, canvas(doc, .route, onRoute: { routed.append($0) }), at: p(5, 75))
        XCTAssertEqual(routed.count, 1)
    }

    /// Delete: a press deletes the trace under it.
    func testDeleteToolDeletesTheTraceUnderIt() throws {
        let doc = TestSupport.newDocument()
        XCTAssertNotNil(doc.addTrace(from: p(5.08, 5.08), to: p(15.24, 5.08), layer: .top, diagonalFirst: false))
        let controller = PCBCanvasController()
        press(controller, canvas(doc, .delete), at: p(10.16, 5.08))
        XCTAssertTrue(doc.traces.isEmpty)
    }

    /// Text: a press adds a text, tells the owner, and the drag moves it.
    func testTextToolPlacesAText() throws {
        let doc = TestSupport.newDocument()
        let controller = PCBCanvasController()
        var placed = 0
        press(controller, canvas(doc, .text, onPlaced: { placed += 1 }), at: p(10.16, 10.16),
              by: CGSize(width: 5.08, height: 0))
        XCTAssertEqual(placed, 1)
        let text = try XCTUnwrap(doc.texts.first)
        XCTAssertEqual(text.anchor.x, 15.24, accuracy: 0.7, "moved by the drag")
    }

    // MARK: Select

    /// A press on a footprint selects and moves it; on empty board it pans
    /// and clears the selection; a Shift-press there draws a box.
    func testSelectMovesPansAndBoxes() throws {
        let (doc, one, _, _) = try pair()
        let controller = PCBCanvasController()
        let viewport = CanvasViewport()
        let view = canvas(doc, viewport: viewport)
        let bounds = try XCTUnwrap(one.boardBounds)
        let before = try XCTUnwrap(one.board?.centre)
        press(controller, view, at: p(bounds.midX, bounds.midY), by: CGSize(width: 2.54, height: 0))
        let after = try XCTUnwrap(doc.chips.first { $0.id == one.id }?.board?.centre)
        XCTAssertEqual(after.x - before.x, 2.54, accuracy: 1e-6)
        XCTAssertEqual(doc.boardSelection.items, [.footprint(one.id)])

        press(controller, view, at: p(5, 75), by: CGSize(width: 1, height: 2))
        XCTAssertTrue(doc.boardSelection.items.isEmpty)
        XCTAssertEqual(viewport.offset.width, ppm, accuracy: 1e-9)
        XCTAssertEqual(viewport.offset.height, 2 * ppm, accuracy: 1e-9)

        // A Shift box from the board's corner over both parts.
        controller.dragChanged(start: screen(p(1, 1)), translation: .zero, in: view, modifiers: [.shift])
        controller.dragChanged(start: screen(p(1, 1)), translation: moved(90, 70), in: view, modifiers: [.shift])
        guard case let .boxSelect(_, to)? = controller.interaction else {
            return XCTFail("\(String(describing: controller.interaction))")
        }
        XCTAssertEqual(to, screen(p(91, 71)))
        controller.dragEnded(translation: moved(90, 70), in: view)
        XCTAssertEqual(doc.boardSelection.items.filter { if case .footprint = $0 { true } else { false } }.count, 2)
    }

    /// A plain drag on a lone trace slides the segment; ⌘ moves it whole.
    func testSelectOnATraceEditsOrMovesIt() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addTrace(from: p(5.08, 5.08), to: p(15.24, 5.08), layer: .top, diagonalFirst: false))
        let controller = PCBCanvasController()
        let view = canvas(doc)
        controller.dragChanged(start: screen(p(10.16, 5.08)), translation: .zero, in: view, modifiers: [])
        guard case .editTrace? = controller.interaction else {
            return XCTFail("\(String(describing: controller.interaction))")
        }
        controller.dragChanged(start: screen(p(10.16, 5.08)), translation: moved(0, 2.54), in: view, modifiers: [])
        controller.dragEnded(translation: moved(0, 2.54), in: view)
        let slid = try XCTUnwrap(doc.traces.first { $0.id == id })
        XCTAssertTrue(slid.points.contains { abs($0.y - 7.62) < 1e-6 }, "\(slid.points)")

        let y = try XCTUnwrap(slid.points.first { abs($0.y - 7.62) < 1e-6 }).y
        controller.dragChanged(start: screen(p(10.16, y)), translation: .zero, in: view, modifiers: [.command])
        guard case .moveGroup? = controller.interaction else {
            return XCTFail("\(String(describing: controller.interaction))")
        }
        controller.dragEnded(translation: .zero, in: view)
    }

    /// ⌘-press on a reference moves only the reference.
    func testCommandPressOnAReferenceMovesIt() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 200, height: 100)
        let part = try TestSupport.place(doc, "SN74HC00N", at: p(100, 100))
        doc.moveFootprint(part.id, to: p(30, 40))
        let chip = try XCTUnwrap(doc.chips.first { $0.id == part.id })
        let centre = try XCTUnwrap(chip.boardReferenceCentre)
        let controller = PCBCanvasController()
        press(controller, canvas(doc), at: centre, by: CGSize(width: 0, height: 5.08), modifiers: [.command])
        let moved = try XCTUnwrap(doc.chips.first { $0.id == part.id })
        XCTAssertEqual(try XCTUnwrap(moved.boardReferenceCentre).y, centre.y + 5.08, accuracy: 0.7)
        XCTAssertEqual(moved.board?.centre, chip.board?.centre, "the part stays")
    }

    /// A press on a selected pour's handle drags that handle.
    func testPressOnAPourHandleResizesIt() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 100, height: 80)
        let id = try XCTUnwrap(doc.addPour(rect: CGRect(x: 10, y: 10, width: 20, height: 20), layer: .top))
        doc.select(.pour(id))
        let rect = try XCTUnwrap(doc.resizableRect)
        let controller = PCBCanvasController()
        let corner = p(rect.maxX, rect.maxY)
        controller.dragChanged(start: screen(corner), translation: .zero, in: canvas(doc), modifiers: [])
        guard case .resize? = controller.interaction else {
            return XCTFail("\(String(describing: controller.interaction))")
        }
        controller.dragChanged(start: screen(corner), translation: moved(10, 10), in: canvas(doc), modifiers: [])
        controller.dragEnded(translation: moved(10, 10), in: canvas(doc))
        let pour = try XCTUnwrap(doc.pours.first)
        XCTAssertGreaterThan(pour.rect.width, rect.width + 5)
    }

    // MARK: Scroll, pinch, hover

    /// A scroll mid-press: a pan's start moves with it; any other press
    /// moves its start and works the drag out again.
    func testScrollingMidPress() throws {
        let doc = TestSupport.newDocument()
        let controller = PCBCanvasController()
        let view = canvas(doc)
        controller.scrolledMidPress(by: CGSize(width: 5, height: 5), in: view)
        XCTAssertNil(controller.interaction, "no press: nothing")

        controller.dragChanged(start: screen(p(5, 75)), translation: .zero, in: view, modifiers: [])
        controller.scrolledMidPress(by: CGSize(width: 10, height: 4), in: view)
        XCTAssertEqual(controller.interaction, .pan(startOffset: CGSize(width: 10, height: 4)))
        controller.dragEnded(translation: .zero, in: view)

        let text = doc.addText(at: p(10.16, 10.16))
        doc.select(.text(text))
        let tool = canvas(doc, .pour)
        controller.dragChanged(start: screen(p(10, 10)), translation: moved(10, 10), in: tool, modifiers: [])
        controller.scrolledMidPress(by: moved(2, 0), in: tool)
        XCTAssertEqual(controller.pressShift, moved(2, 0))
        XCTAssertEqual(controller.pressLocation, screen(p(12, 10)))
        XCTAssertEqual(controller.shifted(moved(10, 10)), moved(8, 10))
        guard case let .drawRect(_, _, to)? = controller.interaction else { return XCTFail() }
        XCTAssertEqual(to.x, 20, accuracy: 1e-9, "the pointer stayed where it was")
        controller.dragEnded(translation: moved(10, 10), in: tool)
        XCTAssertEqual(controller.pressShift, .zero)
        XCTAssertNil(controller.rawPress)
    }

    /// A pinch zooms from the zoom at its start; its end forgets it.
    func testPinchZoomsFromItsStart() {
        let controller = PCBCanvasController()
        let viewport = CanvasViewport()
        controller.pinchChanged(2, at: .zero, viewport: viewport)
        controller.pinchChanged(3, at: .zero, viewport: viewport)
        XCTAssertEqual(viewport.zoom, 3, accuracy: 1e-9)
        XCTAssertEqual(controller.pinchStartZoom, 1)
        controller.pinchEnded()
        XCTAssertNil(controller.pinchStartZoom)
    }

    /// Hovering sets the hover and the pointer in board mm; leaving clears them.
    func testHoverTracksThePointer() {
        let doc = TestSupport.newDocument()
        let pointer = BoardPointer()
        let controller = PCBCanvasController()
        let view = canvas(doc, pointer: pointer)
        controller.hovered(at: screen(p(3, 4)), in: view)
        XCTAssertEqual(controller.hover.point, p(3, 4))
        XCTAssertEqual(controller.hover.location, screen(p(3, 4)))
        XCTAssertEqual(pointer.point, p(3, 4))
        controller.hovered(at: nil, in: view)
        XCTAssertNil(controller.hover.point)
        XCTAssertNil(pointer.point)
        XCTAssertEqual(PCBCanvasView.location(of: .active(p(1, 2))), p(1, 2))
        XCTAssertNil(PCBCanvasView.location(of: .ended))
    }

    // MARK: Drawing keys

    private func key(_ characters: String, code: UInt16, modifiers: NSEvent.ModifierFlags = [],
                     repeats: Bool = false) throws -> NSEvent {
        try XCTUnwrap(NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: modifiers, timestamp: 0,
                                       windowNumber: 0, context: nil, characters: characters,
                                       charactersIgnoringModifiers: characters, isARepeat: repeats, keyCode: code))
    }

    /// With no trace drawn: Esc clears a highlighted net, else no key is used.
    func testKeysWithNoTraceDrawn() throws {
        let doc = TestSupport.newDocument()
        var net: PlaneNet? = .ground
        let binding = Binding(get: { net }, set: { net = $0 })
        let controller = PCBCanvasController()
        let view = canvas(doc, highlighted: binding)
        XCTAssertFalse(controller.handleDrawingKey(try key("a", code: 0), in: view))
        XCTAssertTrue(controller.handleDrawingKey(try key("\u{1b}", code: 53), in: view))
        XCTAssertNil(net)
        XCTAssertFalse(controller.handleDrawingKey(try key("\u{1b}", code: 53), in: view), "nothing to clear")
    }

    /// Space locks a corner (not on a repeat), `/` flips the bend, Esc drops
    /// a lock and then the trace; other keys are left alone.
    func testKeysWhileDrawing() throws {
        let doc = TestSupport.newDocument()
        let controller = PCBCanvasController()
        let view = canvas(doc, .trace)
        controller.dragChanged(start: screen(p(2.54, 2.54)), translation: moved(5.08, 2.54), in: view, modifiers: [])
        func draft() throws -> TraceDraft {
            guard case let .drawTrace(draft, _)? = controller.interaction else { throw XCTSkip("no draft") }
            return draft
        }
        XCTAssertTrue(controller.handleDrawingKey(try key(" ", code: 49, repeats: true), in: view))
        XCTAssertEqual(try draft(), TraceDraft(start: p(2.54, 2.54)), "a repeat locks nothing")
        XCTAssertTrue(controller.handleDrawingKey(try key(" ", code: 49), in: view))
        XCTAssertNotEqual(try draft(), TraceDraft(start: p(2.54, 2.54)))
        let locked = try draft()
        XCTAssertTrue(controller.handleDrawingKey(try key("/", code: 44), in: view))
        XCTAssertNotEqual(try draft(), locked, "flipped")
        XCTAssertFalse(controller.handleDrawingKey(try key("/", code: 44, modifiers: [.command]), in: view))
        XCTAssertFalse(controller.handleDrawingKey(try key("q", code: 12), in: view))
        XCTAssertTrue(controller.handleDrawingKey(try key("\u{1b}", code: 53), in: view))
        XCTAssertEqual(try draft().legStart, p(2.54, 2.54), "the lock dropped")
        XCTAssertTrue(controller.handleDrawingKey(try key("\u{1b}", code: 53), in: view))
        XCTAssertEqual(controller.interaction, .idle, "nothing locked: the trace dropped")
        controller.dragEnded(translation: .zero, in: view)
        XCTAssertTrue(doc.traces.isEmpty)
    }

    /// Space with the assist locks the assist's leg.
    func testSpaceWithTheAssistLocksItsLeg() throws {
        let doc = TestSupport.newDocument()
        let controller = PCBCanvasController()
        let view = canvas(doc, .trace, traceAssist: true)
        controller.dragChanged(start: screen(p(2.54, 2.54)), translation: moved(5.08, 0), in: view, modifiers: [])
        XCTAssertTrue(controller.handleDrawingKey(try key(" ", code: 49), in: view))
        guard case let .drawTrace(draft, _)? = controller.interaction else { return XCTFail() }
        XCTAssertTrue(TraceRoute.same(draft.legStart, p(7.62, 2.54)), "\(draft)")
    }

    /// V drops a via where the trace has got to and goes on on the next
    /// layer, telling the owner; on a 1-layer board it does nothing.
    func testVDropsAViaAndChangesLayer() throws {
        let doc = TestSupport.newDocument()
        var layers: [CopperLayer] = []
        let controller = PCBCanvasController()
        let view = canvas(doc, .trace, traceAssist: true, onLayerChange: { layers.append($0) })
        controller.dragChanged(start: screen(p(2.54, 2.54)), translation: moved(5.08, 0), in: view, modifiers: [])
        XCTAssertTrue(controller.handleDrawingKey(try key("v", code: 9, repeats: true), in: view))
        XCTAssertTrue(doc.vias.isEmpty, "a repeat drops nothing")
        XCTAssertFalse(controller.handleDrawingKey(try key("V", code: 9, modifiers: [.shift]), in: view))
        XCTAssertTrue(controller.handleDrawingKey(try key("v", code: 9), in: view))
        XCTAssertEqual(doc.vias.count, 1)
        XCTAssertEqual(layers, [.bottom])
        XCTAssertEqual(controller.drawingLayer, .bottom)
        XCTAssertNotNil(controller.assist, "the assist goes on, on the new layer")
        controller.dragEnded(translation: moved(5.08, 0), in: view)

        let flat = TestSupport.newDocument()
        flat.setLayerCount(1)
        let single = canvas(flat, .trace, onLayerChange: { layers.append($0) })
        controller.dragChanged(start: screen(p(2.54, 2.54)), translation: moved(5.08, 0), in: single, modifiers: [])
        XCTAssertTrue(controller.handleDrawingKey(try key("v", code: 9), in: single))
        XCTAssertTrue(flat.vias.isEmpty)
        XCTAssertEqual(layers, [.bottom])
        controller.dragEnded(translation: .zero, in: single)
    }

    /// The key monitor is installed once and taken off again.
    func testKeyMonitorInstallsOnce() throws {
        let doc = TestSupport.newDocument()
        let monitors = EditorEventMonitors()
        let controller = PCBCanvasController()
        controller.installDrawingKeyMonitor(monitors, in: canvas(doc))
        controller.installDrawingKeyMonitor(monitors, in: canvas(doc))
        XCTAssertEqual(monitors.count, 1)
        let a = try XCTUnwrap(NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: [], timestamp: 0, windowNumber: 0,
                                               context: nil, characters: "a", charactersIgnoringModifiers: "a",
                                               isARepeat: false, keyCode: 0))
        XCTAssertTrue(controller.monitored(a, in: canvas(doc)) === a, "a key no drawing uses goes on")
        var net: PlaneNet? = .ground
        let esc = try XCTUnwrap(NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: [], timestamp: 0, windowNumber: 0,
                                                 context: nil, characters: "\u{1b}", charactersIgnoringModifiers: "\u{1b}",
                                                 isARepeat: false, keyCode: 53))
        XCTAssertNil(controller.monitored(esc, in: canvas(doc, highlighted: Binding(get: { net }, set: { net = $0 }))),
                     "Esc clearing the highlight is used up")
        XCTAssertNil(net)
        controller.removeDrawingKeyMonitor(monitors)
        XCTAssertEqual(monitors.count, 0)
        XCTAssertNil(controller.drawingKeyMonitor)
    }
}
