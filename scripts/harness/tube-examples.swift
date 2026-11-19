// Builds the three vintage-tube examples: the schematic (parts, wires,
// notes), the board (footprints placed, autorouted) and a check that each
// runs in the simulation. Writes <out>/<name>.schematical with
// <name>-schematic.png and <name>-pcb.png beside it.
//
//   scripts/harness/build.sh scripts/harness/tube-examples.swift /tmp/ex-h && \
//     /tmp/ex-h/H.app/Contents/MacOS/harness /tmp/ex-out
//
// Copy the .schematical files into examples/ when they look right.
import AppKit
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runExamples() }

@MainActor final class Build {
    let d = Harness.newDocument()
    let name: String
    var failures = 0

    init(_ name: String) { self.name = name }

    func fail(_ what: String) {
        print("FAIL \(name): \(what)")
        failures += 1
    }

    @discardableResult func put(_ part: String, _ x: CGFloat, _ y: CGFloat) -> UUID {
        guard let found = Harness.catalog.part(named: part) else { fatalError("no part \(part)") }
        d.placeChip(part: found, symbol: symbol(for: found), centeredAt: CGPoint(x: x, y: y))
        return d.chips.last!.id
    }

    func chip(_ id: UUID) -> PlacedChip { d.chips.first { $0.id == id }! }

    /// A pin by name, or by designator as "#7".
    func pin(_ id: UUID, _ name: String) -> LineAnchor {
        let c = chip(id)
        let slot = name.hasPrefix("#")
            ? c.layout.slots.first { $0.pin.designator == String(name.dropFirst()) }
            : c.layout.slots.first { $0.pin.name == name }
        guard let slot else { fatalError("\(c.part.name) has no drawn pin \(name)") }
        return .pin(PinRef(chipID: id, index: slot.pin.index))
    }

    func tip(_ anchor: LineAnchor) -> CGPoint { d.anchorTip(anchor)! }
    func tip(_ id: UUID, _ name: String) -> CGPoint { tip(pin(id, name)) }

    /// A wire through `via`; every leg must be horizontal or vertical.
    func wire(_ a: LineAnchor, _ b: LineAnchor, _ via: [CGPoint] = []) {
        let path = [tip(a)] + via + [tip(b)]
        for (p, q) in zip(path, path.dropFirst()) where p.x != q.x && p.y != q.y {
            fail("slanted leg \(p) -> \(q)")
        }
        if let refusal = d.connect(a, b, path: path) { fail("connect: \(refusal)") }
    }

    /// Out to the channel at `x`, along it, and in.
    func z(_ a: LineAnchor, _ b: LineAnchor, x: CGFloat) {
        let p = tip(a), q = tip(b)
        wire(a, b, p.y == q.y ? [] : [CGPoint(x: x, y: p.y), CGPoint(x: x, y: q.y)])
    }

    /// A supply symbol beside `pins` of one chip (all on one side), wired
    /// with one riser `dx` away from the pin tips. 5 V style rails sit
    /// above the pins, GND below.
    func rail(_ id: UUID, _ pins: [String], _ supply: String, dx: CGFloat) {
        let ground = supply == "GND"
        // Top to bottom. A ground symbol hangs below the lowest pin; a rail sits above the highest.
        let tips = pins.map { (pin(id, $0), tip(id, $0)) }.sorted { $0.1.y < $1.1.y }
        guard let highest = tips.first, let lowest = tips.last else { return }
        let x = highest.1.x + dx
        // The symbol's pin tip is 20 from its centre.
        let symbolTipY = ground ? lowest.1.y + 20 : highest.1.y - 20
        let symbol = put(supply, x, ground ? symbolTipY + 20 : symbolTipY - 20)
        let end = LineAnchor.pin(PinRef(chipID: symbol, index: chip(symbol).layout.slots[0].pin.index))
        // The pin farthest from the symbol first: its riser passes the others, which tap it.
        let first = ground ? highest : lowest
        wire(first.0, end, [CGPoint(x: x, y: first.1.y)])
        for other in tips where other.1 != first.1 {
            wire(other.0, .point(CGPoint(x: x, y: other.1.y)))
        }
    }

    func note(_ text: String, _ x: CGFloat, _ y: CGFloat, size: Int = 12) {
        let id = d.addNote(at: CGPoint(x: x, y: y))
        d.setNoteText(id, text)
        d.setNoteSize(id, size)
        d.setNoteFramed(id, true)
        d.editingNoteID = nil
    }

    func ohms(_ id: UUID, _ value: Double) { d.setPartOhms(id, value) }

    /// The footprint's centre (mm) and rotation (clockwise degrees).
    func place(_ id: UUID, _ x: Double, _ y: Double, rotation: Int = 0) {
        for _ in 0..<3 where chip(id).board?.rotation != rotation { d.rotateFootprint(id) }
        d.moveFootprint(id, to: CGPoint(x: x, y: y))
    }

    /// Keeps a mark that would sit on a pad off the silkscreen (it stays on the document layer).
    func offSilk(pin1 a: UUID, reference b: UUID) {
        d.setFootprintMarks(a, pin1: .document)
        d.setFootprintMarks(b, reference: .document)
    }

    func boardText(_ text: String, _ x: Double, _ y: Double, size: Int = 12) {
        let id = d.addText(at: CGPoint(x: x, y: y))
        d.setText(id, text)
        d.setTextSize(id, size)
    }

    func autoroute() {
        let suite = "tube-examples-\(UUID().uuidString)"
        let defaults = UserDefaults(suiteName: suite)!
        let model = AutorouteModel(document: d, defaults: defaults)
        let env = ProcessInfo.processInfo.environment
        model.options.strategy = RoutingOptions.Strategy(rawValue: env["STRATEGY"] ?? "") ?? .thorough
        model.options.angle = RoutingOptions.Angle(rawValue: env["ANGLE"] ?? "") ?? .octilinear
        if let grid = env["GRID"].flatMap(Double.init) { model.options.grid = grid }
        if let clearance = env["CLEARANCE"].flatMap(Double.init) { model.options.clearanceOverride = clearance }
        model.refreshSummary()
        guard model.route() else { fail("the autorouter did not start"); return }
        let deadline = Date().addingTimeInterval(600)
        while Date() < deadline {
            if case .finished = model.session.state { break }
            RunLoop.main.run(until: Date().addingTimeInterval(0.2))
        }
        guard case .finished = model.session.state else { fail("the autorouter did not finish"); return }
        for row in model.results { fail("unrouted \(row.name): \(row.count) (\(row.reason))") }
        defaults.removePersistentDomain(forName: suite)
        print("\(name): \(d.traces.count) traces, \(d.vias.count) vias")
    }

    func check() {
        let issues = d.designRuleCheck(rules: d.board.rules)
        for issue in issues {
            print("  DRC \(issue.kind) \(issue.reference): \(issue.issue)")
            guard issue.kind == .short else { continue }
            for case let .trace(id) in issue.items {
                if let t = d.traces.first(where: { $0.id == id }) {
                    print("      trace \(t.layer) " + t.points.map { "(\(String(format: "%.2f", $0.x)),\(String(format: "%.2f", $0.y)))" }.joined(separator: " "))
                }
            }
        }
        if !issues.isEmpty { fail("\(issues.count) DRC issues") }
        for c in d.chips where c.footprint != nil {
            guard let b = c.boardBounds else { continue }
            if b.minX < 0 || b.minY < 0 || b.maxX > d.board.width || b.maxY > d.board.height {
                fail("\(c.reference) is off the board")
            }
        }
    }

    /// Runs the simulation for `seconds` and returns each sampled lights string of `id`.
    func lights(of id: UUID, seconds: Double, samples: Int) -> [String] {
        let sim = Harness.simulation
        sim.start(document: d)
        var seen: [String] = []
        for _ in 0..<samples {
            RunLoop.main.run(until: Date().addingTimeInterval(seconds / Double(samples)))
            seen.append((sim.lights[id] ?? []).map { $0 > 0.5 ? "1" : "0" }.joined())
        }
        if let message = sim.errorMessage { fail("the run stopped: \(message)") }
        sim.stop()
        return seen
    }

    func save(to out: String) {
        do { try Harness.save(d, to: "\(out)/\(name).schematical") } catch { fail("save: \(error)") }
        for view in [ImageExportView.schematic, .pcb] {
            var options = ImageExportOptions()
            options.view = view
            options.format = .png
            options.background = .white
            options.scale = 2
            if let data = ImageExport.data(for: d, options.normalized()) {
                try? data.write(to: URL(fileURLWithPath: "\(out)/\(name)-\(view.rawValue).png"))
            } else {
                fail("no \(view.rawValue) image")
            }
        }
    }
}

private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

// MARK: - Nixie seconds clock

/// 32.768 kHz / 2^14 / 2 = 1 Hz into two 7490s (units, and tens reset at 6),
/// each decoded by a 74141 onto an IN-14.
@MainActor func nixieClock(_ out: String) -> Int {
    let b = Build("nixie-clock")
    let top: CGFloat = 200, low: CGFloat = 640
    let j1 = b.put("GENERIC-HEADER-1X03", -40, top - 300)
    let x1 = b.put("ECS-327SMO-TR", 0, top - 70)
    let u1 = b.put("CD74HC4020E", 280, top)
    let u2 = b.put("SN74HC74N", 640, top + 60)
    let u3 = b.put("SN7490AN", 900, top)
    let u5 = b.put("SN74141N", 1220, top - 30)
    let v1 = b.put("IN-14", 1500, top - 30)
    let r1 = b.put("GENERIC-RESISTOR", 1660, top - 120)
    let u4 = b.put("SN7490AN", 900, low)
    let u6 = b.put("SN74141N", 1220, low - 30)
    let v2 = b.put("IN-14", 1500, low - 30)
    let r2 = b.put("GENERIC-RESISTOR", 1660, low - 120)
    b.ohms(r1, 10_000)
    b.ohms(r2, 10_000)

    // Power in.
    b.rail(j1, ["1"], "5V", dx: -40)
    b.rail(j1, ["3"], "GND", dx: -40)
    let hv = b.put("170V", -170, top - 320)
    b.wire(b.pin(j1, "2"), .pin(PinRef(chipID: hv, index: b.chip(hv).layout.slots[0].pin.index)),
           [p(-170, b.tip(j1, "2").y)])

    // Timebase.
    b.rail(x1, ["VCC", "TRI-STATE"], "5V", dx: -30)
    b.rail(x1, ["Ground"], "GND", dx: 20)
    b.wire(b.pin(x1, "Output"), b.pin(u1, "CP"))
    b.rail(u1, ["VCC"], "5V", dx: -30)
    b.rail(u1, ["MR"], "GND", dx: -30)
    b.rail(u1, ["GND"], "GND", dx: 20)
    b.z(b.pin(u1, "Q14"), b.pin(u2, "1CLK"), x: 470)

    // Divide by two: 1D takes 1QBAR. The second flip-flop is unused: inputs tied high.
    b.rail(u2, ["VCC", "1CLR", "1PRE", "2CLK", "2CLR", "2D", "2PRE"], "5V", dx: -30)
    let qbar = b.tip(u2, "1QBAR"), dIn = b.tip(u2, "1D")
    b.wire(b.pin(u2, "1QBAR"), b.pin(u2, "1D"),
           [p(qbar.x + 60, qbar.y), p(qbar.x + 60, top + 250), p(dIn.x - 60, top + 250), p(dIn.x - 60, dIn.y)])
    b.rail(u2, ["GND"], "GND", dx: 20)
    b.z(b.pin(u2, "1Q"), b.pin(u3, "CKA"), x: 790)

    // One digit: a 7490, its 74141 and tube.
    func digit(_ counter: UUID, _ driver: UUID, _ tube: UUID, _ resistor: UUID, bits: [String]) {
        b.rail(counter, ["VCC"], "5V", dx: -30)
        b.rail(counter, ["GND"], "GND", dx: 20)
        b.rail(driver, ["VCC"], "5V", dx: -30)
        b.rail(driver, ["GND"], "GND", dx: 20)
        for (bit, input) in zip(bits, ["A", "B", "C", "D"]) {
            b.wire(b.pin(counter, bit), b.pin(driver, input))
        }
        for n in 0...9 { b.wire(b.pin(driver, "\(n)"), b.pin(tube, "K\(n)")) }
        // QA back to CKB, over the top of the counter.
        let qa = b.tip(counter, "QA"), ckb = b.tip(counter, "CKB")
        let over = b.chip(counter).origin.y - 40
        b.wire(b.pin(counter, "CKB"), .point(p(qa.x + 40, qa.y)),
               [p(ckb.x - 70, ckb.y), p(ckb.x - 70, over), p(qa.x + 40, over)])
        // The anode resistor up to 170 V.
        let anode = b.tip(tube, "A"), r = b.tip(resistor, "1")
        b.wire(b.pin(tube, "A"), b.pin(resistor, "1"), [p(r.x - 20, anode.y), p(r.x - 20, r.y)])
        b.rail(resistor, ["2"], "170V", dx: 30)
    }
    digit(u3, u5, v1, r1, bits: ["QA", "QB", "QC", "QD"])
    b.rail(u3, ["R0(1)", "R0(2)", "R9(1)", "R9(2)"], "GND", dx: -30)
    digit(u4, u6, v2, r2, bits: ["QA", "QB", "QC"])
    b.rail(u6, ["D"], "GND", dx: -30)
    b.rail(u4, ["R9(1)", "R9(2)"], "GND", dx: -30)

    // Tens: clocked by the units' QD; reset at 6 (QB and QC).
    let qd = b.tip(u3, "QD"), cka = b.tip(u4, "CKA")
    b.wire(b.pin(u4, "CKA"), .point(p(qd.x + 60, qd.y)),
           [p(cka.x - 40, cka.y), p(cka.x - 40, top + 190), p(qd.x + 60, top + 190)])
    for (bit, reset, drop, back) in [("QB", "R0(1)", CGFloat(200), CGFloat(110)), ("QC", "R0(2)", 220, 130)] {
        let q = b.tip(u4, bit), r = b.tip(u4, reset)
        b.wire(b.pin(u4, reset), .point(p(q.x + (bit == "QB" ? 80 : 100), q.y)),
               [p(r.x - back, r.y), p(r.x - back, low + drop), p(q.x + (bit == "QB" ? 80 : 100), low + drop)])
    }

    b.note("Nixie seconds clock\n\nG1 32.768 kHz / 16384 (U1) / 2 (U2) = 1 Hz.\nU3 counts units 0-9; its QD clocks U5, which counts tens\n0-5 (QB and QC reset it at 6). Each 74141 (U4, U6) pulls\none cathode of its IN-14 low: DS1 units, DS2 tens.\n\nP1: 1 = +5 V, 2 = +170 V, 3 = GND.\nR1, R2: 10 k anode resistors (about 2.5 mA).", 80, top - 420)

    // Board: tubes along the top, tens on the left.
    b.d.setBoardSize(width: 118, height: 72)
    b.place(v2, 38, 16)
    b.place(v1, 72, 16)
    b.place(r2, 20, 16, rotation: 90)
    b.place(r1, 90, 16, rotation: 90)
    b.place(u4, 14, 50)
    b.place(u6, 32, 50)
    b.place(u3, 52, 50)
    b.place(u5, 70, 50)
    b.place(u2, 88, 50)
    b.place(u1, 106, 50)
    b.place(x1, 106, 26)
    b.place(j1, 106, 8, rotation: 90)
    b.offSilk(pin1: x1, reference: j1)
    b.boardText("NIXIE SECONDS CLOCK", 59, 67, size: 60)
    b.autoroute()
    b.check()

    let seen = b.lights(of: v1, seconds: 3.6, samples: 12)
    print("nixie-clock units: \(seen)")
    if Set(seen).count < 3 { b.fail("the units tube did not count (\(Set(seen).count) states)") }
    b.save(to: out)
    return b.failures
}

// MARK: - Numitron counter

/// 2 Hz into a 7490, decoded by a 7447 onto an IV-9.
@MainActor func numitronCounter(_ out: String) -> Int {
    let b = Build("numitron-counter")
    let top: CGFloat = 200
    let j1 = b.put("GENERIC-HEADER-1X02", -40, top - 290)
    let x1 = b.put("ECS-327SMO-TR", 0, top - 70)
    let u1 = b.put("CD74HC4020E", 280, top)
    let u2 = b.put("SN7490AN", 600, top + 150)
    let u3 = b.put("SN7447AN", 920, top + 100)
    let ds = b.put("IV-9", 1220, top + 100)
    let d1 = b.put("1N4001-E3/54", 1400, top - 60)
    let d2 = b.put("1N4001-E3/54", 1500, top - 60)

    b.rail(j1, ["1"], "5V", dx: -40)
    b.rail(j1, ["2"], "GND", dx: -40)
    b.rail(x1, ["VCC", "TRI-STATE"], "5V", dx: -30)
    b.rail(x1, ["Ground"], "GND", dx: 20)
    b.wire(b.pin(x1, "Output"), b.pin(u1, "CP"))
    b.rail(u1, ["VCC"], "5V", dx: -30)
    b.rail(u1, ["MR"], "GND", dx: -30)
    b.rail(u1, ["GND"], "GND", dx: 20)
    b.z(b.pin(u1, "Q14"), b.pin(u2, "CKA"), x: 430)

    b.rail(u2, ["VCC"], "5V", dx: -30)
    b.rail(u2, ["R0(1)", "R0(2)", "R9(1)", "R9(2)"], "GND", dx: -30)
    b.rail(u2, ["GND"], "GND", dx: 20)
    for (bit, input) in zip(["QA", "QB", "QC", "QD"], ["A", "B", "C", "D"]) {
        b.wire(b.pin(u2, bit), b.pin(u3, input))
    }
    let qa = b.tip(u2, "QA"), ckb = b.tip(u2, "CKB")
    let over = b.chip(u2).origin.y - 40
    b.wire(b.pin(u2, "CKB"), .point(p(qa.x + 40, qa.y)),
           [p(ckb.x - 70, ckb.y), p(ckb.x - 70, over), p(qa.x + 40, over)])

    // VCC, and the lamp test and blanking inputs held high.
    b.rail(u3, ["VCC", "LT", "RBI"], "5V", dx: -40)
    b.rail(u3, ["GND"], "GND", dx: 20)
    for (index, (segment, lead)) in zip(["a", "b", "c", "d", "e", "f", "g"], ["A", "B", "C", "D", "E", "F", "G"]).enumerated() {
        b.z(b.pin(u3, segment), b.pin(ds, lead), x: 1040 + CGFloat(index == 0 ? 20 : 0))
    }
    // Two diodes drop the 5 V rail to about 3.5 V for the filaments.
    b.rail(d1, ["A"], "5V", dx: -30)
    b.wire(b.pin(d1, "K"), b.pin(d2, "A"))
    let com = b.tip(ds, "COM"), k = b.tip(d2, "K")
    b.wire(b.pin(d2, "K"), b.pin(ds, "COM"), [p(k.x + 30, k.y), p(k.x + 30, com.y)])

    b.note("Numitron counter\n\nG1 32.768 kHz / 16384 (U1) = 2 Hz into the 7490 (U2).\nThe 7447 (U3) sinks the IV-9's segments; the common\nlead goes to +5 V through D1 and D2, which drop the rail\nto about 3.5 V (segments 3.15 V nominal, 4.5 V max).\n\nP1: 1 = +5 V, 2 = GND.", 420, top - 330)

    b.d.setBoardSize(width: 76, height: 56)
    b.place(ds, 30, 12)
    b.place(d1, 48, 8)
    b.place(d2, 48, 16)
    b.place(u3, 30, 38)
    b.place(u2, 48, 38)
    b.place(u1, 66, 38)
    b.place(x1, 66, 14)
    b.place(j1, 10, 12)
    b.offSilk(pin1: x1, reference: j1)
    b.boardText("NUMITRON COUNTER", 38, 52, size: 60)
    return finish(b, ds, out)
}

// MARK: - VFD test pattern

/// An HV5812 with its data output inverted back into its data input: the 20
/// stages fill with ones, then with zeros, at 8 Hz. Stages 1-8 are the
/// IV-18's segments, 9-16 its digit grids.
@MainActor func vfdPattern(_ out: String) -> Int {
    let b = Build("vfd-test-pattern")
    let hx: CGFloat = 760, hy: CGFloat = 300
    let tx = hx + 70 + 120 + 170, ty = hy - 90
    let p1 = b.put("GENERIC-HEADER-1X03", -120, hy - 480)
    let x1 = b.put("ECS-327SMO-TR", -80, hy - 260)
    let u1 = b.put("CD74HC4020E", 200, hy - 190)
    let u2 = b.put("SN74HC04N", 500, hy + 40)
    let u3 = b.put("HV5812P-G", hx, hy)
    let ds = b.put("IV-18", tx, ty)

    b.rail(p1, ["1"], "5V", dx: -40)
    b.rail(p1, ["3"], "GND", dx: -40)
    let rail24 = b.put("24V", -250, hy - 500)
    b.wire(b.pin(p1, "2"), .pin(PinRef(chipID: rail24, index: b.chip(rail24).layout.slots[0].pin.index)),
           [p(-250, b.tip(p1, "2").y)])

    b.rail(x1, ["VCC", "TRI-STATE"], "5V", dx: -30)
    b.rail(x1, ["Ground"], "GND", dx: 20)
    b.wire(b.pin(x1, "Output"), b.pin(u1, "CP"))
    b.rail(u1, ["VCC"], "5V", dx: -30)
    b.rail(u1, ["MR"], "GND", dx: -30)
    b.rail(u1, ["GND"], "GND", dx: 20)
    b.z(b.pin(u1, "Q12"), b.pin(u3, "CLOCK"), x: 670)

    // The driver's supplies and controls.
    b.rail(u3, ["VDD", "STROBE"], "5V", dx: -100)
    b.rail(u3, ["VPP"], "24V", dx: -50)
    b.rail(u3, ["BLANKING"], "GND", dx: -140)
    b.rail(u3, ["GND"], "GND", dx: 20)

    // Data out, inverted, back to data in.
    b.rail(u2, ["VCC"], "5V", dx: -30)
    b.rail(u2, ["2A", "3A", "4A", "5A", "6A"], "GND", dx: -30)
    b.rail(u2, ["GND"], "GND", dx: 20)
    b.z(b.pin(u2, "1Y"), b.pin(u3, "DATA IN"), x: 620)
    let dout = b.tip(u3, "DATA OUT"), a1 = b.tip(u2, "1A")
    b.wire(b.pin(u3, "DATA OUT"), b.pin(u2, "1A"),
           [p(dout.x + 20, dout.y), p(dout.x + 20, hy - 340), p(a1.x - 70, hy - 340), p(a1.x - 70, a1.y)])

    // Segments straight across; grids around the bottom of the tube.
    for (n, segment) in ["A", "B", "C", "D", "E", "F", "G", "DP"].enumerated() {
        b.wire(b.pin(u3, "HVOUT\(n + 1)"), b.pin(ds, segment))
    }
    for g in 1...8 {
        let from = b.pin(u3, "HVOUT\(g + 8)"), to = b.pin(ds, "G\(g)")
        let a = b.tip(from), z = b.tip(to)
        let left = a.x + 110 - CGFloat(g) * 10, lane = hy + 50 + CGFloat(g) * 10, right = z.x + 80 + CGFloat(g) * 10
        b.wire(from, to, [p(left, a.y), p(left, lane), p(right, lane), p(right, z.y)])
    }
    // The filament across 5 V.
    let f1 = b.tip(ds, "F1"), f2 = b.tip(ds, "F2")
    let plus = b.put("5V", f1.x + 40, f1.y - 20)
    b.wire(b.pin(ds, "F1"), .pin(PinRef(chipID: plus, index: b.chip(plus).layout.slots[0].pin.index)))
    let minus = b.put("GND", f2.x + 40, f2.y + 40)
    b.wire(b.pin(ds, "F2"), .pin(PinRef(chipID: minus, index: b.chip(minus).layout.slots[0].pin.index)),
           [p(f2.x + 40, f2.y)])

    b.note("VFD test pattern\n\nG1 32.768 kHz / 4096 (U1) = 8 Hz clocks the HV5812 (U3).\nIts data output goes back to its data input through an\ninverter (U2), so the 20 stages fill with ones and then\nwith zeros: the IV-18's segments light one by one, then\nits digits (grids) from the right, and then go out again.\n\nP1: 1 = +5 V, 2 = +24 V (anodes and grids), 3 = GND.\nThe filament runs from 5 V (85 mA).", 420, hy - 560)

    b.d.setBoardSize(width: 90, height: 62)
    b.place(ds, 34, 14)
    b.place(u3, 34, 38, rotation: 90)
    b.place(u2, 66, 14, rotation: 90)
    b.place(u1, 76, 38)
    b.place(x1, 62, 30)
    b.place(p1, 8, 14, rotation: 90)
    b.offSilk(pin1: x1, reference: p1)
    b.boardText("VFD TEST PATTERN", 40, 57, size: 60)
    return finish(b, ds, out)
}

@MainActor func finish(_ b: Build, _ display: UUID, _ out: String) -> Int {
    b.autoroute()
    b.check()
    let seen = b.lights(of: display, seconds: 3.0, samples: 12)
    print("\(b.name): \(seen)")
    if Set(seen).count < 3 { b.fail("the display did not change (\(Set(seen).count) states)") }
    b.save(to: out)
    return b.failures
}

@MainActor func runExamples() {
    let out = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/ex-out"
    let only = CommandLine.arguments.count > 2 ? CommandLine.arguments[2] : ""
    try? FileManager.default.createDirectory(atPath: out, withIntermediateDirectories: true)
    var failures = 0
    if only.isEmpty || only == "nixie" { failures += nixieClock(out) }
    if only.isEmpty || only == "numitron" { failures += numitronCounter(out) }
    if only.isEmpty || only == "vfd" { failures += vfdPattern(out) }
    print(failures == 0 ? "tube-examples: all built" : "FAIL: \(failures)")
    exit(failures == 0 ? 0 : 1)
}
