import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The pick-and-place CSV in the Gerber export, and the JLCPCB / PCBWay BOM.
@MainActor
final class PickAndPlaceExportTests: XCTestCase {
    private let rules = DesignRules.pcbway

    /// A surface-mount resistor, a surface-mount SOIC and a through-hole DIP, each placed on the board.
    private func board() throws -> (doc: SchematicDocument, smd: PlacedChip, soic: PlacedChip, dip: PlacedChip) {
        let doc = TestSupport.newDocument()
        let pull = try TestSupport.place(doc, "PULLUP-5V", at: CGPoint(x: 100, y: 100))
        doc.setPartFootprint(pull.id, "RES-0805")
        let soic = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 300, y: 100))
        let dip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 500, y: 100))
        func chip(_ id: UUID) throws -> PlacedChip { try XCTUnwrap(doc.chips.first { $0.id == id }) }
        return (doc, try chip(pull.id), try chip(soic.id), try chip(dip.id))
    }

    func testOnlySurfaceMountPartsAreListed() throws {
        let (doc, smd, soic, dip) = try board()
        XCTAssertTrue(smd.isSurfaceMount)
        XCTAssertTrue(soic.isSurfaceMount)
        XCTAssertFalse(dip.isSurfaceMount)
        let rows = PickAndPlaceExport.rows(for: doc.chips, top: 50)
        XCTAssertEqual(rows.map(\.designator), [smd.reference, soic.reference].sorted {
            $0.localizedStandardCompare($1) == .orderedAscending
        })
    }

    /// A row is the footprint's centre, y up from the board's bottom edge, the angle counter-clockwise.
    func testRowPositionLayerAndRotation() throws {
        let (doc, smd, _, _) = try board()
        let centre = try XCTUnwrap(smd.board).centre
        doc.rotateFootprint(smd.id)
        let turned = try XCTUnwrap(doc.chips.first { $0.id == smd.id })
        XCTAssertEqual(turned.board?.rotation, 90)
        let row = try XCTUnwrap(PickAndPlaceExport.rows(for: [turned], top: 60).first)
        XCTAssertEqual(row.rotation, 270, "90° clockwise is 270° counter-clockwise")
        XCTAssertFalse(row.bottom)
        XCTAssertEqual(row.y, 60 - Double(try XCTUnwrap(turned.board).centre.y), accuracy: 1e-9)
        XCTAssertNotEqual(row.x, Double(centre.x) + 1000)

        doc.setFootprintSide(smd.id, .bottom)
        let flipped = try XCTUnwrap(doc.chips.first { $0.id == smd.id })
        let bottom = try XCTUnwrap(PickAndPlaceExport.rows(for: [flipped], top: 60).first)
        XCTAssertTrue(bottom.bottom)
        XCTAssertEqual(bottom.rotation, (360 - (flipped.board?.rotation ?? 0)) % 360)
    }

    func testCSVLayout() {
        let rows = [PickAndPlaceExport.Row(designator: "R1", x: 12.5, y: 3.25, bottom: false, rotation: 90),
                    PickAndPlaceExport.Row(designator: "U2", x: 1, y: 2.12345, bottom: true, rotation: 0)]
        XCTAssertEqual(PickAndPlaceExport.csv(rows),
                       "Designator,Mid X,Mid Y,Layer,Rotation\r\nR1,12.5,3.25,Top,90\r\nU2,1,2.1235,Bottom,0\r\n")
    }

    /// The CSV is in the Gerber zip, before the job file, and the job file does not list it.
    func testGerberSetHasTheCSVWhenThereAreSurfaceMountParts() throws {
        let empty = TestSupport.newDocument()
        XCTAssertFalse(GerberExport.files(for: empty, base: "b", rules: rules).contains { $0.name == "b-CPL.csv" })

        let (doc, _, _, _) = try board()
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        XCTAssertEqual(files.map(\.name).suffix(2), ["b-CPL.csv", "b-job.gbrjob"])
        let cpl = try XCTUnwrap(files.first { $0.name == "b-CPL.csv" })
        XCTAssertTrue(cpl.text.hasPrefix("Designator,Mid X,Mid Y,Layer,Rotation\r\n"))
        XCTAssertEqual(cpl.text.split(separator: "\r\n").count, 3)

        let job = try XCTUnwrap(files.last)
        let json = try XCTUnwrap(JSONSerialization.jsonObject(with: job.data) as? [String: Any])
        let listed = try XCTUnwrap(json["FilesAttributes"] as? [[String: Any]]).compactMap { $0["Path"] as? String }
        XCTAssertFalse(listed.contains("b-CPL.csv"))
        XCTAssertTrue(listed.contains("b-PTH.drl"))
    }

    // MARK: BOM

    func testJLCPCBBOMListsSurfaceMountLines() throws {
        let (doc, _, soic, _) = try board()
        let lines = BillOfMaterials.lines(for: doc.chips)
        let csv = BillOfMaterials.jlcpcbCSV(lines)
        let rows = csv.components(separatedBy: "\r\n")
        XCTAssertEqual(rows[0], "\"Comment\",\"Designator\",\"Footprint\",\"Quantity\",\"Manufacturer\","
                       + "\"Manufacturer Part Number\",\"LCSC Part #\"")
        XCTAssertEqual(rows.count, 4, "header, two surface-mount lines, trailing break")
        XCTAssertTrue(rows[1].hasPrefix("\"1kΩ\",\"R1\",\"0805\",\"1\",\"\",\"\",\"\""), rows[1])
        XCTAssertTrue(rows[2].contains(",\"\(soic.reference)\","), rows[2])
        XCTAssertTrue(rows[2].hasPrefix("\"SN74HC00D\","), rows[2])
        XCTAssertTrue(rows[2].hasSuffix(",\"SN74HC00D\",\"\""), rows[2])
        XCTAssertFalse(csv.contains("SN74HC00N"), "through-hole lines are left out")
    }

    func testJLCPCBBOMSplitsMoreThan200Designators() {
        let references = (1...450).map { "C\($0)" }
        let line = BillOfMaterialsLine(partNumber: "CAPACITOR", package: "0603", value: "100 nF", color: "",
                                       description: "Capacitor", references: references, isSurfaceMount: true)
        let rows = BillOfMaterials.jlcpcbCSV([line]).components(separatedBy: "\r\n").dropFirst().dropLast()
        XCTAssertEqual(rows.count, 3)
        XCTAssertTrue(rows.first!.contains("\"200\""))
        XCTAssertTrue(rows.last!.contains("\"50\""))
        XCTAssertTrue(rows.first!.contains("\"C1,C2,C3,"), "designators joined by a bare comma")
    }
}
