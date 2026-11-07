// plan-20-I: the buffer kind in the app. The Logic drawer lists buffer
// parts under the family filter; a placed dual-supply translator's Details
// show one supply row per side; a single-supply buffer shows Kind "Buffer"
// and one supply row. Prints one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p20-h && scripts/harness/build.sh scripts/harness/buffer-app.swift /tmp/p20-h && \
//     /tmp/p20-h/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp) are p20-sidebar-1t45.png and p20-sidebar-hc244.png.
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runBufferAppChecks() }

@MainActor func runBufferAppChecks() {
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
    func listed(_ filter: DrawerFilter) -> [CatalogPart] {
        var parts: [CatalogPart] = []
        for group in Harness.catalog.groups(matching: filter) {
            parts += group.functions.flatMap(\.parts)
            for sub in group.subcategories {
                parts += sub.parts + sub.functions.flatMap(\.parts)
            }
        }
        return parts
    }
    guard let t45 = Harness.catalog.part(named: "SN74LVC1T45DBVR"),
          let hc244 = Harness.catalog.part(named: "SN74HC244N") else {
        print("FAIL catalog: no SN74LVC1T45DBVR or SN74HC244N")
        exit(1)
    }

    // 1. The Logic drawer, family LVC, lists a transceiver and a translator.
    check("1 Logic drawer, family LVC: SN74LVC245ANSR and SN74LVC1T45DBVR listed") {
        var filter = DrawerFilter()
        filter.category = "Logic"
        filter.family = "LVC"
        let names = Set(listed(filter).map(\.name))
        let missing = ["SN74LVC245ANSR", "SN74LVC1T45DBVR"].filter { !names.contains($0) }
        if !missing.isEmpty { return "missing \(missing)" }
        if names.contains("SN74HC244N") { return "family LVC lists SN74HC244N" }
        return nil
    }

    // 2. Buffers count as logic chips (family and unit filters apply).
    check("2 buffer parts are logic: family and units") {
        guard t45.isLogic, hc244.isLogic else { return "isLogic false" }
        if t45.logicFamily != "LVC" { return "1T45 family \(t45.logicFamily ?? "nil")" }
        if hc244.count != 8 { return "HC244 units \(hc244.count ?? -1)" }
        return nil
    }

    let doc = Harness.newDocument()
    doc.placeChip(part: t45, symbol: symbol(for: t45), centeredAt: CGPoint(x: 100, y: 100))
    let t45ID = doc.chips.last!.id
    doc.placeChip(part: hc244, symbol: symbol(for: hc244), centeredAt: CGPoint(x: 400, y: 100))
    let hcID = doc.chips.last!.id
    func rows(_ id: UUID) -> [String: String] {
        let chip = doc.chips.first { $0.id == id }!
        return Dictionary(PartFacts.rows(for: chip).map { ($0.label, $0.value) }, uniquingKeysWith: { a, _ in a })
    }

    // 3. A placed 1T45: Kind Buffer, Function Level shifter, two supply rows.
    check("3 SN74LVC1T45DBVR Details: Supply A (VCCA) and Supply B (VCCB) 1.65–5.5 V, no single row") {
        let r = rows(t45ID)
        print("     1T45 rows: \(PartFacts.rows(for: doc.chips.first { $0.id == t45ID }!).map { "\($0.label): \($0.value)" })")
        if r["Kind"] != "Buffer" { return "Kind \(r["Kind"] ?? "nil")" }
        if r["Function"] != "Level shifter" { return "Function \(r["Function"] ?? "nil")" }
        if r["Supply A (VCCA)"] != "1.65–5.5 V" { return "Supply A \(r["Supply A (VCCA)"] ?? "nil")" }
        if r["Supply B (VCCB)"] != "1.65–5.5 V" { return "Supply B \(r["Supply B (VCCB)"] ?? "nil")" }
        if r["Supply range"] != nil { return "also a single Supply range row" }
        return nil
    }

    // 4. A placed HC244: Kind Buffer, one supply row (unchanged form).
    check("4 SN74HC244N Details: Kind Buffer, one Supply range row 2–6 V") {
        let r = rows(hcID)
        print("     HC244 rows: \(PartFacts.rows(for: doc.chips.first { $0.id == hcID }!).map { "\($0.label): \($0.value)" })")
        if r["Kind"] != "Buffer" { return "Kind \(r["Kind"] ?? "nil")" }
        if r["Supply range"] != "2–6 V" { return "Supply range \(r["Supply range"] ?? "nil")" }
        if r.keys.contains(where: { $0.hasPrefix("Supply A") || $0.hasPrefix("Supply B") }) {
            return "has per-side rows"
        }
        return nil
    }

    // 5. The sidebar as the app draws it.
    check("5 render the sidebar for both parts") {
        doc.select(.chip(t45ID))
        Harness.renderSidebar(doc, to: "\(images)/p20-sidebar-1t45.png", size: CGSize(width: 320, height: 700))
        doc.select(.chip(hcID))
        Harness.renderSidebar(doc, to: "\(images)/p20-sidebar-hc244.png", size: CGSize(width: 320, height: 700))
        return nil
    }

    exit(failures == 0 ? 0 : 1)
}
