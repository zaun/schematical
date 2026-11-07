// plan-15: Shift multi-selection. Builds a small circuit, selects parts, a
// wire and a junction, drags the group as the canvas does, and renders the
// Schematic and PCB canvases (and, with --sidebars, both inspectors).
//
//   scripts/harness/build.sh scripts/harness/multiselect.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness [image dir] [--sidebars]
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runMultiSelect() }

@MainActor func runMultiSelect() {
    let arguments = CommandLine.arguments.dropFirst()
    let images = arguments.first { !$0.hasPrefix("--") } ?? "/tmp/p15img"
    let sidebars = arguments.contains("--sidebars")
    let doc = Harness.newDocument()
    func place(_ name: String, at point: CGPoint) -> PlacedChip {
        let part = Harness.catalog.part(named: name)!
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point)
        return doc.chips.last!
    }
    func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
        .pin(PinRef(chipID: chip.id, index: chip.layout.slots.first { $0.pin.name == name }!.pin.index))
    }
    let left = place("LED-RED-0603", at: CGPoint(x: 100, y: 100))
    let right = place("LED-RED-0603", at: CGPoint(x: 400, y: 100))
    let below = place("LED-RED-0603", at: CGPoint(x: 250, y: 260))
    _ = doc.connect(pin(left, "K"), pin(right, "A"))
    let path = doc.path(forWire: doc.wires[0].id)
    let middle = CGPoint(x: SchematicGrid.snap((path.first!.x + path.last!.x) / 2), y: path.first!.y)
    _ = doc.connect(pin(below, "A"), .point(middle))
    let junction = doc.junctions[0]
    let branch = doc.wires.first { $0.start == pin(below, "A").wire || $0.end == pin(below, "A").wire }!

    // Click left, Shift-click below, the branch wire and the junction.
    for (item, shift) in [(SchematicItem.chip(left.id), false), (.chip(below.id), true),
                          (.wire(branch.id), true), (.junction(junction.id), true)] {
        let press = doc.pressSelection(item, shift: shift)
        doc.releaseSelection(press, wasClick: true)
    }
    print("schematic selection: \(doc.schematicSelection.count) items")
    let size = CGSize(width: 560, height: 360)
    let origin = CGPoint(x: -20, y: 20)
    Harness.renderCanvas(doc, to: "\(images)/s1-selected.png", size: size, origin: origin)

    // Plain drag on the selected left LED: the group moves 40 right, 40 down.
    let press = doc.pressSelection(.chip(left.id), shift: false)
    let base = doc.schematicGroupBase()
    for step in 1...5 {
        doc.moveSchematicGroup(base, by: CGSize(width: 8.4 * CGFloat(step), height: 7.9 * CGFloat(step)))
    }
    doc.settleSchematicGroup(base)
    doc.releaseSelection(press, wasClick: false)
    for chip in [left, below, right] {
        let now = doc.chips.first { $0.id == chip.id }!.origin
        print("\(chip.reference): \(chip.origin) -> \(now)")
    }
    print("junction: \(junction.point) -> \(doc.junctions[0].point)")
    print("all right angles: \(doc.wires.allSatisfy { OrthogonalRoute.isOrthogonal(doc.path(forWire: $0.id)) })")
    Harness.renderCanvas(doc, to: "\(images)/s2-dragged.png", size: size, origin: origin)

    // PCB: Shift-select both LED footprints and a text, then drag the group.
    let text = doc.addText(at: CGPoint(x: 0, y: 6))
    for item in [BoardItem.footprint(left.id), .footprint(right.id)] {
        doc.releaseSelection(doc.pressSelection(item, shift: true), wasClick: true)
    }
    print("board selection: \(doc.boardSelection.count) items (text \(doc.isSelected(.text(text))))")
    let boardViewport = CanvasViewport()
    let boardSize = CGSize(width: 520, height: 320)
    boardViewport.updateSize(boardSize)
    boardViewport.setZoom(1.5)
    boardViewport.pan(to: CGSize(width: 40, height: 40))
    Harness.render(PCBCanvasView(document: doc, viewport: boardViewport), size: boardSize, to: "\(images)/p1-selected.png")
    let boardBase = doc.boardGroupBase()
    doc.moveBoardGroup(boardBase, by: CGSize(width: 2.6, height: 1.2))
    for (id, centre) in boardBase.footprints {
        print("footprint \(id.uuidString.prefix(4)): \(centre) -> \(doc.chips.first { $0.id == id }!.board!.centre)")
    }
    print("text: \(boardBase.texts[text]!) -> \(doc.texts[0].anchor)")
    Harness.render(PCBCanvasView(document: doc, viewport: boardViewport), size: boardSize, to: "\(images)/p2-dragged.png")

    if sidebars {
        Harness.renderSidebar(doc, to: "\(images)/i1-schematic-sidebar.png")
        Harness.renderHosted(PCBSidebarView(document: doc), size: CGSize(width: 320, height: 600),
                             to: "\(images)/i2-pcb-sidebar.png")
    }
}

private extension LineAnchor {
    var wire: WireEnd {
        switch self {
        case let .pin(ref): .pin(ref)
        case let .busPin(ref): .busPin(ref)
        case .point: .junction(UUID())
        }
    }
}
