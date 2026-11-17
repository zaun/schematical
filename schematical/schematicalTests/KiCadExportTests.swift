import Foundation
import AppKit
import UniformTypeIdentifiers
import XCTest
@testable import schematical

/// File > Save As… with Format KiCad Project: the three files
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
        let controller = SchematicFileController(document: doc, catalog: TestSupport.catalog, willReplaceDocument: {})
        Self.kept.append(controller)
        let directory = FileManager.default.temporaryDirectory.appendingPathComponent("kicad-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: directory) }
        let before = controller.hasUnsavedChanges
        try controller.writeKiCad(to: directory.appendingPathComponent("Board.kicad_pro"))
        let written = try FileManager.default.contentsOfDirectory(atPath: directory.path).sorted()
        XCTAssertEqual(written, ["Board.kicad_pcb", "Board.kicad_pro", "Board.kicad_sch"])
        XCTAssertNil(controller.fileURL)
        XCTAssertEqual(controller.hasUnsavedChanges, before)
    }

    private static var kept: [SchematicFileController] = []

    /// The Save As panel's Format menu changes the file type and the extension of the name,
    /// and keeps what was typed.
    func testSaveFormatPickerSwitchesTypeAndExtension() throws {
        let panel = NSSavePanel()
        let picker = SaveFormatPicker(panel: panel, baseName: "Board")
        XCTAssertEqual(picker.format, .schematical)
        XCTAssertEqual(panel.nameFieldStringValue, "Board.schematical")
        XCTAssertNotNil(panel.accessoryView)
        panel.nameFieldStringValue = "Renamed.schematical"
        let popup = try XCTUnwrap(findPopup(in: try XCTUnwrap(panel.accessoryView)))
        XCTAssertEqual(popup.itemTitles, ["Schematical", "KiCad Project"])
        popup.selectItem(at: 1)
        _ = popup.target?.perform(popup.action, with: popup)
        XCTAssertEqual(picker.format, .kicad)
        XCTAssertEqual(panel.nameFieldStringValue, "Renamed.kicad_pro")
        XCTAssertEqual(panel.allowedContentTypes.first?.preferredFilenameExtension, "kicad_pro")
        popup.selectItem(at: 0)
        _ = popup.target?.perform(popup.action, with: popup)
        XCTAssertEqual(picker.format, .schematical)
        XCTAssertEqual(panel.nameFieldStringValue, "Renamed.schematical")
        // A dot in the name is not an extension.
        let dotted = NSSavePanel()
        let other = SaveFormatPicker(panel: dotted, baseName: "v1.2")
        XCTAssertEqual(dotted.nameFieldStringValue, "v1.2.schematical")
        let menu = try XCTUnwrap(findPopup(in: try XCTUnwrap(dotted.accessoryView)))
        menu.selectItem(at: 1)
        _ = menu.target?.perform(menu.action, with: menu)
        XCTAssertEqual(dotted.nameFieldStringValue, "v1.2.kicad_pro")
        XCTAssertEqual(other.format, .kicad)
    }

    private func findPopup(in view: NSView) -> NSPopUpButton? {
        if let popup = view as? NSPopUpButton { return popup }
        for sub in view.subviews { if let found = findPopup(in: sub) { return found } }
        return nil
    }

    /// A board with a DIP at each turn on Top and on Bottom.
    private func turnedParts() throws -> SchematicDocument {
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

    /// A pad's place in the file, turned the way KiCad turns it, is where
    /// the app puts the pad: KiCad's angle counts counter-clockwise, y down.
    func testFootprintPadsLandWhereTheAppPutsThem() throws {
        let doc = try turnedParts()
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
        let doc = try turnedParts()
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
        if let drill = GerberExport.files(for: doc, base: "turned").first(where: { $0.name.hasSuffix("-PTH.drl") }) {
            print("KICAD_FILE turned/app.drl \(drill.data.base64EncodedString())")
        }
    }
}
