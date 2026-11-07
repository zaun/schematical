// plan-13-A Phase 3: the Bus panel. Each step makes the call the panel's
// field makes on commit, prints the stored result or the refusal message the
// panel shows, and renders the real sidebar.
//
//   scripts/harness/build.sh scripts/harness/bus-panel.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness [image dir]
import CoreGraphics
import Foundation

MainActor.assumeIsolated { runPanelChecks() }

@MainActor func runPanelChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p13img"
    let doc = Harness.newDocument()
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print(ok ? "  ok: \(what)" : "  FAIL: \(what)")
        if !ok { failures += 1 }
    }
    func names() -> [String] { doc.bus(doc.selectedBusID ?? UUID())?.wires.map(\.name) ?? [] }
    func shot(_ name: String) {
        print("  wires: \(names().joined(separator: ","))")
        Harness.renderSidebar(doc, to: "\(images)/a3-\(name).png", size: CGSize(width: 320, height: 700))
    }

    guard let id = doc.addBus(from: CGPoint(x: 100, y: 100), to: CGPoint(x: 300, y: 200)) else {
        print("FAIL: no bus"); return
    }
    print("== new bus")
    expect(names() == (0..<8).map { "B\($0)" }, "8 wires B0-B7")
    shot("1-new")

    print("== count 12")
    _ = doc.setBusWireCount(id, 12)
    expect(names() == (0..<12).map { "B\($0)" }, "B8-B11 added")
    shot("2-count12")

    print("== count 1")
    _ = doc.setBusWireCount(id, 1)
    expect(names() == ["B0", "B1"], "clamped to 2")
    shot("3-count1")
    _ = doc.setBusWireCount(id, 8)

    let wires = doc.bus(id)!.wires
    print("== rename B1 -> D1")
    let r1 = doc.renameBusWire(bus: id, wire: wires[1].id, to: "D1")
    expect(r1 == nil && names()[1] == "D1", "renamed")
    shot("4-D1")

    print("== rename B2 -> d1")
    let r2 = doc.renameBusWire(bus: id, wire: wires[2].id, to: "d1")
    print("  message: \(r2 ?? "nil")")
    expect(r2 == "d1 is already used on this bus." && names()[2] == "B2", "refused, B2 kept")

    print("== rename B3 -> ADDR0")
    let r3 = doc.renameBusWire(bus: id, wire: wires[3].id, to: "ADDR0")
    print("  message: \(r3 ?? "nil")")
    expect(r3 == "A wire name is at most 4 characters." && names()[3] == "B3", "refused, B3 kept")

    print("== other refusals")
    expect(doc.renameBusWire(bus: id, wire: wires[3].id, to: "  ") == "A wire name can't be empty.", "empty")
    expect(doc.renameBusWire(bus: id, wire: wires[3].id, to: "A B") == "A wire name can't contain spaces.", "spaces")

    print("== a joined bus: every line shows the same panel")
    doc.addBus(from: CGPoint(x: 200, y: 100), to: CGPoint(x: 200, y: 40))
    doc.selectedBusID = nil
    for point in [CGPoint(x: 150, y: 100), CGPoint(x: 300, y: 150), CGPoint(x: 200, y: 70)] {
        let hit = doc.busHit(at: point)
        expect(hit?.bus == id, "line at \(Int(point.x)),\(Int(point.y)) is bus \(id.uuidString.prefix(8))")
    }
    doc.selectedBusID = doc.busHit(at: CGPoint(x: 200, y: 70))?.bus
    shot("5-joined")
    Harness.renderCanvas(doc, to: "\(images)/a3-5-canvas.png", size: CGSize(width: 400, height: 260),
                         origin: CGPoint(x: 60, y: 20))

    print(failures == 0 ? "ALL OK" : "\(failures) FAILED")
}
