import Foundation
import XCTest
@testable import schematical

/// A board's design rules are its file's (plan-49): kept in the board
/// settings, saved and opened, one undo step, and what the Default net
/// class and the router work from.
@MainActor
final class FileDesignRulesTests: XCTestCase {
    /// Kept for the life of the test process: freeing a main-actor object in
    /// a test aborts the host.
    private static var kept: [SchematicFileController] = []

    /// Rules unlike any manufacturer's and unlike the app's defaults.
    private var odd: DesignRules {
        var rules = DesignRules.jlcpcb
        rules.minTraceWidth = 0.31
        rules.minClearance = 0.37
        rules.minViaDrill = 0.45
        rules.minViaDiameter = 0.95
        rules.solderMaskExpansion = 0.123
        return rules
    }

    private func json(_ doc: SchematicDocument) throws -> [String: Any] {
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        return try XCTUnwrap(JSONSerialization.jsonObject(with: data) as? [String: Any])
    }

    /// Opens `json` after `change` edits its `board.designRules` (nil removes it).
    private func open(_ json: [String: Any], _ change: (inout [String: Any]?) -> Void) throws -> SchematicDocument {
        var json = json
        var board = try XCTUnwrap(json["board"] as? [String: Any])
        var rules = board["designRules"] as? [String: Any]
        change(&rules)
        board["designRules"] = rules
        json["board"] = board
        let doc = TestSupport.newDocument()
        // Not what the file should give, so a reader that ignores the file shows.
        doc.setDesignRules(.sunstone, manufacturer: .sunstone)
        try SchematicFile.decode(try JSONSerialization.data(withJSONObject: json))
            .apply(to: doc, catalog: TestSupport.catalog, fileURL: nil)
        return doc
    }

    func testNewDocumentCopiesTheAppDefaults() {
        let doc = TestSupport.newDocument()
        XCTAssertEqual(doc.board.rules, DesignRules.appDefaults())
        XCTAssertEqual(doc.board.rulesManufacturer, DRCManufacturer.appDefault())
    }

    func testRulesSurviveSaveAndOpen() throws {
        let doc = TestSupport.newDocument()
        doc.setDesignRules(odd, manufacturer: .jlcpcb)
        let opened = try open(try json(doc)) { _ in }
        XCTAssertEqual(opened.board.rules, odd)
        XCTAssertEqual(opened.board.rulesManufacturer, .jlcpcb)
        XCTAssertEqual(opened.board.rulesTitle, "JLCPCB (modified)")
    }

    func testFileWithoutRulesOpensWithTheAppDefaults() throws {
        let doc = TestSupport.newDocument()
        doc.setDesignRules(odd, manufacturer: .jlcpcb)
        let opened = try open(try json(doc)) { $0 = nil }
        XCTAssertEqual(opened.board.rules, DesignRules.appDefaults())
        XCTAssertEqual(opened.board.rulesManufacturer, DRCManufacturer.appDefault())
        // And they are written from then on.
        let board = try XCTUnwrap(try json(opened)["board"] as? [String: Any])
        XCTAssertNotNil(board["designRules"])
    }

    func testMissingRuleTakesTheAppDefault() throws {
        let doc = TestSupport.newDocument()
        doc.setDesignRules(odd, manufacturer: .custom)
        let opened = try open(try json(doc)) { $0?["minClearance"] = nil }
        var expected = odd
        expected.minClearance = DesignRules.appDefaults().minClearance
        XCTAssertEqual(opened.board.rules, expected)
    }

    func testNegativeRuleIsRefused() throws {
        let doc = TestSupport.newDocument()
        XCTAssertThrowsError(try open(try json(doc)) { $0?["minDrill"] = -1 }) { error in
            XCTAssertTrue(error.localizedDescription.contains("design rule minDrill -1 must be 0 or more"),
                          error.localizedDescription)
        }
    }

    func testUnknownManufacturerIsCustomWithTheStoredValues() throws {
        let doc = TestSupport.newDocument()
        doc.setDesignRules(odd, manufacturer: .jlcpcb)
        let opened = try open(try json(doc)) { $0?["manufacturer"] = "a maker from a later version" }
        XCTAssertEqual(opened.board.rulesManufacturer, .custom)
        XCTAssertEqual(opened.board.rules, odd)
        XCTAssertEqual(opened.board.rulesTitle, "Custom")
    }

    func testTitle() {
        var board = BoardSettings()
        board.rules = .jlcpcb
        board.rulesManufacturer = .jlcpcb
        XCTAssertEqual(board.rulesTitle, "JLCPCB")
        board.rules.minDrill += 0.1
        XCTAssertEqual(board.rulesTitle, "JLCPCB (modified)")
        board.rulesManufacturer = .custom
        XCTAssertEqual(board.rulesTitle, "Custom")
    }

    func testChangingRulesIsOneUndoStep() {
        let doc = TestSupport.newDocument()
        let before = doc.board
        doc.setDesignRules(odd, manufacturer: .jlcpcb)
        XCTAssertEqual(doc.undoName, "Change Design Rules")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.board, before)
        XCTAssertTrue(doc.redo())
        XCTAssertEqual(doc.board.rules, odd)
    }

    func testNegativeAndNonFiniteRulesAreClamped() {
        let doc = TestSupport.newDocument()
        var rules = odd
        rules.minDrill = -2
        rules.minClearance = .nan
        doc.setDesignRules(rules, manufacturer: .custom)
        XCTAssertEqual(doc.board.rules.minDrill, 0)
        XCTAssertEqual(doc.board.rules.minClearance, 0)
    }

    /// Through the window's own file code, on disk: a rule change makes
    /// the window unsaved, Save writes it, and Open in a blank window on
    /// the same Mac reads the file's rules, not the app's.
    func testSaveAndOpenThroughTheWindowsFileCode() async throws {
        let url = FileManager.default.temporaryDirectory
            .appendingPathComponent("plan49-\(UUID().uuidString).schematical")
        defer { try? FileManager.default.removeItem(at: url) }
        let doc = TestSupport.newDocument()
        let window = SchematicFileController(document: doc, catalog: TestSupport.catalog, willReplaceDocument: {})
        Self.kept.append(window)
        XCTAssertFalse(window.hasUnsavedChanges)
        doc.setDesignRules(odd, manufacturer: .jlcpcb)
        XCTAssertTrue(window.hasUnsavedChanges, "a rule change is a change to the file")
        XCTAssertNil(window.saveWithoutPanel(to: url))
        XCTAssertFalse(window.hasUnsavedChanges)
        XCTAssertTrue(String(decoding: try Data(contentsOf: url), as: UTF8.self).contains("\"minClearance\" : 0.37"))

        let other = TestSupport.newDocument()
        let blank = SchematicFileController(document: other, catalog: TestSupport.catalog, willReplaceDocument: {})
        Self.kept.append(blank)
        await blank.open(url)
        XCTAssertEqual(blank.fileURL, url)
        XCTAssertEqual(other.board.rules, odd)
        XCTAssertEqual(other.board.rulesTitle, "JLCPCB (modified)")
        XCTAssertFalse(blank.hasUnsavedChanges)
    }

    // MARK: Readers

    /// The Default class is the smallest copper of the board's own rules.
    func testDefaultClassComesFromTheBoardsRules() throws {
        let doc = TestSupport.newDocument()
        doc.setDesignRules(odd, manufacturer: .custom)
        let id = doc.addNetClass(named: "Mine")
        let made = try XCTUnwrap(doc.board.netClasses.first { $0.id == id })
        let standard = NetRules.defaults(for: odd)
        XCTAssertNotEqual(standard, NetRules.defaults(for: .appDefaults()))
        XCTAssertEqual(made.width, standard.width)
        XCTAssertEqual(made.viaDrill, standard.viaDrill)
        XCTAssertEqual(made.viaDiameter, standard.viaDiameter)
        XCTAssertEqual(doc.netRules(at: .zero, layer: nil), standard)
    }

    /// The router's problem and the check use the board's rules.
    func testRouterAndCheckUseTheBoardsRules() throws {
        let doc = try Fixture.open("RegFile")
        let plain = doc.routingProblem(options: RoutingOptions())
        let plainIssues = doc.designRuleCheck(rules: doc.board.rules)
        doc.setDesignRules(odd, manufacturer: .custom)
        let routing = doc.routingProblem(options: RoutingOptions())
        XCTAssertEqual(routing.rules, odd)
        XCTAssertNotEqual(routing, plain)
        for net in routing.nets { XCTAssertGreaterThanOrEqual(net.clearance, odd.minClearance) }
        XCTAssertNotEqual(doc.designRuleCheck(rules: doc.board.rules), plainIssues,
                          "0.31 mm traces at least: the fixture's copper is narrower")
    }

    /// The Gerber mask opening follows the board's mask expansion.
    func testGerberUsesTheBoardsRules() throws {
        let doc = try Fixture.open("RegFile")
        let before = GerberExport.files(for: doc, base: "b").map(\.data)
        doc.setDesignRules(odd, manufacturer: .custom)
        XCTAssertNotEqual(GerberExport.files(for: doc, base: "b").map(\.data), before)
        XCTAssertEqual(GerberExport.files(for: doc, base: "b").map(\.data),
                       GerberExport.files(for: doc, base: "b", rules: odd).map(\.data))
    }
}
