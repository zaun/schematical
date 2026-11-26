import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AJ: the Schematic canvas as it is drawn: hosted in a window
/// (appearing installs its key monitor) and rendered to a bitmap with every
/// kind of symbol on it, every layer, the previews and the overlays.
@MainActor
final class SchematicCanvasRenderTests: XCTestCase {
    private static var keptSimulations: [SimulationController] = []
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
        for simulation in Self.keptSimulations where simulation.isRunning { simulation.stop() }
    }

    private func simulation() -> SimulationController {
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        return simulation
    }

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func canvas(_ doc: SchematicDocument, _ viewport: CanvasViewport, tool: SchematicTool,
                        simulation: SimulationController, controller: SchematicCanvasController? = nil) -> SchematicCanvasView {
        SchematicCanvasView(catalog: TestSupport.catalog, document: doc, viewport: viewport, tool: tool, simulation: simulation,
                            onOpenMemory: { _ in }, onOpenCpu: { _ in }, onOpenTerminal: { _ in }, onOpenRegisters: { _ in },
                            onOpenDocumentInfo: {}, onPartPlaced: {}, controller: controller)
    }

    /// The first part (by name) drawn as each symbol.
    static func partPerSymbol() -> [PartSymbol: CatalogPart] {
        var found: [PartSymbol: CatalogPart] = [:]
        for part in (TestSupport.allParts + supplies).sorted(by: { $0.name < $1.name }) where found[symbol(for: part)] == nil {
            found[symbol(for: part)] = part
        }
        return found
    }

    /// The supplies the VCC, GND and pull tools place (not in the drawer).
    static var supplies: [CatalogPart] {
        (SupplyChoice.names + SupplyChoice.names.map { "PULLUP-" + $0 } + ["GND", "PULLDOWN"])
            .compactMap { TestSupport.catalog.part(named: $0) }
    }

    /// A document with one part of each symbol on a grid, 300 apart.
    private func everySymbol() throws -> (SchematicDocument, [PartSymbol: PlacedChip]) {
        let doc = TestSupport.newDocument()
        var placed: [PartSymbol: PlacedChip] = [:]
        for (n, (kind, part)) in Self.partPerSymbol().sorted(by: { $0.value.name < $1.value.name }).enumerated() {
            placed[kind] = try TestSupport.place(doc, part.name, at: p(150 + CGFloat(n % 6) * 300, 150 + CGFloat(n / 6) * 300))
        }
        return (doc, placed)
    }

    /// The RGBA pixels of `image`, row 0 at the top.
    private struct Pixels {
        let width: Int, height: Int
        let data: [UInt8]

        init(_ image: CGImage) {
            width = image.width
            height = image.height
            var data = [UInt8](repeating: 0, count: width * height * 4)
            let context = CGContext(data: &data, width: width, height: height, bitsPerComponent: 8, bytesPerRow: width * 4,
                                    space: CGColorSpaceCreateDeviceRGB(),
                                    bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue)!
            context.draw(image, in: CGRect(x: 0, y: 0, width: width, height: height))
            self.data = data
        }

        func rgb(_ x: Int, _ y: Int) -> [UInt8] {
            let i = (y * width + x) * 4
            return Array(data[i..<i + 3])
        }

        /// How many pixels in `rect` (image points at `scale`) differ from `background`.
        func ink(in rect: CGRect, scale: CGFloat, background: [UInt8]) -> Int {
            var count = 0
            let r = rect.applying(CGAffineTransform(scaleX: scale, y: scale)).integral
                .intersection(CGRect(x: 0, y: 0, width: width, height: height))
            guard !r.isNull else { return 0 }
            for y in Int(r.minY)..<Int(r.maxY) {
                for x in Int(r.minX)..<Int(r.maxX) where zip(rgb(x, y), background).contains(where: { abs(Int($0) - Int($1)) > 24 }) {
                    count += 1
                }
            }
            return count
        }
    }

    private func render(_ view: some View, size: CGSize) throws -> (Pixels, CGFloat) {
        let renderer = ImageRenderer(content: view.frame(width: size.width, height: size.height))
        renderer.scale = 1
        let image = try XCTUnwrap(renderer.cgImage)
        return (Pixels(image), CGFloat(image.width) / size.width)
    }

    func testEveryKindOfPartIsDrawnWhereItIs() throws {
        let (doc, placed) = try everySymbol()
        XCTAssertGreaterThanOrEqual(placed.count, 26, "a part for nearly every symbol: \(placed.keys)")
        let viewport = CanvasViewport()
        let view = canvas(doc, viewport, tool: .pointer,
                                       simulation: simulation())
        let size = CGSize(width: 1900, height: 1400)
        viewport.updateSize(size)
        let (pixels, scale) = try render(view, size: size)
        let background = pixels.rgb(2, 2)
        for (kind, chip) in placed where kind != .ground {
            let frame = CGRect(origin: chip.origin, size: chip.layout.size).insetBy(dx: -4, dy: -4)
            XCTAssertGreaterThan(pixels.ink(in: frame, scale: scale, background: background), 20, "\(kind) \(chip.partNumber)")
        }
    }

    func testTheCanvasDrawsEveryLayerPreviewAndOverlay() throws {
        let (doc, placed) = try everySymbol()
        let led = try XCTUnwrap(placed[.diode])
        let other = try XCTUnwrap(placed[.resistor])
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: other.id, index: 0))))
        let bus = try XCTUnwrap(doc.addBus(from: p(100, 1300), to: p(900, 1300)))
        XCTAssertNil(doc.addPort(at: p(500, 1300)))
        _ = doc.addNote(at: p(1200, 1300))
        _ = doc.addNetLabel(at: p(1500, 1300))
        doc.setTitleBlockShown(true)
        doc.select(.chip(led.id))
        doc.toggleSelection(.bus(bus))
        doc.ercMarkers = [ErcMarker(point: p(1700, 1300), severity: .error, key: "k")]
        doc.ercSelectedKeys = ["k"]

        let viewport = CanvasViewport()
        let controller = SchematicCanvasController()
        let simulation = simulation()
        let monitors = EditorEventMonitors()
        let view = canvas(doc, viewport, tool: .line,
                                       simulation: simulation, controller: controller)
            .environment(\.editorEventMonitors, monitors)
        let probe = ViewProbe(view, size: NSSize(width: 900, height: 700))
        probes.append(probe)
        XCTAssertEqual(monitors.count, 1, "appearing installs the drawing-key monitor")
        XCTAssertEqual(viewport.size, CGSize(width: 900, height: 700), "the canvas tells the viewport its size")

        // A wire being drawn, a hovered part, a dropped part's target and an
        // ERC line to show: all drawn.
        let context = SchematicCanvasContext(catalog: TestSupport.catalog, document: doc, viewport: viewport, tool: .line,
                                             simulation: simulation)
        let tip = try XCTUnwrap(doc.pinTip(try TestSupport.pin(led, "A")))
        controller.pressChanged(DragPress(from: tip, to: tip), shift: false, in: context)
        controller.pressChanged(DragPress(from: tip, to: p(tip.x + 60, tip.y + 60)), shift: false, in: context)
        controller.isDropTargeted = true
        doc.ercFocus = ErcFocus(point: p(1700, 1300), serial: 1)
        probe.settle()
        XCTAssertNotEqual(viewport.offset, .zero, "the ERC's point is brought into view")
        let (pixels, scale) = try render(canvas(doc, viewport, tool: .line, simulation: simulation, controller: controller),
                                         size: CGSize(width: 900, height: 700))
        let corner = pixels.rgb(450, 350)
        XCTAssertGreaterThan(pixels.ink(in: CGRect(x: 0, y: 200, width: 3, height: 300), scale: scale, background: corner),
                             10, "the drop target's dashed border")
        controller.pressEnded(DragPress(from: tip, to: p(tip.x + 60, tip.y + 60)), in: context)

        // The Bus tool's preview and the Pointer's selection box.
        viewport.pan(to: .zero)
        let busContext = SchematicCanvasContext(catalog: TestSupport.catalog, document: doc, viewport: viewport, tool: .bus,
                                                simulation: simulation)
        controller.pressChanged(DragPress(from: p(100, 1100), to: p(100, 1100)), shift: false, in: busContext)
        controller.pressChanged(DragPress(from: p(100, 1100), to: p(400, 1100)), shift: false, in: busContext)
        XCTAssertNotNil(try render(canvas(doc, viewport, tool: .bus, simulation: simulation, controller: controller),
                                   size: CGSize(width: 900, height: 700)))
        controller.pressEnded(DragPress(from: p(100, 1100), to: p(100, 1100)), in: busContext)
        controller.isDropTargeted = false
        let pointer = SchematicCanvasContext(catalog: TestSupport.catalog, document: doc, viewport: viewport, tool: .pointer,
                                             simulation: simulation)
        controller.pressChanged(DragPress(from: p(800, 10), to: p(800, 10)), shift: true, in: pointer)
        controller.pressChanged(DragPress(from: p(800, 10), to: p(600, 100)), shift: true, in: pointer)
        let (boxed, boxScale) = try render(canvas(doc, viewport, tool: .pointer, simulation: simulation, controller: controller),
                                           size: CGSize(width: 900, height: 700))
        XCTAssertGreaterThan(boxed.ink(in: CGRect(x: 620, y: 9, width: 160, height: 3), scale: boxScale,
                                       background: boxed.rgb(450, 690)), 20, "the selection box's edge")
        controller.pressEnded(DragPress(from: p(800, 10), to: p(800, 10)), in: pointer)

        // Play mode with every part lit, then stopped.
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        probe.settle()
        XCTAssertNotNil(try render(canvas(doc, viewport, tool: .pointer, simulation: simulation, controller: controller),
                                   size: CGSize(width: 900, height: 700)))
        simulation.stop()
        probe.settle()

        probe.window.contentView = nil
        probe.settle()
        XCTAssertEqual(monitors.count, 0, "disappearing takes the monitor off")
    }

    func testHoverPhaseLocation() {
        XCTAssertEqual(SchematicCanvasView.location(of: .active(p(3, 4))), p(3, 4))
        XCTAssertNil(SchematicCanvasView.location(of: .ended))
    }
}
