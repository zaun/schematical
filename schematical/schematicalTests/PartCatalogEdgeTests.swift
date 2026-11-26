import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-68-W: the part catalog's rarer turns — part files missing or
/// unreadable, a store whose load fails, each error's text, every drawer
/// filter keeping only what it names, a User Library file that clashes, an
/// update that brings a part into the drawer, removals the folder refuses,
/// and the small values (ids, defaults) the catalog's types carry.
@MainActor
final class PartCatalogEdgeTests: XCTestCase {
    private static var kept: [PartCatalogStore] = []
    private var directory: URL!

    override func setUp() {
        super.setUp()
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("PartCatalogEdgeTests-\(UUID().uuidString)")
        try? FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
    }

    override func tearDown() {
        try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: directory.path)
        try? FileManager.default.removeItem(at: directory)
        super.tearDown()
    }

    private func store(attached: Bool = true) -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        Self.kept.append(store)
        if attached { store.attachUserLibrary(UserPartLibrary(directory: directory)) }
        return store
    }

    private func write(_ name: String, _ text: String) throws -> URL {
        let url = directory.appendingPathComponent(name)
        try Data(text.utf8).write(to: url)
        return url
    }

    private func bundled(_ name: String) throws -> URL {
        try XCTUnwrap(Bundle.main.url(forResource: name, withExtension: "json"))
    }

    // MARK: Loading

    func testMissingOrBrokenFilesAreLoadErrors() throws {
        func error(_ body: () throws -> Void) -> String? {
            do { try body(); return nil } catch { return error.localizedDescription }
        }
        let unavailable = "The app has no part files"
        XCTAssertEqual(error { _ = try BackendCatalog.load(into: PartLibraryHandle(), files: []) }, unavailable)
        XCTAssertEqual(error { _ = try BackendCatalog.load(into: PartLibraryHandle(), files: [try bundled("footprints")]) },
                       unavailable, "no datasheets file")
        XCTAssertEqual(error { try BackendCatalog.loadSupportFiles(into: PartLibraryHandle(), datasheets: nil, footprints: nil) },
                       unavailable)
        let garbage = try write("datasheets.json", "{ not json")
        let refused = error { _ = try BackendCatalog.load(into: PartLibraryHandle(), files: [garbage, try bundled("footprints")]) }
        XCTAssertTrue(refused?.hasPrefix("A part file could not be read (") ?? false, refused ?? "")
        let binary = directory.appendingPathComponent("bad/datasheets.json")
        try FileManager.default.createDirectory(at: binary.deletingLastPathComponent(), withIntermediateDirectories: true)
        try Data([0xFF, 0xFE, 0x00, 0xD8]).write(to: binary)
        let unreadable = error { try BackendCatalog.loadSupportFiles(into: PartLibraryHandle(), datasheets: binary,
                                                                     footprints: try bundled("footprints")) }
        XCTAssertTrue(unreadable?.contains("datasheets.json: ") ?? false, unreadable ?? "")
        XCTAssertNil(error { try BackendCatalog.loadSupportFiles(into: PartLibraryHandle()) })
    }

    func testAStoreWhoseLoadFailsSaysWhy() {
        let failing = PartCatalogStore()
        Self.kept.append(failing)
        failing.load { _ in throw BackendCatalog.LoadError.unavailable }
        XCTAssertEqual(failing.loadState, .failed("The app has no part files"))
        XCTAssertNil(failing.part(named: "9V"), "no rails are made before the catalog loads")
    }

    func testEachRegistrationErrorsText() {
        XCTAssertEqual(PartRegistrationError.backend("b").errorDescription, "b")
        XCTAssertEqual(PartRegistrationError.notAPartFile("x").errorDescription, "This is not a part file (x).")
        XCTAssertEqual(PartRegistrationError.notInUserLibrary(name: "LIB").errorDescription,
                       "“LIB” is no longer in your User Library. Use Save As… to keep it.")
        XCTAssertEqual(PartRegistrationError.identityChanged(detail: "kind").errorDescription, "kind")
        XCTAssertEqual(PartRegistrationError.libraryWrite("full").errorDescription,
                       "The part could not be saved in your User Library: full")
        XCTAssertEqual(PartRegistrationError.noUserLibrary.errorDescription, "There is no User Library to add the part to.")
        XCTAssertEqual(BackendCatalog.LoadError.malformed("m").errorDescription, "A part file could not be read (m)")
    }

    // MARK: Drawer filters

    func testEachFilterKeepsOnlyWhatItNames() throws {
        let catalog = TestSupport.catalog
        func parts(_ change: (inout DrawerFilter) -> Void) -> [CatalogPart] {
            var filter = DrawerFilter()
            change(&filter)
            return catalog.groups(matching: filter).flatMap(\.allParts)
        }
        let all = parts { _ in }
        let soic = parts { $0.package = "SOIC" }
        XCTAssertFalse(soic.isEmpty)
        XCTAssertLessThan(soic.count, all.count)
        XCTAssertTrue(soic.allSatisfy { $0.allPackages.contains("SOIC") })
        let quads = parts { $0.gateCount = 4 }
        XCTAssertTrue(quads.filter(\.isLogic).allSatisfy { $0.count == 4 })
        XCTAssertLessThan(quads.filter(\.isLogic).count, all.filter(\.isLogic).count)
        let twelve = parts { $0.voltageMV = 12_000 }
        for part in twelve {
            switch part.details {
            case let .dipSwitch(_, _, rated), let .tactileSwitch(_, rated, _, _, _):
                XCTAssertTrue(rated == 0 || rated >= 12_000, part.name)
            default: break
            }
        }
        XCTAssertLessThan(twelve.filter { $0.kind == PART_SWITCH }.count, all.filter { $0.kind == PART_SWITCH }.count)
        let red = parts { $0.color = "Red" }
        XCTAssertTrue(red.allSatisfy { $0.colors.isEmpty || $0.colors.contains("Red") })
        XCTAssertLessThan(red.filter { !$0.colors.isEmpty }.count, all.filter { !$0.colors.isEmpty }.count)
        let displays = all.compactMap(\.display)
        let digits = try XCTUnwrap(displays.first).digits
        let byDigits = parts { $0.displayDigits = digits }.compactMap(\.display)
        XCTAssertTrue(byDigits.allSatisfy { $0.digits == digits })
        XCTAssertLessThan(byDigits.count, displays.count)
        let size = try XCTUnwrap(displays.first).size
        let bySize = parts { $0.displaySize = size }.compactMap(\.display)
        XCTAssertTrue(bySize.allSatisfy { $0.size == size })
        XCTAssertLessThan(bySize.count, displays.count)
    }

    /// A rail above a switch's rating keeps the switch out.
    func testASwitchRatedBelowTheRailIsLeftOut() {
        var filter = DrawerFilter()
        filter.voltageMV = 250_000
        let parts = TestSupport.catalog.groups(matching: filter).flatMap(\.allParts)
        let all = TestSupport.allParts
        func rated(_ part: CatalogPart) -> UInt32? {
            switch part.details {
            case let .dipSwitch(_, _, mv), let .tactileSwitch(_, mv, _, _, _): mv > 0 ? mv : nil
            default: nil
            }
        }
        XCTAssertTrue(all.contains { $0.isTactileSwitch && rated($0) != nil }, "a rated tactile switch")
        XCTAssertTrue(all.contains { if case .dipSwitch = $0.details { rated($0) != nil } else { false } }, "a rated DIP switch")
        XCTAssertFalse(parts.contains { rated($0).map { $0 < 250_000 } ?? false })
    }

    /// A group made without bit groups or subgroups has none.
    func testGroupsDefaultToNoBitGroupsOrSubgroups() throws {
        let part = try TestSupport.part("SN74HC00N")
        let function = PartCatalogStore.FunctionGroup(function: "NAND", parts: [part])
        XCTAssertEqual(function.bitGroups, [])
        let group = PartCatalogStore.SubcategoryGroup(subcategory: "Gates", path: ["Logic", "Gates"], functions: [function])
        XCTAssertEqual(group.subgroups, [])
        XCTAssertEqual(group.allParts, [part])
    }

    func testTheDrawersGroupsHaveIDs() {
        let groups = TestSupport.catalog.groups(matching: DrawerFilter())
        XCTAssertEqual(Set(groups.map(\.id)).count, groups.count)
        for group in groups {
            XCTAssertEqual(group.id, group.category)
            for subgroup in group.subcategories {
                XCTAssertEqual(subgroup.id, subgroup.path.joined(separator: "/"))
                for function in subgroup.functions {
                    XCTAssertEqual(function.id, function.function)
                    for bits in function.bitGroups { XCTAssertEqual(bits.id, bits.bits) }
                }
                for inner in subgroup.subgroups { XCTAssertFalse(inner.id.isEmpty) }
            }
        }
    }

    // MARK: User Library

    func testALibraryFileClashingWithALoadedPartIsAProblem() throws {
        let uuid = "B1000000-0000-4000-8000-000000000001"
        let catalog = store(attached: false)
        try catalog.register(fileText: PartRegistrationTests.fileText(uuid: uuid, name: "CLASH-A"), source: "a.schpart")
        _ = try write("\(uuid).schpart", PartRegistrationTests.fileText(uuid: uuid, name: "CLASH-A", description: "Something else"))
        catalog.attachUserLibrary(UserPartLibrary(directory: directory))
        XCTAssertEqual(catalog.userLibraryProblems.map(\.reason), ["Another part with this identity is already loaded."])
    }

    /// A library part kept out of the drawer (no drawer path) joins it when
    /// an update gives it one.
    func testAnUpdateBringsAPartIntoTheDrawer() throws {
        let uuid = "B2000000-0000-4000-8000-000000000002"
        let catalog = store()
        let hidden = try catalog.addToUserLibrary(fileText: PartRegistrationTests.fileText(uuid: uuid, name: "LIB-HIDDEN", drawer: []),
                                                  source: "h.schpart")
        XCTAssertTrue(hidden.drawer.isEmpty)
        XCTAssertFalse(catalog.groups(matching: DrawerFilter()).flatMap(\.allParts).contains { $0.uuid == uuid })
        let shown = try catalog.updateUserLibraryPart(uuid: uuid, fileText: PartRegistrationTests.fileText(uuid: uuid, name: "LIB-HIDDEN"))
        XCTAssertFalse(shown.drawer.isEmpty)
        XCTAssertTrue(catalog.groups(matching: DrawerFilter()).flatMap(\.allParts).contains { $0.uuid == uuid })
    }

    func testRemovalsTheFolderRefusesAreLibraryWriteErrors() throws {
        let uuid = "B3000000-0000-4000-8000-000000000003"
        _ = try write("broken.schpart", "not a part file")
        let catalog = store()
        try catalog.addToUserLibrary(fileText: PartRegistrationTests.fileText(uuid: uuid, name: "LIB-KEPT"), source: "k.schpart")
        let problem = try XCTUnwrap(catalog.userLibraryProblems.first)
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: directory.path)
        XCTAssertThrowsError(try catalog.removeFromUserLibrary(uuid: uuid)) { error in
            guard case .libraryWrite = error as? PartRegistrationError else { return XCTFail("\(error)") }
        }
        XCTAssertThrowsError(try catalog.removeUserLibraryProblem(problem)) { error in
            guard case .libraryWrite = error as? PartRegistrationError else { return XCTFail("\(error)") }
        }
        XCTAssertEqual(catalog.userLibraryProblems, [problem], "still listed")
    }

    // MARK: Small values

    func testIDsAndDefaultsOfTheCatalogsTypes() throws {
        let part = try TestSupport.part("SN74HC00N")
        XCTAssertEqual(part.pins.map(\.id), part.pins.map(\.index))
        XCTAssertEqual(PartUnit(key: "A", pins: [], shape: .block).id, "A")
        let flags = CatalogPart.Flags()
        XCTAssertFalse(flags.obsolete || flags.military || flags.automotive || flags.enhanced || flags.ceramic)
        let pad = Footprint.Pad(designator: "1", centre: .zero, size: CGSize(width: 1, height: 1), drill: 0.8, shape: .round,
                                radius: 0, chamfer: 0, corners: [])
        XCTAssertTrue(pad.plated)
        XCTAssertEqual(pad.slotLength, 0)
        XCTAssertFalse(pad.opposite)
        let display = DisplayInfo(digits: 1, commonAnode: false, colonAfter: 0, dotAfter: 0, color: "Red", size: "0.56 in",
                                  chip: "", leds: [], segmentPins: [], commonPins: [])
        XCTAssertEqual([display.vfMinMV, display.vfMaxMV], [0, 0])
        XCTAssertEqual(display.type, "led")
        let footprints = TestSupport.allParts.compactMap(\.footprint)
        XCTAssertEqual(Set(footprints).count, Set(footprints.map(\.id)).count, "a footprint is its id")
        XCTAssertEqual(PartFacts.size(words: 64, bits: 8), "64 × 8 (64 bytes)")
        XCTAssertEqual(PartFacts.inputRange(minMV: 2500, maxMV: 0), "from 2.5 V")
        XCTAssertEqual(PartFacts.inputRange(minMV: 0, maxMV: 5500), "up to 5.5 V")
        XCTAssertEqual(PartFacts.inputRange(minMV: 2500, maxMV: 5500), "2.5–5.5 V")
        XCTAssertEqual(PartFacts.inputRange(minMV: 0, maxMV: 0), "")
    }

    /// A pull resistor's Details say which way it pulls, weakly.
    func testPullResistorsLevel() throws {
        func level(_ name: String) throws -> String? {
            let part = try TestSupport.part(name)
            return PartFacts.rows(for: PlacedChip(reference: "R1", part: part, symbol: symbol(for: part), origin: .zero))
                .first { $0.label == "Level" }?.value
        }
        XCTAssertEqual(try level("PULLUP-5V"), "High, weak (pull-up)")
        XCTAssertEqual(try level("PULLDOWN"), "Low, weak (pull-down)")
        XCTAssertEqual(try level("5V"), "High (VCC)")
        XCTAssertEqual(try level("GND"), "Low (GND)")
    }
}
