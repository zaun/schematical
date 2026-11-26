import AppKit
import CoreGraphics
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AG: the Canvas folder's panels and layers as they are shown and
/// driven: a length field's typing, Return, focus and live modes; the
/// Cutout panel for a rounded cutout and a DXF polygon; each tab of Board
/// Details (Route Net from a net's menu closes it first); and the ERC
/// markers and net labels as drawn.
@MainActor
final class CanvasPanelsRenderTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
    }

    private func show<V: View>(_ view: V, size: NSSize = NSSize(width: 700, height: 700)) -> ViewProbe {
        let probe = ViewProbe(view, size: size)
        probes.append(probe)
        return probe
    }

    /// The commits a field made, and what the document "kept" for each.
    private final class Commits {
        var values: [Double] = []
        var keep: (Double) -> Double = { $0 }
        nonisolated deinit {}
    }

    // MARK: A length field

    func testParsingALength() {
        XCTAssertEqual(LengthField.parse("2", units: .inch), 50.8)
        XCTAssertEqual(LengthField.parse(" 2 in", units: .inch), 50.8)
        XCTAssertEqual(LengthField.parse("1\"", units: .inch), 25.4)
        XCTAssertEqual(LengthField.parse("3 mm", units: .mm), 3)
        XCTAssertNil(LengthField.parse("three", units: .mm))
        XCTAssertNil(LengthField.parse("inf", units: .mm))
    }

    func testALengthFieldCommitsOnReturnAndShowsWhatWasKept() throws {
        let commits = Commits()
        let probe = show(Form {
            LengthField(title: "Width", millimetres: 25.4, units: .inch) { value in
                commits.values.append(value)
                return commits.keep(value)
            }
        })
        let field = try probe.textField(showing: "1")
        // Return with the text unchanged commits nothing.
        probe.submit(field)
        XCTAssertEqual(commits.values, [])
        probe.type("2", into: field)
        XCTAssertEqual(commits.values, [], "not live: typing alone commits nothing")
        probe.submit(field)
        XCTAssertEqual(commits.values, [50.8])
        XCTAssertEqual(field.stringValue, "2")
        // The document clamps: the field shows what it kept.
        commits.keep = { _ in 10 }
        probe.type("9", into: field)
        probe.submit(field)
        XCTAssertEqual(commits.values.last, 9 * 25.4)
        XCTAssertEqual(field.stringValue, LengthField.format(10, units: .inch))
        // Text that isn't a length puts the stored value back.
        probe.type("wide", into: field)
        probe.submit(field)
        XCTAssertEqual(commits.values.count, 2)
        XCTAssertEqual(field.stringValue, "1")
        // Leaving the field commits too.
        commits.keep = { $0 }
        probe.type("3", into: field)
        probe.blur()
        XCTAssertEqual(commits.values.last, 3 * 25.4)
    }

    func testALiveLengthFieldCommitsAsItIsTyped() throws {
        let commits = Commits()
        let probe = show(Form {
            LengthField(title: "Gap", millimetres: 0.2, units: .mm, precise: true, live: true) { value in
                commits.values.append(value)
                return value
            }
        })
        let field = try probe.textField(showing: "0.2")
        probe.type("0.35", into: field)
        XCTAssertEqual(commits.values, [0.35])
        probe.type("0.3x", into: field)
        XCTAssertEqual(commits.values, [0.35], "not a length yet")
    }

    // MARK: The Cutout panel

    func testTheCutoutPanelEditsARoundedCutout() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addCutout(.rounded(rect: CGRect(x: 10, y: 10, width: 10, height: 4), radius: 1)))
        let cutout = try XCTUnwrap(doc.cutouts.first { $0.id == id })
        let probe = show(Form { CutoutPanel(document: doc, cutout: cutout) }.formStyle(.grouped))
        XCTAssertTrue(probe.has(cutout.kind.title), "\(probe.labels())")
        // X, Y, Width, Height, Radius, as the document holds them (snapped to its grid).
        func shown() throws -> [Double] {
            let box = try XCTUnwrap(CutoutPanelActions.shape(doc, id)).bounds
            return [box.minX, box.minY, box.width, box.height, try XCTUnwrap(CutoutPanelActions.radius(doc, id))]
        }
        func format(_ values: [Double]) -> [String] { values.map { LengthField.format($0, units: .mm, precise: true) } }
        XCTAssertEqual(probe.textFields().map(\.stringValue), format(try shown()))
        let wanted: [Double] = [15, 14, 8, 6, 2]
        for (index, value) in wanted.enumerated() {
            let before = try shown()[index]
            probe.type(String(value), into: probe.textFields()[index])
            probe.submit(probe.textFields()[index])
            let now = try shown()[index]
            XCTAssertNotEqual(now, before, "field \(index) changed the cutout")
            XCTAssertEqual(now, value, accuracy: 0.7, "field \(index)")
            XCTAssertEqual(probe.textFields()[index].stringValue, format([now])[0], "field \(index) shows what was kept")
        }
    }

    func testTheCutoutPanelShowsAPolygonsSize() throws {
        let doc = TestSupport.newDocument()
        let loop = [CGPoint(x: 10, y: 10), CGPoint(x: 30, y: 10), CGPoint(x: 20, y: 25)]
        let id = try XCTUnwrap(doc.addCutout(.polygon(loop)))
        let cutout = try XCTUnwrap(doc.cutouts.first { $0.id == id })
        XCTAssertNil(CutoutPanelActions.radius(doc, id))
        let probe = show(Form { CutoutPanel(document: doc, cutout: cutout) }.formStyle(.grouped))
        XCTAssertTrue(probe.has("Polygon, 3 points"), "\(probe.labels())")
        XCTAssertEqual(probe.textFields().count, 2, "X and Y only")
        XCTAssertTrue(probe.has("20 mm"), "its width: \(probe.labels())")
        XCTAssertTrue(probe.has("15 mm"), "its height: \(probe.labels())")
    }

    // MARK: Board Details

    private func sheet(_ doc: SchematicDocument, tab: BoardDetailsSheet.Tab, closed: @escaping () -> Void = {},
                       routed: @escaping (RoutingOptions.Scope) -> Void = { _ in }) -> ViewProbe {
        show(BoardDetailsSheet(document: doc, highlightedNet: .constant(nil), onRoute: routed, onClose: closed, tab: tab),
             size: NSSize(width: 640, height: 620))
    }

    func testThePlanesTabSaysWhyThereAreNone() throws {
        let two = TestSupport.newDocument()
        XCTAssertTrue(sheet(two, tab: .planes).has("A Plane is an inner layer"))
        let four = TestSupport.newDocument()
        four.setLayerCount(4)
        for layer in four.board.copperLayers where layer.isInner { four.setLayerType(layer, .wire) }
        XCTAssertTrue(PlanesSection.planes(of: four.board).isEmpty)
        XCTAssertTrue(sheet(four, tab: .planes).has("Set an inner layer's type to Plane"))
        four.setLayerType(.inner(2), .plane)
        XCTAssertFalse(PlanesSection.planes(of: four.board).isEmpty)
        let planes = sheet(four, tab: .planes)
        XCTAssertFalse(planes.has("No Plane layers"), "\(planes.labels())")
    }

    func testEveryTabShowsAndDoneCloses() throws {
        let doc = TestSupport.newDocument()
        var closes = 0
        for tab in [BoardDetailsSheet.Tab.classes, .groups, .stackup] {
            let probe = sheet(doc, tab: tab, closed: { closes += 1 })
            XCTAssertTrue(probe.has(Self.summary(doc)), "\(tab): \(probe.labels())")
            try probe.press("Done")
        }
        XCTAssertEqual(closes, 3)
        // A custom card and a card type with options each have a Card tab.
        doc.setBoardType(.customCard)
        let custom = sheet(doc, tab: .card)
        XCTAssertTrue(custom.has(BoardType.customCard.title), "\(custom.labels())")
        let withOptions = try XCTUnwrap(BoardType.allCases.first(where: \.hasOptions))
        doc.setBoardType(withOptions)
        let options = sheet(doc, tab: .card)
        XCTAssertTrue(options.has(withOptions.title), "\(options.labels())")
    }

    private static func summary(_ doc: SchematicDocument) -> String { BoardDetailsSheet.summary(of: doc.board) }

    func testRouteNetFromANetsMenuClosesTheSheetFirst() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 100, y: 400))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
        // A second LED on GND: two pads, one connection to route.
        let other = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 100))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(other, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
        var events: [String] = []
        let probe = sheet(doc, tab: .nets, closed: { events.append("close") }, routed: { events.append("route \($0)") })
        XCTAssertTrue(probe.wait { probe.has("GND") }, "\(probe.labels())")
        let row = try probe.element("GND")
        let menu = try XCTUnwrap(probe.contextMenu(on: row), "the net's menu")
        try probe.choose("Route Net", in: menu)
        _ = probe.wait { !events.isEmpty }
        XCTAssertEqual(events, ["close", "route \(RoutingOptions.Scope.nets([.ground]))"])
    }

    // MARK: Layers

    /// RGBA at (x, y) from the top left of `view` rendered at 1×.
    private func pixel<V: View>(_ view: V, _ x: Int, _ y: Int) throws -> [UInt8] {
        let renderer = ImageRenderer(content: view)
        renderer.scale = 1
        let image = try XCTUnwrap(renderer.cgImage)
        var bytes = [UInt8](repeating: 0, count: image.width * image.height * 4)
        let context = try XCTUnwrap(CGContext(data: &bytes, width: image.width, height: image.height,
                                              bitsPerComponent: 8, bytesPerRow: image.width * 4,
                                              space: CGColorSpaceCreateDeviceRGB(),
                                              bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
        context.draw(image, in: CGRect(x: 0, y: 0, width: image.width, height: image.height))
        let at = (y * image.width + x) * 4
        return Array(bytes[at..<at + 4])
    }

    func testErcMarkersAreRedOrYellowAndGrowWhenSelected() throws {
        let area = CGRect(x: 0, y: 0, width: 100, height: 100)
        let markers = [ErcMarker(point: CGPoint(x: 30, y: 50), severity: .error, key: "e"),
                       ErcMarker(point: CGPoint(x: 70, y: 50), severity: .warning, key: "w")]
        let plain = ErcMarkerLayer(markers: markers, area: area)
        let red = try pixel(plain, 30, 50)
        XCTAssertGreaterThan(red[0], 200)
        XCTAssertLessThan(red[2], 60)
        let yellow = try pixel(plain, 70, 50)
        XCTAssertGreaterThan(yellow[0], 200)
        XCTAssertGreaterThan(yellow[1], 150)
        XCTAssertLessThan(yellow[2], 80)
        // Below a small marker's base, inside a selected (larger) one.
        XCTAssertEqual(try pixel(plain, 30, 55)[3], 0)
        let selected = ErcMarkerLayer(markers: markers, selected: ["e"], area: area)
        XCTAssertGreaterThan(try pixel(selected, 30, 55)[0], 200)
        XCTAssertEqual(try pixel(selected, 70, 55)[3], 0, "only the selected line's marker grows")
    }

    func testNetLabelsAsDrawn() throws {
        let input = NetLabel(id: UUID(), name: "CLK", style: .input, origin: CGPoint(x: 20, y: 40))
        let flag = NetLabel(id: UUID(), name: "", style: .powerFlag, origin: CGPoint(x: 120, y: 60))
        // A Power Flag: its stem and pennant, inside its bounds.
        let points = CanvasSymbolRenderTests.RecordingPainter.points(of: flag.path)
        XCTAssertEqual(points.first, flag.origin)
        XCTAssertTrue(points.allSatisfy { flag.rect.insetBy(dx: -0.01, dy: -0.01).contains($0) }, "\(points)")
        XCTAssertEqual(flag.title, "Power Flag")
        var painter = CanvasSymbolRenderTests.RecordingPainter()
        NetLabelLayer.paint(&painter, labels: [input, flag], selectedIDs: [flag.id], hoveredID: input.id)
        XCTAssertTrue(painter.box.strokes.contains { $0.ink == Ink.accent.opacity(0.35) }, "the hovered underlay")
        XCTAssertTrue(painter.box.fills.contains(.accent), "the selected flag's pennant")
        XCTAssertEqual(painter.box.texts.map(\.string), ["CLK"], "a flag has no name")
        var plain = CanvasSymbolRenderTests.RecordingPainter()
        NetLabelLayer.paint(&plain, labels: [flag], selectedIDs: [], hoveredID: nil)
        XCTAssertEqual(plain.box.fills, [.primary])
        // The layer draws them on screen.
        let layer = NetLabelLayer(labels: [input, flag], area: CGRect(x: 0, y: 0, width: 200, height: 100))
        let stem = try pixel(layer, 120, 50)
        XCTAssertGreaterThan(stem[3], 0, "the flag's stem is drawn")
        XCTAssertEqual(try pixel(layer, 190, 95)[3], 0, "nothing far from them")
    }
}
