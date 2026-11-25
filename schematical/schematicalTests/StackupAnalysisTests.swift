import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// plan-50-F: impedance and current estimates. The expected numbers are `planning/plan-50-data/README.md`'s
/// fixtures F1–F16 (made by `reference_formulas.py`, checked against a field solver).
@MainActor
final class StackupAnalysisTests: XCTestCase {
    private func analysis(layers: Int, wire: [Int] = []) -> StackupAnalysis {
        var board = BoardSettings()
        board.layers = layers
        for n in wire { board.styles[.inner(n)] = LayerStyle(color: LayerColor(hex: "#D8B23A")!, type: .wire) }
        return StackupAnalysis(board)
    }

    private func mil(_ solution: WidthSolution?, file: StaticString = #filePath, line: UInt = #line) throws -> Double {
        guard case let .mil(value)? = solution else { throw XCTSkip("no width: \(String(describing: solution))") }
        return value
    }

    // MARK: Formulas (F1–F13)

    func testMicrostripFixtures() {
        XCTAssertEqual(StackupAnalysis.microstripZ0(width: 0.35, height: 0.2104, thickness: 0.035, er: 4.4), 51.711, accuracy: 0.01)
        XCTAssertEqual(StackupAnalysis.microstripZ0(width: 3.0, height: 1.51, thickness: 0.035, er: 4.5), 47.958, accuracy: 0.01)
        XCTAssertEqual(StackupAnalysis.microstripZ0(width: 0.2, height: 0.0994, thickness: 0.035, er: 4.1), 46.455, accuracy: 0.01)
        XCTAssertEqual(StackupAnalysis.microstripDifferential(width: 0.15, gap: 0.15, height: 0.2104, thickness: 0.035, er: 4.4),
                       115.783, accuracy: 0.01)
    }

    func testStriplineFixtures() {
        XCTAssertEqual(StackupAnalysis.striplineZ0(width: 0.15, above: 0.6424, below: 0.6424, thickness: 0.0152, er: 4.6),
                       84.142, accuracy: 0.01)
        XCTAssertEqual(StackupAnalysis.striplineZ0(width: 0.15, above: 0.0994, below: 0.55, thickness: 0.0152, er: 4.3),
                       51.723, accuracy: 0.01)
        XCTAssertEqual(StackupAnalysis.striplineDifferential(width: 0.12, gap: 0.12, above: 0.0994, below: 0.55,
                                                             thickness: 0.0152, er: 4.3), 91.547, accuracy: 0.01)
    }

    func testCurrentFixtures() {
        XCTAssertEqual(StackupAnalysis.currentCapacity(widthMil: 10, thicknessMM: 0.035, rise: 10, external: true),
                       0.8855, accuracy: 0.0001)
        XCTAssertEqual(StackupAnalysis.currentCapacity(widthMil: 10, thicknessMM: 0.0152, rise: 10, external: false),
                       0.2418, accuracy: 0.0001)
        XCTAssertEqual(StackupAnalysis.widthForCurrent(amps: 1, thicknessMM: 0.035, rise: 10, external: true), 11.826, accuracy: 0.01)
        XCTAssertEqual(StackupAnalysis.widthForCurrent(amps: 1, thicknessMM: 0.0152, rise: 10, external: false), 70.841, accuracy: 0.01)
    }

    // MARK: Geometry of the standard boards

    func testTwoLayerTopAssumesTheOtherSideIsGround() throws {
        let board = analysis(layers: 2)
        XCTAssertEqual(board.geometry(of: .top), .microstrip(height: 1.51, er: 4.5, assumedReference: true))
        XCTAssertEqual(try XCTUnwrap(board.impedance(widthMM: 3.0, on: .top)), 47.958, accuracy: 0.01)
        // F11: 50 Ω needs 110 mil on 1.51 mm of FR4, over the 100 mil limit.
        XCTAssertEqual(try mil(board.width(forImpedance: 50, on: .top)), 110.12, accuracy: 0.05)
    }

    func testFourLayerOuterLayersSitOverTheirPlane() throws {
        let board = analysis(layers: 4)
        for layer in [CopperLayer.top, .bottom] {
            guard case let .microstrip(height, er, assumed)? = board.geometry(of: layer) else { return XCTFail("\(layer)") }
            XCTAssertEqual(height, 0.2104, accuracy: 1e-9)
            XCTAssertEqual(er, 4.4, accuracy: 1e-9)
            XCTAssertFalse(assumed)
        }
        XCTAssertEqual(try XCTUnwrap(board.impedance(widthMM: 0.35, on: .top)), 51.711, accuracy: 0.01)
        XCTAssertEqual(try XCTUnwrap(board.differentialImpedance(widthMM: 0.15, gapMM: 0.15, on: .top)), 115.783,
                       accuracy: 0.01)
        XCTAssertEqual(try mil(board.width(forImpedance: 50, on: .top)), 14.650, accuracy: 0.01)
        XCTAssertEqual(try mil(board.width(forDifferential: 100, gapMM: 0.15, on: .top)), 8.418, accuracy: 0.01)
    }

    func testSixLayerTopAndAnInnerWireLayer() throws {
        var board = analysis(layers: 6)
        guard case let .microstrip(height, er, _)? = board.geometry(of: .top) else { return XCTFail("top") }
        XCTAssertEqual(height, 0.0994, accuracy: 1e-9)
        XCTAssertEqual(er, 4.1, accuracy: 1e-9)
        XCTAssertEqual(try XCTUnwrap(board.impedance(widthMM: 0.2, on: .top)), 46.455, accuracy: 0.01)
        XCTAssertEqual(try mil(board.width(forImpedance: 50, on: .top)), 6.881, accuracy: 0.01)

        board = analysis(layers: 6, wire: [3])
        guard case let .stripline(above, below, erInner)? = board.geometry(of: .inner(3)) else { return XCTFail("L3") }
        XCTAssertEqual(above, 0.55, accuracy: 1e-9)
        XCTAssertEqual(below, 0.1164, accuracy: 1e-9)
        XCTAssertEqual(erInner, (0.55 * 4.6 + 0.1164 * 4.16) / (0.55 + 0.1164), accuracy: 1e-9)
        XCTAssertEqual(try XCTUnwrap(board.impedance(widthMM: 0.15, on: .inner(3))), 53.659, accuracy: 0.05)
        XCTAssertEqual(try XCTUnwrap(board.differentialImpedance(widthMM: 0.12, gapMM: 0.12, on: .inner(3))), 92.364,
                       accuracy: 0.05)
    }

    func testAWireLayerBelowTheTraceAddsItsCopperToTheDistance() throws {
        // Layers 3 and 4 both wires: Layer 3's plane below is Layer 5.
        let board = analysis(layers: 6, wire: [3, 4])
        guard case let .stripline(above, below, er)? = board.geometry(of: .inner(3)) else { return XCTFail("L3") }
        XCTAssertEqual(above, 0.55, accuracy: 1e-9)
        XCTAssertEqual(below, 0.1164 + 0.0152 + 0.55, accuracy: 1e-9)
        XCTAssertEqual(er, (0.55 * 4.6 + 0.1164 * 4.16 + 0.55 * 4.6) / (0.55 + 0.1164 + 0.55), accuracy: 1e-9)
    }

    func testNoGeometry() {
        XCTAssertNil(analysis(layers: 1).geometry(of: .top))
        XCTAssertEqual(analysis(layers: 1).unavailableReason(for: .top), "A 1-layer board has no reference plane.")
        XCTAssertNil(analysis(layers: 4).geometry(of: .inner(2)), "a plane layer")
        XCTAssertEqual(analysis(layers: 4).unavailableReason(for: .inner(2)), "A plane layer carries no traces.")
        // Both inner layers wires: nothing under Top.
        let wires = analysis(layers: 4, wire: [2, 3])
        XCTAssertNil(wires.geometry(of: .top))
        XCTAssertEqual(wires.unavailableReason(for: .top), "No plane layer under Top.")
        XCTAssertNil(wires.geometry(of: .inner(2)))
        XCTAssertEqual(wires.unavailableReason(for: .inner(2)), "Needs a plane layer on both sides.")
        // Layers 4 and 5 both wires: Layer 3 has no plane below.
        let sixWires = analysis(layers: 6, wire: [3, 4, 5])
        XCTAssertNil(sixWires.geometry(of: .inner(3)))
        XCTAssertNil(sixWires.impedance(widthMM: 0.2, on: .inner(3)))
    }

    func testUnreachableTargets() {
        let board = analysis(layers: 4)
        XCTAssertEqual(board.width(forImpedance: 1, on: .top), .tooLow)
        XCTAssertEqual(board.width(forImpedance: 10_000, on: .top), .tooHigh)
    }

    func testCurrentUsesTheLayersCopperAndTheRise() {
        var board = BoardSettings()
        board.layers = 4
        board.stackup.temperatureRise = 10
        let top = StackupAnalysis(board).currentCapacity(widthMil: 10, on: .top)
        XCTAssertEqual(top, 0.8855, accuracy: 0.0001)
        board.stackup.temperatureRise = 30
        XCTAssertGreaterThan(StackupAnalysis(board).currentCapacity(widthMil: 10, on: .top), top)
        let inner = StackupAnalysis(board).currentCapacity(widthMil: 10, on: .inner(2))
        XCTAssertLessThan(inner, top, "inner copper is thinner and cooled less")
    }

    // MARK: Readouts (Phase 2)

    func testReadoutStrings() {
        var board = BoardSettings()
        board.layers = 4
        let rows = EstimateText.rows(board: board, layer: .top, widthMil: 14, pairGapMM: nil)
        XCTAssertEqual(rows.map(\.title), ["Impedance", "Current"])
        XCTAssertEqual(rows.map(\.value), ["51.3 Ω", "1.13 A"])
        XCTAssertTrue(rows[0].help.contains("±10%"))
        XCTAssertTrue(rows[1].help.contains("10 °C"))
    }

    func testDifferentialRowOnlyForAPair() {
        var board = BoardSettings()
        board.layers = 4
        let pair = EstimateText.rows(board: board, layer: .top, widthMil: 14, pairGapMM: 0.15)
        XCTAssertEqual(pair.map(\.title), ["Impedance", "Differential", "Current"])
        XCTAssertEqual(pair[1].value, "77.7 Ω")
        XCTAssertTrue(pair[1].help.contains("0.15 mm"))
        XCTAssertEqual(EstimateText.rows(board: board, layer: .top, widthMil: 14, pairGapMM: nil).count, 2)
        // A net in a pair group has the group's gap; one that is not does not.
        let inPair = PlaneNet.rail(mv: 3300), other = PlaneNet.rail(mv: 5000)
        board.netGroups = [NetGroup(id: UUID(), name: "Pair", mode: .diffPair, members: [inPair], gap: 0.31)]
        XCTAssertEqual(EstimateText.pairGapMM(board: board, net: inPair), 0.31)
        XCTAssertNil(EstimateText.pairGapMM(board: board, net: other))
        XCTAssertNil(EstimateText.pairGapMM(board: board, net: nil))
        board.netGroups = [NetGroup(id: UUID(), name: "Match", mode: .lengthMatch, members: [inPair])]
        XCTAssertNil(EstimateText.pairGapMM(board: board, net: inPair), "a length group has no gap")
    }

    func testUnavailableReadoutShowsADashAndTheReason() {
        var board = BoardSettings()
        board.layers = 1
        let rows = EstimateText.rows(board: board, layer: .top, widthMil: 10, pairGapMM: nil)
        XCTAssertEqual(rows[0].value, "—")
        XCTAssertEqual(rows[0].help, "A 1-layer board has no reference plane.")
        XCTAssertTrue(rows[1].value.hasSuffix(" A"), "the current needs no plane")
        board.layers = 2
        XCTAssertTrue(EstimateText.rows(board: board, layer: .top, widthMil: 10, pairGapMM: nil)[0].help
            .contains("solid ground on the other side"))
    }

    func testTheSectionsBuildForEveryLayerCount() throws {
        for layers in BoardSettings.layerCounts {
            let doc = TestSupport.newDocument()
            doc.setLayerCount(layers)
            let id = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10)], layer: .top, width: 10))
            let trace = try XCTUnwrap(doc.traces.first { $0.id == id })
            let panel = Form { TracePanel(document: doc, trace: trace) }.frame(width: 360, height: 700)
            XCTAssertNotNil(ImageRenderer(content: panel).cgImage, "trace panel, \(layers) layers")
            let sheet = WidthCalculatorSheet(document: doc, layers: doc.board.traceLayers, layer: .top, gapMM: 0.2) { _ in }
            XCTAssertNotNil(ImageRenderer(content: sheet).cgImage, "sheet, \(layers) layers")
        }
    }

    // MARK: The sheet (Phase 3)

    func testFiftyOhmsOnTheFourLayerTop() {
        var board = BoardSettings()
        board.layers = 4
        let result = WidthCalculation.result(goal: .impedance(50), layer: .top, board: board)
        XCTAssertEqual(try XCTUnwrap(result.exactMil), 14.650, accuracy: 0.01)
        XCTAssertEqual(result.roundedMil, 15)
        XCTAssertEqual(try XCTUnwrap(result.achieved), 49.347, accuracy: 0.01)
        XCTAssertTrue(result.canApply)
        XCTAssertNil(result.message)
    }

    func testFiftyOhmsOnAnOldTwoLayerBoardCannotBeApplied() {
        let result = WidthCalculation.result(goal: .impedance(50), layer: .top, board: BoardSettings())
        XCTAssertEqual(try XCTUnwrap(result.exactMil), 110.12, accuracy: 0.05)
        XCTAssertEqual(result.roundedMil, 110)
        XCTAssertFalse(result.canApply)
        XCTAssertTrue(result.message?.contains("110") == true && result.message?.contains("100") == true, "\(result.message ?? "")")
    }

    func testCurrentRoundsUp() {
        var board = BoardSettings()
        board.layers = 4
        let result = WidthCalculation.result(goal: .current(1, rise: 10), layer: .top, board: board)
        XCTAssertEqual(try XCTUnwrap(result.exactMil), 11.826, accuracy: 0.01)
        XCTAssertEqual(result.roundedMil, 12)
        XCTAssertEqual(try XCTUnwrap(result.achieved), 1.0106, accuracy: 0.0005)
        XCTAssertTrue(result.canApply)
        // Too narrow for the limit: a milliamp on thick copper.
        let tiny = WidthCalculation.result(goal: .current(0.01, rise: 10), layer: .top, board: board)
        XCTAssertFalse(tiny.canApply)
    }

    func testDifferentialAndRefusals() {
        var board = BoardSettings()
        board.layers = 4
        let pair = WidthCalculation.result(goal: .differential(100, gapMM: 0.15), layer: .top, board: board)
        XCTAssertEqual(try XCTUnwrap(pair.exactMil), 8.418, accuracy: 0.01)
        XCTAssertEqual(pair.roundedMil, 8)
        XCTAssertEqual(WidthCalculation.result(goal: .impedance(1), layer: .top, board: board).message,
                       "No width gives an impedance that low.")
        XCTAssertEqual(WidthCalculation.result(goal: .impedance(10_000), layer: .top, board: board).message,
                       "No width gives an impedance that high.")
        board.layers = 1
        let none = WidthCalculation.result(goal: .impedance(50), layer: .top, board: board)
        XCTAssertFalse(none.canApply)
        XCTAssertEqual(none.message, "A 1-layer board has no reference plane.")
    }

    /// Apply is one undo step on the trace or on the class; the sheet's closures are what the views pass.
    func testApplySetsTheTraceWidthAndTheClassWidthInOneStepEach() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let id = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10)], layer: .top, width: 10))
        let result = WidthCalculation.result(goal: .impedance(50), layer: .top, board: doc.board)
        doc.setTraceWidth(id, try XCTUnwrap(result.roundedMil))
        XCTAssertEqual(doc.traces.first { $0.id == id }?.width, 15)
        XCTAssertEqual(doc.undoName, "Change Trace Width")
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.traces.first { $0.id == id }?.width, 10)
        doc.addNetClass(named: "Fast")
        let classID = try XCTUnwrap(doc.board.netClasses.first?.id)
        doc.setNetClassWidth(classID, 15)
        XCTAssertEqual(doc.board.netClasses.first?.width, 15)
    }
}
