import AppKit
import SwiftUI
import UniformTypeIdentifiers

/// File > Export > Image…: a picture of the schematic, the PCB page, or the
/// made board seen from the front or the back (Board Preview's render),
/// chosen with `ImageExportDialog`. See `docs/export.md`.

// MARK: - Options

/// What the picture shows.
nonisolated enum ImageExportView: String, CaseIterable, Identifiable, Sendable {
    case schematic, pcb, front, back

    var id: Self { self }

    var title: String {
        switch self {
        case .schematic: "Schematic"
        case .pcb: "PCB"
        case .front: "Front Render"
        case .back: "Back Render"
        }
    }

    /// Added to the window's name for the file name: "CPU PCB.png".
    var fileSuffix: String {
        switch self {
        case .schematic: "Schematic"
        case .pcb: "PCB"
        case .front: "Front"
        case .back: "Back"
        }
    }

    /// The board face of a render; nil for the schematic and the PCB page.
    var face: BoardRenderFace? {
        switch self {
        case .front: .front
        case .back: .back
        case .schematic, .pcb: nil
        }
    }

    /// SVG is the schematic's only: the board pictures are raster art.
    var formats: [ImageExportFormat] {
        self == .schematic ? [.png, .jpg, .svg] : [.png, .jpg]
    }

    /// The backgrounds offered, the default first.
    var backgrounds: [ImageExportBackground] {
        self == .schematic ? [.transparent, .white] : [.canvas, .white, .transparent]
    }
}

nonisolated enum ImageExportFormat: String, CaseIterable, Identifiable, Sendable {
    case png, jpg, svg

    var id: Self { self }
    var title: String { rawValue.uppercased() }
    var fileExtension: String { rawValue }

    var contentType: UTType {
        switch self {
        case .png: .png
        case .jpg: .jpeg
        case .svg: .svg
        }
    }

    /// JPG has no alpha.
    var allowsTransparency: Bool { self != .jpg }
}

nonisolated enum ImageExportBackground: String, CaseIterable, Identifiable, Sendable {
    /// The PCB page's dark canvas, or Board Preview's backdrop.
    case canvas
    case white
    case transparent

    var id: Self { self }

    func title(for view: ImageExportView) -> String {
        switch self {
        case .canvas: view == .pcb ? "Dark" : "Backdrop"
        case .white: "White"
        case .transparent: "Transparent"
        }
    }
}

/// Everything the dialog sets. `normalized()` keeps the pieces consistent
/// (no SVG PCB, no transparent JPG).
nonisolated struct ImageExportOptions: Equatable, Sendable {
    var view: ImageExportView = .schematic
    var format: ImageExportFormat = .png
    /// Pixels per point: a schematic point, a PCB page point (20 per mm), or
    /// a render point (`ImageExport.renderPointsPerMM`). Capped so the
    /// longest side is `SchematicExport.maxPixels`.
    var scale: CGFloat = 2
    var background: ImageExportBackground = .transparent
    /// The PCB page's layers left out. Ratsnest by default.
    var hiddenLayers: Set<BoardViewLayer> = [.ratsnest]
    var angle: BoardRenderAngle = .angled
    var finish: BoardRenderFinish = .hasl

    static let scales: [CGFloat] = [1, 2, 4, 8]

    func normalized() -> ImageExportOptions {
        var o = self
        if !o.view.formats.contains(o.format) { o.format = .png }
        if !o.view.backgrounds.contains(o.background) { o.background = o.view.backgrounds[0] }
        if !o.format.allowsTransparency, o.background == .transparent {
            o.background = o.view.backgrounds.first { $0 != .transparent } ?? .white
        }
        if !Self.scales.contains(o.scale) { o.scale = 2 }
        return o
    }

    // The last choices, kept between exports (the layers are not: a
    // board's copper layers differ from file to file).
    private static let key = "imageExport."

    static func saved(_ defaults: UserDefaults = .standard) -> ImageExportOptions {
        var o = ImageExportOptions()
        func value<T: RawRepresentable<String>>(_ name: String, _ fallback: T) -> T {
            defaults.string(forKey: key + name).flatMap(T.init(rawValue:)) ?? fallback
        }
        o.view = value("view", o.view)
        o.format = value("format", o.format)
        o.background = value("background", o.background)
        o.angle = value("angle", o.angle)
        o.finish = value("finish", o.finish)
        let scale = defaults.double(forKey: key + "scale")
        if scale > 0 { o.scale = scale }
        return o.normalized()
    }

    func save(_ defaults: UserDefaults = .standard) {
        let key = Self.key
        defaults.set(view.rawValue, forKey: key + "view")
        defaults.set(format.rawValue, forKey: key + "format")
        defaults.set(background.rawValue, forKey: key + "background")
        defaults.set(angle.rawValue, forKey: key + "angle")
        defaults.set(finish.rawValue, forKey: key + "finish")
        defaults.set(Double(scale), forKey: key + "scale")
    }
}

// MARK: - Drawing

@MainActor
enum ImageExport {
    /// A render's size: points per board mm before `scale`.
    static let renderPointsPerMM: CGFloat = 10
    /// Space round a render's board, in points.
    static let renderMargin: CGFloat = 24
    /// Space round everything on the PCB page, in mm.
    static let pcbPadding: CGFloat = 2
    /// The PCB page's viewport for an export: 100 %, the origin at the top
    /// left; the picture moves the board into place itself. One for the
    /// app's life, never changed.
    private static let viewport = CanvasViewport()

    /// One picture: its size in points, and how to draw it from the
    /// top-left corner. Nil when there is nothing to draw (an empty
    /// schematic).
    struct Picture {
        var size: CGSize
        var draw: (inout GraphicsContext) -> Void
    }

    static func picture(of document: SchematicDocument, _ options: ImageExportOptions) -> Picture? {
        let o = options.normalized()
        switch o.view {
        case .schematic:
            let scene = SchematicScene(document)
            guard let rect = SchematicExport.bounds(of: scene) else { return nil }
            let size = CGSize(width: ceil(rect.width), height: ceil(rect.height))
            return Picture(size: size) { context in
                fillBackground(o, size: size, in: &context)
                SchematicExport.paint(scene, in: rect, into: &context)
            }
        case .pcb:
            let mm = pcbBounds(document)
            let ppm = BoardScale.pointsPerMM
            let size = CGSize(width: ceil(mm.width * ppm), height: ceil(mm.height * ppm))
            let view = PCBCanvasView(document: document, viewport: viewport,
                                     visible: BoardVisibility(hidden: o.hiddenLayers),
                                     exporting: true, clearsHoles: o.background != .canvas)
            return Picture(size: size) { context in
                fillBackground(o, size: size, in: &context)
                var board = context
                board.translateBy(x: -mm.minX * ppm, y: -mm.minY * ppm)
                view.drawExport(in: &board, size: size)
            }
        case .front, .back:
            let renderer = BoardRenderer(document: document, face: o.view.face ?? .front, angle: o.angle,
                                         finish: o.finish, margin: renderMargin)
            let size = renderer.fittedSize(pointsPerMM: renderPointsPerMM)
            return Picture(size: size) { context in
                fillBackground(o, size: size, in: &context)
                renderer.draw(in: &context, size: size)
            }
        }
    }

    /// Everything on the PCB page, in board mm: the board, and any part,
    /// text, trace, via or hole placed off it, plus `pcbPadding`.
    static func pcbBounds(_ document: SchematicDocument) -> CGRect {
        let outline = document.board.outline
        var rect = outline.isEmpty
            ? CGRect(x: 0, y: 0, width: 100, height: 80)
            : CGRect(x: outline.map(\.x).min()!, y: outline.map(\.y).min()!,
                     width: outline.map(\.x).max()! - outline.map(\.x).min()!,
                     height: outline.map(\.y).max()! - outline.map(\.y).min()!)
        for chip in document.chips { if let b = chip.boardBounds { rect = rect.union(b) } }
        for text in document.texts { rect = rect.union(text.boardRect) }
        for via in document.vias { rect = rect.union(via.rect) }
        for hole in document.holes { rect = rect.union(hole.rect) }
        for trace in document.traces {
            for p in trace.points {
                let r = trace.widthMM / 2
                rect = rect.union(CGRect(x: p.x - r, y: p.y - r, width: 2 * r, height: 2 * r))
            }
        }
        return rect.insetBy(dx: -pcbPadding, dy: -pcbPadding)
    }

    private static func fillBackground(_ o: ImageExportOptions, size: CGSize, in context: inout GraphicsContext) {
        let rect = Path(CGRect(origin: .zero, size: size))
        switch o.background {
        case .transparent:
            break
        case .white:
            context.fill(rect, with: .color(.white))
        case .canvas:
            if o.view == .pcb {
                context.fill(rect, with: .color(PCBCanvasView.background))
            } else {
                let centre = CGPoint(x: size.width / 2, y: size.height / 2)
                context.fill(rect, with: .radialGradient(BoardPreviewView.backdrop, center: centre, startRadius: 0,
                                                         endRadius: max(size.width, size.height) * 0.75))
            }
        }
    }

    /// The picture's size in pixels, after the 8192 px cap; nil when there
    /// is nothing to draw. An SVG's is its size in points.
    static func pixelSize(of document: SchematicDocument, _ options: ImageExportOptions) -> CGSize? {
        guard let picture = picture(of: document, options) else { return nil }
        if options.normalized().format == .svg { return picture.size }
        let s = SchematicExport.pixelScale(for: picture.size, wanted: options.scale)
        return CGSize(width: (picture.size.width * s).rounded(), height: (picture.size.height * s).rounded())
    }

    /// The file's bytes; nil for an empty schematic.
    static func data(for document: SchematicDocument, _ options: ImageExportOptions) -> Data? {
        let o = options.normalized()
        if o.format == .svg { return SchematicExport.svg(SchematicScene(document)) }
        guard let picture = picture(of: document, o) else { return nil }
        let scale = SchematicExport.pixelScale(for: picture.size, wanted: o.scale)
        let opaque = o.background != .transparent
        guard let image = rasterize(size: picture.size, scale: scale, opaque: opaque, picture.draw) else { return nil }
        return encode(image, as: o.format)
    }

    /// `draw` in a `size`-point canvas, `scale` pixels per point.
    static func rasterize(size: CGSize, scale: CGFloat, opaque: Bool,
                          _ draw: @escaping (inout GraphicsContext) -> Void) -> CGImage? {
        let canvas = Canvas { context, _ in draw(&context) }
            .frame(width: size.width, height: size.height)
        let renderer = ImageRenderer(content: canvas)
        renderer.scale = scale
        renderer.isOpaque = opaque
        return renderer.cgImage
    }

    static func encode(_ image: CGImage, as format: ImageExportFormat) -> Data? {
        let rep = NSBitmapImageRep(cgImage: image)
        switch format {
        case .png: return rep.representation(using: .png, properties: [:])
        case .jpg: return rep.representation(using: .jpeg, properties: [.compressionFactor: 0.92])
        case .svg: return nil
        }
    }
}
