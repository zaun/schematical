import CoreGraphics
import Foundation

/// Bus ports, and removing ports and bus lines (plan-68-D).
extension BusNetwork {
    /// The bus and index of a port.
    func portLocation(_ portID: UUID) -> (bus: Int, port: Int)? {
        for (b, bus) in buses.enumerated() {
            if let p = bus.ports.firstIndex(where: { $0.id == portID }) { return (b, p) }
        }
        return nil
    }

    /// A port and the bus it is on.
    func port(_ portID: UUID) -> (bus: Bus, port: BusPort)? {
        portLocation(portID).map { (buses[$0.bus], buses[$0.bus].ports[$0.port]) }
    }

    /// Every pin on every bus port.
    var busPinRefs: [BusPinRef] {
        buses.flatMap { bus in
            bus.ports.flatMap { port in bus.pinWires(of: port).map { BusPinRef(portID: port.id, wireID: $0.id) } }
        }
    }

    /// Where a wire meets a bus port pin; nil when the port is gone or the
    /// wire isn't ticked on it.
    func busPinTip(_ ref: BusPinRef) -> CGPoint? {
        guard let (bus, port) = port(ref.portID) else { return nil }
        return bus.pinTip(port, wire: ref.wireID)
    }

    /// The port whose body is under `point` (topmost first).
    func portHit(at point: CGPoint, tolerance: CGFloat) -> UUID? {
        for bus in buses.reversed() {
            for port in bus.ports.reversed() {
                if let body = bus.body(of: port), body.insetBy(dx: -tolerance, dy: -tolerance).contains(point) {
                    return port.id
                }
            }
        }
        return nil
    }

    /// Whether a wire end is on one of `wireIDs`' pins of a port (any pin
    /// when `wireIDs` is nil).
    static func endsOn(_ end: WireEnd, port portID: UUID, wireIDs: Set<UUID>? = nil) -> Bool {
        guard case let .busPin(ref) = end, ref.portID == portID else { return false }
        return wireIDs.map { $0.contains(ref.wireID) } ?? true
    }

    /// The Bus Port tool: an empty port where `point` is on a bus line, on
    /// the line's default side or, when a port there is in the way, the
    /// other. Returns the port added, or why not; nil for a point on no bus.
    mutating func addPort(at point: CGPoint, tolerance: CGFloat) -> Result<BusPort, PortRefusal>? {
        guard let hit = busHit(at: point, tolerance: tolerance),
              let b = buses.firstIndex(where: { $0.id == hit.bus }),
              let line = buses[b].line(hit.line) else { return nil }
        let bus = buses[b]
        let along = line.isHorizontal ? point.x : point.y
        let blocked = [line.alongRange.lowerBound, line.alongRange.upperBound] + bus.teeAlongs(on: line.id)
        if blocked.contains(where: { abs($0 - along) <= tolerance }) {
            return .failure(PortRefusal("A port can't go on the end of a bus line or on a joint."))
        }
        let defaultSide = BusPort.defaultSide(for: line)
        let sides = [defaultSide] + BusPort.sides(for: line).filter { $0 != defaultSide }
        let fitted = sides.lazy.compactMap { side -> BusPort? in
            var port = BusPort(line: line.id, start: SchematicGrid.snap(along), side: side)
            guard let start = bus.fit(port: port, slots: 1, preferredStart: port.start) else { return nil }
            port.start = start
            return port
        }.first
        guard let placed = fitted else { return .failure(PortRefusal("Not enough room on this bus line for a port.")) }
        buses[b].ports.append(placed)
        return .success(placed)
    }

    /// Where a port with the pins `ids` would start, or why it doesn't fit.
    func portWiresFit(_ portID: UUID, _ ids: Set<UUID>) -> Result<(start: CGFloat, removing: Set<UUID>), PortRefusal>? {
        guard let at = portLocation(portID) else { return nil }
        let bus = buses[at.bus]
        let port = bus.ports[at.port]
        let wanted = ids.intersection(bus.wires.map(\.id))
        guard let start = bus.fit(port: port, slots: wanted.count, preferredStart: port.start) else {
            return .failure(PortRefusal("Not enough room on this bus line for \(wanted.count) pins."))
        }
        return .success((start, port.wireIDs.subtracting(wanted)))
    }

    /// Stores a port's pins and start (`portWiresFit` said where).
    mutating func setPortWires(_ portID: UUID, _ ids: Set<UUID>, start: CGFloat) {
        guard let at = portLocation(portID) else { return }
        buses[at.bus].ports[at.port].start = start
        buses[at.bus].ports[at.port].wireIDs = ids.intersection(buses[at.bus].wires.map(\.id))
    }

    /// Sets the side a port's pins come out of, if it suits the line.
    /// Returns nil when it changed, a refusal when a port on that side is
    /// in the way, and `.none` (no change) otherwise.
    mutating func setPortSide(_ portID: UUID, _ side: ChipLayout.Side) -> PortSideResult {
        guard let at = portLocation(portID),
              let line = buses[at.bus].line(buses[at.bus].ports[at.port].line),
              BusPort.sides(for: line).contains(side) else { return .unchanged }
        let bus = buses[at.bus]
        var moved = bus.ports[at.port]
        moved.side = side
        guard bus.fit(port: moved, slots: bus.slots(of: moved), preferredStart: moved.start) == moved.start else {
            return .refused("A port on that side is in the way.")
        }
        buses[at.bus].ports[at.port].side = side
        return .changed
    }

    /// Moves a port along its line to `start`, only if the fit rule keeps it
    /// unchanged. Returns whether it is there.
    mutating func slidePort(_ portID: UUID, to start: CGFloat) -> Bool {
        guard let at = portLocation(portID) else { return false }
        let bus = buses[at.bus]
        let port = bus.ports[at.port]
        guard start != port.start else { return true }
        guard bus.fit(port: port, slots: bus.slots(of: port), preferredStart: start) == start else { return false }
        buses[at.bus].ports[at.port].start = start
        return true
    }

    /// Removes a port (its wires are loosened first, by the caller).
    mutating func removePort(_ portID: UUID) {
        guard let at = portLocation(portID) else { return }
        buses[at.bus].ports.remove(at: at.port)
    }

    /// Removes one bus line with its ports (their wires are loosened first,
    /// by the caller). A bus with no lines left goes. If the rest falls into
    /// pieces, the piece holding the first remaining line keeps the bus;
    /// each other piece becomes a bus of its own with the same wire names and
    /// fresh wire ids. Returns, per new bus, its ports and the old → new wire
    /// ids, for the wire ends on them; nil when the bus is gone.
    mutating func removeLine(bus id: UUID, line lineID: UUID) -> [(ports: Set<UUID>, wires: [UUID: UUID])]? {
        guard let index = buses.firstIndex(where: { $0.id == id }) else { return [] }
        var bus = buses[index]
        bus.ports.removeAll { $0.line == lineID }
        bus.lines.removeAll { $0.id == lineID }
        if bus.lines.isEmpty {
            buses.remove(at: index)
            return nil
        }
        let pieces = bus.pieces()
        func ports(on piece: [BusLine]) -> [BusPort] {
            let ids = Set(piece.map(\.id))
            return bus.ports.filter { ids.contains($0.line) }
        }
        var split: [Bus] = []
        var remaps: [(ports: Set<UUID>, wires: [UUID: UUID])] = []
        for piece in pieces.dropFirst() {
            let newWires = bus.wires.map { BusWire(name: $0.name) }
            let map = Dictionary(uniqueKeysWithValues: zip(bus.wires.map(\.id), newWires.map(\.id)))
            var moved = ports(on: piece)
            for k in moved.indices {
                moved[k].wireIDs = Set(moved[k].wireIDs.compactMap { map[$0] })
            }
            remaps.append((Set(moved.map(\.id)), map))
            split.append(Bus(name: Bus.defaultBusName(taken: (buses + split).map(\.name)), card: bus.card,
                             lines: piece, wires: newWires, ports: moved))
        }
        bus.lines = pieces[0]
        bus.ports = ports(on: pieces[0])
        buses[index] = bus
        buses.insert(contentsOf: split, at: index + 1)
        return remaps
    }
}

/// Why a port edit was refused: the editor's message.
struct PortRefusal: Error, Equatable {
    let message: String
    init(_ message: String) { self.message = message }
}

/// What `BusNetwork.setPortSide` did.
enum PortSideResult: Equatable {
    case changed, unchanged
    case refused(String)
}
