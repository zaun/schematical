import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// The part picker lists a library of tens of thousands of parts, so what it
/// does on each redraw must not grow with the library (the Zener group alone
/// is thousands of rows). These check the drawer's answers and that it is fast.
@MainActor
final class DrawerPerformanceTests: XCTestCase {
    private var catalog: PartCatalogStore { TestSupport.catalog }

    /// The search the drawer used before it was made fast: each of the part's
    /// fields, localized standard contains.
    private func slowMatches(_ query: String) -> Set<String> {
        Set(catalog.allParts.filter { part in
            let fields = [part.name, part.function, part.drawerLeaf, part.description] + part.drawer
            return fields.contains { $0.localizedStandardContains(query) }
        }.map(\.name))
    }

    private func names(_ groups: [PartCatalogStore.CategoryGroup]) -> Set<String> {
        Set(groups.flatMap(\.allParts).map(\.name))
    }

    private func everything() -> DrawerFilter {
        var filter = DrawerFilter()
        filter.visibility = PartVisibility(showObsolete: true, showMilitary: true, showAutomotive: true)
        return filter
    }

    func testSearchFindsWhatTheSlowSearchFound() {
        for query in ["zener", "ZENER", "74hc", "sram", "32 kb", "ω", "µf", "dip", "  nand "] {
            var filter = everything()
            filter.query = query
            let trimmed = query.trimmingCharacters(in: .whitespaces)
            XCTAssertEqual(names(catalog.groups(matching: filter)), slowMatches(trimmed), query)
        }
    }

    func testNoMatchIsEmpty() {
        var filter = everything()
        filter.query = "qqqqqqqq"
        XCTAssertTrue(catalog.groups(matching: filter).isEmpty)
    }

    func testPartsAreInNaturalOrder() {
        for category in catalog.groups(matching: everything()) {
            for function in category.functions + category.subcategories.flatMap(\.functions) {
                let names = function.parts.map(\.name)
                XCTAssertEqual(names, names.sorted { $0.localizedStandardCompare($1) == .orderedAscending },
                               function.function)
            }
        }
    }

    func testHeaderFactsMatchTheParts() {
        for category in catalog.categories {
            let parts = catalog.allParts.filter { $0.drawer.first == category }
            XCTAssertEqual(catalog.kinds(inCategory: category), Set(parts.map(\.kind.rawValue)), category)
            XCTAssertEqual(Set(catalog.families(in: category)), Set(parts.compactMap(\.logicFamily)), category)
            XCTAssertEqual(catalog.logicCounts(in: category),
                           Array(Set(parts.filter(\.isLogic).compactMap(\.count))).sorted(), category)
            XCTAssertEqual(Set(catalog.colors(in: category)), Set(parts.flatMap(\.colors)), category)
        }
        XCTAssertTrue(catalog.isTactile(group: ["Switch", "Tactile"]))
        XCTAssertFalse(catalog.isTactile(group: ["Logic"]))
        XCTAssertEqual(Set(catalog.packages), Set(catalog.allParts.flatMap(\.allPackages)))
    }

    /// The drawer with its biggest group open builds in a fraction of a
    /// second: only the rows near the top are built.
    func testBiggestOpenGroupBuildsFast() {
        let groups: Set<String> = ["category:Discrete", "subcategory:Discrete/Diodes",
                                   "function:Discrete/Diodes/Zener"]
        let host = NSHostingView(rootView: ChipDrawerView(catalog: catalog, openGroups: groups))
        host.frame = NSRect(x: 0, y: 0, width: 360, height: 800)
        let window = NSWindow(contentRect: host.frame, styleMask: [.titled], backing: .buffered, defer: false)
        window.contentView = host
        let start = CFAbsoluteTimeGetCurrent()
        host.layoutSubtreeIfNeeded()
        host.displayIfNeeded()
        RunLoop.current.run(until: Date().addingTimeInterval(0.2))
        host.layoutSubtreeIfNeeded()
        let seconds = CFAbsoluteTimeGetCurrent() - start
        print("PERF open Zener group, first layout (incl. 0.2 s run loop): \(seconds) s")
        XCTAssertLessThan(seconds, 2.0)
    }
}
