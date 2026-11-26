import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// Board Details' net tabs (`NetsSidebar.swift`), hosted and read through the
/// accessibility tree: the Nets list, its row menu, the layer-rule menu, the
/// net classes and the net groups.
@MainActor
final class NetsSidebarViewTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        // Closing a test's window must not quit the host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        await MainActor.run {
            for probe in probes {
                if let sheet = probe.window.attachedSheet { probe.window.endSheet(sheet) }
                probe.close()
            }
            probes = []
        }
        try await super.tearDown()
    }

    /// A highlighted net that outlives the view, as a window's state does.
    private final class Highlight {
        var net: PlaneNet?
        var binding: Binding<PlaneNet?> { Binding(get: { self.net }, set: { self.net = $0 }) }
    }

    /// Two LEDs, K of the first wired to A of the second.
    private func board() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "K")), .pin(try TestSupport.pin(two, "A"))))
        return doc
    }

    private func host<V: View>(_ view: V, height: CGFloat = 900) -> ViewProbe {
        let probe = ViewProbe(Form { view }.formStyle(.grouped), size: NSSize(width: 760, height: height))
        probes.append(probe)
        return probe
    }

    /// As `host`, in a columns Form: a menu-style picker in a grouped Form
    /// never fills its menu in the test host (plan-68-AI), in this one it does.
    private func hostColumns<V: View>(_ view: V, height: CGFloat = 900) -> ViewProbe {
        let probe = ViewProbe(Form { view }.formStyle(.columns), size: NSSize(width: 760, height: height))
        probes.append(probe)
        return probe
    }

    private func row(_ name: String, key: PlaneNet, pads: Int = 2, unrouted: Int = 1, group: UUID? = nil,
                     length: Double? = nil) -> NetRow {
        NetRow(net: 0, key: key, name: name, pads: pads, unrouted: unrouted, classID: nil, layerRule: nil, groupID: group,
               padLabels: [], length: length)
    }

    /// Opens the disclosure group whose label contains `title`.
    private func open(_ title: String, in probe: ViewProbe) throws {
        let triangle = try probe.element(title, role: "AXDisclosureTriangle")
        _ = triangle.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
    }

    private func press(_ element: NSObject, in probe: ViewProbe) {
        _ = element.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
    }

    // MARK: Nets

    /// The list: a heading row, each net's name, status and menus; a click
    /// on the name highlights the net and a second clears it; the class
    /// menu assigns the net.
    func testNetsListShowsRowsAndHighlights() throws {
        let doc = try board()
        let classID = doc.addNetClass(named: "Fast")
        let net = try XCTUnwrap(doc.netRows().first)
        let highlight = Highlight()
        let probe = host(NetsSection(document: doc, rows: doc.netRows(), hasBuilt: true, highlightedNet: highlight.binding))
        for heading in ["Net", "Connections", "Class", "Layers", "1 of 1 unrouted"] {
            XCTAssertTrue(probe.has(heading, role: "AXStaticText"), "\(heading): \(probe.labels())")
        }
        try probe.press(net.name)
        XCTAssertEqual(highlight.net, net.key)
        try probe.press(net.name)
        XCTAssertNil(highlight.net)

        XCTAssertTrue(probe.has("Class | Default", role: "AXPopUpButton"), "\(probe.labels())")
        let columns = hostColumns(NetsSection(document: doc, rows: doc.netRows(), hasBuilt: true, highlightedNet: .constant(nil)))
        columns.choose("Fast", in: try columns.popUp(offering: "Fast"))
        XCTAssertEqual(doc.board.netClasses.first { $0.id == classID }?.members, [net.key], "the pick assigns the class")
        let assigned = host(NetsSection(document: doc, rows: doc.netRows(), hasBuilt: true, highlightedNet: .constant(nil)))
        XCTAssertTrue(assigned.has("Class | Fast", role: "AXPopUpButton"), "the class menu shows the net's class")
        XCTAssertTrue(probe.has("Any", role: "AXPopUpButton"), "the net's own layer rule")
    }

    /// No rows: "…" before the first build, then "No nets on the board"; a
    /// filter that matches nothing says so.
    func testEmptyAndFilteredLists() throws {
        let doc = try board()
        let waiting = host(NetsSection(document: doc, rows: [], hasBuilt: false, highlightedNet: .constant(nil)))
        XCTAssertTrue(waiting.has("…", role: "AXStaticText"))
        XCTAssertFalse(waiting.has("Connections"))
        let none = host(NetsSection(document: doc, rows: [], hasBuilt: true, highlightedNet: .constant(nil)))
        XCTAssertTrue(none.has("No nets on the board"))

        let probe = host(NetsSection(document: doc, rows: doc.netRows(), hasBuilt: true, highlightedNet: .constant(nil)))
        try probe.type("zzz-no-such-net", intoField: "Filter by net or pad")
        XCTAssertTrue(probe.has("No net matches"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Connections"))
    }

    /// The row menu: Route Net (off with nothing to route), Unroute Net and
    /// Highlight / Stop Highlighting.
    func testRowMenuRoutesUnroutesAndHighlights() throws {
        let doc = try board()
        let net = try XCTUnwrap(doc.netRows().first)
        var routed: [RoutingOptions.Scope] = []
        let highlight = Highlight()
        let probe = host(NetRowMenu(document: doc, row: net, highlightedNet: highlight.binding,
                                    onRoute: { routed.append($0) }))
        XCTAssertTrue(ViewProbe.isEnabled(try probe.element("Route Net", role: "AXButton")))
        try probe.press("Route Net")
        XCTAssertEqual(routed, [.nets([net.key])])
        try probe.press("Highlight")
        XCTAssertEqual(highlight.net, net.key)

        // A trace on the net, then Unroute Net takes it off.
        let copper = doc.connectivity()
        let pads = copper.pads.enumerated().filter { copper.netOfPad[$0.offset] == net.net }.map(\.element.pad.rect)
        XCTAssertEqual(pads.count, 2)
        XCTAssertNotNil(doc.addTrace(from: CGPoint(x: pads[0].midX, y: pads[0].midY),
                                     to: CGPoint(x: pads[1].midX, y: pads[1].midY), layer: .top, diagonalFirst: false))
        XCTAssertFalse(doc.traces.isEmpty)
        try probe.press("Unroute Net")
        XCTAssertTrue(doc.traces.isEmpty)

        let routedRow = row(net.name, key: net.key, unrouted: 0)
        let lit = host(NetRowMenu(document: doc, row: routedRow, highlightedNet: highlight.binding, onRoute: { _ in }))
        XCTAssertFalse(ViewProbe.isEnabled(try lit.element("Route Net", role: "AXButton")), "nothing to route")
        try lit.press("Stop Highlighting")
        XCTAssertNil(highlight.net)
    }

    /// The layer-rule menu's items: Any Layer, and a toggle per layer under
    /// Only and Prefer; its title names the rule.
    func testLayerRuleItems() throws {
        var set: [LayerRule?] = []
        let rule = LayerRule(mode: .only, layers: [.top])
        let probe = host(NetLayerRuleItems(rule: rule, layers: [.top, .bottom], set: { set.append($0) }))
        try probe.press("Any Layer")
        XCTAssertEqual(set.count, 1)
        XCTAssertNil(set[0] ?? nil)
        // Only Top, Only Bottom, Prefer Top, Prefer Bottom.
        let toggles = probe.elements().filter { ViewProbe.role($0) == "AXCheckBox" }
        XCTAssertEqual(toggles.count, 4, "\(probe.elements().map { ViewProbe.role($0) + " " + ViewProbe.label($0) })")
        guard toggles.count == 4 else { return }
        XCTAssertEqual(toggles.map { "\(ViewProbe.attribute($0, "accessibilityValue") ?? "")" }, ["1", "0", "0", "0"])
        press(toggles[1], in: probe)
        XCTAssertEqual(set.last ?? nil, LayerRule(mode: .only, layers: [.top, .bottom]))
        press(toggles[3], in: probe)
        XCTAssertEqual(set.last ?? nil, LayerRule(mode: .prefer, layers: [.bottom]))

        XCTAssertEqual(NetLayerRuleMenu.title(nil), "Any")
        XCTAssertEqual(NetLayerRuleMenu.title(LayerRule(mode: .prefer, layers: [.top, .bottom])), "Prefer Top, Bottom")
        XCTAssertNil(NetLayerRuleMenu.toggled(rule, .top, mode: .only), "off on the last layer: no rule")
        let menu = host(NetLayerRuleMenu(rule: rule, layers: [.top], set: { _ in }))
        XCTAssertEqual(NetLayerRuleMenu(rule: nil, layers: [], set: { _ in }).help, "")
        XCTAssertTrue(menu.has("Only Top", role: "AXPopUpButton"), "\(menu.labels())")
    }

    // MARK: Net classes

    /// With no classes: the Default text and Add Power Class; **+** adds
    /// "Class N".
    func testNoClassesAddsPowerOrANewClass() throws {
        let doc = try board()
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 100, y: 400))
        let led = try XCTUnwrap(doc.chips.first)
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "A")), .pin(PinRef(chipID: gnd.id, index: 0))))
        let probe = host(NetClassesSection(document: doc, rows: doc.netRows()))
        XCTAssertTrue(probe.has("Every net is in the Default class"), "\(probe.labels())")
        try probe.press("Add Power Class")
        XCTAssertEqual(doc.board.netClasses.map(\.name), [NetClass.powerName])
        try probe.press("Add")
        XCTAssertEqual(doc.board.netClasses.map(\.name), [NetClass.powerName, "Class 2"])
    }

    /// An open class: its name, width stepper, estimates, clearance, via,
    /// layers, members, order and Delete.
    func testClassEditorEditsTheClass() throws {
        let doc = try board()
        let net = try XCTUnwrap(doc.netRows().first)
        let first = doc.addNetClass(named: "Fast")
        let second = doc.addNetClass(named: "Slow")
        doc.assignNet(net.key, to: first)
        let probe = host(NetClassesSection(document: doc, rows: doc.netRows()), height: 1400)
        let width = try XCTUnwrap(doc.board.netClasses.first).width
        XCTAssertTrue(probe.has("Fast, \(width) mil · 1 net", role: "AXDisclosureTriangle"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Slow, \(width) mil · 0 nets", role: "AXDisclosureTriangle"))
        try open("Fast", in: probe)
        XCTAssertTrue(probe.has("\(width) mil", role: "AXStaticText"), "\(probe.labels())")

        // The open class's fields: Name, then (with its own clearance) Clearance, Via Hole, Via Metal.
        XCTAssertEqual(probe.views(NSTextField.self).filter(\.isEditable).count, 3)
        try probe.type("Quick", intoFieldAt: 0)
        XCTAssertEqual(doc.board.netClasses.first?.name, "Quick")

        let stepper = try probe.element("Width", role: "AXIncrementor")
        _ = stepper.perform(NSSelectorFromString("accessibilityPerformIncrement"))
        probe.settle()
        XCTAssertEqual(doc.board.netClasses.first?.width, width + 1)

        XCTAssertNil(doc.board.netClasses.first?.clearance)
        press(try XCTUnwrap(probe.elements().first { ViewProbe.role($0) == "AXCheckBox" }, "DRC Min Clearance"), in: probe)
        XCTAssertNotNil(doc.board.netClasses.first?.clearance, "the class has its own clearance")
        let units = doc.board.units
        XCTAssertEqual(probe.views(NSTextField.self).filter(\.isEditable).count, 4, "Clearance shows")
        try probe.type("0.5", intoFieldAt: 1)
        XCTAssertEqual(try XCTUnwrap(doc.board.netClasses.first?.clearance),
                       try XCTUnwrap(LengthField.parse("0.5", units: units)), accuracy: 1e-6)
        try probe.type("0.3", intoFieldAt: 2)
        XCTAssertEqual(try XCTUnwrap(doc.board.netClasses.first).viaDrill,
                       try XCTUnwrap(LengthField.parse("0.3", units: units)), accuracy: 1e-6)
        try probe.type("0.9", intoFieldAt: 3)
        XCTAssertEqual(try XCTUnwrap(doc.board.netClasses.first).viaDiameter,
                       try XCTUnwrap(LengthField.parse("0.9", units: units)), accuracy: 1e-6)
        XCTAssertTrue(probe.has("1", role: "AXPopUpButton"), "the members menu: \(probe.labels())")

        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Move Up", role: "AXButton")))
        try probe.press("Move Down")
        XCTAssertEqual(doc.board.netClasses.map(\.id), [second, first])
        try open("Slow", in: probe)
        try probe.press("Delete")
        XCTAssertEqual(doc.board.netClasses.count, 1)
    }

    /// Calculate Width opens the calculator sheet on the window.
    func testCalculateWidthOpensTheCalculator() throws {
        let doc = try board()
        _ = doc.addNetClass(named: "Fast")
        let probe = host(NetClassesSection(document: doc, rows: doc.netRows()), height: 1200)
        try open("Fast", in: probe)
        try probe.press("Calculate Width")
        probe.settle(0.5)
        let sheet = try XCTUnwrap(probe.window.attachedSheet, "the calculator")
        let sheetProbe = ViewProbe(window: sheet)
        try sheetProbe.press("Cancel")
        probe.settle(0.5)
        XCTAssertNil(probe.window.attachedSheet)
    }

    /// A class's members menu: None with no members; each member by name
    /// ("Missing net" for one the board no longer has), chosen to remove.
    func testMemberItemsRemoveAMember() throws {
        let doc = try board()
        let net = try XCTUnwrap(doc.netRows().first)
        var removed: [PlaneNet] = []
        let gone = PlaneNet.rail(mv: 3300)
        let probe = host(NetMemberItems(members: [net.key, gone], rows: doc.netRows(), remove: { removed.append($0) }))
        try probe.press(net.name)
        try probe.press("Missing net")
        XCTAssertEqual(removed, [net.key, gone])

        let id = doc.addNetClass(named: "Empty")
        let classes = host(NetClassesSection(document: doc, rows: doc.netRows()), height: 1200)
        XCTAssertNotNil(doc.board.netClasses.first { $0.id == id })
        try open("Empty", in: classes)
        XCTAssertTrue(classes.has("None", role: "AXStaticText"), "\(classes.labels())")
    }

    // MARK: Net groups

    /// No groups: the explaining text; **+**'s items add a length-match
    /// group and a pair, named by count.
    func testNoGroupsAndAddingThem() throws {
        let doc = try board()
        let empty = host(NetGroupsSection(document: doc, rows: doc.netRows()))
        XCTAssertTrue(empty.has("No groups."), "\(empty.labels())")
        XCTAssertTrue(empty.has("Add", role: "AXPopUpButton"))
        let items = host(NetGroupAddItems(document: doc))
        try items.press("Length Match")
        try items.press("Differential Pair")
        XCTAssertEqual(doc.board.netGroups.map(\.name), ["Group 1", "Pair 2"])
        XCTAssertEqual(doc.board.netGroups.map(\.mode), [.lengthMatch, .diffPair])
    }

    /// An open pair: name, mode, its sizes, each member with its length (a
    /// multi-drop one marked, a missing one named so), − takes one out, and
    /// Add Net is off once the pair has 2.
    func testGroupEditorEditsThePair() throws {
        let doc = try board()
        let net = try XCTUnwrap(doc.netRows().first)
        let id = doc.addNetGroup(named: "Pair 1", mode: .diffPair)
        XCTAssertTrue(doc.assignNet(net.key, toGroup: id))
        let multi = PlaneNet.rail(mv: 3300)
        XCTAssertTrue(doc.assignNet(multi, toGroup: id))
        let rows = [row(net.name, key: net.key, group: id, length: 10),
                    row("BUS", key: multi, pads: 3, group: id, length: 12)]
        let probe = host(NetGroupsSection(document: doc, rows: rows), height: 1400)
        XCTAssertTrue(probe.has("Pair 1, Differential Pair · 2 nets", role: "AXDisclosureTriangle"), "\(probe.labels())")
        try open("Pair 1", in: probe)
        XCTAssertTrue(probe.has("multi-drop", role: "AXStaticText"), "\(probe.labels())")
        let shortBy = NetLengthLabel.text(length: 10, longest: 12, units: doc.board.units)
        XCTAssertTrue(probe.has(shortBy, role: "AXStaticText"), "the shorter member's shortfall: \(probe.labels())")
        XCTAssertTrue(probe.has("Gap"))
        XCTAssertTrue(probe.has("Max Uncoupled"))
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Add Net", role: "AXPopUpButton")), "a pair has its 2")

        let units = doc.board.units
        // The fields: Name, Tolerance, Gap, Max Uncoupled, Max Meander.
        XCTAssertEqual(probe.views(NSTextField.self).filter(\.isEditable).count, 5)
        for (field, value, read) in [(1, "0.2", { (g: NetGroup) in g.tolerance }),
                                     (2, "0.25", { (g: NetGroup) in g.gap }),
                                     (3, "3", { (g: NetGroup) in g.maxUncoupled }),
                                     (4, "1.5", { (g: NetGroup) in g.maxMeander })] {
            try probe.type(value, intoFieldAt: field)
            XCTAssertEqual(read(try XCTUnwrap(doc.board.netGroups.first)), try XCTUnwrap(LengthField.parse(value, units: units)),
                           accuracy: 1e-6, "field \(field)")
        }
        try probe.type("Clock", intoFieldAt: 0)
        XCTAssertEqual(doc.board.netGroups.first?.name, "Clock")

        let button = try XCTUnwrap(probe.buttons(labelled: "Remove").first, "\(probe.labels())")
        probe.pressElement(button)
        XCTAssertEqual(doc.board.netGroups.first?.members.count, 1)

        XCTAssertTrue(probe.labels().contains(NetGroupMode.diffPair.title), "the mode menu: \(probe.labels())")
        try probe.press("Delete")
        XCTAssertTrue(doc.board.netGroups.isEmpty)
    }

    /// A length-match group naming a net the board no longer has: "Missing
    /// net", with no length.
    func testGroupWithAMissingNet() throws {
        let doc = try board()
        let id = doc.addNetGroup(named: "Group 1", mode: .lengthMatch)
        XCTAssertTrue(doc.assignNet(.rail(mv: 5000), toGroup: id))
        let probe = host(NetGroupsSection(document: doc, rows: doc.netRows()), height: 1000)
        XCTAssertTrue(probe.has("Group 1, \(NetGroupMode.lengthMatch.title) · 1 net", role: "AXDisclosureTriangle"), "\(probe.labels())")
        try open("Group 1", in: probe)
        XCTAssertTrue(probe.has("Missing net", role: "AXStaticText"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Gap"), "only a pair has a gap")
        XCTAssertTrue(ViewProbe.isEnabled(try probe.element("Add Net", role: "AXPopUpButton")), "a free net to add")

        // The mode menu's pick changes the group's mode.
        let columns = hostColumns(NetGroupsSection(document: doc, rows: doc.netRows()), height: 1000)
        try open("Group 1", in: columns)
        columns.choose(NetGroupMode.diffPair.title, in: try columns.popUp(offering: NetGroupMode.diffPair.title))
        XCTAssertEqual(doc.board.netGroups.first?.mode, .diffPair)
    }

    /// The Add Net items: each free net, a multi-drop one marked; choosing
    /// one puts it in the group.
    func testFreeItemsAddANet() throws {
        let doc = try board()
        let net = try XCTUnwrap(doc.netRows().first)
        let id = doc.addNetGroup(named: "Group 1", mode: .lengthMatch)
        let probe = host(NetGroupFreeItems(document: doc, free: [net, row("BUS", key: .rail(mv: 3300), pads: 3)], group: id))
        XCTAssertTrue(probe.has("BUS (multi-drop)", role: "AXButton"))
        try probe.press(net.name)
        XCTAssertEqual(doc.board.netGroups.first?.members, [net.key])
    }

    /// A member's length: "—" with none, the shortfall from the longest,
    /// red past the tolerance.
    func testLengthLabel() throws {
        XCTAssertEqual(NetLengthLabel.text(length: nil, longest: 5, units: .mm), "—")
        XCTAssertEqual(NetLengthLabel.text(length: 5, longest: 5, units: .mm),
                       LengthField.format(5, units: .mm, precise: false) + " mm")
        XCTAssertTrue(NetLengthLabel.text(length: 4, longest: 5, units: .mm).hasSuffix(")"))
        XCTAssertTrue(NetLengthLabel.isOver(length: 4, longest: 5, tolerance: 0.5))
        XCTAssertFalse(NetLengthLabel.isOver(length: 4.8, longest: 5, tolerance: 0.5))
        XCTAssertFalse(NetLengthLabel.isOver(length: nil, longest: 5, tolerance: 0.5))
        let probe = host(NetLengthLabel(length: 4, longest: 5, tolerance: 0.5, units: .mm))
        XCTAssertTrue(probe.has(NetLengthLabel.text(length: 4, longest: 5, units: .mm), role: "AXStaticText"))
    }
}
