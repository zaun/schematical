// The nixie kind in the app: the Vintage > Nixie drawer group, each tube's
// layout, and its symbol unlit and glowing, rendered to <out>/nixie.png.
//
//   scripts/harness/build.sh scripts/harness/nixie-app.swift /tmp/nixie-h && \
//     /tmp/nixie-h/H.app/Contents/MacOS/harness /tmp/nixie-out
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runNixie() }

@MainActor func runNixie() {
    let out = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/nixie-out"
    let catalog = Harness.catalog
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print("\(ok ? "PASS" : "FAIL"): \(what)")
        if !ok { failures += 1 }
    }

    let groups = catalog.groups(matching: DrawerFilter())
    let nixies = groups.first { $0.category == "Vintage" }?.subcategories.first { $0.subcategory == "Nixie" }
    let names = nixies?.parts.map(\.name) ?? []
    print("Vintage > Nixie: \(names)")
    expect(names.count == 10 && names.contains("IN-14") && names.contains("IN-18"),
           "the drawer lists the ten tubes under Vintage > Nixie, with no function level")

    let doc = Harness.newDocument()
    var placed: [PlacedChip] = []
    for (index, name) in ["IN-14", "IN-18", "IN-12B", "IN-15B"].enumerated() {
        guard let part = catalog.part(named: name) else {
            expect(false, "part \(name) exists")
            continue
        }
        expect(symbol(for: part) == .nixie, "\(name) draws as a nixie")
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 150 + 220 * index, y: 200))
        if let chip = doc.chips.last {
            placed.append(chip)
            let info = part.nixie
            expect(chip.layout.slots.filter { $0.side == .left }.map(\.pin.index) == info?.cathodePins,
                   "\(name): the cathodes are on the left, in glyph order")
            expect(chip.layout.slots.count == part.pins.filter { !$0.isHidden }.count,
                   "\(name): every pin has a slot")
        }
    }
    if let part = catalog.part(named: "IN-14") {
        expect(part.voltageLabel == "170 V min", "IN-14's drawer voltage is its firing voltage")
        expect(part.footprint?.id == "NIXIE-IN-14" && part.footprint?.pads.count == 13, "IN-14's footprint has 13 pads")
    }

    func lit(_ chip: PlacedChip, _ glyph: String, _ level: Double = 1) -> [Double] {
        (chip.part.nixie?.glyphs ?? []).map { $0 == glyph ? level : 0 }
    }
    let view = ZStack(alignment: .topLeading) {
        ForEach(Array(placed.enumerated()), id: \.offset) { index, chip in
            NixieSymbolView(chip: chip, isSelected: false, isHovered: false)
                .offset(x: 10 + CGFloat(index) * 200, y: 10)
            NixieSymbolView(chip: chip, isSelected: false, isHovered: false,
                            levels: lit(chip, ["7", "3", "LDP", "Hz"][index]))
                .offset(x: 10 + CGFloat(index) * 200, y: 330)
        }
    }
    .frame(width: 820, height: 650, alignment: .topLeading)
    Harness.render(view, size: CGSize(width: 820, height: 650), to: "\(out)/nixie.png")
    print(failures == 0 ? "nixie-app: all checks passed" : "FAIL: \(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
