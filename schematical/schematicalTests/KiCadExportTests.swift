import Foundation
import AppKit
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// File › Export › KiCad Project…: the three files
/// (`KiCadExport`). The files are checked for shape here; that KiCad itself
/// reads them and finds the schematic's nets on the board is checked by
/// running `kicad-cli` over the output (see `docs/export.md` § KiCad).
@MainActor
final class KiCadExportTests: XCTestCase {
    private func files(_ name: String) throws -> (doc: SchematicDocument, files: [KiCadExport.File]) {
        let doc = try Fixture.open(name)
        return (doc, KiCadExport.files(for: doc, base: name))
    }

    private func file(_ files: [KiCadExport.File], _ ext: String) throws -> String {
        try XCTUnwrap(files.first { $0.name.hasSuffix(".\(ext)") }, "no .\(ext)").text
    }

    /// Whether the parentheses of an S-expression balance, outside strings.
    private func balanced(_ text: String) -> Bool {
        var depth = 0
        var inString = false
        var escaped = false
        for character in text {
            if inString {
                if escaped { escaped = false } else if character == "\\" { escaped = true } else if character == "\"" { inString = false }
                continue
            }
            switch character {
            case "\"": inString = true
            case "(": depth += 1
            case ")":
                depth -= 1
                if depth < 0 { return false }
            default: break
            }
        }
        return depth == 0 && !inString
    }

    private func count(_ needle: String, in text: String) -> Int {
        text.components(separatedBy: needle).count - 1
    }

    /// plan-63-G: a library pin's type is the ERC pin type (`erc_pin_type`), so KiCad's own
    /// ERC on the export sees what the app's sees.
    func testLibraryPinsCarryTheErcPinTypes() throws {
        func schematic(placing name: String) throws -> String {
            let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
            try TestSupport.place(doc, name, at: CGPoint(x: 200, y: 200))
            return try file(KiCadExport.files(for: doc, base: "t"), "kicad_sch")
        }
        let buffer = try schematic(placing: "SN74HC244N")
        XCTAssertEqual(count("(pin tri_state line", in: buffer), 8, "the eight Y pins")
        XCTAssertEqual(count("(pin input line", in: buffer), 10, "eight A pins and two OE pins")
        XCTAssertEqual(count("(pin power_in line", in: buffer), 2)
        XCTAssertEqual(count("(pin bidirectional line", in: buffer), 0)
        XCTAssertTrue(try schematic(placing: "SN74LS07N").contains("(pin open_collector line"))
        XCTAssertTrue(try schematic(placing: "GENERIC-RESISTOR").contains("(pin passive line"))
        XCTAssertFalse(try schematic(placing: "GENERIC-RESISTOR").contains("(pin bidirectional line"))
        XCTAssertTrue(try schematic(placing: "GENERIC-HEADER-1X02").contains("(pin passive line"))
        XCTAssertTrue(try schematic(placing: "ATMEGA328P-PU").contains("(pin bidirectional line"))
        XCTAssertTrue(try schematic(placing: "LM7805MPX/NOPB").contains("(pin power_out line"))
        // Without the library, the pin's role: an output is an output.
        let bare = TestSupport.newDocument()
        try TestSupport.place(bare, "SN74HC244N", at: CGPoint(x: 200, y: 200))
        let plain = try file(KiCadExport.files(for: bare, base: "t"), "kicad_sch")
        XCTAssertEqual(count("(pin output line", in: plain), 8)
    }

    /// plan-63-A: a Power Flag net label is a `PWR_FLAG` power symbol with a `power_out` pin.
    func testAPowerFlagExportsAsPWR_FLAG() throws {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let flag = doc.addNetLabel(at: CGPoint(x: 300, y: 100), style: .powerFlag)
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(one, "K")), .label(flag)))
        let plain = try file(KiCadExport.files(for: TestSupport.newDocument(), base: "p"), "kicad_sch")
        XCTAssertEqual(count("lib_id \"Schematical:PWR_FLAG\"", in: plain), 0)

        let text = try file(KiCadExport.files(for: doc, base: "flag"), "kicad_sch")
        XCTAssertTrue(balanced(text))
        XCTAssertEqual(count("(lib_id \"Schematical:PWR_FLAG\")", in: text), 1)
        XCTAssertEqual(count("(pin power_out line", in: text), 1)
        XCTAssertEqual(count("(reference \"#FLG01\")", in: text), 1)
        // Not a label and not a No Connect.
        XCTAssertEqual(count("(no_connect", in: text), 0)
        XCTAssertEqual(count("(label ", in: text), 0)
    }

    func testThreeFilesNamedAfterTheProject() throws {
        let (_, files) = try files("AFU")
        XCTAssertEqual(files.map(\.name), ["AFU.kicad_pro", "AFU.kicad_sch", "AFU.kicad_pcb"])
        let project = try JSONSerialization.jsonObject(with: XCTUnwrap(files.first).data) as? [String: Any]
        XCTAssertEqual((project?["meta"] as? [String: Any])?["filename"] as? String, "AFU.kicad_pro")
    }

    func testFilesAreWellFormed() throws {
        for name in Fixture.names {
            let (_, files) = try files(name)
            XCTAssertTrue(balanced(try file(files, "kicad_sch")), "\(name) schematic")
            XCTAssertTrue(balanced(try file(files, "kicad_pcb")), "\(name) board")
        }
    }

    /// plan-57-B: a test point (one pin, a bare pad) exports as a schematic
    /// symbol and a one-pad board footprint, both well formed.
    func testATestPointExports() throws {
        let doc = TestSupport.newDocument()
        let tp = try TestSupport.place(doc, "GENERIC-TESTPOINT", at: CGPoint(x: 100, y: 100))
        let other = try TestSupport.place(doc, "GENERIC-TESTPOINT", at: CGPoint(x: 300, y: 100))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(tp, "TP")), .pin(try TestSupport.pin(other, "TP"))))
        let files = KiCadExport.files(for: doc, base: "tp")
        let schematic = try file(files, "kicad_sch")
        let board = try file(files, "kicad_pcb")
        XCTAssertTrue(balanced(schematic))
        XCTAssertTrue(balanced(board))
        XCTAssertTrue(schematic.contains("TP1"))
        XCTAssertTrue(board.contains("TP1"))
        XCTAssertEqual(count("(footprint ", in: board), 2)
        XCTAssertEqual(count("(pad ", in: board), 2)
    }

    /// plan-64-D: the Document Info goes into KiCad's own title block, and the
    /// title block on the schematic is lines and text; a document with no
    /// information exports as it did.
    func testDocumentInfoAndTitleBlock() throws {
        let doc = try ExportSamples.titled()
        let sch = try file(KiCadExport.files(for: doc, base: "tb"), "kicad_sch")
        XCTAssertTrue(balanced(sch))
        for field in ["(title \"Clock board\")", "(rev \"B\")", "(company \"J. Smith\")",
                      "(comment 1 \"Divides the 4 MHz clock by 8. Drives the counter.\")",
                      "(date \"\(String(DocumentInfo.dateText(doc.documentInfo.updated).prefix(10)))\")"] {
            XCTAssertTrue(sch.contains(field), field)
        }
        for words in ["Clock board", "J. Smith", "Author", "Revision", "Description", "Divides the 4 MHz clock by 8."] {
            XCTAssertTrue(sch.contains("(text \"\(words)\""), words)
        }
        // The frame, and the lines inside it: 3 between rows and 2 between cells.
        XCTAssertEqual(count("(polyline", in: sch), 1 + 5)
        let again = try file(KiCadExport.files(for: doc, base: "tb"), "kicad_sch")
        XCTAssertEqual(sch, again, "the same bytes twice")

        let plain = TestSupport.newDocument()
        let bare = try file(KiCadExport.files(for: plain, base: "tb"), "kicad_sch")
        XCTAssertTrue(bare.contains("(title \"tb\")"))
        for absent in ["(rev ", "(company ", "(comment ", "(date ", "(polyline"] {
            XCTAssertFalse(bare.contains(absent), absent)
        }
    }

    /// Hidden, the block is not drawn, but the information still goes to KiCad.
    func testHiddenTitleBlockIsNotDrawnButTheInfoIsKept() throws {
        let doc = try ExportSamples.titled()
        doc.setTitleBlockShown(false)
        let sch = try file(KiCadExport.files(for: doc, base: "tb"), "kicad_sch")
        XCTAssertFalse(sch.contains("(polyline"))
        XCTAssertFalse(sch.contains("(text \"Clock board\""))
        XCTAssertTrue(sch.contains("(title \"Clock board\")"))
    }

    func testSameDocumentGivesSameBytes() throws {
        let doc = try Fixture.open("AFU")
        XCTAssertEqual(KiCadExport.files(for: doc, base: "x"), KiCadExport.files(for: doc, base: "x"))
    }

    func testEveryPartIsASymbolAndEveryFootprintAFootprint() throws {
        let (doc, files) = try files("LFU")
        let schematic = try file(files, "kicad_sch")
        let board = try file(files, "kicad_pcb")
        // Parts are `Schematical:<part>`; power symbols, flags and the connector have their own prefixes.
        let all = count("\n\t\t(lib_id \"Schematical:", in: schematic)
        let special = count("(lib_id \"Schematical:PWR_", in: schematic) + count("(lib_id \"Schematical:EDGE_", in: schematic)
        let supplies = doc.chips.filter { $0.symbol == .vcc || $0.symbol == .ground }.count
        XCTAssertEqual(all - special, doc.chips.count - supplies)
        XCTAssertGreaterThanOrEqual(special, supplies)
        XCTAssertEqual(count("\n\t(footprint ", in: board), doc.chips.filter { $0.board != nil }.count + extraFootprints(doc))
    }

    private func extraFootprints(_ doc: SchematicDocument) -> Int {
        (doc.board.fingers.isEmpty && doc.board.extraPads.isEmpty ? 0 : 1) + doc.board.holes.count + doc.holes.count
    }

    func testEveryWireSegmentIsAKiCadWire() throws {
        let (doc, files) = try files("AFU")
        let schematic = try file(files, "kicad_sch")
        var segments = 0
        for wire in doc.wires {
            guard let start = doc.endTip(wire.start), let end = doc.endTip(wire.end) else { continue }
            let route = [start] + wire.interior + [end]
            segments += zip(route, route.dropFirst()).filter { $0.0 != $0.1 }.count
        }
        XCTAssertGreaterThan(segments, 0)
        XCTAssertEqual(count("\n\t(wire\n", in: schematic), segments)
    }

    func testBoardHasOutlineNetsAndCopper() throws {
        let (doc, files) = try files("AFU")
        let board = try file(files, "kicad_pcb")
        XCTAssertEqual(count("(layer \"Edge.Cuts\")", in: board), doc.board.outline.count)
        XCTAssertTrue(board.contains("(net 1 \""))
        XCTAssertEqual(count("\n\t(via\n", in: board), doc.vias.count)
        let segments = doc.traces.reduce(0) { $0 + $1.segments.count }
        XCTAssertEqual(count("\n\t(segment\n", in: board), segments)
        // Rails are named as the power symbols are.
        XCTAssertTrue(board.contains("\"GND\""))
    }

    func testPadsCarryTheNetOfTheirPin() throws {
        let doc = try Fixture.open("AFU")
        let copper = doc.connectivity()
        let board = KiCadExport.files(for: doc, base: "x")[2].text
        let nets = Set(copper.netOfPad.values)
        // A row per net a pad carries and one per pin no wire reaches, plus the unnamed net 0.
        XCTAssertGreaterThan(count("\n\t(net ", in: board), nets.count + 1)
        XCTAssertTrue(board.contains("\"unconnected-("))
    }

    func testBaseNameKeepsFileNamesLegal() {
        XCTAssertEqual(KiCadExport.baseName("a/b:c"), "a_b_c")
        XCTAssertEqual(KiCadExport.baseName("  "), "Untitled")
    }

    func testSaveAsKiCadWritesNextToTheChosenFileAndLeavesTheWindowAlone() throws {
        let doc = try Fixture.open("AFU")
        let document = TestSupport.systemDocument(of: doc)
        let controller = document.model.file
        let directory = FileManager.default.temporaryDirectory.appendingPathComponent("kicad-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: directory) }
        TestSupport.turnRunLoop()
        let before = document.isDocumentEdited
        try controller.writeKiCad(to: directory.appendingPathComponent("Board.kicad_pro"))
        let written = try FileManager.default.contentsOfDirectory(atPath: directory.path).sorted()
        XCTAssertEqual(written, ["Board.kicad_pcb", "Board.kicad_pro", "Board.kicad_sch"])
        XCTAssertNil(document.fileURL)
        XCTAssertNil(controller.fileURL)
        TestSupport.turnRunLoop()
        XCTAssertEqual(document.isDocumentEdited, before, "an export is not a save, and not an edit")
        XCTAssertFalse(doc.undoManager.canUndo)
    }

    private func printRouted() throws {
        let doc = try ExportSamples.routed()
        for chip in doc.chips {
            for pad in chip.boardPads {
                print("RT_PADC \(chip.reference) \(pad.designator ?? "-") \(pad.rect.midX) \(pad.rect.midY) \(pad.rect.width) \(pad.rect.height)")
            }
        }
        for trace in doc.traces {
            let points = trace.points.map { "\($0.x),\($0.y)" }.joined(separator: ";")
            print("RT_TRACE \(trace.layer.rawValue) \(trace.width) \(points)")
        }
        for via in doc.vias { print("RT_VIA \(via.centre.x) \(via.centre.y) \(via.drill) \(via.diameter)") }
        for hole in doc.holes { print("RT_HOLE \(hole.centre.x) \(hole.centre.y) \(hole.drill) \(hole.diameter) \(hole.plated)") }
        print("RT_BOARD \(doc.board.width) \(doc.board.height) \(doc.board.layers)")
        print("EPRO_FILE routed.epro \(EasyEDAProExport.data(for: doc, base: "routed").base64EncodedString())")
    }

    /// File › Export › KiCad Project… asks for a folder (the sandbox lets the
    /// app write only the one file a save panel names) and writes the three
    /// files into it, named after the schematic.
    func testKiCadProjectIsWrittenIntoTheChosenFolder() throws {
        let doc = try Fixture.open("AFU")
        let document = TestSupport.systemDocument(of: doc)
        let exporter = document.model.file
        let panel = exporter.kiCadFolderPanel()
        XCTAssertTrue(panel.canChooseDirectories)
        XCTAssertFalse(panel.canChooseFiles)
        XCTAssertTrue(panel.canCreateDirectories)
        XCTAssertEqual(panel.prompt, "Export")
        XCTAssertNil(panel.accessoryView, "no Format menu")
        XCTAssertEqual(panel.message, "Choose the folder for Untitled.kicad_pro, Untitled.kicad_sch, and Untitled.kicad_pcb.")

        let directory = FileManager.default.temporaryDirectory.appendingPathComponent("kicad-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: directory) }
        // The model's URL, which names the exports. Not the system document's:
        // AppKit would then autosave this edited document to a file that is not there.
        document.model.fileURL = directory.appendingPathComponent("My Board.schematical")
        XCTAssertEqual(exporter.kiCadFiles(in: directory).map(\.lastPathComponent),
                       ["My Board.kicad_pro", "My Board.kicad_sch", "My Board.kicad_pcb"])
        XCTAssertEqual(exporter.kiCadFolderPanel().directoryURL?.standardizedFileURL, directory.standardizedFileURL)
        TestSupport.turnRunLoop()
        let before = document.isDocumentEdited
        try exporter.writeKiCadProject(into: directory)
        let written = try FileManager.default.contentsOfDirectory(atPath: directory.path).sorted()
        XCTAssertEqual(written, ["My Board.kicad_pcb", "My Board.kicad_pro", "My Board.kicad_sch"])
        TestSupport.turnRunLoop()
        XCTAssertEqual(document.isDocumentEdited, before, "an export is not a save, and not an edit")
        // A second export replaces the files.
        try exporter.writeKiCadProject(into: directory)
        XCTAssertEqual(try FileManager.default.contentsOfDirectory(atPath: directory.path).count, 3)
    }

    /// A pad's place in the file, turned the way KiCad turns it, is where
    /// the app puts the pad: KiCad's angle counts counter-clockwise, y down.
    func testFootprintPadsLandWhereTheAppPutsThem() throws {
        let doc = try ExportSamples.turned()
        let board = KiCadExport.files(for: doc, base: "t")[2].text
        let shift = try pageShift(board, doc)
        var checked = 0
        for chip in doc.chips {
            let placement = try XCTUnwrap(chip.board)
            // This footprint's block of the file.
            let start = try XCTUnwrap(board.range(of: "(uuid \"\(KiCad.uuid("pcb/t/fp/\(chip.id)"))\")"))
            let block = String(board[start.lowerBound...].prefix(upTo: board.range(of: "\n\t(footprint", range: start.upperBound..<board.endIndex)?.lowerBound ?? board.endIndex))
            let at = try numbers(after: "(at ", in: block)
            let angle = at.count > 2 ? at[2] : 0
            XCTAssertEqual(angle, Double((360 - placement.rotation) % 360), "\(chip.reference) angle")
            let a = angle * .pi / 180
            let pads = block.components(separatedBy: "\n\t\t(pad ").dropFirst()
            XCTAssertEqual(pads.count, chip.boardPads.count)
            for (text, expected) in zip(pads, chip.boardPads) {
                let p = try numbers(after: "(at ", in: String(text))
                // KiCad's counter-clockwise turn in a y-down frame.
                let x = at[0] + p[0] * cos(a) + p[1] * sin(a)
                let y = at[1] - p[0] * sin(a) + p[1] * cos(a)
                XCTAssertEqual(x, expected.rect.midX + shift.x, accuracy: 0.001, "\(chip.reference) pad x")
                XCTAssertEqual(y, expected.rect.midY + shift.y, accuracy: 0.001, "\(chip.reference) pad y")
                checked += 1
            }
        }
        XCTAssertGreaterThan(checked, 100)
    }

    /// plan-54-G: a pad's `(layers …)` are its layer's, the footprint's side or
    /// the other for an opposite pad, while its x is mirrored by the
    /// footprint's side.
    func testOppositePadsAreOnTheOtherCopperLayer() throws {
        for side in [BoardSide.top, .bottom] {
            let (doc, id) = try OppositePadTests.fixture(side: side)
            let placed = try XCTUnwrap(doc.chips.first { $0.id == id })
            let board = KiCadExport.files(for: doc, base: "t")[2].text
            let start = try XCTUnwrap(board.range(of: "(uuid \"\(KiCad.uuid("pcb/t/fp/\(id)"))\")"))
            let block = String(board[start.lowerBound...].prefix(upTo: board.range(of: "\n\t(footprint", range: start.upperBound..<board.endIndex)?.lowerBound ?? board.endIndex))
            let pads = Array(block.components(separatedBy: "\n\t\t(pad ").dropFirst())
            XCTAssertEqual(pads.count, 14)
            let own = side == .top ? "(layers \"F.Cu\" \"F.Paste\" \"F.Mask\")" : "(layers \"B.Cu\" \"B.Paste\" \"B.Mask\")"
            let far = side == .top ? "(layers \"B.Cu\" \"B.Paste\" \"B.Mask\")" : "(layers \"F.Cu\" \"F.Paste\" \"F.Mask\")"
            for (text, pad) in zip(pads, placed.boardPads) {
                let opposite = pad.designator == "2" || pad.designator == "3"
                XCTAssertTrue(text.contains(opposite ? far : own), "\(side) part, pad \(pad.designator ?? "-")")
                XCTAssertFalse(text.contains(opposite ? own : far), "\(side) part, pad \(pad.designator ?? "-")")
            }
            // Pad 1 is left of pad 3 on a Top part and right of it on a Bottom one (mirrored).
            let x = try pads.map { try numbers(after: "(at ", in: $0)[0] }
            XCTAssertEqual(x[0] < x[2], side == .top, "\(side) part: x mirrored by the footprint's side")
        }
    }

    /// The numbers after the first `marker` in `text`.
    private func numbers(after marker: String, in text: String) throws -> [Double] {
        let start = try XCTUnwrap(text.range(of: marker))
        let rest = text[start.upperBound...].prefix { $0 != ")" }
        return rest.split(separator: " ").compactMap { Double($0) }
    }

    /// The page shift the board writer used: the first outline line starts at the board's corner.
    private func pageShift(_ board: String, _ doc: SchematicDocument) throws -> CGPoint {
        let start = try XCTUnwrap(board.range(of: "(gr_line\n\t\t(start "))
        let at = try numbers(after: "(start ", in: String(board[start.lowerBound...]))
        let corner = try XCTUnwrap(doc.board.outline.first)
        return CGPoint(x: at[0] - corner.x, y: at[1] - corner.y)
    }

    func testViasCarryTheirTenting() throws {
        let doc = try ExportSamples.vias()
        let board = try XCTUnwrap(KiCadExport.files(for: doc, base: "v").first { $0.name.hasSuffix("kicad_pcb") }).text
        var pairs: [String] = []
        var rest = Substring(board)
        while let range = rest.range(of: "\t\t(tenting\n\t\t\t(front ") {
            rest = rest[range.upperBound...]
            let front = rest.prefix { $0 != ")" }
            let back = try XCTUnwrap(rest.range(of: "(back ")).upperBound
            pairs.append("\(front) \(rest[back...].prefix { $0 != ")" })")
        }
        XCTAssertEqual(pairs, ["yes yes", "no yes", "yes no", "no no"])
        XCTAssertEqual(board.components(separatedBy: "(tenting").count - 1, 4, "no board-level tenting key")
    }

    // MARK: Stackup (plan-50-E)

    private func boardText(_ doc: SchematicDocument) throws -> String {
        try XCTUnwrap(KiCadExport.files(for: doc, base: "s").first { $0.name.hasSuffix("kicad_pcb") }).text
    }

    /// The `(layer "<name>" …)` blocks of the board's stackup, by name, as their field lines.
    private func stackupLayers(_ board: String) throws -> [(name: String, fields: [String])] {
        let start = try XCTUnwrap(board.range(of: "\t\t(stackup\n"))
        let end = try XCTUnwrap(board.range(of: "\t\t(copper_finish", range: start.upperBound..<board.endIndex))
        var layers: [(String, [String])] = []
        for block in board[start.upperBound..<end.lowerBound].components(separatedBy: "\t\t\t(layer ").dropFirst() {
            let lines = block.split(separator: "\n").map { $0.trimmingCharacters(in: .whitespaces) }
            let name = String(lines[0].prefix { $0 != "\n" }).trimmingCharacters(in: CharacterSet(charactersIn: "\""))
            layers.append((name, lines.dropFirst().filter { $0 != ")" }.map { $0 }))
        }
        return layers
    }

    func testTwoLayerStackup() throws {
        let board = try boardText(TestSupport.newDocument())
        XCTAssertTrue(board.contains("\t\t(thickness 1.6)\n"), "general thickness")
        let layers = try stackupLayers(board)
        XCTAssertEqual(layers.map(\.name), ["F.SilkS", "F.Paste", "F.Mask", "F.Cu", "dielectric 1", "B.Cu", "B.Mask",
                                           "B.Paste", "B.SilkS"])
        let dielectric = try XCTUnwrap(layers.first { $0.name.hasPrefix("dielectric 1") }).fields
        XCTAssertTrue(dielectric.contains("(type \"core\")") && dielectric.contains("(thickness 1.51)")
                      && dielectric.contains("(epsilon_r 4.5)") && dielectric.contains("(material \"FR4\")"), "\(dielectric)")
        XCTAssertTrue(board.contains("(copper_finish \"HAL lead-free\")"))
        XCTAssertTrue(board.contains("(color \"Green\")"))
    }

    func testFourLayerStackup() throws {
        let board = try boardText(try ExportSamples.fourLayer())
        XCTAssertTrue(board.contains("\t\t(thickness 1.6062)\n"))
        let layers = try stackupLayers(board)
        XCTAssertEqual(layers.map(\.name).filter { $0.contains("Cu") }, ["F.Cu", "In1.Cu", "In2.Cu", "B.Cu"])
        let kinds = layers.filter { $0.name.hasPrefix("dielectric") }.compactMap { $0.fields.first }
        XCTAssertEqual(kinds, ["(type \"prepreg\")", "(type \"core\")", "(type \"prepreg\")"])
        XCTAssertTrue(layers.first { $0.name.hasPrefix("In1.Cu") }?.fields.contains("(thickness 0.0152)") == true)
    }

    func testOneLayerBoardKeepsItsTotalAndGetsABottomCopper() throws {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        let board = try boardText(doc)
        XCTAssertTrue(board.contains("\t\t(thickness 1.61)\n"))
        let layers = try stackupLayers(board)
        XCTAssertEqual(layers.first { $0.name.hasPrefix("B.Cu") }?.fields.contains("(thickness 0.035)"), true)
        XCTAssertEqual(layers.first { $0.name.hasPrefix("dielectric 1") }?.fields.contains("(thickness 1.52)"), true)
    }

    func testEditedStackupIsWritten() throws {
        let doc = TestSupport.newDocument()
        doc.setStackupFinish(.enig)
        doc.setBoardColor(.black)
        doc.setCopperThickness(0.07, at: 0)
        let board = try boardText(doc)
        XCTAssertTrue(board.contains("(copper_finish \"ENIG\")"))
        XCTAssertEqual(board.components(separatedBy: "(color \"Black\")").count - 1, 2, "both masks")
        let layers = try stackupLayers(board)
        XCTAssertEqual(layers.first { $0.name.hasPrefix("F.Cu") }?.fields.contains("(thickness 0.07)"), true)
        XCTAssertTrue(board.contains("\t\t(thickness 1.635)\n"))
    }

    func testNetNamesAreShared() throws {
        let doc = try ExportSamples.vias()
        let nets = ExportNetNames.assign(context: KiCadExport.Context(document: doc, base: "v"))
        let board = try XCTUnwrap(KiCadExport.files(for: doc, base: "v").first { $0.name.hasSuffix("kicad_pcb") }).text
        var written: [Int: String] = [:]
        for line in board.split(separator: "\n") where line.hasPrefix("\t(net ") && !line.hasPrefix("\t(net 0 ") {
            let parts = line.dropFirst(6).dropLast().split(separator: " ", maxSplits: 1)
            if parts.count == 2, let code = Int(parts[0]) { written[code] = String(parts[1].dropFirst().dropLast()) }
        }
        XCTAssertFalse(nets.names.isEmpty)
        XCTAssertEqual(Set(nets.names.values), Set(written.values))
        for (net, name) in nets.names { XCTAssertEqual(written[try XCTUnwrap(nets.codes[net])], name) }
        XCTAssertTrue(nets.names.values.contains { $0.hasPrefix("unconnected-(") }, "pins no wire reaches get their own nets")
    }

    /// Prints every fixture's project as `KICAD_FILE <folder>/<name> <base64>`
    /// lines when `KICAD_OUT` is set, for running `kicad-cli` over. (The test
    /// host is sandboxed: it can write only to its own container, which the
    /// shell can't read.) Also the turned-parts board and the app's own drill
    /// file for it, to compare with KiCad's.
    func testPrintSamplesWhenAsked() throws {
        guard ProcessInfo.processInfo.environment["KICAD_OUT"] != nil else { return }
        for name in Fixture.names {
            for file in KiCadExport.files(for: try Fixture.open(name), base: name) {
                print("KICAD_FILE \(name)/\(file.name) \(file.data.base64EncodedString())")
            }
        }
        try printRouted()
        for file in KiCadExport.files(for: try ExportSamples.titled(), base: "titleblock") {
            print("KICAD_FILE titleblock/\(file.name) \(file.data.base64EncodedString())")
        }
        let doc = try ExportSamples.turned()
        // Every pad's box as the app has it, for the SVG check.
        for chip in doc.chips {
            let side = chip.board?.side == .bottom ? "B" : "F"
            for pad in chip.boardPads where pad.drill == 0 {
                print("APP_PAD \(side) \(chip.reference) \(pad.rect.minX) \(pad.rect.minY) \(pad.rect.maxX) \(pad.rect.maxY)")
            }
        }
        for file in KiCadExport.files(for: doc, base: "turned") where file.name.hasSuffix("kicad_pcb") {
            print("KICAD_FILE turned/\(file.name) \(file.data.base64EncodedString())")
        }
        for chip in doc.chips {
            for pad in chip.boardPads {
                print("APP_PADC \(chip.reference) \(pad.designator ?? "-") \(pad.rect.midX) \(pad.rect.midY) \(pad.rect.width) \(pad.rect.height)")
            }
        }
        print("EPRO_FILE turned.epro \(EasyEDAProExport.data(for: doc, base: "turned").base64EncodedString())")
        for name in Fixture.names {
            print("EPRO_FILE \(name).epro \(EasyEDAProExport.data(for: try Fixture.open(name), base: name).base64EncodedString())")
        }
        if let drill = GerberExport.files(for: doc, base: "turned").first(where: { $0.name.hasSuffix("-PTH.drl") }) {
            print("KICAD_FILE turned/app.drl \(drill.data.base64EncodedString())")
        }
        let viaBoard = try ExportSamples.vias()
        for file in KiCadExport.files(for: viaBoard, base: "vias") where file.name.hasSuffix("kicad_pcb") {
            print("KICAD_FILE vias/\(file.name) \(file.data.base64EncodedString())")
        }
        for file in GerberExport.files(for: viaBoard, base: "vias") {
            print("GERBER_FILE vias/\(file.name) \(file.data.base64EncodedString())")
        }
        let poursBoard = try ExportSamples.pours()
        for file in KiCadExport.files(for: poursBoard, base: "pours") where file.name.hasSuffix("kicad_pcb") {
            print("KICAD_FILE pours/\(file.name) \(file.data.base64EncodedString())")
        }
        for file in GerberExport.files(for: poursBoard, base: "pours") {
            print("GERBER_FILE pours/\(file.name) \(file.data.base64EncodedString())")
        }
        let fourLayerBoard = try ExportSamples.fourLayer()
        for file in KiCadExport.files(for: fourLayerBoard, base: "fourLayer") where file.name.hasSuffix("kicad_pcb") {
            print("KICAD_FILE fourLayer/\(file.name) \(file.data.base64EncodedString())")
        }
        for file in GerberExport.files(for: fourLayerBoard, base: "fourLayer") {
            print("GERBER_FILE fourLayer/\(file.name) \(file.data.base64EncodedString())")
        }
        // plan-54-G: a Top and a Bottom part with opposite pads. APP_OPPOSITE_PAD lines give every pad's centre for the check.
        let oppositeBoard = try ExportSamples.oppositePads()
        for chip in oppositeBoard.chips {
            for pad in chip.boardPads {
                print("APP_OPPOSITE_PAD \(chip.reference) \(pad.designator ?? "-") \(pad.rect.midX) \(pad.rect.midY) \(pad.rect.width) \(pad.rect.height) \(chip.board?.side.rawValue ?? "-") \(pad.opposite)")
            }
        }
        for file in KiCadExport.files(for: oppositeBoard, base: "opposite") where file.name.hasSuffix("kicad_pcb") {
            print("KICAD_FILE opposite/\(file.name) \(file.data.base64EncodedString())")
        }
        for file in GerberExport.files(for: oppositeBoard, base: "opposite") {
            print("GERBER_FILE opposite/\(file.name) \(file.data.base64EncodedString())")
        }
    }
}

/// Boards the export tests (and `kicad-cli` / EasyEDA importer checks) use.
@MainActor
enum ExportSamples {
    /// plan-64-D: two gates, a filled-in Document Info and its title block.
    static func titled() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        _ = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        _ = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 400, y: 100))
        doc.setInfoTitle("Clock board")
        doc.setInfoAuthor("J. Smith")
        doc.setInfoRevision("B")
        doc.setInfoDescription("Divides the 4 MHz clock by 8.\nDrives the counter.")
        doc.setInfoCreated(Date(timeIntervalSince1970: 1_795_615_629))
        doc.setInfoUpdated(Date(timeIntervalSince1970: 1_795_619_999))
        doc.setTitleBlockShown(true)
        return doc
    }

    /// A board with a DIP at each turn on Top and on Bottom.
    static func turned() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(2)
        for (row, (side, part)) in [(BoardSide.top, "SN74HC00N"), (.bottom, "SN74HC00N"), (.top, "SN74HC00DR"), (.bottom, "SN74HC00D")].enumerated() {
            for (column, rotation) in [0, 90, 180, 270].enumerated() {
                let chip = try TestSupport.place(doc, part, at: CGPoint(x: 100 + 200 * column, y: 100 + 200 * row))
                if side == .bottom { doc.setFootprintSide(chip.id, .bottom) }
                while doc.chips.first(where: { $0.id == chip.id })?.board?.rotation != rotation { doc.rotateFootprint(chip.id) }
                doc.moveFootprint(chip.id, to: CGPoint(x: 20 + 25 * Double(column), y: 20 + 25 * Double(row)))
            }
        }
        return doc
    }

    /// Two gates wired together and to the supplies, with copper: a trace, a via, a ground
    /// plane on an inner layer, a text and a plated hole.
    static func routed() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        let a = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 400, y: 100))
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 100, y: 300))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 200, y: 300))
        func pin(_ chip: PlacedChip, _ designator: String) throws -> LineAnchor {
            let index = try XCTUnwrap(chip.part.pins.firstIndex { $0.designator == designator })
            return .pin(PinRef(chipID: chip.id, index: index))
        }
        XCTAssertNil(doc.connect(try pin(a, "3"), try pin(b, "1")))
        XCTAssertNil(doc.connect(try pin(a, "14"), try pin(vcc, "1")))
        XCTAssertNil(doc.connect(try pin(a, "7"), try pin(gnd, "1")))
        let chipA = try XCTUnwrap(doc.chips.first { $0.id == a.id }), chipB = try XCTUnwrap(doc.chips.first { $0.id == b.id })
        func centre(_ chip: PlacedChip, _ designator: String) throws -> CGPoint {
            let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
            return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
        }
        let from = try centre(chipA, "3"), to = try centre(chipB, "1")
        _ = try XCTUnwrap(doc.addTrace(points: [from, CGPoint(x: to.x, y: from.y), to], layer: .top, width: 12))
        _ = try XCTUnwrap(doc.addVia(at: CGPoint(x: to.x, y: from.y)))
        doc.setLayerType(.inner(2), .plane)
        doc.setLayerNet(.inner(2), .ground)
        let text = doc.addText(at: CGPoint(x: 5, y: 5))
        _ = text
        _ = doc.addHole(at: CGPoint(x: 60, y: 40))
        return doc
    }

    /// plan-54-G: the opposite-pad test part (`OppositePadTests.part`) once on Top and once on Bottom.
    static func oppositePads() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(2)
        let part = try OppositePadTests.part()
        for (side, origin) in [(BoardSide.top, CGPoint(x: 30, y: 20)), (.bottom, CGPoint(x: 90, y: 50))] {
            doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100))
            let id = try XCTUnwrap(doc.chips.last).id
            if side == .bottom { doc.setFootprintSide(id, .bottom) }
            doc.moveFootprintOrigin(id, to: origin)
        }
        return doc
    }

    /// The `vias()` board on 4 layers, its two inner layers Planes on ground and on 5 V (the first gate's pins wired
    /// to a supply symbol each), and an `SN74ABT125RGYR`, whose VQFN footprint has an unnumbered centre pad.
    static func fourLayer() throws -> SchematicDocument {
        let doc = try vias()
        doc.setLayerCount(4)
        let gate = try XCTUnwrap(doc.chips.first)
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 100, y: 300))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 200, y: 300))
        func pin(_ chip: PlacedChip, _ designator: String) throws -> LineAnchor {
            let index = try XCTUnwrap(chip.part.pins.firstIndex { $0.designator == designator })
            return .pin(PinRef(chipID: chip.id, index: index))
        }
        XCTAssertNil(doc.connect(try pin(gate, "14"), try pin(vcc, "1")))
        XCTAssertNil(doc.connect(try pin(gate, "7"), try pin(gnd, "1")))
        doc.setLayerType(.inner(2), .plane)
        doc.setLayerNet(.inner(2), .ground)
        doc.setLayerType(.inner(3), .plane)
        doc.setLayerNet(.inner(3), .rail(mv: 5000))
        _ = try TestSupport.place(doc, "SN74ABT125RGYR", at: CGPoint(x: 100, y: 500))
        return doc
    }

    /// The `fourLayer()` board (inner Planes on ground and 5 V) with three cutouts in its far corners: a 6 mm circle, a 10 × 2 mm slot and a
    /// triangle (plan-65-G).
    static func cutouts() throws -> SchematicDocument {
        let doc = try fourLayer()
        _ = try XCTUnwrap(doc.addCutout(.circle(centre: CGPoint(x: 88, y: 12), diameter: 6)))
        _ = try XCTUnwrap(doc.addCutout(.slot(rect: CGRect(x: 80, y: 40, width: 10, height: 2))))
        _ = try XCTUnwrap(doc.addCutout(.polygon([CGPoint(x: 85, y: 60), CGPoint(x: 95, y: 60), CGPoint(x: 90, y: 70)])))
        return doc
    }

    /// Two gates on the board wired together by a trace, the first on ground and 5 V, a ground pour over the whole Top (thermal, gap 0.6,
    /// spokes 0.4, minimum width 0.25, clearance 0.3), a solid 5 V pour on Bottom, and a keep-out on Top only.
    static func pours() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 400, y: 100))
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 100, y: 300))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 200, y: 300))
        doc.moveFootprint(a.id, to: CGPoint(x: 25, y: 30))
        doc.moveFootprint(b.id, to: CGPoint(x: 65, y: 30))
        func pin(_ chip: PlacedChip, _ designator: String) throws -> LineAnchor {
            let index = try XCTUnwrap(chip.part.pins.firstIndex { $0.designator == designator })
            return .pin(PinRef(chipID: chip.id, index: index))
        }
        XCTAssertNil(doc.connect(try pin(a, "3"), try pin(b, "1")))
        XCTAssertNil(doc.connect(try pin(a, "14"), try pin(vcc, "1")))
        XCTAssertNil(doc.connect(try pin(a, "7"), try pin(gnd, "1")))
        let chipA = try XCTUnwrap(doc.chips.first { $0.id == a.id }), chipB = try XCTUnwrap(doc.chips.first { $0.id == b.id })
        func centre(_ chip: PlacedChip, _ designator: String) throws -> CGPoint {
            let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
            return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
        }
        let from = try centre(chipA, "3"), to = try centre(chipB, "1")
        _ = try XCTUnwrap(doc.addTrace(points: [from, CGPoint(x: to.x, y: from.y), to], layer: .top, width: 12))
        let board = CGRect(x: 0, y: 0, width: doc.board.width, height: doc.board.height)
        let top = try XCTUnwrap(doc.addPour(rect: board, layer: .top))
        doc.setPourNet(top, .ground)
        doc.setPourClearance(top, 0.3)
        doc.setPourThermalGap(top, 0.6)
        doc.setPourSpokeWidth(top, 0.4)
        doc.setPourMinWidth(top, 0.25)
        let bottom = try XCTUnwrap(doc.addPour(rect: board, layer: .bottom))
        doc.setPourNet(bottom, .rail(mv: 5000))
        doc.setPourConnection(bottom, .solid)
        doc.addKeepout(rect: CGRect(x: 60, y: 10, width: 12, height: 8), layers: [.top])
        return doc
    }

    /// A 2-layer board of two gates wired together, a trace between them, and a via on the trace for each
    /// (Top, Bottom) tenting given (tented = true): by default (yes, yes), (no, yes), (yes, no), (no, no).
    static func vias(_ tenting: [(Bool, Bool)] = [(true, true), (false, true), (true, false), (false, false)]) throws
        -> SchematicDocument {
        let doc = TestSupport.newDocument()
        let a = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        let b = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 400, y: 100))
        func pin(_ chip: PlacedChip, _ designator: String) throws -> LineAnchor {
            let index = try XCTUnwrap(chip.part.pins.firstIndex { $0.designator == designator })
            return .pin(PinRef(chipID: chip.id, index: index))
        }
        XCTAssertNil(doc.connect(try pin(a, "3"), try pin(b, "1")))
        let chipA = try XCTUnwrap(doc.chips.first { $0.id == a.id }), chipB = try XCTUnwrap(doc.chips.first { $0.id == b.id })
        func centre(_ chip: PlacedChip, _ designator: String) throws -> CGPoint {
            let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
            return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
        }
        let from = try centre(chipA, "3"), to = try centre(chipB, "1")
        _ = try XCTUnwrap(doc.addTrace(points: [from, CGPoint(x: to.x, y: from.y), to], layer: .top, width: 12))
        for (index, (top, bottom)) in tenting.enumerated() {
            let fraction = CGFloat(index + 1) / CGFloat(tenting.count + 1)
            let id = try XCTUnwrap(doc.addVia(at: CGPoint(x: from.x + (to.x - from.x) * fraction, y: from.y)))
            doc.setViaDiameter(id, 0.77)
            doc.setViaDrill(id, 0.4)
            doc.setViaTenting(id, .top, top)
            doc.setViaTenting(id, .bottom, bottom)
        }
        return doc
    }

}
