// plan-28-A/B: the painter pixel gate. Renders four images of the live
// canvas and the symbol views; moving drawing code onto SchematicPainter
// must leave every one byte-identical (cmp against a baseline taken first).
//
//   rm -rf /tmp/p28-h && scripts/harness/build.sh scripts/harness/painter-gate.swift /tmp/p28-h && \
//     /tmp/p28-h/H.app/Contents/MacOS/harness /tmp/p28-base
//
// Run from the repo root (it opens be-6502.schematical and test.schematical).
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runPainterGate() }

@MainActor func runPainterGate() {
    let out = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p28-gate"
    let catalog = Harness.catalog
    func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    @discardableResult
    func place(_ doc: SchematicDocument, _ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip? {
        guard let part = catalog.part(named: name) else {
            print("MISSING part \(name)")
            return nil
        }
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: p(x, y))
        return doc.chips.last
    }
    func pinRef(_ chip: PlacedChip, _ slotIndex: Int) -> PinRef {
        PinRef(chipID: chip.id, index: chip.layout.slots[slotIndex].pin.index)
    }

    // 1. Every symbol kind, plus selected/flipped/lever/gender variants,
    // two wires (one selected) and a bus with a port.
    let doc = Harness.newDocument()
    let nand = place(doc, "SN74HC00N", 120, 120)
    let nand2 = place(doc, "SN74HC00N", 420, 120)
    place(doc, "ECS-100A-010", 700, 120)
    place(doc, "GENERIC-LED", 900, 100)
    let led2 = place(doc, "GENERIC-LED", 900, 180)
    let dip = place(doc, "TE-GDS0204", 1100, 140)
    place(doc, "LTC-4727JR", 200, 420)
    place(doc, "NHD-0216K1Z-FL-YBW", 620, 420)
    place(doc, "PJ-002A", 1050, 420)
    place(doc, "GENERIC-HEADER-1X02", 140, 680)
    let female = place(doc, "GENERIC-HEADER-1X02", 260, 680)
    place(doc, "5V", 380, 660)
    place(doc, "GND", 460, 680)
    place(doc, "PULLUP-5V", 560, 660)
    place(doc, "PULLDOWN", 660, 680)
    if let led2 { doc.flipChip(led2.id) }
    if let nand2 { doc.flipChip(nand2.id) }
    if let dip { doc.toggleSwitch(dip.id, position: 1) }
    if let female { doc.setPartGender(female.id, "Female") }
    var wireIDs: [UUID] = []
    if let a = nand, let b = doc.chips.first(where: { $0.id == nand2?.id }) {
        for (i, j) in [(10, 3), (11, 4)] {
            let before = Set(doc.wires.map(\.id))
            if let why = doc.connect(.pin(pinRef(a, i)), .pin(pinRef(b, j))) { print("wire refused: \(why)") }
            wireIDs += doc.wires.map(\.id).filter { !before.contains($0) }
        }
    }
    if let busID = doc.addBus(from: p(800, 640), to: p(1100, 640)) {
        if doc.addPort(at: p(1000, 641)) == nil, let portID = doc.selectedPortID, let bus = doc.bus(busID) {
            _ = doc.setPortWires(portID, Set(bus.wires.prefix(4).map(\.id)))
        } else {
            print("port refused")
        }
    } else {
        print("bus refused")
    }
    if let nand2 { doc.select(.chip(nand2.id)) }
    if let first = wireIDs.first { doc.toggleSelection(.wire(first)) }
    print("symbols: \(doc.chips.count) parts, \(doc.wires.count) wires, \(doc.buses.count) buses")
    Harness.renderCanvas(doc, to: "\(out)/symbols.png", size: CGSize(width: 1300, height: 800))

    // 2. Lit LEDs and 7-segment, views instantiated directly.
    let litDoc = Harness.newDocument()
    if let led = place(litDoc, "GENERIC-LED", 100, 100),
       let seg = place(litDoc, "LTC-4727JR", 300, 300) {
        let count = seg.part.display?.leds.count ?? 0
        let lit = ZStack(alignment: .topLeading) {
            DiodeSymbolView(chip: led, isSelected: false, isHovered: false, level: 1)
                .offset(x: 10, y: 10)
            DiodeSymbolView(chip: led, isSelected: false, isHovered: false, level: 0.4)
                .offset(x: 10, y: 90)
            SevenSegmentSymbolView(chip: seg, isSelected: false, isHovered: false,
                                   levels: Array(repeating: 1, count: count))
                .offset(x: 160, y: 10)
            SevenSegmentSymbolView(chip: seg, isSelected: false, isHovered: false,
                                   levels: Array(repeating: 0.5, count: count))
                .offset(x: 160, y: 260)
        }
        .frame(width: 700, height: 540, alignment: .topLeading)
        Harness.render(lit, size: CGSize(width: 700, height: 540), to: "\(out)/lit.png")
    }

    // 3–4. The real files, framed on the chips' bounding box.
    for (file, image) in [("be-6502.schematical", "be-6502"), ("test.schematical", "test.schematical")] {
        do {
            let opened = try Harness.open(file)
            let box = opened.chips.map(\.hitRect).reduce(CGRect.null) { $0.union($1) }
            print("\(file): \(opened.chips.count) parts, \(opened.wires.count) wires, \(opened.buses.count) buses")
            Harness.renderCanvas(opened, to: "\(out)/\(image).png",
                                 size: CGSize(width: ceil(box.width) + 80, height: ceil(box.height) + 80),
                                 origin: p(floor(box.minX) - 40, floor(box.minY) - 40))
        } catch {
            print("open failed \(file): \(error)")
        }
    }
}
