import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AA: the Settings window's pages as shown, driven through their
/// controls. They read and write the app's own preferences, so every key a
/// test touches is put back afterwards.
@MainActor
final class PreferencesViewTests: XCTestCase {
    nonisolated deinit {}

    private var saved: [String: Any?] = [:]
    private var probes: [ViewProbe] = []

    override func tearDown() {
        for probe in probes { probe.close() }
        probes = []
        for (key, value) in saved { UserDefaults.standard.set(value ?? nil, forKey: key) }
        saved = [:]
    }

    /// Clears `keys` for the test (as never set), putting the owner's values
    /// back after.
    private func reset(_ keys: String...) {
        for key in keys {
            if saved.index(forKey: key) == nil { saved[key] = UserDefaults.standard.object(forKey: key) }
            UserDefaults.standard.removeObject(forKey: key)
        }
    }

    private var standard: UserDefaults { .standard }

    private func show<V: View>(_ view: V, size: NSSize = NSSize(width: 720, height: 900)) -> ViewProbe {
        let probe = ViewProbe(view, size: size)
        probes.append(probe)
        probe.settle(0.5)
        return probe
    }

    /// Flips the `index`th switch in the window, as a click would.
    private func flipSwitch(_ probe: ViewProbe, _ index: Int, file: StaticString = #filePath, line: UInt = #line) {
        let switches = probe.views(NSSwitch.self)
        guard index < switches.count else { return XCTFail("\(switches.count) switches", file: file, line: line) }
        switches[index].performClick(nil)
        probe.settle()
    }

    /// The segmented control with a segment labelled `label`.
    private func segments(_ probe: ViewProbe, with label: String, file: StaticString = #filePath,
                          line: UInt = #line) throws -> NSSegmentedControl {
        try XCTUnwrap(probe.views(NSSegmentedControl.self).first { control in
            (0..<control.segmentCount).contains { control.label(forSegment: $0) == label }
        }, "no segment \(label)", file: file, line: line)
    }

    /// Picks the segment labelled `label`, as a click would.
    private func pick(segment label: String, in control: NSSegmentedControl, _ probe: ViewProbe) {
        let index = (0..<control.segmentCount).first { control.label(forSegment: $0) == label }!
        control.selectedSegment = index
        control.sendAction(control.action, to: control.target)
        probe.settle()
    }

    // MARK: The window

    func testEachPageShowsUnderItsTitle() throws {
        reset("preferencesPage")
        let probe = show(PreferencesView())
        XCTAssertTrue(probe.has("Part List"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Obsolete parts"), "Part List first")
        let expected: [PreferencesPage: String] = [
            .defaults: "Open documents in", .references: "Reference Defaults", .designRules: "Manufacturer",
            .automation: "Allow AI agents to control Schematical", .partList: "Military parts",
        ]
        for page in PreferencesPage.allCases {
            standard.set(page.rawValue, forKey: "preferencesPage")
            probe.settle(1)
            XCTAssertTrue(probe.has(page.title), "\(page.title): \(probe.labels())")
            XCTAssertTrue(probe.has(expected[page]!), "\(page): \(probe.labels())")
        }
    }

    func testPickingInTheSidebar() throws {
        reset("preferencesPage")
        let probe = show(PreferencesView())
        let sidebar = try XCTUnwrap(probe.views(NSOutlineView.self).first)
        XCTAssertEqual(sidebar.numberOfRows, PreferencesPage.allCases.count)
        sidebar.selectRowIndexes([3], byExtendingSelection: false)
        probe.settle(0.5)
        XCTAssertEqual(standard.string(forKey: "preferencesPage"), PreferencesPage.designRules.rawValue)
        XCTAssertTrue(probe.has("Manufacturer"), "\(probe.labels())")
        sidebar.deselectAll(nil)
        probe.settle(0.5)
        XCTAssertEqual(standard.string(forKey: "preferencesPage"), PreferencesPage.designRules.rawValue,
                       "clicking empty sidebar space keeps the page")
    }

    // MARK: Part List

    func testPartListToggles() throws {
        reset(PreferenceKey.showObsoleteParts, PreferenceKey.showMilitaryParts, PreferenceKey.showAutomotiveParts)
        let probe = show(PartListPreferencesView())
        XCTAssertTrue(probe.has("Include in parts list"))
        flipSwitch(probe, 0)
        flipSwitch(probe, 2)
        XCTAssertTrue(standard.bool(forKey: PreferenceKey.showObsoleteParts))
        XCTAssertFalse(standard.bool(forKey: PreferenceKey.showMilitaryParts))
        XCTAssertTrue(standard.bool(forKey: PreferenceKey.showAutomotiveParts))
    }

    // MARK: General Defaults

    func testGeneralDefaults() throws {
        reset(PreferenceKey.documentTabs, PreferenceKey.uppercaseNames, PreferenceKey.playBuzzerSounds,
              PreferenceKey.headerRows, PreferenceKey.headerPinsPerRow, PreferenceKey.headerGender,
              PreferenceKey.headerFootprintType)
        let probe = show(DefaultsPreferencesView())
        for title in ["Documents", "Simulation", "Net Label and Bus Wire Names", "New Generic LED",
                      "New Pull Resistor", "New Generic Header", "Rows", "Pins per row", "Gender", "Ohms"] {
            XCTAssertTrue(probe.has(title), "\(title): \(probe.labels())")
        }
        // Play buzzer sounds, then Uppercase only.
        flipSwitch(probe, 0)
        XCTAssertEqual(standard.object(forKey: PreferenceKey.playBuzzerSounds) as? Bool, false)
        flipSwitch(probe, 1)
        XCTAssertEqual(standard.object(forKey: PreferenceKey.uppercaseNames) as? Bool, false)
        pick(segment: "2", in: try segments(probe, with: "2"), probe)
        XCTAssertEqual(standard.integer(forKey: PreferenceKey.headerRows), 2)
        pick(segment: "Female", in: try segments(probe, with: "Female"), probe)
        XCTAssertEqual(standard.string(forKey: PreferenceKey.headerGender), "Female")
    }

    /// Open documents in reaches the open document windows as it changes.
    func testDocumentTabsReachOpenWindows() throws {
        reset(PreferenceKey.documentTabs)
        AppDelegate.quitsAfterLastWindowCloses = false
        let document = TestSupport.keep(SchematicNSDocument())
        NSDocumentController.shared.addDocument(document)
        document.makeWindowControllers()
        defer { document.close() }
        let window = try XCTUnwrap(document.windowControllers.first?.window)
        let probe = show(DefaultsPreferencesView())
        standard.set(DocumentTabs.windows.rawValue, forKey: PreferenceKey.documentTabs)
        probe.settle(1)
        XCTAssertEqual(window.tabbingMode, .disallowed)
    }

    // MARK: DRC Defaults

    func testDRCDefaults() throws {
        reset(PreferenceKey.drcManufacturer, PreferenceKey.drcCustomRules)
        let probe = show(DesignRulePreferencesView())
        XCTAssertTrue(probe.has("All lengths are in mm."), "\(probe.labels())")
        XCTAssertFalse(probe.has("Reset to PCBWay Values"), "Custom is PCBWay's")

        XCTAssertEqual(DesignRulePreferences.setCustom(\.minDrill, 0.5), 0.5)
        probe.settle(1)
        try probe.press("Reset to PCBWay Values")
        XCTAssertEqual(DesignRules.decoded(standard.data(forKey: PreferenceKey.drcCustomRules) ?? Data()),
                       .customDefault)

        let other = try XCTUnwrap(DRCManufacturer.allCases.first { $0 != .custom })
        standard.set(other.rawValue, forKey: PreferenceKey.drcManufacturer)
        probe.settle(1)
        XCTAssertTrue(probe.has("Standard limits from \(other.title)."), "\(probe.labels())")
        let width = try XCTUnwrap(other.fixedRules?.minTraceWidth)
        XCTAssertTrue(probe.has(LengthField.format(width, units: .mm, precise: true) + " mm"), "read-only values")
    }

    // MARK: AI Agents

    func testAIAgentsOffShowsOnlyTheSwitch() throws {
        reset(PreferenceKey.automationEnabled, PreferenceKey.automationToken)
        let probe = show(AutomationPreferencesView(server: AutomationServer(port: 0), folder: AutomationFolder()))
        XCTAssertTrue(probe.has("Allow AI agents to control Schematical"))
        XCTAssertFalse(probe.has("Connecting"))
        XCTAssertGreaterThanOrEqual(standard.string(forKey: PreferenceKey.automationToken)?.count ?? 0, 32,
                                    "a key is made")
    }

    func testAIAgentsOn() throws {
        reset(PreferenceKey.automationEnabled, PreferenceKey.automationToken, PreferenceKey.automationFolder)
        let server = AutomationServer(port: 0)
        let folder = AutomationFolder()
        folder.present = { _ in nil }
        let pasteboard = NSPasteboard(name: NSPasteboard.Name("PreferencesViewTests-\(UUID())"))
        defer { pasteboard.releaseGlobally() }
        let probe = show(AutomationPreferencesView(server: server, folder: folder, pasteboard: pasteboard))
        flipSwitch(probe, 0)
        XCTAssertTrue(standard.bool(forKey: PreferenceKey.automationEnabled))
        XCTAssertTrue(probe.has("Connecting"), "\(probe.labels())")
        XCTAssertTrue(probe.has(AutomationServer.url))
        XCTAssertTrue(probe.has("None chosen"))
        XCTAssertFalse(probe.has("Clear", role: "AXButton"))

        let key = try XCTUnwrap(standard.string(forKey: PreferenceKey.automationToken))
        try probe.press("New Key")
        XCTAssertNotEqual(standard.string(forKey: PreferenceKey.automationToken), key)
        try probe.press("Copy Command")
        XCTAssertEqual(pasteboard.string(forType: .string), AutomationServer.addCommand)
        try probe.press("Choose…")
        XCTAssertNil(folder.url, "the panel was cancelled")

        let shared = FileManager.default.temporaryDirectory
        let restore = folder.shareTemporarily(shared)
        defer { restore() }
        probe.settle()
        XCTAssertTrue(probe.has(shared.path), "\(probe.labels())")
        try probe.press("Clear")
        XCTAssertNil(folder.url)

        server.listenerChanged(.ready, port: 1)
        probe.settle()
        XCTAssertTrue(probe.has("Listening; no agent has connected yet"), "\(probe.labels())")
        flipSwitch(probe, 0)
        XCTAssertFalse(standard.bool(forKey: PreferenceKey.automationEnabled))
        XCTAssertFalse(server.isListening, "turned off")
    }
}
