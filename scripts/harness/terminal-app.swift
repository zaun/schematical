// plan-45-H: PCB terminal blocks and JST board headers end to end on the app's
// real catalog, document, simulation and views. Checks the drawer (Connector →
// Terminal block → six pitches in numeric order, each leaf's parts in
// position order; Connector → JST → the series), that a placed
// GENERIC-TERMINAL-5.08-03 is J1 with three PCB pads 5.08 mm apart and a
// placed B2B-XH-A is a JST header (JST-XH-2.50-VT-02, two pads 2.50 mm apart),
// the sidebar facts of both, their Board Preview looks (a green terminal
// block 9.5 mm, a white JST header 6 mm), then a 0.6 s play run with pins 1 and
// 2 of the terminal block on the 5V and GND nets: neither net is shorted.
// Prints one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p45-harness && APP=/tmp/p45-dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/terminal-app.swift /tmp/p45-harness && \
//     /tmp/p45-harness/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p45-terminal.png and p45-jst.png (the Board
// Preview, angled, of a terminal block and a JST header), p45-sidebar.png (the
// terminal block selected).
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runTerminalChecks() }

@MainActor func runTerminalChecks() {
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
    func named(_ name: String) -> CatalogPart? { catalog.part(named: name) }
    let pitches = ["3.50 mm", "3.81 mm", "5.00 mm", "5.08 mm", "7.50 mm", "7.62 mm"]
    let series = ["EH", "GH", "PH", "SH-SR", "VH", "XH", "ZH"]

    // (1) The drawer.
    var everything = DrawerFilter()
    everything.visibility = PartVisibility(showObsolete: true, showMilitary: true, showAutomotive: true)
    let connector = catalog.groups(matching: everything).first { $0.category == "Connector" }
    let block = connector?.subcategories.first { $0.subcategory == "Terminal block" }
    let jst = connector?.subcategories.first { $0.subcategory == "JST" }
    check("1 drawer: Connector → Terminal block → \(block?.subgroups.map(\.subcategory) ?? [])") {
        guard let block else { return "no Connector → Terminal block group" }
        return block.subgroups.map(\.subcategory) == pitches ? nil : "leaves \(block.subgroups.map(\.subcategory))"
    }
    check("1 drawer: Connector → JST → \(jst?.subgroups.map(\.subcategory) ?? [])") {
        guard let jst else { return "no Connector → JST group" }
        return Set(jst.subgroups.map(\.subcategory)) == Set(series) ? nil : "series \(jst.subgroups.map(\.subcategory))"
    }
    check("1 every leaf's parts are in position order, then by name; the generic leaf lists 2 … 12") {
        for group in (block?.subgroups ?? []) + (jst?.subgroups ?? []) {
            for function in group.functions {
                let counts = function.parts.map(\.pins.count)
                if counts != counts.sorted() { return "\(group.subcategory) / \(function.function): \(counts.prefix(8))" }
                for (a, b) in zip(function.parts, function.parts.dropFirst()) where a.pins.count == b.pins.count {
                    if a.name.localizedStandardCompare(b.name) == .orderedDescending { return "\(a.name) before \(b.name)" }
                }
            }
        }
        guard let generic = block?.subgroups.first(where: { $0.subcategory == "5.08 mm" })?
            .functions.first(where: { $0.function == "Generic" })?.parts else { return "no 5.08 mm › Generic" }
        return generic.map(\.pins.count) == Array(2...12) ? nil : "generic positions \(generic.map(\.pins.count))"
    }

    // (2) Placement, facts and the Board Preview look.
    guard let terminal = named("GENERIC-TERMINAL-5.08-03"), let xh = named("B2B-XH-A") else {
        print("FAIL 2: no GENERIC-TERMINAL-5.08-03 or B2B-XH-A")
        exit(1)
    }
    func place(_ part: CatalogPart) -> (SchematicDocument, PlacedChip) {
        let doc = Harness.newDocument()
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 200, y: 200))
        return (doc, doc.chips.last!)
    }
    func rows(_ chip: PlacedChip) -> [String: String] {
        Dictionary(PartFacts.rows(for: chip).map { ($0.label, $0.value) }, uniquingKeysWith: { first, _ in first })
    }
    func padSpacing(_ chip: PlacedChip) -> [CGFloat] {
        let xs = chip.boardPads.map { $0.rect.midX }.sorted()
        return zip(xs, xs.dropFirst()).map { $1 - $0 }
    }
    let (_, terminalChip) = place(terminal)
    let (_, xhChip) = place(xh)
    check("2 GENERIC-TERMINAL-5.08-03 is J1 on \(terminal.footprint?.id ?? "-"), block symbol, three pads 5.08 mm apart") {
        if terminalChip.reference != "J1" { return "reference \(terminalChip.reference)" }
        if terminalChip.symbol != .block { return "symbol \(terminalChip.symbol)" }
        if terminalChip.boardPads.count != 3 { return "\(terminalChip.boardPads.count) PCB pads" }
        let spacing = padSpacing(terminalChip)
        return spacing.allSatisfy { abs($0 - 5.08) < 1e-6 } ? nil : "pad spacing \(spacing)"
    }
    check("2 B2B-XH-A is a JST header: J1 on \(xh.footprint?.id ?? "-"), two pads 2.50 mm apart") {
        guard case let .wireConnector(kind, variant, pitchUM, _, _) = xh.details else { return "details \(xh.details)" }
        if kind != "jst" || variant != "XH" || pitchUM != 2500 { return "\(kind) \(variant) \(pitchUM)" }
        if xhChip.reference != "J1" { return "reference \(xhChip.reference)" }
        if xh.footprint?.id != "JST-XH-2.50-VT-02" { return "footprint \(xh.footprint?.id ?? "nil")" }
        let spacing = padSpacing(xhChip)
        return spacing.count == 1 && abs(spacing[0] - 2.5) < 1e-6 ? nil : "pad spacing \(spacing)"
    }
    let terminalRows = rows(terminalChip), xhRows = rows(xhChip)
    print("     facts \(terminal.name): " + PartFacts.rows(for: terminalChip).map { "\($0.label): \($0.value)" }.joined(separator: "; "))
    print("     facts \(xh.name): " + PartFacts.rows(for: xhChip).map { "\($0.label): \($0.value)" }.joined(separator: "; "))
    check("2 sidebar: Kind Terminal block, Pitch 5.08 mm, Clamp Screw, Positions 3, no rating") {
        let want = ["Kind": "Terminal block", "Type": "Terminal block", "Pitch": "5.08 mm", "Clamp": "Screw", "Positions": "3"]
        for (label, value) in want where terminalRows[label] != value { return "\(label) \(terminalRows[label] ?? "nil")" }
        return terminalRows["Rated voltage"] == nil && terminalRows["Rated current"] == nil ? nil : "a rating on a generic"
    }
    check("2 sidebar: Kind JST connector, Pitch 2.50 mm, Series XH, Positions 2") {
        let want = ["Kind": "JST connector", "Type": "JST header", "Pitch": "2.50 mm", "Series": "XH", "Positions": "2"]
        for (label, value) in want where xhRows[label] != value { return "\(label) \(xhRows[label] ?? "nil")" }
        return xhRows["Clamp"] == nil ? nil : "a clamp on a JST header"
    }
    check("2 Board Preview looks: terminal block 9.5 mm, JST header 6 mm") {
        let a = PartLook.of(terminalChip), b = PartLook.of(xhChip)
        guard case .terminalBlock = a.kind else { return "terminal block look \(a.kind)" }
        guard case .jstHeader = b.kind else { return "JST look \(b.kind)" }
        return a.height == 9.5 && b.height == 6 ? nil : "heights \(a.height) / \(b.height)"
    }

    // (3) The play run: pins 1 and 2 of the terminal block on 5V and GND.
    let d = Harness.newDocument()
    func put(_ part: CatalogPart, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
        d.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: x, y: y))
        return d.chips.last!
    }
    func pin(_ chip: PlacedChip, _ designator: String) -> LineAnchor? {
        guard let slot = chip.layout.slots.first(where: { $0.pin.designator == designator || $0.pin.name == designator })
        else { return nil }
        return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
    }
    let vcc = put(named("5V")!, 60, 60)
    let gnd = put(named("GND")!, 60, 600)
    let socket = put(terminal, 400, 300)
    for (source, sourcePin, socketPin) in [(vcc, "VCC", "1"), (gnd, "GND", "2")] {
        guard let a = pin(source, sourcePin), let b = pin(socket, socketPin) else {
            print("FAIL wiring \(sourcePin) → \(socketPin): no such pin")
            failures += 1
            continue
        }
        if let message = d.connect(a, b) {
            print("FAIL wiring \(sourcePin) → \(socketPin): \(message)")
            failures += 1
        }
    }
    let sim = Harness.simulation
    sim.start(document: d)
    RunLoop.main.run(until: Date().addingTimeInterval(0.6))
    let stopped = sim.errorMessage
    let running = sim.isRunning
    let levels = (sim.level(ofPin: "1", on: socket.id), sim.level(ofPin: "2", on: socket.id))
    let styles = d.wires.map { sim.wireStyle($0.id) }
    sim.stop()
    print("     \(terminal.name) pins 1 / 2: levels \(levels.0 ?? "nil") / \(levels.1 ?? "nil"); wire styles \(styles)")
    check("3 \(terminal.name): pins 1 on 5V and 2 on GND for 0.6 s, neither shorted") {
        if let stopped { return "run stopped: \(stopped)" }
        if !running { return "not running" }
        if d.wires.count != 2 { return "\(d.wires.count) wires" }
        if styles.contains(.shorted) { return "a net is shorted: \(styles)" }
        return levels.0 == "1" && levels.1 == "0" ? nil : "levels \(levels.0 ?? "nil") / \(levels.1 ?? "nil")"
    }

    // (4) Pictures: the Board Preview of a terminal block and a JST header, and the sidebar.
    var written: [String] = []
    for (file, part) in [("p45-terminal.png", terminal), ("p45-jst.png", xh)] {
        let doc = Harness.newDocument()
        doc.setBoardSize(width: 60, height: 40)
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 400, y: 300))
        doc.moveFootprintOrigin(doc.chips.last!.id, to: CGPoint(x: 30, y: 20))
        let path = "\(images)/\(file)"
        Harness.render(BoardPreviewView.canvas(document: doc, face: .front, angle: .angled), size: CGSize(width: 720, height: 420), to: path)
        written.append(path)
    }
    let (sidebarDoc, sidebarChip) = place(terminal)
    sidebarDoc.select(.chip(sidebarChip.id))
    Harness.renderSidebar(sidebarDoc, to: "\(images)/p45-sidebar.png", size: CGSize(width: 320, height: 640))
    written.append("\(images)/p45-sidebar.png")
    check("4 images written: \(written)") {
        let missing = written.filter { !FileManager.default.fileExists(atPath: $0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
