// plan-13-B Phase 2: port pins are wire ends, and pins of one bus wire are
// one net. Runs the real simulation on the proof circuit and reads the LED.
//
//   scripts/harness/build.sh scripts/harness/bus-play.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness [image dir]
import CoreGraphics
import Foundation

MainActor.assumeIsolated { runPlayChecks() }

@MainActor func runPlayChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p13img"
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print(ok ? "  ok: \(what)" : "  FAIL: \(what)")
        if !ok { failures += 1 }
    }
    let size = CGSize(width: 720, height: 360)
    let origin = CGPoint(x: -20, y: 40)

    do {
        let proof = try BusProof.build()
        let doc = proof.document
        print("== drawn: \(doc.chips.count) parts, \(doc.wires.count) wires, \(doc.buses.count) bus")
        expect(doc.wires.contains { $0.end == .busPin(BusPinRef(portID: proof.port1, wireID: doc.bus(proof.busID)!.wires[0].id)) },
               "a wire ends on port 1 B0")
        Harness.renderCanvas(doc, to: "\(images)/b2-1-drawn.png", size: size, origin: origin, tool: .line)

        let netlist = NetlistBuilder.build(chips: doc.chips, wires: doc.wires, buses: doc.buses)
        let ledNets = netlist.connections.filter { $0.part == doc.chips.firstIndex { $0.id == proof.ledID }! }
        let dipNets = netlist.connections.filter { $0.part == doc.chips.firstIndex { $0.id == proof.switchID }! }
        print("  LED pins: \(ledNets.map { "\($0.pin)→net \($0.net)" })  switch pins: \(dipNets.map { "\($0.pin)→net \($0.net)" })")
        let anode = ledNets.first { $0.pin == "2" }?.net
        let oneB = dipNets.first { $0.pin == "4" }?.net
        expect(anode != nil && anode == oneB, "switch 1B and LED anode are one net through the bus")
        let without = NetlistBuilder.build(chips: doc.chips, wires: doc.wires, buses: [])
        let a2 = without.connections.first { $0.part == doc.chips.firstIndex { $0.id == proof.ledID }! && $0.pin == "2" }?.net
        let b2 = without.connections.first { $0.part == doc.chips.firstIndex { $0.id == proof.switchID }! && $0.pin == "4" }?.net
        expect(a2 != b2, "without the bus union they are two nets")

        print("== play mode")
        let off = proof.ledLevel(on: false)
        print("  switch off: LED \(off)")
        expect(off == 0, "LED dark")
        let on = proof.ledLevel(on: true)
        print("  switch on: LED \(on)")
        expect(on > 0, "LED lit")
        Harness.renderCanvas(doc, to: "\(images)/b2-2-on.png", size: size, origin: origin)
        let offAgain = proof.ledLevel(on: false)
        print("  switch off again: LED \(offAgain)")
        expect(offAgain == 0, "LED dark again")
        Harness.renderCanvas(doc, to: "\(images)/b2-3-off.png", size: size, origin: origin)
        Harness.simulation.stop()
    } catch {
        print("FAIL: \(error)"); failures += 1
    }

    print("== voltage check across the bus")
    do {
        let proof = try BusProof.build()
        let doc = proof.document
        let wires = doc.bus(proof.busID)!.wires
        for port in [proof.port1, proof.port2] {
            _ = doc.setPortWires(port, [wires[0].id, wires[2].id])
        }
        func place(_ name: String, _ point: CGPoint) -> PlacedChip {
            doc.placeChip(part: Harness.catalog.part(named: name)!, symbol: symbol(for: Harness.catalog.part(named: name)!),
                          centeredAt: point)
            return doc.chips.last!
        }
        let twelve = place("12V", CGPoint(x: 260, y: 20))
        let gate = place("SN74HC00D", CGPoint(x: 560, y: 420))
        func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
            .pin(PinRef(chipID: chip.id, index: chip.layout.slots.first { $0.pin.name == name }!.pin.index))
        }
        let first = doc.connect(pin(twelve, "VCC"), .busPin(BusPinRef(portID: proof.port1, wireID: wires[2].id)))
        expect(first == nil, "12V → port 1 B2 accepted (nothing else on that net)")
        let count = doc.wires.count
        let clash = doc.connect(.busPin(BusPinRef(portID: proof.port2, wireID: wires[2].id)), pin(gate, "VCC"))
        print("  port 2 B2 → 74HC00 VCC: \(clash ?? "accepted")")
        expect(clash != nil && doc.wires.count == count, "refused across the bus, nothing added")
        let direct = doc.connect(.busPin(BusPinRef(portID: proof.port2, wireID: wires[0].id)), pin(gate, "VCC"))
        print("  port 2 B0 (5V through the switch) → 74HC00 VCC: \(direct ?? "accepted")")
        expect(direct == nil, "the 5V net is fine")
        Harness.renderCanvas(doc, to: "\(images)/b2-4-clash.png", size: CGSize(width: 760, height: 520),
                             origin: CGPoint(x: -20, y: -20), tool: .line)
    } catch {
        print("FAIL: \(error)"); failures += 1
    }

    print(failures == 0 ? "ALL OK" : "\(failures) FAILED")
}
