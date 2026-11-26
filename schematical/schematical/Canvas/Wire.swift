import CoreGraphics
import Foundation

/// A pin on a placed chip.
nonisolated struct PinRef: Hashable {
    let chipID: UUID
    /// The pin's position in its part's pinout (`ChipPin.index`).
    let index: Int
}

/// A pin on a bus port: the port, and the bus wire the pin is for.
nonisolated struct BusPinRef: Hashable {
    let portID: UUID
    let wireID: UUID
}

/// One end of a wire: a pin on a placed part, a junction where wires meet,
/// or a pin on a bus port.
nonisolated enum WireEnd: Hashable {
    case pin(PinRef)
    case junction(UUID)
    case busPin(BusPinRef)
    /// A net label's point (`NetLabel`).
    case label(UUID)
}

/// A grid point where wires meet. Wires that branch from a point on another
/// wire both end here, so the point is fixed in place.
nonisolated struct Junction: Identifiable, Hashable {
    let id: UUID
    /// Moves only with a group drag (plan-15); always on the grid.
    var point: CGPoint
    /// A loose end: the spot where a deleted pin was. The wires on it kept
    /// their routes, and dropping a pin here connects them again (plan-30).
    var isLoose: Bool

    init(id: UUID = UUID(), point: CGPoint, isLoose: Bool = false) {
        self.id = id
        self.point = point
        self.isLoose = isLoose
    }
}

/// Where a line can start or end in Line mode: a pin, a bus port pin, or a
/// grid point that an existing wire passes through.
enum LineAnchor: Hashable {
    case pin(PinRef)
    case point(CGPoint)
    case busPin(BusPinRef)
    case label(UUID)
}

/// A wire joins two ends. Its route is the start tip, then the bends in
/// `interior`, then the end tip. Every segment is horizontal or vertical.
nonisolated struct Wire: Identifiable, Hashable {
    let id: UUID
    var start: WireEnd
    var end: WireEnd
    var interior: [CGPoint]

    init(id: UUID = UUID(), start: WireEnd, end: WireEnd, interior: [CGPoint]) {
        self.id = id
        self.start = start
        self.end = end
        self.interior = interior
    }
}
