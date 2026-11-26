import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// The Bus and Bus Port panels as shown (plan-68-AB): each state's fields,
/// the fields' commits, the link confirmation, and the wire list's buttons.
@MainActor
final class BusPanelViewTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    private var probes: [ViewProbe] = []
    private var savedPresenter: ExportPresenter?
    private var files: [URL] = []

    override func setUp() {
        super.setUp()
        // The test host stays up when the test closes its last window.
        AppDelegate.quitsAfterLastWindowCloses = false
        savedPresenter = BusPanelModel.presenter
    }

    override func tearDown() {
        probes.forEach { $0.close() }
        probes = []
        if let savedPresenter { BusPanelModel.presenter = savedPresenter }
        for url in files { try? FileManager.default.removeItem(at: url) }
        files = []
        super.tearDown()
    }

    private func show<V: View>(_ view: V) -> ViewProbe {
        let probe = ViewProbe(view, size: NSSize(width: 420, height: 1000))
        probes.append(probe)
        return probe
    }

    private func bus(_ doc: SchematicDocument) throws -> UUID {
        try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
    }

    func testACustomBusShowsItsFieldsAndCommitsThem() throws {
        let doc = TestSupport.newDocument()
        let id = try bus(doc)
        let probe = show(BusPanel(document: doc, busID: id))
        XCTAssertTrue(probe.has("BUS 1"), "\(probe.labels())")
        XCTAssertTrue(probe.has("1 line"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Wire Names"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Drag a wire by its handle to reorder"), "\(probe.labels())")
        XCTAssertFalse(probe.has("In use as card edge"))

        // The name commits on Return; empty is refused and shown.
        let name = try probe.textField(showing: "BUS 1")
        probe.type("DATA", into: name)
        probe.submit(name)
        XCTAssertEqual(doc.bus(id)?.name, "DATA")
        probe.type(" ", into: name)
        probe.submit(name)
        XCTAssertTrue(probe.wait { probe.has("A bus name can't be empty.") }, "\(probe.labels())")
        XCTAssertEqual(doc.bus(id)?.name, "DATA")
        // Undo renames from outside; the field follows.
        doc.undo()
        XCTAssertTrue(probe.wait { name.stringValue == "BUS 1" }, name.stringValue)

        // The count commits on Return, and when focus leaves.
        let count = try probe.textField(showing: "8")
        probe.type("10", into: count)
        probe.submit(count)
        XCTAssertEqual(doc.bus(id)?.wires.count, 10)
        probe.type("12", into: count)
        probe.blur()
        XCTAssertTrue(probe.wait { doc.bus(id)?.wires.count == 12 })
        try probe.step(up: true)
        XCTAssertEqual(doc.bus(id)?.wires.count, 13)
        // A change from outside resets the field.
        XCTAssertNil(doc.setBusWireCount(id, 9))
        XCTAssertTrue(probe.wait { count.stringValue == "9" }, count.stringValue)
    }

    func testAWireRowRenamesAndWarnsOfASharedName() throws {
        let doc = TestSupport.newDocument()
        let id = try bus(doc)
        let wires = try XCTUnwrap(doc.bus(id)?.wires)
        let probe = show(BusPanel(document: doc, busID: id))
        let first = try probe.textField(showing: wires[0].name)
        probe.type("clk", into: first)
        XCTAssertTrue(probe.wait { first.stringValue == NamePreferences.typed("clk") }, first.stringValue)
        probe.submit(first)
        XCTAssertEqual(doc.bus(id)?.wires[0].name, NamePreferences.typed("clk"))
        let refusal = try XCTUnwrap(doc.renameBusWire(bus: id, wire: wires[0].id, to: ""))
        probe.type("", into: first)
        probe.blur()
        XCTAssertTrue(probe.wait { probe.has(refusal) && first.stringValue == NamePreferences.typed("clk") },
                      "refused: the row shows why and its name again; \(probe.labels())")

        // A name from outside shows in the row; a shared one warns.
        let shared = NamePreferences.typed("clk")
        XCTAssertNil(doc.renameBusWire(bus: id, wire: wires[1].id, to: shared))
        XCTAssertTrue(probe.wait { probe.textFields().filter { $0.stringValue == shared }.count == 2 },
                      "\(probe.textFields().map(\.stringValue))")
    }

    func testAWireRowsMenuMovesTheWire() throws {
        let doc = TestSupport.newDocument()
        let id = try bus(doc)
        let wires = try XCTUnwrap(doc.bus(id)?.wires)
        let probe = show(BusPanel(document: doc, busID: id))
        let row = try XCTUnwrap(probe.find("0", role: "AXStaticText").first { ViewProbe.label($0) == "0" }, "\(probe.labels())")
        let menu = try XCTUnwrap(probe.contextMenu(on: row), "no menu on the first row")
        XCTAssertEqual(menu.items.filter { !$0.isSeparatorItem }.map(\.title),
                       ["Move Up", "Move Down", "Move to Top", "Move to Bottom"])
        try probe.choose("Move to Bottom", in: menu)
        XCTAssertEqual(doc.bus(id)?.wires.last?.id, wires[0].id)
    }

    func testCardTypesShowTheirSettingsAndLockTheWires() throws {
        let doc = TestSupport.newDocument()
        let id = try bus(doc)
        var card = BusCard()
        card.type = .customCard
        XCTAssertNil(doc.setBusCard(id, card))
        let probe = show(BusPanel(document: doc, busID: id))
        XCTAssertTrue(probe.has("Pads per side"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Both sides"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Drag a wire by its handle"), "a card type's wires don't move")

        card.type = .pcie
        XCTAssertNil(doc.setBusCard(id, card))
        XCTAssertTrue(probe.wait { probe.has(BoardType.pcie.optionsTitle) }, "\(probe.labels())")
        XCTAssertTrue(probe.wait { !probe.has("Pads per side") }, "\(probe.labels())")

        doc.setBoardType(.pcie)
        doc.setEdgeBus(id)
        XCTAssertTrue(probe.wait { probe.has("In use as card edge") }, "\(probe.labels())")
    }

    func testNamingAfterAnotherBusAsksThenLinks() throws {
        let doc = TestSupport.newDocument()
        let id = try bus(doc)
        _ = try XCTUnwrap(doc.addBus(from: p(100, 400), to: p(300, 400)))
        let probe = show(BusPanel(document: doc, busID: id))
        let name = try probe.textField(showing: "BUS 1")
        probe.type("BUS 2", into: name)
        probe.submit(name)
        let sheet = try probe.sheet()
        let dialog = ViewProbe(window: sheet)
        XCTAssertTrue(dialog.has("A bus named BUS 2 already exists."), "\(dialog.labels())")
        try dialog.press("Rename and Link")
        XCTAssertTrue(probe.wait { probe.window.attachedSheet == nil })
        XCTAssertEqual(doc.bus(id)?.name, "BUS 2")

        // Cancel puts the name back.
        XCTAssertNotNil(doc.addBus(from: p(100, 700), to: p(300, 700)))
        probe.type("BUS 1", into: name)
        probe.submit(name)
        let again = ViewProbe(window: try probe.sheet())
        try again.press("Cancel")
        XCTAssertTrue(probe.wait { probe.window.attachedSheet == nil })
        XCTAssertTrue(probe.wait { name.stringValue == "BUS 2" }, name.stringValue)
    }

    func testImportAndExportGoThroughThePresenter() throws {
        let doc = TestSupport.newDocument()
        let id = try bus(doc)
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("BusPanelViewTests-\(UUID()).csv")
        files.append(url)
        var presenter = ExportPresenter()
        presenter.choose = { _, _ in url }
        BusPanelModel.presenter = presenter
        let probe = show(BusPanel(document: doc, busID: id))
        try probe.press("Export…")
        XCTAssertTrue(probe.wait { FileManager.default.fileExists(atPath: url.path) })
        try "P\nQ\n".write(to: url, atomically: true, encoding: .utf8)
        try probe.press("Import…")
        XCTAssertTrue(probe.wait { doc.bus(id)?.wires.map(\.name) == ["P", "Q"] }, "\(String(describing: doc.bus(id)?.wires))")
        try "P\n".write(to: url, atomically: true, encoding: .utf8)
        try probe.press("Import…")
        XCTAssertTrue(probe.wait { probe.has("a bus carries") }, "the reason shows in the footer: \(probe.labels())")
    }

    func testAGoneBusShowsNothing() {
        let probe = show(BusPanel(document: TestSupport.newDocument(), busID: UUID()))
        XCTAssertFalse(probe.has("Details"))
    }

    // MARK: Ports

    func testThePortPanelTicksPins() throws {
        let doc = TestSupport.newDocument()
        let id = try bus(doc)
        XCTAssertNil(doc.addPort(at: p(200, 100)))
        let port = try XCTUnwrap(doc.bus(id)?.ports.first)
        let probe = show(BusPortPanel(document: doc, portID: port.id))
        XCTAssertTrue(probe.has("Bus Port"), "\(probe.labels())")
        XCTAssertTrue(probe.has("0 of 8 wires"), "\(probe.labels())")
        try probe.press("Select All")
        XCTAssertEqual(doc.port(port.id)?.port.wireIDs.count, 8)
        XCTAssertTrue(probe.wait { probe.has("8 of 8 wires") }, "\(probe.labels())")
        try probe.press("Deselect All")
        XCTAssertEqual(doc.port(port.id)?.port.wireIDs.count, 0)
        let wire = try XCTUnwrap(doc.bus(id)?.wires.first)
        try probe.pressControl(wire.name, role: "AXCheckBox")
        XCTAssertEqual(doc.port(port.id)?.port.wireIDs, [wire.id])
        XCTAssertFalse(probe.find(BusPortPanelModel.title(port.side)).isEmpty, "the side picker: \(probe.labels())")
    }

    func testAGonePortShowsNothing() {
        let probe = show(BusPortPanel(document: TestSupport.newDocument(), portID: UUID()))
        XCTAssertFalse(probe.has("Bus Port"))
    }
}
