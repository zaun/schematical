import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AF: the Footprint step's drawings in pixels — a library
/// footprint's thumbnail (pads to scale, pin 1 in the accent colour, holes)
/// and the labelled sketch of each parametric style.
@MainActor
final class FootprintThumbnailTests: XCTestCase {
    override func setUp() async throws {
        // Closing a test window must not quit the test host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    /// An sRGB colour's 0–255 components.
    private struct RGB: Equatable, CustomStringConvertible {
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

        func near(_ other: RGB, _ tolerance: Int = 10) -> Bool {
            abs(r - other.r) <= tolerance && abs(g - other.g) <= tolerance && abs(b - other.b) <= tolerance
        }

        var description: String { "(\(r), \(g), \(b))" }
    }

    /// A rendered image: the colour at a point (top-left origin), nil where nothing opaque is drawn.
    private struct Bitmap {
        let width, height: Int
        let data: [UInt8]

        func at(_ x: CGFloat, _ y: CGFloat) -> RGB? {
            let px = Int(x), py = Int(y)
            guard px >= 0, py >= 0, px < width, py < height else { return nil }
            let i = (py * width + px) * 4
            return data[i + 3] > 200 ? RGB(r: Int(data[i]), g: Int(data[i + 1]), b: Int(data[i + 2])) : nil
        }

        /// Whether anything (a thin or translucent stroke too) is drawn within a pixel of the point.
        func inked(_ x: CGFloat, _ y: CGFloat) -> Bool {
            for dy in -1...1 {
                for dx in -1...1 {
                    let px = Int(x) + dx, py = Int(y) + dy
                    if px >= 0, py >= 0, px < width, py < height, data[(py * width + px) * 4 + 3] > 20 { return true }
                }
            }
            return false
        }

        var opaque: Int { stride(from: 3, to: data.count, by: 4).filter { data[$0] > 200 }.count }
    }

    private func render<V: View>(_ view: V, width: Int, height: Int) throws -> Bitmap {
        let renderer = ImageRenderer(content: view.frame(width: CGFloat(width), height: CGFloat(height)))
        renderer.scale = 1
        let image = try XCTUnwrap(renderer.cgImage)
        var data = [UInt8](repeating: 0, count: width * height * 4)
        let context = try XCTUnwrap(CGContext(data: &data, width: width, height: height, bitsPerComponent: 8,
                                              bytesPerRow: width * 4, space: CGColorSpace(name: CGColorSpace.sRGB)!,
                                              bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
        context.draw(image, in: CGRect(x: 0, y: 0, width: width, height: height))
        return Bitmap(width: width, height: height, data: data)
    }

    private func pad(_ designator: String?, x: Double, y: Double, w: Double, h: Double, drill: Double = 0,
                     shape: Footprint.PadShape = .rect, plated: Bool = true) -> Footprint.Pad {
        Footprint.Pad(designator: designator, centre: CGPoint(x: x, y: y), size: CGSize(width: w, height: h),
                      drill: drill, shape: shape, radius: 0, chamfer: 0, corners: [], plated: plated)
    }

    /// A 10 × 10 mm body with pads at the four corners: pin 2 a drilled circle,
    /// pin 3 an oval, pin 1 a rect, and an unplated hole.
    private var sample: Footprint {
        Footprint(id: "TEST-4", description: "four pads", body: CGSize(width: 10, height: 10), bodyRound: false,
                  pads: [
                      pad("2", x: 3, y: -3, w: 2, h: 2, drill: 1, shape: .round),
                      pad("1", x: -3, y: -3, w: 2, h: 2),
                      pad("3", x: 3, y: 3, w: 2, h: 1, shape: .round),
                      pad(nil, x: -3, y: 3, w: 2, h: 2, drill: 2, shape: .round, plated: false),
                  ])
    }

    // MARK: Thumbnail

    func testTheBoundsCoverTheBodyAndEveryPad() {
        var footprint = sample
        XCTAssertEqual(FootprintThumbnail.bounds(of: footprint), CGRect(x: -5, y: -5, width: 10, height: 10))
        footprint = Footprint(id: "WIDE", description: "", body: CGSize(width: 2, height: 2), bodyRound: true,
                              pads: [pad("1", x: 6, y: 0, w: 2, h: 2)])
        XCTAssertEqual(FootprintThumbnail.bounds(of: footprint), CGRect(x: -1, y: -1, width: 8, height: 2))
    }

    func testPinOneIsThePadOfPinOneElseTheFirst() {
        XCTAssertEqual(FootprintThumbnail.pin1Index(of: sample), 1)
        let unnumbered = Footprint(id: "N", description: "", body: .zero, bodyRound: false,
                                   pads: [pad("A", x: 0, y: 0, w: 1, h: 1), pad("B", x: 1, y: 0, w: 1, h: 1)])
        XCTAssertEqual(FootprintThumbnail.pin1Index(of: unnumbered), 0)
        let empty = Footprint(id: "E", description: "", body: CGSize(width: 1, height: 1), bodyRound: false, pads: [])
        XCTAssertNil(FootprintThumbnail.pin1Index(of: empty))
    }

    func testTheThumbnailDrawsPinOneCopperAndHoles() throws {
        // 230 px for 11.5 mm: 20 px per mm, centred at (115, 115).
        let bitmap = try render(FootprintThumbnail(footprint: sample), width: 230, height: 230)
        func point(_ x: Double, _ y: Double) -> (CGFloat, CGFloat) { (115 + x * 20, 115 + y * 20) }
        let accent = RGB(Color.accentColor), copper = RGB(PCBCanvasView.copper)
        let hole = RGB(PCBCanvasView.background)
        let pin1 = point(-3, -3), pin2 = point(3, -3), pin3 = point(3, 3), unplated = point(-3, 3)
        XCTAssertTrue(try XCTUnwrap(bitmap.at(pin1.0, pin1.1)).near(accent), "pin 1 \(String(describing: bitmap.at(pin1.0, pin1.1)))")
        // Pin 2: copper ring round a hole.
        XCTAssertTrue(try XCTUnwrap(bitmap.at(pin2.0 + 15, pin2.1)).near(copper), "pin 2's ring")
        XCTAssertTrue(try XCTUnwrap(bitmap.at(pin2.0, pin2.1)).near(hole), "pin 2's hole")
        // Pin 3, an oval: copper at its centre, nothing past its rounded end's corner.
        XCTAssertTrue(try XCTUnwrap(bitmap.at(pin3.0, pin3.1)).near(copper), "pin 3")
        XCTAssertNil(bitmap.at(pin3.0 + 19, pin3.1 + 9), "the oval's corner is cut")
        // The unplated hole is an outline: empty inside.
        XCTAssertNil(bitmap.at(unplated.0, unplated.1), "an unplated hole is not filled")
    }

    func testARoundBodyIsAnEllipse() throws {
        let round = Footprint(id: "R", description: "", body: CGSize(width: 10, height: 10), bodyRound: true,
                              pads: [pad("1", x: 0, y: 0, w: 1, h: 1)])
        let square = Footprint(id: "S", description: "", body: CGSize(width: 10, height: 10), bodyRound: false,
                               pads: [pad("1", x: 0, y: 0, w: 1, h: 1)])
        // The corner of the body outline: drawn for a square body, not a round one.
        let corner = (115.0 - 100, 115.0 - 100)
        let r = try render(FootprintThumbnail(footprint: round), width: 230, height: 230)
        let s = try render(FootprintThumbnail(footprint: square), width: 230, height: 230)
        XCTAssertTrue(s.inked(corner.0, corner.1), "the square body's corner")
        XCTAssertFalse(r.inked(corner.0, corner.1), "a round body has no corner")
        XCTAssertTrue(r.inked(115, 15), "the round body's top")
    }

    func testAFootprintWithNoSizeDrawsNothing() throws {
        let flat = Footprint(id: "F", description: "", body: .zero, bodyRound: false, pads: [])
        XCTAssertEqual(try render(FootprintThumbnail(footprint: flat), width: 50, height: 50).opaque, 0)
    }

    func testTheThumbnailIsLabelled() throws {
        let probe = ViewProbe(FootprintThumbnail(footprint: sample).frame(width: 100, height: 100),
                              size: NSSize(width: 120, height: 120))
        defer { probe.close() }
        XCTAssertTrue(probe.has("Drawing of TEST-4"), "\(probe.labels())")
    }

    // MARK: Diagram

    func testEachSketchNamesItsDimensions() {
        XCTAssertEqual(FootprintDiagram.dimensions(.dual), ["Pitch", "Row spacing", "Pad length", "Pad width"])
        XCTAssertEqual(FootprintDiagram.dimensions(.quad),
                       ["Pitch", "Spacing across", "Spacing down", "Pad length", "Pad width"])
        XCTAssertEqual(FootprintDiagram.dimensions(.grid), ["Pitch", "Ball pad"])
        XCTAssertEqual(FootprintDiagram.dimensions(.header), ["Pitch", "Row pitch", "Pad", "Drill"])
    }

    /// Each sketch at 320 × 170: pin 1's pad in the accent colour, the pad beside it copper.
    func testEachSketchMarksPinOne() throws {
        let accent = RGB(Color.accentColor), copper = RGB(PCBCanvasView.copper)
        let cx = 160.0, cy = 89.0
        // (pin 1's centre, another pad's centre) per style, from the sketch's own numbers.
        let points: [(ParametricFootprintDraft.Style, (Double, Double), (Double, Double))] = [
            (.dual, (cx - 55, cy - 39), (cx + 55, cy - 39)),
            (.quad, (cx - 52, cy - 30), (cx + 52, cy - 30)),
            (.grid, (cx - 45, cy - 30), (cx - 15, cy - 30)),
            (.header, (cx - 51 - 7, cy - 17), (cx - 17 - 7, cy - 17)),
        ]
        for (style, first, other) in points {
            let bitmap = try render(FootprintDiagram(style: style), width: 320, height: 170)
            let a = try XCTUnwrap(bitmap.at(first.0, first.1), "\(style): nothing at pin 1")
            XCTAssertTrue(a.near(accent), "\(style): pin 1 is \(a), not \(accent)")
            let b = try XCTUnwrap(bitmap.at(other.0, other.1), "\(style): nothing at the other pad")
            XCTAssertTrue(b.near(copper), "\(style): the other pad is \(b), not \(copper)")
        }
        // The header's holes are the board's colour.
        let header = try render(FootprintDiagram(style: .header), width: 320, height: 170)
        XCTAssertTrue(try XCTUnwrap(header.at(cx - 51, cy - 17)).near(RGB(PCBCanvasView.background)), "the hole")
    }

    func testTheSketchIsLabelled() throws {
        let probe = ViewProbe(FootprintDiagram(style: .quad).frame(width: 320), size: NSSize(width: 340, height: 200))
        defer { probe.close() }
        XCTAssertTrue(probe.has("Sketch of a four sides footprint with its dimensions"), "\(probe.labels())")
    }
}
