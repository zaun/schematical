// plan-13-C Phase 3: the plan-13 landing proof. Run twice, as two processes,
// so the reopen is a fresh app:
//
//   scripts/harness/build.sh scripts/harness/bus-landing.swift
//   H=/tmp/schematical-harness/H.app/Contents/MacOS/harness
//   $H draw /tmp/p13.schematical [image dir]   # draw it fresh and save
//   $H open /tmp/p13.schematical [image dir]   # reopen, run, switch on
import CoreGraphics
import Foundation

MainActor.assumeIsolated { runLanding() }

@MainActor func runLanding() {
    let args = CommandLine.arguments
    guard args.count >= 3 else { print("usage: harness draw|open <file> [image dir]"); exit(2) }
    let path = args[2]
    let images = args.count > 3 ? args[3] : "/tmp/p13img"
    let size = CGSize(width: 640, height: 400)
    let origin = CGPoint(x: -20, y: 40)
    func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    do {
        switch args[1] {
        case "draw":
            let doc = Harness.newDocument()
            func place(_ name: String, _ point: CGPoint) throws -> PlacedChip {
                guard let part = Harness.catalog.part(named: name) else { throw ProofError("no \(name)") }
                doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
                return doc.chips.last!
            }
            func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
                .pin(PinRef(chipID: chip.id, index: chip.layout.slots.first { $0.pin.name == name }!.pin.index))
            }
            func connect(_ a: LineAnchor, _ b: LineAnchor) throws {
                if let m = doc.connect(a, b) { throw ProofError("connect: \(m)") }
            }
            let supply = try place("5V", p(40, 150))
            let dip = try place("TE-GDS0204", p(120, 220))
            // Flipped (the Flip tool), so its anode faces port 2.
            var led = try place("GENERIC-LED", p(240, 330))
            doc.flipChip(led.id)
            led = doc.chips.first { $0.id == led.id }!
            let ground = try place("GND", p(140, 400))

            // A bus drawn as an L (horizontal, then down), plus a T line.
            guard let busID = doc.addBus(from: p(200, 100), to: p(520, 300)) else { throw ProofError("L") }
            doc.addBus(from: p(380, 100), to: p(380, 320))
            let bus = doc.bus(busID)!
            print("bus lines: \(bus.lines.map { "(\(Int($0.a.x)),\(Int($0.a.y)))-(\(Int($0.b.x)),\(Int($0.b.y)))" })")
            let b0 = bus.wires[0].id
            if let m = doc.addPort(at: p(260, 100)) { throw ProofError("port 1: \(m)") }
            let port1 = doc.selectedPortID!
            if let m = doc.addPort(at: p(380, 220)) { throw ProofError("port 2: \(m)") }
            let port2 = doc.selectedPortID!
            for port in [port1, port2] {
                if let m = doc.setPortWires(port, [b0]) { throw ProofError("tick: \(m)") }
            }
            doc.setPortSide(port2, .left)
            try connect(pin(supply, "VCC"), pin(dip, "1A"))
            try connect(pin(dip, "1B"), .busPin(BusPinRef(portID: port1, wireID: b0)))
            try connect(.busPin(BusPinRef(portID: port2, wireID: b0)), pin(led, "A"))
            try connect(pin(led, "K"), pin(ground, "GND"))
            if let m = doc.renameBusWire(bus: busID, wire: b0, to: "D0") { throw ProofError("rename: \(m)") }
            doc.selectedPortID = nil
            try Harness.save(doc, to: path)
            print("saved \(path)")
            Harness.renderCanvas(doc, to: "\(images)/c3-1-drawn.png", size: size, origin: origin)

        case "open":
            let doc = try Harness.open(path)
            print("opened \(path): \(doc.chips.count) parts, \(doc.wires.count) wires, \(doc.buses.count) bus, \(doc.buses.first?.ports.count ?? 0) ports")
            guard let dip = doc.chips.first(where: { $0.partNumber == "TE-GDS0204" }),
                  let led = doc.chips.first(where: { $0.partNumber == "GENERIC-LED" }) else { throw ProofError("parts") }
            let off = BusProof.ledLevel(doc, switchID: dip.id, ledID: led.id, on: false)
            let on = BusProof.ledLevel(doc, switchID: dip.id, ledID: led.id, on: true)
            print("LED: switch off \(off), switch on \(on)")
            print(off == 0 && on > 0 ? "LANDING OK" : "LANDING FAILED")
            let labels = doc.buses[0].ports.flatMap { doc.buses[0].pinWires(of: $0).map(\.name) }
            print("port pin labels: \(labels)")
            Harness.renderCanvas(doc, to: "\(images)/c3-2-reopened-lit.png", size: size, origin: origin)
            Harness.simulation.stop()

        default:
            print("usage: harness draw|open <file> [image dir]")
            exit(2)
        }
    } catch {
        print("FAIL: \(error)")
        exit(1)
    }
}
