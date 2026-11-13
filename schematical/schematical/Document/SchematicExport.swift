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
    /// Pixels per canvas point, unless File > Export > Image asks for another.
    static let pointScale: CGFloat = 4

    /// `wanted`, or less so the longest side is `maxPixels`.
    static func pixelScale(for size: CGSize, wanted: CGFloat = pointScale) -> CGFloat {
        let longest = max(size.width, size.height)
        guard longest > 0 else { return wanted }
        return min(wanted, maxPixels / longest)
    }

    /// Black line art on a transparent background (or `white`). Nil for an
    /// empty scene.
    static func png(_ scene: SchematicScene, scale: CGFloat = pointScale, white: Bool = false) -> Data? {
        guard let image = render(scene, scale: scale, white: white) else { return nil }
        return ImageExport.encode(image, as: .png)
    }

    /// Black line art on white. Nil for an empty scene.
    static func jpg(_ scene: SchematicScene, scale: CGFloat = pointScale) -> Data? {
        guard let image = render(scene, scale: scale, white: true) else { return nil }
        return ImageExport.encode(image, as: .jpg)
    }

    /// The scene drawn through the export palette over the same padded
    /// rectangle as the SVG, at `pixelScale`.
    private static func render(_ scene: SchematicScene, scale: CGFloat, white: Bool) -> CGImage? {
        guard let rect = bounds(of: scene) else { return nil }
        let size = CGSize(width: ceil(rect.width), height: ceil(rect.height))
        return ImageExport.rasterize(size: size, scale: pixelScale(for: size, wanted: scale), opaque: white) { context in
            if white { context.fill(Path(CGRect(origin: .zero, size: size)), with: .color(.white)) }
            paint(scene, in: rect, into: &context)
        }
    }

    /// The scene through the export palette, `rect` (from `bounds(of:)`)
    /// moved to the context's origin.
    static func paint(_ scene: SchematicScene, in rect: CGRect, into context: inout GraphicsContext) {
        var shifted = context
        shifted.translateBy(x: -rect.minX, y: -rect.minY)
        var painter = ScreenPainter(shifted, palette: .export)
        scene.paint(into: &painter)
    }
}
