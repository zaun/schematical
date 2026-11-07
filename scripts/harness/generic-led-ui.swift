// plan-18-B: the Generic LED options. The document setters (setPartColor,
// setPartFootprint with its pad-1 anchor), the Schematic sidebar's Options
// section, and the Settings defaults used by a drop. Prints one ok/FAIL line
// per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p18-h && scripts/harness/build.sh scripts/harness/generic-led-ui.swift /tmp/p18-h && \
//     /tmp/p18-h/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp) are p18-sidebar-led.png, p18-sidebar-gate.png and
// p18-settings.png.
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runGenericLEDUIChecks() }

@MainActor func runGenericLEDUIChecks() {
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
    guard let led = Harness.catalog.part(named: "GENERIC-LED"),
          let gate = Harness.catalog.part(named: "SN74HC00N") else {
        print("FAIL catalog: no GENERIC-LED or SN74HC00N")
        exit(1)
    }
    let doc = Harness.newDocument()
    doc.placeChip(part: led, symbol: symbol(for: led), centeredAt: CGPoint(x: 100, y: 100))
    let ledID = doc.chips.last!.id
    doc.placeChip(part: gate, symbol: symbol(for: gate), centeredAt: CGPoint(x: 300, y: 100))
    let gateID = doc.chips.last!.id
    func chip(_ id: UUID) -> PlacedChip { doc.chips.first { $0.id == id }! }
    func pad1(_ id: UUID) -> CGPoint? {
        guard let rect = chip(id).boardPads.first(where: { $0.designator == "1" })?.rect else { return nil }
        return CGPoint(x: rect.midX, y: rect.midY)
    }

    // 1. Color: a color the part has, one it doesn't, and a part with none.
    check("1 setPartColor: Blue sets it; Purple is ignored; a gate keeps nil") {
        doc.setPartColor(ledID, "Blue")
        if chip(ledID).color != "Blue" { return "Blue → \(chip(ledID).color ?? "nil")" }
        doc.setPartColor(ledID, "Purple")
        if chip(ledID).color != "Blue" { return "Purple → \(chip(ledID).color ?? "nil")" }
        doc.setPartColor(gateID, "Blue")
        if chip(gateID).color != nil { return "gate → \(chip(gateID).color!)" }
        return nil
    }

    // 2. Every choice at every rotation keeps pad 1's board point.
    check("2 setPartFootprint keeps pad 1 on the board: every choice × rotation") {
        var cases = 0
        for rotation in [0, 90, 180, 270] {
            while chip(ledID).board?.rotation != rotation { doc.rotateFootprint(ledID) }
            for choice in led.footprintChoices {
                doc.setPartFootprint(ledID, "LED-RADIAL-5MM")
                guard let before = pad1(ledID) else { return "no pad 1 on Radial at \(rotation)" }
                doc.setPartFootprint(ledID, choice.footprint.id)
                if chip(ledID).footprint?.id != choice.footprint.id {
                    return "\(choice.footprint.id) at \(rotation): footprint \(chip(ledID).footprint?.id ?? "nil")"
                }
                guard let after = pad1(ledID) else { return "no pad 1 on \(choice.footprint.id)" }
                let moved = hypot(after.x - before.x, after.y - before.y)
                if moved > 1e-9 { return "\(choice.footprint.id) at \(rotation): pad 1 moved \(moved) mm" }
                cases += 1
            }
        }
        print("     \(cases) cases")
        return cases == 44 ? nil : "\(cases) cases, wanted 44"
    }
    doc.setPartFootprint(ledID, "LED-1206")

    // 3. An unknown id, and the current id again, leave the board alone.
    check("3 setPartFootprint with LED-9999 or the current id leaves the board unchanged") {
        let board = chip(ledID).board
        doc.setPartFootprint(ledID, "LED-9999")
        if chip(ledID).board != board || chip(ledID).footprintID != "LED-1206" { return "LED-9999 changed it" }
        doc.setPartFootprint(ledID, "LED-1206")
        if chip(ledID).board != board { return "the current id changed it" }
        return nil
    }

    // 4. Saving writes what the setters set.
    check("4 encoding gives \"color\" : \"Blue\" and \"footprint\" : \"LED-1206\"") {
        let text = String(decoding: try SchematicFile(document: doc, fileURL: nil).encoded(), as: UTF8.self)
        for needle in ["\"color\" : \"Blue\"", "\"footprint\" : \"LED-1206\""] where !text.contains(needle) {
            return "no \(needle)"
        }
        return nil
    }

    // 5. The sidebar of a Blue / 0805 Generic LED (read the image).
    check("5 sidebar of a GENERIC-LED, Blue / LED-0805 → \(images)/p18-sidebar-led.png") {
        doc.setPartFootprint(ledID, "LED-0805")
        doc.select(.chip(ledID))
        let path = "\(images)/p18-sidebar-led.png"
        try? FileManager.default.removeItem(atPath: path)
        Harness.renderSidebar(doc, to: path, size: CGSize(width: 320, height: 900))
        if !PartOptionsSection.applies(to: led) { return "Options does not apply to GENERIC-LED" }
        let rows = PartFacts.rows(for: chip(ledID))
        func value(_ label: String) -> String? { rows.first { $0.label == label }?.value }
        if value("Package") != "0805" || value("Color") != "Blue" {
            return "Details Package \(value("Package") ?? "nil"), Color \(value("Color") ?? "nil")"
        }
        return FileManager.default.fileExists(atPath: path) ? nil : "no image"
    }

    // 6. A gate's sidebar has no Options section (read the image).
    check("6 sidebar of SN74HC00N (no Options) → \(images)/p18-sidebar-gate.png") {
        doc.select(.chip(gateID))
        let path = "\(images)/p18-sidebar-gate.png"
        try? FileManager.default.removeItem(atPath: path)
        Harness.renderSidebar(doc, to: path)
        if PartOptionsSection.applies(to: gate) { return "Options applies to SN74HC00N" }
        return FileManager.default.fileExists(atPath: path) ? nil : "no image"
    }

    // 7–9. The Settings defaults a drop uses, in a test suite (never the
    // app's own defaults).
    let suite = "p18-test"
    let defaults = UserDefaults(suiteName: suite)!
    defaults.removePersistentDomain(forName: suite)
    /// Places a part the way the drop does, and returns its color and footprint.
    func dropped(_ part: CatalogPart) -> (color: String?, footprint: String?) {
        let target = Harness.newDocument()
        target.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100),
                         color: NewPartOptions.color(for: part, defaults: defaults),
                         footprintID: NewPartOptions.footprintID(for: part, defaults: defaults))
        let placed = target.chips.last!
        return (placed.color, placed.footprint?.id)
    }

    check("7 empty Settings: NewPartOptions nil/nil; a drop gives Red / LED-RADIAL-5MM") {
        let color = NewPartOptions.color(for: led, defaults: defaults)
        let footprint = NewPartOptions.footprintID(for: led, defaults: defaults)
        if color != nil || footprint != nil { return "options \(color ?? "nil") / \(footprint ?? "nil")" }
        let got = dropped(led)
        return got == ("Red", "LED-RADIAL-5MM") ? nil : "dropped \(got)"
    }

    check("8 Settings Green / LED-0603: a drop gives Green / LED-0603; SN74HC00N gets nil/nil") {
        defaults.set("Green", forKey: PreferenceKey.genericLEDColor)
        defaults.set("LED-0603", forKey: PreferenceKey.genericLEDFootprint)
        let got = dropped(led)
        if got != ("Green", "LED-0603") { return "dropped \(got)" }
        let color = NewPartOptions.color(for: gate, defaults: defaults)
        let footprint = NewPartOptions.footprintID(for: gate, defaults: defaults)
        return color == nil && footprint == nil ? nil : "gate options \(color ?? "nil") / \(footprint ?? "nil")"
    }

    check("9 Settings Purple / LED-9999: a drop gives Red / LED-RADIAL-5MM") {
        defaults.set("Purple", forKey: PreferenceKey.genericLEDColor)
        defaults.set("LED-9999", forKey: PreferenceKey.genericLEDFootprint)
        let got = dropped(led)
        return got == ("Red", "LED-RADIAL-5MM") ? nil : "dropped \(got)"
    }

    // 10. The Settings window, reading the empty suite (read the image).
    check("10 Settings window → \(images)/p18-settings.png") {
        defaults.removePersistentDomain(forName: suite)
        let path = "\(images)/p18-settings.png"
        try? FileManager.default.removeItem(atPath: path)
        Harness.renderHosted(PreferencesView().defaultAppStorage(defaults),
                             size: CGSize(width: 420, height: 400), to: path, settle: 0.5)
        return FileManager.default.fileExists(atPath: path) ? nil : "no image"
    }
    defaults.removePersistentDomain(forName: suite)

    exit(failures == 0 ? 0 : 1)
}
