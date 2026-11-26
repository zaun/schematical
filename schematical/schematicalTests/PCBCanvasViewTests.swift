import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// The PCB canvas hosted in a window (`PCBCanvasView`'s body), its hover
/// layer (`PCBHoverLayer`, `PadLabel`) and the small types beside it
/// (`PCBCanvasTypes.swift`).
@MainActor
final class PCBCanvasViewTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        // Closing a test's window must not quit the host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
    }

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func host<V: View>(_ view: V, size: NSSize = NSSize(width: 600, height: 400)) -> ViewProbe {
        let probe = ViewProbe(view, size: size)
        probes.append(probe)
        return probe
    }

    /// Two LEDs wired K to A, apart on the board; the K pad's centre.
    private func pair() throws -> (doc: SchematicDocument, k: CGPoint) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: p(400, 200))
        doc.moveFootprint(one.id, to: p(10, 8))
        doc.moveFootprint(two.id, to: p(20, 8))
        let kRef = try TestSupport.pin(one, "K"), aRef = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(kRef), .pin(aRef)))
        let chip = try XCTUnwrap(doc.chips.first { $0.id == one.id })
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == chip.part.pins[kRef.index].designator })
        return (doc, p(pad.rect.midX, pad.rect.midY))
    }

    private func mouse(_ probe: ViewProbe, _ type: NSEvent.EventType, _ point: NSPoint) throws -> NSEvent {
        try XCTUnwrap(NSEvent.mouseEvent(with: type, location: point, modifierFlags: [],
                                         timestamp: ProcessInfo.processInfo.systemUptime,
                                         windowNumber: probe.window.windowNumber, context: nil, eventNumber: 0,
                                         clickCount: 1, pressure: 1))
    }

    /// The hosted canvas: it takes its size, keeps the Trace tool's layer
    /// and angle for the key monitor, installs the monitor on appearing and
    /// takes it off on going; a drag on empty board pans it.
    func testTheHostedCanvasSizesInstallsAndPans() throws {
        let doc = TestSupport.newDocument()
        let viewport = CanvasViewport()
        let controller = PCBCanvasController()
        let monitors = EditorEventMonitors()
        let canvas = PCBCanvasView(document: doc, viewport: viewport, tool: .select, traceLayer: .bottom, traceAngle: 90,
                                   controller: controller)
        let probe = host(canvas.environment(\.editorEventMonitors, monitors))
        XCTAssertEqual(viewport.size, CGSize(width: 600, height: 400))
        XCTAssertEqual(controller.live.layer, .bottom)
        XCTAssertEqual(controller.live.angle, 90)
        XCTAssertNotNil(controller.drawingKeyMonitor)
        XCTAssertEqual(monitors.count, 1)

        let content = try XCTUnwrap(probe.window.contentView)
        // Board point (5, 75) mm is off any part: a press there pans.
        let start = content.convert(NSPoint(x: 100, y: content.bounds.height - 300), to: nil)
        let target = try XCTUnwrap(content.hitTest(content.convert(start, from: nil)))
        target.mouseDown(with: try mouse(probe, .leftMouseDown, start))
        for step in 1...4 {
            probe.settle(0.05)
            target.mouseDragged(with: try mouse(probe, .leftMouseDragged, NSPoint(x: start.x + CGFloat(step * 10), y: start.y)))
        }
        probe.settle(0.1)
        guard case .pan? = controller.interaction else {
            return XCTFail("\(String(describing: controller.interaction))")
        }
        target.mouseUp(with: try mouse(probe, .leftMouseUp, NSPoint(x: start.x + 40, y: start.y)))
        probe.settle()
        XCTAssertNil(controller.interaction)
        XCTAssertEqual(viewport.offset.width, 40, accuracy: 1)

        probe.window.contentView = NSView()
        probe.settle()
        XCTAssertNil(controller.drawingKeyMonitor, "taken off when the canvas goes")
        XCTAssertEqual(monitors.count, 0)
    }

    /// A box being drawn shows its selection rectangle over the board.
    func testABoxShowsItsRectangle() throws {
        let doc = TestSupport.newDocument()
        let controller = PCBCanvasController()
        func colour(at point: CGPoint) throws -> NSColor {
            let view = PCBCanvasView(document: doc, viewport: CanvasViewport(), controller: controller)
                .frame(width: 600, height: 400)
            let renderer = ImageRenderer(content: view)
            renderer.scale = 1
            let rep = NSBitmapImageRep(cgImage: try XCTUnwrap(renderer.cgImage))
            return try XCTUnwrap(rep.colorAt(x: Int(point.x), y: Int(point.y)))
        }
        let before = try colour(at: p(150, 150))
        controller.pressLocation = p(100, 100)
        controller.interaction = .boxSelect(base: [], to: p(200, 200))
        XCTAssertNotEqual(try colour(at: p(150, 150)), before, "the box's tint")
        XCTAssertEqual(try colour(at: p(250, 250)), try colour(at: p(250, 250)))
    }

    /// The hosted canvas follows its controller: the hovered pad's tag
    /// shows while nothing is dragged, goes during a pan, and comes back.
    func testTheHostedCanvasRedrawsAsItsControllerChanges() throws {
        let (doc, k) = try pair()
        let label = try XCTUnwrap(doc.padLabel(at: k)).label
        let controller = PCBCanvasController()
        let view = PCBCanvasView(document: doc, viewport: CanvasViewport(), controller: controller)
        let probe = host(view)
        controller.hovered(at: p(k.x * BoardScale.pointsPerMM, k.y * BoardScale.pointsPerMM), in: view)
        probe.settle()
        XCTAssertTrue(probe.has(label), "\(probe.labels())")
        controller.interaction = .pan(startOffset: .zero)
        probe.settle()
        XCTAssertFalse(probe.has(label), "no tag during a pan")
        controller.interaction = nil
        probe.settle()
        XCTAssertTrue(probe.has(label))
    }

    /// The hover layer: the hovered pad's outline and its name and net
    /// beside the pointer; the Trace tool's ring; nothing with no pointer.
    func testHoverLayerNamesThePadAndDrawsTheRing() throws {
        let (doc, k) = try pair()
        let hover = BoardHover()
        hover.point = k
        let scale = BoardScale.pointsPerMM
        hover.location = p(k.x * scale, k.y * scale)
        let label = try XCTUnwrap(doc.padLabel(at: k))
        let layer = PCBHoverLayer(document: doc, viewport: CanvasViewport(), hover: hover, visible: BoardVisibility(),
                                  showsPad: true, ringLayer: .top)
        let probe = host(layer)
        XCTAssertTrue(probe.has(label.label, role: "AXStaticText"), "\(probe.labels())")
        let net = doc.netName(ofPad: label.pad, in: doc.connectivity())
        XCTAssertTrue(probe.has(net), "\(probe.labels())")

        func greenNear(_ view: PCBHoverLayer) throws -> Bool {
            let renderer = ImageRenderer(content: view.frame(width: 600, height: 400))
            renderer.scale = 1
            let rep = NSBitmapImageRep(cgImage: try XCTUnwrap(renderer.cgImage))
            let anchor = TraceDrawing.anchor(doc, at: k, layer: .top, scale: scale)
            let x = Int(anchor.x * scale + 6), y = Int(anchor.y * scale)
            return (-2...2).contains { dx in
                guard let colour = rep.colorAt(x: x + dx, y: y)?.usingColorSpace(.sRGB) else { return false }
                return colour.greenComponent > 0.6 && colour.redComponent < 0.5
            }
        }
        XCTAssertTrue(try greenNear(layer), "the landing ring")
        let noRing = PCBHoverLayer(document: doc, viewport: CanvasViewport(), hover: hover, visible: BoardVisibility(),
                                   showsPad: false, ringLayer: nil)
        XCTAssertFalse(try greenNear(noRing))
        let quiet = host(noRing)
        XCTAssertFalse(quiet.has(label.label), "no pad during a drag")
    }

    /// The pad tag shows its text.
    func testPadLabelShowsItsText() {
        let probe = host(PadLabel(text: "U1.3\nGND"))
        XCTAssertTrue(probe.has("U1.3"), "\(probe.labels())")
    }

    // MARK: The small types

    /// A press off any reference is nothing; a drag with no movement leaves
    /// the reference where it is.
    func testReferenceDraggingMisses() throws {
        let doc = TestSupport.newDocument()
        let part = try TestSupport.place(doc, "SN74HC00N", at: p(100, 100))
        XCTAssertNil(ReferenceDragging.press(doc, at: p(90, 70), scale: 20, visible: BoardVisibility()))
        let before = try XCTUnwrap(doc.chips.first?.boardReferenceCentre)
        ReferenceDragging.drag(doc, id: part.id, from: before, by: .zero)
        XCTAssertEqual(doc.chips.first?.boardReferenceCentre, before)
        ReferenceDragging.drag(doc, id: part.id, from: before, by: CGSize(width: 5.08, height: 0))
        XCTAssertNotEqual(doc.chips.first?.boardReferenceCentre, before)
    }

    /// Of two unrouted lines the Route tool takes the nearer.
    func testRouteToolTakesTheNearerLine() throws {
        let (doc, _) = try pair()
        let three = try TestSupport.place(doc, "GENERIC-LED", at: p(700, 200))
        doc.moveFootprint(three.id, to: p(10, 30))
        let first = try XCTUnwrap(doc.chips.first)
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(first, "A")), .pin(try TestSupport.pin(three, "K"))))
        let lines = doc.ratsnest()
        XCTAssertGreaterThanOrEqual(lines.count, 2)
        let line = try XCTUnwrap(lines.last)
        let middle = p((line.0.x + line.1.x) / 2, (line.0.y + line.1.y) / 2)
        XCTAssertEqual(RouteTool.scope(doc, at: middle, tolerance: 0.1), .connection(from: line.0, to: line.1))
        XCTAssertNil(RouteTool.scope(doc, at: p(60, 70), tolerance: 0.1), "far from every line")
    }

    /// V where the assisted leg is illegal says so and drops nothing.
    func testDropViaOnAnIllegalLegSaysSo() throws {
        let doc = TestSupport.newDocument()
        XCTAssertNotNil(doc.addTrace(from: p(10.16, 2.54), to: p(10.16, 20.32), layer: .top, diagonalFirst: false))
        let start = p(2.54, 10.16)
        let assist = AssistRouter(document: doc, layer: .top, start: start)
        let blocked = p(10.16, 10.16)
        XCTAssertFalse(TraceDrawing.assisted(doc, draft: TraceDraft(start: start), to: blocked, layer: .top, scale: 20,
                                             angle: 45, assist: assist).isLegal)
        XCTAssertNil(TraceDrawing.dropVia(doc, draft: TraceDraft(start: start), to: blocked, layer: .top, scale: 20,
                                          angle: 45, assist: assist))
        XCTAssertEqual(doc.editorMessage, TraceDrawing.noRouteMessage)
        XCTAssertTrue(doc.vias.isEmpty)
    }

    /// A pad's outline: rounded corners at its radius, or the chosen
    /// corners cut at 45°.
    func testPadOutlines() {
        let rect = CGRect(x: 0, y: 0, width: 2, height: 1)
        let rounded = PlacedPad(designator: "1", rect: rect, drill: 0, shape: .rect, radius: 0.3, chamfer: 0, corners: [])
        XCTAssertFalse(PCBCanvasView.outline(of: rounded).contains(p(0.02, 0.02)), "the corner is rounded off")
        XCTAssertTrue(PCBCanvasView.outline(of: rounded).contains(p(0.4, 0.05)))
        let cut = PlacedPad(designator: "1", rect: rect, drill: 0, shape: .rect, radius: 0, chamfer: 0.4,
                            corners: [.topLeft, .bottomRight])
        let path = PCBCanvasView.outline(of: cut)
        XCTAssertFalse(path.contains(p(0.05, 0.05)), "top-left cut")
        XCTAssertFalse(path.contains(p(1.95, 0.95)), "bottom-right cut")
        XCTAssertTrue(path.contains(p(1.95, 0.05)), "top-right square")
        XCTAssertTrue(path.contains(p(0.05, 0.95)), "bottom-left square")
        let none = PlacedPad(designator: "1", rect: rect, drill: 0, shape: .rect, radius: 0, chamfer: 0.4, corners: [])
        XCTAssertTrue(PCBCanvasView.outline(of: none).contains(p(0.02, 0.02)), "no corners named: square")
    }

    /// A via on a Plane's net joins it by its bare drill.
    func testAViaOnThePlanesNetCutsOnlyItsDrill() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let ground = try TestSupport.place(doc, "GND", at: p(100, 400))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
        doc.moveFootprint(led.id, to: p(20, 20))
        let chip = try XCTUnwrap(doc.chips.first { $0.id == led.id })
        let k = try XCTUnwrap(chip.boardPads.first { $0.designator == chip.part.pins[try TestSupport.pin(led, "K").index].designator })
        let from = p(k.rect.midX, k.rect.midY)
        let to = p(BoardGrid.snap(from.x), BoardGrid.snap(from.y + 7.62))
        XCTAssertNotNil(doc.addTrace(from: from, to: to, layer: .top, diagonalFirst: false))
        let end = try XCTUnwrap(doc.traces.first?.points.last)
        XCTAssertNotNil(doc.addVia(at: end))
        doc.setLayerNet(.inner(2), .ground)
        let via = try XCTUnwrap(doc.vias.first)
        let cuts = PCBCanvasView.planeCuts(on: .inner(2), copper: doc.connectivity())
        XCTAssertTrue(cuts.drills.contains(via.centre), "its drill")
        XCTAssertFalse(cuts.copper.contains(via.centre), "no clearance round it")
    }

    /// A canvas made without its callbacks ignores what they would say:
    /// nothing changes in the document.
    func testDefaultCallbacksDoNothing() throws {
        let doc = TestSupport.newDocument()
        let view = PCBCanvasView(document: doc, viewport: CanvasViewport())
        let before = doc.undoName
        view.onShowLayer(.bottom)
        view.onLayerChange(.bottom)
        view.onRoute(.nets([.ground]))
        view.onPlaced()
        XCTAssertEqual(doc.undoName, before)
        XCTAssertNil(view.controller)
        XCTAssertEqual(view.tool, .select)
        XCTAssertEqual(view.traceLayer, .top)
    }

    /// Tools and cutout shapes are identified by their raw values.
    func testToolAndShapeIDs() {
        XCTAssertEqual(PCBTool.allCases.map(\.id), PCBTool.allCases.map(\.rawValue))
        XCTAssertEqual(CutoutShape.allCases.map(\.id), ["rectangle", "circle", "slot"])
    }
}
