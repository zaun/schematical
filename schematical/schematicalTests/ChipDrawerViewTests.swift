import AppKit
import SwiftUI
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// The part drawer as shown (plan-68-AB): its groups' headers and menus,
/// search, the empty states and the status footer.
@MainActor
final class ChipDrawerViewTests: XCTestCase {
    private var probes: [ViewProbe] = []
    /// Stores made by tests, kept for the life of the process (see `UserLibraryCatalogTests`).
    private static var kept: [PartCatalogStore] = []
    private static let adc = "function:Analog/Data converters/ADC"

    override func setUp() {
        super.setUp()
        // The test host stays up when the test closes its last window.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() {
        probes.forEach { $0.close() }
        probes = []
        super.tearDown()
    }

    private func show(_ catalog: PartCatalogStore = TestSupport.catalog, open: Set<String> = []) -> ViewProbe {
        let probe = ViewProbe(ChipDrawerView(catalog: catalog, openGroups: open), size: NSSize(width: 380, height: 900))
        probes.append(probe)
        return probe
    }

    func testClosedDrawerShowsEveryCategoryWithItsMenus() throws {
        let probe = show()
        let catalog = TestSupport.catalog
        XCTAssertTrue(probe.has("Logic"), "\(probe.labels())")
        XCTAssertTrue(probe.has("All families"), "a logic category's menus: \(probe.labels())")
        XCTAssertTrue(probe.has("All counts"), "\(probe.labels())")
        XCTAssertTrue(probe.has("All colors"), "an LED category's menu: \(probe.labels())")
        XCTAssertTrue(probe.has("All packages"), "\(probe.labels())")
        XCTAssertTrue(probe.has("All voltages"), "\(probe.labels())")
        XCTAssertTrue(probe.has("\(catalog.partCount) parts from backend"), "\(probe.labels())")
    }

    func testOpenGroupsShowTheirMenusAndParts() throws {
        let probe = show(open: ["category:Analog", "subcategory:Analog/Data converters", Self.adc, "\(Self.adc)/8-bit"])
        XCTAssertTrue(probe.has("All channels"), "the ADC group's menu: \(probe.labels())")
        XCTAssertTrue(probe.has("8-bit"), "\(probe.labels())")
        let part = try XCTUnwrap(TestSupport.catalog.groups(matching: DrawerFilter()).first { $0.category == "Analog" }?
            .subcategories.first { $0.subcategory == "Data converters" }?.functions.first { $0.function == "ADC" }?
            .bitGroups.first { $0.bits == 8 }?.parts.first)
        XCTAssertTrue(probe.has(part.name), "\(probe.labels())")
        XCTAssertTrue(probe.has(DrawerPartMenu.subtitle(for: part)), "\(probe.labels())")

        let displays = show(open: ["category:Displays & Indicators"])
        XCTAssertTrue(displays.has("All sizes"), "the 7-segment group's menus: \(displays.labels())")
        let switches = show(open: ["category:Switches & Relays"])
        XCTAssertTrue(switches.has("All actuators"), "the tactile group's menu: \(switches.labels())")
        let vintage = show(open: ["category:Vintage", "subcategory:Vintage/Display tubes"])
        XCTAssertTrue(vintage.has("Display tubes"), "\(vintage.labels())")
    }

    func testSearchAndItsEmptyState() throws {
        let probe = show()
        let search = try XCTUnwrap(probe.textFields().first)
        XCTAssertFalse(probe.has("Clear search"))
        probe.type("qqqqqqqq", into: search)
        XCTAssertTrue(probe.wait { probe.has("qqqqqqqq") && probe.has("Clear search") }, "\(probe.labels())")
        XCTAssertFalse(probe.has("Logic"), "nothing matches: \(probe.labels())")
        try probe.press("Clear search")
        XCTAssertTrue(probe.wait { probe.has("Logic") }, "\(probe.labels())")
        XCTAssertEqual(search.stringValue, "")
    }

    func testALibraryNotLoadedYet() {
        let store = PartCatalogStore()
        Self.kept.append(store)
        let probe = show(store)
        XCTAssertTrue(probe.has("No parts yet"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Loading the part library…"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Loading backend"), "\(probe.labels())")
    }

    func testScrollingBuildsTheRowsInView() throws {
        let probe = show(open: ["category:Discrete", "subcategory:Discrete/Diodes", "function:Discrete/Diodes/Zener"])
        let scroll = try XCTUnwrap(probe.views(NSScrollView.self).max { $0.documentView?.frame.height ?? 0 < $1.documentView?.frame.height ?? 0 })
        let height = scroll.documentView?.frame.height ?? 0
        XCTAssertGreaterThan(height, 5000, "the open Zener group is tall")
        let before = Set(probe.labels())
        scroll.contentView.scroll(to: NSPoint(x: 0, y: height / 2))
        scroll.reflectScrolledClipView(scroll.contentView)
        probe.settle(0.5)
        let after = Set(probe.labels())
        XCTAssertFalse(after.subtracting(before).isEmpty, "rows halfway down are built once scrolled to")
    }

    func testEqualityIsTheCatalog() {
        let store = PartCatalogStore()
        Self.kept.append(store)
        XCTAssertTrue(ChipDrawerView(catalog: TestSupport.catalog) == ChipDrawerView(catalog: TestSupport.catalog, openGroups: ["category:Logic"]))
        XCTAssertFalse(ChipDrawerView(catalog: TestSupport.catalog) == ChipDrawerView(catalog: store))
    }

    func testDragPayloadsAreJSON() {
        let part = NSItemProvider()
        part.register(ChipPartDragItem(partNumber: "SN74HC00N"))
        XCTAssertEqual(part.registeredContentTypes, [.json])
        let wire = NSItemProvider()
        wire.register(BusWireDragItem(bus: UUID(), wire: UUID()))
        XCTAssertEqual(wire.registeredContentTypes, [.json])
    }

    /// The groups to open to show the first part `matching`, and that part.
    private func groupsShowing(_ matching: (CatalogPart) -> Bool) throws -> (Set<String>, CatalogPart) {
        func search(_ group: PartCatalogStore.SubcategoryGroup, _ keys: Set<String>) -> (Set<String>, CatalogPart)? {
            let path = group.path.joined(separator: "/")
            let open = keys.union(["subcategory:\(path)"])
            if let part = group.parts.first(where: matching) { return (open, part) }
            for function in group.functions {
                if let part = function.parts.first(where: matching), function.bitGroups.isEmpty {
                    return (open.union(["function:\(path)/\(function.function)"]), part)
                }
            }
            for subgroup in group.subgroups {
                if let found = search(subgroup, open) { return found }
            }
            return nil
        }
        for category in TestSupport.catalog.groups(matching: DrawerFilter()) {
            for subcategory in category.subcategories {
                if let found = search(subcategory, ["category:\(category.category)"]) { return found }
            }
        }
        throw XCTSkip("no such part in the drawer")
    }

    func testAPartRowShowsItsValueAndItsMenuCopiesItToTheEditor() throws {
        let (open, part) = try groupsShowing { $0.trailingLabel != nil && DrawerPartMenu.items(for: $0, listing: nil) == [.copyToEditor] }
        var opened: [PartEditorTarget] = []
        let probe = ViewProbe(ChipDrawerView(catalog: TestSupport.catalog, openGroups: open) { opened.append($0) },
                              size: NSSize(width: 380, height: 900))
        probes.append(probe)
        let label = try XCTUnwrap(part.trailingLabel)
        XCTAssertTrue(probe.wait { probe.has(label) }, "\(part.name) shows \(label): \(probe.labels())")

        let row = try probe.element(part.name)
        let menu = try XCTUnwrap(probe.contextMenu(on: row), "no context menu on \(part.name)")
        XCTAssertEqual(menu.items.map(\.title), ["Copy to Editor…"])
        try probe.choose("Copy to Editor…", in: menu)
        XCTAssertTrue(probe.wait { !opened.isEmpty }, "the part editor opens")
        XCTAssertEqual(opened.map(\.source), [.copy(name: part.name)])
    }
}
