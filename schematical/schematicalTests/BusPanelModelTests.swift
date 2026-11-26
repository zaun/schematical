import AppKit
import XCTest
@testable import schematical

/// The Bus and Bus Port panels' logic (`BusPanelModel`, `BusPortPanelModel`;
/// plan-68-AB), driven without a view.
@MainActor
final class BusPanelModelTests: XCTestCase {
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }
    private var files: [URL] = []

    override func tearDown() {
        for url in files { try? FileManager.default.removeItem(at: url) }
        files = []
        super.tearDown()
    }

    /// A new file in the temporary directory, removed in tearDown.
    private func file(_ text: String?) throws -> URL {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("BusPanelModelTests-\(UUID()).csv")
        files.append(url)
        if let text { try text.write(to: url, atomically: true, encoding: .utf8) }
        return url
    }

    private func model() throws -> (SchematicDocument, BusPanelModel) {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        let model = BusPanelModel(document: doc, busID: id)
        model.appear()
        return (doc, model)
    }

    /// A presenter that answers panels with `url` and records alerts.
    private func presenter(url: URL?, alerts: @escaping (NSAlert) -> Void = { _ in }) -> ExportPresenter {
        var presenter = ExportPresenter()
        presenter.choose = { _, _ in url }
        presenter.alert = { alert, _ in alerts(alert); return .alertFirstButtonReturn }
        return presenter
    }

    func testHeaderAndFieldsShowTheBus() throws {
        let (doc, model) = try model()
        XCTAssertEqual(model.title, "BUS 1")
        XCTAssertEqual(model.nameText, "BUS 1")
        XCTAssertEqual(model.countText, "8")
        XCTAssertEqual(model.lineCountText, "1 line")
        XCTAssertNotNil(doc.addBus(from: p(300, 100), to: p(300, 300)))
        XCTAssertEqual(model.lineCountText, "2 lines")
        XCTAssertFalse(model.isInUse)
        XCTAssertFalse(model.isCountLocked)
        XCTAssertEqual(model.countHelp, "")
        XCTAssertEqual(model.wireHelp, "")
        XCTAssertEqual(model.importHelp, "Load the wire names from a CSV or text file")
        XCTAssertEqual(model.wireFooter?.isError, false)
        XCTAssertEqual(model.exportFileName, "BUS 1 wires.csv")
        XCTAssertEqual(model.type, .custom)
        XCTAssertEqual(model.stepperValue, 8)
        XCTAssertEqual(model.cardOptions, CardOptions())
    }

    func testABusThatIsGoneShowsDefaults() throws {
        let doc = TestSupport.newDocument()
        let model = BusPanelModel(document: doc, busID: UUID())
        model.appear()
        XCTAssertNil(model.bus)
        XCTAssertEqual(model.title, "Bus")
        XCTAssertEqual(model.lineCountText, "0 lines")
        XCTAssertEqual(model.countText, "0")
        XCTAssertEqual(model.nameText, "")
        XCTAssertTrue(model.isCountLocked)
        XCTAssertEqual(model.type, .custom)
        XCTAssertEqual(model.pads, CustomCard().pads)
        XCTAssertFalse(model.bothSides)
        XCTAssertEqual(model.stepperValue, Bus.defaultWireCount)
        XCTAssertNil(model.wireFooter)
        XCTAssertTrue(model.wireUses.isEmpty)
        // An edit with no bus changes nothing and reports nothing.
        model.type = .isa8
        XCTAssertNil(model.typeError)
    }

    func testEmptyNameIsRefusedAndPutBack() throws {
        let (doc, model) = try model()
        model.nameText = "Data"
        model.nameFocusChanged(from: false, to: true)
        XCTAssertEqual(model.bus?.name, "BUS 1", "gaining focus commits nothing")
        model.nameFocusChanged(from: true, to: false)
        XCTAssertEqual(model.bus?.name, "Data")
        model.nameText = " "
        model.commitName()
        XCTAssertEqual(model.nameError, "A bus name can't be empty.")
        XCTAssertEqual(model.nameText, "Data")
        doc.undo()
        model.busNameChanged(model.bus?.name ?? "")
        XCTAssertEqual(model.nameText, "BUS 1")
    }

    func testAnotherBusesNameAsksThenLinks() throws {
        let (doc, model) = try model()
        let other = try XCTUnwrap(doc.addBus(from: p(100, 400), to: p(300, 400)))
        XCTAssertEqual(model.linkTitle, "")
        XCTAssertFalse(model.isAskingToLink)
        model.nameText = "bus 2"
        model.commitName()
        XCTAssertEqual(model.linkTarget?.id, other)
        XCTAssertTrue(model.isAskingToLink)
        XCTAssertEqual(model.linkTitle, "Link with BUS 2?")
        XCTAssertTrue(BusPanelModel.linkMessage(try XCTUnwrap(model.linkTarget))
            .hasPrefix("A bus named BUS 2 already exists."))
        // A second commit while asking (focus leaving) does nothing.
        model.commitName()
        XCTAssertEqual(model.bus?.name, "BUS 1")

        // Dismissing the dialog cancels: the field shows the name again.
        model.isAskingToLink = true
        XCTAssertNotNil(model.linkTarget, "setting true keeps the question up")
        model.isAskingToLink = false
        XCTAssertNil(model.linkTarget)
        XCTAssertEqual(model.nameText, "BUS 1")

        model.nameText = "BUS 2"
        model.commitName()
        model.link(to: try XCTUnwrap(model.linkTarget))
        XCTAssertNil(model.linkTarget)
        XCTAssertNil(model.nameError)
        XCTAssertEqual(model.bus?.name, "BUS 2")
        XCTAssertEqual(model.nameText, "BUS 2")
    }

    func testTypeAndCustomCardSettings() throws {
        let (_, model) = try model()
        model.type = .isa8
        XCTAssertNil(model.typeError)
        XCTAssertEqual(model.bus?.wires.count, 62)
        XCTAssertTrue(model.isCountLocked)
        XCTAssertEqual(model.countHelp, "Set by the bus type")
        XCTAssertEqual(model.wireHelp, "Set by the bus type; set Type to Custom to change it")
        XCTAssertEqual(model.importHelp, model.wireHelp)
        XCTAssertNil(model.wireFooter)

        model.type = .customCard
        model.pads = 500
        XCTAssertEqual(model.pads, CustomCard.padRange.upperBound, "clamped")
        model.pads = 4
        XCTAssertTrue(model.bothSides)
        XCTAssertEqual(model.bus?.wires.count, 8)
        model.bothSides = false
        XCTAssertFalse(model.bothSides)
        XCTAssertEqual(model.bus?.wires.count, 4)

        model.type = .pcie
        var options = CardOptions()
        options["lanes"] = .number(4)
        model.setCardOptions(options)
        XCTAssertEqual(model.cardOptions["lanes"], .number(4))
        XCTAssertEqual(model.bus?.wires.count, 64)
    }

    func testWireCountFieldAndStepper() throws {
        let (_, model) = try model()
        model.countValue = 12
        XCTAssertEqual(model.countText, "12")
        model.countFocusChanged(from: false, to: true)
        XCTAssertEqual(model.bus?.wires.count, 8)
        model.countFocusChanged(from: true, to: false)
        XCTAssertEqual(model.bus?.wires.count, 12)
        model.countValue = nil
        XCTAssertEqual(model.countText, "")
        XCTAssertNil(model.countValue)
        model.commitCount()
        XCTAssertEqual(model.countText, "12", "nothing typed: put back")
        model.stepperValue = 13
        XCTAssertEqual(model.bus?.wires.count, 13)
        XCTAssertEqual(model.countText, "13")
        XCTAssertNil(model.countError)
    }

    func testWireCountLockedByTheCardEdge() throws {
        let (doc, model) = try model()
        model.type = .pcie
        doc.setBoardType(.pcie)
        doc.setEdgeBus(model.busID)
        XCTAssertTrue(model.isInUse)
        XCTAssertEqual(model.countHelp, "Locked: in use as the card edge")
        model.type = .custom
        XCTAssertTrue(model.isCountLocked)
        let count = try XCTUnwrap(model.bus?.wires.count)
        model.countText = "20"
        model.commitCount()
        XCTAssertNotNil(model.countError)
        XCTAssertEqual(model.countText, String(count))
    }

    func testMovesMenuAndDrops() throws {
        XCTAssertEqual(BusPanelModel.moves(index: 0, count: 3).map(\.isEnabled), [false, true, false, true])
        XCTAssertEqual(BusPanelModel.moves(index: 2, count: 3).map(\.isEnabled), [true, false, true, false])
        XCTAssertEqual(BusPanelModel.moves(index: 1, count: 3).map(\.place), [0, 2, 0, 2])
        XCTAssertEqual(BusPanelModel.moves(index: 1, count: 3).map(\.title),
                       ["Move Up", "Move Down", "Move to Top", "Move to Bottom"])

        let (_, model) = try model()
        let wires = try XCTUnwrap(model.bus?.wires)
        model.moveWire(wires[0].id, to: 2)
        XCTAssertEqual(model.bus?.wires.map(\.id).prefix(3), [wires[1].id, wires[2].id, wires[0].id])

        let mine = BusWireDragItem(bus: model.busID, wire: wires[3].id)
        XCTAssertFalse(model.drop([], on: wires[4].id, at: 4))
        XCTAssertFalse(model.drop([BusWireDragItem(bus: UUID(), wire: wires[3].id)], on: wires[4].id, at: 4))
        XCTAssertFalse(model.drop([mine], on: wires[3].id, at: 3), "its own row")
        XCTAssertTrue(model.drop([mine], on: wires[5].id, at: 5))
        XCTAssertEqual(model.bus?.wires[5].id, wires[3].id)

        // A card type's wires can't move: the reason shows in the footer.
        model.type = .isa8
        model.moveWire(wires[0].id, to: 0)
        XCTAssertNotNil(model.wireOrderError)
        XCTAssertEqual(model.wireFooter?.isError, true)
    }

    func testRenamingAWireAndSharedNames() throws {
        let (_, model) = try model()
        let wire = try XCTUnwrap(model.bus?.wires.first)
        let renamed = model.renameWire(wire, to: "CLK")
        XCTAssertNil(renamed.error)
        XCTAssertEqual(renamed.name, "CLK")
        let refused = model.renameWire(wire, to: "")
        XCTAssertNotNil(refused.error)
        XCTAssertEqual(refused.name, "CLK")
        XCTAssertEqual(model.renameWire(BusWire(name: "X"), to: "Y").name, "X", "a wire not on the bus keeps its name")

        let second = try XCTUnwrap(model.bus?.wires[1])
        _ = model.renameWire(second, to: "CLK")
        XCTAssertEqual(model.wireUses["CLK"], 2)
        XCTAssertEqual(BusPanelModel.sharedNameHelp("CLK", uses: 2),
                       "CLK is used 2 times in this bus. Each wire is still its own net.")
        XCTAssertEqual(BusPanelModel.sharedNameHelp("CLK", uses: 1), "")
    }

    func testExportThenImport() async throws {
        let (_, model) = try model()
        let url = try file(nil)
        model.presenter = presenter(url: url)
        model.wireOrderError = "old"
        await model.exportWires()
        XCTAssertNil(model.wireOrderError)
        let names = try XCTUnwrap(model.bus?.wires.map(\.name))
        XCTAssertEqual(try String(contentsOf: url, encoding: .utf8), BusWireList.text(names))

        try "A\nB\nC\n".write(to: url, atomically: true, encoding: .utf8)
        await model.importWires()
        XCTAssertEqual(model.bus?.wires.map(\.name), ["A", "B", "C"])
        XCTAssertNil(model.wireOrderError)

        try "A\n".write(to: url, atomically: true, encoding: .utf8)
        await model.importWires()
        XCTAssertEqual(model.wireOrderError, "The file has 1 wire name; a bus carries 2–999.")
    }

    func testCancelledPanelsChangeNothing() async throws {
        let (_, model) = try model()
        model.presenter = presenter(url: nil)
        let before = model.bus?.wires
        await model.importWires()
        await model.exportWires()
        XCTAssertEqual(model.bus?.wires, before)
        XCTAssertNil(model.wireOrderError)
    }

    func testFileErrorsShowAnAlert() async throws {
        let (_, model) = try model()
        var shown: [String] = []
        let missing = try file(nil)
        model.presenter = presenter(url: missing) { shown.append($0.messageText) }
        await model.importWires(from: missing)
        let folder = FileManager.default.temporaryDirectory.appendingPathComponent("BusPanelModelTests-\(UUID())/no/such.csv")
        await model.exportWires(to: folder)
        XCTAssertEqual(shown.count, 2, "\(shown)")
        XCTAssertNil(model.wireOrderError)
    }

    func testANewModelTakesTheSharedPresenter() throws {
        let saved = BusPanelModel.presenter
        defer { BusPanelModel.presenter = saved }
        var marked = ExportPresenter()
        marked.keyWindow = { nil }
        BusPanelModel.presenter = marked
        let doc = TestSupport.newDocument()
        let model = BusPanelModel(document: doc, busID: UUID())
        XCTAssertNil(model.presenter.keyWindow())
    }

    // MARK: Ports

    private func port() throws -> (SchematicDocument, BusPortPanelModel, Bus) {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addBus(from: p(100, 100), to: p(300, 100)))
        XCTAssertNil(doc.addPort(at: p(200, 100)))
        let port = try XCTUnwrap(doc.bus(id)?.ports.first)
        return (doc, BusPortPanelModel(document: doc, portID: port.id), try XCTUnwrap(doc.bus(id)))
    }

    func testPortPinsAndSide() throws {
        let (_, model, bus) = try port()
        XCTAssertFalse(model.isPin(bus.wires[0].id))
        model.setPin(bus.wires[0].id, true)
        model.setPin(bus.wires[1].id, true)
        XCTAssertTrue(model.isPin(bus.wires[0].id))
        model.setPin(bus.wires[0].id, false)
        XCTAssertEqual(model.port?.wireIDs, [bus.wires[1].id])
        model.selectAll()
        XCTAssertEqual(model.port?.wireIDs.count, 8)
        model.deselectAll()
        XCTAssertEqual(model.port?.wireIDs, [])
        XCTAssertNil(model.message)

        let start = model.side
        let other: ChipLayout.Side = start == .bottom ? .top : .bottom
        model.side = other
        XCTAssertEqual(model.side, other)
        XCTAssertNil(model.message)
    }

    func testAPortThatIsGone() {
        let model = BusPortPanelModel(document: TestSupport.newDocument(), portID: UUID())
        XCTAssertNil(model.port)
        XCTAssertEqual(model.side, .bottom)
        XCTAssertFalse(model.isPin(UUID()))
        model.setPin(UUID(), true)
        XCTAssertNil(model.message)
    }

    func testSideTitles() {
        XCTAssertEqual([ChipLayout.Side.top, .bottom, .left, .right].map(BusPortPanelModel.title),
                       ["Top", "Bottom", "Left", "Right"])
    }
}
