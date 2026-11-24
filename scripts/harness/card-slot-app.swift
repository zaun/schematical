// plan-54-O: card slots end to end on the app's real catalog, document,
// simulation and views. Checks, for every row of planning/plan-54-data/examples.tsv
// and for one library part in each other drawer leaf (SD, microSD, SIM, Smart
// card, Memory module, Card edge; a leaf the library has no part in yet is
// reported and skipped, never failed): the drawer branch Connector → Card slot →
// leaf; that placing the part gives reference J1 and PCB pads at its footprint's
// positions; the sidebar facts (Card, Pitch when above 0, Pins); the Board
// Preview look (a dark housing, 3.0 / 5.2 / 6.0 mm by leaf). Then a 0.6 s play run
// with pins 1 and 2 of the card-edge part (the module part while the library has
// no card-edge one) on two driven nets (5V and GND): neither net is shorted;
// placing a straddle-mount part (one with opposite pads) on a 2-layer board puts
// pads on Top and on Bottom (connectivity()) and the design-rule check has no
// placement issue although its housing hangs over the board edge; and renders a
// socket, a module socket and a card-edge socket (and the sidebar) to the image
// dir. Prints one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p54-harness && APP=/tmp/p54-dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/card-slot-app.swift /tmp/p54-harness && \
//     /tmp/p54-harness/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p54-socket.png, p54-module.png, p54-edge.png (the
// Board Preview, top down, of one socket of each family), p54-sidebar.png.
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runCardSlotChecks() }

@MainActor func runCardSlotChecks() {
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
    let leaves = ["SD", "microSD", "SIM", "Smart card", "Memory module", "Card edge"]

    // The rows of examples.tsv (mpn, family, positions, drawer, footprint); the file grows, no count is assumed.
    struct Example { let mpn: String; let family: String; let drawer: [String]; let footprint: String }
    var examples: [Example] = []
    // Run it from the repository root (or set P54_ROOT).
    let base = ProcessInfo.processInfo.environment["P54_ROOT"] ?? FileManager.default.currentDirectoryPath
    let tsv = try? String(contentsOfFile: "\(base)/planning/plan-54-data/examples.tsv", encoding: .utf8)
    if let tsv {
        for line in tsv.split(separator: "\n").dropFirst() {
            let f = line.split(separator: "\t", omittingEmptySubsequences: false).map(String.init)
            if f.count == 5 { examples.append(Example(mpn: f[0], family: f[1], drawer: f[3].split(separator: "/").map(String.init), footprint: f[4])) }
        }
    }
    check("0 examples.tsv read: \(examples.map(\.mpn))") { examples.isEmpty ? "no examples.tsv rows (run from the repository root)" : nil }

    // The library's card slots and the parts to drive: every example of examples.tsv, and for each drawer leaf
    // with no example the library's smallest card slot in it (nil when the library has none).
    let cardSlots = catalog.allParts.filter(\.isCardSlot)
    func leaf(_ part: CatalogPart) -> String { part.drawer.count == 3 ? part.drawer[2] : "" }
    var representatives: [(leaf: String, part: CatalogPart?)] = []
    for example in examples {
        if let part = catalog.part(named: example.mpn) { representatives.append((example.drawer.last ?? "", part)) }
    }
    for name in leaves where !examples.contains(where: { $0.drawer.last == name }) {
        representatives.append((name, cardSlots.filter { leaf($0) == name }.min {
            $0.pins.count != $1.pins.count ? $0.pins.count < $1.pins.count : $0.name < $1.name
        }))
    }
    let missingLeaves = representatives.filter { $0.part == nil }.map(\.leaf)
    print("     card slots in the library: \(cardSlots.count); drive: \(representatives.compactMap { $0.part?.name }); no part yet in: \(missingLeaves)")

    // (1) The drawer: Connector → Card slot → one subgroup per leaf the library has parts in, no other.
    var everything = DrawerFilter()
    everything.visibility = PartVisibility(showObsolete: true, showMilitary: true, showAutomotive: true)
    let connector = catalog.groups(matching: everything).first { $0.category == "Connector" }
    let slot = connector?.subcategories.first { $0.subcategory == "Card slot" }
    let drawerLeaves = (slot?.subgroups ?? []).map(\.subcategory)
    check("1 drawer: Connector → Card slot → \(drawerLeaves)") {
        guard let slot else { return "no Connector → Card slot group" }
        if Set(drawerLeaves) != Set(cardSlots.map(leaf)) { return "leaves \(drawerLeaves) vs the parts' \(Set(cardSlots.map(leaf)))" }
        if !Set(drawerLeaves).isSubset(of: Set(leaves)) { return "unknown leaves \(drawerLeaves)" }
        if slot.allParts.count != cardSlots.count { return "\(slot.allParts.count) parts listed, \(cardSlots.count) in the library" }
        return nil
    }
    check("1 each leaf's parts are in pin-count order, then by name") {
        for group in slot?.subgroups ?? [] {
            for function in group.functions {
                let counts = function.parts.map(\.pins.count)
                if counts != counts.sorted() { return "\(group.subcategory) / \(function.function): \(counts.prefix(8))" }
                for (a, b) in zip(function.parts, function.parts.dropFirst()) where a.pins.count == b.pins.count {
                    if a.name.localizedStandardCompare(b.name) == .orderedDescending { return "\(a.name) before \(b.name)" }
                }
            }
        }
        return nil
    }
    for entry in representatives where entry.part == nil {
        print("ok   1 leaf \(entry.leaf): no part in the library yet (skipped)")
    }
    for example in examples {
        check("1 \(example.mpn) is listed at \(example.drawer.joined(separator: " › ")) › its function") {
            guard let part = catalog.part(named: example.mpn) else { return "not in the catalog" }
            guard let group = slot?.subgroups.first(where: { $0.subcategory == example.drawer.last }) else { return "no leaf \(example.drawer.last ?? "")" }
            if part.drawer != example.drawer { return "drawer \(part.drawer)" }
            return group.functions.first { $0.function == part.function }?.parts.contains(part) == true ? nil : "not in its function group \(part.function)"
        }
    }

    // (2) Placement, facts and the Board Preview look of each representative.
    for (name, part) in representatives {
        guard let part else { continue }
        // A fresh document for each, so the reference is the first: J1.
        let doc = Harness.newDocument()
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 200, y: 200))
        let chip = doc.chips.last!
        guard case let .cardSlot(card, pitchUM) = part.details else {
            print("FAIL 2 \(name) \(part.name): details \(part.details)")
            failures += 1
            continue
        }
        let rows = PartFacts.rows(for: chip)
        print("     facts \(part.name): " + rows.map { "\($0.label): \($0.value)" }.joined(separator: "; "))
        check("2 \(name) \(part.name): J1, block symbol, pads of \(part.footprint?.id ?? "-")") {
            if chip.reference != "J1" { return "reference \(chip.reference)" }
            if chip.symbol != .block { return "symbol \(chip.symbol)" }
            guard let footprint = part.footprint, let placement = chip.board else { return "no footprint" }
            if let want = examples.first(where: { $0.mpn == part.name })?.footprint, footprint.id != want { return "footprint \(footprint.id), row says \(want)" }
            if chip.boardPads.count != footprint.pads.count || footprint.pads.count < part.pins.count {
                return "\(chip.boardPads.count) PCB pads, \(footprint.pads.count) in the footprint, \(part.pins.count) pins"
            }
            for (placed, pad) in zip(chip.boardPads, footprint.pads) {
                let at = placement.board(pad.centre)
                if placed.designator != pad.designator || abs(placed.rect.midX - at.x) > 1e-6 || abs(placed.rect.midY - at.y) > 1e-6 {
                    return "pad \(pad.designator ?? "-") at (\(placed.rect.midX), \(placed.rect.midY)), the footprint's is (\(at.x), \(at.y))"
                }
            }
            return nil
        }
        check("2 \(name) \(part.name): sidebar Kind, Card \(card), Pitch \(pitchUM > 0 ? String(pitchUM) + " µm" : "none"), Pins \(part.pins.count)") {
            func row(_ label: String) -> String? { rows.first { $0.label == label }?.value }
            if row("Kind") != "Card slot" { return "Kind \(row("Kind") ?? "nil")" }
            if row("Card") != card { return "Card \(row("Card") ?? "nil")" }
            let wantPitch = pitchUM > 0 ? String(format: "%.2f mm", Double(pitchUM) / 1000) : nil
            if row("Pitch") != wantPitch { return "Pitch \(row("Pitch") ?? "nil"), want \(wantPitch ?? "none")" }
            if ["Memory module", "Card edge"].contains(name) && pitchUM == 0 { return "a \(name) socket has no pitch" }
            return row("Pins") == String(part.pins.count) ? nil : "Pins \(row("Pins") ?? "nil")"
        }
        check("2 \(name) \(part.name): Board Preview look is a dark housing, \(name == "Memory module" ? "5.2" : name == "Card edge" ? "6.0" : "3.0") mm") {
            let look = PartLook.of(chip)
            guard case let .cardSlot(height) = look.kind else { return "look \(look.kind)" }
            let want: CGFloat = name == "Memory module" ? 5.2 : name == "Card edge" ? 6.0 : 3.0
            return height == want && look.height == want ? nil : "height \(height) / \(look.height)"
        }
    }

    // (3) The play run: pins 1 and 2 of the card-edge part (the module's while there is none) on 5V and GND.
    let driven = representatives.first { $0.leaf == "Card edge" }?.part ?? representatives.first { $0.leaf == "Memory module" }?.part
        ?? representatives.compactMap(\.part).first
    if let driven {
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
        let one = driven.pins[0].designator, two = driven.pins[1].designator
        let vcc = put(catalog.part(named: "5V")!, 60, 60)
        let gnd = put(catalog.part(named: "GND")!, 60, 600)
        let socket = put(driven, 400, 300)
        let wired: [(PlacedChip, String, String)] = [(vcc, "VCC", one), (gnd, "GND", two)]
        for (source, sourcePin, socketPin) in wired {
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
        let levels = (sim.level(ofPin: one, on: socket.id), sim.level(ofPin: two, on: socket.id))
        let styles = d.wires.map { sim.wireStyle($0.id) }
        sim.stop()
        print("     \(driven.name) pins \(one) / \(two): levels \(levels.0 ?? "nil") / \(levels.1 ?? "nil"); wire styles \(styles)")
        check("3 \(driven.name): pins \(one) on 5V and \(two) on GND for 0.6 s, neither shorted") {
            if let stopped { return "run stopped: \(stopped)" }
            if !running { return "not running" }
            if d.wires.count != 2 { return "\(d.wires.count) wires" }
            if styles.contains(.shorted) { return "a net is shorted: \(styles)" }
            return levels.0 == "1" && levels.1 == "0" ? nil : "levels \(levels.0 ?? "nil") / \(levels.1 ?? "nil")"
        }
    } else {
        print("FAIL 3: no card slot to drive")
        failures += 1
    }

    // (4) A straddle-mount part (opposite pads) on a 2-layer board: pads on Top and on Bottom, and no
    // placement issue though its housing hangs over the board edge.
    // The straddle example of examples.tsv, else the library's smallest card slot with opposite pads.
    let straddle = examples.filter { $0.family == "straddle" }.compactMap { catalog.part(named: $0.mpn) }.first
        ?? cardSlots.filter { $0.footprint?.pads.contains(where: \.opposite) == true }
        .min { $0.pins.count != $1.pins.count ? $0.pins.count < $1.pins.count : $0.name < $1.name }
    if let straddle {
        let d = Harness.newDocument()
        d.setLayerCount(2)
        d.setBoardSize(width: 150, height: 100)
        d.placeChip(part: straddle, symbol: symbol(for: straddle), centeredAt: CGPoint(x: 400, y: 300))
        let id = d.chips.last!.id
        func chip() -> PlacedChip { d.chips.first { $0.id == id }! }
        func overhang() -> (edge: String, bounds: CGRect, pads: CGRect)? {
            guard let bounds = chip().boardBounds, let pads = chip().boardPadBounds else { return nil }
            return ("", bounds, pads)
        }
        // Put the pads 0.4 mm inside the edge the housing extends past the most, the pads centred along it.
        d.moveFootprintOrigin(id, to: CGPoint(x: 75, y: 50))
        if let (_, bounds, pads) = overhang(), let origin = chip().boardOrigin {
            let w = d.board.width, h = d.board.height, margin = 0.4
            let past = [("top", pads.minY - bounds.minY), ("bottom", bounds.maxY - pads.maxY),
                        ("left", pads.minX - bounds.minX), ("right", bounds.maxX - pads.maxX)].max { $0.1 < $1.1 }!
            var to = origin
            switch past.0 {
            case "top": to.y += margin - pads.minY; to.x += w / 2 - pads.midX
            case "bottom": to.y += (h - margin) - pads.maxY; to.x += w / 2 - pads.midX
            case "left": to.x += margin - pads.minX; to.y += h / 2 - pads.midY
            default: to.x += (w - margin) - pads.maxX; to.y += h / 2 - pads.midY
            }
            d.moveFootprintOrigin(id, to: to)
        }
        let placed = chip()
        let copper = d.connectivity()
        let sides = Set(copper.pads.filter { $0.label.hasPrefix("\(placed.reference).") }.map(\.side))
        let issues = d.designRuleCheck(rules: DesignRules.pcbway).filter { $0.reference == placed.reference && $0.kind == .placement }
        let bounds = placed.boardBounds, pads = placed.boardPadBounds
        print("     \(straddle.name): pad sides \(sides), body \(bounds.map { "\($0)" } ?? "nil"), pads \(pads.map { "\($0)" } ?? "nil")")
        check("4 \(straddle.name) on a 2-layer board: pads on Top and Bottom; no placement issue with the housing over the edge") {
            if sides != [.top, .bottom] { return "pad sides \(sides)" }
            guard let bounds, let pads else { return "no bounds" }
            let outside = bounds.minX < 0 || bounds.minY < 0 || bounds.maxX > d.board.width || bounds.maxY > d.board.height
            if !outside { return "the housing does not hang over the edge: \(bounds)" }
            if pads.minX < 0 || pads.minY < 0 || pads.maxX > d.board.width || pads.maxY > d.board.height { return "pads off the board: \(pads)" }
            return issues.isEmpty ? nil : "placement issues \(issues.map(\.issue))"
        }
    } else {
        print("ok   4 straddle-mount: no card slot with opposite pads in the library yet (skipped)")
    }

    // (5) Pictures: the Board Preview of a socket, a module socket and a card-edge socket, and the sidebar.
    var written: [String] = []
    var pictures: [(String, String?)] = [("p54-socket.png", "microSD"), ("p54-module.png", "Memory module"), ("p54-edge.png", "Card edge")]
    pictures = pictures.map { name, wanted in
        (name, representatives.first { $0.leaf == wanted }?.part != nil ? wanted : nil)
    }
    for (file, wanted) in pictures {
        guard let wanted, let part = representatives.first(where: { $0.leaf == wanted })?.part else {
            print("ok   5 \(file): no part in the leaf yet (skipped)")
            continue
        }
        let d = Harness.newDocument()
        d.setBoardSize(width: 110, height: 60)
        d.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 400, y: 300))
        d.moveFootprintOrigin(d.chips.last!.id, to: CGPoint(x: 30, y: 30))
        let path = "\(images)/\(file)"
        Harness.render(BoardPreviewView.canvas(document: d, face: .front, angle: .angled), size: CGSize(width: 720, height: 420), to: path)
        written.append(path)
    }
    if let module = representatives.first(where: { $0.leaf == "Memory module" })?.part ?? representatives.compactMap(\.part).first {
        let d = Harness.newDocument()
        d.placeChip(part: module, symbol: symbol(for: module), centeredAt: CGPoint(x: 200, y: 200))
        d.select(.chip(d.chips.last!.id))
        let path = "\(images)/p54-sidebar.png"
        Harness.renderSidebar(d, to: path, size: CGSize(width: 320, height: 640))
        written.append(path)
    }
    check("5 images written: \(written)") {
        let missing = written.filter { !FileManager.default.fileExists(atPath: $0) }
        return missing.isEmpty && !written.isEmpty ? nil : "missing \(missing)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
