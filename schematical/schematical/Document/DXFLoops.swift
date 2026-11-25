import CoreGraphics
import Foundation

// DXF entities to closed loops (plan-65-H): flatten the curves to polylines
// (no point more than `BoardCutout.flatness` from the true curve), join the
// pieces that meet end to end, and keep the closed ones.

/// A closed loop of a drawing: mm, in the DXF's own y-up frame, counter-clockwise
/// (positive area), no repeated end point.
nonisolated struct DXFLoop: Hashable, Sendable {
    var points: [CGPoint]
    var layer: String

    var area: Double { abs(BoardGeometry.area(points)) }
}

nonisolated struct DXFLoopSet: Hashable, Sendable {
    var loops: [DXFLoop] = []
    /// Chains of entities whose ends do not meet.
    var openChains = 0
    /// Loops dropped because they cross themselves or enclose nothing.
    var crossingSelf = 0
}

nonisolated enum DXFLoops {
    /// Two ends this close (mm) are one point.
    static let joinTolerance = 0.01

    /// The closed loops of the entities on `layers`, scaled by `scale` (drawing units to mm).
    static func build(_ drawing: DXFDrawing, layers: Set<String>, scale: Double) -> DXFLoopSet {
        var set = DXFLoopSet()
        var open: [(points: [CGPoint], layer: String)] = []
        func add(_ points: [CGPoint], layer: String) {
            let cleaned = removeRepeats(points)
            guard cleaned.count >= 3 else { return }
            let oriented = BoardGeometry.area(cleaned) < 0 ? Array(cleaned.reversed()) : cleaned
            if abs(BoardGeometry.area(oriented)) < 1e-6 || BoardGeometry.crossesItself(oriented) {
                set.crossingSelf += 1
            } else {
                set.loops.append(DXFLoop(points: oriented, layer: layer))
            }
        }
        for entity in drawing.entities where layers.contains(entity.layer) {
            let piece = flatten(entity, scale: scale)
            guard piece.points.count >= 2 else { continue }
            if piece.closed {
                add(piece.points, layer: entity.layer)
            } else if let first = piece.points.first, let last = piece.points.last, distance(first, last) < joinTolerance,
                      piece.points.count >= 4 {
                add(Array(piece.points.dropLast()), layer: entity.layer)
            } else {
                open.append((piece.points, entity.layer))
            }
        }

        // Chain the open pieces: grow a chain at its tail, then at its head, with the unused piece whose end is
        // nearest (within the tolerance; ties go to the earlier piece).
        var used = Array(repeating: false, count: open.count)
        for start in open.indices where !used[start] {
            used[start] = true
            var chain = open[start].points
            for atTail in [true, false] {
                while true {
                    let end = atTail ? chain[chain.count - 1] : chain[0]
                    var best: (index: Int, reversed: Bool, gap: Double)?
                    for j in open.indices where !used[j] {
                        let a = distance(end, open[j].points[0]), b = distance(end, open[j].points[open[j].points.count - 1])
                        for (gap, reversed) in [(a, false), (b, true)] where gap < joinTolerance {
                            if best == nil || gap < best!.gap - 1e-12 { best = (j, reversed, gap) }
                        }
                    }
                    guard let found = best else { break }
                    used[found.index] = true
                    var next = open[found.index].points
                    // Tail: the piece starts at the tail; head: it ends at the head.
                    if atTail == found.reversed { next.reverse() }
                    if atTail { chain += next.dropFirst() } else { chain = next.dropLast() + chain }
                }
            }
            if chain.count >= 4, distance(chain[0], chain[chain.count - 1]) < joinTolerance {
                add(Array(chain.dropLast()), layer: open[start].layer)
            } else {
                set.openChains += 1
            }
        }
        return set
    }

    // MARK: Flattening

    /// An entity as one polyline in mm, and whether it is closed by itself.
    static func flatten(_ entity: DXFEntity, scale: Double) -> (points: [CGPoint], closed: Bool) {
        func s(_ p: CGPoint) -> CGPoint { CGPoint(x: p.x * scale, y: p.y * scale) }
        switch entity {
        case let .line(_, a, b):
            return ([s(a), s(b)], false)
        case let .circle(_, centre, radius):
            let r = abs(radius * scale)
            guard r > 1e-9 else { return ([], false) }
            let n = 4 * BoardCutout.quarterSegments(radius: r)
            let c = s(centre)
            return ((0..<n).map { i in
                let angle = 2 * Double.pi * Double(i) / Double(n)
                return CGPoint(x: c.x + r * cos(angle), y: c.y + r * sin(angle))
            }, true)
        case let .arc(_, centre, radius, start, end):
            let r = abs(radius * scale)
            guard r > 1e-9 else { return ([], false) }
            var sweep = (end - start) * Double.pi / 180
            while sweep <= 0 { sweep += 2 * Double.pi }
            return (arcPoints(centre: s(centre), radius: r, from: start * Double.pi / 180, sweep: sweep), false)
        case let .polyline(_, vertices, closed):
            return (polylinePoints(vertices.map { DXFVertex(point: s($0.point), bulge: $0.bulge) }, closed: closed), closed)
        case let .ellipse(_, centre, major, ratio, start, end):
            let a = hypot(major.x, major.y) * scale
            guard a > 1e-9, ratio > 0 else { return ([], false) }
            var sweep = end - start
            while sweep <= 1e-12 { sweep += 2 * Double.pi }
            let full = abs(sweep - 2 * Double.pi) < 1e-9
            let n = max(8, Int((Double(4 * BoardCutout.quarterSegments(radius: a)) * sweep / (2 * Double.pi)).rounded(.up)))
            let c = s(centre)
            let mx = major.x * scale, my = major.y * scale
            let nx = -my * ratio, ny = mx * ratio
            let count = full ? n : n + 1
            return ((0..<count).map { i in
                let t = start + sweep * Double(i) / Double(n)
                return CGPoint(x: c.x + mx * cos(t) + nx * sin(t), y: c.y + my * cos(t) + ny * sin(t))
            }, full)
        case let .spline(_, spline):
            return splinePoints(spline, scale: scale)
        }
    }

    /// Points on the arc of `radius` round `centre` from angle `from` through `sweep` (radians; negative runs
    /// clockwise), both ends included, enough segments for the flatness.
    static func arcPoints(centre: CGPoint, radius: Double, from: Double, sweep: Double) -> [CGPoint] {
        let quarter = BoardCutout.quarterSegments(radius: radius)
        let n = max(1, Int((abs(sweep) / (Double.pi / 2) * Double(quarter)).rounded(.up)))
        return (0...n).map { i in
            let angle = from + sweep * Double(i) / Double(n)
            return CGPoint(x: centre.x + radius * cos(angle), y: centre.y + radius * sin(angle))
        }
    }

    /// A polyline's points (mm), each segment with a bulge an arc; `closed` adds the segment back to the first.
    static func polylinePoints(_ vertices: [DXFVertex], closed: Bool) -> [CGPoint] {
        guard vertices.count >= 2 else { return [] }
        var out: [CGPoint] = [vertices[0].point]
        let count = closed ? vertices.count : vertices.count - 1
        for i in 0..<count {
            let a = vertices[i], b = vertices[(i + 1) % vertices.count]
            if abs(a.bulge) > 1e-9, distance(a.point, b.point) > 1e-9 {
                // An arc: included angle 4·atan(bulge) (positive counter-clockwise), the centre on the perpendicular
                // bisector, to the left of the chord for a counter-clockwise arc and to the right for a clockwise one.
                let theta = 4 * atan(a.bulge)
                let chord = distance(a.point, b.point)
                let radius = chord / (2 * abs(sin(theta / 2)))
                let dx = Double(b.point.x - a.point.x) / chord, dy = Double(b.point.y - a.point.y) / chord
                let offset = radius * cos(theta / 2) * (theta > 0 ? 1 : -1)
                let mid = CGPoint(x: (a.point.x + b.point.x) / 2, y: (a.point.y + b.point.y) / 2)
                let centre = CGPoint(x: mid.x + CGFloat(-dy * offset), y: mid.y + CGFloat(dx * offset))
                let from = atan2(Double(a.point.y - centre.y), Double(a.point.x - centre.x))
                out += arcPoints(centre: centre, radius: radius, from: from, sweep: theta).dropFirst()
            } else {
                out.append(b.point)
            }
        }
        // A closed polyline ends where it began: its last point repeats the first, which the loop builder drops.
        return out
    }

    /// A spline as a polyline (mm): evaluated as a NURBS from its knots, controls and weights when it has
    /// controls (weights 1 when absent), else the polyline through its fit points.
    static func splinePoints(_ spline: DXFSpline, scale: Double) -> (points: [CGPoint], closed: Bool) {
        let controls = spline.controls.map { CGPoint(x: $0.x * scale, y: $0.y * scale) }
        guard controls.count > spline.degree, spline.degree >= 1 else {
            let fit = spline.fit.map { CGPoint(x: $0.x * scale, y: $0.y * scale) }
            return (fit, spline.closed)
        }
        let p = spline.degree
        let n = controls.count - 1
        var knots = spline.knots
        if knots.count != n + p + 2 {
            // No usable knot vector: clamped and uniform.
            knots = (0...(n + p + 1)).map { i in
                i <= p ? 0 : (i >= n + 1 ? Double(n - p + 1) : Double(i - p))
            }
        }
        let weights = spline.weights.count == controls.count ? spline.weights : Array(repeating: 1.0, count: controls.count)
        let lower = knots[p], upper = knots[n + 1]
        guard upper > lower else { return (controls, false) }

        func evaluate(_ t: Double) -> CGPoint {
            // The knot span, with the end of the domain in the last non-empty span.
            var k = p
            while k < n, knots[k + 1] <= t { k += 1 }
            // de Boor on the homogeneous points (w·x, w·y, w).
            var d: [(x: Double, y: Double, w: Double)] = (0...p).map { j in
                let i = k - p + j
                return (Double(controls[i].x) * weights[i], Double(controls[i].y) * weights[i], weights[i])
            }
            if p >= 1 {
                for r in 1...p {
                    for j in stride(from: p, through: r, by: -1) {
                        let i = k - p + j
                        let denominator = knots[i + p - r + 1] - knots[i]
                        let alpha = denominator == 0 ? 0 : (t - knots[i]) / denominator
                        d[j] = ((1 - alpha) * d[j - 1].x + alpha * d[j].x, (1 - alpha) * d[j - 1].y + alpha * d[j].y,
                                (1 - alpha) * d[j - 1].w + alpha * d[j].w)
                    }
                }
            }
            let last = d[p]
            return CGPoint(x: last.x / last.w, y: last.y / last.w)
        }

        // Each knot span is sampled, then split where its midpoint is off the chord by more than the flatness.
        var spans: [Double] = Array(Set(knots[p...(n + 1)])).sorted()
        spans = spans.filter { $0 >= lower && $0 <= upper }
        var points: [CGPoint] = [evaluate(lower)]
        func refine(_ t0: Double, _ p0: CGPoint, _ t1: Double, _ p1: CGPoint, depth: Int) {
            let tm = (t0 + t1) / 2
            let pm = evaluate(tm)
            let chordLength = distance(p0, p1)
            let deviation = chordLength < 1e-12 ? distance(p0, pm)
                : abs(Double((p1.x - p0.x) * (p0.y - pm.y) - (p0.x - pm.x) * (p1.y - p0.y))) / chordLength
            if depth < 10, deviation > BoardCutout.flatness * 0.5 || depth < 3 {
                refine(t0, p0, tm, pm, depth: depth + 1)
                points.append(pm)
                refine(tm, pm, t1, p1, depth: depth + 1)
            }
        }
        var previous = lower
        var previousPoint = points[0]
        for t in spans.dropFirst() {
            let end = evaluate(t)
            refine(previous, previousPoint, t, end, depth: 0)
            points.append(end)
            previous = t
            previousPoint = end
        }
        return (points, spline.closed)
    }

    // MARK: Helpers

    static func distance(_ a: CGPoint, _ b: CGPoint) -> Double { Double(hypot(a.x - b.x, a.y - b.y)) }

    /// `points` without a point that repeats the one before it, or the first one at the end.
    static func removeRepeats(_ points: [CGPoint]) -> [CGPoint] {
        var out: [CGPoint] = []
        for point in points where out.last.map({ distance($0, point) > 1e-6 }) ?? true { out.append(point) }
        while out.count > 1, let first = out.first, let last = out.last, distance(first, last) <= 1e-6 { out.removeLast() }
        return out
    }
}

/// Which layers a DXF import starts with checked.
nonisolated enum DXFLayerPick {
    /// The layers whose lower-cased name has `edge`, `outline`, `board`, `profile` or `cut` in it when there are
    /// any, else every layer.
    static func defaultSelection(layers: [String]) -> Set<String> {
        let words = ["edge", "outline", "board", "profile", "cut"]
        let named = layers.filter { layer in words.contains { layer.lowercased().contains($0) } }
        return Set(named.isEmpty ? layers : named)
    }
}
