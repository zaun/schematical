import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AJ: the sidebar panels of the Canvas folder as they are shown:
/// a part's header and reference fields, the pour and keep-out panels, the
/// selection list, the stackup section and the Calculate Width sheet. Each
/// edit typed or picked reaches the document.
@MainActor
final class CanvasPanelsViewTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
    }

    private func show(_ view: some View, height: CGFloat = 760) -> ViewProbe {
        // A columns form: a grouped form's pop-ups never fill their menus in
        // the test host (as `PCBSidebarViewTests` found).
        let probe = ViewProbe(Form { view }.formStyle(.columns), size: NSSize(width: 460, height: height))
        probes.append(probe)
        return probe
    }

    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect { CGRect(x: x, y: y, width: w, height: h) }

    /// The pop-up offering `title`, its items filled as it opens (retried
    /// while SwiftUI fills them).
    private func popUp(offering title: String, _ probe: ViewProbe, file: StaticString = #filePath,
                       line: UInt = #line) throws -> NSPopUpButton {
        for _ in 0..<5 {
            if let found = probe.popUps().map(probe.filled).first(where: { $0.itemTitles.contains(title) }) { return found }
            probe.settle(0.3)
        }
        return try XCTUnwrap(nil as NSPopUpButton?, "no pop-up offering \(title): \(probe.popUps().map(\.itemTitles))",
                             file: file, line: line)
    }

    /// Types `text` into the field showing `value` and presses Return.
    private func enter(_ text: String, inFieldShowing value: String, _ probe: ViewProbe,
                       file: StaticString = #filePath, line: UInt = #line) throws {
        let field = try probe.textField(showing: value, file: file, line: line)
        probe.type(text, into: field)
        probe.submit(field)
    }

    // MARK: Part header and reference

    func testThePartHeaderAndReferenceFields() throws {
        let doc = TestSupport.newDocument()
        let part = try XCTUnwrap(TestSupport.allParts.first { !$0.manufacturer.isEmpty && !$0.description.isEmpty
            && symbol(for: $0) == .block })
        let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 200, y: 200))
        let probe = show(ReferenceHost(document: doc, id: chip.id))
        XCTAssertTrue(probe.has(chip.reference), "\(probe.labels())")
        XCTAssertTrue(probe.has(part.manufacturer))
        XCTAssertTrue(probe.has(part.description))
        let parts = ReferenceParts(chip.reference)

        // A new number, then a refused one: the reason shows and the field goes back.
        try enter("42", inFieldShowing: String(try XCTUnwrap(parts.number)), probe)
        XCTAssertEqual(doc.chips.first?.reference, "\(parts.prefix)42")
        let other = try TestSupport.place(doc, part.name, at: CGPoint(x: 600, y: 200))
        try enter(String(try XCTUnwrap(ReferenceParts(other.reference).number)), inFieldShowing: "42", probe)
        XCTAssertEqual(doc.chips.first?.reference, "\(parts.prefix)42", "a taken reference is refused")
        XCTAssertTrue(probe.labels().contains { $0.contains(other.reference) }, "the reason names it: \(probe.labels())")
        XCTAssertNoThrow(try probe.textField(showing: "42"))

        // Leaving the fields commits too.
        let field = try probe.textField(showing: "42")
        probe.type("43", into: field)
        probe.blur()
        XCTAssertEqual(doc.chips.first?.reference, "\(parts.prefix)43")

        // Undo renames from outside: the fields follow.
        XCTAssertTrue(doc.undo())
        probe.settle()
        XCTAssertNoThrow(try probe.textField(showing: "42"))

        // Display Reference.
        let shown = try XCTUnwrap(doc.chips.first?.showsReference)
        try probe.pressControl("Display Reference")
        XCTAssertEqual(doc.chips.first?.showsReference, !shown)

        // A power symbol has no Display Reference.
        let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 200, y: 600))
        let power = show(ReferenceSection(document: doc, chip: ground, page: .schematic))
        XCTAssertFalse(power.has("Display Reference"), "\(power.labels())")
    }

    // MARK: Pour and keep-out

    func testThePourPanelEditsThePour() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 100, y: 400))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
        XCTAssertFalse(doc.planeNetChoices().isEmpty)
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 20), layer: .top))
        let pour = { try XCTUnwrap(doc.pours.first { $0.id == id }) }
        let probe = show(PourPanel(document: doc, pour: try pour()))
        XCTAssertTrue(probe.has("Fill") || probe.has("No fill"), "\(probe.labels())")
        let units = doc.board.units
        func value(_ mm: Double) -> String { LengthField.format(mm, units: units, precise: true) }

        let x = probe.textFields()[0]
        probe.type(value(12), into: x)
        probe.submit(x)
        let grid = doc.board.traceGrid
        XCTAssertEqual(try pour().rect.minX, 12, accuracy: grid)
        let rebuilt = { () throws -> ViewProbe in
            self.probes.removeLast().close()
            return self.show(PourPanel(document: doc, pour: try pour()))
        }
        var shown = try rebuilt()
        let ys = shown.textFields().map(\.stringValue)
        XCTAssertEqual(ys.first, value(try pour().rect.minX), "\(ys)")
        // Y, Width, Height: the fields after X, in order.
        for (index, (typed, read)) in [(14.0, { try pour().rect.minY }), (30, { try pour().rect.width }),
                                       (25, { try pour().rect.height })].enumerated() {
            let field = shown.textFields()[index + 1]
            shown.type(value(typed), into: field)
            shown.submit(field)
            XCTAssertEqual(try read(), typed, accuracy: grid, "field \(index + 1)")
            shown = try rebuilt()
        }
        // Clearance, thermal gap, spoke width, minimum width.
        let before = shown.textFields().map(\.stringValue)
        XCTAssertGreaterThanOrEqual(before.count, 8, "\(before)")
        for index in 4..<8 {
            let field = shown.textFields()[index]
            shown.type(value(0.5 + Double(index) / 10), into: field)
            shown.submit(field)
            shown = try rebuilt()
        }
        XCTAssertEqual(try pour().clearance ?? 0, 0.9, accuracy: 0.01)
        XCTAssertEqual(try pour().thermalGap, 1.0, accuracy: 0.01)
        XCTAssertEqual(try pour().spokeWidth, 1.1, accuracy: 0.01)
        XCTAssertEqual(try pour().minWidth, 1.2, accuracy: 0.01)

        try shown.press("Fit to Board")
        let bounds = try XCTUnwrap(doc.boardOutlineBounds)
        XCTAssertEqual(try pour().rect.width, bounds.width, accuracy: doc.board.traceGrid)

        // Layer, net and connection pickers.
        shown = try rebuilt()
        let layer = try popUp(offering: CopperLayer.bottom.title, shown)
        shown.choose(CopperLayer.bottom.title, in: layer)
        XCTAssertEqual(try pour().layer, .bottom)
        let connection = try popUp(offering: PourConnection.solid.title, shown)
        shown.choose(PourConnection.solid.title, in: connection)
        XCTAssertEqual(try pour().connection, .solid)
        shown = try rebuilt()
        XCTAssertLessThan(shown.textFields().count, 8, "no thermal fields for a solid connection")
        doc.setPourNet(id, .rail(mv: 9000))
        shown = try rebuilt()
        XCTAssertNoThrow(try popUp(offering: "Missing net", shown), "a net the schematic lacks stays listed")
        let none = try popUp(offering: "None", shown)
        shown.choose("None", in: none)
        XCTAssertNil(try pour().net)

        // Locked: the fields are disabled and Fit does nothing.
        doc.setLocked(.pour(id), true)
        shown = try rebuilt()
        XCTAssertTrue(shown.textFields().prefix(4).allSatisfy { !$0.isEnabled })
    }

    func testTheKeepoutPanelEditsItsRectAndLayers() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let id = doc.addKeepout(rect: r(40, 10, 5, 5))
        let keepout = { try XCTUnwrap(doc.keepouts.first { $0.id == id }) }
        let units = doc.board.units
        func value(_ mm: Double) -> String { LengthField.format(mm, units: units, precise: true) }
        var probe = show(KeepoutPanel(document: doc, keepout: try keepout()))
        let rebuilt = { () throws -> ViewProbe in
            self.probes.removeLast().close()
            return self.show(KeepoutPanel(document: doc, keepout: try keepout()))
        }
        for (index, (typed, read)) in [(42.0, { try keepout().rect.minX }), (12, { try keepout().rect.minY }),
                                       (8, { try keepout().rect.width }), (6, { try keepout().rect.height })].enumerated() {
            let field = probe.textFields()[index]
            probe.type(value(typed), into: field)
            probe.submit(field)
            XCTAssertEqual(try read(), typed, accuracy: doc.board.traceGrid, "field \(index)")
            probe = try rebuilt()
        }
        try probe.pressControl("All layers")
        XCTAssertEqual(try keepout().layers, Set(doc.board.copperLayers))
        probe = try rebuilt()
        try probe.pressControl(CopperLayer.bottom.title)
        XCTAssertEqual(try keepout().layers, Set(doc.board.copperLayers).subtracting([.bottom]))
        probe = try rebuilt()
        try probe.press("Fit to Board")
        let bounds = try XCTUnwrap(doc.boardOutlineBounds)
        XCTAssertEqual(try keepout().rect.minX, bounds.minX, accuracy: doc.board.traceGrid)
    }

    func testResizeHandlesForAKeepoutAndARoundedCutout() throws {
        let doc = TestSupport.newDocument()
        let keepout = doc.addKeepout(rect: r(40, 10, 10, 10))
        let placed = doc.keepouts[0].rect
        XCTAssertEqual(doc.resizableRect, placed)
        let handle = try XCTUnwrap(doc.pourHandle(at: CGPoint(x: placed.maxX, y: placed.maxY), tolerance: 1))
        XCTAssertEqual(handle.handle, .bottomRight)
        doc.resizePourItem(.keepout(keepout), handle: .bottomRight, to: CGPoint(x: 55, y: 25))
        XCTAssertEqual(doc.keepouts[0].rect.maxX, 55, accuracy: doc.board.traceGrid)
        doc.setLocked(.keepout(keepout), true)
        XCTAssertNil(doc.resizableRect, "a locked keep-out has no handles")

        let cutout = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 10, 10, 10), radius: 0)))
        doc.selectBoardItems([.cutout(cutout)])
        XCTAssertEqual(doc.selectedCutoutID, cutout)
        XCTAssertNotNil(doc.resizableRect)
        doc.resizePourItem(.cutout(cutout), handle: .left, to: CGPoint(x: 5, y: 15))
        guard case let .rounded(rect, _)? = doc.cutouts.first?.shape else { return XCTFail("a rounded cutout") }
        XCTAssertEqual(rect.minX, 5, accuracy: doc.board.traceGrid)
        doc.resizePourItem(.via(UUID()), handle: .left, to: .zero)
        let polygon = try XCTUnwrap(doc.addCutout(.polygon([CGPoint(x: 30, y: 30), CGPoint(x: 40, y: 30),
                                                             CGPoint(x: 35, y: 40)])))
        doc.selectBoardItems([.cutout(polygon)])
        XCTAssertNil(doc.resizableRect, "a polygon cutout has no handles")
    }

    // MARK: Selection list

    func testTheBoardSelectionListsEveryKindOfItem() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 200, y: 200))
        let text = doc.addText(at: CGPoint(x: 10, y: 10))
        let trace = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 5, y: 5), CGPoint(x: 15, y: 5)], layer: .top))
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: 20, y: 20)))
        let hole = doc.addHole(at: CGPoint(x: 30, y: 30))
        let pour = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 20), layer: .top))
        let keepout = doc.addKeepout(rect: r(40, 10, 5, 5))
        let cutout = try XCTUnwrap(doc.addCutout(.rounded(rect: r(10, 30, 4, 2), radius: 0)))
        doc.setPourNet(pour, nil)
        doc.selectBoardItems([.footprint(led.id), .text(text), .trace(trace), .via(via), .hole(hole), .pour(pour),
                              .keepout(keepout), .cutout(cutout)])
        let rows = SelectionRows.board(doc)
        XCTAssertEqual(rows.map(\.title), [led.reference, "Text", "Trace", "Via", "Hole", "Pour", "Keep-out", "Cutout"])
        XCTAssertEqual(rows[5].detail, "no net on \(CopperLayer.top.title)")
        XCTAssertEqual(rows[6].detail, "All layers")
        XCTAssertTrue(rows[7].detail.hasPrefix("Rectangle "), rows[7].detail)

        var picked: BoardItem?
        var cleared = false
        let probe = show(SelectionListSection(rows: rows, onSelect: { picked = $0 }, onClear: { cleared = true }))
        XCTAssertTrue(probe.has("8 selected items"), "\(probe.labels())")
        try probe.press("Trace")
        XCTAssertEqual(picked, .trace(trace))
        try probe.pressControl("Clear selection")
        XCTAssertTrue(cleared)
        let one = show(SelectionListSection(rows: [rows[1]], onSelect: { _ in }, onClear: {}))
        XCTAssertTrue(one.has("1 selected item"), "\(one.labels())")
    }

    func testCutoutDetailsAndWireEnds() throws {
        func cutout(_ shape: BoardCutout.Shape, _ kind: BoardCutout.Kind) -> BoardCutout {
            let cutout = BoardCutout(id: UUID(), shape: shape)
            XCTAssertEqual(cutout.kind, kind)
            return cutout
        }
        XCTAssertEqual(SelectionRows.cutoutDetail(cutout(.rounded(rect: r(0, 0, 3.2, 3.2), radius: 1.6), .circle)), "Circle ⌀3.2 mm")
        XCTAssertEqual(SelectionRows.cutoutDetail(cutout(.rounded(rect: r(0, 0, 8, 2), radius: 1), .slot)), "Slot 8 × 2 mm")
        XCTAssertEqual(SelectionRows.cutoutDetail(cutout(.polygon([.zero, CGPoint(x: 1, y: 0), CGPoint(x: 0, y: 1)]), .polygon)),
                       "Polygon, 3 points")
        XCTAssertEqual(SelectionRows.preview(String(repeating: "x", count: 30)), String(repeating: "x", count: 23) + "…")

        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(SelectionRows.end(.pin(PinRef(chipID: UUID(), index: 0)), in: doc), "?")
        XCTAssertEqual(SelectionRows.end(.pin(PinRef(chipID: led.id, index: 99)), in: doc), "\(led.reference) 100")
        XCTAssertEqual(SelectionRows.end(.junction(UUID()), in: doc), "junction")
        XCTAssertEqual(SelectionRows.end(.busPin(BusPinRef(portID: UUID(), wireID: UUID())), in: doc), "bus port")
        XCTAssertEqual(SelectionRows.end(.label(UUID()), in: doc), "net label")
        let flag = doc.addNetLabel(at: CGPoint(x: 400, y: 400), style: .powerFlag)
        let cross = doc.addNetLabel(at: CGPoint(x: 600, y: 400), style: .noConnect)
        XCTAssertEqual(SelectionRows.end(.label(flag), in: doc), "power flag")
        XCTAssertEqual(SelectionRows.end(.label(cross), in: doc), "no connect")

        // Schematic rows for every kind, in drawing order.
        let other = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 600, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(try TestSupport.pin(other, "A"))))
        let wire = try XCTUnwrap(doc.wires.first)
        let route = doc.path(forWire: wire.id)
        let mid = SchematicGrid.snap(CGPoint(x: (route[0].x + route[1].x) / 2, y: (route[0].y + route[1].y) / 2))
        let third = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 700))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(third, "A")), .point(mid)))
        let junction = try XCTUnwrap(doc.junctions.first)
        let note = doc.addNote(at: CGPoint(x: 900, y: 900))
        let bus = try XCTUnwrap(doc.addBus(from: CGPoint(x: 100, y: 1100), to: CGPoint(x: 500, y: 1100)))
        let label = doc.addNetLabel(at: CGPoint(x: 1000, y: 100))
        doc.select(.chip(led.id))
        for item in [SchematicItem.junction(junction.id), .wire(wire.id), .note(note), .bus(bus), .label(label)] {
            doc.toggleSelection(item)
        }
        let all = SelectionRows.schematic(doc)
        XCTAssertEqual(all.map(\.title), [led.reference, "Junction", "Wire", "Text", "Net Label", "Bus"])
        XCTAssertEqual(all[1].detail, "(\(Int(junction.point.x)), \(Int(junction.point.y)))")
        XCTAssertTrue(all[2].detail.hasPrefix("\(led.reference) K – "), all[2].detail)
        XCTAssertTrue(all[5].detail.hasSuffix("wires"), all[5].detail)
        XCTAssertEqual(SelectionRows.end(.junction(junction.id), in: doc), "junction \(all[1].detail)")

        // Schematic rows for a title block and a label that is not a net.
        doc.setTitleBlockShown(true)
        doc.select(.titleBlock)
        doc.toggleSelection(.label(cross))
        let rows = SelectionRows.schematic(doc)
        XCTAssertEqual(rows.map(\.title), ["Title Block", NetLabel.Style.noConnect.title])
        XCTAssertEqual(rows[1].accessibilityLabel, NetLabel.Style.noConnect.title)
    }

    // MARK: Stackup

    func testTheStackupSectionEditsEachLayer() throws {
        let doc = TestSupport.newDocument()
        let probe = show(StackupSection(document: doc), height: 900)
        let stackup = { doc.board.stackup }

        try enter("40", inFieldShowing: ValueField.format(stackup().temperatureRise, places: 0), probe)
        XCTAssertEqual(stackup().temperatureRise, 40)
        try enter("50", inFieldShowing: ValueField.format(stackup().copper[0] * 1000, places: 1), probe)
        XCTAssertEqual(stackup().copper[0], 0.05, accuracy: 1e-9)
        let dielectric = stackup().dielectrics[0]
        try enter("4.2", inFieldShowing: ValueField.format(dielectric.er, places: 2), probe)
        XCTAssertEqual(stackup().dielectrics[0].er, 4.2, accuracy: 1e-9)
        let units = doc.board.units
        try enter(LengthField.format(1.2, units: units, precise: true),
                  inFieldShowing: LengthField.format(dielectric.thickness, units: units, precise: true), probe)
        XCTAssertEqual(stackup().dielectrics[0].thickness, 1.2, accuracy: 0.01)

        let weight = try popUp(offering: CopperWeight.two.title, probe)
        XCTAssertEqual(weight.titleOfSelectedItem, "Custom", "50 µm is no standard weight")
        probe.choose(CopperWeight.two.title, in: weight)
        XCTAssertEqual(stackup().copper[0], CopperWeight.two.thickness, accuracy: 1e-9)
        let kind = try popUp(offering: DielectricKind.allCases.last!.title, probe)
        probe.choose(DielectricKind.allCases.last!.title, in: kind)
        XCTAssertEqual(stackup().dielectrics[0].kind, DielectricKind.allCases.last)
        let finish = try popUp(offering: SurfaceFinish.allCases.last!.title, probe)
        probe.choose(SurfaceFinish.allCases.last!.title, in: finish)
        XCTAssertEqual(stackup().finish, SurfaceFinish.allCases.last)

        // A number field puts back text that is not a number, and a live
        // field stores each number as it is typed.
        let rise = try probe.textField(showing: "40")
        probe.type("abc", into: rise)
        probe.submit(rise)
        XCTAssertEqual(rise.stringValue, "40")
        XCTAssertEqual(stackup().temperatureRise, 40)

        try probe.press("Reset to Standard")
        XCTAssertEqual(stackup().copper[0], BoardStackup.standard(layers: doc.board.layers).copper[0], accuracy: 1e-9)
    }

    // MARK: Calculate Width

    func testTheWidthCalculatorSolvesEachGoalAndApplies() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        var applied: [Int] = []
        let sheet = WidthCalculatorSheet(document: doc, layers: doc.board.copperLayers, layer: .top, gapMM: 0.2) {
            applied.append($0)
        }
        let probe = ViewProbe(sheet, size: NSSize(width: 420, height: 520))
        probes.append(probe)
        XCTAssertTrue(probe.has("gives"), "the 50 Ω width and what it gives: \(probe.labels())")
        try probe.press("Apply")
        XCTAssertEqual(applied.count, 1)
        XCTAssertGreaterThan(applied[0], 0)

        let goal = try popUp(offering: WidthCalculatorSheet.Kind.differential.title, probe)
        probe.choose(WidthCalculatorSheet.Kind.differential.title, in: goal)
        XCTAssertTrue(probe.has("gives"), "\(probe.labels())")
        let target = try probe.textField(showing: "100")
        probe.type("90", into: target)
        XCTAssertTrue(probe.has("Ω"))
        let gap = try probe.textField(showing: LengthField.format(0.2, units: doc.board.units, precise: true))
        probe.type(LengthField.format(0.3, units: doc.board.units, precise: true), into: gap)
        try probe.press("Apply")
        XCTAssertEqual(applied.count, 2)

        probe.choose(WidthCalculatorSheet.Kind.current.title, in: goal)
        XCTAssertTrue(probe.has("carries"), "\(probe.labels())")
        let amps = try probe.textField(showing: "1")
        probe.type("200", into: amps)
        XCTAssertTrue(probe.has("Needs"), "too wide a trace: \(probe.labels())")
        let rise = try XCTUnwrap(probe.textFields().last)
        probe.type("20", into: rise)

        probe.choose(WidthCalculatorSheet.Kind.impedance.title, in: goal)
        let impedance = try probe.textField(showing: "50")
        probe.type("5000", into: impedance)
        XCTAssertTrue(probe.has("No width"), "\(probe.labels())")
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Apply", role: "AXButton")))
        try probe.press("Cancel")
        XCTAssertEqual(applied.count, 2)
    }
}

/// The reference section of a document's part, as the sidebar shows it:
/// rebuilt with the part as the document now has it.
private struct ReferenceHost: View {
    let document: SchematicDocument
    let id: UUID

    var body: some View {
        if let chip = document.chips.first(where: { $0.id == id }) {
            PartHeader(chip: chip)
            ReferenceSection(document: document, chip: chip, page: .schematic)
        }
    }
}
