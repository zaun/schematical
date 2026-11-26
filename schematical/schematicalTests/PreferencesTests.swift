import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AA: the preference values behind Settings (`PreferenceKey`'s
/// readers), without the window.
@MainActor
final class PreferencesTests: XCTestCase {
    nonisolated deinit {}

    private var defaults: UserDefaults!
    /// A catalog with no parts, kept for the life of the test process.
    private static let emptyCatalog = PartCatalogStore()
    private var suite = ""

    override func setUp() {
        suite = "PreferencesTests-\(UUID().uuidString)"
        defaults = UserDefaults(suiteName: suite)
    }

    override func tearDown() {
        defaults.removePersistentDomain(forName: suite)
    }

    private func part(_ name: String) throws -> CatalogPart {
        try XCTUnwrap(TestSupport.catalog.part(named: name), "no part \(name)")
    }

    func testDocumentTabs() {
        XCTAssertEqual(DocumentTabs.allCases.map(\.id), ["system", "tabs", "windows"])
        XCTAssertEqual(DocumentTabs.allCases.map(\.title), ["System Setting", "Tabs", "Windows"])
        XCTAssertEqual(DocumentTabs.allCases.map(\.tabbingMode), [.automatic, .preferred, .disallowed])
        XCTAssertEqual(DocumentTabs.current(defaults: defaults), .tabs, "Tabs while unset")
        defaults.set("windows", forKey: PreferenceKey.documentTabs)
        XCTAssertEqual(DocumentTabs.current(defaults: defaults), .windows)
        defaults.set("nonsense", forKey: PreferenceKey.documentTabs)
        XCTAssertEqual(DocumentTabs.current(defaults: defaults), .tabs)
    }

    /// Every open document window takes the setting.
    func testDocumentTabsReachOpenWindows() throws {
        AppDelegate.quitsAfterLastWindowCloses = false
        let document = TestSupport.keep(SchematicNSDocument())
        NSDocumentController.shared.addDocument(document)
        document.makeWindowControllers()
        defer { document.close() }
        let window = try XCTUnwrap(document.windowControllers.first?.window)
        defaults.set("windows", forKey: PreferenceKey.documentTabs)
        DocumentTabs.applyToOpenDocuments(defaults: defaults)
        XCTAssertEqual(window.tabbingMode, .disallowed)
        defaults.set("system", forKey: PreferenceKey.documentTabs)
        DocumentTabs.applyToOpenDocuments(defaults: defaults)
        XCTAssertEqual(window.tabbingMode, .automatic)
    }

    func testNames() {
        XCTAssertTrue(NamePreferences.uppercaseOnly(defaults: defaults), "on while unset")
        XCTAssertEqual(NamePreferences.typed("clk", defaults: defaults), "CLK")
        defaults.set(false, forKey: PreferenceKey.uppercaseNames)
        XCTAssertEqual(NamePreferences.typed("clk", defaults: defaults), "clk")
    }

    func testNewLEDOptions() throws {
        let led = try part(NewPartOptions.genericLED)
        let gate = try part("SN74HC00N")
        XCTAssertNil(NewPartOptions.color(for: led, defaults: defaults), "never picked")
        defaults.set("", forKey: PreferenceKey.genericLEDColor)
        XCTAssertNil(NewPartOptions.color(for: led, defaults: defaults))
        defaults.set("Green", forKey: PreferenceKey.genericLEDColor)
        XCTAssertEqual(NewPartOptions.color(for: led, defaults: defaults), "Green")
        XCTAssertNil(NewPartOptions.color(for: gate, defaults: defaults), "not an LED")
        defaults.set("LED-0805", forKey: PreferenceKey.genericLEDFootprint)
        XCTAssertEqual(NewPartOptions.footprintID(for: led, defaults: defaults), "LED-0805")
        XCTAssertNil(NewPartOptions.footprintID(for: gate, defaults: defaults))
        XCTAssertNil(NewPartOptions.gender(for: led, defaults: defaults))
    }

    func testNewHeaderOptions() throws {
        let header = try part(HeaderFamily.defaultPart)
        XCTAssertNil(NewPartOptions.footprintID(for: header, defaults: defaults), "never picked")
        defaults.set("", forKey: PreferenceKey.headerFootprintType)
        XCTAssertNil(NewPartOptions.footprintID(for: header, defaults: defaults))
        let choice = try XCTUnwrap(header.footprintChoices.last)
        defaults.set(choice.package, forKey: PreferenceKey.headerFootprintType)
        XCTAssertEqual(NewPartOptions.footprintID(for: header, defaults: defaults), choice.footprint.id)

        XCTAssertNil(NewPartOptions.gender(for: header, defaults: defaults))
        defaults.set("", forKey: PreferenceKey.headerGender)
        XCTAssertNil(NewPartOptions.gender(for: header, defaults: defaults))
        defaults.set("Female", forKey: PreferenceKey.headerGender)
        XCTAssertEqual(NewPartOptions.gender(for: header, defaults: defaults), "Female")

        let catalog = TestSupport.catalog
        XCTAssertEqual(NewPartOptions.headerPart(for: header, catalog: catalog, defaults: defaults).name,
                       header.name, "the stored size is the part's own while unset")
        defaults.set(2, forKey: PreferenceKey.headerRows)
        defaults.set(5, forKey: PreferenceKey.headerPinsPerRow)
        XCTAssertEqual(NewPartOptions.headerPart(for: header, catalog: catalog, defaults: defaults).name,
                       HeaderFamily.name(rows: 2, perRow: 5))
        defaults.set(9, forKey: PreferenceKey.headerRows)
        defaults.set(HeaderFamily.maxPerRow + 1, forKey: PreferenceKey.headerPinsPerRow)
        let size = try XCTUnwrap(header.headerSize)
        XCTAssertEqual(NewPartOptions.headerPart(for: header, catalog: catalog, defaults: defaults).name,
                       HeaderFamily.name(rows: size.rows, perRow: size.perRow), "out of range: the part's own")
        let gate = try part("SN74HC00N")
        XCTAssertEqual(NewPartOptions.headerPart(for: gate, catalog: catalog, defaults: defaults).name, gate.name)
        XCTAssertEqual(NewPartOptions.headerPart(for: header, catalog: Self.emptyCatalog, defaults: defaults).name,
                       header.name, "a size the catalog doesn't have")
    }

    func testNewPullResistorOptions() throws {
        let pull = try part(NewPartOptions.pullDown)
        let led = try part(NewPartOptions.genericLED)
        XCTAssertNil(NewPartOptions.ohms(for: pull, defaults: defaults), "never set")
        defaults.set(4700.0, forKey: PreferenceKey.pullResistorOhms)
        XCTAssertEqual(NewPartOptions.ohms(for: pull, defaults: defaults), 4700)
        XCTAssertNil(NewPartOptions.ohms(for: led, defaults: defaults), "not a pull resistor")
        let footprint = try XCTUnwrap(pull.footprintChoices.last).footprint.id
        defaults.set(footprint, forKey: PreferenceKey.pullResistorFootprint)
        XCTAssertEqual(NewPartOptions.footprintID(for: pull, defaults: defaults), footprint)
    }

    /// A plain launch with no document shows an Untitled one; a launch
    /// with a file, a restored document or the test host does not.
    func testAPlainLaunchOpensUntitled() {
        var made = 0
        let open = AEEventID(kAEOpenApplication)
        AppDelegate.openUntitledIfNeeded(launchEvent: open, documentCount: 0, isTestHost: true) { _ in made += 1 }
        AppDelegate.openUntitledIfNeeded(launchEvent: AEEventID(kAEOpenDocuments), documentCount: 0,
                                         isTestHost: false) { _ in made += 1 }
        AppDelegate.openUntitledIfNeeded(launchEvent: open, documentCount: 1, isTestHost: false) { _ in made += 1 }
        XCTAssertEqual(made, 0)
        AppDelegate.openUntitledIfNeeded(launchEvent: open, documentCount: 0, isTestHost: false) { _ in made += 1 }
        XCTAssertEqual(made, 1)
    }

    /// A scene window's content: nothing until it has a target.
    func testSceneTargetRoot() {
        struct Shown: SceneTargetContent {
            let target: String
            var body: some View { Text("Showing \(target)") }
        }
        let empty = ViewProbe(SceneTargetRoot<Shown>(.constant(nil)), size: NSSize(width: 200, height: 80))
        XCTAssertFalse(empty.has("Showing"))
        empty.close()
        let shown = ViewProbe(SceneTargetRoot<Shown>(.constant("A")), size: NSSize(width: 200, height: 80))
        XCTAssertTrue(shown.has("Showing A"), "\(shown.labels())")
        shown.close()
    }

    func testPreferencesPages() {
        XCTAssertEqual(PreferencesPage.allCases.map(\.id),
                       ["partList", "defaults", "references", "designRules", "automation"])
        XCTAssertEqual(PreferencesPage.allCases.map(\.title),
                       ["Part List", "General Defaults", "Reference Defaults", "DRC Defaults", "AI Agents"])
        XCTAssertEqual(PreferencesPage.allCases.map(\.systemImage),
                       ["list.bullet", "slider.horizontal.3", "number", "checkmark.seal", "sparkles"])
    }

    func testAutomationStatusText() {
        let now = Date()
        XCTAssertEqual(AutomationPreferencesView.statusText(isListening: false, lastRequest: nil, clientName: nil,
                                                            callCount: 0, now: now), "Starting…")
        XCTAssertEqual(AutomationPreferencesView.statusText(isListening: true, lastRequest: nil, clientName: nil,
                                                            callCount: 0, now: now),
                       "Listening; no agent has connected yet")
        XCTAssertEqual(AutomationPreferencesView.statusText(isListening: true, lastRequest: now.addingTimeInterval(-2),
                                                            clientName: "claude-code", callCount: 3, now: now),
                       "claude-code connected; last request just now; 3 tool calls")
        XCTAssertEqual(AutomationPreferencesView.statusText(isListening: true, lastRequest: now.addingTimeInterval(-42),
                                                            clientName: nil, callCount: 7, now: now),
                       "An agent connected; last request 42 s ago; 7 tool calls")
    }

    // MARK: The pickers' bindings

    /// A binding over a plain variable.
    private final class Store<Value> {
        var value: Value
        init(_ value: Value) { self.value = value }
        nonisolated deinit {}
        var binding: Binding<Value> { Binding(get: { self.value }, set: { self.value = $0 }) }
    }

    func testChoiceBinding() {
        let store = Store("")
        let shown = store.binding.choice(of: ["Red", "Green"], default: "Red")
        XCTAssertEqual(shown.wrappedValue, "Red", "never picked: the default")
        shown.wrappedValue = "Green"
        XCTAssertEqual(store.value, "Green")
        XCTAssertEqual(shown.wrappedValue, "Green")
        store.value = "Blue"
        XCTAssertEqual(shown.wrappedValue, "Red", "a choice the part lost")
    }

    func testWithinBinding() {
        let store = Store(0)
        let shown = store.binding.within(1...2, default: 1)
        XCTAssertEqual(shown.wrappedValue, 1)
        shown.wrappedValue = 2
        XCTAssertEqual(store.value, 2)
        XCTAssertEqual(shown.wrappedValue, 2)
        store.value = 9
        XCTAssertEqual(shown.wrappedValue, 1)
    }

    func testFootprintTypeBinding() throws {
        let header = try part(HeaderFamily.defaultPart)
        let choices = header.footprintChoices
        let first = try XCTUnwrap(choices.first), last = try XCTUnwrap(choices.last)
        XCTAssertNotEqual(first.package, last.package)
        let store = Store("")
        let shown = store.binding.footprintID(in: choices, default: first.footprint.id)
        XCTAssertEqual(shown.wrappedValue, first.footprint.id, "never picked")
        shown.wrappedValue = last.footprint.id
        XCTAssertEqual(store.value, last.package, "stored as its package")
        XCTAssertEqual(shown.wrappedValue, last.footprint.id)
        shown.wrappedValue = "no such footprint"
        XCTAssertEqual(store.value, "")
    }

    func testStore() {
        let store = Store(1.0)
        store.binding.store(4700)
        XCTAssertEqual(store.value, 4700)
    }

    // MARK: DRC Defaults

    func testDesignRulesStoredAndDefaults() {
        XCTAssertEqual(DesignRules.decoded(Data()), .customDefault, "nothing stored")
        XCTAssertEqual(DesignRules.decoded(Data("{}".utf8)), .customDefault, "doesn't decode")
        var rules = DesignRules.customDefault
        rules.minDrill = 0.42
        XCTAssertEqual(DesignRules.decoded(rules.encoded), rules)
        XCTAssertEqual(DesignRules.groups.flatMap(\.rules).count, 13)
        XCTAssertEqual(DesignRules.groups.first?.rules.first?.id, "Min trace width")
    }

    func testSettingAndResettingCustomRules() {
        XCTAssertEqual(DesignRulePreferences.setCustom(\.minDrill, 0.42, defaults: defaults), 0.42)
        XCTAssertEqual(DesignRulePreferences.setCustom(\.minClearance, -1, defaults: defaults), 0, "never below 0")
        let stored = DesignRules.decoded(defaults.data(forKey: PreferenceKey.drcCustomRules) ?? Data())
        XCTAssertEqual(stored.minDrill, 0.42)
        XCTAssertEqual(stored.minClearance, 0)
        XCTAssertEqual(stored.minTraceWidth, DesignRules.customDefault.minTraceWidth, "the rest kept")
        DesignRulePreferences.resetCustom(defaults: defaults)
        XCTAssertEqual(DesignRules.decoded(defaults.data(forKey: PreferenceKey.drcCustomRules) ?? Data()),
                       .customDefault)
    }

    /// The `.standard` forms Settings' fields call, with the owner's stored
    /// rules put back after.
    func testTheStandardDefaultsForms() {
        let standard = UserDefaults.standard
        let saved = standard.object(forKey: PreferenceKey.drcCustomRules)
        defer { standard.set(saved, forKey: PreferenceKey.drcCustomRules) }
        XCTAssertEqual(DesignRulePreferences.setCustom(\.minDrill, 0.37), 0.37)
        XCTAssertEqual(DesignRules.decoded(standard.data(forKey: PreferenceKey.drcCustomRules) ?? Data()).minDrill, 0.37)
        DesignRulePreferences.resetCustom()
        XCTAssertEqual(DesignRules.decoded(standard.data(forKey: PreferenceKey.drcCustomRules) ?? Data()),
                       .customDefault)
    }
}
