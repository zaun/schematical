// Vintage tubes and their drivers in the app: the drawer groups, each
// symbol unlit and glowing (<out>/tubes.png), live circuits in the real
// simulation controller (numitron, VFD, neon lamp, HV5622 driving an IN-14,
// HV5812 driving an IV-11), the playing canvas (<out>/canvas.png) and the
// demo schematic saved as <out>/tubes.schematical.
//
//   scripts/harness/build.sh scripts/harness/tubes-app.swift /tmp/tubes-h && \
//     /tmp/tubes-h/H.app/Contents/MacOS/harness /tmp/tubes-out
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runTubes() }

@MainActor func runTubes() {
    let out = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/tubes-out"
    try? FileManager.default.createDirectory(atPath: out, withIntermediateDirectories: true)
    let catalog = Harness.catalog
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print("\(ok ? "PASS" : "FAIL"): \(what)")
        if !ok { failures += 1 }
    }

    // (1) The drawer.
    let groups = catalog.groups(matching: DrawerFilter())
    let vintage = groups.first { $0.category == "Vintage" }
    func names(_ sub: String) -> [String] {
        let found = sub == "Support ICs"
            ? vintage?.subcategories.first { $0.subcategory == sub }
            : vintage?.subcategories.first { $0.subcategory == "Display tubes" }?
                .subgroups.first { $0.subcategory == sub }
        return found?.allParts.map(\.name).sorted() ?? []
    }
    for sub in ["Nixie", "Numitron", "VFD", "Neon", "Support ICs"] { print("Vintage > \(sub): \(names(sub))") }
    expect(names("Numitron") == ["IV-16", "IV-9"], "Vintage > Display tubes > Numitron lists IV-9 and IV-16")
    expect(names("VFD") == ["IV-11", "IV-12", "IV-18", "IV-6"], "Vintage > Display tubes > VFD lists the four tubes")
    expect(names("Neon") == ["IN-3", "INS-1", "NE-2"], "Vintage > Display tubes > Neon lists the three lamps")
    expect(names("Nixie").count == 19, "Vintage > Display tubes > Nixie lists 19 tubes")
    expect(Set(names("Support ICs")) == ["SN74141N", "DM74141N", "K155ID1", "SN7490AN", "K155IE2", "SN7475N", "K155TM7"],
           "Vintage > Support ICs lists the driver, counter and latch")
    let logic = groups.first { $0.category == "Logic" }?.subcategories.first { $0.subcategory == "Registers" }
    let registers = Set(logic?.allParts.map(\.name) ?? [])
    expect(["HV5122PJ-G", "HV5222PG-G", "HV5622PG-G", "HV5812P-G", "MAX6921AWI+"].allSatisfy(registers.contains),
           "the modern drivers are under Logic > Registers")
    expect(!["HV5622PG-G", "MAX6921AWI+"].contains { n in names("Support ICs").contains(n) }, "and not under Vintage")

    // (2) Symbols, unlit and glowing.
    let doc = Harness.newDocument()
    func put(_ d: SchematicDocument, _ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
        guard let part = catalog.part(named: name) else { fatalError("no part \(name)") }
        d.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: x, y: y))
        return d.chips.last!
    }
    let shown = ["IV-9", "IV-11", "IV-18", "IN-19A", "IN-7", "IN-1", "B-5870", "INS-1"].map { put(doc, $0, 0, 0) }
    for chip in shown {
        let drawn = chip.layout.slots.count
        let pins = chip.part.pins.filter { !$0.isHidden && $0.role != .noConnect }.count
        expect(drawn >= pins, "\(chip.part.name): every connected pin has a slot (\(drawn) slots, \(pins) pins)")
        expect(chip.part.footprint != nil, "\(chip.part.name): has footprint \(chip.part.footprint?.id ?? "-")")
    }
    if let iv11 = shown.first(where: { $0.part.name == "IV-11" }) {
        let right = iv11.layout.slots.filter { $0.side == .right }.map(\.pin.name)
        expect(right == ["GRID", "F1", "F2"], "IV-11: grid, then the filament leads, on the right (\(right))")
    }
    @ViewBuilder func symbolView(_ chip: PlacedChip, lit: Bool) -> some View {
        if let nixie = chip.part.nixie {
            NixieSymbolView(chip: chip, isSelected: false, isHovered: false,
                            levels: lit ? nixie.glyphs.indices.map { $0 == 1 || nixie.glyphs.count == 1 ? 1.0 : 0 } : [])
        } else if case let .display(info) = chip.part.details {
            // "3": segments A B C D G of every digit.
            SevenSegmentSymbolView(chip: chip, isSelected: false, isHovered: false,
                                   levels: lit ? info.leds.map { ["A", "B", "C", "D", "G"].contains($0.segment) ? 1.0 : 0 } : [])
        }
    }
    var x: CGFloat = 10
    var offsets: [CGFloat] = []
    for chip in shown { offsets.append(x); x += chip.layout.size.width + 2 * ChipLayout.labelMargin + 20 }
    let tall = (shown.map(\.layout.size.height).max() ?? 300) + 20
    let view = ZStack(alignment: .topLeading) {
        ForEach(Array(shown.enumerated()), id: \.offset) { index, chip in
            symbolView(chip, lit: false).offset(x: offsets[index], y: 10)
            symbolView(chip, lit: true).offset(x: offsets[index], y: 10 + tall)
        }
    }
    .frame(width: x, height: 2 * tall + 20, alignment: .topLeading)
    Harness.render(view, size: CGSize(width: x, height: 2 * tall + 20), to: "\(out)/tubes.png")

    // (3) Live circuits, all in one document (saved for the real app).
    let d = Harness.newDocument()
    func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
        guard let slot = chip.layout.slots.first(where: { $0.pin.name == name }) else {
            fatalError("\(chip.part.name) has no drawn pin \(name)")
        }
        return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
    }
    func rail(_ name: String, _ x: CGFloat, _ y: CGFloat) -> LineAnchor {
        let chip = put(d, name, x, y)
        return .pin(PinRef(chipID: chip.id, index: chip.layout.slots[0].pin.index))
    }
    func connect(_ a: LineAnchor, _ b: LineAnchor) {
        if let message = d.connect(a, b) { print("FAIL wiring: \(message)"); failures += 1 }
    }
    // Numitron: common to 5 V, B and C to ground: "1".
    let iv9 = put(d, "IV-9", 300, 200)
    connect(rail("5V", 480, 120), pin(iv9, "COM"))
    for s in ["B", "C"] { connect(rail("GND", 120, s == "B" ? 160 : 220), pin(iv9, s)) }
    // Neon lamp on 170 V.
    let ne2 = put(d, "NE-2", 300, 480)
    connect(rail("170V", 480, 420), pin(ne2, "A"))
    connect(rail("GND", 120, 480), pin(ne2, "K"))
    // HV5622 shifting ones into an IN-14: numerals 1, 2 and 3 on its first three outputs.
    let hv = put(d, "HV5622PJ-G", 900, 500)
    let in14 = put(d, "IN-14", 1400, 300)
    let osc = put(d, "OSC-SIT1534-2012", 620, 300)
    connect(rail("3V3", 620, 180), pin(osc, "VDD"))
    connect(rail("GND", 620, 420), pin(osc, "GND"))
    connect(pin(osc, "CLK OUT"), pin(hv, "CLK"))
    let v12 = rail("12V", 760, 120)
    for name in ["VDD", "LE", "BL", "POL", "DATA IN"] { connect(v12, pin(hv, name)) }
    connect(rail("GND", 760, 900), pin(hv, "VSS"))
    connect(rail("170V", 1560, 120), pin(in14, "A"))
    for n in 1...3 { connect(pin(hv, "HVOUT\(n)"), pin(in14, "K\(n)")) }
    // HV5812 shifting ones into an IV-11: segments A, B, C and the grid.
    let vf = put(d, "HV5812P-G", 900, 1300)
    let iv11 = put(d, "IV-11", 1400, 1300)
    connect(pin(osc, "CLK OUT"), pin(vf, "CLOCK"))
    let v5 = rail("5V", 760, 1100)
    for name in ["VDD", "STROBE", "DATA IN"] { connect(v5, pin(vf, name)) }
    let g = rail("GND", 760, 1600)
    for name in ["GND", "BLANKING"] { connect(g, pin(vf, name)) }
    for (n, s) in ["A", "B", "C", "GRID"].enumerated() { connect(pin(vf, "HVOUT\(n + 1)"), pin(iv11, s)) }

    do { try Harness.save(d, to: "\(out)/tubes.schematical") } catch { expect(false, "save: \(error)") }
    let sim = Harness.simulation
    sim.start(document: d)
    RunLoop.main.run(until: Date().addingTimeInterval(1.5))
    func lights(_ chip: PlacedChip) -> String {
        (sim.lights[chip.id] ?? []).map { $0 > 0.5 ? "1" : "0" }.joined()
    }
    if let message = sim.errorMessage { expect(false, "the run stopped: \(message)") }
    expect(lights(iv9) == "01100000", "IV-9 shows B and C (\(lights(iv9)))")
    expect(lights(ne2) == "1", "NE-2 glows on 170 V (\(lights(ne2)))")
    expect(lights(in14).hasPrefix("0111000000"), "HV5622 lights 1, 2 and 3 of the IN-14 (\(lights(in14)))")
    expect(lights(iv11) == "11100000", "HV5812 lights A, B and C of the IV-11 (\(lights(iv11)))")
    Harness.renderCanvas(d, to: "\(out)/canvas.png", size: CGSize(width: 1700, height: 1750))
    sim.stop()

    print(failures == 0 ? "tubes-app: all checks passed" : "FAIL: \(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
