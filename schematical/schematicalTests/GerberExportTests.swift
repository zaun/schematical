import Foundation
import XCTest
@testable import schematical

/// File > Export > Gerber PCB: the layer files and their zip.
@MainActor
final class GerberExportTests: XCTestCase {
    private static var kept: [SchematicFileController] = []

    private let rules = DesignRules.pcbway

    private func file(_ files: [GerberExport.File], _ suffix: String) throws -> String {
        try XCTUnwrap(files.first { $0.name.hasSuffix(suffix) }, "no \(suffix)").text
    }

    /// Lines of the image (after the header), for counting operations.
    private func lines(_ text: String) -> [Substring] {
        text.split(separator: "\n")
    }

    func testTwoLayerBoardHasEveryFile() {
        let doc = TestSupport.newDocument()
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        XCTAssertEqual(files.map(\.name), ["b.GTL", "b.GBL", "b.GTS", "b.GBS", "b.GTP", "b.GBP", "b.GTO", "b.GBO", "b.GKO", "b-PTH.drl"])
        for file in files where !file.name.hasSuffix(".drl") {
            XCTAssertTrue(file.text.contains("%FSLAX46Y46*%"), file.name)
            XCTAssertTrue(file.text.contains("%MOMM*%"), file.name)
            XCTAssertTrue(file.text.hasSuffix("M02*\n"), file.name)
        }
    }

    func testFileFunctions() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        XCTAssertEqual(Array(files.map(\.name).prefix(4)), ["b.GTL", "b.G1", "b.G2", "b.GBL"])
        XCTAssertTrue(try file(files, ".GTL").contains("%TF.FileFunction,Copper,L1,Top*%"))
        XCTAssertTrue(try file(files, ".G1").contains("%TF.FileFunction,Copper,L2,Inr*%"))
        XCTAssertTrue(try file(files, ".GBL").contains("%TF.FileFunction,Copper,L4,Bot*%"))
        XCTAssertTrue(try file(files, ".GTS").contains("%TF.FilePolarity,Negative*%"))
        XCTAssertTrue(try file(files, ".GKO").contains("%TF.FileFunction,Profile,NP*%"))
    }

    func testOneLayerBoardHasNoBottomFiles() {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        let names = GerberExport.files(for: doc, base: "b", rules: rules).map(\.name)
        XCTAssertEqual(names, ["b.GTL", "b.GTS", "b.GTP", "b.GTO", "b.GKO", "b-PTH.drl"])
    }

    func testOutlineIsTheBoardRectangleYUp() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        let profile = try file(GerberExport.files(for: doc, base: "b", rules: rules), ".GKO")
        // Board (0, 0) is its top-left: Gerber (0, 30).
        XCTAssertTrue(profile.contains("X0Y30000000D02*"))
        XCTAssertTrue(profile.contains("X50000000Y30000000D01*"))
        XCTAssertTrue(profile.contains("X50000000Y0D01*"))
        XCTAssertTrue(profile.contains("%ADD10C,0.100000*%"))
    }

    func testTraceIsDrawnOnItsLayerOnly() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        _ = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10)], layer: .bottom))
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        let bottom = try file(files, ".GBL")
        // 10 mil = 0.254 mm.
        XCTAssertTrue(bottom.contains("C,0.254000*%"))
        XCTAssertTrue(bottom.contains("X10000000Y20000000D02*"))
        XCTAssertTrue(bottom.contains("X20000000Y20000000D01*"))
        XCTAssertFalse(try file(files, ".GTL").contains("D01*"))
    }

    func testThroughHolePartPadsOnBothSidesAndMask() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let placed = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
        let pads = placed.boardPads
        XCTAssertEqual(pads.count, 14)
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        for suffix in [".GTL", ".GBL", ".GTS", ".GBS"] {
            let text = lines(try file(files, suffix))
            // A flash, or an outline for an oval pad.
            XCTAssertEqual(text.filter { $0.hasSuffix("D03*") }.count + text.filter { $0 == "G36*" }.count, 14, suffix)
        }
        // Through-hole: no paste.
        XCTAssertFalse(try file(files, ".GTP").contains("D03*"))
        // The reference and pin-1 dot on the top silkscreen; the body
        // outline is on the Document layer by default, so not printed.
        let silk = try file(files, ".GTO")
        XCTAssertTrue(silk.contains("G36*"), "reference glyphs")
        XCTAssertTrue(silk.contains("C,0.400000*%"), "pin-1 dot")
        XCTAssertFalse(silk.contains("C,0.150000*%"), "body outline pen")
        XCTAssertFalse(try file(files, ".GBO").contains("G36*"))
    }

    func testSurfaceMountPadsOnTheirSideWithPaste() throws {
        let doc = TestSupport.newDocument()
        _ = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 200, y: 200))
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        func pads(_ suffix: String) throws -> Int {
            let text = try file(files, suffix)
            return lines(text).filter { $0.hasSuffix("D03*") }.count + lines(text).filter { $0 == "G36*" }.count
        }
        XCTAssertEqual(try pads(".GTL"), 14)
        XCTAssertEqual(try pads(".GTP"), 14)
        XCTAssertEqual(try pads(".GBL"), 0)
        XCTAssertEqual(try pads(".GBP"), 0)
    }

    func testMaskOpeningsGrowByTheExpansion() throws {
        let doc = TestSupport.newDocument()
        _ = try XCTUnwrap(doc.addVia(at: CGPoint(x: 5, y: 5)))
        let hole = doc.addHole(at: CGPoint(x: 20, y: 20))
        XCTAssertNotNil(doc.holes.first { $0.id == hole })
        var rules = rules
        rules.solderMaskExpansion = 0.1
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        let mask = try file(files, ".GTS")
        // The non-plated M3 hole (3.2 mm) opens 3.4 mm; the via is tented.
        XCTAssertTrue(mask.contains("C,3.400000*%"))
        XCTAssertEqual(lines(mask).filter { $0.hasSuffix("D03*") }.count, 1)
        // The via's copper on both sides; the bare hole has none.
        XCTAssertTrue(try file(files, ".GTL").contains("C,0.800000*%"))
        XCTAssertFalse(try file(files, ".GTL").contains("C,3.200000*%"))
    }

    func testPlaneIsTheBoardLessItsClearances() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        doc.setLayerNet(.inner(2), .ground)
        _ = try XCTUnwrap(doc.addVia(at: CGPoint(x: 5, y: 5)))
        let plane = try file(GerberExport.files(for: doc, base: "b", rules: rules), ".G1")
        let text = lines(plane)
        let region = try XCTUnwrap(text.firstIndex(of: "G36*"))
        let clear = try XCTUnwrap(text.firstIndex(of: "%LPC*%"))
        let dark = try XCTUnwrap(text.lastIndex(of: "%LPD*%"))
        XCTAssertLessThan(region, clear)
        XCTAssertLessThan(clear, dark)
        // The via (no net) is cut 0.8 + 2 × clearance wide.
        let cut = 0.8 + 2 * rules.minClearance
        XCTAssertTrue(plane.contains(String(format: "C,%.6f*%%", cut)), plane)
    }

    func testTextGlyphsHaveCountersCleared() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addText(at: CGPoint(x: 10, y: 10))
        doc.setText(id, "O")
        let silk = try file(GerberExport.files(for: doc, base: "b", rules: rules), ".GTO")
        let text = lines(silk)
        XCTAssertEqual(text.filter { $0 == "G36*" }.count, 2, "outer edge and counter")
        XCTAssertTrue(text.contains("%LPC*%"))
        XCTAssertEqual(text.last, "M02*")
    }

    func testZipListsEveryFile() throws {
        let doc = TestSupport.newDocument()
        let zip = GerberExport.zip(for: doc, base: "b", rules: rules)
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("gerber-\(UUID().uuidString).zip")
        try zip.write(to: url)
        defer { try? FileManager.default.removeItem(at: url) }
        let process = Process()
        process.executableURL = URL(fileURLWithPath: "/usr/bin/unzip")
        process.arguments = ["-Z1", url.path]
        let pipe = Pipe()
        process.standardOutput = pipe
        try process.run()
        process.waitUntilExit()
        XCTAssertEqual(process.terminationStatus, 0)
        let listed = String(decoding: pipe.fileHandleForReading.readDataToEndOfFile(), as: UTF8.self)
            .split(separator: "\n").map(String.init)
        XCTAssertEqual(listed, GerberExport.files(for: doc, base: "b", rules: rules).map(\.name))
    }

    func testDrillFilesSplitPlatedAndBareHoles() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        _ = try XCTUnwrap(doc.addVia(at: CGPoint(x: 5, y: 5)))
        let holeID = doc.addHole(at: CGPoint(x: 20, y: 20))
        let hole = try XCTUnwrap(doc.holes.first { $0.id == holeID }).centre
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        XCTAssertEqual(Array(files.map(\.name).suffix(2)), ["b-PTH.drl", "b-NPTH.drl"])
        let plated = try file(files, "-PTH.drl")
        XCTAssertTrue(plated.hasPrefix("M48\n"))
        XCTAssertTrue(plated.contains("; #@! TF.FileFunction,Plated,1,2,PTH"))
        XCTAssertTrue(plated.contains("METRIC"))
        XCTAssertTrue(plated.contains("T1C0.400\n"))
        // Board (5, 5) on a 30 mm board is y 25 up.
        XCTAssertTrue(plated.contains("\nX5.000Y25.000\n"))
        XCTAssertTrue(plated.hasSuffix("M30\n"))
        let bare = try file(files, "-NPTH.drl")
        XCTAssertTrue(bare.contains("; #@! TF.FileFunction,NonPlated,1,2,NPTH"))
        XCTAssertTrue(bare.contains("T1C3.200\n"))
        // Holes snap to the trace grid.
        let at = "X\(DrillExport.number(hole.x))Y\(DrillExport.number(30 - hole.y))"
        XCTAssertTrue(bare.contains("\n\(at)\n"), bare)
    }

    func testThroughHolePartHolesAreToolsBySize() throws {
        let doc = TestSupport.newDocument()
        _ = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let plated = try file(GerberExport.files(for: doc, base: "b", rules: rules), "-PTH.drl")
        let hits = lines(plated).filter { $0.hasPrefix("X") }
        XCTAssertEqual(hits.count, 14)
        let tools = lines(plated).filter { $0.hasPrefix("T") && $0.contains("C") }
        XCTAssertFalse(tools.isEmpty)
    }

    func testSlotIsRouted() {
        let slot = DrillExport.Hole(a: CGPoint(x: 1, y: 2), b: CGPoint(x: 3, y: 2), diameter: 1)
        let text = String(decoding: DrillExport.file([slot], plated: false, layers: 2, top: 10, software: "S"),
                          as: UTF8.self)
        XCTAssertTrue(text.contains("\nX1.000Y8.000G85X3.000Y8.000\n"), text)
        XCTAssertEqual(DrillExport.number(0.0005), "0.0005")
        XCTAssertEqual(DrillExport.number(1.2), "1.200")
    }

    func testCRC32() {
        XCTAssertEqual(ZipArchive.crc32(Data("123456789".utf8)), 0xCBF4_3926)
    }

    func testControllerWritesTheZipNamedAfterTheFile() throws {
        let doc = TestSupport.newDocument()
        let controller = SchematicFileController(document: doc, catalog: TestSupport.catalog, willReplaceDocument: {})
        Self.kept.append(controller)
        XCTAssertEqual(controller.gerberBaseName, "Untitled")
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("gerber-\(UUID().uuidString).zip")
        try controller.writeGerber(to: url)
        defer { try? FileManager.default.removeItem(at: url) }
        let data = try Data(contentsOf: url)
        XCTAssertEqual(Array(data.prefix(4)), [0x50, 0x4B, 0x03, 0x04])
        XCTAssertNil(controller.fileURL, "an export is not a save")
    }
}
