// plan-22-E: the arith kind in the app. The Logic drawer, family LS, lists
// the SN74LS85N comparator and the SN74LS283N adder; arith parts are logic
// chips; a placed '283 shows Kind "Arithmetic" and Family "LS". Prints one
// ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p22-h && scripts/harness/build.sh scripts/harness/arith-app.swift /tmp/p22-h && \
//     /tmp/p22-h/H.app/Contents/MacOS/harness [image dir]
//
// Image (default dir /tmp): p22-sidebar-ls283.png.
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runArithAppChecks() }

@MainActor func runArithAppChecks() {
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
    guard let ls283 = Harness.catalog.part(named: "SN74LS283N"),
          let ls85 = Harness.catalog.part(named: "SN74LS85N") else {
        print("FAIL catalog: no SN74LS283N or SN74LS85N")
        exit(1)
    }

    // 1. The Logic drawer, family LS, lists an arith comparator and adder.
    check("1 Logic drawer, family LS: SN74LS85N and SN74LS283N listed") {
        var filter = DrawerFilter()
        filter.category = "Logic"
        filter.family = "LS"
        let names = Set(listed(filter).map(\.name))
        let missing = ["SN74LS85N", "SN74LS283N"].filter { !names.contains($0) }
        if !missing.isEmpty { return "missing \(missing)" }
        if names.contains("CD74HC283E") { return "family LS lists CD74HC283E" }
        return nil
    }

    // 2. Arith parts are logic chips (family and unit filters apply).
    check("2 arith parts are logic: family LS, 1 unit") {
        guard ls283.isLogic, ls85.isLogic else { return "isLogic false" }
        if ls283.logicFamily != "LS" { return "'283 family \(ls283.logicFamily ?? "nil")" }
        if ls283.count != 1 { return "'283 units \(ls283.count ?? -1)" }
        return nil
    }

    let doc = Harness.newDocument()
    doc.placeChip(part: ls283, symbol: symbol(for: ls283), centeredAt: CGPoint(x: 100, y: 100))
    let id = doc.chips.last!.id

    // 3. A placed '283: Kind Arithmetic, Family LS.
    check("3 SN74LS283N Details: Kind Arithmetic, Family LS") {
        let chip = doc.chips.first { $0.id == id }!
        let all = PartFacts.rows(for: chip)
        print("     LS283 rows: \(all.map { "\($0.label): \($0.value)" })")
        let r = Dictionary(all.map { ($0.label, $0.value) }, uniquingKeysWith: { a, _ in a })
        if r["Kind"] != "Arithmetic" { return "Kind \(r["Kind"] ?? "nil")" }
        if r["Family"] != "LS" { return "Family \(r["Family"] ?? "nil")" }
        return nil
    }

    // 4. The sidebar as the app draws it.
    check("4 render the sidebar for the '283") {
        doc.select(.chip(id))
        Harness.renderSidebar(doc, to: "\(images)/p22-sidebar-ls283.png", size: CGSize(width: 320, height: 700))
        return nil
    }

    exit(failures == 0 ? 0 : 1)
}
