// plan-13-C Phase 1: bus drags with ports. Each drag runs frame by frame
// against the press-time bus, as the canvas does, then settles on release.
//
//   scripts/harness/build.sh scripts/harness/bus-drag-ports.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness [image dir]
import CoreGraphics
import Foundation

MainActor.assumeIsolated { runDragPortChecks() }

@MainActor func runDragPortChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p13img"
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print(ok ? "  ok: \(what)" : "  FAIL: \(what)")
        if !ok { failures += 1 }
    }
    func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    func fmt(_ q: CGPoint) -> String { "(\(Int(q.x)),\(Int(q.y)))" }

    func dragLine(_ doc: SchematicDocument, at point: CGPoint, by delta: CGSize) {
        if BusDrag.line(doc, at: point, by: delta) == nil { print("  no line"); failures += 1 }
    }
    func dragEnd(_ doc: SchematicDocument, at point: CGPoint, to target: CGPoint) {
        if BusDrag.end(doc, at: point, to: target) == nil { print("  no end"); failures += 1 }
    }
    /// One jump: the whole drag in a single frame (a fast mouse).
    func jumpEnd(_ doc: SchematicDocument, at point: CGPoint, to target: CGPoint) {
        if BusDrag.end(doc, at: point, to: target, frame: 10_000) == nil { print("  no end"); failures += 1 }
    }
    func line(_ proof: BusProof, _ id: UUID) -> BusLine { proof.document.bus(proof.busID)!.line(id)! }
    func port(_ proof: BusProof, _ id: UUID) -> BusPort { proof.document.port(id)!.port }
    func tip(_ proof: BusProof, _ id: UUID) -> CGPoint {
        let (bus, port) = proof.document.port(id)!
        return bus.pinTip(port, wire: bus.wires[0].id)!
    }
    /// The end of the wire on a port's B0 pin.
    func wireEnd(_ proof: BusProof, _ id: UUID) -> CGPoint? {
        let doc = proof.document
        let ref = BusPinRef(portID: id, wireID: doc.bus(proof.busID)!.wires[0].id)
        guard let wire = doc.wires.first(where: { $0.start == .busPin(ref) || $0.end == .busPin(ref) }) else { return nil }
        let path = doc.path(forWire: wire.id)
        return wire.start == .busPin(ref) ? path.first : path.last
    }
    let size = CGSize(width: 720, height: 420)
    let origin = p(-20, 40)

    do {
        print("== drag the horizontal line down 50")
        let proof = try BusProof.build()
        let doc = proof.document
        let start2 = port(proof, proof.port2).start
        dragLine(doc, at: p(320, 100), by: CGSize(width: 0, height: 50))
        let h = line(proof, proof.horizontalLine), v = line(proof, proof.verticalLine)
        print("  horizontal \(fmt(h.a))-\(fmt(h.b)), vertical \(fmt(v.a))-\(fmt(v.b)); port 1 tip \(fmt(tip(proof, proof.port1)))")
        expect(h.a.y == 150, "line at y 150")
        expect(tip(proof, proof.port1) == p(260, 200), "port 1 rode along (tip 260,200)")
        expect(wireEnd(proof, proof.port1) == tip(proof, proof.port1), "its wire follows")
        expect(min(v.a.y, v.b.y) == 150 && port(proof, proof.port2).start == start2, "vertical line shorter, port 2 not moved")
        Harness.renderCanvas(doc, to: "\(images)/c1-1-down50.png", size: size, origin: origin)

        print("== drag it further, past port 2")
        dragLine(doc, at: p(320, 150), by: CGSize(width: 0, height: 120))
        let h2 = line(proof, proof.horizontalLine), v2 = line(proof, proof.verticalLine)
        print("  horizontal at y \(Int(h2.a.y)), vertical \(fmt(v2.a))-\(fmt(v2.b)); port 2 extent \(doc.bus(proof.busID)!.extent(of: port(proof, proof.port2)))")
        expect(h2.a.y == 210, "stops at y 210, where the T meets port 2's top edge (extent 210..230)")
        Harness.renderCanvas(doc, to: "\(images)/c1-2-stopped-at-port2.png", size: size, origin: origin)
    } catch {
        print("FAIL: \(error)"); failures += 1
    }

    do {
        print("== drag the T end toward port 1")
        let proof = try BusProof.build()
        dragEnd(proof.document, at: p(380, 100), to: p(200, 100))
        let v = line(proof, proof.verticalLine)
        print("  vertical \(fmt(v.a))-\(fmt(v.b)); port 1 extent \(proof.document.bus(proof.busID)!.extent(of: port(proof, proof.port1)))")
        expect(v.a.x == 270, "stops at x 270, port 1's right edge")
        expect(wireEnd(proof, proof.port2) == tip(proof, proof.port2), "port 2 rode along with its line; its wire follows")
        Harness.renderCanvas(proof.document, to: "\(images)/c1-3-tee-at-port1.png", size: size, origin: origin)

        print("== the same drag in one frame (a fast mouse) can't jump over port 1")
        let fast = try BusProof.build()
        jumpEnd(fast.document, at: p(380, 100), to: p(200, 100))
        expect(line(fast, fast.verticalLine).a.x == 270, "still stops at x 270")
    } catch {
        print("FAIL: \(error)"); failures += 1
    }

    do {
        print("== drag the horizontal free end inward")
        let proof = try BusProof.build()
        dragEnd(proof.document, at: p(200, 100), to: p(400, 100))
        let h = line(proof, proof.horizontalLine)
        print("  horizontal \(fmt(h.a))-\(fmt(h.b))")
        expect(min(h.a.x, h.b.x) == 250, "stops at x 250, port 1's left edge")
        Harness.renderCanvas(proof.document, to: "\(images)/c1-4-end-at-port1.png", size: size, origin: origin)
    } catch {
        print("FAIL: \(error)"); failures += 1
    }

    print(failures == 0 ? "ALL OK" : "\(failures) FAILED")
}
