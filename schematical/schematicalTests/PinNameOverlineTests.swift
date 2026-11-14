import SwiftUI
import XCTest
@testable import schematical

/// Records every `PaintedText` a symbol draws (`RecordingPainter` keeps only
/// the kind and ink).
private struct TextRecordingPainter: SchematicPainter {
    nonisolated final class Record {
        var texts: [PaintedText] = []
    }

    let record = Record()

    mutating func fill(_ path: Path, with ink: Ink) {}
    mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle) {}
    mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint) {
        record.texts.append(text)
    }
    mutating func translateBy(x: CGFloat, y: CGFloat) {}
    mutating func rotate(by angle: Angle) {}
}

/// The overline over active-low pin names (plan-39-C Phase 2).
@MainActor
final class PinNameOverlineTests: XCTestCase {
    private static let store: PartCatalogStore = {
        let store = PartCatalogStore()
        store.load()
        return store
    }()

    /// (a) A flagged block chip's names carry the overline ranges; an
    /// unflagged pin's name has none and is drawn as written.
    func testBlockChipPaintsOverlinedNames() throws {
        let part = try Self.store.register(
            fileText: ChipPinActiveLowTests.fileText(uuid: "C39C0000-0000-4000-8000-000000000001", name: "LOW-C"),
            source: "low-c.schpart")
        var painter = TextRecordingPainter()
        ChipBlockView.paint(&painter, chip: PartPreview.chip(for: part), isSelected: false, isHovered: false)
        let byString = Dictionary(painter.record.texts.map { ($0.string, $0) }, uniquingKeysWith: { first, _ in first })

        XCTAssertNil(byString["CE#"], "a flagged CE# draws without its #")
        XCTAssertEqual(byString["CE"]?.overlines, [0..<2])
        XCTAssertEqual(byString["A0/CS"]?.overlines, [3..<5])
        XCTAssertEqual(byString["AIN7/GPIO3/DRDY/CLK"]?.overlines, [11..<15])
        XCTAssertEqual(byString["D0"]?.overlines, [])
        XCTAssertEqual(byString["VCC"]?.overlines, [])
        XCTAssertEqual(byString["CE"]?.size, 10)
        XCTAssertEqual(byString["CE"]?.monospaced, true)
    }

    /// A shipped part whose `#` pins plan-39-D flagged draws them overlined.
    func testShippedSRAMIsOverlined() throws {
        let part = try TestSupport.part("AS6C1008-55PCN")
        var painter = TextRecordingPainter()
        ChipBlockView.paint(&painter, chip: PartPreview.chip(for: part), isSelected: false, isHovered: false)
        let strings = Set(painter.record.texts.map(\.string))
        XCTAssertTrue(strings.isDisjoint(with: ["CE#", "OE#", "WE#"]), "\(strings)")
        let overlined = painter.record.texts.filter { !$0.overlines.isEmpty }.map(\.string)
        XCTAssertEqual(Set(overlined), ["CE", "OE", "WE"])
    }

    // MARK: (b) pixels

    nonisolated final class Measure: @unchecked Sendable {
        var width: CGFloat = 0
    }

    /// "OE", 10 pt monospaced, in export black at (10, 20), scale 2.
    private func render(overlines: [Range<Int>], measure: Measure) throws -> (pixels: [UInt8], width: Int, height: Int) {
        let view = Canvas { context, _ in
            var painter = ScreenPainter(context, palette: .export)
            painter.draw(PaintedText("OE", size: 10, monospaced: true, overlines: overlines),
                         at: CGPoint(x: 10, y: 20), anchor: .leading)
            let text = Text("OE").font(.system(size: 10, weight: .regular, design: .monospaced))
            measure.width = context.resolve(text).measure(in: CGSize(width: 1000, height: 1000)).width
        }
        .frame(width: 60, height: 40)
        let renderer = ImageRenderer(content: view)
        renderer.scale = 2
        let image = try XCTUnwrap(renderer.cgImage)
        let width = image.width, height = image.height
        var pixels = [UInt8](repeating: 0, count: width * height * 4)
        let context = try XCTUnwrap(CGContext(
            data: &pixels, width: width, height: height, bitsPerComponent: 8, bytesPerRow: width * 4,
            space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
        context.draw(image, in: CGRect(x: 0, y: 0, width: width, height: height))
        return (pixels, width, height)
    }

    /// (b) The bar spans ≥ 90 % of the text's width, is ≤ 4 px tall, and
    /// lies entirely above the plain glyphs' topmost ink row.
    func testOverlinePixels() throws {
        let measure = Measure()
        let plain = try render(overlines: [], measure: measure)
        let barred = try render(overlines: [0..<2], measure: measure)
        XCTAssertEqual(plain.width, barred.width)
        let alpha = { (image: [UInt8], x: Int, y: Int, width: Int) in Int(image[(y * width + x) * 4 + 3]) }

        var inkTop = Int.max
        var minX = Int.max, maxX = -1, minY = Int.max, maxY = -1
        for y in 0..<plain.height {
            for x in 0..<plain.width {
                let a = alpha(plain.pixels, x, y, plain.width)
                let b = alpha(barred.pixels, x, y, plain.width)
                if a > 40 { inkTop = min(inkTop, y) }
                if abs(a - b) > 40 {
                    minX = min(minX, x); maxX = max(maxX, x)
                    minY = min(minY, y); maxY = max(maxY, y)
                }
            }
        }
        XCTAssertGreaterThan(maxX, -1, "no overline drawn")
        XCTAssertLessThan(inkTop, Int.max, "no glyphs drawn")
        let textWidth = measure.width * 2
        XCTAssertGreaterThanOrEqual(CGFloat(maxX - minX + 1), 0.9 * textWidth,
                                    "bar \(minX)…\(maxX) vs text width \(textWidth) px")
        XCTAssertLessThanOrEqual(maxY - minY + 1, 4, "bar rows \(minY)…\(maxY)")
        XCTAssertLessThan(maxY, inkTop, "bar rows \(minY)…\(maxY), glyph top \(inkTop)")
    }
}
