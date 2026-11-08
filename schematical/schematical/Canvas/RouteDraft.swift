import CoreGraphics

/// A Line or Bus route being drawn: locked legs, then a live leg to the
/// cursor. Space locks the live leg (`lock`), Esc drops the last lock
/// (`unlock`), and `/` flips the live leg's L (`flip`). Each leg follows
/// `OrthogonalRoute.leg`.
struct RouteDraft: Equatable {
    /// Where the route starts.
    let start: CGPoint
    /// The axis the start pin forces on the first segment; nil for a free point.
    let startAxis: RouteAxis?
    /// Each locked leg: the points after its start, up to its end.
    private(set) var legs: [[CGPoint]] = []
    /// Whether `/` flipped the live leg.
    private(set) var flipped = false

    init(start: CGPoint, startAxis: RouteAxis?) {
        self.start = start
        self.startAxis = startAxis
    }

    var hasLocks: Bool { !legs.isEmpty }

    /// Where the live leg starts: the end of the last lock, or the start.
    var legStart: CGPoint { legs.last?.last ?? start }

    /// The live leg's first axis when nothing forces it: across the last
    /// locked segment, so the new leg turns; horizontal with no locks.
    /// `/` swaps it.
    var preferredAxis: RouteAxis {
        let locked = [start] + legs.flatMap { $0 }
        var axis = RouteAxis.horizontal
        if locked.count >= 2 {
            let a = locked[locked.count - 2], b = locked[locked.count - 1]
            axis = a.y == b.y ? .vertical : .horizontal
        }
        return flipped ? axis.other : axis
    }

    /// The axis forced on the live leg's first segment: the start pin's,
    /// only while nothing is locked.
    private var firstAxis: RouteAxis? { legs.isEmpty ? startAxis : nil }

    /// The live leg to `end`. `endAxis` is the axis a pin at `end` forces.
    func liveLeg(to end: CGPoint, endAxis: RouteAxis?) -> [CGPoint] {
        OrthogonalRoute.leg(from: legStart, to: end, first: firstAxis, last: endAxis, preferred: preferredAxis)
    }

    /// The whole route to `end`, start included, cleaned up.
    func path(to end: CGPoint, endAxis: RouteAxis?) -> [CGPoint] {
        OrthogonalRoute.simplified([start] + legs.flatMap { $0 } + liveLeg(to: end, endAxis: endAxis))
    }

    /// Whether `/` changes the live leg: only when neither of its ends is forced.
    func canFlip(endAxis: RouteAxis?) -> Bool {
        firstAxis == nil && endAxis == nil
    }

    mutating func flip() {
        flipped.toggle()
    }

    /// Locks the live leg to `end` as `path` shows it. Returns false, and
    /// changes nothing, when the leg is empty.
    @discardableResult
    mutating func lock(to end: CGPoint, endAxis: RouteAxis?) -> Bool {
        let leg = liveLeg(to: end, endAxis: endAxis)
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

    /// What the Bus tool tries for a route to `end`, in order: the route as
    /// drawn, then (when the live leg is a free L) the same route with the
    /// live leg flipped.
    func busCandidates(to end: CGPoint) -> [[CGPoint]] {
        var candidates = [path(to: end, endAxis: nil)]
        if canFlip(endAxis: nil), liveLeg(to: end, endAxis: nil).count == 2 {
            var other = self
            other.flip()
            candidates.append(other.path(to: end, endAxis: nil))
        }
        return candidates
    }
}
