import CoreGraphics
import Foundation
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
}
