// plan-18-A Phase 3: GENERIC-LED in files. Old LED names migrate, format 4
// saves the color and footprint, bad values fail, and a chosen footprint
// drives the PCB. Prints one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p18-h && scripts/harness/build.sh scripts/harness/generic-led-file.swift /tmp/p18-h && \
//     /tmp/p18-h/H.app/Contents/MacOS/harness [repo dir]
//
// The repo dir (default: the current directory) holds be-6502.schematical
// and test.schematical. Use a fresh out dir: build.sh reuses an existing app
// copy, whose part files may be old.
import CoreGraphics
import Foundation

MainActor.assumeIsolated { runGenericLEDFileChecks() }

@MainActor func runGenericLEDFileChecks() {
    let repo = CommandLine.arguments.count > 1
        ? CommandLine.arguments[1] : FileManager.default.currentDirectoryPath
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
    func json(_ data: Data) -> String { String(decoding: data, as: UTF8.self) }
    /// The encoded text of one part entry: from its `"id"` back to the `{`
    /// before it, through the next `}` that closes the entry. Sorted keys put
    /// `color`, `flipped`, `footprint`, `id`, `partNumber`, `pcb`, … in order.
    func entry(_ text: String, reference: String) -> String? {
        guard let ref = text.range(of: "\"reference\" : \"\(reference)\"") else { return nil }
        let before = text[..<ref.lowerBound]
        // The entry's opening brace: the last "{\n      \"" at part depth.
        guard let open = before.range(of: "\n    {", options: .backwards) else { return nil }
        guard let close = text.range(of: "\n    }", range: ref.upperBound..<text.endIndex) else { return nil }
        return String(text[open.lowerBound..<close.upperBound])
    }

    // 1. The repo's be-6502 schematic: U22 was LED-GREEN-RADIAL.
    var u22Doc: SchematicDocument?
    check("1 be-6502.schematical: U22 is GENERIC-LED, Green, LED-RADIAL-5MM, at (9.525, 13.97)") {
        let doc = try Harness.open("\(repo)/be-6502.schematical")
        u22Doc = doc
        guard let chip = doc.chips.first(where: { $0.reference == "U22" }) else { return "no U22" }
        if chip.partNumber != "GENERIC-LED" { return "part \(chip.partNumber)" }
        if chip.color != "Green" { return "color \(chip.color ?? "nil")" }
        if chip.footprint?.id != "LED-RADIAL-5MM" { return "footprint \(chip.footprint?.id ?? "nil")" }
        guard let board = chip.board else { return "no board placement" }
        if board.centre != CGPoint(x: 9.525, y: 13.97) { return "centre \(board.centre)" }
        return nil
    }

    // 2. test.schematical, a version 1 file: U3 was LED-AMBER-0603.
    check("2 test.schematical (v1): its LED is GENERIC-LED, Amber, LED-0603") {
        let doc = try Harness.open("\(repo)/test.schematical")
        let leds = doc.chips.filter { $0.part.kind == PART_LED }
        guard leds.count == 1, let chip = leds.first else { return "\(leds.count) LEDs" }
        if chip.partNumber != "GENERIC-LED" { return "part \(chip.partNumber)" }
        if chip.color != "Amber" { return "color \(chip.color ?? "nil")" }
        if chip.footprint?.id != "LED-0603" { return "footprint \(chip.footprint?.id ?? "nil")" }
        return nil
    }

    // 3. Saving (1) writes version 4 and U22's options; reopening it is the same file.
    var savedFile: SchematicFile?
    check("3 encode be-6502: version 4, U22 color/footprint/partNumber; round trip equal") {
        guard let doc = u22Doc else { return "check 1 did not open the file" }
        let file = SchematicFile(document: doc, fileURL: nil)
        savedFile = file
        let text = json(try file.encoded())
        if !text.contains("\"version\" : 4") { return "no \"version\" : 4" }
        guard let u22 = entry(text, reference: "U22") else { return "no U22 entry" }
        for needle in ["\"color\" : \"Green\"", "\"footprint\" : \"LED-RADIAL-5MM\"",
                       "\"partNumber\" : \"GENERIC-LED\""] where !u22.contains(needle) {
            return "U22 entry lacks \(needle): \(u22)"
        }
        let reopened = Harness.newDocument()
        try SchematicFile.decode(try file.encoded()).apply(to: reopened, catalog: Harness.catalog, fileURL: nil)
        let again = SchematicFile(document: reopened, fileURL: nil)
        return again == file ? nil : "the reopened file differs"
    }

    // 4. Placing a GENERIC-LED: defaults, then a chosen footprint on the PCB.
    var chip0805: PlacedChip?
    check("4 placeChip: default Red/LED-RADIAL-5MM; LED-0805 → 2 pads; LED-DIP4 → 4 pads") {
        guard let led = Harness.catalog.part(named: "GENERIC-LED") else { return "no GENERIC-LED" }
        let doc = Harness.newDocument()
        doc.placeChip(part: led, symbol: symbol(for: led), centeredAt: CGPoint(x: 100, y: 100))
        let plain = doc.chips.last!
        if plain.color != "Red" || plain.footprint?.id != "LED-RADIAL-5MM" {
            return "default \(plain.color ?? "nil") / \(plain.footprint?.id ?? "nil")"
        }
        doc.placeChip(part: led, symbol: symbol(for: led), centeredAt: CGPoint(x: 300, y: 100),
                      footprintID: "LED-0805")
        let small = doc.chips.last!
        chip0805 = small
        if small.footprint?.id != "LED-0805" { return "0805 chip footprint \(small.footprint?.id ?? "nil")" }
        if small.boardPads.count != 2 { return "0805 boardPads \(small.boardPads.count)" }
        doc.placeChip(part: led, symbol: symbol(for: led), centeredAt: CGPoint(x: 500, y: 100),
                      footprintID: "LED-DIP4")
        let dip = doc.chips.last!
        if dip.footprint?.id != "LED-DIP4" { return "DIP chip footprint \(dip.footprint?.id ?? "nil")" }
        if dip.boardPads.count != 4 { return "DIP boardPads \(dip.boardPads.count)" }
        return nil
    }

    // 5. A version 4 file with a color or footprint the part doesn't offer.
    check("5 bad color / footprint fail with the badPartOption messages") {
        guard var file = savedFile, let index = file.parts.firstIndex(where: { $0.reference == "U22" })
        else { return "no saved file from check 3" }
        @MainActor func openError(_ file: SchematicFile) -> String? {
            do {
                let data = try file.encoded()
                try SchematicFile.decode(data).apply(to: Harness.newDocument(), catalog: Harness.catalog, fileURL: nil)
                return nil
            } catch {
                return error.localizedDescription
            }
        }
        file.parts[index].color = "Purple"
        let colorError = openError(file)
        if colorError != "U22: \"Purple\" is not a color of GENERIC-LED" {
            return "color: \(colorError ?? "opened")"
        }
        file.parts[index].color = "Green"
        file.parts[index].footprint = "LED-9999"
        let footprintError = openError(file)
        if footprintError != "U22: \"LED-9999\" is not a footprint of GENERIC-LED" {
            return "footprint: \(footprintError ?? "opened")"
        }
        return nil
    }

    // 6. The Details rows of the 0805 chip from check 4.
    check("6 PartFacts.rows(for:) of the 0805 chip: Package 0805, Color Red") {
        guard let chip = chip0805 else { return "no chip from check 4" }
        let rows = PartFacts.rows(for: chip)
        func value(_ label: String) -> String? { rows.first { $0.label == label }?.value }
        if value("Package") != "0805" { return "Package \(value("Package") ?? "nil")" }
        if value("Color") != "Red" { return "Color \(value("Color") ?? "nil")" }
        if value("Footprint")?.hasPrefix("LED-0805") != true { return "Footprint \(value("Footprint") ?? "nil")" }
        return nil
    }

    // 7. The drawer lists one LED-kind part.
    check("7 the catalog lists one LED-kind part, GENERIC-LED") {
        var parts: [CatalogPart] = []
        for group in Harness.catalog.groups(matching: DrawerFilter()) {
            parts += group.functions.flatMap(\.parts)
            for sub in group.subcategories {
                parts += sub.parts + sub.functions.flatMap(\.parts)
            }
        }
        let leds = parts.filter { $0.kind == PART_LED }.map(\.name)
        return leds == ["GENERIC-LED"] ? nil : "LED parts \(leds)"
    }

    exit(failures == 0 ? 0 : 1)
}
