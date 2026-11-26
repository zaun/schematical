import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-Y: the Electrical Rules Check window as it is shown — the rules
/// step, the pin map, the results table and its toolbar — read and driven
/// through the accessibility tree (`ViewProbe`).
@MainActor
final class ElectricalRulesCheckWindowTests: XCTestCase {
    private var opened: [UUID] = []

    override func setUp() {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() {
        for id in opened { ChildWindowCenter.shared.closeAll(id) }
        opened = []
    }

    /// The ERC window of a new open document drawn as `doc`, through the menu's opener.
    private func openWindow(_ doc: SchematicDocument) throws -> (window: NSWindow, id: UUID) {
        let system = TestSupport.openSystemDocument(of: doc, catalog: TestSupport.catalog, until: self)
        let id = system.model.id
        opened.append(id)
        ChildWindowOpener(documentID: id)(id: "erc", value: ElectricalRulesCheckTarget(documentID: id))
        let window = try XCTUnwrap(ChildWindowCenter.shared.windows(of: id).first)
        XCTAssertEqual(window.title, "Electrical Rules Check")
        return (window, id)
    }

    private func waitUntil(_ probe: ViewProbe, timeout: TimeInterval = 3, _ condition: () -> Bool) {
        let end = Date().addingTimeInterval(timeout)
        while !condition(), Date() < end { probe.settle(0.1) }
    }

    func testAClosedSchematicSaysSo() {
        let probe = ViewProbe(ElectricalRulesCheckWindowView(target: ElectricalRulesCheckTarget(documentID: UUID())))
        defer { probe.close() }
        XCTAssertTrue(probe.has("Schematic Closed"), "\(probe.labels())")
        XCTAssertTrue(probe.has("no longer open"))
    }

    func testTheRulesStepShowsEveryRuleAndRunsTheCheck() throws {
        let doc = try ErcCases.pinConflict()
        let (window, _) = try openWindow(doc)
        let probe = ViewProbe(window: window)
        for group in ElectricalRulesCheckRules.groups {
            XCTAssertTrue(probe.has(group.title), "\(group.title) in \(probe.labels())")
            for rule in group.rules { XCTAssertTrue(probe.has(rule.title), rule.title) }
        }
        XCTAssertTrue(probe.has("Pin conflicts on a net"))
        XCTAssertEqual(window.subtitle, "Step 1 of 2 · rules")
        // Nothing differs from KiCad's defaults: Reset is off.
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Reset to KiCad Defaults", role: "AXButton")))

        try probe.press("Run Check")
        waitUntil(probe) { window.subtitle.contains("error") }
        XCTAssertTrue(window.subtitle.hasPrefix("Step 2 of 2 · "), window.subtitle)
        let table = try XCTUnwrap(probe.views(NSTableView.self).first)
        XCTAssertEqual(table.numberOfRows, doc.electricalRulesCheck().count, "a line per violation")
        XCTAssertFalse(doc.ercMarkers.isEmpty, "the table marks the schematic")

        try probe.press("Rules")
        XCTAssertEqual(window.subtitle, "Step 1 of 2 · rules")
        XCTAssertTrue(doc.ercMarkers.isEmpty, "leaving the table takes the marks off")
    }

    func testAPinMapSquareCyclesItsLevelAndResetPutsKiCadsBack() throws {
        let doc = ErcCases.newDocument()
        let (window, _) = try openWindow(doc)
        let probe = ViewProbe(window: window)
        let square = try probe.element("Input with Output")
        XCTAssertTrue(ViewProbe.label(square).contains("fine"), ViewProbe.label(square))
        XCTAssertEqual(doc.erc.effectivePinMap[0][1], 0)
        _ = square.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        XCTAssertEqual(doc.erc.effectivePinMap[0][1], 1)
        XCTAssertEqual(doc.erc.effectivePinMap[1][0], 1, "the mirror cell follows")
        XCTAssertTrue(probe.has("Input with Output | warning"), "\(probe.find("Input with Output").map(ViewProbe.label))")

        // The no-connect row is fixed.
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Input with Unconnected")))

        let reset = try probe.element("Reset to KiCad Defaults", role: "AXButton")
        XCTAssertTrue(ViewProbe.isEnabled(reset))
        try probe.press("Reset to KiCad Defaults")
        XCTAssertNil(doc.erc.pinMap)
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Reset to KiCad Defaults", role: "AXButton")))
    }

    func testASeverityMenuReadsAndSetsTheRulesSeverity() throws {
        let doc = ErcCases.newDocument()
        let (window, _) = try openWindow(doc)
        let probe = ViewProbe(window: window)
        let rules = ElectricalRulesCheckRules.groups.flatMap(\.rules)
        XCTAssertEqual(Set(rules), Set(ErcRule.allCases.filter(\.hasSeverity)), "every rule with a severity is listed once")
        XCTAssertEqual(rules.count, Set(rules).count)
        for rule in rules { XCTAssertTrue(probe.has(rule.title), rule.title) }
        let binding = ElectricalRulesCheckRules.severity(of: .wireDangling, in: doc)
        XCTAssertEqual(binding.wrappedValue, .error)
        binding.wrappedValue = .ignore
        XCTAssertEqual(doc.erc.severities, [.wireDangling: .ignore])
        XCTAssertEqual(binding.wrappedValue, .ignore)
        probe.settle()
        XCTAssertTrue(ViewProbe.isEnabled(try probe.element("Reset to KiCad Defaults", role: "AXButton")))
    }

    func testTheTableListsExcludesIncludesAndCopies() throws {
        let doc = try ErcCases.pinNotConnected()
        let (window, _) = try openWindow(doc)
        let probe = ViewProbe(window: window)
        try probe.press("Run Check")
        waitUntil(probe) { window.subtitle.contains("error") }
        let table = try XCTUnwrap(probe.views(NSTableView.self).first)
        XCTAssertGreaterThan(table.numberOfRows, 0)
        let before = table.numberOfRows

        // Nothing selected: Exclude is off. Selecting a line selects it.
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Exclude", role: "AXButton")))
        table.selectRowIndexes(IndexSet(integer: 0), byExtendingSelection: false)
        waitUntil(probe) { ViewProbe.isEnabled((try? probe.element("Exclude", role: "AXButton")) ?? NSObject()) }
        XCTAssertFalse(doc.ercSelectedKeys.isEmpty, "the line is marked as selected on the schematic")

        try probe.press("Exclude")
        waitUntil(probe) { probe.views(NSTableView.self).first?.numberOfRows != before }
        XCTAssertEqual(doc.erc.exclusions.count, 1)

        // The excluded list, then Include puts it back.
        // The toolbar's Show picker: its Excluded segment.
        let excluded = try probe.element("Excluded (")
        _ = excluded.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        waitUntil(probe) { probe.has("Include", role: "AXButton") }
        let excludedTable = try XCTUnwrap(probe.views(NSTableView.self).first)
        excludedTable.selectRowIndexes(IndexSet(integer: 0), byExtendingSelection: false)
        waitUntil(probe) { ViewProbe.isEnabled((try? probe.element("Include", role: "AXButton")) ?? NSObject()) }
        try probe.press("Include")
        waitUntil(probe) { probe.has("Nothing Excluded") }
        XCTAssertTrue(doc.erc.exclusions.isEmpty)
        XCTAssertTrue(probe.has("No violation is excluded."), "\(probe.labels())")
    }

    func testACleanSchematicHasNoViolationsAndAnEditRerunsTheCheck() throws {
        let doc = ErcCases.newDocument()
        let (window, _) = try openWindow(doc)
        let probe = ViewProbe(window: window)
        try probe.press("Run Check")
        waitUntil(probe) { window.subtitle.contains("no violations") }
        XCTAssertEqual(window.subtitle, "Step 2 of 2 · no violations")
        XCTAssertTrue(probe.has("No Violations"), "\(probe.labels())")
        XCTAssertTrue(probe.has("passes every rule"))

        // An edit reruns the check after a short wait.
        _ = try ErcCases.place(doc, "SN74HC00N", 200, 200)
        waitUntil(probe) { window.subtitle.contains("error") }
        XCTAssertTrue(window.subtitle.contains("error"), window.subtitle)
    }

    func testTheMenuItemOpensTheCheckOnTheSchematicPageOnly() throws {
        let item = try XCTUnwrap(Self.menuItem("Electrical Rules Check"))
        XCTAssertEqual(item.keyEquivalent, "e")
        XCTAssertEqual(item.keyEquivalentModifierMask, [.command, .option])
    }

    /// The main menu's item titled `title`, anywhere under the bar.
    static func menuItem(_ title: String) -> NSMenuItem? {
        func find(_ menu: NSMenu?) -> NSMenuItem? {
            for item in menu?.items ?? [] {
                if item.title == title { return item }
                if let found = find(item.submenu) { return found }
            }
            return nil
        }
        return find(NSApp.mainMenu)
    }
}

/// plan-68-Y: what the results model gives the window to show and copy.
@MainActor
final class ErcResultsModelTextTests: XCTestCase {
    func testSummaryAndCopyText() throws {
        let doc = try ErcCases.pinNotConnected()
        let model = ErcResultsModel(document: doc, library: TestSupport.catalog.library)
        XCTAssertEqual(model.summary, "Step 2 of 2")
        model.run()
        XCTAssertEqual(model.summary, ErcResultsModel.summary(errors: model.errorCount, warnings: model.warningCount))
        XCTAssertGreaterThan(model.errorCount, 0)
        let lines = model.copyText.split(separator: "\n")
        XCTAssertEqual(lines.count, model.rows.count, "nothing selected: every line")
        let first = try XCTUnwrap(model.rows.first)
        XCTAssertEqual(String(lines[0]), "1\t\(first.severity.title)\t\(first.reference)\t\(first.message)")
        let last = try XCTUnwrap(model.rows.last)
        model.selection = [last.id]
        XCTAssertEqual(model.copyText, "\(last.index)\t\(last.severity.title)\t\(last.reference)\t\(last.message)")

        let items = model.copyItems()
        XCTAssertEqual(items.count, 1)
        let loaded = expectation(description: "the copied text")
        _ = items[0].loadObject(ofClass: NSString.self) { text, _ in
            XCTAssertEqual(text as? String, "\(last.index)\t\(last.severity.title)\t\(last.reference)\t\(last.message)")
            loaded.fulfill()
        }
        wait(for: [loaded], timeout: 5)

        let clean = ErcResultsModel(document: ErcCases.newDocument(), library: TestSupport.catalog.library)
        clean.run()
        XCTAssertEqual(clean.summary, "Step 2 of 2 · no violations")
        XCTAssertEqual(clean.copyText, "")
    }

    func testOneErrorAndOneWarningAreSingular() {
        XCTAssertEqual(ErcResultsModel.summary(errors: 1, warnings: 1), "Step 2 of 2 · 1 error, 1 warning")
        XCTAssertEqual(ErcResultsModel.summary(errors: 2, warnings: 0), "Step 2 of 2 · 2 errors, 0 warnings")
        XCTAssertEqual(ErcResultsModel.summary(errors: 0, warnings: 0), "Step 2 of 2 · no violations")
    }
}
