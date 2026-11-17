import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-50-A: the board's stackup (copper weights, dielectrics, finish): its
/// standard values, its tie to the layer count, the file and the edits.
@MainActor
final class BoardStackupTests: XCTestCase {
    // MARK: Model

    func testStandardThicknesses() {
        XCTAssertEqual(BoardStackup.standard(layers: 1).thickness, 1.6, accuracy: 1e-9)
        XCTAssertEqual(BoardStackup.standard(layers: 2).thickness, 1.6, accuracy: 1e-9)
        XCTAssertEqual(BoardStackup.standard(layers: 4).thickness, 1.6062, accuracy: 1e-9)
        XCTAssertEqual(BoardStackup.standard(layers: 6).thickness, 1.566, accuracy: 1e-9)
    }

    func testStandardShapeMatchesTheLayerCount() {
        for layers in BoardSettings.layerCounts {
            let stackup = BoardStackup.standard(layers: layers)
            XCTAssertEqual(stackup.copper.count, layers)
            XCTAssertEqual(stackup.dielectrics.count, BoardStackup.dielectricCount(layers: layers))
            XCTAssertNil(BoardStackup.problem(copper: stackup.copper, dielectrics: stackup.dielectrics,
                                              temperatureRise: stackup.temperatureRise, layers: layers))
        }
        XCTAssertEqual(BoardStackup.standard(layers: 6).finish, .enig)
        XCTAssertEqual(BoardStackup.standard(layers: 4).finish, .leadFreeHasl)
    }

    func testStandardKeepsAGivenFinishAndRise() {
        let stackup = BoardStackup.standard(layers: 4, finish: .osp, temperatureRise: 25)
        XCTAssertEqual(stackup.finish, .osp)
        XCTAssertEqual(stackup.temperatureRise, 25)
    }

    func testLayerCountKeepsTheStackupInStep() {
        var board = BoardSettings()
        board.stackup.finish = .enig
        board.stackup.temperatureRise = 30
        board.layers = 4
        XCTAssertEqual(board.stackup.copper.count, 4)
        XCTAssertEqual(board.stackup.dielectrics.count, 3)
        XCTAssertEqual(board.stackup.finish, .enig)
        XCTAssertEqual(board.stackup.temperatureRise, 30)
        board.layers = 1
        XCTAssertEqual(board.stackup.dielectrics.count, 1)
        XCTAssertEqual(board.stackup.copper, [0.035])
    }

    func testProblemRefusesBadValues() {
        let two = BoardStackup.standard(layers: 2)
        func problem(copper: [Double]? = nil, dielectrics: [Dielectric]? = nil, rise: Double? = nil,
                     layers: Int = 2) -> String? {
            BoardStackup.problem(copper: copper ?? two.copper, dielectrics: dielectrics ?? two.dielectrics,
                                 temperatureRise: rise ?? two.temperatureRise, layers: layers)
        }
        XCTAssertNil(problem())
        XCTAssertNotNil(problem(copper: [0.035]), "wrong copper count")
        XCTAssertNotNil(problem(dielectrics: []), "wrong dielectric count")
        XCTAssertNotNil(problem(copper: [0.001, 0.035]), "thin copper")
        XCTAssertNotNil(problem(dielectrics: [Dielectric(kind: .core, thickness: 0.001, er: 4.5)]))
        XCTAssertNotNil(problem(dielectrics: [Dielectric(kind: .core, thickness: 1.5, er: 0.5)]))
        XCTAssertNotNil(problem(rise: 0))
        XCTAssertNotNil(problem(layers: 4))
    }
}
