import AppKit
import XCTest
@testable import schematical

/// The drawer's rows and open groups, its header menus, a part's right-click
/// menu and the status footer (plan-68-AB: the types `ChipDrawerView` uses).
@MainActor
final class DrawerLayoutTests: XCTestCase {
    private var catalog: PartCatalogStore { TestSupport.catalog }
    private static let adc = "function:Analog/Data converters/ADC"

    private func everything() -> DrawerFilter {
        var filter = DrawerFilter()
        filter.visibility = PartVisibility(showObsolete: true, showMilitary: true, showAutomotive: true)
        return filter
    }

    private func layout(_ open: DrawerOpenGroups) -> DrawerLayout {
        DrawerMemo.flatten(catalog.groups(matching: everything()), openCategory: open.category, toggled: open.toggled)
    }

    // MARK: Rows

    func testClosedDrawerListsOnlyCategories() {
        let groups = catalog.groups(matching: everything())
        let rows = layout(DrawerOpenGroups())
        XCTAssertEqual(rows.items.map(\.id), groups.map { "category:\($0.category)" })
        XCTAssertTrue(rows.items.allSatisfy { $0.depth == 0 && !$0.isOpen })
        XCTAssertEqual(rows.tops.last, CGFloat(groups.count) * DrawerRowItem.headerHeight)
    }

    func testOpenConverterGroupsListBitsAndParts() throws {
        let open = DrawerOpenGroups(["category:Analog", "subcategory:Analog/Data converters", Self.adc,
                                     "\(Self.adc)/8-bit"])
        let rows = layout(open)
        let ids = rows.items.map(\.id)
        let bits = try XCTUnwrap(ids.firstIndex(of: "\(Self.adc)/8-bit"))
        XCTAssertEqual(rows.items[bits].depth, 3)
        XCTAssertTrue(rows.items[bits].isOpen)
        let part = rows.items[bits + 1]
        XCTAssertTrue(part.id.hasPrefix("part:"))
        XCTAssertEqual(part.depth, 4)
        XCTAssertEqual(part.height, DrawerRowItem.partHeight)
        XCTAssertTrue(ids.contains("\(Self.adc)/10-bit"), "a closed bits group still lists")
        // Each row begins where the one above ends.
        for index in rows.items.indices {
            XCTAssertEqual(rows.tops[index + 1] - rows.tops[index], rows.items[index].height)
        }

        // A subcategory lists its own parts.
        let display = layout(DrawerOpenGroups(["category:Displays & Indicators",
                                               "subcategory:Displays & Indicators/7-Segment"]))
        let seg = try XCTUnwrap(display.items.firstIndex { $0.id == "subcategory:Displays & Indicators/7-Segment" })
        XCTAssertTrue(display.items[seg + 1].id.hasPrefix("part:"))
    }

    func testCategoryFunctionsAndSubgroups() throws {
        // Vintage › Display tubes has sub-groups.
        let vintage = layout(DrawerOpenGroups(["category:Vintage", "subcategory:Vintage/Display tubes",
                                               "subcategory:Vintage/Display tubes/VFD"]))
        let vfd = try XCTUnwrap(vintage.items.first { $0.id == "subcategory:Vintage/Display tubes/VFD" })
        XCTAssertEqual(vfd.depth, 2)
        XCTAssertTrue(vfd.isOpen)
        // A category's own function groups (no second level) list at depth 1.
        let group = try XCTUnwrap(catalog.groups(matching: everything()).first { !$0.functions.isEmpty })
        let function = try XCTUnwrap(group.functions.first)
        let key = "function:\(group.category)/\(function.function)"
        let rows = layout(DrawerOpenGroups(["category:\(group.category)", key]))
        let row = try XCTUnwrap(rows.items.firstIndex { $0.id == key })
        XCTAssertEqual(rows.items[row].depth, 1)
        XCTAssertEqual(rows.items[row + 1].id, "part:\(try XCTUnwrap(function.parts.first).name)")
    }

    func testRowsInAWindow() {
        let empty = DrawerLayout()
        XCTAssertEqual(empty.rows(from: 0, to: 100).range, 0..<0)
        let rows = layout(DrawerOpenGroups())
        let window = rows.rows(from: 40, to: 100)
        XCTAssertEqual(window.range, 1..<3)
        XCTAssertEqual(window.top, DrawerRowItem.headerHeight)
        XCTAssertEqual(window.bottom, rows.tops[rows.items.count] - rows.tops[3])
        XCTAssertEqual(rows.rows(from: -300, to: 10).range, 0..<1)
        XCTAssertEqual(rows.rows(from: 1e6, to: 2e6).range.isEmpty, true)
    }

    func testRowEquality() {
        let a = DrawerRowItem(id: "category:A", kind: .category("A"), depth: 0, isOpen: false)
        XCTAssertEqual(a, DrawerRowItem(id: "category:A", kind: .category("B"), depth: 0, isOpen: false))
        XCTAssertNotEqual(a, DrawerRowItem(id: "category:A", kind: .category("A"), depth: 0, isOpen: true))
        XCTAssertEqual(a.height, DrawerRowItem.headerHeight)
    }

    func testScrollWindowSteps() {
        let first = ScrollWindow()
        XCTAssertEqual([first.top, first.bottom], [0, 1200])
        let window = ScrollWindow(offset: 250, height: 500)
        XCTAssertEqual([window.top, window.bottom], [200, 800])
        let clamped = ScrollWindow(offset: 5000, height: 500, content: 1000)
        XCTAssertEqual([clamped.top, clamped.bottom], [500, 1000])
        XCTAssertEqual(ScrollWindow(offset: -50, height: 100).top, 0)
    }

    func testMemoRebuildsOnlyWhenItsInputsChange() {
        let memo = DrawerMemo()
        var filter = everything()
        let groups = memo.groups(for: catalog, filter: filter)
        let first = memo.layout(groups: groups, openCategory: nil, toggled: [])
        XCTAssertEqual(memo.layout(groups: groups, openCategory: nil, toggled: []).items.count, first.items.count)
        let open = memo.layout(groups: groups, openCategory: "Analog", toggled: [])
        XCTAssertGreaterThan(open.items.count, first.items.count)
        filter.query = "qqqqqqqq"
        let none = memo.groups(for: catalog, filter: filter)
        XCTAssertTrue(none.isEmpty)
        XCTAssertTrue(memo.layout(groups: none, openCategory: "Analog", toggled: []).items.isEmpty,
                      "a new filter rebuilds the rows")
    }

    // MARK: Open groups

    func testOpeningGroups() throws {
        var open = DrawerOpenGroups(["category:Analog", "subcategory:Analog/Data converters"])
        XCTAssertEqual(open.category, "Analog")
        XCTAssertEqual(open.toggled, ["subcategory:Analog/Data converters"])

        let logic = DrawerRowItem(id: "category:Logic", kind: .category("Logic"), depth: 0, isOpen: false)
        open.toggle(logic)
        XCTAssertEqual(open.category, "Logic")
        XCTAssertEqual(open.toggled, [], "the closed category forgets its groups")

        let group = try XCTUnwrap(layout(open).items.first { item in
            if case .category = item.kind { return false }
            return true
        })
        open.toggle(group)
        XCTAssertEqual(open.toggled, [group.id])
        open.toggle(DrawerRowItem(id: group.id, kind: group.kind, depth: group.depth, isOpen: true))
        XCTAssertEqual(open.toggled, [])

        let part = DrawerRowItem(id: "part:X", kind: .part(try TestSupport.part("SN74HC00N")), depth: 2, isOpen: false)
        open.toggle(part)
        XCTAssertEqual(open, DrawerOpenGroups(["category:Logic"]))

        // Closing the open category leaves none open; closing another leaves it.
        open.toggle(DrawerRowItem(id: "category:Analog", kind: .category("Analog"), depth: 0, isOpen: false))
        open.toggle(DrawerRowItem(id: "category:Logic", kind: .category("Logic"), depth: 0, isOpen: false))
        XCTAssertEqual(open.category, "Logic")
        open.toggle(DrawerRowItem(id: "category:Analog", kind: .category("Analog"), depth: 0, isOpen: true))
        XCTAssertEqual(open.category, "Logic")
        open.toggle(DrawerRowItem(id: "category:Logic", kind: .category("Logic"), depth: 0, isOpen: true))
        XCTAssertNil(open.category)
    }

    // MARK: Header menus

    func testHeaderMenus() throws {
        XCTAssertEqual(DrawerHeaderMenu.forCategory(kinds: [PART_GATE.rawValue]), .logic)
        XCTAssertEqual(DrawerHeaderMenu.forCategory(kinds: [PART_REGISTER.rawValue, PART_SWITCH.rawValue]), .logic)
        XCTAssertEqual(DrawerHeaderMenu.forCategory(kinds: [PART_SWITCH.rawValue]), .switches)
        XCTAssertEqual(DrawerHeaderMenu.forCategory(kinds: [PART_7SEG.rawValue]), .colors)
        XCTAssertEqual(DrawerHeaderMenu.forCategory(kinds: [PART_LED.rawValue]), .colors)
        XCTAssertEqual(DrawerHeaderMenu.forCategory(kinds: [PART_SRAM.rawValue]), .none)

        let groups = catalog.groups(matching: everything())
        func group(_ category: String, _ name: String) throws -> PartCatalogStore.SubcategoryGroup {
            try XCTUnwrap(groups.first { $0.category == category }?.subcategories.first { $0.subcategory == name })
        }
        XCTAssertEqual(DrawerHeaderMenu.forGroup(try group("Displays & Indicators", "7-Segment"), catalog: catalog), .displays)
        XCTAssertEqual(DrawerHeaderMenu.forGroup(try group("Switches & Relays", "Tactile"), catalog: catalog), .tactile)
        XCTAssertEqual(DrawerHeaderMenu.forGroup(try group("Switches & Relays", "DIP"), catalog: catalog), .none)
        XCTAssertEqual(DrawerHeaderMenu.forGroup(try group("Vintage", "Display tubes"), catalog: catalog), .none,
                       "a group with sub-groups leaves the menus to them")

        XCTAssertEqual(DrawerHeaderMenu.countLabel(1, unit: "gate"), "1 gate")
        XCTAssertEqual(DrawerHeaderMenu.countLabel(4, unit: "gate"), "4 gates")
    }

    // MARK: Status

    func testStatus() {
        XCTAssertEqual(DrawerStatus(.notLoaded, partCount: 0), DrawerStatus(.notLoaded, partCount: 9))
        XCTAssertNil(DrawerStatus(.notLoaded, partCount: 0).icon)
        XCTAssertEqual(DrawerStatus(.notLoaded, partCount: 0).text, "Loading backend")
        let loaded = DrawerStatus(.loaded, partCount: 12)
        XCTAssertEqual(loaded.text, "12 parts from backend")
        XCTAssertEqual(loaded.icon, "checkmark.circle.fill")
        XCTAssertFalse(loaded.isFailure)
        let failed = DrawerStatus(.failed("No backend"), partCount: 0)
        XCTAssertEqual(failed.text, "No backend")
        XCTAssertTrue(failed.isFailure)
        XCTAssertEqual(failed.icon, "exclamationmark.triangle.fill")

        XCTAssertEqual(DrawerStatus.emptyDescription(.notLoaded), "Loading the part library…")
        XCTAssertEqual(DrawerStatus.emptyDescription(.loaded), "The backend reports no supported parts.")
        XCTAssertEqual(DrawerStatus.emptyDescription(.failed("No backend")), "No backend")
    }

    func testSubtitles() throws {
        XCTAssertEqual(DrawerPartMenu.subtitle(for: try TestSupport.part(HeaderFamily.defaultPart)), HeaderFamily.subtitle)
        let gate = try TestSupport.part("SN74HC00N")
        XCTAssertTrue(DrawerPartMenu.subtitle(for: gate).hasPrefix(gate.package), DrawerPartMenu.subtitle(for: gate))
        let choices = try XCTUnwrap(catalog.allParts.first { !$0.footprintChoices.isEmpty && $0.kind != PART_HEADER })
        XCTAssertTrue(DrawerPartMenu.subtitle(for: choices).hasPrefix("\(choices.footprintChoices.count) footprints"))
    }
}
