import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-W: the Document Info window and form as shown, the keyboard help
/// window, the About panel's menu item, the Window and Simulation menus'
/// items (drawn with an editor's menu closures), and small backend values
/// (header names, the User Library's folder).
@MainActor
final class WindowMenuItemsTests: XCTestCase {
    private var aboutWindows: [NSWindow] = []

    override func setUp() {
        super.setUp()
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() {
        for window in aboutWindows { window.close() }
        aboutWindows = []
        super.tearDown()
    }

    /// The form's toggles in order (their labels are separate text): Include on Schematic, Set dates automatically.
    private func toggle(_ probe: ViewProbe, _ index: Int) throws -> NSObject {
        let toggles = probe.elements().filter { ["AXCheckBox", "AXSwitch", "AXToggle"].contains(ViewProbe.role($0)) }
        return try XCTUnwrap(toggles.indices.contains(index) ? toggles[index] : nil,
                             "\(probe.elements().map { ViewProbe.role($0) + " " + ViewProbe.label($0) })")
    }

    private func editor() -> SchematicEditorModel {
        TestSupport.systemDocument(of: TestSupport.newDocument()).model
    }

    // MARK: Document Info

    func testTheFormEditsTheDocument() throws {
        let doc = TestSupport.newDocument()
        let probe = ViewProbe(Form { DocumentInfoForm(document: doc, isLocked: false) }.formStyle(.grouped),
                              size: NSSize(width: 480, height: 620))
        defer { probe.close() }
        XCTAssertTrue(probe.has("Set when the file is saved"), "\(probe.labels())")
        try probe.type("Clock", intoFieldAt: 0)
        XCTAssertEqual(doc.documentInfo.title, "Clock")
        try probe.type("J. Smith", intoFieldAt: 1)
        XCTAssertEqual(doc.documentInfo.author, "J. Smith")
        try probe.type("B", intoFieldAt: 2)
        XCTAssertEqual(doc.documentInfo.revision, "B")

        let include = try toggle(probe, 0)
        _ = include.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        XCTAssertNotNil(doc.documentInfo.titleBlock)

        let automatic = try toggle(probe, 1)
        _ = automatic.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        XCTAssertFalse(doc.documentInfo.automaticDates)
        XCTAssertEqual(probe.find("Set to Now", role: "AXButton").count, 2, "\(probe.labels())")
        try probe.press("Set to Now")
        XCTAssertNotNil(doc.documentInfo.created ?? doc.documentInfo.updated)
        XCTAssertEqual(probe.find("Set to Now", role: "AXButton").count, 1, "a date is now a picker")

        doc.setAutomaticDates(true)
        doc.stampDates(now: Date(timeIntervalSince1970: 1_000_000))
        probe.settle()
        XCTAssertTrue(probe.has(DocumentInfo.dateText(Date(timeIntervalSince1970: 1_000_000))), "\(probe.labels())")
    }

    func testALockedFormChangesNothing() throws {
        let probe = ViewProbe(Form { DocumentInfoForm(document: TestSupport.newDocument(), isLocked: true) },
                              size: NSSize(width: 480, height: 620))
        defer { probe.close() }
        XCTAssertFalse(ViewProbe.isEnabled(try toggle(probe, 0)))
    }

    func testTheWindowOfAnOpenSchematicAndOfAClosedOne() {
        let system = TestSupport.openSystemDocument(of: TestSupport.newDocument(), catalog: TestSupport.catalog, until: self)
        let open = ViewProbe(DocumentInfoWindowView(target: DocumentInfoTarget(documentID: system.model.id)))
        defer { open.close() }
        XCTAssertTrue(open.has("Include on Schematic"), "\(open.labels())")
        let closed = ViewProbe(DocumentInfoWindowView(target: DocumentInfoTarget(documentID: UUID())))
        defer { closed.close() }
        XCTAssertTrue(closed.has("Schematic Closed"), "\(closed.labels())")
    }

    // MARK: Menu items

    func testTheWindowMenuItemsOpenTheirWindows() throws {
        let editor = editor()
        var opened: [String] = []
        editor.menus.openDocumentInfo = { opened.append("info") }
        editor.menus.openDesignRuleCheck = { opened.append("drc") }
        let info = ViewProbe(DocumentInfoMenuItem(editor: editor))
        defer { info.close() }
        try info.press("Document Info")
        let drcOnSchematic = ViewProbe(DesignRuleCheckMenuItem(editor: editor))
        defer { drcOnSchematic.close() }
        XCTAssertFalse(ViewProbe.isEnabled(try drcOnSchematic.element("Design Rule Check", role: "AXButton")),
                       "the check is the PCB's")
        editor.page = .pcb
        let drc = ViewProbe(DesignRuleCheckMenuItem(editor: editor))
        defer { drc.close() }
        try drc.press("Design Rule Check")
        XCTAssertEqual(opened, ["info", "drc"])
        let none = ViewProbe(DocumentInfoMenuItem(editor: nil))
        defer { none.close() }
        XCTAssertFalse(ViewProbe.isEnabled(try none.element("Document Info", role: "AXButton")))
    }

    func testTheSimulationMenuRunsStopsAndSaysWhyNot() throws {
        let editor = editor()
        var toggles = 0
        var target = SimulationTarget(isAvailable: false, blockedReason: "U1 (ATMEGA328P-PU) is not simulated",
                                      isRunning: false, toggle: { toggles += 1 })
        editor.menus.simulation = { target }
        let blocked = ViewProbe(SimulationMenuItems(editor: editor))
        defer { blocked.close() }
        XCTAssertTrue(blocked.has("U1 (ATMEGA328P-PU) is not simulated"), "\(blocked.labels())")
        XCTAssertFalse(ViewProbe.isEnabled(try blocked.element("Run", role: "AXButton")))

        target = SimulationTarget(isAvailable: true, blockedReason: nil, isRunning: true, toggle: { toggles += 1 })
        let running = ViewProbe(SimulationMenuItems(editor: editor))
        defer { running.close() }
        try running.press("Stop")
        XCTAssertEqual(toggles, 1)

        let key = "simulationSpeed"
        let saved = UserDefaults.standard.object(forKey: key)
        defer { UserDefaults.standard.set(saved, forKey: key) }
        UserDefaults.standard.set(1.0, forKey: key)
        try running.choose("1/100")
        XCTAssertEqual(UserDefaults.standard.double(forKey: key), 1e-2)
    }

    // MARK: Help

    func testKeyboardHelpListsEachPagesKeys() throws {
        for page in EditorPage.allCases {
            let probe = ViewProbe(KeyboardHelpView(page: page), size: NSSize(width: 600, height: 700))
            defer { probe.close() }
            let first = try XCTUnwrap(page.helpSections.first)
            XCTAssertTrue(probe.has(first.title), "\(page): \(probe.labels())")
            XCTAssertTrue(probe.has(try XCTUnwrap(first.entries.first).action))
        }
    }

    /// Schematical › About Schematical puts up the standard panel with the
    /// acknowledgements as its credits.
    func testTheAboutItemShowsTheAcknowledgements() throws {
        let menu = try XCTUnwrap(NSApp.mainMenu?.items.first?.submenu)
        let item = try XCTUnwrap(menu.items.first { $0.title == "About Schematical" }, "\(menu.items.map(\.title))")
        let before = Set(NSApp.windows.filter(\.isVisible).map(ObjectIdentifier.init))
        XCTAssertTrue(NSApp.sendAction(try XCTUnwrap(item.action), to: item.target, from: item))
        RunLoop.main.run(until: Date().addingTimeInterval(0.5))
        let shown = NSApp.windows.filter { $0.isVisible && !before.contains(ObjectIdentifier($0)) }
        aboutWindows = shown
        let panel = try XCTUnwrap(shown.first, "the About panel")
        let text = ViewProbe(window: panel).labels().joined(separator: "\n")
        XCTAssertTrue(text.contains("Acknowledgements") || text.contains("cJSON"), text)
    }

    // MARK: Backend values

    func testHeaderNamesPadThePinCount() {
        XCTAssertEqual(HeaderFamily.name(rows: 2, perRow: 8), "GENERIC-HEADER-2X08")
        XCTAssertEqual(HeaderFamily.name(rows: 1, perRow: 40), "GENERIC-HEADER-1X40")
        XCTAssertEqual(HeaderFamily.name(rows: 1, perRow: 10), "GENERIC-HEADER-1X10")
        XCTAssertNotNil(TestSupport.catalog.part(named: HeaderFamily.name(rows: 2, perRow: 8)))
    }

    func testTheUserLibrarysFolderOutsideAndInsideTests() throws {
        let app = UserPartLibrary.directory(environment: [:])
        XCTAssertEqual(app.lastPathComponent, "UserLibrary")
        XCTAssertEqual(app.deletingLastPathComponent().lastPathComponent, "Application Support")
        XCTAssertEqual(UserPartLibrary.directory(environment: ["XCTestConfigurationFilePath": "x"]),
                       UserPartLibrary.defaultDirectory)
        XCTAssertTrue(UserPartLibrary.defaultDirectory.path.hasPrefix(FileManager.default.temporaryDirectory.path))

        let folder = FileManager.default.temporaryDirectory.appendingPathComponent("userlib-\(UUID().uuidString)")
        defer { try? FileManager.default.removeItem(at: folder) }
        let unreadable = folder.appendingPathComponent("a-folder.\(PartFileType.fileExtension)")
        try FileManager.default.createDirectory(at: unreadable, withIntermediateDirectories: true)
        let problems = UserPartLibrary(directory: folder).files().problems
        XCTAssertEqual(problems.map(\.reason), ["The file could not be read."])
        XCTAssertEqual(problems.first?.id.resolvingSymlinksInPath().path, unreadable.resolvingSymlinksInPath().path)
    }
}
