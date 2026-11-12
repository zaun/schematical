import CoreGraphics
import CoreText
import Foundation

/// A shape of copper, mask or ink, in board mm (y down).
nonisolated enum GerberShape: Equatable {
    case circle(centre: CGPoint, diameter: Double)
    /// An axis-aligned rectangle.
    case rect(CGRect)
    /// One closed outline (no holes): a pad's ellipse, rounded or chamfered
    /// rectangle, flattened.
    case polygon([CGPoint])
    /// A line `diameter` wide with round ends, `a` to `b`: a slotted hole.
    case capsule(a: CGPoint, b: CGPoint, diameter: Double)
}

/// One Gerber X2 (RS-274X) layer file being built: apertures, then the
/// image in draw order. Coordinates are mm, 6 decimals (`%FSLAX46Y46*%`).
/// Board y grows down; Gerber y grows up, so `y` is written as `top − y`,
/// which keeps the whole board at y ≥ 0.
nonisolated struct GerberWriter {
    enum Aperture: Hashable {
        /// Diameter in nm.
        case circle(Int)
        /// Width and height in nm.
        case rect(Int, Int)
    }

    /// `%TF…*%` attributes, without the `%TF.` and `*%`.
    let attributes: [String]
    /// The board's top edge (mm): Gerber y = `top` − board y.
    let top: Double

    private var apertures: [Aperture: Int] = [:]
    private var apertureOrder: [Aperture] = []
    private var body: [String] = []
    private var current: Int?
    private var dark = true

    init(attributes: [String], top: Double) {
        self.attributes = attributes
        self.top = top
    }

    /// Whether anything has been drawn.
    var isEmpty: Bool { body.isEmpty }

    // MARK: Primitives

    /// Dark (adds) or clear (erases what is already drawn).
    mutating func setDark(_ on: Bool) {
        guard on != dark else { return }
        dark = on
        body.append(on ? "%LPD*%" : "%LPC*%")
    }

    mutating func flash(_ aperture: Aperture, at point: CGPoint) {
        select(aperture)
        body.append("\(xy(point))D03*")
    }

    /// A polyline stroked with a round pen `width` mm wide (round ends and
    /// joins). `closed` draws back to the first point.
    mutating func stroke(_ points: [CGPoint], width: Double, closed: Bool = false) {
        guard let first = points.first, width > 0 else { return }
        let aperture = Aperture.circle(Self.nm(width))
        if points.count == 1 || (points.dropFirst().allSatisfy { TraceRoute.same($0, first) }) {
            flash(aperture, at: first)
            return
        }
        select(aperture)
        body.append("\(xy(first))D02*")
        for point in points.dropFirst() { body.append("\(xy(point))D01*") }
        if closed, let last = points.last, !TraceRoute.same(last, first) { body.append("\(xy(first))D01*") }
    }

    /// A filled outline (G36/G37). Fewer than 3 points draws nothing.
    mutating func region(_ contour: [CGPoint]) {
        guard contour.count >= 3 else { return }
        body.append("G36*")
        body.append("\(xy(contour[0]))D02*")
        for point in contour.dropFirst() { body.append("\(xy(point))D01*") }
        if !TraceRoute.same(contour[contour.count - 1], contour[0]) { body.append("\(xy(contour[0]))D01*") }
        body.append("G37*")
    }

    /// `shape`, grown by `grow` mm all round (its outline stroked with a
    /// pen 2 × `grow` wide, so corners round off as a clearance does).
    mutating func fill(_ shape: GerberShape, grownBy grow: Double = 0) {
        let grow = max(grow, 0)
        switch shape {
        case let .circle(centre, diameter):
            let d = diameter + 2 * grow
            if d > 0 { flash(.circle(Self.nm(d)), at: centre) }
        case let .rect(rect):
            guard rect.width > 0, rect.height > 0 else { return }
            if grow == 0 {
                flash(.rect(Self.nm(rect.width), Self.nm(rect.height)), at: CGPoint(x: rect.midX, y: rect.midY))
            } else {
                let corners = [CGPoint(x: rect.minX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.minY),
                               CGPoint(x: rect.maxX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.maxY)]
                region(corners)
                stroke(corners, width: 2 * grow, closed: true)
            }
        case let .polygon(points):
            region(points)
            if grow > 0 { stroke(points, width: 2 * grow, closed: true) }
        case let .capsule(a, b, diameter):
            stroke([a, b], width: diameter + 2 * grow)
        }
    }

    /// One glyph's outlines: those wound like its largest (its outer
    /// edges) dark, then the rest (its counters, the hole in an O) clear.
    /// A clear counter also erases anything drawn before it under it, so
    /// a layer draws its text first.
    mutating func glyph(_ contours: [[CGPoint]]) {
        guard let outer = contours.max(by: { abs(Self.area($0)) < abs(Self.area($1)) }) else { return }
        let positive = Self.area(outer) > 0
        let (solid, holes) = contours.reduce(into: ([[CGPoint]](), [[CGPoint]]())) { split, contour in
            if (Self.area(contour) > 0) == positive { split.0.append(contour) } else { split.1.append(contour) }
        }
        setDark(true)
        solid.forEach { region($0) }
        if !holes.isEmpty {
            setDark(false)
            holes.forEach { region($0) }
            setDark(true)
        }
    }

    // MARK: Output

    /// The finished file.
    func data() -> Data {
        var lines = ["G04 Schematical Gerber export*"]
        lines += attributes.map { "%TF.\($0)*%" }
        lines += ["%FSLAX46Y46*%", "%MOMM*%", "%LPD*%", "G01*", "G75*"]
        for aperture in apertureOrder {
            let code = apertures[aperture]!
            switch aperture {
            case let .circle(d): lines.append("%ADD\(code)C,\(Self.mm(d))*%")
            case let .rect(w, h): lines.append("%ADD\(code)R,\(Self.mm(w))X\(Self.mm(h))*%")
            }
        }
        lines += body
        lines.append("M02*")
        return Data((lines.joined(separator: "\n") + "\n").utf8)
    }

    // MARK: Helpers

    private mutating func select(_ aperture: Aperture) {
        let code: Int
        if let known = apertures[aperture] {
            code = known
        } else {
            code = 10 + apertureOrder.count
            apertures[aperture] = code
            apertureOrder.append(aperture)
        }
        if current != code {
            body.append("D\(code)*")
            current = code
        }
    }

    private func xy(_ point: CGPoint) -> String {
        "X\(Self.nm(point.x))Y\(Self.nm(top - point.y))"
    }

    static func nm(_ mm: Double) -> Int { Int((mm * 1_000_000).rounded()) }

    static func mm(_ nm: Int) -> String { String(format: "%.6f", Double(nm) / 1_000_000) }

    /// Shoelace area; its sign is the winding.
    static func area(_ points: [CGPoint]) -> Double {
        guard points.count >= 3 else { return 0 }
        var sum = 0.0
        for (a, b) in zip(points, points.dropFirst() + [points[0]]) { sum += a.x * b.y - b.x * a.y }
        return sum / 2
    }
}

// MARK: - Outlines

nonisolated enum GerberOutline {
    /// mm. Curves are cut into straight steps no longer than this.
    static let step = 0.05

    /// A path's closed outlines as polygons, its curves flattened.
    static func contours(of path: CGPath) -> [[CGPoint]] {
        var contours: [[CGPoint]] = []
        var current: [CGPoint] = []
        func finish() {
            var cleaned = TraceRoute.clean(current)
            if cleaned.count > 1, let first = cleaned.first, TraceRoute.same(first, cleaned[cleaned.count - 1]) {
                cleaned.removeLast()
            }
            if cleaned.count >= 3 { contours.append(cleaned) }
            current = []
        }
        path.applyWithBlock { pointer in
            let element = pointer.pointee
            let p = element.points
            switch element.type {
            case .moveToPoint:
                finish()
                current = [p[0]]
            case .addLineToPoint:
                current.append(p[0])
            case .addQuadCurveToPoint:
                guard let start = current.last else { return }
                current += quadratic(start, p[0], p[1])
            case .addCurveToPoint:
                guard let start = current.last else { return }
                current += cubic(start, p[0], p[1], p[2])
            case .closeSubpath:
                finish()
            @unknown default:
                break
            }
        }
        finish()
        return contours
    }

    /// Steps for a curve whose control polygon is `length` mm long.
    private static func steps(_ length: Double) -> Int {
        min(max(Int((length / step).rounded(.up)), 2), 64)
    }

    private static func distance(_ a: CGPoint, _ b: CGPoint) -> Double {
        Double(hypot(b.x - a.x, b.y - a.y))
    }

    /// A quadratic Bézier's points after `a`.
    private static func quadratic(_ a: CGPoint, _ c: CGPoint, _ b: CGPoint) -> [CGPoint] {
        let n = steps(distance(a, c) + distance(c, b))
        return (1...n).map { i in
            let t = Double(i) / Double(n), u = 1 - t
            let wa = u * u, wc = 2 * u * t, wb = t * t
            let x: Double = wa * a.x + wc * c.x + wb * b.x
            let y: Double = wa * a.y + wc * c.y + wb * b.y
            return CGPoint(x: x, y: y)
        }
    }

    /// A cubic Bézier's points after `a`.
    private static func cubic(_ a: CGPoint, _ c1: CGPoint, _ c2: CGPoint, _ b: CGPoint) -> [CGPoint] {
        let n = steps(distance(a, c1) + distance(c1, c2) + distance(c2, b))
        return (1...n).map { i in
            let t = Double(i) / Double(n), u = 1 - t
            let wa = u * u * u, w1 = 3 * u * u * t, w2 = 3 * u * t * t, wb = t * t * t
            let x: Double = wa * a.x + w1 * c1.x + (w2 * c2.x + wb * b.x)
            let y: Double = wa * a.y + w1 * c1.y + (w2 * c2.y + wb * b.y)
            return CGPoint(x: x, y: y)
        }
    }

    /// `text` set in `font`, each glyph's outlines through `transform`.
    /// Text space is the font's points, y down, the first line's top at
    /// y = 0, lines left-aligned at x = 0. Returns the laid-out size too
    /// (font points).
    static func glyphs(_ text: String, font: CTFont, transform: (CGSize) -> CGAffineTransform)
        -> [[[CGPoint]]] {
        let string = NSAttributedString(string: text, attributes: [.init(kCTFontAttributeName as String): font])
        let setter = CTFramesetterCreateWithAttributedString(string)
        let big: CGFloat = 1_000_000
        let size = CTFramesetterSuggestFrameSizeWithConstraints(setter, CFRange(location: 0, length: 0), nil,
                                                                CGSize(width: big, height: big), nil)
        let frame = CTFramesetterCreateFrame(setter, CFRange(location: 0, length: 0),
                                             CGPath(rect: CGRect(x: 0, y: 0, width: big, height: big), transform: nil),
                                             nil)
        let lines = CTFrameGetLines(frame) as? [CTLine] ?? []
        var origins = [CGPoint](repeating: .zero, count: lines.count)
        CTFrameGetLineOrigins(frame, CFRange(location: 0, length: 0), &origins)
        let outer = transform(size)
        var result: [[[CGPoint]]] = []
        for (line, origin) in zip(lines, origins) {
            for run in CTLineGetGlyphRuns(line) as? [CTRun] ?? [] {
                let count = CTRunGetGlyphCount(run)
                guard count > 0 else { continue }
                let attributes = CTRunGetAttributes(run) as NSDictionary
                let runFont = attributes[kCTFontAttributeName as String].map { $0 as! CTFont } ?? font
                var glyphs = [CGGlyph](repeating: 0, count: count)
                var positions = [CGPoint](repeating: .zero, count: count)
                CTRunGetGlyphs(run, CFRange(location: 0, length: 0), &glyphs)
                CTRunGetPositions(run, CFRange(location: 0, length: 0), &positions)
                for (glyph, position) in zip(glyphs, positions) {
                    // Glyph space is y up from the baseline; text space is y
                    // down from the frame's top.
                    let place = CGAffineTransform(a: 1, b: 0, c: 0, d: -1,
                                                  tx: origin.x + position.x, ty: big - origin.y - position.y)
                    var whole = place.concatenating(outer)
                    guard let path = CTFontCreatePathForGlyph(runFont, glyph, &whole) else { continue }
                    let contours = contours(of: path)
                    if !contours.isEmpty { result.append(contours) }
                }
            }
        }
        return result
    }
}
