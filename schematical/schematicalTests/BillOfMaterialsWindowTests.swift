import AppKit
import SwiftUI
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// plan-68-W: the Bill of Materials window — its model (checkmarks, the
/// summary, the copy, both exports with the save panel answered), the table
/// and toolbar as shown, and the Window menu's item.
@MainActor
final class BillOfMaterialsWindowTests: XCTestCase {
    private var opened: [UUID] = []
    private var folder: URL!

    override func setUp() {
        super.setUp()
        AppDelegate.quitsAfterLastWindowCloses = false
        folder = FileManager.default.temporaryDirectory.appendingPathComponent("BillOfMaterialsWindowTests-\(UUID().uuidString)")
        try? FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
    }

    override func tearDown() {
        for id in opened { ChildWindowCenter.shared.closeAll(id) }
        opened = []
        try? FileManager.default.removeItem(at: folder)
        super.tearDown()
    }

    /// Two SMD gates, a through-hole LED and a resistor: three lines, one with a color, one with a value.
    private func document() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 300, y: 100))
        try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 400))
        try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 300, y: 400))
        return doc
    }

    // MARK: Model

    func testCheckmarksAndTheSummary() throws {
        let model = BillOfMaterialsModel(document: try document())
        let lines = model.lines
        XCTAssertEqual(lines.count, 3)
        XCTAssertTrue(model.allChecked)
        XCTAssertEqual(model.summary, "4 parts, 3 lines")
        model.setChecked(lines[0].id, false)
        XCTAssertFalse(model.isChecked(lines[0].id))
        XCTAssertFalse(model.allChecked)
        XCTAssertEqual(model.summary, "4 parts, 3 lines, 2 checked")
        model.setChecked(lines[0].id, true)
        XCTAssertTrue(model.allChecked)
        model.setAllChecked(false)
        XCTAssertEqual(model.checked, [])
        model.setAllChecked(true)
        XCTAssertEqual(model.checked, lines)

        let one = TestSupport.newDocument()
        try TestSupport.place(one, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(BillOfMaterialsModel(document: one).summary, "1 part, 1 line")
        let none = BillOfMaterialsModel(document: TestSupport.newDocument())
        XCTAssertFalse(none.allChecked, "no lines: Select All is off")
        XCTAssertEqual(none.summary, "0 parts, 0 lines")
    }

    func testCopyTakesTheSelectionElseTheCheckedLines() throws {
        let model = BillOfMaterialsModel(document: try document())
        let lines = model.lines
        XCTAssertEqual(model.copyText, BillOfMaterials.csv(lines))
        model.setChecked(lines[2].id, false)
        XCTAssertEqual(model.copyText, BillOfMaterials.csv(Array(lines.prefix(2))))
        model.selection = [lines[2].id]
        XCTAssertEqual(model.copyText, BillOfMaterials.csv([lines[2]]), "a selected line, checked or not")
        let items = model.copyItems()
        XCTAssertEqual(items.count, 1)
        XCTAssertTrue(items[0].canLoadObject(ofClass: NSString.self))
    }

    func testTheExportsWriteTheChosenFileOrSayWhyNot() async throws {
        let model = BillOfMaterialsModel(document: try document())
        var names: [String] = []
        var target: URL? = folder.appendingPathComponent("bom.csv")
        model.presenter.choose = { panel, window in
            XCTAssertNil(window)
            XCTAssertEqual(panel.allowedContentTypes, [.commaSeparatedText])
            names.append(panel.nameFieldStringValue)
            return target
        }
        var alerts: [String] = []
        model.presenter.alert = { alert, _ in alerts.append(alert.messageText); return .OK }

        await model.exportCSV()
        XCTAssertEqual(try String(contentsOf: folder.appendingPathComponent("bom.csv"), encoding: .utf8),
                       BillOfMaterials.csv(model.checked))
        target = folder.appendingPathComponent("jlc.csv")
        await model.exportJLCPCB()
        XCTAssertEqual(try String(contentsOf: folder.appendingPathComponent("jlc.csv"), encoding: .utf8),
                       BillOfMaterials.jlcpcbCSV(model.checked))
        XCTAssertEqual(names, ["Bill of Materials.csv", "Bill of Materials JLCPCB.csv"])

        target = nil
        await model.exportCSV()
        XCTAssertEqual(names.count, 3)
        XCTAssertEqual(alerts, [], "cancelled: nothing written, nothing said")

        target = folder.appendingPathComponent("no such folder/bom.csv")
        await model.exportCSV()
        XCTAssertEqual(alerts.count, 1, "a file that can't be written is an alert")
    }

    // MARK: Drawn

    func testTheTableAndItsToolbar() throws {
        let model = BillOfMaterialsModel(document: try document())
        var saved = 0
        model.presenter.choose = { _, _ in saved += 1; return nil }
        let probe = ViewProbe(BillOfMaterialsTable(model: model), size: NSSize(width: 900, height: 400))
        defer { probe.close() }
        let table = try XCTUnwrap(probe.views(NSTableView.self).first)
        for _ in 0..<30 where table.numberOfRows < 3 { probe.settle(0.1) }
        XCTAssertEqual(table.numberOfRows, 3)
        XCTAssertEqual(probe.window.subtitle, "4 parts, 3 lines")
        let headers = table.tableColumns.map(\.title)
        XCTAssertTrue(headers.contains("Value") && headers.contains("Color"), "\(headers)")

        // A checkmark in each row (the model's checkmarks are tested above).
        for row in 0..<3 { _ = table.view(atColumn: 0, row: row, makeIfNecessary: true) }
        XCTAssertEqual(probe.views(NSButton.self).filter { $0.isDescendant(of: table) }.count, 3)
        model.setChecked(model.lines[0].id, false)
        probe.settle()
        XCTAssertEqual(model.checked.count, 2)
        XCTAssertEqual(probe.window.subtitle, "4 parts, 3 lines, 2 checked")

        try probe.press("Export CSV…")
        try probe.press("Export JLCPCB / PCBWay BOM…")
        XCTAssertEqual(saved, 2)

        let selectAll = try probe.element("Select All", role: "AXCheckBox")
        _ = selectAll.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        XCTAssertTrue(model.allChecked)
        _ = selectAll.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        XCTAssertEqual(model.checked, [])
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Export CSV…", role: "AXButton")))

        table.selectRowIndexes(IndexSet(integer: 1), byExtendingSelection: false)
        probe.settle()
        XCTAssertEqual(model.selection, [model.lines[1].id])
    }

    func testTheWindowOfAnOpenSchematicAndOfAClosedOne() throws {
        let doc = try document()
        let system = TestSupport.openSystemDocument(of: doc, catalog: TestSupport.catalog, until: self)
        let id = system.model.id
        opened.append(id)
        ChildWindowOpener(documentID: id)(id: "bom", value: BillOfMaterialsTarget(documentID: id))
        let window = try XCTUnwrap(ChildWindowCenter.shared.windows(of: id).first)
        let probe = ViewProbe(window: window)
        XCTAssertEqual(probe.views(NSTableView.self).first?.numberOfRows, 3)
        XCTAssertEqual(window.subtitle, "4 parts, 3 lines")

        let empty = ViewProbe(BillOfMaterialsTable(document: TestSupport.newDocument()))
        defer { empty.close() }
        XCTAssertTrue(empty.has("No Parts"), "\(empty.labels())")
        let closed = ViewProbe(BillOfMaterialsWindowView(target: BillOfMaterialsTarget(documentID: UUID())))
        defer { closed.close() }
        XCTAssertTrue(closed.has("Schematic Closed"), "\(closed.labels())")
    }

    func testTheMenuItemOpensTheWindowWhenItCan() throws {
        let editor = TestSupport.systemDocument(of: TestSupport.newDocument()).model
        var opens = 0
        editor.menus.openBillOfMaterials = { opens += 1 }
        let probe = ViewProbe(BillOfMaterialsMenuItem(editor: editor))
        defer { probe.close() }
        try probe.press("Bill of Materials")
        XCTAssertEqual(opens, 1)
        let none = ViewProbe(BillOfMaterialsMenuItem(editor: nil))
        defer { none.close() }
        XCTAssertFalse(ViewProbe.isEnabled(try none.element("Bill of Materials", role: "AXButton")))
    }
}
