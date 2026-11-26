import AppKit
import CoreGraphics
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AI: the PCB sidebar as it is shown, for each selection: the board's
/// settings (units, grid, board, layers, Board Details…), a selection of
/// several items (position, texts, marks, the list) and the routing of each
/// single selection to its panel; and the card sections it builds.
@MainActor
final class PCBSidebarViewTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        // Closing a test window must not quit the test host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        for probe in probes {
            if let sheet = probe.window.attachedSheet { probe.window.endSheet(sheet) }
            probe.close()
        }
        probes = []
    }

    private func show<V: View>(_ view: V, height: CGFloat = 900) -> ViewProbe {
        let probe = ViewProbe(view, size: NSSize(width: 420, height: height))
        probes.append(probe)
        return probe
    }

    private func sidebar(_ doc: SchematicDocument) -> ViewProbe {
        show(PCBSidebarView(document: doc))
    }

    /// The Board and Layers sections in a columns form: a grouped form's
    /// pop-ups (the sidebar's) never fill their menus in the test host, so a
    /// menu is chosen from in this one.
    private func boardSections(_ doc: SchematicDocument) -> ViewProbe {
        show(Form { BoardPanel(document: doc); LayersSection(document: doc) }.formStyle(.columns))
    }

    // MARK: The helpers

    func testSharedMilAndSideHelpers() throws {
        XCTAssertEqual(PCBSidebarView.shared(Set([3])), 3)
        XCTAssertNil(PCBSidebarView.shared(Set([3, 4])), "they differ")
        XCTAssertNil(PCBSidebarView.shared(Set<Int>()), "nothing to share")
        XCTAssertEqual(PCBSidebarView.mil(" 25 "), 25)
        XCTAssertNil(PCBSidebarView.mil("25 mil"))
        XCTAssertNil(PCBSidebarView.mil(""))

        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let text = doc.addText(at: .zero)
        doc.setTextSide(text, .bottom)
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: 5, y: 5)))
        XCTAssertEqual(MultiPositionSection.side(of: .footprint(led.id), in: doc), .top)
        XCTAssertEqual(MultiPositionSection.side(of: .text(text), in: doc), .bottom)
        XCTAssertNil(MultiPositionSection.side(of: .via(via), in: doc))
        doc.select(.via(via))
        doc.toggleSelection(.footprint(led.id))
        XCTAssertEqual(PCBSidebarView.footprintIDs(of: doc), [led.id])
        doc.select(.via(via))
        XCTAssertTrue(PCBSidebarView.footprintIDs(of: doc).isEmpty)
    }

    // MARK: The board

    func testUnitsSwitchSetsTheBoardUnits() throws {
        let doc = TestSupport.newDocument()
        let probe = sidebar(doc)
        XCTAssertEqual(doc.board.units, .mm)
        try probe.pick(BoardUnits.inch.title, in: try probe.segments(with: BoardUnits.inch.title))
        XCTAssertEqual(doc.board.units, .inch)
        XCTAssertTrue(probe.has("Traces"), "\(probe.labels())")
    }

    func testGridAndSizeFieldsSetTheBoard() throws {
        let doc = TestSupport.newDocument()
        let probe = sidebar(doc)
        let grid = LengthField.format(doc.board.grid, units: .mm, precise: true)
        let gridField = try probe.field(showing: grid)
        probe.type("2.54", into: gridField)
        probe.submit(gridField)
        XCTAssertEqual(doc.board.grid, 2.54, accuracy: 1e-9)
        XCTAssertTrue(probe.has(LengthField.format(doc.board.traceGrid, units: .mm, precise: true) + " mm"),
                      "the trace grid follows: \(probe.labels())")
        let width = LengthField.format(doc.board.width, units: .mm)
        let widthField = try probe.field(showing: width)
        probe.type("77", into: widthField)
        probe.submit(widthField)
        XCTAssertEqual(doc.board.width, 77, accuracy: 1e-9)
        let height = try probe.field(showing: LengthField.format(doc.board.height, units: .mm))
        probe.type("66", into: height)
        probe.submit(height)
        XCTAssertEqual(doc.board.height, 66, accuracy: 1e-9)
    }

    func testTypeLayersAndColourMenusSetTheBoard() throws {
        let doc = TestSupport.newDocument()
        let probe = boardSections(doc)
        // The type menu: Custom and Custom Card first, then each group's boards.
        let types = try probe.popUp(offering: BoardType.customCard.title)
        for type in BoardType.allCases { XCTAssertTrue(types.itemTitles.contains(type.title), type.title) }
        probe.choose(BoardType.customCard.title, in: types)
        XCTAssertEqual(doc.board.type, .customCard)

        probe.choose("4 layers", in: try probe.popUp(offering: "4 layers"))
        XCTAssertEqual(doc.board.layers, 4)

        let other = try XCTUnwrap(BoardColor.allCases.first { $0 != doc.board.color })
        try probe.press(other.title)
        XCTAssertEqual(doc.board.color, other)
        XCTAssertTrue(probe.has(other.title, role: "AXStaticText"), "the colour's name is shown")
    }

    func testALayerCountThatDropsSomethingIsRefused() throws {
        let doc = TestSupport.newDocument()
        XCTAssertNotNil(doc.addVia(at: CGPoint(x: 5, y: 5)))
        let probe = boardSections(doc)
        let layers = try probe.popUp(offering: "1 layer")
        let one = try XCTUnwrap(layers.item(withTitle: "1 layer"))
        XCTAssertFalse(one.isEnabled, "a via needs two layers")
        XCTAssertEqual(one.toolTip, "Delete the vias first.")
    }

    func testInnerLayerRowsSetTypeAndColour() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let probe = boardSections(doc)
        let inner = try XCTUnwrap(doc.board.copperLayers.first { $0.isInner })
        XCTAssertTrue(probe.has(LayerType.component.title), "Top and Bottom are Component: \(probe.labels())")
        let type = try probe.popUp(offering: LayerType.wire.title)
        let other: LayerType = doc.board.style(of: inner).type == .plane ? .wire : .plane
        probe.choose(other.title, in: type)
        XCTAssertEqual(doc.board.style(of: inner).type, other)

        let well = try XCTUnwrap(probe.views(NSColorWell.self).first, "a colour well per layer")
        well.color = NSColor(srgbRed: 1, green: 0, blue: 0, alpha: 1)
        well.sendAction(well.action, to: well.target)
        probe.settle()
        let top = doc.board.style(of: doc.board.copperLayers[0]).color.color
        XCTAssertEqual(NSColor(top).usingColorSpace(.sRGB)?.redComponent ?? 0, 1, accuracy: 0.01)
    }

    func testALayerTypeHoldingTracesIsRefused() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let inner = try XCTUnwrap(doc.board.copperLayers.first { $0.isInner })
        doc.setLayerType(inner, .wire)
        XCTAssertNotNil(doc.addTrace(points: [CGPoint(x: 5, y: 5), CGPoint(x: 15, y: 5)], layer: inner, width: 10))
        let probe = boardSections(doc)
        let type = try probe.popUp(offering: LayerType.plane.title)
        let plane = try XCTUnwrap(type.item(withTitle: LayerType.plane.title))
        XCTAssertFalse(plane.isEnabled)
        XCTAssertEqual(plane.toolTip, doc.canSetLayerType(inner, .plane))
    }

    func testBoardDetailsOpensTheSheetAndItsCloseClosesIt() throws {
        let doc = TestSupport.newDocument()
        let probe = sidebar(doc)
        try probe.press("Board Details…")
        let sheet = try probe.sheet()
        let contents = ViewProbe.texts(in: try XCTUnwrap(sheet.contentView))
        XCTAssertTrue(contents.contains { $0.contains("Stackup") }, "\(contents)")
        let done = ViewProbe(window: sheet)
        try done.press("Done")
        XCTAssertTrue(probe.wait { probe.window.attachedSheet == nil }, "Done closes the sheet")
    }

    func testACardBoardOffersItsBusAndFixesItsHeight() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(try XCTUnwrap(BoardType(rawValue: "isa8")))
        let busID = try XCTUnwrap(doc.addBus(from: CGPoint(x: 200, y: 300), to: CGPoint(x: 400, y: 300)))
        XCTAssertNil(doc.setBusWireCount(busID, doc.board.pinCount))
        let name = try XCTUnwrap(doc.bus(busID)?.name)
        let probe = boardSections(doc)
        XCTAssertTrue(probe.has("(\(doc.board.pinCount) Wires)"), "\(probe.labels())")
        probe.choose(name, in: try probe.popUp(offering: name))
        XCTAssertEqual(doc.edgeBusID, busID)
        probe.choose("None", in: try probe.popUp(offering: "None"))
        XCTAssertNil(doc.edgeBusID)
        XCTAssertTrue(probe.has(LengthField.format(doc.board.height, units: .mm) + " mm"), "the height is shown, not typed")
        XCTAssertNoThrow(try probe.field(showing: LengthField.format(doc.board.width, units: .mm)), "the length stays editable")
    }

    func testAFixedSizeBoardShowsItsWidth() throws {
        for id in ["nes", "piHat"] {
            let doc = TestSupport.newDocument()
            doc.setBoardType(try XCTUnwrap(BoardType(rawValue: id)))
            XCTAssertEqual(doc.board.card?.isFixedSize, true, id)
            let probe = sidebar(doc)
            XCTAssertTrue(probe.has(LengthField.format(doc.board.width, units: .mm) + " mm"), "\(id): \(probe.labels())")
            XCTAssertFalse(probe.textFields().contains { $0.stringValue == LengthField.format(doc.board.width, units: .mm) },
                           "\(id): no width field")
        }
    }

    func testAFreeFormBoardShowsItsSizeAndResets() throws {
        let doc = TestSupport.newDocument()
        XCTAssertTrue(doc.setBoardOutline([CGPoint(x: 0, y: 0), CGPoint(x: 40, y: 0), CGPoint(x: 40, y: 30),
                                           CGPoint(x: 20, y: 40), CGPoint(x: 0, y: 30)]))
        let probe = sidebar(doc)
        XCTAssertTrue(probe.has(LengthField.format(40, units: .mm) + " mm"), "\(probe.labels())")
        XCTAssertTrue(probe.has(LengthField.format(40, units: .mm) + " mm"))
        try probe.press("Reset to Rectangle")
        XCTAssertFalse(doc.board.isFreeForm)
        XCTAssertNoThrow(try probe.field(showing: LengthField.format(40, units: .mm)), "the width is a field again")
    }

    // MARK: Each single selection

    func testEachSingleSelectionShowsItsPanel() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let probe = sidebar(doc)
        doc.select(.footprint(led.id))
        probe.settle()
        XCTAssertTrue(probe.has("Marks"), "the footprint panel: \(probe.labels())")
        let text = doc.addText(at: CGPoint(x: 5, y: 5))
        probe.settle()
        XCTAssertTrue(probe.has("Font Size"), "the text panel: \(probe.labels())")
        doc.select(.trace(try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 5, y: 20), CGPoint(x: 20, y: 20)],
                                                        layer: .top, width: 10))))
        probe.settle()
        XCTAssertTrue(probe.has("Calculate Width…"), "the trace panel: \(probe.labels())")
        _ = try XCTUnwrap(doc.addVia(at: CGPoint(x: 30, y: 30)))
        probe.settle()
        XCTAssertTrue(probe.has("Tented on Top"), "the via panel: \(probe.labels())")
        doc.select(.hole(doc.addHole(at: CGPoint(x: 40, y: 10))))
        probe.settle()
        XCTAssertTrue(probe.has("Plated"), "the hole panel: \(probe.labels())")
        doc.select(.pour(try XCTUnwrap(doc.addPour(rect: CGRect(x: 2, y: 2, width: 10, height: 10), layer: .top))))
        probe.settle()
        XCTAssertEqual(PCBSidebarView.content(for: doc), .pour(try XCTUnwrap(doc.selectedPourID)))
        let pourLabels = probe.labels()
        doc.select(.keepout(doc.addKeepout(rect: CGRect(x: 2, y: 2, width: 10, height: 10))))
        probe.settle()
        XCTAssertNotEqual(probe.labels(), pourLabels, "the keep-out panel replaces the pour's")
        let keepoutLabels = probe.labels()
        doc.select(.cutout(try XCTUnwrap(doc.addCutout(.rounded(rect: CGRect(x: 20, y: 20, width: 5, height: 5), radius: 0)))))
        probe.settle()
        XCTAssertNotEqual(probe.labels(), keepoutLabels, "the cutout panel replaces the keep-out's")
        doc.select(.text(text))
        doc.clearBoardSelection()
        probe.settle()
        XCTAssertTrue(probe.has("Board Details…"), "nothing selected: the board")
    }

    // MARK: Several items

    func testSeveralTextsShareSideLayerAndSize() throws {
        let doc = TestSupport.newDocument()
        let a = doc.addText(at: CGPoint(x: 5, y: 5))
        let b = doc.addText(at: CGPoint(x: 15, y: 5))
        doc.setTextSize([b], BoardText.defaultSize + 20)
        doc.select(.text(a))
        doc.toggleSelection(.text(b))
        XCTAssertEqual(Set(PCBSidebarView.textIDs(of: doc)), [a, b])
        let probe = sidebar(doc)
        let size = try XCTUnwrap(probe.textFields().first { $0.placeholderString == "Mixed" })
        XCTAssertEqual(size.stringValue, "", "sizes differ: Mixed")
        probe.type("80", into: size)
        probe.submit(size)
        XCTAssertEqual(Set(doc.texts.map(\.size)), [80])
        XCTAssertEqual(size.stringValue, "80")
        probe.type("eighty", into: size)
        probe.blur()
        XCTAssertEqual(Set(doc.texts.map(\.size)), [80], "a word is not a size")
        XCTAssertEqual(size.stringValue, "80")
        try probe.step(up: true)
        XCTAssertEqual(Set(doc.texts.map(\.size)), [85])

        let side = try probe.segments(with: BoardSide.bottom.title)
        XCTAssertEqual(probe.views(NSSegmentedControl.self).filter { c in
            (0..<c.segmentCount).contains { c.label(forSegment: $0) == BoardSide.bottom.title } }.count, 1,
                       "the Text section offers the side, the Position section does not")
        try probe.pick(BoardSide.bottom.title, in: side)
        XCTAssertEqual(Set(doc.texts.map(\.side)), [.bottom])
        let other = try XCTUnwrap(TextLayer.allCases.first { $0 != doc.texts[0].layer })
        try probe.pick(other.title, in: try probe.segments(with: other.title))
        XCTAssertEqual(Set(doc.texts.map(\.layer)), [other])
    }

    func testSeveralFootprintsShareLayerLockAndMarks() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 300, y: 100))
        doc.setFootprintSide(b.id, .bottom)
        doc.moveReference(a.id, toCentre: CGPoint(x: 30, y: 30))
        doc.select(.footprint(a.id))
        doc.toggleSelection(.footprint(b.id))
        let probe = sidebar(doc)
        XCTAssertTrue(PCBSidebarView.textIDs(of: doc).isEmpty)
        let side = try probe.segments(with: BoardSide.top.title)
        XCTAssertEqual(side.selectedSegment, -1, "they disagree: no side shown")
        try probe.pick(BoardSide.top.title, in: side)
        XCTAssertEqual(doc.chips.compactMap(\.board?.side), [.top, .top])

        try probe.pressControl("Lock Position")
        XCTAssertTrue(doc.isLocked(.footprint(a.id)) && doc.isLocked(.footprint(b.id)))

        let other = try XCTUnwrap(MarkLayer.allCases.first { $0 != doc.chips[0].board?.outline })
        try probe.pick(other.title, in: try probe.segments(with: other.title))
        XCTAssertEqual(doc.chips.compactMap(\.board?.outline), [other, other])
        try probe.pick(other.title, in: try probe.segments(with: other.title, after: 1))
        XCTAssertEqual(doc.chips.compactMap(\.board?.pin1), [other, other])
        try probe.pick(other.title, in: try probe.segments(with: other.title, after: 2))
        XCTAssertEqual(doc.chips.compactMap(\.board?.reference), [other, other])

        XCTAssertNotEqual(doc.chips[0].board?.referenceOffset, .zero)
        try probe.press("Reset Reference Position")
        XCTAssertEqual(doc.chips.compactMap(\.board?.referenceOffset), [.zero, .zero])
        XCTAssertEqual(doc.undoName, "Reset Reference")
        XCTAssertFalse(probe.has("Reset Reference Position"))
    }

    func testTheSelectionListSelectsAndClears() throws {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: 30, y: 30)))
        doc.select(.footprint(a.id))
        doc.toggleSelection(.via(via))
        let probe = sidebar(doc)
        XCTAssertFalse(probe.has(BoardSide.top.title, role: "AXRadioButton") && doc.board.layers == 1)
        try probe.press(a.reference)
        XCTAssertEqual(PCBSidebarView.content(for: doc), .footprint(a.id))
        doc.select(.footprint(a.id))
        doc.toggleSelection(.via(via))
        probe.settle()
        try probe.pressControl("Clear selection")
        XCTAssertEqual(PCBSidebarView.content(for: doc), .board)
    }

    // MARK: Card sections

    func testCardOptionsToggleSegmentAndMenu() throws {
        let doc = TestSupport.newDocument()
        let dimm = try XCTUnwrap(BoardType(rawValue: "dimm"))
        doc.setBoardType(dimm)
        let probe = show(Form {
            CardOptionsSection(type: dimm, options: { doc.board.cardOptions }, set: { doc.setCardOptions($0) })
        }.formStyle(.grouped))
        XCTAssertTrue(probe.has(dimm.optionsTitle), "\(probe.labels())")
        let definition = try XCTUnwrap(dimm.definition)
        let registered = try XCTUnwrap(definition.options.first { $0.id == "registered" })
        XCTAssertFalse(probe.has(registered.title), "only for DDR5")
        let memory = try XCTUnwrap(definition.options.first { $0.id == "memory" })
        let ddr5 = try XCTUnwrap(memory.choices.last)
        try probe.pick(ddr5.title)
        XCTAssertEqual(definition.resolved(doc.board.cardOptions)["memory"], ddr5.value)
        try probe.pressControl(registered.title)
        XCTAssertEqual(definition.resolved(doc.board.cardOptions)["registered"], BoardValue.bool(true))

        let m2 = try XCTUnwrap(BoardType(rawValue: "m2"))
        doc.setBoardType(m2)
        let menu = show(Form {
            CardOptionsSection(type: m2, bus: true, options: { doc.board.cardOptions }, set: { doc.setCardOptions($0) })
        }.formStyle(.grouped))
        let m2Definition = try XCTUnwrap(m2.definition)
        let length = try XCTUnwrap(m2Definition.options.first { $0.id == "length" })
        XCTAssertFalse(length.onBus)
        XCTAssertTrue(menu.popUps().isEmpty, "on a bus only the settings that change the pins")
        let key = try XCTUnwrap(m2Definition.options.first { $0.id == "key" })
        XCTAssertTrue(menu.has(key.title), "\(menu.labels())")

        let full = show(Form {
            CardOptionsSection(type: m2, options: { doc.board.cardOptions }, set: { doc.setCardOptions($0) })
        }.formStyle(.columns))
        let choice = try XCTUnwrap(length.choices.last)
        full.choose(choice.title, in: try full.popUp(offering: choice.title))
        XCTAssertEqual(m2Definition.resolved(doc.board.cardOptions)["length"], choice.value)

        let none = show(Form {
            CardOptionsSection(type: .custom, options: { CardOptions() }, set: { _ in XCTFail("nothing to set") })
        }.formStyle(.grouped))
        XCTAssertTrue(none.has(BoardType.custom.optionsTitle), "\(none.labels())")
    }

    func testCustomCardEdgeFields() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.customCard)
        let probe = show(Form { CustomCardSection(document: doc) }.formStyle(.grouped))
        let pads = try probe.field(showing: "\(doc.board.customCard.pads)")
        probe.enter("12", into: pads)
        XCTAssertEqual(doc.board.customCard.pads, 12)
        try probe.step(up: true)
        XCTAssertEqual(doc.board.customCard.pads, 13)
        let both = doc.board.customCard.bothSides
        try probe.pressControl("Both sides")
        XCTAssertEqual(doc.board.customCard.bothSides, !both)
        let pitch = try probe.field(showing: LengthField.format(doc.board.customCard.pitch, units: .mm, precise: true))
        probe.type("3", into: pitch)
        probe.submit(pitch)
        XCTAssertEqual(doc.board.customCard.pitch, 3, accuracy: 1e-9)
        XCTAssertTrue(probe.has(LengthField.format(doc.board.customCard.tabLength, units: .mm, precise: true) + " mm"),
                      "\(probe.labels())")
    }
}
