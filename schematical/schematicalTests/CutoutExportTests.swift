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
}
