import Foundation
import XCTest
@testable import schematical

/// plan-50-E: the Gerber job file (`<base>-job.gbrjob`).
@MainActor
final class GerberJobTests: XCTestCase {
    private let rules = DesignRules.pcbway

    private func job(_ doc: SchematicDocument) throws -> (json: [String: Any], files: [GerberExport.File]) {
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        let file = try XCTUnwrap(files.first { $0.name == "b-job.gbrjob" })
        return (try XCTUnwrap(JSONSerialization.jsonObject(with: file.data) as? [String: Any]), files)
    }

    private func specs(_ json: [String: Any]) throws -> [String: Any] { try XCTUnwrap(json["GeneralSpecs"] as? [String: Any]) }
    private func stack(_ json: [String: Any]) throws -> [[String: Any]] { try XCTUnwrap(json["MaterialStackup"] as? [[String: Any]]) }

    func testThicknessIsTheStandardStackupsForEveryLayerCount() throws {
        for (layers, thickness) in [(1, 1.6), (2, 1.6), (4, 1.6062), (6, 1.566)] {
            let doc = TestSupport.newDocument()
            doc.setLayerCount(layers)
            let specs = try specs(try job(doc).json)
            XCTAssertEqual(specs["LayerNumber"] as? Int, layers)
            XCTAssertEqual(try XCTUnwrap(specs["BoardThickness"] as? Double), thickness, accuracy: 1e-9, "\(layers) layers")
        }
    }

    func testStackupCopperAndDielectricsInOrder() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let stack = try stack(try job(doc).json)
        XCTAssertEqual(stack.compactMap { $0["Type"] as? String }, [
            "Legend", "SolderPaste", "SolderMask", "Copper", "Dielectric", "Copper", "Dielectric", "Copper", "Dielectric",
            "Copper", "SolderMask", "SolderPaste", "Legend",
        ])
        XCTAssertEqual(stack.filter { $0["Type"] as? String == "Copper" }.compactMap { $0["Thickness"] as? Double },
                       doc.board.stackup.copper)
        XCTAssertEqual(stack.filter { $0["Type"] as? String == "Dielectric" }.compactMap { $0["Thickness"] as? Double },
                       doc.board.stackup.dielectrics.map(\.thickness))
        XCTAssertEqual(stack.filter { $0["Type"] as? String == "Dielectric" }.compactMap { $0["DielectricConstant"] as? Double },
                       doc.board.stackup.dielectrics.map(\.er))
        XCTAssertEqual(stack.first { $0["Type"] as? String == "SolderMask" }?["Color"] as? String, "Green")
    }

    func testEditedStackupShowsInTheJob() throws {
        let doc = TestSupport.newDocument()
        doc.setStackupFinish(.enig)
        doc.setCopperThickness(0.07, at: 0)
        doc.setBoardColor(.black)
        let json = try job(doc).json
        XCTAssertEqual(try specs(json)["Finish"] as? String, "ENIG")
        let copper = try stack(json).first { $0["Type"] as? String == "Copper" }
        XCTAssertEqual(copper?["Thickness"] as? Double, 0.07)
        XCTAssertEqual(try stack(json).first { $0["Type"] as? String == "SolderMask" }?["Color"] as? String, "Black")
        XCTAssertEqual(try specs(json)["BoardThickness"] as? Double, doc.board.stackup.thickness)
    }

    func testFilesAttributesListEveryOtherFile() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        _ = doc.addHole(at: CGPoint(x: 20, y: 20))
        let (json, files) = try job(doc)
        let listed = try XCTUnwrap(json["FilesAttributes"] as? [[String: Any]])
        XCTAssertEqual(listed.compactMap { $0["Path"] as? String }, files.map(\.name).filter { $0 != "b-job.gbrjob" })
        let byPath = Dictionary(uniqueKeysWithValues: listed.map { ($0["Path"] as! String, $0) })
        XCTAssertEqual(byPath["b.GTL"]?["FileFunction"] as? String, "Copper,L1,Top")
        XCTAssertEqual(byPath["b.GBL"]?["FileFunction"] as? String, "Copper,L4,Bot")
        XCTAssertEqual(byPath["b.GTS"]?["FilePolarity"] as? String, "Negative")
        XCTAssertEqual(byPath["b.GKO"]?["FileFunction"] as? String, "Profile,NP")
        XCTAssertEqual(byPath["b-PTH.drl"]?["FileFunction"] as? String, "Plated,1,4,PTH")
        XCTAssertNil(byPath["b-PTH.drl"]?["FilePolarity"])
        // Each function is the string in the file itself.
        for file in files where file.function != nil && !file.name.hasSuffix(".drl") {
            XCTAssertTrue(file.text.contains("%TF.FileFunction,\(file.function!)*%"), file.name)
        }
    }

    func testDesignRulesAndProject() throws {
        let doc = TestSupport.newDocument()
        let two = try job(doc).json
        let outer = try XCTUnwrap((two["DesignRules"] as? [[String: Any]])?.first)
        XCTAssertEqual(outer["Layers"] as? String, "Outer")
        XCTAssertEqual(outer["TrackToTrack"] as? Double, rules.minClearance)
        XCTAssertEqual(outer["MinLineWidth"] as? Double, rules.minTraceWidth)
        XCTAssertEqual(outer["MinRing"] as? Double, rules.minAnnularRing)
        XCTAssertEqual((two["DesignRules"] as? [[String: Any]])?.count, 1)
        doc.setLayerCount(4)
        XCTAssertEqual((try job(doc).json["DesignRules"] as? [[String: Any]])?.compactMap { $0["Layers"] as? String },
                       ["Outer", "Inner"])
        let project = try XCTUnwrap(try specs(two)["ProjectId"] as? [String: Any])
        XCTAssertEqual(project["Name"] as? String, "b")
        XCTAssertEqual(project["GUID"] as? String, KiCad.uuid("gerber/b"))
        let size = try XCTUnwrap(try specs(two)["Size"] as? [String: Double])
        XCTAssertEqual(size["X"], 100)
        XCTAssertEqual(size["Y"], 80)
    }

    func testOneLayerBoardHasNoBottomGroup() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        let stack = try stack(try job(doc).json)
        XCTAssertEqual(stack.compactMap { $0["Type"] as? String },
                       ["Legend", "SolderPaste", "SolderMask", "Copper", "Dielectric"])
        XCTAssertEqual(stack.last?["Name"] as? String, "Substrate")
    }

    func testTheJobIsDeterministic() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let first = try job(doc).files.first { $0.name == "b-job.gbrjob" }
        let second = try job(doc).files.first { $0.name == "b-job.gbrjob" }
        XCTAssertEqual(first, second)
    }
}
