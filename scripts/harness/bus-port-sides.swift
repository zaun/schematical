// Ports on opposite sides of a bus line may share a place; ports on the same
// side may not.
//
//   scripts/harness/build.sh scripts/harness/bus-port-sides.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness [saved file] [image dir]
import CoreGraphics
import Foundation

MainActor.assumeIsolated { runSideChecks() }

@MainActor func runSideChecks() {
    let args = CommandLine.arguments
    let path = args.count > 1 ? args[1] : "/tmp/p13-sides.schematical"
    let images = args.count > 2 ? args[2] : "/tmp/p13img"
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print(ok ? "  ok: \(what)" : "  FAIL: \(what)")
        if !ok { failures += 1 }
    }
    func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    let doc = Harness.newDocument()
    let busID = doc.addBus(from: p(100, 100), to: p(100, 300))!
    let wires = doc.bus(busID)!.wires
    func port(_ id: UUID) -> BusPort { doc.port(id)!.port }

    print("== a right port, then a click at the same place")
    _ = doc.addPort(at: p(100, 200)); let right = doc.selectedPortID!
    _ = doc.setPortWires(right, Set(wires[0..<3].map(\.id)))
    _ = doc.addPort(at: p(100, 200)); let left = doc.selectedPortID!
    expect(left != right && port(right).side == .right && port(left).side == .left,
           "the second port goes on the other side")
    expect(doc.setPortWires(left, Set(wires[0..<3].map(\.id))) == nil, "3 pins on the left port")
    print("  right start \(Int(port(right).start)), left start \(Int(port(left).start))")
    expect(port(left).start == port(right).start, "both ports share the same place")

    print("== same side still blocks")
    let third = doc.addPort(at: p(100, 210))
    print("  third click: \(third ?? "placed")")
    expect(third == "Not enough room on this bus line for a port.", "no room on either side")
    expect(doc.setPortSide(right, .left) == "A port on that side is in the way.", "switching onto the left port is refused")
    expect(port(right).side == .right, "side unchanged")

    print("== sliding is blocked only by the same side")
    _ = doc.addPort(at: p(100, 120)); let top = doc.selectedPortID!
    expect(port(top).side == .right, "a right port near the top")
    doc.slidePort(left, from: port(left).start, by: -200)
    let leftExtent = doc.bus(busID)!.extent(of: port(left))
    print("  left port slid to \(Int(leftExtent.lowerBound))..\(Int(leftExtent.upperBound))")
    expect(leftExtent.lowerBound == 100, "the left port slides past the right ones to the line's end")
    doc.slidePort(right, from: port(right).start, by: -200)
    let rightExtent = doc.bus(busID)!.extent(of: port(right))
    let topExtent = doc.bus(busID)!.extent(of: port(top))
    print("  right port slid to \(Int(rightExtent.lowerBound))..\(Int(rightExtent.upperBound)), top right port \(Int(topExtent.lowerBound))..\(Int(topExtent.upperBound))")
    expect(rightExtent.lowerBound == topExtent.upperBound, "the right port stops at the other right port")

    print("== save and open")
    doc.selectedPortID = nil
    do {
        try Harness.save(doc, to: path)
        let reopened = try Harness.open(path)
        expect(reopened.buses == doc.buses, "reopens with the overlapping ports")
    } catch {
        print("  FAIL: \(error)"); failures += 1
    }
    _ = doc.slidePort(left, to: port(right).start)
    Harness.renderCanvas(doc, to: "\(images)/sides.png", size: CGSize(width: 300, height: 260), origin: p(0, 70))

    print(failures == 0 ? "ALL OK" : "\(failures) FAILED")
}
