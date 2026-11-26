import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// The net label panel (plan-68-AB): its name field's rules
/// (`NetLabelEditing`) and the panel as shown.
@MainActor
final class NetLabelPanelTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func setUp() {
        super.setUp()
        // The test host stays up when the test closes its last window.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() {
        probes.forEach { $0.close() }
        probes = []
        super.tearDown()
    }

    /// The panel for the label `id`, redrawn as the document changes (as the sidebar shows it).
    private struct Host: View {
        let document: SchematicDocument
        let id: UUID
        var body: some View {
            Form {
                if let label = document.netLabels.first(where: { $0.id == id }) {
                    NetLabelPanel(document: document, label: label)
                }
            }
        }
    }

    private func show(_ doc: SchematicDocument, _ id: UUID) -> ViewProbe {
        let probe = ViewProbe(Host(document: doc, id: id), size: NSSize(width: 380, height: 400))
        probes.append(probe)
        return probe
    }

    private func label(_ doc: SchematicDocument, _ id: UUID) throws -> NetLabel {
        try XCTUnwrap(doc.netLabels.first { $0.id == id })
    }

    func testFooterCountsTheOtherLabelsOnTheNet() throws {
        let doc = TestSupport.newDocument()
        let a = doc.addNetLabel(at: CGPoint(x: 100, y: 100))
        let editing = { NetLabelEditing(document: doc, label: try self.label(doc, a)) }
        let name = try label(doc, a).name
        XCTAssertEqual(try editing().others, 0)
        XCTAssertEqual(try editing().footer, "No other label is named \(name).")
        let b = doc.addNetLabel(at: CGPoint(x: 100, y: 300))
        XCTAssertTrue(doc.setNetLabelName(b, name))
        XCTAssertEqual(try editing().footer, "Joined to 1 other label named \(name).")
        let c = doc.addNetLabel(at: CGPoint(x: 100, y: 500))
        XCTAssertTrue(doc.setNetLabelName(c, name))
        XCTAssertEqual(try editing().footer, "Joined to 2 other labels named \(name).")

        let flag = doc.addNetLabel(at: CGPoint(x: 400, y: 100), style: .powerFlag)
        XCTAssertTrue(NetLabelEditing(document: doc, label: try label(doc, flag)).footer.hasPrefix("Marks the net it is on as driven"))
        let none = doc.addNetLabel(at: CGPoint(x: 400, y: 300), style: .noConnect)
        XCTAssertTrue(NetLabelEditing(document: doc, label: try label(doc, none)).footer.hasPrefix("Marks a pin as left unconnected"))
    }

    func testTypingCleansAndApplies() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNetLabel(at: CGPoint(x: 100, y: 100))
        let editing = NetLabelEditing(document: doc, label: try label(doc, id))
        XCTAssertEqual(editing.nameAfterTyping("a b"), NamePreferences.typed("ab"), "spaces go")
        XCTAssertEqual(editing.nameAfterTyping("ABCDEFGHIJ"), "ABCDEFGH", "at most eight")
        XCTAssertNil(editing.nameAfterTyping(""), "empty: the label keeps its name")
        XCTAssertNil(editing.nameAfterTyping("CLK"))
        XCTAssertEqual(try label(doc, id).name, "CLK")

        XCTAssertEqual(editing.nameAfterEditing(""), editing.label.name, "left empty: shows the kept name")
        XCTAssertEqual(editing.nameAfterEditing("CLK"), "CLK")
        XCTAssertEqual(NetLabelEditing.nameAfterRename(field: "", to: "X"), "")
        XCTAssertEqual(NetLabelEditing.nameAfterRename(field: "CLK", to: "X"), "X")
    }

    func testThePanelEditsTheName() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNetLabel(at: CGPoint(x: 100, y: 100))
        let start = try label(doc, id).name
        let probe = show(doc, id)
        XCTAssertTrue(probe.has("Net Label"), "\(probe.labels())")
        XCTAssertTrue(probe.has("No other label is named \(start)."), "\(probe.labels())")
        let field = try probe.textField(showing: start)

        probe.type("RST", into: field)
        XCTAssertEqual(try label(doc, id).name, "RST")
        probe.type("", into: field)
        XCTAssertEqual(try label(doc, id).name, "RST", "emptied: the label keeps its name")
        probe.submit(field)
        probe.blur()
        XCTAssertTrue(probe.wait { field.stringValue == "RST" }, field.stringValue)

        // Undo renames the label from outside the field.
        doc.undo()
        XCTAssertEqual(try label(doc, id).name, start)
        XCTAssertTrue(probe.wait { field.stringValue == start }, field.stringValue)
    }

    func testTheTypePickerAndAFlagWithNoName() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNetLabel(at: CGPoint(x: 100, y: 100), style: .noConnect)
        let probe = show(doc, id)
        XCTAssertTrue(probe.has("No Connect"), "\(probe.labels())")
        XCTAssertTrue(probe.textFields().isEmpty, "a No Connect has no name to edit")
        XCTAssertTrue(probe.has("Marks a pin as left unconnected"), "\(probe.labels())")
        doc.setNetLabelStyle(id, .powerFlag)
        probe.settle()
        XCTAssertTrue(probe.has("Power Flag"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Marks the net it is on as driven"), "\(probe.labels())")
        let popUp = try XCTUnwrap(probe.views(NSPopUpButton.self).first { $0.itemTitles.contains("Output") },
                                  "\(probe.views(NSPopUpButton.self).map(\.itemTitles))")
        // As a click on the menu's item would.
        popUp.menu?.performActionForItem(at: popUp.indexOfItem(withTitle: "Output"))
        probe.settle()
        XCTAssertEqual(try label(doc, id).style, .output, "\(popUp.itemTitles) \(String(describing: popUp.titleOfSelectedItem)) \(String(describing: doc.editorMessage))")
        XCTAssertTrue(probe.has("Net Label"), "\(probe.labels())")
    }
}
