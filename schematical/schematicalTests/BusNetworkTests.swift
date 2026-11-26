import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// `BusNetwork` on hand-built buses, with no document (plan-68-D): drawing,
/// wires and names, linked buses, types, lines, ports and removals.
@MainActor
final class BusNetworkTests: XCTestCase {
    /// A bus drawn from `a` to `b` by the Bus tool's rules.
    private func drawn(_ a: CGPoint, _ b: CGPoint, name: String = "BUS 1", among buses: [Bus] = []) throws -> Bus {
        guard case var .success(draft) = BusJoin.join(buses, from: a, to: b) else { throw Missing() }
        var bus = draft.bus
        bus.name = name
        draft = BusDraft(busIndex: nil, bus: bus, path: [])
        return draft.bus
    }

    /// One horizontal bus, 0…400 at y 100, with a port at 200 on its first two wires.
    private func network() throws -> (BusNetwork, Bus, BusPort) {
        var value = BusNetwork(buses: [try drawn(CGPoint(x: 0, y: 100), CGPoint(x: 400, y: 100))])
        guard case let .success(port)? = value.addPort(at: CGPoint(x: 200, y: 100), tolerance: 6) else {
            throw Missing()
        }
        let wires = Set(value.buses[0].wires.prefix(2).map(\.id))
        guard case let .success(fit)? = value.portWiresFit(port.id, wires) else { throw Missing() }
        value.setPortWires(port.id, wires, start: fit.start)
        return (value, value.buses[0], value.buses[0].ports[0])
    }

    func testLookupsAndMessages() throws {
        let (value, bus, port) = try network()
        XCTAssertEqual(value.bus(bus.id)?.id, bus.id)
        XCTAssertNil(value.bus(UUID()))
        XCTAssertTrue(BusNetwork.edgeBusUndeletable(bus).contains("can't be deleted"))
        XCTAssertTrue(BusNetwork.edgeBusLock(bus).contains("locked"))
        XCTAssertEqual(value.portLocation(port.id)?.bus, 0)
        XCTAssertNil(value.portLocation(UUID()))
        XCTAssertEqual(value.port(port.id)?.port.id, port.id)
        XCTAssertEqual(value.busPinRefs.count, 2)
        XCTAssertNotNil(value.busPinTip(value.busPinRefs[0]))
        XCTAssertNil(value.busPinTip(BusPinRef(portID: UUID(), wireID: UUID())))
        let body = try XCTUnwrap(bus.body(of: port))
        XCTAssertEqual(value.portHit(at: CGPoint(x: body.midX, y: body.midY), tolerance: 0), port.id)
        XCTAssertNil(value.portHit(at: CGPoint(x: -500, y: -500), tolerance: 0))
        XCTAssertEqual(value.busHit(at: CGPoint(x: 50, y: 102), tolerance: 6)?.bus, bus.id)
        XCTAssertNil(value.busHit(at: CGPoint(x: 50, y: 150), tolerance: 6))
        XCTAssertEqual(value.busEndHit(at: CGPoint(x: 1, y: 100), tolerance: 6)?.end, .a)
        XCTAssertNil(value.busEndHit(at: CGPoint(x: 200, y: 160), tolerance: 6))
        let ref = BusPinRef(portID: port.id, wireID: bus.wires[0].id)
        XCTAssertTrue(BusNetwork.endsOn(.busPin(ref), port: port.id))
        XCTAssertTrue(BusNetwork.endsOn(.busPin(ref), port: port.id, wireIDs: [bus.wires[0].id]))
        XCTAssertFalse(BusNetwork.endsOn(.busPin(ref), port: port.id, wireIDs: []))
        XCTAssertFalse(BusNetwork.endsOn(.busPin(ref), port: UUID()))
        XCTAssertFalse(BusNetwork.endsOn(.junction(UUID()), port: port.id))
    }

    func testPlaceNamesAddsReplacesAndRefusesALinkedJoin() throws {
        var value = BusNetwork(buses: [])
        guard case let .success(draft) = BusJoin.join([], from: .zero, to: CGPoint(x: 200, y: 0)) else { return XCTFail() }
        XCTAssertNil(value.place(draft))
        XCTAssertEqual(value.buses.map(\.name), ["BUS 1"])
        var grown = value.buses[0]
        grown.wires.removeLast()
        XCTAssertNil(value.place(BusDraft(busIndex: 0, bus: grown, path: [])))
        XCTAssertEqual(value.buses[0].wires.count, grown.wires.count)
        // A linked bus may not change its wires through a join.
        let twin = try drawn(CGPoint(x: 0, y: 300), CGPoint(x: 200, y: 300), name: "bus 1")
        value.buses.append(twin)
        var changed = value.buses[0]
        changed.wires.removeLast()
        XCTAssertNotNil(value.place(BusDraft(busIndex: 0, bus: changed, path: [])))
        XCTAssertEqual(value.buses[0].wires.count, grown.wires.count)
    }

    func testWireRules() {
        XCTAssertNotNil(BusNetwork.wireNameProblem(""))
        XCTAssertNotNil(BusNetwork.wireNameProblem(String(repeating: "x", count: Bus.nameLengthRange.upperBound + 1)))
        XCTAssertNotNil(BusNetwork.wireNameProblem("A B"))
        XCTAssertNil(BusNetwork.wireNameProblem("D0"))
        let old = [BusWire(name: "A"), BusWire(name: "B")]
        let renamed = BusNetwork.renamed(old, ["X", "Y", "Z"])
        XCTAssertEqual(renamed.map(\.name), ["X", "Y", "Z"])
        XCTAssertEqual(renamed.prefix(2).map(\.id), old.map(\.id))
        XCTAssertEqual(BusNetwork.renamed(old, ["Q"]).map(\.id), [old[0].id])
    }

    func testWireCountNamesAndMoves() throws {
        var (value, bus, _) = try network()
        let count = bus.wires.count
        XCTAssertNil(value.setWireCount(UUID(), 4, edge: nil))
        XCTAssertNotNil(value.setWireCount(bus.id, 4, edge: bus.id), "the card edge's count is locked")
        XCTAssertNotNil(value.setWireCount(bus.id, 1, edge: nil), "a port uses wire 2")
        XCTAssertNil(value.setWireCount(bus.id, count + 2, edge: nil))
        XCTAssertEqual(value.buses[0].wires.count, count + 2)
        XCTAssertNil(value.setWireCount(bus.id, 3, edge: nil))
        XCTAssertEqual(value.buses[0].wires.count, 3)

        XCTAssertNil(value.renameWire(bus: UUID(), wire: UUID(), to: "X"))
        XCTAssertNotNil(value.renameWire(bus: bus.id, wire: bus.wires[0].id, to: " "))
        XCTAssertNil(value.renameWire(bus: bus.id, wire: bus.wires[0].id, to: " CLK "))
        XCTAssertEqual(value.buses[0].wires[0].name, "CLK")

        XCTAssertNil(value.setWireNames(UUID(), ["A", "B"], edge: nil))
        XCTAssertNotNil(value.setWireNames(bus.id, ["A"], edge: nil), "too few")
        XCTAssertEqual(value.setWireNames(bus.id, ["A", ""], edge: nil), "A wire name can't be empty.")
        XCTAssertNotNil(value.setWireNames(bus.id, ["A", String(repeating: "x", count: 20)], edge: nil))
        XCTAssertEqual(value.setWireNames(bus.id, ["A B", "C D"], edge: nil), "A wire name can't contain spaces.")
        XCTAssertNotNil(value.setWireNames(bus.id, ["A", "B", "C", "D"], edge: bus.id))
        var used = value
        used.buses[0].ports[0].wireIDs.insert(used.buses[0].wires[2].id)
        XCTAssertEqual(used.setWireNames(bus.id, ["A", "B"], edge: nil),
                       "\(used.buses[0].wires[2].name) is used by a port; this bus needs at least 3 wires.")
        XCTAssertNil(value.setWireNames(bus.id, ["A", "B", "C"], edge: bus.id), "the same count is fine on the card edge")
        XCTAssertNil(value.setWireNames(bus.id, ["A", "B", "C", "D"], edge: nil))
        XCTAssertEqual(value.buses[0].wires.map(\.name), ["A", "B", "C", "D"])

        XCTAssertNil(value.moveWire(bus: UUID(), wire: UUID(), to: 0).message)
        XCTAssertTrue(value.moveWire(bus: bus.id, wire: value.buses[0].wires[0].id, to: 0).ports.isEmpty, "same place")
        let moved = value.moveWire(bus: bus.id, wire: value.buses[0].wires[0].id, to: 9)
        XCTAssertNil(moved.message)
        XCTAssertEqual(moved.ports, value.buses[0].ports.map(\.id))
        XCTAssertEqual(value.buses[0].wires.map(\.name), ["B", "C", "D", "A"])
    }

    func testAPresetTypeSetsTheWires() throws {
        var (value, bus, _) = try network()
        var card = BusCard()
        card.type = .isa8
        let pins = try XCTUnwrap(card.pinNames)
        value.buses[0].ports = []
        XCTAssertNil(value.setCard(bus.id, card, edge: nil))
        XCTAssertEqual(value.buses[0].wires.map(\.name), pins)
        XCTAssertNil(value.setCard(bus.id, card, edge: nil), "no change")
        XCTAssertNil(value.setCard(UUID(), card, edge: nil))
        XCTAssertEqual(value.setWireCount(bus.id, 4, edge: nil), BusNetwork.typeSetsWires)
        XCTAssertEqual(value.renameWire(bus: bus.id, wire: value.buses[0].wires[0].id, to: "X"), BusNetwork.typeSetsWires)
        XCTAssertEqual(value.setWireNames(bus.id, ["A", "B"], edge: nil), BusNetwork.typeSetsWires)
        XCTAssertEqual(value.moveWire(bus: bus.id, wire: value.buses[0].wires[0].id, to: 1).message, BusNetwork.typeSetsWires)
        XCTAssertNil(value.setCard(bus.id, BusCard(), edge: nil), "Custom keeps the wires")
        XCTAssertEqual(value.buses[0].wires.count, pins.count)
    }

    func testTypeChangesARefusedCardKeepsTheWires() throws {
        var (value, bus, _) = try network()
        var card = BusCard()
        card.type = .isa8
        XCTAssertNotNil(value.setCard(bus.id, card, edge: bus.id), "the card edge would change count")
        var one = BusCard()
        one.type = .customCard
        one.customCard.pads = 1
        one.customCard.bothSides = false
        XCTAssertEqual(one.pinNames?.count, 1)
        XCTAssertEqual(value.setCard(bus.id, one, edge: nil),
                       "A bus needs 2 to 999 wires; this card has 1 pins.")
        // A port on a wire past the card's pins refuses it.
        let last = value.buses[0].wires.count - 1
        value.buses[0].wires += (0..<200).map { BusWire(name: "W\($0)") }
        value.buses[0].ports[0].wireIDs.insert(value.buses[0].wires[last + 100].id)
        XCTAssertNotNil(value.setCard(bus.id, card, edge: nil))
    }

    func testLinkedBusesFollowEachOther() throws {
        var (value, bus, _) = try network()
        var twin = try drawn(CGPoint(x: 0, y: 400), CGPoint(x: 400, y: 400), name: "Bus 1")
        twin.wires = BusNetwork.renamed(twin.wires, ["X", "Y"])
        value.buses.append(twin)
        XCTAssertEqual(value.linkGroup(0), [0, 1])
        XCTAssertTrue(value.groupHasEdge([0, 1], edge: twin.id))
        XCTAssertFalse(value.groupHasEdge([0, 1], edge: nil))
        XCTAssertFalse(value.groupHasEdge([0], edge: twin.id))
        XCTAssertEqual(value.highestUsedWire([0, 1]), 1)
        value.syncLinked(0)
        XCTAssertEqual(value.buses[1].wires.map(\.name), bus.wires.map(\.name))
        value.syncLinked(0)
        XCTAssertEqual(value.buses[1].wires.count, bus.wires.count, "already matching")
        // A rename to the group's name in another case renames them all.
        XCTAssertNil(value.setName(bus.id, "bus 1", link: false, edge: nil))
        XCTAssertEqual(value.buses.map(\.name), ["bus 1", "bus 1"])
        XCTAssertNil(value.setName(bus.id, "MAIN", link: false, edge: nil))
        XCTAssertEqual(value.buses.map(\.name), ["MAIN", "bus 1"])
        XCTAssertNil(value.setName(bus.id, "MAIN", link: false, edge: nil), "the same name")
        XCTAssertNil(value.setName(UUID(), "X", link: false, edge: nil))
        XCTAssertNotNil(value.setName(bus.id, "  ", link: false, edge: nil))
    }

    func testRenamingOntoAnotherBusLinksOnlyWhenAsked() throws {
        var (value, bus, _) = try network()
        var other = try drawn(CGPoint(x: 0, y: 400), CGPoint(x: 400, y: 400), name: "DATA")
        other.wires = BusNetwork.renamed(other.wires, ["P", "Q", "R"])
        value.buses.append(other)
        XCTAssertEqual(value.linkTarget(bus.id, named: " data ")?.id, other.id)
        XCTAssertNil(value.linkTarget(UUID(), named: "DATA"))
        XCTAssertNotNil(value.setName(bus.id, "data", link: false, edge: nil))
        XCTAssertNotNil(value.setName(bus.id, "data", link: true, edge: bus.id), "the card edge's count would change")
        XCTAssertNil(value.setName(bus.id, "data", link: true, edge: nil))
        XCTAssertEqual(value.buses[0].name, "DATA")
        XCTAssertEqual(value.buses[0].wires.map(\.name), ["P", "Q", "R"])
        // A port on a wire the other bus lacks refuses the link.
        var short = try drawn(CGPoint(x: 0, y: 700), CGPoint(x: 400, y: 700), name: "TWO")
        short.wires = BusNetwork.renamed(short.wires, ["P"])
        value.buses.append(short)
        value.buses[0].ports[0].wireIDs.insert(value.buses[0].wires[2].id)
        XCTAssertNotNil(value.setName(bus.id, "two", link: true, edge: nil))
    }

    func testLinesMoveOnlyWhileTheBusStaysValid() throws {
        var (value, bus, _) = try network()
        func moved(_ dy: CGFloat) -> [BusLine] {
            bus.lines.map { BusLine(id: $0.id, a: CGPoint(x: $0.a.x, y: $0.a.y + dy), b: CGPoint(x: $0.b.x, y: $0.b.y + dy)) }
        }
        XCTAssertFalse(value.setLines(UUID(), moved(20)))
        XCTAssertFalse(value.setLines(bus.id, [BusLine(a: .zero, b: CGPoint(x: 100, y: 0))]), "other line ids")
        let turned = bus.lines.map { BusLine(id: $0.id, a: CGPoint(x: 0, y: 0), b: CGPoint(x: 0, y: 400)) }
        XCTAssertFalse(value.setLines(bus.id, turned), "a line may not change direction")
        let short = bus.lines.map { BusLine(id: $0.id, a: CGPoint(x: 0, y: 100), b: CGPoint(x: 20, y: 100)) }
        XCTAssertFalse(value.setLines(bus.id, short), "its port would not fit")
        value.buses.append(try drawn(CGPoint(x: 0, y: 300), CGPoint(x: 400, y: 300), name: "OTHER"))
        XCTAssertFalse(value.setLines(bus.id, moved(200)), "it would touch the other bus")
        XCTAssertTrue(value.setLines(bus.id, moved(20)))
        XCTAssertEqual(value.buses[0].lines[0].a.y, 120)
        XCTAssertTrue(BusNetwork.areApart(value.buses))
    }

    func testPorts() throws {
        var (value, bus, port) = try network()
        XCTAssertNil(value.addPort(at: CGPoint(x: 200, y: 500), tolerance: 6), "no bus there")
        XCTAssertEqual(value.addPort(at: CGPoint(x: 2, y: 100), tolerance: 6),
                       .failure(PortRefusal("A port can't go on the end of a bus line or on a joint.")))
        XCTAssertEqual(value.addPort(at: CGPoint(x: 200, y: 100), tolerance: 6)?.isSuccess, true,
                       "the other side of the one in the way")
        XCTAssertEqual(value.addPort(at: CGPoint(x: 200, y: 100), tolerance: 6),
                       .failure(PortRefusal("Not enough room on this bus line for a port.")))
        XCTAssertNil(value.portWiresFit(UUID(), []))
        let all = Set(bus.wires.map(\.id))
        if case .success? = value.portWiresFit(port.id, all.union((0..<100).map { _ in UUID() })) {} else {
            XCTFail("unknown wire ids do not count")
        }
        value.buses[0].wires += (0..<300).map { BusWire(name: "W\($0)") }
        XCTAssertEqual(value.portWiresFit(port.id, Set(value.buses[0].wires.map(\.id)))?.isSuccess, false)
        value.setPortWires(UUID(), [], start: 0)
        XCTAssertEqual(value.setPortSide(UUID(), .top), .unchanged)
        XCTAssertEqual(value.setPortSide(port.id, .left), .unchanged, "not a side of a horizontal line")
        XCTAssertEqual(value.setPortSide(port.id, port.side == .top ? .bottom : .top),
                       .refused("A port on that side is in the way."))
        value.buses[0].ports.removeLast()
        XCTAssertEqual(value.setPortSide(port.id, port.side == .top ? .bottom : .top), .changed)
        XCTAssertFalse(value.slidePort(UUID(), to: 0))
        XCTAssertTrue(value.slidePort(port.id, to: value.buses[0].ports[0].start))
        XCTAssertFalse(value.slidePort(port.id, to: 10_000))
        XCTAssertTrue(value.slidePort(port.id, to: 100))
        XCTAssertEqual(value.buses[0].ports[0].start, 100)
        value.removePort(UUID())
        value.removePort(port.id)
        XCTAssertTrue(value.buses[0].ports.isEmpty)
    }

    func testRemovingLines() throws {
        // An S of three lines: removing the middle one leaves two buses.
        var value = BusNetwork(buses: [])
        guard case let .success(first) = BusJoin.join([], from: .zero, to: CGPoint(x: 200, y: 0)) else { return XCTFail() }
        _ = value.place(first)
        for (a, b) in [(CGPoint(x: 200, y: 0), CGPoint(x: 200, y: 200)), (CGPoint(x: 200, y: 200), CGPoint(x: 400, y: 200))] {
            guard case let .success(draft) = BusJoin.join(value.buses, from: a, to: b) else { return XCTFail() }
            _ = value.place(draft)
        }
        XCTAssertEqual(value.buses.count, 1)
        XCTAssertEqual(value.buses[0].lines.count, 3)
        let bus = value.buses[0]
        let middle = try XCTUnwrap(bus.lines.first { !$0.isHorizontal })
        let far = try XCTUnwrap(bus.lines.first { $0.a.y == 200 || $0.b.y == 200 && $0.isHorizontal })
        guard case let .success(port)? = value.addPort(at: CGPoint(x: (far.a.x + far.b.x) / 2, y: 200), tolerance: 6) else {
            return XCTFail("no port")
        }
        value.setPortWires(port.id, [bus.wires[0].id], start: port.start)
        XCTAssertEqual(value.removeLine(bus: UUID(), line: UUID())?.count, 0)
        let remaps = try XCTUnwrap(value.removeLine(bus: bus.id, line: middle.id))
        XCTAssertEqual(value.buses.count, 2)
        XCTAssertEqual(remaps.count, 1)
        XCTAssertEqual(remaps[0].ports, [port.id])
        XCTAssertEqual(remaps[0].wires[bus.wires[0].id], value.buses[1].wires[0].id)
        XCTAssertEqual(value.buses[1].ports[0].wireIDs, [value.buses[1].wires[0].id])
        // The last line of a bus takes the bus with it.
        XCTAssertNil(value.removeLine(bus: value.buses[1].id, line: value.buses[1].lines[0].id))
        XCTAssertEqual(value.buses.count, 1)
    }
}

private struct Missing: Error {}

private extension Result {
    var isSuccess: Bool {
        if case .success = self { return true }
        return false
    }
}
