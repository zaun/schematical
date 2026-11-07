// plan-13-B Phase 1: ports, the fit rule, side, the wire-count minimum.
//
//   scripts/harness/build.sh scripts/harness/bus-ports.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness [image dir]
import CoreGraphics
import Foundation

MainActor.assumeIsolated { runPortChecks() }

@MainActor func runPortChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p13img"
    let doc = Harness.newDocument()
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print(ok ? "  ok: \(what)" : "  FAIL: \(what)")
        if !ok { failures += 1 }
    }
    func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    var shotCount = 0
    func shot(_ name: String) {
        shotCount += 1
        Harness.renderCanvas(doc, to: "\(images)/b1-\(shotCount)-\(name)-canvas.png",
                             size: CGSize(width: 360, height: 220), origin: p(60, 20))
        Harness.renderSidebar(doc, to: "\(images)/b1-\(shotCount)-\(name)-panel.png",
                              size: CGSize(width: 320, height: 560))
    }
    func describe(_ portID: UUID) -> String {
        guard let (bus, port) = doc.port(portID) else { return "gone" }
        let e = bus.extent(of: port)
        return "start \(Int(port.start)) extent \(Int(e.lowerBound))..\(Int(e.upperBound)) side \(port.side) pins \(bus.pinWires(of: port).map(\.name).joined(separator: ","))"
    }

    guard let busID = doc.addBus(from: p(100, 100), to: p(300, 100)) else { print("FAIL: bus"); return }
    let wires = { doc.bus(busID)!.wires }

    print("== port at x 260")
    expect(doc.addPort(at: p(260, 101)) == nil, "placed")
    guard let port1 = doc.selectedPortID else { print("FAIL: not selected"); return }
    print("  \(describe(port1))")
    expect(doc.port(port1)!.port.wireIDs.isEmpty, "empty and selected")
    shot("placed")

    print("== tick B0-B3")
    expect(doc.setPortWires(port1, Set(wires()[0..<4].map(\.id))) == nil, "accepted")
    print("  \(describe(port1))")
    expect(doc.port(port1)!.port.start == 230, "shifted left to end at 300 (start 230)")
    shot("B0-B3")

    print("== 12 wires, Select All")
    _ = doc.setBusWireCount(busID, 12)
    let before = doc.port(port1)!.port
    let refusal = doc.setPortWires(port1, Set(wires().map(\.id)))
    print("  message: \(refusal ?? "nil")")
    expect(refusal == "Not enough room on this bus line for 12 pins.", "room message")
    expect(doc.port(port1)!.port == before, "nothing changed")

    print("== 8 pins fit by moving left")
    let probe = doc.bus(busID)!.fit(port: before, slots: 8, preferredStart: before.start)
    print("  fit(slots: 8) from 230 → \(probe.map { "\(Int($0))" } ?? "nil")")
    expect(probe == 150, "8 pins (160) fit on the 200-long line at start 150")
    _ = doc.setBusWireCount(busID, 8)

    print("== side Top")
    let tipBottom = doc.bus(busID)!.pinTip(doc.port(port1)!.port, wire: wires()[0].id)
    doc.setPortSide(port1, .top)
    let tipTop = doc.bus(busID)!.pinTip(doc.port(port1)!.port, wire: wires()[0].id)
    print("  B0 tip bottom \(tipBottom.map { "\($0)" } ?? "-") → top \(tipTop.map { "\($0)" } ?? "-")")
    expect(tipBottom == p(230, 150) && tipTop == p(230, 50), "pins moved across")
    doc.setPortSide(port1, .left)
    expect(doc.port(port1)!.port.side == .top, "Left refused on a horizontal line")
    shot("top")

    print("== wire count 3 with B3 ticked")
    let countMessage = doc.setBusWireCount(busID, 3)
    print("  message: \(countMessage ?? "nil")")
    expect(countMessage == "B3 is used by a port; this bus needs at least 4 wires.", "minimum message")
    expect(wires().count == 8, "count unchanged")
    doc.selectedBusID = busID
    shot("count3")

    print("== T at 200, port at x 180 ticks B0-B1")
    doc.addBus(from: p(200, 100), to: p(200, 160))
    expect(doc.bus(busID)!.lines.count == 2, "T line joined")
    expect(doc.addPort(at: p(180, 99)) == nil, "port 2 placed")
    let port2 = doc.selectedPortID!
    expect(doc.setPortWires(port2, Set(wires()[0..<2].map(\.id))) == nil, "ticked")
    print("  \(describe(port2))")
    expect(doc.port(port2)!.port.start == 170, "shifted left to end at the T (extent 160..200)")
    _ = doc.setPortWires(port2, Set(wires()[0..<3].map(\.id)))
    let e2 = doc.bus(busID)!.extent(of: doc.port(port2)!.port)
    expect(!(e2.lowerBound < 200 && 200 < e2.upperBound), "never covers the T (\(Int(e2.lowerBound))..\(Int(e2.upperBound)))")
    _ = doc.setPortWires(port2, Set(wires()[0..<2].map(\.id)))
    shot("T")

    print("== refusals")
    let atT = doc.addPort(at: p(200, 100))
    print("  click the T: \(atT ?? "nil")")
    expect(atT == "A port can't go on the end of a bus line or on a joint.", "T refused")
    let atEnd = doc.addPort(at: p(102, 100))
    print("  click the end: \(atEnd ?? "nil")")
    expect(atEnd == "A port can't go on the end of a bus line or on a joint.", "end refused")
    let snapshot = doc.buses
    doc.addBus(from: p(260, 100), to: p(260, 40))
    print("  T into port 1: \(doc.editorMessage ?? "nil")")
    expect(doc.buses == snapshot, "a T inside a port is refused")
    shot("refusals")

    print(failures == 0 ? "ALL OK" : "\(failures) FAILED")
}
