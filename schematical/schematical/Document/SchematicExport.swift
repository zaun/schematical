import SwiftUI

/// Schematical > Export: a schematic as SVG (plan-28-C) or PNG / JPG
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
