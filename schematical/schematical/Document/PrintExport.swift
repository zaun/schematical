import AppKit
import CoreGraphics
import CoreText
import PDFKit
import SwiftUI

/// File > Print… and File > Export > PDF…: the schematic, and each PCB layer
/// at exactly 1:1, as the pages of one PDF. See `docs/export.md` § PDF and print.
///
/// A PCB page is drawn from the same artwork as the Gerber files
/// (`GerberExport.drawCopper` / `drawSilkscreen`, recorded by
/// `GerberWriter.records`), so a printout shows what the fab gets. A board
/// bigger than the paper is cut into overlapping tiles.

// MARK: - Options

/// A sheet size, portrait, in mm.
nonisolated enum PrintPaper: String, CaseIterable, Identifiable, Sendable {
    case letter, legal, tabloid, a4, a3

    var id: Self { self }

    var title: String {
        switch self {
        case .letter: "Letter"
        case .legal: "Legal"
        case .tabloid: "Tabloid (11 × 17 in)"
        case .a4: "A4"
        case .a3: "A3"
        }
    }

    /// Width × height in mm, portrait.
    var size: CGSize {
        switch self {
        case .letter: CGSize(width: 215.9, height: 279.4)
        case .legal: CGSize(width: 215.9, height: 355.6)
        case .tabloid: CGSize(width: 279.4, height: 431.8)
        case .a4: CGSize(width: 210, height: 297)
        case .a3: CGSize(width: 297, height: 420)
        }
    }

    /// Letter where the system measures in inches, A4 elsewhere.
    static var systemDefault: PrintPaper {
        Locale.current.measurementSystem == .us ? .letter : .a4
    }
}

/// One kind of page, which `PrintOptions.hidden` can leave out.
nonisolated enum PrintPageKind: Hashable, Sendable {
    case schematic
    /// One copper layer: its pads, traces, vias and planes, black on white.
    case copper(CopperLayer)
    /// A side's silkscreen alone.
    case silkscreen(BoardSide)
    /// A side's pads, true-size drill holes and silkscreen: lay the real parts on it.
    case parts(BoardSide)
    /// The outline and every drill hole at its true size.
    case board

    var title: String {
        switch self {
        case .schematic: "Schematic"
        case let .copper(layer): "\(layer.title) copper"
        case let .silkscreen(side): "\(side.title) silkscreen"
        case let .parts(side): "\(side.title) parts (footprint check)"
        case .board: "Board outline and drills"
        }
    }

    /// The side of the board whose view the page is: a Bottom layer is seen from below, so mirrored
    /// against the PCB page.
    var side: BoardSide {
        switch self {
        case let .copper(layer): layer == .bottom ? .bottom : .top
        case let .silkscreen(side), let .parts(side): side
        case .schematic, .board: .top
        }
    }
}

nonisolated struct PrintOptions: Equatable, Sendable {
    var paper: PrintPaper = .systemDefault
    /// Copper pages flipped left to right, for toner transfer or film laid face down.
    var mirror = false
    /// Copper pages inverted: copper is paper-white on black, for positive-resist boards.
    var negative = false
    /// Registration crosses round each copper page's board.
    var marks = true
    /// The pages left out. Silkscreen by default.
    var hidden: Set<PrintPageKind> = [.silkscreen(.top), .silkscreen(.bottom)]

    // The last choices, kept between prints (the hidden pages are not: a board's layers differ from file to file).
    private static let key = "printExport."

    static func saved(_ defaults: UserDefaults = .standard) -> PrintOptions {
        var o = PrintOptions()
        o.paper = defaults.string(forKey: key + "paper").flatMap(PrintPaper.init(rawValue:)) ?? o.paper
        if defaults.object(forKey: key + "mirror") != nil { o.mirror = defaults.bool(forKey: key + "mirror") }
        if defaults.object(forKey: key + "negative") != nil { o.negative = defaults.bool(forKey: key + "negative") }
        if defaults.object(forKey: key + "marks") != nil { o.marks = defaults.bool(forKey: key + "marks") }
        return o
    }

    func save(_ defaults: UserDefaults = .standard) {
        defaults.set(paper.rawValue, forKey: Self.key + "paper")
        defaults.set(mirror, forKey: Self.key + "mirror")
        defaults.set(negative, forKey: Self.key + "negative")
        defaults.set(marks, forKey: Self.key + "marks")
    }
}

/// One page of the document.
nonisolated struct PrintPage: Equatable, Sendable {
    var kind: PrintPageKind
    var landscape: Bool
    /// The tile (0-based) of `columns` × `rows`; 0 of 1 × 1 for a board that fits.
    var column = 0, row = 0, columns = 1, rows = 1
    /// The board point shown at the printable area's top-left, in the view of the page (mm).
    var origin: CGPoint = .zero
    /// The schematic page's scale: 1 is as on screen, less when it must shrink to fit the sheet.
    var scale: CGFloat = 1

    var isTile: Bool { columns * rows > 1 }
}

/// The recorded artwork of a board.
struct BoardArtwork {
    var copper: [CopperLayer: [GerberWriter.Mark]]
    var silkscreen: [BoardSide: [GerberWriter.Mark]]
    var holes: [DrillExport.Hole]
    var outline: [CGPoint]
    /// Each cutout's loop, mm (plan-65-G).
    var cutouts: [[CGPoint]] = []
    /// The board's rectangle, mm.
    var bounds: CGRect
    var layers: [CopperLayer]
}

// MARK: - Export

@MainActor
enum PrintExport {
    /// Space round the page, mm. Printers cannot reach the edge; this keeps clear of the usual margin.
    static let margin: CGFloat = 10
    /// The footer's height, mm: the name, the scale and a 50 mm ruler.
    static let footer: CGFloat = 9
    /// Shared by neighbouring tiles, mm.
    static let overlap: CGFloat = 10
    /// PDF points per mm.
    static let pointsPerMM: CGFloat = 72 / 25.4
    /// The ruler's length on every board page, mm.
    static let rulerLength: CGFloat = 50
    /// Artwork's pen for outlines and the like, mm.
    static let hairline: CGFloat = 0.1
    /// The drill dot on a copper page, mm: a guide for the bit, not the hole's size.
    static let drillDot: CGFloat = 0.4

    // MARK: Pages

    /// The pages the document can have, in order: the schematic (when it has something), each copper layer, each
    /// side's parts and silkscreen pages, the board.
    static func kinds(of document: SchematicDocument) -> [PrintPageKind] {
        var kinds: [PrintPageKind] = []
        if !SchematicScene(document).isEmpty { kinds.append(.schematic) }
        let layers = document.board.copperLayers
        kinds += layers.map { .copper($0) }
        let sides: [BoardSide] = layers.count > 1 ? [.top, .bottom] : [.top]
        kinds += sides.map { .parts($0) }
        kinds += sides.map { .silkscreen($0) }
        kinds.append(.board)
        return kinds
    }

    /// The board's rectangle (mm): its outline, or its width and height from the origin.
    static func boardBounds(_ document: SchematicDocument) -> CGRect {
        let outline = document.board.outline
        guard let minX = outline.map(\.x).min(), let maxX = outline.map(\.x).max(),
              let minY = outline.map(\.y).min(), let maxY = outline.map(\.y).max() else {
            return CGRect(x: 0, y: 0, width: CGFloat(document.board.width), height: CGFloat(document.board.height))
        }
        return CGRect(x: minX, y: minY, width: maxX - minX, height: maxY - minY)
    }

    /// The pages of `options`' kinds that exist, tiled for the paper.
    static func pages(for document: SchematicDocument, _ options: PrintOptions) -> [PrintPage] {
        var pages: [PrintPage] = []
        let bounds = boardBounds(document)
        for kind in kinds(of: document) where !options.hidden.contains(kind) {
            switch kind {
            case .schematic:
                if let page = schematicPage(document, paper: options.paper) { pages.append(page) }
            default:
                let room = padding(of: kind, options)
                pages += tiles(kind: kind, content: bounds.insetBy(dx: -room, dy: -room), paper: options.paper)
            }
        }
        return pages
    }

    /// Room round the board a page keeps: the registration marks need more.
    static func padding(of kind: PrintPageKind, _ options: PrintOptions) -> CGFloat {
        if case .copper = kind, options.marks { return markOffset + markArm }
        return 2
    }

    /// A copper page's black area (negative) reaches this far past the board, mm; the marks sit outside it.
    static let inkMargin: CGFloat = 1.5
    static let markOffset: CGFloat = 4
    static let markArm: CGFloat = 1.5

    /// The printable area of a sheet, mm: its size, and where it starts.
    static func printable(_ paper: PrintPaper, landscape: Bool) -> CGRect {
        let size = paper.size
        let w = landscape ? size.height : size.width, h = landscape ? size.width : size.height
        return CGRect(x: margin, y: margin, width: w - 2 * margin, height: h - 2 * margin - footer)
    }

    private static func count(_ length: CGFloat, _ available: CGFloat) -> Int {
        guard length > available + 1e-6, available > overlap else { return 1 }
        return Int(ceil((length - overlap) / (available - overlap) - 1e-9))
    }

    /// `content` on as few sheets as possible: a board that fits goes on one, centred; a bigger one is cut
    /// into evenly spaced tiles that overlap by at least `overlap`, in the orientation that needs fewer.
    static func tiles(kind: PrintPageKind, content: CGRect, paper: PrintPaper) -> [PrintPage] {
        func plan(landscape: Bool) -> (columns: Int, rows: Int, area: CGRect) {
            let area = printable(paper, landscape: landscape)
            return (count(content.width, area.width), count(content.height, area.height), area)
        }
        let portrait = plan(landscape: false), landscape = plan(landscape: true)
        let usesLandscape: Bool
        if landscape.columns * landscape.rows != portrait.columns * portrait.rows {
            usesLandscape = landscape.columns * landscape.rows < portrait.columns * portrait.rows
        } else {
            usesLandscape = content.width > content.height
        }
        let (columns, rows, area) = usesLandscape ? landscape : portrait
        func start(_ index: Int, of n: Int, from low: CGFloat, length: CGFloat, available: CGFloat) -> CGFloat {
            if n == 1 { return low - (available - length) / 2 }
            return low + CGFloat(index) * (length - available) / CGFloat(n - 1)
        }
        var pages: [PrintPage] = []
        for row in 0..<rows {
            for column in 0..<columns {
                let x = start(column, of: columns, from: content.minX, length: content.width, available: area.width)
                let y = start(row, of: rows, from: content.minY, length: content.height, available: area.height)
                pages.append(PrintPage(kind: kind, landscape: usesLandscape, column: column, row: row,
                                       columns: columns, rows: rows, origin: CGPoint(x: x, y: y)))
            }
        }
        return pages
    }

    /// The schematic on one sheet, in the orientation where it prints largest, at most as large as on screen.
    static func schematicPage(_ document: SchematicDocument, paper: PrintPaper) -> PrintPage? {
        let scene = SchematicScene(document)
        guard let rect = SchematicExport.bounds(of: scene) else { return nil }
        func fit(landscape: Bool) -> CGFloat {
            let area = printable(paper, landscape: landscape)
            return min(1, area.width * pointsPerMM / rect.width, area.height * pointsPerMM / rect.height)
        }
        let landscape = fit(landscape: true) > fit(landscape: false)
        return PrintPage(kind: .schematic, landscape: landscape, scale: fit(landscape: landscape))
    }

    // MARK: Artwork

    /// The board's layers as the Gerber files draw them, recorded.
    static func artwork(for document: SchematicDocument) -> BoardArtwork {
        let board = document.board
        let context = KiCadExport.Context(document: document, base: "print")
        let copper = context.copper
        let nets = ExportNetNames.assign(context: context)
        let top = Double(board.outline.map(\.y).max() ?? CGFloat(board.height))
        func writer() -> GerberWriter { GerberWriter(attributes: [], top: top, records: true) }
        var layers: [CopperLayer: [GerberWriter.Mark]] = [:]
        for layer in board.copperLayers {
            var file = writer()
            GerberExport.drawCopper(on: layer, board: board, document: document, copper: copper, nets: nets,
                                    rules: board.rules, into: &file)
            layers[layer] = file.marks
        }
        var silkscreen: [BoardSide: [GerberWriter.Mark]] = [:]
        for side in BoardSide.allCases {
            var file = writer()
            GerberExport.drawSilkscreen(on: side, document: document, into: &file)
            silkscreen[side] = file.marks
        }
        let holes = DrillExport.holes(copper)
        return BoardArtwork(copper: layers, silkscreen: silkscreen, holes: holes.plated + holes.bare,
                            outline: board.outline, cutouts: document.boardEdge.cutouts, bounds: boardBounds(document),
                            layers: board.copperLayers)
    }

    // MARK: PDF

    /// The pages `options` choose as one PDF; nil when there are none.
    static func pdf(for document: SchematicDocument, name: String, _ options: PrintOptions,
                    artwork: BoardArtwork? = nil) -> Data? {
        let pages = pages(for: document, options)
        guard !pages.isEmpty else { return nil }
        let art = pages.contains { $0.kind != .schematic } ? (artwork ?? self.artwork(for: document)) : nil
        let data = NSMutableData()
        guard let consumer = CGDataConsumer(data: data as CFMutableData) else { return nil }
        let info: [CFString: Any] = [kCGPDFContextTitle: name, kCGPDFContextCreator: "Schematical"]
        var first = mediaBox(pages[0], options.paper)
        guard let pdf = CGContext(consumer: consumer, mediaBox: &first, info as CFDictionary) else { return nil }
        for (index, page) in pages.enumerated() {
            var box = mediaBox(page, options.paper)
            let boxData = withUnsafeBytes(of: &box) { Data($0) }
            pdf.beginPDFPage([kCGPDFContextMediaBox: boxData] as CFDictionary)
            draw(page, number: index + 1, of: pages.count, document: document, name: name, options: options,
                 artwork: art, into: pdf)
            pdf.endPDFPage()
        }
        pdf.closePDF()
        return data as Data
    }

    /// The system print operation for `pdf`: each page at its own size (no scaling), on `paper`, turned
    /// to fit it, with no margin of its own: the pages keep their own.
    static func printOperation(for pdf: PDFDocument, paper: PrintPaper) -> NSPrintOperation? {
        let info = (NSPrintInfo.shared.copy() as? NSPrintInfo) ?? NSPrintInfo()
        info.paperSize = NSSize(width: paper.size.width * pointsPerMM, height: paper.size.height * pointsPerMM)
        info.topMargin = 0
        info.bottomMargin = 0
        info.leftMargin = 0
        info.rightMargin = 0
        return pdf.printOperation(for: info, scalingMode: .pageScaleNone, autoRotate: true)
    }

    private static func mediaBox(_ page: PrintPage, _ paper: PrintPaper) -> CGRect {
        let size = paper.size
        let w = page.landscape ? size.height : size.width, h = page.landscape ? size.width : size.height
        return CGRect(x: 0, y: 0, width: w * pointsPerMM, height: h * pointsPerMM)
    }

    private static func draw(_ page: PrintPage, number: Int, of total: Int, document: SchematicDocument, name: String,
                             options: PrintOptions, artwork: BoardArtwork?, into pdf: CGContext) {
        let box = mediaBox(page, options.paper)
        pdf.setFillColor(CGColor(gray: 1, alpha: 1))
        pdf.fill(box)
        drawFooter(page, number: number, of: total, name: name, box: box, into: pdf)
        let area = printable(options.paper, landscape: page.landscape)
        if page.kind == .schematic {
            drawSchematic(page, document: document, area: area, box: box, into: pdf)
            return
        }
        guard let artwork else { return }
        // Board mm (y down) to the sheet.
        let k = pointsPerMM
        let content = artwork.bounds.insetBy(dx: -padding(of: page.kind, options), dy: -padding(of: page.kind, options))
        var mirrored = page.kind.side == .bottom
        if case .copper = page.kind, options.mirror { mirrored.toggle() }
        let sum = content.minX + content.maxX
        let transform = CGAffineTransform(a: mirrored ? -k : k, b: 0, c: 0, d: -k,
                                          tx: k * (area.minX - page.origin.x) + (mirrored ? k * sum : 0),
                                          ty: k * (box.height / k - area.minY + page.origin.y))
        pdf.saveGState()
        pdf.clip(to: CGRect(x: area.minX * k, y: box.height - area.maxY * k, width: area.width * k, height: area.height * k))
        pdf.concatenate(transform)
        drawBoard(page.kind, artwork: artwork, options: options, into: pdf)
        pdf.restoreGState()
    }

    // MARK: Board pages

    private static let padFunctions: Set<String> = [
        "SMDPad,CuDef", "HeatsinkPad", "ComponentPad", "ConnectorPad", "OtherPad,PlatedHole",
    ]

    /// `kind`'s artwork in board mm, the context's transform already set.
    private static func drawBoard(_ kind: PrintPageKind, artwork: BoardArtwork, options: PrintOptions,
                                  into ctx: CGContext) {
        let black = CGColor(gray: 0, alpha: 1), white = CGColor(gray: 1, alpha: 1)
        switch kind {
        case .schematic:
            break
        case let .copper(layer):
            let ink = options.negative ? white : black, paper = options.negative ? black : white
            let bounds = artwork.bounds
            if options.negative {
                ctx.setFillColor(black)
                ctx.fill(bounds.insetBy(dx: -inkMargin, dy: -inkMargin))
            }
            replay(artwork.copper[layer] ?? [], ink: ink, paper: paper, into: ctx)
            // A dot at each hole's centre where the bit goes.
            ctx.setFillColor(paper)
            ctx.setStrokeColor(paper)
            for hole in artwork.holes {
                let d = min(CGFloat(hole.diameter), drillDot)
                ctx.setLineWidth(d)
                ctx.setLineCap(.round)
                ctx.move(to: hole.a)
                ctx.addLine(to: hole.b)
                ctx.strokePath()
            }
            if options.marks { drawRegistration(bounds, into: ctx) }
        case let .silkscreen(side):
            replay(artwork.silkscreen[side] ?? [], ink: black, paper: white, into: ctx)
        case let .parts(side):
            let pads = (artwork.copper[side.copperLayer] ?? []).filter { $0.dark && padFunctions.contains($0.function ?? "") }
            ctx.setFillColor(CGColor(gray: 0.82, alpha: 1))
            ctx.setStrokeColor(black)
            ctx.setLineWidth(hairline)
            for mark in pads {
                guard let path = path(of: mark.kind) else { continue }
                ctx.addPath(path)
                ctx.drawPath(using: .fillStroke)
            }
            ctx.setFillColor(white)
            for hole in artwork.holes where hole.function != .via {
                let path = holePath(hole)
                ctx.addPath(path)
                ctx.drawPath(using: .fillStroke)
            }
            replay(artwork.silkscreen[side] ?? [], ink: black, paper: white, into: ctx)
        case .board:
            ctx.setStrokeColor(black)
            ctx.setLineWidth(2 * hairline)
            ctx.setLineJoin(.round)
            if artwork.outline.count > 1 {
                ctx.addLines(between: artwork.outline)
                ctx.closePath()
                ctx.strokePath()
                for loop in artwork.cutouts where loop.count > 1 {
                    ctx.addLines(between: loop)
                    ctx.closePath()
                    ctx.strokePath()
                }
            } else {
                ctx.stroke(artwork.bounds)
            }
            ctx.setLineWidth(hairline)
            for hole in artwork.holes {
                ctx.addPath(holePath(hole))
                ctx.strokePath()
                // The centre, for the bit.
                let c = CGPoint(x: (hole.a.x + hole.b.x) / 2, y: (hole.a.y + hole.b.y) / 2)
                let r = max(CGFloat(hole.diameter) / 2, 0.4) + 0.4
                ctx.move(to: CGPoint(x: c.x - r, y: c.y))
                ctx.addLine(to: CGPoint(x: c.x + r, y: c.y))
                ctx.move(to: CGPoint(x: c.x, y: c.y - r))
                ctx.addLine(to: CGPoint(x: c.x, y: c.y + r))
                ctx.strokePath()
            }
        }
    }

    /// A hole at its true size: a circle, or a slot as a capsule.
    private static func holePath(_ hole: DrillExport.Hole) -> CGPath {
        let r = CGFloat(hole.diameter) / 2
        let path = CGMutablePath()
        if !hole.isSlot {
            path.addEllipse(in: CGRect(x: hole.a.x - r, y: hole.a.y - r, width: 2 * r, height: 2 * r))
            return path
        }
        let angle = atan2(hole.b.y - hole.a.y, hole.b.x - hole.a.x)
        let n = CGPoint(x: -sin(angle) * r, y: cos(angle) * r)
        path.move(to: CGPoint(x: hole.a.x + n.x, y: hole.a.y + n.y))
        path.addArc(center: hole.b, radius: r, startAngle: angle + .pi / 2, endAngle: angle - .pi / 2, clockwise: true)
        path.addArc(center: hole.a, radius: r, startAngle: angle - .pi / 2, endAngle: angle + .pi / 2, clockwise: true)
        path.closeSubpath()
        return path
    }

    /// The shape of a pad's mark; nil for a pen stroke.
    private static func path(of kind: GerberWriter.Mark.Kind) -> CGPath? {
        switch kind {
        case let .circle(centre, diameter):
            return CGPath(ellipseIn: CGRect(x: centre.x - diameter / 2, y: centre.y - diameter / 2,
                                            width: diameter, height: diameter), transform: nil)
        case let .rect(rect):
            return CGPath(rect: rect, transform: nil)
        case let .region(points):
            let path = CGMutablePath()
            path.addLines(between: points)
            path.closeSubpath()
            return path
        case .stroke:
            return nil
        }
    }

    /// Dark objects in `ink`, clear ones in `paper`, in drawing order.
    private static func replay(_ marks: [GerberWriter.Mark], ink: CGColor, paper: CGColor, into ctx: CGContext) {
        for mark in marks {
            let color = mark.dark ? ink : paper
            ctx.setFillColor(color)
            ctx.setStrokeColor(color)
            if case let .stroke(points, width, closed) = mark.kind {
                guard points.count > 1 else { continue }
                ctx.setLineWidth(width)
                ctx.setLineCap(.round)
                ctx.setLineJoin(.round)
                ctx.addLines(between: points)
                if closed { ctx.closePath() }
                ctx.strokePath()
            } else if let path = path(of: mark.kind) {
                ctx.addPath(path)
                ctx.fillPath()
            }
        }
    }

    /// Crosses at the board's four corners, `markOffset` out, the same on every copper page.
    private static func drawRegistration(_ bounds: CGRect, into ctx: CGContext) {
        ctx.setStrokeColor(CGColor(gray: 0, alpha: 1))
        ctx.setLineWidth(0.15)
        ctx.setLineCap(.butt)
        let box = bounds.insetBy(dx: -markOffset, dy: -markOffset)
        for x in [box.minX, box.maxX] {
            for y in [box.minY, box.maxY] {
                ctx.move(to: CGPoint(x: x - markArm, y: y))
                ctx.addLine(to: CGPoint(x: x + markArm, y: y))
                ctx.move(to: CGPoint(x: x, y: y - markArm))
                ctx.addLine(to: CGPoint(x: x, y: y + markArm))
            }
        }
        ctx.strokePath()
    }

    // MARK: Schematic page

    private static func drawSchematic(_ page: PrintPage, document: SchematicDocument, area: CGRect, box: CGRect,
                                      into pdf: CGContext) {
        let scene = SchematicScene(document)
        guard let rect = SchematicExport.bounds(of: scene) else { return }
        let k = pointsPerMM
        let s = page.scale
        let width = rect.width * s, height = rect.height * s
        let x = area.minX * k + (area.width * k - width) / 2
        let y = area.minY * k + (area.height * k - height) / 2
        let view = Canvas { context, _ in
            var c = context
            c.translateBy(x: x, y: y)
            c.scaleBy(x: s, y: s)
            SchematicExport.paint(scene, in: rect, into: &c)
        }
        .frame(width: box.width, height: box.height)
        let renderer = ImageRenderer(content: view)
        renderer.render { _, draw in
            pdf.saveGState()
            draw(pdf)
            pdf.restoreGState()
        }
    }

    // MARK: Footer

    private static func drawFooter(_ page: PrintPage, number: Int, of total: Int, name: String, box: CGRect,
                                   into pdf: CGContext) {
        let k = pointsPerMM
        let left = margin * k
        let baseline = margin * k
        pdf.setFillColor(CGColor(gray: 0, alpha: 1))
        pdf.setStrokeColor(CGColor(gray: 0, alpha: 1))
        var title = "\(name) · \(page.kind.title)"
        var detail = "Page \(number) of \(total)"
        if page.kind != .schematic {
            detail += " · 1:1, print at 100% (Actual Size)"
            if page.isTile {
                detail += " · tile \(page.column + 1) × \(page.row + 1) of \(page.columns) × \(page.rows)"
            }
        } else if page.scale < 0.999 {
            detail += " · scaled to \(Int((page.scale * 100).rounded())) %"
        }
        if title.count > 90 { title = String(title.prefix(89)) + "…" }
        text(title, at: CGPoint(x: left, y: baseline + 3.4 * k), size: 8, into: pdf)
        text(detail, at: CGPoint(x: left, y: baseline), size: 7, into: pdf)
        guard page.kind != .schematic else { return }
        // A 50 mm ruler to check the printer's scale: ticks every 10 mm.
        let right = box.width - margin * k
        let x0 = right - rulerLength * k
        pdf.setLineWidth(0.5)
        pdf.move(to: CGPoint(x: x0, y: baseline + 2 * k))
        pdf.addLine(to: CGPoint(x: right, y: baseline + 2 * k))
        for tick in 0...Int(rulerLength / 10) {
            let x = x0 + CGFloat(tick) * 10 * k
            pdf.move(to: CGPoint(x: x, y: baseline + 1 * k))
            pdf.addLine(to: CGPoint(x: x, y: baseline + 3 * k))
        }
        pdf.strokePath()
        text("\(Int(rulerLength)) mm", at: CGPoint(x: x0, y: baseline + 3.6 * k), size: 6, into: pdf)
    }

    private static func text(_ string: String, at point: CGPoint, size: CGFloat, into pdf: CGContext) {
        let font = CTFontCreateWithName("Helvetica" as CFString, size, nil)
        let line = CTLineCreateWithAttributedString(NSAttributedString(string: string, attributes: [
            kCTFontAttributeName as NSAttributedString.Key: font,
            kCTForegroundColorFromContextAttributeName as NSAttributedString.Key: true,
        ]))
        pdf.textMatrix = .identity
        pdf.textPosition = point
        CTLineDraw(line, pdf)
    }
}
