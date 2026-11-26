import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// How the PCB page draws (`PCBCanvasView.draw`): rendered into a bitmap
/// (no offset, 10 points per mm at the default 50 % zoom), checked at points
/// the board's geometry gives, against the colour expected there or against
/// the same board drawn without the feature.
@MainActor
final class PCBCanvasDrawingTests: XCTestCase {
    private static let size = CGSize(width: 800, height: 600)

    /// An sRGB colour with alpha, 0–255.
    private struct RGBA: CustomStringConvertible {
        let r, g, b, a: Int

        init(_ color: Color) {
            let ns = NSColor(color).usingColorSpace(.sRGB)!
            r = Int((ns.redComponent * 255).rounded())
            g = Int((ns.greenComponent * 255).rounded())
            b = Int((ns.blueComponent * 255).rounded())
            a = Int((ns.alphaComponent * 255).rounded())
        }

        init(r: Int, g: Int, b: Int, a: Int) {
            self.r = r; self.g = g; self.b = b; self.a = a
        }

        func near(_ other: RGBA, _ tolerance: Int = 16) -> Bool {
            abs(r - other.r) <= tolerance && abs(g - other.g) <= tolerance && abs(b - other.b) <= tolerance
                && abs(a - other.a) <= tolerance
        }

        var description: String { "(\(r) \(g) \(b) \(a))" }
    }

    /// A drawn picture, read in board mm.
    private struct Picture {
        let data: [UInt8]
        let width: Int, height: Int
        /// Points per mm.
        let scale: CGFloat

        func at(_ x: CGFloat, _ y: CGFloat) -> RGBA {
            let px = Int(x * scale), py = Int(y * scale)
            guard px >= 0, py >= 0, px < width, py < height else { return RGBA(r: 0, g: 0, b: 0, a: 0) }
            let i = (py * width + px) * 4
            // Premultiplied: undo it for a comparable colour.
            let a = Int(data[i + 3])
            func un(_ v: UInt8) -> Int { a == 0 ? 0 : min(255, Int(v) * 255 / a) }
            return RGBA(r: un(data[i]), g: un(data[i + 1]), b: un(data[i + 2]), a: a)
        }

        /// Whether any pixel in `rect` (mm) is near `colour`.
        func any(in rect: CGRect, near colour: RGBA, _ tolerance: Int = 16) -> Bool {
            any(in: rect) { at($0, $1).near(colour, tolerance) }
        }

        /// Whether any pixel in `rect` (mm) is far from `other`'s there.
        func differs(from other: Picture, in rect: CGRect, by tolerance: Int = 24) -> Bool {
            any(in: rect) { !at($0, $1).near(other.at($0, $1), tolerance) }
        }

        private func any(in rect: CGRect, _ test: (CGFloat, CGFloat) -> Bool) -> Bool {
            var y = rect.minY
            while y <= rect.maxY {
                var x = rect.minX
                while x <= rect.maxX {
                    if test(x, y) { return true }
                    x += 1 / scale
                }
                y += 1 / scale
            }
            return false
        }
    }

    private func draw(_ view: PCBCanvasView, press: PCBCanvasView.DrawState = PCBCanvasView.DrawState()) throws -> Picture {
        let size = Self.size
        let canvas = Canvas { context, size in view.draw(in: &context, size: size, press: press) }
            .frame(width: size.width, height: size.height)
        let renderer = ImageRenderer(content: canvas)
        renderer.scale = 1
        let image = try XCTUnwrap(renderer.cgImage)
        let width = Int(size.width), height = Int(size.height)
        var data = [UInt8](repeating: 0, count: width * height * 4)
        let space = try XCTUnwrap(CGColorSpace(name: CGColorSpace.sRGB))
        let context = try XCTUnwrap(CGContext(data: &data, width: width, height: height, bitsPerComponent: 8,
                                              bytesPerRow: width * 4, space: space,
                                              bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
        context.draw(image, in: CGRect(x: 0, y: 0, width: width, height: height))
        return Picture(data: data, width: width, height: height, scale: view.scale)
    }

    private func canvas(_ doc: SchematicDocument, tool: PCBTool = .select, visible: BoardVisibility = BoardVisibility(),
                        zoom: CGFloat = 0.5, highlighted: PlaneNet? = nil,
                        routePreview: (traces: [RoutedTrace], vias: [RoutedVia])? = nil, assist: Bool = false,
                        exporting: Bool = false, clearsHoles: Bool = false) -> PCBCanvasView {
        PCBCanvasView(document: doc, viewport: CanvasViewport(zoom: zoom), tool: tool, visible: visible,
                      traceAssist: assist, highlightedNet: .constant(highlighted), routePreview: routePreview,
                      exporting: exporting, clearsHoles: clearsHoles)
    }

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    private func newBoard() -> SchematicDocument {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 70, height: 50)
        return doc
    }

    private func layerColour(_ doc: SchematicDocument, _ layer: CopperLayer) -> RGBA {
        RGBA(doc.board.style(of: layer).color.color)
    }

    /// Two LEDs wired K to A, apart on the board, and their net's key.
    private func pair(_ doc: SchematicDocument) throws -> (k: CGPoint, a: CGPoint, key: PlaneNet) {
        let one = try TestSupport.place(doc, "GENERIC-LED", at: p(100, 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: p(400, 200))
        doc.moveFootprint(one.id, to: p(20, 25))
        doc.moveFootprint(two.id, to: p(50, 25))
        let kRef = try TestSupport.pin(one, "K"), aRef = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(kRef), .pin(aRef)))
        func centre(_ ref: PinRef) throws -> CGPoint {
            let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
            let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == chip.part.pins[ref.index].designator })
            return p(pad.rect.midX, pad.rect.midY)
        }
        return (try centre(kRef), try centre(aRef), .pin(chip: one.id, designator: one.part.pins[kRef.index].designator))
    }

    // MARK: Board and grid

    /// The board's fill, its grid dots (not in an export, nor when they'd
    /// be closer than 8 points), nothing off the board, and only the
    /// outline with the board layer hidden.
    func testBoardAndGrid() throws {
        let doc = newBoard()
        let fill = RGBA(doc.board.color.fill)
        let dot = RGBA(Color(white: 0.42))
        let step = doc.board.grid * 4
        let picture = try draw(canvas(doc))
        XCTAssertTrue(picture.at(step * 3 + step / 2, step * 3 + step / 2).near(fill), "\(picture.at(15, 15))")
        XCTAssertTrue(picture.at(step * 3, step * 3).near(dot), "\(picture.at(step * 3, step * 3))")
        XCTAssertEqual(picture.at(75, 55).a, 0, "off the board")

        let export = try draw(canvas(doc, exporting: true))
        XCTAssertTrue(export.at(step * 3, step * 3).near(fill), "no grid in an export")
        // A 0.25 mm grid's dots are 1 mm apart: 5 points at 25 %, too close to draw.
        doc.setBoardGrid(0.25)
        let far = try draw(canvas(doc, zoom: 0.25))
        XCTAssertTrue(far.at(10, 10).near(fill), "dots too close to draw: \(far.at(10, 10))")
        doc.setBoardGrid(step / 4)

        var hidden = BoardVisibility()
        hidden.hidden = [.board]
        let outline = try draw(canvas(doc, visible: hidden))
        XCTAssertEqual(outline.at(step * 3 + step / 2, step * 3 + step / 2).a, 0, "no fill")
        XCTAssertTrue(outline.any(in: CGRect(x: 69.5, y: 20, width: 1, height: 1), near: RGBA(doc.board.color.silkscreen)),
                      "the outline still")
    }

    // MARK: Traces

    /// Traces in their layer's colour at their width; a selected one gets
    /// an accent underlay, the rest of its run a fainter one; a hidden
    /// layer's traces aren't drawn.
    func testTracesAndTheirSelection() throws {
        let doc = newBoard()
        let one = try XCTUnwrap(doc.addTrace(from: p(10.16, 10.16), to: p(30.48, 10.16), layer: .top, diagonalFirst: false))
        XCTAssertNotNil(doc.addTrace(from: p(30.48, 10.16), to: p(30.48, 30.48), layer: .top, diagonalFirst: false))
        XCTAssertNotNil(doc.addTrace(from: p(10.16, 40.64), to: p(30.48, 40.64), layer: .bottom, diagonalFirst: false))
        let width = try XCTUnwrap(doc.traces.first).widthMM
        let plain = try draw(canvas(doc))
        XCTAssertTrue(plain.at(20.32, 10.16).near(layerColour(doc, .top)), "\(plain.at(20.32, 10.16))")
        XCTAssertTrue(plain.at(20.32, 40.64).near(layerColour(doc, .bottom)))
        let edge = 10.16 + width / 2 + 0.12
        XCTAssertTrue(plain.at(20.32, edge).near(RGBA(doc.board.color.fill)), "no underlay")

        doc.select(.trace(one))
        let selected = try draw(canvas(doc))
        XCTAssertTrue(selected.at(20.32, edge).near(RGBA(Color.accentColor), 12), "\(selected.at(20.32, edge))")
        let joined = selected.at(30.48 + width / 2 + 0.12, 20.32)
        XCTAssertFalse(joined.near(RGBA(doc.board.color.fill), 4), "the run's fainter underlay: \(joined)")
        XCTAssertFalse(joined.near(RGBA(Color.accentColor), 12))
        let export = try draw(canvas(doc, exporting: true))
        XCTAssertTrue(export.at(20.32, edge).near(RGBA(doc.board.color.fill)), "no selection in an export")

        var hidden = BoardVisibility()
        hidden.hidden = [.copper(.top)]
        let without = try draw(canvas(doc, visible: hidden))
        XCTAssertFalse(without.at(20.32, 10.16).near(layerColour(doc, .top)))
    }

    // MARK: Vias, holes, pads

    /// A via is copper round its drill, the drill the canvas colour, or cut
    /// through (`clearsHoles`); a selected via or hole has an accent ring.
    func testViasAndHoles() throws {
        let doc = newBoard()
        let via = try XCTUnwrap(doc.addVia(at: p(5.08, 5.08)))
        let hole = doc.addHole(at: p(13.97, 5.08))
        let v = try XCTUnwrap(doc.vias.first)
        let ring = v.centre.x + (v.drill + v.diameter) / 4
        doc.select(.trace(UUID()))
        let picture = try draw(canvas(doc, zoom: 2))
        XCTAssertTrue(picture.at(ring, v.centre.y).near(RGBA(PCBCanvasView.copper)), "\(picture.at(ring, v.centre.y))")
        XCTAssertTrue(picture.at(v.centre.x, v.centre.y).near(RGBA(PCBCanvasView.background)))
        let h = try XCTUnwrap(doc.holes.first)
        XCTAssertTrue(picture.at(h.centre.x, h.centre.y).near(RGBA(PCBCanvasView.background)))
        let cut = try draw(canvas(doc, zoom: 2, exporting: true, clearsHoles: true))
        XCTAssertEqual(cut.at(v.centre.x, v.centre.y).a, 0, "cut through")

        doc.select(.via(via))
        let ringed = try draw(canvas(doc, zoom: 2))
        // The ring is 3 points (0.075 mm at 40 points per mm) outside.
        let out = v.centre.x + v.diameter / 2 + 0.075
        XCTAssertTrue(ringed.differs(from: picture, in: CGRect(x: out - 0.04, y: v.centre.y - 0.03, width: 0.08, height: 0.06)),
                      "the via's ring")
        doc.select(.hole(hole))
        let holed = try draw(canvas(doc, zoom: 2))
        let edge = h.rect.maxX + 0.075
        XCTAssertTrue(holed.differs(from: picture, in: CGRect(x: edge - 0.04, y: h.centre.y - 0.03, width: 0.08, height: 0.06)),
                      "the hole's ring")

        var noCopper = BoardVisibility()
        noCopper.hidden = Set(doc.board.copperLayers.map { .copper($0) })
        let bare = try draw(canvas(doc, visible: noCopper, zoom: 2))
        XCTAssertFalse(bare.at(ring, v.centre.y).near(RGBA(PCBCanvasView.copper)), "no copper shown: no via")
    }

    /// A surface-mount part's pads in its side's copper colour (Bottom's on
    /// the bottom), a through-hole part's in copper round its drill; a
    /// selected footprint gets a box; its marks are drawn.
    func testFootprintPadsMarksAndSelection() throws {
        let doc = newBoard()
        let smd = try TestSupport.place(doc, "SN74HC00D", at: p(100, 100))
        let tht = try TestSupport.place(doc, "SN74HC00N", at: p(300, 100))
        doc.moveFootprint(smd.id, to: p(10, 10))
        doc.moveFootprint(tht.id, to: p(30, 15))
        func pad(_ id: UUID) throws -> PlacedPad {
            try XCTUnwrap(doc.chips.first { $0.id == id }?.boardPads.first)
        }
        let picture = try draw(canvas(doc, zoom: 0.75))
        let s = try pad(smd.id), t = try pad(tht.id)
        XCTAssertTrue(picture.at(s.rect.midX, s.rect.midY).near(layerColour(doc, .top)), "\(picture.at(s.rect.midX, s.rect.midY))")
        let land = t.rect.midY + (t.rect.height / 2 + t.drill / 2) / 2
        XCTAssertTrue(picture.at(t.rect.midX, land).near(RGBA(PCBCanvasView.copper)), "\(picture.at(t.rect.midX, land))")
        XCTAssertTrue(picture.at(t.rect.midX, t.rect.midY).near(RGBA(PCBCanvasView.background)))
        let chip = try XCTUnwrap(doc.chips.first { $0.id == smd.id })
        let body = try XCTUnwrap(chip.boardBody)
        let colour = doc.board.color
        if let ink = try XCTUnwrap(chip.board).outline.color(on: colour) {
            XCTAssertTrue(picture.any(in: CGRect(x: body.rect.minX - 0.15, y: body.rect.midY - 0.3, width: 0.3, height: 0.6),
                                      near: RGBA(ink), 30), "the body outline")
        }

        doc.select(.footprint(smd.id))
        let boxed = try draw(canvas(doc, zoom: 0.75))
        let bounds = try XCTUnwrap(chip.boardBounds)
        // 3 points (0.2 mm at 15 points per mm) outside the part.
        XCTAssertTrue(boxed.differs(from: picture, in: CGRect(x: bounds.minX - 0.27, y: bounds.midY - 0.2, width: 0.14,
                                                              height: 0.4)), "the selection box")

        doc.setFootprintSide(smd.id, .bottom)
        let flipped = try draw(canvas(doc, zoom: 0.75))
        let b = try pad(smd.id)
        XCTAssertTrue(flipped.at(b.rect.midX, b.rect.midY).near(layerColour(doc, .bottom)), "\(flipped.at(b.rect.midX, b.rect.midY))")
    }

    /// A diode's body outline has its cathode band across it.
    func testADiodesBandIsDrawn() throws {
        let doc = newBoard()
        let diode = try TestSupport.place(doc, "1N4001-E3/54", at: p(100, 100))
        doc.moveFootprint(diode.id, to: p(10, 10))
        let chip = try XCTUnwrap(doc.chips.first { $0.id == diode.id })
        let band = try XCTUnwrap(chip.cathodeBand)
        let mid = p((band.a.x + band.b.x) / 2, (band.a.y + band.b.y) / 2)
        let picture = try draw(canvas(doc, zoom: 1))
        let hidden = try draw(canvas(doc, visible: BoardVisibility(hidden: [.document, .silkscreen]), zoom: 1))
        XCTAssertTrue(picture.differs(from: hidden, in: CGRect(x: mid.x - 0.1, y: mid.y - 0.1, width: 0.2, height: 0.2)),
                      "the band")
    }

    /// A card's gold fingers on their side, and an add-on board's header pads
    /// and holes.
    func testCardFingersAndHeaderPads() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.isa8)
        let finger = try XCTUnwrap(doc.board.fingers.first { !$0.isThroughHole && $0.side == .top })
        let picture = try draw(canvas(doc, zoom: 0.25))
        let centre = p(finger.rect.midX, finger.rect.midY)
        XCTAssertTrue(picture.at(centre.x, centre.y).near(layerColour(doc, .top)), "\(picture.at(centre.x, centre.y))")

        let shield = TestSupport.newDocument()
        shield.setBoardType(.arduinoShield)
        let header = try XCTUnwrap(shield.board.fingers.first { $0.isThroughHole })
        let drawn = try draw(canvas(shield))
        XCTAssertTrue(drawn.at(header.rect.midX, header.rect.midY).near(RGBA(PCBCanvasView.background)), "the drill")
        let land = header.rect.midY + (header.rect.height / 2 + header.drill / 2) / 2
        XCTAssertTrue(drawn.at(header.rect.midX, land).near(RGBA(PCBCanvasView.copper)), "\(drawn.at(header.rect.midX, land))")
    }

    // MARK: Pours, keep-outs, planes

    /// A pour's dashed outline in its layer's colour (not in an export); a
    /// keep-out in red; a selected pour's handles in white.
    func testPoursKeepoutsAndHandles() throws {
        let doc = newBoard()
        let pour = try XCTUnwrap(doc.addPour(rect: CGRect(x: 5, y: 5, width: 20, height: 20), layer: .top))
        _ = doc.addKeepout(rect: CGRect(x: 40, y: 5, width: 20, height: 20))
        let rect = try XCTUnwrap(doc.pours.first).rect
        doc.select(.trace(UUID()))
        let picture = try draw(canvas(doc))
        let export = try draw(canvas(doc, exporting: true))
        let empty = newBoard()
        let bare = try draw(canvas(empty))
        let keepout = try XCTUnwrap(doc.keepouts.first).rect
        let pourEdge = CGRect(x: rect.minX - 0.1, y: rect.minY + 0.3, width: 0.2, height: 2)
        let keepoutEdge = CGRect(x: keepout.minX - 0.1, y: keepout.minY + 0.3, width: 0.2, height: 2)
        XCTAssertTrue(picture.differs(from: bare, in: pourEdge), "the pour's outline")
        XCTAssertTrue(picture.differs(from: bare, in: keepoutEdge), "the keep-out's outline")
        XCTAssertTrue(picture.differs(from: bare, in: CGRect(x: keepout.midX - 0.3, y: keepout.midY - 0.3, width: 0.6, height: 0.6)),
                      "the keep-out's hatch")
        XCTAssertFalse(export.differs(from: try draw(canvas(empty, exporting: true)), in: keepoutEdge),
                       "no keep-outs in an export")

        doc.select(.pour(pour))
        let handles = try draw(canvas(doc))
        XCTAssertTrue(handles.at(rect.minX, rect.minY).near(RGBA(Color.white)), "\(handles.at(rect.minX, rect.minY))")
        XCTAssertTrue(handles.differs(from: picture, in: CGRect(x: rect.minX + rect.width / 4, y: rect.minY - 0.45,
                                                                width: 1, height: 0.3)), "the selection box")
    }

    /// A ground pour fills in its layer's colour; a Plane layer tints the
    /// board, more strongly with the board layer hidden.
    func testPourFillAndPlane() throws {
        let doc = newBoard()
        _ = try pair(doc)
        let gnd = try TestSupport.place(doc, "GND", at: p(100, 400))
        let led = try XCTUnwrap(doc.chips.first)
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "A")), .pin(PinRef(chipID: gnd.id, index: 0))))
        let before = try draw(canvas(doc))
        // Round the grounded LED, so its fill is kept.
        let pour = try XCTUnwrap(doc.addPour(rect: CGRect(x: 8, y: 16, width: 26, height: 18), layer: .bottom))
        XCTAssertEqual(doc.pours.first?.net, .ground)
        XCTAssertEqual(doc.connectivity().pourFills[pour]?.isEmpty, false, "the pour has a fill")
        let picture = try draw(canvas(doc))
        XCTAssertTrue(picture.differs(from: before, in: CGRect(x: 10.3, y: 31.3, width: 1, height: 1)), "the pour's fill")

        doc.setLayerCount(4)
        doc.setLayerType(.inner(2), .plane)
        XCTAssertTrue(doc.board.copperLayers.contains { BoardVisibility().showsPlane($0, style: doc.board.style(of: $0)) })
        let plane = try draw(canvas(doc))
        XCTAssertTrue(plane.differs(from: picture, in: CGRect(x: 44.5, y: 10.5, width: 1, height: 1), by: 6), "the plane's tint")
        var hidden = BoardVisibility()
        hidden.hidden = [.board]
        let bare = try draw(canvas(doc, visible: hidden))
        XCTAssertGreaterThan(bare.at(45, 10).a, 0, "the plane shows without the board")
    }

    // MARK: Ratsnest, highlight, route preview

    /// An unrouted connection is a red line; a highlighted net dims the rest
    /// and draws its pads white; an Autoroute preview draws at 60 %.
    func testRatsnestHighlightAndRoutePreview() throws {
        let doc = newBoard()
        let (k, a, key) = try pair(doc)
        let middle = p((k.x + a.x) / 2, (k.y + a.y) / 2)
        let red = RGBA(Color.red)
        let picture = try draw(canvas(doc))
        var noLines = BoardVisibility()
        noLines.hidden = [.ratsnest]
        let unlined = try draw(canvas(doc, visible: noLines))
        let around = CGRect(x: middle.x - 0.3, y: middle.y - 0.3, width: 0.6, height: 0.6)
        XCTAssertTrue(picture.differs(from: unlined, in: around), "the unrouted line")
        XCTAssertTrue(picture.any(in: around, near: red, 40), "in red")

        let lit = try draw(canvas(doc, highlighted: key))
        let fill = RGBA(doc.board.color.fill)
        XCTAssertFalse(lit.at(5, 45).near(fill, 4), "dimmed: \(lit.at(5, 45))")
        XCTAssertTrue(lit.at(k.x, k.y - 0.5).near(RGBA(Color.white), 24), "the net's pad: \(lit.at(k.x, k.y - 0.5))")
        XCTAssertTrue(try draw(canvas(doc, highlighted: .rail(mv: 1234))).at(5, 45).near(fill), "no such net: no dimming")
        XCTAssertTrue(try draw(canvas(doc, highlighted: key, exporting: true)).at(5, 45).near(fill), "not in an export")
        XCTAssertNotNil(doc.addTrace(from: k, to: a, layer: .top, diagonalFirst: false))
        // A via on the net's trace, a little way along it.
        XCTAssertNotNil(doc.addVia(at: p(middle.x + 5.08, middle.y)))
        let routed = try draw(canvas(doc, visible: noLines, highlighted: key))
        XCTAssertTrue(routed.at(middle.x, middle.y).near(RGBA(Color.white), 24), "the net's trace: \(routed.at(middle.x, middle.y))")

        let preview = (traces: [RoutedTrace(layer: .bottom, width: 0.5, points: [p(5, 40), p(30, 40)])],
                       vias: [RoutedVia(centre: p(40, 40), drill: 0.4, diameter: 0.8)])
        let shown = try draw(canvas(doc, routePreview: preview))
        let colour = shown.at(15, 40)
        XCTAssertFalse(colour.near(fill, 4), "the preview trace: \(colour)")
        XCTAssertFalse(shown.at(40, 40).near(fill, 4), "the preview via")
    }

    // MARK: Tool previews

    /// The Trace tool's live leg in its layer's colour with a green ring at
    /// its end; a Pour drag's dashed rectangle; a Cutout drag's shape.
    func testToolPreviews() throws {
        let doc = newBoard()
        let draft = TraceDraft(start: p(10.16, 10.16))
        let press = PCBCanvasView.DrawState(interaction: .drawTrace(draft: draft, cursor: p(30.48, 10.16)),
                                            drawingLayer: .top)
        let picture = try draw(canvas(doc, tool: .trace), press: press)
        let none = try draw(canvas(doc, tool: .trace))
        XCTAssertTrue(picture.differs(from: none, in: CGRect(x: 21, y: 10, width: 0.3, height: 0.3)), "the live leg")
        XCTAssertTrue(picture.any(in: CGRect(x: 30.48 + 0.4, y: 10.0, width: 0.4, height: 0.3), near: RGBA(Color.green), 40),
                      "the ring")
        let other = try draw(canvas(doc), press: press)
        XCTAssertFalse(other.differs(from: none, in: CGRect(x: 21, y: 10, width: 0.3, height: 0.3)), "another tool: no preview")

        let pour = PCBCanvasView.DrawState(interaction: .drawRect(kind: .pour, from: p(5, 30), to: p(25, 45)))
        let rect = try draw(canvas(doc, tool: .pour), press: pour)
        XCTAssertTrue(rect.differs(from: none, in: CGRect(x: 4.9, y: 30.3, width: 0.2, height: 3)), "the dashed box")
        let cutout = PCBCanvasView.DrawState(interaction: .drawRect(kind: .cutout, from: p(40, 30), to: p(60, 45)))
        let shape = try draw(PCBCanvasView(document: doc, viewport: CanvasViewport(zoom: 0.5), tool: .cutout,
                                           cutoutShape: .circle), press: cutout)
        // A circle as wide as the longer side (20 mm, centred at 50, 40):
        // drawn round its left side, not at the dragged box's corner.
        XCTAssertTrue(shape.differs(from: none, in: CGRect(x: 39.8, y: 38, width: 0.4, height: 4)), "the circle's side")
        XCTAssertFalse(shape.differs(from: none, in: CGRect(x: 40.5, y: 30.3, width: 0.5, height: 0.5)), "not the box's corner")
    }

    /// With the assist and no legal way, the leg is red and the end a red
    /// cross; the ratsnest then counts no drawn trace.
    func testIllegalAssistedLegIsRed() throws {
        let doc = newBoard()
        let (k, a, _) = try pair(doc)
        // Another net's trace straight across the way.
        XCTAssertNotNil(doc.addTrace(from: p(35, 5), to: p(35, 48), layer: .top, diagonalFirst: false))
        let assist = AssistRouter(document: doc, layer: .top, start: k)
        let draft = TraceDraft(start: k)
        let blocked = p(35, 20)
        let press = PCBCanvasView.DrawState(interaction: .drawTrace(draft: draft, cursor: blocked), assist: assist,
                                            drawingLayer: .top)
        let (_, legal) = TraceDrawing.assisted(doc, draft: draft, to: blocked, layer: .top, scale: 10, angle: 45,
                                               assist: assist)
        XCTAssertFalse(legal, "the leg can't end on another net's trace")
        let picture = try draw(canvas(doc, tool: .trace), press: press)
        let end = TraceDrawing.anchor(doc, at: blocked, layer: .top, scale: 10)
        XCTAssertTrue(picture.any(in: CGRect(x: end.x - 0.6, y: end.y - 0.6, width: 1.2, height: 1.2), near: RGBA(Color.red), 30),
                      "the red cross")
        _ = a
    }

    // MARK: Selection, texts

    /// A selected text, keep-out and cutout each get a box; texts on both
    /// sides are drawn; an item on a hidden layer has no box.
    func testSelectionBoxesAndTexts() throws {
        let doc = newBoard()
        let top = doc.addText(at: p(10, 10))
        let bottom = doc.addText(at: p(10, 30))
        doc.setSide(of: [.text(bottom)], .bottom)
        let keepout = doc.addKeepout(rect: CGRect(x: 40, y: 5, width: 10, height: 10))
        let cutout = try XCTUnwrap(doc.addCutout(CutoutShape.rectangle.shape(from: p(40, 30), to: p(50, 40))))
        // A trace too, so the traces are drawn with other items selected.
        XCTAssertNotNil(doc.addTrace(from: p(25.4, 5.08), to: p(25.4, 15.24), layer: .top, diagonalFirst: false))
        let accent = RGBA(Color.accentColor)
        let fill = RGBA(doc.board.color.fill)
        let plain = try draw(canvas(doc))
        for id in [top, bottom] {
            let rect = try XCTUnwrap(doc.texts.first { $0.id == id }?.boardRect)
            var differs = false
            var x = rect.minX
            while x < rect.maxX, !differs {
                differs = !plain.at(x, rect.midY).near(fill, 20)
                x += 0.1
            }
            XCTAssertTrue(differs, "the text is drawn")
        }
        for (item, rect) in [(BoardItem.text(top), try XCTUnwrap(doc.texts.first { $0.id == top }?.boardRect)),
                             (.keepout(keepout), CGRect(x: 40, y: 5, width: 10, height: 10)),
                             (.cutout(cutout), CGRect(x: 40, y: 30, width: 10, height: 10))] {
            doc.select(item)
            let boxed = try draw(canvas(doc))
            XCTAssertTrue(boxed.any(in: CGRect(x: rect.midX - 0.5, y: rect.minY - 0.45, width: 1, height: 0.3), near: accent, 12),
                          "\(item)")
        }
        var hidden = BoardVisibility()
        hidden.hidden = [.copper(.top)]
        let pour = try XCTUnwrap(doc.addPour(rect: CGRect(x: 5, y: 40, width: 10, height: 8), layer: .top))
        doc.select(.pour(pour))
        let rect = try XCTUnwrap(doc.pours.first { $0.id == pour }).rect
        let none = try draw(canvas(doc, visible: hidden))
        XCTAssertFalse(none.any(in: CGRect(x: rect.minX + rect.width / 4 - 0.3, y: rect.minY - 0.45, width: 0.6, height: 0.3),
                                near: accent, 12), "a pour on a hidden layer has no box")
        doc.select(.trace(UUID()))
        _ = try draw(canvas(doc))
    }

    /// The export draws as the page, without the press.
    func testDrawExportIsThePageWithoutAPress() throws {
        let doc = newBoard()
        XCTAssertNotNil(doc.addTrace(from: p(10.16, 10.16), to: p(30.48, 10.16), layer: .top, diagonalFirst: false))
        let view = canvas(doc, tool: .pour, exporting: true)
        let page = try draw(view)
        let pressed = try draw(view, press: PCBCanvasView.DrawState(interaction: .drawRect(kind: .pour, from: p(5, 30),
                                                                                           to: p(25, 45))))
        let canvasView = Canvas { context, size in view.drawExport(in: &context, size: size) }
            .frame(width: Self.size.width, height: Self.size.height)
        let renderer = ImageRenderer(content: canvasView)
        renderer.scale = 1
        let image = try XCTUnwrap(renderer.cgImage)
        let width = Int(Self.size.width), height = Int(Self.size.height)
        var data = [UInt8](repeating: 0, count: width * height * 4)
        let space = try XCTUnwrap(CGColorSpace(name: CGColorSpace.sRGB))
        let context = try XCTUnwrap(CGContext(data: &data, width: width, height: height, bitsPerComponent: 8,
                                              bytesPerRow: width * 4, space: space,
                                              bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
        context.draw(image, in: CGRect(x: 0, y: 0, width: width, height: height))
        XCTAssertEqual(data, page.data, "the export is the page")
        XCTAssertNotEqual(page.data, pressed.data, "a press would show")
    }
}
