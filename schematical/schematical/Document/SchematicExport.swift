import AppKit
import SwiftUI

/// File > Export: a schematic as SVG (plan-28-C) or PNG / JPG
/// (plan-28-D), black line art cropped to the ink plus `padding`.
/// See `docs/export.md`.
enum SchematicExport {
    /// Space around the measured ink, in points. Also covers the error of
    /// `BoundsPainter`'s text estimate and a viewer's wider font.
    static let padding: CGFloat = 12

    /// The ink rectangle outset by `padding`; nil for an empty scene.
    static func bounds(of scene: SchematicScene) -> CGRect? {
        guard !scene.isEmpty else { return nil }
        var painter = BoundsPainter()
        scene.paint(into: &painter)
        return painter.bounds?.insetBy(dx: -padding, dy: -padding)
    }

    /// A complete SVG document of the scene: transparent, no `<style>`, text
    /// as `<text>`. Nil for an empty scene.
    static func svg(_ scene: SchematicScene) -> Data? {
        guard let rect = bounds(of: scene) else { return nil }
        let width = SVGPainter.number(ceil(rect.width))
        let height = SVGPainter.number(ceil(rect.height))
        var painter = SVGPainter()
        painter.translateBy(x: -rect.minX, y: -rect.minY)
        scene.paint(into: &painter)
        var lines = [
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>",
            "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"\(width)\" height=\"\(height)\" viewBox=\"0 0 \(width) \(height)\">",
            "<g>",
        ]
        lines += painter.output.elements
        lines += ["</g>", "</svg>", ""]
        return Data(lines.joined(separator: "\n").utf8)
    }
}

// MARK: - Raster (plan-28-D)

extension SchematicExport {
    /// The longest side of a PNG/JPG, in pixels: 8192 × 8192 × 4 bytes is 256 MiB.
    static let maxPixels: CGFloat = 8192
    /// Pixels per canvas point.
    static let pointScale: CGFloat = 4

    /// `pointScale`, or less so the longest side is `maxPixels`.
    static func pixelScale(for size: CGSize) -> CGFloat {
        let longest = max(size.width, size.height)
        guard longest > 0 else { return pointScale }
        return min(pointScale, maxPixels / longest)
    }

    /// Black line art on a transparent background. Nil for an empty scene.
    static func png(_ scene: SchematicScene) -> Data? {
        guard let image = render(scene, background: nil) else { return nil }
        return NSBitmapImageRep(cgImage: image).representation(using: .png, properties: [:])
    }

    /// Black line art on white. Nil for an empty scene.
    static func jpg(_ scene: SchematicScene) -> Data? {
        guard let image = render(scene, background: .white) else { return nil }
        return NSBitmapImageRep(cgImage: image).representation(using: .jpeg, properties: [.compressionFactor: 0.92])
    }

    /// The scene drawn through the export palette over the same padded
    /// rectangle as the SVG, at `pixelScale`.
    private static func render(_ scene: SchematicScene, background: Color?) -> CGImage? {
        guard let rect = bounds(of: scene) else { return nil }
        let size = CGSize(width: ceil(rect.width), height: ceil(rect.height))
        let canvas = Canvas { context, _ in
            var shifted = context
            shifted.translateBy(x: -rect.minX, y: -rect.minY)
            var painter = ScreenPainter(shifted, palette: .export)
            scene.paint(into: &painter)
        }
        .frame(width: size.width, height: size.height)
        let renderer = ImageRenderer(content: canvas.background(background ?? .clear))
        renderer.scale = pixelScale(for: size)
        renderer.isOpaque = background != nil
        return renderer.cgImage
    }
}
