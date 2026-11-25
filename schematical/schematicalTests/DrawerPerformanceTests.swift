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
                if function.parts.allSatisfy({ $0.drawerValue != nil }) {
                    // A crystal leaf goes by frequency, an inductor leaf by inductance, a fuse
                    // leaf by rated current, a buzzer leaf by frequency and a relay leaf by
                    // coil voltage, then by name (plan-44-G, plan-47-M).
                    let want = function.parts.sorted {
                        ($0.drawerValue ?? 0) != ($1.drawerValue ?? 0)
                            ? ($0.drawerValue ?? 0) < ($1.drawerValue ?? 0)
                            : $0.name.localizedStandardCompare($1.name) == .orderedAscending
                    }.map(\.name)
                    XCTAssertEqual(names, want, function.function)
                    continue
                }
                if function.parts.allSatisfy(\.isCardSlot) {
                    // A card-slot leaf goes by pin count, then by name (plan-54-O).
                    let want = function.parts.sorted {
                        $0.pins.count != $1.pins.count
                            ? $0.pins.count < $1.pins.count
                            : $0.name.localizedStandardCompare($1.name) == .orderedAscending
                    }.map(\.name)
                    XCTAssertEqual(names, want, function.function)
                    continue
                }
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
        XCTAssertTrue(catalog.isTactile(group: ["Switches & Relays", "Tactile"]))
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

    /// plan-54-O: the Connector › Card slot branch, opened down to its biggest function group (the card slots
    /// go by pin count, up to 694 pins each), builds in the same time class as the Zener group.
    func testCardSlotGroupBuildsFast() throws {
        let connector = try XCTUnwrap(catalog.groups(matching: everything()).first { $0.category == "Connector" })
        let slot = try XCTUnwrap(connector.subcategories.first { $0.subcategory == "Card slot" })
        let leaf = try XCTUnwrap(slot.subgroups.max { $0.allParts.count < $1.allParts.count })
        let function = try XCTUnwrap(leaf.functions.max { $0.parts.count < $1.parts.count })
        let groups: Set<String> = ["category:Connector", "subcategory:Connector/Card slot",
                                   "subcategory:\(leaf.path.joined(separator: "/"))",
                                   "function:\(leaf.path.joined(separator: "/"))/\(function.function)"]
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
        print("PERF open Card slot group \(function.function) (\(function.parts.count) parts), first layout (incl. 0.2 s run loop): \(seconds) s")
        XCTAssertLessThan(seconds, 2.0)
    }

    /// Closing a category forgets its open sub-groups and no other category's.
    func testClosingACategoryClosesItsGroups() {
        var open: Set<String> = ["subcategory:Discrete/Diodes", "function:Discrete/Diodes/Zener",
                                 "function:Discrete/Diodes/Zener/8-bit", "subcategory:Memory/SRAM",
                                 "function:Logic/Gates", "subcategory:DiscreteX/Other"]
        open.closeGroups(inCategory: "Discrete")
        XCTAssertEqual(open, ["subcategory:Memory/SRAM", "function:Logic/Gates", "subcategory:DiscreteX/Other"])
    }
}
