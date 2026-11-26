import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AG: the Canvas folder's values and models, each answer checked:
/// the board's settings and their titles, holes, pours, pads, traces, net
/// classes and rules, the net list's lookups, the viewport's zoom, and a
/// bus's joints, slides and end drags.
@MainActor
final class CanvasModelCoverageTests: XCTestCase {
    // MARK: Titles

    func testEveryBoardSettingHasItsTitle() {
        XCTAssertEqual(MarkLayer.allCases.map(\.title), ["Hidden", "Document", "Silkscreen"])
        XCTAssertEqual(FootprintStyle.allCases.map(\.title), ["Full Module", "Connector Only"])
        XCTAssertEqual(BoardSide.allCases.map(\.title), ["Top", "Bottom"])
        XCTAssertEqual(TextLayer.allCases.map(\.title), ["Document", "Silkscreen"])
        XCTAssertEqual(FootprintOrigin.allCases.map(\.title),
                       ["Center", "Top Left", "Center Left", "Bottom Left", "Pad 1 Center"])
        XCTAssertEqual([BoardViewLayer.copper(.top), .board, .silkscreen, .document, .ratsnest].map(\.title),
                       [CopperLayer.top.title, "PCB", "Silkscreen", "Document", "Ratsnest"])
        XCTAssertEqual(PourConnection.allCases.map(\.title), ["Thermal Relief", "Solid"])
        XCTAssertEqual(LayerRuleMode.allCases.map(\.title), ["Only on", "Prefer"])
        XCTAssertEqual(NetGroupMode.allCases.map(\.title), ["Length Match", "Differential Pair"])
        XCTAssertEqual(HoleSize.allCases.map(\.title),
                       ["Custom", "M2", "M2.5", "M3", "M4", "M5", "M6", "#2", "#4", "#6", "#8", "#10"])
        XCTAssertEqual(NetLabel.Style.allCases.map(\.title),
                       ["Input", "Output", "In/Out", "Passive", "No Connect", "Power Flag"])
        XCTAssertEqual(NetLabel.Style.allCases.map(\.id), NetLabel.Style.allCases.map(\.rawValue))
    }

    func testTextColoursFollowTheBoard() {
        XCTAssertEqual(TextLayer.document.color(on: .green), BoardColor.document)
        XCTAssertEqual(TextLayer.silkscreen.color(on: .green), BoardColor.whiteInk)
        XCTAssertEqual(TextLayer.silkscreen.color(on: .white), BoardColor.blackInk)
    }

    func testACapacitanceThatIsNotANumber() {
        XCTAssertNil(CapacitorValue.parse("ten"))
        XCTAssertNil(CapacitorValue.parse("inf"))
        XCTAssertEqual(CapacitorValue.parse(" 4.7 "), 4.7)
    }

    // MARK: Board settings

    func testTheRulesTitleSaysWhenTheRulesWereChanged() throws {
        var board = BoardSettings()
        board.rulesManufacturer = .jlcpcb
        board.rules = try XCTUnwrap(DRCManufacturer.jlcpcb.fixedRules)
        XCTAssertEqual(board.rulesTitle, DRCManufacturer.jlcpcb.title)
        board.rules.minClearance += 0.05
        XCTAssertEqual(board.rulesTitle, DRCManufacturer.jlcpcb.title + " (modified)")
    }

    func testAnOutlineMustStartAtTheOrigin() {
        let moved = [CGPoint(x: 5, y: 2), CGPoint(x: 55, y: 2), CGPoint(x: 55, y: 42), CGPoint(x: 5, y: 42)]
        XCTAssertEqual(BoardSettings.problem(outline: moved), "must start at (0, 0): its top-left is (5, 2)")
    }

    func testAReferenceSplitsIntoPrefixAndNumber() {
        func split(_ reference: String) -> String {
            let parts = ReferenceParts(reference)
            return "\(parts.prefix) \(parts.number.map(String.init) ?? "-")"
        }
        XCTAssertEqual(split("U12"), "U 12")
        XCTAssertEqual(split("J1A"), "J1A -")
        XCTAssertEqual(split("12"), "12 -")
        XCTAssertEqual(ReferenceParts.validate(prefix: "", number: 1), "The prefix must be 1 to 8 letters.")
        XCTAssertEqual(ReferenceParts.validate(prefix: "U2", number: 1), "The prefix must be 1 to 8 letters.")
        XCTAssertEqual(ReferenceParts.validate(prefix: "U", number: nil), "The number must be 1 or more.")
        XCTAssertEqual(ReferenceParts.validate(prefix: "U", number: 0), "The number must be 1 or more.")
        XCTAssertNil(ReferenceParts.validate(prefix: "U", number: 3))
    }

    // MARK: Holes, pads, traces

    func testAHoleRingIsHalfTheLandLessTheHole() {
        var hole = BoardHole(id: UUID(), centre: .zero)
        hole.apply(.m4)
        XCTAssertEqual(hole.ring, (8.0 - 4.3) / 2, accuracy: 1e-9)
    }

    private func pad(_ rect: CGRect, drill: Double = 0, slot: Double = 0, opposite: Bool = false) -> PlacedPad {
        PlacedPad(designator: "1", rect: rect, drill: drill, shape: .round, radius: 0, chamfer: 0, corners: [],
                  slotLength: slot, plated: true, opposite: opposite)
    }

    func testAnOppositePadIsOnTheOtherSide() {
        let opposite = pad(CGRect(x: 0, y: 0, width: 1, height: 1), opposite: true)
        XCTAssertEqual(opposite.layerSide(on: .top), .bottom)
        XCTAssertEqual(opposite.layerSide(on: .bottom), .top)
        XCTAssertEqual(pad(CGRect(x: 0, y: 0, width: 1, height: 1)).layerSide(on: .bottom), .bottom)
    }

    func testATallSlotRunsUpAndDown() throws {
        let slot = pad(CGRect(x: 0, y: 0, width: 2, height: 4), drill: 1, slot: 3)
        let segment = try XCTUnwrap(slot.holeSegment)
        XCTAssertEqual(segment.a, CGPoint(x: 1, y: 0.5 + 0.5))
        XCTAssertEqual(segment.b, CGPoint(x: 1, y: 3.5 - 0.5))
        XCTAssertEqual(segment.radius, 0.5)
    }

    func testPadBoundsAndPin1MarkOfPlacedParts() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let union = led.boardPads.map(\.rect).reduce(CGRect.null) { $0.union($1) }
        XCTAssertEqual(try XCTUnwrap(led.boardPadBounds), union)
        let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 300, y: 100))
        XCTAssertNil(ground.boardPadBounds, "a supply has no footprint")
        XCTAssertNil(ground.pin1Mark)
    }

    func testARoundBodysCathodeBandIsItsChord() throws {
        let diode = try TestSupport.place(TestSupport.newDocument(), "1N4001-E3/54", at: CGPoint(x: 100, y: 100))
        let square = try XCTUnwrap(diode.footprint)
        let placement = try XCTUnwrap(diode.board)
        // As a custom footprint could have it: the same body, round.
        let round = Footprint(id: square.id, description: square.description, body: square.body, bodyRound: true,
                              pads: square.pads)
        func length(_ band: (a: CGPoint, b: CGPoint)?) throws -> CGFloat {
            let band = try XCTUnwrap(band)
            return hypot(band.b.x - band.a.x, band.b.y - band.a.y)
        }
        let full = try length(BoardLayout.cathodeBand(of: square, on: placement, cathode: "1", anode: "2"))
        XCTAssertTrue(abs(full - square.body.height) < 1e-9 || abs(full - square.body.width) < 1e-9,
                      "across the whole body: \(full)")
        let chord = try length(BoardLayout.cathodeBand(of: round, on: placement, cathode: "1", anode: "2"))
        // 3/5 of the way out from the centre: a chord of √(1 − 0.6²) = 0.8 of the diameter.
        XCTAssertEqual(chord, full * 0.8, accuracy: 1e-9)
        XCTAssertNil(BoardLayout.cathodeBand(of: square, on: placement, cathode: "1", anode: "1"), "pads in one place")
        XCTAssertNil(BoardLayout.cathodeBand(of: square, on: placement, cathode: "9", anode: "2"), "no such pad")
    }

    func testPad1CentreFallsBackToTheBodyCentre() throws {
        let part = try TestSupport.part("SN74HC00N")
        let footprint = try XCTUnwrap(part.footprint)
        let placement = BoardPlacement(centre: CGPoint(x: 20, y: 30), rotation: 0, origin: .pad1Center)
        XCTAssertEqual(BoardLayout.originPoint(of: placement, footprint: footprint, pad1: "no-such-pad"),
                       placement.centre)
        XCTAssertNotEqual(BoardLayout.originPoint(of: placement, footprint: footprint, pad1: "1"), placement.centre)
    }

    func testLockingAnAssistLeg() {
        var draft = TraceDraft(start: .zero)
        XCTAssertFalse(draft.lock(leg: [.zero]), "a leg that goes nowhere")
        XCTAssertFalse(draft.lock(leg: [.zero, .zero]))
        XCTAssertFalse(draft.hasLocks)
        draft.flip()
        XCTAssertTrue(draft.lock(leg: [.zero, CGPoint(x: 5, y: 0), CGPoint(x: 5, y: 5)]))
        XCTAssertEqual(draft.legStart, CGPoint(x: 5, y: 5))
        XCTAssertEqual(draft.legs, [[CGPoint(x: 5, y: 0), CGPoint(x: 5, y: 5)]])
        XCTAssertFalse(draft.flipped, "locking ends the flip")
    }

    func testTheBoardClipboardIsEmptyUntilACopy() {
        BoardClipboard.copy([])
        XCTAssertTrue(BoardClipboard.isEmpty)
        let trace = BoardTrace(id: UUID(), layer: .top, width: 10, points: [.zero, CGPoint(x: 1, y: 0)])
        BoardClipboard.copy([trace])
        XCTAssertFalse(BoardClipboard.isEmpty)
        BoardClipboard.copy([])
        XCTAssertTrue(BoardClipboard.isEmpty)
    }

    // MARK: Net classes and rules

    func testAClassMadeFromValuesAloneHasTheFileDefaults() {
        let made = NetClass(id: UUID(), name: "A")
        XCTAssertEqual(made.width, BoardTrace.defaultWidth)
        XCTAssertEqual(made.viaDrill, BoardVia.defaultDrill)
        XCTAssertEqual(made.viaDiameter, BoardVia.defaultDiameter)
        XCTAssertEqual(made.clearance(rules: .customDefault), DesignRules.customDefault.minClearance)
        var own = made
        own.clearance = 0.4
        XCTAssertEqual(own.clearance(rules: .customDefault), 0.4)
        let rules = NetRules(width: 8, viaDrill: 0.3, viaDiameter: 0.6)
        XCTAssertNil(rules.clearance, "no clearance is the DRC's minimum")
    }

    func testAGroupsMaxUncoupledIsChecked() {
        XCTAssertEqual(NetGroup.problem(mode: .diffPair, members: 2, tolerance: 0.25, gap: 0.2, maxUncoupled: 60,
                                        maxMeander: 4, name: "USB"),
                       "maxUncoupled 60 must be 0 to 50 mm")
    }

    // MARK: The net list

    func testNetLookupsOfViasPadsAndNames() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 100, y: 400))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
        let placed = try XCTUnwrap(doc.chips.first { $0.id == led.id })
        let k = try XCTUnwrap(placed.part.pins.first { $0.name == "K" })
        let kPad = try XCTUnwrap(placed.boardPads.first { $0.designator == k.designator })
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: kPad.rect.midX, y: kPad.rect.midY)))
        let free = try XCTUnwrap(doc.addVia(at: CGPoint(x: kPad.rect.midX + 30, y: kPad.rect.midY + 30)))
        let copper = doc.connectivity()
        let net = try XCTUnwrap(doc.net(ofVia: via, in: copper), "the via on the pad is on its net")
        XCTAssertEqual(copper.netNames[net], "GND")
        XCTAssertNil(doc.net(ofVia: free, in: copper), "a via on nothing")
        XCTAssertNil(doc.net(ofVia: UUID(), in: copper), "no such via")
        let stray = pad(CGRect(x: -50, y: -50, width: 1, height: 1))
        XCTAssertEqual(doc.netName(ofPad: stray, in: copper), "No net")
        XCTAssertEqual(doc.netAndClassNames(ofNet: nil, in: copper).net, "None")
        var unnamed = copper
        unnamed.netNames = [:]
        XCTAssertEqual(doc.netName(ofPad: kPad, in: unnamed), "Net \(net + 1)")
        XCTAssertEqual(doc.netAndClassNames(ofNet: net, in: unnamed).net, "Net \(net + 1)")
        let row = NetRow(net: net, key: .ground, name: "GND", pads: 2, unrouted: 1, classID: nil, layerRule: nil,
                         groupID: nil, padLabels: [])
        XCTAssertEqual(row.id, .ground)
    }

    // MARK: The viewport

    func testZoomStepsAndAPinchKeepTheirAnchors() {
        let viewport = CanvasViewport(zoom: 1)
        viewport.updateSize(CGSize(width: 400, height: 200))
        viewport.zoomIn()
        XCTAssertEqual(viewport.zoom, 1.5)
        viewport.zoomOut()
        viewport.zoomOut()
        XCTAssertEqual(viewport.zoom, 0.75)
        // The centre stays put through the steps.
        XCTAssertEqual(viewport.world(CGPoint(x: 200, y: 100)).x, 200, accuracy: 1e-9)
        while viewport.zoom > CanvasViewport.zoomLevels[0] { viewport.zoomOut() }
        viewport.zoomOut()
        XCTAssertEqual(viewport.zoom, 0.25, "no step below the least")
        for _ in 0..<10 { viewport.zoomIn() }
        XCTAssertEqual(viewport.zoom, 3, "nor above the most")
        // A pinch keeps the point under the fingers, unsnapped and clamped.
        let anchor = CGPoint(x: 50, y: 40)
        let before = viewport.world(anchor)
        viewport.setZoom(1.3, around: anchor)
        XCTAssertEqual(viewport.zoom, 1.3)
        XCTAssertEqual(viewport.world(anchor).x, before.x, accuracy: 1e-9)
        XCTAssertEqual(viewport.world(anchor).y, before.y, accuracy: 1e-9)
        viewport.setZoom(9, around: anchor)
        XCTAssertEqual(viewport.zoom, 3)
    }

    // MARK: Buses

    private let h = BusLine(a: CGPoint(x: 0, y: 0), b: CGPoint(x: 100, y: 0))

    private func bus(_ lines: [BusLine], ports: [BusPort] = []) -> Bus {
        Bus(lines: lines, wires: Bus.defaultWires(count: 2), ports: ports)
    }

    func testALineContainsOnlyItsOwnPoints() {
        let diagonal = BusLine(a: .zero, b: CGPoint(x: 10, y: 10))
        XCTAssertFalse(diagonal.contains(CGPoint(x: 5, y: 5)), "a diagonal holds nothing")
        XCTAssertTrue(h.contains(CGPoint(x: 50, y: 0)))
    }

    func testADefaultWireNameSkipsTakenNames() {
        XCTAssertEqual(Bus.defaultName(for: 2, taken: ["B0"]), "B2")
        XCTAssertEqual(Bus.defaultName(for: 1, taken: ["B0", "B1"]), "B2")
        XCTAssertEqual(Bus.defaultName(for: 0, taken: ["B0", "B2"]), "B1")
    }

    func testThreeEndsAtAPointAreABadContact() {
        let a = BusLine(a: CGPoint(x: 0, y: 0), b: CGPoint(x: 0, y: 100))
        let b = BusLine(a: CGPoint(x: 0, y: 0), b: CGPoint(x: 100, y: 0))
        let c = BusLine(a: CGPoint(x: 0, y: 0), b: CGPoint(x: -100, y: 0))
        XCTAssertEqual(bus([a, b, c]).contacts().bad, [.zero])
    }

    func testAJointKnowsTheLinesItJoins() {
        let v = BusLine(a: CGPoint(x: 100, y: 0), b: CGPoint(x: 100, y: 100))
        let t = BusLine(a: CGPoint(x: 50, y: 0), b: CGPoint(x: 50, y: 60))
        let joints = bus([h, v, t]).joints
        let corner = joints.first { if case .corner = $0 { true } else { false } }
        let tee = joints.first { if case .tee = $0 { true } else { false } }
        XCTAssertEqual(corner?.involves(h.id), true)
        XCTAssertEqual(corner?.involves(v.id), true)
        XCTAssertEqual(corner?.involves(t.id), false)
        XCTAssertEqual(tee?.involves(t.id), true, "its end")
        XCTAssertEqual(tee?.involves(h.id), true, "the line it is on")
        XCTAssertEqual(tee?.involves(v.id), false)
    }

    func testAPortSlidesBetweenItsNeighbours() {
        let port = BusPort(line: h.id, start: 40, side: .bottom, wireIDs: [])
        let range = bus([h], ports: [port]).slideRange(of: port)
        XCTAssertEqual(range, (0 + BusPort.pitch / 2)...(100 - BusPort.pitch + BusPort.pitch / 2))
        // A port that doesn't fit where it is has no range.
        let off = BusPort(line: h.id, start: 200, side: .bottom)
        XCTAssertNil(bus([h], ports: [off]).slideRange(of: off))
        // One that fills its interval exactly cannot move, but has its one start.
        let short = BusLine(a: .zero, b: CGPoint(x: 20, y: 0))
        let tight = BusPort(line: short.id, start: 10, side: .top)
        XCTAssertEqual(bus([short], ports: [tight]).slideRange(of: tight), 10...10)
    }

    func testMovingALineCarriesItsCornersAndTees() throws {
        let v = BusLine(a: CGPoint(x: 100, y: 0), b: CGPoint(x: 100, y: 100))
        let t = BusLine(a: CGPoint(x: 50, y: 100), b: CGPoint(x: 50, y: 0))
        let shape = bus([h, v, t])
        // Down by 20 (snapped): the corner's partner and the T's end follow.
        let moved = try XCTUnwrap(BusEdit.moveLine(shape, line: h.id, by: CGSize(width: 3, height: 21)))
        XCTAssertEqual(moved.first { $0.id == h.id }?.a, CGPoint(x: 0, y: 20))
        XCTAssertEqual(moved.first { $0.id == v.id }?.a, CGPoint(x: 100, y: 20))
        XCTAssertEqual(moved.first { $0.id == t.id }?.b, CGPoint(x: 50, y: 20), "a T end on it is carried along")
        XCTAssertEqual(moved.first { $0.id == t.id }?.a, CGPoint(x: 50, y: 100), "the T's other end stays")
        // The T's own line slides its T end along the line it is on.
        let slid = try XCTUnwrap(BusEdit.moveLine(shape, line: t.id, by: CGSize(width: 20, height: 0)))
        XCTAssertEqual(slid.first { $0.id == t.id }?.b, CGPoint(x: 70, y: 0))
        // Nothing to move: the lines as they are; an unknown line: nil.
        XCTAssertEqual(BusEdit.moveLine(shape, line: h.id, by: .zero), shape.lines)
        XCTAssertNil(BusEdit.moveLine(shape, line: UUID(), by: CGSize(width: 10, height: 10)))
    }

    func testDraggingALineEnd() throws {
        let v = BusLine(a: CGPoint(x: 100, y: 0), b: CGPoint(x: 100, y: 100))
        let t = BusLine(a: CGPoint(x: 50, y: 60), b: CGPoint(x: 50, y: 0))
        let shape = bus([h, v, t])
        // A free end moves along its line only.
        let free = try XCTUnwrap(BusEdit.moveEnd(shape, line: h.id, end: .a, to: CGPoint(x: -21, y: 40)))
        XCTAssertEqual(free.first { $0.id == h.id }?.a, CGPoint(x: -20, y: 0))
        // A corner moves both its lines.
        let corner = try XCTUnwrap(BusEdit.moveEnd(shape, line: h.id, end: .b, to: CGPoint(x: 130, y: 30)))
        XCTAssertEqual(corner.first { $0.id == h.id }?.b, CGPoint(x: 130, y: 30), "the corner goes where it is dragged")
        XCTAssertEqual(corner.first { $0.id == h.id }?.a, CGPoint(x: 0, y: 30))
        XCTAssertEqual(corner.first { $0.id == v.id }?.a, CGPoint(x: 130, y: 30))
        XCTAssertEqual(corner.first { $0.id == v.id }?.b, CGPoint(x: 130, y: 100))
        // A T end slides along the line it is on.
        let tee = try XCTUnwrap(BusEdit.moveEnd(shape, line: t.id, end: .b, to: CGPoint(x: 80, y: 0)))
        XCTAssertEqual(tee.first { $0.id == t.id }?.b, CGPoint(x: 80, y: 0))
        XCTAssertNil(BusEdit.moveEnd(shape, line: UUID(), end: .a, to: .zero))
    }

    func testAJoinAtACrossingOrOnTheSamePointIsRefused() {
        let v = BusLine(a: CGPoint(x: 50, y: -50), b: CGPoint(x: 50, y: 50))
        let crossing = [bus([h]), bus([v])]
        if case .other = BusJoin.touch(CGPoint(x: 50, y: 0), in: crossing) {} else { XCTFail("a crossing is other") }
        XCTAssertEqual(BusJoin.join(crossing, from: CGPoint(x: 50, y: 0), to: CGPoint(x: 50, y: 0)).failure, .samePoint)
    }
}

private extension Result {
    var failure: Failure? {
        if case let .failure(error) = self { return error }
        return nil
    }
}
