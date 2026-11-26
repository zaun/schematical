import CoreGraphics
import Foundation

/// Routing's primitives, segment drags and group drags of copper (plan-68-E).
extension BoardCopper {
    // MARK: Routing primitives

    /// Removes traces and vias by id, locked or not.
    mutating func remove(traces ids: Set<UUID>, vias viaIDs: Set<UUID>) {
        if !ids.isEmpty { traces.removeAll { ids.contains($0.id) } }
        if !viaIDs.isEmpty { vias.removeAll { viaIDs.contains($0.id) } }
    }

    /// Appends a trace as it is (no join). False when `settings` refuses its
    /// layer or it has no length.
    mutating func append(_ trace: BoardTrace, settings: BoardSettings) -> Bool {
        guard settings.traceLayerRefusal(trace.layer) == nil, TraceRoute.isLegal(trace.points) else { return false }
        traces.append(trace)
        return true
    }

    /// Appends a via. False on a board of `layers` 1.
    mutating func append(_ via: BoardVia, layers: Int) -> Bool {
        guard layers > 1 else { return false }
        vias.append(via)
        return true
    }

    // MARK: Segment drags

    /// What a plain Select drag on trace `id` at `point` takes hold of: the
    /// segment nearest the press, the trace's points and which of them stay
    /// (shared with another trace on the layer, under a via, or on one of
    /// `pads`). Nil for a locked trace or none.
    func segmentBase(_ id: UUID, at point: CGPoint, pads: [CGRect]) -> TraceSegmentBase? {
        guard let trace = traces.first(where: { $0.id == id }), !trace.locked,
              let segment = trace.segments.indices.min(by: {
                  SchematicDocument.distance(from: point, toSegment: trace.segments[$0].0, trace.segments[$0].1)
                      < SchematicDocument.distance(from: point, toSegment: trace.segments[$1].0, trace.segments[$1].1)
              }) else { return nil }
        let others = traces.filter { $0.id != id && $0.layer == trace.layer }
        var anchored: Set<Int> = []
        for (index, vertex) in trace.points.enumerated() {
            let shared = others.contains { $0.points.contains { TraceRoute.same($0, vertex) } }
            let viaHere = vias.contains { SchematicDocument.distance(from: $0.centre, to: vertex) <= $0.diameter / 2 }
            if shared || viaHere || pads.contains(where: { $0.contains(vertex) }) { anchored.insert(index) }
        }
        return TraceSegmentBase(id: id, segment: segment, points: trace.points, anchored: anchored)
    }

    /// One frame of a segment drag: the segment slides sideways by the part
    /// of `delta` (mm) at right angles to it, snapped to `grid`, and the rest
    /// of the trace follows (`TraceRoute.slide`).
    mutating func dragSegment(_ base: TraceSegmentBase, by delta: CGSize, grid spacing: Double) {
        guard let index = traces.firstIndex(where: { $0.id == base.id }) else { return }
        let a = base.points[base.segment], b = base.points[base.segment + 1]
        let length = SchematicDocument.distance(from: a, to: b)
        guard length > TraceRoute.epsilon else { return }
        let normal = CGPoint(x: -(b.y - a.y) / length, y: (b.x - a.x) / length)
        var distance = delta.width * normal.x + delta.height * normal.y
        if abs(normal.x) < 1e-9 || abs(normal.y) < 1e-9 {
            // Along an axis: the segment lands on a trace grid line.
            let axis: CGFloat = abs(normal.x) > abs(normal.y) ? a.x : a.y
            let direction: CGFloat = abs(normal.x) > abs(normal.y) ? normal.x : normal.y
            distance = (BoardGrid.snap(axis + distance * direction, spacing: spacing) - axis) * direction
        } else {
            distance = BoardGrid.snap(distance, spacing: spacing)
        }
        let target = TraceRoute.slide(base.points, segment: base.segment, by: distance, anchored: base.anchored)
        if traces[index].points != target { traces[index].points = target }
    }

    // MARK: Group drags

    /// One frame of a group drag: each trace in `points` and via in
    /// `centres` moves by `offset` from where it was at the press.
    mutating func moveGroup(traces points: [UUID: [CGPoint]], vias centres: [UUID: CGPoint], by offset: CGSize) {
        func moved(_ point: CGPoint) -> CGPoint { CGPoint(x: point.x + offset.width, y: point.y + offset.height) }
        for index in traces.indices {
            guard let base = points[traces[index].id] else { continue }
            let target = base.map(moved)
            if traces[index].points != target { traces[index].points = target }
        }
        for index in vias.indices {
            guard let centre = centres[vias[index].id] else { continue }
            let target = moved(centre)
            if vias[index].centre != target { vias[index].centre = target }
        }
    }
}
