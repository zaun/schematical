// plan-23: the generic pin header in the app. The drawer row, defaults,
// layout, file round trip, the size swap (wire removal, pad-1 anchor),
// gender, and (plan-23-C) the sidebar and Settings. Prints one ok/FAIL line
// per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p23-h && scripts/harness/build.sh scripts/harness/generic-header.swift /tmp/p23-h && \
//     /tmp/p23-h/H.app/Contents/MacOS/harness
//
// Use a fresh out dir: build.sh reuses an existing app copy, whose part files
// may be old.
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runGenericHeaderChecks() }

@MainActor func runGenericHeaderChecks() {
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
    let catalog = Harness.catalog
    func header(_ rows: Int, _ perRow: Int) -> CatalogPart? {
        catalog.part(named: HeaderFamily.name(rows: rows, perRow: perRow))
    }
    /// Places a header and returns the document and the placed chip's id.
    func place(_ part: CatalogPart, in doc: SchematicDocument, at point: CGPoint = CGPoint(x: 200, y: 200),
               footprintID: String? = nil, gender: String? = nil) -> UUID {
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: point,
                      footprintID: footprintID, gender: gender)
        return doc.chips.last!.id
    }
    func chip(_ doc: SchematicDocument, _ id: UUID) -> PlacedChip? { doc.chips.first { $0.id == id } }
    func pad1(_ chip: PlacedChip?) -> CGPoint? {
        guard let rect = chip?.boardPads.first(where: { $0.designator == "1" })?.rect else { return nil }
        return CGPoint(x: rect.midX, y: rect.midY)
    }

    // 1. One drawer row; every size still in the catalog.
    check("1 the drawer lists one header-kind part, GENERIC-HEADER-1X02; GENERIC-HEADER-2X40 exists") {
        var parts: [CatalogPart] = []
        for group in catalog.groups(matching: DrawerFilter()) {
            parts += group.functions.flatMap(\.parts)
            for sub in group.subcategories {
                parts += sub.parts + sub.functions.flatMap(\.parts)
            }
        }
        let headers = parts.filter { $0.kind == PART_HEADER }.map(\.name)
        if headers != ["GENERIC-HEADER-1X02"] { return "header parts \(headers)" }
        return catalog.part(named: "GENERIC-HEADER-2X40") == nil ? "no GENERIC-HEADER-2X40" : nil
    }

    // 2. The default a drop places.
    check("2 placeChip(GENERIC-HEADER-1X02): Male, PINHDR-1X02-V, 2 pads, 2 left slots at 10 and 30") {
        guard let part = catalog.part(named: HeaderFamily.defaultPart) else { return "no default part" }
        let doc = Harness.newDocument()
        guard let placed = chip(doc, place(part, in: doc)) else { return "not placed" }
        if placed.gender != "Male" { return "gender \(placed.gender ?? "nil")" }
        if placed.footprint?.id != "PINHDR-1X02-V" { return "footprint \(placed.footprint?.id ?? "nil")" }
        if placed.boardPads.count != 2 { return "\(placed.boardPads.count) pads" }
        let slots = placed.layout.slots
        if slots.map(\.side) != [.left, .left] || slots.map(\.centerY) != [10, 30] {
            return "slots \(slots.map { "\($0.pin.designator):\($0.side):\($0.centerY)" })"
        }
        return nil
    }

    // 3. Two rows zigzag.
    check("3 GENERIC-HEADER-2X04: 1,3,5,7 left, 2,4,6,8 right, pin 2 beside pin 1") {
        guard let part = header(2, 4) else { return "no 2X04" }
        let layout = ChipLayout(part: part, symbol: symbol(for: part))
        func slot(_ designator: String) -> ChipLayout.PinSlot? {
            layout.slots.first { $0.pin.designator == designator }
        }
        for n in [1, 3, 5, 7] where slot("\(n)")?.side != .left { return "pin \(n) not left" }
        for n in [2, 4, 6, 8] where slot("\(n)")?.side != .right { return "pin \(n) not right" }
        if slot("2")?.centerY != slot("1")?.centerY { return "pin 2 at \(slot("2")?.centerY ?? -1)" }
        if layout.size != CGSize(width: 40, height: 80) { return "size \(layout.size)" }
        return nil
    }

    // 4. File round trip, and a bad gender.
    check("4 Female 1X08 on -RA saves gender and footprint, reopens equal; \"Neutral\" fails") {
        guard let part = header(1, 8) else { return "no 1X08" }
        let doc = Harness.newDocument()
        _ = place(part, in: doc, footprintID: "PINHDR-1X08-RA", gender: "Female")
        let file = SchematicFile(document: doc, fileURL: nil)
        let text = json(try file.encoded())
        for needle in ["\"gender\" : \"Female\"", "\"footprint\" : \"PINHDR-1X08-RA\"",
                       "\"partNumber\" : \"GENERIC-HEADER-1X08\""] where !text.contains(needle) {
            return "no \(needle)"
        }
        let reopened = Harness.newDocument()
        try SchematicFile.decode(try file.encoded()).apply(to: reopened, catalog: catalog, fileURL: nil)
        if SchematicFile(document: reopened, fileURL: nil) != file { return "the reopened file differs" }
        var bad = file
        bad.parts[0].gender = "Neutral"
        do {
            try SchematicFile.decode(try bad.encoded()).apply(to: Harness.newDocument(), catalog: catalog, fileURL: nil)
            return "Neutral opened"
        } catch {
            let message = error.localizedDescription
            return message == "U1: \"Neutral\" is not a gender of GENERIC-HEADER-1X08" ? nil : "error \(message)"
        }
    }

    // 5. The symbols.
    check("5 render a male 1x4 and a female 2x3 → /tmp/p23-headers.png") {
        guard let small = header(1, 4), let wide = header(2, 3) else { return "no parts" }
        let doc = Harness.newDocument()
        _ = place(small, in: doc, at: CGPoint(x: 100, y: 120))
        _ = place(wide, in: doc, at: CGPoint(x: 300, y: 120), gender: "Female")
        Harness.renderCanvas(doc, to: "/tmp/p23-headers.png", size: CGSize(width: 420, height: 240))
        return FileManager.default.fileExists(atPath: "/tmp/p23-headers.png") ? nil : "no image"
    }

    // 6. Shrinking removes the wires on pins that go away and re-fits the rest.
    check("6 1X08 → 1X04: the pin-8 wire and its junction go; pins 1, 2 stay wired at their tips") {
        guard let big = header(1, 8), let small = header(1, 4) else { return "no parts" }
        let doc = Harness.newDocument()
        let id = place(big, in: doc)
        guard let placed = chip(doc, id) else { return "not placed" }
        let reference = placed.reference, origin = placed.origin
        var junctions: [Junction] = []
        var wires: [Wire] = []
        var wireByPin: [Int: UUID] = [:]
        for pin in [1, 2, 8] {
            guard let slot = placed.layout.slots.first(where: { $0.pin.designator == "\(pin)" }) else {
                return "no slot \(pin)"
            }
            let tip = placed.tip(of: slot)
            let junction = Junction(point: CGPoint(x: tip.x - 40, y: tip.y))
            junctions.append(junction)
            let wire = Wire(start: .pin(PinRef(chipID: id, index: slot.pin.index)), end: .junction(junction.id),
                            interior: [])
            wires.append(wire)
            wireByPin[pin] = wire.id
        }
        doc.replaceContents(chips: doc.chips, wires: wires, junctions: junctions, buses: [],
                            nextReference: doc.nextReference, board: doc.board, texts: [], traces: [])
        doc.setHeaderSize(id, to: small)
        guard let after = chip(doc, id) else { return "chip gone" }
        if after.partNumber != "GENERIC-HEADER-1X04" { return "part \(after.partNumber)" }
        if after.reference != reference || after.origin != origin { return "reference/origin changed" }
        if doc.wires.contains(where: { $0.id == wireByPin[8] }) { return "pin-8 wire kept" }
        if doc.junctions.contains(where: { $0.id == junctions[2].id }) { return "pin-8 junction kept" }
        if doc.wires.count != 2 || doc.junctions.count != 2 { return "\(doc.wires.count) wires, \(doc.junctions.count) junctions" }
        for pin in [1, 2] {
            guard let wire = wireByPin[pin], let slot = after.layout.slots.first(where: { $0.pin.designator == "\(pin)" })
            else { return "pin \(pin) missing" }
            if doc.path(forWire: wire).first != after.tip(of: slot) { return "pin \(pin) wire not at its tip" }
        }
        return nil
    }

    // 7. Pad 1 never moves, and the footprint keeps its type, across 16 cases.
    var cases = 0
    check("7 pad 1 fixed and footprint type kept: 1X08 → 2X04 → 1X02, V/RA × 0/90/180/270") {
        guard let start = header(1, 8), let mid = header(2, 4), let end = header(1, 2) else { return "no parts" }
        for suffix in ["V", "RA"] {
            for turns in 0..<4 {
                let doc = Harness.newDocument()
                let id = place(start, in: doc, footprintID: "PINHDR-1X08-\(suffix)")
                for _ in 0..<turns { doc.rotateFootprint(id) }
                guard var point = pad1(chip(doc, id)) else { return "\(suffix) r\(turns * 90): no pad 1" }
                for next in [mid, end] {
                    doc.setHeaderSize(id, to: next)
                    guard let now = chip(doc, id), let moved = pad1(now) else { return "no chip/pad" }
                    if abs(moved.x - point.x) > 1e-9 || abs(moved.y - point.y) > 1e-9 {
                        return "\(suffix) r\(turns * 90) → \(next.name): pad 1 \(point) → \(moved)"
                    }
                    if now.footprint?.id.hasSuffix("-\(suffix)") != true || now.partNumber != next.name {
                        return "\(suffix) r\(turns * 90) → \(next.name): footprint \(now.footprint?.id ?? "nil")"
                    }
                    if now.board?.rotation != turns * 90 { return "rotation \(now.board?.rotation ?? -1)" }
                    point = moved
                    cases += 1
                }
            }
        }
        return cases == 16 ? nil : "\(cases) cases"
    }
    print("     (\(cases) pad-1 cases)")

    // 8. Calls that change nothing.
    check("8 setHeaderSize to SN74HC00N or the same part changes nothing; 1X41 is not a part") {
        guard let part = header(1, 8), let gate = catalog.part(named: "SN74HC00N") else { return "no parts" }
        let doc = Harness.newDocument()
        let id = place(part, in: doc)
        let before = doc.chips
        doc.setHeaderSize(id, to: gate)
        if doc.chips != before { return "changed by SN74HC00N" }
        doc.setHeaderSize(id, to: part)
        if doc.chips != before { return "changed by the same part" }
        let gateID = place(gate, in: doc)
        let withGate = doc.chips
        doc.setHeaderSize(gateID, to: part)
        if doc.chips != withGate { return "a gate became a header" }
        return catalog.part(named: HeaderFamily.name(rows: 1, perRow: 41)) == nil ? nil : "1X41 exists"
    }

    // 9. Gender.
    check("9 setPartGender: Female; Neutral ignored; a gate has none; Female survives a resize") {
        guard let part = header(1, 8), let small = header(1, 4), let gate = catalog.part(named: "SN74HC00N")
        else { return "no parts" }
        let doc = Harness.newDocument()
        let id = place(part, in: doc)
        doc.setPartGender(id, "Female")
        if chip(doc, id)?.gender != "Female" { return "not Female" }
        doc.setPartGender(id, "Neutral")
        if chip(doc, id)?.gender != "Female" { return "Neutral took" }
        let gateID = place(gate, in: doc)
        doc.setPartGender(gateID, "Female")
        if chip(doc, gateID)?.gender != nil { return "gate has a gender" }
        doc.setHeaderSize(id, to: small)
        return chip(doc, id)?.gender == "Female" ? nil : "resize lost the gender"
    }

    // ---- plan-23-C: the sidebar Options and Settings ----

    // 10. Sidebar of a selected 1×8 with wires on pins 1 and 7; the menu labels.
    let sidebarDoc = Harness.newDocument()
    var sidebarID: UUID?
    check("10 sidebar of a 1x8 → /tmp/p23-sidebar-header.png; pinsPerRowLabel counts removed wire ends") {
        guard let part = header(1, 8) else { return "no 1X08" }
        let id = place(part, in: sidebarDoc)
        sidebarID = id
        guard let placed = chip(sidebarDoc, id) else { return "not placed" }
        var junctions: [Junction] = []
        var wires: [Wire] = []
        for pin in [1, 7] {
            guard let slot = placed.layout.slots.first(where: { $0.pin.designator == "\(pin)" }) else { return "no slot" }
            let tip = placed.tip(of: slot)
            let junction = Junction(point: CGPoint(x: tip.x - 40, y: tip.y))
            junctions.append(junction)
            wires.append(Wire(start: .pin(PinRef(chipID: id, index: slot.pin.index)), end: .junction(junction.id),
                              interior: []))
        }
        sidebarDoc.replaceContents(chips: sidebarDoc.chips, wires: wires, junctions: junctions, buses: [],
                                   nextReference: sidebarDoc.nextReference, board: sidebarDoc.board,
                                   texts: [], traces: [])
        if PartOptionsSection.wiredPins(of: id, in: sidebarDoc).sorted() != [1, 7] {
            return "wiredPins \(PartOptionsSection.wiredPins(of: id, in: sidebarDoc))"
        }
        let labels = [PartOptionsSection.pinsPerRowLabel(7, rows: 1, wiredPins: [1, 7]),
                      PartOptionsSection.pinsPerRowLabel(6, rows: 1, wiredPins: [1, 7]),
                      PartOptionsSection.pinsPerRowLabel(3, rows: 2, wiredPins: [1, 7, 8, 8])]
        if labels != ["7", "6 — removes 1 wire", "3 — removes 3 wires"] { return "labels \(labels)" }
        if !PartOptionsSection.applies(to: part) { return "Options does not apply" }
        sidebarDoc.select(.chip(id))
        let path = "/tmp/p23-sidebar-header.png"
        try? FileManager.default.removeItem(atPath: path)
        Harness.renderSidebar(sidebarDoc, to: path, size: CGSize(width: 320, height: 900))
        return FileManager.default.fileExists(atPath: path) ? nil : "no image"
    }

    // 11. What the four controls call, then the file and the sidebar again.
    check("11 2x5, Female, PINHDR-2X05-RA: saved in the file → /tmp/p23-sidebar-header-2x5.png") {
        guard let id = sidebarID, let part = catalog.part(named: HeaderFamily.name(rows: 2, perRow: 5))
        else { return "no chip or 2X05" }
        sidebarDoc.setHeaderSize(id, to: part)
        sidebarDoc.setPartGender(id, "Female")
        sidebarDoc.setPartFootprint(id, "PINHDR-2X05-RA")
        let text = json(try SchematicFile(document: sidebarDoc, fileURL: nil).encoded())
        for needle in ["\"partNumber\" : \"GENERIC-HEADER-2X05\"", "\"gender\" : \"Female\"",
                       "\"footprint\" : \"PINHDR-2X05-RA\""] where !text.contains(needle) {
            return "no \(needle)"
        }
        let path = "/tmp/p23-sidebar-header-2x5.png"
        try? FileManager.default.removeItem(atPath: path)
        Harness.renderSidebar(sidebarDoc, to: path, size: CGSize(width: 320, height: 900))
        return FileManager.default.fileExists(atPath: path) ? nil : "no image"
    }

    // 12–15. Settings and the drop, in a test suite (never the app's defaults).
    let suite = "p23-test"
    let defaults = UserDefaults(suiteName: suite)!
    defaults.removePersistentDomain(forName: suite)
    /// Places a dropped part the way the canvas drop does.
    func dropped(_ name: String) -> (part: String, gender: String?, footprint: String?, color: String?)? {
        guard let dropped = catalog.part(named: name) else { return nil }
        let part = NewPartOptions.headerPart(for: dropped, catalog: catalog, defaults: defaults)
        let target = Harness.newDocument()
        target.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100),
                         color: NewPartOptions.color(for: part, defaults: defaults),
                         footprintID: NewPartOptions.footprintID(for: part, defaults: defaults),
                         gender: NewPartOptions.gender(for: part, defaults: defaults))
        let placed = target.chips.last!
        return (placed.partNumber, placed.gender, placed.footprint?.id, placed.color)
    }

    check("12 empty Settings: a dropped header is GENERIC-HEADER-1X02, Male, PINHDR-1X02-V") {
        guard let got = dropped(HeaderFamily.defaultPart) else { return "no part" }
        return got.part == "GENERIC-HEADER-1X02" && got.gender == "Male" && got.footprint == "PINHDR-1X02-V"
            ? nil : "dropped \(got)"
    }

    check("13 Settings 1 / 8 / Female / Right angle → 1X08, Female, -RA; a GENERIC-LED is unchanged") {
        defaults.set(1, forKey: PreferenceKey.headerRows)
        defaults.set(8, forKey: PreferenceKey.headerPinsPerRow)
        defaults.set("Female", forKey: PreferenceKey.headerGender)
        defaults.set("Right angle", forKey: PreferenceKey.headerFootprintType)
        guard let got = dropped(HeaderFamily.defaultPart), let led = dropped("GENERIC-LED") else { return "no part" }
        if got.part != "GENERIC-HEADER-1X08" || got.gender != "Female" || got.footprint != "PINHDR-1X08-RA" {
            return "header \(got)"
        }
        return led.part == "GENERIC-LED" && led.color == "Red" && led.footprint == "LED-RADIAL-5MM" && led.gender == nil
            ? nil : "LED \(led)"
    }

    check("14 Settings 99 per row / Neutral / Sideways → 1X02, Male, -V") {
        defaults.set(99, forKey: PreferenceKey.headerPinsPerRow)
        defaults.set("Neutral", forKey: PreferenceKey.headerGender)
        defaults.set("Sideways", forKey: PreferenceKey.headerFootprintType)
        defaults.set(0, forKey: PreferenceKey.headerRows)
        guard let got = dropped(HeaderFamily.defaultPart) else { return "no part" }
        return got.part == "GENERIC-HEADER-1X02" && got.gender == "Male" && got.footprint == "PINHDR-1X02-V"
            ? nil : "dropped \(got)"
    }

    check("15 Settings window → /tmp/p23-settings.png") {
        defaults.removePersistentDomain(forName: suite)
        let path = "/tmp/p23-settings.png"
        try? FileManager.default.removeItem(atPath: path)
        Harness.renderHosted(PreferencesView().defaultAppStorage(defaults),
                             size: CGSize(width: 420, height: 700), to: path, settle: 0.5)
        return FileManager.default.fileExists(atPath: path) ? nil : "no image"
    }
    defaults.removePersistentDomain(forName: suite)

    exit(failures == 0 ? 0 : 1)
}
