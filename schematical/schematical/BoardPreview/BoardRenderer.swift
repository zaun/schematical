import AppKit
import SwiftUI

// MARK: - Options

/// Which face of the board the preview shows.
nonisolated enum BoardRenderFace: String, CaseIterable, Identifiable, Sendable {
    case front, back

    var id: String { rawValue }
    var title: String { self == .front ? "Front" : "Back" }
    /// The board side seen from this face.
    var side: BoardSide { self == .front ? .top : .bottom }
}

/// How the preview looks at the board: straight down, or from above at an
/// angle (an axonometric drawing, still flat 2D: every part is a box or a
/// can raised off the board).
nonisolated enum BoardRenderAngle: String, CaseIterable, Identifiable, Sendable {
    case top, angled

    var id: String { rawValue }
    var title: String { self == .top ? "Top" : "Angled" }
    var symbol: String { self == .top ? "square" : "cube" }
}

/// The finish on bare copper: pads with no part on them, rings, mounting
/// holes. Soldered pads look like solder either way.
nonisolated enum BoardRenderFinish: String, CaseIterable, Identifiable, Sendable {
    case hasl, enig

    var id: String { rawValue }
    var title: String { self == .hasl ? "HASL (Silver)" : "ENIG (Gold)" }
}

// MARK: - Colour

/// An sRGB colour that can be shaded, for walls and gradients.
nonisolated struct Tint: Hashable, Sendable {
    var r, g, b: Double

    init(_ r: Double, _ g: Double, _ b: Double) {
        self.r = r; self.g = g; self.b = b
    }

    init(white: Double) { self.init(white, white, white) }

    init(_ color: Color) {
        let rgb = NSColor(color).usingColorSpace(.sRGB) ?? .black
        self.init(Double(rgb.redComponent), Double(rgb.greenComponent), Double(rgb.blueComponent))
    }

    var color: Color { Color(.sRGB, red: r, green: g, blue: b) }

    /// Darker (< 1) or lighter (> 1); lighter moves toward white.
    func shaded(_ f: Double) -> Tint {
        if f <= 1 { return Tint(r * f, g * f, b * f) }
        let t = min(f - 1, 1)
        return Tint(r + (1 - r) * t, g + (1 - g) * t, b + (1 - b) * t)
    }

    func mixed(with other: Tint, _ t: Double) -> Tint {
        Tint(r + (other.r - r) * t, g + (other.g - g) * t, b + (other.b - b) * t)
    }

    static let epoxy = Tint(0.11, 0.11, 0.12)
    static let ceramicIC = Tint(0.24, 0.24, 0.26)
    static let solder = Tint(0.80, 0.81, 0.83)
    static let tin = Tint(0.72, 0.73, 0.75)
    static let gold = Tint(0.88, 0.71, 0.32)
    static let fr4 = Tint(0.78, 0.71, 0.50)
    static let hole = Tint(0.05, 0.05, 0.06)
    static let steel = Tint(0.74, 0.75, 0.78)
}

// MARK: - Projection

/// Board millimetres to the screen: `flat` maps the board plane (z = 0),
/// and a point `z` mm above it moves `lift` points up the screen. Top-down,
/// `lift` is 0.
struct RenderProjection {
    let flat: CGAffineTransform
    let lift: CGFloat
    /// Board mm to the screen before scaling, for depth: a bigger `y` is
    /// nearer the viewer.
    let base: CGAffineTransform
    /// Screen points per mm.
    let scale: CGFloat

    var isAngled: Bool { lift > 0 }

    func point(_ p: CGPoint, _ z: CGFloat = 0) -> CGPoint {
        let q = p.applying(flat)
        return CGPoint(x: q.x, y: q.y - z * lift)
    }

    func depth(_ p: CGPoint) -> CGFloat { p.applying(base).y }

    /// Yaw and elevation of the angled view.
    static let yaw = 28.0 * .pi / 180
    static let elevation = 52.0 * .pi / 180

    /// Fits the board outline (and `height` mm of parts above it, `below`
    /// mm under it) into `size`, `margin` points in from each edge.
    static func fitting(_ outline: [CGPoint], face: BoardRenderFace, angle: BoardRenderAngle, size: CGSize,
                        height: CGFloat, below: CGFloat, margin: CGFloat = 36) -> RenderProjection {
        let (base, liftUnit, minX, maxX, minY, maxY) = extent(outline, face: face, angle: angle,
                                                              height: height, below: below)
        let s = max(0.01, min((size.width - 2 * margin) / (maxX - minX), (size.height - 2 * margin) / (maxY - minY)))
        let flat = base
            .concatenating(CGAffineTransform(scaleX: s, y: s))
            .concatenating(CGAffineTransform(translationX: size.width / 2 - s * (minX + maxX) / 2,
                                             y: size.height / 2 - s * (minY + maxY) / 2))
        return RenderProjection(flat: flat, lift: liftUnit * s, base: base, scale: s)
    }
}

extension RenderProjection {
    /// The view's turn and squash of the board plane, how far up the screen
    /// 1 mm of height moves, and the board's (plus parts') extent in that
    /// turned plane, in mm.
    static func extent(_ outline: [CGPoint], face: BoardRenderFace, angle: BoardRenderAngle,
                       height: CGFloat, below: CGFloat)
        -> (base: CGAffineTransform, lift: CGFloat, minX: CGFloat, maxX: CGFloat, minY: CGFloat, maxY: CGFloat) {
        var base = face == .back ? CGAffineTransform(scaleX: -1, y: 1) : .identity
        var liftUnit: CGFloat = 0
        if angle == .angled {
            base = base.concatenating(CGAffineTransform(rotationAngle: yaw))
                .concatenating(CGAffineTransform(scaleX: 1, y: sin(elevation)))
            liftUnit = cos(elevation)
        }
        let points = (outline.isEmpty ? [.zero, CGPoint(x: 100, y: 80)] : outline).map { $0.applying(base) }
        var minX = points.map(\.x).min()!, maxX = points.map(\.x).max()!
        var minY = points.map(\.y).min()!, maxY = points.map(\.y).max()!
        minY -= height * liftUnit
        maxY += below * liftUnit
        if maxX - minX < 1 { minX -= 1; maxX += 1 }
        if maxY - minY < 1 { minY -= 1; maxY += 1 }
        return (base, liftUnit, minX, maxX, minY, maxY)
    }
}

// MARK: - Solids

/// One thing standing on the board, drawn back to front.
struct RenderSolid {
    /// Bigger is nearer the viewer (angled) or higher (top-down).
    var depth: CGFloat
    /// Its height above the board, for its shadow.
    var height: CGFloat
    /// Its footprint on the board (board mm), for its shadow.
    var shadow: Path?
    var draw: (inout GraphicsContext) -> Void
}

/// Draws boxes and cans on the board through a projection.
struct SolidPainter {
    let projection: RenderProjection

    /// Draws `body` in board mm on the plane `z` mm up.
    func face(_ z: CGFloat, in context: inout GraphicsContext, _ body: (inout GraphicsContext) -> Void) {
        context.drawLayer { layer in
            layer.translateBy(x: 0, y: -z * projection.lift)
            layer.concatenate(projection.flat)
            body(&layer)
        }
    }

    /// A light from the upper left: walls facing left are lighter.
    private func wallShade(_ normal: CGPoint) -> Double {
        let length = max(hypot(normal.x, normal.y), 0.0001)
        return 0.58 + 0.22 * Double(-normal.x / length)
    }

    /// Walls of a prism over a polygon (board mm) from `z0` to `z1`; only
    /// the walls facing the viewer.
    func walls(_ polygon: [CGPoint], from z0: CGFloat, to z1: CGFloat, tint: Tint, in context: inout GraphicsContext) {
        guard projection.isAngled, polygon.count > 2, z1 > z0 else { return }
        let screen = polygon.map { projection.point($0) }
        // Winding on screen, so "outward" is right whichever way the
        // polygon runs or the view mirrors.
        var area: CGFloat = 0
        for i in screen.indices {
            let a = screen[i], b = screen[(i + 1) % screen.count]
            area += a.x * b.y - b.x * a.y
        }
        let sign: CGFloat = area > 0 ? 1 : -1
        for i in polygon.indices {
            let a = polygon[i], b = polygon[(i + 1) % polygon.count]
            let sa = screen[i], sb = screen[(i + 1) % screen.count]
            // Outward normal on screen (y down): for a clockwise-on-screen
            // polygon (positive area), (dy, -dx).
            let normal = CGPoint(x: sign * (sb.y - sa.y), y: -sign * (sb.x - sa.x))
            guard normal.y > 0.0001 else { continue }
            let quad = Path { p in
                p.addLines([projection.point(a, z0), projection.point(b, z0), projection.point(b, z1), projection.point(a, z1)])
                p.closeSubpath()
            }
            let shade = tint.shaded(wallShade(normal))
            context.fill(quad, with: .color(shade.color))
            context.stroke(quad, with: .color(shade.color), lineWidth: 0.5)
        }
    }

    /// A box over `rect` (board mm) from `z0` to `z1`, its top in `top`
    /// (else `tint`), then `decorate` drawn on the top in board mm.
    func box(_ rect: CGRect, from z0: CGFloat, to z1: CGFloat, tint: Tint, top: Tint? = nil, radius: CGFloat = 0,
             in context: inout GraphicsContext, decorate: ((inout GraphicsContext) -> Void)? = nil) {
        let corners = [CGPoint(x: rect.minX, y: rect.minY), CGPoint(x: rect.maxX, y: rect.minY),
                       CGPoint(x: rect.maxX, y: rect.maxY), CGPoint(x: rect.minX, y: rect.maxY)]
        walls(corners, from: z0, to: z1, tint: tint, in: &context)
        let topTint = top ?? tint
        face(z1, in: &context) { layer in
            let shape = radius > 0 ? Path(roundedRect: rect, cornerRadius: radius) : Path(rect)
            // A soft sheen across the top.
            layer.fill(shape, with: .linearGradient(
                Gradient(colors: [topTint.shaded(1.08).color, topTint.color, topTint.shaded(0.9).color]),
                startPoint: CGPoint(x: rect.minX, y: rect.minY), endPoint: CGPoint(x: rect.maxX, y: rect.maxY)))
            decorate?(&layer)
        }
    }

    /// An upright can: a circle (board mm) from `z0` to `z1`.
    func can(_ centre: CGPoint, radius r: CGFloat, from z0: CGFloat, to z1: CGFloat, tint: Tint, top: Tint? = nil,
             in context: inout GraphicsContext, decorate: ((inout GraphicsContext) -> Void)? = nil) {
        let circle = CGRect(x: centre.x - r, y: centre.y - r, width: 2 * r, height: 2 * r)
        if projection.isAngled, z1 > z0 {
            // The circle projects to an upright ellipse; the side is the
            // ellipse swept from z0 to z1.
            let ellipse = Path(ellipseIn: circle).applying(projection.flat).boundingRect
            let bottom = ellipse.offsetBy(dx: 0, dy: -z0 * projection.lift)
            let topE = ellipse.offsetBy(dx: 0, dy: -z1 * projection.lift)
            let side = Path { p in
                p.addEllipse(in: bottom)
                p.addRect(CGRect(x: bottom.minX, y: topE.midY, width: bottom.width, height: bottom.midY - topE.midY))
            }
            context.fill(side, with: .linearGradient(
                Gradient(colors: [tint.shaded(0.95).color, tint.shaded(1.12).color, tint.shaded(0.7).color,
                                  tint.shaded(0.45).color]),
                startPoint: CGPoint(x: bottom.minX, y: 0), endPoint: CGPoint(x: bottom.maxX, y: 0)))
        }
        let topTint = top ?? tint
        face(z1, in: &context) { layer in
            layer.fill(Path(ellipseIn: circle), with: .radialGradient(
                Gradient(colors: [topTint.shaded(1.12).color, topTint.color, topTint.shaded(0.82).color]),
                center: CGPoint(x: centre.x - r * 0.3, y: centre.y - r * 0.3), startRadius: 0, endRadius: r * 1.4))
            decorate?(&layer)
        }
    }
}

// MARK: - Renderer

/// Draws a board as it might look made: solder mask, copper under it,
/// bare pads, silkscreen, and each part on the side shown as a plain
/// generic body (a blank DIP, a can, a header) — no logos, no markings.
/// Parts on the other side show only as solder joints where their leads
/// come through.
struct BoardRenderer {
    let document: SchematicDocument
    let face: BoardRenderFace
    let angle: BoardRenderAngle
    let finish: BoardRenderFinish

    static let thickness: CGFloat = 1.6
    /// Space between the board (and its parts) and the picture's edge, in points.
    var margin: CGFloat = 36

    /// How far up the fit reaches: the tallest part, or 2 mm.
    static func height(of looks: [PartLook]) -> CGFloat {
        max(looks.map(\.height).max() ?? 0, 2)
    }

    /// The picture's size, in points, that fits the board at `pointsPerMM`
    /// with `margin` points round it: the size `draw` fills edge to edge.
    func fittedSize(pointsPerMM: CGFloat) -> CGSize {
        let looks = document.chips.filter { $0.board != nil && $0.footprint != nil && $0.board?.side == face.side }
            .map { PartLook.of($0) }
        let e = RenderProjection.extent(document.board.outline, face: face, angle: angle,
                                        height: Self.height(of: looks), below: Self.thickness + 2)
        return CGSize(width: ceil((e.maxX - e.minX) * pointsPerMM + 2 * margin),
                      height: ceil((e.maxY - e.minY) * pointsPerMM + 2 * margin))
    }

    func draw(in context: inout GraphicsContext, size: CGSize) {
        let board = document.board
        let side = face.side
        let chips = document.chips.filter { $0.board != nil && $0.footprint != nil }
        let looks = chips.filter { $0.board?.side == side }.map { PartLook.of($0) }
        // Fit the board: parts placed off it are not part of the product.
        let projection = RenderProjection.fitting(board.outline, face: face, angle: angle, size: size,
                                                  height: Self.height(of: looks), below: Self.thickness + 2,
                                                  margin: margin)
        let painter = SolidPainter(projection: projection)
        let outline = board.outline
        let boardShape = Path { path in path.addLines(outline); path.closeSubpath() }
        let mask = Tint(board.color.fill)
        let finishTint = finish == .enig ? Tint.gold : Tint.tin

        // The board's shadow on the backdrop.
        context.drawLayer { layer in
            layer.addFilter(.blur(radius: projection.isAngled ? 18 : 10))
            let offset = projection.isAngled
                ? CGSize(width: 0, height: (Self.thickness + 3) * projection.lift)
                : CGSize(width: 6, height: 10)
            layer.translateBy(x: offset.width, y: offset.height)
            layer.concatenate(projection.flat)
            layer.fill(boardShape, with: .color(.black.opacity(0.55)))
        }

        // The board's edge: FR-4 under a thin skin of mask.
        if projection.isAngled {
            painter.walls(outline, from: -Self.thickness, to: 0, tint: Tint.fr4.mixed(with: mask, 0.25), in: &context)
            painter.walls(outline, from: -0.08, to: 0, tint: mask, in: &context)
            painter.walls(outline, from: -Self.thickness, to: -Self.thickness + 0.08, tint: mask, in: &context)
        }

        // The surface, flat.
        painter.face(0, in: &context) { layer in
            drawSurface(in: &layer, side: side, shape: boardShape, mask: mask, finish: finishTint, chips: chips)
        }

        // What stands on it: shadows first, then back to front.
        var solids = looks.flatMap { $0.solids(painter: painter) }
        if projection.isAngled {
            solids.sort { $0.depth < $1.depth }
        } else {
            solids.sort { $0.height < $1.height }
        }
        context.drawLayer { layer in
            layer.addFilter(.blur(radius: max(1, projection.scale * 0.4)))
            for solid in solids {
                guard let shadow = solid.shadow, solid.height > 0.2 else { continue }
                let h = solid.height * projection.scale
                let offset = projection.isAngled
                    ? CGSize(width: min(h * 0.12, 10), height: min(h * 0.06, 6))
                    : CGSize(width: min(h * 0.28, 30), height: min(h * 0.4, 40))
                layer.drawLayer { s in
                    s.translateBy(x: offset.width, y: offset.height)
                    s.concatenate(projection.flat)
                    s.fill(shadow, with: .color(.black.opacity(0.4)))
                }
            }
        }
        for solid in solids { solid.draw(&context) }
    }

    /// Everything flat on the side shown, in board mm.
    private func drawSurface(in context: inout GraphicsContext, side: BoardSide, shape: Path, mask: Tint,
                             finish: Tint, chips: [PlacedChip]) {
        let board = document.board
        let bounds = shape.boundingRect
        context.fill(shape, with: .linearGradient(
            Gradient(colors: [mask.shaded(1.06).color, mask.color, mask.shaded(0.88).color]),
            startPoint: CGPoint(x: bounds.minX, y: bounds.minY), endPoint: CGPoint(x: bounds.maxX, y: bounds.maxY)))

        // Copper under the mask shows as a lighter tint of it.
        let underMask = mask.mixed(with: Tint(0.85, 0.70, 0.40), 0.18).shaded(1.05).color
        let layer = side.copperLayer
        if board.copperLayers.contains(layer) {
            if board.style(of: layer).type == .plane {
                drawPlane(on: layer, colour: underMask, shape: shape, in: &context)
            }
            for trace in document.traces where trace.layer == layer {
                context.stroke(PCBCanvasView.path(trace.points), with: .color(underMask),
                               style: PCBCanvasView.traceStyle(width: trace.widthMM))
            }
        }
        // Tented vias: a ring under the mask and the hole.
        for via in document.vias {
            context.fill(Path(ellipseIn: via.rect), with: .color(underMask))
        }

        // Bare copper and solder. A pad with a part on it is soldered.
        func metal(_ path: Path, _ tint: Tint, _ rect: CGRect) {
            context.fill(path, with: .linearGradient(
                Gradient(colors: [tint.shaded(1.15).color, tint.color, tint.shaded(0.8).color]),
                startPoint: CGPoint(x: rect.minX, y: rect.minY), endPoint: CGPoint(x: rect.maxX, y: rect.maxY)))
        }
        for finger in board.fingers where !finger.isThroughHole && finger.side == side {
            metal(Path(finger.rect), .gold, finger.rect)
        }
        for chip in chips where chip.board?.side == side {
            for pad in chip.boardPads where pad.drill == 0 {
                metal(PCBCanvasView.outline(of: pad), .solder, pad.rect)
            }
        }
        let boardPads = (board.fingers.filter(\.isThroughHole) + board.extraPads).map(\.pad) + board.holes.map(\.pad)
            + document.holes.map(\.pad)
        for pad in boardPads where pad.plated {
            metal(PCBCanvasView.outline(of: pad), finish, pad.rect)
        }
        for chip in chips {
            let far = chip.board?.side != side
            for pad in chip.boardPads where pad.drill > 0 {
                if far {
                    // The leads come through here: a solder joint and the
                    // clipped lead.
                    let r = pad.rect
                    context.fill(PCBCanvasView.outline(of: pad), with: .radialGradient(
                        Gradient(colors: [Tint.solder.shaded(1.25).color, Tint.solder.color, Tint.solder.shaded(0.62).color]),
                        center: CGPoint(x: r.midX - r.width * 0.15, y: r.midY - r.height * 0.15),
                        startRadius: 0, endRadius: max(r.width, r.height) * 0.6))
                    let tip = pad.drill * 0.55
                    context.fill(Path(ellipseIn: CGRect(x: r.midX - tip / 2, y: r.midY - tip / 2, width: tip, height: tip)),
                                 with: .color(Tint.tin.shaded(1.2).color))
                } else if pad.plated {
                    metal(PCBCanvasView.outline(of: pad), finish, pad.rect)
                }
            }
        }

        // Holes.
        let holeColour = Tint.hole.color
        for via in document.vias {
            let d = via.drill
            context.fill(Path(ellipseIn: CGRect(x: via.centre.x - d / 2, y: via.centre.y - d / 2, width: d, height: d)),
                         with: .color(holeColour))
        }
        for pad in boardPads {
            if let hole = pad.holeRect {
                context.fill(Path(roundedRect: hole, cornerRadius: pad.drill / 2), with: .color(holeColour))
            }
        }
        for chip in chips where chip.board?.side == side {
            for pad in chip.boardPads where pad.drill > 0 {
                if let hole = pad.holeRect {
                    context.fill(Path(roundedRect: hole, cornerRadius: pad.drill / 2), with: .color(holeColour))
                }
            }
        }

        // Silkscreen: only what is printed.
        let ink = board.color.silkscreen
        for chip in chips where chip.board?.side == side {
            guard let placement = chip.board, let body = chip.boardBody else { continue }
            if placement.outline == .silkscreen {
                let path = body.round ? Path(ellipseIn: body.rect) : Path(body.rect)
                context.stroke(path, with: .color(ink), lineWidth: 0.15)
            }
            if placement.pin1 == .silkscreen, let mark = chip.pin1Mark {
                context.fill(Path(ellipseIn: CGRect(x: mark.x - 0.2, y: mark.y - 0.2, width: 0.4, height: 0.4)),
                             with: .color(ink))
            }
            if placement.reference == .silkscreen, let at = chip.boardReferenceCentre {
                context.drawLayer { l in
                    l.translateBy(x: at.x, y: at.y)
                    if side == .bottom { l.scaleBy(x: -1, y: 1) }
                    l.draw(Text(chip.reference).font(.system(size: 1, weight: .medium)).foregroundStyle(ink),
                           at: .zero, anchor: .center)
                }
            }
        }
        for text in document.texts where text.side == side && text.layer == .silkscreen {
            let width = text.side == .bottom ? text.localSize.width : 0
            context.drawLayer { l in
                l.translateBy(x: text.anchor.x, y: text.anchor.y)
                l.rotate(by: .degrees(Double(text.rotation)))
                if text.side == .bottom {
                    l.translateBy(x: width, y: 0)
                    l.scaleBy(x: -1, y: 1)
                }
                l.scaleBy(x: text.drawScale, y: text.drawScale)
                l.draw(Text(text.text).font(Font(text.font)).foregroundStyle(ink), at: .zero, anchor: .topLeading)
            }
        }
    }

    /// A Plane's copper under the mask, with its clearances cut out.
    private func drawPlane(on layer: CopperLayer, colour: Color, shape: Path, in context: inout GraphicsContext) {
        let rules = document.board.rules
        let style = document.board.style(of: layer)
        let cuts = PCBCanvasView.planeCuts(on: layer, copper: document.connectivity())
        let clearance = style.clearance(rules: rules), edge = rules.minCopperToEdge
        context.drawLayer { plane in
            plane.fill(shape, with: .color(colour))
            plane.blendMode = .clear
            func clear(_ path: Path, by distance: Double) {
                guard !path.isEmpty else { return }
                plane.fill(path, with: .color(.black))
                if distance > 0 {
                    plane.stroke(path, with: .color(.black), style: StrokeStyle(lineWidth: 2 * distance, lineJoin: .round))
                }
            }
            if edge > 0 {
                plane.stroke(shape, with: .color(.black), style: StrokeStyle(lineWidth: 2 * edge, lineJoin: .round))
            }
            clear(cuts.copper, by: clearance)
            clear(cuts.edges, by: edge)
            clear(cuts.drills, by: 0)
        }
    }
}
