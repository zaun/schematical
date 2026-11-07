// plan-13-B Phase 3: erasing ports and bus lines, unticking wired pins, the
// confirm step, and a split that leaves two buses.
//
//   scripts/harness/build.sh scripts/harness/bus-erase.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness [image dir]
import CoreGraphics
import Foundation

MainActor.assumeIsolated { runEraseChecks() }

@MainActor func runEraseChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p13img"
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print(ok ? "  ok: \(what)" : "  FAIL: \(what)")
        if !ok { failures += 1 }
    }
    func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    let size = CGSize(width: 720, height: 360)
    let origin = p(-20, 40)

    do {
        print("== unwired port: no dialog")
        var proof = try BusProof.build()
        var doc = proof.document
        _ = doc.addPort(at: p(430, 100))
        let port3 = doc.selectedPortID!
        let portsBefore = doc.bus(proof.busID)!.ports.count
        doc.erase(at: p(430, 115))
        expect(doc.pendingRemoval == nil && doc.port(port3) == nil, "removed at once")
        expect(doc.bus(proof.busID)!.ports.count == portsBefore - 1, "one port fewer")

        print("== wired port: asks; Cancel keeps; Delete removes it and its wire")
        let wiresBefore = doc.wires.count
        doc.erase(at: p(260, 115))
        print("  dialog: \(doc.pendingRemoval?.message ?? "none")")
        expect(doc.pendingRemoval?.message == "Delete this port and the 1 wire connected to it?", "asks")
        doc.erase(at: p(380, 300))
        expect(doc.bus(proof.busID)!.lines.count == 2, "erasing is paused while it asks")
        doc.cancelPendingRemoval()
        expect(doc.port(proof.port1) != nil && doc.wires.count == wiresBefore, "Cancel keeps everything")
        doc.erase(at: p(260, 115))
        doc.confirmPendingRemoval()
        expect(doc.port(proof.port1) == nil && doc.wires.count == wiresBefore - 1, "Delete removes the port and its wire")
        Harness.renderCanvas(doc, to: "\(images)/b3-1-port-deleted.png", size: size, origin: origin)

        print("== bus line with a port: asks, then removes line, port and wire")
        proof = try BusProof.build()
        doc = proof.document
        let count = doc.wires.count
        doc.erase(at: p(380, 300))
        print("  dialog: \(doc.pendingRemoval?.message ?? "none")")
        expect(doc.pendingRemoval?.message == "Delete this bus line, its 1 port and the 1 wire connected to them?", "asks")
        doc.confirmPendingRemoval()
        let bus = doc.bus(proof.busID)
        expect(bus?.lines.map(\.id) == [proof.horizontalLine], "the horizontal line stays alone")
        expect(doc.port(proof.port2) == nil && doc.port(proof.port1) != nil, "port 2 gone, port 1 stays")
        expect(doc.wires.count == count - 1, "port 2's wire gone")
        Harness.renderCanvas(doc, to: "\(images)/b3-2-line-deleted.png", size: size, origin: origin)

        print("== untick a wired pin: asks, then removes the pin and its wire")
        let b0 = doc.bus(proof.busID)!.wires[0].id
        let before = doc.wires.count
        expect(doc.requestPortWires(proof.port1, []) == nil, "no refusal")
        print("  dialog: \(doc.pendingRemoval?.message ?? "none")")
        expect(doc.pendingRemoval?.message == "Remove 1 pin and the 1 wire connected to them?", "asks")
        expect(doc.port(proof.port1)!.port.wireIDs == [b0], "nothing changed yet")
        doc.confirmPendingRemoval()
        expect(doc.port(proof.port1)!.port.wireIDs.isEmpty && doc.wires.count == before - 1, "pin and wire gone")
        let b1 = doc.bus(proof.busID)!.wires[1].id
        expect(doc.requestPortWires(proof.port1, [b1]) == nil && doc.pendingRemoval == nil, "ticking never asks")
        expect(doc.requestPortWires(proof.port1, []) == nil && doc.pendingRemoval == nil
               && doc.port(proof.port1)!.port.wireIDs.isEmpty, "unticking an unwired pin doesn't ask")
    } catch {
        print("FAIL: \(error)"); failures += 1
    }

    print("== split: erase the line holding two T lines apart")
    do {
        let doc = Harness.newDocument()
        func place(_ name: String, _ point: CGPoint) -> PlacedChip {
            let part = Harness.catalog.part(named: name)!
            doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
            return doc.chips.last!
        }
        func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
            .pin(PinRef(chipID: chip.id, index: chip.layout.slots.first { $0.pin.name == name }!.pin.index))
        }
        let supply = place("5V", p(40, 150))
        let dip = place("TE-GDS0204", p(120, 220))
        let led = place("LED-RED-0603", p(540, 220))
        let ground = place("GND", p(640, 280))
        let busID = doc.addBus(from: p(200, 100), to: p(460, 100))!
        doc.addBus(from: p(240, 100), to: p(240, 300))
        doc.addBus(from: p(420, 100), to: p(420, 300))
        let b0 = doc.bus(busID)!.wires[0].id
        _ = doc.addPort(at: p(240, 200)); let port1 = doc.selectedPortID!
        _ = doc.addPort(at: p(420, 200)); let port2 = doc.selectedPortID!
        _ = doc.setPortWires(port1, [b0]); _ = doc.setPortWires(port2, [b0])
        doc.setPortSide(port1, .left)
        _ = doc.connect(pin(supply, "VCC"), pin(dip, "1A"))
        _ = doc.connect(pin(dip, "1B"), .busPin(BusPinRef(portID: port1, wireID: b0)))
        _ = doc.connect(.busPin(BusPinRef(portID: port2, wireID: b0)), pin(led, "A"))
        _ = doc.connect(pin(led, "K"), pin(ground, "GND"))
        doc.selectedPortID = nil
        let lit = BusProof.ledLevel(doc, switchID: dip.id, ledID: led.id, on: true)
        print("  before the split, switch on: LED \(lit)")
        expect(lit > 0, "lights through the bus")
        _ = BusProof.ledLevel(doc, switchID: dip.id, ledID: led.id, on: false)
        Harness.simulation.stop()
        Harness.renderCanvas(doc, to: "\(images)/b3-3-before-split.png", size: size, origin: origin)

        let wireCount = doc.wires.count
        doc.erase(at: p(330, 100))
        expect(doc.pendingRemoval == nil, "no ports on that line, so no dialog")
        expect(doc.buses.count == 2, "two buses")
        expect(doc.port(port1) != nil && doc.port(port2) != nil && doc.wires.count == wireCount, "both ports and all wires stay")
        let other = doc.buses.first { $0.id != busID }!
        expect(other.ports.map(\.id) == [port2], "port 2 went with its piece")
        let p2wires = doc.port(port2)!.port.wireIDs
        expect(p2wires == [other.wires[0].id] && other.wires[0].id != b0, "its pin maps to the new bus's B0")
        let ends = doc.wires.flatMap { [$0.start, $0.end] }.filter {
            if case let .busPin(ref) = $0 { return ref.portID == port2 } else { return false }
        }
        expect(ends == [.busPin(BusPinRef(portID: port2, wireID: other.wires[0].id))], "its wire end follows")
        for (i, bus) in doc.buses.enumerated() {
            doc.selectedBusID = bus.id
            print("  bus \(i): wires \(bus.wires.map(\.name).joined(separator: ","))")
            expect(bus.wires.map(\.name) == (0..<8).map { "B\($0)" }, "bus \(i) shows B0-B7")
            Harness.renderSidebar(doc, to: "\(images)/b3-4-bus\(i)-panel.png", size: CGSize(width: 320, height: 520))
        }
        doc.selectedBusID = nil
        let after = BusProof.ledLevel(doc, switchID: dip.id, ledID: led.id, on: true)
        print("  after the split, switch on: LED \(after)")
        expect(after == 0, "the switch no longer lights the LED")
        Harness.renderCanvas(doc, to: "\(images)/b3-5-after-split-on.png", size: size, origin: origin)
        Harness.simulation.stop()
    }

    print(failures == 0 ? "ALL OK" : "\(failures) FAILED")
}
