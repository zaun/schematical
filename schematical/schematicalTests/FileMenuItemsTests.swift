import AppKit
import SwiftUI
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// plan-68-Y: the File, Edit and Window menu items the app adds, rendered as
/// the views they are and pressed through the accessibility tree; what each
/// item does is answered by the test (`ExportAnswers`).
@MainActor
final class FileMenuItemsTests: XCTestCase {
    private var parent: NSWindow!
    private var answers: ExportAnswers!
    private var probes: [ViewProbe] = []
    /// Models kept for the life of the process: freeing one in a test aborts the host.
    private static var kept: [AnyObject] = []

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
        parent = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 900, height: 700), styleMask: [.titled],
                          backing: .buffered, defer: false)
        parent.isReleasedWhenClosed = false
        parent.orderFront(nil)
        answers = ExportAnswers()
        PartFileLoading.presenter = answers.presenter
    }

    override func tearDown() async throws {
        PartFileLoading.presenter = ExportPresenter()
        for probe in probes { probe.close() }
        probes = []
        if let sheet = parent.attachedSheet { parent.endSheet(sheet) }
        parent.close()
    }

    private func probe<V: View>(_ view: V) -> ViewProbe {
        let probe = ViewProbe(view, size: NSSize(width: 400, height: 400))
        probes.append(probe)
        return probe
    }

    private func exporter() throws -> SchematicExporter {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let file = SchematicExporter(document: doc, catalog: TestSupport.catalog)
        file.window = parent
        file.presenter = answers.presenter
        return file
    }

    private func wait(_ timeout: TimeInterval = 5, until condition: () -> Bool) async {
        let end = Date().addingTimeInterval(timeout)
        while !condition(), Date() < end { try? await Task.sleep(for: .milliseconds(50)) }
    }

    /// Cancels the sheet on the test's window once it is up; its labels.
    private func cancelSheet() async throws -> [String] {
        await wait { parent.attachedSheet != nil }
        let sheet = ViewProbe(window: try XCTUnwrap(parent.attachedSheet, "no sheet"))
        let labels = sheet.labels()
        try sheet.press("Cancel")
        await wait { parent.attachedSheet == nil }
        return labels
    }

    // MARK: File › Import and Export

    func testWithNoSchematicWindowTheItemsAreOff() {
        let export = probe(SchematicExportItems(file: nil))
        for title in ["Image…", "PDF…", "Gerber PCB…", "KiCad Project…", "EasyEDA Pro Project…"] {
            XCTAssertFalse(ViewProbe.isEnabled((try? export.element(title, role: "AXButton")) ?? NSObject()), title)
        }
        let other = probe(VStack { SchematicImportItems(file: nil); SchematicFileSaveItems(file: nil, editor: nil) })
        XCTAssertFalse(ViewProbe.isEnabled(try XCTUnwrap(try? other.element("DXF…", role: "AXButton"))))
        XCTAssertFalse(ViewProbe.isEnabled(try XCTUnwrap(try? other.element("Print…", role: "AXButton"))))
        XCTAssertFalse(other.has("Save Part…"), "only in a Part Editor window")
        // The menus themselves.
        let menus = probe(VStack { SchematicImportMenu(file: nil); SchematicExportMenu(file: nil) })
        XCTAssertTrue(menus.has("Import"), "\(menus.labels())")
        XCTAssertTrue(menus.has("Export"))
    }

    func testEachExportItemStartsItsExport() async throws {
        let file = try exporter()
        let export = probe(SchematicExportItems(file: file))

        try export.press("Image…")
        let image = try await cancelSheet()
        XCTAssertTrue(image.contains { $0.contains("Export Image") }, "\(image)")
        try export.press("PDF…")
        let pdf = try await cancelSheet()
        XCTAssertTrue(pdf.contains { $0.contains("Export PDF") }, "\(pdf)")

        try export.press("Gerber PCB…")
        await wait { answers.panels.count == 1 || !answers.alerts.isEmpty }
        let gerber = answers.panels.count == 1 ? answers.panels[0].nameFieldStringValue : answers.alerts.first?.messageText ?? ""
        XCTAssertTrue(gerber.contains("Gerbers.zip") || gerber.contains("design rule"), gerber)

        let before = answers.panels.count
        try export.press("KiCad Project…")
        await wait { answers.panels.count == before + 1 }
        XCTAssertTrue(answers.panels.last is NSOpenPanel, "a folder")
        try export.press("EasyEDA Pro Project…")
        await wait { answers.panels.count == before + 2 }
        XCTAssertEqual(answers.panels.last?.nameFieldStringValue, "Untitled.epro")

        let imports = probe(SchematicImportItems(file: file))
        try imports.press("DXF…")
        await wait { answers.panels.count == before + 3 }
        XCTAssertEqual(answers.panels.last?.allowedContentTypes.first?.preferredFilenameExtension, "dxf")
    }

    func testPrintOpensThePagesSheet() async throws {
        let items = probe(SchematicFileSaveItems(file: try exporter(), editor: nil))
        try items.press("Print…")
        let sheet = try await cancelSheet()
        XCTAssertTrue(sheet.contains { $0 == "Print" || $0.hasPrefix("Print |") }, "the Print sheet: \(sheet)")
    }

    // MARK: The part file items

    func testLoadPartAndUserLibraryItems() async throws {
        let items = probe(PartFileOpenItems(editor: nil))
        XCTAssertFalse(items.has("Open Part…"), "only in a Part Editor window")
        try items.press("Load Part…")
        await wait { !answers.panels.isEmpty }
        XCTAssertEqual(answers.panels.first?.message, "Choose a part file to load")

        func libraryWindows() -> [NSWindow] { NSApp.windows.filter { $0.isVisible && $0.title == "User Library" } }
        let open = libraryWindows()
        try items.press("User Library…")
        await wait { libraryWindows().count > open.count }
        let opened = libraryWindows().filter { !open.contains($0) }
        XCTAssertEqual(opened.count, 1, "\(NSApp.windows.map(\.title))")
        for window in opened { window.close() }
    }

    func testOpenAndSavePartInAPartEditorWindow() async throws {
        let editor = PartEditorModel(target: PartEditorTarget(source: .copy(name: "SN74HC00N")))
        Self.kept.append(editor)
        _ = editor.validateNow()
        XCTAssertTrue(editor.isValid)
        editor.window = parent
        let open = probe(PartFileOpenItems(editor: editor))
        try open.press("Open Part…")
        await wait { parent.attachedSheet != nil }
        let panel = try XCTUnwrap(parent.attachedSheet as? NSOpenPanel, "the open panel on the editor's window")
        panel.cancel(nil)
        await wait { parent.attachedSheet == nil }

        let save = probe(SchematicFileSaveItems(file: nil, editor: editor))
        let item = try save.element("Save Part…", role: "AXButton")
        XCTAssertEqual(ViewProbe.isEnabled(item), editor.isValid)
        if editor.isValid {
            try save.press("Save Part…")
            await wait { parent.attachedSheet != nil }
            let savePanel = try XCTUnwrap(parent.attachedSheet as? NSSavePanel)
            savePanel.cancel(nil)
            await wait { parent.attachedSheet == nil }
        }
    }

    // MARK: Window › Electrical Rules Check

    func testTheCheckItemIsOnOnTheSchematicPageOnly() throws {
        let model = TestSupport.systemDocument(of: TestSupport.newDocument()).model
        var opened = 0
        model.menus.openElectricalRulesCheck = { opened += 1 }
        let items = probe(ElectricalRulesCheckMenuItem(editor: model))
        try items.press("Electrical Rules Check")
        XCTAssertEqual(opened, 1)
        model.page = .pcb
        items.settle()
        XCTAssertFalse(ViewProbe.isEnabled(try items.element("Electrical Rules Check", role: "AXButton")))
        let none = probe(ElectricalRulesCheckMenuItem(editor: nil))
        XCTAssertFalse(ViewProbe.isEnabled(try none.element("Electrical Rules Check", role: "AXButton")))
    }

    // MARK: Edit

    /// What the Edit items did, by name.
    private final class Log {
        nonisolated deinit {}
        var done: [String] = []
        var answer = true
    }

    private func actions(_ log: Log, canFlip: Bool = true, canRotate: Bool = true, canReset: Bool = true) -> schematical.EditActions {
        func record(_ name: String) -> () -> Bool { { log.done.append(name); return log.answer } }
        return schematical.EditActions(cut: record("cut"), copy: record("copy"), paste: record("paste"), delete: record("delete"),
                           selectAll: record("selectAll"), flip: record("flip"), canFlip: canFlip,
                           rotate: { log.done.append($0 ? "rotate ccw" : "rotate cw"); return true }, canRotate: canRotate,
                           resetReferences: { log.done.append("reset") }, canResetReferences: canReset)
    }

    func testEachEditItemActsOnTheSchematic() throws {
        let model = TestSupport.systemDocument(of: TestSupport.newDocument()).model
        let log = Log()
        model.menus.edit = { self.actions(log) }
        let items = probe(EditMenuItems(editor: model))
        // The probe's window is not key, so no text field has the keys.
        for title in ["Cut", "Copy", "Paste", "Delete", "Select All", "Flip", "Rotate Clockwise",
                      "Rotate Counterclockwise", "Reset References…"] {
            try items.press(title)
        }
        XCTAssertEqual(log.done, ["cut", "copy", "paste", "delete", "selectAll", "flip", "rotate cw", "rotate ccw", "reset"])

        let off = probe(EditMenuItems(editor: nil))
        for title in ["Flip", "Rotate Clockwise", "Reset References…"] {
            XCTAssertFalse(ViewProbe.isEnabled(try off.element(title, role: "AXButton")), title)
        }
    }

    func testAnEditItemGoesToTheTextBeingEditedOrBeepsWhenThereIsNothingToDo() {
        let log = Log()
        let target = actions(log)
        var sent: [Selector] = []
        var beeps = 0
        func run(_ item: EditMenuItem, _ target: schematical.EditActions?, _ responder: NSResponder?) {
            item.perform(on: target, firstResponder: responder, send: { sent.append($0) }, beep: { beeps += 1 })
        }
        for item in EditMenuItem.allCases { run(item, target, nil) }
        XCTAssertEqual(log.done, ["cut", "copy", "paste", "delete", "selectAll"])
        XCTAssertEqual(beeps, 0)

        log.answer = false
        run(.paste, target, nil)
        XCTAssertEqual(beeps, 1, "nothing to paste")

        // A field being edited gets every item; no schematic window, likewise.
        let field = NSTextView()
        field.isEditable = true
        for item in EditMenuItem.allCases { run(item, target, field) }
        run(.copy, nil, nil)
        XCTAssertEqual(sent, [#selector(NSText.cut(_:)), #selector(NSText.copy(_:)), #selector(NSText.paste(_:)),
                              #selector(NSText.delete(_:)), #selector(NSText.selectAll(_:)), #selector(NSText.copy(_:))])

        // Text that can only be selected gets Copy while some of it is selected.
        let label = NSTextView()
        label.string = "U1"
        label.isEditable = false
        label.isSelectable = true
        XCTAssertFalse(EditMenuItem.copy.goesToText(label))
        label.setSelectedRange(NSRange(location: 0, length: 2))
        XCTAssertTrue(EditMenuItem.copy.goesToText(label))
        XCTAssertFalse(EditMenuItem.cut.goesToText(label))
        XCTAssertFalse(EditMenuItem.copy.goesToText(NSView()))
    }

    @objc private func nothingHandlesThis(_ sender: Any?) {}

    func testTheResponderChainGetsTheItemWithNoSchematic() throws {
        // The application is the end of every chain: it takes what it answers, and nothing takes the rest.
        XCTAssertTrue(EditMenuItem.sendToResponders(#selector(NSApplication.updateWindows)))
        XCTAssertFalse(EditMenuItem.sendToResponders(#selector(nothingHandlesThis(_:))))
        // With no schematic window the items go down the chain (here no one edits text, so nothing changes).
        let off = probe(EditMenuItems(editor: nil))
        let before = NSPasteboard.general.changeCount
        try off.press("Copy")
        XCTAssertEqual(NSPasteboard.general.changeCount, before)
    }

    func testTheEditMenuEndsAtResetReferences() throws {
        let edit = try XCTUnwrap(NSApp.mainMenu?.items.first { $0.title == "Edit" }?.submenu)
        EditMenuTrimmer.install()
        edit.addItem(withTitle: "Added by AppKit", action: nil, keyEquivalent: "")
        RunLoop.main.run(until: Date().addingTimeInterval(0.3))
        XCTAssertEqual(edit.items.last?.title, "Reset References…", "\(edit.items.map(\.title))")
        // Opening a menu trims too.
        edit.addItem(withTitle: "Slipped by", action: nil, keyEquivalent: "")
        NotificationCenter.default.post(name: NSMenu.didBeginTrackingNotification, object: edit)
        XCTAssertEqual(edit.items.last?.title, "Reset References…", "\(edit.items.map(\.title))")
        // Any other menu is left alone.
        let other = NSMenu(title: "Other")
        other.addItem(withTitle: "Kept", action: nil, keyEquivalent: "")
        RunLoop.main.run(until: Date().addingTimeInterval(0.1))
        XCTAssertEqual(other.items.map(\.title), ["Kept"])
    }

    // MARK: The child windows

    func testEveryChildWindowOpensWithItsTitleAndAnUnknownTargetOpensNothing() throws {
        let system = TestSupport.openSystemDocument(of: TestSupport.newDocument(), catalog: TestSupport.catalog, until: self)
        let id = system.model.id
        let chip = UUID()
        let open = ChildWindowOpener(documentID: id)
        defer { ChildWindowCenter.shared.closeAll(id) }
        open(id: "cpu", value: CpuWindowTarget(documentID: id, chipID: chip))
        open(id: "registers", value: RegisterWindowTarget(documentID: id, chipID: chip))
        open(id: "terminal", value: TerminalWindowTarget(documentID: id, chipID: chip))
        open(id: "erc", value: ElectricalRulesCheckTarget(documentID: id))
        open(id: "preview", value: BoardPreviewTarget(documentID: id))
        let titles = ["Processor", "Registers", "Terminal", "Electrical Rules Check", "Board Preview"]
        XCTAssertEqual(Set(ChildWindowCenter.shared.windows(of: id).map(\.title)), Set(titles))
        let count = ChildWindowCenter.shared.windows(of: id).count
        XCTAssertEqual(count, titles.count)
        open(id: "nothing", value: "not a target")
        XCTAssertEqual(ChildWindowCenter.shared.windows(of: id).count, count)
    }
}
