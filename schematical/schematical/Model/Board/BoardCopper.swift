import CoreGraphics
import Foundation

/// The board's traces and vias and the rules for editing them (plan-68-E):
/// adding and joining traces, widths and layers, vias and their sizes, and
/// the geometry traces are joined by. Routing's primitives and segment
/// drags are in `BoardCopper+Routing`. The board's settings come in as a
/// parameter; nothing here reads a document.
struct BoardCopper {
    var traces: [BoardTrace]
    var vias: [BoardVia]

    private func traceIndex(_ id: UUID) -> Int? {
        traces.firstIndex { $0.id == id }
    }

    private func viaIndex(_ id: UUID) -> Int? {
        vias.firstIndex { $0.id == id }
    }

    // MARK: Traces

    /// Adds a trace along `points` (simplified) on `layer`, `width` mil wide
    /// (clamped to `BoardTrace.widthRange`), joined to every trace it
    /// touches. Nil when it has no length or `settings` refuses the layer.
    mutating func addTrace(points: [CGPoint], layer: CopperLayer, width: Int, settings: BoardSettings) -> UUID? {
        let points = TraceRoute.simplified(points)
        guard TraceRoute.isLegal(points), settings.traceLayerRefusal(layer) == nil else { return nil }
        let width = min(max(width, BoardTrace.widthRange.lowerBound), BoardTrace.widthRange.upperBound)
        let trace = BoardTrace(id: UUID(), layer: layer, width: width, points: points)
        traces.append(trace)
        join(trace.id)
        return trace.id
    }

    /// Makes every point where trace `id` touches or crosses another trace
    /// on its layer a vertex of both. Inserting a vertex on a straight
    /// segment keeps its angle, so both stay legal.
    mutating func join(_ id: UUID) {
        guard let index = traceIndex(id) else { return }
        let layer = traces[index].layer
        for other in traces.indices where other != index && traces[other].layer == layer {
            var contacts: [CGPoint] = []
            for (a, b) in traces[index].segments {
                for (c, d) in traces[other].segments {
                    contacts += Self.contacts(a, b, c, d)
                }
            }
            for point in contacts {
                traces[index].points = Self.inserting(point, into: traces[index].points)
                traces[other].points = Self.inserting(point, into: traces[other].points)
            }
        }
    }

    /// Whether `points` may replace an unlocked trace's vertices (at least
    /// two, none of zero length).
    func canSetPoints(_ id: UUID, _ points: [CGPoint]) -> Bool {
        guard let index = traceIndex(id) else { return false }
        return !traces[index].locked && TraceRoute.isLegal(points)
    }

    mutating func setPoints(_ id: UUID, _ points: [CGPoint]) {
        guard let index = traceIndex(id), traces[index].points != points else { return }
        traces[index].points = points
    }

    /// The traces on the same layer reachable from `id` through shared
    /// vertices, `id` first.
    func connectedRun(of id: UUID) -> [UUID] {
        guard let start = traces.first(where: { $0.id == id }) else { return [] }
        let same = traces.filter { $0.layer == start.layer }
        var run = [id], seen: Set<UUID> = [id], queue = [start]
        while !queue.isEmpty {
            let trace = queue.removeFirst()
            for other in same where !seen.contains(other.id)
                && trace.points.contains(where: { p in other.points.contains { TraceRoute.same($0, p) } }) {
                seen.insert(other.id)
                run.append(other.id)
                queue.append(other)
            }
        }
        return run
    }

    /// A trace's width in mil, clamped to `BoardTrace.widthRange`.
    mutating func setWidth(_ id: UUID, _ mil: Int) {
        guard let index = traceIndex(id) else { return }
        traces[index].width = min(max(mil, BoardTrace.widthRange.lowerBound), BoardTrace.widthRange.upperBound)
    }

    /// Moves one trace to `layer` and joins it there, unless `settings`
    /// refuses the layer.
    mutating func setLayer(_ id: UUID, _ layer: CopperLayer, settings: BoardSettings) {
        guard settings.traceLayerRefusal(layer) == nil, let index = traceIndex(id), traces[index].layer != layer else {
            return
        }
        traces[index].layer = layer
        join(id)
    }

    /// Moves every vertex of the unlocked traces `ids` by `offset` (mm),
    /// then joins each where it now lands.
    mutating func moveTraces(_ ids: Set<UUID>, by offset: CGSize) {
        for index in traces.indices where ids.contains(traces[index].id) && !traces[index].locked {
            traces[index].points = traces[index].points.map { CGPoint(x: $0.x + offset.width, y: $0.y + offset.height) }
        }
        for id in ids { join(id) }
    }

    /// Copies of `copied` (on layers `settings` allows) and `copiedVias`
    /// (none on a 1-layer board), moved by `offset` (mm), with new ids; the
    /// traces joined, the vias on the trace grid. Returns the copies.
    mutating func paste(traces copied: [BoardTrace], vias copiedVias: [BoardVia], offset: CGSize,
                        settings: BoardSettings) -> (traces: [UUID], vias: [UUID]) {
        let added = copied.filter { settings.traceLayerRefusal($0.layer) == nil }.map { trace in
            BoardTrace(id: UUID(), layer: trace.layer, width: trace.width,
                       points: trace.points.map { CGPoint(x: $0.x + offset.width, y: $0.y + offset.height) })
        }
        traces += added
        for trace in added { join(trace.id) }
        let addedVias = (settings.layers > 1 ? copiedVias : []).map { via in
            let moved = CGPoint(x: via.centre.x + offset.width, y: via.centre.y + offset.height)
            return BoardVia(id: UUID(), centre: BoardGrid.snap(moved, spacing: settings.traceGrid),
                            drill: via.drill, diameter: via.diameter,
                            tentedTop: via.tentedTop, tentedBottom: via.tentedBottom)
        }
        vias += addedVias
        return (added.map(\.id), addedVias.map(\.id))
    }

    /// The trace under a board point (mm): the last one drawn there, on the
    /// first of `layers` it is on, within half its width plus `tolerance`.
    func trace(at point: CGPoint, tolerance: CGFloat, layers: [CopperLayer]) -> BoardTrace? {
        for layer in layers {
            if let hit = traces.last(where: { trace in
                trace.layer == layer && trace.segments.contains {
                    SchematicDocument.distance(from: point, toSegment: $0.0, $0.1) <= trace.widthMM / 2 + tolerance
                }
            }) { return hit }
        }
        return nil
    }

    /// The vertex of a trace (on `layer`, or any when nil) nearest `point`
    /// within `tolerance`, else the nearest point of a trace segment within
    /// it, else nil.
    func nearestTracePoint(to point: CGPoint, layer: CopperLayer?, tolerance: CGFloat) -> CGPoint? {
        let same = traces.filter { layer == nil || $0.layer == layer }
        var best: (point: CGPoint, distance: CGFloat)?
        func consider(_ candidate: CGPoint) {
            let d = SchematicDocument.distance(from: point, to: candidate)
            if d <= tolerance, d < best?.distance ?? .infinity { best = (candidate, d) }
        }
        same.lazy.flatMap(\.points).forEach(consider)
        if let best { return best.point }
        for trace in same {
            for (a, b) in trace.segments { consider(Self.nearest(on: a, b, to: point)) }
        }
        return best?.point
    }

    /// The traces whose copper touches a circle of `radius` round `centre`, on any layer.
    func traces(touching centre: CGPoint, radius: CGFloat) -> [BoardTrace] {
        traces.filter { trace in
            trace.segments.contains {
                SchematicDocument.distance(from: centre, toSegment: $0.0, $0.1) <= radius + trace.widthMM / 2
            }
        }
    }

    /// Gives `items`' traces and vias `rules`' width and via.
    mutating func apply(_ rules: NetRules, traces traceIDs: Set<UUID>, vias viaIDs: Set<UUID>) {
        for index in traces.indices where traceIDs.contains(traces[index].id) && traces[index].width != rules.width {
            traces[index].width = rules.width
        }
        for index in vias.indices where viaIDs.contains(vias[index].id)
            && (vias[index].drill != rules.viaDrill || vias[index].diameter != rules.viaDiameter) {
            vias[index].drill = rules.viaDrill
            vias[index].diameter = rules.viaDiameter
        }
    }

    mutating func setLocked(trace id: UUID, _ locked: Bool) {
        if let index = traceIndex(id) { traces[index].locked = locked }
    }

    mutating func setLocked(via id: UUID, _ locked: Bool) {
        if let index = viaIndex(id) { vias[index].locked = locked }
    }

    // MARK: Vias

    /// The via whose metal is under `point` (mm): the last one drawn there.
    func via(at point: CGPoint) -> BoardVia? {
        vias.last { SchematicDocument.distance(from: point, to: $0.centre) <= $0.diameter / 2 }
    }

    /// Adds a via at `point` with `rules`' sizes.
    mutating func addVia(at point: CGPoint, rules: NetRules) -> UUID {
        let via = BoardVia(id: UUID(), centre: point, drill: rules.viaDrill, diameter: rules.viaDiameter)
        vias.append(via)
        return via.id
    }

    /// Moves an unlocked via's centre to `point` (already where it goes).
    mutating func setViaCentre(_ id: UUID, _ point: CGPoint) {
        guard let index = viaIndex(id), !vias[index].locked, vias[index].centre != point else { return }
        vias[index].centre = point
    }

    /// A via's hole, clamped to `BoardVia.drillRange` and to leave the
    /// thinnest ring inside its metal.
    mutating func setViaDrill(_ id: UUID, _ mm: Double) {
        guard let index = viaIndex(id) else { return }
        let most = min(BoardVia.drillRange.upperBound, vias[index].diameter - 2 * BoardVia.minimumRing)
        let value = min(max(mm, BoardVia.drillRange.lowerBound), most)
        if vias[index].drill != value { vias[index].drill = value }
    }

    /// A via's metal diameter, clamped to `BoardVia.diameterRange` and to
    /// keep the thinnest ring round its hole.
    mutating func setViaDiameter(_ id: UUID, _ mm: Double) {
        guard let index = viaIndex(id) else { return }
        let least = max(BoardVia.diameterRange.lowerBound, vias[index].drill + 2 * BoardVia.minimumRing)
        let value = max(min(mm, BoardVia.diameterRange.upperBound), least)
        if vias[index].diameter != value { vias[index].diameter = value }
    }

    /// Whether the solder mask covers a via on `side`.
    mutating func setViaTenting(_ id: UUID, _ side: BoardSide, _ tented: Bool) {
        guard let index = viaIndex(id) else { return }
        switch side {
        case .top: if vias[index].tentedTop != tented { vias[index].tentedTop = tented }
        case .bottom: if vias[index].tentedBottom != tented { vias[index].tentedBottom = tented }
        }
    }

    // MARK: Geometry

    /// The point of segment a–b nearest `point`.
    static func nearest(on a: CGPoint, _ b: CGPoint, to point: CGPoint) -> CGPoint {
        let dx = b.x - a.x, dy = b.y - a.y
        let lengthSquared = dx * dx + dy * dy
        guard lengthSquared > 0 else { return a }
        let t = min(max(((point.x - a.x) * dx + (point.y - a.y) * dy) / lengthSquared, 0), 1)
        return CGPoint(x: a.x + t * dx, y: a.y + t * dy)
    }

    /// Where segments a–b and c–d touch: their crossing point, or both ends
    /// of a collinear overlap. Empty when they don't touch.
    static func contacts(_ a: CGPoint, _ b: CGPoint, _ c: CGPoint, _ d: CGPoint) -> [CGPoint] {
        let eps = TraceRoute.epsilon
        let r = CGPoint(x: b.x - a.x, y: b.y - a.y), s = CGPoint(x: d.x - c.x, y: d.y - c.y)
        let denom = r.x * s.y - r.y * s.x
        let lengths = hypot(r.x, r.y) * hypot(s.x, s.y)
        if abs(denom) > 1e-9 * lengths {
            let q = CGPoint(x: c.x - a.x, y: c.y - a.y)
            let t = (q.x * s.y - q.y * s.x) / denom
            let point = CGPoint(x: a.x + t * r.x, y: a.y + t * r.y)
            let onBoth = SchematicDocument.distance(from: point, toSegment: a, b) < eps
                && SchematicDocument.distance(from: point, toSegment: c, d) < eps
            return onBoth ? [[a, b, c, d].first { TraceRoute.same($0, point) } ?? point] : []
        }
        // Parallel: they touch only when collinear, at the ends of each that
        // lie on the other.
        var points: [CGPoint] = []
        for (end, segment) in [(a, (c, d)), (b, (c, d)), (c, (a, b)), (d, (a, b))]
        where SchematicDocument.distance(from: end, toSegment: segment.0, segment.1) < eps
            && !points.contains(where: { TraceRoute.same($0, end) }) {
            points.append(end)
        }
        return points
    }

    /// `points` with `point` added as a vertex on the segment it lies on.
    /// Unchanged when it is already a vertex or on no segment.
    static func inserting(_ point: CGPoint, into points: [CGPoint]) -> [CGPoint] {
        if points.contains(where: { TraceRoute.same($0, point) }) { return points }
        guard let index = points.indices.dropLast().first(where: {
            SchematicDocument.distance(from: point, toSegment: points[$0], points[$0 + 1]) < TraceRoute.epsilon
        }) else { return points }
        var result = points
        result.insert(point, at: index + 1)
        return result
    }

    /// Whether segment a–b meets `rect` (inside it, or crossing or touching
    /// its edge).
    nonisolated static func segment(_ a: CGPoint, _ b: CGPoint, touches rect: CGRect) -> Bool {
        // Liang–Barsky: clip the segment's parameter range to the rect.
        var low: CGFloat = 0, high: CGFloat = 1
        let dx = b.x - a.x, dy = b.y - a.y
        for (p, q) in [(-dx, a.x - rect.minX), (dx, rect.maxX - a.x), (-dy, a.y - rect.minY), (dy, rect.maxY - a.y)] {
            if p == 0 {
                if q < 0 { return false }
            } else {
                let t = q / p
                if p < 0 { low = max(low, t) } else { high = min(high, t) }
                if low > high { return false }
            }
        }
        return true
    }
}
