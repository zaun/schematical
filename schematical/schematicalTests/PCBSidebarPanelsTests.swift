import AppKit
import CoreGraphics
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AI: the PCB sidebar's single-item panels as they are shown: a
/// board text's, a via's, a hole's, a trace's (and its net and class), a
/// footprint's, and the Planes section, each edit reaching the document.
@MainActor
final class PCBSidebarPanelsTests: XCTestCase {
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

    /// The whole sidebar, which picks the panel from the selection.
    private func sidebar(_ doc: SchematicDocument) -> ViewProbe {
        let probe = ViewProbe(PCBSidebarView(document: doc), size: NSSize(width: 420, height: 1000))
        probes.append(probe)
        return probe
    }

    /// `view` in a columns form: a grouped form's pop-ups (the sidebar's)
    /// never fill their menus in the test host, so a menu is chosen from in this one.
    private func columns<V: View>(_ view: V) -> ViewProbe {
        let probe = ViewProbe(Form { view }.formStyle(.columns), size: NSSize(width: 520, height: 1000))
        probes.append(probe)
        return probe
    }

    private func mm(_ value: Double, precise: Bool = false) -> String {
        LengthField.format(value, units: .mm, precise: precise)
    }

    /// Two 8-pin headers, pin 1 of each wired, and a trace from the first's pin 1.
    private func wiredHeaders() throws -> (doc: SchematicDocument, one: PlacedChip, trace: UUID, key: PlaneNet) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-HEADER-1X08", at: CGPoint(x: 400, y: 100))
        doc.moveFootprint(one.id, to: CGPoint(x: 15, y: 30))
        doc.moveFootprint(two.id, to: CGPoint(x: 45, y: 30))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "1")), .pin(try TestSupport.pin(two, "1"))))
        let placed = try XCTUnwrap(doc.chips.first { $0.id == one.id })
        let designator = placed.part.pins[try TestSupport.pin(one, "1").index].designator
        let pad = try XCTUnwrap(placed.boardPads.first { $0.designator == designator })
        let from = CGPoint(x: pad.rect.midX, y: pad.rect.midY)
        let trace = try XCTUnwrap(doc.addTrace(points: [from, CGPoint(x: from.x, y: from.y + 10)], layer: .top, width: 10))
        let copper = doc.connectivity()
        let net = try XCTUnwrap(doc.net(at: from, layer: .top, in: copper))
        return (doc, placed, trace, try XCTUnwrap(copper.planeNet(ofNet: net)))
    }

    // MARK: Text

    func testTheTextPanelEditsWordsSideLayerAndSize() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addText(at: CGPoint(x: 10, y: 10))
        let probe = sidebar(doc)
        let text = try XCTUnwrap(doc.texts.first { $0.id == id })
        let words = try probe.field(showing: text.text)
        probe.type("Hello", into: words)
        probe.blur()
        XCTAssertEqual(doc.texts[0].text, "Hello")
        probe.type("", into: words)
        probe.blur()
        XCTAssertEqual(doc.texts[0].text, "Hello", "an empty text is refused")
        XCTAssertEqual(words.stringValue, "Hello", "the field shows the kept text")
        // Undo changes the words from outside the field.
        doc.setText(id, "Again")
        probe.settle()
        XCTAssertEqual(words.stringValue, "Again")

        try probe.retype("\(text.size)", as: "90")
        XCTAssertEqual(doc.texts[0].size, 90)
        let size = try probe.field(showing: "90")
        probe.type("big", into: size)
        probe.blur()
        XCTAssertEqual(size.stringValue, "90", "a word is not a size")
        try probe.step(up: true)
        XCTAssertEqual(doc.texts[0].size, 95)
        XCTAssertEqual(size.stringValue, "95")
        doc.setTextSize(id, 60)
        probe.settle()
        XCTAssertEqual(size.stringValue, "60", "an outside change shows")

        try probe.pick(BoardSide.bottom.title)
        XCTAssertEqual(doc.texts[0].side, .bottom)
        let layer = try XCTUnwrap(TextLayer.allCases.first { $0 != doc.texts[0].layer })
        try probe.pick(layer.title)
        XCTAssertEqual(doc.texts[0].layer, layer)
        try probe.pick("90°")
        XCTAssertEqual(doc.texts[0].rotation, 90)

        let anchor = doc.texts[0].anchor
        try probe.retype(mm(anchor.x), as: "20")
        XCTAssertEqual(doc.texts[0].anchor.x, 20, accuracy: doc.board.grid, "snapped near 20")
        XCTAssertNoThrow(try probe.field(showing: mm(doc.texts[0].anchor.x)), "the field shows where it snapped")
        try probe.retype(mm(doc.texts[0].anchor.y), as: "25")
        XCTAssertEqual(doc.texts[0].anchor.y, 25, accuracy: doc.board.grid)

        try probe.pressControl("Lock Position")
        XCTAssertTrue(doc.texts[0].locked)
    }

    // MARK: Via

    func testTheViaPanelEditsAndCountsItsTraces() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addVia(at: CGPoint(x: 10.16, y: 10.16)))
        let probe = sidebar(doc)
        XCTAssertTrue(probe.has("No traces"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Through, all 2"), "\(probe.labels())")
        let via = try XCTUnwrap(doc.vias.first)
        try probe.retype(mm(via.centre.x, precise: true), as: "12")
        XCTAssertEqual(doc.vias[0].centre.x, 12, accuracy: 1e-6)
        try probe.retype(mm(doc.vias[0].centre.y, precise: true), as: "14")
        XCTAssertEqual(doc.vias[0].centre.y, 14, accuracy: 1e-6)
        try probe.retype(mm(via.diameter, precise: true), as: "1.2")
        XCTAssertEqual(doc.vias[0].diameter, 1.2, accuracy: 1e-6)
        try probe.retype(mm(via.drill, precise: true), as: "0.5")
        XCTAssertEqual(doc.vias[0].drill, 0.5, accuracy: 1e-6)
        XCTAssertTrue(probe.has(mm(0.35, precise: true) + " mm"), "the ring: \(probe.labels())")
        let top = doc.vias[0].tentedTop, bottom = doc.vias[0].tentedBottom
        try probe.pressControl("Tented on Top")
        XCTAssertEqual(doc.vias[0].tentedTop, !top)
        try probe.pressControl("Tented on Bottom")
        XCTAssertEqual(doc.vias[0].tentedBottom, !bottom)

        let centre = doc.vias[0].centre
        XCTAssertNotNil(doc.addTrace(points: [centre, CGPoint(x: centre.x + 10, y: centre.y)], layer: .top, width: 10))
        XCTAssertNotNil(doc.addTrace(points: [centre, CGPoint(x: centre.x, y: centre.y + 10)], layer: .bottom, width: 10))
        doc.select(.via(id))
        probe.settle()
        XCTAssertTrue(probe.has("2 traces on Top, Bottom"), "\(probe.labels())")
    }

    // MARK: Hole

    func testTheHolePanelEditsAPlatedHole() throws {
        let doc = TestSupport.newDocument()
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 60))
        let id = doc.addHole(at: CGPoint(x: 10, y: 10))
        doc.select(.hole(id))
        let probe = sidebar(doc)
        XCTAssertFalse(probe.has("Land"), "unplated: no copper")
        let hole = try XCTUnwrap(doc.holes.first)
        try probe.retype(mm(hole.centre.x, precise: true), as: "12")
        XCTAssertEqual(doc.holes[0].centre.x, 12, accuracy: 1e-6)
        try probe.retype(mm(doc.holes[0].centre.y, precise: true), as: "13")
        XCTAssertEqual(doc.holes[0].centre.y, 13, accuracy: 1e-6)
        try probe.retype(mm(doc.holes[0].drill, precise: true), as: "2")
        XCTAssertEqual(doc.holes[0].drill, 2, accuracy: 1e-6)
        let plated = doc.holes[0].plated
        try probe.pressControl("Plated")
        XCTAssertEqual(doc.holes[0].plated, !plated)
        if !doc.holes[0].plated { try probe.pressControl("Plated") }
        XCTAssertTrue(doc.holes[0].plated)
        XCTAssertTrue(probe.has("Traces"), "\(probe.labels())")
        try probe.retype(mm(doc.holes[0].diameter, precise: true), as: "4")
        XCTAssertEqual(doc.holes[0].diameter, 4, accuracy: 1e-6)

        let menus = columns(HolePanel(document: doc, hole: doc.holes[0]))
        menus.choose("GND", in: try menus.popUp(offering: "GND"))
        XCTAssertEqual(doc.holes[0].net, .ground)
        menus.choose(HoleSize.m3.title, in: try menus.popUp(offering: HoleSize.m3.title))
        XCTAssertEqual(doc.holes[0].size, .m3)
        // A net the schematic no longer has stays listed as missing.
        doc.setHoleNet(id, .pin(chip: gnd.id, designator: "nowhere"))
        let missing = columns(HolePanel(document: doc, hole: doc.holes[0]))
        XCTAssertTrue(try missing.popUp(offering: "Missing net").itemTitles.contains("None"))
        let centre = doc.holes[0].centre
        XCTAssertNotNil(doc.addTrace(points: [CGPoint(x: centre.x + 2, y: centre.y), CGPoint(x: centre.x + 12, y: centre.y)],
                                     layer: .top, width: 10))
        doc.select(.hole(id))
        probe.settle()
        let joined = doc.traces(touching: doc.holes[0]).count
        XCTAssertTrue(probe.has(joined == 0 ? "None" : "\(joined)"), "\(probe.labels())")
        XCTAssertGreaterThan(joined, 0)
    }

    // MARK: Trace

    func testTheTracePanelEditsWidthLayerAndClass() throws {
        let (doc, one, trace, key) = try wiredHeaders()
        let power = doc.addNetClass(named: "Power")
        doc.setLayerCount(4)
        doc.select(.trace(trace))
        let probe = sidebar(doc)
        XCTAssertTrue(probe.has("\(one.reference).1"), "the run's pads: \(probe.labels())")
        try probe.retype("10", as: "20")
        XCTAssertEqual(doc.traces[0].width, 20)
        let width = try probe.field(showing: "20")
        probe.type("wide", into: width)
        probe.blur()
        XCTAssertEqual(width.stringValue, "20", "a word is not a width")
        try probe.step(up: true)
        XCTAssertEqual(doc.traces[0].width, 25)
        doc.setTraceWidth(trace, 30)
        probe.settle()
        XCTAssertEqual(width.stringValue, "30", "an outside change shows")

        let menus = columns(TracePanel(document: doc, trace: doc.traces[0]))
        let layers = try menus.popUp(offering: CopperLayer.bottom.title)
        XCTAssertEqual(layers.itemTitles, TracePanel.layers(for: doc.board).map(\.title))
        menus.choose(CopperLayer.bottom.title, in: layers)
        XCTAssertEqual(doc.traces[0].layer, .bottom)

        menus.choose("Power", in: try menus.popUp(offering: "Power"))
        let copper = doc.connectivity()
        let net = try XCTUnwrap(doc.net(ofTrace: trace, in: copper))
        XCTAssertEqual(copper.planeNet(ofNet: net), key)
        XCTAssertEqual(doc.netClass(ofNet: net, in: copper)?.id, power)

        let end = doc.traces[0].points.last!
        XCTAssertNotNil(doc.addTrace(points: [end, CGPoint(x: end.x + 10, y: end.y)], layer: .bottom, width: 10))
        doc.select(.trace(trace))
        probe.settle()
        XCTAssertTrue(probe.has("Joined to 1 other trace"), "\(probe.labels())")

        try probe.press("Calculate Width…")
        let sheet = try probe.sheet()
        try ViewProbe(window: sheet).press("Apply")
        XCTAssertTrue(probe.wait { probe.window.attachedSheet == nil }, "Apply closes the sheet")
        XCTAssertNotEqual(doc.traces.first { $0.id == trace }?.width, 30, "the calculated width is applied")
    }

    func testALoneTraceConnectsNothingAndHasNoClass() throws {
        let doc = TestSupport.newDocument()
        let trace = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 5, y: 5), CGPoint(x: 15, y: 5)], layer: .top, width: 10))
        doc.select(.trace(trace))
        let probe = columns(TracePanel(document: doc, trace: doc.traces[0]))
        XCTAssertTrue(probe.has("Nothing"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Joined to"))
        let classes = try probe.popUp(offering: "Default")
        XCTAssertFalse(classes.isEnabled, "no net: no class to set")
    }

    // MARK: Footprint

    func testTheFootprintPanelEditsSideOriginPositionAndMarks() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        doc.select(.footprint(led.id))
        let probe = sidebar(doc)
        let chip = try XCTUnwrap(doc.chips.first)
        let footprint = try XCTUnwrap(chip.footprint)
        XCTAssertTrue(probe.has(footprint.id), "\(probe.labels())")
        XCTAssertFalse(probe.has("Full Module"), "no connector footprint: no style")

        try probe.pick(BoardSide.bottom.title)
        XCTAssertEqual(doc.chips[0].board?.side, .bottom)
        let origin = try XCTUnwrap(FootprintOrigin.allCases.first { $0 != chip.board?.origin })
        let menus = columns(FootprintPanel(document: doc, chip: doc.chips[0]))
        menus.choose(origin.title, in: try menus.popUp(offering: origin.title))
        XCTAssertEqual(doc.chips[0].board?.origin, origin)
        let point = try XCTUnwrap(doc.chips[0].boardOrigin)
        try probe.retype(mm(point.x), as: "30")
        XCTAssertEqual(try XCTUnwrap(doc.chips[0].boardOrigin).x, 30, accuracy: doc.board.grid)
        try probe.retype(mm(try XCTUnwrap(doc.chips[0].boardOrigin).y), as: "35")
        XCTAssertEqual(try XCTUnwrap(doc.chips[0].boardOrigin).y, 35, accuracy: doc.board.grid)
        XCTAssertTrue(probe.has("\(chip.board!.rotation)°"))
        try probe.pressControl("Lock Position")
        XCTAssertTrue(doc.isLocked(.footprint(led.id)))
    }

    func testAConnectorPartPicksItsFootprintStyle() throws {
        let doc = TestSupport.newDocument()
        let part = try XCTUnwrap(TestSupport.allParts.first { $0.connectorFootprint != nil })
        let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 100, y: 100))
        doc.select(.footprint(chip.id))
        let probe = sidebar(doc)
        try probe.pick(FootprintStyle.connector.title)
        XCTAssertEqual(doc.chips[0].board?.style, .connector)
    }

    // MARK: Planes

    func testThePlanesSectionSetsNetAndClearance() throws {
        let doc = TestSupport.newDocument()
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 60))
        doc.setLayerCount(4)
        let plane = try XCTUnwrap(doc.board.copperLayers.first { $0.isInner })
        doc.setLayerType(plane, .plane)
        let probe = columns(PlanesSection(document: doc))
        XCTAssertTrue(probe.has("Planes"), "\(probe.labels())")
        let rules = doc.board.rules
        let clearance = doc.board.style(of: plane).clearance(rules: rules)
        XCTAssertFalse(probe.has("Undo", role: "AXButton"), "no clearance set: nothing to reset")
        try probe.retype(mm(clearance, precise: true), as: "0.5")
        XCTAssertEqual(doc.board.style(of: plane).clearance, 0.5)
        try probe.press("Undo")
        XCTAssertNil(doc.board.style(of: plane).clearance, "back to the DRC's minimum")

        probe.choose("GND", in: try probe.popUp(offering: "GND"))
        XCTAssertEqual(doc.board.style(of: plane).net, .ground)
        doc.setLayerNet(plane, .pin(chip: gnd.id, designator: "nowhere"))
        probe.settle()
        XCTAssertNoThrow(try probe.popUp(offering: "Missing net"))

        for layer in doc.board.copperLayers where layer.isInner { doc.setLayerType(layer, .wire) }
        XCTAssertTrue(PlanesSection.planes(of: doc.board).isEmpty)
        probe.settle()
        XCTAssertFalse(probe.has("Planes"), "no Plane: no section")
    }
}
