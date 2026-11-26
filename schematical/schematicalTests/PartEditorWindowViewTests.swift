import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AE: the Part Editor window as it is shown — the scene root, both
/// modes and the toolbar, the preview panes, the All fields form — with its
/// panels and alerts answered by the test and its User Library in a folder
/// the test made.
@MainActor
final class PartEditorWindowViewTests: XCTestCase {
    /// Kept for the life of the process (freeing a model inside a test aborts the host).
    private static var kept: [AnyObject] = []
    private var directory: URL!
    private var probes: [ViewProbe] = []
    private var windows: [NSWindow] = []
    private var panels = 0

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("pewindow-\(UUID().uuidString)")
        panels = 0
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
        windows.forEach { $0.close() }
        windows = []
        try? FileManager.default.removeItem(at: directory)
    }

    /// A catalog whose User Library is the test's folder.
    private func store() -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        store.attachUserLibrary(UserPartLibrary(directory: directory))
        Self.kept.append(store)
        return store
    }

    /// A model whose panels count and cancel, and whose alerts answer Cancel.
    private func model(_ source: PartEditorTarget.Source, catalog: PartCatalogStore = .shared) -> PartEditorModel {
        let model = PartEditorModel(target: PartEditorTarget(source: source), catalog: catalog)
        Self.kept.append(model)
        model.presenter.choose = { [weak self] _, _ in
            self?.panels += 1
            return nil
        }
        model.presenter.alert = { _, _ in .alertSecondButtonReturn }
        return model
    }

    private func show<V: View>(_ view: V, width: CGFloat = 1100) -> ViewProbe {
        let probe = ViewProbe(view, size: NSSize(width: width, height: 700))
        probes.append(probe)
        return probe
    }

    /// `view` in a window whose toolbar and title are the view's, as the scene's window is.
    private func host<V: View>(_ view: V) -> ViewProbe {
        let controller = NSHostingController(rootView: view)
        controller.sceneBridgingOptions = [.title, .toolbars]
        let window = NSWindow(contentViewController: controller)
        window.setContentSize(NSSize(width: 1200, height: 720))
        window.isReleasedWhenClosed = false
        windows.append(window)
        return ViewProbe(window: window)
    }

    /// Waits for the model's first check.
    private func checked(_ model: PartEditorModel) {
        let deadline = Date().addingTimeInterval(10)
        while model.validation == nil, Date() < deadline { RunLoop.main.run(until: Date().addingTimeInterval(0.05)) }
    }

    /// A value a binding reads and writes, as a parent view's state would hold
    /// it; observable, so a view reading it through the binding redraws.
    @Observable
    final class Box<T> {
        var value: T
        init(_ value: T) { self.value = value }
        var binding: Binding<T> { Binding(get: { self.value }, set: { self.value = $0 }) }
        nonisolated deinit {}
    }

    /// Opens the window's first pop-up menu (SwiftUI builds its items only
    /// then), reads its item titles, chooses the item titled `choose` (none
    /// when nil) and closes it again at once.
    @discardableResult
    static func pick(_ choose: String?, in probe: ViewProbe) throws -> [String] {
        let popUp = try XCTUnwrap(probe.views(NSPopUpButton.self).first, "no menu")
        var titles: [String] = []
        let timer = Timer(timeInterval: 0.2, repeats: false) { _ in
            MainActor.assumeIsolated {
                guard let menu = popUp.menu else { return }
                titles = menu.items.map(\.title).filter { !$0.isEmpty }
                if let choose, let index = menu.items.firstIndex(where: { $0.title == choose }) {
                    menu.cancelTracking()
                    menu.performActionForItem(at: index)
                } else {
                    menu.cancelTracking()
                }
            }
        }
        RunLoop.main.add(timer, forMode: .common)
        popUp.performClick(nil)
        probe.settle()
        return titles
    }

    /// Types into the `index`th editable text field (a grouped form's fields
    /// carry no label of their own), then presses Return.
    static func type(_ text: String, intoFieldAt index: Int, of probe: ViewProbe) throws {
        let fields = probe.views(NSTextField.self).filter(\.isEditable)
        XCTAssertGreaterThan(fields.count, index)
        let field = fields[index]
        XCTAssertTrue(probe.window.makeFirstResponder(field))
        let editor = try XCTUnwrap(field.currentEditor() as? NSTextView)
        editor.selectAll(nil)
        editor.insertText(text, replacementRange: editor.selectedRange())
        editor.insertNewline(nil)
        probe.settle()
    }

    // MARK: The scene root

    func testAFileSentToTheSceneFillsAnEmptyWindowOrOpensAnother() throws {
        let url = URL(fileURLWithPath: "/tmp/sent.schpart")
        var empty: PartEditorTarget?
        XCTAssertNil(PartEditorSceneRoot.open(url, in: &empty))
        XCTAssertEqual(empty?.source, .file(url))

        let own = PartEditorTarget(source: .copy(name: "SN74HC00N"))
        var full: PartEditorTarget? = own
        let other = try XCTUnwrap(PartEditorSceneRoot.open(url, in: &full))
        XCTAssertEqual(other.source, .file(url))
        XCTAssertEqual(full, own, "the window keeps its part")
        XCTAssertNotEqual(other.id, own.id)
    }

    func testTheSceneRootShowsItsEditorOrNothing() {
        let none = show(PartEditorSceneRoot(target: .constant(nil)))
        XCTAssertFalse(none.has("Identity"), "\(none.labels())")
        let one = show(PartEditorSceneRoot(target: .constant(PartEditorTarget(source: .copy(name: "SN74HC00N")))))
        XCTAssertTrue(one.has("Identity"), "\(one.labels())")
        XCTAssertTrue(one.has("SN74HC00N"))
    }

    // MARK: The window and its toolbar

    func testTheToolbarOpensSavesAndAdds() throws {
        let store = store()
        let model = model(.copy(name: "SN74HC00N"), catalog: store)
        checked(model)
        let probe = host(PartEditorWindowView(model: model, target: PartEditorTarget(source: .copy(name: "SN74HC00N"))))
        XCTAssertTrue(model.window === probe.window, "the window reader hands the model its window")
        XCTAssertTrue(probe.has("Identity"), "All fields shows the form: \(probe.labels())")
        XCTAssertTrue(probe.has("Block"), "and the preview")

        try probe.press("Save Part…")
        XCTAssertEqual(panels, 1, "a part never saved asks where")
        try probe.press("Open Part…")
        XCTAssertEqual(panels, 2, "an unchanged part opens without asking")
        XCTAssertNil(store.userLibraryItems.first)
        let name = model.libraryName
        try probe.press("Add to Library")
        XCTAssertEqual(model.status, "Added \(name) to your User Library")
        XCTAssertEqual(store.userLibraryItems.count, 1)
    }

    func testALibraryWindowHasNoAddButtonAndGuidedShowsTheSteps() throws {
        let store = store()
        let uuid = "AE000000-0000-4000-8000-000000000001"
        try store.addToUserLibrary(fileText: PartRegistrationTests.fileText(uuid: uuid, name: "AE-LIB"), source: "AE-LIB")
        let target = PartEditorTarget(source: .libraryPart(uuid: uuid))
        let model = model(target.source, catalog: store)
        checked(model)
        let probe = host(PartEditorWindowView(model: model, target: target))
        XCTAssertTrue(probe.has("Save Part…"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Add to Library"))

        model.mode = .guided
        probe.settle()
        XCTAssertTrue(probe.has("Back"), "the wizard's nav bar: \(probe.labels())")
        XCTAssertTrue(probe.has("Part number"), "the step list")
        XCTAssertTrue(probe.has("Datasheet"), "the guided pane's tabs")
    }

    // MARK: The preview panes

    func testTheGuidedPaneShowsTheDatasheetOrThePart() {
        let model = model(.copy(name: "SN74HC00N"))
        checked(model)
        XCTAssertNotNil(model.lastValidPart)
        model.guidedPane = .datasheet
        let probe = show(GuidedPreviewPane(model: model), width: 400)
        XCTAssertTrue(probe.has("No datasheet"), "\(probe.labels())")
        try? probe.press("Go to Datasheet")
        XCTAssertEqual(model.currentStepID, WizardStep.datasheet.id)

        model.guidedPane = .block
        probe.settle()
        XCTAssertFalse(probe.has("No datasheet"))
        XCTAssertFalse(probe.has("No preview"), "\(probe.labels())")
        model.guidedPane = .footprint
        probe.settle()
        XCTAssertFalse(probe.has("No preview"), "\(probe.labels())")
    }

    func testThePanesSayWhenNoPartHasLoaded() {
        let model = model(.copy(name: "NO-SUCH-PART"))
        model.draft.name = ""
        checked(model)
        XCTAssertNil(model.lastValidPart, "\(String(describing: model.validationMessage))")
        model.guidedPane = .footprint
        let guided = show(GuidedPreviewPane(model: model), width: 400)
        XCTAssertTrue(guided.has("No preview"), "\(guided.labels())")
        XCTAssertTrue(guided.has("The preview shows the part once it loads."))
        let form = show(PartPreviewPane(model: model), width: 400)
        XCTAssertTrue(form.has("No preview"), "\(form.labels())")
    }

    func testThePreviewPaneShowsBlockOrFootprint() {
        let model = model(.copy(name: "SN74HC00N"))
        checked(model)
        let probe = show(PartPreviewPane(model: model), width: 400)
        XCTAssertTrue(probe.has("Block"), "\(probe.labels())")
        XCTAssertFalse(probe.has("No preview"))
        model.previewPane = .footprint
        probe.settle()
        XCTAssertTrue(probe.has("Footprint"), "\(probe.labels())")
        XCTAssertFalse(probe.has("No preview"))
    }

    // MARK: The form

    func testTheFormShowsWhyATargetDidNotOpen() {
        let model = model(.copy(name: "NO-SUCH-PART"))
        let probe = show(PartEditorForm(model: model))
        XCTAssertTrue(probe.has("There is no part named NO-SUCH-PART to copy."), "\(probe.labels())")
        XCTAssertFalse(probe.has("Change…"), "a copy is not a new part")
    }

    func testANewPartShowsItsKindAndChangeGoesBackToTheKindStep() throws {
        let wizard = model(.wizard)
        wizard.mode = .allFields
        let unchosen = show(PartEditorForm(model: wizard))
        XCTAssertTrue(unchosen.has("Not chosen yet"), "\(unchosen.labels())")

        let gate = model(.new(kind: "gate"))
        gate.mode = .allFields
        let probe = show(PartEditorForm(model: gate))
        XCTAssertTrue(probe.has("Logic Gate"), "\(probe.labels())")
        try probe.press("Change…")
        XCTAssertEqual(gate.mode, .guided)
        XCTAssertEqual(gate.currentStepID, WizardStep.kind.id)
    }

    func testKeysTheFormDoesNotKnowAreListed() {
        let model = model(.copy(name: "SN74HC00N"))
        model.draft.deviceExtras["zzUnknown"] = .string("kept")
        let probe = show(PartEditorForm(model: model))
        XCTAssertTrue(probe.has("Kept as they are"), "\(probe.labels())")
        XCTAssertTrue(probe.has("zzUnknown"))
    }

    // MARK: Footprint choices and the datasheet field

    func testFootprintChoicesAddAndRemove() throws {
        let box = Box<[FootprintChoiceDraft]?>(nil)
        let probe = show(Form { FootprintChoicesEditor(choices: box.binding, ids: []) }, width: 600)
        XCTAssertFalse(probe.has("Remove Choice"))
        try probe.press("Add Choice")
        XCTAssertEqual(box.value, [FootprintChoiceDraft(package: "", footprint: "")])
        try probe.press("Remove Choice")
        XCTAssertNil(box.value, "with none left the key is absent")

        let a = FootprintChoiceDraft(package: "DIP", footprint: "DIP-14")
        let b = FootprintChoiceDraft(package: "SOIC", footprint: "SOIC-14")
        XCTAssertEqual(FootprintChoicesEditor.adding(to: [a]), [a, FootprintChoiceDraft(package: "", footprint: "")])
        XCTAssertEqual(FootprintChoicesEditor.removing(a.id, from: [a, b]), [b])
        XCTAssertNil(FootprintChoicesEditor.removing(a.id, from: [a]))
        XCTAssertNil(FootprintChoicesEditor.removing(a.id, from: nil))
    }

    func testTheDatasheetMenuOffersMatchingIds() throws {
        let ids = (1...50).map { String(format: "ID-%02d", $0) } + ["sn74hc00", "SN74HC04"]
        XCTAssertEqual(DatasheetField.matches(ids, typed: "SN74HC"), ["SN74HC04", "sn74hc00"])
        XCTAssertEqual(DatasheetField.matches(ids, typed: nil).count, 40)
        XCTAssertEqual(DatasheetField.matches(ids, typed: "").first, "ID-01")

        let box = Box<String?>("hc0")
        let probe = show(Form { DatasheetField(datasheet: box.binding, ids: ids) }, width: 500)
        XCTAssertEqual(try Self.pick("SN74HC04", in: probe), ["None", "SN74HC04", "sn74hc00"])
        XCTAssertEqual(box.value, "SN74HC04")
        try Self.pick("None", in: probe)
        XCTAssertNil(box.value)
        try Self.type("ID-07", intoFieldAt: 0, of: probe)
        XCTAssertEqual(box.value, "ID-07")
        try Self.type("", intoFieldAt: 0, of: probe)
        XCTAssertNil(box.value, "an emptied field removes the id")
    }
}
