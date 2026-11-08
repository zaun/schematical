// plan-25-B model check: a Line U and a 3-leg Bus drawn with Space locks,
// through the calls SchematicCanvasView makes (RouteDraft, anchorAxis,
// connect(path:), busDraft/addBus(candidates:)). Renders the previews and
// the placed result.
//
//   scripts/harness/build.sh scripts/harness/route-locks.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness /tmp/p25img
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runChecks() }

@MainActor func runChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p25img"
    var failures = 0
    func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    func expect(_ ok: Bool, _ what: String) {
        print(ok ? "  ok: \(what)" : "  FAIL: \(what)")
        if !ok { failures += 1 }
    }
    let area = CGRect(x: 0, y: 0, width: 480, height: 400)

    // 1. Line: from LED a's cathode (right side), out, down, and back into
    // LED b's cathode, with one Space lock.
    print("== 1 Line U")
    let doc = Harness.newDocument()
    guard let led = Harness.catalog.part(named: "GENERIC-LED") else { print("FAIL: no GENERIC-LED"); exit(1) }
    doc.placeChip(part: led, symbol: symbol(for: led), centeredAt: p(150, 100))
    doc.placeChip(part: led, symbol: symbol(for: led), centeredAt: p(150, 260))
    let a = doc.chips[0], b = doc.chips[1]
    func cathode(_ chip: PlacedChip) -> LineAnchor {
        let slot = chip.layout.slots.first { $0.pin.name == "K" }!
        return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
    }
    let from = cathode(a), to = cathode(b)
    let start = doc.anchorTip(from)!, end = doc.anchorTip(to)!
    var line = RouteDraft(start: start, startAxis: doc.anchorAxis(from))
    let corner = p(start.x + 60, end.y)
    expect(line.lock(to: corner, endAxis: nil), "Space locks the leg out and down")
    let preview = line.path(to: end, endAxis: doc.anchorAxis(to))
    let u = [start, p(start.x + 60, start.y), corner, end]
    expect(preview == u, "preview is the U \(preview)")
    Harness.render(
        WireLayer(routes: [], junctions: [], hoveredWireID: nil,
                  preview: ConnectionPreview(points: preview, target: end), area: area)
            .background(Color.white),
        size: area.size, to: "\(images)/line-preview.png")
    expect(doc.connect(from, to, path: preview) == nil, "connect accepted")
    let wire = doc.wires.first
    expect(wire?.interior == Array(u.dropFirst().dropLast()), "wire's bends are the U's two bends: \(wire?.interior ?? [])")
    Harness.renderCanvas(doc, to: "\(images)/line-placed.png", size: area.size, tool: .line)

    // 2. Bus: right, Space, down, Space, left, release.
    print("== 2 Bus, 3 legs")
    let busDoc = Harness.newDocument()
    var bus = RouteDraft(start: p(100, 100), startAxis: nil)
    expect(bus.lock(to: p(300, 100), endAxis: nil), "Space locks the first leg")
    expect(bus.preferredAxis == .vertical, "the next leg turns down")
    expect(bus.lock(to: p(300, 200), endAxis: nil), "Space locks the second leg")
    let candidates = bus.busCandidates(to: p(100, 200))
    if case let .success(draft) = busDoc.busDraft(candidates: candidates) {
        expect(draft.path == [p(100, 100), p(300, 100), p(300, 200), p(100, 200)], "preview path \(draft.path)")
        Harness.render(
            BusLayer(buses: busDoc.buses, selectedBusID: nil, hoveredBusID: nil,
                     preview: BusPreview(points: draft.path, refused: false), area: area)
                .background(Color.white),
            size: area.size, to: "\(images)/bus-preview.png")
    } else {
        expect(false, "bus preview refused")
    }

    // Esc once: the second lock comes loose; the live leg runs from the first lock.
    var unlocked = bus
    expect(unlocked.unlock(), "Esc drops the last lock")
    expect(unlocked.legs.count == 1, "one lock left")
    expect(unlocked.path(to: p(300, 200), endAxis: nil) == [p(100, 100), p(300, 100), p(300, 200)],
           "back to the one-lock preview")
    var none = RouteDraft(start: p(100, 100), startAxis: nil)
    expect(!none.unlock(), "Esc with no locks cancels (unlock is false)")

    let id = busDoc.addBus(candidates: candidates)
    let placed = id.flatMap { busDoc.bus($0) }
    expect(placed?.lines.count == 3, "3 lines")
    let corners = placed?.joints.filter { if case .corner = $0 { true } else { false } }.count
    expect(corners == 2, "2 L joints")
    Harness.renderCanvas(busDoc, to: "\(images)/bus-placed.png", size: area.size)

    print(failures == 0 ? "PASS" : "FAILURES: \(failures)")
    exit(failures == 0 ? 0 : 1)
}
