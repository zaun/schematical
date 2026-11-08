import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-28-D: the PNG and JPG writers, checked on decoded bitmaps.
@MainActor
final class SchematicRasterTests: XCTestCase {
    func testPixelScale() {
        XCTAssertEqual(SchematicExport.pixelScale(for: CGSize(width: 100, height: 100)), 4)
        XCTAssertEqual(SchematicExport.pixelScale(for: CGSize(width: 2048, height: 500)), 4)
        XCTAssertEqual(SchematicExport.pixelScale(for: CGSize(width: 3000, height: 500)), 8192 / 3000)
        XCTAssertEqual(SchematicExport.pixelScale(for: CGSize(width: 500, height: 4096)), 2)
    }

    /// One SN74HC00N with a wire; its padded size in points.
    private func chipWithWire() throws -> (SchematicScene, CGSize) {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(chip, "1Y")), .pin(try TestSupport.pin(chip, "2A"))))
        let scene = SchematicScene(doc)
        let rect = try XCTUnwrap(SchematicExport.bounds(of: scene))
        return (scene, CGSize(width: ceil(rect.width), height: ceil(rect.height)))
    }

    private func bitmap(_ data: Data?) throws -> NSBitmapImageRep {
        try XCTUnwrap(NSBitmapImageRep(data: try XCTUnwrap(data)))
    }

    /// Whether some pixel passes `test` (sampled every other pixel).
    private func anyPixel(_ rep: NSBitmapImageRep, _ test: (NSColor) -> Bool) -> Bool {
        for y in stride(from: 0, to: rep.pixelsHigh, by: 2) {
            for x in stride(from: 0, to: rep.pixelsWide, by: 2) {
                if let color = rep.colorAt(x: x, y: y), test(color) { return true }
            }
        }
        return false
    }

    func testPNG() throws {
        let (scene, size) = try chipWithWire()
        let rep = try bitmap(SchematicExport.png(scene))
        XCTAssertEqual(rep.pixelsWide, Int(size.width * 4))
        XCTAssertEqual(rep.pixelsHigh, Int(size.height * 4))
        XCTAssertTrue(rep.hasAlpha)
        XCTAssertEqual(try XCTUnwrap(rep.colorAt(x: 0, y: 0)).alphaComponent, 0)
        XCTAssertTrue(anyPixel(rep) { color in
            guard let c = color.usingColorSpace(.sRGB) else { return false }
            return c.alphaComponent > 0.9 && c.redComponent < 0.1 && c.greenComponent < 0.1 && c.blueComponent < 0.1
        }, "no black pixel")
    }

    func testJPG() throws {
        let (scene, size) = try chipWithWire()
        let rep = try bitmap(SchematicExport.jpg(scene))
        XCTAssertEqual(rep.pixelsWide, Int(size.width * 4))
        XCTAssertEqual(rep.pixelsHigh, Int(size.height * 4))
        XCTAssertFalse(rep.hasAlpha)
        let corner = try XCTUnwrap(rep.colorAt(x: 0, y: 0)?.usingColorSpace(.sRGB))
        for channel in [corner.redComponent, corner.greenComponent, corner.blueComponent] {
            XCTAssertGreaterThanOrEqual(channel, 0.98, "corner \(corner)")
        }
        XCTAssertTrue(anyPixel(rep) { color in
            guard let c = color.usingColorSpace(.sRGB) else { return false }
            return c.redComponent < 0.3 && c.greenComponent < 0.3 && c.blueComponent < 0.3
        }, "no dark pixel")
    }

    func testEmptyDocument() {
        let scene = SchematicScene(TestSupport.newDocument())
        XCTAssertNil(SchematicExport.png(scene))
        XCTAssertNil(SchematicExport.jpg(scene))
    }

    /// Wider than 2048 pt: the longest side is capped at 8192 px, aspect kept.
    func testPixelCap() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 2700, y: 200))
        let scene = SchematicScene(doc)
        let rect = try XCTUnwrap(SchematicExport.bounds(of: scene))
        let width = ceil(rect.width), height = ceil(rect.height)
        XCTAssertGreaterThan(width, 2048)
        let rep = try bitmap(SchematicExport.png(scene))
        XCTAssertTrue((8191...8192).contains(rep.pixelsWide), "\(rep.pixelsWide) wide")
        let ratio = Double(rep.pixelsWide) / Double(rep.pixelsHigh)
        XCTAssertEqual(ratio, width / height, accuracy: width / height * 0.001)
    }
}
