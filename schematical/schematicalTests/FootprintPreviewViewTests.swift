import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// The Part Editor's footprint preview as drawn and driven (plan-68-AD): the
/// drawing in pixels, the zoom and pan controller, and the view's controls.
@MainActor
final class FootprintPreviewViewTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        // Closing a test's window must not quit the host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
    }

    // MARK: Drawing

    /// An sRGB colour's 0–255 components.
    private struct RGB: Equatable {
        let r, g, b: Int

        init(_ color: Color) {
            let ns = NSColor(color).usingColorSpace(.sRGB)!
            r = Int((ns.redComponent * 255).rounded())
            g = Int((ns.greenComponent * 255).rounded())
            b = Int((ns.blueComponent * 255).rounded())
        }

        init(r: Int, g: Int, b: Int) {
            self.r = r
            self.g = g
            self.b = b
        }

        func near(_ other: RGB, _ tolerance: Int = 6) -> Bool {
            abs(r - other.r) <= tolerance && abs(g - other.g) <= tolerance && abs(b - other.b) <= tolerance
        }
    }

    private static let side = 480

    /// The chip drawn into a 480 × 480 sRGB bitmap: each opaque pixel's colour.
    private func pixels(_ chip: PlacedChip, viewport given: CanvasViewport? = nil) throws -> [RGB] {
        let viewport = given ?? CanvasViewport(baseScale: 1)
        let size = CGSize(width: Self.side, height: Self.side)
        let canvas = Canvas { context, size in
            FootprintPreview.draw(chip, in: &context, size: size, viewport: viewport)
        }
        .frame(width: size.width, height: size.height)
        let renderer = ImageRenderer(content: canvas)
        renderer.scale = 1
        let image = try XCTUnwrap(renderer.cgImage)
        let space = CGColorSpace(name: CGColorSpace.sRGB)!
        var data = [UInt8](repeating: 0, count: Self.side * Self.side * 4)
        let context = try XCTUnwrap(CGContext(data: &data, width: Self.side, height: Self.side, bitsPerComponent: 8,
                                              bytesPerRow: Self.side * 4, space: space,
                                              bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
        context.draw(image, in: CGRect(origin: .zero, size: size))
        return stride(from: 0, to: data.count, by: 4).compactMap { i in
            data[i + 3] > 250 ? RGB(r: Int(data[i]), g: Int(data[i + 1]), b: Int(data[i + 2])) : nil
        }
    }

    private func count(_ color: Color, in pixels: [RGB]) -> Int {
        let target = RGB(color)
        return pixels.filter { $0.near(target) }.count
    }

    /// The bundled part with the fewest pads (the largest drawn) whose
    /// preview chip passes `test`.
    private func chip(where test: (PlacedChip) -> Bool, _ what: String) throws -> PlacedChip {
        let chips = TestSupport.catalog.allParts.filter { $0.footprint != nil }.map(FootprintPreview.chip(for:))
        return try XCTUnwrap(chips.filter(test).min { $0.boardPads.count < $1.boardPads.count },
                             "no bundled part with \(what)")
    }

    func testThroughHolePadsAreCopperWithHoles() throws {
        let chip = try chip(where: { chip in chip.boardPads.contains { $0.drill > 0 } && chip.pin1Mark != nil },
                            "a through-hole pad")
        let drawn = try pixels(chip)
        XCTAssertGreaterThan(count(PCBCanvasView.copper, in: drawn), 50)
        // Holes are cut in the background colour.
        XCTAssertGreaterThan(count(PCBCanvasView.background, in: drawn), 5)
        // Pin 1's dot and the outline are silkscreen ink.
        XCTAssertGreaterThan(count(BoardColor.green.silkscreen, in: drawn), 0)
    }

    func testAnOppositePadIsBottomCopper() throws {
        let chip = try chip(where: { $0.boardPads.contains { $0.opposite && $0.drill == 0 } }, "an opposite pad")
        let drawn = try pixels(chip)
        XCTAssertGreaterThan(count(LayerStyle.defaultStyle(of: .bottom).color.color, in: drawn), 0)
    }

    func testANonPlatedHoleIsOutlinedNotFilled() throws {
        let chip = try chip(where: { chip in chip.boardPads.contains { !$0.plated } }, "a non-plated hole")
        let pad = try XCTUnwrap(chip.boardPads.first { !$0.plated })
        XCTAssertNotNil(pad.holeSegment)
        let drawn = try pixels(chip)
        XCTAssertGreaterThan(count(BoardColor.green.silkscreen, in: drawn), 0)
    }

    func testARoundBodyIsDrawn() throws {
        let chip = try chip(where: { $0.boardBody?.round == true }, "a round body")
        let drawn = try pixels(chip)
        XCTAssertGreaterThan(count(BoardColor.document, in: drawn), 0)
    }

    func testAPartWithoutAFootprintDrawsNothing() throws {
        // A supply has no footprint.
        var draft = PartDraft(newOf: "supply")
        draft.name = "PEM-VCC"
        draft.pinouts[0].pins = [PinDraft(designator: "1", name: "VCC", role: "power")]
        let part: CatalogPart
        switch PartValidator.shared.validate(draft, source: "supply.schpart") {
        case let .success(valid): part = valid
        case let .failure(failure): return XCTFail(failure.message)
        }
        XCTAssertNil(part.footprint)
        XCTAssertEqual(try pixels(FootprintPreview.chip(for: part)), [])
        XCTAssertEqual(FootprintPreview.caption(for: part), "No footprint")
    }

    func testZoomDrawsLarger() throws {
        let chip = try chip(where: { chip in chip.boardPads.contains { $0.drill > 0 } }, "a through-hole pad")
        let plain = count(PCBCanvasView.copper, in: try pixels(chip))
        let viewport = CanvasViewport(baseScale: 1)
        viewport.zoomIn()
        let zoomed = count(PCBCanvasView.copper, in: try pixels(chip, viewport: viewport))
        XCTAssertGreaterThan(zoomed, plain)
    }

    // MARK: Zoom and pan

    func testDragPansFromWhereItBegan() {
        let controller = FootprintPreviewController()
        controller.drag(CGSize(width: 10, height: 5))
        XCTAssertEqual(controller.panStart, .zero)
        controller.drag(CGSize(width: 30, height: -5))
        XCTAssertEqual(controller.viewport.offset, CGSize(width: 30, height: -5))
        controller.endDrag()
        XCTAssertNil(controller.panStart)
        controller.drag(CGSize(width: 1, height: 1))
        XCTAssertEqual(controller.viewport.offset, CGSize(width: 31, height: -4))
    }

    func testZoomStepsAndFit() {
        let controller = FootprintPreviewController()
        controller.zoomIn()
        XCTAssertEqual(controller.viewport.zoom, 1.5)
        controller.zoomOut()
        controller.zoomOut()
        XCTAssertEqual(controller.viewport.zoom, 0.75)
        controller.drag(CGSize(width: 9, height: 9))
        controller.fit()
        XCTAssertEqual(controller.viewport.zoom, 1)
        XCTAssertEqual(controller.viewport.offset, .zero)
    }

    // MARK: The view

    func testTheViewsButtonsAndDragDriveItsController() throws {
        let part = try XCTUnwrap(TestSupport.allParts.first { $0.footprint?.id == "PDIP-14" })
        let controller = FootprintPreviewController()
        let probe = ViewProbe(FootprintPreviewView(part: part, controller: controller),
                              size: NSSize(width: 320, height: 300))
        probes.append(probe)
        XCTAssertTrue(probe.has("PDIP-14: 14 pads"), "\(probe.labels())")

        try probe.press("Zoom In")
        XCTAssertEqual(controller.viewport.zoom, 1.5)
        try probe.press("Zoom Out")
        try probe.press("Zoom Out")
        XCTAssertEqual(controller.viewport.zoom, 0.75)
        try probe.press("Fit")
        XCTAssertEqual(controller.viewport.zoom, 1)

        // A drag on the canvas pans; letting go ends the drag.
        let content = try XCTUnwrap(probe.window.contentView)
        let start = content.convert(NSPoint(x: 160, y: content.bounds.height - 80), to: nil)
        let target = try XCTUnwrap(content.hitTest(content.convert(start, from: nil)))
        func event(_ type: NSEvent.EventType, _ point: NSPoint) -> NSEvent {
            NSEvent.mouseEvent(with: type, location: point, modifierFlags: [], timestamp: ProcessInfo.processInfo.systemUptime,
                               windowNumber: probe.window.windowNumber, context: nil, eventNumber: 0, clickCount: 1,
                               pressure: 1)!
        }
        target.mouseDown(with: event(.leftMouseDown, start))
        for step in 1...4 {
            probe.settle(0.05)
            target.mouseDragged(with: event(.leftMouseDragged, NSPoint(x: start.x + CGFloat(step * 10), y: start.y)))
        }
        probe.settle(0.1)
        XCTAssertNotNil(controller.panStart)
        target.mouseUp(with: event(.leftMouseUp, NSPoint(x: start.x + 40, y: start.y)))
        probe.settle()
        XCTAssertNil(controller.panStart)
        XCTAssertGreaterThan(controller.viewport.offset.width, 0)
    }
}
