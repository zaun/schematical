import CoreGraphics
import Foundation

// MARK: - Orthogonal routing

/// The direction of a straight run: along a row or along a column.
nonisolated enum RouteAxis: Hashable {
    case horizontal
    case vertical

    var other: RouteAxis { self == .horizontal ? .vertical : .horizontal }
}

/// Right-angle routing helpers. Paths are arrays of grid points where
/// consecutive points share an x or a y.
nonisolated enum OrthogonalRoute {
    /// Pin to pin with both pins facing sideways: out horizontally, a
    /// vertical run at the midpoint, then horizontally into the target.
    static func route(from a: CGPoint, to b: CGPoint) -> [CGPoint] {
        normalize([a] + leg(from: a, to: b, first: .horizontal, last: .horizontal, preferred: .horizontal))
    }

    /// One drawn leg from `p` to `q`: the points after `p`, up to and
    /// including `q`. A straight line when the two are aligned, else an L.
    /// `first` and `last` are the axes a pin forces on the leg's first and
    /// last segment. When both are forced along the same axis no L fits, so
    /// the leg is a Z with its middle run halfway. `preferred` picks the
    /// L's first segment when nothing forces it.
    static func leg(from p: CGPoint, to q: CGPoint,
                    first: RouteAxis?, last: RouteAxis?,
                    preferred: RouteAxis) -> [CGPoint] {
        if p == q { return [] }
        if p.x == q.x || p.y == q.y { return [q] }
        let start: RouteAxis
        switch (first, last) {
        case let (first?, last?) where first == last:
            if first == .horizontal {
                let midX = SchematicGrid.snap((p.x + q.x) / 2)
                return mergeCoincident([p, CGPoint(x: midX, y: p.y), CGPoint(x: midX, y: q.y), q]).filter { $0 != p }
            }
            let midY = SchematicGrid.snap((p.y + q.y) / 2)
            return mergeCoincident([p, CGPoint(x: p.x, y: midY), CGPoint(x: q.x, y: midY), q]).filter { $0 != p }
        case let (first?, _):
            start = first
        case let (nil, last?):
            start = last.other
        case (nil, nil):
            start = preferred
        }
        return start == .horizontal ? [CGPoint(x: q.x, y: p.y), q] : [CGPoint(x: p.x, y: q.y), q]
    }

    /// A drawn route cleaned up: coincident points merged, double-backs
    /// dropped, and any bend in the middle of a straight run removed. The
    /// first and last points stay.
    static func simplified(_ path: [CGPoint]) -> [CGPoint] {
        var result = removeDoubleBacks(mergeCoincident(path))
        var index = 1
        while index + 1 < result.count {
            let a = result[index - 1], b = result[index], c = result[index + 1]
            let straight = (a.y == b.y && b.y == c.y) || (a.x == b.x && b.x == c.x)
            if straight {
                result.remove(at: index)
            } else {
                index += 1
            }
        }
        return result
    }

    /// Merges points that overlap, adds a corner wherever two points are
    /// diagonal, and merges again in case the corners landed on existing points.
    /// Bends that sit on a straight run are kept, so a node the user added by
    /// double-clicking survives.
    static func normalize(_ input: [CGPoint]) -> [CGPoint] {
        let merged = mergeCoincident(input)

        var cornered: [CGPoint] = []
        for point in merged {
            if let previous = cornered.last, previous.x != point.x, previous.y != point.y {
                cornered.append(CGPoint(x: point.x, y: previous.y))
            }
            cornered.append(point)
        }
        return mergeCoincident(cornered)
    }

    /// Collapses points that land on the same position. When a point returns to
    /// an earlier position, the loop between the two is removed.
    static func mergeCoincident(_ points: [CGPoint]) -> [CGPoint] {
        var result: [CGPoint] = []
        for point in points {
            if let index = result.lastIndex(of: point) {
                if index + 1 < result.count {
                    result.removeSubrange((index + 1)..<result.count)
                }
            } else {
                result.append(point)
            }
        }
        return result
    }

    /// Re-fits a wire's stored vertices after one of its pins moved. Segments
    /// keep their angles: the segments leaving and entering a pin stay horizontal.
    ///
    /// - With vertices already present, none are added. The vertex next to each
    ///   pin is moved onto the pin's row.
    /// - With no vertices and a straight wire, one vertex goes at the midpoint.
    /// - With no vertices and a diagonal wire, one corner is added.
    static func settled(start: CGPoint, interior: [CGPoint], end: CGPoint) -> [CGPoint] {
        var points = [start] + interior + [end]

        if interior.isEmpty {
            if start.x == end.x || start.y == end.y {
                let middle = SchematicGrid.snap(CGPoint(x: (start.x + end.x) / 2, y: (start.y + end.y) / 2))
                points.insert(middle, at: 1)
            } else {
                points.insert(CGPoint(x: end.x, y: start.y), at: 1)
            }
        } else if interior.count == 1 {
            if points[1].x != start.x {
                points[1].y = start.y
            }
        } else {
            if points[1].x != start.x {
                points[1].y = start.y
            }
            let last = points.count - 2
            if points[last].x != end.x {
                points[last].y = end.y
            }
        }

        return Array(removeDoubleBacks(mergeCoincident(points)).dropFirst().dropLast())
    }

    /// `path` (all right angles) with its ends moved to `start` and `end`.
    /// Each end's segment keeps its direction: a wire that left its pin
    /// sideways still does, and one that left it up or down still does. A
    /// slanted segment left over gets a corner; at an end, the corner keeps
    /// that end's direction.
    static func refit(_ path: [CGPoint], start: CGPoint, end: CGPoint) -> [CGPoint] {
        guard path.count >= 2 else { return path }
        func axis(_ a: CGPoint, _ b: CGPoint) -> RouteAxis? {
            a == b ? nil : a.y == b.y ? .horizontal : a.x == b.x ? .vertical : nil
        }
        let last = path.count - 1
        let startAxis = axis(path[0], path[1])
        let endAxis = axis(path[last - 1], path[last])
        if path.count == 2 {
            return [start] + leg(from: start, to: end, first: startAxis, last: endAxis, preferred: .horizontal)
        }

        var points = path
        points[0] = start
        points[last] = end
        if start != path[0], let startAxis {
            if startAxis == .horizontal { points[1].y = start.y } else { points[1].x = start.x }
        }
        if end != path[last], let endAxis {
            if endAxis == .horizontal { points[last - 1].y = end.y } else { points[last - 1].x = end.x }
        }

        var result = [points[0]]
        for index in 1...last {
            let p = result[result.count - 1], q = points[index]
            if p.x != q.x, p.y != q.y {
                // Leave the start along its axis, reach the end along its
                // axis; elsewhere go across first.
                let across: Bool
                if index == 1, let startAxis {
                    across = startAxis == .horizontal
                } else if index == last, let endAxis {
                    across = endAxis == .vertical
                } else {
                    across = true
                }
                result.append(across ? CGPoint(x: q.x, y: p.y) : CGPoint(x: p.x, y: q.y))
            }
            result.append(q)
        }
        return removeDoubleBacks(mergeCoincident(result))
    }

    /// Drops a vertex where the route turns straight back on itself (three
    /// points on one line, the middle one outside the other two). That is
    /// what an end moving past a stored vertex leaves behind. The first and
    /// last points never go.
    static func removeDoubleBacks(_ points: [CGPoint]) -> [CGPoint] {
        var result = points
        var index = 1
        while index + 1 < result.count {
            let a = result[index - 1], b = result[index], c = result[index + 1]
            let sameRow = a.y == b.y && b.y == c.y
            let sameColumn = a.x == b.x && b.x == c.x
            let turnsBack = (sameRow && (b.x - a.x) * (c.x - b.x) < 0)
                || (sameColumn && (b.y - a.y) * (c.y - b.y) < 0)
            if turnsBack {
                result.remove(at: index)
                result = mergeCoincident(result)
                index = max(index - 1, 1)
            } else {
                index += 1
            }
        }
        return result
    }

    /// Moves only the bend at `index`. The neighbouring bends stay where they are,
    /// so the segments on either side may be at any angle.
    static func moveNode(_ path: [CGPoint], at index: Int, to point: CGPoint) -> [CGPoint] {
        guard index > 0, index < path.count - 1 else { return path }
        var moved = path
        moved[index] = point
        return mergeCoincident(moved)
    }

    /// Moves the two vertices at the ends of the segment at `index` by `offset`.
    /// Pin tips never move, so a segment touching a pin moves only its other end.
    static func moveSegment(_ path: [CGPoint], at index: Int, by offset: CGSize) -> [CGPoint] {
        guard index >= 0, index + 1 < path.count else { return path }

        var moved = path
        if index > 0 {
            moved[index] = CGPoint(x: path[index].x + offset.width, y: path[index].y + offset.height)
        }
        if index + 1 < path.count - 1 {
            moved[index + 1] = CGPoint(x: path[index + 1].x + offset.width, y: path[index + 1].y + offset.height)
        }
        return mergeCoincident(moved)
    }

    /// Adds a bend on the segment at `index`, at the grid point nearest `point`.
    static func insertNode(_ path: [CGPoint], segment index: Int, at point: CGPoint) -> [CGPoint] {
        guard index >= 0, index + 1 < path.count else { return path }

        // Project the clicked point onto the segment, then snap it to the grid.
        let a = path[index]
        let b = path[index + 1]
        let dx = b.x - a.x
        let dy = b.y - a.y
        let lengthSquared = dx * dx + dy * dy
        let t = lengthSquared > 0
            ? min(max(((point.x - a.x) * dx + (point.y - a.y) * dy) / lengthSquared, 0), 1)
            : 0
        let node = SchematicGrid.snap(CGPoint(x: a.x + t * dx, y: a.y + t * dy))

        var result = path
        result.insert(node, at: index + 1)
        return mergeCoincident(result)
    }
}
