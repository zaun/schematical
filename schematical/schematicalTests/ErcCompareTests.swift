import Foundation
import XCTest
@testable import schematical

/// plan-63-G: prints, for each design below, its KiCad export and what the app's ERC reports,
/// for `scripts/erc-compare.py` to hold against `kicad-cli sch erc`. Does nothing unless
/// `ERC_OUT` is set (run with `TEST_RUNNER_ERC_OUT=1`; the test host is sandboxed, so its
/// output is how files reach the shell, as in `KiCadExportTests.testPrintSamplesWhenAsked`).
@MainActor
final class ErcCompareTests: XCTestCase {
    /// A fixture opened with the part library, so pin types are the app's.
    private func fixture(_ name: String) throws -> SchematicDocument {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        try SchematicFile.decode(Data(contentsOf: Fixture.url(name)))
            .apply(to: doc, catalog: TestSupport.catalog, fileURL: nil)
        return doc
    }

    /// The designs: the three fixtures and one drawing per rule (`ErcCases`).
    private func designs() throws -> [(name: String, doc: SchematicDocument)] {
        var list: [(String, SchematicDocument)] = []
        for name in Fixture.names { list.append((name, try fixture(name))) }
        list.append(("pinConflict", try ErcCases.pinConflict()))
        list.append(("pinNotConnected", try ErcCases.pinNotConnected()))
        list.append(("pinNotDriven", try ErcCases.pinNotDriven()))
        list.append(("powerPinNotDriven", try ErcCases.powerPinNotDriven()))
        list.append(("noConnectConnected", try ErcCases.noConnectConnected()))
        list.append(("noConnectDangling", ErcCases.noConnectDangling()))
        list.append(("wireDangling", try ErcCases.wireDangling()))
        list.append(("wireToNowhere", try ErcCases.wireToNowhere()))
        list.append(("labelDangling", ErcCases.labelDangling()))
        list.append(("isolatedPinLabel", try ErcCases.isolatedPinLabel()))
        list.append(("multipleNetNames", try ErcCases.multipleNetNames()))
        list.append(("similarLabels", try ErcCases.similarLabels()))
        list.append(("similarLabelAndPower", try ErcCases.similarLabelAndPower()))
        list.append(("singleLabel", try ErcCases.singleLabel()))
        list.append(("fourWayJunction", try ErcCases.fourWayJunction()))
        list.append(("duplicateReference", try ErcCases.duplicateReference()))
        return list
    }

    /// Every rule on, as KiCad's default severities have them, plus the two it leaves off
    /// (`single_global_label`, `four_way_junction`) switched on at their default's level so
    /// the comparison sees them too.
    private func allRulesOn(_ doc: SchematicDocument) {
        doc.setErcSeverity(.singleGlobalLabel, .warning)
        doc.setErcSeverity(.fourWayJunction, .warning)
    }

    func testPrintSamplesWhenAsked() throws {
        guard ProcessInfo.processInfo.environment["ERC_OUT"] != nil else { return }
        for (name, doc) in try designs() {
            for file in KiCadExport.files(for: doc, base: name) where !file.name.hasSuffix("kicad_pcb") {
                print("KICAD_FILE erc/\(name)/\(file.name) \(file.data.base64EncodedString())")
            }
            allRulesOn(doc)
            let violations = doc.electricalRulesCheck(library: TestSupport.catalog.library).map { v in
                ["rule": v.rule.rawValue, "severity": v.severity.rawValue, "reference": v.reference,
                 "message": v.message] as [String: String]
            }
            let data = try JSONSerialization.data(withJSONObject: ["name": name, "violations": violations],
                                                  options: [.sortedKeys])
            print("ERC_RESULT \(name) \(data.base64EncodedString())")
        }
    }

    /// Always on: the designs are all made and each one checks (so the printing test has something to print).
    func testEveryDesignChecks() throws {
        let all = try designs()
        XCTAssertEqual(all.count, 19)
        for (name, doc) in all {
            XCTAssertNoThrow(doc.electricalRulesCheck(library: TestSupport.catalog.library), name)
        }
    }
}
