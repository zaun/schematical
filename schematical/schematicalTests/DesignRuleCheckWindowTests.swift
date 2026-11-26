import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-W: the Design Rule Check window's results model (check, settle,
/// selection, copy, subtitle), its rules step's manufacturer menu and typed
/// rules, and the window of a closed schematic.
@MainActor
final class DesignRuleCheckWindowTests: XCTestCase {
    override func setUp() {
        super.setUp()
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    /// Two LEDs on the board wired K to A, nothing routed: an unrouted connection.
    private func unrouted() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "K")), .pin(try TestSupport.pin(two, "A"))))
        return doc
    }

    private func spin(_ seconds: TimeInterval) { RunLoop.main.run(until: Date().addingTimeInterval(seconds)) }

    func testTheResultsModel() throws {
        let doc = try unrouted()
        let model = DesignRuleCheckResults(document: doc)
        XCTAssertFalse(model.hasRun)
        model.check()
        XCTAssertTrue(model.hasRun)
        let issues = model.issues
        XCTAssertEqual(issues, doc.designRuleCheck(rules: doc.board.rules))
        XCTAssertFalse(issues.isEmpty)
        XCTAssertEqual(model.summary, "Step 2 of 2 · \(issues.count) issue\(issues.count == 1 ? "" : "s") · \(doc.board.rulesTitle) rules")
        XCTAssertEqual(model.copyText, issues.map { "\($0.index)\t\($0.reference)\t\($0.issue)" }.joined(separator: "\n"))
        XCTAssertEqual(model.copyItems().count, 1)

        let line = try XCTUnwrap(issues.first { !$0.items.isEmpty })
        model.selection = [line.id]
        XCTAssertEqual(model.copyText, "\(line.index)\t\(line.reference)\t\(line.issue)")
        model.selectionChanged()
        XCTAssertEqual(doc.boardSelection.items, line.items)
        model.selection = []
        model.selectionChanged()
        XCTAssertEqual(doc.boardSelection.items, line.items, "clearing the lines keeps the board's selection")

        // A change that adds a problem clears the selected lines (their numbers move).
        model.selection = [line.id]
        doc.moveFootprint(doc.chips[0].id, to: CGPoint(x: -40, y: -40))
        model.check()
        XCTAssertEqual(model.selection, [])

        let clean = DesignRuleCheckResults(document: TestSupport.newDocument())
        clean.check()
        XCTAssertEqual(clean.summary, "Step 2 of 2 · No issues · \(clean.document.board.rulesTitle) rules")
        XCTAssertEqual(clean.copyText, "")
    }

    /// The first check runs at once; later ones wait for the change to
    /// settle, and a newer change cancels the wait.
    func testAChangeWaitsToSettle() throws {
        let doc = try unrouted()
        let model = DesignRuleCheckResults(document: doc)
        model.settleDelay = .milliseconds(200)
        Task { await model.inputChanged() }
        spin(0.05)
        XCTAssertTrue(model.hasRun)
        let before = model.issues.count
        doc.moveFootprint(doc.chips[0].id, to: CGPoint(x: -40, y: -40))
        let waiting = Task { await model.inputChanged() }
        spin(0.05)
        XCTAssertEqual(model.issues.count, before, "still settling")
        waiting.cancel()
        spin(0.4)
        XCTAssertEqual(model.issues.count, before, "a cancelled wait checks nothing")
        Task { await model.inputChanged() }
        spin(0.4)
        XCTAssertGreaterThan(model.issues.count, before)
    }

    func testRulesChosenAndTyped() throws {
        let doc = TestSupport.newDocument()
        DesignRuleCheckRules.choose(.jlcpcb, for: doc)
        XCTAssertEqual(doc.board.rulesManufacturer, .jlcpcb)
        XCTAssertEqual(doc.board.rules, DRCManufacturer.jlcpcb.fixedRules)
        DesignRuleCheckRules.choose(.custom, for: doc)
        XCTAssertEqual(doc.board.rulesManufacturer, .custom)
        XCTAssertEqual(doc.board.rules, DRCManufacturer.jlcpcb.fixedRules, "Custom keeps the rules")
        XCTAssertEqual(DesignRuleCheckRules.set(\.minClearance, to: 0.3, in: doc), 0.3)
        XCTAssertEqual(DesignRuleCheckRules.set(\.minClearance, to: -1, in: doc), 0, "never below 0")
        XCTAssertEqual(doc.board.rules.minClearance, 0)
    }

    func testTheResultsTableAndTheManufacturerMenu() throws {
        let doc = try unrouted()
        let probe = ViewProbe(DesignRuleCheckWizard(document: doc), size: NSSize(width: 640, height: 720))
        defer { probe.close() }
        try probe.press("Run Check")
        probe.settle(0.5)
        let issues = doc.designRuleCheck(rules: doc.board.rules)
        let table = try XCTUnwrap(probe.views(NSTableView.self).first)
        XCTAssertEqual(table.numberOfRows, issues.count)
        let row = try XCTUnwrap(issues.firstIndex { !$0.items.isEmpty }, "\(issues.map(\.issue))")
        table.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
        probe.settle()
        XCTAssertEqual(doc.boardSelection.items, issues[row].items, "the line's copper is selected on the PCB")

        try probe.press("Rules")
        XCTAssertTrue(probe.has("Manufacturer"), "\(probe.labels())")
        try probe.type("0.5", intoField: "Min clearance")
        XCTAssertEqual(doc.board.rules.minClearance, 0.5)
    }

    func testABoardWithNoProblemsAndAClosedSchematic() {
        let probe = ViewProbe(DesignRuleCheckTable(document: TestSupport.newDocument(), showRules: {}))
        defer { probe.close() }
        probe.settle(0.3)
        XCTAssertTrue(probe.has("No Problems"), "\(probe.labels())")
        let closed = ViewProbe(DesignRuleCheckWindowView(target: DesignRuleCheckTarget(documentID: UUID())))
        defer { closed.close() }
        XCTAssertTrue(closed.has("Schematic Closed"), "\(closed.labels())")
    }
}
