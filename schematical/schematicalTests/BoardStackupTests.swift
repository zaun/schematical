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

    // MARK: File

    private func json(_ doc: SchematicDocument) throws -> [String: Any] {
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        return try XCTUnwrap(JSONSerialization.jsonObject(with: data) as? [String: Any])
    }

    /// Opens `json` after `change` edits its `board.stackup` (nil removes it).
    private func open(_ json: [String: Any], _ change: (inout [String: Any]?) -> Void) throws -> SchematicDocument {
        var json = json
        var board = try XCTUnwrap(json["board"] as? [String: Any])
        var stackup = board["stackup"] as? [String: Any]
        change(&stackup)
        board["stackup"] = stackup
        json["board"] = board
        let doc = TestSupport.newDocument()
        try SchematicFile.decode(try JSONSerialization.data(withJSONObject: json))
            .apply(to: doc, catalog: TestSupport.catalog, fileURL: nil)
        return doc
    }

    func testSavedFileHasTheStackupAndOpensToTheSame() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        doc.setStackupFinish(.enig)
        doc.setStackupTemperatureRise(20)
        doc.setCopperThickness(0.07, at: 0)
        doc.setDielectric(at: 1, kind: .prepreg, thickness: 0.9, er: 4.0)
        let saved = try json(doc)
        let entry = try XCTUnwrap((saved["board"] as? [String: Any])?["stackup"] as? [String: Any])
        XCTAssertEqual(entry["finish"] as? String, "enig")
        XCTAssertEqual(entry["temperatureRise"] as? Double, 20)
        XCTAssertEqual((entry["copper"] as? [Double])?.first, 0.07)
        let opened = try open(saved) { _ in }
        XCTAssertEqual(opened.board.stackup, doc.board.stackup)
        XCTAssertEqual(opened.board.stackup.dielectrics[1], Dielectric(kind: .prepreg, thickness: 0.9, er: 4.0))
    }

    func testAFileWithNoStackupOpensToTheStandardOne() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(6)
        let opened = try open(try json(doc)) { $0 = nil }
        XCTAssertEqual(opened.board.stackup, BoardStackup.standard(layers: 6))
        // A missing key inside it is the standard value too.
        let partial = try open(try json(doc)) { $0 = ["finish": "osp"] }
        XCTAssertEqual(partial.board.stackup, BoardStackup.standard(layers: 6, finish: .osp))
    }

    func testBadStackupsAreRefused() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let saved = try json(doc)
        func refusal(_ change: @escaping (inout [String: Any]?) -> Void) -> String? {
            do { _ = try open(saved, change); return nil } catch { return error.localizedDescription }
        }
        let short = refusal { $0?["copper"] = [0.035, 0.035] }
        XCTAssertTrue(short?.contains("copper") == true && short?.contains("needs 4") == true, short ?? "no error")
        let finish = refusal { $0?["finish"] = "tin" }
        XCTAssertTrue(finish?.contains("finish") == true, finish ?? "no error")
        let kind = refusal { $0?["dielectrics"] = [["type": "foam", "thickness": 1.0, "er": 4.0],
                                                   ["type": "core", "thickness": 1.0, "er": 4.0],
                                                   ["type": "core", "thickness": 1.0, "er": 4.0]] }
        XCTAssertTrue(kind?.contains("foam") == true, kind ?? "no error")
        let thin = refusal { $0?["copper"] = [0.001, 0.0152, 0.0152, 0.035] }
        XCTAssertTrue(thin?.contains("0.005") == true, thin ?? "no error")
    }

    // MARK: Edits

    func testEachEditIsOneUndoStep() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        func step(_ name: String, _ edit: () -> Void) {
            let was = doc.board.stackup
            edit()
            XCTAssertNotEqual(doc.board.stackup, was, name)
            XCTAssertEqual(doc.undoName, name)
            XCTAssertTrue(doc.undo())
            XCTAssertEqual(doc.board.stackup, was, "undo \(name)")
        }
        step("Change Stackup") { doc.setStackupFinish(.osp) }
        step("Change Stackup") { doc.setStackupTemperatureRise(40) }
        step("Change Stackup") { doc.setCopperThickness(0.07, at: 3) }
        step("Change Stackup") { doc.setDielectric(at: 0, kind: .core, thickness: 0.3, er: 3.9) }
        doc.setStackupFinish(.enig)
        doc.setCopperThickness(0.05, at: 1)
        step("Reset Stackup") {
            doc.resetStackup()
            XCTAssertEqual(doc.board.stackup, BoardStackup.standard(layers: 4))
        }
    }

    func testAnEditToTheSameValueRecordsNoStep() {
        let doc = TestSupport.newDocument()
        let name = doc.undoName
        doc.setStackupFinish(doc.board.stackup.finish)
        doc.setCopperThickness(doc.board.stackup.copper[0], at: 0)
        doc.setDielectric(at: 0, er: doc.board.stackup.dielectrics[0].er)
        doc.setStackupTemperatureRise(doc.board.stackup.temperatureRise)
        doc.resetStackup()
        doc.setCopperThickness(0.07, at: 9)
        XCTAssertEqual(doc.undoName, name)
    }

    func testEditsClampToTheirRanges() {
        let doc = TestSupport.newDocument()
        doc.setCopperThickness(0, at: 0)
        XCTAssertEqual(doc.board.stackup.copper[0], BoardStackup.copperRange.lowerBound)
        doc.setCopperThickness(5, at: 0)
        XCTAssertEqual(doc.board.stackup.copper[0], BoardStackup.copperRange.upperBound)
        doc.setDielectric(at: 0, thickness: 0, er: 99)
        XCTAssertEqual(doc.board.stackup.dielectrics[0].thickness, BoardStackup.thicknessRange.lowerBound)
        XCTAssertEqual(doc.board.stackup.dielectrics[0].er, BoardStackup.erRange.upperBound)
        doc.setDielectric(at: 0, thickness: 50, er: 0)
        XCTAssertEqual(doc.board.stackup.dielectrics[0].thickness, BoardStackup.thicknessRange.upperBound)
        XCTAssertEqual(doc.board.stackup.dielectrics[0].er, BoardStackup.erRange.lowerBound)
        doc.setStackupTemperatureRise(0)
        XCTAssertEqual(doc.board.stackup.temperatureRise, BoardStackup.temperatureRiseRange.lowerBound)
        doc.setStackupTemperatureRise(500)
        XCTAssertEqual(doc.board.stackup.temperatureRise, BoardStackup.temperatureRiseRange.upperBound)
    }

    func testLayerCountChangeIsOneUndoStepAndRestoresTheStackup() {
        let doc = TestSupport.newDocument()
        doc.setStackupFinish(.osp)
        let two = doc.board.stackup
        doc.setLayerCount(4)
        XCTAssertEqual(doc.board.stackup, BoardStackup.standard(layers: 4, finish: .osp))
        XCTAssertTrue(doc.undo())
        XCTAssertEqual(doc.board.stackup, two)
        XCTAssertEqual(doc.board.layers, 2)
    }

    func testACardOnAOneLayerBoardGetsATwoLayerStackup() {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        XCTAssertEqual(doc.board.stackup.copper.count, 1)
        doc.setBoardType(.pci)
        XCTAssertEqual(doc.board.layers, 2)
        XCTAssertEqual(doc.board.stackup.copper.count, 2)
    }
}
