import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Pours and keep-outs in the exports (plan-52-G): Gerber regions, KiCad zones
/// and rule areas, EasyEDA Pro `POUR` lines.
@MainActor
final class PourExportTests: XCTestCase {
    private let rules = DesignRules.pcbway

    private func text(_ files: [GerberExport.File], _ suffix: String) throws -> String {
        try XCTUnwrap(files.first { $0.name.hasSuffix(suffix) }, "no \(suffix)").text
    }

    private func kicad(_ doc: SchematicDocument) throws -> String {
        let file = try XCTUnwrap(KiCadExport.files(for: doc, base: "b").first { $0.name.hasSuffix("kicad_pcb") })
        return String(decoding: file.data, as: UTF8.self)
    }

    private func count(_ needle: String, in text: String) -> Int { text.components(separatedBy: needle).count - 1 }

    /// The area (mm²) of every kept piece of every pour's fill: outer loops less holes.
    private func fillArea(_ doc: SchematicDocument) -> Double {
        var total = 0.0
        for fill in doc.connectivity().pourFills.values {
            for piece in fill.pieces {
                let loops = GerberOutline.contours(of: piece).sorted { abs(GerberWriter.area($0)) > abs(GerberWriter.area($1)) }
                guard let outer = loops.first else { continue }
                total += abs(GerberWriter.area(outer)) - loops.dropFirst().reduce(0) { $0 + abs(GerberWriter.area($1)) }
            }
        }
        return total
    }

    // MARK: Gerber

    func testGerberTopCopperHasTheFillAsRegionsWithItsHolesCleared() throws {
        let doc = try ExportSamples.pours()
        let bare = try ExportSamples.pours()
        for id in bare.pours.map(\.id) { bare.deletePour(id) }
        for id in bare.keepouts.map(\.id) { bare.deleteKeepout(id) }
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        let baseline = GerberExport.files(for: bare, base: "b", rules: rules)
        let top = try text(files, ".GTL"), topBare = try text(baseline, ".GTL")
        let lines = top.split(separator: "\n")
        let region = try XCTUnwrap(lines.firstIndex(of: "G36*"))
        let clear = try XCTUnwrap(lines.firstIndex(of: "%LPC*%"))
        let flash = try XCTUnwrap(lines.firstIndex { $0.hasSuffix("D03*") })
        XCTAssertLessThan(region, clear)
        XCTAssertLessThan(clear, flash, "the pour is drawn before the pads and traces")
        XCTAssertGreaterThan(count("G36*", in: top), count("G36*", in: topBare))
        let ground = try XCTUnwrap(top.range(of: "%TO.N,")).upperBound
        XCTAssertTrue(top[ground...].hasPrefix("GND") || top[ground...].hasPrefix("/GND"), "the pour's net is ground: \(top[ground...].prefix(12))")
        // The Bottom pour is its own region on .GBL, and the bare board has none.
        XCTAssertGreaterThan(count("G36*", in: try text(files, ".GBL")), count("G36*", in: try text(baseline, ".GBL")))
    }

    func testAPourOnTopLeavesBottomCopperAlone() throws {
        let doc = try ExportSamples.pours()
        let top = try XCTUnwrap(doc.pours.first { $0.layer == .top })
        let bottom = try XCTUnwrap(doc.pours.first { $0.layer == .bottom })
        let withBoth = try text(GerberExport.files(for: doc, base: "b", rules: rules), ".GBL")
        doc.deletePour(top.id)
        XCTAssertEqual(try text(GerberExport.files(for: doc, base: "b", rules: rules), ".GBL"), withBoth)
        doc.deletePour(bottom.id)
        XCTAssertNotEqual(try text(GerberExport.files(for: doc, base: "b", rules: rules), ".GBL"), withBoth)
    }

    func testAKeepoutRemovesItsAreaFromTheFill() throws {
        let doc = try ExportSamples.pours()
        let keepout = try XCTUnwrap(doc.keepouts.first)
        let with = fillArea(doc)
        doc.deleteKeepout(keepout.id)
        let without = fillArea(doc)
        XCTAssertEqual(without - with, keepout.rect.width * keepout.rect.height, accuracy: 1.5)
    }

    func testExistingBoardsExportTheSameBytesAfterAPourComesAndGoes() throws {
        let doc = try Fixture.open("AFU")
        let gerber = GerberExport.files(for: doc, base: "b", rules: rules).map(\.data)
        let kicad = KiCadExport.files(for: doc, base: "b").map(\.data)
        let easy = EasyEDAProExport.entries(for: doc, base: "b").map(\.data)
        let id = try XCTUnwrap(doc.addPour(rect: CGRect(x: 0, y: 0, width: 20, height: 20), layer: .top))
        let keepout = doc.addKeepout(rect: CGRect(x: 5, y: 5, width: 5, height: 5))
        doc.deletePour(id)
        doc.deleteKeepout(keepout)
        XCTAssertEqual(GerberExport.files(for: doc, base: "b", rules: rules).map(\.data), gerber)
        XCTAssertEqual(KiCadExport.files(for: doc, base: "b").map(\.data), kicad)
        XCTAssertEqual(EasyEDAProExport.entries(for: doc, base: "b").map(\.data), easy)
    }

    // MARK: KiCad

    func testKiCadHasAZonePerPourAndARuleAreaPerKeepout() throws {
        let doc = try ExportSamples.pours()
        let text = try kicad(doc)
        XCTAssertEqual(count("\t(zone\n", in: text), 3, "two pours and a keep-out")
        XCTAssertEqual(count("(connect_pads yes", in: text), 1, "the solid pour")
        XCTAssertEqual(count("(connect_pads\n", in: text), 2, "the thermal pour and the rule area")
        XCTAssertEqual(count("(copperpour not_allowed)", in: text), 1)
        XCTAssertEqual(count("(tracks allowed)", in: text), 1)
        XCTAssertTrue(text.contains("(layers \"F.Cu\")"))
        XCTAssertTrue(text.contains("(thermal_gap 0.6)"))
        XCTAssertTrue(text.contains("(thermal_bridge_width 0.4)"))
        XCTAssertTrue(text.contains("(min_thickness 0.25)"))
        XCTAssertTrue(text.contains("(layer \"B.Cu\")"))
    }

    func testAPourWithNoNetWritesNoZone() throws {
        let doc = try ExportSamples.pours()
        let top = try XCTUnwrap(doc.pours.first { $0.layer == .top })
        doc.setPourNet(top.id, nil)
        XCTAssertEqual(count("\t(zone\n", in: try kicad(doc)), 2)
        doc.setPourNet(top.id, .rail(mv: 9000))
        XCTAssertEqual(count("\t(zone\n", in: try kicad(doc)), 2, "a net the schematic doesn't have")
    }

    func testAKeepoutNamesEveryLayerItAppliesTo() throws {
        let doc = try ExportSamples.pours()
        doc.setLayerCount(4)
        let keepout = try XCTUnwrap(doc.keepouts.first)
        doc.setKeepoutLayers(keepout.id, nil)
        XCTAssertTrue(try kicad(doc).contains("(layers \"F.Cu\" \"In1.Cu\" \"In2.Cu\" \"B.Cu\")"))
        doc.setKeepoutLayers(keepout.id, [])
        XCTAssertEqual(count("(copperpour not_allowed)", in: try kicad(doc)), 0, "no layer: it blocks nothing, so no rule area")
    }

    func testKiCadExportIsDeterministicWithPours() throws {
        let doc = try ExportSamples.pours()
        XCTAssertEqual(try kicad(doc), try kicad(doc))
    }

    // MARK: EasyEDA Pro

    func testEasyEDAGetsOnePourLinePerPourAndNoKeepout() throws {
        let doc = try ExportSamples.pours()
        func pours(_ doc: SchematicDocument) throws -> Int {
            let entries = EasyEDAProExport.entries(for: doc, base: "p")
            let board = try XCTUnwrap(entries.first { $0.name.hasSuffix(".epcb") })
            return String(decoding: board.data, as: UTF8.self).split(separator: "\n").filter { $0.hasPrefix("[\"POUR\"") }.count
        }
        XCTAssertEqual(try pours(doc), 2)
        let top = try XCTUnwrap(doc.pours.first)
        doc.deletePour(top.id)
        XCTAssertEqual(try pours(doc), 1)
        for id in doc.keepouts.map(\.id) { doc.deleteKeepout(id) }
        XCTAssertEqual(try pours(doc), 1, "a keep-out adds no line")
    }
}
