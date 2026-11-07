import CoreGraphics
import Foundation

/// plan-13's proof circuit, drawn through the same document calls the tools
/// make: 5V → DIP switch position 1 → port 1 `B0` on a horizontal bus line;
/// port 2 `B0` on a vertical line joined to it at a T → LED anode; LED
/// cathode → GND.
@MainActor
struct BusProof {
    let document: SchematicDocument
    let busID: UUID
    let horizontalLine: UUID
    let verticalLine: UUID
    let port1: UUID
    let port2: UUID
    let switchID: UUID
    let ledID: UUID

    /// Builds the circuit. `port2Side` is the side of port 2's pins.
    static func build(in given: SchematicDocument? = nil,
                      port2Side: ChipLayout.Side = .right) throws -> BusProof {
        let doc = given ?? Harness.newDocument()
        func place(_ name: String, at point: CGPoint) throws -> PlacedChip {
            guard let part = Harness.catalog.part(named: name) else { throw ProofError("no part \(name)") }
            doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
            return doc.chips.last!
        }
        func pin(_ chip: PlacedChip, _ name: String) throws -> LineAnchor {
            guard let slot = chip.layout.slots.first(where: { $0.pin.name == name }) else {
                throw ProofError("\(chip.partNumber) has no pin \(name)")
            }
            return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
        }
        func connect(_ a: LineAnchor, _ b: LineAnchor) throws {
            if let message = doc.connect(a, b) { throw ProofError("connect refused: \(message)") }
        }

        let supply = try place("5V", at: CGPoint(x: 40, y: 150))
        let dip = try place("TE-GDS0204", at: CGPoint(x: 120, y: 220))
        let led = try place("LED-RED-0603", at: CGPoint(x: 540, y: 220))
        let ground = try place("GND", at: CGPoint(x: 640, y: 280))

        guard let busID = doc.addBus(from: CGPoint(x: 200, y: 100), to: CGPoint(x: 460, y: 100)) else {
            throw ProofError("bus: \(doc.editorMessage ?? "?")")
        }
        doc.addBus(from: CGPoint(x: 380, y: 100), to: CGPoint(x: 380, y: 320))
        guard let bus = doc.bus(busID), bus.lines.count == 2 else { throw ProofError("T line") }
        let horizontal = bus.lines.first { $0.isHorizontal }!.id
        let vertical = bus.lines.first { $0.isVertical }!.id
        let b0 = bus.wires[0].id

        if let m = doc.addPort(at: CGPoint(x: 260, y: 100)) { throw ProofError("port 1: \(m)") }
        let port1 = doc.selectedPortID!
        if let m = doc.addPort(at: CGPoint(x: 380, y: 220)) { throw ProofError("port 2: \(m)") }
        let port2 = doc.selectedPortID!
        for port in [port1, port2] {
            if let m = doc.setPortWires(port, [b0]) { throw ProofError("tick: \(m)") }
        }
        doc.setPortSide(port2, port2Side)

        try connect(try pin(supply, "VCC"), try pin(dip, "1A"))
        try connect(try pin(dip, "1B"), .busPin(BusPinRef(portID: port1, wireID: b0)))
        try connect(.busPin(BusPinRef(portID: port2, wireID: b0)), try pin(led, "A"))
        try connect(try pin(led, "K"), try pin(ground, "GND"))
        doc.selectedPortID = nil

        return BusProof(document: doc, busID: busID, horizontalLine: horizontal, verticalLine: vertical,
                        port1: port1, port2: port2, switchID: dip.id, ledID: led.id)
    }

    /// Opens a saved proof circuit, finding its parts and ports again.
    static func find(in doc: SchematicDocument) throws -> BusProof {
        guard let bus = doc.buses.first,
              let horizontal = bus.lines.first(where: { $0.isHorizontal }),
              let vertical = bus.lines.first(where: { $0.isVertical }),
              let port1 = bus.ports.first(where: { $0.line == horizontal.id }),
              let port2 = bus.ports.first(where: { $0.line == vertical.id }),
              let dip = doc.chips.first(where: { $0.partNumber == "TE-GDS0204" }),
              let led = doc.chips.first(where: { $0.partNumber == "LED-RED-0603" }) else {
            throw ProofError("not the proof circuit")
        }
        return BusProof(document: doc, busID: bus.id, horizontalLine: horizontal.id, verticalLine: vertical.id,
                        port1: port1.id, port2: port2.id, switchID: dip.id, ledID: led.id)
    }

    /// Runs the simulation, sets switch position 1, and returns the LED's
    /// light level once the circuit has settled.
    static func ledLevel(_ doc: SchematicDocument, switchID: UUID, ledID: UUID, on: Bool) -> Double {
        let sim = Harness.simulation
        if !sim.isRunning { sim.start(document: doc) }
        let chip = doc.chips.first { $0.id == switchID }!
        if chip.switchesOn.first != on, let isOn = doc.toggleSwitch(switchID, position: 1) {
            sim.setSwitch(switchID, position: 1, on: isOn)
        }
        RunLoop.main.run(until: Date().addingTimeInterval(0.6))
        return sim.lights[ledID]?.first ?? -1
    }

    func ledLevel(on: Bool) -> Double {
        Self.ledLevel(document, switchID: switchID, ledID: ledID, on: on)
    }
}

struct ProofError: Error, CustomStringConvertible {
    let description: String
    init(_ description: String) { self.description = description }
}
