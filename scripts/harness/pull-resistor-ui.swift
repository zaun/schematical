// plan-26-B: the pull resistor options, end to end on the app's real
// document and views. Places a Pull-up the way the Pull-up tool does, sets
// its Ohms and Footprint, reads the Bill of Materials, places a Pull-down
// with Settings defaults, saves and reopens. Prints one ok/FAIL line per
// check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p26-h && scripts/harness/build.sh scripts/harness/pull-resistor-ui.swift /tmp/p26-h && \
//     /tmp/p26-h/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p26-sidebar.png, p26-canvas-1k.png,
// p26-canvas-4k7.png, p26-pcb.png, p26-bom.png, p26-settings.png.
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runPullResistorUIChecks() }

@MainActor func runPullResistorUIChecks() {
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
            print("FAIL \(name): \(error.localizedDescription)")
            failures += 1
        }
    }
    // Settings in a test suite, never the app's own defaults.
    let suite = "p26-test"
    let defaults = UserDefaults(suiteName: suite)!
    defaults.removePersistentDomain(forName: suite)

    let doc = Harness.newDocument()
    /// Places what `tool` places, with the Settings options, as the canvas does.
    func place(_ tool: SchematicTool, at point: CGPoint) -> PlacedChip? {
        guard let name = tool.supplyName(voltage: "5V"), let part = Harness.catalog.part(named: name) else {
            return nil
        }
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point,
                      footprintID: NewPartOptions.footprintID(for: part, defaults: defaults),
                      ohms: NewPartOptions.ohms(for: part, defaults: defaults))
        return doc.chips.last
    }
    func chip(_ id: UUID) -> PlacedChip { doc.chips.first { $0.id == id }! }

    guard let pullUp = place(.pullUp, at: CGPoint(x: 100, y: 100)) else {
        print("FAIL catalog: no PULLUP-5V")
        exit(1)
    }
    let id = pullUp.id

    check("1 Pull-up tool: R1, 1kΩ, Radial, on the PCB with 2 pads") {
        let c = chip(id)
        if c.reference != "R1" { return "reference \(c.reference)" }
        if c.ohms != 1000 { return "ohms \(c.ohms.map { "\($0)" } ?? "nil")" }
        if c.footprintID != "RES-RADIAL-2.54" { return "footprint \(c.footprintID ?? "nil")" }
        if c.board == nil || c.boardPads.count != 2 { return "board \(c.board == nil ? "nil" : "set"), \(c.boardPads.count) pads" }
        return nil
    }

    check("2 sidebar: Options with Ohms 1 kΩ and Footprint Radial → \(images)/p26-sidebar.png") {
        if !PartOptionsSection.applies(to: pullUp.part) { return "Options does not apply" }
        let split = ResistorValue.split(chip(id).ohms!)
        if ResistorValue.numberText(split.number) != "1" || split.unit != .kilo {
            return "field shows \(ResistorValue.numberText(split.number)) \(split.unit.symbol)"
        }
        doc.select(.chip(id))
        let path = "\(images)/p26-sidebar.png"
        try? FileManager.default.removeItem(atPath: path)
        Harness.renderSidebar(doc, to: path, size: CGSize(width: 320, height: 700))
        return FileManager.default.fileExists(atPath: path) ? nil : "no image"
    }

    check("3 canvas shows 1kΩ → \(images)/p26-canvas-1k.png") {
        if ResistorValue.text(chip(id).ohms!) != "1kΩ" { return ResistorValue.text(chip(id).ohms!) }
        Harness.renderCanvas(doc, to: "\(images)/p26-canvas-1k.png", size: CGSize(width: 300, height: 260),
                             origin: CGPoint(x: 0, y: 20), zoom: 1.5)
        return nil
    }

    check("4 typing 4.7 with kΩ commits 4700; canvas shows 4.7kΩ → \(images)/p26-canvas-4k7.png") {
        guard let number = ResistorValue.parse("4.7"),
              let value = ResistorValue.ohms(number: number, unit: .kilo) else { return "4.7 kΩ did not parse" }
        doc.setPartOhms(id, value)
        if chip(id).ohms != 4700 { return "ohms \(chip(id).ohms!)" }
        if ResistorValue.text(4700) != "4.7kΩ" { return ResistorValue.text(4700) }
        Harness.renderCanvas(doc, to: "\(images)/p26-canvas-4k7.png", size: CGSize(width: 300, height: 260),
                             origin: CGPoint(x: 0, y: 20), zoom: 1.5)
        return nil
    }

    check("5 Footprint 0805: a 2-pad 0805 at R1, pad 1 kept → \(images)/p26-pcb.png") {
        let before = chip(id).boardPads.first { $0.designator == "1" }!.rect
        doc.setPartFootprint(id, "RES-0805")
        let c = chip(id)
        if c.footprint?.id != "RES-0805" || c.boardPads.count != 2 { return "\(c.footprint?.id ?? "nil"), \(c.boardPads.count) pads" }
        let after = c.boardPads.first { $0.designator == "1" }!.rect
        let moved = hypot(after.midX - before.midX, after.midY - before.midY)
        if moved > 1e-9 { return "pad 1 moved \(moved) mm" }
        let viewport = CanvasViewport()
        let size = CGSize(width: 360, height: 260)
        viewport.updateSize(size)
        viewport.setZoom(6)
        let centre = c.board!.centre
        viewport.pan(to: CGSize(width: size.width / 2 - centre.x * viewport.zoom,
                                height: size.height / 2 - centre.y * viewport.zoom))
        Harness.render(PCBCanvasView(document: doc, viewport: viewport), size: size, to: "\(images)/p26-pcb.png")
        return nil
    }

    check("6 Bill of Materials: RESISTOR, 0805, 4.7kΩ → \(images)/p26-bom.png") {
        let lines = BillOfMaterials.lines(for: doc.chips)
        guard lines.count == 1, let line = lines.first else { return "\(lines.count) lines" }
        if (line.partNumber, line.package, line.value, line.referenceText) != ("RESISTOR", "0805", "4.7kΩ", "R1") {
            return "\(line.partNumber) \(line.package) \(line.value) \(line.referenceText)"
        }
        Harness.renderHosted(BillOfMaterialsTable(document: doc), size: CGSize(width: 640, height: 200),
                             to: "\(images)/p26-bom.png")
        return nil
    }

    check("7 Settings 10 kΩ / Axial: a Pull-down is R2, 10kΩ, Axial → \(images)/p26-settings.png") {
        defaults.set(10_000.0, forKey: PreferenceKey.pullResistorOhms)
        defaults.set("RES-AXIAL-10.16", forKey: PreferenceKey.pullResistorFootprint)
        guard let down = place(.pullDown, at: CGPoint(x: 300, y: 100)) else { return "no PULLDOWN" }
        if down.reference != "R2" { return "reference \(down.reference)" }
        if down.ohms.map(ResistorValue.text) != "10kΩ" { return "ohms \(down.ohms.map { "\($0)" } ?? "nil")" }
        if down.footprintChoice?.package != "Axial" { return "package \(down.footprintChoice?.package ?? "nil")" }
        Harness.renderHosted(PreferencesView().defaultAppStorage(defaults),
                             size: CGSize(width: 420, height: 560), to: "\(images)/p26-settings.png", settle: 0.5)
        return nil
    }

    check("8 save, close, reopen keeps every value") {
        let path = "\(images)/p26-proof.schematical"
        try Harness.save(doc, to: path)
        let text = try String(contentsOfFile: path, encoding: .utf8)
        for needle in ["\"ohms\" : 4700", "\"ohms\" : 10000", "\"footprint\" : \"RES-0805\"",
                       "\"footprint\" : \"RES-AXIAL-10.16\""] where !text.contains(needle) {
            return "file has no \(needle)"
        }
        let opened = try Harness.open(path)
        func summary(_ chips: [PlacedChip]) -> [String] {
            chips.map { "\($0.reference) \($0.partNumber) \($0.ohms ?? -1) \($0.footprintID ?? "-") \($0.board.map { "\($0.centre)" } ?? "-")" }
        }
        let was = summary(doc.chips), now = summary(opened.chips)
        print("     \(now.joined(separator: " | "))")
        return was == now ? nil : "was \(was), now \(now)"
    }

    defaults.removePersistentDomain(forName: suite)
    exit(failures == 0 ? 0 : 1)
}
