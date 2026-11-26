import Foundation
import XCTest
@testable import schematical

/// File > Export > Gerber PCB: the layer files and their zip.
@MainActor
final class GerberExportTests: XCTestCase {

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
        XCTAssertEqual(files.map(\.name), ["b.GTL", "b.GBL", "b.GTS", "b.GBS", "b.GTP", "b.GBP", "b.GTO", "b.GBO", "b.GKO", "b-PTH.drl", "b-job.gbrjob"])
        for file in files where !file.name.hasSuffix(".drl") && !file.name.hasSuffix(".gbrjob") {
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
        XCTAssertEqual(names, ["b.GTL", "b.GTS", "b.GTP", "b.GTO", "b.GKO", "b-PTH.drl", "b-job.gbrjob"])
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
        _ = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 10, y: 10), CGPoint(x: 20, y: 10)], layer: .bottom, width: 10))
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

    /// plan-54-G: the X and Y of a coordinate line (`X12Y34D01*`, `X12Y34I5J6D01*`), Gerber units (1e-6 mm).
    private func coordinate(_ line: Substring) -> (x: Int, y: Int)? {
        guard line.hasPrefix("X"), let y = line.firstIndex(of: "Y") else { return nil }
        let rest = line[line.index(after: y)...].prefix { $0 == "-" || $0.isNumber }
        guard let x = Int(line[line.index(after: line.startIndex)..<y]), let value = Int(rest) else { return nil }
        return (x, value)
    }

    /// The centres of what a layer file draws, as Gerber coordinates: each flash, and each region's box (a
    /// mask opening is a region, the pad grown and rounded).
    private func centres(_ text: String) -> [(x: Int, y: Int)] {
        var result: [(x: Int, y: Int)] = []
        var region: [(x: Int, y: Int)]?
        for line in lines(text) {
            if line == "G36*" { region = []; continue }
            if line == "G37*" {
                if let points = region, let first = points.first {
                    let xs = points.map(\.x) + [first.x], ys = points.map(\.y) + [first.y]
                    result.append(((xs.min()! + xs.max()!) / 2, (ys.min()! + ys.max()!) / 2))
                }
                region = nil
                continue
            }
            guard let point = coordinate(line) else { continue }
            if region != nil { region?.append(point) } else if line.hasSuffix("D03*") { result.append(point) }
        }
        return result
    }

    /// An opposite-side pad (plan-54-E) is in the other side's copper, mask and
    /// paste files than its part's, for a part on either side; the part's own
    /// pads are in its own side's.
    func testOppositePadsAreInTheOtherSidesFiles() throws {
        for side in [BoardSide.top, .bottom] {
            let (doc, id) = try OppositePadTests.fixture(side: side)
            let placed = try XCTUnwrap(doc.chips.first { $0.id == id })
            let height = doc.board.height
            func gerber(_ designator: String) throws -> (x: Int, y: Int) {
                let rect = try XCTUnwrap(placed.boardPads.first { $0.designator == designator }).rect
                return (Int((rect.midX * 1e6).rounded()), Int(((height - rect.midY) * 1e6).rounded()))
            }
            func has(_ list: [(x: Int, y: Int)], _ point: (x: Int, y: Int)) -> Bool {
                list.contains { abs($0.x - point.x) <= 2 && abs($0.y - point.y) <= 2 }
            }
            let one = try gerber("1"), two = try gerber("2"), three = try gerber("3")
            let files = GerberExport.files(for: doc, base: "b", rules: rules)
            let own = side == .top ? [".GTL", ".GTS", ".GTP"] : [".GBL", ".GBS", ".GBP"]
            let other = side == .top ? [".GBL", ".GBS", ".GBP"] : [".GTL", ".GTS", ".GTP"]
            for suffix in own {
                let list = centres(try file(files, suffix))
                XCTAssertTrue(has(list, one), "\(side) part: pad 1 in \(suffix)")
                XCTAssertFalse(has(list, two) || has(list, three), "\(side) part: an opposite pad in \(suffix)")
                // Pads 1 and 4 to 14.
                XCTAssertEqual(list.count, 12, "\(side) part: \(suffix)")
            }
            for suffix in other {
                let list = centres(try file(files, suffix))
                XCTAssertTrue(has(list, two) && has(list, three), "\(side) part: pads 2 and 3 in \(suffix)")
                XCTAssertFalse(has(list, one), "\(side) part: pad 1 in \(suffix)")
                XCTAssertEqual(list.count, 2, "\(side) part: \(suffix)")
            }
        }
    }

    func testMaskOpeningsGrowByTheExpansion() throws {
        let doc = TestSupport.newDocument()
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: 5, y: 5)))
        // A 0.4 / 0.8 mm via, whatever the Default class is.
        doc.setViaDiameter(via, 0.8)
        doc.setViaDrill(via, 0.4)
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
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: 5, y: 5)))
        // A 0.4 / 0.8 mm via, whatever the Default class is.
        doc.setViaDiameter(via, 0.8)
        doc.setViaDrill(via, 0.4)
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
        let via = try XCTUnwrap(doc.addVia(at: CGPoint(x: 5, y: 5)))
        // A 0.4 / 0.8 mm via, whatever the Default class is.
        doc.setViaDiameter(via, 0.8)
        doc.setViaDrill(via, 0.4)
        let holeID = doc.addHole(at: CGPoint(x: 20, y: 20))
        let hole = try XCTUnwrap(doc.holes.first { $0.id == holeID }).centre
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        XCTAssertEqual(Array(files.map(\.name).suffix(3)), ["b-PTH.drl", "b-NPTH.drl", "b-job.gbrjob"])
        let plated = try file(files, "-PTH.drl")
        XCTAssertTrue(plated.hasPrefix("M48\n"))
        XCTAssertTrue(plated.contains("; #@! TF.FileFunction,Plated,1,2,PTH"))
        XCTAssertTrue(plated.contains("METRIC"))
        XCTAssertTrue(plated.contains("T1C0.400\n"))
        XCTAssertTrue(plated.contains("; #@! TA.AperFunction,Plated,PTH,ViaDrill\nT1C0.400\n"))
        // Board (5, 5) on a 30 mm board is y 25 up.
        XCTAssertTrue(plated.contains("\nX5.000Y25.000\n"))
        XCTAssertTrue(plated.hasSuffix("M30\n"))
        let bare = try file(files, "-NPTH.drl")
        XCTAssertTrue(bare.contains("; #@! TF.FileFunction,NonPlated,1,2,NPTH"))
        XCTAssertTrue(bare.contains("T1C3.200\n"))
        XCTAssertTrue(bare.contains("; #@! TA.AperFunction,NonPlated,NPTH,MechanicalDrill\nT1C3.200\n"))
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

    func testPartHolesAreComponentDrills() throws {
        let doc = TestSupport.newDocument()
        _ = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let plated = try file(GerberExport.files(for: doc, base: "b", rules: rules), "-PTH.drl")
        let tools = lines(plated).filter { $0.hasPrefix("T") && $0.contains("C") && !$0.hasPrefix("T0") }
        XCTAssertFalse(tools.isEmpty)
        let functions = lines(plated).filter { $0.hasPrefix("; #@! TA.AperFunction") }
        XCTAssertEqual(functions.count, tools.count, "a function line for each tool")
        XCTAssertTrue(functions.allSatisfy { $0 == "; #@! TA.AperFunction,Plated,PTH,ComponentDrill" }, "\(functions)")
    }

    func testHeaderHolesAreComponentDrills() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.featherWing)
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        let plated = try file(files, "-PTH.drl")
        XCTAssertTrue(plated.contains("TA.AperFunction,Plated,PTH,ComponentDrill"))
        XCTAssertFalse(plated.contains("MechanicalDrill\nT"), "a header pad is a component hole")
        // The standard's mounting holes are the board's own.
        let bare = try file(files, "-NPTH.drl")
        XCTAssertTrue(bare.contains("TA.AperFunction,NonPlated,NPTH,MechanicalDrill"), bare)
    }

    /// Two functions with one diameter are two tools, the via's first.
    func testEqualDiametersOfDifferentFunctionsAreSeparateTools() throws {
        let doc = TestSupport.newDocument()
        _ = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let before = try file(GerberExport.files(for: doc, base: "b", rules: rules), "-PTH.drl")
        let tool = try XCTUnwrap(lines(before).first { $0.hasPrefix("T1C") })
        let diameter = try XCTUnwrap(Double(tool.dropFirst(3)))
        let id = try XCTUnwrap(doc.addVia(at: CGPoint(x: 5, y: 5)))
        doc.setViaDiameter(id, diameter + 0.5)
        doc.setViaDrill(id, diameter)
        let plated = try file(GerberExport.files(for: doc, base: "b", rules: rules), "-PTH.drl")
        XCTAssertTrue(plated.contains("; #@! TA.AperFunction,Plated,PTH,ViaDrill\nT1C\(tool.dropFirst(3))\n"
                                      + "; #@! TA.AperFunction,Plated,PTH,ComponentDrill\nT2C\(tool.dropFirst(3))\n"), plated)
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
        let document = TestSupport.systemDocument(of: doc)
        let controller = document.model.file
        XCTAssertEqual(controller.gerberBaseName, "Untitled")
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("gerber-\(UUID().uuidString).zip")
        try controller.writeGerber(to: url)
        defer { try? FileManager.default.removeItem(at: url) }
        let data = try Data(contentsOf: url)
        XCTAssertEqual(Array(data.prefix(4)), [0x50, 0x4B, 0x03, 0x04])
        XCTAssertNil(document.fileURL, "an export is not a save")
        XCTAssertNil(controller.fileURL)
        // The exporter names its files after the document's file.
        // The model's URL, which names the exports. Not the system document's:
        // AppKit would then autosave this edited document to a file that is not there.
        document.model.fileURL = URL(fileURLWithPath: "/tmp/My Board.schematical")
        XCTAssertEqual(controller.gerberBaseName, "My_Board")
    }

    /// The centre of the bounding box of every filled region (glyph) in a
    /// silkscreen file, in Gerber mm (y up).
    private func regionCentre(_ silk: String) -> CGPoint? {
        var points: [CGPoint] = []
        var inRegion = false
        for line in lines(silk) {
            if line == "G36*" { inRegion = true; continue }
            if line == "G37*" { inRegion = false; continue }
            guard inRegion, line.hasPrefix("X"), let y = line.firstIndex(of: "Y") else { continue }
            let x = Double(line[line.index(after: line.startIndex)..<y]) ?? 0
            let end = line.firstIndex(where: { $0 == "D" }) ?? line.endIndex
            let yv = Double(line[line.index(after: y)..<end]) ?? 0
            points.append(CGPoint(x: x / 1e6, y: yv / 1e6))
        }
        guard let first = points.first else { return nil }
        let box = points.reduce(CGRect(origin: first, size: .zero)) { $0.union(CGRect(origin: $1, size: .zero)) }
        return CGPoint(x: box.midX, y: box.midY)
    }

    /// plan-41-A: the reference's glyphs are drawn at the reference's
    /// position, not the body centre.
    func testMovedReferenceGlyphsAreCentredWhereItIs() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        doc.setBoardSize(width: 100, height: 80)
        doc.moveFootprint(chip.id, to: CGPoint(x: 40, y: 30))
        let before = try XCTUnwrap(regionCentre(try file(GerberExport.files(for: doc, base: "b", rules: rules), ".GTO")))
        let from = try XCTUnwrap(doc.chips.first?.boardReferenceCentre)
        doc.moveReference(chip.id, toCentre: CGPoint(x: from.x + 7.5, y: from.y - 11))
        let to = try XCTUnwrap(doc.chips.first?.boardReferenceCentre)
        let after = try XCTUnwrap(regionCentre(try file(GerberExport.files(for: doc, base: "b", rules: rules), ".GTO")))
        // Gerber's y is up, so a move up the board is a larger y.
        XCTAssertEqual(after.x - before.x, to.x - from.x, accuracy: 1e-4)
        XCTAssertEqual(after.y - before.y, -(to.y - from.y), accuracy: 1e-4)
        XCTAssertNotEqual(to.x, from.x)
    }

    // MARK: Via tenting (plan-50-C)

    /// The (x, y) of each flash of a circle `diameter` mm wide in `text`.
    private func flashes(ofDiameter diameter: Double, in text: String) -> [String] {
        let wanted = "C,\(GerberWriter.mm(GerberWriter.nm(diameter)))*%"
        var code: String?
        var current: String?
        var hits: [String] = []
        for line in text.split(separator: "\n") {
            if line.hasPrefix("%ADD"), line.hasSuffix(wanted) { code = String(line.dropFirst(4).prefix { $0.isNumber }) }
            else if line.hasPrefix("D"), line.hasSuffix("*"), !line.hasSuffix("D03*"), !line.hasSuffix("D02*"),
                    !line.hasSuffix("D01*") { current = String(line.dropFirst().dropLast()) }
            else if line.hasSuffix("D03*"), current == code, code != nil { hits.append(String(line.dropLast(4))) }
        }
        return hits
    }

    func testOpenViasOpenTheirMaskOnThatSideOnly() throws {
        let doc = try ExportSamples.vias()
        XCTAssertEqual(doc.vias.map { [$0.tentedTop, $0.tentedBottom] }, [[true, true], [false, true], [true, false], [false, false]])
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        let opening = 0.77 + 2 * rules.solderMaskExpansion
        let top = flashes(ofDiameter: opening, in: try file(files, ".GTS"))
        let bottom = flashes(ofDiameter: opening, in: try file(files, ".GBS"))
        XCTAssertEqual(top.count, 2, "vias 2 and 4 are open on Top")
        XCTAssertEqual(bottom.count, 2, "vias 3 and 4 are open on Bottom")
        // The openings are at the vias' centres (Gerber y is up from the board's bottom edge).
        let topEdge = Double(doc.board.outline.map(\.y).max() ?? 0)
        func at(_ via: BoardVia) -> String {
            "X\(GerberWriter.nm(Double(via.centre.x)))Y\(GerberWriter.nm(topEdge - Double(via.centre.y)))"
        }
        XCTAssertEqual(Set(top), [at(doc.vias[1]), at(doc.vias[3])])
        XCTAssertEqual(Set(bottom), [at(doc.vias[2]), at(doc.vias[3])])
    }

    func testTentedViasAddNothingToTheMask() throws {
        let tented = GerberExport.files(for: try ExportSamples.vias([(true, true), (true, true), (true, true), (true, true)]),
                                        base: "b", rules: rules)
        let none = GerberExport.files(for: try ExportSamples.vias([]), base: "b", rules: rules)
        XCTAssertEqual(try file(tented, ".GTS"), try file(none, ".GTS"))
        XCTAssertEqual(try file(tented, ".GBS"), try file(none, ".GBS"))
    }

    // MARK: plan-68-Y

    /// A board point as the Gerber files write it: nm, y up from the bottom edge.
    private func gerber(_ point: CGPoint, height: Double) -> String {
        "X\(Int((point.x * 1e6).rounded()))Y\(Int(((height - point.y) * 1e6).rounded()))"
    }

    func testBodyOutlinesOnTheSilkscreenAreDrawnWithTheDiodesBand() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let diode = try TestSupport.place(doc, "1N4001-E3/54", at: CGPoint(x: 600, y: 200))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 600, y: 600))
        let before = try file(GerberExport.files(for: doc, base: "b", rules: rules), ".GTO")
        XCTAssertFalse(before.contains("C,0.150000*%"), "outlines are on the Document layer by default")
        doc.setFootprintMarks([chip.id, diode.id, led.id], outline: .silkscreen)
        let silk = try file(GerberExport.files(for: doc, base: "b", rules: rules), ".GTO")
        XCTAssertTrue(silk.contains("C,0.150000*%"), "the body outline pen")
        let height = doc.board.height
        let placed = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
        let body = try XCTUnwrap(placed.boardBody)
        XCTAssertFalse(body.round)
        XCTAssertTrue(silk.contains(gerber(CGPoint(x: body.rect.minX, y: body.rect.minY), height: height) + "D02*"), "the rectangle starts at a corner")
        XCTAssertTrue(silk.contains(gerber(CGPoint(x: body.rect.maxX, y: body.rect.maxY), height: height) + "D01*"))
        let band = try XCTUnwrap(doc.chips.first { $0.id == diode.id }?.cathodeBand)
        XCTAssertTrue(silk.contains(gerber(band.a, height: height) + "D02*\n" + gerber(band.b, height: height) + "D01*"), "the band")
        let round = try XCTUnwrap(doc.chips.first { $0.id == led.id }?.boardBody)
        XCTAssertTrue(round.round, "the LED's body is round")
        let strokes = silk.split(separator: "\n").filter { $0.hasSuffix("D01*") }.count
        XCTAssertGreaterThan(strokes, 20, "the round body is a polyline of many steps")
    }

    func testAPadThatIsNotACircleOrPlainRectangleIsItsOutlineAndASlotIsACapsule() throws {
        let oval = PlacedPad(designator: "1", rect: CGRect(x: 0, y: 0, width: 2, height: 1), drill: 0.6, shape: .round,
                             radius: 0, chamfer: 0, corners: [])
        guard case let .polygon(points) = GerberExport.shape(of: oval) else { return XCTFail("an oval is an outline") }
        XCTAssertGreaterThan(points.count, 8)
        XCTAssertEqual(Double(points.map(\.x).max()!), 2, accuracy: 0.01)
        XCTAssertEqual(Double(points.map(\.y).min()!), 0, accuracy: 0.01)
        let rounded = PlacedPad(designator: "2", rect: CGRect(x: 0, y: 0, width: 2, height: 2), drill: 0, shape: .rect,
                                radius: 0.3, chamfer: 0, corners: [])
        guard case .polygon = GerberExport.shape(of: rounded) else { return XCTFail("a rounded rectangle is an outline") }

        var slotted = PlacedPad(designator: "3", rect: CGRect(x: 0, y: 0, width: 3, height: 1.5), drill: 0.8, shape: .round,
                                radius: 0, chamfer: 0, corners: [])
        slotted.slotLength = 2
        guard case let .capsule(a, b, diameter) = GerberExport.holeShape(of: slotted) else { return XCTFail("a slot is a capsule") }
        XCTAssertEqual(diameter, 0.8, accuracy: 1e-9)
        XCTAssertEqual(Double(b.x - a.x), 2 - 0.8, accuracy: 1e-9)
        XCTAssertEqual(a.y, b.y)
    }
}
