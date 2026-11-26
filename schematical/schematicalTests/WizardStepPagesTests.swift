import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AF: the Part Wizard's Kind, Part number and Pins steps as they are
/// shown and driven: the library search and the kind rows (with the "settings
/// will be cleared" question), the identity fields and their menus, and the
/// pasted pin table.
@MainActor
final class WizardStepPagesTests: XCTestCase {
    /// Kept for the life of the test process (see PartFileSaveOpenTests).
    private static var kept: [PartEditorModel] = []
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        for probe in probes {
            if let sheet = probe.window.attachedSheet { probe.window.endSheet(sheet) }
            probe.close()
        }
        probes = []
    }

    private func model(_ source: PartEditorTarget.Source) -> PartEditorModel {
        let model = PartEditorModel(target: PartEditorTarget(source: source), catalog: .shared)
        Self.kept.append(model)
        return model
    }

    private func show<V: View>(_ page: V) -> ViewProbe {
        // A plain stack, not a grouped form: a form builds only the rows on screen.
        let probe = ViewProbe(ScrollView { VStack(alignment: .leading) { page }.padding() },
                              size: NSSize(width: 720, height: 900))
        probes.append(probe)
        return probe
    }

    /// Presses a button of the alert sheet on `probe`'s window.
    private func answer(_ title: String, on probe: ViewProbe) throws {
        let sheet = ViewProbe(window: try probe.sheet())
        try sheet.press(title)
        probe.wait { probe.window.attachedSheet == nil }
    }

    // MARK: Kind

    func testTheLibrarySearchStartsFromASimilarPart() throws {
        let model = model(.wizard)
        let probe = show(KindStepPage(model: model))
        XCTAssertTrue(probe.has("Start from a similar part"), "\(probe.labels())")
        try probe.type("NOSUCHPART999", intoField: "A part number, such as 74HC00")
        XCTAssertTrue(probe.has("No library part matches."), "\(probe.labels())")
        try probe.type("SN74HC00N", intoField: "A part number, such as 74HC00")
        XCTAssertFalse(probe.has("No library part matches."), "\(probe.labels())")
        let row = try probe.element(KindStepPage.similarRow(try XCTUnwrap(model.similarParts(matching: "SN74HC00N").first)))
        probe.pressElement(row)
        XCTAssertTrue(model.kindChosen)
        XCTAssertEqual(model.draft.kind, "gate")
        XCTAssertTrue(model.draft.name.hasPrefix("SN74HC00N."), model.draft.name)
    }

    func testASimilarRowNamesWhatItKnows() throws {
        let part = try XCTUnwrap(PartCatalogStore.shared.part(named: "SN74HC00N"))
        XCTAssertEqual(KindStepPage.similarRow(part), "SN74HC00N — \(part.function), DIP, Texas Instruments")
        XCTAssertEqual(KindStepPage.clearMessage(from: "gate"),
                       "The logic gate's own settings will be cleared. Pins, name, datasheet and footprint are kept.")
    }

    func testAKindRowChoosesTheKind() throws {
        let model = model(.wizard)
        let probe = show(KindStepPage(model: model))
        XCTAssertTrue(probe.has("Just a symbol and footprint (no simulation)"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Chosen"), "nothing is chosen yet")
        probe.pressElement(try probe.element(identifier: "kind-gate"))
        XCTAssertTrue(model.kindChosen)
        XCTAssertEqual(model.draft.kind, "gate")
        XCTAssertNil(probe.window.attachedSheet, "a new part has no settings to clear")
        XCTAssertTrue(probe.has("Chosen"), "\(probe.labels())")
    }

    func testChangingAKindWithSettingsAsksFirst() throws {
        let model = model(.copy(name: "SN74HC00N"))
        XCTAssertTrue(model.kindChangeClearsSettings(to: "counter"))
        let probe = show(KindStepPage(model: model))
        probe.pressElement(try probe.element(identifier: "kind-counter"))
        XCTAssertEqual(model.draft.kind, "gate", "asked first")
        try answer("Cancel", on: probe)
        XCTAssertEqual(model.draft.kind, "gate")
        probe.pressElement(try probe.element(identifier: "kind-counter"))
        try answer("Change", on: probe)
        XCTAssertEqual(model.draft.kind, "counter")
    }

    func testRequestKindChoosesOrAsks() {
        let fresh = model(.wizard)
        XCTAssertNil(fresh.requestKind("gate"))
        XCTAssertEqual(fresh.draft.kind, "gate")
        let copy = model(.copy(name: "SN74HC00N"))
        XCTAssertEqual(copy.requestKind("counter"), "counter")
        XCTAssertEqual(copy.draft.kind, "gate", "not changed until the answer")
        XCTAssertNil(copy.requestKind("gate"), "the same kind clears nothing")
    }

    // MARK: Part number

    func testTheIdentityFieldsAndTheirMenus() throws {
        let model = model(.copy(name: "SN74HC00N"))
        let probe = show(IdentityStepPage(model: model))
        // The copy has the library part's name, so Add to Library suffixes it.
        XCTAssertNotEqual(model.libraryName, model.draft.name)
        XCTAssertTrue(probe.has("A part named SN74HC00N is loaded; Add to Library adds this one as \(model.libraryName)."),
                      "\(probe.labels())")
        // The two menus are there; their items are built only as a menu opens on screen.
        XCTAssertTrue(probe.has("Manufacturers"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Packages"), "\(probe.labels())")
        XCTAssertFalse(probe.has("“Generic” means"), "\(probe.labels())")
        model.draft.manufacturer = "Generic"
        probe.settle()
        XCTAssertTrue(probe.has("“Generic” means your own design or an unknown maker."), "\(probe.labels())")
        probe.enter("", into: try probe.field(named: "Texas Instruments"))
        XCTAssertNil(model.draft.manufacturer, "an empty maker is none")
        probe.enter("ACME", into: try probe.field(named: "Texas Instruments"))
        XCTAssertEqual(model.draft.manufacturer, "ACME")
    }

    func testMoreOptionsHoldTheFilingKeys() throws {
        let model = model(.new(kind: "gate"))
        XCTAssertFalse(model.autoKeys.isEmpty)
        let probe = show(IdentityStepPage(model: model))
        XCTAssertFalse(probe.textFields().contains { $0.accessibilityLabel() == "Device id" })
        try probe.pressControl("More options")
        XCTAssertTrue(probe.has("Filled in from the part number and the kind until you change them."), "\(probe.labels())")
        probe.enter("A note", into: try probe.field(named: "Note"))
        XCTAssertEqual(model.draft.note, "A note")
        probe.enter("", into: try probe.field(named: "Note"))
        XCTAssertNil(model.draft.note)
        try probe.pressControl("Military")
        XCTAssertEqual(model.draft.flags, ["military"])
    }

    func testThePackageMenuFilters() {
        let all = ["DIP", "SOIC", "TSSOP", "QFN"]
        XCTAssertEqual(IdentityStepPage.packages(all, matching: ""), all)
        XCTAssertEqual(IdentityStepPage.packages(all, matching: " so "), ["SOIC", "TSSOP"])
        XCTAssertEqual(IdentityStepPage.packages((0..<50).map { "P\($0)" }, matching: "").count, 40)
    }

    // MARK: Pins

    private let table = ["1\t1A\tI", "2\t1B\tI", "3\t1Y\tO"].joined(separator: "\n")

    func testThePastedTableIsReadAndGuessed() {
        var pasted = PastedPinTable()
        XCTAssertTrue(pasted.result.pins.isEmpty)
        pasted.paste(table)
        XCTAssertEqual(pasted.text, table)
        XCTAssertEqual(pasted.table.rows.count, 3)
        XCTAssertEqual(pasted.columns, [.number, .name, .type])
        XCTAssertEqual(pasted.result.pins.map(\.name), ["1A", "1B", "1Y"])
        XCTAssertEqual(pasted.style, .numbered)
        XCTAssertEqual(pasted.hiddenRows, 0)
        // The same text keeps the user's columns; new text guesses again.
        pasted.columns[1] = .ignore
        pasted.paste(table)
        XCTAssertEqual(pasted.columns[1], .ignore)
        pasted.paste((1...14).map { "\($0)\tP\($0)" }.joined(separator: "\n"))
        XCTAssertEqual(pasted.columns, [.number, .name])
        XCTAssertEqual(pasted.hiddenRows, 4)
    }

    func testPasteApplyAndStartNumbered() throws {
        let model = model(.new(kind: "generic"))
        XCTAssertEqual(model.draft.pinCount, 0)
        let board = NSPasteboard(name: NSPasteboard.Name("plan-68-AF-\(UUID().uuidString)"))
        defer { board.releaseGlobally() }
        board.clearContents()
        board.setString(table, forType: .string)
        let probe = show(PinoutStepPage(model: model, pasteboard: board))
        XCTAssertTrue(probe.has("Start with 8 numbered pins"), "\(probe.labels())")
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Clear", role: "AXButton")))
        try probe.press("Paste")
        XCTAssertTrue(probe.has("Check the columns"), "\(probe.labels())")
        XCTAssertTrue(probe.has("3 pins, 0 rows skipped"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Roles and active-low marks are guessed"), "\(probe.labels())")
        try probe.press("Apply")
        XCTAssertEqual(model.draft.pinouts.first?.pins.map(\.name), ["1A", "1B", "1Y"])
        XCTAssertNil(probe.window.attachedSheet, "no pins to replace: applied at once")
        // Now the part has pins: Apply asks first.
        try probe.press("Apply")
        try answer("Cancel", on: probe)
        try probe.press("Apply")
        try answer("Replace", on: probe)
        XCTAssertEqual(model.draft.pinCount, 3)
        try probe.press("Clear")
        XCTAssertFalse(probe.has("Check the columns"), "\(probe.labels())")
    }

    func testAColumnChoiceThatGivesNoPinsSaysWhatIsNeeded() throws {
        let model = model(.new(kind: "generic"))
        let probe = show(PinoutStepPage(model: model))
        let editor = try XCTUnwrap(probe.views(NSTextView.self).first { $0.isEditable })
        probe.enter((1...12).map { "x\($0)" }.joined(separator: "\n"), into: editor)
        XCTAssertTrue(probe.has("… and 2 more rows"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Choose which column holds the pin number (or ball) and which the name."),
                      "\(probe.labels())")
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Apply", role: "AXButton")))
        let column = try XCTUnwrap(probe.popUps().map(probe.filled).first { $0.itemTitles.contains("Pin number") })
        probe.choose("Ignore", in: column)
        XCTAssertTrue(probe.has("0 pins, 12 rows skipped"), "\(probe.labels())")
    }

    func testStartWithNumberedPins() throws {
        let model = model(.new(kind: "generic"))
        let probe = show(PinoutStepPage(model: model))
        try probe.step(up: true)
        XCTAssertTrue(probe.has("Start with 9 numbered pins"), "\(probe.labels())")
        try probe.press("Start")
        XCTAssertEqual(model.draft.pinDesignators, (1...9).map(String.init))
        XCTAssertFalse(probe.has("No table to copy?"), "\(probe.labels())")
    }

    func testApplyingPinsMakesThePinoutWhenThereIsNone() {
        let model = model(.new(kind: "generic"))
        var draft = model.draft
        draft.pinouts = []
        draft.pinout = ""
        model.draft = draft
        model.applyPins([PinDraft(designator: "1", name: "A", role: "input")], style: .numbered)
        XCTAssertEqual(model.draft.pinout, "default")
        XCTAssertEqual(model.draft.pinouts.map(\.name), ["default"])
        draft = model.draft
        draft.pinout = "p2"
        model.draft = draft
        model.applyPins([PinDraft(designator: "A1", name: "B", role: "input")], style: .ball)
        XCTAssertEqual(model.draft.pinouts.map(\.name), ["default", "p2"])
        XCTAssertTrue(model.draft.isBallPinout)
    }

    func testSummaryWords() {
        XCTAssertEqual(PinoutStepPage.summary(pins: 1, skipped: 1), "1 pin, 1 row skipped")
        XCTAssertEqual(PinoutStepPage.summary(pins: 14, skipped: 0), "14 pins, 0 rows skipped")
    }
}
