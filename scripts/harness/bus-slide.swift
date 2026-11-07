// plan-13-C Phase 2: sliding ports along their line, as the canvas does it
// (each frame: press-time start + drag, clamped; settle on release).
//
//   scripts/harness/build.sh scripts/harness/bus-slide.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness [image dir]
import CoreGraphics
import Foundation

MainActor.assumeIsolated { runSlideChecks() }

@MainActor func runSlideChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p13img"
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print(ok ? "  ok: \(what)" : "  FAIL: \(what)")
        if !ok { failures += 1 }
    }
    func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    let doc = Harness.newDocument()
    let busID = doc.addBus(from: p(100, 100), to: p(400, 100))!
    let wires = doc.bus(busID)!.wires
    _ = doc.addPort(at: p(140, 100)); let left = doc.selectedPortID!
    _ = doc.setPortWires(left, [wires[0].id, wires[1].id])
    _ = doc.addPort(at: p(300, 100)); let right = doc.selectedPortID!
    _ = doc.setPortWires(right, [wires[0].id])
    let led = Harness.catalog.part(named: "LED-RED-0603")!
    doc.placeChip(part: led, symbol: symbol(for: led), centeredAt: p(330, 240))
    let ledChip = doc.chips.last!
    let anode = PinRef(chipID: ledChip.id, index: ledChip.layout.slots.first { $0.pin.name == "A" }!.pin.index)
    let rightPin = BusPinRef(portID: right, wireID: wires[0].id)
    _ = doc.connect(.busPin(rightPin), .pin(anode))
    doc.selectedPortID = nil

    func start(_ id: UUID) -> CGFloat { doc.port(id)!.port.start }
    func extentText(_ id: UUID) -> String {
        let (bus, port) = doc.port(id)!
        let e = bus.extent(of: port)
        return "\(Int(e.lowerBound))..\(Int(e.upperBound))"
    }
    func wireFollows() -> Bool {
        let wire = doc.wires.first { $0.start == .busPin(rightPin) }!
        return doc.path(forWire: wire.id).first == doc.busPinTip(rightPin)
    }
    /// A Pointer drag on the port's body, frame by frame, as the canvas does.
    func slide(_ id: UUID, by along: CGFloat) -> [Int] {
        let base = start(id)
        doc.selectedPortID = id
        var seen: [Int] = []
        let count = max(Int(abs(along) / 7), 1)          // uneven frames, like a real mouse
        for k in 1...count {
            doc.slidePort(id, from: base, by: along * CGFloat(k) / CGFloat(count))
            if seen.last != Int(start(id)) { seen.append(Int(start(id))) }
        }
        doc.settleWires(attachedToPort: id)
        return seen
    }
    let size = CGSize(width: 420, height: 260)
    let origin = p(80, 40)
    Harness.renderCanvas(doc, to: "\(images)/c2-1-placed.png", size: size, origin: origin)

    print("== slide the right port left into the left one")
    print("  left \(extentText(left)), right \(extentText(right))")
    let steps = slide(right, by: -300)
    print("  starts: \(steps)  → right \(extentText(right))")
    expect(start(right) == 180, "stops at the left port's edge (extent 170..190)")
    expect(steps.allSatisfy { $0 % 10 == 0 }, "10-unit grid steps")
    expect(wireFollows(), "its wire follows")
    Harness.renderCanvas(doc, to: "\(images)/c2-2-at-neighbour.png", size: size, origin: origin)

    print("== slide the left port left past the line's end")
    _ = slide(left, by: -200)
    print("  left \(extentText(left))")
    expect(start(left) == 110, "stops at the end (extent 100..140)")
    expect(doc.port(left)!.port.line == doc.bus(busID)!.lines[0].id, "never leaves its line")

    print("== a T line between them; slide the right port into it")
    _ = slide(right, by: 60)                      // back to 240: extent 230..250
    doc.addBus(from: p(280, 100), to: p(280, 40))
    expect(doc.bus(busID)!.lines.count == 2, "T line at x 280")
    let toT = slide(right, by: 200)
    print("  starts: \(toT)  → right \(extentText(right))")
    expect(start(right) == 270, "stops at the T (extent 260..280)")
    expect(wireFollows(), "its wire follows")
    Harness.renderCanvas(doc, to: "\(images)/c2-3-at-tee.png", size: size, origin: origin)

    print("== a press that doesn't move only selects")
    let before = start(left)
    doc.selectedPortID = nil
    _ = slide(left, by: 0)
    expect(start(left) == before && doc.selectedPortID == left, "selected, not moved")

    print(failures == 0 ? "ALL OK" : "\(failures) FAILED")
}
