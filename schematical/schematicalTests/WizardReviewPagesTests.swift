import AppKit
import PDFKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AG: the Part Wizard's frame and last pages as they are shown and
/// driven: the step list, a step's header and page, the Back / Skip / Next
/// bar, "Which chip does yours behave like?" with its "settings will be
/// removed" question, and Review (still to do, the backend's verdict, Save
/// Part… and Add to Library, the previews, the pins and pads).
@MainActor
final class WizardReviewPagesTests: XCTestCase {
    /// Kept for the life of the test process (see PartFileSaveOpenTests).
    private static var kept: [AnyObject] = []
    private var probes: [ViewProbe] = []
    private var directory: URL!
    private var panels = 0

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("wizrev-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        panels = 0
    }

    override func tearDown() async throws {
        for probe in probes {
            if let sheet = probe.window.attachedSheet { probe.window.endSheet(sheet) }
            probe.close()
        }
        probes = []
        try? FileManager.default.removeItem(at: directory)
    }

    private func model(_ source: PartEditorTarget.Source, catalog: PartCatalogStore = .shared) -> PartEditorModel {
        let model = PartEditorModel(target: PartEditorTarget(source: source), catalog: catalog)
        Self.kept.append(model)
        model.presenter.choose = { [weak self] _, _ in
            self?.panels += 1
            return nil
        }
        return model
    }

    /// A catalog whose User Library is this test's directory.
    private func isolatedCatalog() -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        store.attachUserLibrary(UserPartLibrary(directory: directory))
        Self.kept.append(store)
        return store
    }

    private func show<V: View>(_ view: V, size: NSSize = NSSize(width: 820, height: 1000)) -> ViewProbe {
        let probe = ViewProbe(view, size: size)
        probes.append(probe)
        return probe
    }

    /// A page as the step view hosts it, in a plain stack (a grouped form
    /// builds only the rows on screen).
    private func page<V: View>(_ page: V) -> ViewProbe {
        show(ScrollView { VStack(alignment: .leading) { page }.padding() })
    }

    /// Waits for the backend's verdict on the model's current draft.
    private func validated(_ model: PartEditorModel, _ probe: ViewProbe? = nil) {
        let checked = { model.validation != nil }
        if let probe { _ = probe.wait(5, until: checked) } else {
            let end = Date().addingTimeInterval(5)
            while !checked(), Date() < end { RunLoop.main.run(until: Date().addingTimeInterval(0.05)) }
        }
        XCTAssertNotNil(model.validation, "the check finished")
    }

    private func answer(_ title: String, on probe: ViewProbe) throws {
        let sheet = ViewProbe(window: try probe.sheet())
        try sheet.press(title)
        _ = probe.wait { probe.window.attachedSheet == nil }
    }

    // MARK: Review

    func testReviewListsWhatIsLeftAndGoesThere() throws {
        let model = model(.wizard)
        model.go(to: WizardStep.review.id)
        let probe = page(ReviewStepPage(model: model))
        XCTAssertTrue(probe.has("Still to do"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Choose what you are making"), "\(probe.labels())")
        XCTAssertEqual(ReviewStepPage.todo(model).first?.stepID, WizardStep.kind.id)
        try probe.press("Go to \(WizardStep.kind.title)")
        XCTAssertEqual(model.currentStep?.id, WizardStep.kind.id)
    }

    func testReviewOfAPartThatLoads() throws {
        let store = isolatedCatalog()
        let model = model(.new(kind: "gate"), catalog: store)
        model.draft = WizardIssueLocatorTests.gateDraft()
        model.status = "Saved"
        validated(model)
        let probe = page(ReviewStepPage(model: model))
        _ = probe.wait { model.isValid }
        XCTAssertFalse(probe.has("Still to do"), "\(probe.labels())")
        XCTAssertTrue(probe.has("The part loads: 14 pins"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Saved"), "\(probe.labels())")
        XCTAssertTrue(probe.has("SOIC-14: 14 pads"), "the footprint preview: \(probe.labels())")
        XCTAssertFalse(probe.has("Both are available once the part loads."), "\(probe.labels())")
        XCTAssertFalse(probe.has("The previews show the part once it loads."), "\(probe.labels())")
        XCTAssertFalse(probe.has("Showing the part as it last loaded."), "\(probe.labels())")
        // Every pin has its pad on the SOIC-14.
        XCTAssertTrue(probe.has("Pad 1"), "\(probe.labels())")
        XCTAssertFalse(probe.has("No pad"), "\(probe.labels())")

        // Save Part… asks where (the test answers Cancel).
        try probe.press("Save Part…")
        _ = probe.wait { self.panels == 1 }
        XCTAssertEqual(panels, 1)
        // Add to Library adds it to this test's library, and the button goes.
        try probe.press("Add to Library")
        _ = probe.wait { model.status != "Saved" }
        XCTAssertEqual(model.status, "Added \(model.draft.name) to your User Library")
        XCTAssertNotNil(store.allParts.first { $0.name == model.draft.name }, "in this test's library")
        XCTAssertTrue(probe.has("Added \(model.draft.name) to your User Library"), "\(probe.labels())")
    }

    func testReviewOfARejectedPartPointsAtItsStep() throws {
        let model = model(.new(kind: "gate"))
        model.draft = WizardIssueLocatorTests.gateDraft()
        validated(model)
        _ = model.validateNow()
        XCTAssertTrue(model.isValid)
        // A gate output that names no pin: the loader says so, and Review
        // offers the step that holds the gates.
        var broken = model.draft
        broken.kindDevice["gates"] = .array([.object([
            "op": .string("nand"), "inputs": .array([.string("1A"), .string("1B")]), "output": .string("Q"),
        ])])
        model.draft = broken
        if case .success = model.validateNow() { XCTFail("the broken draft loads") }
        let message = try XCTUnwrap(model.validationMessage)
        let target = try XCTUnwrap(ReviewStepPage.target(of: message, in: model))
        XCTAssertEqual(target.id, "kind.gates")
        let probe = page(ReviewStepPage(model: model))
        XCTAssertTrue(probe.has(message), "\(probe.labels())")
        XCTAssertTrue(probe.has("Both are available once the part loads."), "\(probe.labels())")
        // The previews keep the part as it last loaded; its pads are unknown now.
        XCTAssertTrue(probe.has("Showing the part as it last loaded."), "\(probe.labels())")
        XCTAssertTrue(probe.has("—"), "\(probe.labels())")
        let save = try probe.element("Save Part…", role: "AXButton")
        XCTAssertFalse(ViewProbe.isEnabled(save))
        try probe.press("Go to \(target.title)")
        XCTAssertEqual(model.currentStep?.id, "kind.gates")
        // A message no rule places offers no step.
        XCTAssertNil(ReviewStepPage.target(of: "something else entirely", in: model))
    }

    func testReviewBeforeTheFirstCheckAndWithNoPins() throws {
        let model = model(.new(kind: "generic"))
        model.draft.pinouts[0].pins = []
        // Hosted before the run loop turns: the first check has not finished.
        let labels = ViewProbe.labelsOnFirstLayout(ScrollView { VStack { ReviewStepPage(model: model) } },
                                                   kept: &probes)
        XCTAssertTrue(labels.contains { $0.contains("Checking…") }, "\(labels)")
        XCTAssertTrue(labels.contains { $0.contains("The previews show the part once it loads.") }, "\(labels)")
        XCTAssertTrue(labels.contains { $0.contains("The part has no pins yet.") }, "\(labels)")
    }

    func testPinRowsSayWhereEachPinLands() {
        var draft = PartDraft(newOf: "generic")
        draft.pinouts[0].pins = [
            PinDraft(designator: "1", name: "A", role: "input"),
            PinDraft(designator: "2", name: "NC", role: "absent"),
            PinDraft(designator: "3", name: "B", role: "output"),
        ]
        XCTAssertEqual(ReviewStepPage.pinRows(draft: draft, part: nil).map(\.pad), [.unknown, .notNeeded, .unknown])
        draft.pinout = "nothing"
        XCTAssertEqual(ReviewStepPage.pinRows(draft: draft, part: nil), [])
    }

    func testAMissingPadShows() throws {
        let model = model(.new(kind: "gate"))
        var draft = WizardIssueLocatorTests.gateDraft()
        draft.footprint = "SOIC-8"
        model.draft = draft
        guard case let .success(part) = model.validateNow() else {
            // A gate on a smaller footprint may be refused; then the rows are unknown.
            XCTAssertTrue(ReviewStepPage.pinRows(draft: draft, part: nil).allSatisfy { $0.pad == .unknown })
            return
        }
        let rows = ReviewStepPage.pinRows(draft: draft, part: part)
        XCTAssertTrue(rows.contains { $0.pad == .missing }, "\(rows)")
        let probe = page(ReviewStepPage(model: model))
        XCTAssertTrue(probe.has("No pad"), "\(probe.labels())")
    }

    // MARK: Which chip does yours behave like?

    func testTheVariantPageChoosesAModel() throws {
        let model = model(.new(kind: "counter"))
        let probe = page(VariantStepPage(model: model))
        XCTAssertTrue(probe.has("Pick the one whose behavior matches your chip"), "\(probe.labels())")
        probe.pressElement(try probe.element(identifier: "variant-ripple"))
        XCTAssertEqual(model.draft.variant, "ripple")
        XCTAssertNil(probe.window.attachedSheet, "nothing to remove")
        XCTAssertTrue(probe.has("Chosen"), "\(probe.labels())")
        if let examples = BundledPartFile.examplesByVariant(kind: "counter")["ripple"], let first = examples.first {
            XCTAssertTrue(probe.has("In the library: \(first)"), "\(probe.labels())")
        }
    }

    func testChangingAModelWithSettingsAsksFirst() throws {
        let model = model(.new(kind: "counter"))
        model.chooseVariant("ripple")
        model.draft.kindDevice["sections"] = .array([.object([:])])
        let probe = page(VariantStepPage(model: model))
        probe.pressElement(try probe.element(identifier: "variant-sync"))
        _ = probe.wait { probe.window.attachedSheet != nil }
        let sheet = ViewProbe(window: try probe.sheet())
        XCTAssertTrue(sheet.has("These settings do not exist for it and will be removed: sections."), "\(sheet.labels())")
        try answer("Cancel", on: probe)
        XCTAssertEqual(model.draft.variant, "ripple")
        XCTAssertNotNil(model.draft.kindDevice["sections"])
        probe.pressElement(try probe.element(identifier: "variant-sync"))
        _ = probe.wait { probe.window.attachedSheet != nil }
        try answer("Change", on: probe)
        XCTAssertEqual(model.draft.variant, "sync")
        XCTAssertNil(model.draft.kindDevice["sections"])
    }

    func testAKindWithoutModelsHasNoVariantPage() throws {
        let model = model(.new(kind: "gate"))
        let before = model.draft
        model.chooseVariant("anything")
        XCTAssertEqual(model.draft, before, "a gate has no model to choose")
        let probe = page(VariantStepPage(model: model))
        XCTAssertFalse(probe.has("Pick the one whose behavior matches your chip"), "\(probe.labels())")
    }

    // MARK: Back, Skip, Next

    func testTheNavBarMovesBetweenSteps() throws {
        let model = model(.new(kind: "counter"))
        model.go(to: WizardStep.kind.id)
        let probe = show(WizardNavBar(model: model), size: NSSize(width: 600, height: 80))
        let count = model.steps.count
        XCTAssertTrue(probe.has("Step 1 of \(count)"), "\(probe.labels())")
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Back", role: "AXButton")))
        try probe.press("Next")
        XCTAssertEqual(model.currentStepIndex, 1)
        XCTAssertTrue(probe.has("Step 2 of \(count)"), "\(probe.labels())")
        try probe.press("Back")
        XCTAssertEqual(model.currentStepIndex, 0)
        // An optional step offers Skip, which goes on.
        let optional = try XCTUnwrap(model.steps.first(where: \.optional))
        model.go(to: optional.id)
        probe.settle()
        let index = model.currentStepIndex
        try probe.press("Skip")
        XCTAssertEqual(model.currentStepIndex, index + 1)
        // The last step has no Next.
        model.go(to: WizardStep.review.id)
        probe.settle()
        XCTAssertFalse(probe.has("Next", role: "AXButton"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Skip", role: "AXButton"), "\(probe.labels())")
    }

    // MARK: The step list

    func testTheStepListMarksEachStepAndGoesToIt() throws {
        let model = model(.wizard)
        let steps = model.steps
        XCTAssertEqual(WizardStepList.state(of: WizardStep.kind, in: model), .issues)
        XCTAssertEqual(WizardStepList.state(of: WizardStep.datasheet, in: model), .plain)
        XCTAssertEqual(WizardStepList.state(of: WizardStep.review, in: model), .plain)
        let probe = show(WizardStepList(model: model), size: NSSize(width: 260, height: 900))
        let table = try XCTUnwrap(probe.views(NSTableView.self).first)
        func shown() -> [String] {
            (0..<table.numberOfRows).flatMap { row in
                table.rowView(atRow: row, makeIfNecessary: true).map { rowView in
                    ViewProbe.texts(in: rowView) + rowView.subviews.flatMap { ViewProbe.texts(in: $0) }
                } ?? []
            }
        }
        XCTAssertTrue(shown().contains { $0.contains("1. \(WizardStep.kind.title)") }, "\(table.numberOfRows) rows: \(shown())")
        XCTAssertTrue(shown().contains { $0.contains("Needs attention") }, "\(shown())")
        // Choosing the kind completes it; a counter has optional steps.
        model.chooseKind("counter")
        probe.settle()
        XCTAssertEqual(WizardStepList.state(of: WizardStep.kind, in: model), .done)
        XCTAssertTrue(shown().contains { $0.contains("Complete") }, "\(shown())")
        XCTAssertTrue(shown().contains { $0.contains("Optional") }, "\(shown())")
        // Selecting a row goes to its step.
        let row = try XCTUnwrap(model.steps.firstIndex { $0.id == WizardStep.footprint.id })
        table.selectRowIndexes([row], byExtendingSelection: false)
        probe.settle()
        XCTAssertEqual(model.currentStep?.id, WizardStep.footprint.id)
        XCTAssertNotEqual(steps.count, 0)
    }

    // MARK: One step

    func testEveryStepShowsItsHeaderAndPage() throws {
        let gate = model(.new(kind: "gate"))
        let counter = model(.new(kind: "counter"))
        let wizard = model(.wizard)
        var pages: Set<WizardPage> = []
        for (model, step) in wizard.steps.prefix(1).map({ (wizard, $0) }) + gate.steps.map({ (gate, $0) })
            + counter.steps.map({ (counter, $0) }) {
            let probe = show(WizardStepView(model: model, step: step))
            XCTAssertTrue(probe.has(step.title), "\(step.id): \(probe.labels())")
            if step.optional {
                XCTAssertTrue(probe.has("Optional — skip this if you don't need it in the simulation."),
                              "\(step.id): \(probe.labels())")
            }
            pages.insert(step.page)
            probe.close()
        }
        XCTAssertEqual(pages, [.kind, .identity, .datasheet, .pinout, .variant, .supply, .kindFields, .footprint, .review])
    }

    func testAStepShowsItsIssuesAndTheOpenError() throws {
        let model = model(.copy(name: "NO-SUCH-PART-AG"))
        model.draft.name = ""
        let probe = show(WizardStepView(model: model, step: WizardStep.identity))
        XCTAssertTrue(probe.has("There is no part named NO-SUCH-PART-AG to copy."), "\(probe.labels())")
        XCTAssertTrue(probe.has("The part has no part number"), "\(probe.labels())")
    }

    func testFindInDatasheetWaitsForTheDatasheetThenLooks() throws {
        let model = model(.new(kind: "gate"))
        let step = WizardStep.pinout
        let probe = show(WizardStepView(model: model, step: step))
        let find = try probe.element("Find in datasheet", role: "AXButton")
        XCTAssertFalse(ViewProbe.isEnabled(find), "no datasheet yet")
        let document = PDFDocument()
        document.insert(PDFPage(), at: 0)
        model.datasheet.document = document
        model.guidedPane = .block
        probe.settle()
        try probe.press("Find in datasheet")
        XCTAssertEqual(model.guidedPane, .datasheet)
        let result = try XCTUnwrap(model.datasheet.lastResult)
        XCTAssertTrue(result.hasPrefix("Not found: "), result)
        XCTAssertTrue(probe.has(result), "\(probe.labels())")
    }

    // MARK: What a step still needs, and where a message points

    func testIssuesReachEveryShapeOfValue() {
        let pins: Set<String> = ["A", "B"]
        let pin = Field(key: "p", label: "P", type: .pins)
        XCTAssertEqual(pin.issues(value: .array([.string("A"), .string("Z"), .number(1)]), pins: pins, path: "P"),
                       ["P: there is no pin named “Z”"])
        let inner = Field(key: "in", label: "In", type: .pin)
        let object = Field(key: "o", label: "O", type: .object([inner]))
        XCTAssertEqual(object.issues(value: .object(["in": .string("Q")]), pins: pins, path: "O"),
                       ["O › In: there is no pin named “Q”"])
        let values = Field(key: "v", label: "V", type: .valueMap(keyLabel: "Name", .pin))
        XCTAssertEqual(values.issues(value: .object(["y": .string("A"), "x": .string("Q")]), pins: pins, path: "V"),
                       ["V x: there is no pin named “Q”"])
        let list = Field(key: "l", label: "L", type: .listOf(.pin))
        XCTAssertEqual(list.issues(value: .array([.string("A"), .string("Q")]), pins: pins, path: "L"),
                       ["L 2: there is no pin named “Q”"])
        let either = Field(key: "e", label: "E", type: .oneOf([.pin, .pins]))
        XCTAssertEqual(either.issues(value: .array([.string("Q")]), pins: pins, path: "E"),
                       ["E: there is no pin named “Q”"])
        XCTAssertEqual(either.issues(value: .object(["k": .string("Q")]), pins: pins, path: "E"), [],
                       "no shape matches an object")
    }

    func testAMissingPinoutIsAnIssue() {
        var draft = PartDraft(newOf: "generic")
        draft.pinout = "gone"
        XCTAssertEqual(WizardStep.pinout.issues(in: draft), ["The part's pinout “gone” does not exist"])
    }

    func testMessagesWithTheCommonKeysPointAtTheirSteps() {
        let draft = PartDraft(newOf: "generic")
        let steps = WizardPlan.steps(for: draft)
        func step(_ message: String) -> String? { WizardIssueLocator.step(for: message, steps: steps, draft: draft) }
        XCTAssertEqual(step("p.schpart: devices[0] (x): the pin count is wrong"), WizardStep.pinout.id)
        XCTAssertEqual(step("p.schpart: devices[0] (x): bad \"manufacturer\""), WizardStep.identity.id)
        XCTAssertEqual(step("p.schpart: devices[0] (x): bad \"datasheet\""), WizardStep.datasheet.id)
        XCTAssertEqual(step("p.schpart: devices[0] (x): bad \"fits\""), WizardStep.footprint.id)
        XCTAssertEqual(step("p.schpart: devices[0] (x): bad \"pinouts\""), WizardStep.pinout.id)
        XCTAssertEqual(step("p.schpart: devices[0] (x): bad \"vcc\""), "kind.supply")
        // A kind with no supply step has no vcc key either: no step.
        let plain = PartKinds.schemas.keys.sorted().lazy.map { PartDraft(newOf: $0) }
            .first { !WizardPlan.steps(for: $0).contains { $0.page == .supply } }
        if let plain {
            XCTAssertNil(WizardIssueLocator.step(for: "p: bad \"vcc\"", steps: WizardPlan.steps(for: plain), draft: plain))
        } else {
            XCTFail("every kind has a supply step")
        }
        XCTAssertNil(step("p.schpart: devices[0] (x): bad \"other\" and \"words\""))
    }

    func testAGuideWithoutFieldGuidance() {
        let guide = KindGuide(steps: { _ in [WizardStep.review] })
        XCTAssertTrue(guide.fields.isEmpty)
        XCTAssertEqual(guide.steps(nil).map(\.id), [WizardStep.review.id])
    }
}

extension ViewProbe {
    /// The labels `view` shows on its first layout, before the run loop
    /// turns (an async check has not finished yet). The probe is added to
    /// `kept` so the caller's tearDown closes it. (plan-68-AG)
    static func labelsOnFirstLayout<V: View>(_ view: V, kept: inout [ViewProbe]) -> [String] {
        enableAccessibility()
        let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 820, height: 1000), styleMask: [.titled],
                              backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        let host = NSHostingView(rootView: view)
        window.contentView = host
        host.layoutSubtreeIfNeeded()
        var found: [String] = []
        func walk(_ object: Any, _ depth: Int) {
            guard depth < 60, let element = object as? NSObject else { return }
            let label = Self.label(element)
            if !label.isEmpty { found.append(label) }
            for child in attribute(element, "accessibilityChildren") as? [Any] ?? [] { walk(child, depth + 1) }
        }
        walk(host, 0)
        // Closed by the caller's tearDown (ViewProbe settles, which is fine now).
        kept.append(ViewProbe(window: window, owns: true))
        return found
    }
}
