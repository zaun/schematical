import AppKit
import CoreGraphics
import XCTest
@testable import schematical

/// plan-68-AJ: the Schematic canvas's gestures, hover, drawing keys,
/// double-click and drop, through `SchematicCanvasController` as
/// `SchematicCanvasView` calls it. The viewport is at zoom 1 and no offset,
/// so screen points are canvas points.
@MainActor
final class SchematicCanvasControllerTests: XCTestCase {
    private static var keptSimulations: [SimulationController] = []
    private var windows: [NSWindow] = []

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        for simulation in Self.keptSimulations where simulation.isRunning { simulation.stop() }
        windows.forEach { $0.close() }
        windows = []
    }

    private func simulation() -> SimulationController {
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        return simulation
    }

    /// Callbacks a context reports, by name.
    private final class Calls {
        var names: [String] = []
        nonisolated deinit {}
    }

    private func context(_ doc: SchematicDocument, _ tool: SchematicTool = .pointer, viewport: CanvasViewport? = nil,
                         simulation: SimulationController? = nil, calls: Calls? = nil, supply: String = SupplyChoice.defaultName,
                         pointer: BoardPointer? = nil) -> SchematicCanvasContext {
        let viewport = viewport ?? CanvasViewport()
        viewport.updateSize(CGSize(width: 1200, height: 900))
        let calls = calls ?? Calls()
        return SchematicCanvasContext(
            catalog: TestSupport.catalog, document: doc, viewport: viewport, tool: tool, supplyVoltage: supply,
            simulation: simulation ?? self.simulation(),
            onOpenMemory: { _ in calls.names.append("memory") }, onOpenCpu: { _ in calls.names.append("cpu") },
            onOpenTerminal: { _ in calls.names.append("terminal") },
            onOpenRegisters: { _ in calls.names.append("registers") },
            onOpenDocumentInfo: { calls.names.append("info") }, pointer: pointer,
            onPartPlaced: { calls.names.append("placed") })
    }

    /// A press at `from`, moved to `to` in `steps`, and released there.
    private func drag(_ controller: SchematicCanvasController, _ context: SchematicCanvasContext, from: CGPoint,
                      to: CGPoint, shift: Bool = false, steps: Int = 3) {
        controller.pressChanged(DragPress(from: from, to: from), shift: shift, in: context)
        for step in 1...steps {
            let t = CGFloat(step) / CGFloat(steps)
            controller.pressChanged(DragPress(from: from, to: CGPoint(x: from.x + (to.x - from.x) * t,
                                                                     y: from.y + (to.y - from.y) * t)),
                                    shift: shift, in: context)
        }
        controller.pressEnded(DragPress(from: from, to: to), in: context)
    }

    private func click(_ controller: SchematicCanvasController, _ context: SchematicCanvasContext, at point: CGPoint,
                       shift: Bool = false) {
        drag(controller, context, from: point, to: point, shift: shift, steps: 1)
    }

    private func centre(_ chip: PlacedChip) -> CGPoint {
        CGPoint(x: chip.origin.x + chip.layout.size.width / 2, y: chip.origin.y + chip.layout.size.height / 2)
    }

    private func chip(_ doc: SchematicDocument, _ id: UUID) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.id == id })
    }

    private func key(_ code: UInt16, _ characters: String, modifiers: NSEvent.ModifierFlags = [],
                     isRepeat: Bool = false) -> NSEvent {
        NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: modifiers, timestamp: 0, windowNumber: 0,
                         context: nil, characters: characters, charactersIgnoringModifiers: characters,
                         isARepeat: isRepeat, keyCode: code)!
    }

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    // MARK: Pointer

    func testAPartDragsAsOneUndoStepAndAClickSelectsIt() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        let controller = SchematicCanvasController()
        let context = context(doc)
        click(controller, context, at: centre(led))
        XCTAssertEqual(doc.schematicSelection.items, [.chip(led.id)])
        XCTAssertNil(controller.interaction, "a release ends the press")

        drag(controller, context, from: centre(led), to: p(centre(led).x + 40, centre(led).y + 20))
        XCTAssertEqual(try chip(doc, led.id).origin, p(led.origin.x + 40, led.origin.y + 20))
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(try chip(doc, led.id).origin, led.origin, "the whole drag is one undo step")
    }

    func testAnEmptyPressPansAndClearsTheSelection() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        doc.select(.chip(led.id))
        let controller = SchematicCanvasController()
        let context = context(doc)
        drag(controller, context, from: p(600, 600), to: p(650, 630))
        XCTAssertEqual(context.viewport.offset, CGSize(width: 50, height: 30))
        XCTAssertTrue(doc.schematicSelection.isEmpty)
    }

    func testAShiftDragOnEmptySpaceBoxSelects() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        let controller = SchematicCanvasController()
        let context = context(doc)
        controller.pressChanged(DragPress(from: p(20, 20), to: p(20, 20)), shift: true, in: context)
        controller.pressChanged(DragPress(from: p(20, 20), to: p(500, 500)), shift: true, in: context)
        let box = try XCTUnwrap(controller.selectionBox)
        XCTAssertEqual(box.from, p(20, 20))
        XCTAssertEqual(box.to, p(500, 500))
        XCTAssertTrue(doc.schematicSelection.contains(.chip(led.id)))
        controller.pressEnded(DragPress(from: p(20, 20), to: p(500, 500)), in: context)
        XCTAssertNil(controller.selectionBox)
        XCTAssertTrue(doc.schematicSelection.contains(.chip(led.id)))
    }

    func testLabelsNotesTheTitleBlockAndJunctionsMoveWithAPress() throws {
        let doc = TestSupport.newDocument()
        let controller = SchematicCanvasController()
        let context = context(doc)
        let label = doc.addNetLabel(at: p(400, 100))
        let labelPoint = try XCTUnwrap(doc.netLabels.first { $0.id == label }).origin.applying(CGAffineTransform(translationX: 20, y: 0))
        drag(controller, context, from: labelPoint, to: p(labelPoint.x + 20, labelPoint.y))
        XCTAssertEqual(try XCTUnwrap(doc.netLabels.first { $0.id == label }).origin.x, labelPoint.x)
        XCTAssertEqual(doc.schematicSelection.items, [.label(label)])

        let note = doc.addNote(at: p(600, 400))
        let origin = try XCTUnwrap(doc.notes.first { $0.id == note }).origin
        drag(controller, context, from: p(origin.x + 5, origin.y + 5), to: p(origin.x + 25, origin.y + 45))
        XCTAssertEqual(try XCTUnwrap(doc.notes.first { $0.id == note }).origin, p(origin.x + 20, origin.y + 40))

        doc.setTitleBlockShown(true)
        let rect = try XCTUnwrap(doc.titleBlockRect)
        click(controller, context, at: p(rect.midX, rect.midY))
        XCTAssertEqual(doc.schematicSelection.items, [.titleBlock])
    }

    // MARK: Wires

    /// Two LEDs, the first's K wired to the second's A with a bend.
    private func wired(_ doc: SchematicDocument) throws -> (wire: UUID, path: [CGPoint]) {
        let a = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: p(500, 400))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(a, "K")), .pin(try TestSupport.pin(b, "A"))))
        let wire = try XCTUnwrap(doc.wires.first)
        return (wire.id, doc.path(forWire: wire.id))
    }

    func testTheLineToolDrawsAWireFromPinToPin() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: p(500, 400))
        let from = try XCTUnwrap(doc.pinTip(try TestSupport.pin(a, "K")))
        let to = try XCTUnwrap(doc.pinTip(try TestSupport.pin(b, "A")))
        let controller = SchematicCanvasController()
        let context = context(doc, .line)

        // Hovering shows the connection point under the pointer.
        controller.hover(from, in: context)
        XCTAssertEqual(controller.highlightPoint(in: context), from)

        controller.pressChanged(DragPress(from: from, to: from), shift: false, in: context)
        controller.pressChanged(DragPress(from: from, to: to), shift: false, in: context)
        let preview = try XCTUnwrap(controller.connectionPreview(in: context))
        XCTAssertEqual(preview.target, to)
        XCTAssertEqual(preview.points.first, from)
        XCTAssertEqual(controller.highlightPoint(in: context), to, "the pin it would land on")
        XCTAssertNil(controller.busPreview(in: context))
        controller.pressEnded(DragPress(from: from, to: to), in: context)
        XCTAssertEqual(doc.wires.count, 1)

        // Ending on nothing places nothing; a press off any anchor does nothing.
        drag(controller, context, from: from, to: p(900, 800))
        XCTAssertEqual(doc.wires.count, 1)
        drag(controller, context, from: p(900, 100), to: p(950, 100))
        XCTAssertEqual(doc.wires.count, 1)
        XCTAssertNil(controller.highlightPoint(in: self.context(doc, .pointer)), "only the Line tool highlights")
    }

    func testDrawingKeysLockFlipAndDropTheLiveLeg() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        let from = try XCTUnwrap(doc.pinTip(try TestSupport.pin(a, "K")))
        let controller = SchematicCanvasController()
        let context = context(doc, .line)
        XCTAssertFalse(controller.handleDrawingKey(key(49, " "), in: context), "nothing is being drawn")

        controller.pressChanged(DragPress(from: from, to: from), shift: false, in: context)
        controller.pressChanged(DragPress(from: from, to: p(from.x + 100, from.y + 100)), shift: false, in: context)
        let before = try XCTUnwrap(controller.connectionPreview(in: context)).points
        XCTAssertFalse(controller.handleDrawingKey(key(0, "a"), in: context), "another key is left alone")
        XCTAssertFalse(controller.handleDrawingKey(key(44, "/", modifiers: .command), in: context), "⌘/ is left alone")
        XCTAssertTrue(controller.handleDrawingKey(key(44, "/"), in: context))
        let flipped = try XCTUnwrap(controller.connectionPreview(in: context)).points
        XCTAssertNotEqual(flipped, before, "/ flips the L")
        XCTAssertTrue(controller.handleDrawingKey(key(49, " ", isRepeat: true), in: context))
        XCTAssertEqual(try XCTUnwrap(controller.connectionPreview(in: context)).points, flipped, "a repeat locks nothing")
        XCTAssertNil(controller.monitoredKey(key(49, " "), in: context), "the monitor swallows a used key")
        XCTAssertNotNil(controller.monitoredKey(key(0, "a"), in: context))
        // Esc drops the lock, then the whole drawing.
        XCTAssertTrue(controller.handleDrawingKey(key(53, "\u{1b}"), in: context))
        XCTAssertNotNil(controller.connectionPreview(in: context))
        XCTAssertTrue(controller.handleDrawingKey(key(53, "\u{1b}"), in: context))
        XCTAssertNil(controller.connectionPreview(in: context))
        controller.pressEnded(DragPress(from: from, to: p(from.x + 100, from.y + 100)), in: context)
        XCTAssertTrue(doc.wires.isEmpty)
    }

    func testAWireBendAndSegmentDragAndADoubleClickAddsAJunction() throws {
        let doc = TestSupport.newDocument()
        let (wire, path) = try wired(doc)
        XCTAssertGreaterThanOrEqual(path.count, 3, "\(path)")
        let controller = SchematicCanvasController()
        let context = context(doc)
        // A segment's middle: dragging shifts it.
        let mid = p((path[0].x + path[1].x) / 2, (path[0].y + path[1].y) / 2)
        drag(controller, context, from: mid, to: p(mid.x + 20, mid.y + 20))
        XCTAssertNotEqual(doc.path(forWire: wire), path)
        XCTAssertEqual(doc.schematicSelection.items, [.wire(wire)])

        // A bend: dragging moves it.
        let bent = doc.path(forWire: wire)
        let node = bent[1]
        drag(controller, context, from: node, to: p(node.x + 30, node.y + 30))
        XCTAssertNotEqual(doc.path(forWire: wire), bent)

        // A double-click on the wire adds a bend there.
        let now = doc.path(forWire: wire)
        let on = SchematicGrid.snap(p((now[0].x + now[1].x) / 2, (now[0].y + now[1].y) / 2))
        controller.doubleClick(at: on, in: context)
        XCTAssertEqual(doc.path(forWire: wire).count, now.count + 1)

        // A wire from a third LED to a grid point on it makes a junction; the junction drags.
        let third = try TestSupport.place(doc, "GENERIC-LED", at: p(800, 200))
        let start = doc.path(forWire: wire)[0]
        let onWire = SchematicGrid.snap(p((start.x + on.x) / 2, (start.y + on.y) / 2))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(third, "A")), .point(onWire)))
        let junction = try XCTUnwrap(doc.junctions.first)
        drag(controller, context, from: junction.point, to: p(junction.point.x, junction.point.y + 40))
        XCTAssertNotEqual(try XCTUnwrap(doc.junctions.first).point, junction.point)

        // With the wire in a larger selection, a drag on it moves the group.
        let led = try XCTUnwrap(doc.chips.first)
        doc.select(.chip(led.id))
        let segment = doc.path(forWire: wire)
        let at = p((segment[0].x + segment[1].x) / 2, (segment[0].y + segment[1].y) / 2)
        drag(controller, context, from: at, to: p(at.x, at.y + 20), shift: true)
        XCTAssertEqual(try chip(doc, led.id).origin.y, led.origin.y + 20)
        let bend = doc.path(forWire: wire)[1]
        drag(controller, context, from: bend, to: p(bend.x, bend.y + 20))
        XCTAssertEqual(try chip(doc, led.id).origin.y, led.origin.y + 40, "a bend in a larger selection moves it too")
    }

    func testTheEraserErasesOnAClickOnly() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        let controller = SchematicCanvasController()
        let context = context(doc, .eraser)
        drag(controller, context, from: centre(led), to: p(centre(led).x + 50, centre(led).y))
        XCTAssertEqual(doc.chips.count, 1, "a drag erases nothing")
        click(controller, context, at: centre(led))
        XCTAssertTrue(doc.chips.isEmpty)
    }

    // MARK: Buses

    func testTheBusToolDrawsAndItsKeysWork() throws {
        let doc = TestSupport.newDocument()
        let controller = SchematicCanvasController()
        let context = context(doc, .bus)
        controller.pressChanged(DragPress(from: p(100, 500), to: p(100, 500)), shift: false, in: context)
        XCTAssertNil(controller.busPreview(in: context), "no line yet")
        controller.pressChanged(DragPress(from: p(100, 500), to: p(300, 600)), shift: false, in: context)
        let preview = try XCTUnwrap(controller.busPreview(in: context))
        XCTAssertFalse(preview.refused)
        XCTAssertNil(controller.connectionPreview(in: context))
        XCTAssertTrue(controller.handleDrawingKey(key(44, "/"), in: context))
        XCTAssertTrue(controller.handleDrawingKey(key(49, " ", isRepeat: true), in: context))
        XCTAssertTrue(controller.handleDrawingKey(key(49, " "), in: context))
        XCTAssertTrue(controller.handleDrawingKey(key(53, "\u{1b}"), in: context), "drops the lock")
        XCTAssertNotNil(controller.busPreview(in: context))
        controller.pressEnded(DragPress(from: p(100, 500), to: p(300, 600)), in: context)
        XCTAssertEqual(doc.buses.count, 1)
        XCTAssertEqual(try XCTUnwrap(doc.buses.first).lines.count, 2, "an L")

        // Esc with nothing locked drops the whole bus.
        controller.pressChanged(DragPress(from: p(100, 800), to: p(100, 800)), shift: false, in: context)
        controller.pressChanged(DragPress(from: p(100, 800), to: p(400, 800)), shift: false, in: context)
        XCTAssertTrue(controller.handleDrawingKey(key(53, "\u{1b}"), in: context))
        XCTAssertNil(controller.busPreview(in: context))
        controller.pressEnded(DragPress(from: p(100, 800), to: p(400, 800)), in: context)
        XCTAssertEqual(doc.buses.count, 1)
    }

    func testABusRefusedDrawShowsRefusedAndASpaceLocksTheAcceptedLeg() throws {
        let doc = TestSupport.newDocument()
        XCTAssertNotNil(doc.addBus(from: p(100, 100), to: p(500, 100)))
        let controller = SchematicCanvasController()
        let context = context(doc, .bus)
        // Along the bus from its middle: refused.
        controller.pressChanged(DragPress(from: p(200, 100), to: p(200, 100)), shift: false, in: context)
        controller.pressChanged(DragPress(from: p(200, 100), to: p(400, 100)), shift: false, in: context)
        XCTAssertEqual(controller.busPreview(in: context)?.refused, true)
        controller.pressEnded(DragPress(from: p(200, 100), to: p(400, 100)), in: context)
        XCTAssertEqual(doc.buses.count, 1)
        XCTAssertEqual(try XCTUnwrap(doc.buses.first).lines.count, 1, "the refused drag changed nothing")

        // From the bus's end, an L whose first leg runs back along the bus is
        // refused; Space locks the flipped L the preview shows.
        controller.pressChanged(DragPress(from: p(500, 100), to: p(500, 100)), shift: false, in: context)
        controller.pressChanged(DragPress(from: p(500, 100), to: p(300, 300)), shift: false, in: context)
        let shown = try XCTUnwrap(controller.busPreview(in: context))
        XCTAssertTrue(controller.handleDrawingKey(key(49, " "), in: context))
        controller.pressEnded(DragPress(from: p(500, 100), to: p(300, 300)), in: context)
        XCTAssertFalse(shown.refused, "\(shown.points)")
        XCTAssertGreaterThan(try XCTUnwrap(doc.buses.first).lines.count, 1)
    }

    func testABusEndALineAndAPortDrag() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(500, 100)))
        let controller = SchematicCanvasController()
        let context = context(doc)
        // The end: the line grows.
        drag(controller, context, from: p(500, 100), to: p(600, 100))
        XCTAssertEqual(try XCTUnwrap(doc.bus(id)).lines.first?.b.x ?? 0, 600, accuracy: 0.5)
        // The line: it moves.
        drag(controller, context, from: p(300, 100), to: p(300, 160))
        XCTAssertEqual(try XCTUnwrap(doc.bus(id)).lines.first?.a.y ?? 0, 160, accuracy: 0.5)

        // The port tool adds a port on a line; the pointer slides it.
        click(controller, self.context(doc, .busPort), at: p(300, 160))
        let port = try XCTUnwrap(doc.bus(id)?.ports.first)
        let body = try XCTUnwrap(doc.bus(id)?.body(of: port))
        let at = p(body.midX, body.midY)
        controller.hover(at, in: context)
        XCTAssertEqual(controller.hoveredPortID, port.id, "\(port)")
        drag(controller, context, from: at, to: p(at.x + 40, at.y))
        XCTAssertEqual(doc.selectedPortID, port.id)
        XCTAssertEqual(try XCTUnwrap(doc.bus(id)?.ports.first).start, port.start + 40, accuracy: 0.5)

        // In a larger selection, a bus end or line moves the group.
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(700, 400))
        doc.select(.chip(led.id))
        let moved = try XCTUnwrap(doc.bus(id)?.lines.first)
        drag(controller, context, from: moved.a, to: p(moved.a.x, moved.a.y + 20), shift: true)
        XCTAssertEqual(try chip(doc, led.id).origin.y, led.origin.y + 20)
        let mid = p((moved.a.x + moved.b.x) / 2 + 100, moved.a.y + 20)
        drag(controller, context, from: mid, to: p(mid.x, mid.y + 20))
        XCTAssertEqual(try chip(doc, led.id).origin.y, led.origin.y + 40)
    }

    // MARK: Placing tools

    func testTextAndNetLabelToolsAddOrPick() throws {
        let doc = TestSupport.newDocument()
        let calls = Calls()
        let controller = SchematicCanvasController()
        let text = context(doc, .text, calls: calls)
        click(controller, text, at: p(300, 300))
        let note = try XCTUnwrap(doc.notes.first)
        XCTAssertEqual(calls.names, ["placed"])
        doc.editingNoteID = nil
        doc.select(.titleBlock)
        click(controller, text, at: p(note.origin.x + 5, note.origin.y + 5))
        XCTAssertEqual(doc.notes.count, 1, "a press on a block picks it")
        XCTAssertEqual(doc.editingNoteID, note.id)
        XCTAssertEqual(doc.schematicSelection.items, [.note(note.id)])

        let label = context(doc, .netLabel, calls: calls)
        click(controller, label, at: p(700, 200))
        let added = try XCTUnwrap(doc.netLabels.first)
        click(controller, label, at: CGPoint(x: added.origin.x + 20, y: added.origin.y))
        XCTAssertEqual(doc.netLabels.count, 1)
        XCTAssertEqual(doc.schematicSelection.items, [.label(added.id)])
        XCTAssertEqual(calls.names.count, 4)
    }

    func testTheSupplyToolsPlaceTheirSupply() throws {
        let doc = TestSupport.newDocument()
        let controller = SchematicCanvasController()
        for (n, tool) in [SchematicTool.vcc, .ground, .pullUp, .pullDown].enumerated() {
            click(controller, context(doc, tool), at: p(200 + CGFloat(n) * 150, 300))
            XCTAssertEqual(doc.chips.count, n + 1, "\(tool)")
            XCTAssertEqual(doc.chips.last?.partNumber, tool.supplyName(voltage: SupplyChoice.defaultName), "\(tool)")
        }
    }

    // MARK: Hover

    func testHoverFindsWhatIsUnderThePointer() throws {
        let doc = TestSupport.newDocument()
        let (wire, path) = try wired(doc)
        let led = try XCTUnwrap(doc.chips.first)
        let label = doc.addNetLabel(at: p(900, 100))
        let labelPoint = try XCTUnwrap(doc.netLabels.first { $0.id == label }).origin.applying(CGAffineTransform(translationX: 20, y: 0))
        let bus = try XCTUnwrap(doc.addBus(from: p(100, 700), to: p(500, 700)))
        let note = doc.addNote(at: p(900, 500))
        let noteOrigin = try XCTUnwrap(doc.notes.first { $0.id == note }).origin
        doc.setTitleBlockShown(true)
        let block = try XCTUnwrap(doc.titleBlockRect)
        let pointer = BoardPointer()
        let controller = SchematicCanvasController()
        let context = context(doc, pointer: pointer)

        controller.hover(centre(led), in: context)
        XCTAssertEqual(controller.hoveredChipID, led.id)
        XCTAssertEqual(pointer.point, centre(led))
        controller.hover(labelPoint, in: context)
        XCTAssertEqual(controller.hoveredLabelID, label)
        controller.hover(p((path[0].x + path[1].x) / 2, (path[0].y + path[1].y) / 2), in: context)
        XCTAssertEqual(controller.hoveredWireID, wire)
        controller.hover(p(300, 700), in: context)
        XCTAssertEqual(controller.hoveredBusID, bus)
        controller.hover(p(noteOrigin.x + 5, noteOrigin.y + 5), in: context)
        XCTAssertEqual(controller.hoveredNoteID, note)
        controller.hover(p(block.midX, block.midY), in: context)
        XCTAssertTrue(controller.isTitleBlockHovered)
        controller.hover(nil, in: context)
        XCTAssertFalse(controller.isTitleBlockHovered)
        XCTAssertNil(pointer.point)

        // Play mode highlights nothing, and starting it drops a highlight.
        let simulation = simulation()
        let running = self.context(doc, simulation: simulation)
        controller.hover(centre(led), in: running)
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        controller.runningChanged(true, in: running)
        XCTAssertNil(controller.hoveredChipID)
        controller.hover(centre(led), in: running)
        XCTAssertNil(controller.hoveredChipID)
    }

    // MARK: Double-click and drop

    func testADoubleClickOpensAPartsWindowANoteOrDocumentInfo() throws {
        let doc = TestSupport.newDocument()
        let calls = Calls()
        let controller = SchematicCanvasController()
        let context = context(doc, calls: calls)
        let kinds: [(String, (CatalogPart) -> Bool)] = [
            ("memory", \.isMemory), ("cpu", { $0.hasProcessorWindow && !$0.isMemory }),
            ("terminal", { $0.hasTerminal && !$0.hasProcessorWindow && !$0.isMemory }),
            ("registers", { $0.hasRegisters && !$0.hasTerminal && !$0.hasProcessorWindow && !$0.isMemory }),
        ]
        for (n, (name, wanted)) in kinds.enumerated() {
            let part = try XCTUnwrap(TestSupport.allParts.first(where: wanted), name)
            let placed = try TestSupport.place(doc, part.name, at: p(200 + CGFloat(n) * 400, 200))
            controller.doubleClick(at: centre(placed), in: context)
            XCTAssertEqual(calls.names.last, name, part.name)
        }
        XCTAssertEqual(calls.names.count, 4)
        // Another tool does nothing; a plain part opens nothing.
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 700))
        controller.doubleClick(at: centre(doc.chips[0]), in: self.context(doc, .eraser, calls: calls))
        controller.doubleClick(at: centre(led), in: context)
        XCTAssertEqual(calls.names.count, 4)

        let note = doc.addNote(at: p(1000, 700))
        doc.editingNoteID = nil
        let origin = try XCTUnwrap(doc.notes.first { $0.id == note }).origin
        controller.doubleClick(at: p(origin.x + 5, origin.y + 5), in: context)
        XCTAssertEqual(doc.editingNoteID, note)
        doc.setTitleBlockShown(true)
        let block = try XCTUnwrap(doc.titleBlockRect)
        controller.doubleClick(at: p(block.midX, block.midY), in: context)
        XCTAssertEqual(calls.names.last, "info")
        XCTAssertEqual(doc.schematicSelection.items, [.titleBlock])

        // In play mode only the parts' windows open.
        let simulation = simulation()
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        let running = self.context(doc, simulation: simulation, calls: calls)
        controller.doubleClick(at: p(block.midX, block.midY), in: running)
        XCTAssertEqual(calls.names.count, 5)
        controller.doubleClick(at: centre(doc.chips[0]), in: running)
        XCTAssertEqual(calls.names.last, "memory")
    }

    func testADroppedPartIsPlacedUnlessRefused() throws {
        let doc = TestSupport.newDocument()
        let calls = Calls()
        let controller = SchematicCanvasController()
        let context = context(doc, calls: calls)
        XCTAssertTrue(controller.drop(partNumber: "GENERIC-LED", at: p(300, 300), in: context))
        XCTAssertEqual(doc.chips.first?.partNumber, "GENERIC-LED")
        XCTAssertEqual(calls.names, ["placed"])
        XCTAssertFalse(controller.drop(partNumber: "NO-SUCH-PART", at: p(300, 300), in: context))
        XCTAssertFalse(controller.drop(partNumber: nil, at: p(300, 300), in: context))
        let simulation = simulation()
        simulation.start(document: doc)
        XCTAssertFalse(controller.drop(partNumber: "GENERIC-LED", at: p(600, 300),
                                       in: self.context(doc, simulation: simulation)))
        XCTAssertEqual(doc.chips.count, 1)
    }

    // MARK: Pinch, scroll, keys, focus

    func testAPinchZoomsFromWhereItStarted() {
        let doc = TestSupport.newDocument()
        let controller = SchematicCanvasController()
        let context = context(doc)
        controller.pinchChanged(magnification: 2, startLocation: p(100, 100), in: context)
        XCTAssertEqual(controller.pinchStartZoom, 1)
        controller.pinchChanged(magnification: 1.5, startLocation: p(100, 100), in: context)
        XCTAssertEqual(context.viewport.zoom, 1.5, accuracy: 1e-9, "from the zoom at the start")
        controller.pinchEnded()
        XCTAssertNil(controller.pinchStartZoom)
        XCTAssertEqual(context.zoom, 1.5, accuracy: 1e-9)
        XCTAssertEqual(context.gridStep, SchematicGrid.spacing)
        context.viewport.setZoom(0.25)
        XCTAssertEqual(context.gridStep, SchematicGrid.spacing * 5, "sparse dots zoomed out")
        let rect = context.gridRect
        XCTAssertEqual(rect.minX.truncatingRemainder(dividingBy: context.gridStep), 0)
    }

    func testAScrollDuringAPressKeepsItOnTheWorld() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        let controller = SchematicCanvasController()
        let context = context(doc)
        controller.scrolledMidPress(by: CGSize(width: 10, height: 0), in: context)
        XCTAssertEqual(controller.pressShift, .zero, "no press: nothing")

        // A pan: its start moves with the scroll.
        controller.pressChanged(DragPress(from: p(600, 600), to: p(620, 600)), shift: false, in: context)
        context.viewport.pan(to: CGSize(width: 30, height: 0))
        controller.scrolledMidPress(by: CGSize(width: 10, height: 0), in: context)
        XCTAssertEqual(context.viewport.offset, CGSize(width: 30, height: 0))
        controller.pressEnded(DragPress(from: p(600, 600), to: p(620, 600)), in: context)

        // A box: its start moves, and the press is worked out again.
        context.viewport.pan(to: .zero)
        controller.pressChanged(DragPress(from: p(20, 20), to: p(100, 100)), shift: true, in: context)
        controller.scrolledMidPress(by: CGSize(width: 0, height: 5), in: context)
        XCTAssertEqual(controller.selectionBox?.from, p(20, 25))
        XCTAssertEqual(controller.pressShift, CGSize(width: 0, height: 5))
        controller.pressEnded(DragPress(from: p(20, 20), to: p(100, 100)), in: context)
        XCTAssertEqual(controller.pressShift, .zero)

        // A part drag: the scroll moves the part with the world under the pointer.
        let start = centre(led)
        controller.pressChanged(DragPress(from: start, to: start), shift: false, in: context)
        context.viewport.pan(to: CGSize(width: -20, height: 0))
        controller.scrolledMidPress(by: CGSize(width: -20, height: 0), in: context)
        controller.pressEnded(DragPress(from: start, to: start), in: context)
        XCTAssertEqual(try chip(doc, led.id).origin.x, led.origin.x + 20)
    }

    func testTheKeyMonitorIsInstalledOnceAndRemoved() {
        let doc = TestSupport.newDocument()
        let monitors = EditorEventMonitors()
        let controller = SchematicCanvasController()
        controller.installDrawingKeyMonitor(monitors, context: context(doc))
        controller.installDrawingKeyMonitor(monitors, context: context(doc))
        XCTAssertEqual(monitors.count, 1)
        XCTAssertNotNil(controller.drawingKeyMonitor)
        // A key reaching the app passes through it untouched when nothing is drawn.
        let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 100, height: 100), styleMask: [.titled],
                              backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        windows.append(window)
        let event = NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: [], timestamp: 0,
                                     windowNumber: window.windowNumber, context: nil, characters: "a",
                                     charactersIgnoringModifiers: "a", isARepeat: false, keyCode: 0)!
        NSApp.postEvent(event, atStart: true)
        RunLoop.main.run(until: Date().addingTimeInterval(0.2))
        controller.removeDrawingKeyMonitor(monitors)
        XCTAssertEqual(monitors.count, 0)
        XCTAssertNil(controller.drawingKeyMonitor)
    }

    func testAPressEndsATextFieldsEditing() throws {
        let doc = TestSupport.newDocument()
        let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 200, height: 100), styleMask: [.titled],
                              backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        windows.append(window)
        let field = NSTextField(frame: NSRect(x: 10, y: 10, width: 150, height: 24))
        window.contentView?.addSubview(field)
        window.orderFront(nil)
        XCTAssertTrue(window.makeFirstResponder(field))
        XCTAssertTrue(window.firstResponder is NSText, "the field is being edited")
        let controller = SchematicCanvasController()
        let context = context(doc)
        // Mid-press, the field keeps its editing; a new press ends it.
        controller.pressChanged(DragPress(from: p(600, 600), to: p(600, 600)), shift: false, in: context)
        controller.pressChanged(DragPress(from: p(600, 600), to: p(610, 600)), shift: false, keyWindow: window, in: context)
        XCTAssertTrue(window.firstResponder is NSText)
        controller.pressEnded(DragPress(from: p(600, 600), to: p(610, 600)), in: context)
        controller.pressChanged(DragPress(from: p(600, 600), to: p(600, 600)), shift: false, keyWindow: window, in: context)
        controller.pressEnded(DragPress(from: p(600, 600), to: p(600, 600)), in: context)
        XCTAssertFalse(window.firstResponder is NSText)
    }

    func testAToolChangedMidPressDoesNothingMore() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: p(200, 200))
        let from = try XCTUnwrap(doc.pinTip(try TestSupport.pin(a, "K")))
        let controller = SchematicCanvasController()
        controller.pressChanged(DragPress(from: from, to: from), shift: false, in: context(doc, .line))
        let pointer = context(doc, .pointer)
        controller.pressChanged(DragPress(from: from, to: p(from.x + 100, from.y)), shift: false, in: pointer)
        controller.pressEnded(DragPress(from: from, to: p(from.x + 100, from.y)), in: pointer)
        XCTAssertTrue(doc.wires.isEmpty)
        XCTAssertEqual(pointer.viewport.offset, .zero, "no pan either")
        XCTAssertEqual(doc.chips[0].origin, a.origin)
    }

    // MARK: Switches and play mode

    private func firstPart(_ what: String, _ wanted: (CatalogPart) -> Bool) throws -> CatalogPart {
        try XCTUnwrap(TestSupport.allParts.filter(wanted).sorted { $0.name < $1.name }.first, what)
    }

    func testAClickFlipsADipLeverOrMovesASlideKnob() throws {
        let doc = TestSupport.newDocument()
        let dip = try firstPart("DIP") { $0.kind == PART_SWITCH && !$0.isTactileSwitch && $0.slideStates == nil
            && ($0.count ?? 0) >= 2 }
        let chip = try TestSupport.place(doc, dip.name, at: p(200, 200))
        let lever = ChipLayout.switchLeverRect(position: 1, bodyWidth: chip.layout.size.width)
        let point = p(chip.origin.x + lever.midX, chip.origin.y + lever.midY)
        XCTAssertEqual(chip.switchPosition(at: point), 1)
        let controller = SchematicCanvasController()
        let context = context(doc)
        click(controller, context, at: point)
        XCTAssertEqual(try self.chip(doc, chip.id).switchesOn.first, !chip.switchesOn[0])
        // Shift-click selects and flips nothing.
        click(controller, context, at: point, shift: true)
        XCTAssertEqual(try self.chip(doc, chip.id).switchesOn.first, !chip.switchesOn[0])

        let slide = try firstPart("slide") { $0.slideStates != nil }
        let knob = try TestSupport.place(doc, slide.name, at: p(600, 200))
        let next = try XCTUnwrap(knob.nextSlidePosition)
        click(controller, context, at: centre(knob))
        XCTAssertEqual(try self.chip(doc, knob.id).slidePosition, next)
    }

    func testPlayModeHoldsAButtonTogglesAFuseAndPans() throws {
        let doc = TestSupport.newDocument()
        let tactile = try firstPart("tactile") { $0.isTactileSwitch }
        let button = try TestSupport.place(doc, tactile.name, at: p(200, 200))
        let fusePart = try firstPart("fuse") { $0.kind == PART_FUSE }
        let fuse = try TestSupport.place(doc, fusePart.name, at: p(600, 200))
        let dip = try firstPart("DIP") { $0.kind == PART_SWITCH && !$0.isTactileSwitch && $0.slideStates == nil
            && ($0.count ?? 0) >= 2 }
        let switches = try TestSupport.place(doc, dip.name, at: p(200, 500))
        let simulation = simulation()
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        let controller = SchematicCanvasController()
        let context = context(doc, simulation: simulation)

        let rect = try XCTUnwrap(button.buttonRect)
        let on = p(rect.midX, rect.midY)
        controller.pressChanged(DragPress(from: on, to: on), shift: false, in: context)
        XCTAssertTrue(try chip(doc, button.id).isPressed)
        controller.pressChanged(DragPress(from: on, to: p(on.x + 30, on.y)), shift: false, in: context)
        XCTAssertEqual(context.viewport.offset, .zero, "a held button does not pan")
        controller.pressEnded(DragPress(from: on, to: on), in: context)
        XCTAssertFalse(try chip(doc, button.id).isPressed)

        // Held when play mode stops: let go.
        controller.pressChanged(DragPress(from: on, to: on), shift: false, in: context)
        controller.runningChanged(false, in: context)
        XCTAssertNil(controller.interaction)
        XCTAssertFalse(try chip(doc, button.id).isPressed)
        controller.runningChanged(false, in: context)

        click(controller, context, at: centre(fuse))
        XCTAssertTrue(simulation.blownFuses.contains(fuse.id))
        let lever = ChipLayout.switchLeverRect(position: 1, bodyWidth: switches.layout.size.width)
        click(controller, context, at: p(switches.origin.x + lever.midX, switches.origin.y + lever.midY))
        XCTAssertEqual(try chip(doc, switches.id).switchesOn.first, !switches.switchesOn[0])

        drag(controller, context, from: p(900, 800), to: p(950, 820))
        XCTAssertEqual(context.viewport.offset, CGSize(width: 50, height: 20))
    }
}
