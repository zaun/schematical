import CoreGraphics
import Foundation
import PDFKit
import XCTest
@testable import schematical

/// Cutouts in the exports (plan-65-G): Gerber, PDF and print, KiCad, EasyEDA Pro.
@MainActor
final class CutoutExportTests: XCTestCase {
    private func files(_ doc: SchematicDocument) -> [GerberExport.File] {
        GerberExport.files(for: doc, base: "b", rules: doc.board.rules)
    }

    private func count(_ text: String, _ needle: String) -> Int { text.components(separatedBy: needle).count - 1 }

    // MARK: Gerber and print

    func testTheProfileHasTheOutlineAndEveryCutoutAsClosedLoops() throws {
        let doc = try ExportSamples.cutouts()
        let profile = try XCTUnwrap(files(doc).first { $0.name == "b.GKO" }).text
        XCTAssertEqual(count(profile, "D02*"), 1 + 3, "the outline and three cutouts: one move each")
        // Each run of draws ends where it began.
        var runs: [[String]] = []
        for line in profile.split(separator: "\n") where line.hasSuffix("D02*") || line.hasSuffix("D01*") {
            let coordinates = String(line.dropLast(4))
            if line.hasSuffix("D02*") { runs.append([coordinates]) } else { runs[runs.count - 1].append(coordinates) }
        }
        XCTAssertEqual(runs.count, 4)
        for run in runs { XCTAssertEqual(run.first, run.last, "the loop closes") }
    }

    func testAPlaneClearsEveryCutoutAndNothingElseMoves() throws {
        let with = try ExportSamples.cutouts()
        let without = try ExportSamples.fourLayer()
        let a = files(with), b = files(without)
        XCTAssertEqual(a.map(\.name), b.map(\.name))
        var differing: [String] = []
        for (x, y) in zip(a, b) where x.data != y.data { differing.append(x.name) }
        // The profile, and each inner Plane's file (an inner layer is `.G1`, `.G2`), and nothing else.
        func isInner(_ name: String) -> Bool { name.range(of: #"\.G[0-9]$"#, options: .regularExpression) != nil }
        XCTAssertTrue(differing.contains("b.GKO"))
        XCTAssertEqual(differing.filter(isInner).count, 2, "both Planes: \(differing)")
        for name in differing { XCTAssertTrue(name == "b.GKO" || isInner(name), "\(name) changed") }
        let plane = try XCTUnwrap(a.first { differing.contains($0.name) && $0.name != "b.GKO" })
        let bare = try XCTUnwrap(b.first { $0.name == plane.name })
        XCTAssertEqual(count(plane.text, "G36*") - count(bare.text, "G36*"), 3, "a clear region for each cutout")
        XCTAssertEqual(count(plane.text, "D02*") - count(bare.text, "D02*"), 6, "a region and a band stroke for each")
        XCTAssertEqual(try XCTUnwrap(a.first { $0.name.hasSuffix(".gbrjob") }).data,
                       try XCTUnwrap(b.first { $0.name.hasSuffix(".gbrjob") }).data, "the job file's size is the outline's")
    }

    func testTheBoardPageStrokesEveryLoop() throws {
        let doc = try ExportSamples.cutouts()
        let artwork = PrintExport.artwork(for: doc)
        XCTAssertEqual(artwork.cutouts.count, 3)
        XCTAssertEqual(artwork.outline.count, 4)
        var options = PrintOptions()
        options.paper = .a4
        options.marks = false
        options.hidden = [.schematic, .copper(.top), .copper(.bottom), .copper(.inner(2)), .copper(.inner(3)),
                          .parts(.top), .parts(.bottom), .silkscreen(.top), .silkscreen(.bottom)]
        let data = try XCTUnwrap(PrintExport.pdf(for: doc, name: "t", options))
        XCTAssertEqual(PDFDocument(data: data)?.pageCount, 1)
        XCTAssertTrue(PrintExport.artwork(for: try ExportSamples.fourLayer()).cutouts.isEmpty)
    }

    // MARK: KiCad and EasyEDA Pro

    private func kicad(_ doc: SchematicDocument) throws -> String {
        let files = KiCadExport.files(for: doc, base: "c")
        return try XCTUnwrap(files.first { $0.name.hasSuffix(".kicad_pcb") }).text
    }

    func testKiCadHasEveryLoopOnEdgeCuts() throws {
        let doc = try ExportSamples.cutouts()
        let text = try kicad(doc)
        let edgeLines = text.components(separatedBy: "\n\t(gr_line").dropFirst().filter { $0.contains("(layer \"Edge.Cuts\")") }
        let expected = doc.boardEdge.segments.count
        XCTAssertEqual(edgeLines.count, expected, "the outline's and every cutout's segments")
        func uuid(_ chunk: String) -> String? { chunk.components(separatedBy: "(uuid ").dropFirst().first?.components(separatedBy: ")").first }
        let uuids = edgeLines.compactMap(uuid)
        XCTAssertEqual(uuids.count, edgeLines.count)
        XCTAssertEqual(Set(uuids).count, uuids.count, "unique")
        // The outline's own segments (uuid and start/end) are as without cutouts.
        let bare = try kicad(try ExportSamples.fourLayer())
        let bareEdge = bare.components(separatedBy: "\n\t(gr_line").dropFirst().filter { $0.contains("Edge.Cuts") }
        XCTAssertEqual(bareEdge.count, 4)
        for line in bareEdge {
            let head = line.components(separatedBy: "(uuid ")[0]
            XCTAssertTrue(text.contains(head + "(uuid " + (uuid(line) ?? "?")), "an outline segment is as without cutouts")
        }
    }

    func testAnUnchangedBoardsKiCadFileIsUnchanged() throws {
        // The plan-65 change must not touch a board with no cutouts: its Edge.Cuts is the four outline lines.
        let text = try kicad(try ExportSamples.fourLayer())
        XCTAssertEqual(text.components(separatedBy: "(layer \"Edge.Cuts\")").count - 1, 4)
        XCTAssertFalse(text.contains("cutout/"))
    }

    func testEasyEDAHasAClosedOutlinePolyForEachCutoutAfterTheOutlines() throws {
        let doc = try ExportSamples.cutouts()
        let entries = EasyEDAProExport.entries(for: doc, base: "c")
        let board = String(decoding: try XCTUnwrap(entries.first { $0.name.hasSuffix(".epcb") }).data, as: UTF8.self)
        let polys = try board.split(separator: "\n").compactMap { line -> [Any]? in
            let value = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(line.utf8)) as? [Any])
            return value.first as? String == "POLY" && (value[4] as? Int) == 11 ? value : nil
        }
        XCTAssertEqual(polys.count, 1 + 3)
        for poly in polys {
            let points = try XCTUnwrap(poly[6] as? [Any])
            XCTAssertEqual(points.count % 2, 1, "x, y pairs after the L: \(points.count)")
        }
        let bare = EasyEDAProExport.entries(for: try ExportSamples.fourLayer(), base: "c")
        let bareBoard = String(decoding: try XCTUnwrap(bare.first { $0.name.hasSuffix(".epcb") }).data, as: UTF8.self)
        XCTAssertEqual(bareBoard.components(separatedBy: "\"POLY\"").count - 1, 1)
    }

    /// Prints the cutouts board's KiCad, Gerber and EasyEDA Pro files as base64 lines when `KICAD_OUT` is set
    /// (`TEST_RUNNER_KICAD_OUT=1` through xcodebuild), for `kicad-cli` and KiCad's importer to read: the test host is
    /// sandboxed, so this is how the files leave it (`KiCadExportTests.testPrintSamplesWhenAsked`).
    func testPrintCutoutSamplesWhenAsked() throws {
        guard ProcessInfo.processInfo.environment["KICAD_OUT"] != nil else { return }
        let doc = try ExportSamples.cutouts()
        for file in KiCadExport.files(for: doc, base: "cutouts") {
            print("KICAD_FILE cutouts/\(file.name) \(file.data.base64EncodedString())")
        }
        for file in GerberExport.files(for: doc, base: "cutouts") {
            print("GERBER_FILE cutouts/\(file.name) \(file.data.base64EncodedString())")
        }
        // The same board without the cutouts, to tell the sample's own findings from the cutouts'.
        for file in KiCadExport.files(for: try ExportSamples.fourLayer(), base: "bare") where file.name.hasSuffix("kicad_pcb") {
            print("KICAD_FILE bare/\(file.name) \(file.data.base64EncodedString())")
        }
        print("EPRO_FILE cutouts/cutouts.epro \(EasyEDAProExport.data(for: doc, base: "cutouts").base64EncodedString())")
    }
}
