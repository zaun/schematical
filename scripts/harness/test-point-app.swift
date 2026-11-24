// plan-57: the test point end to end on the app's real catalog, document and views.
// Checks: the part is listed as the leaf Test Point of Connector; placing it gives
// references TP1, TP2 and the circle-on-a-stub symbol; its pin takes a wire and the
// pad is on the PCB at the default 1.5 mm; the Bill of Materials has no line for it.
// Renders the schematic (a 5V rail through a resistor to two test points) to the image dir.
// Prints one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p57-harness && APP=/tmp/plan57/dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/test-point-app.swift /tmp/p57-harness && \
//     /tmp/p57-harness/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p57-schematic.png.
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runTestPointChecks() }

@MainActor func runTestPointChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp"
    var failures = 0
    func check(_ name: String, _ body: () throws -> String?) {
        do {
            if let problem = try body() {
                print("FAIL \(name): \(problem)")
                failures += 1
            } else {
                print("ok   \(name)")
            }
        } catch {
            print("FAIL \(name): \(error)")
            failures += 1
        }
    }
    let catalog = Harness.catalog
    guard let part = catalog.part(named: "GENERIC-TESTPOINT") else {
        print("FAIL 0 GENERIC-TESTPOINT is not in the catalog")
        exit(1)
    }

    var everything = DrawerFilter()
    everything.visibility = PartVisibility(showObsolete: true, showMilitary: true, showAutomotive: true)
    let connector = catalog.groups(matching: everything).first { $0.category == "Connector" }
    check("1 drawer: Connector lists the leaf Test Point beside its groups") {
        guard let connector else { return "no Connector category" }
        if connector.functions.map(\.function) != ["Test Point"] { return "leaves \(connector.functions.map(\.function))" }
        if connector.subcategories.map(\.subcategory) != ["Barrel", "Card slot", "Header", "USB"] {
            return "groups \(connector.subcategories.map(\.subcategory))"
        }
        return nil
    }

    let doc = Harness.newDocument()
    func put(_ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
        let p = catalog.part(named: name)!
        doc.placeChip(part: p, symbol: symbol(for: p), centeredAt: CGPoint(x: x, y: y))
        return doc.chips.last!
    }
    func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
        .pin(PinRef(chipID: chip.id, index: chip.part.pins.first { $0.name == name }!.index))
    }
    let rail = put("5V", 60, 80)
    let resistor = put("GENERIC-RESISTOR", 200, 80)
    let tp1 = put("GENERIC-TESTPOINT", 340, 80)
    let tp2 = put("GENERIC-TESTPOINT", 340, 180)
    _ = doc.connect(pin(rail, "VCC"), pin(resistor, "1"))
    _ = doc.connect(pin(resistor, "2"), pin(tp1, "TP"))
    _ = doc.connect(pin(resistor, "2"), pin(tp2, "TP"))

    check("2 placing gives TP1 and TP2 and the circle-on-a-stub symbol") {
        let refs = doc.chips.filter { $0.part.isTestPoint }.map(\.reference)
        if refs != ["TP1", "TP2"] { return "references \(refs)" }
        if tp1.symbol != .testPoint { return "symbol \(tp1.symbol)" }
        return nil
    }
    check("3 the pin takes wires: three wires drawn") {
        doc.wires.count == 3 ? nil : "\(doc.wires.count) wires"
    }
    check("4 the pad is on the PCB: one 1.5 mm round pad, no hole") {
        guard let footprint = doc.chips.first(where: { $0.id == tp1.id })?.footprint else { return "no footprint" }
        guard footprint.id == "TESTPOINT-PAD-1.5", footprint.pads.count == 1 else { return "\(footprint.id), \(footprint.pads.count) pads" }
        let pad = footprint.pads[0]
        return pad.size.width == 1.5 && pad.drill == 0 ? nil : "pad \(pad.size) drill \(pad.drill)"
    }
    check("5 the Bill of Materials has no test point line") {
        let lines = BillOfMaterials.lines(for: doc.chips)
        return lines.contains { $0.partNumber.contains("TESTPOINT") } ? "a line: \(lines.map(\.partNumber))" : nil
    }

    Harness.renderCanvas(doc, to: "\(images)/p57-schematic.png", size: CGSize(width: 480, height: 260))
    if failures > 0 { exit(1) }
}
