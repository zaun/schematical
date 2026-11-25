import Foundation
import XCTest
@testable import schematical

/// File > Save As…, format EasyEDA Pro Project (`EasyEDAProExport`). The
/// files are checked for shape and for where things land here; that EasyEDA's
/// format is what KiCad's EasyEDA Pro importer reads, with the same pads, nets,
/// copper and zones, was checked by loading the output with it (see
/// `docs/export.md` § EasyEDA Pro).
@MainActor
final class EasyEDAProExportTests: XCTestCase {
    private typealias Entries = [(name: String, data: Data)]

    private func text(_ entries: Entries, _ name: String) throws -> String {
        String(decoding: try XCTUnwrap(entries.first { $0.name == name }, "no \(name)").data, as: UTF8.self)
    }

    private func entry(_ entries: Entries, suffix: String) throws -> (name: String, text: String) {
        let found = try XCTUnwrap(entries.first { $0.name.hasSuffix(suffix) }, "no \(suffix)")
        return (found.name, String(decoding: found.data, as: UTF8.self))
    }

    /// A file's lines, each parsed as the JSON array it must be.
    private func lines(_ text: String) throws -> [[Any]] {
        try text.split(separator: "\n").map { line in
            let value = try JSONSerialization.jsonObject(with: Data(line.utf8))
            return try XCTUnwrap(value as? [Any], "not an array: \(line.prefix(80))")
        }
    }

    private func number(_ value: Any?) -> Double { (value as? NSNumber)?.doubleValue ?? .nan }

    func testArchiveHoldsTheFilesEasyEDAReads() throws {
        let doc = try Fixture.open("AFU")
        let entries = EasyEDAProExport.entries(for: doc, base: "AFU")
        let names = entries.map(\.name)
        XCTAssertEqual(names.first, "project.json")
        XCTAssertTrue(names.contains { $0.hasPrefix("SYMBOL/") && $0.hasSuffix(".esym") })
        XCTAssertTrue(names.contains { $0.hasPrefix("FOOTPRINT/") && $0.hasSuffix(".efoo") })
        XCTAssertEqual(names.filter { $0.hasPrefix("SHEET/") && $0.hasSuffix("/1.esch") }.count, 1)
        XCTAssertEqual(names.filter { $0.hasPrefix("PCB/") && $0.hasSuffix(".epcb") }.count, 1)
        XCTAssertEqual(Array(EasyEDAProExport.data(for: doc, base: "AFU").prefix(4)), [0x50, 0x4B, 0x03, 0x04])
    }

    func testEveryLineOfEveryFileIsAJSONArrayWithAName() throws {
        for name in Fixture.names {
            let entries = EasyEDAProExport.entries(for: try Fixture.open(name), base: name)
            for entry in entries where entry.name != "project.json" {
                for line in try lines(String(decoding: entry.data, as: UTF8.self)) {
                    XCTAssertTrue(line.first is String, "\(name) \(entry.name)")
                }
            }
            _ = try JSONSerialization.jsonObject(with: try XCTUnwrap(entries.first).data)
        }
    }

    func testSameDocumentGivesSameBytes() throws {
        let doc = try Fixture.open("LFU")
        XCTAssertEqual(EasyEDAProExport.data(for: doc, base: "x"), EasyEDAProExport.data(for: doc, base: "x"))
    }

    func testProjectNamesOnlyFilesThatAreThere() throws {
        let entries = EasyEDAProExport.entries(for: try Fixture.open("AFU"), base: "AFU")
        let project = try XCTUnwrap(JSONSerialization.jsonObject(with: try XCTUnwrap(entries.first).data) as? [String: Any])
        let names = Set(entries.map(\.name))
        let devices = try XCTUnwrap(project["devices"] as? [String: [String: Any]])
        XCTAssertFalse(devices.isEmpty)
        for (_, device) in devices {
            let attributes = try XCTUnwrap(device["attributes"] as? [String: String])
            XCTAssertTrue(names.contains("SYMBOL/\(attributes["Symbol"] ?? "").esym"))
            if let footprint = attributes["Footprint"] { XCTAssertTrue(names.contains("FOOTPRINT/\(footprint).efoo")) }
        }
        for id in (try XCTUnwrap(project["symbols"] as? [String: Any])).keys { XCTAssertTrue(names.contains("SYMBOL/\(id).esym")) }
        for id in (try XCTUnwrap(project["footprints"] as? [String: Any])).keys { XCTAssertTrue(names.contains("FOOTPRINT/\(id).efoo")) }
        let schematics = try XCTUnwrap(project["schematics"] as? [String: Any])
        let pcbs = try XCTUnwrap(project["pcbs"] as? [String: Any])
        XCTAssertTrue(names.contains("SHEET/\(try XCTUnwrap(schematics.keys.first))/1.esch"))
        XCTAssertTrue(names.contains("PCB/\(try XCTUnwrap(pcbs.keys.first)).epcb"))
        let boards = try XCTUnwrap(project["boards"] as? [String: [String: String]])
        XCTAssertEqual(boards["AFU"]?["pcb"], pcbs.keys.first)
    }

    /// A footprint's pads, put where its component says (turned counter-clockwise, flipped
    /// top to bottom first on Bottom, y up, mil), are where the app puts them. The
    /// same formula was checked against KiCad's importer.
    func testFootprintPadsLandWhereTheAppPutsThem() throws {
        let doc = try ExportSamples.turned()
        let entries = EasyEDAProExport.entries(for: doc, base: "t")
        let board = try lines(try entry(entries, suffix: ".epcb").text)
        var componentOf: [String: [Any]] = [:]
        var attributes: [String: [String: String]] = [:]
        for line in board {
            switch line[0] as? String {
            case "COMPONENT": componentOf[line[1] as? String ?? ""] = line
            case "ATTR":
                if let parent = line[3] as? String, let key = line[7] as? String { attributes[parent, default: [:]][key] = line[8] as? String }
            default: break
            }
        }
        XCTAssertEqual(componentOf.count, doc.chips.count)
        var checked = 0
        for chip in doc.chips {
            let id = try XCTUnwrap(componentOf.keys.first { attributes[$0]?["Designator"] == chip.reference })
            let component = try XCTUnwrap(componentOf[id])
            let footprintID = try XCTUnwrap(attributes[id]?["Footprint"])
            let footprint = try lines(try text(entries, "FOOTPRINT/\(footprintID).efoo"))
            let bottom = (component[3] as? Int) == 2
            let angle = number(component[6]) * .pi / 180
            let cx = number(component[4]), cy = number(component[5])
            let pads = footprint.filter { $0[0] as? String == "PAD" }
            XCTAssertEqual(pads.count, chip.boardPads.count, chip.reference)
            for (pad, expected) in zip(pads, chip.boardPads) {
                let x = number(pad[6]), y = bottom ? -number(pad[7]) : number(pad[7])
                let worldX = cx + x * cos(angle) - y * sin(angle)
                let worldY = cy + x * sin(angle) + y * cos(angle)
                XCTAssertEqual(worldX * 0.0254, expected.rect.midX, accuracy: 0.001, "\(chip.reference) x")
                XCTAssertEqual(-worldY * 0.0254, expected.rect.midY, accuracy: 0.001, "\(chip.reference) y")
                checked += 1
            }
        }
        XCTAssertGreaterThan(checked, 200)
    }

    /// plan-54-G: a footprint file is the footprint as seen from its own side, so
    /// an opposite pad is a layer 2 (bottom) pad of it and every other pad a
    /// layer 1 one, whichever side the part is placed on.
    func testOppositePadsAreOnTheBottomLayerOfTheirFootprint() throws {
        for side in [BoardSide.top, .bottom] {
            let (doc, _) = try OppositePadTests.fixture(side: side)
            let entries = EasyEDAProExport.entries(for: doc, base: "t")
            let footprint = try lines(try entry(entries, suffix: ".efoo").text)
            let pads = footprint.filter { $0[0] as? String == "PAD" }
            XCTAssertEqual(pads.count, 14, "\(side) part")
            for pad in pads {
                let number = try XCTUnwrap(pad[5] as? String)
                XCTAssertEqual(pad[4] as? Int, number == "2" || number == "3" ? 2 : 1, "\(side) part, pad \(number)")
            }
        }
    }

    func testBoardHasNetsCopperPlaneAndText() throws {
        let doc = try ExportSamples.routed()
        let entries = EasyEDAProExport.entries(for: doc, base: "r")
        let board = try lines(try entry(entries, suffix: ".epcb").text)
        func kind(_ name: String) -> [[Any]] { board.filter { $0[0] as? String == name } }
        XCTAssertEqual(kind("LINE").count, doc.traces.reduce(0) { $0 + $1.segments.count })
        XCTAssertEqual(kind("VIA").count, 1)
        XCTAssertEqual(kind("POUR").count, 1)
        XCTAssertEqual(kind("STRING").count, doc.texts.count)
        XCTAssertEqual(kind("COMPONENT").count, 2)
        // The outline, closed, on the outline layer.
        let outline = kind("POLY").first { number($0[4]) == 11 }
        XCTAssertNotNil(outline)
        // Layers: Top, Bottom, the two inner layers of a 4-layer board.
        let layers = kind("LAYER").map { $0[2] as? String }
        XCTAssertTrue(layers.contains("TOP") && layers.contains("BOTTOM"))
        XCTAssertEqual(layers.filter { $0 == "SIGNAL" }.count, 2)
        // Pad nets: the rails by name, the wired gates on one net.
        var padNet: [String: String] = [:]
        var refOf: [String: String] = [:]
        for line in board {
            if line[0] as? String == "ATTR", line[7] as? String == "Designator" { refOf[line[3] as? String ?? ""] = line[8] as? String }
        }
        for line in kind("PAD_NET") { padNet["\(refOf[line[1] as? String ?? ""] ?? "?").\(line[2] as? String ?? "")"] = line[3] as? String }
        XCTAssertEqual(padNet["U1.14"], "5V")
        XCTAssertEqual(padNet["U1.7"], "GND")
        XCTAssertNotNil(padNet["U1.3"])
        XCTAssertEqual(padNet["U1.3"], padNet["U2.1"])
        XCTAssertNil(padNet["U1.1"], "a pin on nothing has no net")
        // Every net a pad names is declared.
        let declared = Set(kind("NET").compactMap { $0[1] as? String })
        for net in padNet.values { XCTAssertTrue(declared.contains(net), net) }
    }

    /// Every wire's end that is a part pin is at that pin's place, the sheet's y running up and a
    /// symbol's y down. (The same placement was checked against KiCad's EasyEDA arithmetic.)
    func testSchematicWiresMeetPinsAndEveryWireIsNamed() throws {
        let doc = try Fixture.open("AFU")
        let entries = EasyEDAProExport.entries(for: doc, base: "AFU")
        let sheet = try lines(try entry(entries, suffix: ".esch").text)
        var attributes: [String: [String: String]] = [:]
        for line in sheet where line[0] as? String == "ATTR" {
            attributes[line[2] as? String ?? "", default: [:]][line[3] as? String ?? ""] = line[4] as? String
        }
        let project = try XCTUnwrap(JSONSerialization.jsonObject(with: try XCTUnwrap(entries.first).data) as? [String: Any])
        let devices = try XCTUnwrap(project["devices"] as? [String: [String: Any]])
        var pins = Set<String>()
        for line in sheet where line[0] as? String == "COMPONENT" {
            let id = try XCTUnwrap(line[1] as? String)
            let device = try XCTUnwrap(devices[attributes[id]?["Device"] ?? ""])
            let symbolID = try XCTUnwrap((device["attributes"] as? [String: String])?["Symbol"])
            for pin in try lines(try text(entries, "SYMBOL/\(symbolID).esym")) where pin[0] as? String == "PIN" {
                pins.insert("\(Int((number(line[3]) + number(pin[4])).rounded())),\(Int((number(line[4]) - number(pin[5])).rounded()))")
            }
        }
        // The connector's short wires to its labels come first, then the schematic's own.
        let allWires = sheet.filter { $0[0] as? String == "WIRE" }
        XCTAssertGreaterThanOrEqual(allWires.count, doc.wires.count)
        let wires = Array(allWires.dropFirst(allWires.count - doc.wires.count))
        var expected = 0, found = 0
        for (line, wire) in zip(wires, doc.wires) {
            let flat = try XCTUnwrap((line[2] as? [[NSNumber]])?.first)
            let ends = [(flat[0], flat[1]), (flat[flat.count - 2], flat[flat.count - 1])]
            for (end, point) in zip([wire.start, wire.end], ends) {
                guard case .pin = end else { continue }
                expected += 1
                if pins.contains("\(point.0.intValue),\(point.1.intValue)") { found += 1 }
            }
            XCTAssertNotNil(attributes[line[1] as? String ?? ""]?["NET"], "an unnamed wire")
        }
        XCTAssertGreaterThan(expected, 100)
        XCTAssertEqual(found, expected)
    }

    func testSaveAsEasyEDAProWritesOneFileAndLeavesTheWindowAlone() throws {
        let doc = try Fixture.open("AFU")
        let document = TestSupport.systemDocument(of: doc)
        let controller = document.model.file
        let directory = FileManager.default.temporaryDirectory.appendingPathComponent("epro-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: directory) }
        TestSupport.turnRunLoop()
        let before = document.isDocumentEdited
        try controller.writeEasyEDAPro(to: directory.appendingPathComponent("Board.epro"))
        XCTAssertEqual(try FileManager.default.contentsOfDirectory(atPath: directory.path), ["Board.epro"])
        XCTAssertNil(document.fileURL)
        XCTAssertNil(controller.fileURL)
        TestSupport.turnRunLoop()
        XCTAssertEqual(document.isDocumentEdited, before, "an export is not a save, and not an edit")
        XCTAssertFalse(doc.undoManager.canUndo)
    }
}
