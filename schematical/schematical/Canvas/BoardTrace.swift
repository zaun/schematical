import CoreGraphics
import Foundation

/// A copper trace on the PCB: a polyline on one copper layer. The Trace
/// tool draws its segments at the chosen angle (`TraceRoute.angles`), but
/// any angle is legal. Traces that touch on the same layer share a vertex
/// at each contact point, so connection is just shared vertices (see
/// `SchematicDocument.join`).
nonisolated struct BoardTrace: Identifiable, Hashable, Sendable {
    let id: UUID
    var layer: CopperLayer
    /// mil, within `widthRange`.
    var width: Int
    /// mm; at least 2, consecutive points distinct (`TraceRoute.isLegal`).
    /// Not snapped to the grid: a bend, a crossing of two diagonals, or a
    /// pad centre, can be off it.
    var points: [CGPoint]
    /// Locked: dragging leaves it where it is.
    var locked = false

    static let widthRange = 5...100
    static let defaultWidth = 10

    /// The width in mm (1 mil = 0.0254 mm).
    var widthMM: Double { Double(width) * 0.0254 }

    /// Consecutive point pairs.
    var segments: [(CGPoint, CGPoint)] { Array(zip(points, points.dropFirst())) }
}

/// Angled routing for traces.
nonisolated enum TraceRoute {
    /// mm. Points closer than this are the same point.
    static let epsilon: CGFloat = 1e-6

    /// The Trace tool's angles, in the order A steps through them.
    static let angles = [20, 40, 45, 60, 80, 90]
    static let defaultAngle = 45

    /// The angle after `angle` in `angles`, round from 90 back to 20. An
    /// angle not in the list goes to the default.
    static func nextAngle(after angle: Int) -> Int {
        guard let index = angles.firstIndex(of: angle) else { return defaultAngle }
        return angles[(index + 1) % angles.count]
    }

    /// A route from `a` to `b` with at most one bend: a straight run along
    /// the longer axis and a leg at `angle` degrees to that run, the run
    /// first unless `legFirst`. When a leg at `angle` can't cover the other
    /// axis (a small angle to a target well off the axis), the leg uses
    /// 90° − `angle` instead. When `a` and `b` are on one axis, or already
    /// on the leg's line, it is the straight line. 90° gives a plain L.
    static func route(from a: CGPoint, to b: CGPoint, angle: Int = defaultAngle, legFirst: Bool) -> [CGPoint] {
        let dx = b.x - a.x, dy = b.y - a.y
        if abs(dx) < epsilon || abs(dy) < epsilon { return [a, b] }
        let horizontal = abs(dx) >= abs(dy)
        let long = horizontal ? abs(dx) : abs(dy), short = horizontal ? abs(dy) : abs(dx)
        let slope = short / long
        var degrees = Double(min(max(angle, 1), 90))
        if degrees < 90, tangent(degrees) < slope - 1e-12 { degrees = 90 - degrees }
        // How far along the long axis the leg goes.
        let legLong = degrees >= 90 ? 0 : short / tangent(degrees)
        if abs(legLong - long) < epsilon { return [a, b] }
        let sx: CGFloat = dx < 0 ? -1 : 1, sy: CGFloat = dy < 0 ? -1 : 1
        let along = legFirst ? legLong : long - legLong
        let bend: CGPoint
        if horizontal {
            bend = CGPoint(x: a.x + sx * along, y: legFirst ? b.y : a.y)
        } else {
            bend = CGPoint(x: legFirst ? b.x : a.x, y: a.y + sy * along)
        }
        return [a, bend, b]
    }

    /// tan of `degrees`, exactly 1 at 45° so 45° bends stay on the grid.
    private static func tangent(_ degrees: Double) -> Double {
        degrees == 45 ? 1 : tan(degrees * .pi / 180)
    }

    /// At least 2 points and no zero-length segment.
    static func isLegal(_ points: [CGPoint]) -> Bool {
        problem(points) == nil
    }

    /// Why `points` isn't a legal trace, as a file error detail after
    /// "trace <id> ", or nil.
    static func problem(_ points: [CGPoint]) -> String? {
        guard points.count >= 2 else { return "has fewer than 2 points" }
        for (index, (a, b)) in zip(points, points.dropFirst()).enumerated() where same(a, b) {
            return "segment \(index + 1) has no length"
        }
        return nil
    }

    /// The points with consecutive duplicates removed.
    static func clean(_ points: [CGPoint]) -> [CGPoint] {
        var result: [CGPoint] = []
        for point in points where result.last.map({ !same($0, point) }) ?? true {
            result.append(point)
        }
        return result
    }

    /// `clean`, then every vertex dropped that sits on the line through its
    /// neighbours, so a run that carries on past a lock is one segment.
    static func simplified(_ points: [CGPoint]) -> [CGPoint] {
        var result: [CGPoint] = []
        for point in clean(points) {
            if result.count >= 2 {
                let a = result[result.count - 2], b = result[result.count - 1]
                let cross = (b.x - a.x) * (point.y - b.y) - (b.y - a.y) * (point.x - b.x)
                let dot = (b.x - a.x) * (point.x - b.x) + (b.y - a.y) * (point.y - b.y)
                let scale = hypot(b.x - a.x, b.y - a.y) * hypot(point.x - b.x, point.y - b.y)
                if abs(cross) <= 1e-9 * scale, dot > 0 { result.removeLast() }
            }
            result.append(point)
        }
        return result
    }

    static func same(_ a: CGPoint, _ b: CGPoint) -> Bool {
        abs(a.x - b.x) < epsilon && abs(a.y - b.y) < epsilon
    }
}

/// A trace being drawn: locked legs, then a live leg to the cursor, as the
/// schematic's `RouteDraft`. Space locks the live leg (`lock`), Esc drops
/// the last lock (`unlock`), and `/` puts the live leg's angled part first
/// (`flip`). Each leg follows `TraceRoute.route` at the angle given.
nonisolated struct TraceDraft: Equatable {
    /// Where the trace starts.
    let start: CGPoint
    /// Each locked leg: the points after its start, up to its end.
    private(set) var legs: [[CGPoint]] = []
    /// Whether `/` flipped the live leg.
    private(set) var flipped = false

    init(start: CGPoint) {
        self.start = start
    }

    var hasLocks: Bool { !legs.isEmpty }

    /// Where the live leg starts: the end of the last lock, or the start.
    var legStart: CGPoint { legs.last?.last ?? start }

    /// The live leg to `end`: its points after `legStart`. Empty when `end`
    /// is `legStart`.
    func liveLeg(to end: CGPoint, angle: Int) -> [CGPoint] {
        guard !TraceRoute.same(legStart, end) else { return [] }
        return Array(TraceRoute.route(from: legStart, to: end, angle: angle, legFirst: flipped).dropFirst())
    }

    /// The whole trace to `end`, start included, simplified.
    func path(to end: CGPoint, angle: Int) -> [CGPoint] {
        TraceRoute.simplified([start] + legs.flatMap { $0 } + liveLeg(to: end, angle: angle))
    }

    mutating func flip() {
        flipped.toggle()
    }

    /// Locks the live leg to `end` as `path` shows it. Returns false, and
    /// changes nothing, when the leg is empty.
    @discardableResult
    mutating func lock(to end: CGPoint, angle: Int) -> Bool {
        let leg = liveLeg(to: end, angle: angle)
        guard !leg.isEmpty else { return false }
        legs.append(leg)
        flipped = false
        return true
    }

    /// Drops the last locked leg. Returns false when nothing was locked.
    @discardableResult
    mutating func unlock() -> Bool {
        guard !legs.isEmpty else { return false }
        legs.removeLast()
        flipped = false
        return true
    }
}

/// Traces copied with ⌘C on the PCB, shared by every window, for ⌘V. A
/// paste lands centred on the pointer (`offset(centring:on:)`); with the
/// pointer off the board, each paste lands one more `step` right and down
/// from the copied traces instead (`nextOffset`).
@MainActor
enum TraceClipboard {
    private(set) static var traces: [BoardTrace] = []
    /// Pastes since the last copy.
    private(set) static var pastes = 0
    /// mm: four grid steps.
    static let step: CGFloat = 2.54

    static func copy(_ copied: [BoardTrace]) {
        traces = copied
        pastes = 0
    }

    /// The offset that puts the middle of `traces`' bounding box on
    /// `point` (mm), rounded to whole steps of `grid` (the trace grid) so
    /// points on that grid stay on it.
    static func offset(centring traces: [BoardTrace], on point: CGPoint,
                       grid: CGFloat = BoardGrid.spacing / 2) -> CGSize {
        let points = traces.flatMap(\.points)
        guard let first = points.first else { return .zero }
        var box = CGRect(origin: first, size: .zero)
        for p in points { box = box.union(CGRect(origin: p, size: .zero)) }
        return CGSize(width: BoardGrid.snap(point.x - box.midX, spacing: grid),
                      height: BoardGrid.snap(point.y - box.midY, spacing: grid))
    }

    /// The offset for the next paste with the pointer off the board,
    /// counting it.
    static func nextOffset() -> CGSize {
        pastes += 1
        let d = step * CGFloat(pastes)
        return CGSize(width: d, height: d)
    }
}

/// A via: a plated hole through every copper layer, joining the traces
/// that touch its metal on any layer. Sizes in mm.
nonisolated struct BoardVia: Identifiable, Hashable, Sendable {
    let id: UUID
    /// mm. Placed and moved on the trace grid.
    var centre: CGPoint
    /// The hole's diameter, within `drillRange`.
    var drill: Double = BoardVia.defaultDrill
    /// The metal's outer diameter, within `diameterRange` and at least
    /// `minimumRing` × 2 more than the hole.
    var diameter: Double = BoardVia.defaultDiameter
    /// Locked: dragging and the sidebar's X and Y leave it where it is.
    var locked = false

    static let defaultDrill = 0.4
    static let defaultDiameter = 0.8
    static let drillRange: ClosedRange<Double> = 0.1...5
    static let diameterRange: ClosedRange<Double> = 0.2...10
    /// The thinnest ring of metal round the hole.
    static let minimumRing = 0.05

    /// The ring of metal round the hole, each side.
    var ring: Double { (diameter - drill) / 2 }

    /// The metal's square bounds.
    var rect: CGRect {
        CGRect(x: centre.x - diameter / 2, y: centre.y - diameter / 2, width: diameter, height: diameter)
    }

    /// Why these sizes can't go together, as a file error detail after
    /// "via <id> ", or nil.
    static func problem(drill: Double, diameter: Double) -> String? {
        func n(_ value: Double) -> String { String(format: "%g", value) }
        if !drillRange.contains(drill) { return "drill \(n(drill)) must be 0.1 to 5 mm" }
        if !diameterRange.contains(diameter) { return "diameter \(n(diameter)) must be 0.2 to 10 mm" }
        if diameter < drill + 2 * minimumRing - 1e-9 {
            return "diameter \(n(diameter)) must be at least 0.1 mm more than its drill \(n(drill))"
        }
        return nil
    }
}
