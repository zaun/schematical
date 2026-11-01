import CoreGraphics
import Foundation
import Observation

// MARK: - Grid

enum SchematicGrid {
    /// Every placed block corner, pin tip and wire bend snaps to this spacing.
    static let spacing: CGFloat = 10

    static func snap(_ value: CGFloat) -> CGFloat {
        (value / spacing).rounded() * spacing
    }

    static func snap(_ point: CGPoint) -> CGPoint {
        CGPoint(x: snap(point.x), y: snap(point.y))
    }
}

// MARK: - Tools

enum SchematicTool: String, CaseIterable, Identifiable {
    case pointer
    case eraser
    case line

    var id: String { rawValue }

    var title: String {
        switch self {
        case .pointer: "Pointer"
        case .eraser: "Eraser"
        case .line: "Line"
        }
    }

    var symbol: String {
        switch self {
        case .pointer: "cursorarrow"
        case .eraser: "eraser"
        case .line: "line.diagonal"
        }
    }
}

// MARK: - Block geometry

/// Schematic block geometry, derived from the backend's pin roles.
///
/// Left column: power pins at the top, a one-row gap, then inputs, then any
/// unconnected pins.
/// Right column: outputs and in/outs at the top, and ground pins at the bottom,
/// with a gap before them.
nonisolated struct ChipLayout: Hashable {
    enum Side: Hashable {
        case left
        case right
    }

    struct PinSlot: Identifiable, Hashable {
        let pin: ChipPin
        let side: Side
        /// Vertical centre of the pin inside the body, in body coordinates.
        let centerY: CGFloat

        var id: Int { pin.number }
    }

    /// Height of one pin row. Two grid cells, so every pin lands on the grid.
    static let pitch: CGFloat = 20
    static let headerHeight: CGFloat = 30
    static let bodyWidth: CGFloat = 120
    static let stubLength: CGFloat = 10
    /// Room on each side of the body for pin numbers.
    static let labelMargin: CGFloat = 30

    let size: CGSize
    let slots: [PinSlot]

    init(chip: ChipDefinition) {
        func byNumber(_ a: ChipPin, _ b: ChipPin) -> Bool { a.number < b.number }
        func pins(_ roles: PinRole...) -> [ChipPin] {
            chip.pins.filter { roles.contains($0.role) }.sorted(by: byNumber)
        }

        let power = pins(.power)
        let inputs = pins(.input)
        let unused = pins(.noConnect)
        let outputs = pins(.output, .inOut)
        let ground = pins(.ground)

        let left = Self.stack([power, inputs, unused])
        let rightTop = outputs.count + (ground.isEmpty ? 0 : 1 + ground.count)
        let rowCount = max(left.count, rightTop, 1)

        let leftRows: [ChipPin?] = left + [ChipPin?](repeating: nil, count: rowCount - left.count)
        var rightRows = [ChipPin?](repeating: nil, count: rowCount)
        for (index, pin) in outputs.enumerated() {
            rightRows[index] = pin
        }
        for (index, pin) in ground.enumerated() {
            rightRows[rowCount - ground.count + index] = pin
        }

        var slots: [PinSlot] = []
        for (row, pin) in leftRows.enumerated() {
            if let pin {
                slots.append(PinSlot(pin: pin, side: .left, centerY: Self.centerY(forRow: row)))
            }
        }
        for (row, pin) in rightRows.enumerated() {
            if let pin {
                slots.append(PinSlot(pin: pin, side: .right, centerY: Self.centerY(forRow: row)))
            }
        }

        self.slots = slots
        self.size = CGSize(
            width: Self.bodyWidth,
            height: Self.headerHeight + Self.pitch * CGFloat(rowCount)
        )
    }

    /// Joins non-empty groups top to bottom, with one empty row between groups.
    private static func stack(_ groups: [[ChipPin]]) -> [ChipPin?] {
        var rows: [ChipPin?] = []
        for group in groups where !group.isEmpty {
            if !rows.isEmpty {
                rows.append(nil)
            }
            rows.append(contentsOf: group.map { Optional($0) })
        }
        return rows
    }

    private static func centerY(forRow row: Int) -> CGFloat {
        headerHeight + pitch * CGFloat(row) + pitch / 2
    }
}

// MARK: - Placed items

nonisolated struct PlacedChip: Identifiable, Hashable {
    let id: UUID
    let reference: String
    let partNumber: String
    let chip: ChipDefinition
    let layout: ChipLayout
    /// Top-left corner of the body. Always on the grid.
    var origin: CGPoint

    init(reference: String, partNumber: String, chip: ChipDefinition, origin: CGPoint) {
        self.id = UUID()
        self.reference = reference
        self.partNumber = partNumber
        self.chip = chip
        self.layout = ChipLayout(chip: chip)
        self.origin = origin
    }

    var bodyRect: CGRect {
        CGRect(origin: origin, size: layout.size)
    }

    /// The body plus its pin stubs, used for hit testing.
    var hitRect: CGRect {
        bodyRect.insetBy(dx: -ChipLayout.stubLength, dy: 0)
    }
}

/// A pin on a placed chip.
struct PinRef: Hashable {
    let chipID: UUID
    let number: Int
}

/// A wire joins two pins. Its route is the pin tip, then the bends in
/// `interior`, then the other pin tip. Every segment is horizontal or vertical.
struct Wire: Identifiable, Hashable {
    let id = UUID()
    var start: PinRef
    var end: PinRef
    var interior: [CGPoint]
}

// MARK: - Orthogonal routing

/// Right-angle routing helpers. Paths are arrays of grid points where
/// consecutive points share an x or a y.
enum OrthogonalRoute {
    /// Default route between two tips: out horizontally, a vertical run at the
    /// midpoint, then horizontally into the target.
    static func route(from a: CGPoint, to b: CGPoint) -> [CGPoint] {
        if a.x == b.x || a.y == b.y {
            return normalize([a, b])
        }
        let midX = SchematicGrid.snap((a.x + b.x) / 2)
        return normalize([a, CGPoint(x: midX, y: a.y), CGPoint(x: midX, y: b.y), b])
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

        return Array(mergeCoincident(points).dropFirst().dropLast())
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

// MARK: - Document

/// The schematic being edited: placed blocks, wires and the selection.
@MainActor
@Observable
final class SchematicDocument {
    private(set) var chips: [PlacedChip] = []
    private(set) var wires: [Wire] = []
    var selectedChipID: UUID?

    private var nextReference = 1

    // MARK: Chips

    /// Places a block with its centre under `point`, snapped to the grid.
    func placeChip(partNumber: String, chip: ChipDefinition, centeredAt point: CGPoint) {
        let layout = ChipLayout(chip: chip)
        let origin = SchematicGrid.snap(
            CGPoint(x: point.x - layout.size.width / 2, y: point.y - layout.size.height / 2)
        )
        let placed = PlacedChip(
            reference: "U\(nextReference)",
            partNumber: partNumber,
            chip: chip,
            origin: origin
        )
        nextReference += 1
        chips.append(placed)
        selectedChipID = placed.id
    }

    func moveChip(_ id: UUID, to origin: CGPoint) {
        guard let index = chips.firstIndex(where: { $0.id == id }) else { return }
        chips[index].origin = SchematicGrid.snap(origin)
    }

    func chip(at point: CGPoint) -> PlacedChip? {
        chips.last { $0.hitRect.contains(point) }
    }

    // MARK: Pins

    /// Where a wire leaves or enters a pin: the end of its pin stub.
    func pinTip(_ ref: PinRef) -> CGPoint? {
        guard let chip = chips.first(where: { $0.id == ref.chipID }),
              let slot = chip.layout.slots.first(where: { $0.pin.number == ref.number }) else {
            return nil
        }
        let stub = ChipLayout.stubLength
        let x = slot.side == .left
            ? chip.origin.x - stub
            : chip.origin.x + chip.layout.size.width + stub
        return CGPoint(x: x, y: chip.origin.y + slot.centerY)
    }

    /// Tip position of every pin on every placed chip.
    var pinTips: [CGPoint] {
        chips.flatMap { chip in
            chip.layout.slots.compactMap { slot in
                pinTip(PinRef(chipID: chip.id, number: slot.pin.number))
            }
        }
    }

    /// The pin whose tip is within `radius` of `point`.
    func pin(at point: CGPoint, radius: CGFloat = 8) -> PinRef? {
        for chip in chips.reversed() {
            for slot in chip.layout.slots {
                let ref = PinRef(chipID: chip.id, number: slot.pin.number)
                if let tip = pinTip(ref), Self.distance(from: point, to: tip) <= radius {
                    return ref
                }
            }
        }
        return nil
    }

    // MARK: Wires

    /// Full route of a wire, from one pin tip to the other.
    func path(forWire id: UUID) -> [CGPoint] {
        guard let wire = wires.first(where: { $0.id == id }),
              let start = pinTip(wire.start),
              let end = pinTip(wire.end) else {
            return []
        }
        // Same rules as settleWires, applied on the fly, so the wire looks the
        // same while dragging as it does after the drop.
        let interior = OrthogonalRoute.settled(start: start, interior: wire.interior, end: end)
        return [start] + interior + [end]
    }

    var wirePaths: [[CGPoint]] {
        wires.map { path(forWire: $0.id) }
    }

    /// Connects two pins with an orthogonal route.
    func addWire(from start: PinRef, to end: PinRef) {
        guard start != end, let a = pinTip(start), let b = pinTip(end) else { return }
        let route = OrthogonalRoute.route(from: a, to: b)
        wires.append(Wire(start: start, end: end, interior: Array(route.dropFirst().dropLast())))
    }

    /// Called when a block is released. Re-fits the wires attached to it and
    /// stores the result, so the change stays rather than being recomputed per frame.
    func settleWires(attachedTo chipID: UUID) {
        for index in wires.indices where wires[index].start.chipID == chipID || wires[index].end.chipID == chipID {
            guard let start = pinTip(wires[index].start), let end = pinTip(wires[index].end) else { continue }
            wires[index].interior = OrthogonalRoute.settled(
                start: start,
                interior: wires[index].interior,
                end: end
            )
        }
    }

    /// Stores a new route for a wire. The pin tips stay where they are.
    func setWirePath(_ id: UUID, _ path: [CGPoint]) {
        guard let index = wires.firstIndex(where: { $0.id == id }), path.count >= 2 else { return }
        wires[index].interior = Array(path.dropFirst().dropLast())
    }

    /// A bend point on a wire, if `point` is on one.
    func nodeHit(at point: CGPoint, tolerance: CGFloat = 6) -> (wire: UUID, index: Int)? {
        for wire in wires.reversed() {
            let path = path(forWire: wire.id)
            guard path.count > 2 else { continue }
            for index in 1..<(path.count - 1)
                where Self.distance(from: point, to: path[index]) <= tolerance {
                return (wire.id, index)
            }
        }
        return nil
    }

    /// A segment of a wire, if `point` is on one. `index` is the segment's
    /// start point within the wire's path.
    func segmentHit(at point: CGPoint, tolerance: CGFloat = 6) -> (wire: UUID, index: Int)? {
        for wire in wires.reversed() {
            let path = path(forWire: wire.id)
            for index in 0..<max(path.count - 1, 0)
                where Self.distance(from: point, toSegment: path[index], path[index + 1]) <= tolerance {
                return (wire.id, index)
            }
        }
        return nil
    }

    /// Double-click on a wire: adds a bend at that point.
    func insertNode(at point: CGPoint, tolerance: CGFloat = 6) {
        guard let hit = segmentHit(at: point, tolerance: tolerance) else { return }
        let path = path(forWire: hit.wire)
        setWirePath(hit.wire, OrthogonalRoute.insertNode(path, segment: hit.index, at: point))
    }

    // MARK: Erasing

    /// Removes whatever is under `point`: a block (and its wires) first, then a wire.
    func erase(at point: CGPoint, tolerance: CGFloat = 6) {
        if let hit = chip(at: point) {
            chips.removeAll { $0.id == hit.id }
            wires.removeAll { $0.start.chipID == hit.id || $0.end.chipID == hit.id }
            if selectedChipID == hit.id {
                selectedChipID = nil
            }
        } else if let hit = segmentHit(at: point, tolerance: tolerance) {
            wires.removeAll { $0.id == hit.wire }
        }
    }

    // MARK: Geometry

    static func distance(from a: CGPoint, to b: CGPoint) -> CGFloat {
        hypot(a.x - b.x, a.y - b.y)
    }

    static func distance(from point: CGPoint, toSegment a: CGPoint, _ b: CGPoint) -> CGFloat {
        let dx = b.x - a.x
        let dy = b.y - a.y
        let lengthSquared = dx * dx + dy * dy

        guard lengthSquared > 0 else {
            return distance(from: point, to: a)
        }

        let t = min(max(((point.x - a.x) * dx + (point.y - a.y) * dy) / lengthSquared, 0), 1)
        return distance(from: point, to: CGPoint(x: a.x + t * dx, y: a.y + t * dy))
    }
}
