import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AF: the Part Wizard's Footprint step as it is shown and driven —
/// the library thumbnails, "Make one from the drawing" for each style, the
/// pad table and the preview — and the model calls behind its buttons.
@MainActor
final class FootprintStepPageTests: XCTestCase {
    /// Kept for the life of the test process (see PartFileSaveOpenTests).
    private static var kept: [PartEditorModel] = []
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

    /// A copy of SN74HC00N (14 pins, library footprint PDIP-14).
    private func gate() -> PartEditorModel {
        model(.copy(name: "SN74HC00N"))
    }

    /// A generic part with `count` numbered pins (or balls, when `balls` is given) and no footprint.
    private func generic(pins count: Int, balls: [String]? = nil) -> PartEditorModel {
        let model = model(.new(kind: "generic"))
        if let balls {
            model.applyPins(balls.map { PinDraft(designator: $0, name: "P\($0)", role: "input") }, style: .ball)
        } else if count > 0 {
            model.startWithNumberedPins(count)
        }
        return model
    }

    private func show(_ model: PartEditorModel) -> ViewProbe {
        // A plain stack, not a grouped form: a form builds only the rows on screen.
        let probe = ViewProbe(ScrollView { VStack(alignment: .leading) { FootprintStepPage(model: model) }.padding() },
                              size: NSSize(width: 720, height: 900))
        probes.append(probe)
        return probe
    }

    private func field(_ probe: ViewProbe, _ name: String, file: StaticString = #filePath,
                       line: UInt = #line) throws -> NSTextField {
        try probe.field(named: name, file: file, line: line)
    }

    private func parametric(_ model: PartEditorModel, file: StaticString = #filePath,
                            line: UInt = #line) throws -> ParametricFootprintDraft {
        guard case let .parametric(draft)? = model.draft.inlineFootprint else {
            XCTFail("no parametric footprint: \(String(describing: model.draft.inlineFootprint))", file: file, line: line)
            throw XCTSkip("no parametric footprint")
        }
        return draft
    }

    /// The segmented "How" control.
    private func how(_ probe: ViewProbe) throws -> NSSegmentedControl {
        try XCTUnwrap(probe.views(NSSegmentedControl.self).first, "no segmented control")
    }

    // MARK: The tab a draft opens on

    func testTheTabFollowsTheDraftsFootprint() throws {
        let gate = gate()
        XCTAssertEqual(FootprintStepPage.choice(for: gate.draft), .library)
        gate.startParametricFootprint(.dual)
        XCTAssertEqual(FootprintStepPage.choice(for: gate.draft), .make)
        gate.startPadTable()
        XCTAssertEqual(FootprintStepPage.choice(for: gate.draft), .pads)
        XCTAssertEqual(FootprintStepPage.Choice.allCases.map(\.title),
                       ["Use a library footprint", "Make one from the drawing", "Place pads one by one"])
    }

    // MARK: Library

    func testTheLibraryShowsThumbnailsAndChoosesOne() throws {
        let model = gate()
        let probe = show(model)
        XCTAssertEqual(try how(probe).selectedSegment, 0)
        for line in FootprintStepPage.guidance { XCTAssertTrue(probe.has(line), "\(probe.labels())") }
        XCTAssertTrue(probe.has("library footprints with 14 pads"), "\(probe.labels())")
        XCTAssertTrue(probe.has("The ones matching the package “DIP” come first"), "\(probe.labels())")
        // Narrowed to one footprint by the search.
        try probe.type("SOP-14", intoField: "SOIC, 1.27 mm, TSSOP …")
        XCTAssertTrue(probe.has("library footprint with 14 pads") || probe.has("library footprints with 14 pads"),
                      "\(probe.labels())")
        let thumbnail = try probe.element("SOP-14:", role: "AXButton")
        probe.pressElement(thumbnail)
        XCTAssertEqual(model.draft.footprint, "SOP-14")
        XCTAssertNil(model.draft.inlineFootprint)
        XCTAssertTrue(probe.has("Chosen"), "\(probe.labels())")
    }

    func testASearchWithNoMatchSaysSo() throws {
        let probe = show(gate())
        try probe.type("NOSUCHFOOTPRINT", intoField: "SOIC, 1.27 mm, TSSOP …")
        XCTAssertTrue(probe.has("No library footprint has 14 pads and matches the search"), "\(probe.labels())")
    }

    func testStartFromALibraryFootprintOpensTheNumbers() throws {
        let model = gate()
        let probe = show(model)
        XCTAssertTrue(probe.has("PDIP-14"), "\(probe.labels())")
        try probe.press("Start from this one")
        let draft = try parametric(model)
        XCTAssertEqual(draft.style, .dual)
        XCTAssertEqual(draft.pins, 14)
        XCTAssertEqual(draft.id, CustomFootprintDraft.id(forPart: model.draft.uuid))
        XCTAssertEqual(try how(probe).selectedSegment, 1, "the Make tab")
        XCTAssertTrue(probe.has("The numbers (mm)"), "\(probe.labels())")
    }

    func testStartFromAnExplicitLibraryFootprintMakesAPadTable() throws {
        let model = model(.copy(name: "SN74HC00N"))
        let footprint = try XCTUnwrap(model.validator.footprints.first { $0.id == "NIXIE-IN-18" })
        XCTAssertEqual(model.startFromLibraryFootprint(footprint), .pads)
        guard case let .explicit(custom)? = model.draft.inlineFootprint else { return XCTFail("no pad table") }
        XCTAssertEqual(custom.id, CustomFootprintDraft.id(forPart: model.draft.uuid))
        XCTAssertEqual(custom.pads.count, footprint.pads.count)
        // Choosing a library footprint again drops the copy.
        model.useLibraryFootprint("PDIP-14")
        XCTAssertNil(model.draft.inlineFootprint)
        XCTAssertNil(model.draft.footprints)
        XCTAssertEqual(model.draft.footprint, "PDIP-14")
    }

    func testAPartWithNoPinsIsAskedForThemFirst() throws {
        let model = generic(pins: 0)
        XCTAssertEqual(model.draft.pinCount, 0)
        let probe = show(model)
        XCTAssertTrue(probe.has("Give the part its pins first: the library is searched by pin count."),
                      "\(probe.labels())")
        XCTAssertTrue(probe.has("Pads are drawn to scale; pin 1 is highlighted."), "\(probe.labels())")
        probe.choose(segment: 1, in: try how(probe))
        XCTAssertTrue(probe.has("Give the part its pins first."), "\(probe.labels())")
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Make a two rows footprint", role: "AXButton")))
        probe.choose(segment: 2, in: try how(probe))
        XCTAssertTrue(probe.has("For shapes the other two cannot make"), "\(probe.labels())")
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Start a pad table", role: "AXButton")))
    }

    // MARK: Make one

    func testMakeAFootprintOfTheChosenShape() throws {
        let model = generic(pins: 8)
        let probe = show(model)
        probe.choose(segment: 1, in: try how(probe))
        XCTAssertTrue(probe.has("Sketch of a two rows footprint"), "\(probe.labels())")
        let shape = try XCTUnwrap(probe.popUps().map(probe.filled).first { $0.itemTitles.contains { $0.hasPrefix("Four sides") } })
        probe.choose("Four sides — QFN, QFP, LCC", in: shape)
        XCTAssertTrue(probe.has("Sketch of a four sides footprint"), "\(probe.labels())")
        try probe.press("Make a four sides footprint")
        let draft = try parametric(model)
        XCTAssertEqual(draft.style, .quad)
        XCTAssertEqual(draft.pins, 8)
        XCTAssertTrue(probe.has("Pads per side"), "\(probe.labels())")
        // The shape picker of a made footprint starts the footprint again in the new style.
        let made = try XCTUnwrap(probe.popUps().map(probe.filled).first { $0.itemTitles.contains { $0.hasPrefix("Pin header") } })
        probe.choose("Pin header — Through-hole pin headers and sockets", in: made)
        XCTAssertEqual(try parametric(model).style, .header)
        try probe.press("Remove this footprint")
        XCTAssertNil(model.draft.inlineFootprint)
        // The unmade form keeps the shape it last showed.
        XCTAssertTrue(probe.has("Make a four sides footprint"), "\(probe.labels())")
    }

    func testABallPinoutStartsOnTheBallGrid() throws {
        let model = generic(pins: 0, balls: ["A1", "A2", "B1", "B2"])
        XCTAssertTrue(model.draft.isBallPinout)
        let probe = show(model)
        probe.choose(segment: 1, in: try how(probe))
        XCTAssertTrue(probe.has("Make a ball grid footprint"), "\(probe.labels())")
        try probe.press("Make a ball grid footprint")
        var draft = try parametric(model)
        XCTAssertEqual(draft.style, .grid)
        XCTAssertEqual(draft.balls, ["A1", "A2", "B1", "B2"])
        XCTAssertTrue(probe.has("4 of"), "\(probe.labels())")
        // A ball the footprint lacks: the button puts the pinout's balls back.
        draft.balls = ["A1"]
        draft.pins = 1
        model.draft.inlineFootprint = .parametric(draft)
        probe.settle()
        XCTAssertTrue(probe.has("The footprint has 1 pads; the pinout has 4 pins."), "\(probe.labels())")
        try probe.press("Use the pinout's 4 balls")
        XCTAssertEqual(try parametric(model).balls, ["A1", "A2", "B1", "B2"])
        XCTAssertEqual(try parametric(model).pins, 4)
        probe.enter("0.5", into: try field(probe, "Ball pad"))
        XCTAssertEqual(try parametric(model).padWidth, 0.5)
        XCTAssertEqual(try parametric(model).padHeight, 0.5)
    }

    func testAMismatchedPadCountIsMatchedToThePinout() throws {
        let model = generic(pins: 7)
        model.startParametricFootprint(.dual)
        var draft = try parametric(model)
        draft.pins = 6
        model.draft.inlineFootprint = .parametric(draft)
        let probe = show(model)
        XCTAssertEqual(try how(probe).selectedSegment, 1)
        XCTAssertTrue(probe.has("The footprint has 6 pads; the pinout has 7 pins."), "\(probe.labels())")
        try probe.press("Match the pinout")
        XCTAssertEqual(try parametric(model).pins, 7)
        // Seven pins: the rows are uneven, so the pads per row are asked.
        probe.enter("4", into: try field(probe, "Pads per row"))
        XCTAssertEqual(try parametric(model).perSide, 4)
        for (name, value) in [("Pitch", 1.27), ("Row spacing", 5.4), ("Pad length", 1.5), ("Pad width", 0.6)] {
            probe.enter("\(value)", into: try field(probe, name))
        }
        draft = try parametric(model)
        XCTAssertEqual([draft.pitch, draft.span, draft.padWidth, draft.padHeight], [1.27, 5.4, 1.5, 0.6])
        // Surface mount until the toggle drills the pads.
        try probe.pressControl("Through-hole (drilled pads)")
        XCTAssertEqual(try parametric(model).drill, 0.8)
        probe.enter("0", into: try field(probe, "Drill"))
        XCTAssertNil(try parametric(model).drill)
        probe.enter("3.9", into: try field(probe, "Body width"))
        probe.enter("4.9", into: try field(probe, "Body height"))
        XCTAssertEqual(try parametric(model).bodyWidth, 3.9)
        XCTAssertEqual(try parametric(model).bodyHeight, 4.9)
    }

    func testTheQuadFieldsSetTheSides() throws {
        let model = generic(pins: 16)
        model.startParametricFootprint(.quad)
        let probe = show(model)
        probe.enter("-2", into: try field(probe, "left"))
        XCTAssertEqual(try parametric(model).sides[0], 0, "a side never has fewer than no pads")
        probe.enter("5", into: try field(probe, "top"))
        XCTAssertEqual(try parametric(model).sides[3], 5)
        probe.enter("4.1", into: try field(probe, "Spacing across"))
        probe.enter("4.2", into: try field(probe, "Spacing down"))
        XCTAssertEqual(try parametric(model).spanX, 4.1)
        XCTAssertEqual(try parametric(model).spanY, 4.2)
    }

    func testTheHeaderFieldsSetItsRows() throws {
        let model = generic(pins: 8)
        model.startParametricFootprint(.header)
        var draft = try parametric(model)
        draft.rows = 1
        draft.rowPitch = nil
        model.draft.inlineFootprint = .parametric(draft)
        let probe = show(model)
        XCTAssertFalse(probe.textFields().contains { $0.accessibilityLabel() == "Row pitch" })
        let rows = try XCTUnwrap(probe.popUps().map(probe.filled).first { $0.itemTitles == ["1", "2"] }, "\(probe.popUps().map(probe.filled).map(\.itemTitles))")
        probe.choose("2", in: rows)
        draft = try parametric(model)
        XCTAssertEqual(draft.rows, 2)
        XCTAssertEqual(draft.rowPitch, draft.pitch)
        probe.enter("2.0", into: try field(probe, "Row pitch"))
        XCTAssertEqual(try parametric(model).rowPitch, 2.0)
        probe.enter("1.7", into: try field(probe, "Pad"))
        XCTAssertEqual(try parametric(model).padWidth, 1.7)
        XCTAssertEqual(try parametric(model).padHeight, 1.7)
        probe.enter("1.1", into: try field(probe, "Drill"))
        XCTAssertEqual(try parametric(model).drill, 1.1)
    }

    func testUseBallsAndHeaderRows() {
        var grid = ParametricFootprintDraft.starter(style: .grid, id: "x", pins: 1, balls: ["A1"])
        grid.rows = 1
        grid.columns = 1
        grid.useBalls(["A1", "A2", "C3"])
        XCTAssertEqual(grid.balls, ["A1", "A2", "C3"])
        XCTAssertEqual(grid.pins, 3)
        let size = ParametricFootprintDraft.gridSize(balls: ["A1", "A2", "C3"], count: 3)
        XCTAssertEqual([grid.rows, grid.columns], [size.rows, size.columns])
        grid.rows = 10
        grid.useBalls(["A1"])
        XCTAssertEqual(grid.rows, 10, "a grid never shrinks")

        var header = ParametricFootprintDraft.starter(style: .header, id: "h", pins: 4)
        header.pitch = 2.54
        header.rowPitch = nil
        header.setHeaderRows(2)
        XCTAssertEqual(header.rowPitch, 2.54)
        header.rowPitch = 2.0
        header.setHeaderRows(2)
        XCTAssertEqual(header.rowPitch, 2.0, "a row pitch given is kept")
        header.setHeaderRows(1)
        XCTAssertEqual(header.rows, 1)
        XCTAssertNil(header.rowPitch)
    }

    // MARK: Pad by pad

    func testStartAPadTableOnePadPerPin() throws {
        let model = generic(pins: 3)
        let probe = show(model)
        probe.choose(segment: 2, in: try how(probe))
        try probe.press("Start a pad table")
        guard case let .explicit(custom)? = model.draft.inlineFootprint else { return XCTFail("no pad table") }
        XCTAssertEqual(custom.pads.map(\.pin), [1, 2, 3])
        XCTAssertFalse(probe.has("Start a pad table"), "the pad table replaces the button: \(probe.labels())")
    }

    func testAPadTableForABallPinoutNamesTheBalls() throws {
        let model = generic(pins: 0, balls: ["A1", "B1"])
        model.startPadTable()
        guard case let .explicit(custom)? = model.draft.inlineFootprint else { return XCTFail("no pad table") }
        XCTAssertEqual(custom.pads.map(\.ball), ["A1", "B1"])
        XCTAssertEqual(custom.pads.map(\.pin), [0, 0])
    }

    func testAPadTableIsEditedAndRemoved() throws {
        let model = generic(pins: 2)
        model.startPadTable()
        let probe = show(model)
        XCTAssertEqual(try how(probe).selectedSegment, 2)
        try probe.press("Remove Custom Footprint")
        XCTAssertNil(model.draft.customFootprint)
        XCTAssertTrue(probe.has("Start a pad table"), "\(probe.labels())")
    }

    // MARK: Socket and preview

    func testASocketListsWhatFitsIt() throws {
        let model = model(.new(kind: "socket"))
        model.startWithNumberedPins(14)
        let probe = show(model)
        XCTAssertTrue(probe.has("Fits"), "\(probe.labels())")
    }

    func testThePreviewShowsTheFootprintOrWhyNot() throws {
        let model = gate()
        _ = model.validateNow()
        XCTAssertTrue(model.isValid)
        let probe = show(model)
        XCTAssertFalse(probe.has("The footprint is drawn here once the part loads."), "\(probe.labels())")
        XCTAssertFalse(probe.has("exclamationmark.triangle"))

        // A pad on a pin the pinout lacks: the backend's message.
        var custom = CustomFootprintDraft.starter(id: CustomFootprintDraft.id(forPart: model.draft.uuid), pins: 14)
        for index in custom.pads.indices { custom.pads[index].pin = index + 1 }
        custom.pads[13].pin = 99
        model.draft.customFootprint = custom
        let message = try XCTUnwrap({ () -> String? in
            if case let .failure(failure) = model.validateNow() { return failure.message }
            return nil
        }())
        probe.settle()
        XCTAssertTrue(probe.has(message), "\(probe.labels())")

        // A valid part with no footprint (a supply has none): nothing to draw.
        let bare = self.model(.copy(name: "5V"))
        _ = bare.validateNow()
        XCTAssertNil(bare.lastValidPart?.footprint)
        XCTAssertTrue(bare.isValid, bare.validationMessage ?? "")
        let other = show(bare)
        XCTAssertTrue(other.has("The footprint is drawn here once the part loads."), "\(other.labels())")
    }
}
