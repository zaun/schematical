// plan-13-A model check: walks the Phase 2 steps through the calls the
// canvas gestures make (a drag is the frames' BusEdit + setBusLines calls,
// always against the bus as it was when the drag started).
//
//   scripts/harness/build.sh scripts/harness/bus-lines.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runChecks() }

@MainActor func runChecks() {
    let doc = SchematicDocument()
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p13img"
    var failures = 0

    func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    func fmt(_ q: CGPoint) -> String { "(\(Int(q.x)),\(Int(q.y)))" }
    func shape(_ bus: Bus) -> String {
        bus.lines.map { "\(fmt($0.a))-\(fmt($0.b))" }.joined(separator: " ")
    }
    func dump(_ label: String) {
        print("== \(label)")
        for (i, bus) in doc.buses.enumerated() {
            let joints = bus.joints.map { joint -> String in
                switch joint {
                case .corner: "L\(fmt(joint.point))"
                case .tee: "T\(fmt(joint.point))"
                }
            }.sorted().joined(separator: " ")
            print("  bus \(i): \(shape(bus))  joints: \(joints)  wires: \(bus.wires.map(\.name).joined(separator: ","))")
        }
        if let message = doc.editorMessage { print("  message: \(message)") }
        // The real canvas after the step: images/a2-<step>.png.
        let step = label.split(separator: " ").first.map(String.init) ?? label
        Harness.renderCanvas(doc, to: "\(images)/a2-\(step).png",
                             size: CGSize(width: 480, height: 360), origin: p(-20, -20))
    }
    /// A Bus tool drag in progress: the preview the canvas would draw.
    func renderPreview(from start: CGPoint, to end: CGPoint, name: String) {
        let preview: BusPreview
        switch doc.busDraft(from: start, to: end) {
        case let .success(draft): preview = BusPreview(points: draft.path, refused: false)
        case .failure: preview = BusPreview(points: [start, end], refused: true)
        }
        let area = CGRect(x: -20, y: -20, width: 480, height: 360)
        Harness.render(
            BusLayer(buses: doc.buses, selectedBusID: doc.selectedBusID, hoveredBusID: nil, preview: preview, area: area)
                .background(Color.white),
            size: area.size, to: "\(images)/a2-\(name).png")
    }
    func expect(_ ok: Bool, _ what: String) {
        print(ok ? "  ok: \(what)" : "  FAIL: \(what)")
        if !ok { failures += 1 }
    }
    func hasLine(_ a: CGPoint, _ b: CGPoint, bus index: Int = 0) -> Bool {
        guard doc.buses.indices.contains(index) else { return false }
        return doc.buses[index].lines.contains { ($0.a == a && $0.b == b) || ($0.a == b && $0.b == a) }
    }

    /// A Pointer drag on a line: every frame from the press to `delta` in grid steps.
    func dragLine(at point: CGPoint, by delta: CGSize) {
        guard let hit = doc.busHit(at: point), let base = doc.bus(hit.bus) else {
            print("  no bus line at \(fmt(point))"); failures += 1; return
        }
        doc.selectedBusID = hit.bus
        let steps = Int(max(abs(delta.width), abs(delta.height)) / 10)
        for k in 0...max(steps, 1) {
            let t = CGFloat(k) / CGFloat(max(steps, 1))
            if let lines = BusEdit.moveLine(base, line: hit.line, by: CGSize(width: delta.width * t, height: delta.height * t)) {
                doc.setBusLines(hit.bus, lines)
            }
        }
    }
    /// A Pointer drag on a line end, frame by frame from the end to `target`.
    func dragEnd(at point: CGPoint, to target: CGPoint) {
        guard let hit = doc.busEndHit(at: point), let base = doc.bus(hit.bus) else {
            print("  no bus end at \(fmt(point))"); failures += 1; return
        }
        let steps = Int(max(abs(target.x - point.x), abs(target.y - point.y)) / 10)
        for k in 0...max(steps, 1) {
            let t = CGFloat(k) / CGFloat(max(steps, 1))
            let q = CGPoint(x: point.x + (target.x - point.x) * t, y: point.y + (target.y - point.y) * t)
            if let lines = BusEdit.moveEnd(base, line: hit.line, end: hit.end, to: q) {
                doc.setBusLines(hit.bus, lines)
            }
        }
    }

    // 1. An L.
    doc.addBus(from: p(100, 100), to: p(300, 200))
    dump("1 drag (100,100)->(300,200)")
    expect(doc.buses.count == 1 && doc.buses[0].lines.count == 2, "one bus, two lines")
    expect(hasLine(p(100, 100), p(300, 100)) && hasLine(p(300, 100), p(300, 200)), "L lines")
    expect(doc.buses[0].wires.map(\.name) == (0..<8).map { "B\($0)" }, "8 wires B0-B7")
    expect(doc.selectedBusID == doc.buses[0].id, "new bus selected")

    // 2. Pointer drags.
    dragLine(at: p(300, 150), by: CGSize(width: 40, height: 0))
    dump("2a drag vertical line right 40")
    expect(hasLine(p(100, 100), p(340, 100)) && hasLine(p(340, 100), p(340, 200)), "horizontal stretches to 340")
    dragEnd(at: p(100, 100), to: p(60, 100))
    dump("2b drag free end (100,100)->(60,100)")
    expect(hasLine(p(60, 100), p(340, 100)), "free end at 60")
    dragEnd(at: p(340, 100), to: p(360, 80))
    dump("2c drag corner (340,100)->(360,80)")
    expect(hasLine(p(60, 80), p(360, 80)) && hasLine(p(360, 80), p(360, 200)), "corner at (360,80)")

    // 3. Joining.
    renderPreview(from: p(200, 80), to: p(200, 20), name: "3-preview-T")
    doc.addBus(from: p(200, 80), to: p(200, 20))
    dump("3a drag (200,80)->(200,20)")
    expect(doc.buses.count == 1 && hasLine(p(200, 80), p(200, 20)), "T line added to the bus")
    expect(doc.buses[0].joints.contains { if case .tee = $0 { return $0.point == p(200, 80) }; return false }, "T at (200,80)")
    doc.addBus(from: p(200, 50), to: p(250, 50))
    dump("3b drag (200,50)->(250,50)")
    expect(hasLine(p(200, 50), p(250, 50)), "T on the x 200 line")
    let before = doc.buses[0].lines.count
    doc.addBus(from: p(60, 80), to: p(20, 80))
    dump("3c drag (60,80)->(20,80)")
    expect(doc.buses[0].lines.count == before && hasLine(p(20, 80), p(360, 80)), "extended, still one line")

    // 4. More drags.
    dragLine(at: p(100, 80), by: CGSize(width: 0, height: 20))
    dump("4a drag the y 80 line down 20")
    expect(hasLine(p(20, 100), p(360, 100)), "line now at y 100")
    expect(hasLine(p(200, 100), p(200, 20)), "T line at x 200 stretches")
    expect(hasLine(p(360, 100), p(360, 200)), "corner line stretches")
    expect(hasLine(p(200, 50), p(250, 50)), "T at y 50 stays")
    dragEnd(at: p(200, 50), to: p(200, 0))
    dump("4b drag the T end (200,50) up past (200,20)")
    expect(hasLine(p(200, 30), p(250, 30)), "stops at y 30")

    // 5. Refusals.
    let snapshot = doc.buses
    renderPreview(from: p(360, 100), to: p(420, 100), name: "5-preview-refused")
    doc.addBus(from: p(360, 100), to: p(420, 100))
    dump("5a drag from the corner")
    expect(doc.buses == snapshot && doc.editorMessage == "A bus line can only join another at a T or an L.", "corner refused")
    doc.addBus(from: p(100, 300), to: p(200, 300))
    doc.addBus(from: p(150, 300), to: p(150, 100))
    dump("5b second bus, then join the two")
    expect(doc.buses.count == 2 && doc.editorMessage == "Two buses can't be joined.", "two buses refused")

    // 6. Eraser.
    doc.erase(at: p(200, 70))
    dump("6a erase the x 200 line")
    expect(doc.buses.count == 3, "the T line at y 30 is its own bus")
    let lone = doc.buses.first { $0.lines.count == 1 && $0.lines[0].a.y == 30 }
    expect(lone != nil, "found the split bus")
    if let lone {
        doc.selectedBusID = lone.id
        expect(lone.wires.map(\.name) == (0..<8).map { "B\($0)" }, "its wires are B0-B7")
        let original = doc.buses[0].wires.map(\.id)
        expect(Set(lone.wires.map(\.id)).isDisjoint(with: original), "with fresh wire ids")
    }
    for point in [p(250, 30), p(150, 300), p(100, 100), p(360, 150)] { doc.erase(at: point) }
    dump("6b erase everything else")
    expect(doc.buses.isEmpty && doc.selectedBusID == nil, "no buses left, nothing selected")

    // Extra: the other L is tried, a straight-on meet is refused, a loop is allowed.
    doc.addBus(from: p(0, 0), to: p(100, 0))
    doc.addBus(from: p(100, 0), to: p(50, 60))
    dump("x1 L from a free end, first L would overlap")
    expect(hasLine(p(100, 0), p(100, 60)) && hasLine(p(100, 60), p(50, 60)), "picks the L that turns at the free end")
    doc.addBus(from: p(0, 0), to: p(0, 60))
    doc.addBus(from: p(0, 60), to: p(50, 60))
    dump("x2 close a loop")
    expect(doc.buses.count == 1 && doc.buses[0].isValid, "loop allowed")

    print(failures == 0 ? "ALL OK" : "\(failures) FAILED")
}
