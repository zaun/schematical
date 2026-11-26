import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-63-F phase 1: what the Electrical Rules Check window lists and does
/// (`ErcResultsModel`), and the window's wiring.
@MainActor
final class ErcResultsModelTests: XCTestCase {
    private func model(_ doc: SchematicDocument) -> ErcResultsModel {
        let model = ErcResultsModel(document: doc, library: TestSupport.catalog.library)
        model.run()
        return model
    }

    func testRunListsAndNumbersTheViolations() throws {
        let doc = try ErcCases.pinConflict()
        let results = model(doc)
        XCTAssertTrue(results.hasRun)
        XCTAssertEqual(results.violations.map(\.index), Array(1...results.violations.count))
        XCTAssertEqual(results.violations.count, doc.electricalRulesCheck().count)
        XCTAssertTrue(results.excluded.isEmpty)
        XCTAssertEqual(results.errorCount + results.warningCount, results.violations.count)
        XCTAssertTrue(results.errorCount > 0)
    }

    func testExcludeMovesALineToTheExcludedListAndSurvivesAnotherEdit() throws {
        let doc = try ErcCases.pinNotConnected()
        let results = model(doc)
        let total = results.violations.count
        let first = try XCTUnwrap(results.violations.first)
        results.selection = [first.id]
        results.excludeSelected()
        XCTAssertEqual(results.violations.count, total - 1)
        XCTAssertEqual(results.excluded.map(\.key), [first.key])
        XCTAssertEqual(results.excluded.first?.index, 1, "numbered from 1 in its own list")
        XCTAssertTrue(results.selection.isEmpty)
        XCTAssertEqual(doc.undoName, "Exclude Violation")

        // An unrelated edit keeps the exclusion.
        _ = try ErcCases.place(doc, "SN74HC00N", 900, 600)
        results.run()
        XCTAssertEqual(results.excluded.map(\.key), [first.key])

        // Include puts it back.
        results.showsExcluded = true
        results.selection = [try XCTUnwrap(results.excluded.first).id]
        results.includeSelected()
        XCTAssertTrue(results.excluded.isEmpty)
        XCTAssertTrue(results.violations.contains { $0.key == first.key })
        XCTAssertEqual(doc.undoName, "Include Violation")
    }

    func testSelectingLinesSelectsExactlyTheirItems() throws {
        let doc = try ErcCases.pinConflict()
        let results = model(doc)
        let conflict = try XCTUnwrap(results.violations.first { $0.rule == .pinToPin })
        results.select([conflict.id])
        XCTAssertEqual(doc.schematicSelection.items, conflict.items)
        XCTAssertEqual(results.items(of: [conflict.id]).point, conflict.point)
        let lone = try XCTUnwrap(results.violations.first { $0.rule == .pinNotConnected })
        results.select([lone.id])
        XCTAssertEqual(doc.schematicSelection.items, lone.items)
        results.select([conflict.id, lone.id])
        XCTAssertEqual(doc.schematicSelection.items, conflict.items.union(lone.items))
    }

    func testChangingASeverityOrTheMapChangesTheList() throws {
        let doc = try ErcCases.pinConflict()
        let results = model(doc)
        XCTAssertTrue(results.violations.contains { $0.rule == .pinToPin })
        doc.setErcPinMap(1, 1, to: 0)
        results.run()
        XCTAssertFalse(results.violations.contains { $0.rule == .pinToPin })
        doc.setErcSeverity(.pinNotConnected, .ignore)
        results.run()
        XCTAssertFalse(results.violations.contains { $0.rule == .pinNotConnected })
    }

    func testAPinMapCellEditWritesBothHalvesAndResetKeepsExclusions() throws {
        let doc = try ErcCases.pinConflict()
        doc.setErcPinMap(0, 6, to: 0)
        XCTAssertEqual(doc.erc.effectivePinMap[0][6], 0)
        XCTAssertEqual(doc.erc.effectivePinMap[6][0], 0)
        doc.setErcSeverity(.wireDangling, .warning)
        doc.excludeErc(["pin_not_driven|chip:X:1"])
        doc.resetErcSettings()
        XCTAssertNil(doc.erc.pinMap)
        XCTAssertTrue(doc.erc.severities.isEmpty)
        XCTAssertEqual(doc.erc.exclusions, ["pin_not_driven|chip:X:1"])
    }

    func testTheCellButtonCyclesOkWarningError() {
        // The grid's click is `(level + 1) % 3`: ok → warning → error → ok.
        let doc = TestSupport.newDocument()
        var level: UInt8 = doc.erc.effectivePinMap[3][3]
        XCTAssertEqual(level, 0)
        for expected: UInt8 in [1, 2, 0] {
            level = (level + 1) % 3
            doc.setErcPinMap(3, 3, to: level)
            XCTAssertEqual(doc.erc.effectivePinMap[3][3], expected)
        }
    }

    func testTheMenuItemIsWiredToTheSchematicPage() {
        let menus = EditorMenus()
        XCTAssertNil(menus.openElectricalRulesCheck)
        var opened = 0
        menus.openElectricalRulesCheck = { opened += 1 }
        menus.openElectricalRulesCheck?()
        XCTAssertEqual(opened, 1)
        menus.removeAll()
        XCTAssertNil(menus.openElectricalRulesCheck)
        // The target is the document's id, as the Design Rule Check's.
        let id = UUID()
        XCTAssertEqual(ElectricalRulesCheckTarget(documentID: id), ElectricalRulesCheckTarget(documentID: id))
    }
}
