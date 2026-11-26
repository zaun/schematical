import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AE: the Part Editor form's sections as they are shown (Identity,
/// the verdict, Footprint and Fits, Supply, the kind's fields, the pinouts),
/// and the model helpers and pure functions they call.
@MainActor
final class PartEditorSectionsTests: XCTestCase {
    private static var kept: [AnyObject] = []
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
    }

    private func model(_ source: PartEditorTarget.Source) -> PartEditorModel {
        let model = PartEditorModel(target: PartEditorTarget(source: source), catalog: .shared)
        Self.kept.append(model)
        return model
    }

    private func show<V: View>(_ view: V, width: CGFloat = 900) -> ViewProbe {
        let probe = ViewProbe(Form { view }.formStyle(.grouped), size: NSSize(width: width, height: 900))
        probes.append(probe)
        return probe
    }

    private func checked(_ model: PartEditorModel) {
        let deadline = Date().addingTimeInterval(10)
        while model.validation == nil, Date() < deadline { RunLoop.main.run(until: Date().addingTimeInterval(0.05)) }
    }

    /// Presses the toggle titled `title` (a checkbox, else the element that carries the title).
    private func toggle(_ probe: ViewProbe, _ title: String, index: Int = 0) throws {
        if let button = probe.views(NSButton.self).first(where: { $0.title == title }) {
            button.performClick(nil)
        } else {
            // A grouped form shows a toggle's title as text beside a box
            // labelled only by its value ("1"): the box at `index`.
            let unlabelled = probe.elements().filter { ViewProbe.role($0) == "AXCheckBox" }
            let box = try XCTUnwrap(probe.find(title, role: "AXCheckBox").first
                                        ?? (unlabelled.indices.contains(index) ? unlabelled[index] : nil),
                                    "\(probe.elements().map { "\(ViewProbe.role($0)):\(ViewProbe.label($0))" })")
            _ = box.perform(NSSelectorFromString("accessibilityPerformPress"))
        }
        probe.settle()
    }

    typealias Box = PartEditorWindowViewTests.Box

    // MARK: Identity

    func testIdentityNamesTheLibraryCopyAndEditsOptionalKeys() throws {
        let model = model(.copy(name: "SN74HC00N"))
        XCTAssertEqual(model.libraryName, "SN74HC00N.2")
        let probe = show(IdentitySection(model: model))
        XCTAssertTrue(probe.has("A part named SN74HC00N is loaded; Add to Library adds this one as SN74HC00N.2."),
                      "\(probe.labels())")
        try PartEditorWindowViewTests.type("Acme", intoFieldAt: 1, of: probe)
        XCTAssertEqual(model.draft.manufacturer, "Acme")
        try PartEditorWindowViewTests.type("", intoFieldAt: 1, of: probe)
        XCTAssertNil(model.draft.manufacturer, "an emptied field removes the key")
        try toggle(probe, "Obsolete")
        XCTAssertTrue(model.draft.flags.contains("obsolete"), "\(model.draft.flags)")

        model.draft.name = ""
        probe.settle()
        XCTAssertFalse(probe.has("is loaded; Add to Library"))
    }

    func testTheOptionalTextAndDrawerAndFlagBindings() {
        let model = model(.copy(name: "SN74HC00N"))
        let note = model.optionalText(\.note)
        note.wrappedValue = "A note"
        XCTAssertEqual(model.draft.note, "A note")
        XCTAssertEqual(note.wrappedValue, "A note")
        note.wrappedValue = ""
        XCTAssertNil(model.draft.note)
        XCTAssertEqual(note.wrappedValue, "")

        model.draft.drawer = ["Logic"]
        XCTAssertEqual(model.drawerLevel(2).wrappedValue, "")
        model.drawerLevel(2).wrappedValue = "Deep"
        XCTAssertEqual(model.draft.drawer, ["Logic", "", "Deep"])
        model.drawerLevel(2).wrappedValue = ""
        XCTAssertEqual(model.draft.drawer, ["Logic"], "trailing empty levels go")

        model.draft.flags = []
        let flags = PartDraft.flagNames
        XCTAssertGreaterThan(flags.count, 1)
        let first = flags[0], last = flags[flags.count - 1]
        model.flagBinding(last).wrappedValue = true
        model.flagBinding(first).wrappedValue = true
        model.flagBinding(first).wrappedValue = true
        XCTAssertEqual(model.draft.flags, [first, last], "kept in the names' order, once")
        XCTAssertTrue(model.flagBinding(first).wrappedValue)
        model.flagBinding(first).wrappedValue = false
        XCTAssertEqual(model.draft.flags, [last])
    }

    // MARK: The verdict

    func testTheVerdictShowsEachState() throws {
        let checking = show(VerdictSection(validation: nil, status: nil))
        XCTAssertTrue(checking.has("Checking…"), "\(checking.labels())")

        let model = model(.copy(name: "SN74HC00N"))
        checked(model)
        let loads = show(VerdictSection(validation: model.validation, status: "Saved x.schpart"))
        XCTAssertTrue(loads.has("The part loads: 14 pins"), "\(loads.labels())")
        XCTAssertTrue(loads.has("Saved x.schpart"))

        let fails = show(VerdictSection(validation: .failure(.backend("part: devices[0] is wrong")), status: nil))
        XCTAssertTrue(fails.has("part: devices[0] is wrong"), "\(fails.labels())")
        XCTAssertFalse(fails.has("Checking…"))
    }

    // MARK: Footprint

    func testTheBundledFootprintPickerAndCustomFootprint() throws {
        let model = model(.copy(name: "SN74HC00N"))
        model.draft.footprint = "NOT-OFFERED-FP"
        model.draft.connectorFootprint = "NOT-OFFERED-CONN"
        let probe = show(FootprintSection(model: model))
        XCTAssertTrue(probe.has("Bundled footprints with 14 pads (the part's pinout has 14 pins)."), "\(probe.labels())")
        XCTAssertTrue(probe.has("NOT-OFFERED-FP"), "an id that is not offered stays listed")
        XCTAssertFalse(probe.has("Fits"), "not a socket")

        try probe.press("Custom Footprint…")
        let custom = try XCTUnwrap(model.draft.customFootprint)
        XCTAssertEqual(custom.pads.map(\.pin), Array(1...14))
        XCTAssertEqual(custom.id, CustomFootprintDraft.id(forPart: model.draft.uuid))
        XCTAssertTrue(probe.has("Custom Footprint"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Bundled footprints with"))

        var edited = custom
        edited.pads.removeLast()
        model.customFootprintBinding.wrappedValue = edited
        XCTAssertEqual(model.draft.customFootprint, edited)
        XCTAssertEqual(model.customFootprintBinding.wrappedValue, edited)
    }

    func testCustomFootprintNumbersOnlyThePadsThePinoutHas() {
        let model = model(.new(kind: "generic"))
        model.draft.pinouts[0].pins = [PinDraft(designator: "7", name: "A", role: "inout"),
                                       PinDraft(designator: "X", name: "B", role: "inout")]
        model.startCustomFootprint()
        XCTAssertEqual(model.draft.customFootprint?.pads.map(\.pin), [7, 2], "a pin with no number keeps the starter's")
    }

    func testASocketListsTheFootprintsItFits() throws {
        let model = model(.new(kind: "socket"))
        model.draft.fits = ["AE-NOT-OFFERED"]
        let probe = show(FootprintSection(model: model))
        XCTAssertTrue(probe.has("Fits"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Pin n of the chip sits on the socket's pad n."))
        try toggle(probe, "AE-NOT-OFFERED")
        XCTAssertNil(model.draft.fits, "unchecking the last one removes the key")

        XCTAssertEqual(FitsEditor.toggled(nil, "A", on: true), ["A"])
        XCTAssertEqual(FitsEditor.toggled(["A"], "A", on: true), ["A"], "once")
        XCTAssertEqual(FitsEditor.toggled(["A", "B"], "A", on: false), ["B"])
        XCTAssertNil(FitsEditor.toggled(["A"], "A", on: false))

        // An 8-pin socket is offered the bundled 8-pad chip footprints, in a box each.
        model.draft.pinouts[0].pins = (1...8).map { PinDraft(designator: "\($0)", name: "P\($0)", role: "inout") }
        probe.settle()
        let offered = FitsEditor.choices(model.validator.footprints, pins: 8, own: model.draft.footprint)
        let first = try XCTUnwrap(offered.first, "no 8-pad footprint")
        XCTAssertTrue(probe.has("\(first.id) — \(first.description)"), "\(probe.labels())")
        try toggle(probe, first.id)
        XCTAssertEqual(model.draft.fits, [first.id])
    }

    // MARK: Supply and the kind's fields

    func testSupplyShowsForAKindWithSupplyKeysOnly() throws {
        let kind = try XCTUnwrap(PartKinds.schemas.keys.sorted().first { kind in
            PartKinds.schemas[kind]!.part.contains { PartEditorForm.supplyKeys.contains($0.key) }
        })
        let with = show(SupplySection(model: model(.new(kind: kind))))
        XCTAssertTrue(with.has("Supply"), "\(kind): \(with.labels())")

        let without = try XCTUnwrap(PartKinds.schemas.keys.sorted().first { kind in
            !PartKinds.schemas[kind]!.part.contains { PartEditorForm.supplyKeys.contains($0.key) }
        })
        let none = show(SupplySection(model: model(.new(kind: without))))
        XCTAssertFalse(none.has("Supply"), "\(without): \(none.labels())")

        let unknown = model(.new(kind: "generic"))
        unknown.draft.kind = "no-such-kind"
        let nothing = show(SupplySection(model: unknown))
        XCTAssertFalse(nothing.has("Supply"))
    }

    func testTheKindSectionHeaderAndAKindWithNoForm() {
        let gate = model(.new(kind: "gate"))
        let titled = show(KindFieldsSection(model: gate))
        XCTAssertTrue(titled.has("Logic Gate"), "\(titled.labels())")
        let bare = show(KindFieldsSection(model: gate, keys: ["part.family"], title: "", guided: true))
        XCTAssertFalse(bare.has("Logic Gate"), "\(bare.labels())")
        XCTAssertTrue(bare.has("family"), "a guided field shows its key: \(bare.labels())")

        let unknown = model(.new(kind: "generic"))
        unknown.draft.kind = "no-such-kind"
        let none = show(KindFieldsSection(model: unknown))
        XCTAssertTrue(none.has("The no-such-kind kind has no form yet."), "\(none.labels())")
    }

    func testTheKindSectionsFields() throws {
        let schema = try XCTUnwrap(PartKinds.schemas["gate"])
        let all = KindFieldsSection.fields(schema: schema, variant: nil, keys: nil)
        XCTAssertEqual(all.device.map(\.key), schema.deviceFields(variant: nil).map(\.key))
        XCTAssertFalse(all.part.contains { PartEditorForm.supplyKeys.contains($0.key) })
        let some = KindFieldsSection.fields(schema: schema, variant: nil, keys: ["part.family", "device.timing", "device.nope"])
        XCTAssertEqual(some.device.map(\.key), ["timing"])
        XCTAssertEqual(some.part.map(\.key), ["family"])
    }

    // MARK: Pinouts

    func testPinoutsAddRemoveAndRename() throws {
        let pinouts = Box([PinoutDraft(name: "p1", style: .numbered,
                                       pins: [PinDraft(designator: "1", name: "A", role: "input")])])
        let part = Box("p1")
        let probe = show(PinoutsSection(pinouts: pinouts.binding, partPinout: part.binding, status: "Renamed in 2 settings"))
        XCTAssertTrue(probe.has("Renamed in 2 settings"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Remove Pinout"), "the only pinout cannot go")
        try probe.press("Add Numbered Pinout")
        try probe.press("Add Ball Pinout")
        XCTAssertEqual(pinouts.value.map(\.name), ["p1", "p2", "p3"])
        XCTAssertEqual(pinouts.value.map(\.style), [.numbered, .numbered, .ball])
        XCTAssertTrue(probe.has("Ball"))
        try probe.press("Remove Pinout")
        XCTAssertEqual(pinouts.value.map(\.name), ["p2", "p3"])
        XCTAssertEqual(part.value, "p2", "the part's pinout moves to the first one left")

        try PartEditorWindowViewTests.type("main", intoFieldAt: 0, of: probe)
        XCTAssertEqual(pinouts.value.first?.name, "main")
        XCTAssertEqual(part.value, "main", "renaming the part's pinout renames its choice")

        part.value = "gone"
        probe.settle()
        XCTAssertTrue(probe.has("gone"), "a pinout name the list lacks stays in the picker: \(probe.labels())")
    }

    func testPinoutsWithNoRenameStatusShowNone() {
        let pinouts = Box([PinoutDraft(name: "p1", style: .ball, pins: [])])
        let probe = show(PinoutsSection(pinouts: pinouts.binding, partPinout: .constant("p1")))
        XCTAssertTrue(probe.has("Add Ball Pinout"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Renamed in"))
    }

    func testAddingAPinoutSkipsTakenNames() {
        var pinouts = [PinoutDraft(name: "p2", style: .numbered, pins: [])]
        PinoutsSection.add(.ball, to: &pinouts)
        XCTAssertEqual(pinouts.map(\.name), ["p2", "p3"])
        var part = "p2"
        PinoutsSection.remove(pinouts[1].id, from: &pinouts, partPinout: &part)
        XCTAssertEqual(part, "p2", "removing another pinout keeps the part's")
        PinoutsSection.remove(pinouts[0].id, from: &pinouts, partPinout: &part)
        XCTAssertEqual(part, "p2", "nothing left to move to")
    }

    func testThePinTableAddsMovesRemovesAndNamesPins() throws {
        let a = PinDraft(designator: "1", name: "A", role: "input")
        let b = PinDraft(designator: "5", name: "B", role: "output")
        let pinout = Box(PinoutDraft(name: "p1", style: .numbered, pins: [a, b]))
        var committed: [String] = []
        let probe = show(PinoutEditor(pinout: pinout.binding, onRemove: nil, onRename: { _, _ in },
                                      commitName: { _, name in committed.append(name) }))
        XCTAssertTrue(probe.has("Numbered"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Active low"))
        try probe.press("Add Pin")
        XCTAssertEqual(pinout.value.pins.map(\.designator), ["1", "5", "6"])
        XCTAssertEqual(pinout.value.pins.last?.role, "input")
        try probe.press("Remove Pin")
        XCTAssertEqual(pinout.value.pins.map(\.designator), ["5", "6"])
        try probe.press("Move Down")
        XCTAssertEqual(pinout.value.pins.map(\.designator), ["6", "5"])
        try probe.press("Move Up")
        XCTAssertEqual(pinout.value.pins.map(\.designator), ["6", "5"], "the top pin does not move up")

        let editor = PinoutEditor(pinout: pinout.binding, onRemove: nil, onRename: { _, _ in },
                                  commitName: { _, name in committed.append(name) })
        editor.commitPinName(b.id, "BB")
        XCTAssertEqual(committed, ["BB"])
        let direct = PinoutEditor(pinout: pinout.binding, onRemove: nil, onRename: { _, _ in })
        direct.commitPinName(b.id, "BB")
        XCTAssertEqual(pinout.value.pins.first { $0.id == b.id }?.name, "BB")
        direct.commitPinName(UUID(), "none")
        XCTAssertFalse(pinout.value.pins.contains { $0.name == "none" })
    }

    func testABallPinoutAndMoves() {
        var ball = PinoutDraft(name: "bga", style: .ball, pins: [PinDraft(designator: "A1", name: "X", role: "input")])
        PinoutEditor.addPin(to: &ball)
        XCTAssertEqual(ball.pins.map(\.designator), ["A1", "A2"])
        var empty = PinoutDraft(name: "p", style: .numbered, pins: [])
        PinoutEditor.addPin(to: &empty)
        XCTAssertEqual(empty.pins.map(\.designator), ["1"])
        PinoutEditor.move(ball.pins[0].id, by: 1, in: &ball)
        XCTAssertEqual(ball.pins.map(\.designator), ["A2", "A1"])
        PinoutEditor.move(UUID(), by: 1, in: &ball)
        PinoutEditor.move(ball.pins[1].id, by: 1, in: &ball)
        XCTAssertEqual(ball.pins.map(\.designator), ["A2", "A1"])
    }

    func testTheActiveLowMenu() throws {
        var pin = PinDraft(designator: "1", name: "CS/OE", role: "input")
        XCTAssertEqual(ActiveLowMenu.label(for: pin), "Low")
        ActiveLowMenu.set("OE", activeLow: true, on: &pin)
        XCTAssertEqual(ActiveLowMenu.label(for: pin), "Low: OE")
        ActiveLowMenu.set("CS", activeLow: true, on: &pin)
        XCTAssertEqual(ActiveLowMenu.label(for: pin), "Low ✓")
        ActiveLowMenu.set("CS", activeLow: false, on: &pin)
        XCTAssertEqual(pin.activeLow, ["OE"])
        pin.role = "power"
        XCTAssertEqual(ActiveLowMenu.label(for: pin), "—")

        let box = Box(PinDraft(designator: "1", name: "CS/OE", role: "input"))
        let probe = show(ActiveLowMenu(pin: box.binding), width: 300)
        XCTAssertEqual(try PartEditorWindowViewTests.pick("OE", in: probe), ["CS", "OE"])
        XCTAssertEqual(box.value.activeLow, ["OE"])
    }
}
